#!/usr/bin/env python3
"""Resolve the conflicts merge_lane.sh leaves behind when integrating a lane.

Usage: resolve_lane_conflicts.py <lane-branch> [--own-prefix PREFIX ...]
       resolve_lane_conflicts.py <lane-commit> --runlink-retirements-only

Rules (all hunk-level or three-way; NEVER whole-file --ours/--theirs on a
shared file):
  README.md                        -> OURS; only its generated scoreboard
                                      block differs and finish_merge rebuilds
                                      that, so the lane's copy can only revert
                                      the integration branch's prose
  config/runlink-residuals.us.json -> validated whole-record retirements from
                                      both parents; never one parent's copy
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
  Makefile                         -> as "everything else" below
  files only the lane touched
  (AA add/add or under --own-prefix) -> theirs
  docs/resident.md, docs/overlays.md -> theirs per hunk when ours is a subset
                                       of the lane's rows (else keep both)
  everything else (symbol_addrs,
  yaml, headers, shared .c, .mk)   -> git merge-file --diff3 against the
                                       lane's merge-base, then each residual
                                       hunk re-merged line by line: edits to
                                       disjoint base lines are both applied,
                                       insertions at one point are kept ours
                                       first; edits to the same base lines
                                       stay conflicted for a human
Exits nonzero if any conflict marker survives.
"""
import ast
import copy
import difflib
import json
import re
import subprocess
import sys
from pathlib import Path

MARK = re.compile(r"<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n", re.S)


def git(*a, **k):
    return subprocess.run(["git", *a], capture_output=True, text=True, **k)


RUNLINK_RESIDUALS = "config/runlink-residuals.us.json"
RETIREMENT_CATEGORIES = {"bss-size", "hybrid-data", "placeholder", "placement", "rodata-base"}


