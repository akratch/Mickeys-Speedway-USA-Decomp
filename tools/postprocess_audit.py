#!/usr/bin/env python3
"""Audit every object's POSTPROCESS step against docs/acceleration-survey.md
sec.13.2's ruling: no gold-standard N64 decomp edits an instruction word
after compilation to reach a match.

This tool does not parse Makefile text by hand -- line continuations, the
`$(FOO_OBJ)` indirection a handful of overlays use as a target list, and
`ifeq`/`include` conditionals make that brittle and easy to silently
mis-scan.  Instead it asks GNU Make itself to expand every rule
(`gmake -p -q`, the "print database" dump) and reads the target-specific
`POSTPROCESS = ...` assignments straight out of that: what Make would
actually run, expanded.

For each such object it:

  * classifies the command as ``altered`` (touches instruction words),
    ``metadata`` (reviewed symbol/section/relocation metadata only), or
    ``review-required``. Unknown helpers fail closed instead of silently
    receiving metadata status. Anchored section externalization is altered
    because it rewrites relocated instruction immediates;
  * records which tool(s) the command invokes;
  * joins the object to its ownership range in config/overlays.us.json's
    per-overlay ``text_ownership`` list (offset, size, C ownership and
    NON_MATCHING state),
    keyed by the object's path under src/ with the .c/.o stripped -- the
    same string overlay_atlas.py stores as ``source``.

Usage:
    tools/postprocess_audit.py               # table to stdout
    tools/postprocess_audit.py --write        # also refresh the committed JSON
    tools/postprocess_audit.py --check        # fail if the JSON is stale
"""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
DEFAULT_OUT = os.path.join(ROOT_DIR, "config", "postprocess-audit.us.json")
ATLAS_PATH = os.path.join(ROOT_DIR, "config", "overlays.us.json")

ALTERED_TOOLS = {
    "normalize_elf_instructions.py",
    "resize_elf_function.py",
    "extend_elf_function_to_text.py",
    "patch_elf_words.py",
}
# normalize_o63_* is a family (normalize_o63_foo.py, ...), matched by prefix.
ALTERED_PREFIXES = ("normalize_o63_",)

METADATA_TOOLS = {
    "add_elf_relocations.py",
    "filter_elf_relocations.py",
    "overlay52CopyOffsetEntries.sort.py",
    "rebind_elf_relocations.py",
    "set_elf_flags.py",
    "trim_elf_section.py",
}

EXTERNALIZE_RE = re.compile(
    r"externalize_elf_section\.py\s+\S+\s+\S+\s+\S+(?:\s+(\S+))?"
)
DANGEROUS_OBJCOPY_RE = re.compile(
    r"--(?:update|add)-section|--change-section|"
    r"--set-section-flags(?:=|\s+)\.text|--remove-section(?:=|\s+)\.text"
)

TOOL_RE = re.compile(r"([A-Za-z0-9_./-]+\.py)")
OBJCOPY_RE = re.compile(r"\bobjcopy\b", re.IGNORECASE)

TARGET_LINE_RE = re.compile(r"^(\S+): POSTPROCESS = (.*)$")


def postprocess_commands(dump):
    """Return the effective non-empty POSTPROCESS command for each object."""
    seen = {}
    for line in dump.splitlines():
        m = TARGET_LINE_RE.match(line)
        if not m:
            continue
        target, command = m.group(1), m.group(2)
        if not target.startswith("build/") or not target.endswith(".o"):
            continue
        if command.strip() == "@:":
            continue
        seen[target] = command  # dedupe: -p repeats a rule per prerequisite
    return seen


def _objcopy_redefine_groups(command):
    """Return rename pairs grouped by individual objcopy invocation."""
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|")
    lexer.whitespace_split = True
    lexer.commenters = ""
    segments = []
    segment = []
    for token in lexer:
        if token in {"&&", "||", ";", "&", "|"}:
            if segment:
                segments.append(segment)
                segment = []
        else:
            segment.append(token)
    if segment:
        segments.append(segment)

    groups = []
    for tokens in segments:
        if not any(OBJCOPY_RE.search(token) for token in tokens):
            continue
        pairs = []
        index = 0
        while index < len(tokens):
            token = tokens[index]
            spec = None
            if token == "--redefine-sym" and index + 1 < len(tokens):
                index += 1
                spec = tokens[index]
            elif token.startswith("--redefine-sym="):
                spec = token[len("--redefine-sym=") :]
            if spec and "=" in spec:
                source, destination = spec.split("=", 1)
                pairs.append((source, destination))
            index += 1
        groups.append(pairs)
    return groups


