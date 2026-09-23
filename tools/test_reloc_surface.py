#!/usr/bin/env python3
"""Focused source-level regression tests for reloc_surface safety gates."""

from __future__ import annotations

import contextlib
import io
import json
import os
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface as rs  # noqa: E402


class LinkedObjectCompletenessTests(unittest.TestCase):
    def test_missing_linker_object_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            (repo / "mickey.us.ld").write_text(
                "build/src/overlays/o001/present.c.o(.text)\n"
                "build/src/overlays/o002/missing.c.o(.text)\n"
            )
            present = repo / "build/src/overlays/o001/present.c.o"
            present.parent.mkdir(parents=True)
            present.write_bytes(b"placeholder")

            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaises(SystemExit) as caught:
                    rs.linked_overlay_objects()

            message = str(caught.exception)
            self.assertIn("complete linker object set", message)
            self.assertIn("build/src/overlays/o002/missing.c.o", message)

    def test_complete_linker_object_set_is_returned_in_order(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            rels = [
                "build/src/overlays/o002/second.c.o",
                "build/src/overlays/o001/first.c.o",
            ]
            (repo / "mickey.us.ld").write_text(
                "\n".join(rel + "(.text)" for rel in rels) + "\n"
            )
            for rel in rels:
                path = repo / rel
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"placeholder")

            with mock.patch.object(rs, "REPO", repo):
                got = rs.linked_overlay_objects()

            self.assertEqual([(2, repo / rels[0]), (1, repo / rels[1])], got)


class ResidentRebindSafetyTests(unittest.TestCase):
    def test_dry_run_reports_plan_without_objcopy(self):
        objects = [(40, Path("build/src/overlays/o040/example.c.o"))]
        with mock.patch.object(rs, "resident_defined_names", return_value=set()):
            with mock.patch.object(
                rs,
                "resident_call_aliases",
                return_value=({"func_80000000": "func_80000000_o040Reloc"}, [], []),
            ):
                refusals, notes, planned = rs.rebind_resident_calls(
                    objects,
                    b"",
                    {40: []},
                    [None] * 40,
                    [],
                    records_cache={40: []},
                    apply=False,
                )

        self.assertEqual([], refusals)
        self.assertEqual([], notes)
        self.assertEqual(
            [("example.c.o", "func_80000000", "func_80000000_o040Reloc")],
            planned,
        )

    def test_check_mode_refuses_pending_rebind(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            config = repo / "config"
            config.mkdir()
            (config / "overlays.us.json").write_text(json.dumps({"modules": []}))
            rom = repo / "rom.z64"
            out = repo / "surface.txt"
            rom.write_bytes(b"")
            out.write_text("tracked\n")
            diag = {
                "pending_rebinds": [
                    ("example.c.o", "func_80000000", "func_80000000_o040Reloc")
                ]
            }

            stderr = io.StringIO()
            with mock.patch.object(rs, "REPO", repo):
                with mock.patch.object(rs, "generate", return_value=("generated\n", diag)) as gen:
                    with contextlib.redirect_stderr(stderr):
                        status = rs.cmd_generate(
                            ["--check", "--rom", str(rom), "--out", str(out)]
                        )

            self.assertEqual(1, status)
            self.assertFalse(gen.call_args.kwargs["mutate_objects"])
            self.assertIn("read-only generation will not modify", stderr.getvalue())


class FunctionSurfaceComparisonTests(unittest.TestCase):
    GENERATED_O22 = "func_overlay_022_F0000D30_1878E38"

    class BoundaryElf:
        def __init__(self, *, path=Path("missing"), names=(), symbols=(),
                     relocations=(), text=b""):
            self.path = Path(path)
            self.names = list(names)
            self._symbols = list(symbols)
            self._relocations = list(relocations)
            self._text = bytes(text)

        def section(self, name):
            if name == ".text" and ".text" in self.names:
                return self.names.index(".text"), object()
            return None, None

        def section_bytes(self, name):
            return self._text if name == ".text" else b""

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def o22_atlas(self, ownership=None):
        if ownership is None:
            ownership = [{
                "offset": "0xD30",
                "end_offset": "0xE9C",
                "size": "0x16C",
                "type": "c",
                "source": "overlays/o022/overlay22RemoveObject",
            }]
        return {
            "modules": [{
                "overlay": 22,
                "identity": "overlay:22",
                "synthetic_vma": "0xF0000000",
                "rom": {"start": "0x1878108"},
                "sections": {
                    "text": {
                        "start": "0x1878108",
                        "end": "0x1878FA8",
                        "size": "0xEA0",
                    }
                },
                "text_ownership": ownership,
            }]
        }

    def o22_boundary_fixture(self, root, *, rtype=None, linked_value=None):
        source = root / "src/overlays/o022/overlay22RemoveObject.c"
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text("void placeholder(void) {}\n", encoding="utf-8")
        obj = root / "build/src/overlays/o022/overlay22RemoveObject.c.o"
        obj.parent.mkdir(parents=True, exist_ok=True)
        obj.write_bytes(b"ELF placeholder")
        generated = self.GENERATED_O22
        candidate = self.BoundaryElf(
            names=["", ".text"],
            symbols=[(generated, 0, 0, 0, rs.SHN_UNDEF)],
            relocations=[(
                ".text", 0x274,
                rs.R_MIPS_26 if rtype is None else rtype,
                0,
            )],
            text=b"\0" * 0x2B0,
        )
        canonical = self.BoundaryElf(
            path=obj,
            names=["", ".text"],
            # Metadata keeps the pre-trim size; physical .text is the exact
            # atlas-owned function boundary.
            symbols=[(generated, 0, 0x1B0, rs.STT_FUNC, 1)],
            text=b"\0" * 0x16C,
        )
        target = self.BoundaryElf(
            names=["", ".overlay_022"],
            symbols=[(
                generated,
                rs.SYNTHETIC_VMA + 0xD30 if linked_value is None else linked_value,
                0x1B0,
                rs.STT_FUNC,
                1,
            )],
        )
        return candidate, canonical, target, source, obj

    def test_o22_generated_call_uses_unique_owned_function_boundary(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            resolved, ambiguous = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt",
                candidate,
                22,
                target,
                self.o22_atlas(),
                0,
                0x2B0,
                root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({self.GENERATED_O22: (22, 0xD30)}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_generated_call_accepts_named_function_in_multi_function_object(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            canonical._symbols.append(
                ("neighbor", 0x100, 0x20, rs.STT_FUNC, 1)
            )
            resolved, ambiguous = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt",
                candidate,
                22,
                target,
                self.o22_atlas(),
                0,
                0x2B0,
                root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({self.GENERATED_O22: (22, 0xD30)}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_generated_call_rejects_duplicate_named_function(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            canonical._symbols.append(
                (self.GENERATED_O22, 0, 0x1B0, rs.STT_FUNC, 1)
            )
            with self.assertRaisesRegex(
                rs.SurfaceComparisonError, "ambiguous function symbols"
            ):
                rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt",
                    candidate,
                    22,
                    target,
                    self.o22_atlas(),
                    0,
                    0x2B0,
                    root=root,
                    elf_loader=lambda _path: canonical,
                )

    def test_generated_call_rejects_ambiguous_ownership(self):
        owner = self.o22_atlas()["modules"][0]["text_ownership"][0]
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            with self.assertRaisesRegex(
                rs.SurfaceComparisonError, "ambiguous overlapping atlas owners"
            ):
                rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 22, target,
                    self.o22_atlas([dict(owner), dict(owner)]), 0, 0x2B0,
                    root=root, elf_loader=lambda _path: canonical,
                )

    def test_generated_call_rejects_cross_overlay_and_non_call(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            cross, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 21, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )
            non_call_candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root, rtype=rs.R_MIPS_HI16)
            )
            non_call, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", non_call_candidate, 22, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, cross)
        self.assertEqual({}, non_call)

    def test_generated_call_rejects_section_only_and_missing_boundary(self):
        broad = [{
            "offset": "0x0",
            "end_offset": "0xEA0",
            "size": "0xEA0",
            "type": "c",
            "source": "overlays/o022/overlay_022",
        }]
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(root)
            )
            section_only, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas(broad), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )
            missing, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas([]), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, section_only)
        self.assertEqual({}, missing)

    def test_generated_call_rejects_stale_object(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, source, obj = (
                self.o22_boundary_fixture(root)
            )
            newer = obj.stat().st_mtime + 5
            os.utime(source, (newer, newer))
            resolved, _ = rs._stable_overlay_call_identities(
                root / "missing-aliases.txt", candidate, 22, target,
                self.o22_atlas(), 0, 0x2B0, root=root,
                elf_loader=lambda _path: canonical,
            )

        self.assertEqual({}, resolved)

    def test_generated_call_refuses_conflicting_linked_symbol(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            candidate, canonical, target, _source, _obj = (
                self.o22_boundary_fixture(
                    root, linked_value=rs.SYNTHETIC_VMA + 0xE00
                )
            )
            with self.assertRaisesRegex(
                rs.SurfaceComparisonError,
                "linked symbol conflicts with canonical overlay ownership",
            ):
                rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 22, target,
                    self.o22_atlas(), 0, 0x2B0, root=root,
                    elf_loader=lambda _path: canonical,
                )

    def test_redefine_alias_propagates_stable_identity(self):
        class FakeElf:
            def __init__(self, target=False):
                self.target = target

            def section(self, name):
                self.assert_name = name
                return 1, object()

            def symbols(self):
                old = (
                    ("func_80005750", 0x80005750, 4, 0, 1)
                    if self.target
                    else ("func_80005750", 0, 0, 0, rs.SHN_UNDEF)
                )
                return [
                    old,
                    ("func_80005750_o001Reloc", 0, 0, 0, rs.SHN_UNDEF),
                ]

        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"), FakeElf(), 1, 0x3578, FakeElf(target=True),
            {"func_80005750_o001Reloc": "func_80005750"},
        )
        self.assertEqual(
            resolved["func_80005750_o001Reloc"], (0, 0x5300),
        )
        self.assertNotIn("func_80005750_o001Reloc", ambiguous)

    def test_resident_absolute_symbol_uses_absolute_identity_namespace(self):
        class FakeElf:
            def __init__(self, target=False):
                self.target = target

            def symbols(self):
                return [
                    ("D_7BE08", 0x7BE08 if self.target else 0,
                     0, 0, rs.SHN_ABS if self.target else rs.SHN_UNDEF),
                ]

            def section(self, _name):
                return None, None

        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"), FakeElf(), None, 0, FakeElf(target=True)
        )
        self.assertEqual(
            (rs.ri.ABSOLUTE_IDENTITY, 0x7BE08), resolved["D_7BE08"]
        )
        self.assertNotIn("D_7BE08", ambiguous)

    def test_section_symbols_are_never_program_wide_identities(self):
        # An externalized overlay `.rodata` survives in the linked ELF as an
        # `*ABS*` zero section symbol; func_8005BA40's own `.rodata` jump
        # table must not borrow it.
        class FakeElf:
            def __init__(self, rows):
                self.rows = rows

            def symbols(self):
                return self.rows

            def section(self, _name):
                return None, None

        candidate = FakeElf([(".rodata", 0, 0, rs.STT_SECTION, 5)])
        target = FakeElf([(".rodata", 0, 0, rs.STT_SECTION, rs.SHN_ABS),
                          (".main", 0x80000450, 0, rs.STT_SECTION, 1)])
        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"), candidate, None, 0, target)
        self.assertNotIn(".rodata", resolved)
        self.assertNotIn(".main", resolved)

    def test_resident_object_symbols_are_placed_by_the_link_map(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            link_map = root / "link.map"
            link_map.write_text(
                " build/src/main/a.c.o(.rodata)\n"
                " .rodata        0x80084320      0x150 build/src/main/a.c.o\n"
                " .bss           0x800cf3b0       0x10 build/src/main/a.c.o\n"
                " .rodata        0x80084470       0x20 build/src/main/b.c.o\n")

            class Obj:
                names = ["", ".text", ".rodata", ".bss"]
                sh = [(0,) * 10, (0,) * 10, (0, 1, 2, 0, 0, 0x150, 0, 0, 4, 0),
                      (0, 8, 3, 0, 0, 0x10, 0, 0, 4, 0)]

                def symbols(self):
                    # joy.c.o: a local carrier and a global alias share a name.
                    return [("", 0, 0, 0, 0),
                            (".rodata", 0, 0, rs.STT_SECTION, 2),
                            ("D_800CF3B5", 4, 0, rs.STT_OBJECT, 3),
                            ("D_800CF3B5", 5, 0, 0x10 | rs.STT_OBJECT, 3),
                            ("extern", 0, 0, 0x10, 0),
                            ("D_800D3044", 8, 4, 0x20 | rs.STT_OBJECT, 3)]

            base = rs.ot.RESIDENT_VRAM_BASE
            obj_path = rs.REPO / "build/src/main/a.c.o"
            # Without the linked ELF, globals and weaks are not placed.
            self.assertEqual({0: (rs.ri.ABSOLUTE_IDENTITY, 0), 1: (0, 0x80084320 - base),
                              2: (0, 0x800CF3B4 - base)},
                             rs._resident_object_identities(Obj(), obj_path, link_map))

            class Linked:
                def symbols(self):
                    # The global alias links at its own slot; a linker-script
                    # assignment overrides the weak definition.
                    return [("D_800CF3B5", 0x800CF3B5, 0, 0x11, 6),
                            ("D_800CF3B5", 0x800CF3B4, 0, 0x01, 6),
                            ("D_800D3044", 0x800D3044, 0, 0x10, 0xFFF1)]

            self.assertEqual({0: (rs.ri.ABSOLUTE_IDENTITY, 0), 1: (0, 0x80084320 - base),
                              2: (0, 0x800CF3B4 - base), 3: (0, 0x800CF3B5 - base)},
                             rs._resident_object_identities(Obj(), obj_path, link_map,
                                                            linked_elf=Linked()))
            # An object the map does not place keeps only the null symbol.
            self.assertEqual({0: (rs.ri.ABSOLUTE_IDENTITY, 0)}, rs._resident_object_identities(
                Obj(), rs.REPO / "build/src/main/c.c.o", link_map))
            # A size disagreement is a contradiction, not a guess.
            Obj.sh[2] = (0, 1, 2, 0, 0, 0x154, 0, 0, 4, 0)
            with self.assertRaises(rs.SurfaceComparisonError):
                rs._resident_object_identities(Obj(), obj_path, link_map)
            link_map.write_text(link_map.read_text()
                                + " .rodata        0x80090000      0x150 build/src/main/a.c.o\n")
            with self.assertRaises(rs.SurfaceComparisonError):
                rs.linked_input_sections(link_map)

    def test_hilo_pairs_on_the_symbol_entry_not_the_name(self):
        class Obj:
            def symbols(self):
                return [("", 0, 0, 0, 0), ("dup", 4, 0, 1, 3), ("dup", 5, 0, 0x11, 3)]

            def relocations(self, target=r"\.text"):
                # HI(entry 1), HI(entry 2), LO(entry 2), LO(entry 1)
                return [(".text", 0, rs.R_MIPS_HI16, 1), (".text", 4, rs.R_MIPS_HI16, 2),
                        (".text", 8, rs.R_MIPS_LO16, 2), (".text", 12, rs.R_MIPS_LO16, 1)]

            def section_bytes(self, _name):
                return bytes.fromhex("3c010000" "3c020000" "24420001" "24210002")

        records = rs._candidate_surface_records(
            Obj(), 0, 16, [], {}, {}, set(), None,
            index_identities={1: (0, 0x100), 2: (0, 0x200)})
        self.assertEqual([(0, (0, 0x102)), (4, (0, 0x201)), (8, (0, 0x201)), (12, (0, 0x102))],
                         [(row.offset, row.identity) for row in records])

    def test_transitive_redefine_alias_propagates_stable_identity(self):
        class FakeElf:
            def __init__(self, target=False):
                self.target = target

            def section(self, _name):
                return 1, object()

            def symbols(self):
                original = (
                    ("func_80005750", 0x80005750, 4, 0, 1)
                    if self.target
                    else ("func_80005750", 0, 0, 0, rs.SHN_UNDEF)
                )
                return [
                    original,
                    ("middle", 0, 0, 0, rs.SHN_UNDEF),
                    ("final", 0, 0, 0, rs.SHN_UNDEF),
                ]

        resolved, ambiguous = rs._stable_symbol_identities(
            Path("missing"),
            FakeElf(),
            1,
            0x3578,
            FakeElf(target=True),
            {"middle": "func_80005750", "final": "func_80005750"},
        )
        self.assertEqual((0, 0x5300), resolved["final"])
        self.assertNotIn("final", ambiguous)

    def test_exact_surface_counts_shape_and_identity(self):
        target = [
            rs.SurfaceRecord(0x10, rs.R_MIPS_26, (0, 0x1234)),
            rs.SurfaceRecord(0x28, rs.R_MIPS_HI16, (7, 0x1BA8)),
            rs.SurfaceRecord(0x2C, rs.R_MIPS_LO16, (7, 0x1BA8)),
        ]
        result = rs.compare_record_sets(target, list(target))

        self.assertEqual(3, result["target_runtime_record_count"])
        self.assertEqual(3, result["candidate_record_count"])
        self.assertEqual(3, result["offset_type_alignment_count"])
        self.assertEqual(3, result["stable_identity_alignment_count"])
        self.assertTrue(result["offset_type_exact"])
        self.assertTrue(result["stable_identity_exact"])

    def test_shifted_surface_does_not_count_as_aligned(self):
        target = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (0, 0x1234))]
        candidate = [rs.SurfaceRecord(0x14, rs.R_MIPS_26, (0, 0x1234))]

        result = rs.compare_record_sets(target, candidate)

        self.assertEqual(0, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])
        self.assertFalse(result["offset_type_exact"])
        self.assertFalse(result["stable_identity_exact"])

    def test_wrong_identity_preserves_shape_but_not_identity(self):
        target = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (7, 0xCCC))]
        candidate = [rs.SurfaceRecord(0x10, rs.R_MIPS_26, (59, 0x70))]

        result = rs.compare_record_sets(target, candidate)

        self.assertEqual(1, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])
        self.assertTrue(result["offset_type_exact"])
        self.assertFalse(result["stable_identity_exact"])

    def test_ambiguous_overlay_owner_fails_closed(self):
        row = {
            "type": "c",
            "source": "overlays/o007/example",
            "offset": "0x0",
            "end_offset": "0x20",
            "size": "0x20",
        }
        atlas = {
            "modules": [
                {"overlay": 7, "text_ownership": [dict(row)]},
                {"overlay": 8, "text_ownership": [dict(row)]},
            ]
        }

        with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                    "2 overlay text owners"):
            rs.resolve_overlay_ownership(
                Path("build/src/overlays/o007/example.c.o"), atlas)


