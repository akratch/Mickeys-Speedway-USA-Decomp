#!/usr/bin/env python3
"""Shared, file-based coordination for the three-session Codex crew.

Runtime state lives under the repository's Git common directory, so every
worktree sees the same mailbox while no coordination message can be committed.
See ADRs 0013 and 0014 for the ownership and lifecycle rules.
"""

from __future__ import annotations

import argparse
import datetime as dt
from fractions import Fraction
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys
import tempfile
import uuid


SCHEMA_VERSION = 1
HEARTBEAT_SCHEMA_VERSION = 4
MEASURED_HEARTBEAT_SCHEMA_VERSION = 3
CHECKPOINT_HEARTBEAT_SCHEMA_VERSION = 2
LEGACY_HEARTBEAT_SCHEMA_VERSION = 1
HEARTBEAT_STATES = ("active", "stopping", "handoff", "complete", "failed")
MEASUREMENT_FIELDS = (
    "target_words",
    "candidate_words",
    "raw_differing_words",
    "relocation_masked_differing_words",
    "candidate_relocations",
    "target_relocations",
    "exact_relocation_identities",
)
PROMOTION_STATES = (
    "unmeasured",
    "compiled",
    "object-exact",
    "canonical-staged",
    "rom-exact",
)
EXACT_PROMOTION_STATES = frozenset(PROMOTION_STATES[2:])
WB_SUMMARY_SCHEMA = "mickey-wb-summary-v1"
WB_SUMMARY_MAX_BYTES = 64 * 1024
WB_SUMMARY_MAX_AGE_SECONDS = 15 * 60
WB_SUMMARY_FUTURE_SLOP_SECONDS = 5 * 60
BEST_ARTIFACT_SCHEMA = "mickey-best-artifact-v1"
BEST_BACKUP_SCHEMA = "mickey-best-backup-v1"
BEST_ARTIFACT_ID_RE = re.compile(r"^[0-9a-f]{24}$")
BEST_BACKUP_ID_RE = re.compile(r"^[0-9]{8}T[0-9]{6}Z-[0-9a-f]{12}$")
SOURCE_ARCHIVE_MAX_BYTES = 8 * 1024 * 1024
CANDIDATE_ARCHIVE_MAX_BYTES = 64 * 1024 * 1024
STATES = (
    "ACTIVE",
    "READY",
    "ASSIGNED",
    "ACKED",
    "WORKING",
    "HANDOFF",
    "PIPELINED",
    "INTEGRATED",
    "RELEASED",
    "PLATEAU",
    "BLOCKED",
    "IDLE",
)
MESSAGE_TYPES = (
    "ASSIGN",
    "ACK",
    "UPDATE",
    "QUESTION",
    "ANSWER",
    "HANDOFF",
    "PIPELINE",
    "INTEGRATED",
    "RELEASE",
    "RELEASED",
    "BLOCKED",
)
NAME_RE = re.compile(r"^[a-z0-9][a-z0-9-]{0,31}$")
TASK_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")
COMMIT_RE = re.compile(r"^[0-9a-f]{7,40}$")
MISMATCH_CLASS_RE = re.compile(r"^[a-z][a-z0-9+._/-]{0,63}$")


class CrewError(RuntimeError):
    """A concise, user-actionable crew-control error."""


def run_git(*args: str) -> str:
    result = subprocess.run(
        ["git", *args],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        detail = result.stderr.strip() or "git command failed"
        raise CrewError(f"git {' '.join(args)}: {detail}")
    return result.stdout.strip()


def default_crew_root() -> Path:
    common = run_git("rev-parse", "--path-format=absolute", "--git-common-dir")
    return Path(common).resolve() / "codex-crew"


def primary_checkout_root() -> Path:
    common = Path(run_git("rev-parse", "--path-format=absolute", "--git-common-dir")).resolve()
    if common.name != ".git":
        raise CrewError(f"expected a non-bare repository with a .git common directory, got {common}")
    return common.parent


def utc_now() -> dt.datetime:
    return dt.datetime.now(dt.timezone.utc)


def timestamp(value: dt.datetime | None = None) -> str:
    return (value or utc_now()).isoformat(timespec="seconds").replace("+00:00", "Z")


def parse_unix_timestamp(value: int, label: str) -> dt.datetime:
    try:
        return dt.datetime.fromtimestamp(value, dt.timezone.utc)
    except (OverflowError, OSError, ValueError) as error:
        raise CrewError(f"{label} is out of range") from error


def clean_line(value: str) -> str:
    return " ".join(value.replace("\x00", "").splitlines()).strip()


def concise_line(value: str, label: str, limit: int = 160) -> str:
    cleaned = clean_line(value)
    if not cleaned or len(cleaned) > limit or "\t" in value:
        raise CrewError(f"{label} must be one concise line")
    return cleaned


def stored_concise_line(value: str, label: str, limit: int = 160) -> str:
    cleaned = concise_line(value, label, limit)
    if cleaned != value:
        raise CrewError(f"{label} is not stored as one canonical concise line")
    return cleaned


def validate_name(value: str, label: str) -> str:
    if not NAME_RE.fullmatch(value):
        raise CrewError(f"invalid {label} {value!r}; use lowercase letters, digits, and hyphens")
    return value


def validate_task(value: str) -> str:
    if not TASK_RE.fullmatch(value):
        raise CrewError(f"invalid task id {value!r}")
    return value


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temp_path = Path(temporary)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_path, path)
    finally:
        temp_path.unlink(missing_ok=True)


def heartbeat_path(root: Path, worker: str) -> Path:
    return root / "heartbeats" / f"{validate_name(worker, 'heartbeat worker')}.json"


def unmeasured_result() -> dict[str, object]:
    return {
        **{field: None for field in MEASUREMENT_FIELDS},
        "promotion_state": "unmeasured",
    }


def validate_measured_result(
    result: dict[str, object], label: str = "heartbeat measured result"
) -> dict[str, object]:
    """Validate and return one normalized, code-free compiler result."""

    promotion_state = result.get("promotion_state")
    if promotion_state not in PROMOTION_STATES:
        raise CrewError(f"{label} has invalid promotion_state")
    present = [result.get(field) is not None for field in MEASUREMENT_FIELDS]
    if not any(present):
        if promotion_state != "unmeasured":
            raise CrewError(f"{label} is unmeasured but promotion_state is not unmeasured")
        return unmeasured_result()
    if not all(present):
        missing = [field for field in MEASUREMENT_FIELDS if result.get(field) is None]
        raise CrewError(f"{label} is incomplete; missing " + ", ".join(missing))

    normalized: dict[str, object] = {}
    for field in MEASUREMENT_FIELDS:
        value = result.get(field)
        if type(value) is not int or int(value) < 0:
            raise CrewError(f"{label} has invalid {field}")
        normalized[field] = int(value)
    if promotion_state == "unmeasured":
        raise CrewError(f"{label} has metrics but promotion_state is unmeasured")
    normalized["promotion_state"] = promotion_state

    target_words = int(normalized["target_words"])
    candidate_words = int(normalized["candidate_words"])
    raw = int(normalized["raw_differing_words"])
    masked = int(normalized["relocation_masked_differing_words"])
    candidate_relocations = int(normalized["candidate_relocations"])
    target_relocations = int(normalized["target_relocations"])
    exact_identities = int(normalized["exact_relocation_identities"])
    word_surface = max(target_words, candidate_words)
    if raw > word_surface or masked > word_surface:
        raise CrewError(f"{label} has differences larger than its word geometry")
    if masked > raw:
        raise CrewError(f"{label} has more relocation-masked than raw differences")
    if exact_identities > min(candidate_relocations, target_relocations):
        raise CrewError(f"{label} has too many exact relocation identities")
    if promotion_state in EXACT_PROMOTION_STATES and not (
        target_words == candidate_words
        and raw == 0
        and masked == 0
        and candidate_relocations == target_relocations == exact_identities
    ):
        raise CrewError(
            f"{label} claims {promotion_state} without exact words and relocation identities"
        )
    return normalized


def validate_measurement_object(value: object, label: str) -> dict[str, object]:
    expected = {*MEASUREMENT_FIELDS, "promotion_state"}
    if not isinstance(value, dict) or set(value) != expected:
        raise CrewError(f"{label} has an unsupported measurement shape")
    return validate_measured_result(value, label)


def measured_score(result: dict[str, object]) -> str:
    if result["promotion_state"] == "unmeasured":
        raise CrewError("cannot format an unmeasured result")
    return (
        f"{result['relocation_masked_differing_words']}/{result['target_words']} "
        f"relocation-masked differing words ({result['raw_differing_words']} raw; "
        f"candidate {result['candidate_words']})"
    )


def measurement_rank(result: dict[str, object]) -> tuple[object, ...]:
    """Return the documented deterministic best-candidate ordering key."""

    normalized = validate_measured_result(result, "candidate measurement")
    state = str(normalized["promotion_state"])
    if state == "unmeasured":
        return (0, 0, 0, 0, 0, Fraction(0, 1), 0, 0, 0)
    target_words = int(normalized["target_words"])
    candidate_words = int(normalized["candidate_words"])
    raw = int(normalized["raw_differing_words"])
    masked = int(normalized["relocation_masked_differing_words"])
    candidate_relocations = int(normalized["candidate_relocations"])
    target_relocations = int(normalized["target_relocations"])
    exact_identities = int(normalized["exact_relocation_identities"])
    relocation_surface = max(candidate_relocations, target_relocations)
    return (
        PROMOTION_STATES.index(state),
        int(target_words == candidate_words),
        -masked,
        -raw,
        int(candidate_relocations == target_relocations == exact_identities),
        Fraction(exact_identities, relocation_surface or 1),
        int(candidate_relocations == target_relocations),
        exact_identities,
        -abs(candidate_words - target_words),
    )


def better_measurement(current: dict[str, object], best: dict[str, object]) -> bool:
    """Return true only for a strict improvement; exact ties retain the older best."""

    return measurement_rank(current) > measurement_rank(best)


def validate_best_artifact_reference(value: object, label: str) -> dict[str, str] | None:
    if value is None:
        return None
    if not isinstance(value, dict) or set(value) != {"id", "manifest_sha256"}:
        raise CrewError(f"{label} has malformed best_artifact")
    artifact_id = value.get("id")
    manifest_hash = value.get("manifest_sha256")
    if not isinstance(artifact_id, str) or not BEST_ARTIFACT_ID_RE.fullmatch(artifact_id):
        raise CrewError(f"{label} has invalid best artifact id")
    if not isinstance(manifest_hash, str) or not re.fullmatch(r"[0-9a-f]{64}", manifest_hash):
        raise CrewError(f"{label} has invalid best artifact manifest hash")
    return {"id": artifact_id, "manifest_sha256": manifest_hash}