def merge_runlink_retirements(base_text, ours_text, theirs_text):
    """Combine only complete record deletions, not evidence or new allowances.

    This deliberately supports the current zero-difference manifest only.
    A fresh whole-dictionary bootstrap comparison is still required before
    accepting the merged source; this function proves no source identity.
    """
    import runlink_pack as pack

    def require(condition, message):
        if not condition:
            raise ValueError("runlink retirement merge: " + message)

    def unique(pairs):
        result = {}
        for key, value in pairs:
            require(key not in result, "duplicate JSON key")
            result[key] = value
        return result

    def integer(value, minimum=0):
        return type(value) is int and value >= minimum

    def module(value):
        return integer(value, 1) and value < 0xFFC

    def offset(value):
        return isinstance(value, str) and re.fullmatch(r"0x(?:0|[1-9A-F][0-9A-F]*)", value)

    def parse(text):
        document = json.loads(text, object_pairs_hook=unique)
        require(isinstance(document, dict) and set(document) ==
                {"schema", "generated_by", "about", "totals", "differences", "borrowed"},
                "unsupported manifest fields")
        require(type(document["schema"]) is int and document["schema"] == 1
                and document["generated_by"] == "tools/runlink_pack.py compare --write-residuals"
                and isinstance(document["about"], str), "unsupported manifest metadata")
        require(document["differences"] == {} and isinstance(document["differences"], dict),
                "nonempty or malformed differences")
        blocks, records = document["borrowed"], {}
        require(isinstance(blocks, dict) and set(blocks) <= RETIREMENT_CATEGORIES,
                "unsupported borrowing categories")
        for category, block in blocks.items():
            payload = "modules" if category == "hybrid-data" else "entries"
            require(isinstance(block, dict) and set(block) == {"about", "count", payload}
                    and isinstance(block["about"], str), "unsupported category fields")
            rows = []
            if payload == "modules":
                require(isinstance(block[payload], dict), "malformed hybrid modules")
                for number, sites in block[payload].items():
                    require(re.fullmatch(r"[1-9][0-9]*", number) and module(int(number))
                            and isinstance(sites, list) and sites, "malformed hybrid module")
                    require(all(offset(site) for site in sites), "malformed hybrid site")
                    rows.extend((int(number), site) for site in sites)
            else:
                require(isinstance(block[payload], list), "malformed borrowing entries")
                for row in block[payload]:
                    require(isinstance(row, list) and row and module(row[0]), "malformed borrowing record")
                    if category in ("bss-size", "rodata-base"):
                        valid = len(row) == 1
                    elif category == "placeholder":
                        valid = (len(row) == 3 and isinstance(row[1], str) and bool(row[1])
                                 and integer(row[2], 1))
                    else:
                        valid = (len(row) == 5 and isinstance(row[1], str) and bool(row[1])
                                 and row[2] in (".text", ".data", ".rodata", ".bss")
                                 and offset(row[3]) and integer(row[4], 1))
                    require(valid, "unsupported borrowing record shape")
                    rows.append(tuple(row))
            require(len(set(rows)) == len(rows), "duplicate borrowing record")
            require(integer(block["count"]) and block["count"] == len(rows), "malformed category count")
            records[category] = set(rows)
        total = sum(row[2] for row in records.get("placeholder", ())) + len(records.get("hybrid-data", ()))
        totals = document["totals"]
        require(isinstance(totals, dict) and set(totals) == {"differences", "borrowed_records"}
                and integer(totals["differences"]) and totals["differences"] == 0
                and integer(totals["borrowed_records"]) and totals["borrowed_records"] == total,
                "malformed total counts")
        return document, records

    base, before = parse(base_text)
    ours, left = parse(ours_text)
    theirs, right = parse(theirs_text)
    for side, records in ((ours, left), (theirs, right)):
        require(all(side[key] == base[key] for key in ("schema", "generated_by", "about", "differences")),
                "manifest metadata changed")
        require(set(records) <= set(before), "borrowing category added")
        for category, rows in records.items():
            require(side["borrowed"][category]["about"] == base["borrowed"][category]["about"],
                    "category metadata changed")
            require(rows <= before[category], "borrowing record added or changed")
    merged = copy.deepcopy(base)
    merged["borrowed"] = {}
    for category in sorted(before):
        rows = sorted(left.get(category, set()) & right.get(category, set()), key=pack.entry_sort_key)
        if not rows:
            continue
        block = {"about": base["borrowed"][category]["about"], "count": len(rows)}
        if category == "hybrid-data":
            block["modules"] = {}
            for number, site in rows:
                block["modules"].setdefault(str(number), []).append(site)
        else:
            block["entries"] = [list(row) for row in rows]
        merged["borrowed"][category] = block
    merged["totals"]["borrowed_records"] = (
        sum(row[2] for row in merged["borrowed"].get("placeholder", {}).get("entries", []))
        + merged["borrowed"].get("hybrid-data", {}).get("count", 0))
    return pack.render_manifest(merged)


def reconcile_runlink_retirements(branch):
    """Reconcile committed parents during a merge, including a clean auto-merge."""
    def checked(*args):
        result = git(*args)
        if result.returncode:
            raise ValueError("runlink retirement merge: git " + " ".join(args) + " failed")
        return result.stdout.strip()

    ours = checked("rev-parse", "HEAD^{commit}")
    theirs = checked("rev-parse", branch + "^{commit}")
    bases = checked("merge-base", "--all", ours, theirs).splitlines()
    if len(bases) != 1:
        raise ValueError("runlink retirement merge: ambiguous merge base")
    contents = [git("show", f"{revision}:{RUNLINK_RESIDUALS}") for revision in (bases[0], ours, theirs)]
    if all(result.returncode for result in contents):
        return  # Repositories predating this manifest have nothing to retire.
    if any(result.returncode for result in contents):
        raise ValueError("runlink retirement merge: manifest added or removed")
    merged = merge_runlink_retirements(*(result.stdout for result in contents))
    if checked("rev-parse", "HEAD^{commit}") != ours or checked("rev-parse", "MERGE_HEAD") != theirs:
        raise ValueError("runlink retirement merge: active merge parents changed")
    path = Path(RUNLINK_RESIDUALS)
    if path.is_symlink():
        raise ValueError("runlink retirement merge: symlink output")
    path.write_text(merged)
    checked("add", "--", RUNLINK_RESIDUALS)


