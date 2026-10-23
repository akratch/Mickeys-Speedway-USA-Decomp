#!/usr/bin/env python3
"""Synthetic ELF tests for branch metadata and digest-guarded data removal."""
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import time
import unittest

import proof_provenance as provenance
from reloc_surface import Elf
from runlink.synth import ElfBuilder, words


TOOLS = Path(__file__).resolve().parent
LD = TOOLS / "binutils/mips64-elf-ld"


def fixture(path, *, anchor=0, displacement=1, opcode=4, rodata_flags=2):
    names = ["", ".text", ".rodata", ".rel.text", ".symtab", ".strtab", ".shstrtab"]
    name_blob = b"\0"
    name_offsets = [0]
    for name in names[1:]:
        name_offsets.append(len(name_blob))
        name_blob += name.encode() + b"\0"
    strings = b"\0branch_site\0callee\0"
    symbols = bytes(16)
    symbols += struct.pack(">IIIBBH", 1, anchor, 0, 0, 0, 1)
    symbols += struct.pack(">IIIBBH", 13, 0, 0, 16, 0, 0)
    text = struct.pack(">III", (opcode << 26) | (1 << 21) | displacement, 3 << 26, 0)
    payloads = [b"", text, b"keepdrop", struct.pack(">II", 4, (2 << 8) | 4),
                symbols, strings, name_blob]
    data = bytearray(52)
    data[:16] = b"\x7fELF\x01\x02\x01" + bytes(9)
    struct.pack_into(">HHIIIIIHHHHHH", data, 16, 1, 8, 1, 0, 0, 0, 0, 52, 0, 0, 40, 7, 6)
    headers = []
    for index, payload in enumerate(payloads):
        while len(data) % 4:
            data.append(0)
        offset = len(data)
        data.extend(payload)
        kind = [0, 1, 1, 9, 2, 3, 3][index]
        flags = 6 if index == 1 else rodata_flags if index == 2 else 0
        link = 4 if index == 3 else 5 if index == 4 else 0
        info = 1 if index == 3 else 2 if index == 4 else 0
        entsize = 8 if index == 3 else 16 if index == 4 else 0
        headers.append((name_offsets[index], kind, flags, 0, offset, len(payload), link, info, 4, entsize))
    while len(data) % 4:
        data.append(0)
    struct.pack_into(">I", data, 32, len(data))
    for header in headers:
        data.extend(struct.pack(">10I", *header))
    path.write_bytes(data)


class MetadataContracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / "synthetic.o"
        fixture(self.path)

    def run_tool(self, tool, *args):
        env = dict(os.environ)
        env.pop("PROMOTION_TRIAL", None)
        return subprocess.run([sys.executable, str(TOOLS / tool), str(self.path), *args],
                              env=env, capture_output=True, text=True)

    def add_branch(self, digest=None, spec="0:PC16:branch_site:1"):
        if digest is None:
            digest = hashlib.sha256(Elf(self.path).section_bytes(".text")).hexdigest()
        return self.run_tool("add_elf_relocations.py", ".text", "12", digest, spec)

    def test_branch_preserves_bytes_symbols_and_existing_relocation(self):
        before = Elf(self.path)
        result = self.add_branch()
        self.assertEqual(result.returncode, 0, result.stderr)
        after = Elf(self.path)
        self.assertEqual(before.symbols(), after.symbols())
        for name in (".text", ".rodata", ".strtab", ".shstrtab"):
            self.assertEqual(before.section_bytes(name), after.section_bytes(name))
        self.assertEqual(after.relocations(), [(".text", 0, 10, 1), *before.relocations()])
        # S == P means the PC16 link formula leaves the stored field intact.
        branch_site = after.symbols()[1][1]
        field = struct.unpack_from(">I", after.section_bytes(".text"))[0] & 0xFFFF
        self.assertEqual((branch_site + field * 4 - 0) // 4, field)

    def test_branch_rejects_wrong_anchor_opcode_destination_addend_or_hash(self):
        for kwargs, spec, digest in [
            ({"anchor": 4}, "0:PC16:branch_site:1", None),
            ({"opcode": 8}, "0:PC16:branch_site:1", None),
            ({"displacement": 8}, "0:PC16:branch_site:8", None),
            ({}, "0:PC16:branch_site:2", None),
            ({}, "0:PC16:branch_site:1", "0" * 64),
        ]:
            with self.subTest(kwargs=kwargs, spec=spec, digest=digest):
                fixture(self.path, **kwargs)
                before = self.path.read_bytes()
                self.assertNotEqual(self.add_branch(digest, spec).returncode, 0)
                self.assertEqual(self.path.read_bytes(), before)

    def test_branch_rejects_duplicate(self):
        self.assertEqual(self.add_branch().returncode, 0)
        before = self.path.read_bytes()
        self.assertNotEqual(self.add_branch().returncode, 0)
        self.assertEqual(self.path.read_bytes(), before)

    def test_digest_trim_keeps_prefix_and_instruction_bytes(self):
        before = Elf(self.path)
        digest = hashlib.sha256(b"drop").hexdigest()
        result = self.run_tool("trim_elf_section.py", ".rodata", "4", "sha256:" + digest)
        self.assertEqual(result.returncode, 0, result.stderr)
        after = Elf(self.path)
        self.assertEqual(after.section_bytes(".rodata"), b"keep")
        self.assertEqual(before.section_bytes(".text"), after.section_bytes(".text"))
        self.assertEqual(before.relocations(), after.relocations())

    def test_digest_trim_rejects_mismatch_malformed_and_executable_sections(self):
        for section, digest, flags in [
            (".rodata", "0" * 64, 2), (".rodata", "bad", 2),
            (".rodata", hashlib.sha256(b"drop").hexdigest(), 6),
            (".text", "0" * 64, 2),
        ]:
            with self.subTest(section=section, digest=digest, flags=flags):
                fixture(self.path, rodata_flags=flags)
                before = self.path.read_bytes()
                self.assertNotEqual(self.run_tool("trim_elf_section.py", section, "4", "sha256:" + digest).returncode, 0)
                self.assertEqual(self.path.read_bytes(), before)

    def test_default_trim_still_rejects_nonzero_data(self):
        before = self.path.read_bytes()
        self.assertNotEqual(self.run_tool("trim_elf_section.py", ".rodata", "4").returncode, 0)
        self.assertEqual(self.path.read_bytes(), before)


def section_anchor_fixture(path, *, executable=False):
    # Synthetic HI16/LO16 pairs include a carry that ordinary ld must resolve.
    obj = ElfBuilder().section(".text", words(
        0x3C010000, 0x24210000, 0x3C020000, 0x24427FF0))
    obj.section(".rodata", struct.pack(">ff", 1.25, 2.5))
    for offset, kind in ((0, 5), (4, 6), (8, 5), (12, 6)):
        obj.rel(".text", offset, kind, ("section", ".rodata"))
    data = bytearray(obj.build())
    shoff = struct.unpack_from(">I", data, 32)[0]
    struct.pack_into(">I", data, shoff + 2 * 40 + 8, 6 if executable else 2)
    path.write_bytes(data)


class SectionAnchorContracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.path = self.root / "anchor.o"
        section_anchor_fixture(self.path)

    def externalize(self, *extras, expected=None, section=".rodata"):
        if expected is None:
            expected = "sha256:" + hashlib.sha256(
                Elf(self.path).section_bytes(section)).hexdigest()
        return subprocess.run(
            [sys.executable, str(TOOLS / "externalize_elf_section.py"),
             str(self.path), section, expected, *extras],
            capture_output=True, text=True)

    def test_zero_and_nonzero_anchors_only_change_declared_metadata(self):
        for extras, anchor, keep in (((), 0, False), (("0xe8",), 0xe8, False),
                                     (("keep", "0xe8"), 0xe8, True),
                                     (("0", "keep"), 0, True)):
            with self.subTest(extras=extras):
                section_anchor_fixture(self.path)
                before_bytes = self.path.read_bytes()
                before = Elf(self.path)
                result = self.externalize(*extras)
                self.assertEqual(result.returncode, 0, result.stderr)
                after_bytes = self.path.read_bytes()
                after = Elf(self.path)
                self.assertEqual(before.section_bytes(".text"), after.section_bytes(".text"))
                self.assertEqual(before.section_bytes(".rel.text"), after.section_bytes(".rel.text"))
                self.assertEqual(before.relocations(), after.relocations())
                target_index = before.section(".rodata")[0]
                symbol_index = next(i for i, row in enumerate(before.symbols())
                                    if row[4] == target_index and row[3] & 15 == 3)
                symtab = before.section(".symtab")[1]
                symbol_offset = symtab[4] + symbol_index * symtab[9]
                permitted = set(range(symbol_offset + 4, symbol_offset + 12))
                permitted.update(range(symbol_offset + 14, symbol_offset + 16))
                if anchor:
                    permitted.add(symbol_offset + 12)
                if not keep:
                    shoff = struct.unpack_from(">I", before_bytes, 32)[0]
                    permitted.update(range(shoff + target_index * 40 + 20,
                                           shoff + target_index * 40 + 24))
                self.assertEqual(len(before_bytes), len(after_bytes))
                self.assertTrue(all(i in permitted for i, (a, b) in
                                    enumerate(zip(before_bytes, after_bytes)) if a != b))
                self.assertEqual(after.symbols()[symbol_index][1:], (anchor, 0, 0 if anchor else 3, 0xfff1))
                self.assertEqual(after.section_bytes(".rodata"),
                                 before.section_bytes(".rodata") if keep else b"")

    @unittest.skipUnless(LD.is_file(), "needs MIPS GNU ld")
    def test_ordinary_link_applies_anchor_and_original_addends(self):
        original_text = Elf(self.path).section_bytes(".text")
        result = self.externalize("0xe8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(original_text, Elf(self.path).section_bytes(".text"))
        output = self.root / "linked.elf"
        linked = subprocess.run([str(LD), "-m", "elf32ebmip", "-e", "0",
                                 "-Ttext", "0", "-o", str(output), str(self.path)],
                                capture_output=True, text=True)
        self.assertEqual(linked.returncode, 0, linked.stderr)
        text = Elf(output).section_bytes(".text")
        self.assertEqual(struct.unpack(">4I", text),
                         (0x3C010000, 0x242100E8, 0x3C020001, 0x244280D8))

    def test_existing_proof_parser_and_replay_preserve_anchored_object(self):
        digest = hashlib.sha256(Elf(self.path).section_bytes(".rodata")).hexdigest()
        command = (f"{sys.executable} tools/externalize_elf_section.py input.o "
                   f".rodata sha256:{digest} 0xe8 keep")
        plan = provenance.metadata_filter_plan(command, "input.o")
        self.assertEqual(plan, [("externalize", (".rodata", "sha256:" + digest, 0xe8))])
        output = provenance.replay_metadata(
            self.path, command, "input.o", self.root / "replayed.o", TOOLS.parent,
            skip_filters=True, deadline=time.monotonic() + 30, plan=plan)
        original = self.path.read_bytes()
        result = self.externalize("0xe8", "keep")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output.read_bytes(), self.path.read_bytes())
        self.assertNotEqual(original, output.read_bytes())

    def test_guards_refuse_without_writing(self):
        for extras, expected, executable, section in (
                (("-1",), None, False, ".rodata"),
                (("0x8000",), None, False, ".rodata"),
                (("bad",), None, False, ".rodata"),
                (("keep", "keep"), None, False, ".rodata"),
                (("0", "1"), None, False, ".rodata"),
                ((), "sha256:bad", False, ".rodata"),
                ((), "sha256:" + "0" * 64, False, ".rodata"),
                ((), "not-hex", False, ".rodata"),
                ((), None, True, ".rodata"),
                ((), None, False, ".text")):
            with self.subTest(extras=extras, expected=expected, section=section):
                section_anchor_fixture(self.path, executable=executable)
                before = self.path.read_bytes()
                result = self.externalize(*extras, expected=expected, section=section)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(before, self.path.read_bytes())


if __name__ == "__main__":
    unittest.main()
