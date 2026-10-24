#!/usr/bin/env python3
"""tools/postprocess_audit.py: classification, credit refusal, object reading.

Synthetic commands, rows and ELF objects only; no build, no Makefile scan.
"""
import contextlib
import io
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import postprocess_audit as pa

DIGEST = "sha256:" + "0" * 64
EXTERNALIZE = "$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata " + DIGEST


class ClassifyTests(unittest.TestCase):
    def test_anchored_externalization_is_metadata(self):
        # The helper sets an ABS section-symbol value; ld applies relocations.
        for anchor in ("0x20", "4", "0x3C"):
            with self.subTest(anchor=anchor):
                self.assertEqual(pa.classify(EXTERNALIZE + " " + anchor)[0], "metadata")
                self.assertEqual(pa.classify(EXTERNALIZE + " keep " + anchor)[0], "metadata")

    def test_unanchored_externalization_is_metadata(self):
        for extra in ("", " 0", " keep", " 0x0 keep"):
            with self.subTest(extra=extra):
                self.assertEqual(pa.classify(EXTERNALIZE + extra)[0], "metadata")

    def test_pool_base_rebind_is_metadata(self):
        # The replacement for an anchor: an absolute pool base symbol, the
        # .rodata relocations rebound to it, and the private copy dropped.
        command = (
            "$(OBJCOPY) --redefine-sym sqrtf=sqrtf_o079Reloc "
            "--add-symbol gOverlay79PoolReloc=0x30,global $@ && "
            "$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text "
            "0x34:.rodata:gOverlay79PoolReloc 0x38:.rodata:gOverlay79PoolReloc && "
            "$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E0 && "
            + EXTERNALIZE + " && $(OBJCOPY) --remove-section .rel.rodata $@"
        )
        cls, tools = pa.classify(command)
        self.assertEqual(cls, "metadata")
        self.assertEqual(
            tools,
            ["rebind_elf_relocations.py", "trim_elf_section.py",
             "externalize_elf_section.py", "objcopy"],
        )

    def test_guarded_nobits_tail_trim_is_metadata(self):
        command = "$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_bss.py $@ gravity 4"
        self.assertEqual(("metadata", ["trim_elf_bss.py"]), pa.classify(command))
        # Reviewing this helper does not extend to a new helper in the chain.
        self.assertEqual("review-required", pa.classify(
            command + " && python unknown_helper.py $@")[0])

    def test_instruction_tools_and_section_rewrites_are_altered(self):
        for command in (
            "$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ x",
            "$(HOST_PYTHON) $(TOOLS_DIR)/normalize_o63_example.py $@",
            "$(HOST_PYTHON) $(TOOLS_DIR)/patch_elf_words.py $@ 0x0:0",
            "$(OBJCOPY) --update-section .text=blob.bin $@",
            "$(OBJCOPY) --remove-section .text $@",
        ):
            with self.subTest(command=command):
                self.assertEqual(pa.classify(command)[0], "altered")

    def test_unknown_or_malformed_helpers_need_review(self):
        for command in (
            "$(HOST_PYTHON) $(TOOLS_DIR)/brand_new_helper.py $@",
            EXTERNALIZE + " keep keep",
            EXTERNALIZE + " 0x10 0x20",
            EXTERNALIZE + " anchor",
            EXTERNALIZE + " -1",
            EXTERNALIZE + " 0x8000",
        ):
            with self.subTest(command=command):
                self.assertEqual(pa.classify(command)[0], "review-required")


def row(obj, cls, *, overlay=None, size=None, matched_c=None):
    return {"object": obj, "class": cls, "overlay": overlay, "size": size,
            "matched_c": matched_c, "tools": ["tool.py"]}


