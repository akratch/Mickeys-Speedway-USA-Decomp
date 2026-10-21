#!/usr/bin/env python3
"""symbol_addrs.us.txt parses the way splat parses it.

splat's handle_sym_addrs counts semicolons over the whole line, comment
included, and refuses any symbol line that does not carry exactly one. A
prose edit to a trailing `//` comment can therefore break `gmake extract`
for every clone, and nothing short of a re-split notices. This checks the
tracked file with splat's own rule, so check-tooling catches it without a
build.
"""

import re
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SYMBOL_FILE = ROOT / "symbol_addrs.us.txt"
MAIN_RE = re.compile(r"^[A-Za-z_.$][\w.$]*\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;$")


def symbol_line_errors(lines):
    """[(line number, reason)] for lines splat would refuse."""
    errors = []
    for number, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith("//"):
            continue
        if line.count(";") != 1:
            errors.append((number, "%d semicolons; splat requires exactly one per "
                           "line, comment included" % line.count(";")))
            continue
        main = line.split("//", 1)[0].strip()
        if not MAIN_RE.match(main):
            errors.append((number, "not of the form `<name> = <address>;`"))
    return errors


class SymbolLineRuleTests(unittest.TestCase):
    def test_semicolon_in_trailing_comment_is_refused(self):
        lines = ["func_80000000 = 0x80000000; // one; two"]
        self.assertEqual([n for n, _ in symbol_line_errors(lines)], [1])

    def test_comment_lines_and_blank_lines_are_skipped(self):
        lines = ["", "// a; prose; comment", "  // indented; too",
                 "func_80000000 = 0x80000000; // type:func size:0x8"]
        self.assertEqual(symbol_line_errors(lines), [])

    def test_missing_assignment_is_refused(self):
        self.assertEqual(len(symbol_line_errors(["func_80000000 0x80000000;"])), 1)


class TrackedSymbolFileTests(unittest.TestCase):
    def test_tracked_symbol_file_parses_under_splats_rule(self):
        errors = symbol_line_errors(SYMBOL_FILE.read_text(encoding="utf-8").splitlines())
        self.assertEqual(errors, [], "\n".join(
            "symbol_addrs.us.txt:%d: %s" % error for error in errors))


if __name__ == "__main__":
    sys.exit(unittest.main())
