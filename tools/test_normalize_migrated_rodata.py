"""Fixture tests for tools/normalize_migrated_rodata.py (no build needed).

The listings below are synthetic: invented labels and an invented string,
shaped like what splat writes when it migrates a TU's rodata into a
function's listing.
"""

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import normalize_migrated_rodata as nmr  # noqa: E402

STRING_LISTING = """.section .rodata
.align 2
dlabel D_EXAMPLE
    .asciz "example"
.align 2
enddlabel D_EXAMPLE

.section .text
glabel exampleFunction
    nop
  .LEXAMPLE_1:
    nop
endlabel exampleFunction
"""

TABLE_LISTING = """.section .late_rodata
.late_rodata_alignment 4
dlabel jtbl_EXAMPLE
    .word .LEXAMPLE_A
    .word .LEXAMPLE_B
    .word .LEXAMPLE_A
enddlabel jtbl_EXAMPLE

.section .text
glabel exampleSwitch
    nop
  .LEXAMPLE_A:
    nop
  .LEXAMPLE_OTHER:
    nop
  .LEXAMPLE_B:
    nop
endlabel exampleSwitch
"""

PLAIN_LISTING = """glabel examplePlain
    nop
  .LEXAMPLE_1:
    nop
endlabel examplePlain
"""


class NormalizeTests(unittest.TestCase):
    def test_string_alignment_becomes_balign_4(self):
        out = nmr.normalize(STRING_LISTING)
        self.assertNotIn(".align 2", out)
        self.assertEqual(out.count(".balign 4"), 2)
        self.assertIn("  .LEXAMPLE_1:", out, "an unrelated local label is left alone")

    def test_table_targets_become_global_labels(self):
        out = nmr.normalize(TABLE_LISTING)
        self.assertIn("  glabel .LEXAMPLE_A\n", out)
        self.assertIn("  glabel .LEXAMPLE_B\n", out)
        self.assertIn("  .LEXAMPLE_OTHER:", out, "a label no table names stays local")
        self.assertIn("    .word .LEXAMPLE_A\n", out, "the table itself is unchanged")

    def test_is_idempotent(self):
        for listing in (STRING_LISTING, TABLE_LISTING):
            once = nmr.normalize(listing)
            self.assertEqual(nmr.normalize(once), once)

    def test_text_alignment_outside_a_migrated_section_is_untouched(self):
        listing = ".section .text\n.align 2\nglabel exampleAligned\n    nop\n"
        self.assertEqual(nmr.normalize(listing), listing)

    def test_only_files_with_a_migrated_section_are_rewritten(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "tu").mkdir()
            plain = root / "tu" / "plain.s"
            table = root / "tu" / "table.s"
            plain.write_text(PLAIN_LISTING)
            table.write_text(TABLE_LISTING)
            self.assertEqual(nmr.main(["--check", "--root", str(root)]), 1)
            self.assertEqual(table.read_text(), TABLE_LISTING, "--check writes nothing")
            self.assertEqual(nmr.main(["--root", str(root)]), 0)
            self.assertEqual(plain.read_text(), PLAIN_LISTING)
            self.assertIn("glabel .LEXAMPLE_A", table.read_text())
            self.assertEqual(nmr.main(["--check", "--root", str(root)]), 0)


if __name__ == "__main__":
    unittest.main()