class OverlayDataIdentityTests(unittest.TestCase):
    class FakeElf:
        def __init__(self, path, names, symbols=(), relocations=(), text=b"",
                     section_headers=None):
            self.path = Path(path)
            self.names = list(names)
            self._symbols = list(symbols)
            self._relocations = list(relocations)
            self._text = bytes(text)
            self._section_headers = section_headers or {}

        def section(self, name):
            if name in self.names:
                return (self.names.index(name),
                        self._section_headers.get(name, object()))
            return None, None

        def section_bytes(self, name):
            return self._text if name == ".text" else b""

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def fixture(self, root, *, overlay=7, name="D_0", assignment=0,
                object_value=0, linked_offset=0xFC0, object_section=".data",
                linked_overlay=None, sites=None, stale_object=False,
                stale_link=False, extra_linked=(), linked_name=True):
        linked_overlay = overlay if linked_overlay is None else linked_overlay
        caller = "overlays/o%03d/caller" % overlay
        owner = "overlays/o%03d/owner" % overlay
        caller_source = root / "src" / (caller + ".c")
        owner_source = root / "src" / (owner + ".c")
        candidate_path = root / "build_non_matching/src" / (caller + ".c.o")
        owner_path = root / "build/src" / (owner + ".c.o")
        target_path = root / "build/mickey.us.elf"
        for path in (caller_source, owner_source, candidate_path,
                     owner_path, target_path):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"fixture")
        now = max(path.stat().st_mtime_ns for path in
                  (caller_source, owner_source, candidate_path,
                   owner_path, target_path))
        os.utime(caller_source, ns=(now, now))
        os.utime(owner_source, ns=(now, now))
        os.utime(candidate_path, ns=(now + 1, now + 1))
        os.utime(owner_path, ns=(now + 1, now + 1))
        os.utime(target_path, ns=(now + 2, now + 2))
        if stale_object:
            os.utime(owner_source, ns=(now + 3, now + 3))
        if stale_link:
            os.utime(target_path, ns=(now, now))

        if sites is None:
            sites = [(0, rs.R_MIPS_HI16), (4, rs.R_MIPS_LO16)]
        candidate = self.FakeElf(
            candidate_path, ["", ".text"],
            [(name, 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", offset, rtype, 0) for offset, rtype in sites],
            b"\0" * 0x40)
        canonical_name = name if linked_name else "gCanonicalStorage"
        canonical = self.FakeElf(
            owner_path, ["", object_section],
            [(canonical_name, object_value, 0, rs.STT_OBJECT, 1)],
            section_headers={
                object_section: (0, 0, 0, 0, 0,
                                 0x2A0 if object_section == ".bss" else 0x950,
                                 0, 0, 0, 0),
            })
        linked_section = ".overlay_%03d%s" % (
            linked_overlay, "_bss" if object_section == ".bss" else "")
        target = self.FakeElf(
            target_path, ["", linked_section] + [row[4] for row in extra_linked],
            [(name, assignment, 0, rs.STT_OBJECT | 0x10, rs.SHN_ABS)] +
            ([(name, rs.SYNTHETIC_VMA + linked_offset, 0,
               rs.STT_OBJECT, 1)] if linked_name else []) +
             [row[:4] + (2 + i,) for i, row in enumerate(extra_linked)],
            section_headers={
                linked_section: (
                    0, 0, 0,
                    rs.SYNTHETIC_VMA + (0x1ED0 if object_section == ".bss"
                                        else 0),
                    0, 0x2A0 if object_section == ".bss" else 0x1ED0,
                    0, 0, 0, 0),
            })
        module = {
            "overlay": overlay,
            "identity": "overlay:%d" % overlay,
            "synthetic_vma": "0xF0000000",
            "rom": {"size": "0x1ED0"},
            "bss_size": "0x2B0",
            "sections": {
                "text": {"size": "0xFC0"},
                "data_rodata": {"size": "0x950"},
            },
            "text_ownership": [
                {"type": "c", "source": caller},
                {"type": "c", "source": owner},
            ],
            "data_rodata_ownership": ([{
                "offset": "0x0", "end_offset": "0x950", "size": "0x950",
                "type": "c", "section": object_section, "source": owner,
            }] if object_section != ".bss" else None),
        }
        values = root / "values.txt"
        values.write_text("%s = 0x%X;\n" % (name, assignment))
        return candidate, canonical, target, module, values

    def resolve(self, fixture, *, loader=None):
        candidate, canonical, target, module, values = fixture
        return rs._stable_overlay_data_identities(
            values, candidate, module, target, 0, 0x40,
            root=values.parent,
            elf_loader=loader or (lambda _path: canonical))

    def test_generic_data_pair_authenticates_without_hand_alias(self):
        with tempfile.TemporaryDirectory() as td:
            resolved, ambiguous = self.resolve(self.fixture(Path(td)))
        self.assertEqual({"D_0": (7, 0xFC0)}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_multiple_hilo_references_share_candidate_side_identity(self):
        sites = [(0, rs.R_MIPS_HI16), (4, rs.R_MIPS_LO16),
                 (8, rs.R_MIPS_HI16), (12, rs.R_MIPS_LO16)]
        with tempfile.TemporaryDirectory() as td:
            resolved, _ = self.resolve(self.fixture(Path(td), sites=sites))
        self.assertEqual((7, 0xFC0), resolved["D_0"])

    def test_unpaired_hilo_is_not_authenticated(self):
        with tempfile.TemporaryDirectory() as td:
            resolved, _ = self.resolve(self.fixture(
                Path(td), sites=[(0, rs.R_MIPS_HI16)]))
        self.assertEqual({}, resolved)

    def test_bss_definition_uses_exact_overlay_offset(self):
        with tempfile.TemporaryDirectory() as td:
            resolved, _ = self.resolve(self.fixture(
                Path(td), name="gPool", assignment=0x284,
                object_value=0x284, linked_offset=0x2154,
                object_section=".bss"))
        self.assertEqual((7, 0x1B94), resolved["gPool"])

    def test_friendly_name_without_linked_alias_uses_unique_whole_bss(self):
        with tempfile.TemporaryDirectory() as td:
            resolved, _ = self.resolve(self.fixture(
                Path(td), name="gFriendlyState", assignment=0x11C,
                object_value=0x11C, object_section=".bss",
                linked_name=False))
        self.assertEqual((7, 0x1A2C), resolved["gFriendlyState"])

    def test_same_numeric_assignment_in_two_overlays_uses_caller_context(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            fixture = self.fixture(root, name="D_0")
            candidate, canonical, target, module, values = fixture
            target.names.append(".overlay_008")
            target._symbols.append(("D_0", rs.SYNTHETIC_VMA + 0xFC0, 0,
                                    rs.STT_OBJECT, 2))
            resolved, _ = self.resolve(
                (candidate, canonical, target, module, values))
        self.assertEqual((7, 0xFC0), resolved["D_0"])

    def test_ambiguous_same_overlay_aliases_fail_closed(self):
        with tempfile.TemporaryDirectory() as td:
            fixture = self.fixture(Path(td), name="D_0")
            fixture[2]._symbols.append(
                ("D_0", rs.SYNTHETIC_VMA + 0xFC4, 0, rs.STT_OBJECT, 1))
            with self.assertRaisesRegex(
                    rs.SurfaceComparisonError, "ambiguous overlay definitions"):
                self.resolve(fixture)

    def test_cross_overlay_definition_is_not_used(self):
        with tempfile.TemporaryDirectory() as td:
            resolved, _ = self.resolve(self.fixture(
                Path(td), linked_overlay=8))
        self.assertEqual({}, resolved)

    def test_stale_object_or_linked_elf_is_not_used(self):
        with tempfile.TemporaryDirectory() as td:
            stale_object, _ = self.resolve(self.fixture(
                Path(td) / "object", stale_object=True))
            stale_link, _ = self.resolve(self.fixture(
                Path(td) / "link", stale_link=True))
        self.assertEqual({}, stale_object)
        self.assertEqual({}, stale_link)

    def test_conflicting_assignment_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            fixture = self.fixture(Path(td), assignment=4, object_value=0)
            with self.assertRaisesRegex(
                    rs.SurfaceComparisonError, "canonical object"):
                self.resolve(fixture)

    def test_overlay_numeric_value_never_borrows_target_identity(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("D_0", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0)], b"\0" * 8)
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC0), 0),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC0), 0),
        ]
        records = rs._candidate_surface_records(
            candidate, 0, 8, target, {}, {"D_0": 0}, set(), 7)
        self.assertTrue(all(record.identity is None for record in records))

    def test_authenticated_hilo_applies_signed_rel_addend(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("D_base", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0)],
            struct.pack(">II", 0, 0xFFFC))
        records = rs._candidate_surface_records(
            candidate, 0, 8, [], {"D_base": (7, 0x1000)}, {}, set(), 7)
        self.assertEqual([(7, 0xFFC), (7, 0xFFC)],
                         [record.identity for record in records])

    def test_ambiguous_hilo_identity_remains_structurally_visible(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("D_base", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0)], b"\0" * 8)
        records = rs._candidate_surface_records(
            candidate, 0, 8, [], {"D_base": (7, 0x1000)}, {},
            {"D_base"}, 7)
        self.assertEqual([None, None], [record.identity for record in records])

    def test_ambiguous_overlay_call_remains_structurally_visible(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("overlayCall", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_26, 0)], b"\0" * 4)
        records = rs._candidate_surface_records(
            candidate, 0, 4, [], {}, {}, set(), 7,
            {"overlayCall": (7, 0x200)}, {"overlayCall"})
        self.assertEqual([None], [record.identity for record in records])

    def test_call_name_shape_conflict_leaves_site_unresolved(self):
        # A resident call stores a zero jump field, which is also the field of
        # a call to the overlay's offset-0 function, so the C can carry the
        # generated `func_overlay_007_F0000000_*` name for a resident callee.
        # The name's own shape says (7, 0); an exact sibling's runtime tuple
        # says resident. Neither may be chosen, and the disagreement must not
        # abort the whole comparison: the site stays unresolved.
        name = "func_overlay_007_F0000000_0000100"
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [(name, 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_26, 0)], b"\0" * 4)
        records = rs._candidate_surface_records(
            candidate, 0, 4, [], {name: (7, 0)}, {}, set(), 7,
            {name: (0, 0x1234)}, set())
        self.assertEqual([None], [record.identity for record in records])
        agreeing = rs._candidate_surface_records(
            candidate, 0, 4, [], {name: (0, 0x1234)}, {}, set(), 7,
            {name: (0, 0x1234)}, set())
        self.assertEqual([(0, 0x1234)],
                         [record.identity for record in agreeing])

    @staticmethod
    def runtime_module(overlay):
        return {
            "overlay": overlay,
            "identity": "overlay:%d" % overlay,
            "synthetic_vma": "0xF0000000",
        }

    def test_runtime_correlated_local_pair_resolves_o29_proxy(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("gOverlay29MinimumYReloc", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0x20, rs.R_MIPS_HI16, 0),
             (".text", 0x24, rs.R_MIPS_LO16, 0)],
            b"\0" * 0x28)
        target = [
            rs.SurfaceRecord(0x20, rs.R_MIPS_HI16, (29, 0x16E4), 20),
            rs.SurfaceRecord(0x24, rs.R_MIPS_LO16, (29, 0x16E4), 20),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(29), 0, 0x28, target)

        self.assertEqual({"gOverlay29MinimumYReloc": (29, 0x16E4)}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_runtime_correlated_pairs_resolve_o41_proxy_addends(self):
        symbols = [
            ("gQueueActive", 0, 0, 0, rs.SHN_UNDEF),
            ("gQueueEntries", 0, 0, 0, rs.SHN_UNDEF),
        ]
        text = bytearray(0x10)
        struct.pack_into(">I", text, 0x4, 11)
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"], symbols,
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0),
             (".text", 8, rs.R_MIPS_HI16, 1),
             (".text", 12, rs.R_MIPS_LO16, 1)],
            bytes(text))
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (0xFFF, 0x512E3), 11),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (0xFFF, 0x512E3), 11),
            rs.SurfaceRecord(8, rs.R_MIPS_HI16, (0xFFF, 0x512E4), 12),
            rs.SurfaceRecord(12, rs.R_MIPS_LO16, (0xFFF, 0x512E4), 12),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(41), 0, 0x10, target)

        self.assertEqual(
            {"gQueueActive": (0xFFF, 0x512D8),
             "gQueueEntries": (0xFFF, 0x512E4)},
            resolved)
        self.assertEqual(set(), ambiguous)

    def test_shared_vma_overlay_identities_remain_distinct(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("gOverlay7", 0, 0, 0, rs.SHN_UNDEF),
             ("gOverlay8", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0),
             (".text", 8, rs.R_MIPS_HI16, 1),
             (".text", 12, rs.R_MIPS_LO16, 1)],
            b"\0" * 0x10)
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC0), 0),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC0), 0),
            rs.SurfaceRecord(8, rs.R_MIPS_HI16, (8, 0xFC0), 0),
            rs.SurfaceRecord(12, rs.R_MIPS_LO16, (8, 0xFC0), 0),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(7), 0, 0x10, target)

        self.assertEqual((7, 0xFC0), resolved["gOverlay7"])
        self.assertEqual((8, 0xFC0), resolved["gOverlay8"])
        self.assertEqual(set(), ambiguous)

    def test_conflicting_overlay_identities_for_one_proxy_fail_closed(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("gAmbiguous", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0),
             (".text", 8, rs.R_MIPS_HI16, 0),
             (".text", 12, rs.R_MIPS_LO16, 0)],
            b"\0" * 0x10)
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC0), 0),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC0), 0),
            rs.SurfaceRecord(8, rs.R_MIPS_HI16, (8, 0xFC0), 0),
            rs.SurfaceRecord(12, rs.R_MIPS_LO16, (8, 0xFC0), 0),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(7), 0, 0x10, target)

        self.assertNotIn("gAmbiguous", resolved)
        self.assertIn("gAmbiguous", ambiguous)

    def test_defined_candidate_symbol_requires_canonical_section_owner(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text", ".data"],
            [("gCandidateData", 0, 4, rs.STT_OBJECT, 2)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0)],
            b"\0" * 8)
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC0), 0),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC0), 0),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(7), 0, 8, target)

        self.assertEqual({}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_runtime_correlated_proxy_keeps_pre_objcopy_name(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("D_current", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0, rs.R_MIPS_HI16, 0),
             (".text", 4, rs.R_MIPS_LO16, 0)],
            b"\0" * 8)
        target = [
            rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC0), 0),
            rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC0), 0),
        ]

        resolved, ambiguous = rs._runtime_correlated_overlay_hilo_identities(
            candidate, self.runtime_module(7), 0, 8, target,
            {"D_current": "gOriginalProxy"})

        self.assertEqual({"gOriginalProxy": (7, 0xFC0)}, resolved)
        self.assertEqual(set(), ambiguous)


