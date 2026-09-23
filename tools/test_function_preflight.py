#!/usr/bin/env python3
"""Focused tests for function_preflight identity and fail-closed routing."""

from __future__ import annotations

import contextlib
import dataclasses
import hashlib
import io
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import function_preflight as fp  # noqa: E402
import crew  # noqa: E402


class SymbolResolutionTests(unittest.TestCase):
    def test_rebuild_reauthenticates_inner_container_before_consumption(self) -> None:
        base = fp.rs.SYNTHETIC_VMA
        symbols = [
            ("previous", base, 0x20, 0x12, 1),
            ("friendly", base + 0x20, 0x20, 0x12, 1),
            ("next", base + 0x40, 0x20, 0x12, 1),
        ]
        module = {"rom": {"start": "0x10"}, "text_ownership": [{
            "offset": "0x0", "end_offset": "0x60", "size": "0x60",
            "type": "c", "matched": True, "nonmatching": False,
            "source": "overlays/o004/example",
        }]}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            (root / "build/mickey.us.elf").touch()
            (root / "baseroms").mkdir()
            (root / "baseroms/mickey.us.z64").write_bytes(bytes(0x70))
            elf = mock.Mock()
            resolution = mock.Mock()

            def resolve(_symbol: str) -> object:
                fp._fully_matched_inner_geometry(
                    module, 4, 0x20, "friendly", "func_overlay_004_F0000020_30",
                    "overlays/o004/example", root,
                )
                return resolution

            for mode in ([], ["--resolve-rom"]):
                for corruption in ("container", "section", "zero alias", "boundary"):
                    with self.subTest(mode=mode, corruption=corruption):
                        elf.section.return_value = (1, (0, 1, 6, base, 0, 0x60))
                        elf.section_bytes.return_value = bytes(0x60)
                        elf.symbols.return_value = symbols

                        def rebuild(*_args: object) -> None:
                            if corruption == "container":
                                elf.section_bytes.return_value = b"\x01" + bytes(0x5F)
                            elif corruption == "section":
                                elf.symbols.return_value = [
                                    symbols[0], ("friendly", base + 0x20, 0x20, 0x12, 2), symbols[2],
                                ]
                            elif corruption == "zero alias":
                                elf.symbols.return_value = symbols + [
                                    ("func_overlay_004_F0000020_30", base + 0x24, 0, 0x12, 1),
                                ]
                            else:
                                elf.symbols.return_value = [
                                    symbols[0], symbols[1], ("next", base + 0x44, 0x1C, 0x12, 1),
                                ]

                        with mock.patch.object(fp.rs, "Elf", return_value=elf), mock.patch.object(
                            fp, "resolve", side_effect=resolve
                        ) as resolve_call, mock.patch.object(
                            fp, "_build", side_effect=rebuild
                        ), mock.patch.object(
                            fp, "_build_linked_boundary", side_effect=rebuild
                        ), mock.patch.object(fp, "_require_fresh_linked_boundary"), mock.patch.object(
                            fp, "require_fresh_evidence"
                        ), mock.patch.object(fp, "collect") as collect, mock.patch.object(
                            fp, "_linked_boundary"
                        ) as boundary, contextlib.redirect_stderr(io.StringIO()):
                            with self.assertRaises(SystemExit) as error:
                                fp.main(["friendly", *mode])
                        self.assertEqual(error.exception.code, 2)
                        self.assertEqual(resolve_call.call_count, 2)
                        collect.assert_not_called()
                        boundary.assert_not_called()

    def test_fully_matched_inner_function_requires_complete_container_evidence(self) -> None:
        base = fp.rs.SYNTHETIC_VMA
        ordinary_symbols = [
            ("previous", base, 0x20, 0x12, 1),
            ("friendly", base + 0x20, 0x20, 0x12, 1),
            ("next", base + 0x40, 0x20, 0x12, 1),
        ]
        owner = {"offset": "0x0", "end_offset": "0x60", "size": "0x60",
                 "type": "c", "matched": True, "nonmatching": False,
                 "source": "overlays/o004/example"}
        module = {"overlay": 4, "rom": {"start": "0x10"},
                  "text_ownership": [owner]}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            (root / "build/mickey.us.elf").touch()
            (root / "baseroms").mkdir()
            (root / "baseroms/mickey.us.z64").write_bytes(bytes(0x70))
            source = root / "src/overlays/o004/example.c"
            source.parent.mkdir(parents=True)
            source.write_text("void friendly(void) {}\n")
            alias = root / "aliases.txt"
            generated = "func_overlay_004_F0000020_30"
            alias.write_text(f"{generated} = friendly;\n")
            atlas = root / "atlas.json"
            atlas.write_text(json.dumps({"modules": [module]}))
            elf = mock.Mock()
            elf.section.return_value = (1, (0, 1, 6, base, 0, 0x60))
            elf.section_bytes.return_value = bytes(0x60)
            elf.symbols.return_value = ordinary_symbols
            with mock.patch.object(fp.rs, "Elf", return_value=elf):
                resolution = fp.resolve("friendly", root=root, alias_path=alias,
                                        atlas_path=atlas, symbol_path=root / "unused")
                self.assertEqual((base + 0x20, 0x20),
                                 (resolution.expected_value, resolution.expected_size))
                self.assertIn("whole-container ROM", resolution.identity_evidence)
                elf.symbols.return_value = ordinary_symbols + [(generated, base + 0x20, 0, 0x12, 1)]
                alias_resolution = fp.resolve("friendly", root=root, alias_path=alias,
                                              atlas_path=atlas, symbol_path=root / "unused")
                self.assertEqual(resolution.expected_size, alias_resolution.expected_size)

                cases = {
                    "missing": [ordinary_symbols[0], ordinary_symbols[2]],
                    "alias conflict": ordinary_symbols + [(generated, base + 0x20, 0x24, 0x12, 1)],
                    "zero alias wrong start": ordinary_symbols + [(generated, base + 0x24, 0, 0x12, 1)],
                    "zero candidate": [ordinary_symbols[0], ("friendly", base + 0x20, 0, 0x12, 1), ordinary_symbols[2]],
                    "wrong start": [ordinary_symbols[0], ("friendly", base + 0x24, 0x1C, 0x12, 1), ordinary_symbols[2]],
                    "overflow": [ordinary_symbols[0], ("friendly", base + 0x20, 0x44, 0x12, 1)],
                    "gap": [ordinary_symbols[0], ("friendly", base + 0x20, 0x1C, 0x12, 1), ordinary_symbols[2]],
                    "overlap": ordinary_symbols + [("inside", base + 0x30, 4, 0x12, 1)],
                    "data": ordinary_symbols + [("data", base + 0x30, 4, 0x11, 1)],
                    "data boundary": [ordinary_symbols[0], ordinary_symbols[1], ("data", base + 0x40, 0x20, 0x11, 1)],
                    "wrong section": [ordinary_symbols[0], ("friendly", base + 0x20, 0x20, 0x12, 2), ordinary_symbols[2]],
                    "not function": [ordinary_symbols[0], ("friendly", base + 0x20, 0x20, 0x11, 1), ordinary_symbols[2]],
                }
                for label, symbols in cases.items():
                    with self.subTest(label=label):
                        elf.symbols.return_value = symbols
                        with self.assertRaises(fp.PreflightError):
                            fp.resolve("friendly", root=root, alias_path=alias,
                                       atlas_path=atlas, symbol_path=root / "unused")
                elf.symbols.return_value = ordinary_symbols
                elf.section_bytes.return_value = b"\x01" + bytes(0x5F)
                with self.assertRaisesRegex(fp.PreflightError, "container is not linked-ROM exact"):
                    fp.resolve("friendly", root=root, alias_path=alias,
                               atlas_path=atlas, symbol_path=root / "unused")
                elf.section_bytes.return_value = bytes(0x60)
                module["text_ownership"].append(dict(owner))
                atlas.write_text(json.dumps({"modules": [module]}))
                with self.assertRaisesRegex(fp.PreflightError, "unambiguous atlas owner"):
                    fp.resolve("friendly", root=root, alias_path=alias,
                               atlas_path=atlas, symbol_path=root / "unused")

    def fixture(self, root: Path, alias_lines: str) -> Path:
        alias = root / "overlay_undefined_syms.us.txt"
        alias.write_text(alias_lines, encoding="utf-8")
        source = root / "src/overlays/o016/example.c"
        source.parent.mkdir(parents=True)
        source.write_text(
            "#ifdef NON_MATCHING\n"
            "void friendly(s32 value) { (void)value; }\n"
            "#else\n"
            '#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o016/example/'
            'func_overlay_016_F0000000_1.s")\n'
            "#endif\n",
            encoding="utf-8",
        )
        asm = root / "asm/nonmatchings/overlays/o016/example/func_overlay_016_F0000000_1.s"
        asm.parent.mkdir(parents=True)
        asm.write_text("glabel func_overlay_016_F0000000_1\n", encoding="utf-8")
        return alias

    def test_friendly_and_generated_names_resolve_to_same_nonmatching_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = self.fixture(
                root,
                "func_overlay_016_F0000000_1 = friendly;\n",
            )
            friendly = fp.resolve("friendly", root=root, alias_path=alias)
            generated = fp.resolve(
                "func_overlay_016_F0000000_1", root=root, alias_path=alias
            )

        self.assertEqual(friendly.target_symbol, generated.target_symbol)
        self.assertEqual(friendly.candidate_symbol, "friendly")
        self.assertEqual(friendly.candidate_build_dir, "build_non_matching")
        self.assertEqual(
            friendly.translation_unit, "overlays/o016/example"
        )

    def test_ambiguous_friendly_alias_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F0000000_1 = friendly;\n"
                "func_overlay_017_F0000000_2 = friendly;\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(fp.PreflightError, "maps to 2"):
                fp._resolve_names("friendly", alias)

    def test_unresolved_runtime_identity_fails_closed(self) -> None:
        with self.assertRaisesRegex(fp.PreflightError, "ambiguous"):
            fp._identity_text(None)

    def test_candidate_redefine_aliases_preserve_sources(self) -> None:
        command = (
            "tools/binutils/mips64-elf-objcopy "
            "--redefine-sym func_80005750=func_80005750_o001Reloc "
            "--redefine-sym=mathRnd=mathRnd_o001Reloc "
            "--redefine-sym first=sharedProxy && "
            "tools/binutils/mips64-elf-objcopy --redefine-sym second=sharedProxy "
            "build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            aliases = fp._candidate_redefine_aliases(
                fp.REPO / "build/src/example.c.o"
            )
        self.assertEqual(
            aliases,
            {
                "func_80005750_o001Reloc": "func_80005750",
                "mathRnd_o001Reloc": "mathRnd",
            },
        )

    def test_candidate_redefine_aliases_collapse_transitive_chain(self) -> None:
        command = (
            "tools/binutils/mips64-elf-objcopy --redefine-sym original=proxy "
            "build/src/example.c.o && "
            "tools/binutils/mips64-elf-objcopy --redefine-sym proxy=final "
            "build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            aliases = fp._candidate_redefine_aliases(
                fp.REPO / "build/src/example.c.o"
            )
        self.assertEqual(
            {"proxy": "original", "final": "original"}, aliases
        )

    def test_candidate_redefine_alias_cycle_fails_closed(self) -> None:
        command = (
            "$(OBJCOPY) --redefine-sym first=second build/src/example.c.o && "
            "$(OBJCOPY) --redefine-sym second=first build/src/example.c.o"
        )
        with mock.patch.object(
            fp.pa, "run_make_database", return_value="database"
        ), mock.patch.object(
            fp.pa, "postprocess_commands",
            return_value={"build/src/example.c.o": command},
        ):
            with self.assertRaisesRegex(fp.PreflightError, "contain a cycle"):
                fp._candidate_redefine_aliases(
                    fp.REPO / "build/src/example.c.o"
                )

    def test_promoted_overlay_resolves_from_exact_atlas_owner_without_asm(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F00001E0_1873678 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o016/friendly.c"
            source.parent.mkdir(parents=True)
            source.write_text("void friendly(void) {}\n", encoding="utf-8")
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 16,
                                "text_ownership": [
                                    {
                                        "offset": "0x1E0",
                                        "end_offset": "0x220",
                                        "size": "0x40",
                                        "type": "c",
                                        "source": "overlays/o016/friendly",
                                        "matched": True,
                                        "nonmatching": False,
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            symbols = root / "symbols.txt"
            symbols.write_text("", encoding="utf-8")

            friendly = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=symbols,
            )
            generated = fp.resolve(
                "func_overlay_016_F00001E0_1873678",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=symbols,
            )

        self.assertEqual("post_promotion", friendly.resolution_mode)
        self.assertIsNone(friendly.target_asm)
        self.assertEqual("build", friendly.candidate_build_dir)
        self.assertEqual(0xF00001E0, friendly.expected_value)
        self.assertEqual(0x40, friendly.expected_size)
        self.assertEqual(friendly.target_symbol, generated.target_symbol)
        self.assertEqual(friendly.candidate_symbol, generated.candidate_symbol)

    def test_promoted_mixed_tu_overlay_uses_function_sized_exact_range(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_025_F0000000_1879C88 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o025/overlay_025.c"
            source.parent.mkdir(parents=True)
            source.write_text(
                "void friendly(void) {}\n"
                "#ifdef NON_MATCHING\n"
                "void another(void) {}\n"
                "#else\n"
                '#pragma GLOBAL_ASM("asm/nonmatchings/x/another.s")\n'
                "#endif\n",
                encoding="utf-8",
            )
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 25,
                                "text_ownership": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x608",
                                        "size": "0x608",
                                        "type": "c",
                                        "source": "overlays/o025/overlay_025",
                                        "matched": True,
                                        "nonmatching": True,
                                    }
                                ],
                                "mixed_tu_exact_c_ranges": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x17C",
                                        "size": "0x17C",
                                        "label": "friendly",
                                        "source": "overlays/o025/overlay_025",
                                        "evidence": "linked exact",
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=root / "unused-symbols.txt",
            )

        self.assertEqual(0x17C, resolution.expected_size)
        self.assertIn("mixed_tu_exact_c_ranges", resolution.identity_evidence)

    def test_promoted_exact_tu_uses_next_export_as_function_boundary(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_049_F0000000_1896410 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o049/overlay_049.c"
            source.parent.mkdir(parents=True)
            source.write_text("void friendly(void) {}\n", encoding="utf-8")
            atlas = root / "atlas.json"
            atlas.write_text(
                json.dumps(
                    {
                        "modules": [
                            {
                                "overlay": 49,
                                "text_ownership": [
                                    {
                                        "offset": "0x0",
                                        "end_offset": "0x374",
                                        "size": "0x374",
                                        "type": "c",
                                        "source": "overlays/o049/overlay_049",
                                        "matched": True,
                                        "nonmatching": False,
                                    }
                                ],
                                "exports": [
                                    {"rom_table_index": 1415, "offset": "0x0"},
                                    {"rom_table_index": 1433, "offset": "0x1F4"},
                                    {"rom_table_index": 1397, "offset": "0x354"},
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "friendly",
                root=root,
                alias_path=alias,
                atlas_path=atlas,
                symbol_path=root / "unused-symbols.txt",
            )

        self.assertEqual(0x1F4, resolution.expected_size)
        self.assertIn("export boundary", resolution.identity_evidence)

    def test_promoted_resident_requires_matched_c_symbol_geometry(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text("", encoding="utf-8")
            source = root / "src/main/memory.c"
            source.parent.mkdir(parents=True)
            source.write_text("void func_8002BB40(void) {}\n", encoding="utf-8")
            symbols = root / "symbols.txt"
            symbols.write_text(
                "func_8002BB40 = 0x8002BB40; // type:func size:0x120 matched C\n",
                encoding="utf-8",
            )

            resolution = fp.resolve(
                "func_8002BB40",
                root=root,
                alias_path=alias,
                atlas_path=root / "unused-atlas.json",
                symbol_path=symbols,
            )

        self.assertEqual("post_promotion", resolution.resolution_mode)
        self.assertEqual(0x8002BB40, resolution.expected_value)
        self.assertEqual(0x120, resolution.expected_size)

    def test_missing_fallback_does_not_promote_guarded_nonmatching_source(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text(
                "func_overlay_016_F00001E0_1873678 = friendly;\n",
                encoding="utf-8",
            )
            source = root / "src/overlays/o016/friendly.c"
            source.parent.mkdir(parents=True)
            source.write_text(
                "#ifdef NON_MATCHING\n"
                "void friendly(void) {}\n"
                "#else\n"
                '#pragma GLOBAL_ASM("asm/nonmatchings/x/'
                'func_overlay_016_F00001E0_1873678.s")\n'
                "#endif\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "not one unconditional"):
                fp.resolve(
                    "friendly",
                    root=root,
                    alias_path=alias,
                    atlas_path=root / "unused-atlas.json",
                    symbol_path=root / "unused-symbols.txt",
                )

    def test_untracked_ordinary_c_is_not_assumed_promoted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            alias = root / "aliases.txt"
            alias.write_text("", encoding="utf-8")
            source = root / "src/main/example.c"
            source.parent.mkdir(parents=True)
            source.write_text("void example(void) {}\n", encoding="utf-8")
            symbols = root / "symbols.txt"
            symbols.write_text(
                "example = 0x80001000; // type:func size:0x20 tier-D\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "matched-C"):
                fp.resolve(
                    "example",
                    root=root,
                    alias_path=alias,
                    atlas_path=root / "unused-atlas.json",
                    symbol_path=symbols,
                )


class GeometryAndWorkbenchTests(unittest.TestCase):
    def test_func_80010b4c_metadata_agrees_with_preflight_boundary(self) -> None:
        evidence = fp._optional_size_annotation("func_80010B4C", 0xA98)

        self.assertEqual(
            "symbol-size-annotation+preflight-owned-boundary", evidence
        )

    def test_func_8001291c_metadata_agrees_with_preflight_boundary(self) -> None:
        evidence = fp._optional_size_annotation("func_8001291C", 0x890)

        self.assertEqual(
            "symbol-size-annotation+preflight-owned-boundary", evidence
        )

    def test_func_8001398c_metadata_agrees_with_preflight_boundary(self) -> None:
        evidence = fp._optional_size_annotation("func_8001398C", 0x528)

        self.assertEqual(
            "symbol-size-annotation+preflight-owned-boundary", evidence
        )

    def test_equal_alias_geometries_are_one_unambiguous_range(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    ("friendly", 0xF0000010, 0x20, 0, 1),
                    ("generated", 0xF0000010, 0x20, 0, 1),
                ]

        name, value, size, section = fp._symbol_geometry(
            FakeElf(), ("friendly", "generated")
        )
        self.assertEqual((name, value, size, section), ("friendly", 0xF0000010, 0x20, ".text"))

    def test_preflight_boundary_serves_when_size_annotation_is_absent(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            symbols = Path(directory) / "symbols.txt"
            symbols.write_text(
                "friendly = 0x80001000; // type:func tier-D\n",
                encoding="utf-8",
            )

            evidence = fp._optional_size_annotation("friendly", 0x20, symbols)

        self.assertEqual("preflight-owned-boundary", evidence)

    def test_size_annotation_must_agree_with_preflight_boundary(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            symbols = Path(directory) / "symbols.txt"
            symbols.write_text(
                "friendly = 0x80001000; // type:func size:0x24 tier-D\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "disagrees"):
                fp._optional_size_annotation("friendly", 0x20, symbols)

    def test_duplicate_size_rows_fail_closed_even_when_values_agree(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            symbols = Path(directory) / "symbols.txt"
            symbols.write_text(
                "friendly = 0x80001000; // type:func size:0x20 tier-D\n"
                "friendly = 0x80001000; // type:func size:0x20 tier-D\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(fp.PreflightError, "ambiguous"):
                fp._optional_size_annotation("friendly", 0x20, symbols)


class GeometryAndWorkbenchSummaryTests(unittest.TestCase):
    @staticmethod
    def proof_context() -> dict[str, object]:
        digest = "a" * 64
        return {
            "base": "b" * 40,
            "branch": "lane/test",
            "owner": "src/example.c",
            "manifest_sha256": "c" * 64,
            "created_unix_ns": 1,
            "artifacts": {
                "source": {
                    "path": "src/example.c",
                    "sha256": digest,
                    "size": 1,
                },
                "candidate_object": {
                    "path": "build/src/example.c.o",
                    "sha256": digest,
                    "size": 1,
                },
                "target_object": {
                    "path": "build/wb/generated.target.o",
                    "sha256": digest,
                    "size": 1,
                },
            },
        }

    def inputs(
        self, root: Path, *, exact: bool = False, build_claim: str | None = None
    ) -> tuple[Path, Path]:
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "exact": exact,
            "accepted": exact,
            "acceptance_basis": "function-exact" if exact else "mismatch",
            "verdict": "exact" if exact else "register-mismatch",
            "target_instructions": 8,
            "target_insns": 8,
            "candidate_instructions": 8,
            "insns": 8,
            "instruction_delta": 0,
            "insn_delta": 0,
            "word_mismatches": 0 if exact else 2,
            "words": 0 if exact else 2,
            "raw_word_mismatches": 0 if exact else 3,
            "raw": 0 if exact else 3,
            "normalized_distance": 0 if exact else 2,
            "norm": 0 if exact else 2,
            "opcode_mismatches": 0,
            "opcodes": 0,
            "register_mismatches": 0 if exact else 2,
            "regs": 0 if exact else 2,
            "fp_register_mismatches": 0,
            "fp": 0,
            "aligned_total": 0 if exact else 2,
            "aligned_structural": 0,
            "aligned_schedule": 0,
            "aligned_register": 0 if exact else 2,
            "aligned_constant": 0,
            "first_divergent_row": None if exact else 1,
            "target_frame_size": None if exact else -16,
            "target_frame": None if exact else -16,
            "candidate_frame_size": None if exact else -16,
            "frame": None if exact else -16,
            "relocation_metadata_mismatches": 0,
            "relocation_target_mismatches": 0,
            "diff_sites": [{"instruction": "must not escape"}],
        }
        if build_claim is not None:
            payload["build_provenance"] = {"claim": build_claim}
        raw = root / "raw.json"
        raw.write_text(json.dumps(payload), encoding="utf-8")
        digest = {"sha256": "a" * 64}
        manifest = root / "manifest.json"
        manifest.write_text(
            json.dumps(
                {
                    "schema": "mickey-wb-proof-provenance-v1",
                    "selection": {"classification": "non_matching_candidate"},
                    "exact_claim_allowed": True,
                    "verdict": "c_evidence",
                    "source": digest,
                    "candidate_object": digest,
                    "target_object": digest,
                }
            ),
            encoding="utf-8",
        )
        return raw, manifest

    def summarize(
        self,
        root: Path,
        *,
        exact: bool = False,
        size: int = 32,
        build_claim: str | None = None,
    ):
        raw, manifest = self.inputs(root, exact=exact, build_claim=build_claim)
        return fp.workbench_summary(
            raw,
            manifest,
            requested_symbol="friendly",
            target_symbol="generated",
            candidate_symbol="friendly",
            comparison_mode="asm",
            boundary_evidence="extracted-fallback-symbol",
            boundary_size=size,
        )

    def test_summary_is_code_free_and_keeps_decision_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(Path(directory))

        self.assertEqual("mickey-wb-summary-v1", report["schema"])
        self.assertEqual(6, report["comparison"]["matched_words"])
        self.assertEqual(4, report["comparison"]["first_mismatch_offset"])
        self.assertEqual(16, report["comparison"]["target_frame_bytes"])
        self.assertFalse(report["evidence"]["admissible_exact_comparison"])
        self.assertNotIn("diff_sites", json.dumps(report))
        self.assertNotIn("relocations", report)

    def test_exact_without_build_provenance_block_is_unconstrained(self) -> None:
        # Older raw workbench reports (or ones from a workbench build that
        # predates build_provenance) carry no such block at all. That must
        # not be treated as a denial: behaviour matches what this test
        # expected before build_provenance existed.
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(Path(directory), exact=True)
        self.assertTrue(report["comparison"]["exact"])
        self.assertIsNone(report["provenance"]["build_claim"])
        self.assertTrue(report["evidence"]["admissible_exact_comparison"])

    def test_exact_with_matching_build_claim_stays_admissible(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(
                Path(directory), exact=True, build_claim="match"
            )
        self.assertEqual("match", report["provenance"]["build_claim"])
        self.assertTrue(report["evidence"]["admissible_exact_comparison"])

    def test_exact_with_forced_build_claim_is_not_admissible(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(
                Path(directory), exact=True, build_claim="reachability-proof"
            )
        # exact is unchanged -- it reports what the comparator measured --
        # but the claim disqualifies the result from being admissible.
        self.assertTrue(report["comparison"]["exact"])
        self.assertEqual("reachability-proof", report["provenance"]["build_claim"])
        self.assertFalse(report["evidence"]["admissible_exact_comparison"])

    def test_exact_with_unknown_build_claim_is_not_admissible(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(
                Path(directory), exact=True, build_claim="unknown"
            )
        self.assertFalse(report["evidence"]["admissible_exact_comparison"])

    def test_summary_includes_only_authenticated_relocation_scalars(self) -> None:
        comparison = {
            "candidate_record_count": 21,
            "target_record_count": 21,
            "offset_type_alignment_count": 21,
            "stable_identity_alignment_count": 11,
            "candidate_identity_resolved_count": 21,
            "candidate_identity_unresolved_records": [],
            "offset_type_exact": True,
        }
        with tempfile.TemporaryDirectory() as directory:
            with mock.patch.object(
                fp,
                "_authenticated_summary_relocation_comparison",
                return_value=comparison,
            ), mock.patch.object(
                fp,
                "_canonical_summary_symbols",
                return_value=("generated", "friendly"),
            ), mock.patch.object(
                fp, "_summary_proof_context", return_value=self.proof_context()
            ):
                report = self.summarize(Path(directory))

        self.assertEqual(
            {
                "candidate_relocations": 21,
                "target_relocations": 21,
                "exact_relocation_identities": 11,
            },
            report["relocations"],
        )
        self.assertEqual(
            {
                "candidate_relocations": 21,
                "target_relocations": 21,
                "offset_type_relocations": 21,
                "resolved_candidate_identities": 21,
                "exact_relocation_identities": 11,
                "evidence_mode": "fallback-static",
                "complete": True,
            },
            report["relocation_surfaces"]["fallback_static"],
        )

    def test_promoted_summary_keeps_its_own_linked_surface(self) -> None:
        comparison = {
            "candidate_record_count": 46,
            "target_record_count": 46,
            "offset_type_alignment_count": 46,
            "stable_identity_alignment_count": 34,
            "candidate_identity_resolved_count": 46,
            "candidate_identity_unresolved_records": [],
            "offset_type_exact": True,
        }
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            raw, manifest = self.inputs(root)
            with mock.patch.object(
                fp,
                "_authenticated_summary_relocation_comparison",
                return_value=comparison,
            ), mock.patch.object(
                fp,
                "_canonical_summary_symbols",
                return_value=("generated", "friendly"),
            ), mock.patch.object(
                fp, "_summary_proof_context", return_value=self.proof_context()
            ):
                report = fp.workbench_summary(
                    raw,
                    manifest,
                    requested_symbol="friendly",
                    target_symbol="generated",
                    candidate_symbol="friendly",
                    comparison_mode="rom",
                    boundary_evidence="preflight-owned-boundary",
                    boundary_size=32,
                )

        self.assertEqual(
            "promoted-linked",
            report["relocation_surfaces"]["promoted_linked"]["evidence_mode"],
        )
        self.assertEqual("generated", report["symbol"]["target"])
        self.assertEqual(
            34,
            report["relocation_surfaces"]["promoted_linked"][
                "exact_relocation_identities"
            ],
        )
        self.assertTrue(
            report["relocation_surfaces"]["promoted_linked"]["complete"]
        )

    def test_summary_rejects_inconsistent_relocation_counts(self) -> None:
        comparison = {
            "candidate_record_count": 1,
            "target_record_count": 1,
            "offset_type_alignment_count": 1,
            "stable_identity_alignment_count": 2,
            "candidate_identity_resolved_count": 1,
            "candidate_identity_unresolved_records": [],
        }
        with tempfile.TemporaryDirectory() as directory:
            with mock.patch.object(
                fp,
                "_authenticated_summary_relocation_comparison",
                return_value=comparison,
            ):
                with self.assertRaisesRegex(
                    fp.PreflightError, "exact relocation identities exceed"
                ):
                    self.summarize(Path(directory))

    def test_declared_relocation_artifact_digest_conflict_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            artifact = root / "candidate.o"
            artifact.write_bytes(b"current object")
            manifest = {
                "candidate_object": {
                    "path": "candidate.o",
                    "sha256": hashlib.sha256(b"stale object").hexdigest(),
                    "size": artifact.stat().st_size,
                }
            }

            with self.assertRaisesRegex(fp.PreflightError, "digest no longer agrees"):
                fp._verified_summary_artifact(
                    manifest, "candidate_object", root=root
                )

    def test_incomplete_relocation_artifact_record_is_unavailable(self) -> None:
        manifest = {"candidate_object": {"sha256": "a" * 64}}
        with tempfile.TemporaryDirectory() as directory:
            self.assertIsNone(
                fp._verified_summary_artifact(
                    manifest, "candidate_object", root=Path(directory)
                )
            )

    def test_friendly_requested_name_authenticates_generated_target_object(self) -> None:
        source = fp.REPO / "src/overlays/o001/example.c"
        candidate = fp.REPO / "build_non_matching/src/overlays/o001/example.c.o"
        friendly_target = fp.REPO / "build/wb/overlay1AllocateRecord.target.o"
        artifacts = {
            "source": source,
            "candidate_object": candidate,
            "target_object": friendly_target,
        }
        comparison = {"candidate_record_count": 0}

        with mock.patch.object(
            fp,
            "_verified_summary_artifact",
            side_effect=lambda _manifest, name: artifacts[name],
        ), mock.patch.object(Path, "is_file", return_value=True), mock.patch.object(
            fp.rs, "Elf", return_value=mock.sentinel.target_elf
        ), mock.patch.object(
            fp,
            "_symbol_geometry",
            return_value=("overlay1AllocateRecord", fp.rs.SYNTHETIC_VMA, 0xA0, ".text"),
        ), mock.patch.object(
            fp, "_candidate_redefine_aliases", return_value={}
        ), mock.patch.object(
            fp.rs, "function_surface_comparison", return_value=comparison
        ) as compare:
            result = fp._authenticated_summary_relocation_comparison(
                {
                    "mode": "asm",
                    "symbol": "func_overlay_001_F0000000_0000000",
                    "candidate_symbol": "overlay1AllocateRecord",
                },
                requested_symbol="overlay1AllocateRecord",
                target_symbol="func_overlay_001_F0000000_0000000",
                candidate_symbol="overlay1AllocateRecord",
                comparison_mode="asm",
            )

        self.assertIs(result, comparison)
        self.assertEqual(
            "overlay1AllocateRecord",
            compare.call_args.kwargs["target_symbol"],
        )

    def test_friendly_requested_name_rejects_other_target_artifact(self) -> None:
        artifacts = {
            "source": fp.REPO / "src/overlays/o001/example.c",
            "candidate_object": fp.REPO
            / "build_non_matching/src/overlays/o001/example.c.o",
            "target_object": fp.REPO / "build/wb/otherAlias.target.o",
        }
        with mock.patch.object(
            fp,
            "_verified_summary_artifact",
            side_effect=lambda _manifest, name: artifacts[name],
        ):
            with self.assertRaisesRegex(
                fp.PreflightError, "target object disagrees with requested symbol"
            ):
                fp._authenticated_summary_relocation_comparison(
                    {
                        "mode": "asm",
                        "symbol": "func_overlay_001_F0000000_0000000",
                        "candidate_symbol": "overlay1AllocateRecord",
                    },
                    requested_symbol="overlay1AllocateRecord",
                    target_symbol="func_overlay_001_F0000000_0000000",
                    candidate_symbol="overlay1AllocateRecord",
                    comparison_mode="asm",
                )

    def test_friendly_alias_still_authenticates_generated_target_offset(self) -> None:
        artifacts = {
            "source": fp.REPO / "src/overlays/o001/example.c",
            "candidate_object": fp.REPO
            / "build_non_matching/src/overlays/o001/example.c.o",
            "target_object": fp.REPO / "build/wb/overlay1AllocateRecord.target.o",
        }
        with mock.patch.object(
            fp,
            "_verified_summary_artifact",
            side_effect=lambda _manifest, name: artifacts[name],
        ), mock.patch.object(Path, "is_file", return_value=True), mock.patch.object(
            fp.rs, "Elf", return_value=mock.sentinel.target_elf
        ), mock.patch.object(
            fp,
            "_symbol_geometry",
            return_value=("overlay1AllocateRecord", fp.rs.SYNTHETIC_VMA, 0xA0, ".text"),
        ):
            with self.assertRaisesRegex(
                fp.PreflightError, "generated target symbol offset disagrees"
            ):
                fp._authenticated_summary_relocation_comparison(
                    {
                        "mode": "asm",
                        "symbol": "func_overlay_001_F0000004_0000000",
                        "candidate_symbol": "overlay1AllocateRecord",
                    },
                    requested_symbol="overlay1AllocateRecord",
                    target_symbol="func_overlay_001_F0000004_0000000",
                    candidate_symbol="overlay1AllocateRecord",
                    comparison_mode="asm",
                )

    def test_promoted_friendly_summary_authenticates_linked_surface(self) -> None:
        source = fp.REPO / "src/overlays/o047/example.c"
        candidate = fp.REPO / "build/src/overlays/o047/example.c.o"
        artifacts = {
            "source": source,
            "candidate_object": candidate,
            "target_object": fp.REPO / "build/wb/friendly.target.objdump",
        }
        resolution = fp.Resolution(
            "friendly",
            "func_overlay_047_F00009D0_18917E8",
            "friendly",
            source,
            "overlays/o047/example",
            "build",
            candidate,
            None,
            "promoted",
            resolution_mode="post_promotion",
            expected_value=fp.rs.SYNTHETIC_VMA + 0x9D0,
            expected_size=0x160,
        )
        comparison = {"candidate_record_count": 46}
        with mock.patch.object(
            fp,
            "_verified_summary_artifact",
            side_effect=lambda _manifest, name: artifacts[name],
        ), mock.patch.object(fp, "resolve", return_value=resolution), mock.patch.object(
            Path, "is_file", return_value=True
        ), mock.patch.object(
            fp, "_require_fresh_target"
        ), mock.patch.object(
            fp, "_require_fresh_linked_boundary"
        ), mock.patch.object(
            fp.rs, "Elf", return_value=mock.sentinel.target_elf
        ), mock.patch.object(
            fp,
            "_symbol_geometry",
            return_value=("friendly", fp.rs.SYNTHETIC_VMA + 0x9D0, 0x160, ".text"),
        ), mock.patch.object(
            fp, "_require_tracked_geometry"
        ), mock.patch.object(
            Path, "read_text", return_value='{"modules": []}'
        ), mock.patch.object(
            Path, "read_bytes", return_value=b""
        ), mock.patch.object(
            fp, "_target_context", return_value=({}, [])
        ), mock.patch.object(
            fp, "_candidate_redefine_aliases", return_value={}
        ), mock.patch.object(
            fp.rs, "function_surface_comparison", return_value=comparison
        ) as compare:
            result = fp._authenticated_promoted_summary_relocation_comparison(
                {},
                requested_symbol="friendly",
                target_symbol="func_overlay_047_F00009D0_18917E8",
                candidate_symbol="friendly",
            )

        self.assertIs(result, comparison)
        self.assertEqual("friendly", compare.call_args.kwargs["target_symbol"])

    def test_exact_summary_accepts_null_optional_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report = self.summarize(Path(directory), exact=True)

        self.assertIsNone(report["comparison"]["first_mismatch_offset"])
        self.assertIsNone(report["comparison"]["target_frame_bytes"])
        self.assertTrue(report["evidence"]["admissible_exact_comparison"])
        self.assertFalse(report["evidence"]["promotion_proof_included"])

    def test_summary_rejects_boundary_disagreement(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(fp.PreflightError, "boundary size disagrees"):
                self.summarize(Path(directory), size=36)

    def test_consolidated_tu_escape_reports_target_and_candidate_drift(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    (
                        "func_overlay_009_F00010B4_186772C",
                        0x10DC,
                        0x468,
                        2,
                        1,
                    )
                ]

        resolution = fp.Resolution(
            "func_overlay_009_F00010B4_186772C",
            "func_overlay_009_F00010B4_186772C",
            "func_overlay_009_F00010B4_186772C",
            fp.REPO / "src/overlays/o009/overlay_009.c",
            "overlays/o009/overlay_009",
            "build_non_matching",
            fp.REPO / "build_non_matching/src/overlays/o009/overlay_009.c.o",
            fp.REPO / "asm/target.s",
            "guarded",
        )
        atlas = {
            "modules": [
                {
                    "overlay": 9,
                    "text_ownership": [
                        {
                            "offset": "0x0",
                            "end_offset": "0x1520",
                            "size": "0x1520",
                            "type": "c",
                            "source": "overlays/o009/overlay_009",
                        }
                    ],
                }
            ]
        }

        with mock.patch.object(fp.rs, "Elf", return_value=FakeElf()):
            message = fp._surface_error_diagnostic(
                fp.rs.SurfaceComparisonError("candidate function escapes TU ownership"),
                resolution,
                atlas,
                0xF00010B4,
                0x468,
            )

        self.assertIn("TU .text+0x10DC..+0x1544", message)
        self.assertIn("target belongs at TU+0x10B4..+0x151C", message)
        self.assertIn("shifted this function by +0x28", message)
        self.assertIn("exceeds the owner by 0x24", message)
        self.assertIn("will not reinterpret bytes outside the atlas owner", message)

    def test_consolidated_alias_error_reports_compiled_identity_drift(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def section(name):
                if name != ".text":
                    raise ValueError(name)
                return 1, {"size": 0x4664}

            @staticmethod
            def symbols():
                return [
                    ("aimed", 0x37D4, 0x3E4, 2, 1),
                    ("overlay1ReadSelection", 0x2ECC, 0xD4, 2, 1),
                ]

        resolution = fp.Resolution(
            "aimed",
            "func_overlay_001_F0006D4C_185312C",
            "aimed",
            fp.REPO / "src/overlays/o001/overlay_001_tail.c",
            "overlays/o001/overlay_001_tail",
            "build_non_matching",
            fp.REPO / "build_non_matching/src/overlays/o001/overlay_001_tail.c.o",
            fp.REPO / "asm/target.s",
            "guarded",
        )
        atlas = {
            "modules": [
                {
                    "overlay": 1,
                    "text_ownership": [
                        {
                            "offset": "0x3578",
                            "end_offset": "0x7BDC",
                            "size": "0x4664",
                            "type": "c",
                            "source": "overlays/o001/overlay_001_tail",
                        }
                    ],
                }
            ]
        }
        with tempfile.TemporaryDirectory() as directory:
            aliases = Path(directory) / "aliases.txt"
            aliases.write_text(
                "func_overlay_001_F0006424_1852804 = overlay1ReadSelection;\n",
                encoding="utf-8",
            )
            error = fp.rs.SurfaceComparisonError(
                "candidate relocation symbol overlay1ReadSelection "
                "has ambiguous runtime identity"
            )
            with mock.patch.object(fp.rs, "Elf", return_value=FakeElf()):
                message = fp._surface_error_diagnostic(
                    error,
                    resolution,
                    atlas,
                    0xF0006D4C,
                    0x3E4,
                    alias_path=aliases,
                )

        self.assertIn("tracked alias identity is overlay:1:+0x6424", message)
        self.assertIn("lands at overlay:1:+0x6444", message)
        self.assertIn("TU .text+0x2ECC", message)
        self.assertIn("disagreement +0x20", message)
        self.assertIn("candidate prefix-layout drift", message)

    def test_resident_boundary_reports_exact_gap_to_next_function(self) -> None:
        class FakeElf:
            names = ["", ".text"]

            @staticmethod
            def symbols():
                return [
                    ("current", 0x80001000, 0x20, 2, 1),
                    ("next", 0x80001028, 0x10, 2, 1),
                ]

        boundary = fp._resident_boundary(FakeElf(), 0x80001000, 0x20, ".text")
        self.assertEqual(boundary["next_symbol"], "next")
        self.assertEqual(boundary["padding_size"], 8)

    def test_workbench_extracts_only_scalar_score_context(self) -> None:
        resolution = fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build_non_matching",
            Path("build_non_matching/src/example.c.o"),
            Path("asm/generated.s"),
            "guarded",
        )
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "words": 3,
            "target_instructions": 10,
            "candidate_instructions": 10,
            "first_divergent_row": 2,
            "verdict": "register-mismatch",
            "target_frame_size": 32,
            "candidate_frame_size": 32,
            "diff_sites": [{"target": "ROM-derived listing is not propagated"}],
        }
        completed = subprocess.CompletedProcess(
            [], 0, stdout=json.dumps(payload), stderr=""
        )
        with mock.patch.object(fp, "_run", return_value=completed) as run:
            report = fp._workbench(resolution)

        self.assertEqual(
            [str(fp.WB_COMPARE), "--no-build", "friendly"],
            run.call_args.args[0][:3],
        )
        self.assertEqual(report["matched_words"], 7)
        self.assertEqual(report["first_mismatch"], "+0x8")
        self.assertNotIn("diff_sites", report)

    def test_promoted_workbench_uses_friendly_symbol_and_rom_oracle(self) -> None:
        resolution = fp.Resolution(
            "generated",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build",
            Path("build/src/example.c.o"),
            None,
            "promoted",
            resolution_mode="post_promotion",
            expected_value=0xF0000020,
            expected_size=0x20,
        )
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "words": 0,
            "target_instructions": 8,
            "candidate_instructions": 8,
            "first_divergent_row": None,
            "verdict": "exact",
        }
        completed = subprocess.CompletedProcess(
            [], 0, stdout=json.dumps(payload), stderr=""
        )
        with mock.patch.object(fp, "_run", return_value=completed) as run:
            report = fp._workbench(resolution)

        command = run.call_args.args[0]
        self.assertEqual(command[1:3], ["--rom", "friendly"])
        self.assertEqual("rom", report["comparison_mode"])

    def test_promoted_geometry_must_equal_tracked_range(self) -> None:
        resolution = fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            Path("src/example.c"),
            "example",
            "build",
            Path("build/src/example.c.o"),
            None,
            "promoted",
            resolution_mode="post_promotion",
            expected_value=0xF0000020,
            expected_size=0x20,
        )
        with self.assertRaisesRegex(fp.PreflightError, "linked geometry disagrees"):
            fp._require_tracked_geometry(resolution, 0xF0000020, 0x24)

    def test_promoted_mode_accepts_exact_relocation_shape_with_proxy_identities(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 9,
            "candidate_identity_resolved_count": 4,
            "target_record_count": 9,
            "offset_type_exact": True,
        }
        fp._require_static_relocation_evidence(resolution, comparison)

    def test_fallback_mode_still_rejects_unresolved_relocation_identity(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded fallback",
        )
        comparison = {
            "candidate_record_count": 3,
            "candidate_identity_resolved_count": 2,
            "target_record_count": 3,
            "offset_type_exact": True,
        }
        with self.assertRaisesRegex(fp.PreflightError, "unresolved at 1"):
            fp._require_static_relocation_evidence(resolution, comparison)

    def test_unresolved_fallback_identity_becomes_structured_partial_evidence(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded fallback",
        )
        comparison = {
            "candidate_record_count": 3,
            "candidate_identity_resolved_count": 2,
            "candidate_identity_unresolved_records": [
                {"offset": 0x18, "rtype": fp.rs.R_MIPS_26}
            ],
            "target_record_count": 3,
            "offset_type_alignment_count": 3,
            "stable_identity_alignment_count": 2,
            "effective_identity_alignment_count": 2,
            "offset_type_exact": True,
            "effective_identity_exact": False,
        }

        status = fp._preflight_evidence_status(resolution, comparison)

        self.assertEqual("partial", status["status"])
        self.assertEqual(
            "resolve_candidate_static_relocation_identities", status["action"]
        )
        self.assertEqual(1, status["counts"]["candidate_identities_unresolved"])
        diagnostic = status["diagnostics"][0]
        self.assertEqual(
            "candidate_static_relocation_identity_unresolved", diagnostic["code"]
        )
        self.assertEqual(
            [{"offset": "+0x18", "type": "R_MIPS_26"}],
            diagnostic["sites"],
        )
        self.assertNotIn("identity", diagnostic["sites"][0])

    def test_post_promotion_runtime_proof_keeps_static_gap_visible(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 3,
            "candidate_identity_resolved_count": 2,
            "candidate_identity_unresolved_records": [
                {"offset": 0x18, "rtype": fp.rs.R_MIPS_26}
            ],
            "target_record_count": 3,
            "offset_type_alignment_count": 3,
            "stable_identity_alignment_count": 2,
            "effective_identity_alignment_count": 3,
            "offset_type_exact": True,
            "effective_identity_exact": True,
        }

        status = fp._preflight_evidence_status(resolution, comparison)

        self.assertEqual("complete", status["status"])
        self.assertEqual("run_promotion_proof", status["action"])
        self.assertEqual("warning", status["diagnostics"][0]["severity"])

    def test_unresolved_diagnostic_count_mismatch_fails_closed(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded fallback",
        )
        comparison = {
            "candidate_record_count": 2,
            "candidate_identity_resolved_count": 1,
            "candidate_identity_unresolved_records": [],
            "target_record_count": 2,
            "offset_type_alignment_count": 2,
            "stable_identity_alignment_count": 1,
        }

        with self.assertRaisesRegex(fp.PreflightError, "disagree with the count"):
            fp._preflight_evidence_status(resolution, comparison)

    def test_promoted_mode_rejects_relocation_shape_drift(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 9,
            "candidate_identity_resolved_count": 9,
            "target_record_count": 8,
            "offset_type_exact": False,
        }
        with self.assertRaisesRegex(fp.PreflightError, "shape disagrees"):
            fp._require_static_relocation_evidence(resolution, comparison)

    def test_promoted_linked_exact_completes_runtime_identity_proof(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build", Path("build/src/example.c.o"), None,
            "promoted", resolution_mode="post_promotion",
        )
        comparison = {
            "candidate_record_count": 13,
            "target_record_count": 13,
            "offset_type_exact": True,
            "stable_identity_alignment_count": 8,
            "stable_identity_exact": False,
        }
        workbench = {
            "differing_words": 0,
            "target_words": 133,
            "candidate_words": 133,
        }
        result = fp._augment_runtime_identity_evidence(
            resolution, comparison, workbench
        )
        self.assertEqual(5, result["linked_runtime_identity_alignment_count"])
        self.assertEqual(13, result["effective_identity_alignment_count"])
        self.assertTrue(result["effective_identity_exact"])
        self.assertEqual(
            "static-plus-runtime-table-and-linked-rom",
            result["identity_proof_mode"],
        )

    def test_unpromoted_exact_words_do_not_replace_static_identity(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching",
            Path("build_non_matching/src/example.c.o"), Path("target.s"),
            "guarded",
        )
        comparison = {
            "candidate_record_count": 3,
            "target_record_count": 3,
            "offset_type_exact": True,
            "stable_identity_alignment_count": 2,
            "stable_identity_exact": False,
        }
        workbench = {
            "differing_words": 0,
            "target_words": 20,
            "candidate_words": 20,
        }
        result = fp._augment_runtime_identity_evidence(
            resolution, comparison, workbench
        )
        self.assertEqual(0, result["linked_runtime_identity_alignment_count"])
        self.assertEqual(2, result["effective_identity_alignment_count"])
        self.assertFalse(result["effective_identity_exact"])
        self.assertEqual("partial-static", result["identity_proof_mode"])


class FreshnessTests(unittest.TestCase):
    def test_build_jobs_default_and_explicit_override_reach_both_phases(self) -> None:
        for cpu_count, override, expected in (
            (14, None, 14), (None, None, 1), (0, None, 1),
            (14, "1", 1), (14, "2", 2), (2, "14", 14),
        ):
            with self.subTest(cpu_count=cpu_count, override=override):
                environment = {} if override is None else {"MICKEY_BUILD_JOBS": override}
                with tempfile.TemporaryDirectory() as directory:
                    resolution = self.resolution(Path(directory))
                    completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
                    with mock.patch.dict(os.environ, environment, clear=True), mock.patch.object(
                        fp.os, "cpu_count", return_value=cpu_count
                    ), mock.patch.object(fp, "_newer_inputs", return_value=[]), mock.patch.object(
                        fp, "_run", return_value=completed
                    ) as run:
                        fp._build_target(resolution.candidate_object, non_matching=True, label="candidate")
                    commands = [call.args[0] for call in run.call_args_list]
                    self.assertEqual(len(commands), 2)
                    for command in commands:
                        self.assertEqual(command[:5], ["nice", "-n", "10", "gmake", f"-j{expected}"])
                        self.assertIn("NON_MATCHING=1", command)
                    self.assertEqual(commands[0][-1], "build_non_matching/.splat-stamp")
                    self.assertEqual(commands[1][-1], fp._relative(resolution.candidate_object))

    def test_invalid_build_jobs_fail_before_any_build(self) -> None:
        for value in ("", "0", "00", "-1", "+2", "1.5", " 2", "2 ", "2\n",
                      "two", "2;echo no", "２", "9" * 5000):
            with self.subTest(value=value[:20]), mock.patch.dict(
                os.environ, {"MICKEY_BUILD_JOBS": value}
            ), mock.patch.object(fp, "_run") as run, mock.patch.object(fp, "_newer_inputs") as newer:
                with self.assertRaisesRegex(fp.PreflightError, "MICKEY_BUILD_JOBS"):
                    fp._build_target(Path("build/example.o"), non_matching=False, label="candidate")
                run.assert_not_called()
                newer.assert_not_called()

    def test_stale_make_hint_uses_resolved_build_jobs(self) -> None:
        for override, expected in ((None, 14), ("2", 2)):
            with self.subTest(override=override), tempfile.TemporaryDirectory() as directory:
                resolution = self.resolution(Path(directory))
                environment = {} if override is None else {"MICKEY_BUILD_JOBS": override}
                completed = subprocess.CompletedProcess([], 1, stdout="", stderr="")
                with mock.patch.dict(os.environ, environment, clear=True), mock.patch.object(
                    fp.os, "cpu_count", return_value=14
                ), mock.patch.object(fp, "_run", return_value=completed):
                    with self.assertRaisesRegex(fp.StaleEvidenceError, f"gmake -j{expected} NON_MATCHING=1"):
                        fp._require_fresh_target(resolution.candidate_object, label="candidate",
                                                 non_matching=True, build_logic_inputs=())

    def test_no_build_staleness_fails_without_rebuild_or_reauthentication(self) -> None:
        for mode in ([], ["--resolve-rom"]):
            with self.subTest(mode=mode), mock.patch.object(
                fp, "resolve", return_value=mock.Mock()
            ) as resolve, mock.patch.object(fp, "_build") as build, mock.patch.object(
                fp, "_build_linked_boundary"
            ) as build_boundary, mock.patch.object(
                fp, "require_fresh_evidence", side_effect=fp.StaleEvidenceError("stale")
            ), mock.patch.object(
                fp, "_require_fresh_linked_boundary", side_effect=fp.StaleEvidenceError("stale")
            ), mock.patch.object(fp, "collect") as collect, mock.patch.object(
                fp, "_linked_boundary"
            ) as boundary, contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as error:
                    fp.main(["friendly", "--no-build", *mode])
            self.assertEqual(error.exception.code, 2)
            resolve.assert_called_once_with("friendly")
            build.assert_not_called()
            build_boundary.assert_not_called()
            collect.assert_not_called()
            boundary.assert_not_called()

    def resolution(self, root: Path) -> fp.Resolution:
        source = root / "src/example.c"
        source.parent.mkdir(parents=True)
        source.write_text("void friendly(void) {}\n", encoding="utf-8")
        candidate = root / "build_non_matching/src/example.c.o"
        candidate.parent.mkdir(parents=True)
        candidate.write_bytes(b"object")
        return fp.Resolution(
            "friendly",
            "generated",
            "friendly",
            source,
            "example",
            "build_non_matching",
            candidate,
            root / "asm/generated.s",
            "guarded",
        )

    def test_make_graph_staleness_fails_before_comparison(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 1, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed):
                with self.assertRaisesRegex(fp.PreflightError, "according to the Make"):
                    fp._require_fresh_target(
                        resolution.candidate_object,
                        label="candidate object",
                        non_matching=True,
                        build_logic_inputs=(),
                    )

    def test_newer_recipe_or_config_fails_even_when_make_would_not(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            config = root / "config/normalizations/example.mk"
            config.parent.mkdir(parents=True)
            config.write_text("# flags changed\n", encoding="utf-8")
            object_time = resolution.candidate_object.stat().st_mtime_ns
            os.utime(config, ns=(object_time + 1_000_000, object_time + 1_000_000))

            with self.assertRaisesRegex(fp.PreflightError, "newer build recipe/config"):
                fp._require_fresh_target(
                    resolution.candidate_object,
                    label="candidate object",
                    non_matching=True,
                    build_logic_inputs=(config,),
                )

    def test_build_logic_inputs_include_make_policy_fragments(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            makefile = root / "Makefile"
            overlay_policy = root / "mk/overlays.mk"
            normalization = root / "config/normalizations/example.mk"
            for path in (makefile, overlay_policy, normalization):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("# build logic\n", encoding="utf-8")

            self.assertEqual(
                (makefile, overlay_policy, normalization),
                fp._build_logic_inputs(root),
            )

    def test_fresh_nonmatching_object_queries_its_actual_build_mode(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._require_fresh_target(
                    resolution.candidate_object,
                    label="candidate object",
                    non_matching=True,
                    build_logic_inputs=(),
                )

            command = run.call_args.args[0]
            self.assertIn("NON_MATCHING=1", command)
            self.assertEqual(command[-1], fp._relative(resolution.candidate_object))

    def test_make_query_error_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess(
                [], 2, stdout="", stderr="dependency parse failed\n"
            )
            with mock.patch.object(fp, "_run", return_value=completed):
                with self.assertRaisesRegex(fp.PreflightError, "could not prove"):
                    fp._require_fresh_target(
                        resolution.candidate_object,
                        label="candidate object",
                        non_matching=True,
                        build_logic_inputs=(),
                    )

    def test_workbench_boundary_fails_closed_on_a_modified_fallback(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            resolution.target_asm.parent.mkdir(parents=True)
            resolution.target_asm.write_text("glabel generated\n", encoding="utf-8")
            stamp = root / "build_non_matching/.splat-stamp"
            stamp.parent.mkdir(parents=True, exist_ok=True)
            stamp.write_text("split\n", encoding="utf-8")
            stamp_time = stamp.stat().st_mtime_ns
            os.utime(
                resolution.target_asm,
                ns=(stamp_time + 1_000_000, stamp_time + 1_000_000),
            )

            with mock.patch.object(
                fp, "_wb_split_receipts", return_value=((stamp, True),)
            ), mock.patch.object(fp, "_require_fresh_target"):
                with self.assertRaisesRegex(fp.PreflightError, "newer than"):
                    fp._require_fresh_wb_evidence(resolution)

    def test_canonical_split_receipt_can_prove_shared_fallback(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            resolution.target_asm.parent.mkdir(parents=True)
            resolution.target_asm.write_text("glabel generated\n", encoding="utf-8")
            stamp = root / "build/.splat-stamp"
            stamp.parent.mkdir(parents=True, exist_ok=True)
            stamp.write_text("split\n", encoding="utf-8")
            asm_time = resolution.target_asm.stat().st_mtime_ns
            os.utime(stamp, ns=(asm_time + 1_000_000, asm_time + 1_000_000))

            with mock.patch.object(
                fp, "_wb_split_receipts", return_value=((stamp, False),)
            ), mock.patch.object(fp, "_require_fresh_target"):
                fp._require_fresh_wb_boundary(resolution)

    def test_build_runs_split_and_target_as_separate_make_invocations(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._build_target(
                    resolution.candidate_object,
                    non_matching=True,
                    label="candidate",
                )

            commands = [call.args[0] for call in run.call_args_list]
            self.assertEqual(len(commands), 2)
            self.assertEqual(commands[0][-1], "build_non_matching/.splat-stamp")
            self.assertEqual(commands[1][-1], fp._relative(resolution.candidate_object))
            self.assertIn("NON_MATCHING=1", commands[0])
            self.assertIn("NON_MATCHING=1", commands[1])

    def test_newer_build_policy_forces_complete_target_dependency_graph(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolution = self.resolution(root)
            policy = root / "mk/flags.mk"
            policy.parent.mkdir(parents=True)
            policy.write_text("# changed flags\n", encoding="utf-8")
            object_time = resolution.candidate_object.stat().st_mtime_ns
            os.utime(policy, ns=(object_time + 1_000_000, object_time + 1_000_000))
            completed = subprocess.CompletedProcess([], 0, stdout="", stderr="")
            with mock.patch.object(
                fp, "_build_logic_inputs", return_value=(policy,)
            ), mock.patch.object(fp, "_run", return_value=completed) as run:
                fp._build_target(
                    resolution.candidate_object,
                    non_matching=True,
                    label="candidate",
                )

            commands = [call.args[0] for call in run.call_args_list]
            self.assertNotIn("--always-make", commands[0])
            self.assertIn("--always-make", commands[1])
            self.assertIn("--assume-old=.venv/bin/python", commands[1])
            self.assertEqual(commands[1][-1], fp._relative(resolution.candidate_object))

    def test_resolve_wb_refreshes_by_default_and_no_build_is_explicit(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            resolution.target_asm.parent.mkdir(parents=True)
            resolution.target_asm.write_text("glabel generated\n", encoding="utf-8")
            with mock.patch.object(
                fp, "resolve", return_value=resolution
            ), mock.patch.object(fp, "_build_wb_evidence") as build, mock.patch.object(
                fp, "_require_fresh_wb_evidence"
            ) as freshness:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(0, fp.main(["friendly", "--resolve-wb"]))
            build.assert_called_once_with(resolution)
            freshness.assert_called_once_with(resolution)

            with mock.patch.object(
                fp, "resolve", return_value=resolution
            ), mock.patch.object(fp, "_build_wb_evidence") as build, mock.patch.object(
                fp, "_require_fresh_wb_evidence"
            ) as freshness:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(
                        0,
                        fp.main(["friendly", "--resolve-wb", "--no-build"]),
                    )
            build.assert_not_called()
            freshness.assert_called_once_with(resolution)

    def test_nonmatching_candidate_build_precedes_final_canonical_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            with mock.patch.object(
                fp,
                "_require_fresh_target",
                side_effect=fp.StaleEvidenceError("stale"),
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build(resolution)

            self.assertEqual(
                [
                    mock.call(
                        resolution.candidate_object,
                        non_matching=True,
                        label="candidate",
                    ),
                    mock.call(fp.TARGET_ELF, non_matching=False, label="canonical"),
                ],
                build_target.call_args_list,
            )

    def test_ordinary_candidate_is_supplied_by_canonical_link_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = dataclasses.replace(
                self.resolution(Path(directory)), candidate_build_dir="build"
            )
            with mock.patch.object(
                fp,
                "_require_fresh_target",
                side_effect=fp.StaleEvidenceError("stale"),
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build(resolution)

            build_target.assert_called_once_with(
                fp.TARGET_ELF, non_matching=False, label="canonical"
            )

    def test_current_full_evidence_skips_every_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            with mock.patch.object(
                fp, "_require_fresh_target"
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build(resolution)

            build_target.assert_not_called()

    def test_workbench_refresh_builds_candidate_without_linking_canonical_elf(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            with mock.patch.object(
                fp, "_require_fresh_wb_boundary"
            ), mock.patch.object(
                fp,
                "_require_fresh_target",
                side_effect=fp.StaleEvidenceError("stale candidate"),
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build_wb_evidence(resolution)

            build_target.assert_called_once_with(
                resolution.candidate_object,
                non_matching=True,
                label="candidate",
            )
            self.assertNotIn(fp.TARGET_ELF, [call.args[0] for call in build_target.call_args_list])

    def test_current_workbench_evidence_skips_candidate_build(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            with mock.patch.object(
                fp, "_require_fresh_wb_boundary"
            ), mock.patch.object(
                fp, "_require_fresh_target"
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build_wb_evidence(resolution)

            build_target.assert_not_called()

    def test_stale_workbench_boundary_refreshes_split_before_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            stamp = Path(directory) / "build_non_matching/.splat-stamp"
            with mock.patch.object(
                fp,
                "_require_fresh_wb_boundary",
                side_effect=fp.StaleEvidenceError("stale split"),
            ), mock.patch.object(
                fp, "_require_fresh_target"
            ), mock.patch.object(
                fp, "_candidate_split_stamp", return_value=stamp
            ), mock.patch.object(fp, "_build_target") as build_target:
                fp._build_wb_evidence(resolution)

            self.assertEqual(
                [
                    mock.call(stamp, non_matching=True, label="target boundary"),
                    mock.call(
                        resolution.candidate_object,
                        non_matching=True,
                        label="candidate",
                    ),
                ],
                build_target.call_args_list,
            )

    def test_resolve_rom_emits_one_preflight_proved_boundary(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            resolution = self.resolution(Path(directory))
            boundary = {
                "linked_symbol": "friendly",
                "value": 0x80001000,
                "size": 0x20,
                "section": ".text",
                "evidence": "preflight-owned-boundary",
            }
            output = io.StringIO()
            with mock.patch.object(
                fp, "resolve", return_value=resolution
            ), mock.patch.object(fp, "_build_linked_boundary") as build, mock.patch.object(
                fp, "_require_fresh_linked_boundary"
            ) as freshness, mock.patch.object(
                fp, "_linked_boundary", return_value=boundary
            ), contextlib.redirect_stdout(output):
                self.assertEqual(0, fp.main(["friendly", "--resolve-rom"]))

            build.assert_called_once_with()
            freshness.assert_called_once_with()
            self.assertEqual(
                "friendly\t80001000\t20\t.text\tpreflight-owned-boundary\n",
                output.getvalue(),
            )


class PairedRelocationSummaryTests(unittest.TestCase):
    NOW_NS = 2_000_000_000_000
    BASE = "b" * 40
    BRANCH = "lane/wave10-reloc-dual"
    OWNER = "src/overlays/o047/overlay47ReleaseResources.c"

    def summary(
        self,
        surface_name: str,
        *,
        exact_identities: int,
        resolved_identities: int,
        base: str | None = None,
        symbol: str = "func_overlay_047_F00009D0_18917E8",
        created_ns: int | None = None,
    ) -> dict[str, object]:
        mode = "asm" if surface_name == "fallback_static" else "rom"
        evidence_mode = fp.WB_RELOCATION_SURFACE_MODES[surface_name]
        digest_prefix = "a" if surface_name == "fallback_static" else "d"
        digest = digest_prefix * 64
        surface = {
            "candidate_relocations": 46,
            "target_relocations": 46,
            "offset_type_relocations": 46,
            "resolved_candidate_identities": resolved_identities,
            "exact_relocation_identities": exact_identities,
            "evidence_mode": evidence_mode,
            "complete": resolved_identities == 46,
        }
        artifacts = {
            "source": {
                "path": self.OWNER,
                "sha256": digest,
                "size": 100,
            },
            "candidate_object": {
                "path": f"build/{surface_name}.candidate.o",
                "sha256": digest,
                "size": 200,
            },
            "target_object": {
                "path": f"build/wb/{surface_name}.target.o",
                "sha256": digest,
                "size": 300,
            },
        }
        return {
            "schema": fp.WB_SUMMARY_SCHEMA,
            "symbol": {
                "requested": symbol,
                "target": symbol,
                "candidate": symbol,
            },
            "mode": mode,
            "boundary": {
                "bytes": 352,
                "evidence": (
                    "extracted-fallback-symbol"
                    if mode == "asm"
                    else "preflight-owned-boundary"
                ),
            },
            "comparison": {
                "exact": True,
                "accepted": True,
                "target_words": 88,
                "candidate_words": 88,
                "matched_words": 88,
                "differing_words": 0,
                "raw_differing_words": 0,
            },
            "provenance": {
                "classification": "non_matching_candidate",
                "exact_claim_allowed": True,
                "verdict": "c_evidence",
                "source_sha256": digest,
                "candidate_object_sha256": digest,
                "target_object_sha256": digest,
            },
            "evidence": {
                "admissible_exact_comparison": True,
                "promotion_proof_included": False,
                "scope": "workbench-comparison-not-canonical-promotion-proof",
                "raw_report_sha256": ("e" if mode == "asm" else "f") * 64,
            },
            "relocations": fp._summary_relocations(surface),
            "relocation_surfaces": {surface_name: surface},
            "proof_context": {
                "base": self.BASE if base is None else base,
                "branch": self.BRANCH,
                "owner": self.OWNER,
                "manifest_sha256": ("1" if mode == "asm" else "2") * 64,
                "created_unix_ns": (
                    self.NOW_NS if created_ns is None else created_ns
                ),
                "artifacts": artifacts,
            },
        }

    def write_pair(
        self,
        root: Path,
        *,
        fallback: dict[str, object] | None = None,
        promoted: dict[str, object] | None = None,
    ) -> tuple[Path, Path]:
        build = root / "build"
        build.mkdir()
        fallback_path = build / "fallback.json"
        promoted_path = build / "promoted.json"
        fallback_path.write_text(
            json.dumps(
                self.summary(
                    "fallback_static", exact_identities=9, resolved_identities=20
                )
                if fallback is None
                else fallback
            ),
            encoding="utf-8",
        )
        promoted_path.write_text(
            json.dumps(
                self.summary(
                    "promoted_linked", exact_identities=34, resolved_identities=46
                )
                if promoted is None
                else promoted
            ),
            encoding="utf-8",
        )
        os.utime(
            fallback_path,
            ns=(self.NOW_NS, self.NOW_NS),
        )
        os.utime(
            promoted_path,
            ns=(self.NOW_NS, self.NOW_NS),
        )
        return fallback_path, promoted_path

    def compose(
        self,
        root: Path,
        fallback: Path,
        promoted: Path,
    ) -> dict[str, object]:
        return fp.compose_relocation_summaries(
            fallback,
            promoted,
            root=root,
            now_ns=self.NOW_NS,
            repository_context={"base": self.BASE, "branch": self.BRANCH},
        )

    def test_pairs_nine_static_with_thirty_four_promoted_without_inference(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root)
            report = self.compose(root, fallback, promoted)

        surfaces = report["relocation_surfaces"]
        self.assertEqual(9, surfaces["fallback_static"]["exact_relocation_identities"])
        self.assertFalse(surfaces["fallback_static"]["complete"])
        self.assertEqual(34, surfaces["promoted_linked"]["exact_relocation_identities"])
        self.assertTrue(surfaces["promoted_linked"]["complete"])
        self.assertEqual(9, report["relocations"]["exact_relocation_identities"])
        self.assertEqual("fallback_static", report["relocation_pair"]["legacy_surface"])

    def test_human_rendering_shows_both_surfaces(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root)
            report = self.compose(root, fallback, promoted)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            fp._render_workbench_summary_human(report)
        rendered = output.getvalue()
        self.assertIn("fallback-static", rendered)
        self.assertIn("exact identities=9", rendered)
        self.assertIn("promoted-linked", rendered)
        self.assertIn("exact identities=34", rendered)

    def test_stale_input_fails_closed(self) -> None:
        stale = self.summary(
            "fallback_static",
            exact_identities=9,
            resolved_identities=20,
            created_ns=self.NOW_NS - (fp.WB_SUMMARY_MAX_AGE_SECONDS + 1) * 1_000_000_000,
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root, fallback=stale)
            with self.assertRaisesRegex(fp.PreflightError, "summary is stale"):
                self.compose(root, fallback, promoted)

    def test_malformed_surface_fails_closed(self) -> None:
        malformed = self.summary(
            "fallback_static", exact_identities=9, resolved_identities=20
        )
        malformed["relocation_surfaces"]["fallback_static"][
            "exact_relocation_identities"
        ] = 47
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root, fallback=malformed)
            with self.assertRaisesRegex(fp.PreflightError, "exceed"):
                self.compose(root, fallback, promoted)

    def test_cross_symbol_fails_closed(self) -> None:
        promoted_payload = self.summary(
            "promoted_linked",
            exact_identities=34,
            resolved_identities=46,
            symbol="other_symbol",
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root, promoted=promoted_payload)
            with self.assertRaisesRegex(fp.PreflightError, "cross-symbol"):
                self.compose(root, fallback, promoted)

    def test_cross_base_fails_closed(self) -> None:
        promoted_payload = self.summary(
            "promoted_linked",
            exact_identities=34,
            resolved_identities=46,
            base="c" * 40,
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root, promoted=promoted_payload)
            with self.assertRaisesRegex(fp.PreflightError, "cross-base"):
                self.compose(root, fallback, promoted)

    def test_duplicate_surface_inputs_fail_closed(self) -> None:
        duplicate = self.summary(
            "fallback_static", exact_identities=9, resolved_identities=20
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root, promoted=duplicate)
            with self.assertRaisesRegex(fp.PreflightError, "wrong comparison mode"):
                self.compose(root, fallback, promoted)

    def test_paired_linked_evidence_cannot_claim_object_exact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fallback, promoted = self.write_pair(root)
            report = self.compose(root, fallback, promoted)
        with mock.patch.object(crew, "read_fresh_workbench_summary", return_value=report):
            result = crew.workbench_summary_result(
                "build/paired.json",
                worker="worker-1",
                target="func_overlay_047_F00009D0_18917E8",
            )
        self.assertEqual("compiled", result["promotion_state"])
        self.assertEqual(9, result["exact_relocation_identities"])


class PartialEvidenceCliTests(unittest.TestCase):
    def report(self) -> dict[str, object]:
        return {
            "schema": "mickey-function-evidence-preflight-v1",
            "preflight": {
                "status": "partial",
                "action": "resolve_candidate_static_relocation_identities",
                "counts": {
                    "target_relocations": 3,
                    "candidate_static_relocations": 3,
                    "offset_type_aligned": 3,
                    "stable_identities_aligned": 2,
                    "effective_identities_aligned": 2,
                    "candidate_identities_resolved": 2,
                    "candidate_identities_unresolved": 1,
                },
                "diagnostics": [
                    {
                        "code": "candidate_static_relocation_identity_unresolved",
                        "severity": "error",
                        "count": 1,
                        "sites": [{"offset": "+0x18", "type": "R_MIPS_26"}],
                        "message": "no identity was inferred",
                    }
                ],
            },
            "workbench": {
                "target_words": 20,
                "candidate_words": 20,
                "matched_words": 19,
                "differing_words": 1,
                "target_frame": 32,
                "candidate_frame": 32,
                "first_mismatch": "+0x8",
                "verdict": "register-mismatch",
            },
        }

    def invoke(self, *extra: str) -> tuple[int, dict[str, object]]:
        output = io.StringIO()
        with (
            mock.patch.object(fp, "resolve", return_value=mock.sentinel.resolution),
            mock.patch.object(fp, "require_fresh_evidence"),
            mock.patch.object(fp, "collect", return_value=self.report()),
            contextlib.redirect_stdout(output),
        ):
            result = fp.main(["friendly", "--no-build", "--json", *extra])
        return result, json.loads(output.getvalue())

    def test_proof_mode_emits_partial_json_and_distinct_nonzero_exit(self) -> None:
        result, payload = self.invoke()

        self.assertEqual(fp.PARTIAL_EVIDENCE_EXIT, result)
        self.assertEqual("partial", payload["preflight"]["status"])
        self.assertEqual(20, payload["workbench"]["target_words"])
        self.assertEqual(32, payload["workbench"]["candidate_frame"])
        self.assertEqual("+0x8", payload["workbench"]["first_mismatch"])

    def test_analysis_only_keeps_partial_status_and_returns_success(self) -> None:
        result, payload = self.invoke("--analysis-only")

        self.assertEqual(0, result)
        self.assertEqual("partial", payload["preflight"]["status"])
        self.assertEqual("error", payload["preflight"]["diagnostics"][0]["severity"])

    def test_human_partial_status_includes_counts_and_unresolved_site(self) -> None:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            fp._render_preflight_status(self.report()["preflight"])

        rendered = output.getvalue()
        self.assertIn("status: partial (non-exact)", rendered)
        self.assertIn("action: resolve_candidate_static_relocation_identities", rendered)
        self.assertIn("resolved=2 unresolved=1", rendered)
        self.assertIn("+0x18 R_MIPS_26 identity=unresolved", rendered)


class RelocationEvidenceTests(unittest.TestCase):
    def test_resident_reports_eight_static_tuples_and_zero_runtime_records(self) -> None:
        resolution = fp.Resolution(
            "func_8002BB40",
            "func_8002BB40",
            "func_8002BB40",
            Path("src/main/memory.c"),
            "main/memory",
            "build",
            Path("build/src/main/memory.c.o"),
            Path("asm/unused.s"),
            "ordinary C",
        )
        static_records = [
            fp.rs.SurfaceRecord(0x14, fp.rs.R_MIPS_HI16, (0, 0xD1D60)),
            fp.rs.SurfaceRecord(0x18, fp.rs.R_MIPS_LO16, (0, 0xD1D60)),
            fp.rs.SurfaceRecord(0x28, fp.rs.R_MIPS_HI16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x2C, fp.rs.R_MIPS_LO16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x08, fp.rs.R_MIPS_HI16, (0, 0x79E20)),
            fp.rs.SurfaceRecord(0x64, fp.rs.R_MIPS_LO16, (0, 0x79E20)),
            fp.rs.SurfaceRecord(0x40, fp.rs.R_MIPS_HI16, (0, 0xD1810)),
            fp.rs.SurfaceRecord(0x44, fp.rs.R_MIPS_LO16, (0, 0xD1810)),
        ]

        with mock.patch.object(
            fp.rs, "_resident_target_records", return_value=static_records
        ) as authenticate:
            evidence = fp._relocation_evidence(
                resolution,
                {"kind": "resident"},
                [],
                mock.sentinel.target_elf,
                "func_8002BB40",
                0x8002BB40,
                0x120,
                ".main",
            )

        self.assertEqual(8, len(evidence["target_static_relocations"]))
        self.assertEqual([], evidence["resident_runtime_records"])
        self.assertEqual([], evidence["runtime_overlay_records"])
        self.assertEqual(
            ["+0x14", "+0x18", "+0x28", "+0x2C", "+0x8", "+0x64", "+0x40", "+0x44"],
            [row["offset"] for row in evidence["target_static_relocations"]],
        )
        authenticate.assert_called_once_with(
            resolution.candidate_object,
            "main/memory",
            mock.sentinel.target_elf,
            "func_8002BB40",
            0x8002BB40,
            0x120,
            ".main",
            fp.ALIASES,
            [],
        )

        report = {
            "target_symbol": "func_8002BB40",
            "candidate_symbol": "func_8002BB40",
            "resolution_mode": "post_promotion",
            "identity_evidence": "symbol_addrs matched-C function row",
            "preflight": {
                "status": "complete",
                "action": "run_promotion_proof",
                "counts": {
                    "target_relocations": 8,
                    "candidate_static_relocations": 8,
                    "offset_type_aligned": 8,
                    "stable_identities_aligned": 8,
                    "effective_identities_aligned": 8,
                    "candidate_identities_resolved": 8,
                    "candidate_identities_unresolved": 0,
                },
                "diagnostics": [],
            },
            "source": "src/main/memory.c",
            "candidate_build_dir": "build",
            "candidate_signature": "s32 func_8002BB40(void)",
            "owned_size": 0x120,
            "context": {
                "kind": "resident",
                "vram_start": 0x8002BB40,
                "vram_end": 0x8002BC60,
                "rom_start": 0x2C740,
                "rom_end": 0x2C860,
                "next_symbol": None,
            },
            "exports": [],
            "callers": [],
            "inbound_references": [],
            **evidence,
            "relocation_comparison": {
                "target_surface_source": "resident-canonical-static-object",
                "target_record_count": 8,
                "offset_type_alignment_count": 8,
                "stable_identity_alignment_count": 8,
            },
            "workbench": {
                "comparison_mode": "rom",
                "matched_words": 72,
                "target_words": 72,
                "candidate_words": 72,
                "differing_words": 0,
                "first_mismatch": None,
                "verdict": "exact",
                "target_frame": 0,
                "candidate_frame": 0,
            },
        }
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            fp._render_human(report)
        rendered = output.getvalue()
        self.assertIn("target static relocations: 8", rendered)
        self.assertIn("resident runtime records: 0", rendered)
        self.assertIn("runtime overlay records: not applicable", rendered)
        self.assertIn("offset/type=8/8 identity=8/8", rendered)
        self.assertIn("workbench [rom]: matched=72/72", rendered)

    def test_overlay_runtime_records_are_not_called_static_target_records(self) -> None:
        resolution = fp.Resolution(
            "friendly", "generated", "friendly", Path("src/example.c"),
            "example", "build_non_matching", Path("build/example.o"),
            Path("asm/generated.s"), "guarded",
        )
        runtime = [fp.rs.SurfaceRecord(0x10, fp.rs.R_MIPS_26, (7, 0x20), 0)]

        evidence = fp._relocation_evidence(
            resolution, {"kind": "overlay"}, runtime,
            mock.sentinel.target_elf, "generated", 0xF0000020, 0x40, ".ovl",
        )

        self.assertEqual([], evidence["target_static_relocations"])
        self.assertEqual(1, len(evidence["runtime_overlay_records"]))
        self.assertEqual([], evidence["resident_runtime_records"])
if __name__ == "__main__":
    unittest.main()
