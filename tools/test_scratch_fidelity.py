#!/usr/bin/env python3
"""Tests for tools/scratch_fidelity.py: synthetic C and synthetic ELF objects only."""

from __future__ import annotations

import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import scratch_fidelity as sf  # noqa: E402

ROOT = TOOLS.parent
PERMUTER = ROOT / "tools" / "permuter"
IDO_CC = ROOT / "tools" / "ido" / "cc"


def _permuter():
    """(ast_util, c_ast) from the local decomp-permuter checkout, or None."""
    try:
        if str(PERMUTER) not in sys.path:
            sys.path.insert(0, str(PERMUTER))
        from perm_pycparser import c_ast
        from src import ast_util
        return ast_util, c_ast
    except Exception:
        return None


HAVE_PERMUTER = _permuter() is not None

# The anim.c shape in miniature: a header declaration, a K&R definition whose
# parameter is a byte, a block-scope prototype inside an earlier function, and
# a target whose call relies on that prototype for its u8 conversion.
LEAK_TU = """typedef unsigned char u8;
void callee();
void callee(a)
u8 a;
{
}
void earlier(void)
{
  void callee(u8 a);
}
void knr(b)
int b;
{
}
void target(int x)
{
  callee(x);
  knr(x);
}
"""


def restored_source(text: str, fn: str = "target", restore: bool = True) -> tuple[str, list[dict]]:
    ast_util, c_ast = _permuter()
    ast = ast_util.parse_c(text)
    leaked = sf.leaked_prototypes(ast, fn, c_ast)
    knr = sf.knr_names(ast, fn, c_ast)
    target, index = ast_util.extract_fn(ast, fn)
    notes: list[dict] = []
    if restore:
        _index, notes = sf.apply_restorations(ast, target, index, leaked, knr, c_ast)
    return ast_util.to_c_raw(ast), notes


@unittest.skipUnless(HAVE_PERMUTER, "tools/permuter (decomp-permuter checkout) unavailable")
class RestorationTests(unittest.TestCase):
    def test_leaked_prototype_rewrites_unprototyped_declarations(self):
        out, notes = restored_source(LEAK_TU)
        self.assertEqual(out.count("void callee(u8 a);"), 2, out)
        self.assertNotIn("callee();", out)
        self.assertNotIn("callee(a);", out)
        leak = [n for n in notes if not n.get("knr")]
        self.assertEqual(len(leak), 1)
        self.assertEqual(leak[0]["from"], "earlier")
        self.assertEqual(leak[0]["rewritten"], 2)
        self.assertFalse(leak[0]["inserted"])

    def test_knr_identifier_list_is_dropped(self):
        out, notes = restored_source(LEAK_TU)
        self.assertIn("void knr();", out)
        self.assertNotIn("knr(b);", out)
        self.assertIn({"name": "knr", "knr": True}, notes)

    def test_target_body_is_untouched(self):
        plain, _ = restored_source(LEAK_TU, restore=False)
        out, _ = restored_source(LEAK_TU)
        body = lambda s: s[s.index("void target(int x)"):]  # noqa: E731
        self.assertEqual(body(plain), body(out))

    def test_without_restoration_the_importer_loses_the_prototype(self):
        plain, _ = restored_source(LEAK_TU, restore=False)
        self.assertNotIn("u8 a);", plain)
        self.assertIn("void callee(a);", plain)

    def test_prototype_declared_after_the_target_is_not_restored(self):
        text = LEAK_TU.replace("void earlier(void)\n{\n  void callee(u8 a);\n}\n", "")
        text += "void later(void)\n{\n  void callee(u8 a);\n}\n"
        out, notes = restored_source(text)
        self.assertNotIn("u8 a);", out)
        self.assertFalse([n for n in notes if not n.get("knr")])

    def test_prototype_with_no_file_scope_declaration_is_inserted_before_target(self):
        text = ("typedef unsigned char u8;\nvoid earlier(void)\n{\n  void lone(u8 a);\n}\n"
                "void target(int x)\n{\n  lone(x);\n}\n")
        out, notes = restored_source(text)
        self.assertLess(out.index("void lone(u8 a);"), out.index("void target(int x)"))
        self.assertTrue(notes[0]["inserted"])
        self.assertIn("inserted before the target", sf.describe(notes[0]))

    def test_restoring_context_installs_and_removes_the_wrapper(self):
        ast_util, c_ast = _permuter()
        original = ast_util.extract_fn
        with sf.restoring(ast_util, c_ast) as notes:
            self.assertIsNot(ast_util.extract_fn, original)
            ast = ast_util.parse_c(LEAK_TU)
            ast_util.extract_fn(ast, "target")
            self.assertIn("void callee(u8 a);", ast_util.to_c_raw(ast))
        self.assertIs(ast_util.extract_fn, original)
        self.assertTrue(any(n.get("from") == "earlier" for n in notes))

    def test_permute_batch_imports_through_the_restoration(self):
        text = (TOOLS / "permute_batch.py").read_text()
        self.assertIn("scratch_fidelity.restoring(ast_util, c_ast)", text)
        self.assertIn('"scratch_fidelity"', text, "receipts must digest the restoration module")

    def test_unprototyped_block_declaration_is_not_a_leak(self):
        text = LEAK_TU.replace("void callee(u8 a);", "void callee();")
        _out, notes = restored_source(text)
        self.assertFalse([n for n in notes if not n.get("knr")])


