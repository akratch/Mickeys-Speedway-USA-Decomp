"""Fixture tests for tools/shape_product.py's axis parsing (no build needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import shape_product  # noqa: E402

SRC = """
#if SHAPE_loop == 0
    while (n--) { }
#elif SHAPE_loop == 1
    for (i = 0; i < n; i++) { }
#elif SHAPE_loop == 2
    do { } while (--n);
#endif
#if SHAPE_carrier == 0
    v = g;
#else
    g;
#endif
#if SHAPE_pad != 0
    s32 pad[SHAPE_pad];
#endif
"""


class AxisTests(unittest.TestCase):
    def test_axes_and_values(self):
        axes = shape_product.axes_of(SRC)
        self.assertEqual(axes["loop"], [0, 1, 2])
        self.assertEqual(axes["carrier"], [0, 1], "a bare #if/#else axis is two cells")
        self.assertEqual(axes["pad"], [0, 1])

    def test_bare_else_adds_the_smallest_uncompared_value(self):
        # 0 is not compared, so it already reaches the #else arm: no fourth cell.
        src = "#if SHAPE_a == 1\nx;\n#elif SHAPE_a == 2\ny;\n#else\nz;\n#endif\n"
        self.assertEqual(shape_product.axes_of(src)["a"], [0, 1, 2])
        # A chain that tests 0 upward needs one value past it.
        src = "#if SHAPE_a == 0\nx;\n#elif SHAPE_a == 1\ny;\n#else\nz;\n#endif\n"
        self.assertEqual(shape_product.axes_of(src)["a"], [0, 1, 2])

    def test_if_equals_one_is_not_enumerated_one_past(self):
        for tail in ("", "#else\nb;\n"):
            src = "#if SHAPE_x == 1\na;\n" + tail + "#endif\n"
            self.assertEqual(shape_product.axes_of(src)["x"], [0, 1], tail)

    def test_ordering_operators_count(self):
        src = "#if SHAPE_a >= 2\nx;\n#endif\n#if SHAPE_a != 4\ny;\n#endif\n"
        self.assertEqual(shape_product.axes_of(src)["a"], [0, 2, 4])

    def test_define_fixes_axis_unless_all_axes(self):
        src = "#define SHAPE_a 2\n#if SHAPE_a == 0\nx;\n#else\ny;\n#endif\n#if SHAPE_b == 1\n#endif\n"
        self.assertEqual(list(shape_product.axes_of(src)), ["b"])
        self.assertEqual(shape_product.axes_of(src, all_axes=True)["a"], [0, 1])

    def test_defines_name_every_axis(self):
        defs = shape_product.cell_defines({"loop": 2, "carrier": 0})
        self.assertEqual(defs, ["-DSHAPE_loop=2", "-DSHAPE_carrier=0"])

    def test_sort_puts_exact_first_and_errors_last(self):
        rows = [
            {"masked": 4, "delta": 0},
            {"error": ["x"]},
            {"masked": 0, "delta": 0},
            {"masked": 0, "delta": 4},
        ]
        rows.sort(key=shape_product.sort_key)
        self.assertEqual(rows[0], {"masked": 0, "delta": 0})
        self.assertEqual(rows[1], {"masked": 4, "delta": 0}, "a size mismatch ranks below any delta-0 cell")
        self.assertIn("error", rows[-1])


def _cell(residual, delta, masked=0):
    return {"residual": residual, "delta": delta, "masked": masked}


class AlignedRankTests(unittest.TestCase):
    def test_aligned_orders_by_residual_then_size_delta(self):
        rows = [_cell(10, 0, 40), _cell(3, 4, 90), _cell(3, 0, 95), {"error": ["x"]}]
        rows.sort(key=shape_product.aligned_key)
        self.assertEqual([(r.get("residual"), r.get("delta")) for r in rows],
                         [(3, 0), (3, 4), (10, 0), (None, None)])

    def test_positional_would_have_picked_the_other_cell(self):
        rows = [_cell(10, 0, 40), _cell(3, 4, 90)]
        self.assertEqual(min(rows, key=shape_product.sort_key)["residual"], 10)
        self.assertEqual(min(rows, key=shape_product.aligned_key)["residual"], 3)

    def test_auto_is_aligned_only_when_a_cell_is_off_size(self):
        flat = [_cell(1, 0), _cell(2, 0), {"error": ["x"]}]
        self.assertEqual(shape_product.choose_rank(flat, "auto"), "positional")
        self.assertEqual(shape_product.choose_rank(flat + [_cell(5, -4)], "auto"), "aligned")
        self.assertEqual(shape_product.choose_rank(flat, "aligned"), "aligned")

    def test_strip_defines_leaves_conditionals(self):
        src = "#define SHAPE_a 2\nint x;\n#if SHAPE_a == 0\n#endif\n"
        out = shape_product.strip_defines(src)
        self.assertNotIn("#define", out)
        self.assertIn("#if SHAPE_a == 0", out)


class AlignedBucketTests(unittest.TestCase):
    def test_buckets_and_one_sided_words(self):
        import align_symbol
        import nm_ranking as nr
        nop, addu = 0x00000000, 0x00851021          # addu v0,a0,a1
        addu_v1 = 0x00851821                        # same shape, other dest register
        streams = nr.WordStreams(
            base_words=[addu_v1, nop, nop, nop], target_words=[addu, nop, nop],
            base_reloc={}, target_reloc={}, base_size=16, target_size=12)
        row = align_symbol.align(streams)
        row["residual"] = (row["aligned_register_naming"] + row["aligned_immediate_only"]
                           + row["aligned_really_different"])
        self.assertEqual(row["aligned_register_naming"], 1)
        self.assertEqual(sum(s["words"] for s in row["insertions"]), 1)
        self.assertEqual(row["residual"], 2)
        self.assertIn("candidate-only 1", align_symbol.render_buckets(row))


if __name__ == "__main__":
    unittest.main()
