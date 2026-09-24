#!/usr/bin/env python3
"""Resolve the conflicts merge_lane.sh leaves behind when integrating a lane.

Usage: resolve_lane_conflicts.py <lane-branch> [--own-prefix PREFIX ...]

Rules (all hunk-level or three-way; NEVER whole-file --ours/--theirs on a
shared file):
  README.md                        -> OURS; only its generated scoreboard
                                      block differs and finish_merge rebuilds
                                      that, so the lane's copy can only revert
                                      the integration branch's prose
  *.json and generated tables      -> theirs; a keep-both hunk cannot
                                      produce valid JSON, and the result is
                                      parse-checked before it is staged
  config/nonmatching-ranking.us.json,
  docs/nm-ranking.md, and
  docs/matching-triage-handoffs/   -> OURS (or dropped, if the lane added
                                      the file). These are generated. The
                                      merge regenerates them from the merged
                                      tree; an older lane copy must not win.
  *.py                             -> resolved as below, then parse-checked:
                                      a keep-both residue can splice a
                                      function tail onto nothing
  Makefile                         -> keep both sides of every hunk
  files only the lane touched
  (AA add/add or under --own-prefix) -> theirs
  docs/resident.md, docs/overlays.md -> theirs per hunk when ours is a subset
                                       of the lane's rows (else keep both)
  everything else (symbol_addrs,
  yaml, headers, shared .c)        -> git merge-file against the lane's
                                       merge-base, then keep both on residual
                                       conflicts
Exits nonzero if any conflict marker survives.
"""
import ast
import json
import re
import subprocess
import sys

MARK = re.compile(r"<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n", re.S)


def git(*a, **k):
    return subprocess.run(["git", *a], capture_output=True, text=True, **k)


def keep_both(text):
    return MARK.sub(lambda m: m.group(1) + m.group(2), text)


GENERATED_RANKING = "config/nonmatching-ranking.us.json"
GENERATED_RANKING_DOC = "docs/nm-ranking.md"
GENERATED_SHARD_PREFIX = "docs/matching-triage-handoffs/"


def regenerates_in_merged_tree(path):
    """True for generated ranking and handoff shards.

    A lane's copy of these is a measurement of the lane's own tree. The
    merge regenerates them after the sources are combined, and that
    result is what the commit keeps.
    """
    return (
        path in (GENERATED_RANKING, GENERATED_RANKING_DOC)
        or path.startswith(GENERATED_SHARD_PREFIX)
    )


def choose_regenerated(lane_text, regenerated_text):
    """Keep the merged tree's regeneration when it differs from the lane.

    ``lane_text`` is the generated ranking or shard the lane brought.
    ``regenerated_text`` is what the merged tree just produced. The lane
    copy is not a candidate once those two differ, and it is not a
    candidate when they agree either: the regeneration is the result.
    """
    if regenerated_text != lane_text:
        return regenerated_text
    return regenerated_text


def theirs_if_subset(text):
    def fix(m):
        ours, theirs = m.group(1), m.group(2)
        ours_rows = [l for l in ours.splitlines() if l.strip()]
        if all(l in theirs for l in ours_rows):
            return theirs
        return ours + theirs
    return MARK.sub(fix, text)


def duplicate_redefines(path):
    """Symbols renamed more than once inside a single target's recipe.

    objcopy takes --redefine-sym once per symbol; a repeat is a hard error.
    Scoped per recipe because two *different* targets legitimately rename the
    same symbol to their own per-overlay alias.
    """
    duplicated = set()
    seen = set()
    in_recipe = False
    for line in open(path, errors="replace"):
        if not line.startswith("\t"):
            in_recipe = line.rstrip().endswith(":") or ": " in line
            seen = set()
            continue
        if not in_recipe:
            continue
        for name in re.findall(r"--redefine-sym\s+(\S+?)=", line):
            if name in seen:
                duplicated.add(name)
            seen.add(name)
    return duplicated


def validate_resolved(path):
    """Return a message if a just-resolved file is structurally broken.

    Both cases below happened. A keep-both hunk spliced two JSON records
    together without a delimiter and produced a file no reader could parse;
    a keep-both residue in a Python tool kept one side's function tail
    without its head. Neither leaves a conflict marker, so the tree looks
    clean and the failure lands several gates downstream, reading like a
    defect in the file rather than in the resolution. The resolver is the
    only place that knows a resolution just happened.
    """
    if not path.endswith((".json", ".py")):
        return None
    with open(path, errors="replace") as handle:
        text = handle.read()
    if path.endswith(".json"):
        try:
            json.loads(text)
        except ValueError as error:
            return f"INVALID JSON after resolving {path}: {error}"
    else:
        try:
            ast.parse(text, filename=path)
        except SyntaxError as error:
            return (f"INVALID PYTHON after resolving {path}: "
                    f"line {error.lineno}: {error.msg}")
    return None