@unittest.skipUnless(HAVE_PERMUTER and IDO_CC.is_file(), "decomp-permuter or stock IDO unavailable")
class IdoSemanticsTests(unittest.TestCase):
    """The claim the restoration rests on, measured on IDO itself."""

    def compile(self, text: str, tmp: Path, name: str) -> Path:
        src, obj = tmp / f"{name}.c", tmp / f"{name}.o"
        src.write_text(text)
        subprocess.run([str(IDO_CC), "-c", "-O2", "-mips2", "-G", "0", "-non_shared", "-woff", "649,838",
                        str(src), "-o", str(obj)], cwd=ROOT, check=True, capture_output=True)
        return obj

    def test_restored_scratch_equals_the_full_tu_and_the_plain_one_does_not(self):
        with tempfile.TemporaryDirectory() as d:
            tmp = Path(d)
            tree = self.compile(LEAK_TU, tmp, "tree")
            fixed = self.compile(restored_source(LEAK_TU)[0], tmp, "fixed")
            plain = self.compile(restored_source(LEAK_TU, restore=False)[0], tmp, "plain")
            self.assertTrue(sf.compare_objects("target", fixed, tree)["identical"])
            lost = sf.compare_objects("target", plain, tree)
            self.assertFalse(lost["identical"])
            self.assertTrue(sf.render("target", lost).startswith("SCRATCH FIDELITY: "))


# ------------------------------------------------------------- synthetic ELF