class RepeatedOverlayCallIdentityTests(unittest.TestCase):
    FakeElf = OverlayDataIdentityTests.FakeElf

    @staticmethod
    def module(overlay=101):
        return {
            "overlay": overlay,
            "identity": "overlay:%d" % overlay,
            "synthetic_vma": "0xF0000000",
        }

    def candidate(self, *, sites=(0x3C, 0x6C), words=None,
                  shndx=rs.SHN_UNDEF, extra_relocations=()):
        size = max(sites + tuple(row[1] for row in extra_relocations)) + 4
        text = bytearray(size)
        for offset, word in (words or {}).items():
            struct.pack_into(">I", text, offset, word)
        return self.FakeElf(
            Path("missing"), ["", ".text", ".data"],
            [("overlay101BuildIntensityColorsReloc", 0, 0, 0, shndx)],
            ([('.text', offset, rs.R_MIPS_26, 0) for offset in sites]
             + list(extra_relocations)),
            bytes(text))

    @staticmethod
    def target(sites=(0x3C, 0x6C), identities=None):
        identities = identities or [(101, 0x2CE4)] * len(sites)
        return [
            rs.SurfaceRecord(offset, rs.R_MIPS_26, identity, 0)
            for offset, identity in zip(sites, identities)
        ]

    def test_repeated_o101_proxy_resolves_one_same_overlay_base(self):
        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70, self.target()))

        self.assertEqual(
            {"overlay101BuildIntensityColorsReloc": (101, 0x2CE4)},
            resolved)
        self.assertEqual(set(), ambiguous)

    def test_each_candidate_rel_addend_derives_same_base(self):
        candidate = self.candidate(
            words={0x3C: 1, 0x6C: 2})
        target = self.target(
            identities=[(101, 0x2CE8), (101, 0x2CEC)])

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                candidate, self.module(), 0, 0x70, target))

        self.assertEqual((101, 0x2CE4),
                         resolved["overlay101BuildIntensityColorsReloc"])
        self.assertEqual(set(), ambiguous)

    def test_single_site_is_not_an_independent_witness(self):
        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(sites=(0x3C,)), self.module(), 0, 0x40,
                self.target(sites=(0x3C,))))

        self.assertEqual({}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_conflicting_repeated_bases_are_ambiguous(self):
        target = self.target(
            identities=[(101, 0x2CE4), (101, 0x2CF0)])

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70, target))

        self.assertEqual({}, resolved)
        self.assertIn("overlay101BuildIntensityColorsReloc", ambiguous)

    def test_duplicate_runtime_shape_is_ambiguous(self):
        target = self.target()
        target.append(rs.SurfaceRecord(
            0x3C, rs.R_MIPS_26, (101, 0x2CE4), 0))

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70, target))

        self.assertEqual({}, resolved)
        self.assertIn("overlay101BuildIntensityColorsReloc", ambiguous)

    def test_cross_overlay_runtime_tuple_is_unresolved(self):
        target = self.target(
            identities=[(101, 0x2CE4), (102, 0x2CE4)])

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70, target))

        self.assertEqual({}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_nonzero_runtime_call_addend_is_ambiguous(self):
        target = self.target()
        target[1] = rs.SurfaceRecord(
            0x6C, rs.R_MIPS_26, (101, 0x2CE4), 4)

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70, target))

        self.assertEqual({}, resolved)
        self.assertIn("overlay101BuildIntensityColorsReloc", ambiguous)

    def test_defined_or_mixed_type_proxy_is_not_runtime_correlated(self):
        defined, defined_ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(shndx=2), self.module(), 0, 0x70,
                self.target()))
        mixed_candidate = self.candidate(
            extra_relocations=((".text", 0x70, rs.R_MIPS_HI16, 0),))
        mixed, mixed_ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                mixed_candidate, self.module(), 0, 0x74, self.target()))

        self.assertEqual(({}, set()), (defined, defined_ambiguous))
        self.assertEqual(({}, set()), (mixed, mixed_ambiguous))

    def test_missing_aligned_runtime_tuple_remains_unresolved(self):
        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                self.candidate(), self.module(), 0, 0x70,
                self.target(sites=(0x3C,))))

        self.assertEqual({}, resolved)
        self.assertEqual(set(), ambiguous)

    def test_call_identity_pipeline_uses_repeated_runtime_witness(self):
        candidate = self.candidate()
        empty_target = self.FakeElf(Path("missing"), [""], [], [], b"")

        resolved, ambiguous = rs._stable_overlay_call_identities(
            Path("missing"), candidate, 101, empty_target, {}, 0, 0x70,
            module=self.module(), target_records=self.target())

        self.assertEqual(
            {"overlay101BuildIntensityColorsReloc": (101, 0x2CE4)},
            resolved)
        self.assertEqual(set(), ambiguous)

    def test_call_identity_pipeline_rejects_cross_overlay_owner(self):
        candidate = self.candidate()
        empty_target = self.FakeElf(Path("missing"), [""], [], [], b"")

        with self.assertRaisesRegex(
                rs.SurfaceComparisonError, "owner conflicts"):
            rs._stable_overlay_call_identities(
                Path("missing"), candidate, 100, empty_target, {}, 0, 0x70,
                module=self.module(), target_records=self.target())

    def test_o101_draw_mixed_groups_resolve_local_and_leave_resident_partial(self):
        symbols = [
            ("overlay101SetScissor2Reloc", 0, 0, 0, rs.SHN_UNDEF),
            ("overlay101SetTransformModeReloc", 0, 0, 0, rs.SHN_UNDEF),
        ]
        sites = [(0x1B8, 0), (0x1DC, 1), (0x22C, 1), (0x27C, 0)]
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"], symbols,
            [(".text", offset, rs.R_MIPS_26, index)
             for offset, index in sites],
            b"\0" * 0x280)
        target = [
            rs.SurfaceRecord(0x1B8, rs.R_MIPS_26, (101, 0x1F80), 0),
            rs.SurfaceRecord(0x1DC, rs.R_MIPS_26, (0, 0x34994), 0),
            rs.SurfaceRecord(0x22C, rs.R_MIPS_26, (0, 0x34994), 0),
            rs.SurfaceRecord(0x27C, rs.R_MIPS_26, (101, 0x1F80), 0),
        ]
        empty_target = self.FakeElf(Path("missing"), [""], [], [], b"")

        resolved, ambiguous = rs._stable_overlay_call_identities(
            Path("missing"), candidate, 101, empty_target, {}, 0, 0x280,
            module=self.module(), target_records=target)
        records = rs._candidate_surface_records(
            candidate, 0, 0x280, target, {}, {}, set(), 101,
            resolved, ambiguous)

        self.assertEqual(
            {"overlay101SetScissor2Reloc": (101, 0x1F80)}, resolved)
        self.assertEqual(set(), ambiguous)
        self.assertEqual(2, sum(record.identity is not None for record in records))
        self.assertEqual(2, sum(record.identity is None for record in records))

    def test_o3_repeated_resident_proxy_remains_partial_not_ambiguous(self):
        candidate = self.FakeElf(
            Path("missing"), ["", ".text"],
            [("overlay3RandomRangeReloc", 0, 0, 0, rs.SHN_UNDEF)],
            [(".text", 0x11C, rs.R_MIPS_26, 0),
             (".text", 0x15C, rs.R_MIPS_26, 0)],
            b"\0" * 0x160)
        target = [
            rs.SurfaceRecord(0x11C, rs.R_MIPS_26, (0, 0x2952C), 0),
            rs.SurfaceRecord(0x15C, rs.R_MIPS_26, (0, 0x2952C), 0),
        ]

        resolved, ambiguous = (
            rs._runtime_correlated_overlay_r26_identities(
                candidate, self.module(3), 0, 0x160, target))
        records = rs._candidate_surface_records(
            candidate, 0, 0x160, target, {}, {}, set(), 3,
            resolved, ambiguous)

        self.assertEqual({}, resolved)
        self.assertEqual(set(), ambiguous)
        self.assertTrue(all(record.identity is None for record in records))


