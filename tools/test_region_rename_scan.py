"""Fixture tests for tools/region_rename_scan.py and tools/behaviour_check.py (no build needed)."""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import behaviour_check  # noqa: E402
import region_rename_scan as rrs  # noqa: E402

SOURCE = """typedef int s32;

s32 sample(s32 n, s32 *out)
{
    s32 i;
    s32 j;
    s32 sum;
    s32 *p;

    sum = 0;
    i = n;
    while (i--) {
        sum += i;
    }

    j = 0;
    p = out;
    for (j = 0; j < n; j++) {
        *p++ = j;
    }
    return sum;
}
"""

DRIVER = """#include <stdio.h>
typedef int s32;
s32 sample(s32 n, s32 *out);
int main(void)
{
    s32 buf[16];
    s32 n;
    for (n = 0; n < 16; n++) {
        printf("%d %d\\n", sample(n, buf), n ? buf[n - 1] : 0);
    }
    return 0;
}
"""


class RegionTests(unittest.TestCase):
    def setUp(self):
        self.lines = SOURCE.split("\n")

    def test_declared_locals_by_type(self):
        decl = rrs.declared_locals(self.lines, "sample")
        self.assertEqual(decl, {"i": "s32", "j": "s32", "sum": "s32", "p": "s32*"})

    def test_auto_regions_find_both_loops(self):
        regions = rrs.auto_regions(self.lines, "sample")
        spans = sorted(tuple(v) for v in regions.values())
        self.assertIn((12, 14), spans)  # while body
        self.assertIn((18, 20), spans)  # for body

    def test_moves_refuse_a_name_already_in_the_region(self):
        moves = rrs.candidate_moves(self.lines, "sample", {"w": [12, 14]})
        self.assertIn((12, 14, "i", "j"), moves)
        self.assertNotIn((12, 14, "i", "sum"), moves, "sum occurs in the region")
        self.assertFalse(any(m[3] == "p" for m in moves), "a pointer is not an int's class")

    def test_apply_moves_keeps_line_numbers(self):
        out = rrs.apply_moves(self.lines, [(11, 14, "i", "j")])
        self.assertEqual(len(out), len(self.lines))
        self.assertEqual(out[10].strip(), "j = n;")
        self.assertEqual(out[18].strip(), "*p++ = j;")


@unittest.skipUnless(shutil.which("cc"), "no host C compiler")
class BehaviourTests(unittest.TestCase):
    def test_same_and_different(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            driver = tmp / "driver.c"
            driver.write_text(DRIVER)
            ref = tmp / "ref.c"
            ref.write_text(SOURCE)
            renamed = tmp / "renamed.c"
            renamed.write_text("\n".join(rrs.apply_moves(SOURCE.split("\n"), [(11, 14, "i", "j")])))
            broken = tmp / "broken.c"
            broken.write_text(SOURCE.replace("sum += i;", "sum += i + 1;"))
            self.assertEqual(behaviour_check.same_behaviour(renamed, ref, driver)[0], 0)
            self.assertEqual(behaviour_check.same_behaviour(broken, ref, driver)[0], 1)
            bad = tmp / "bad.c"
            bad.write_text("this is not C")
            self.assertEqual(behaviour_check.same_behaviour(bad, ref, driver)[0], 2)


if __name__ == "__main__":
    unittest.main()