def main():
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 2
    branch = args[0]
    own = []
    i = 1
    while i < len(args):
        if args[i] == "--own-prefix":
            own.append(args[i + 1])
            i += 2
        else:
            i += 1
    base = git("merge-base", "HEAD", branch).stdout.strip()
    status = git("status", "--porcelain").stdout.splitlines()
    conflicted = [(l[:2], l[3:]) for l in status if l[:2] in ("UU", "AA", "AU", "UA", "DU", "UD")]
    for code, path in conflicted:
        if path == "README.md":
            # OURS, not theirs. The only part of this file a lane legitimately
            # changes is the generated scoreboard block, and finish_merge
            # regenerates that from the merged tree anyway -- so the lane's
            # copy carries nothing worth keeping and everything worth losing.
            # Taking theirs silently reverted the integration branch's prose:
            # an AI-assistance section and a rewritten roadmap were undone by
            # three consecutive lane merges, with no conflict and nothing in
            # any gate to notice, because the scoreboard block regenerated
            # correctly each time and that is all check-scoreboard reads.
            git("checkout", "--ours", "--", path)
            how = "ours (lane README is stale; scoreboard regenerates)"
        elif regenerates_in_merged_tree(path):
            # Not --theirs. The lane's generated ranking or shard is a
            # measurement of the lane, and an older lane has won this
            # merge before. Keep the integration copy when there is one,
            # and drop a lane-only file. The merge then regenerates both
            # from the combined tree.
            if git("checkout", "--ours", "--", path).returncode != 0:
                git("rm", "-q", "-f", "--", path)
                how = "dropped lane-only generated file pending regeneration"
            else:
                how = "ours; merged-tree regeneration replaces the lane copy"
        elif path == "Makefile":
            text = open(path).read()
            open(path, "w").write(keep_both(text))
            how = "keep both"
        elif path.endswith(".json") or path in (
                "overlay_undefined_syms.us.txt", "mk/overlay_aliases.generated.mk"):
            # Generated or JSON artifacts: never merge by hunks. A keep-both
            # hunk once duplicated the atlas sha256 key and broke the pin
            # file's JSON, which is why the named files were listed here; the
            # list then went stale and config/nonmatching-ranking.us.json fell
            # through to keep-both, which spliced two records together without
            # a delimiter and produced a file no reader could parse.
            #
            # Match by suffix rather than by name so the next generated JSON
            # is covered the day it is added. Take the lane's copy;
            # finish_merge regenerates the generated ones and JSON
            # dictionaries are re-unioned by hand.
            git("checkout", "--theirs", "--", path)
            how = "theirs (generated/JSON)"
        elif code == "AA" or any(path.startswith(p) for p in own):
            git("checkout", "--theirs", "--", path)
            how = "theirs (lane-owned)"
        elif path in ("docs/resident.md", "docs/overlays.md"):
            text = open(path).read()
            open(path, "w").write(theirs_if_subset(text))
            how = "theirs per hunk where ours is a subset"
        else:
            ours = git("show", f"HEAD:{path}").stdout
            theirs = git("show", f"{branch}:{path}").stdout
            basetxt = git("show", f"{base}:{path}").stdout
            import tempfile, os
            d = tempfile.mkdtemp()
            po, pb, pt = [os.path.join(d, n) for n in ("ours", "base", "theirs")]
            open(po, "w").write(ours); open(pb, "w").write(basetxt); open(pt, "w").write(theirs)
            subprocess.run(["git", "merge-file", po, pb, pt], capture_output=True)
            merged = open(po).read()
            open(path, "w").write(keep_both(merged))
            how = "three-way merge-file, keep both on residue"
        if "<<<<<<<" in open(path, errors="replace").read():
            print(f"UNRESOLVED {path}")
            return 1
        # A POSTPROCESS rule is a LIST, and keep-both is as wrong for it as
        # it is for JSON. Two lanes independently restoring overlay 60's
        # missing resident renames merged into 110 --redefine-sym entries for
        # 59 unique symbols; objcopy refuses a duplicate ("Multiple
        # redefinition of symbol"), so the object could not build, and
        # reloc_surface then read a stale one and silently dropped nine
        # aliases until the ROM stopped verifying. Fail here, where the cause
        # is one line away, rather than four steps downstream.
        if path.endswith((".mk", "Makefile")):
            duplicated = duplicate_redefines(path)
            if duplicated:
                shown = ", ".join(sorted(duplicated)[:5])
                more = f" (and {len(duplicated) - 5} more)" if len(duplicated) > 5 else ""
                print(
                    f"DUPLICATE --redefine-sym after resolving {path}: {shown}{more}."
                    f"\n  Two sides added the same rename. Keep one side's rule "
                    f"whole rather than merging them."
                )
                return 1

        # Fail here, where the cause is one resolution away, rather than
        # several gates downstream where it reads like a corrupt input.
        broken = validate_resolved(path)
        if broken:
            print(broken)
            return 1
        git("add", "--", path)
        print(f"resolved {path}: {how}")
    left = git("grep", "-l", "^<<<<<<< ", "--", ".").stdout.strip()
    if left:
        print("markers remain:", left)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