def objcopy_redefine_pairs(command):
    """Return ordered ``(source, destination)`` objcopy rename pairs."""
    return [
        pair for group in _objcopy_redefine_groups(command) for pair in group
    ]


def duplicate_redefine_targets(command):
    """Find destination symbols repeated in one objcopy invocation.

    GNU objcopy rejects two ``--redefine-sym`` options with the same target.
    More importantly for overlays, distinct relocation identities must not be
    collapsed merely because their encoded addends happen to agree.
    """
    conflicts = []
    for group in _objcopy_redefine_groups(command):
        destinations = {}
        for source, destination in group:
            destinations.setdefault(destination, []).append(source)
        for destination, sources in destinations.items():
            if len(sources) > 1:
                conflicts.append((destination, sources))
    return conflicts


def redefine_conflicts(commands):
    problems = []
    for target, command in sorted(commands.items()):
        for destination, sources in duplicate_redefine_targets(command):
            problems.append((target, destination, sources))
    return problems


def run_make_database():
    """Return gmake's expanded rule database as text.

    `-p` prints the database, `-q` ("question mode") stops it from actually
    building anything; the exit code from `-q` is meaningless here (it means
    "up to date or not") and is ignored.
    """
    proc = subprocess.run(
        ["gmake", "-p", "-q"],
        cwd=ROOT_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        timeout=120,
    )
    return proc.stdout


def classify(command):
    tools = []
    for m in TOOL_RE.finditer(command):
        name = os.path.basename(m.group(1))
        if name not in tools:
            tools.append(name)
    if OBJCOPY_RE.search(command) and "objcopy" not in tools:
        tools.append("objcopy")

    is_altered = any(t in ALTERED_TOOLS for t in tools) or any(
        t.startswith(p) for t in tools for p in ALTERED_PREFIXES
    )
    externalize = EXTERNALIZE_RE.search(command)
    if externalize and externalize.group(1) not in (None, "&&", ";", "keep"):
        try:
            is_altered = is_altered or int(externalize.group(1), 0) != 0
        except ValueError:
            return "review-required", tools
    if OBJCOPY_RE.search(command) and DANGEROUS_OBJCOPY_RE.search(command):
        is_altered = True
    if is_altered:
        return "altered", tools

    reviewed = METADATA_TOOLS | {"externalize_elf_section.py", "objcopy"}
    if any(tool not in reviewed for tool in tools):
        return "review-required", tools
    return "metadata", tools


def load_atlas_index(atlas_path):
    """{('overlays/oNNN/name'): ownership metadata} from the
    committed overlay atlas, plus the atlas's own totals for cross-checking.
    """
    with open(atlas_path, encoding="utf-8") as fh:
        atlas = json.load(fh)
    index = {}
    for module in atlas["modules"]:
        overlay = module["overlay"]
        for part in module["text_ownership"]:
            index[part["source"]] = {
                "overlay": overlay,
                "offset": int(part["offset"], 16),
                "size": int(part["size"], 16),
                "matched": part["matched"],
                "nonmatching": part.get("nonmatching", False),
            }
    return index, atlas["totals"]


def object_to_src(target):
    """'build/src/overlays/o005/foo.c.o' -> 'src/overlays/o005/foo.c'."""
    if not target.startswith("build/") or not target.endswith(".o"):
        return None
    return target[len("build/") :][: -len(".o")]


def audit(root_dir=ROOT_DIR, atlas_path=ATLAS_PATH):
    dump = run_make_database()
    seen = postprocess_commands(dump)

    atlas_index, atlas_totals = load_atlas_index(atlas_path)

    rows = []
    for target in sorted(seen):
        command = seen[target]
        cls, tools = classify(command)
        src = object_to_src(target)
        overlay = None
        object_key = None
        m = re.search(r"overlays/o(\d+)/", src or "")
        if m:
            overlay = int(m.group(1))
        if src is not None and src.startswith("src/") and src.endswith(".c"):
            object_key = src[len("src/") : -len(".c")]
        atlas_hit = atlas_index.get(object_key) if object_key else None
        rows.append(
            {
                "object": target,
                "source": src,
                "overlay": overlay,
                "class": cls,
                "tools": tools,
                "offset": atlas_hit["offset"] if atlas_hit else None,
                "size": atlas_hit["size"] if atlas_hit else None,
                "c_owned": atlas_hit["matched"] if atlas_hit else None,
                "nonmatching": atlas_hit["nonmatching"] if atlas_hit else None,
                "matched_c": (
                    atlas_hit["matched"] and not atlas_hit["nonmatching"]
                    if atlas_hit
                    else None
                ),
            }
        )
    return rows, atlas_totals