class MatchedOverlayRelocationWitnessTests(unittest.TestCase):
    class FakeElf:
        def __init__(self, path, names, symbols=(), relocations=(), sections=None):
            self.path = Path(path)
            self.names = list(names)
            self._symbols = list(symbols)
            self._relocations = list(relocations)
            self._sections = dict(sections or {})

        def section(self, name):
            if name not in self.names:
                return None, None
            data = self._sections.get(name, b"")
            return self.names.index(name), (0, 0, 0, 0, 0, len(data), 0, 0, 0, 0)

        def section_bytes(self, name):
            return self._sections.get(name, b"")

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def fixture(self, root, *, matched=True, owners=1):
        overlay = 7
        rom_start = 0x100
        row_size = 0x20
        linked_bytes = b"\0" * (row_size * owners)
        rom = bytearray(rom_start + len(linked_bytes))
        rom[rom_start:] = linked_bytes
        rows = []
        canonicals = {}
        linked_symbols = []
        for index in range(owners):
            source = "overlays/o007/witness%d" % index
            source_path = root / "src" / (source + ".c")
            object_path = root / "build" / "src" / (source + ".c.o")
            source_path.parent.mkdir(parents=True, exist_ok=True)
            object_path.parent.mkdir(parents=True, exist_ok=True)
            source_path.write_text("void witness%d(void) {}\n" % index)
            object_path.write_bytes(b"ELF fixture")
            now = max(source_path.stat().st_mtime_ns,
                      object_path.stat().st_mtime_ns)
            os.utime(source_path, ns=(now, now))
            os.utime(object_path, ns=(now + 1, now + 1))
            function = "witness%d" % index
            canonical = self.FakeElf(
                object_path, ["", ".text"],
                symbols=[
                    ("gSharedProxy", 0, 0, 0, rs.SHN_UNDEF),
                    (function, 0, row_size, rs.STT_FUNC, 1),
                ],
                relocations=[
                    (".text", 0x8, rs.R_MIPS_HI16, 0),
                    (".text", 0xC, rs.R_MIPS_LO16, 0),
                ],
                sections={".text": b"\0" * row_size},
            )
            canonicals[object_path] = canonical
            start = index * row_size
            rows.append({
                "offset": "0x%X" % start,
                "end_offset": "0x%X" % (start + row_size),
                "size": "0x%X" % row_size,
                "type": "c",
                "source": source,
                "matched": matched,
                "nonmatching": False,
            })
            linked_symbols.append((
                function, rs.SYNTHETIC_VMA + start, row_size,
                rs.STT_FUNC, 1,
            ))
        target_path = root / "build/mickey.us.elf"
        target_path.parent.mkdir(parents=True, exist_ok=True)
        target_path.write_bytes(b"ELF fixture")
        newest = max(path.stat().st_mtime_ns for path in canonicals)
        os.utime(target_path, ns=(newest + 1, newest + 1))
        target = self.FakeElf(
            target_path, ["", ".overlay_007"],
            symbols=linked_symbols,
            sections={".overlay_007": linked_bytes},
        )
        module = {
            "overlay": overlay,
            "identity": "overlay:7",
            "synthetic_vma": "0xF0000000",
            "rom": {"start": "0x100", "size": "0x%X" % len(linked_bytes)},
            "sections": {"text": {"size": "0x%X" % len(linked_bytes)}},
            "text_ownership": rows,
        }
        runtime_module = {"overlay": overlay, "rom_start": rom_start}
        return bytes(rom), module, runtime_module, target, canonicals

    def call_fixture(self, root, *, matched=True, owners=1):
        fixture = self.fixture(root, matched=matched, owners=owners)
        for canonical in fixture[-1].values():
            canonical._relocations = [
                (".text", 0x8, rs.R_MIPS_26, 0),
            ]
        return fixture

    def test_exact_function_sized_sibling_authenticates_proxy_name(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = self.fixture(root)
            records = [
                rs.SurfaceRecord(0x8, rs.R_MIPS_HI16, (7, 0xFC0)),
                rs.SurfaceRecord(0xC, rs.R_MIPS_LO16, (7, 0xFC0)),
            ]
            with mock.patch.object(rs, "_target_runtime_records",
                                   return_value=records):
                witnessed = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path])
        self.assertEqual({(7, 0xFC0)}, witnessed["gSharedProxy"])

    def test_nonmatched_owner_is_not_a_witness(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = self.fixture(
                root, matched=False)
            with mock.patch.object(rs, "_target_runtime_records") as runtime:
                witnessed = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path])
        self.assertEqual({}, witnessed)
        runtime.assert_not_called()

    def test_linked_rom_mismatch_is_not_a_witness(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = self.fixture(root)
            mismatched_rom = bytearray(rom)
            mismatched_rom[0x100] = 1
            with mock.patch.object(rs, "_target_runtime_records") as runtime:
                witnessed = rs._matched_overlay_relocation_witnesses(
                    module, target, bytes(mismatched_rom), runtime_module,
                    {"gSharedProxy"}, root=root,
                    elf_loader=lambda path: canonicals[path])
        self.assertEqual({}, witnessed)
        runtime.assert_not_called()

    def test_conflicting_exact_witnesses_remain_ambiguous(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = self.fixture(
                root, owners=2)
            values = root / "values.txt"
            values.write_text("gSharedProxy = 0x0;\n")
            candidate = self.FakeElf(
                root / "build_non_matching/src/overlays/o007/caller.c.o",
                ["", ".text"],
                symbols=[("gSharedProxy", 0, 0, 0, rs.SHN_UNDEF)],
                relocations=[
                    (".text", 0, rs.R_MIPS_HI16, 0),
                    (".text", 4, rs.R_MIPS_LO16, 0),
                ],
                sections={".text": b"\0" * 8},
            )

            def runtime_records(_rom, _context, start, _size):
                identity = (7, 0xFC0 if start == 0 else 0x1FC0)
                return [
                    rs.SurfaceRecord(0x8, rs.R_MIPS_HI16, identity),
                    rs.SurfaceRecord(0xC, rs.R_MIPS_LO16, identity),
                ]

            with mock.patch.object(rs, "_target_runtime_records",
                                   side_effect=runtime_records):
                resolved, ambiguous = rs._stable_overlay_data_identities(
                    values, candidate, module, target, 0, 8,
                    root=root, elf_loader=lambda path: canonicals[path],
                    rom=rom, runtime_module=runtime_module)
        self.assertNotIn("gSharedProxy", resolved)
        self.assertIn("gSharedProxy", ambiguous)

    def test_witness_identity_does_not_align_shifted_offset(self):
        identity = (7, 0xFC0)
        result = rs.compare_record_sets(
            [rs.SurfaceRecord(0x8, rs.R_MIPS_HI16, identity)],
            [rs.SurfaceRecord(0xC, rs.R_MIPS_HI16, identity)],
        )
        self.assertEqual(0, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])

    def test_exact_sibling_authenticates_call_proxy_name(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = (
                self.call_fixture(root)
            )
            object_text = bytearray(0x20)
            struct.pack_into(">I", object_text, 0x8, 1)
            next(iter(canonicals.values()))._sections[".text"] = bytes(object_text)
            records = [
                rs.SurfaceRecord(0x8, rs.R_MIPS_26, (0, 0x1234)),
            ]
            with mock.patch.object(rs, "_target_runtime_records",
                                   return_value=records):
                witnessed = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path])
        self.assertEqual({(0, 0x1230)}, witnessed["gSharedProxy"])

    def test_nonmatched_sibling_does_not_authenticate_call_proxy(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = (
                self.call_fixture(root, matched=False)
            )
            with mock.patch.object(rs, "_target_runtime_records") as runtime:
                witnessed = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path])
        self.assertEqual({}, witnessed)
        runtime.assert_not_called()

    def test_conflicting_call_proxy_witnesses_remain_ambiguous(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = (
                self.call_fixture(root, owners=2)
            )
            candidate = self.FakeElf(
                root / "build_non_matching/src/overlays/o007/caller.c.o",
                ["", ".text"],
                symbols=[("gSharedProxy", 0, 0, 0, rs.SHN_UNDEF)],
                relocations=[(".text", 0, rs.R_MIPS_26, 0)],
                sections={".text": b"\0" * 8},
            )

            def runtime_records(_rom, _context, start, _size):
                identity = (0, 0x1234 if start == 0 else 0x5678)
                return [rs.SurfaceRecord(0x8, rs.R_MIPS_26, identity)]

            with mock.patch.object(rs, "_target_runtime_records",
                                   side_effect=runtime_records):
                resolved, ambiguous = rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 7, target,
                    {"modules": []}, 0, 8, root=root,
                    elf_loader=lambda path: canonicals[path], module=module,
                    rom=rom, runtime_module=runtime_module)
        self.assertNotIn("gSharedProxy", resolved)
        self.assertIn("gSharedProxy", ambiguous)

    def test_function_under_proof_is_not_its_own_witness(self):
        # Re-proving an already promoted function finds its own canonical
        # object among the matched siblings. Its call tuple is the target's
        # tuple at the same site, so it would "witness" whatever name the
        # candidate happens to use -- even a generated overlay name whose own
        # identity is a different function. That is the circular one-site
        # copy the runtime-correlated route refuses; it must not re-enter
        # through the sibling route.
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = (
                self.call_fixture(root)
            )
            records = [rs.SurfaceRecord(0x8, rs.R_MIPS_26, (0, 0x1234))]
            with mock.patch.object(rs, "_target_runtime_records",
                                   return_value=records) as runtime:
                own = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path],
                    exclude_range=(0x4, 0x10))
                other = rs._matched_overlay_relocation_witnesses(
                    module, target, rom, runtime_module, {"gSharedProxy"},
                    root=root, elf_loader=lambda path: canonicals[path],
                    exclude_range=(0x20, 0x40))
        self.assertEqual({}, own)
        self.assertEqual({(0, 0x1234)}, other["gSharedProxy"])
        self.assertEqual(1, runtime.call_count)

    def test_self_witness_cannot_override_generated_call_name(self):
        # A resident call whose stored field is zero links against the same
        # word as a call to the overlay's offset-0 function, so splat names
        # both after the overlay's own function. The call pass must not
        # rebind that generated name to the resident target from the function
        # being proved; the name keeps no call identity and the candidate's
        # surface is compared on its own evidence.
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            rom, module, runtime_module, target, canonicals = (
                self.call_fixture(root)
            )
            name = "func_overlay_007_F0000000_0000100"
            canonical = next(iter(canonicals.values()))
            canonical._symbols[0] = (name, 0, 0, 0, rs.SHN_UNDEF)
            candidate = self.FakeElf(
                root / "build/src/overlays/o007/witness0.c.o",
                ["", ".text"],
                symbols=[(name, 0, 0, 0, rs.SHN_UNDEF)],
                relocations=[(".text", 0x8, rs.R_MIPS_26, 0)],
                sections={".text": b"\0" * 0x20},
            )
            records = [rs.SurfaceRecord(0x8, rs.R_MIPS_26, (0, 0x1234))]
            kwargs = dict(root=root,
                          elf_loader=lambda path: canonicals[path],
                          module=module, rom=rom,
                          runtime_module=runtime_module)
            with mock.patch.object(rs, "_target_runtime_records",
                                   return_value=records):
                circular, _ = rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 7, target,
                    {"modules": []}, 0, 0x20, **kwargs)
                honest, _ = rs._stable_overlay_call_identities(
                    root / "missing-aliases.txt", candidate, 7, target,
                    {"modules": []}, 0, 0x20, own_range=(0, 0x20), **kwargs)
        self.assertEqual((0, 0x1234), circular[name])
        self.assertNotIn(name, honest)

    def test_call_proxy_witness_does_not_align_shifted_offset(self):
        identity = (0, 0x1234)
        result = rs.compare_record_sets(
            [rs.SurfaceRecord(0x8, rs.R_MIPS_26, identity)],
            [rs.SurfaceRecord(0xC, rs.R_MIPS_26, identity)],
        )
        self.assertEqual(0, result["offset_type_alignment_count"])
        self.assertEqual(0, result["stable_identity_alignment_count"])