def elf_object(functions: dict[str, list[int]], relocs: list[tuple[int, int, str]] = (),
               sections: tuple[str, ...] = (".rodata",)) -> bytes:
    """A big-endian ELF32 relocatable: .text with the named functions back to
    back, one SECTION symbol per extra section, and REL records (offset, type,
    symbol name or section name) against .text."""
    text = b"".join(struct.pack(">I", w) for words in functions.values() for w in words)
    names = ["", ".text", ".rel.text", ".symtab", ".strtab", ".shstrtab", *sections]
    shstr = b"\0".join(n.encode() for n in names) + b"\0"
    shname = {}
    pos = 0
    for n in names:
        shname[n] = pos
        pos += len(n.encode()) + 1
    strtab = b"\0"
    symbols = [(0, 0, 0, 0, 0)]  # name offset, value, size, info, shndx
    index_of = {}
    for sec in (".text", *sections):
        index_of[sec] = len(symbols)
        symbols.append((0, 0, 0, sf.STT_SECTION, names.index(sec)))
    offset = 0
    for fn, words in functions.items():
        index_of[fn] = len(symbols)
        symbols.append((len(strtab), offset, len(words) * 4, (1 << 4) | sf.STT_FUNC, 1))
        strtab += fn.encode() + b"\0"
        offset += len(words) * 4
    for _off, _kind, target in relocs:
        if target not in index_of:
            index_of[target] = len(symbols)
            symbols.append((len(strtab), 0, 0, (1 << 4), 0))
            strtab += target.encode() + b"\0"
    symtab = b"".join(struct.pack(">IIIBBH", n, v, s, i, 0, x) for n, v, s, i, x in symbols)
    rel = b"".join(struct.pack(">II", off, (index_of[t] << 8) | kind) for off, kind, t in relocs)
    blobs = {".text": text, ".rel.text": rel, ".symtab": symtab, ".strtab": strtab,
             ".shstrtab": shstr, **{s: b"" for s in sections}}
    data = bytearray(52)
    placed = {}
    for n in names[1:]:
        placed[n] = len(data)
        data += blobs[n]
        data += b"\0" * (-len(data) % 4)
    shoff = len(data)
    headers = [b"\0" * 40]
    for n in names[1:]:
        kind = {".text": 1, ".rel.text": sf.SHT_REL, ".symtab": 2, ".strtab": 3, ".shstrtab": 3}.get(n, 1)
        link = names.index(".symtab") if n == ".rel.text" else names.index(".strtab") if n == ".symtab" else 0
        info = 1 if n == ".rel.text" else 0
        entsize = 8 if n == ".rel.text" else 16 if n == ".symtab" else 0
        headers.append(struct.pack(">10I", shname[n], kind, 0, 0, placed[n], len(blobs[n]), link, info, 4, entsize))
    data += b"".join(headers)
    struct.pack_into(">4sBBBB", data, 0, b"\x7fELF", 1, 2, 1, 0)
    struct.pack_into(">HHIIIIIHHHHHH", data, 16, 1, 8, 1, 0, 0, shoff, 0, 52, 0, 0, 40, len(names),
                     names.index(".shstrtab"))
    return bytes(data)


