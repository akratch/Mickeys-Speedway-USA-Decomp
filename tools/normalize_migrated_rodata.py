#!/usr/bin/env python3
"""Make splat's migrated rodata acceptable to the vendored asm-processor.

Attaching a `.rodata` subsegment to a `c` subsegment makes splat migrate the
translation unit's constants into the `asm/nonmatchings/**.s` listing of the
function that reads them, as a leading `.section .rodata` or
`.section .late_rodata` block. asm-processor then reserves the bytes in the
compiled object and copies the assembled ones over them, which is what lets
a TU own its whole `.rodata` while some of its functions are still assembled.

Two things splat writes in such a listing stop asm-processor, and neither is
worth a patch to the submodule:

  * `.align 2` around a migrated string. asm-processor's line scanner reads
    the operand as a byte count and accepts only 4 ("only .balign 4 is
    supported"). The directive means four bytes, so it is rewritten to
    `.balign 4`, the spelling the scanner takes.

  * a migrated jump table's targets. While the table lived in a shared data
    file, splat marked each target in the function with `glabel .L<addr>`
    (`asm_jtbl_label_macro: glabel` in the yaml), which makes the label a
    global symbol. Once the table is in the function's own listing splat
    writes the targets as plain local labels, so gas relocates the table
    against the listing's `.text` section symbol, and asm-processor refuses
    the second `.text` it then has to merge ("symbol \".text\" defined
    twice"). Every label a rodata `.word` names is therefore put back to the
    `glabel` form the unmigrated listing had.

The rewrite is idempotent and touches only files that carry a migrated
section. It runs after every split, beside `prune_stale_asm.py`.

Usage:
    tools/normalize_migrated_rodata.py            (rewrites asm/nonmatchings)
    tools/normalize_migrated_rodata.py --check    (exit 1 if a file would change)
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
NONMATCHINGS = ROOT / "asm" / "nonmatchings"

SECTION = re.compile(r"^\s*\.section\s+(\.[A-Za-z_]+)")
MIGRATED = (".rodata", ".late_rodata")
ALIGN_2 = re.compile(r"^(\s*)\.align\s+2\s*$")
WORD_LABEL = re.compile(r"\.word\s+(\.L[0-9A-Za-z_]+)\s*$")
LOCAL_LABEL = re.compile(r"^(\s*)(\.L[0-9A-Za-z_]+):\s*$")


def normalize(text: str) -> str:
    """Return `text` with migrated-rodata alignment and jump targets rewritten."""
    lines = text.split("\n")
    section = ".text"
    targets: set[str] = set()
    for index, line in enumerate(lines):
        match = SECTION.match(line)
        if match:
            section = match.group(1)
            continue
        if section not in MIGRATED:
            continue
        align = ALIGN_2.match(line)
        if align:
            lines[index] = f"{align.group(1)}.balign 4"
            continue
        word = WORD_LABEL.search(line)
        if word:
            targets.add(word.group(1))
    if targets:
        section = ".text"
        for index, line in enumerate(lines):
            match = SECTION.match(line)
            if match:
                section = match.group(1)
                continue
            if section != ".text":
                continue
            label = LOCAL_LABEL.match(line)
            if label and label.group(2) in targets:
                lines[index] = f"{label.group(1)}glabel {label.group(2)}"
    return "\n".join(lines)


def carries_migrated_section(text: str) -> bool:
    return any(
        (match := SECTION.match(line)) and match.group(1) in MIGRATED
        for line in text.split("\n")
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("--check", action="store_true",
                        help="report files that would change and exit 1")
    parser.add_argument("--root", type=pathlib.Path, default=NONMATCHINGS,
                        help="directory of listings (default: asm/nonmatchings)")
    args = parser.parse_args(argv)

    changed = []
    if args.root.is_dir():
        for path in sorted(args.root.rglob("*.s")):
            text = path.read_text()
            if not carries_migrated_section(text):
                continue
            new = normalize(text)
            if new != text:
                changed.append(path)
                if not args.check:
                    path.write_text(new)
    for path in changed:
        verb = "would normalize" if args.check else "normalized"
        try:
            shown = path.relative_to(ROOT)
        except ValueError:
            shown = path
        print(f"normalize-migrated-rodata: {verb} {shown}")
    return 1 if (args.check and changed) else 0


if __name__ == "__main__":
    sys.exit(main())
