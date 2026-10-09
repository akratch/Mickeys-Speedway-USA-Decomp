#!/usr/bin/env python3
"""Compatibility entry point: prints objdiff.json for objdiff-cli.

The derivation lives in tools/objdiff_config.py (yaml + overlay atlas + split
tree, not a walk of build/). This wrapper keeps the old command line working
for tools/objdiff_report.sh and tools/nm_ranking.py: it prints to stdout,
honours --base-dir, and applies the local tools/objdiff_exclude.txt list.
"""
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import objdiff_config  # noqa: E402

if __name__ == "__main__":
    raise SystemExit(objdiff_config.main(["--stdout", "--apply-excludes", *sys.argv[1:]]))
