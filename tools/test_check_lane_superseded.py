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

    def test_discovery_retains_friendly_and_generated_names(self):
        # The friendly symbol need not match its GLOBAL_ASM filename. Use a
        # controlled discovery result so matching the last such function does
        # not silently skip this regression or make project progress a failure.
        from types import SimpleNamespace
        entries = [SimpleNamespace(func="overlayExample"),
                   SimpleNamespace(func="func_example")]
        with mock.patch("permute_batch.discover_queue", return_value=entries) as discover:
            self.assertEqual(cls.live_queue(), {"overlayExample", "func_example"})
        discover.assert_called_once_with()

    def test_a_fully_matched_queue_can_be_empty(self):
        with mock.patch("permute_batch.discover_queue", return_value=[]) as discover:
            self.assertEqual(cls.live_queue(), set())
        discover.assert_called_once_with()


if __name__ == "__main__":
    unittest.main()