class ComparisonTests(unittest.TestCase):
    LUI, LW, JAL, NOP, ANDI, JR = 0x3C040000, 0x8C850010, 0x0C000000, 0, 0x308400FF, 0x03E00008

    def objects(self, scratch: bytes, tree: bytes):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        a, b = Path(tmp.name) / "scratch.o", Path(tmp.name) / "tree.o"
        a.write_bytes(scratch)
        b.write_bytes(tree)
        return a, b

    def test_identical_function_reports_identical(self):
        body = [self.LUI, self.LW, self.JAL, self.NOP, self.JR, self.NOP]
        relocs = [(0, sf.R_MIPS_HI16, "D_1"), (4, sf.R_MIPS_LO16, "D_1"), (8, sf.R_MIPS_26, "callee")]
        a, b = self.objects(elf_object({"fn": body}, relocs), elf_object({"fn": body}, relocs))
        result = sf.compare_objects("fn", a, b)
        self.assertTrue(result["identical"])
        self.assertEqual(sf.render("fn", result),
                         "SCRATCH FIDELITY: identical (fn: 6 words, 3 relocation records)")

    def test_section_relative_addend_is_not_codegen(self):
        # A pruned scratch moves the function's literals within .rodata: the
        # in-place addend differs, the record and the instruction do not.
        relocs = [(0, sf.R_MIPS_HI16, ".rodata"), (4, sf.R_MIPS_LO16, ".rodata")]
        a, b = self.objects(elf_object({"fn": [self.LUI, self.LW | 0x0010]}, relocs),
                            elf_object({"fn": [self.LUI | 1, self.LW | 0x0230]}, relocs))
        self.assertTrue(sf.compare_objects("fn", a, b)["identical"])

    def test_one_differing_word_is_located(self):
        a, b = self.objects(elf_object({"fn": [self.NOP, self.ANDI, self.JR, self.NOP]}),
                            elf_object({"fn": [self.NOP, self.ANDI ^ 0x80, self.JR, self.NOP]}))
        result = sf.compare_objects("fn", a, b)
        self.assertFalse(result["identical"])
        self.assertEqual(result["sites"], [4])
        self.assertTrue(sf.render("fn", result).startswith("SCRATCH FIDELITY: 1 words off at +0x4 "))

    def test_relocation_target_difference_is_reported(self):
        body = [self.JAL, self.NOP, self.JR, self.NOP]
        a, b = self.objects(elf_object({"fn": body}, [(0, sf.R_MIPS_26, "alpha")]),
                            elf_object({"fn": body}, [(0, sf.R_MIPS_26, "beta")]))
        result = sf.compare_objects("fn", a, b)
        self.assertEqual(result["reloc_sites"], [0])
        self.assertIn("relocation records differ at 1 site(s)", sf.render("fn", result))

    def test_text_section_jump_resolves_to_the_function_it_lands_on(self):
        # The tree calls a TU-local function through the .text SECTION symbol;
        # the scratch, whose callee is only declared, calls it by name.
        tree = elf_object({"callee": [self.JR, self.NOP], "fn": [self.JAL, self.NOP, self.JR, self.NOP]},
                          [(8, sf.R_MIPS_26, ".text")])
        scratch = elf_object({"fn": [self.JAL, self.NOP, self.JR, self.NOP]}, [(0, sf.R_MIPS_26, "callee")])
        a, b = self.objects(scratch, tree)
        self.assertTrue(sf.compare_objects("fn", a, b)["identical"])

    def test_size_mismatch_counts_after_alignment(self):
        tree = [self.NOP, self.LW, self.ANDI, self.JAL, self.JR, self.NOP]
        scratch = [self.NOP, self.LW, self.JAL, self.JR, self.NOP]
        a, b = self.objects(elf_object({"fn": scratch}), elf_object({"fn": tree}))
        result = sf.compare_objects("fn", a, b)
        self.assertEqual(result["count"], 1)
        self.assertEqual(result["sites"], [8])
        self.assertIn("sizes differ", sf.render("fn", result))

    def test_missing_symbol_is_an_error(self):
        a, b = self.objects(elf_object({"fn": [self.NOP]}), elf_object({"fn": [self.NOP]}))
        with self.assertRaises(sf.FidelityError):
            sf.compare_objects("absent", a, b)

    def test_compare_cli_exit_status(self):
        a, b = self.objects(elf_object({"fn": [self.NOP, self.JR]}), elf_object({"fn": [self.NOP, self.NOP]}))
        self.assertEqual(sf.main(["compare", "fn", str(a), str(b)]), 3)
        self.assertEqual(sf.main(["compare", "fn", str(a), str(a)]), 0)


class CompileScriptTests(unittest.TestCase):
    def test_postcompile_lines_are_the_objcopy_steps_after_cc(self):
        csh = ('#!/usr/bin/env bash\nINPUT="$(realpath "$1")"\nOUTPUT="$(realpath "$3")"\ncd /repo\n'
               'tools/ido/cc -O2 -mips2 -c "$INPUT" -o "$OUTPUT"\n'
               'tools/binutils/mips64-elf-objcopy --redefine-sym a=b "$OUTPUT"\n')
        self.assertEqual(sf.postcompile_lines(csh),
                         ['tools/binutils/mips64-elf-objcopy --redefine-sym a=b "$OUTPUT"'])
        self.assertEqual(sf.postcompile_lines(csh.rsplit("\n", 2)[0] + "\n"), [])

    def test_permute_sh_gates_before_searching(self):
        text = (TOOLS / "permute.sh").read_text()
        gate = text.index('"$PYTHON" tools/scratch_fidelity.py check')
        self.assertLess(text.index('"$PYTHON" tools/scratch_fidelity.py import'), gate)
        self.assertLess(gate, text.index('"$PERMUTER" "${args[@]}"'))
        self.assertIn("--allow-unfaithful", text)
        self.assertNotIn('"$IMPORT"', text, "the importer must run through the restoration wrapper")


if __name__ == "__main__":
    unittest.main()
