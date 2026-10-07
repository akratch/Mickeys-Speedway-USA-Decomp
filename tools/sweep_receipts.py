#!/usr/bin/env python3
"""Untracked receipts and immutable evidence for bounded permuter searches.

Receipts live in Git's common directory so independent lanes share completed
search knowledge without opening one another's worktrees. A kernel file lock
owns each search key; crashes release it without stale PID bookkeeping.
"""
from __future__ import annotations

import contextlib
import fcntl
import hashlib
import io
import json
import os
import re
import subprocess
import stat
import tempfile
import time
import uuid
import zipfile
from pathlib import Path

def loaded_implementations(paths):
    """Pin proof implementation files at module load for subsequent drift checks."""
    return {name: (Path(path), hashlib.sha256(Path(path).read_bytes()).hexdigest())
            for name, path in paths}


SCHEMA = 4
MAX_ARTIFACT_BYTES = 128 * 1024 * 1024  # Encoded ZIP, including metadata.
MAX_PAYLOAD_BYTES = 120 * 1024 * 1024
MAX_ARTIFACT_ENTRIES = 4096
MAX_MEMBER_NAME = 512
REQUIRED_ARTIFACTS = {"baseline/base.c", "baseline/base.o", "baseline/compile.sh",
                      "baseline/target.s", "baseline/settings.toml", "baseline/recipe.json",
                      "baseline/tu.c", "baseline/permuter_settings.toml",
                      "baseline/compiled.c", "baseline/compiled.o", "baseline/measurement.json",
                      "best/source.c", "best/object.o", "context/baseline.c",
                      "context/winner.c", "context/report.json"}


def seed_candidate_name(value: str) -> str:
    """A vendor output identifier, never an arbitrary path or source input."""
    if not isinstance(value, str) or re.fullmatch(r"output-(0|[1-9][0-9]{0,9})-([1-9][0-9]{0,9})", value) is None:
        raise ValueError("seed candidate must be an output-SCORE-ORDINAL identifier")
    return value


def resolve_seed_candidate(parent: dict, files: dict[str, bytes], candidate: str) -> tuple[bytes, dict]:
    """Resolve only already authenticated archive members; leave parent untouched.

    The source/score pair is retained vendor evidence, not a retained worker
    object or semantic approval. Fresh emission and actual-search proofs remain
    mandatory. Reused by durable validation, not just the launch-side selector.
    """
    seed_candidate_name(candidate)
    if (not isinstance(parent, dict) or not isinstance(parent.get("result"), dict)
            or not isinstance(parent.get("inputs"), dict) or not isinstance(files, dict)):
        raise ValueError("invalid alternate seed parent or archive shape")
    result = parent["result"]
    report = json.loads(files["context/report.json"])
    original = result.get("original_base_score") if result.get("seed_proof") else result.get("base_score")
    if (not isinstance(report, dict)
            or parent.get("schema") != SCHEMA or parent.get("key") != digest(parent["inputs"])
            or report != result.get("context_review") or report.get("status") != "unchanged"
            or report.get("winner_source_sha256") != hashlib.sha256(files["best/source.c"]).hexdigest()
            or report.get("baseline_source_sha256") != hashlib.sha256(files["baseline/compiled.c"]).hexdigest()
            or type(original) is not int or type(result.get("best_score")) is not int
            or not 0 <= result["best_score"] < original):
        raise ValueError("alternate seed parent is not a context-unchanged measured improvement")
    prefix = "attempt/scratch/" + candidate
    source_member, score_member = prefix + "/source.c", prefix + "/score.txt"
    source, score_bytes = files[source_member], files[score_member]
    if (not source or len(source) > 4 * 1024 * 1024
            or re.fullmatch(rb"(0|[1-9][0-9]{0,9})\n", score_bytes) is None):
        raise ValueError("alternate seed source or paired score is invalid")
    source.decode("utf-8")  # Reject malformed source before importer preparation.
    score = int(score_bytes)
    if score != int(candidate.split("-")[1]) or not 0 <= score < original:
        raise ValueError("alternate seed score is unpaired or not a canonical improvement")
    return source, {"schema": "mickey-seed-selection-v1", "candidate": candidate,
        "receipt": parent["key"], "bundle": result["artifact_bundle"],
        "parent_receipt_sha256": digest(parent),
        "source_member": source_member, "source_sha256": hashlib.sha256(source).hexdigest(),
        "score_member": score_member, "score_sha256": hashlib.sha256(score_bytes).hexdigest(),
        "selected_score": score, "parent_best_score": result["best_score"]}


