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

    def test_bare_else_adds_max_plus_one(self):
        src = "#if SHAPE_a == 1\nx;\n#elif SHAPE_a == 2\ny;\n#else\nz;\n#endif\n"
        self.assertEqual(shape_product.axes_of(src)["a"], [0, 1, 2, 3])

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


if __name__ == "__main__":
    unittest.main()