DIFF3 = re.compile(
    r"<<<<<<< [^\n]*\n(.*?)\|\|\|\|\|\|\| [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n",
    re.S)


def _edits(base, side):
    matcher = difflib.SequenceMatcher(None, base, side, autojunk=False)
    return [(i1, i2, side[j1:j2])
            for op, i1, i2, j1, j2 in matcher.get_opcodes() if op != "equal"]


def merge_hunk(base, ours, theirs):
    """Three-way merge of one conflict hunk's lines, or None.

    git reports a conflict whenever two sides' edits are adjacent, which in
    an overlay rule file is the normal case: two lanes each delete the
    per-function rules of their own overlays, and those blocks touch.
    Concatenating the sides ("keep both") is wrong there, because each side
    still carries the base lines the other deleted; one integration
    resurrected 128 rules for deleted sources that way. Here both sides'
    edits are applied to the base when they touch disjoint base lines.
    Two pure insertions at one point keep ours first, as keep-both did.
    Anything else is a real conflict and returns None.
    """
    eo, et = _edits(base, ours), _edits(base, theirs)
    for a in eo:
        for b in et:
            if a == b:
                continue
            if a[0] == a[1] == b[0] == b[1]:
                continue
            if a[0] < b[1] and b[0] < a[1]:
                return None
            for ins, rng in ((a, b), (b, a)):
                if ins[0] == ins[1] and rng[0] < ins[0] < rng[1]:
                    return None
            if (a[0] == a[1] and a[0] in (b[0], b[1]) and b[0] != b[1]) or (
                    b[0] == b[1] and b[0] in (a[0], a[1]) and a[0] != a[1]):
                return None
    ordered, seen = [], set()
    for rank, side in enumerate((eo, et)):
        for i1, i2, rep in side:
            key = (i1, i2, tuple(rep))
            if key not in seen:
                seen.add(key)
                ordered.append((i1, i2, rank, rep))
    ordered.sort(key=lambda e: (e[0], e[1], e[2]))
    out, pos = [], 0
    for i1, i2, _, rep in ordered:
        out += base[pos:i1] + rep
        pos = max(pos, i2)
    return out + base[pos:]


def remerge_hunks(text):
    """Resolve every diff3 hunk merge_hunk can; leave the rest marked."""
    def fix(m):
        merged = merge_hunk(*(g.splitlines(keepends=True) for g in
                              (m.group(2), m.group(1), m.group(3))))
        return m.group(0) if merged is None else "".join(merged)
    return DIFF3.sub(fix, text)


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
    if args[1:] == ["--runlink-retirements-only"]:
        try:
            reconcile_runlink_retirements(branch)
        except (OSError, ValueError) as error:
            print(error, file=sys.stderr)
            return 1
        return 0
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
        if path == RUNLINK_RESIDUALS:
            try:
                reconcile_runlink_retirements(branch)
            except (OSError, ValueError) as error:
                print(error, file=sys.stderr)
                return 1
            print(f"resolved {path}: whole-record retirements from both parents")
            continue
        elif path == "README.md":
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
        elif path.endswith(".json") or path in (
                "overlay_undefined_syms.us.txt", "mk/overlay_aliases.generated.mk",
                "mk/overlay_resident_rebinds.generated.mk"):
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
            subprocess.run(["git", "merge-file", "--diff3", po, pb, pt],
                           capture_output=True)
            merged = open(po).read()
            open(path, "w").write(remerge_hunks(merged))
            how = "three-way merge-file, residue re-merged by line"
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
