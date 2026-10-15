#!/usr/bin/env python3
"""Run a candidate and a reference source on synthetic inputs and compare what they do.

    tools/behaviour_check.py <candidate.c> --driver DRIVER.c [--reference SRC.c] [--cc cc]
    tools/behaviour_check.py <candidate.c> --driver DRIVER.c --digest   # print the candidate's digest only

A score says how close a candidate is to the target's bytes; it says nothing
about whether the candidate still means what the source means. Permuter and
rename-scan cells change C freely, and a cell that writes into a live local
(the row counter of overlay64GenerateTexture, twice on 2026-10-15) can score
better than the honest source while computing something else. This tool is
the semantic gate for such cells.

Both translation units are compiled for the HOST with the system C compiler,
`-DNON_MATCHING` (so the candidate C is what runs, never a GLOBAL_ASM pragma)
and `-I include -I .`, each linked with the same DRIVER. The driver owns
`main`: it defines every external the TU needs (globals, callees with
deterministic stand-ins), builds synthetic inputs, calls the function, and
prints a digest of everything the function may write. The candidate passes
when its output equals the reference's (by default the tracked file at the
same path as the candidate's TU, given with --reference).

Limits, stated so nobody reads more into a pass than it carries: the host is
64-bit and little-endian, so a driver must not depend on pointer size or
byte order in what it hashes; behaviour is compared only on the inputs the
driver builds; undefined behaviour the reference shares is not caught. A
driver is synthetic input only. Nothing ROM-derived goes in one.

Exit status: 0 same behaviour, 1 different, 2 a compile or run failure.
Drivers live in tools/behaviour_drivers/.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run_tu(source: Path, driver: Path, cc: str = "cc", timeout: int = 60) -> tuple[bool, str]:
    """Compile SOURCE with DRIVER for the host, run it, return (ok, stdout or error)."""
    with tempfile.TemporaryDirectory(prefix="behaviour_") as tmp:
        exe = Path(tmp) / "run"
        cmd = [cc, "-w", "-O0", "-DNON_MATCHING", "-I", str(ROOT / "include"), "-I", str(ROOT),
               "-o", str(exe), str(driver), str(source)]
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode:
            return False, "compile failed: " + (r.stderr.strip().splitlines() or ["?"])[0]
        try:
            r = subprocess.run([str(exe)], capture_output=True, text=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            return False, "run timed out"
        if r.returncode:
            return False, f"run failed with status {r.returncode}"
        return True, r.stdout


def same_behaviour(candidate: Path, reference: Path, driver: Path, cc: str = "cc") -> tuple[int, str]:
    ok_r, out_r = run_tu(reference, driver, cc)
    if not ok_r:
        return 2, f"reference: {out_r}"
    ok_c, out_c = run_tu(candidate, driver, cc)
    if not ok_c:
        return 2, f"candidate: {out_c}"
    return (0, "same behaviour") if out_c == out_r else (1, "different behaviour")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("candidate", type=Path)
    ap.add_argument("--driver", type=Path, required=True)
    ap.add_argument("--reference", type=Path, help="reference TU (default: required unless --digest)")
    ap.add_argument("--digest", action="store_true", help="print the candidate's output and exit")
    ap.add_argument("--cc", default=os.environ.get("CC", "cc"))
    a = ap.parse_args(argv)
    if a.digest:
        ok, out = run_tu(a.candidate, a.driver, a.cc)
        print(out.strip() if ok else out)
        return 0 if ok else 2
    if a.reference is None:
        ap.error("--reference is required unless --digest")
    status, msg = same_behaviour(a.candidate, a.reference, a.driver, a.cc)
    print(msg)
    return status


if __name__ == "__main__":
    sys.exit(main())