def load_heartbeat(path: Path) -> dict[str, object]:
    try:
        record = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as error:
        raise CrewError(f"cannot read heartbeat {path}: {error}") from error
    if not isinstance(record, dict):
        raise CrewError(f"heartbeat {path} must contain one JSON object")
    schema = record.get("schema")
    if type(schema) is not int or schema not in (
        LEGACY_HEARTBEAT_SCHEMA_VERSION,
        CHECKPOINT_HEARTBEAT_SCHEMA_VERSION,
        MEASURED_HEARTBEAT_SCHEMA_VERSION,
        HEARTBEAT_SCHEMA_VERSION,
    ):
        raise CrewError(f"unsupported heartbeat schema in {path}")
    required = {
        "worker": str,
        "target": str,
        "base": str,
        "last_progress": str,
        "last_progress_at": str,
        "last_commit": str,
        "deadline_unix": int,
        "state": str,
        "updated_at": str,
    }
    if schema in (
        CHECKPOINT_HEARTBEAT_SCHEMA_VERSION,
        MEASURED_HEARTBEAT_SCHEMA_VERSION,
        HEARTBEAT_SCHEMA_VERSION,
    ):
        required.update({
            "attempt_count": int,
            "best_score": str,
            "mismatch_class": str,
            "eta_unix": (int, type(None)),
        })
    if schema in (MEASURED_HEARTBEAT_SCHEMA_VERSION, HEARTBEAT_SCHEMA_VERSION):
        required.update({
            **{field: (int, type(None)) for field in MEASUREMENT_FIELDS},
            "promotion_state": str,
        })
    if schema == HEARTBEAT_SCHEMA_VERSION:
        required.update({
            "current_score": str,
            "best_result": dict,
            "best_mismatch_class": str,
            "best_artifact": (dict, type(None)),
        })
    unexpected = sorted(set(record) - {"schema", *required})
    if unexpected:
        raise CrewError(f"heartbeat {path} has unknown field(s): {', '.join(unexpected)}")
    for field, field_type in required.items():
        value = record.get(field)
        if field in ("deadline_unix", "attempt_count"):
            valid = type(value) is int
        elif field == "eta_unix":
            valid = value is None or type(value) is int
        elif field in MEASUREMENT_FIELDS:
            valid = value is None or type(value) is int
        else:
            valid = isinstance(value, field_type)
        if not valid:
            raise CrewError(f"heartbeat {path} has invalid {field}")
    worker = validate_name(str(record["worker"]), "heartbeat worker")
    if worker != path.stem:
        raise CrewError(
            f"heartbeat {path} names worker {worker!r}; expected {path.stem!r} from its filename"
        )
    stored_concise_line(str(record["target"]), "heartbeat target", 128)
    stored_concise_line(str(record["last_progress"]), "heartbeat progress")
    if not COMMIT_RE.fullmatch(str(record["base"])):
        raise CrewError(f"heartbeat {path} has invalid base")
    if not COMMIT_RE.fullmatch(str(record["last_commit"])):
        raise CrewError(f"heartbeat {path} has invalid last_commit")
    if record["state"] not in HEARTBEAT_STATES:
        raise CrewError(f"heartbeat {path} has invalid state")
    if int(record["deadline_unix"]) <= 0:
        raise CrewError(f"heartbeat {path} has invalid deadline_unix")
    parse_unix_timestamp(int(record["deadline_unix"]), f"heartbeat {path} deadline_unix")
    last_progress_at = parse_timestamp(str(record["last_progress_at"]))
    updated_at = parse_timestamp(str(record["updated_at"]))
    if updated_at < last_progress_at:
        raise CrewError(f"heartbeat {path} has updated_at before last_progress_at")

    if schema in (
        CHECKPOINT_HEARTBEAT_SCHEMA_VERSION,
        MEASURED_HEARTBEAT_SCHEMA_VERSION,
        HEARTBEAT_SCHEMA_VERSION,
    ):
        attempt_count = record.get("attempt_count")
        if type(attempt_count) is not int or attempt_count < 0:
            raise CrewError(f"heartbeat {path} has invalid attempt_count")
        stored_concise_line(str(record["best_score"]), "heartbeat best_score", 128)
        mismatch_class = record.get("mismatch_class")
        if not isinstance(mismatch_class, str) or not MISMATCH_CLASS_RE.fullmatch(mismatch_class):
            raise CrewError(f"heartbeat {path} has invalid mismatch_class")
        eta_unix = record.get("eta_unix")
        if eta_unix is not None and (type(eta_unix) is not int or eta_unix <= 0):
            raise CrewError(f"heartbeat {path} has invalid eta_unix")
        if eta_unix is not None:
            parse_unix_timestamp(int(eta_unix), f"heartbeat {path} eta_unix")
    else:
        # Schema 1 remains readable so running workers can update in place.
        # The next update rewrites the record as schema 4 with explicit
        # checkpoint defaults.
        record = dict(record)
        record.update({
            "attempt_count": 0,
            "best_score": "not recorded",
            "mismatch_class": "unclassified",
            "eta_unix": None,
        })
    if schema in (MEASURED_HEARTBEAT_SCHEMA_VERSION, HEARTBEAT_SCHEMA_VERSION):
        result = validate_measured_result(record, f"heartbeat {path} measured result")
        record = dict(record)
        record.update(result)
        if result["promotion_state"] != "unmeasured":
            expected_score = measured_score(result)
            score_field = "current_score" if schema == HEARTBEAT_SCHEMA_VERSION else "best_score"
            if record[score_field] != expected_score:
                raise CrewError(f"heartbeat {path} has noncanonical measured {score_field}")
    else:
        # Schemas 1 and 2 had no unambiguous numeric result. They remain
        # readable and their next update is normalized to schema 4.
        record = dict(record)
        record.update(unmeasured_result())
    if schema == HEARTBEAT_SCHEMA_VERSION:
        stored_concise_line(str(record["current_score"]), "heartbeat current_score", 128)
        best_result_value = record["best_result"]
        if not isinstance(best_result_value, dict):
            raise CrewError(f"heartbeat {path} has invalid best_result")
        best_result = validate_measurement_object(
            best_result_value, f"heartbeat {path} best result"
        )
        best_mismatch = record["best_mismatch_class"]
        if not isinstance(best_mismatch, str) or not MISMATCH_CLASS_RE.fullmatch(best_mismatch):
            raise CrewError(f"heartbeat {path} has invalid best_mismatch_class")
        best_artifact = validate_best_artifact_reference(record["best_artifact"], f"heartbeat {path}")
        if best_result["promotion_state"] != "unmeasured":
            if record["best_score"] != measured_score(best_result):
                raise CrewError(f"heartbeat {path} has noncanonical measured best_score")
        elif best_artifact is not None:
            raise CrewError(f"heartbeat {path} archives an unmeasured best result")
        if measurement_rank(best_result) < measurement_rank(result):
            raise CrewError(f"heartbeat {path} current result outranks its recorded best")
        record = dict(record)
        record["best_result"] = best_result
        record["best_artifact"] = best_artifact
    else:
        # Schema 3's sole measurement was both current and best.  Schemas 1/2
        # have no numeric result, but their prose remains visible until the
        # next schema-4 update.
        record = dict(record)
        record["current_score"] = record["best_score"]
        record["best_result"] = {
            field: record[field] for field in (*MEASUREMENT_FIELDS, "promotion_state")
        }
        record["best_mismatch_class"] = record["mismatch_class"]
        record["best_artifact"] = None
    return record


def parse_timestamp(value: str) -> dt.datetime:
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as error:
        raise CrewError(f"invalid heartbeat timestamp {value!r}") from error
    if parsed.tzinfo is None:
        raise CrewError(f"heartbeat timestamp lacks a timezone: {value!r}")
    return parsed.astimezone(dt.timezone.utc)


