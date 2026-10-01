#!/usr/bin/env python3
"""Regression coverage for safe omission of unowned objcopy section aliases."""

from __future__ import annotations

import importlib.util
import re
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SPEC = importlib.util.spec_from_file_location("permute_batch_recipe_test", ROOT / "tools/permute_batch.py")
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class FakeElf:
    def __init__(self, symbols, relocs, text):
        self._symbols = symbols
        self._relocs = relocs
        self._text = text
        self.names = [".text", ".rodata"]

    def section(self, name):
        if name == ".text":
            return 0, (0, 1, 0x4, 0, 0, len(self._text), 0, 0, 4, 0)
        if name == ".rodata":
            if any(symbol[0] == ".rodata" for symbol in self._symbols):
                return 1, (0, 1, 0, 0, 0, 32, 0, 0, 4, 0)
            return None, None
        return None, None

    def symbols(self):
        return self._symbols

    def relocations(self, target=r"\.text"):
        return [row for row in self._relocs if re.fullmatch(target, row[0])]

    def section_bytes(self, name):
        return self._text if name == ".text" else b""


def fake_elf(kind, owned_dependency=False, section_addend=False,
             data_dependency=False):
    target = ("func_80006534", 0, 16, 0x12, 0)
    other = ("other_function", 16, 16, 0x12, 0)
    aliases = [
        ("objectsSizeDefaultBranch", 0x6500, 0, 0x00, 0),
        ("objectsInitDefaultBranch", 0x6718, 0, 0x00, 0),
        ("objectsControlDefaultBranch", 0x6C00, 0, 0x00, 0),
        ("objectsSwitchTablesBase", 0, 0, 0x11, 1),
    ]
    externals = [(f"callee_{i}", 0, 0, 0x10, 0) for i in range(7)]
    symbols = [target, *externals, other, *aliases, (".rodata", 0, 0, 0x03, 1)]
    # Seven target call relocs; alias/section references belong to other_function.
    relocs = [(".text", i * 2, 4, i + 1) for i in range(7)]
    for offset, symbol_index in ((17, 9), (18, 10), (19, 11), (20, 12), (21, 13)):
        relocs.append((".text", offset, 5, symbol_index))
    if owned_dependency:
        relocs.append((".text", 3, 5, 12))  # alias-name dependency
    if section_addend:
        relocs.append((".text", 4, 6, 13))  # implicit .rodata section-symbol addend
    text = bytes(range(32 if kind == "full" else 16))
    # The full TU's target occupies exactly its first 16 bytes.
    if kind == "full":
        text = bytes(range(32))
    if kind == "candidate" and owned_dependency:
        pass
    if kind in {"candidate", "target"}:
        symbols = [target, *externals]
        relocs = [(".text", i * 2, 4, i + 1) for i in range(7)]
        if owned_dependency:
            symbols.append(("objectsSwitchTablesBase", 0, 0, 0x10, 0))
            relocs.append((".text", 3, 5, len(symbols) - 1))
        if section_addend:
            symbols.append((".rodata", 0, 0, 0x03, 1))
            relocs.append((".text", 4, 6, len(symbols) - 1))
    if data_dependency and kind in {"candidate", "target"}:
        symbols.append(("objectsSwitchTablesBase", 0, 0, 0x10, 0))
        relocs.append((".data", 3, 5, len(symbols) - 1))  # offset overlaps apparent .text function
    if data_dependency and kind == "full":
        relocs.append((".data", 17, 5, 12))  # outside target .text, inside another apparent range
    return FakeElf(symbols, relocs, text)


def test_unowned_metadata_variant_and_preserved_recipe():
    original = ("tools/binutils/mips64-elf-objcopy --add-symbol "
                "objectsSizeDefaultBranch=.text:0x6500,local --add-symbol "
                "objectsInitDefaultBranch=.text:0x6718,local --add-symbol "
                "objectsControlDefaultBranch=.text:0x6C00,local --add-symbol "
                "objectsSwitchTablesBase=.rodata:0,global build/src/main/objects.c.o",)
    recipe = module.BuildRecipe((), original, (), True)
    with tempfile.TemporaryDirectory(prefix="recipe-metadata-", dir=ROOT) as tmp:
        root = Path(tmp)
        scratch = root / "scratch"
        scratch.mkdir()
        (scratch / "base.o").write_bytes(b"candidate")
        (scratch / "target.o").write_bytes(b"target")
        full = root / "full.o"
        full.write_bytes(b"full")
        source = ROOT / "src/main/objects.c"
        old_elf = module.reloc_surface.Elf
        fixtures = {"base.o": fake_elf("candidate"), "target.o": fake_elf("target"),
                    "full.o": fake_elf("full")}
        module.reloc_surface.Elf = lambda path: fixtures[Path(path).name]
        try:
            effective, proof = module._section_metadata_variant(
                scratch, recipe, source, "func_80006534", full)
        finally:
            module.reloc_surface.Elf = old_elf
        assert recipe.objcopy_steps == original
        assert proof["status"] == "unowned-section-metadata-omitted"
        assert proof["full_tu_function"] == {
            "name": "func_80006534", "start": 0, "size": 16, "end": 16}
        assert len(proof["candidate"]["relocations"]) == 7
        assert len(proof["target"]["relocations"]) == 7
        assert proof["candidate"]["function_bytes_sha256"] == proof["configured_c_baseline_function_bytes_sha256"]
        assert proof["omitted_additions"][0]["symbol"] == "objectsSwitchTablesBase"
        assert {row["symbol"] for row in proof["kept_additions"]} == {
            "objectsSizeDefaultBranch", "objectsInitDefaultBranch", "objectsControlDefaultBranch"}
        assert ".rodata" not in effective[0]
        assert ".text:0x6500" in effective[0]


