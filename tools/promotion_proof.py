#!/usr/bin/env python3
"""Prove that one promoted C function remains exact in the canonical build.

This is deliberately a policy wrapper around ``function_preflight.py``.  The
preflight remains the sole owner of symbol resolution, linked geometry, ROM
comparison, and relocation analysis; this command only checks that its JSON
report satisfies the stricter post-promotion contract.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Callable


REPO = Path(__file__).resolve().parent.parent
PREFLIGHT = REPO / "tools" / "function_preflight.py"
REPORT_SCHEMA = "mickey-function-evidence-preflight-v1"
PROOF_SCHEMA = "mickey-promotion-proof-v1"


class ProofError(RuntimeError):
    """The preflight report does not prove an exact promoted function."""


Runner = Callable[..., subprocess.CompletedProcess[str]]


def _integer(value: object, field: str, *, minimum: int = 0) -> int:
    if not isinstance(value, int) or isinstance(value, bool) or value < minimum:
        raise ProofError(f"{field} must be an integer >= {minimum}, got {value!r}")
    return value


def run_preflight(
    symbol: str,
    *,
    no_build: bool = False,
    runner: Runner = subprocess.run,
) -> dict[str, object]:
    command = [sys.executable, str(PREFLIGHT), symbol, "--json"]
    if no_build:
        command.append("--no-build")
    result = runner(
        command,
        cwd=REPO,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode:
        detail = (result.stderr or result.stdout or "preflight failed").strip()
        raise ProofError(
            f"function preflight failed with exit {result.returncode}: {detail}"
        )
    try:
        report = json.loads(result.stdout)
    except (json.JSONDecodeError, TypeError) as error:
        raise ProofError("function preflight did not return one JSON report") from error
    if not isinstance(report, dict):
        raise ProofError("function preflight JSON root is not an object")
    return report


def validate_report(symbol: str, report: dict[str, object]) -> dict[str, object]:
    """Reduce a full preflight report to a compact, fail-closed proof receipt."""

    if report.get("schema") != REPORT_SCHEMA:
        raise ProofError(f"unexpected preflight schema: {report.get('schema')!r}")
    if report.get("requested_symbol") != symbol:
        raise ProofError(
            "preflight report belongs to a different request: "
            f"{report.get('requested_symbol')!r}"
        )
    if report.get("resolution_mode") != "post_promotion":
        raise ProofError(
            f"{symbol} is not post-promotion (mode={report.get('resolution_mode')!r})"
        )
    preflight = report.get("preflight")
    if not isinstance(preflight, dict) or preflight.get("status") != "complete":
        raise ProofError(
            "preflight evidence is not complete: "
            f"status={preflight.get('status') if isinstance(preflight, dict) else None!r}"
        )

    workbench = report.get("workbench")
    if not isinstance(workbench, dict):
        raise ProofError("preflight report lacks workbench evidence")
    if workbench.get("comparison_mode") != "rom":
        raise ProofError(
            "post-promotion proof requires the linked-ROM oracle, got "
            f"{workbench.get('comparison_mode')!r}"
        )
    differing_words = _integer(workbench.get("differing_words"), "differing_words")
    target_words = _integer(workbench.get("target_words"), "target_words", minimum=1)
    candidate_words = _integer(
        workbench.get("candidate_words"), "candidate_words", minimum=1
    )
    if differing_words != 0 or target_words != candidate_words:
        raise ProofError(
            "linked words are not exact: "
            f"differing={differing_words}, target={target_words}, "
            f"candidate={candidate_words}"
        )
    if workbench.get("first_mismatch") is not None:
        raise ProofError(
            "linked comparison reports a first mismatch despite zero differing words"
        )
    if workbench.get("verdict") not in {"exact", "instruction-words-identical"}:
        raise ProofError(
            "linked comparison verdict is not an exact-word verdict: "
            f"{workbench.get('verdict')!r}"
        )
    target_frame = workbench.get("target_frame")
    candidate_frame = workbench.get("candidate_frame")
    if target_frame is not None:
        _integer(target_frame, "target_frame")
    if candidate_frame is not None:
        _integer(candidate_frame, "candidate_frame")
    if target_frame != candidate_frame:
        raise ProofError(
            f"frame is not exact: target={target_frame!r}, candidate={candidate_frame!r}"
        )

    relocation = report.get("relocation_comparison")
    if not isinstance(relocation, dict):
        raise ProofError("preflight report lacks relocation comparison evidence")
    configured_relocations = _integer(relocation.get("candidate_record_count"), "candidate_record_count")
    configured_target_relocations = _integer(relocation.get("target_record_count"), "target_record_count")
    metadata = relocation.get("declared_metadata_proof")
    if metadata is not None:
        original = relocation.get("original_raw_comparison")
        if not isinstance(original, dict) or relocation.get("identity_proof_mode") != "raw-static-with-declared-metadata-filters":
            raise ProofError("missing original raw relocation evidence")
        relocation = dict(original, declared_metadata_proof=metadata)
    target_relocations = _integer(
        relocation.get("target_record_count"), "target_record_count"
    )
    if target_relocations != configured_target_relocations:
        raise ProofError("configured and original raw target counts disagree")
    candidate_relocations = _integer(
        relocation.get("candidate_record_count"), "candidate_record_count"
    )
    effective_identities = _integer(
        relocation.get("effective_identity_alignment_count"),
        "effective_identity_alignment_count",
    )
    if candidate_relocations != target_relocations:
        raise ProofError(
            "relocation counts are not exact: "
            f"target={target_relocations}, candidate={candidate_relocations}"
        )
    if relocation.get("offset_type_exact") is not True:
        raise ProofError("relocation offsets/types are not exact")
    if (
        relocation.get("effective_identity_exact") is not True
        or effective_identities != target_relocations
    ):
        raise ProofError(
            "relocation identities are not exact: "
            f"effective={effective_identities}, target={target_relocations}"
        )

    identity_mode = relocation.get("identity_proof_mode")
    if identity_mode not in {"static", "static-plus-runtime-table-and-linked-rom", "raw-static-with-declared-metadata-filters"}:
        raise ProofError(f"unexpected relocation identity proof mode: {identity_mode!r}")
    metadata = relocation.get("declared_metadata_proof")
    if identity_mode == "raw-static-with-declared-metadata-filters":
        if not isinstance(metadata, dict) or metadata.get("schema") != "mickey-declared-metadata-proof-v1":
            raise ProofError("missing declared metadata accounting receipt")
        raw_count = _integer(metadata.get("raw_count"), "raw_count")
        retained = _integer(metadata.get("retained_count"), "retained_count")
        removed = metadata.get("filtered")
        if (not isinstance(removed, list) or not removed or raw_count != target_relocations
                or retained != configured_relocations
                or retained + len(removed) != raw_count or metadata.get("source_selection") != "ordinary_c"):
            raise ProofError("declared metadata accounting counts/source are inconsistent")
        identities = metadata.get("filtered_identities")
        if not isinstance(identities, list) or len(identities) != len(removed):
            raise ProofError("missing independent filtered identities")
        keys = []
        for row in removed:
            # `.bss` sites are HI/LO proved through the linked BSS base; a
            # named site is any type the filter plan admits, proved by the
            # raw static surface.
            if (not isinstance(row, dict) or not isinstance(row.get("symbol"), str)
                    or not row["symbol"]
                    or type(row.get("offset")) is not int or row["offset"] < 0
                    or row["offset"] % 4 or row["offset"] >= target_words * 4
                    or type(row.get("rtype")) is not int
                    or row["rtype"] not in ((5, 6) if row["symbol"] == ".bss" else (4, 5, 6))):
                raise ProofError("malformed declared filter site")
            keys.append((row["offset"], row["rtype"]))
        identity_keys = []
        for row in identities:
            identity = row.get("identity") if isinstance(row, dict) else None
            if (not isinstance(identity, (list, tuple)) or len(identity) != 2
                    or any(type(value) is not int or value < 0 for value in identity)
                    or type(row.get("offset")) is not int or type(row.get("rtype")) is not int):
                raise ProofError("malformed independently proved filter identity")
            identity_keys.append((row.get("offset"), row.get("rtype")))
        if len(set(keys)) != len(keys) or sorted(keys) != sorted(identity_keys):
            raise ProofError("filtered identity sites disagree with declared removals")
        inputs = metadata.get("inputs")
        if not isinstance(inputs, dict) or not inputs.get("command") or not inputs.get("postprocess"):
            raise ProofError("missing fresh compiler/metadata binding")
        for field in ("raw_sha256", "configured_sha256", "linked_sha256"):
            if not isinstance(metadata.get(field), str) or not re.fullmatch(r"[0-9a-f]{64}", metadata[field]):
                raise ProofError("missing fresh object binding")
        if inputs.get("configured") != metadata["configured_sha256"] or inputs.get("linked") != metadata["linked_sha256"]:
            raise ProofError("fresh configured/linked binding disagrees")

    candidate_symbol = report.get("candidate_symbol")
    linked_symbol = report.get("linked_symbol")
    if not isinstance(candidate_symbol, str) or not candidate_symbol:
        raise ProofError("preflight report lacks a candidate symbol")
    if not isinstance(linked_symbol, str) or not linked_symbol:
        raise ProofError("preflight report lacks a linked symbol")

    return {
        "schema": PROOF_SCHEMA,
        "requested_symbol": symbol,
        "candidate_symbol": candidate_symbol,
        "linked_symbol": linked_symbol,
        "resolution_mode": "post_promotion",
        "comparison_mode": "rom",
        "exact_words": target_words,
        "frame_size": target_frame,
        "exact_relocations": target_relocations,
        "configured_relocations": configured_relocations,
        "identity_proof_mode": identity_mode,
        "declared_metadata_proof": metadata,
        "canonical_commands": [],
        "verdict": "exact",
    }


def canonical_commands() -> tuple[tuple[str, list[str]], ...]:
    """Return bounded canonical proofs without hiding their resource policy."""

    return (
        ("full-ROM identity", ["nice", "-n", "10", "gmake", "-j2", "verify"]),
        (
            "overlay relocation surface",
            ["nice", "-n", "10", "gmake", "-j2", "check-overlay-syms"],
        ),
    )


def run_canonical_proofs(
    *, json_mode: bool = False, runner: Runner = subprocess.run
) -> list[str]:
    commands = canonical_commands()
    completed: list[str] = []
    for index, (label, command) in enumerate(commands, start=1):
        print(
            f"canonical proof [{index}/{len(commands)}]: {label}: {' '.join(command)}",
            file=sys.stderr if json_mode else sys.stdout,
            flush=True,
        )
        result = runner(
            command,
            cwd=REPO,
            check=False,
            text=True,
            stdout=sys.stderr if json_mode else None,
        )
        if result.returncode:
            raise ProofError(f"canonical proof {label!r} failed with exit {result.returncode}")
        completed.append(label)
    return completed


def _render_human(receipt: dict[str, object]) -> None:
    frame = receipt["frame_size"]
    frame_text = "none" if frame is None else f"0x{frame:X}"
    print(
        f"promotion proof: PASS {receipt['candidate_symbol']} "
        f"({receipt['exact_words']} words, frame={frame_text}, "
        f"relocations={receipt['exact_relocations']}/"
        f"{receipt['exact_relocations']}, identity={receipt['identity_proof_mode']})"
    )
    if receipt["canonical_commands"]:
        print("canonical proof: PASS " + ", ".join(receipt["canonical_commands"]))


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol")
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="require function_preflight.py evidence to already be fresh",
    )
    parser.add_argument(
        "--canonical",
        action="store_true",
        help="also run bounded full-ROM and overlay-relocation Make proofs",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    try:
        report = run_preflight(args.symbol, no_build=args.no_build)
        receipt = validate_report(args.symbol, report)
        if args.canonical:
            receipt["canonical_commands"] = run_canonical_proofs(json_mode=args.json)
    except (OSError, ProofError) as error:
        parser.error(str(error))

    if args.json:
        print(json.dumps(receipt, indent=2, sort_keys=True))
    else:
        _render_human(receipt)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