def heartbeat_staleness(
    record: dict[str, object], now: dt.datetime, stale_after: dt.timedelta
) -> list[str]:
    if record["state"] not in ("active", "stopping"):
        return []
    reasons: list[str] = []
    last_progress = parse_timestamp(str(record["last_progress_at"]))
    if last_progress - now > dt.timedelta(minutes=5):
        minutes = int((last_progress - now).total_seconds() // 60)
        reasons.append(f"progress timestamp is {minutes}m in the future")
    if now - last_progress > stale_after:
        minutes = int((now - last_progress).total_seconds() // 60)
        reasons.append(f"no progress for {minutes}m")
    deadline = parse_unix_timestamp(int(record["deadline_unix"]), "heartbeat deadline_unix")
    if now > deadline:
        minutes = int((now - deadline).total_seconds() // 60)
        reasons.append(f"deadline passed {minutes}m ago")
    eta_unix = record.get("eta_unix")
    if eta_unix is not None:
        eta = parse_unix_timestamp(int(eta_unix), "heartbeat eta_unix")
        if now > eta:
            minutes = int((now - eta).total_seconds() // 60)
            reasons.append(f"ETA passed {minutes}m ago")
    return reasons


def graceful_stop_guidance(worker: str) -> str:
    return (
        f"ask {worker} to finish its current bounded call, guard the best candidate, "
        "run tools/finalize_plateau.py, and hand off; if it is unresponsive, report "
        "the exact launcher PID for owner inspection. crew.py never signals or stops a process."
    )


def repository_root() -> Path:
    value = run_git("rev-parse", "--show-toplevel")
    root = Path(value)
    if not root.is_absolute():
        raise CrewError("git returned a non-absolute repository root")
    return root.resolve()


def run_git_in(root: Path, *args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", *args], cwd=root, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        detail = result.stderr.strip() or "git command failed"
        raise CrewError(f"git {' '.join(args)}: {detail}")
    return result


def current_branch(root: Path) -> str:
    result = run_git_in(root, "symbolic-ref", "--quiet", "--short", "HEAD", check=False)
    branch = result.stdout.strip()
    if result.returncode != 0 or not branch or "\n" in branch:
        raise CrewError("best-artifact operations require an attached branch")
    return branch


def canonical_relative_path(value: str, label: str) -> Path:
    supplied = Path(value)
    if supplied.is_absolute() or not supplied.parts or any(
        part in ("", ".", "..") for part in supplied.parts
    ) or supplied.as_posix() != value:
        raise CrewError(f"{label} must be a canonical repository-relative path")
    return supplied


def read_regular_path(path: Path, *, label: str, maximum: int) -> tuple[bytes, int]:
    """Read a bounded regular file without following a final symlink."""

    try:
        metadata = path.lstat()
    except OSError as error:
        raise CrewError(f"cannot inspect {label}") from error
    if stat.S_ISLNK(metadata.st_mode) or not stat.S_ISREG(metadata.st_mode):
        raise CrewError(f"{label} is a symlink or is not a regular file")
    if metadata.st_size > maximum:
        raise CrewError(f"{label} is unexpectedly large")
    flags = os.O_RDONLY | getattr(os, "O_CLOEXEC", 0) | getattr(os, "O_NOFOLLOW", 0)
    try:
        descriptor = os.open(path, flags)
    except OSError as error:
        raise CrewError(f"cannot open {label}") from error
    try:
        before = os.fstat(descriptor)
        chunks: list[bytes] = []
        total = 0
        while total <= maximum:
            chunk = os.read(descriptor, min(64 * 1024, maximum + 1 - total))
            if not chunk:
                break
            chunks.append(chunk)
            total += len(chunk)
        after = os.fstat(descriptor)
    finally:
        os.close(descriptor)
    data = b"".join(chunks)
    if len(data) > maximum:
        raise CrewError(f"{label} is unexpectedly large")
    try:
        current = path.lstat()
    except OSError as error:
        raise CrewError(f"{label} changed while it was read") from error
    identity = lambda value: (value.st_dev, value.st_ino, value.st_size, value.st_mtime_ns)
    if identity(metadata) != identity(before) or identity(before) != identity(after) \
            or identity(after) != identity(current):
        raise CrewError(f"{label} changed while it was read")
    return data, stat.S_IMODE(before.st_mode)


def read_repository_file(
    root: Path, relative: str, *, label: str, maximum: int
) -> tuple[bytes, int, Path]:
    supplied = canonical_relative_path(relative, label)
    path = root
    for part in supplied.parts[:-1]:
        path /= part
        try:
            metadata = path.lstat()
        except OSError as error:
            raise CrewError(f"cannot inspect {label} path") from error
        if stat.S_ISLNK(metadata.st_mode) or not stat.S_ISDIR(metadata.st_mode):
            raise CrewError(f"{label} path contains a symlink or non-directory")
    path /= supplied.parts[-1]
    data, mode = read_regular_path(path, label=label, maximum=maximum)
    return data, mode, path


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_bytes(path: Path, data: bytes, mode: int = 0o600) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temp_path = Path(temporary)
    try:
        with os.fdopen(descriptor, "wb") as handle:
            handle.write(data)
            handle.flush()
            os.fsync(handle.fileno())
        os.chmod(temp_path, mode)
        os.replace(temp_path, path)
    finally:
        temp_path.unlink(missing_ok=True)


def artifact_root(crew_root: Path, worker: str, artifact_id: str) -> Path:
    validate_name(worker, "heartbeat worker")
    if not BEST_ARTIFACT_ID_RE.fullmatch(artifact_id):
        raise CrewError("invalid best artifact id")
    return crew_root / "best-artifacts" / worker / artifact_id


def require_directory(path: Path, label: str) -> None:
    try:
        mode = path.lstat().st_mode
    except OSError as error:
        raise CrewError(f"{label} is missing") from error
    if stat.S_ISLNK(mode) or not stat.S_ISDIR(mode):
        raise CrewError(f"{label} is a symlink or non-directory")


def archive_best_artifact(
    crew_root: Path,
    *,
    worker: str,
    target: str,
    base: str,
    source_relative: str,
    object_relative: str,
    summary_relative: str,
    measurement: dict[str, object],
) -> dict[str, str]:
    root = repository_root()
    source, source_mode, _ = read_repository_file(
        root, source_relative, label="best source", maximum=SOURCE_ARCHIVE_MAX_BYTES
    )
    if canonical_relative_path(source_relative, "best source").suffix != ".c":
        raise CrewError("best source must be one tracked C path")
    tracked = run_git_in(root, "ls-files", "--error-unmatch", "--", source_relative, check=False)
    if tracked.returncode != 0 or tracked.stdout.strip() != source_relative:
        raise CrewError("best source must be one exact tracked repository path")
    object_path = canonical_relative_path(object_relative, "candidate object")
    if object_path.parts[0] != "build" or object_path.suffix != ".o" \
            or not git_ignored(root, object_relative):
        raise CrewError("candidate object must be below ignored build/")
    candidate, object_mode, _ = read_repository_file(
        root, object_relative, label="candidate object", maximum=CANDIDATE_ARCHIVE_MAX_BYTES
    )
    summary_path = canonical_relative_path(summary_relative, "workbench summary")
    if summary_path.parts[0] != "build" or summary_path.suffix != ".json" \
            or not git_ignored(root, summary_relative):
        raise CrewError("workbench summary must be below ignored build/")
    summary, summary_mode, _ = read_repository_file(
        root, summary_relative, label="workbench summary", maximum=WB_SUMMARY_MAX_BYTES
    )
    imported = workbench_summary_result(summary_relative, worker=worker, target=target)
    if validate_measured_result(imported) != validate_measured_result(measurement):
        raise CrewError("archived workbench summary does not reproduce the accepted best")

    created_at = timestamp()
    provenance = {
        "schema": BEST_ARTIFACT_SCHEMA,
        "worker": worker,
        "target": target,
        "base": base,
        "branch": current_branch(root),
        "worktree": str(root),
        "head": run_git_in(root, "rev-parse", "HEAD").stdout.strip(),
        "source_path": source_relative,
        "candidate_object_path": object_relative,
        "summary_path": summary_relative,
        "source_sha256": sha256_bytes(source),
        "candidate_object_sha256": sha256_bytes(candidate),
        "summary_sha256": sha256_bytes(summary),
        "source_mode": source_mode,
        "candidate_object_mode": object_mode,
        "summary_mode": summary_mode,
        "measurement": validate_measured_result(measurement),
        "created_at": created_at,
    }
    seed = json.dumps(provenance, sort_keys=True, separators=(",", ":")).encode("utf-8")
    artifact_id = sha256_bytes(seed)[:24]
    crew_root.mkdir(parents=True, exist_ok=True)
    require_directory(crew_root, "crew state directory")
    best_root = crew_root / "best-artifacts"
    best_root.mkdir(exist_ok=True)
    require_directory(best_root, "best-artifact directory")
    parent = best_root / worker
    parent.mkdir(exist_ok=True)
    require_directory(parent, "worker best-artifact directory")
    temporary = Path(tempfile.mkdtemp(prefix=f".{artifact_id}.", dir=parent))
    destination = artifact_root(crew_root, worker, artifact_id)
    try:
        write_bytes(temporary / "source", source, source_mode)
        write_bytes(temporary / "candidate.o", candidate, object_mode)
        write_bytes(temporary / "summary.json", summary, summary_mode)
        manifest_bytes = (json.dumps(provenance, indent=2, sort_keys=True) + "\n").encode("utf-8")
        write_bytes(temporary / "manifest.json", manifest_bytes)
        if destination.exists():
            raise CrewError("best artifact identifier collision")
        os.replace(temporary, destination)
    finally:
        if temporary.exists():
            for child in temporary.iterdir():
                child.unlink(missing_ok=True)
            temporary.rmdir()
    return {"id": artifact_id, "manifest_sha256": sha256_bytes(manifest_bytes)}


def load_best_artifact(
    crew_root: Path, worker: str, reference: dict[str, str]
) -> tuple[dict[str, object], dict[str, bytes], Path]:
    reference = validate_best_artifact_reference(reference, "heartbeat") or {}
    artifact_id = reference.get("id", "")
    directory = artifact_root(crew_root, worker, artifact_id)
    for path, label in (
        (crew_root, "crew state directory"),
        (crew_root / "best-artifacts", "best-artifact directory"),
        (crew_root / "best-artifacts" / worker, "worker best-artifact directory"),
        (directory, "recorded best artifact directory"),
    ):
        require_directory(path, label)
    manifest_bytes, _ = read_regular_path(
        directory / "manifest.json", label="best artifact manifest", maximum=WB_SUMMARY_MAX_BYTES
    )
    if sha256_bytes(manifest_bytes) != reference.get("manifest_sha256"):
        raise CrewError("best artifact manifest hash drift")
    try:
        manifest = json.loads(manifest_bytes.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise CrewError("best artifact manifest is malformed") from error
    required = {
        "schema", "worker", "target", "base", "branch", "worktree", "head",
        "source_path", "candidate_object_path", "summary_path", "source_sha256",
        "candidate_object_sha256", "summary_sha256", "source_mode",
        "candidate_object_mode", "summary_mode", "measurement", "created_at",
    }
    if not isinstance(manifest, dict) or set(manifest) != required:
        raise CrewError("best artifact manifest has an unsupported shape")
    if manifest.get("schema") != BEST_ARTIFACT_SCHEMA or manifest.get("worker") != worker:
        raise CrewError("best artifact manifest identity drift")
    for field in ("target", "branch", "worktree", "source_path", "candidate_object_path", "summary_path"):
        if not isinstance(manifest.get(field), str) or not manifest[field]:
            raise CrewError(f"best artifact manifest has invalid {field}")
    stored_concise_line(str(manifest["target"]), "best artifact target", 128)
    worktree = Path(str(manifest["worktree"]))
    if not worktree.is_absolute() or str(worktree.resolve()) != manifest["worktree"]:
        raise CrewError("best artifact manifest has noncanonical worktree")
    source_relative = canonical_relative_path(str(manifest["source_path"]), "best source")
    object_relative = canonical_relative_path(
        str(manifest["candidate_object_path"]), "candidate object"
    )
    summary_relative = canonical_relative_path(str(manifest["summary_path"]), "workbench summary")
    if source_relative.suffix != ".c":
        raise CrewError("best artifact source path is not C source")
    if object_relative.parts[0] != "build" or object_relative.suffix != ".o":
        raise CrewError("best artifact candidate path is not an ignored build object")
    if summary_relative.parts[0] != "build" or summary_relative.suffix != ".json":
        raise CrewError("best artifact summary path is not an ignored build report")
    for field in ("base", "head"):
        if not isinstance(manifest.get(field), str) or not COMMIT_RE.fullmatch(str(manifest[field])):
            raise CrewError(f"best artifact manifest has invalid {field}")
    for field in ("source_sha256", "candidate_object_sha256", "summary_sha256"):
        if not isinstance(manifest.get(field), str) or not re.fullmatch(
            r"[0-9a-f]{64}", str(manifest[field])
        ):
            raise CrewError(f"best artifact manifest has invalid {field}")
    for field in ("source_mode", "candidate_object_mode", "summary_mode"):
        if type(manifest.get(field)) is not int or not 0 <= int(manifest[field]) <= 0o7777:
            raise CrewError(f"best artifact manifest has invalid {field}")
    if not isinstance(manifest.get("measurement"), dict):
        raise CrewError("best artifact manifest has invalid measurement")
    manifest["measurement"] = validate_measurement_object(
        manifest["measurement"], "best artifact measurement"
    )
    parse_timestamp(str(manifest.get("created_at")))

    payloads: dict[str, bytes] = {}
    for filename, hash_field, mode_field, maximum in (
        ("source", "source_sha256", "source_mode", SOURCE_ARCHIVE_MAX_BYTES),
        ("candidate.o", "candidate_object_sha256", "candidate_object_mode", CANDIDATE_ARCHIVE_MAX_BYTES),
        ("summary.json", "summary_sha256", "summary_mode", WB_SUMMARY_MAX_BYTES),
    ):
        payload, payload_mode = read_regular_path(
            directory / filename, label=f"best artifact {filename}", maximum=maximum
        )
        if sha256_bytes(payload) != manifest[hash_field]:
            raise CrewError(f"best artifact {filename} hash drift")
        if payload_mode != manifest[mode_field]:
            raise CrewError(f"best artifact {filename} mode drift")
        payloads[filename] = payload
    return manifest, payloads, directory


def validate_restore_context(
    heartbeat: dict[str, object], manifest: dict[str, object]
) -> tuple[Path, Path]:
    root = repository_root()
    if str(root) != manifest["worktree"]:
        raise CrewError("best artifact belongs to a different worktree")
    if current_branch(root) != manifest["branch"]:
        raise CrewError("best artifact belongs to a different branch")
    if heartbeat["target"] != manifest["target"]:
        raise CrewError("best artifact target drift")
    if heartbeat["base"] != manifest["base"]:
        raise CrewError("best artifact assignment base drift")
    ancestry = run_git_in(
        root, "merge-base", "--is-ancestor", str(manifest["base"]), "HEAD", check=False
    )
    if ancestry.returncode != 0:
        raise CrewError("assignment base is no longer an ancestor of this lane")
    source_relative = str(manifest["source_path"])
    tracked = run_git_in(root, "ls-files", "--error-unmatch", "--", source_relative, check=False)
    if tracked.returncode != 0 or tracked.stdout.strip() != source_relative:
        raise CrewError("recorded best source path is no longer tracked exactly")
    unmerged = run_git_in(root, "ls-files", "-u", "--", source_relative)
    if unmerged.stdout.strip():
        raise CrewError("recorded best source has an unresolved index conflict")
    staged = run_git_in(root, "diff", "--cached", "--quiet", "--", source_relative, check=False)
    if staged.returncode not in (0, 1):
        raise CrewError("cannot inspect recorded best source index state")
    if staged.returncode == 1:
        raise CrewError("recorded best source has staged path/content drift")
    _, _, source_path = read_repository_file(
        root, source_relative, label="recorded best source", maximum=SOURCE_ARCHIVE_MAX_BYTES
    )
    return root, source_path


def create_source_backup(
    directory: Path,
    manifest: dict[str, object],
    source: bytes,
    source_mode: int,
) -> str:
    backup_id = utc_now().strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:12]
    backup_root = directory / "backups"
    backup_root.mkdir(parents=True, exist_ok=True)
    require_directory(backup_root, "best-source backup directory")
    temporary = Path(tempfile.mkdtemp(prefix=f".{backup_id}.", dir=backup_root))
    destination = backup_root / backup_id
    backup_manifest = {
        "schema": BEST_BACKUP_SCHEMA,
        "artifact_id": directory.name,
        "artifact_manifest_sha256": sha256_bytes(
            read_regular_path(
                directory / "manifest.json",
                label="best artifact manifest", maximum=WB_SUMMARY_MAX_BYTES,
            )[0]
        ),
        "worker": manifest["worker"],
        "target": manifest["target"],
        "base": manifest["base"],
        "branch": manifest["branch"],
        "worktree": manifest["worktree"],
        "source_path": manifest["source_path"],
        "source_sha256": sha256_bytes(source),
        "source_mode": source_mode,
        "created_at": timestamp(),
    }
    try:
        write_bytes(temporary / "source", source, source_mode)
        write_bytes(
            temporary / "manifest.json",
            (json.dumps(backup_manifest, indent=2, sort_keys=True) + "\n").encode("utf-8"),
        )
        os.replace(temporary, destination)
    finally:
        if temporary.exists():
            for child in temporary.iterdir():
                child.unlink(missing_ok=True)
            temporary.rmdir()
    return backup_id


def backup_artifact_reference(
    crew_root: Path, worker: str, artifact_id: str, backup_id: str
) -> dict[str, str]:
    directory = artifact_root(crew_root, worker, artifact_id) / "backups" / backup_id
    if not BEST_BACKUP_ID_RE.fullmatch(backup_id):
        raise CrewError("invalid best-source backup id")
    require_directory(directory.parent, "best-source backup directory")
    require_directory(directory, "best-source backup")
    metadata_bytes, _ = read_regular_path(
        directory / "manifest.json", label="best-source backup manifest", maximum=WB_SUMMARY_MAX_BYTES
    )
    try:
        metadata = json.loads(metadata_bytes.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise CrewError("best-source backup manifest is malformed") from error
    manifest_hash = metadata.get("artifact_manifest_sha256") if isinstance(metadata, dict) else None
    if (
        not isinstance(metadata, dict)
        or metadata.get("schema") != BEST_BACKUP_SCHEMA
        or metadata.get("artifact_id") != artifact_id
        or metadata.get("worker") != worker
        or not isinstance(manifest_hash, str)
        or not re.fullmatch(r"[0-9a-f]{64}", manifest_hash)
    ):
        raise CrewError("best-source backup artifact reference drift")
    return {"id": artifact_id, "manifest_sha256": manifest_hash}


def load_source_backup(
    artifact_directory: Path, backup_id: str, manifest: dict[str, object]
) -> tuple[bytes, int]:
    if not BEST_BACKUP_ID_RE.fullmatch(backup_id):
        raise CrewError("invalid best-source backup id")
    directory = artifact_directory / "backups" / backup_id
    require_directory(artifact_directory / "backups", "best-source backup directory")
    require_directory(directory, "best-source backup")
    metadata_bytes, _ = read_regular_path(
        directory / "manifest.json", label="best-source backup manifest", maximum=WB_SUMMARY_MAX_BYTES
    )
    try:
        metadata = json.loads(metadata_bytes.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise CrewError("best-source backup manifest is malformed") from error
    required = {
        "schema", "artifact_id", "artifact_manifest_sha256", "worker", "target",
        "base", "branch", "worktree",
        "source_path", "source_sha256", "source_mode", "created_at",
    }
    if not isinstance(metadata, dict) or set(metadata) != required:
        raise CrewError("best-source backup manifest has an unsupported shape")
    for field in ("artifact_id", "worker", "target", "base", "branch", "worktree", "source_path"):
        expected = artifact_directory.name if field == "artifact_id" else manifest[field]
        if metadata.get(field) != expected:
            raise CrewError(f"best-source backup {field} drift")
    if metadata.get("schema") != BEST_BACKUP_SCHEMA:
        raise CrewError("best-source backup schema drift")
    manifest_bytes, _ = read_regular_path(
        artifact_directory / "manifest.json",
        label="best artifact manifest", maximum=WB_SUMMARY_MAX_BYTES,
    )
    if metadata.get("artifact_manifest_sha256") != sha256_bytes(manifest_bytes):
        raise CrewError("best-source backup artifact manifest hash drift")
    if type(metadata.get("source_mode")) is not int or not 0 <= int(metadata["source_mode"]) <= 0o7777:
        raise CrewError("best-source backup has invalid source_mode")
    parse_timestamp(str(metadata.get("created_at")))
    source, _ = read_regular_path(
        directory / "source", label="best-source backup", maximum=SOURCE_ARCHIVE_MAX_BYTES
    )
    if sha256_bytes(source) != metadata.get("source_sha256"):
        raise CrewError("best-source backup hash drift")
    return source, int(metadata["source_mode"])


def command_restore_best(args: argparse.Namespace, crew_root: Path) -> None:
    heartbeat = load_heartbeat(heartbeat_path(crew_root, args.worker))
    recover = args.recover_backup
    if recover:
        try:
            artifact_id, backup_id = recover.split("/", 1)
        except ValueError as error:
            raise CrewError("--recover-backup must be ARTIFACT_ID/BACKUP_ID") from error
        reference = backup_artifact_reference(
            crew_root, args.worker, artifact_id, backup_id
        )
    else:
        reference = heartbeat.get("best_artifact")
        if not isinstance(reference, dict):
            raise CrewError("the recorded best has no archived source to restore")
        backup_id = ""
    manifest, payloads, directory = load_best_artifact(crew_root, args.worker, reference)
    root, source_path = validate_restore_context(heartbeat, manifest)
    current_source, current_mode, _ = read_repository_file(
        root, str(manifest["source_path"]),
        label="recorded best source", maximum=SOURCE_ARCHIVE_MAX_BYTES,
    )
    if recover:
        replacement, replacement_mode = load_source_backup(directory, backup_id, manifest)
    else:
        replacement, replacement_mode = payloads["source"], int(manifest["source_mode"])
        if validate_measured_result(manifest["measurement"]) != heartbeat["best_result"]:
            raise CrewError("best artifact measurement no longer agrees with the heartbeat")
    if current_source == replacement and current_mode == replacement_mode:
        print(json.dumps({"restored": False, "reason": "source already exact"}, sort_keys=True))
        return
    displaced_backup = create_source_backup(
        directory, manifest, current_source, current_mode
    )
    write_bytes(source_path, replacement, replacement_mode)
    restored, restored_mode, _ = read_repository_file(
        root, str(manifest["source_path"]),
        label="restored best source", maximum=SOURCE_ARCHIVE_MAX_BYTES,
    )
    if restored != replacement or restored_mode != replacement_mode:
        raise CrewError("restored source failed its post-write hash/mode proof")
    print(json.dumps({
        "restored": True,
        "source": manifest["source_path"],
        "sha256": sha256_bytes(restored),
        "displaced_backup": f"{directory.name}/{displaced_backup}",
        "recovery_command": (
            f"python3 tools/crew.py restore-best --worker {args.worker} "
            f"--recover-backup {directory.name}/{displaced_backup}"
        ),
    }, sort_keys=True))


def command_reprove_best(args: argparse.Namespace, crew_root: Path) -> None:
    heartbeat = load_heartbeat(heartbeat_path(crew_root, args.worker))
    reference = heartbeat.get("best_artifact")
    if not isinstance(reference, dict):
        raise CrewError("the recorded best has no archived source to re-prove")
    manifest, _, _ = load_best_artifact(crew_root, args.worker, reference)
    root, _ = validate_restore_context(heartbeat, manifest)
    source, _, _ = read_repository_file(
        root, str(manifest["source_path"]),
        label="recorded best source", maximum=SOURCE_ARCHIVE_MAX_BYTES,
    )
    if sha256_bytes(source) != manifest["source_sha256"]:
        raise CrewError("source is not the archived best; run restore-best first")
    summary_relative = f"build/wb/{reference['id']}.restore-reproof-summary.json"
    summary_path = root / summary_relative
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    environment = os.environ.copy()
    environment["MICKEY_BUILD_JOBS"] = "2"
    environment["MICKEY_BUILD_NICE"] = "15"
    environment["MAKEFLAGS"] = "-j2"
    result = subprocess.run(
        ["nice", "-n", "15", "tools/wb_compare.sh", "--summary-json", str(heartbeat["target"])],
        cwd=root, env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )
    if result.returncode != 0:
        detail = clean_line(result.stderr)[-500:] or f"exit {result.returncode}"
        raise CrewError(f"best-source reproof failed: {detail}")
    write_bytes(summary_path, result.stdout.encode("utf-8"), 0o600)
    checkpoint_args = argparse.Namespace(
        worker=args.worker, target=None, base=None, deadline_unix=None,
        progress=args.progress, state=heartbeat["state"], attempt_count=args.attempt_count,
        best_score=None, mismatch_class=args.mismatch_class, eta_unix=None,
        wb_summary=summary_relative, archive_best=False, source=None, candidate_object=None,
        **{field: None for field in MEASUREMENT_FIELDS}, promotion_state=None,
    )
    command_heartbeat(checkpoint_args, crew_root)


def git_ignored(root: Path, relative: str) -> bool:
    result = subprocess.run(
        ["git", "check-ignore", "--quiet", "--", relative],
        cwd=root,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode not in (0, 1):
        detail = result.stderr.strip() or "git check-ignore failed"
        raise CrewError(f"cannot validate workbench summary location: {detail}")
    return result.returncode == 0


def read_fresh_workbench_summary(relative: str) -> dict[str, object]:
    """Read one fresh, ignored build report without following symlinks."""

    supplied = Path(relative)
    if supplied.is_absolute() or not supplied.parts or any(
        part in ("", ".", "..") for part in supplied.parts
    ) or supplied.as_posix() != relative:
        raise CrewError("workbench summary path must be a canonical repository-relative path")
    if supplied.parts[0] != "build":
        raise CrewError("workbench summary must be below the ignored build/ directory")
    root = repository_root()
    canonical_relative = supplied.as_posix()
    if not git_ignored(root, canonical_relative):
        raise CrewError("workbench summary path is not ignored by Git")

    path = root
    for part in supplied.parts[:-1]:
        path /= part
        try:
            metadata = path.lstat()
        except OSError as error:
            raise CrewError(f"cannot inspect workbench summary path {canonical_relative}") from error
        if stat.S_ISLNK(metadata.st_mode) or not stat.S_ISDIR(metadata.st_mode):
            raise CrewError("workbench summary path contains a symlink or non-directory")
    path /= supplied.parts[-1]
    try:
        expected = path.lstat()
    except OSError as error:
        raise CrewError(f"cannot inspect workbench summary {canonical_relative}") from error
    if stat.S_ISLNK(expected.st_mode) or not stat.S_ISREG(expected.st_mode):
        raise CrewError("workbench summary is a symlink or is not a regular file")

    flags = os.O_RDONLY | getattr(os, "O_CLOEXEC", 0) | getattr(os, "O_NOFOLLOW", 0)
    try:
        descriptor = os.open(path, flags)
    except OSError as error:
        raise CrewError(f"cannot open workbench summary {canonical_relative}") from error
    try:
        before = os.fstat(descriptor)
        if not stat.S_ISREG(before.st_mode):
            raise CrewError("workbench summary is not a regular file")
        if before.st_size > WB_SUMMARY_MAX_BYTES:
            raise CrewError("workbench summary is unexpectedly large")
        chunks: list[bytes] = []
        remaining = WB_SUMMARY_MAX_BYTES + 1
        while remaining:
            chunk = os.read(descriptor, min(remaining, 16 * 1024))
            if not chunk:
                break
            chunks.append(chunk)
            remaining -= len(chunk)
        if remaining == 0 and os.read(descriptor, 1):
            raise CrewError("workbench summary is unexpectedly large")
        after = os.fstat(descriptor)
    finally:
        os.close(descriptor)
    try:
        current = path.lstat()
    except OSError as error:
        raise CrewError("workbench summary changed while it was read") from error
    def identity(value: os.stat_result) -> tuple[int, int, int, int]:
        return (value.st_dev, value.st_ino, value.st_size, value.st_mtime_ns)

    if (
        identity(expected) != identity(before)
        or identity(before) != identity(after)
        or identity(after) != identity(current)
    ):
        raise CrewError("workbench summary changed while it was read")

    age = utc_now().timestamp() - after.st_mtime
    if age < -WB_SUMMARY_FUTURE_SLOP_SECONDS:
        raise CrewError("workbench summary timestamp is implausibly in the future")
    if age > WB_SUMMARY_MAX_AGE_SECONDS:
        raise CrewError("workbench summary is stale; regenerate it before checkpointing")
    try:
        payload = json.loads(b"".join(chunks).decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise CrewError("workbench summary is not one valid UTF-8 JSON value") from error
    if not isinstance(payload, dict):
        raise CrewError("workbench summary must contain one JSON object")
    return payload


def summary_integer(record: dict[str, object], field: str) -> int:
    value = record.get(field)
    if type(value) is not int or int(value) < 0:
        raise CrewError(f"workbench summary has invalid {field}")
    return int(value)


def workbench_summary_result(
    relative: str, *, worker: str, target: str
) -> dict[str, object]:
    payload = read_fresh_workbench_summary(relative)
    if payload.get("schema") != WB_SUMMARY_SCHEMA:
        raise CrewError("workbench summary has unsupported schema")
    symbols = payload.get("symbol")
    if not isinstance(symbols, dict):
        raise CrewError("workbench summary has malformed symbol identity")
    identities: set[str] = set()
    for field in ("requested", "target", "candidate"):
        value = symbols.get(field)
        if not isinstance(value, str):
            raise CrewError(f"workbench summary has invalid symbol.{field}")
        identities.add(stored_concise_line(value, f"workbench symbol.{field}", 128))
    if target not in identities:
        raise CrewError("workbench summary symbol does not agree with the assigned target")
    summary_worker = payload.get("worker")
    if summary_worker is not None and summary_worker != worker:
        raise CrewError("workbench summary worker does not agree with the checkpoint worker")

    comparison = payload.get("comparison")
    evidence = payload.get("evidence")
    boundary = payload.get("boundary")
    if not isinstance(comparison, dict) or not isinstance(evidence, dict):
        raise CrewError("workbench summary lacks comparison/evidence objects")
    if not isinstance(boundary, dict):
        raise CrewError("workbench summary lacks a boundary object")
    target_words = summary_integer(comparison, "target_words")
    candidate_words = summary_integer(comparison, "candidate_words")
    raw = summary_integer(comparison, "raw_differing_words")
    masked = summary_integer(comparison, "differing_words")
    boundary_bytes = boundary.get("bytes")
    if type(boundary_bytes) is not int or boundary_bytes != target_words * 4:
        raise CrewError("workbench summary boundary disagrees with target_words")
    matched_words = comparison.get("matched_words")
    expected_matched = target_words - masked if target_words == candidate_words else None
    if matched_words != expected_matched:
        raise CrewError("workbench summary matched_words is inconsistent")
    exact = comparison.get("exact")
    admissible = evidence.get("admissible_exact_comparison")
    promotion_proof = evidence.get("promotion_proof_included")
    if (
        not isinstance(exact, bool)
        or not isinstance(admissible, bool)
        or not isinstance(promotion_proof, bool)
    ):
        raise CrewError("workbench summary has invalid exact-comparison state")
    if promotion_proof:
        raise CrewError("workbench summary unexpectedly claims canonical promotion proof")
    if exact and not (
        target_words == candidate_words and raw == 0 and masked == 0
    ):
        raise CrewError("workbench summary claims exact with nonexact word metrics")
    if not exact and target_words == candidate_words and raw == 0 and masked == 0:
        raise CrewError("workbench summary denies exact despite exact word metrics")

    # decomp-workbench's build_provenance.claim only reaches "match" for a
    # declared stock build; forced/instrumented/unknown/undeclared builds
    # still report exact=True. function_preflight.workbench_summary() already
    # folds this into admissible_exact_comparison, so this is defense in
    # depth against a stale or hand-edited summary -- and it names the reason
    # rather than letting a mismatched claim promote silently. A summary with
    # no provenance.build_claim at all (older format) is unconstrained here,
    # matching prior behaviour.
    provenance = payload.get("provenance")
    build_claim = provenance.get("build_claim") if isinstance(provenance, dict) else None
    build_provenance_verified = build_claim is None or build_claim == "match"
    if exact and not build_provenance_verified:
        print(
            "crew: workbench summary reports exact=True but "
            f"provenance.build_claim={build_claim!r} (not 'match'); "
            "not treating this as a verified match.",
            file=sys.stderr,
        )
        if admissible:
            raise CrewError(
                "workbench summary claims admissible_exact_comparison despite "
                "an unverified build_provenance.claim"
            )

    result: dict[str, object] = {
        "target_words": target_words,
        "candidate_words": candidate_words,
        "raw_differing_words": raw,
        "relocation_masked_differing_words": masked,
        # Word equality alone is not object-exact under ADR 0001.  Derive that
        # state only after the authenticated relocation block below proves the
        # complete candidate/target surface and every identity.
        "promotion_state": "compiled",
    }
    relocations = payload.get("relocations")
    if relocations is not None and not isinstance(relocations, dict):
        raise CrewError("workbench summary has malformed relocations")
    relocation_source = relocations if isinstance(relocations, dict) else comparison
    for field in (
        "candidate_relocations", "target_relocations", "exact_relocation_identities"
    ):
        value = relocation_source.get(field)
        if value is not None and (type(value) is not int or int(value) < 0):
            raise CrewError(f"workbench summary has invalid {field}")
        result[field] = value
    relocation_exact = (
        result["candidate_relocations"] is not None
        and result["candidate_relocations"]
        == result["target_relocations"]
        == result["exact_relocation_identities"]
    )
    if exact and admissible and relocation_exact:
        result["promotion_state"] = "object-exact"
    return result


def checkpoint_result(
    args: argparse.Namespace,
    *,
    target: str,
    existing: dict[str, object] | None,
    assignment_changed: bool,
) -> dict[str, object]:
    explicit = {
        field: getattr(args, field, None)
        for field in (*MEASUREMENT_FIELDS, "promotion_state")
    }
    summary_path = getattr(args, "wb_summary", None)
    imported = (
        workbench_summary_result(summary_path, worker=args.worker, target=target)
        if summary_path is not None else None
    )
    supplied = imported is not None or any(value is not None for value in explicit.values())
    if not supplied:
        if existing is not None and not assignment_changed:
            return validate_measured_result(existing)
        return unmeasured_result()

    if imported is not None:
        combined: dict[str, object] = dict(imported)
    elif existing is not None and not assignment_changed:
        combined = {
            field: existing.get(field)
            for field in (*MEASUREMENT_FIELDS, "promotion_state")
        }
    else:
        combined = {}
    for field, value in explicit.items():
        if value is None:
            continue
        if (
            imported is not None
            and field in combined
            and combined[field] is not None
            and combined[field] != value
        ):
            raise CrewError(f"explicit {field} conflicts with workbench summary")
        combined[field] = value
    return validate_measured_result(combined, "checkpoint measured result")


def load_config(root: Path) -> dict[str, object]:
    path = root / "config.json"
    if not path.is_file():
        raise CrewError(f"crew is not initialized; run tools/crew.py init (expected {path})")
    try:
        config = json.loads(path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as error:
        raise CrewError(f"cannot read {path}: {error}") from error
    if config.get("schema") != SCHEMA_VERSION:
        raise CrewError(f"unsupported crew schema in {path}")
    return config


def configured_roles(config: dict[str, object]) -> list[str]:
    workers = config.get("workers")
    if not isinstance(workers, list):
        raise CrewError("crew config has no worker list")
    roles = ["leader"]
    for row in workers:
        if not isinstance(row, dict) or not isinstance(row.get("role"), str):
            raise CrewError("crew config contains an invalid worker")
        roles.append(row["role"])
    return roles


def require_role(config: dict[str, object], role: str) -> str:
    if role not in configured_roles(config):
        raise CrewError(f"unknown role {role!r}")
    return role


def status_document(
    role: str,
    state: str,
    task: str,
    branch: str,
    ownership: str,
    compile_permit: str,
    summary: str,
) -> str:
    return (
        f"# {role} status\n\n"
        f"- state: {state}\n"
        f"- task: {task}\n"
        f"- branch: {branch}\n"
        f"- ownership: {ownership}\n"
        f"- compile-permit: {compile_permit}\n"
        f"- updated-at: {timestamp()}\n"
        f"- summary: {summary}\n"
    )


def parse_worker(value: str) -> tuple[str, str]:
    role, separator, lane = value.partition("=")
    role = validate_name(role, "worker role")
    lane = validate_name(lane if separator else f"crew-{role}", "lane name")
    return role, lane


def command_init(args: argparse.Namespace, root: Path) -> None:
    workers = [parse_worker(value) for value in args.worker]
    roles = [role for role, _ in workers]
    if len(roles) != 2 or len(set(roles)) != 2:
        raise CrewError("this protocol requires exactly two distinct workers")
    if "leader" in roles:
        raise CrewError("leader is reserved and cannot be a worker role")

    repo = primary_checkout_root()
    rows = []
    for role, lane in workers:
        rows.append({
            "role": role,
            "lane": lane,
            "parking_branch": f"lane/{lane}",
            "worktree": str(repo.parent / f"{repo.name}-lane-{lane}"),
        })
    desired = {
        "schema": SCHEMA_VERSION,
        "integration_branch": args.integration_branch,
        "leader": "leader",
        "workers": rows,
    }

    config_path = root / "config.json"
    if config_path.exists():
        existing = load_config(root)
        for key in ("schema", "integration_branch", "leader", "workers"):
            if existing.get(key) != desired.get(key):
                raise CrewError(f"existing {config_path} disagrees on {key}; refusing to overwrite")
    else:
        root.mkdir(parents=True, exist_ok=True)
        desired["initialized_at"] = timestamp()
        atomic_write(config_path, json.dumps(desired, indent=2, sort_keys=True) + "\n")

    all_roles = ["leader", *roles]
    for role in all_roles:
        (root / "inbox" / role).mkdir(parents=True, exist_ok=True)
        (root / "archive" / role).mkdir(parents=True, exist_ok=True)

    queue_path = root / "queue.md"
    if not queue_path.exists():
        atomic_write(
            queue_path,
            "# Crew queue\n\n"
            "Leader-owned. Assign disjoint work; one isolated pipeline task may follow a handoff.\n\n"
            "| Priority | Task | Owner | State | Ownership | Base | Notes |\n"
            "|---:|---|---|---|---|---|---|\n"
        )

    leader_status = root / "status" / "leader.md"
    if not leader_status.exists():
        atomic_write(
            leader_status,
            status_document("leader", "ACTIVE", "none", args.integration_branch,
                            "queue and integration", "idle", "crew initialized"),
        )
    for row in rows:
        path = root / "status" / f"{row['role']}.md"
        if not path.exists():
            atomic_write(
                path,
                status_document(
                    str(row["role"]), "READY", "none", str(row["parking_branch"]),
                    "unassigned", "idle", "waiting for assignment",
                ),
            )
    print(root)


def body_from_args(args: argparse.Namespace) -> str:
    if getattr(args, "body", None) is not None:
        return args.body.strip()
    body_file = getattr(args, "body_file", None)
    if body_file:
        if body_file == "-":
            return sys.stdin.read().strip()
        return Path(body_file).read_text(encoding="utf-8").strip()
    if not sys.stdin.isatty():
        return sys.stdin.read().strip()
    return ""


def create_message(
    root: Path,
    config: dict[str, object],
    sender: str,
    recipient: str,
    kind: str,
    task: str,
    subject: str,
    body: str,
) -> Path:
    require_role(config, sender)
    require_role(config, recipient)
    validate_task(task)
    if kind not in MESSAGE_TYPES:
        raise CrewError(f"invalid message type {kind!r}")
    now = utc_now()
    message_id = (
        now.strftime("%Y%m%dT%H%M%S.%fZ")
        + f"-{uuid.uuid4().hex[:8]}-{sender}-{kind.lower()}-{task}"
    )
    path = root / "inbox" / recipient / f"{message_id}.md"
    content = (
        "---\n"
        f"schema: {SCHEMA_VERSION}\n"
        f"id: {message_id}\n"
        f"from: {sender}\n"
        f"to: {recipient}\n"
        f"type: {kind}\n"
        f"task: {task}\n"
        f"created-at: {timestamp(now)}\n"
        f"subject: {clean_line(subject) or kind}\n"
        "---\n\n"
        f"{body}\n"
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        with path.open("x", encoding="utf-8", newline="\n") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
    except FileExistsError as error:
        raise CrewError(f"message collision at {path}; retry") from error
    return path


def command_send(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    path = create_message(
        root,
        config,
        args.sender,
        args.recipient,
        args.kind,
        args.task,
        args.subject,
        body_from_args(args),
    )
    print(path.name)


def message_path(root: Path, role: str, message: str, archive: bool = False) -> Path:
    if Path(message).name != message:
        raise CrewError("message must be a file name from tools/crew.py inbox")
    folder = "archive" if archive else "inbox"
    path = root / folder / role / message
    if not path.is_file():
        raise CrewError(f"message not found: {path}")
    return path


def parse_front_matter(path: Path) -> dict[str, str]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "---":
        raise CrewError(f"invalid message header: {path}")
    fields: dict[str, str] = {}
    for line in lines[1:]:
        if line == "---":
            return fields
        key, separator, value = line.partition(":")
        if not separator:
            raise CrewError(f"invalid message header: {path}")
        fields[key.strip()] = value.strip()
    raise CrewError(f"unterminated message header: {path}")


def command_inbox(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    require_role(config, args.role)
    paths = sorted((root / "inbox" / args.role).glob("*.md"))
    if not paths:
        print(f"{args.role} inbox empty")
        return
    for path in paths:
        fields = parse_front_matter(path)
        print(
            "\t".join((
                path.name,
                fields.get("type", "?"),
                fields.get("task", "?"),
                fields.get("from", "?"),
                fields.get("subject", ""),
            ))
        )


def command_read(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    require_role(config, args.role)
    print(message_path(root, args.role, args.message, args.archive).read_text(encoding="utf-8"), end="")


def archive_message(root: Path, role: str, message: str) -> Path:
    source = message_path(root, role, message)
    destination = root / "archive" / role / message
    if destination.exists():
        raise CrewError(f"archive already contains {message}")
    os.replace(source, destination)
    return destination


def command_archive(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    require_role(config, args.role)
    print(archive_message(root, args.role, args.message))


def command_ack(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    require_role(config, args.role)
    source = message_path(root, args.role, args.message)
    fields = parse_front_matter(source)
    sender = fields.get("from")
    task = fields.get("task")
    if not sender or not task:
        raise CrewError(f"message lacks sender or task: {source}")
    ack = create_message(
        root,
        config,
        args.role,
        sender,
        "ACK",
        task,
        f"ACK {fields.get('type', 'message')}: {fields.get('subject', '')}",
        body_from_args(args) or f"Received {source.name}.",
    )
    archive_message(root, args.role, args.message)
    print(ack.name)


def command_status(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    roles = [require_role(config, args.role)] if args.role else configured_roles(config)
    for index, role in enumerate(roles):
        if index:
            print()
        path = root / "status" / f"{role}.md"
        if not path.is_file():
            raise CrewError(f"missing status file: {path}")
        print(path.read_text(encoding="utf-8"), end="")


def command_set_status(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    role = require_role(config, args.role)
    if args.state not in STATES:
        raise CrewError(f"invalid state {args.state!r}")
    task = args.task if args.task == "none" else validate_task(args.task)
    document = status_document(
        role,
        args.state,
        task,
        clean_line(args.branch),
        clean_line(args.ownership),
        args.compile_permit,
        clean_line(args.summary),
    )
    atomic_write(root / "status" / f"{role}.md", document)
    print(root / "status" / f"{role}.md")


def command_heartbeat(args: argparse.Namespace, root: Path) -> None:
    path = heartbeat_path(root, args.worker)
    existing = load_heartbeat(path) if path.is_file() else None
    target = concise_line(args.target, "target", 128) if args.target else None
    base = args.base
    deadline = args.deadline_unix
    if existing is None:
        missing = [
            name for name, value in (("--target", target), ("--base", base),
                                     ("--deadline-unix", deadline)) if value is None
        ]
        if missing:
            raise CrewError("new heartbeat requires " + ", ".join(missing))
    else:
        target = target or str(existing["target"])
        base = base or str(existing["base"])
        deadline = deadline if deadline is not None else int(existing["deadline_unix"])

    assert target is not None and base is not None and deadline is not None
    if not COMMIT_RE.fullmatch(base):
        raise CrewError("heartbeat base must be a 7-40 character lowercase commit id")
    if deadline <= 0:
        raise CrewError("heartbeat deadline must be a positive Unix timestamp")
    parse_unix_timestamp(deadline, "heartbeat deadline")
    progress = concise_line(args.progress, "progress")
    assignment_changed = existing is not None and (
        target != str(existing["target"]) or base != str(existing["base"])
    )
    attempt_arg = getattr(args, "attempt_count", None)
    if attempt_arg is not None and (type(attempt_arg) is not int or attempt_arg < 0):
        raise CrewError("heartbeat attempt-count must be non-negative")
    if assignment_changed:
        attempt_count = attempt_arg if attempt_arg is not None else 0
        best_score = getattr(args, "best_score", None) or "not measured"
        mismatch_class = getattr(args, "mismatch_class", None) or "unclassified"
        eta_unix = getattr(args, "eta_unix", None)
    else:
        attempt_count = attempt_arg if attempt_arg is not None else (
            int(existing["attempt_count"]) if existing is not None else 0
        )
        best_score_arg = getattr(args, "best_score", None)
        best_score = best_score_arg if best_score_arg is not None else (
            str(existing["best_score"]) if existing is not None else "not measured"
        )
        mismatch_arg = getattr(args, "mismatch_class", None)
        mismatch_class = mismatch_arg if mismatch_arg is not None else (
            str(existing["mismatch_class"]) if existing is not None else "unclassified"
        )
        eta_arg = getattr(args, "eta_unix", None)
        eta_unix = eta_arg if eta_arg is not None else (
            existing.get("eta_unix") if existing is not None else None
        )
    if existing is not None and not assignment_changed and attempt_count < int(existing["attempt_count"]):
        raise CrewError("heartbeat attempt-count cannot decrease for the same assignment")
    measured_result = checkpoint_result(
        args,
        target=target,
        existing=existing,
        assignment_changed=assignment_changed,
    )
    current_score = best_score
    if measured_result["promotion_state"] != "unmeasured":
        if getattr(args, "best_score", None) is not None:
            raise CrewError("--best-score cannot accompany unambiguous measured metrics")
        current_score = measured_score(measured_result)
    elif getattr(args, "best_score", None) is None and existing is not None and not assignment_changed:
        current_score = str(existing.get("current_score", existing["best_score"]))
    current_score = concise_line(current_score, "current-score", 128)

    previous_best = (
        validate_measured_result(existing["best_result"])
        if existing is not None and not assignment_changed else unmeasured_result()
    )
    accepted_best = assignment_changed or existing is None or better_measurement(
        measured_result, previous_best
    )
    if accepted_best:
        best_result = measured_result
        best_score = current_score
        best_mismatch_class = mismatch_class
        best_artifact = None
    else:
        best_result = previous_best
        best_score = str(existing["best_score"])
        best_mismatch_class = str(existing["best_mismatch_class"])
        best_artifact = existing.get("best_artifact")
    if measured_result["promotion_state"] == "unmeasured" and previous_best["promotion_state"] == "unmeasured":
        # Preserve schema-1/2 score prose as legacy telemetry. Numeric records
        # never use prose to choose or replace a best result.
        best_score = current_score
        best_mismatch_class = mismatch_class

    if not MISMATCH_CLASS_RE.fullmatch(mismatch_class):
        raise CrewError(
            "heartbeat mismatch-class must start with a lowercase letter and use "
            "lowercase letters, digits, '+', '.', '_', '/', or '-'"
        )

    archive_requested = bool(getattr(args, "archive_best", False))
    if archive_requested:
        missing_archive = [
            flag for flag, value in (
                ("--source", getattr(args, "source", None)),
                ("--candidate-object", getattr(args, "candidate_object", None)),
                ("--wb-summary", getattr(args, "wb_summary", None)),
            ) if value is None
        ]
        if missing_archive:
            raise CrewError("--archive-best requires " + ", ".join(missing_archive))
        if accepted_best and measured_result["promotion_state"] != "unmeasured":
            best_artifact = archive_best_artifact(
                root,
                worker=args.worker,
                target=target,
                base=base,
                source_relative=str(args.source),
                object_relative=str(args.candidate_object),
                summary_relative=str(args.wb_summary),
                measurement=measured_result,
            )
    best_score = concise_line(best_score, "best-score", 128)
    if eta_unix is not None and (type(eta_unix) is not int or eta_unix <= 0):
        raise CrewError("heartbeat ETA must be a positive Unix timestamp")
    if eta_unix is not None:
        parse_unix_timestamp(eta_unix, "heartbeat ETA")
    last_commit = run_git("rev-parse", "HEAD")
    if not COMMIT_RE.fullmatch(last_commit):
        raise CrewError("could not resolve the current lane commit")
    now = utc_now()
    record = {
        "schema": HEARTBEAT_SCHEMA_VERSION,
        "worker": args.worker,
        "target": target,
        "base": base,
        "last_progress": progress,
        "last_progress_at": timestamp(now),
        "last_commit": last_commit,
        "deadline_unix": deadline,
        "attempt_count": attempt_count,
        "current_score": current_score,
        "best_score": best_score,
        "mismatch_class": mismatch_class,
        "best_mismatch_class": best_mismatch_class,
        "best_result": best_result,
        "best_artifact": best_artifact,
        "eta_unix": eta_unix,
        **measured_result,
        "state": args.state,
        "updated_at": timestamp(now),
    }
    atomic_write(path, json.dumps(record, indent=2, sort_keys=True) + "\n")
    print(path)


def command_heartbeat_status(args: argparse.Namespace, root: Path) -> None:
    directory = root / "heartbeats"
    paths = [heartbeat_path(root, args.worker)] if args.worker else sorted(directory.glob("*.json"))
    if args.worker and not paths[0].is_file():
        raise CrewError(f"no heartbeat for {args.worker}")
    now = utc_now()
    stale_after = dt.timedelta(minutes=args.stale_after_minutes)
    stale_records: list[tuple[dict[str, object], list[str]]] = []
    malformed_records: list[tuple[Path, str]] = []
    rows: list[dict[str, object]] = []
    for path in paths:
        try:
            record = load_heartbeat(path)
        except CrewError as error:
            detail = str(error).replace(str(path), path.name)
            malformed_records.append((path, detail))
            rows.append({
                "worker": path.stem,
                "health": "malformed",
                "error": detail,
            })
            continue
        reasons = heartbeat_staleness(record, now, stale_after)
        progress_age_seconds = max(
            0, int((now - parse_timestamp(str(record["last_progress_at"]))).total_seconds())
        )
        deadline_unix = int(record["deadline_unix"])
        eta_unix = record.get("eta_unix")
        terminal = record["state"] not in ("active", "stopping")
        health = "stale" if reasons else "terminal" if terminal else "current"
        rows.append({
            "worker": record["worker"],
            "state": record["state"],
            "target": record["target"],
            "base": record["base"],
            "commit": record["last_commit"],
            "attempt_count": record["attempt_count"],
            "current_score": record["current_score"],
            "best_score": record["best_score"],
            "mismatch_class": record["mismatch_class"],
            "best_mismatch_class": record["best_mismatch_class"],
            **{field: record[field] for field in MEASUREMENT_FIELDS},
            "promotion_state": record["promotion_state"],
            "best_result": record["best_result"],
            "best_artifact": record["best_artifact"] is not None,
            "progress": record["last_progress"],
            "progress_age_seconds": progress_age_seconds,
            "deadline_unix": deadline_unix,
            "seconds_to_deadline": deadline_unix - int(now.timestamp()),
            "eta_unix": eta_unix,
            "seconds_to_eta": (
                int(eta_unix) - int(now.timestamp()) if eta_unix is not None else None
            ),
            "health": health,
            "reasons": reasons,
        })
        if reasons:
            stale_records.append((record, reasons))

    if getattr(args, "json", False):
        summary = {
            "total": len(rows),
            "current": sum(row["health"] == "current" for row in rows),
            "terminal": sum(row["health"] == "terminal" for row in rows),
            "stale": sum(row["health"] == "stale" for row in rows),
            "malformed": sum(row["health"] == "malformed" for row in rows),
        }
        report = {
            "schema": 1,
            "generated_at": timestamp(now),
            "stale_after_seconds": int(stale_after.total_seconds()),
            "ok": not stale_records and not malformed_records,
            "summary": summary,
            "workers": rows,
        }
        print(json.dumps(report, separators=(",", ":"), sort_keys=True))
    elif not paths:
        print("no heartbeat records")
    else:
        print(
            "worker\tstate\ttarget\tattempt\tcurrent\tbest\tbest-mismatch\tbase\tlast-commit\t"
            "last-progress\tdeadline\tstatus"
        )
        for row in rows:
            if row["health"] == "malformed":
                print(f"{row['worker']}\t?\t?\t?\t?\t?\t?\t?\t?\t?\t?\tmalformed: {row['error']}")
                continue
            status = "; ".join(row["reasons"]) if row["reasons"] else str(row["health"])
            print(
                "\t".join((
                    str(row["worker"]),
                    str(row["state"]),
                    str(row["target"]),
                    str(row["attempt_count"]),
                    str(row["current_score"]),
                    str(row["best_score"]),
                    str(row["best_mismatch_class"]),
                    str(row["base"])[:12],
                    str(row["commit"])[:12],
                    f"{int(row['progress_age_seconds']) // 60}m: {row['progress']}",
                    timestamp(parse_unix_timestamp(int(row["deadline_unix"]), "heartbeat deadline")),
                    status,
                ))
            )
        for record, _ in stale_records:
            print(f"guidance: {graceful_stop_guidance(str(record['worker']))}")
    if malformed_records:
        raise CrewError(f"{len(malformed_records)} malformed heartbeat(s)")
    if stale_records and (args.check or getattr(args, "json", False)):
        raise CrewError(f"{len(stale_records)} stale heartbeat(s)")


def command_queue(args: argparse.Namespace, root: Path) -> None:
    load_config(root)
    path = root / "queue.md"
    if not path.is_file():
        raise CrewError(f"missing queue: {path}")
    print(path.read_text(encoding="utf-8"), end="")


def command_set_queue(args: argparse.Namespace, root: Path) -> None:
    load_config(root)
    if args.role != "leader":
        raise CrewError("only leader may replace queue.md")
    body = body_from_args(args)
    if not body:
        raise CrewError("queue body is empty")
    atomic_write(root / "queue.md", body.rstrip() + "\n")
    print(root / "queue.md")


def registered_worktrees() -> dict[str, str]:
    rows: dict[str, str] = {}
    path: str | None = None
    for line in run_git("worktree", "list", "--porcelain").splitlines():
        if line.startswith("worktree "):
            path = str(Path(line.removeprefix("worktree ")).resolve())
        elif line.startswith("branch refs/heads/") and path:
            rows[path] = line.removeprefix("branch refs/heads/")
        elif not line:
            path = None
    return rows


def command_doctor(args: argparse.Namespace, root: Path) -> None:
    config = load_config(root)
    problems: list[str] = []
    roles = configured_roles(config)
    for role in roles:
        for path in (root / "inbox" / role, root / "archive" / role):
            if not path.is_dir():
                problems.append(f"missing directory {path}")
        path = root / "status" / f"{role}.md"
        if not path.is_file():
            problems.append(f"missing status {path}")
    if not (root / "queue.md").is_file():
        problems.append(f"missing queue {root / 'queue.md'}")

    worktrees = registered_worktrees()
    for row in config["workers"]:
        path = str(Path(str(row["worktree"])).resolve())
        expected = str(row["parking_branch"])
        actual = worktrees.get(path)
        if actual is None:
            problems.append(f"worker worktree is not registered: {path}")
        elif actual != expected:
            problems.append(f"{row['role']} is on {actual}, expected {expected}")

    if problems:
        for problem in problems:
            print(f"ERROR: {problem}")
        raise CrewError(f"doctor found {len(problems)} problem(s)")
    print(f"OK: {root} ({', '.join(roles)})")


def add_body_arguments(parser: argparse.ArgumentParser) -> None:
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--body", help="Message body as one argument")
    group.add_argument("--body-file", help="Read body from this file, or - for stdin")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, help="Override the Git-common crew directory")
    commands = parser.add_subparsers(dest="command", required=True)

    init = commands.add_parser("init", help="Initialize the leader and two worker mailboxes")
    init.add_argument(
        "--worker",
        action="append",
        default=[],
        metavar="ROLE=LANE",
        help="Add one worker and its permanent lane (exactly two required)",
    )
    init.add_argument("--integration-branch", default="campaign/unchain")
    init.set_defaults(function=command_init)

    root = commands.add_parser("root", help="Print the shared crew directory")
    root.set_defaults(function=lambda args, crew_root: print(crew_root))

    send = commands.add_parser("send", help="Atomically append an immutable inbox message")
    send.add_argument("--from", dest="sender", required=True)
    send.add_argument("--to", dest="recipient", required=True)
    send.add_argument("--type", dest="kind", choices=MESSAGE_TYPES, required=True)
    send.add_argument("--task", required=True)
    send.add_argument("--subject", default="")
    add_body_arguments(send)
    send.set_defaults(function=command_send)

    inbox = commands.add_parser("inbox", help="List one role's pending messages")
    inbox.add_argument("role")
    inbox.set_defaults(function=command_inbox)

    read = commands.add_parser("read", help="Read one message")
    read.add_argument("role")
    read.add_argument("message")
    read.add_argument("--archive", action="store_true")
    read.set_defaults(function=command_read)

    archive = commands.add_parser("archive", help="Archive a processed message")
    archive.add_argument("role")
    archive.add_argument("message")
    archive.set_defaults(function=command_archive)

    ack = commands.add_parser("ack", help="Acknowledge and archive one received message")
    ack.add_argument("role")
    ack.add_argument("message")
    add_body_arguments(ack)
    ack.set_defaults(function=command_ack)

    status = commands.add_parser("status", help="Show one or all actor-owned status files")
    status.add_argument("role", nargs="?")
    status.set_defaults(function=command_status)

    set_status = commands.add_parser("set-status", help="Replace the caller's status file")
    set_status.add_argument("role")
    set_status.add_argument("--state", choices=STATES, required=True)
    set_status.add_argument("--task", default="none")
    set_status.add_argument("--branch", default="none")
    set_status.add_argument("--ownership", default="unassigned")
    set_status.add_argument("--compile-permit", choices=("idle", "held"), default="idle")
    set_status.add_argument("--summary", default="")
    set_status.set_defaults(function=command_set_status)

    heartbeat = commands.add_parser(
        "heartbeat", aliases=("checkpoint",),
        help="Record one worker's target, matching checkpoint, commit, and deadline"
    )
    heartbeat.add_argument("--worker", required=True)
    heartbeat.add_argument("--target", help="Exact assigned target (required for a new record)")
    heartbeat.add_argument("--base", help="Assignment base commit (required for a new record)")
    heartbeat.add_argument(
        "--deadline-unix", type=int, help="Soft deadline (required for a new record)"
    )
    heartbeat.add_argument("--progress", required=True, help="One-line material progress update")
    heartbeat.add_argument("--attempt-count", type=int, help="Completed bounded source attempts")
    heartbeat.add_argument("--best-score", help="Concise best comparison score")
    heartbeat.add_argument("--mismatch-class", help="Lowercase mismatch classification slug")
    heartbeat.add_argument(
        "--wb-summary",
        help="Fresh repository-relative build/ path containing mickey-wb-summary-v1 JSON",
    )
    heartbeat.add_argument(
        "--archive-best", action="store_true",
        help="Archive source/object/summary when this checkpoint is a strict new best",
    )
    heartbeat.add_argument("--source", help="Exact tracked source path for --archive-best")
    heartbeat.add_argument(
        "--candidate-object", help="Ignored build/ object path for --archive-best"
    )
    heartbeat.add_argument("--target-words", type=int)
    heartbeat.add_argument("--candidate-words", type=int)
    heartbeat.add_argument("--raw-differing-words", type=int)
    heartbeat.add_argument("--relocation-masked-differing-words", type=int)
    heartbeat.add_argument("--candidate-relocations", type=int)
    heartbeat.add_argument("--target-relocations", type=int)
    heartbeat.add_argument("--exact-relocation-identities", type=int)
    heartbeat.add_argument("--promotion-state", choices=PROMOTION_STATES)
    heartbeat.add_argument("--eta-unix", type=int, help="Optional expected handoff Unix timestamp")
    heartbeat.add_argument(
        "--state", choices=HEARTBEAT_STATES,
        default="active",
    )
    heartbeat.set_defaults(function=command_heartbeat)

    heartbeat_status = commands.add_parser(
        "heartbeat-status", help="Report current and stale workers without stopping them"
    )
    heartbeat_status.add_argument("--worker")
    heartbeat_status.add_argument("--stale-after-minutes", type=int, default=15)
    heartbeat_status.add_argument(
        "--json", action="store_true",
        help="Emit concise JSON and exit nonzero when any record is unhealthy",
    )
    heartbeat_status.add_argument(
        "--check", action="store_true", help="Exit nonzero when an active heartbeat is stale"
    )
    heartbeat_status.set_defaults(function=command_heartbeat_status)

    restore_best = commands.add_parser(
        "restore-best", help="Restore the archived best source with a recoverable backup"
    )
    restore_best.add_argument("--worker", required=True)
    restore_best.add_argument(
        "--recover-backup", metavar="ARTIFACT_ID/BACKUP_ID",
        help="Recover one source displaced by an earlier restore",
    )
    restore_best.set_defaults(function=command_restore_best)

    reprove_best = commands.add_parser(
        "reprove-best", help="Re-run wb_compare for the restored source and refresh its checkpoint"
    )
    reprove_best.add_argument("--worker", required=True)
    reprove_best.add_argument("--progress", required=True)
    reprove_best.add_argument("--attempt-count", type=int)
    reprove_best.add_argument("--mismatch-class")
    reprove_best.set_defaults(function=command_reprove_best)

    queue = commands.add_parser("queue", help="Show the leader-owned ready queue")
    queue.set_defaults(function=command_queue)

    set_queue = commands.add_parser("set-queue", help="Replace the leader-owned ready queue")
    set_queue.add_argument("--by", dest="role", required=True)
    add_body_arguments(set_queue)
    set_queue.set_defaults(function=command_set_queue)

    doctor = commands.add_parser("doctor", help="Check mailbox and permanent worktree wiring")
    doctor.set_defaults(function=command_doctor)
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    try:
        if getattr(args, "stale_after_minutes", 1) <= 0:
            raise CrewError("stale-after-minutes must be positive")
        root = (args.root.resolve() if args.root else default_crew_root())
        args.function(args, root)
    except (CrewError, OSError) as error:
        print(f"crew: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
