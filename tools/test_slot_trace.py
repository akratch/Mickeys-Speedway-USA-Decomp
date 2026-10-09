#!/usr/bin/env python3
"""Tests for tools/slot_trace.py on synthetic DKWB-SLOT rows (no compile)."""
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import slot_trace as st  # noqa: E402

LOG = """\
cfe: Warning 835: something on stderr that is not a row
DKWB-SLOT event=procedure proc=0
DKWB-SLOT event=procedure proc=1
DKWB-SLOT event=request proc=1 request=1 path=spill owner=0x10062410 size=4 index=11 reserve=4 caller=spilltemps raw_kind=4
DKWB-SLOT event=candidate proc=1 request=1 reason=busy index=0 available=0 size=4
DKWB-SLOT event=chosen proc=1 request=1 slot=0x1005e588 index=0 offset=-8 size=4 reused=0 before=4 after=8
DKWB-SLOT event=request proc=1 request=2 path=temp owner=0x10062198 size=4 index=15 reserve=8 caller=spilltemps raw_kind=4
DKWB-SLOT event=chosen proc=1 request=2 slot=0x1005e588 index=0 offset=-8 size=4 reused=1 before=8 after=8
DKWB-SLOT event=request proc=0 request=3 path=spill owner=0x1 size=4 index=1 reserve=4 caller=x raw_kind=4
DKWB-SLOT event=home proc=1 opcode=3 mtype=1 block=2 length=4 offset=-8 before=4 after=8
"""


class SlotTraceTests(unittest.TestCase):
    def test_rows_parse_and_noise_is_ignored(self):
        rows = st.parse_rows(LOG)
        self.assertEqual(len(rows), 9)
        self.assertEqual(st.procedure_count(rows), 2)

    def test_requests_join_their_candidates_and_choice(self):
        entries = st.by_request(st.parse_rows(LOG), 1)
        self.assertEqual([e["request"] for e in entries], ["1", "2"])
        self.assertEqual(len(entries[0]["candidates"]), 1)
        self.assertEqual(entries[1]["chosen"]["reused"], "1")

    def test_render_reads_owner_and_reuse(self):
        text = st.render("f", 1, st.parse_rows(LOG), candidates=True)
        self.assertIn("req 1  spill  owner 0x10062410 size 4 index 11 reserve 4", text)
        self.assertIn("-> slot 0x1005e588 index 0 offset -8 new  frame 4 -> 8", text)
        self.assertIn("reuse  frame 8 -> 8", text)
        self.assertIn("candidate index 0 size 4 available 0: busy", text)
        self.assertNotIn("0x1 size", text)         # proc 0's request is not mixed in
        self.assertNotIn("candidate index", st.render("f", 1, st.parse_rows(LOG), False))

    def test_missing_source_is_refused(self):
        with self.assertRaises(SystemExit):
            st.main(["f", "--source", "nope/cand.c"])


if __name__ == "__main__":
    unittest.main()
