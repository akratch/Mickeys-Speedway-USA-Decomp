"""Fixture tests for tools/stream_surgery.py (synthetic listings; no build, no ROM content)."""

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import stream_surgery as ss  # noqa: E402

LISTING = """\
\t.text
\t.globl\tfixture
\t.ent\tfixture 2
fixture:
\tone
\ttwo
\tthree
\tfour
\tfive
\t.end\tfixture
\t.ent\tother
other:
\tzzz
\t.end\tother
"""

SHOW = """\
/usr/lib/cfe -D_MIPS_FPSET=16 -DLANGUAGE_C
/usr/lib/uopt -v -G 0 -mips2 -EB -g0 -O2 /var/folders/x/ctmA /var/folders/x/ctmB
/usr/lib/ugen -v -G 0 -mips2 -EB -g0 -O2 /var/folders/x/ctmB -o /var/folders/x/ctmC
/usr/lib/as1 -elf -v -G 0 -p0 -mips2 -EB -g0 -O2 -r4300_mul /var/folders/x/ctmC -o out.o
"""


class Listing(unittest.TestCase):
    def test_split_and_join_round_trip(self):
        head, func, tail = ss.split_function(LISTING, "fixture")
        self.assertEqual(func[0].strip(), ".ent\tfixture 2")
        self.assertEqual(func[-1].strip(), ".end\tfixture")
        self.assertEqual(len(func), 8)
        self.assertEqual(ss.join_listing(head, func, tail), LISTING)

    def test_other_function_untouched(self):
        _, func, tail = ss.split_function(LISTING, "fixture")
        self.assertIn("\t.ent\tother", "\n".join(tail))
        self.assertNotIn("zzz", "\n".join(func))

    def test_missing_symbol_exits(self):
        with self.assertRaises(SystemExit):
            ss.split_function(LISTING, "absent")


class Ranges(unittest.TestCase):
    def setUp(self):
        _, self.func, _ = ss.split_function(LISTING, "fixture")

    def test_parse_sorts_and_validates(self):
        self.assertEqual(ss.parse_ranges("6-7,3", 8), [(3, 3), (6, 7)])
        with self.assertRaises(SystemExit):
            ss.parse_ranges("3-5,4-6", 8)       # overlap
        with self.assertRaises(SystemExit):
            ss.parse_ranges("3,9", 8)           # out of range
        with self.assertRaises(SystemExit):
            ss.parse_ranges("3-4", 8)           # one chunk cannot be permuted

    def test_identity_order_is_unchanged(self):
        r = ss.parse_ranges("5-5,7-8", len(self.func))
        self.assertEqual(ss.permute_chunks(self.func, r, (0, 1)), self.func)

    def test_swap_keeps_text_between_slots(self):
        r = ss.parse_ranges("5,7", len(self.func))     # `three` and `five`
        got = ss.permute_chunks(self.func, r, (1, 0))
        self.assertEqual([l.strip() for l in got[2:7]], ["one", "two", "five", "four", "three"])
        self.assertEqual(got[-1], self.func[-1])
        self.assertEqual(len(got), len(self.func))

    def test_unequal_chunks_resize_the_listing_not_the_gaps(self):
        r = ss.parse_ranges("3,5-6", len(self.func))   # `one` and `three`+`four`
        got = ss.permute_chunks(self.func, r, (1, 0))
        self.assertEqual([l.strip() for l in got[2:7]], ["three", "four", "two", "one", "five"])

    def test_order_count_limit(self):
        self.assertEqual(len(list(ss.orders(3, 100))), 6)
        with self.assertRaises(SystemExit):
            ss.orders(6, 100)


class Flags(unittest.TestCase):
    def test_tool_flags_stop_at_first_path(self):
        self.assertEqual(ss.tool_flags(SHOW, "as1"),
                         ["-elf", "-v", "-G", "0", "-p0", "-mips2", "-EB", "-g0", "-O2", "-r4300_mul"])
        self.assertEqual(ss.tool_flags(SHOW, "ugen"), ["-v", "-G", "0", "-mips2", "-EB", "-g0", "-O2"])

    def test_missing_tool_exits(self):
        with self.assertRaises(SystemExit):
            ss.tool_flags("nothing here", "as1")

    def test_swap_to_S(self):
        got = ss.swap_to_S(["tools/ido/cc", "-c", "-O2", "-I", "include", "-o", "x.o", "a.c"])
        self.assertEqual(got[0], str(ss.ROOT / "tools/ido/cc"))
        self.assertEqual(got[1:4], ["-S", "-O2", "-I"])
        self.assertEqual(got[4], str((ss.ROOT / "include").resolve()))
        self.assertNotIn("-o", got)
        self.assertEqual(got[-1], "a.c")


class Edits(unittest.TestCase):
    def test_script_edit(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.py"
            p.write_text("def edit(lines):\n    return [l for l in lines if 'two' not in l]\n")
            _, func, _ = ss.split_function(LISTING, "fixture")
            got = ss.apply_script(p, func)
            self.assertEqual(len(got), len(func) - 1)

    def test_script_without_edit_exits(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.py"
            p.write_text("x = 1\n")
            with self.assertRaises(SystemExit):
                ss.apply_script(p, ["a"])

    def test_script_must_return_strings(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.py"
            p.write_text("def edit(lines):\n    return 3\n")
            with self.assertRaises(SystemExit):
                ss.apply_script(p, ["a"])

    def test_diff_edit(self):
        diff = ("--- a\n+++ b\n@@ -5,3 +5,3 @@\n \tone\n-\ttwo\n+\ttwo2\n \tthree\n")
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.diff"
            p.write_text(diff)
            out = ss.apply_diff(p, LISTING, Path(d))
        self.assertIn("\ttwo2\n", out)
        self.assertNotIn("\ttwo\n", out)

    def test_bad_diff_exits(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.diff"
            p.write_text("--- a\n+++ b\n@@ -1,1 +1,1 @@\n-nothing like it\n+x\n")
            with self.assertRaises(SystemExit):
                ss.apply_diff(p, LISTING, Path(d))


class Instrumented(unittest.TestCase):
    def test_wanted_by_env_or_flag(self):
        self.assertFalse(ss.instrumented_wanted({"PATH": "x", "CDX_FORCE": ""}))
        self.assertTrue(ss.instrumented_wanted({"CDX_BIAS": "w1=2"}))
        self.assertTrue(ss.instrumented_wanted({"DKWB_CUT_X": "1"}))
        self.assertFalse(ss.instrumented_wanted({"DKWB_UGEN_TRACE": "1"}))
        self.assertTrue(ss.instrumented_wanted({}, flag=True))

    def test_use_compiler_swaps_only_the_driver(self):
        self.assertEqual(ss.use_compiler(["cc", "-O2"], Path("/i/cc")), ["/i/cc", "-O2"])
        self.assertEqual(ss.use_compiler(["cc", "-O2"], None), ["cc", "-O2"])

    def test_swap_to_S_keeps_a_symlinked_driver_unresolved(self):
        with tempfile.TemporaryDirectory() as d:
            real = Path(d) / "real"
            real.write_text("")
            link = Path(d) / "link"
            link.symlink_to(real)
            got = ss.swap_to_S([str(link), "-c", "-o", "x.o", "a.c"])
            self.assertEqual(got[0], str(link))
            self.assertEqual(got[1:], ["-S", "a.c"])


if __name__ == "__main__":
    unittest.main()
