#!/usr/bin/env python3
"""Bounded, compact preflight/diagnosis/optional flag-lattice report.

Only scalar evidence is printed. Full tool reports and compiler logs remain
in a unique ignored build directory. Never edits source or runs a promotion.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parent.parent
PYTHON = str(ROOT / ".venv/bin/python")
COMPARISON_FIELDS = (
    "exact", "words", "raw_word_mismatches", "target_instructions",
    "candidate_instructions", "instruction_delta", "true_instruction_delta",
    "normalized_distance", "opcode_distance", "opcode_mismatches",
    "geometry_front", "geometry_edit_distance",
    "target_frame_size", "candidate_frame_size", "first_divergent_row",
    "relocation_metadata_mismatches", "relocation_target_mismatches",
)


def run_phase(command, directory, label, deadline):
    output = directory / f"{label}.json"
    errors = directory / f"{label}.log"
    receipt = {"status": "failed", "output": str(output.relative_to(ROOT)),
               "log": str(errors.relative_to(ROOT))}
    remaining = deadline - time.monotonic()
    if remaining <= 0:
        return {**receipt, "status": "budget-exhausted"}, None
    started = time.monotonic()
    with output.open("wb") as stdout, errors.open("wb") as stderr:
        try:
            process = subprocess.Popen(command, cwd=ROOT, stdout=stdout, stderr=stderr,
                                       start_new_session=True)
        except OSError:
            return {**receipt, "status": "launch-failed"}, None
        try:
            code = process.wait(timeout=remaining)
        except (subprocess.TimeoutExpired, KeyboardInterrupt) as error:
            # These read/compile-only phases inherit our dedicated process
            # group. Do not leave their compilers running after a timeout.
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            process.wait()
            if isinstance(error, KeyboardInterrupt):
                raise
            return {**receipt, "status": "timeout"}, None
    receipt.update(returncode=code, seconds=round(time.monotonic() - started, 3))
    if code != 0:
        return receipt, None
    try:
        if output.stat().st_size > 16 * 1024 * 1024:
            raise ValueError("report too large")
        payload = json.loads(output.read_text())
        if not isinstance(payload, dict):
            raise ValueError("report is not an object")
    except (ValueError, OSError):
        return {**receipt, "status": "invalid-report"}, None
    return {**receipt, "status": "ok"}, payload


def compact(preflight, diagnosis):
    if preflight is not None and preflight.get("schema") != "mickey-function-evidence-preflight-v1":
        raise ValueError("unsupported preflight schema")
    if diagnosis is not None and diagnosis.get("schema") != "decomp-workbench-diagnosis-v3":
        raise ValueError("unsupported diagnosis schema")
    comparison = (diagnosis or {}).get("comparison", {})
    evidence = (preflight or {}).get("preflight", {})
    counts = evidence.get("counts", {})
    lever = (diagnosis or {}).get("lever") or {}
    # Allowlist scalars, never propagate aligned rows, disassembly or traces.
    scores = {key: comparison[key] for key in COMPARISON_FIELDS if key in comparison
              and (comparison[key] is None or isinstance(comparison[key], (bool, int)))}
    geometry = comparison.get("geometry") or {}
    complete = evidence.get("status") == "complete"
    target = scores.get("target_instructions")
    candidate = scores.get("candidate_instructions")
    count_fields = (
        "target_relocations", "candidate_static_relocations",
        "candidate_identities_resolved", "candidate_identities_unresolved",
        "offset_type_aligned", "stable_identities_aligned", "effective_identities_aligned",
    )
    counts_present = all(type(counts.get(key)) is int and counts[key] >= 0 for key in count_fields)
    expected = counts.get("target_relocations")
    identities_exact = (counts_present and counts["candidate_identities_unresolved"] == 0
                        and all(counts[key] == expected for key in count_fields
                                if key != "candidate_identities_unresolved"))
    relocation_comparison_exact = all(
        type(scores.get(key)) is int and scores[key] == 0
        for key in ("relocation_metadata_mismatches", "relocation_target_mismatches")
    )
    # An exact word comparison only claims a real match when the object it
    # scored was built stock; a forced/traced/undeclared build still reports
    # exact=True but is not admissible evidence of a match (see
    # decomp-workbench's build_provenance block). Older diagnosis reports
    # carry no such block at all -- treat that as unconstrained, not as a
    # denial, so behaviour on pre-provenance summaries is unchanged.
    build_provenance = (diagnosis or {}).get("build_provenance")
    build_claim = (
        build_provenance.get("claim") if isinstance(build_provenance, dict) else None
    )
    exact_verified = scores.get("exact") is True and (
        build_claim is None or build_claim == "match"
    )
    unverified_exact = scores.get("exact") is True and not exact_verified
    eligible = (complete and (preflight or {}).get("resolution_mode") == "fallback"
                and exact_verified and type(target) is int
                and target > 0 and type(candidate) is int and candidate == target
                and type(scores.get("words")) is int and scores["words"] == 0
                and identities_exact and relocation_comparison_exact)
    next_action = evidence.get("action", "inspect_phase_logs_and_restore_preflight_evidence")
    if eligible:
        next_action = "run_separate_linked_promotion_proof"
    elif unverified_exact:
        next_action = f"reject_exact_result_build_provenance_claim_{build_claim}_not_match"
    return {
        "preflight_status": evidence.get("status", "unavailable"),
        "owned_bytes": (preflight or {}).get("owned_size"),
        "comparison": scores,
        "geometry": {key: geometry[key] for key in
                     ("absolute_extent_delta", "edit_distance", "opcode_distance")
                     if type(geometry.get(key)) is int},
        "relocations": {key: value for key, value in counts.items() if type(value) is int},
        "mechanism": {"routing": (diagnosis or {}).get("routing"),
                      "lever_class": lever.get("lever_class"),
                      "edit_family": lever.get("edit_family")},
        "build_provenance_claim": build_claim,
        "candidate_for_linked_trial": eligible,
        "promotion_proof_included": False,
        "next_action": next_action,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--flags", action="store_true", help="also run the diagnostic flag lattice")
    parser.add_argument("--seconds", type=float, default=300, help="total subprocess wall budget")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    args = parser.parse_args(argv)
    if not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", args.symbol) or not 0 < args.seconds <= 3600 or args.jobs < 1:
        parser.error("provide a C symbol, positive jobs, and a wall budget in (0, 3600]")
    if args.flags and args.no_build:
        parser.error("--flags compiles; it cannot be combined with --no-build")
    started = time.monotonic()
    parent = ROOT / "build/function-probe"
    parent.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix=args.symbol + "-", dir=parent))
    deadline = started + args.seconds
    pre_command = [PYTHON, "tools/function_preflight.py", args.symbol, "--analysis-only", "--json"]
    if args.no_build:
        pre_command.append("--no-build")
    phases = {}
    phases["preflight"], preflight = run_phase(pre_command, directory, "preflight", deadline)
    # The preflight can fail on oversized TU ownership but still leave a
    # fresh, useful diagnostic object. --no-build must authenticate it first.
    command = ["tools/wb_compare.sh", "--diagnose", "--no-build", args.symbol, "--json", "--color", "never"]
    if preflight and preflight.get("resolution_mode") == "post_promotion":
        command.insert(1, "--rom")
    phases["diagnosis"], diagnosis = run_phase(command, directory, "diagnosis", deadline)
    try:
        report = compact(preflight, diagnosis)
    except (ValueError, TypeError, AttributeError):
        parser.exit(2, f"function-probe: invalid evidence schema; inspect {directory.relative_to(ROOT)}\n")
    flags = None
    if args.flags:
        if preflight is None:
            phases["flags"] = {"status": "skipped-missing-owned-context"}
        else:
            command = [PYTHON, "tools/flag_sweep.py", "src/" + preflight["translation_unit"] + ".c",
                       "--function", preflight["candidate_symbol"], "--target-symbol", preflight["target_symbol"],
                       "--jobs", str(args.jobs), "--json"]
            phases["flags"], flags = run_phase(command, directory, "flags", deadline)
            if flags and flags.get("schema") != "mickey-flag-sweep-summary-v1":
                phases["flags"]["status"] = "invalid-report"
                flags = None
    if flags:
        report["flags"] = {key: flags[key] for key in
                           ("complete", "total_combinations", "scored_combinations", "failed_combinations", "cache_key")}
        report["flags"]["best"] = flags["ranked"][:1]
    report.update(schema="mickey-function-probe-v1", symbol=args.symbol, phases=phases,
                  artifacts=str(directory.relative_to(ROOT)), seconds=round(time.monotonic() - started, 3))
    if preflight:
        report["source"] = preflight["source"]
        report["identity"] = {key: preflight[key] for key in ("target_symbol", "candidate_symbol", "linked_section")}
    (directory / "summary.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(f"{args.symbol}: preflight={report['preflight_status']}; linked trial candidate={report['candidate_for_linked_trial']}")
        print("comparison: " + json.dumps(report["comparison"], sort_keys=True))
        print("relocations: " + json.dumps(report["relocations"], sort_keys=True))
        print("mechanism: " + json.dumps(report["mechanism"], sort_keys=True))
        print("next: " + report["next_action"])
        print(f"artifacts: {report['artifacts']}/summary.json (no promotion performed)")
    return int(report["preflight_status"] != "complete" or any(p["status"] != "ok" for p in phases.values())
               or (args.flags and not (flags or {}).get("complete")))


if __name__ == "__main__":
    raise SystemExit(main())
