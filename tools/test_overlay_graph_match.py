#!/usr/bin/env python3
"""Synthetic function-boundary regressions for the overlay comparison."""

from pathlib import Path
import unittest

import overlay_graph_match as graph


class FakeElf:
    def __init__(self, symbols):
        self.rows = symbols

    def section(self, name):
        return (1, None) if name == ".text" else (None, None)

    def symbols(self):
        return self.rows


def module(rows):
    return {
        "overlay": 1, "text_ownership": rows, "imports": [],
        "sections": {"text": {"size": "0x40"}, "data_rodata": {"size": "0x0"}},
        "bss_size": "0x0", "cross_overlay_inbound_relocations": 0,
    }


def owner(name, offset, size):
    return {"type": "c", "source": "overlays/o001/" + name,
            "offset": hex(offset), "size": hex(size), "matched": True}


class FunctionBoundaryTests(unittest.TestCase):
    def ranges(self, rows, objects):
        atlas = {"modules": [module(rows)]}
        by_path = {path: FakeElf(symbols) for path, symbols in objects}
        return graph.mickey_function_ranges(
            atlas, [(1, path) for path in by_path], elf_factory=by_path.__getitem__)

    def test_fold_preserves_functions_including_statics_and_deduplicates_aliases(self):
        global_fn = ("first", 0, 0x20, 0x12, 1)
        static_fn = ("helper", 0x20, 0x20, 0x02, 1)
        alias = ("func_alias", 0, 0x20, 0x12, 1)
        folded = self.ranges([owner("folded", 0, 0x40)], [
            (Path("build/src/overlays/o001/folded.c.o"), [global_fn, static_fn, alias])])
        split = self.ranges([owner("first", 0, 0x20), owner("tail", 0x20, 0x20)], [
            (Path("build/src/overlays/o001/first.c.o"), [global_fn]),
            (Path("build/src/overlays/o001/tail.c.o"), [("helper", 0, 0x20, 0x02, 1)])])
        self.assertEqual(split, folded)
        self.assertEqual([("first", 0, 0x20), ("helper", 0x20, 0x20)], folded[1])
        atlas = {"modules": [module([owner("folded", 0, 0x40)])]}
        features, _ = graph.build_mickey_features(atlas, function_ranges=folded)
        self.assertEqual(2, features[1]["function_count"])
        self.assertEqual(2, features[1]["matched_function_count"])

    def test_data_only_object_does_not_need_a_text_owner(self):
        functions = self.ranges([owner("folded", 0, 0x40)], [
            (Path("build/src/overlays/o001/data.c.o"), [("table", 0, 8, 0x11, 2)]),
            (Path("build/src/overlays/o001/folded.c.o"), [("first", 0, 0x40, 0x12, 1)])])
        self.assertEqual([("first", 0, 0x40)], functions[1])

    def test_function_outside_its_owned_extent_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "function escapes its TU"):
            self.ranges([owner("folded", 0, 0x40)], [
                (Path("build/src/overlays/o001/folded.c.o"), [("first", 0, 0x44, 0x12, 1)])])


if __name__ == "__main__":
    unittest.main()
