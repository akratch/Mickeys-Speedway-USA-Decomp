#!/usr/bin/env python3
"""Synthetic tests for tools/real_names.py: classification and rewriting."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import real_names as rn  # noqa: E402


def tables(**extra):
    t = {
        "overlay_defs": {("ov", 2, 0x40): ["overlay2Helper"],
                         ("ov", 2, 0x900): ["gOverlay2Table"]},
        "overlay_statics": {("ov", 2, 0x910): ["sOverlay2Private"]},
        "resident": {0x80001000: [("mmAlloc", True)],
                     0x8007BEF8: [("D_8007BEF8", False)]},
        "module_info": {2: {"text_size": 0x800, "prefix": 0xA00}},
        "definers": {"overlay2Helper": {"overlays/o002/b"}},
    }
    t.update(extra)
    return t


class ClassifyTests(unittest.TestCase):
    def classify(self, sites, ov=2):
        return rn.classify_groups({(ov, "P"): sites}, tables())[0]

    def test_resident_function_and_data(self):
        p = self.classify([(("res", 0x80001000), "overlays/o002/a", "SYMBOL")])
        self.assertEqual(("resident-func", "mmAlloc"), (p.cls, p.real))
        p = self.classify([(("res", 0x8007BEF8), "overlays/o002/a", "SYMBOL")])
        self.assertEqual(("resident-data", "D_8007BEF8"), (p.cls, p.real))

    def test_overlay_function_and_c_defined_data(self):
        p = self.classify([(("ov", 2, 0x40), "overlays/o002/a", "JUMP")])
        self.assertEqual(("overlay-func", "overlay2Helper"), (p.cls, p.real))
        p = self.classify([(("ov", 2, 0x900), "overlays/o002/a", "LOCAL")])
        self.assertEqual(("overlay-data", "gOverlay2Table"), (p.cls, p.real))

    def test_blob_data_and_bss_are_refused_not_named(self):
        self.assertEqual("blob-data", self.classify([(("ov", 2, 0x980), "s", "LOCAL")]).cls)
        self.assertEqual("blob-bss", self.classify([(("ov", 2, 0xA10), "s", "LOCAL")]).cls)
        self.assertIsNone(self.classify([(("ov", 2, 0xA10), "s", "LOCAL")]).real)

    def test_unnamed_static_disagree_and_no_record(self):
        self.assertEqual("unnamed", self.classify([(("res", 0x80001004), "s", "SYMBOL")]).cls)
        self.assertEqual("unnamed", self.classify([(("ov", 2, 0x44), "s", "JUMP")]).cls)
        self.assertEqual("static", self.classify([(("ov", 2, 0x910), "s", "LOCAL")]).cls)
        self.assertEqual("disagree", self.classify(
            [(("ov", 2, 0x40), "s", "JUMP"), (("res", 0x80001000), "s", "SYMBOL")]).cls)
        self.assertEqual("no-record", self.classify([(None, "s", None)]).cls)

    def test_symbol_site_in_the_defining_object_is_self_export(self):
        p = self.classify([(("ov", 2, 0x40), "overlays/o002/b", "SYMBOL")])
        self.assertEqual("self-export", p.cls)
        # A JUMP there is an ordinary direct call.
        p = self.classify([(("ov", 2, 0x40), "overlays/o002/b", "JUMP")])
        self.assertEqual("overlay-func", p.cls)

    def test_field_of_an_export_is_refused(self):
        p = self.classify([(("res", 0x8007BEF8), "overlays/o002/a", "SYMBOL-offset")])
        self.assertEqual("export-offset", p.cls)

    def test_kinds(self):
        self.assertEqual("Reloc", rn.placeholder_kind("overlay1SqrtReloc"))
        self.assertEqual("D_short", rn.placeholder_kind("D_1D68"))
        self.assertEqual("ext", rn.placeholder_kind("ext_o7_ccc"))
        self.assertEqual("D_8", rn.placeholder_kind("D_80000040"))
        self.assertEqual("friendly", rn.placeholder_kind("gO1PlayerCount"))


class RewriteTextTests(unittest.TestCase):
    def test_identifiers_comments_and_strings(self):
        src = ('extern s32 P(s32);  /* P is a placeholder */\n'
               'const char *s = "P";\n'
               's32 f(void) { return P(1) + PX(2); }\n')
        out = rn.rewrite_text(src, {"P": "realFn"})
        self.assertIn("extern s32 realFn(s32);", out)
        self.assertIn("/* realFn is a placeholder */", out)
        self.assertIn('"P"', out)
        self.assertIn("realFn(1) + PX(2)", out)

    def test_short_data_name_is_kept_in_prose(self):
        out = rn.rewrite_text("/* D_1D68 */ extern s32 D_1D68;\n", {"D_1D68": "gReal"})
        self.assertEqual("/* D_1D68 */ extern s32 gReal;\n", out)

    def test_two_placeholders_for_one_symbol_collapse_their_declarations(self):
        src = "extern void *A(s32);\nextern void *B(s32);\nextern s32 other;\nextern s32 other;\n"
        out = rn.rewrite_text(src, {"A": "real", "B": "real"})
        self.assertEqual("extern void *real(s32);\nextern s32 other;\nextern s32 other;\n", out)


MK = """\
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/a.c.o: POSTPROCESS = \\
\t$(OBJCOPY) \\
\t\t--redefine-sym P=PReloc \\
\t\t--redefine-sym realB=realB_o001Reloc \\
\t\t--redefine-sym Q=QReloc $@ && \\
\t$(OBJCOPY) --redefine-sym S=S_target $@ && \\
\t$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/b.c.o: POSTPROCESS = \\
\t$(OBJCOPY) --redefine-sym P=PReloc $@
"""


class RewriteMkTests(unittest.TestCase):
    def test_respells_only_the_module_and_drops_collisions(self):
        out = rn.rewrite_mk(MK, 1, {"P": "realA", "Q": "realB", "S": "realB"})
        self.assertIn("--redefine-sym realA=PReloc \\\n", out)
        # realB is already renamed by the rule: the respelled entries go.
        self.assertNotIn("QReloc", out)
        self.assertNotIn("S_target", out)
        self.assertIn("--redefine-sym realB=realB_o001Reloc \\\n\t\t$@ && \\\n", out)
        self.assertIn("trim_elf_section.py", out)
        # Overlay 2's rule is another module's business.
        self.assertIn("o002/b.c.o: POSTPROCESS = \\\n\t$(OBJCOPY) --redefine-sym P=PReloc $@", out)


class SplitRenameTests(unittest.TestCase):
    def test_wrapped_rename_is_joined_respelled_and_collisions_dropped(self):
        mk = ("$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/a.c.o: POSTPROCESS = \\\n"
              "\t$(OBJCOPY) --redefine-sym \\\n\t\tP=func_x $@ && \\\n"
              "\t$(OBJCOPY) --redefine-sym \\\n\t\tQ=func_x $@ && \\\n"
              "\t$(OBJCOPY) --redefine-sym \\\n\t\tKEEP=func_y $@ && \\\n"
              "\t$(HOST_PYTHON) trim $@\n")
        out = rn.rewrite_mk(mk, 8, {"P": "realP", "Q": "realP"})
        self.assertIn("\t$(OBJCOPY) --redefine-sym realP=func_x $@ && \\\n", out)
        self.assertNotIn("Q=", out)
        self.assertIn("--redefine-sym \\\n\t\tKEEP=func_y", out)


class PlanModuleTests(unittest.TestCase):
    def test_shared_and_normalization_refusals_and_exclusive_headers(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "src/overlays/o001").mkdir(parents=True)
            (root / "src/overlays/o009").mkdir(parents=True)
            (root / "include/overlays").mkdir(parents=True)
            (root / "include/overlays/overlay_001.h").write_text("extern s32 Own;\n")
            (root / "include/common.h").write_text("extern s32 Shared;\n")
            (root / "src/overlays/o001/a.c").write_text(
                '#include "overlays/overlay_001.h"\n#include "common.h"\n'
                "s32 f(void) { return Own + Shared + Spec + Fine; }\n")
            (root / "src/overlays/o009/b.c").write_text('#include "common.h"\nextern s32 Fine;\n')
            files = rn.tree_files(root)
            texts = {f: f.read_text() for f in files}
            inc = rn.includers(root, files, texts)
            own = rn.module_files(root, 1, files, texts, inc)
            self.assertIn(root / "include/overlays/overlay_001.h", own)
            self.assertNotIn(root / "include/common.h", own)
            ph = [rn.Placeholder(1, n, 1, "resident-data", "r" + n)
                  for n in ("Own", "Shared", "Spec", "Fine")]
            ph.append(rn.Placeholder(1, "Blob", 1, "blob-bss"))
            norm = root / "config/normalizations"
            norm.mkdir(parents=True)
            (norm / "both.rebind.spec").write_text("0x10:Spec:func_x\n")
            (norm / "dead.rebind.spec").write_text("0x10:Fine:func_x\n")
            mk = ("$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/a.c.o: POSTPROCESS = \\\n"
                  "\t$(HOST_PYTHON) rebind.py $@ .text @config/normalizations/both.rebind.spec\n"
                  "$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/b.c.o: POSTPROCESS = \\\n"
                  "\t$(HOST_PYTHON) rebind.py $@ .text @config/normalizations/both.rebind.spec\n")
            mapping, refusals = rn.plan_module(root, 1, ph, files, texts, inc, mk_texts=[mk])
            # A spec no rule names takes no part in the build.
            self.assertEqual({"Own": "rOwn", "Fine": "rFine"}, mapping)
            self.assertEqual({"Shared": "shared", "Spec": "normalization", "Blob": "blob-bss"},
                             {n: why.split(" ")[0] for n, why in refusals})

    def test_rule_on_a_one_object_variable(self):
        mk = ("O28_OBJ := \\\n\t$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/a.c.o\n"
              "$(O28_OBJ): POSTPROCESS = \\\n"
              "\t$(HOST_PYTHON) rebind.py $@ .text 0x10:P:func_x\n")
        self.assertEqual(1, len(rn.module_rules([mk], 28)))
        self.assertIn("0x10:rP:func_x", rn.rewrite_mk(mk, 28, {"P": "rP"}))

    def test_recipe_spellings_refuse_only_on_a_clash(self):
        mk = ("$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/a.c.o: POSTPROCESS = \\\n"
              "\t$(HOST_PYTHON) rebind.py $@ .text 0x10:P:func_x 0x20:Q:rQ \\\n"
              "\t&& $(OBJCOPY) --redefine-sym rN=N $@\n")
        rules = rn.module_rules([mk], 1)
        self.assertEqual(1, len(rules))
        self.assertEqual(1, rn.recipe_uses(rules[0][0], "P"))
        self.assertEqual(0, rn.recipe_uses(rules[0][0], "N"))
        out = rn.rewrite_mk(mk, 1, {"P": "rP", "N": "rN2"})
        self.assertIn("0x10:rP:func_x", out)
        self.assertIn("--redefine-sym rN=N $@", out)
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "src/overlays/o001").mkdir(parents=True)
            (root / "src/overlays/o001/a.c").write_text("s32 f(void) { return P() + Q() + N(); }\n")
            files = rn.tree_files(root)
            texts = {f: f.read_text() for f in files}
            inc = rn.includers(root, files, texts)
            ph = [rn.Placeholder(1, n, 1, "resident-func", "r" + n) for n in ("P", "Q", "N")]
            mapping, refusals = rn.plan_module(root, 1, ph, files, texts, inc, mk_texts=[mk])
            self.assertEqual({"P": "rP", "N": "rN"}, mapping)
            self.assertEqual([("Q", "normalization (recipe)")], refusals)


if __name__ == "__main__":
    unittest.main()
