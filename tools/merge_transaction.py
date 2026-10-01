#!/usr/bin/env python3
"""Stage only the validated generated delta of an in-progress merge.

The merge index is the reviewed input. Generators may update known derived
files and source paths already changed by this merge; all other dirt fails
closed. Failure leaves every worktree edit and the merge intact for review.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import tempfile
from pathlib import Path


GENERATED = {
    "README.md", "config/overlays.us.json", "config/overlay-donors.us.json",
    "config/postprocess-audit.us.json", "docs/modules.md", "docs/overlays.md",
    "config/nonmatching-ranking.us.json", "docs/nm-ranking.md",
    "mickey.us.yaml", "symbol_addrs.us.txt", "overlay_undefined_syms.us.txt",
}


def git(*args: str) -> str:
    return subprocess.check_output(["git", *args], text=True).strip()


def paths(*args: str) -> set[str]:
    data = subprocess.check_output(["git", args[0], "-z", *args[1:]])
    return {os.fsdecode(p) for p in data.split(b"\0") if p}


def merge_identity() -> dict[str, str]:
    return {"head": git("rev-parse", "HEAD"),
            "merge": git("rev-parse", "--verify", "MERGE_HEAD")}


def require_clean() -> None:
    if git("status", "--porcelain", "--untracked-files=no"):
        raise ValueError("tracked changes present; preserve and commit them before merging")
    if untracked_allowed(GENERATED):
        raise ValueError("untracked generated paths present; preserve and review them before merging")


def untracked_allowed(allowed: set[str]) -> set[str]:
    # Do not exclude ignored files: a formerly tracked generated file may
    # become ignored after the incoming commit deletes it. Query only the
    # explicit reviewed paths, never the potentially huge build directories.
    return paths("ls-files", "--others", "--", *sorted(allowed))


def begin(state: Path) -> None:
    identity = merge_identity()
    if paths("diff", "--name-only", "--diff-filter=U"):
        raise ValueError("resolve and stage all conflicts before running integration gates")
    if paths("diff", "--name-only"):
        raise ValueError("unstaged tracked edits present; review and stage them before integration gates")
    merged = paths("diff", "--cached", "--name-only", "HEAD")
    payload = {**identity, "index": git("write-tree"),
               "allowed": sorted(GENERATED | {p for p in merged
                   if p.startswith(("src/", "include/")) and p.endswith((".c", ".h"))})}
    if untracked_allowed(set(payload["allowed"])):
        raise ValueError("preexisting untracked generated input; review before integration gates")
    state.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode="w", dir=state.parent, delete=False) as stream:
        json.dump(payload, stream)
        temporary = stream.name
    os.replace(temporary, state)


def stage(state: Path) -> None:
    payload = json.loads(state.read_text())
    if merge_identity() != {k: payload[k] for k in ("head", "merge")}:
        raise ValueError("merge changed while gates ran; repeat integration gates")
    if git("write-tree") != payload["index"]:
        raise ValueError("index changed while gates ran; review and repeat integration gates")
    dirty = paths("diff", "--name-only") | untracked_allowed(set(payload["allowed"]))
    unexpected = dirty - set(payload["allowed"])
    if unexpected:
        raise ValueError("unexpected generated edits (preserved): " + ", ".join(sorted(unexpected)))
    # Record exact content before staging; a racing writer must not silently
    # substitute unvalidated bytes into the commit.
    expected = {p: git("hash-object", "--", p) if Path(p).is_file() else None
                for p in dirty}
    if dirty:
        # -f is confined to reviewed generated paths, including outputs that
        # became ignored when a lane removed their tracked predecessor.
        subprocess.run(["git", "add", "-A", "-f", "--", *sorted(dirty)], check=True)
    for path, oid in expected.items():
        entry = git("ls-files", "--stage", "--", path)
        actual = entry.split()[1] if entry else None
        if actual != oid:
            raise ValueError(f"{path} changed during staging; review and repeat gates")
    if paths("diff", "--name-only"):
        raise ValueError("tracked edits remain after staging; merge left uncommitted")
    subprocess.run(["git", "diff", "--cached", "--check"], check=True)
    print(f"merge transaction: staged {len(dirty)} validated generated path(s)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("clean", "begin", "stage"))
    args = parser.parse_args()
    os.chdir(git("rev-parse", "--show-toplevel"))
    state = Path(git("rev-parse", "--git-path", "mickey-merge-transaction.json"))
    try:
        if args.action == "clean":
            require_clean()
        else:
            globals()[args.action](state)
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f"merge transaction: {exc}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