class ResidentTargetRangeTests(unittest.TestCase):
    def setUp(self):
        # Hermetic: no linker map, so no object-local placement.
        patcher = mock.patch.object(rs, "LINK_MAP", Path("missing.map"))
        patcher.start()
        self.addCleanup(patcher.stop)

    class FakeElf:
        def __init__(self, path, section_name, section_address, section_data,
                     symbols, relocations=()):
            self.path = Path(path)
            self.names = ["", section_name]
            self._section_name = section_name
            self._section_address = section_address
            self._section_data = bytes(section_data)
            self._symbols = list(symbols)
            self._relocations = list(relocations)

        def section(self, name):
            if name != self._section_name:
                return None, None
            return 1, (0, 0, 0, self._section_address, 0,
                       len(self._section_data), 0, 0, 0, 0)

        def section_bytes(self, name):
            return self._section_data if name == self._section_name else b""

        def symbols(self):
            return list(self._symbols)

        def relocations(self, target=r"\.text"):
            return list(self._relocations)

    def _fixture(self, repo):
        candidate = repo / "build_non_matching/src/main/fx.c.o"
        target_path = repo / "build/src/main/fx.c.o"
        candidate.parent.mkdir(parents=True)
        target_path.parent.mkdir(parents=True)
        candidate.write_bytes(b"candidate")
        target_path.write_bytes(b"target")

        object_start = 0x100
        size = 0x190
        names = [
            ("func_800498FC", object_start, size, 2, 1),
            ("D_800D5F58", 0, 0, 0, rs.SHN_UNDEF),
            ("func_80021FB0", 0, 0, 0, rs.SHN_UNDEF),
            ("camGetMode", 0, 0, 0, rs.SHN_UNDEF),
        ]
        relative = [
            (0x2C, rs.R_MIPS_HI16, 1),
            (0x30, rs.R_MIPS_LO16, 1),
            (0x88, rs.R_MIPS_26, 2),
            (0x9C, rs.R_MIPS_26, 3),
            (0xC4, rs.R_MIPS_26, 2),
        ]
        relocations = [
            (".text", object_start + offset, rtype, symbol_index)
            for offset, rtype, symbol_index in relative
        ]
        object_data = bytearray(object_start + size)
        linked_start = 0x80049800
        linked_value = 0x800498FC
        linked_data = bytearray(0x300)
        linked_offset = linked_value - linked_start
        linked_data[linked_offset:linked_offset + size] = object_data[
            object_start:object_start + size]
        addresses = {
            1: 0x800D5F58,
            2: 0x80021FB0,
            3: 0x80012340,
        }
        for offset, rtype, symbol_index in relative:
            address = addresses[symbol_index]
            if rtype == rs.R_MIPS_26:
                word = (0x03 << 26) | ((address >> 2) & 0x03FFFFFF)
            elif rtype == rs.R_MIPS_HI16:
                word = (0x0F << 26) | (((address + 0x8000) >> 16) & 0xFFFF)
            else:
                word = (0x0D << 26) | (address & 0xFFFF)
            struct.pack_into(">I", linked_data, linked_offset + offset, word)

        target_object = self.FakeElf(
            target_path, ".text", 0, object_data, names, relocations)
        linked_symbols = [
            ("func_800498FC", linked_value, size, 2, 1),
            ("D_800D5F58", 0x800D5F58, 4, 1, 1),
            ("func_80021FB0", 0x80021FB0, 4, 2, 1),
            ("camGetMode", 0x80012340, 4, 2, 1),
        ]
        linked = self.FakeElf(
            repo / "build/mickey.us.elf", ".main", linked_start,
            linked_data, linked_symbols)
        return candidate, target_path, target_object, linked, linked_value, size

    def test_func_800498fc_boundary_yields_five_static_tuples(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, target_path, target_object, linked, value, size = \
                self._fixture(repo)

            def open_elf(path):
                self.assertEqual(target_path, Path(path))
                return target_object

            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", side_effect=open_elf):
                records = rs._resident_target_records(
                    candidate, "main/fx", linked, "func_800498FC",
                    value, size, ".main", repo / "missing-values.txt")

            self.assertEqual([0x2C, 0x30, 0x88, 0x9C, 0xC4],
                             [record.offset for record in records])
            self.assertTrue(all(record.identity is not None for record in records))
            self.assertEqual(records[0].identity, records[1].identity)

    def _pc16_fixture(self, repo, destination_offset=0x40, addend_words=-1):
        candidate, target_path, target, linked, value, size = self._fixture(repo)
        offset = 0x10
        object_start = target.symbols()[0][1]
        # A global auxiliary label causes GAS to retain PC16 with A=-4;
        # the final branch still targets S, because the architectural PC is P+4.
        word = (4 << 26) | (1 << 21)
        object_data = bytearray(target._section_data)
        struct.pack_into(">I", object_data, object_start + offset,
                         word | (addend_words & 0xFFFF))
        names = target.symbols() + [("local_case", object_start + destination_offset, 0, 18, 1)]
        relocations = target.relocations() + [(".text", object_start + offset, rs.R_MIPS_PC16, len(names) - 1)]
        target = self.FakeElf(target_path, ".text", 0, object_data, names, relocations)
        effective_offset = destination_offset + addend_words * 4 + 4
        linked_data = bytearray(linked._section_data)
        struct.pack_into(">I", linked_data, value - linked._section_address + offset,
                         word | (((effective_offset - offset - 4) // 4) & 0xFFFF))
        linked = self.FakeElf(linked.path, ".main", linked._section_address, linked_data,
                              linked.symbols() + [("local_case", value + destination_offset, 0, 18, 1)])
        return candidate, target, linked, value, size, effective_offset

    def _pc16_records(self, repo, fixture):
        candidate, target, linked, value, size, _ = fixture
        with mock.patch.object(rs, "REPO", repo), mock.patch.object(rs, "Elf", return_value=target):
            return rs._resident_target_records(candidate, None, linked, "func_800498FC",
                                                value, size, ".main", repo / "missing-values.txt")

    def test_pc16_proves_forward_backward_and_nonzero_addend_without_dropping_tuple(self):
        for destination, addend in ((0x40, -1), (0, -1), (0x40, 1)):
            with self.subTest(destination=destination, addend=addend), tempfile.TemporaryDirectory() as td:
                repo = Path(td)
                fixture = self._pc16_fixture(repo, destination, addend)
                records = self._pc16_records(repo, fixture)
                self.assertEqual(len(records), 6)
                branch = next(record for record in records if record.rtype == rs.R_MIPS_PC16)
                self.assertEqual(branch.offset, 0x10)
                self.assertEqual(branch.identity, (0, fixture[3] + fixture[5] - rs.ot.RESIDENT_VRAM_BASE))

    def test_pc16_refuses_wrong_opcode_or_linked_displacement(self):
        for bit, error in ((1 << 21, "opcode/register"), (1, "does not reproduce")):
            with self.subTest(bit=bit), tempfile.TemporaryDirectory() as td:
                repo = Path(td)
                fixture = list(self._pc16_fixture(repo))
                linked = fixture[2]
                data = bytearray(linked._section_data)
                offset = fixture[3] - linked._section_address + 0x10
                word = struct.unpack_from(">I", data, offset)[0]
                struct.pack_into(">I", data, offset, word ^ bit)
                linked._section_data = bytes(data)
                with self.assertRaisesRegex(rs.SurfaceComparisonError, error):
                    self._pc16_records(repo, fixture)

    def test_pc16_refuses_external_unaligned_and_out_of_function_destinations(self):
        for kind in ("external", "unaligned", "outside"):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory() as td:
                repo = Path(td)
                fixture = self._pc16_fixture(repo)
                target = fixture[1]
                name, value, size, info, section = target._symbols[-1]
                if kind == "external":
                    section = rs.SHN_UNDEF
                else:
                    value += 1 if kind == "unaligned" else fixture[4]
                target._symbols[-1] = (name, value, size, info, section)
                with self.assertRaisesRegex(rs.SurfaceComparisonError, "resident PC16"):
                    self._pc16_records(repo, fixture)

    def test_pc16_refuses_symbol_address_inconsistent_with_local_layout(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            fixture = self._pc16_fixture(repo)
            linked = fixture[2]
            name, value, size, info, section = linked._symbols[-1]
            linked._symbols[-1] = (name, value + 4, size, info, section)
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "symbol identity disagrees"):
                self._pc16_records(repo, fixture)

    def test_pc16_refuses_nonbranch_instruction(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            fixture = self._pc16_fixture(repo)
            target = fixture[1]
            data = bytearray(target._section_data)
            struct.pack_into(">I", data, target.symbols()[0][1] + 0x10, 0)
            target._section_data = bytes(data)
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "supported branch"):
                self._pc16_records(repo, fixture)

    def test_conflicting_source_assertion_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, *_rest = self._fixture(repo)
            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                            "source is ambiguous"):
                    rs.resolve_resident_target_object(candidate, "main/other")

    def test_sparse_runtime_identity_overrides_matching_static_tuple(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, _target_path, target_object, linked, value, size = \
                self._fixture(repo)
            runtime = [
                rs.SurfaceRecord(0x88, rs.R_MIPS_26, (7, 0x1234), 0)
            ]
            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", return_value=target_object):
                records = rs._resident_target_records(
                    candidate, None, linked, "func_800498FC", value, size,
                    ".main", repo / "missing-values.txt", runtime)

            by_shape = {(record.offset, record.rtype): record for record in records}
            self.assertEqual((7, 0x1234),
                             by_shape[(0x88, rs.R_MIPS_26)].identity)
            self.assertEqual(5, len(records))

    def test_missing_canonical_target_object_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate = repo / "build_non_matching/src/main/fx.c.o"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"candidate")
            with mock.patch.object(rs, "REPO", repo):
                with self.assertRaisesRegex(rs.SurfaceComparisonError,
                                            "missing canonical resident"):
                    rs.resolve_resident_target_object(candidate)

    def test_nonrelocation_byte_mismatch_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            repo = Path(td)
            candidate, target_path, target_object, linked, value, size = \
                self._fixture(repo)
            linked_data = bytearray(linked._section_data)
            linked_data[value - linked._section_address + 4] = 1
            bad_linked = self.FakeElf(
                linked.path, ".main", linked._section_address, linked_data,
                linked.symbols())

            with mock.patch.object(rs, "REPO", repo), \
                    mock.patch.object(rs, "Elf", return_value=target_object):
                with self.assertRaisesRegex(
                        rs.SurfaceComparisonError, "outside relocation words"):
                    rs._resident_target_records(
                        candidate, None, bad_linked, "func_800498FC",
                        value, size, ".main", repo / "missing-values.txt")


def _site(module_off, rtype, symbol="s", in_table=False):
    return {"symbol": symbol, "obj_off": module_off, "type": rtype,
            "module_off": module_off, "table_off": module_off if in_table
            else None, "shifted": False, "stored": None, "obj": None,
            "in_table": in_table, "op": None, "note": ""}


def _record(target_offset, mode, op_name="LOCAL"):
    return {"target_offset": target_offset, "mode": mode, "op_name": op_name}


class OrderPreservingEmbeddingTests(unittest.TestCase):
    """The alignment rule itself, on synthetic sequences: no ROM data."""

    def embed(self, candidate, retail):
        return rs.unique_order_preserving_embedding(candidate, retail)

    def test_identical_sequence_is_the_identity(self):
        seq = [rs.R_MIPS_HI16, rs.R_MIPS_LO16, rs.R_MIPS_26]
        self.assertEqual([0, 1, 2], self.embed(seq, list(seq)))

    def test_extra_retail_record_of_an_unused_type_is_skipped(self):
        # An intra-module JUMP the object resolves itself leaves an R_MIPS_26
        # record with no candidate site; the rest still embeds uniquely.
        candidate = [rs.R_MIPS_HI16, rs.R_MIPS_LO16]
        retail = [rs.R_MIPS_26, rs.R_MIPS_HI16, rs.R_MIPS_LO16]
        self.assertEqual([1, 2], self.embed(candidate, retail))

    def test_ambiguous_placement_is_refused(self):
        # Two ways to lay one HI16 down over two HI16 records: choosing either
        # would be inventing an addend.
        self.assertIsNone(self.embed([rs.R_MIPS_HI16],
                                     [rs.R_MIPS_HI16, rs.R_MIPS_HI16]))

    def test_a_type_the_retail_order_cannot_supply_is_refused(self):
        self.assertIsNone(self.embed([rs.R_MIPS_HI16, rs.R_MIPS_26],
                                     [rs.R_MIPS_26, rs.R_MIPS_HI16]))

    def test_more_candidate_sites_than_records_is_refused(self):
        self.assertIsNone(self.embed([rs.R_MIPS_HI16, rs.R_MIPS_HI16],
                                     [rs.R_MIPS_HI16]))

    def test_empty_candidate_embeds_trivially(self):
        self.assertEqual([], self.embed([], [rs.R_MIPS_HI16]))


class SiteAlignmentTests(unittest.TestCase):
    """`align_sites` over synthetic site and record lists: no ROM data."""

    def test_fully_corroborated_object_is_not_realigned(self):
        sites = [_site(0x10, rs.R_MIPS_HI16, in_table=True),
                 _site(0x14, rs.R_MIPS_LO16, in_table=True)]
        records = [_record(0x10, rs.R_MIPS_HI16),
                   _record(0x14, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x100)
        self.assertFalse(summary["attempted"])
        self.assertEqual(2, summary["aligned"])
        self.assertEqual(0, summary["shifted"])
        self.assertEqual([0x10, 0x14], [s["table_off"] for s in sites])

    def test_a_shifted_site_is_aligned_and_counted_as_shifted(self):
        # The candidate moved its LO16 two words later; the HI16 still lands
        # where the table names it.
        sites = [_site(0x10, rs.R_MIPS_HI16, in_table=True),
                 _site(0x1C, rs.R_MIPS_LO16)]
        records = [_record(0x10, rs.R_MIPS_HI16),
                   _record(0x14, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x100)
        self.assertTrue(summary["attempted"])
        self.assertIsNone(summary["refused"])
        self.assertEqual(2, summary["aligned"])
        self.assertEqual(1, summary["shifted"])
        self.assertEqual([False, True], [s["shifted"] for s in sites])
        self.assertEqual([0x10, 0x14], [s["table_off"] for s in sites])
        self.assertTrue(all(s["in_table"] for s in sites))

    def test_an_exact_offset_that_belongs_to_another_record_is_moved(self):
        # A site can sit exactly on a record and still belong to the previous
        # one: the order is authoritative, so the alignment overrides the
        # offset coincidence and reports the move.
        sites = [_site(0x14, rs.R_MIPS_HI16, in_table=True),
                 _site(0x18, rs.R_MIPS_HI16),
                 _site(0x1C, rs.R_MIPS_LO16),
                 _site(0x20, rs.R_MIPS_LO16)]
        records = [_record(0x10, rs.R_MIPS_HI16),
                   _record(0x14, rs.R_MIPS_HI16),
                   _record(0x18, rs.R_MIPS_LO16),
                   _record(0x1C, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x100)
        self.assertEqual(4, summary["aligned"])
        self.assertEqual(4, summary["shifted"])
        self.assertEqual([0x10, 0x14, 0x18, 0x1C],
                         [s["table_off"] for s in sites])

    def test_ambiguous_alignment_refuses_and_leaves_every_site_unvalued(self):
        sites = [_site(0x10, rs.R_MIPS_HI16),
                 _site(0x20, rs.R_MIPS_LO16)]
        records = [_record(0x10, rs.R_MIPS_HI16),
                   _record(0x14, rs.R_MIPS_HI16),
                   _record(0x18, rs.R_MIPS_LO16),
                   _record(0x1C, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x100)
        self.assertTrue(summary["attempted"])
        self.assertIn("no unique order-preserving alignment",
                      summary["refused"])
        self.assertEqual(0, summary["aligned"])
        self.assertEqual([None, None], [s["table_off"] for s in sites])
        self.assertEqual([False, False], [s["in_table"] for s in sites])

    def test_records_outside_the_translation_unit_are_not_candidates(self):
        # The pool is bounded by the object's own text range, so the next
        # module function's records cannot absorb a shifted site.
        sites = [_site(0x10, rs.R_MIPS_HI16), _site(0x14, rs.R_MIPS_LO16)]
        records = [_record(0x10, rs.R_MIPS_HI16),
                   _record(0x14, rs.R_MIPS_LO16),
                   _record(0x40, rs.R_MIPS_HI16),
                   _record(0x44, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x20)
        self.assertEqual(2, summary["aligned"])
        self.assertEqual(0, summary["shifted"])

    def test_unmapped_sites_are_ignored_by_the_alignment(self):
        unmapped = _site(0x10, rs.R_MIPS_HI16)
        unmapped["module_off"] = None
        sites = [unmapped, _site(0x18, rs.R_MIPS_LO16)]
        records = [_record(0x14, rs.R_MIPS_LO16)]
        summary = rs.align_sites(sites, records, 0, 0x100)
        self.assertEqual(1, summary["sites"])
        self.assertEqual(1, summary["aligned"])
        self.assertEqual(0x14, sites[1]["table_off"])


class PermuterTargetCoverageTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="annotation-synthetic-")
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.bin = rs.REPO / "tools/binutils"
        self.prelude = (".set noreorder\n.set noat\n.text\n"
            ".macro glabel name\n.globl \\name\n.type \\name,@function\n\\name:\n.endm\n"
            ".macro endlabel name\n.size \\name,.-\\name\n.endm\n")
        self.ops = ["lui $at, 1", "lw $t0, -32768($at)", "lw $t1, -32764($at)",
                    "lui $at, 1", "lw $t2, -32760($at)", "jal 0", "nop"]
        raw = self.assemble("raw", "glabel fixture\n" + "\n".join(self.ops) + "\nendlabel fixture\n")
        self.words = rs.Elf(raw).section_bytes(".text")[:28]
        self.rom = bytes(0x100) + self.words
        self.text = "glabel fixture\n" + "\n".join(
            f"/* {0x100+i*4:X} {rs.SYNTHETIC_VMA+i*4:08X} {struct.unpack_from('>I', self.words, i*4)[0]:08X} */ {op}"
            for i, op in enumerate(self.ops)) + "\nendlabel fixture\n"
        candidate = ["lui $at, %hi(known)", "lw $t0, %lo(known)($at)",
                     "lw $t1, %lo(known+4)($at)", "lui $at, %hi(known+8)",
                     "lw $t2, %lo(known+8)($at)", "jal call_target", "nop"]
        self.base = self.assemble("base", "glabel fixture\n" + "\n".join(candidate) + "\nendlabel fixture\n")
        self.records = [dict(table=1, index=i, target_offset=offset, mode=mode,
                             op=0 if mode == 4 else 1, op_name="SYMBOL" if mode == 4 else "LOCAL",
                             symbol_index=100, target_overlay=0, target_symbol_offset=64)
                        for i, (offset, mode) in enumerate(((0,5),(4,6),(8,6),(12,5),(16,6),(20,4)))]
        self.module = dict(overlay=1, rom_start=0x100, text_size=28, data_size=0)

    def assemble(self, name, text):
        source, obj = self.root / (name + ".s"), self.root / (name + ".o")
        source.write_text(self.prelude + text)
        subprocess.run([str(self.bin / "mips64-elf-as"), "-march=vr4300", "-32", "-o", str(obj), str(source)],
                       check=True, capture_output=True)
        return obj

    def annotate(self, text=None, records=None, empty_candidate=False, shifted_candidate=False):
        elf = rs.Elf(self.base)
        if empty_candidate:
            elf.relocations = lambda *args: []
        elif shifted_candidate:
            relocations = [(sec, 8 if off == 12 else off, kind, sym)
                           for sec, off, kind, sym in elf.relocations()]
            elf.relocations = lambda *args: relocations
        with mock.patch.object(rs.ot, "read_headers", return_value=[]), \
             mock.patch.object(rs.ot, "build_modules", return_value=[self.module]), \
             mock.patch.object(rs, "Elf", return_value=elf):
            return rs.permuter_annotation(self.text if text is None else text, self.base, ["fixture"], 1,
                                         self.rom, self.records if records is None else records)

    def roundtrip(self, text, notes):
        obj = self.assemble("annotated", text)
        proof = json.loads(next(note[len("target-proof: "):] for note in notes if note.startswith("target-proof: ")))
        self.assertEqual(sorted((offset, mode) for _, offset, mode, _ in rs.Elf(obj).relocations()),
                         sorted((r["target_offset"], r["mode"]) for r in self.records))
        linked = self.root / "linked.elf"
        args = [str(self.bin / "mips64-elf-ld"), "-m", "elf32ebmip", "-Ttext", "0", "-e", "0",
                "-o", str(linked), str(obj)]
        for name, value in proof["values"].items():
            args.extend(["--defsym", f"{name}=0x{value:X}"])
        subprocess.run(args, check=True, capture_output=True)
        self.assertEqual(rs.Elf(linked).section_bytes(".text")[:28], self.words)
        return obj

    def test_complete_target_independent_of_candidate_with_signed_shared_low_roundtrip(self):
        text, renames, notes = self.annotate(empty_candidate=True)
        self.assertEqual(renames, {})
        self.assertIn("6/6", notes[1])
        self.assertIn(self.text.splitlines()[-2], text)
        self.roundtrip(text, notes)

    def test_proven_shared_low_candidate_renames_score_zero(self):
        text, renames, notes = self.annotate()
        self.assertEqual(renames["known"], "__ovval_00008000")
        self.assertIn("%lo(__ovval_00008000+0x4)", text)
        target = self.roundtrip(text, notes)
        self.assertEqual(self.renamed_score(target, renames), 0)

    def test_scratch_annotation_composes_actual_ordered_recipe_aliases(self):
        import permute_batch as pb
        from types import SimpleNamespace
        original = self.root / "original.o"
        original.write_bytes(self.base.read_bytes())
        (self.root / "base.c").write_text("synthetic compile input\n")
        (self.root / "target.s").write_text(self.prelude + self.text)
        script = self.root / "compile.sh"
        script.write_text(f'#!/bin/sh\nset -e\nOUTPUT="$3"\ncp "{original}" "$OUTPUT"\n')
        target_path = "build/src/f.c.o"
        recipe = pb.BuildRecipe((), (
            f"{pb.OBJCOPY} --redefine-sym known=middle --redefine-sym call_target=callReloc {target_path}",
            f"{pb.OBJCOPY} --redefine-sym middle=last {target_path}",
        ), (), True)
        history = []
        with mock.patch.object(pb, "ROOT", self.root):
            pb.replicate_objcopy(self.root, recipe, self.root / "src/f.c", self.root, history)
        text, renames, notes = self.annotate()
        rom = self.root / "rom.bin"
        rom.write_bytes(self.rom)
        with mock.patch.object(pb, "BASEROM", rom), \
             mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb, "ASSEMBLER_COMMAND", str(self.bin / "mips64-elf-as") + " -march=vr4300 -32"), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(self.prelude + text, renames, notes)):
            self.assertEqual(pb.annotate_overlay_scratch(SimpleNamespace(overlay=1, func="fixture"),
                             self.root, self.root, alias_history=history), 2)
        self.assertIn(f'--redefine-sym last={renames["known"]}', script.read_text())
        self.assertIn(f'--redefine-sym callReloc={renames["call_target"]}', script.read_text())
        self.assertEqual(rs.Elf(self.base).section_bytes(".text"), rs.Elf(original).section_bytes(".text"))
        self.assertEqual(self.renamed_score(self.root / "target.o", {}), 0)

    def test_supported_add_remove_recipes_preserve_real_objcopy_semantics(self):
        import permute_batch as pb
        base = self.assemble("metadata-base", "glabel fixture\nnop\nendlabel fixture\n"
                             ".data\n.globl disposable\ndisposable: .word 7\n")
        script = self.root / "compile.sh"
        script.write_text(f'#!/bin/sh\nset -e\nOUTPUT="$3"\ncp "{base}" "$OUTPUT"\n')
        target = "build/src/f.c.o"
        # Exact option spellings used by the O8/O31 recipes, with a mixed
        # rename/add invocation to establish that the new symbol is NOT renamed.
        recipe = pb.BuildRecipe((), (
            f"{pb.OBJCOPY} --redefine-sym disposable=gone --add-symbol gOverlay8UpdateChannelConstants=0x1BC,global --add-symbol=disposable=0,global --remove-section=.data {target}",
            f"{pb.OBJCOPY} --remove-section=.data --remove-section=.rel.data --remove-section=.gptab.data {target}",
            f"{pb.OBJCOPY} --add-symbol fresh=.text:0,global --redefine-sym fresh=later {target}",
        ), (), True)
        history = []
        with mock.patch.object(pb, "ROOT", self.root):
            pb.replicate_objcopy(self.root, recipe, self.root / "src/f.c", self.root, history)
        output = self.root / "metadata-output.o"
        subprocess.run(["bash", str(script), "ignored.c", "-o", str(output)], check=True, capture_output=True)
        old_elf, new_elf = rs.Elf(base), rs.Elf(output)
        self.assertEqual(old_elf.section_bytes(".text"), new_elf.section_bytes(".text"))
        names = {row[0] for row in new_elf.symbols()}
        self.assertIn("disposable", names)
        self.assertIn("fresh", names)
        self.assertNotIn("gone", names)
        self.assertNotIn("later", names)
        self.assertIsNone(new_elf.section(".data")[0])
        sections = {row[0]: old_elf.names[row[4]] if 0 < row[4] < len(old_elf.names) else None
                    for row in old_elf.symbols()}
        self.assertEqual(pb.annotation_aliases({"fixture": "proved"}, history,
                         [row[0] for row in old_elf.symbols()], sections), {"fixture": "proved"})
        with self.assertRaises(RuntimeError):
            pb.annotation_aliases({"disposable": "proved"}, history,
                                  [row[0] for row in old_elf.symbols()], sections)

    def test_added_symbols_never_inherit_scalar_runtime_identity(self):
        import permute_batch as pb
        add = {"renames": (), "additions": (("extra", None),), "removals": ()}
        for history, renames in (([add, (("a", "extra"),)], {"a": "X"}),
                                 ([add], {"a": "extra"}),
                                 ([{"renames": (), "additions": (("a", None),), "removals": ()}], {"a": "X"})):
            with self.subTest(history=history), self.assertRaises(RuntimeError):
                pb.annotation_aliases(renames, history, ["a"], {"a": None})

    def test_alias_invocations_are_simultaneous_and_ambiguity_fails_closed(self):
        import permute_batch as pb
        self.assertEqual(pb.annotation_aliases({"a": "X", "b": "Y"},
            [[("a", "b"), ("b", "c")]], ["a", "b"]), {"b": "X", "c": "Y"})
        self.assertEqual(pb.annotation_aliases({"a": "X"},
            [[("a", "b")], [("b", "c")]], ["a"]), {"c": "X"})
        self.assertEqual(pb.annotation_aliases({"a": "X", "b": "X"},
            [[("a", "x")], [("b", "x")]], ["a", "b"]), {"x": "X"})
        for history, symbols in (([[("a", "x"), ("b", "x")]], ["a", "b"]),
                                 ([[("a", "b")]], ["a", "b"]),
                                 ([[("a", "b")], [("b", "a")]], ["a"]),
                                 ([[("a", "b"), ("a", "c")]], ["a"])):
            with self.subTest(history=history), self.assertRaises(RuntimeError):
                pb.annotation_aliases({"a": "X"}, history, symbols)

    def test_alias_collision_restores_all_annotation_scratch_files(self):
        import permute_batch as pb
        from types import SimpleNamespace
        text, renames, notes = self.annotate()
        (self.root / "target.s").write_text(self.prelude + self.text)
        (self.root / "target.o").write_bytes(self.base.read_bytes())
        (self.root / "compile.sh").write_text("unchanged compiler script\n")
        names = ("target.s", "target.o", "compile.sh", "base.o")
        before = {name: (self.root / name).read_bytes() for name in names}
        rom = self.root / "rom.bin"
        rom.write_bytes(self.rom)
        with mock.patch.object(pb, "BASEROM", rom), \
             mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb, "bounded_capture", side_effect=AssertionError("must reject before compile")), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(self.prelude + text, renames, notes)):
            self.assertEqual(pb.annotate_overlay_scratch(SimpleNamespace(overlay=1, func="fixture"),
                self.root, self.root, alias_history=[[("known", "call_target")]]), 0)
        self.assertEqual(before, {name: (self.root / name).read_bytes() for name in names})
        self.assertIn("distinct runtime identities", (self.root / "annotation.txt").read_text())

    def test_removed_symbol_refusal_restores_annotation_scratch(self):
        import permute_batch as pb
        from types import SimpleNamespace
        (self.root / "target.s").write_text(self.prelude + self.text)
        (self.root / "target.o").write_bytes(self.base.read_bytes())
        (self.root / "compile.sh").write_text("unchanged compiler script\n")
        names = ("target.s", "target.o", "compile.sh", "base.o")
        before = {name: (self.root / name).read_bytes() for name in names}
        rom = self.root / "rom.bin"; rom.write_bytes(self.rom)
        history = [{"renames": (), "additions": (), "removals": (".bss",)}]
        with mock.patch.object(pb, "BASEROM", rom), mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=("changed target", {".bss": "proved"}, [])):
            self.assertEqual(pb.annotate_overlay_scratch(SimpleNamespace(overlay=1, func="fixture"),
                             self.root, self.root, alias_history=history), 0)
        self.assertEqual(before, {name: (self.root / name).read_bytes() for name in names})
        self.assertIn("original candidate symbol", (self.root / "annotation.txt").read_text())

    def renamed_score(self, target, renames):
        candidate = self.root / "renamed.o"
        objcopy = str(self.bin / "mips64-elf-objcopy")
        subprocess.run([objcopy, str(self.base), str(candidate)], check=True, capture_output=True)
        # Production also separates duplicate destinations into successive
        # objcopy invocations, preserving aliases of one runtime identity.
        for old, new in renames.items():
            subprocess.run([objcopy, f"--redefine-sym={old}={new}", str(candidate)],
                           check=True, capture_output=True)
        self.assertEqual(rs.Elf(candidate).section_bytes(".text"),
                         rs.Elf(self.base).section_bytes(".text"))
        self.assertEqual([(sec, off, kind) for sec, off, kind, _ in rs.Elf(candidate).relocations()],
                         [(sec, off, kind) for sec, off, kind, _ in rs.Elf(self.base).relocations()])
        program = ("import sys;sys.path.insert(0,sys.argv[1]);from src.scorer import Scorer;"
                   "print(Scorer(sys.argv[2],stack_differences=True,algorithm='difflib',debug_mode=False,"
                   "ign_branch_targets=False,objdump_command=sys.argv[4]).score(sys.argv[3])[0])")
        score = subprocess.check_output([str(rs.REPO / ".venv/bin/python"), "-c", program,
            str(rs.REPO / "tools/permuter"), str(target), str(candidate),
            str(self.bin / "mips64-elf-objdump") + " -drz -m mips:4300"], text=True)
        return int(score.strip())

    def test_duplicate_address_record_and_out_of_owner_fail_closed(self):
        with self.assertRaises(rs.AnnotationError):
            self.annotate(text=self.text + self.text.splitlines()[1] + "\n")
        for records in (self.records + [self.records[0]],
                        [{**self.records[0], "target_offset": 32}, *self.records[1:]],
                        [{**self.records[0], "mode": 2}, *self.records[1:]],
                        [{**self.records[0], "target_offset": -4}, *self.records[1:]]):
            with self.subTest(records=records), self.assertRaises(rs.AnnotationError):
                self.annotate(records=records)

    def test_conflicting_candidate_identity_does_not_hide_target_sites(self):
        records = [dict(record) for record in self.records]
        records[3]["symbol_index"] = records[4]["symbol_index"] = 200
        text, renames, notes = self.annotate(records=records)
        self.assertNotIn("known", renames)
        self.assertIn("ambiguous", " ".join(notes))
        self.roundtrip(text, notes)

    def test_one_coincident_pair_cannot_bind_shifted_remaining_symbol_sites(self):
        text, renames, notes = self.annotate(shifted_candidate=True)
        self.assertNotIn("known", renames)
        self.assertIn("incomplete candidate", " ".join(notes))
        self.roundtrip(text, notes)

    def test_equal_zero_bases_do_not_merge_distinct_runtime_identities(self):
        source = ("glabel fixture\n"
                  "lui $at,%hi(left+0x8000)\nlw $t0,%lo(left+0x8000)($at)\n"
                  "lw $t1,%lo(left+0x8004)($at)\nlui $at,%hi(right+0x8008)\n"
                  "lw $t2,%lo(right+0x8008)($at)\njal call_target\nnop\nendlabel fixture\n")
        self.base = self.assemble("distinct", source)
        same_text, same_renames, same_notes = self.annotate()
        self.assertEqual(same_renames["left"], "__ovval_00000000")
        self.assertEqual(same_renames["right"], "__ovval_00000000")
        same_target = self.roundtrip(same_text, same_notes)
        self.assertEqual(self.renamed_score(same_target, same_renames), 0)
        for changes in ({"symbol_index": 200}, {"op_name": "DATA", "op": 3}):
            records = [dict(record) for record in self.records]
            records[3].update(changes)
            records[4].update(changes)
            with self.subTest(changes=changes):
                text, renames, notes = self.annotate(records=records)
                self.assertNotEqual(renames["left"], renames["right"])
                self.assertNotIn(".", renames["left"] + renames["right"])
                self.assertIn("%lo(" + renames["left"] + "-0x7FFC)", text)
                target = self.roundtrip(text, notes)
                self.assertEqual(self.renamed_score(target, renames), 0)
                swapped = {**renames, "left": renames["right"], "right": renames["left"]}
                self.assertGreater(self.renamed_score(target, swapped), 0)

    def test_bad_word_operand_and_incomplete_pair_are_refused(self):
        with self.assertRaises(rs.AnnotationError):
            self.annotate(text=self.text.replace("lui $at, 1", "lui $v0, 1", 1))
        with self.assertRaises(rs.AnnotationError):
            self.annotate(text=self.text.replace("3C010001", "3C010002", 1))
        with self.assertRaises(rs.AnnotationError):
            self.annotate(records=[self.records[0]])

    def test_footer_padding_and_neighbor_records_are_not_owned(self):
        self.module["text_size"] = 36
        footer = "/* 11C F000001C 00000000 */ nop\n/* 120 F0000020 00000000 */ nop\n"
        records = [*self.records, {**self.records[0], "target_offset": 28, "mode": 4, "index": 6}]
        text, _, notes = self.annotate(text=self.text + footer, records=records)
        self.assertTrue(text.endswith(footer))
        self.assertIn("6/6", notes[1])
        self.roundtrip(text, notes)

    def test_caller_applies_target_only_and_proves_nonrelocation_words(self):
        import permute_batch as pb
        from types import SimpleNamespace
        text, renames, notes = self.annotate(empty_candidate=True)
        rom = self.root / "rom.bin"
        rom.write_bytes(self.rom)
        target = self.root / "target.s"
        original = self.prelude + self.text
        target.write_text(original)
        compile_script = self.root / "compile.sh"
        compile_script.write_text("manual recipe preserved\n")
        item = SimpleNamespace(func="fixture", overlay=1)
        with mock.patch.object(pb, "BASEROM", rom), \
             mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb, "ASSEMBLER_COMMAND", str(self.bin / "mips64-elf-as") + " -march=vr4300 -32"), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(self.prelude + text, renames, notes)):
            self.assertEqual(pb.annotate_overlay_scratch(item, self.root, self.root), 0)
            self.assertIn("__ovtarget_", target.read_text())
            self.assertEqual(compile_script.read_text(), "manual recipe preserved\n")
        # ROM-correct comment tokens cannot hide an edited nonrelocation operand.
        target.write_text(original)
        bad = (self.prelude + text).replace("*/ nop", "*/ addiu $v0, $zero, 1")
        with mock.patch.object(pb, "BASEROM", rom), \
             mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb, "ASSEMBLER_COMMAND", str(self.bin / "mips64-elf-as") + " -march=vr4300 -32"), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(bad, renames, notes)):
            self.assertEqual(pb.annotate_overlay_scratch(item, self.root, self.root), 0)
            self.assertEqual(target.read_text(), original)
            self.assertIn("target proof failed", (self.root / "annotation.txt").read_text())
        with mock.patch.object(pb, "BASEROM", rom), \
             mock.patch.object(pb, "find_asm_target", return_value=None), \
             mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(original, {}, [])), \
             mock.patch.object(pb, "bounded_capture") as capture:
            self.assertEqual(pb.annotate_overlay_scratch(item, self.root, self.root), 0)
            capture.assert_not_called()


