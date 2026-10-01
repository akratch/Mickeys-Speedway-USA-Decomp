#!/usr/bin/env python3
"""One line per lane: what it has committed, what is dirty, what it matched.

    tools/lane_fleet.py [--prefix d-] [--base campaign/unchain]

A coordinator running a dozen lanes needs, several times an hour, the answer
to "which lanes have finished work I can merge, and which are mid-flight".
Reading `git log` and `git status` in a dozen worktrees by hand is slow and
was done wrongly once (a reclaimed worktree's dangling symlink read as
present). This prints, per lane branch: commits ahead of the base, dirty
files in its worktree (or `gone` when the worktree was reclaimed), the
matches among those commits (subjects starting with `Match `), and the last
subject. Lanes with commits ahead and a clean worktree are merge candidates.
"""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


def git(*args: str, cwd: Path | None = None) -> str:
    return subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True).stdout.strip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--prefix", default="", help="only lanes whose name starts with this")
    ap.add_argument("--base", default="campaign/unchain")
    ns = ap.parse_args()
    root = Path(git("rev-parse", "--show-toplevel"))
    worktrees: dict[str, Path] = {}
    cur = None
    for line in git("worktree", "list", "--porcelain").splitlines():
        if line.startswith("worktree "):
            cur = Path(line.split(" ", 1)[1])
        elif line.startswith("branch ") and cur is not None:
            worktrees[line.split(" ", 1)[1].removeprefix("refs/heads/")] = cur
    branches = [b for b in git("for-each-ref", "--format=%(refname:short)", "refs/heads/lane/").splitlines()
                if b.removeprefix("lane/").startswith(ns.prefix)]
    print(f"{'lane':16s} {'ahead':>5s} {'dirty':>5s} {'matches':>7s}  last subject")
    for b in sorted(branches):
        name = b.removeprefix("lane/")
        ahead = int(git("rev-list", "--count", f"{ns.base}..{b}") or 0)
        subjects = git("log", "--format=%s", f"{ns.base}..{b}").splitlines() if ahead else []
        matches = sum(1 for s in subjects if s.startswith("Match "))
        wt = worktrees.get(b)
        if wt and wt.is_dir():
            dirty = str(len(git("status", "--short", cwd=wt).splitlines()))
        else:
            dirty = "gone"
        last = subjects[0] if subjects else "-"
        print(f"{name:16s} {ahead:>5d} {dirty:>5s} {matches:>7d}  {last[:70]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
