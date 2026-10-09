#!/usr/bin/env python3
"""tools/objdiff_config.py against a synthetic project tree (no build, no ROM)."""
import json
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import objdiff_config as oc  # noqa: E402

YAML = """\
options: {asm_path: asm, src_path: src, asset_path: assets}
segments:
  - name: main
    type: code
    subsegments:
      - [0x1000, c, main/game]
      - [0x2000, c, libultra/osFoo]
      - [0x3000, asm, main/fastmath]
      - [0x4000]
  - name: overlay_001
    type: code
    dir: overlays/o001
    subsegments:
      - [0x5000, c, ov1_a]
      - [0x5100, asm, ov1_tail]
      - [0x5200, bin, ov1_reloc1]
"""
ATLAS = {"modules": [{"overlay": 1, "text_ownership": [
    {"type": "c", "source": "overlays/o001/ov1_a"},
    {"type": "asm", "source": "overlays/o001/ov1_padding"}]}]}
FLAGS = {"src/main/game.c": "-O2", "src/libultra/osFoo.c": "-O1",
         "src/overlays/o001/ov1_a.c": "-O2 -mips2"}


def make_tree(root, orphan=False, global_asm=False, atlas=ATLAS):
    files = {
        "mickey.us.yaml": YAML,
        "src/main/game.c": "#pragma GLOBAL_ASM(\"x.s\")\n" if global_asm else "int x;\n",
        "src/libultra/osFoo.c": "int y;\n",
        "src/overlays/o001/ov1_a.c": "int z;\n",
        "asm/main/fastmath.s": "", "asm/data/81590.rodata.s": "",
        "asm/overlays/o001/ov1_tail.s": "",
        "asm/nonmatchings/main/skip.s": "",
        "assets/overlays/o001/ov1_reloc1.bin": "",
    }
    if orphan:
        files["src/main/stray.c"] = "int s;\n"
    if atlas is not None:
        files["config/overlays.us.json"] = json.dumps(atlas)
    for rel, text in files.items():
        p = root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text)


class ObjdiffConfigTests(unittest.TestCase):
    def build(self, **kw):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        root = pathlib.Path(d.name)
        make_tree(root, **kw)
        cfg, orphans, problems = oc.build_config(root, flags=FLAGS)
        return root, cfg, orphans, problems, {u["name"]: u for u in cfg["units"]}

    def test_unit_set_and_paths(self):
        _, cfg, _, problems, units = self.build()
        self.assertEqual(problems, [])
        self.assertEqual(sorted(units), [
            "asm/data/81590.rodata.s", "asm/main/fastmath.s",
            "asm/overlays/o001/ov1_tail.s", "assets/overlays/o001/ov1_reloc1.bin",
            "src/libultra/osFoo.c", "src/main/game.c", "src/overlays/o001/ov1_a.c"])
        u = units["src/main/game.c"]
        self.assertEqual(u["base_path"], "build/src/main/game.c.o")
        self.assertEqual(u["target_path"], "expected/build/src/main/game.c.o")
        self.assertEqual(u["metadata"]["source_path"], "src/main/game.c")
        self.assertEqual(u["scratch"]["c_flags"], "-O2")

    def test_asm_units_have_no_base_and_are_not_complete(self):
        _, _, _, _, units = self.build()
        for n in ("asm/main/fastmath.s", "assets/overlays/o001/ov1_reloc1.bin"):
            self.assertNotIn("base_path", units[n])
            self.assertNotIn("complete", units[n]["metadata"])
        self.assertTrue(units["src/main/game.c"]["metadata"]["complete"])

    def test_global_asm_c_unit_is_incomplete(self):
        _, _, _, _, units = self.build(global_asm=True)
        self.assertFalse(units["src/main/game.c"]["metadata"]["complete"])

    def test_categories(self):
        _, cfg, _, _, units = self.build()
        cat = lambda n: units[n]["metadata"]["progress_categories"]
        self.assertEqual(cat("src/main/game.c"), ["resident"])
        self.assertEqual(cat("src/libultra/osFoo.c"), ["libultra"])
        self.assertEqual(cat("asm/overlays/o001/ov1_tail.s"), ["overlay", "overlay-o001"])
        ids = [c["id"] for c in cfg["progress_categories"]]
        self.assertEqual(ids, ["libultra", "overlay", "resident", "overlay-o001"])

    def test_orphan_source_gets_no_unit(self):
        _, _, orphans, _, units = self.build(orphan=True)
        self.assertEqual(orphans, ["src/main/stray.c"])
        self.assertNotIn("src/main/stray.c", units)

    def test_nonmatchings_are_not_units(self):
        _, _, _, _, units = self.build()
        self.assertFalse(any("nonmatchings" in n for n in units))

    def test_atlas_yaml_disagreement_is_reported(self):
        atlas = {"modules": [{"overlay": 1, "text_ownership": [
            {"type": "c", "source": "overlays/o001/ghost"}]}]}
        _, _, _, problems, _ = self.build(atlas=atlas)
        text = "\n".join(problems)
        self.assertIn("ghost", text)
        self.assertIn("ov1_a", text)

    def test_deterministic(self):
        root, cfg, _, _, _ = self.build()
        again, _, _ = oc.build_config(root, flags=FLAGS)
        self.assertEqual(oc.render(cfg), oc.render(again))

    def test_exclusions_apply_only_on_request(self):
        root, cfg, _, _, _ = self.build()
        (root / "tools").mkdir()
        (root / "tools/objdiff_exclude.txt").write_text("src/main/game.c.o\n")
        plain, _, _ = oc.build_config(root, flags=FLAGS)
        self.assertEqual(oc.render(plain), oc.render(cfg))
        cut, _, _ = oc.build_config(root, flags=FLAGS, apply_excludes=True)
        self.assertNotIn("src/main/game.c", [u["name"] for u in cut["units"]])

    def test_base_dir_swaps_only_the_base_side(self):
        root, _, _, _, _ = self.build()
        cfg, _, _ = oc.build_config(root, base_dir="build_non_matching", flags=FLAGS)
        u = {u["name"]: u for u in cfg["units"]}["src/main/game.c"]
        self.assertEqual(u["base_path"], "build_non_matching/src/main/game.c.o")
        self.assertTrue(u["target_path"].startswith("expected/build/"))


if __name__ == "__main__":
    unittest.main()