class AnnotationScratchTransactionTests(unittest.TestCase):
    def exercise(self, phase, kind, missing_target=False, preservation_failure=False):
        import permute_batch as pb
        import time
        from types import SimpleNamespace
        with tempfile.TemporaryDirectory(prefix="annotation-transaction-") as directory:
            root = Path(directory)
            scratch, output = root / "scratch", root / "output"
            scratch.mkdir()
            output.mkdir()
            before = {"target.s": b"original target\r\n", "target.o": b"original target object",
                      "compile.sh": b"original script without newline", "base.o": b"original base"}
            if missing_target:
                del before["target.o"]
            for name, data in before.items():
                (scratch / name).write_bytes(data)
                (scratch / name).chmod(0o750 if name == "compile.sh" else 0o640)
            (output / "annotation.txt").write_text("prior diagnostic\n")
            rom = root / "rom"
            rom.write_bytes(b"synthetic")
            error = (subprocess.TimeoutExpired([phase], 1, output="partial sentinel") if kind == "timeout"
                     else RuntimeError("batch cancelled") if kind == "cancel"
                     else KeyboardInterrupt("interrupted sentinel") if kind == "interrupt"
                     else subprocess.CalledProcessError(7, [phase], output="failure sentinel"))
            calls = []
            def fault():
                if kind == "cancel":
                    pb.CANCEL_EVENT.set()
                raise error
            def capture(command, deadline, **kwargs):
                calls.append(command)
                step = "refresh" if command[0] == "bash" else "assembly"
                target = scratch / ("base.o" if step == "refresh" else "target.o")
                target.write_bytes(b"failed attempt artifact")
                target.chmod(0o600)
                if step == phase:
                    fault()
                return subprocess.CompletedProcess(command, 0, "ok", "")
            def proof(*args):
                if phase == "proof":
                    fault()
            pb.CANCEL_EVENT.clear()
            write_bytes = Path.write_bytes
            def preserve(path, data):
                if preservation_failure and path.name == "failed-target.o":
                    raise OSError("synthetic evidence write failure")
                return write_bytes(path, data)
            try:
                with mock.patch.object(pb, "BASEROM", rom), \
                     mock.patch.object(pb, "find_asm_target", return_value=None), \
                     mock.patch.object(pb.reloc_surface, "permuter_annotation", return_value=(
                         "annotated target\n", {"old": "__ovval_00000000"}, [])), \
                     mock.patch.object(pb, "bounded_capture", side_effect=capture), \
                     mock.patch.object(pb, "validate_annotation_target", side_effect=proof), \
                     mock.patch.object(Path, "write_bytes", preserve):
                    if preservation_failure:
                        with self.assertRaisesRegex(RuntimeError, "annotation recovery needs review"):
                            pb.annotate_overlay_scratch(SimpleNamespace(overlay=1, func="fixture"), scratch, output)
                    elif kind in {"timeout", "cancel", "interrupt"}:
                        with self.assertRaises(type(error)) as caught:
                            pb.annotate_overlay_scratch(SimpleNamespace(overlay=1, func="fixture"), scratch, output,
                                                        time.monotonic() + 60)
                        self.assertIs(caught.exception, error)
                    else:
                        self.assertEqual(pb.annotate_overlay_scratch(
                            SimpleNamespace(overlay=1, func="fixture"), scratch, output, time.monotonic() + 60), 0)
            finally:
                pb.CANCEL_EVENT.clear()
            for name, data in before.items():
                self.assertEqual((scratch / name).read_bytes(), data)
                self.assertEqual((scratch / name).stat().st_mode & 0o777, 0o750 if name == "compile.sh" else 0o640)
            if missing_target:
                self.assertFalse((scratch / "target.o").exists())
            self.assertEqual(len(calls), 2 if phase == "refresh" else 1)
            self.assertIn(str(error), (output / "annotation.txt").read_text())
            attempt, = output.glob("annotation-attempt-*")
            self.assertEqual((attempt / "before-annotation.txt").read_text(), "prior diagnostic\n")
            if not preservation_failure:
                self.assertEqual((attempt / "failed-target.o").read_bytes(), b"failed attempt artifact")
            if kind == "timeout":
                self.assertIn("partial sentinel", (attempt / "failure.txt").read_text())

    def test_every_mutation_phase_restores_bytes_modes_and_exception(self):
        for phase in ("assembly", "proof", "refresh"):
            for kind in ("nonzero", "timeout", "cancel", "interrupt"):
                with self.subTest(phase=phase, kind=kind):
                    self.exercise(phase, kind)

    def test_failed_annotation_restores_initial_absence(self):
        self.exercise("proof", "nonzero", missing_target=True)

    def test_evidence_write_failure_does_not_prevent_restoration(self):
        self.exercise("refresh", "nonzero", preservation_failure=True)