def test_owned_alias_and_section_symbol_dependencies_refuse():
    original = ("tools/binutils/mips64-elf-objcopy --add-symbol "
                "objectsSizeDefaultBranch=.text:0x6500,local --add-symbol "
                "objectsInitDefaultBranch=.text:0x6718,local --add-symbol "
                "objectsControlDefaultBranch=.text:0x6C00,local --add-symbol "
                "objectsSwitchTablesBase=.rodata:0,global build/src/main/objects.c.o",)
    recipe = module.BuildRecipe((), original, (), True)
    for owned_alias, section_addend in ((True, False), (False, True)):
        with tempfile.TemporaryDirectory(prefix="recipe-owned-ref-", dir=ROOT) as tmp:
            root = Path(tmp)
            scratch = root / "scratch"
            scratch.mkdir()
            (scratch / "base.o").write_bytes(b"candidate")
            (scratch / "target.o").write_bytes(b"target")
            full = root / "full.o"
            full.write_bytes(b"full")
            source = ROOT / "src/main/objects.c"
            fixtures = {"base.o": fake_elf("candidate"),
                        "target.o": fake_elf("target", owned_dependency=owned_alias,
                                             section_addend=section_addend),
                        "full.o": fake_elf("full")}
            old_elf = module.reloc_surface.Elf
            module.reloc_surface.Elf = lambda path: fixtures[Path(path).name]
            try:
                try:
                    module._section_metadata_variant(
                        scratch, recipe, source, "func_80006534", full,
                        c_baseline_object=full)
                except RuntimeError as error:
                    assert "dependency" in str(error) or "target has .rodata" in str(error)
                else:
                    raise AssertionError("owned .rodata dependency was omitted")
            finally:
                module.reloc_surface.Elf = old_elf


def test_nontext_relocation_sources_refuse():
    original = ("tools/binutils/mips64-elf-objcopy --add-symbol "
                "objectsSizeDefaultBranch=.text:0x6500,local --add-symbol "
                "objectsInitDefaultBranch=.text:0x6718,local --add-symbol "
                "objectsControlDefaultBranch=.text:0x6C00,local --add-symbol "
                "objectsSwitchTablesBase=.rodata:0,global build/src/main/objects.c.o",)
    recipe = module.BuildRecipe((), original, (), True)
    cases = ((True, False, False), (False, True, False), (False, False, True))
    for candidate_data, target_data, full_data in cases:
        with tempfile.TemporaryDirectory(prefix="recipe-section-ref-", dir=ROOT) as tmp:
            root = Path(tmp)
            scratch = root / "scratch"
            scratch.mkdir()
            (scratch / "base.o").write_bytes(b"candidate")
            (scratch / "target.o").write_bytes(b"target")
            full = root / "full.o"
            full.write_bytes(b"full")
            source = ROOT / "src/main/objects.c"
            fixtures = {
                "base.o": fake_elf("candidate", data_dependency=candidate_data),
                "target.o": fake_elf("target", data_dependency=target_data),
                "full.o": fake_elf("full", data_dependency=full_data),
            }
            old_elf = module.reloc_surface.Elf
            module.reloc_surface.Elf = lambda path: fixtures[Path(path).name]
            try:
                try:
                    module._section_metadata_variant(
                        scratch, recipe, source, "func_80006534", full,
                        c_baseline_object=full)
                except RuntimeError as error:
                    assert "section" in str(error) or "relocations outside .text" in str(error)
                else:
                    raise AssertionError("non-.text relocation source was accepted")
            finally:
                module.reloc_surface.Elf = old_elf


def test_unrelated_recipe_stays_unchanged_without_section_parsing():
    with tempfile.TemporaryDirectory(prefix="recipe-unrelated-", dir=ROOT) as tmp:
        root = Path(tmp)
        scratch = root / "scratch"
        scratch.mkdir()
        compile_script = scratch / "compile.sh"
        compile_script.write_text("#!/bin/sh\n")
        out = root / "out"
        out.mkdir()
        step = ("tools/binutils/mips64-elf-objcopy --redefine-sym old=new "
                "build/src/main/objects.c.o")
        recipe = module.BuildRecipe((), (step,), (), True)
        module.replicate_objcopy(scratch, recipe, ROOT / "src/main/objects.c", out)
        variant = __import__("json").loads((out / "recipe-variant.json").read_text())
        assert variant["status"] == "unchanged"
        assert variant["original_steps"] == variant["effective_steps"] == [step]


def main() -> int:
    test_unowned_metadata_variant_and_preserved_recipe()
    test_owned_alias_and_section_symbol_dependencies_refuse()
    test_nontext_relocation_sources_refuse()
    test_unrelated_recipe_stays_unchanged_without_section_parsing()
    print("permute recipe metadata tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