class CreditRefusalTests(unittest.TestCase):
    def test_only_credited_altered_rows_are_refused(self):
        rows = [
            row("build/ovl_matched.c.o", "altered", overlay=1, size=8, matched_c=True),
            row("build/ovl_guarded.c.o", "altered", overlay=1, size=8, matched_c=False),
            row("build/ovl_unjoined.c.o", "altered", overlay=1),
            row("build/resident.c.o", "altered"),
            row("build/ovl_clean.c.o", "metadata", overlay=1, size=8, matched_c=True),
            row("build/resident_clean.c.o", "metadata"),
            row("build/unknown.c.o", "review-required"),
        ]
        self.assertEqual(
            [r["object"] for r in pa.credit_refusals(rows)],
            ["build/ovl_matched.c.o", "build/ovl_unjoined.c.o", "build/resident.c.o"],
        )

    def test_check_credit_exit_status(self):
        cases = (
            ([row("build/a.c.o", "metadata", overlay=1, size=8, matched_c=True)], 0),
            ([row("build/a.c.o", "altered", overlay=1, size=8, matched_c=True)], 1),
            ([row("build/r.c.o", "altered")], 1),
        )
        for rows, expected in cases:
            with self.subTest(rows=rows), \
                    patch.object(pa, "audit", return_value=(rows, {})), \
                    patch.object(sys, "argv", ["postprocess_audit.py", "--check-credit"]), \
                    contextlib.redirect_stdout(io.StringIO()), \
                    contextlib.redirect_stderr(io.StringIO()) as err:
                self.assertEqual(pa.main(), expected)
                self.assertEqual(bool(err.getvalue()), bool(expected))


class AuditInputTests(unittest.TestCase):
    def test_effective_commands_come_from_the_make_database(self):
        dump = "\n".join((
            "build/src/a.c.o: POSTPROCESS = first",
            "build/src/a.c.o: POSTPROCESS = second",
            "build/src/b.c.o: POSTPROCESS = @:",
            "src/c.c: POSTPROCESS = ignored",
            "build/src/d.c.o: CFLAGS += -g",
        ))
        self.assertEqual(pa.postprocess_commands(dump), {"build/src/a.c.o": "second"})

    def test_an_empty_rule_set_refuses(self):
        # Without this a failed `gmake -p -q` would read as zero altered objects.
        with patch.object(pa, "run_make_database", return_value=""), \
                self.assertRaisesRegex(RuntimeError, "no POSTPROCESS rules"):
            pa.audit()


def elf_object(symbols):
    """A minimal big-endian ELF32 relocatable with .text, .data and a symtab.

    ``symbols`` is ``[(name, st_info, section_name)]``; section ``None`` is
    SHN_UNDEF.
    """
    shstr = b"\0.text\0.data\0.symtab\0.strtab\0.shstrtab\0"
    def shname(name):
        return shstr.index(name.encode() + b"\0")
    section_index = {".text": 1, ".data": 2, None: 0}
    strtab = b"\0"
    entries = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    for name, info, section in symbols:
        entries.append(struct.pack(">IIIBBH", len(strtab), 0, 4, info, 0,
                                   section_index[section]))
        strtab += name.encode() + b"\0"
    symtab = b"".join(entries)
    text, data = b"\0" * 8, b"\0" * 4
    body = [text, data, symtab, strtab, shstr]
    offsets, cursor = [], 0x34
    for blob in body:
        offsets.append(cursor)
        cursor += len(blob)
    shoff = cursor
    header = bytearray(0x34)
    header[:6] = b"\x7fELF\x01\x02"
    struct.pack_into(">HH", header, 16, 1, 8)
    struct.pack_into(">I", header, 0x20, shoff)
    struct.pack_into(">HHH", header, 0x2E, 40, 6, 5)
    sections = [struct.pack(">10I", *([0] * 10))]
    layout = (  # name, type, link, entsize
        (".text", 1, 0, 0), (".data", 1, 0, 0), (".symtab", 2, 4, 16),
        (".strtab", 3, 0, 0), (".shstrtab", 3, 0, 0),
    )
    for (name, kind, link, entsize), offset, blob in zip(layout, offsets, body):
        sections.append(struct.pack(">10I", shname(name), kind, 0, 0, offset,
                                    len(blob), link, 0, 4, entsize))
    return bytes(header) + b"".join(body) + b"".join(sections)


class ObjectTextFunctionsTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / "unit.c.o"

    def test_names_only_functions_defined_in_text(self):
        self.path.write_bytes(elf_object([
            ("globalFunc", 0x12, ".text"),   # STB_GLOBAL | STT_FUNC
            ("staticFunc", 0x02, ".text"),   # STB_LOCAL | STT_FUNC
            ("textLabel", 0x10, ".text"),    # STT_NOTYPE
            ("dataObject", 0x11, ".data"),   # STT_OBJECT
            ("dataFunc", 0x12, ".data"),     # function symbol outside .text
            ("importedFunc", 0x12, None),    # undefined
        ]))
        self.assertEqual(pa.object_text_functions(self.path), ["globalFunc", "staticFunc"])

    def test_missing_or_foreign_objects_refuse(self):
        with self.assertRaises(OSError):
            pa.object_text_functions(self.path)
        self.path.write_bytes(b"\x7fELF\x02\x01" + b"\0" * 64)
        with self.assertRaisesRegex(RuntimeError, "big-endian ELF32"):
            pa.object_text_functions(self.path)


class CompilerEndPaddingTests(unittest.TestCase):
    def object(self, *, data=False, missing=False, width=False, nonzero=False):
        from runlink.synth import ElfBuilder, GLOBAL, FUNC, words
        import runlink_pack as rp
        if data:
            builder = ElfBuilder().section(".data", bytes(48)).sym("text", 0, 0, GLOBAL, 1, ".data")
            builder.sym("lastFloat", 40, 4, GLOBAL, 1, ".data")
        else:
            builder = ElfBuilder().section(".text", words(0x2D010002, 0x10200006,
                0x00084880, 0x3C010000, 0x00290821, 0xDC390000 if width else 0x8C390000,
                0x03200008, 0)).section(".rodata", words(24,28,1 if nonzero else 0,0))
            builder.sym("function", 0, 32, GLOBAL, FUNC, ".text")
            builder.rel(".text",12,rp.R_MIPS_HI16,("section",".rodata"))
            builder.rel(".text",20,rp.R_MIPS_LO16,("section",".rodata"))
            if not missing: builder.rel(".rodata",0,rp.R_MIPS_32,("section",".text"))
            builder.rel(".rodata",4,rp.R_MIPS_32,("section",".text"))
        payload=bytearray(builder.build());elf=rp.Elf("fixture",payload)
        for section in elf.sections:
            if section["name"] in (".data", ".rodata"):
                struct.pack_into(">I",payload,struct.unpack_from(">I",payload,32)[0]+section["index"]*40+8,3 if section["name"]==".data" else 2)
        return rp.Elf("fixture",payload)

    def test_real_sized_end_and_complete_bounded_table_derive_padding(self):
        import trim_elf_section as trim
        self.assertEqual(44,trim.compiler_end(self.object(data=True),".data"))
        self.assertEqual(8,trim.compiler_end(self.object(),".rodata"))

    def test_effective_view_retains_original_and_requires_explicit_profile(self):
        import runlink_pack as rp
        raw=self.object(data=True);data=bytearray(raw.data)
        struct.pack_into(">I",data,struct.unpack_from(">I",data,32)[0]+raw.section(".data")["index"]*40+20,44)
        configured=rp.Elf("configured",data)
        with self.assertRaises(rp.PackError): rp.effective_compiler_object(raw,configured)
        view=rp.effective_compiler_object(raw,configured,{".data":44})
        self.assertIs(view.compiler_original,raw)
        self.assertEqual((raw.size(".data"),view.size(".data")),(48,44))
        self.assertEqual(view.symbols,raw.symbols)
        self.assertEqual(view.compiler_original.data,raw.data)

    def test_lost_relocation_wide_read_nonzero_and_executable_refuse(self):
        import trim_elf_section as trim
        for kwargs in ({"missing":True},{"width":True},{"nonzero":True}):
            with self.subTest(kwargs=kwargs),self.assertRaises(ValueError):
                trim.compiler_end(self.object(**kwargs),".rodata")
        with self.assertRaises(ValueError): trim.compiler_end(self.object(),".text")


if __name__ == "__main__":
    unittest.main()