class GeneratedCrossOverlayCIdentityTests(unittest.TestCase):
    NAME = "func_overlay_008_F0000010_110"

    class Elf(FunctionSurfaceComparisonTests.BoundaryElf):
        def section(self, name):
            if name in self.names:
                address = rs.SYNTHETIC_VMA if name.startswith('.overlay_') else 0
                return self.names.index(name), (0, 1, 0, address, 0, len(self._text))
            return None, None

        def section_bytes(self, name):
            return self._text if name in self.names else b''

    def fixture(self, root):
        import permute_batch as batch
        self.enterContext(mock.patch.object(batch, 'ROOT', root))
        self.recipe_mock = self.enterContext(mock.patch.object(
            batch, 'build_recipe_for', return_value=batch.BuildRecipe(
                ('-O2',), (), (), True, ('-nostdinc', '-I', 'include'))))
        source = root / 'src/overlays/o008/mixed.c'
        source.parent.mkdir(parents=True)
        source.write_text('void %s(void) {}\n' % self.NAME)
        obj = root / 'build/src/overlays/o008/mixed.c.o'
        obj.parent.mkdir(parents=True)
        obj.write_bytes(b'fixture')
        linked_path = root / 'build/game.elf'
        linked_path.write_bytes(b'fixture')
        atlas = {'modules': [{
            'overlay': 8, 'identity': 'overlay:8', 'synthetic_vma': '0xF0000000',
            'rom': {'start': '0x100'},
            'sections': {'text': {'start': '0x100', 'end': '0x140', 'size': '0x40'}},
            'text_ownership': [{'offset': '0x0', 'end_offset': '0x40', 'size': '0x40',
                                'type': 'c', 'source': 'overlays/o008/mixed',
                                'matched': True, 'nonmatching': True}],
            'mixed_tu_exact_c_ranges': [{'offset': '0x10', 'end_offset': '0x18',
                'size': '0x8', 'source': 'overlays/o008/mixed', 'label': 'friendly'}],
        }]}
        canonical = self.Elf(path=obj, names=['', '.text'], text=bytes(64),
            symbols=[(self.NAME, 16, 8, rs.STT_FUNC, 1), ('neighbor', 0, 8, rs.STT_FUNC, 1)])
        linked = self.Elf(path=linked_path, names=['', '.overlay_008'], text=bytes(64),
            symbols=[(self.NAME, rs.SYNTHETIC_VMA+16, 8, rs.STT_FUNC, 1)])
        canonical.data = obj.read_bytes()
        linked.data = linked_path.read_bytes()
        caller = self.Elf(names=['', '.text'], text=bytes(4),
            symbols=[(self.NAME,0,0,0,rs.SHN_UNDEF)],
            relocations=[('.text',0,rs.R_MIPS_26,0)])
        return atlas, canonical, linked, caller, source, obj

    def resolve(self, root, atlas, canonical, linked, caller, source, obj, *, rom=None,
                source_overlay=1):
        return rs._stable_overlay_call_identities(root/'missing', caller, source_overlay,
            linked, atlas, 0, 4, root=root, elf_loader=lambda _path:canonical,
            rom=bytes(0x140) if rom is None else rom)[0]

    def test_mixed_interior_c_cross_overlay_and_resident_call(self):
        for origin in (1, 8, None):
            with self.subTest(origin=origin), tempfile.TemporaryDirectory() as td:
                root=Path(td); args=self.fixture(root)
                self.assertEqual({self.NAME:(8,16)}, self.resolve(root,*args,source_overlay=origin))

    def test_instruction_relocation_value_may_change_but_opcode_may_not(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td); args=self.fixture(root); canonical=args[1]
            canonical._relocations=[('.text',16,rs.R_MIPS_LO16,0)]
            canonical._text=bytes(18)+b'\x00\x01'+bytes(44)
            self.assertEqual({self.NAME:(8,16)},self.resolve(root,*args))
            canonical._text=bytes(16)+bytes((1,0,0,0))+bytes(44)
            self.assertEqual({},self.resolve(root,*args))

    def test_missing_or_unproved_evidence_is_unresolved(self):
        for defect in ('definition','guard','conditional','spliced_conditional',
                       'registry','object','stale','header','local_header','recipe',
                       'compiler_wrapper','linked_stale','rom','rom_short','object_bytes',
                       'parsed_linked','parsed_object'):
            with self.subTest(defect=defect), tempfile.TemporaryDirectory() as td:
                root=Path(td); args=self.fixture(root)
                atlas,canonical,linked,caller,source,obj=args
                rom=None
                if defect=='definition':source.write_text('void another(void) {}')
                elif defect=='guard':source.write_text('#ifdef NON_MATCHING\nvoid %s(void) {}\n#endif\n'%self.NAME)
                elif defect=='conditional':source.write_text('#if 0\nvoid %s(void) {}\n#endif\n'%self.NAME)
                elif defect=='spliced_conditional':source.write_text('#\\\nif 0\nvoid %s(void) {}\n#endif\n'%self.NAME)
                elif defect=='registry':atlas['modules'][0]['mixed_tu_exact_c_ranges']=[]
                elif defect=='object':obj.unlink()
                elif defect=='stale':os.utime(source,ns=(obj.stat().st_mtime_ns+1000000,)*2)
                elif defect=='header':
                    p=root/'include/header.h';p.parent.mkdir();p.write_text('typedef int changed;')
                    os.utime(p,ns=(obj.stat().st_mtime_ns+1000000,)*2)
                elif defect=='local_header':
                    source.write_text('#include "local.h"\nvoid %s(void) {}\n'%self.NAME)
                    os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
                    p=source.parent/'local.h';p.write_text('typedef int Changed;')
                    os.utime(p,ns=(obj.stat().st_mtime_ns+1000000,)*2)
                elif defect in ('recipe','compiler_wrapper'):
                    p=root/('mk/overlays.mk' if defect=='recipe' else 'tools/asm-processor/build.py')
                    p.parent.mkdir(parents=True);p.write_text('changed')
                    os.utime(p,ns=(obj.stat().st_mtime_ns+1000000,)*2)
                elif defect=='linked_stale':os.utime(linked.path,ns=(obj.stat().st_mtime_ns-1000000,)*2)
                elif defect=='rom':rom=bytes(0x110)+b'\x01'+bytes(0x2f)
                elif defect=='rom_short':rom=b''
                elif defect=='object_bytes':canonical._text=bytes(16)+b'\x01'+bytes(47)
                elif defect=='parsed_linked':linked.path.write_bytes(b'new file, old parsed ELF')
                elif defect=='parsed_object':canonical.data=b'different parsed object'
                # Definition/guard checks, not just newer-source rejection.
                if defect in ('definition','guard','conditional','spliced_conditional'):
                    os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
                self.assertEqual({},self.resolve(root,*args,rom=rom))

    def test_conflicting_boundary_evidence_raises(self):
        for defect in ('rom_suffix','overlay_section','linked_value','linked_size',
                       'duplicate_linked','duplicate_object','overlap','extent','offset',
                       'owners','registry_overlap','registry_extent','source_overlay'):
            with self.subTest(defect=defect), tempfile.TemporaryDirectory() as td:
                root=Path(td); args=self.fixture(root)
                atlas,canonical,linked,caller,source,obj=args
                module=atlas['modules'][0]
                if defect=='rom_suffix':caller._symbols=[(self.NAME[:-3]+'111',0,0,0,0)]
                elif defect=='overlay_section':linked.names[1]='.overlay_009'
                elif defect=='linked_value':linked._symbols=[(self.NAME,rs.SYNTHETIC_VMA+20,8,rs.STT_FUNC,1)]
                elif defect=='linked_size':linked._symbols=[(self.NAME,rs.SYNTHETIC_VMA+16,12,rs.STT_FUNC,1)]
                elif defect=='duplicate_linked':linked._symbols*=2
                elif defect=='duplicate_object':canonical._symbols.append(canonical._symbols[0])
                elif defect=='overlap':canonical._symbols.append(('bad',20,8,rs.STT_FUNC,1))
                elif defect=='extent':canonical._symbols[0]=(self.NAME,16,64,rs.STT_FUNC,1)
                elif defect=='offset':canonical._symbols[0]=(self.NAME,20,8,rs.STT_FUNC,1)
                elif defect=='owners':module['text_ownership']*=2
                elif defect=='registry_overlap':module['mixed_tu_exact_c_ranges']*=2
                elif defect=='registry_extent':module['mixed_tu_exact_c_ranges'][0]['size']='0x4'
                elif defect=='source_overlay':module['text_ownership'][0]['source']='overlays/o009/mixed'
                with self.assertRaises(rs.SurfaceComparisonError):self.resolve(root,*args)

    def test_nonfunction_or_abs_symbols_cannot_authenticate(self):
        for which, bad_type, bad_section in (
                ('canonical', 1, 1), ('linked', 1, 1),
                ('canonical', rs.STT_FUNC, rs.SHN_ABS),
                ('linked', rs.STT_FUNC, rs.SHN_ABS)):
            with self.subTest(which=which, section=bad_section),tempfile.TemporaryDirectory() as td:
                root=Path(td);args=self.fixture(root)
                elf=args[1] if which=='canonical' else args[2]
                n,v,s,info,idx=elf._symbols[0]
                elf._symbols[0]=(n,v,s,bad_type,bad_section)
                self.assertEqual({},self.resolve(root,*args))

    def test_changed_source_during_object_read_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td)
            atlas,canonical,linked,caller,source,obj=self.fixture(root)
            def mutate(_path):
                source.write_text('void removed(void) {}\n')
                return canonical
            with self.assertRaisesRegex(rs.SurfaceComparisonError, 'inputs changed'):
                rs._stable_overlay_call_identities(root/'missing',caller,1,linked,
                    atlas,0,4,root=root,elf_loader=mutate,rom=bytes(0x140))

    def test_snapshot_window_replacement_cannot_authenticate_old_inputs(self):
        for replaced in ('source', 'linked'):
            with self.subTest(replaced=replaced), tempfile.TemporaryDirectory() as td:
                root=Path(td);args=self.fixture(root)
                source,obj,linked=args[4],args[5],args[2]
                original=Path.rglob
                changed=False
                def replace_before_snapshot(path, pattern):
                    nonlocal changed
                    if path == root/'include' and not changed:
                        changed=True
                        if replaced == 'source':
                            source.write_text('void %s(void) { different_call(); }\n'%self.NAME)
                            os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
                        else:
                            linked.path.write_bytes(b'new linked ELF at the old path')
                    return original(path,pattern)
                with mock.patch.object(Path,'rglob',replace_before_snapshot):
                    self.assertEqual({},self.resolve(root,*args))
                self.assertTrue(changed)

    def test_old_source_local_include_is_supported_but_macro_include_is_not(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);args=self.fixture(root)
            source,obj=args[4],args[5]
            header=source.parent/'local.h';header.write_text('typedef int Local;\n')
            os.utime(header,ns=(obj.stat().st_mtime_ns-2000000,)*2)
            source.write_text('#include "local.h"\nvoid %s(void) {}\n'%self.NAME)
            os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
            self.assertEqual({self.NAME:(8,16)},self.resolve(root,*args))
            source.write_text('#include UNKNOWN_HEADER\nvoid %s(void) {}\n'%self.NAME)
            os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
            self.assertEqual({},self.resolve(root,*args))

    def test_outside_owner_is_unresolved(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);args=self.fixture(root)
            args[3]._symbols=[('func_overlay_008_F0000040_140',0,0,0,0)]
            self.assertEqual({},self.resolve(root,*args))

    def test_actual_alternate_include_recipe_is_used(self):
        import dataclasses
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);args=self.fixture(root)
            source,obj=args[4],args[5]
            header=root/'alternate/header.h';header.parent.mkdir()
            header.write_text('typedef int External;\n')
            source.write_text('#include <header.h>\nvoid %s(void) {}\n'%self.NAME)
            os.utime(source,ns=(obj.stat().st_mtime_ns-1000000,)*2)
            os.utime(header,ns=(obj.stat().st_mtime_ns-2000000,)*2)
            self.recipe_mock.return_value=dataclasses.replace(
                self.recipe_mock.return_value,compiler_args=('-nostdinc','-I','alternate'))
            self.assertEqual({self.NAME:(8,16)},self.resolve(root,*args))
            os.utime(header,ns=(obj.stat().st_mtime_ns+1000000,)*2)
            self.assertEqual({},self.resolve(root,*args))

    def test_recipe_change_during_inspection_is_rejected(self):
        import dataclasses
        with tempfile.TemporaryDirectory() as td:
            root=Path(td)
            atlas,canonical,linked,caller,source,obj=self.fixture(root)
            def change_recipe(_path):
                self.recipe_mock.return_value=dataclasses.replace(
                    self.recipe_mock.return_value,
                    compiler_args=('-nostdinc','-DCHANGED','-I','include'))
                return canonical
            with self.assertRaisesRegex(rs.SurfaceComparisonError, 'dependency set changed'):
                rs._stable_overlay_call_identities(root/'missing',caller,1,linked,
                    atlas,0,4,root=root,elf_loader=change_recipe,rom=bytes(0x140))

    def test_newer_normalization_spec_invalidates_callee_object(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);args=self.fixture(root)
            obj=args[5]
            spec=root/'config/normalizations/callee.rebind.spec'
            spec.parent.mkdir(parents=True)
            spec.write_text('synthetic metadata dependency\n')
            os.utime(spec,ns=(obj.stat().st_mtime_ns-2000000,)*2)
            self.assertEqual({self.NAME:(8,16)},self.resolve(root,*args))
            spec.write_text('changed synthetic metadata dependency\n')
            os.utime(spec,ns=(obj.stat().st_mtime_ns+1000000,)*2)
            self.assertEqual({},self.resolve(root,*args))


