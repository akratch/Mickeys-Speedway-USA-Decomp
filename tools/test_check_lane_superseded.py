#!/usr/bin/env python3
"""Guards on the supersession advisory, both from defects it actually had."""
import unittest
from unittest import mock

import check_lane_superseded as cls


class WorkedSymbolTests(unittest.TestCase):
    """A lane names its functions however it likes; read every subject."""

    def subjects(self, *lines):
        log = "".join(f"{'a' * 40}\x01{line}\n" for line in lines)
        with mock.patch.object(cls, "git", return_value=log):
            return cls.worked_symbols("lane/x", "base")

    def test_a_subject_leading_with_the_symbol_is_read(self):
        """The shape that made the first draft silently find nothing."""
        found = self.subjects("func_80004454 and func_8000471C: a spilltemps temporary")
        self.assertEqual(set(found), {"func_80004454", "func_8000471C"})

    def test_a_conventional_match_subject_is_still_read(self):
        self.assertIn("func_80038E1C", self.subjects("Match func_80038E1C (1116 bytes)"))

    def test_friendly_overlay_names_are_read(self):
        self.assertIn("overlay41AddSlot", self.subjects("Promote overlay41AddSlot"))

    def test_a_subject_naming_no_symbol_contributes_nothing(self):
        self.assertEqual(self.subjects("Close the three objects.c plateau handoffs"), {})

    def test_the_first_mentioning_commit_is_the_one_reported(self):
        found = self.subjects("func_A: later", "func_A: earlier")
        self.assertEqual(len(found), 1)


class LiveQueueTests(unittest.TestCase):
    """The regression that nearly shipped.

    An overlay function carrying a friendly name has its GLOBAL_ASM fallback
    written under the GENERATED name, so deciding "is it still queued?" by
    grepping for a pragma named after the symbol finds nothing and reports
    every such function as already matched -- inverting the tool's answer for
    exactly the functions it exists to reason about. The queue must come from
    the project's own discovery.
    """

    def test_a_friendly_named_overlay_function_is_seen_as_queued(self):
        # Any friendly-named overlay candidate will do. Naming one symbol made
        # the test fail whenever that symbol matched (overlay1UpdateRangeFlags
        # did on 2026-10-01); the regression hides all of them at once.
        queue = cls.live_queue()
        friendly = [s for s in queue if s.startswith("overlay")]
        self.assertTrue(friendly,
                        "a friendly-named overlay candidate must read as queued")

    def test_the_queue_is_not_empty(self):
        self.assertGreater(len(cls.live_queue()), 100)


if __name__ == "__main__":
    unittest.main()
