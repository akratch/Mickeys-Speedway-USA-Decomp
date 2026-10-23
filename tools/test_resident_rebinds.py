#!/usr/bin/env python3
"""Format tests for mk/overlay_resident_rebinds.generated.mk."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import resident_rebinds as rr  # noqa: E402

A = "build/src/overlays/o014/overlay_014.c.o"
B = "build/src/overlays/o058/func_overlay_058_F000138C_18B0574.c.o"
PLAN = {
    B: {"sprintf": "sprintf_o058Reloc"},
    A: {"joyGetStickY": "joyGetStickY_o014Reloc",
        "joyGetPressed": "joyGetPressed_o014Reloc"},
}


class RenderParseTests(unittest.TestCase):
    def test_round_trip(self):
        self.assertEqual(PLAN, rr.parse(rr.render(PLAN)))

    def test_render_is_sorted(self):
        text = rr.render(PLAN)
        self.assertLess(text.index("o014/"), text.index("o058/"))
        self.assertLess(text.index("joyGetPressed="), text.index("joyGetStickY="))
        target = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14PrepareInputState.c.o"
        self.assertIn("%s: SURFACE_REBIND = $(OBJCOPY) \\\n" % target, text)
        self.assertTrue(text.endswith("sprintf=sprintf_o058Reloc $@\n"))

    def test_empty_plan_still_parses(self):
        text = rr.render({})
        self.assertIn("No overlay object currently needs one", text)
        self.assertEqual({}, rr.parse(text))

    def test_out_of_order_file_is_read_but_is_not_canonical(self):
        # A clean merge of two lanes' additions can reorder blocks: the planner
        # must still read it, and --check must still reject it.
        text = rr.render(PLAN)
        header, first, second = text.split("\n\n", 2)
        swapped = "\n\n".join([header, second.rstrip("\n"), first]) + "\n"
        self.assertEqual(PLAN, rr.parse(swapped))
        self.assertNotEqual(rr.render(PLAN), swapped)

    def test_rejects_a_hand_edited_rule(self):
        bad = rr.render(PLAN).replace("$(OBJCOPY)", "$(OBJCOPY) --strip-all", 1)
        with self.assertRaises(rr.RebindFileError):
            rr.parse(bad)

    def test_rejects_a_second_rule_for_one_object(self):
        text = rr.render({A: PLAN[A]})
        block = text.split("\n\n", 1)[1]
        with self.assertRaises(rr.RebindFileError):
            rr.parse(text + "\n" + block)

    def test_rejects_a_non_overlay_object(self):
        with self.assertRaises(rr.RebindFileError):
            rr.render({"build/src/main/menu.c.o": {"a": "b"}})

    def test_inverse(self):
        self.assertEqual({"sprintf_o058Reloc": "sprintf"}, rr.inverse(PLAN[B]))

    def test_write_if_changed(self):
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "rules.mk"
            self.assertTrue(rr.write_if_changed(path, "x\n"))
            self.assertFalse(rr.write_if_changed(path, "x\n"))
            self.assertTrue(rr.write_if_changed(path, "y\n"))
            self.assertEqual("y\n", path.read_text())


class TrackedFileTests(unittest.TestCase):
    def test_tracked_file_is_in_generated_form(self):
        # `check-overlay-syms` needs a build; this half of it does not, so a
        # hand edit or a bad merge of the rules fails `check-tooling` too.
        text = rr.GENERATED_MK.read_text()
        self.assertEqual(rr.render(rr.parse(text)), text)

    def test_tracked_file_is_included_and_every_overlay_object_depends_on_it(self):
        # Not only the listed objects: an entry that disappears must rebuild
        # its object, or the stale object hides the removal from the check.
        policy = (rr.REPO / "mk" / "overlays.mk").read_text()
        self.assertEqual(1, policy.count("include " + rr.GENERATED_MK_REL))
        self.assertIn("$(filter $(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o,$(O_FILES)): "
                      + rr.GENERATED_MK_REL, policy)


if __name__ == "__main__":
    unittest.main()