class OverlayIdentityExtentTests(unittest.TestCase):
    """The bound that keeps a resident absolute out of an overlay's namespace.

    splat names every unresolved absolute after its own value
    (`D_80000078 = 0x80000078`). Paired against a matched sibling those names
    can yield the absolute itself as a proposed overlay "offset", which then
    collides with the same name's resident identity. `promotion-proof` refused
    four byte-exact functions that way, two of them promoted weeks earlier.
    """

    MODULE = {"overlay": 58, "rom": {"size": "0x9568"}, "bss_size": "0x100"}

    def test_extent_is_rom_plus_bss(self):
        self.assertEqual(
            rs._overlay_identity_extent(self.MODULE), 0x9568 + 0x100)

    def test_missing_rows_yield_no_bound(self):
        """An atlas row that cannot supply both sizes skips the bound."""
        self.assertIsNone(rs._overlay_identity_extent({"overlay": 58}))
        self.assertIsNone(rs._overlay_identity_extent({}))

    def test_a_resident_absolute_is_outside_it(self):
        """0x80000078 against an overlay whose whole extent is 0x9568."""
        extent = rs._overlay_identity_extent(self.MODULE)
        self.assertFalse(0 <= 0x80005A98 < extent)

    def test_a_genuine_datum_is_inside_it(self):
        """Overlay data lies within the module by construction."""
        extent = rs._overlay_identity_extent(self.MODULE)
        self.assertTrue(0 <= 0x5EA0 < extent)


class ForeignTextValueTests(unittest.TestCase):
    """A value line must never name another module's own .text definition."""

    def test_foreign_text_names_are_found_with_their_owner(self):
        values = {"overlay65Initialize": (0xF0000000, "overlay_049.c.o"),
                  "overlay49TimerReloc": (0x28, "overlay_049.c.o")}
        local = {49: {"overlay49Initialize"}, 65: {"overlay65Initialize"}}
        self.assertEqual({"overlay65Initialize": ("overlay_049.c.o", 65)},
                         rs.foreign_text_values(values, local))

    def test_generate_refuses_the_value_line_and_reports_it(self):
        caller = Path("build/src/overlays/o049/overlay_049.c.o")
        callee = Path("build/src/overlays/o065/overlay65Initialize.c.o")
        atlas = {"modules": [
            {"overlay": 49, "text_ownership": [], "rom": {"start": "0x100"}},
            {"overlay": 65, "text_ownership": [], "rom": {"start": "0x200"}},
        ]}
        defs = {49: {"overlay49Initialize"}, 65: {"overlay65Initialize"}}

        def synthesize(obj, overlay, *_args):
            vals = ({"overlay65Initialize": 0xF0000000, "overlay49TimerReloc": 0x28}
                    if obj == caller else {})
            return [], vals, [], {"attempted": False}

        with contextlib.ExitStack() as stack:
            patch = stack.enter_context
            patch(mock.patch.object(rs.ot, "read_rom_table", return_value=[]))
            patch(mock.patch.object(rs.ot, "read_headers", return_value=[]))
            patch(mock.patch.object(rs.ot, "build_modules", return_value=[None] * 65))
            patch(mock.patch.object(rs.ot, "read_module_relocations", return_value=[]))
            patch(mock.patch.object(rs, "module_text_defs",
                                    side_effect=lambda _objs, ov, _rows: defs[ov]))
            patch(mock.patch.object(rs, "synthesize", side_effect=synthesize))
            patch(mock.patch.object(rs, "object_aliases", return_value=[]))
            text, diag = rs.generate(b"", atlas, objects=[(49, caller), (65, callee)],
                                     rebind_resident=False, mutate_objects=False)

        self.assertIn("overlay49TimerReloc = 0x00000028;", text)
        self.assertNotIn("overlay65Initialize =", text)
        self.assertEqual(1, len(diag["conflicts"]))
        obj, name, why = diag["conflicts"][0]
        self.assertEqual(("overlay_049.c.o", "overlay65Initialize"), (obj, name))
        self.assertIn("overlay 65", why)


class MisspelledGeneratedNameTests(unittest.TestCase):
    ATLAS = {"modules": [{"overlay": 45, "rom": {"start": "0x188C458"}}]}

    def test_a_wrong_rom_suffix_is_reported_with_the_right_name(self):
        rows = rs.misspelled_generated_names(self.ATLAS, [
            ("src/a.c", "x = func_overlay_045_F000000C_188B438(1);"),
            ("src/b.c", "y = func_overlay_045_F000000C_188C464(1);"),
            ("mk/c.mk", "--redefine-sym func_overlay_099_F0000000_1=z"),
        ])
        self.assertEqual([("src/a.c", "func_overlay_045_F000000C_188B438",
                           "func_overlay_045_F000000C_188C464")], rows)

    def test_the_tree_spells_no_misspelled_generated_name(self):
        atlas = json.loads((rs.REPO / "config" / "overlays.us.json").read_text())
        self.assertEqual([], rs.misspelled_generated_names(
            atlas, rs.tree_generated_name_texts()))


class ResidentVersusReservedTests(unittest.TestCase):
    """A resident address and a reserved-selector tuple are not one identity.

    overlay11InitializeFour's D_800D31BC reads statically as resident, while
    the shipped table reaches it through selector 0xFFD. The disagreement is
    an ambiguity for the linked-ROM route, not an abort; any other
    disagreement still aborts.
    """

    def test_only_resident_against_reserved_is_an_ambiguity(self):
        self.assertTrue(rs._resident_versus_reserved((0, 0x10), (0xFFD, 0x10)))
        self.assertTrue(rs._resident_versus_reserved((0xFFF, 4), (0, 8)))
        self.assertFalse(rs._resident_versus_reserved((0, 0x10), (11, 0x10)))
        self.assertFalse(rs._resident_versus_reserved((0xFFD, 0), (0xFFE, 0)))
        self.assertFalse(rs._resident_versus_reserved((0, 4), (0, 8)))


if __name__ == "__main__":
    unittest.main()