def owned_bytes(root: Path, relative: str, *, limit: int = MAX_ARTIFACT_BYTES,
                deadline: float | None = None) -> bytes:
    """Read only regular files below an owned directory, without following links."""
    parts = relative.split("/")
    if any(p in ("", ".", "..") for p in parts) or relative.startswith("/"):
        raise ValueError("unsafe artifact path")
    fd = os.open(root, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
    try:
        for part in parts[:-1]:
            child = os.open(part, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW, dir_fd=fd)
            os.close(fd)
            fd = child
        file_fd = os.open(parts[-1], os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK, dir_fd=fd)
        with os.fdopen(file_fd, "rb") as stream:
            if not stat.S_ISREG(os.fstat(stream.fileno()).st_mode):
                raise ValueError("artifact is not a regular file")
            if os.fstat(stream.fileno()).st_size > limit:
                raise ValueError("artifact exceeds preservation byte cap")
            chunks, size = [], 0
            while True:
                if deadline is not None and time.monotonic() >= deadline:
                    raise TimeoutError("artifact preservation deadline reached")
                chunk = stream.read(min(1024 * 1024, limit - size + 1))
                if not chunk:
                    return b"".join(chunks)
                size += len(chunk)
                if size > limit:
                    raise ValueError("artifact exceeds preservation byte cap")
                chunks.append(chunk)
    finally:
        os.close(fd)


def attempt_files(root: Path, *, byte_budget: int = MAX_PAYLOAD_BYTES,
                  entry_budget: int = MAX_ARTIFACT_ENTRIES,
                  deadline: float | None = None) -> tuple[dict[str, bytes], list[str]]:
    files, errors = {}, []
    pending, entries = [root], 0
    try:
        while pending:
            with os.scandir(pending.pop()) as children:
                for child in children:
                    if deadline is not None and time.monotonic() >= deadline:
                        raise TimeoutError("artifact preservation deadline reached")
                    entries += 1
                    if entries > entry_budget or byte_budget <= 0:
                        raise ValueError("artifact preservation entry/byte budget exhausted")
                    path = Path(child.path)
                    rel = path.relative_to(root).as_posix()
                    if len(("attempt/" + rel).encode()) > MAX_MEMBER_NAME:
                        raise ValueError("artifact member name exceeds metadata cap")
                    if child.is_dir(follow_symlinks=False):
                        pending.append(path)
                        continue
                    data = owned_bytes(root, rel, limit=byte_budget, deadline=deadline)
                    byte_budget -= len(data)
                    files["attempt/" + rel] = data
    except (OSError, ValueError) as error:
        errors.append(str(error))
    return files, errors


def digest(value: object) -> str:
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


def file_digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def tree_digest(root: Path, *, source_only: bool = False) -> str:
    """Hash actual files, including dirty source, without build/cache churn."""
    excluded = {".git", ".venv", "__pycache__", ".pytest_cache", "build", "nonmatchings", "test"}
    rows = []
    for directory, dirs, names in os.walk(root, followlinks=True):
        dirs[:] = sorted(d for d in dirs if d not in excluded)
        for name in sorted(names):
            path = Path(directory) / name
            if source_only and path.suffix not in {".py", ".toml", ".inc", ".h", ".c", ".txt", ".sh"}:
                continue
            if path.is_file() and not name.startswith("debug_"):
                rows.append((path.relative_to(root).as_posix(), file_digest(path)))
    if not rows:
        raise ValueError(f"no tool identity inputs under {root}")
    return digest(rows)


def atomic_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", dir=path.parent, delete=False) as stream:
            temporary = Path(stream.name)
            json.dump(value, stream, sort_keys=True, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def reusable(result: dict) -> bool:
    """A failed, interrupted, or unpromoted zero-score search is retryable."""
    return (isinstance(result, dict) and result.get("ok") is True
            and type(result.get("base_score")) is int and result["base_score"] >= 0
            and not any(result.get(k) for k in
                        ("error", "promote_error", "commit_error", "stopped_batch"))
            and (not result.get("zero_found") or result.get("promoted") is True))


class ReceiptStore:
    def __init__(self, root: Path):
        self.root = root

    @classmethod
    def for_repo(cls, repo: Path) -> "ReceiptStore":
        common = subprocess.check_output(
            ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"],
            cwd=repo, text=True).strip()
        return cls(Path(common) / "mickey-sweep-receipts" / f"v{SCHEMA}")

    def directory(self, key: str) -> Path:
        if len(key) != 64 or any(c not in "0123456789abcdef" for c in key):
            raise ValueError("invalid receipt key")
        return self.root / key[:2] / key

    def seed_parent(self, key: str) -> tuple[dict, bytes]:
        """Read a proven-complete improvement as input, never as match proof."""
        self.directory(key)
        value = self.completed(key)
        if value is None:
            raise ValueError("seed receipt is missing, incomplete or corrupt")
        result = value["result"]
        original = result.get("original_base_score") if result.get("seed_proof") else result.get("base_score")
        if (result["context_review"]["status"] != "unchanged"
                or type(result.get("best_score")) is not int
                or type(original) is not int
                or not 0 <= result["best_score"] < original):
            raise ValueError("seed receipt has no context-unchanged measured improvement")
        files = self.read_bundle(result["artifact_bundle"], inputs=value["inputs"])
        return value, files["best/source.c"]

    def select_seed(self, key: str, candidate: str | None = None) -> tuple[dict, bytes, dict | None]:
        """Default behavior is unchanged; alternates carry independent evidence."""
        if candidate is not None:
            seed_candidate_name(candidate)
        parent, source = self.seed_parent(key)
        if candidate is None:
            return parent, source, None
        files = self.read_bundle(parent["result"]["artifact_bundle"], inputs=parent["inputs"])
        source, selection = resolve_seed_candidate(parent, files, candidate)
        return parent, source, selection

    def save_bundle(self, files: dict[str, bytes], *, complete: bool, inputs: dict) -> str:
        """Immutable deterministic archive; never extract or execute archive paths."""
        if complete and not REQUIRED_ARTIFACTS <= files.keys():
            raise ValueError("incomplete artifact bundle")
        if sum(map(len, files.values())) > MAX_PAYLOAD_BYTES or len(files) > MAX_ARTIFACT_ENTRIES:
            raise ValueError("bundle exceeds preservation byte cap")
        manifest = {"schema": SCHEMA, "complete": complete, "files": {},
                    "receipt_key": digest(inputs), "context_key": digest(inputs["context"]),
                    "baseline_hashes": inputs.get("baseline_hashes", {})}
        if complete and (not manifest["baseline_hashes"] or any(
                name not in files or hashlib.sha256(files[name]).hexdigest() != expected
                for name, expected in manifest["baseline_hashes"].items())):
            raise ValueError("baseline input binding mismatch")
        output = io.BytesIO()
        with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_STORED) as archive:
            for name, data in sorted(files.items()):
                if any(p in ("", ".", "..") for p in name.split("/")) or "\\" in name or name == "manifest.json":
                    raise ValueError("unsafe bundle member")
                if len(name.encode()) > MAX_MEMBER_NAME:
                    raise ValueError("bundle member name exceeds metadata cap")
                manifest["files"][name] = {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}
                archive.writestr(zipfile.ZipInfo(name), data)
            archive.writestr(zipfile.ZipInfo("manifest.json"), json.dumps(manifest, sort_keys=True).encode())
        data = output.getvalue()
        if len(data) > MAX_ARTIFACT_BYTES:
            raise ValueError("encoded bundle exceeds preservation byte cap")
        key = hashlib.sha256(data).hexdigest()
        directory = self.root / "bundles"
        directory.mkdir(parents=True, exist_ok=True)
        if directory.is_symlink():
            raise ValueError("bundle directory must not be a symlink")
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=directory, delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(data)
                stream.flush()
                os.fsync(stream.fileno())
            try:
                os.link(temporary, directory / (key + ".zip"))
            except FileExistsError:
                if owned_bytes(directory, key + ".zip") != data:
                    raise ValueError("existing content-addressed bundle is corrupt; preserved for review")
            dir_fd = os.open(directory, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
            try:
                os.fsync(dir_fd)
            finally:
                os.close(dir_fd)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)
        return key

    def read_bundle(self, key: str, *, require_complete: bool = True,
                    inputs: dict | None = None) -> dict[str, bytes]:
        self.directory(key)  # Validate digest before constructing a filesystem path.
        data = owned_bytes(self.root / "bundles", key + ".zip")
        if hashlib.sha256(data).hexdigest() != key:
            raise ValueError("artifact bundle digest mismatch")
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            if (len(archive.infolist()) > MAX_ARTIFACT_ENTRIES + 1
                    or sum(info.file_size for info in archive.infolist()) > MAX_ARTIFACT_BYTES
                    or any(info.compress_type != zipfile.ZIP_STORED for info in archive.infolist())):
                raise ValueError("unsupported or oversized artifact archive")
            manifest = json.loads(archive.read("manifest.json"))
            if not isinstance(manifest, dict) or not isinstance(manifest.get("files"), dict):
                raise ValueError("invalid artifact manifest")
            files = manifest["files"]
            if inputs is not None and (manifest.get("receipt_key") != digest(inputs)
                    or manifest.get("context_key") != digest(inputs["context"])
                    or manifest.get("baseline_hashes") != inputs.get("baseline_hashes", {})):
                raise ValueError("bundle belongs to different receipt inputs")
            if manifest.get("schema") != SCHEMA or (require_complete and
                    (manifest.get("complete") is not True or not REQUIRED_ARTIFACTS <= files.keys())):
                raise ValueError("incomplete artifact bundle")
            if len(archive.namelist()) != len(files) + 1 or set(archive.namelist()) != {*files, "manifest.json"}:
                raise ValueError("unexpected artifact bundle members")
            result = {}
            for name, metadata in files.items():
                if not isinstance(metadata, dict):
                    raise ValueError("invalid artifact member metadata")
                if any(p in ("", ".", "..") for p in name.split("/")) or "\\" in name:
                    raise ValueError("unsafe artifact bundle member")
                content = archive.read(name)
                if len(content) != metadata["size"] or hashlib.sha256(content).hexdigest() != metadata["sha256"]:
                    raise ValueError("artifact member digest mismatch")
                result[name] = content
            for name, expected in manifest.get("baseline_hashes", {}).items():
                if require_complete and (name not in result or hashlib.sha256(result[name]).hexdigest() != expected):
                    raise ValueError("baseline input digest mismatch")
            return result

    def artifacts_valid(self, value: dict) -> bool:
        if not isinstance(value, dict):
            return False
        inputs = value.get("inputs")
        if (not isinstance(inputs, dict) or not isinstance(value.get("result"), dict)
                or not isinstance(inputs.get("search"), dict)
                or not isinstance(inputs.get("context"), dict)):
            return False
        try:
            files = self.read_bundle(value["result"]["artifact_bundle"], inputs=value["inputs"])
            seed = value["inputs"]["search"].get("seed")
            if seed is not None:
                if not isinstance(seed, dict):
                    return False
                proof = json.loads(files["seed/proof.json"])
                parent = json.loads(files["seed/parent.json"])
                if (not isinstance(parent, dict) or not isinstance(parent.get("result"), dict)
                        or not isinstance(parent.get("inputs"), dict)):
                    return False
                fidelity = json.loads(files["seed/fidelity.json"])
                search_fidelity = json.loads(files["seed/search-fidelity.json"])
                plan = json.loads(files["seed/plan.json"])
                parent_files = self.read_bundle(seed["bundle"], inputs=parent["inputs"])
                selection = seed.get("selection")
                selected_source, selected_score = parent_files["best/source.c"], parent["result"]["best_score"]
                if "selection" in seed:
                    if (not isinstance(selection, dict)
                            or type(selection.get("selected_score")) is not int
                            or type(selection.get("parent_best_score")) is not int):
                        return False
                    selected_source, expected = resolve_seed_candidate(parent, parent_files, selection.get("candidate"))
                    if selection != expected or json.loads(files["seed/selection.json"]) != expected:
                        return False
                    selected_score = expected["selected_score"]
                elif ("seed/selection.json" in files
                        or parent["result"]["context_review"]["winner_source_sha256"] != seed.get("source_sha256")):
                    return False
                if (not isinstance(seed, dict) or not isinstance(proof, dict)
                        or not isinstance(fidelity, dict) or not isinstance(plan, dict)
                        or not isinstance(search_fidelity, dict)
                        or proof.get("search_fidelity") != search_fidelity
                        or search_fidelity.get("contract") != "mickey-seed-emission-v1"
                        or search_fidelity.get("source_fidelity_exact") is not True
                        or search_fidelity.get("inputs_sha256") != digest(inputs)
                        or search_fidelity.get("measured_object_sha256") != proof.get("compiled_object_sha256")
                        or search_fidelity.get("search_object_sha256") != proof.get("search_object_sha256")
                        or type(search_fidelity.get("owned_bytes")) is not int
                        or search_fidelity.get("owned_bytes") != fidelity.get("owned_bytes")
                        or type(search_fidelity.get("relocation_count")) is not int
                        or search_fidelity.get("relocation_count") != fidelity.get("relocation_count")
                        or search_fidelity.get("identity_route") not in (
                            "runtime-identities", "raw-source-symbols-not-runtime-proof")
                        or seed.get("preparation_contract") != "mickey-seed-emission-v1"
                        or fidelity.get("contract") != seed["preparation_contract"]
                        or proof.get("fidelity") != fidelity
                        or fidelity.get("source_fidelity_exact") is not True
                        or type(fidelity.get("owned_bytes")) is not int or fidelity["owned_bytes"] <= 0
                        or type(fidelity.get("relocation_count")) is not int or fidelity["relocation_count"] < 0
                        or fidelity.get("identity_route") not in (
                            "runtime-identities", "raw-source-symbols-not-runtime-proof")
                        or fidelity.get("inputs_sha256") != digest(inputs)
                        or fidelity.get("original_source_sha256") != seed.get("source_sha256")
                        or fidelity.get("original_object_sha256") != hashlib.sha256(files["seed/original.o"]).hexdigest()
                        or not files["seed/original.o"]
                        or fidelity.get("emitted_source_sha256") != proof.get("compiled_source_sha256")
                        or fidelity.get("emitted_object_sha256") != proof.get("compiled_object_sha256")
                        or fidelity.get("recipe_sha256") != hashlib.sha256(files["baseline/compile.sh"]).hexdigest()
                        or seed.get("prepared_source_sha256") != hashlib.sha256(files["seed/prepared.c"]).hexdigest()
                        or seed.get("plan_sha256") != hashlib.sha256(files["seed/plan.json"]).hexdigest()
                        or plan.get("contract") != "original-coordinate-sameline-v1"
                        or plan.get("symbol") != inputs["context"]["identity"]["symbol"]
                        or not isinstance(plan.get("groups"), list)
                        or not ((plan.get("status") == "ungrouped" and not plan["groups"])
                            or (plan.get("status") == "preserved" and bool(plan["groups"]))
                            or (plan.get("status") == "measurement-required" and not plan["groups"]
                                and isinstance(plan.get("reason"), str) and bool(plan["reason"])))):
                    return False
                if (not isinstance(seed, dict) or not isinstance(proof, dict)
                        or not isinstance(parent, dict)
                        or proof != value["result"].get("seed_proof")
                        or proof.get("seed") != seed
                        or value["inputs"]["context"].get("seed") != seed
                        or seed.get("receipt") != parent.get("key")
                        or digest(parent.get("inputs")) != seed["receipt"]
                        or parent["result"]["artifact_bundle"] != seed.get("bundle")
                        or parent["inputs"]["context"]["identity"] != value["inputs"]["context"]["identity"]
                        or selected_score != value["result"].get("seed_parent_score")
                        or hashlib.sha256(files["seed/source.c"]).hexdigest() != seed.get("source_sha256")
                        or hashlib.sha256(files["baseline/target.o"]).hexdigest() != seed.get("target_object_sha256")
                        or proof.get("status") != "validated"
                        or type(proof.get("original_score")) is not int
                        or type(proof.get("seed_score")) is not int
                        or proof["seed_score"] != value["result"].get("seed_score")
                        or proof["seed_score"] != value["result"].get("seed_parent_score")
                        or proof["seed_score"] != value["result"].get("base_score")
                        or value["result"].get("search_gain") != proof["seed_score"] - value["result"]["best_score"]
                        or proof["original_score"] != value["result"].get("original_base_score")
                        or proof.get("fresh_baseline_sha256") != hashlib.sha256(files["context/baseline.c"]).hexdigest()
                        or hashlib.sha256(files["seed/compiled.c"]).hexdigest() != proof.get("compiled_source_sha256")
                        or hashlib.sha256(files["seed/compiled.o"]).hexdigest() != proof.get("compiled_object_sha256")
                        or not files["seed/compiled.o"]
                        or files["seed/search.c"] != files["seed/compiled.c"]
                        or hashlib.sha256(files["seed/search.o"]).hexdigest() != proof.get("search_object_sha256")
                        or not files["seed/search.o"]):
                    return False
                if selected_source != files["seed/source.c"]:
                    return False
                for name, source in (("parent_comparison", files["seed/source.c"]),
                                     ("compiled_comparison", files["seed/compiled.c"])):
                    review = proof[name]
                    if (not isinstance(review, dict) or review.get("status") != "unchanged"
                            or review.get("schema") != "mickey-prepared-context-review-v1"
                            or review.get("symbol") != value["inputs"]["context"]["identity"]["symbol"]
                            or review.get("canonical_source_sha256") != value["inputs"]["context"]["source"]
                            or review.get("comparator_identity") != value["inputs"]["context"]["tools"]["candidate_context"]
                            or review.get("baseline_source_sha256") != proof["fresh_baseline_sha256"]
                            or review.get("winner_source_sha256") != hashlib.sha256(source).hexdigest()
                            or not isinstance(review.get("capture_binding"), dict)
                            or review.get("capture_binding", {}).get("inputs_sha256") != digest(value["inputs"])):
                        return False
                    comparison = review.get("comparison")
                    if (not isinstance(comparison, dict)
                            or comparison.get("schema") != "mickey-candidate-context-v1"
                            or comparison.get("status") != "unchanged"
                            or comparison.get("symbol") != review["symbol"]
                            or comparison.get("baseline_sha256") != review["baseline_source_sha256"]
                            or comparison.get("winner_sha256") != review["winner_source_sha256"]
                            or comparison.get("changes") != []):
                        return False
            report = json.loads(files["context/report.json"])
            if not isinstance(report, dict):
                return False
            context = value["inputs"]["context"]
            if (report != value["result"].get("context_review")
                    or report.get("schema") != "mickey-prepared-context-review-v1"
                    or report.get("status") not in {"unchanged", "changed"}
                    or report.get("symbol") != context["identity"]["symbol"]
                    or report.get("canonical_source_sha256") != context["source"]
                    or report.get("comparator_identity") != context["tools"]["candidate_context"]
                    or files["context/baseline.c"] != files["baseline/compiled.c"]
                    or files["context/winner.c"] != files["best/source.c"]
                    or report.get("baseline_source_sha256") != hashlib.sha256(files["context/baseline.c"]).hexdigest()
                    or report.get("winner_source_sha256") != hashlib.sha256(files["context/winner.c"]).hexdigest()):
                return False
            capture = json.loads(files["baseline/measurement.json"])
            if not isinstance(capture, dict):
                return False
            binding = report.get("capture_binding")
            if (not isinstance(binding, dict) or capture.get("binding") != binding
                    or binding.get("inputs_sha256") != digest(value["inputs"])
                    or not isinstance(binding.get("run_id"), str) or not binding["run_id"]):
                return False
            comparison = report.get("comparison")
            if (not isinstance(comparison, dict)
                    or comparison.get("schema") != "mickey-candidate-context-v1"
                    or comparison.get("status") != report["status"]
                    or comparison.get("symbol") != report["symbol"]
                    or comparison.get("baseline_sha256") != report["baseline_source_sha256"]
                    or comparison.get("winner_sha256") != report["winner_source_sha256"]
                    or not isinstance(comparison.get("changes"), list)
                    or len(comparison["changes"]) > 32):
                return False
            if (type(capture.get("returncode")) is not int or capture["returncode"] != 0
                    or capture.get("source_sha256") != report["baseline_source_sha256"]
                    or capture.get("object_sha256") != hashlib.sha256(files["baseline/compiled.o"]).hexdigest()
                    or not files["baseline/compiled.o"]):
                return False
            return True
        except (OSError, ValueError, KeyError, TypeError, zipfile.BadZipFile):
            return False

    @contextlib.contextmanager
    def claim(self, key: str):
        directory = self.directory(key)
        directory.mkdir(parents=True, exist_ok=True)
        with (directory / "search.lock").open("a") as lock:
            try:
                fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                yield False
                return
            try:
                yield True
            finally:
                fcntl.flock(lock, fcntl.LOCK_UN)

    def completed(self, key: str) -> dict | None:
        try:
            value = json.loads((self.directory(key) / "complete.json").read_text())
        except (OSError, ValueError):
            return None
        if (not isinstance(value, dict) or value.get("schema") != SCHEMA or value.get("key") != key
                or digest(value.get("inputs")) != key
                or not reusable(value.get("result", {})) or not self.artifacts_valid(value)):
            return None
        return value

    def record(self, inputs: dict, result: dict) -> str:
        """Called while holding claim(). Retain attempts and best scalar score."""
        key = digest(inputs)
        directory = self.directory(key)
        # Scalar JSON only references the separate ignored evidence bundle.
        scalar_fields = ("func", "c_file", "overlay", "ok", "base_score", "best_score",
                         "zero_found", "promoted", "seconds", "extended", "stopped_flat",
                         "stopped_batch", "annotated_relocs", "scratch_path", "artifact_bundle", "context_review",
                         "original_base_score", "seed_score", "seed_parent_score", "seed_proof", "search_gain")
        scalar = {k: result.get(k) for k in scalar_fields}
        for name in ("error", "promote_error", "commit_error"):
            scalar[name] = bool(result.get(name))
        value = {"schema": SCHEMA, "key": key, "inputs": inputs,
                 "finished": time.time(), "result": scalar}
        atomic_json(directory / "attempts" / f"{time.time_ns()}-{uuid.uuid4().hex}.json", value)
        if reusable(scalar) and self.artifacts_valid(value):
            atomic_json(directory / "complete.json", value)
            # Each settings key owns its own context-index file; writers do
            # not share a read/modify/write index and cannot lose one another.
            context_path = self.root / "contexts" / digest(inputs["context"]) / f"{key}.json"
            try:
                old_value = json.loads(context_path.read_text())
                if not self.artifacts_valid(old_value):
                    raise ValueError("previous artifacts unavailable")
                previous = old_value["result"]
                old_score = min(v for v in (previous.get("base_score"), previous.get("best_score"))
                                if isinstance(v, int))
            except (OSError, ValueError, KeyError, TypeError):
                old_score = None
            new_score = min(v for v in (scalar["base_score"], scalar["best_score"])
                            if isinstance(v, int))
            if old_score is None or new_score < old_score:
                atomic_json(context_path, value)
        scores = [scalar[k] for k in ("best_score", "base_score") if isinstance(scalar[k], int)]
        if scores:
            usable = reusable(scalar) and self.artifacts_valid(value)
            best_path = directory / ("best.json" if usable else "partial-best.json")
            try:
                previous = json.loads(best_path.read_text())
                if usable and (not reusable(previous.get("result", {})) or not self.artifacts_valid(previous)):
                    raise ValueError("previous usable-best artifacts unavailable")
                previous_score = previous["score"]
            except (OSError, ValueError, KeyError):
                previous_score = None
            if previous_score is None or min(scores) < previous_score:
                atomic_json(best_path, {**value, "score": min(scores)})
        return key

    def descending(self, context: dict) -> bool:
        """Find a successful improvement under any caps for this exact input."""
        for path in (self.root / "contexts" / digest(context)).glob("*.json"):
            try:
                value = json.loads(path.read_text())
                result = value["result"]
                if (value.get("schema") == SCHEMA and value.get("key") == digest(value["inputs"])
                        and value["inputs"]["context"] == context and reusable(result) and self.artifacts_valid(value)
                        and isinstance(result.get("best_score"), int)
                        and 0 < result["best_score"] < result["base_score"]):
                    return True
            except (OSError, ValueError, KeyError, TypeError):
                continue
        return False
