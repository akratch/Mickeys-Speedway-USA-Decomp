"""Fixture tests for tools/donor_match.py's parsers (no build, no donor tree needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import donor_match  # noqa: E402

ADDRS = """rumbleStart = 0x8002BD58; // type:func size:0x11C tier-B donor-jfg-src/saves.c
func_80002FE0 = 0x80002FE0; // type:func size:0xBC tier-B (JFG amSndPlayXYZ body/order, name unproved)
func_80036544 = 0x80036544; // type:func size:0x260 tier-D frame-exact candidate
"""

DONOR_C = """
void helper(void) { }
s32 flare_update(Object *obj) {
    if (sqrtf(x) > 1.0f) { amSndPlayXYZ(1, 2); }
    for (i = 0; i < n; i++) rumbleStart(i);
    return dAngle(a, b);
}
"""


class DonorMatchTests(unittest.TestCase):
    def test_name_map_reads_adopted_names_and_hints(self):
        m = donor_match.name_map(ADDRS)
        self.assertEqual(m["rumbleStart"], {"rumbleStart"})
        self.assertEqual(m["func_80002FE0"], {"amSndPlayXYZ"})
        self.assertNotIn("func_80036544", m, "a func_ with no donor hint maps to nothing")

    def test_callee_extraction_skips_keywords(self):
        heads = list(donor_match.FUNC_HEAD.finditer(DONOR_C))
        self.assertEqual([h.group(1) for h in heads], ["helper", "flare_update"])
        body = DONOR_C[heads[1].end():]
        callees = [c for c in donor_match.CALL.findall(body) if c not in donor_match.KEYWORDS]
        self.assertEqual(sorted(set(callees)), ["amSndPlayXYZ", "dAngle", "rumbleStart", "sqrtf"])

    def test_score_prefers_overlap_then_call_count(self):
        target = {"sqrtf", "amSndPlayXYZ", "dAngle"}
        good = donor_match.score(target, 3, 100, ["sqrtf", "amSndPlayXYZ", "dAngle"])
        partial = donor_match.score(target, 3, 100, ["sqrtf", "other", "more", "x"])
        self.assertGreater(good[0], partial[0])
        self.assertEqual(donor_match.score(target, 3, 100, [])[0], 0.0)


if __name__ == "__main__":
    unittest.main()