def summarize(rows):
    total = len(rows)
    by_class = {}
    altered_bytes = 0
    matched_c_bytes_with_postprocess = 0
    for r in rows:
        by_class[r["class"]] = by_class.get(r["class"], 0) + 1
        if r["matched_c"] and r["size"] is not None:
            matched_c_bytes_with_postprocess += r["size"]
            if r["class"] == "altered":
                altered_bytes += r["size"]
    return {
        "objects_with_postprocess": total,
        "by_class": by_class,
        "altered_bytes": altered_bytes,
        "matched_c_bytes_with_postprocess": matched_c_bytes_with_postprocess,
    }


def render_table(rows, summary, atlas_totals):
    lines = []
    lines.append(
        "%-70s %-9s %-6s %8s %8s  %s"
        % ("object", "class", "ovl", "offset", "size", "tools")
    )
    for r in sorted(rows, key=lambda r: (r["overlay"] or -1, r["object"])):
        lines.append(
            "%-70s %-9s %-6s %8s %8s  %s"
            % (
                r["object"],
                r["class"],
                r["overlay"] if r["overlay"] is not None else "-",
                hex(r["offset"]) if r["offset"] is not None else "-",
                hex(r["size"]) if r["size"] is not None else "-",
                ",".join(r["tools"]),
            )
        )
    lines.append("")
    lines.append(
        "objects with POSTPROCESS: %d (altered=%d metadata=%d review-required=%d)"
        % (
            summary["objects_with_postprocess"],
            summary["by_class"].get("altered", 0),
            summary["by_class"].get("metadata", 0),
            summary["by_class"].get("review-required", 0),
        )
    )
    lines.append(
        "altered bytes: %d of %d matched-C bytes touching a POSTPROCESS'd "
        "object (atlas matched_overlay_c_bytes total: %d)"
        % (
            summary["altered_bytes"],
            summary["matched_c_bytes_with_postprocess"],
            atlas_totals["matched_overlay_c_bytes"],
        )
    )
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--write", action="store_true", help="refresh " + DEFAULT_OUT)
    ap.add_argument(
        "--check", action="store_true", help="fail if " + DEFAULT_OUT + " is stale"
    )
    ap.add_argument(
        "--check-redefines",
        action="store_true",
        help="fail on duplicate objcopy --redefine-sym destinations",
    )
    ap.add_argument("--out", default=DEFAULT_OUT)
    ap.add_argument(
        "--atlas",
        default=ATLAS_PATH,
        help="overlay atlas to join (default: " + ATLAS_PATH + ")",
    )
    args = ap.parse_args()

    if args.check_redefines:
        problems = redefine_conflicts(postprocess_commands(run_make_database()))
        if problems:
            for target, destination, sources in problems:
                print(
                    "%s: objcopy destination %s has multiple sources: %s"
                    % (target, destination, ", ".join(sources)),
                    file=sys.stderr,
                )
            return 1
        print("OK: objcopy --redefine-sym destinations are unique per invocation")
        return 0

    rows, atlas_totals = audit(atlas_path=args.atlas)
    summary = summarize(rows)
    doc = {
        "schema_version": 3,
        "generated_by": "tools/postprocess_audit.py",
        "summary": summary,
        "objects": rows,
    }
    payload = json.dumps(doc, indent=2, sort_keys=True) + "\n"

    if args.check:
        if not os.path.isfile(args.out):
            print("missing " + args.out, file=sys.stderr)
            return 1
        with open(args.out, encoding="utf-8") as fh:
            current = fh.read()
        if current != payload:
            print(args.out + " is stale; run with --write", file=sys.stderr)
            return 1
        if summary["by_class"].get("review-required", 0):
            print(args.out + " contains review-required helpers", file=sys.stderr)
            return 1
        print("OK: " + args.out + " matches the current tree")
        return 0

    if args.write:
        with open(args.out, "w", encoding="utf-8") as fh:
            fh.write(payload)
        print("wrote " + args.out)

    print(render_table(rows, summary, atlas_totals))
    return 0


if __name__ == "__main__":
    sys.exit(main())
