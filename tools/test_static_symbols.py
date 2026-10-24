#!/usr/bin/env python3
"""Synthetic-object tests for tools/static_symbols.py.

The fixture is a hand-built IDO-shaped relocatable: an exported ``entry``
followed by two static functions, ``helper`` (forward-declared, so IDO left a
sized ``UND`` placeholder) and ``tail`` (no symbol at all), plus a relocation
against an undefined ``callee`` whose symbol index must survive the rewrite.
Its .text is zero filler; no ROM content.
"""

from __future__ import annotations

import pathlib
import struct
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import static_symbols as ss  # noqa: E402

STB_GLOBAL = 1
R_MIPS_26 = 4


def _pack_symbol(name, value, size, info, shndx):
    return struct.pack(">IIIBBH", name, value, size, info, 0, shndx)


def build_object(*, statics=(("helper", 0x10, 0x10), ("tail", 0x20, 0x8)),
                 placeholder=True, entry_size=0x10, asmproc_local=False,
                 bss=(), nested_bss=(), nil_bss=(), bss_size=0x20,
                 bss_kind=ss.SHT_NOBITS, bss_flags=2, existing_bss=(), emit_bss=True,
                 data_anchors=(), nested_data=(), data_size=0x20,
                 data_kind=ss.SHT_PROGBITS, data_flags=3, data_addr=0,
                 existing_data=(), emit_data=True):
    text = bytes(0x30)
    strings = bytearray(b"\0")

    def string(name):
        offset = len(strings)
        strings.extend(name.encode() + b"\0")
        return offset

    # [null, .text section, (asm-processor's size-0 local)] | globals
    symbols = [_pack_symbol(0, 0, 0, 0, 0), _pack_symbol(0, 0, len(text), 3, 1)]
    if asmproc_local:
        symbols.append(_pack_symbol(string("helper"), 0x10, 0, ss.STT_FUNC, 1))
    has_bss = bool(bss or nested_bss or nil_bss or existing_bss) and emit_bss
    has_data = bool(data_anchors or nested_data or existing_data) and emit_data
    data_index = 8 if has_bss else 7
    bss_symbol = None
    if has_bss:
        bss_symbol = len(symbols)
        symbols.append(_pack_symbol(0, 0, bss_size, 3, 7))
    for name, value, size, kind in existing_bss:
        if kind == "local":
            symbols.append(_pack_symbol(string(name), value, size, ss.STT_OBJECT, 7))
    data_symbol = None
    if has_data:
        data_symbol = len(symbols)
        symbols.append(_pack_symbol(0, 0, data_size, 3, data_index))
    for name, value, size, kind in existing_data:
        if kind == "local":
            symbols.append(_pack_symbol(string(name), value, size, ss.STT_OBJECT, data_index))
    first_global = len(symbols)
    for name, value, size, kind in existing_bss:
        if kind != "local":
            symbols.append(_pack_symbol(string(name), value, size,
                                        STB_GLOBAL << 4 | ss.STT_OBJECT,
                                        0 if kind == "import" else 7))
    for name, value, size, kind in existing_data:
        if kind != "local":
            symbols.append(_pack_symbol(string(name), value, size,
                                        STB_GLOBAL << 4 | ss.STT_OBJECT,
                                        0 if kind == "import" else data_index))
    if placeholder and not asmproc_local:
        symbols.append(_pack_symbol(string("helper"), 0, 0x10, STB_GLOBAL << 4, 0))
    symbols.append(_pack_symbol(string("entry"), 0, entry_size, STB_GLOBAL << 4 | ss.STT_FUNC, 1))
    callee = len(symbols)
    symbols.append(_pack_symbol(string("callee"), 0, 0, STB_GLOBAL << 4 | ss.STT_FUNC, 0))
    rel = struct.pack(">II", 0x4, callee << 8 | R_MIPS_26)
    if bss_symbol is not None:
        rel += struct.pack(">IIII", 0x8, bss_symbol << 8 | 5, 0xC, bss_symbol << 8 | 6)

    if data_symbol is not None:
        rel += struct.pack(">IIII", 0x10, data_symbol << 8 | 5, 0x14, data_symbol << 8 | 6)

    # .mdebug: header, local symbols, local strings, one file descriptor.
    procs = [("entry", 0, entry_size, ss.ST_PROC)] + [
        (name, start, size, ss.ST_STATIC_PROC) for name, start, size in statics]
    local_strings = bytearray(b"\0")
    local_syms = b""

    def debug(name, value, st, sc):
        nonlocal local_syms
        iss = len(local_strings)
        local_strings.extend(name.encode() + b"\0")
        local_syms += struct.pack(">III", iss, value, st << 26 | sc << 21)

    debug("fixture.c", 0, ss.ST_FILE, ss.SC_TEXT)
    for name, offset in bss:
        debug(name, offset, ss.ST_STATIC, ss.SC_BSS)
    for name, offset in data_anchors:
        debug(name, offset, ss.ST_STATIC, ss.SC_DATA)
    for name, offset in nil_bss:
        debug(name, offset, ss.ST_STATIC, 0)
    for name, start, size, st in procs:
        debug(name, start, st, ss.SC_TEXT)
        if name == "entry" and (nested_bss or nested_data):
            debug("", 0, ss.ST_BLOCK, ss.SC_TEXT)
            for nested_name, offset in nested_bss:
                debug(nested_name, offset, ss.ST_STATIC, ss.SC_BSS)
            for nested_name, offset in nested_data:
                debug(nested_name, offset, ss.ST_STATIC, ss.SC_DATA)
            debug("", 0, ss.ST_END, ss.SC_TEXT)
        debug(name, size, ss.ST_END, ss.SC_TEXT)
    debug("fixture.c", 0, ss.ST_END, ss.SC_TEXT)

    shstr = b"\0.text\0.rel.text\0.mdebug\0.symtab\0.strtab\0.shstrtab\0.bss\0.data\0"
    names = {n: shstr.index(n.encode() + b"\0") for n in
             (".text", ".rel.text", ".mdebug", ".symtab", ".strtab", ".shstrtab", ".bss", ".data")}
    body = bytearray(0x34)
    placed = {}

    def put(name, blob):
        body.extend(bytes(-len(body) % 4))
        placed[name] = (len(body), len(blob))
        body.extend(blob)

    put(".text", text)
    put(".rel.text", rel)
    mdebug_at = len(body) + (-len(body) % 4)
    sym_at = mdebug_at + 0x60
    ss_at = sym_at + len(local_syms)
    fd_at = ss_at + len(local_strings) + (-(ss_at + len(local_strings)) % 4)
    header = [0] * 24
    header[0] = ss.HDRR_MAGIC << 16
    header[8], header[9] = len(local_syms) // 12, sym_at
    header[14], header[15] = len(local_strings), ss_at
    header[18], header[19] = 1, fd_at
    fd = bytearray(72)
    struct.pack_into(">IIII", fd, 8, 0, 0, 0, len(local_syms) // 12)
    mdebug = (struct.pack(">24I", *header) + local_syms + local_strings
              + bytes(fd_at - ss_at - len(local_strings)) + fd)
    put(".mdebug", mdebug)
    put(".symtab", b"".join(symbols))
    put(".strtab", bytes(strings))
    put(".shstrtab", shstr)
    if has_data:
        put(".data", bytes(data_size))
    body.extend(bytes(-len(body) % 4))
    shoff = len(body)
    order = [".text", ".rel.text", ".mdebug", ".symtab", ".strtab", ".shstrtab"]
    if has_bss:
        order.append(".bss")
        placed[".bss"] = (0, bss_size)
    if has_data:
        order.append(".data")
    kinds = {".data": data_kind, ".bss": bss_kind, ".text": 1, ".rel.text": ss.SHT_REL, ".mdebug": 0x70000005,
             ".symtab": ss.SHT_SYMTAB, ".strtab": 3, ".shstrtab": 3}
    links = {".rel.text": (4, 1), ".symtab": (5, first_global)}
    body.extend(bytes(40))
    for name in order:
        offset, length = placed[name]
        link, info = links.get(name, (0, 0))
        body.extend(struct.pack(">10I", names[name], kinds[name], data_flags if name == ".data" else bss_flags if name == ".bss" else 6 if name == ".text" else 0,
                                data_addr if name == ".data" else 0, offset, length,
                                link, info, 4, {".symtab": 16, ".rel.text": 8}.get(name, 0)))
    ident = b"\x7fELF\x01\x02\x01" + bytes(9)
    body[:0x34] = ident + struct.pack(">HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0,
                                      0x34, 0, 0, 40, len(order) + 1, order.index(".shstrtab") + 1)
    return bytes(body)


def symbols(data):
    sections = ss._sections(data)
    index = next(i for i, s in enumerate(sections) if s[1] == ss.SHT_SYMTAB)
    symtab, strtab = sections[index], sections[sections[index][6]]
    strings = data[strtab[4]:strtab[4] + strtab[5]]
    rows = [(ss._cstring(strings, row[0]), row[1], row[2], row[3], row[5])
            for row in ss._symbols(data, symtab)]
    return rows, symtab[7], sections


def relocation_targets(data):
    rows, _first_global, sections = symbols(data)
    rel = next(s for s in sections if s[1] == ss.SHT_REL)
    return [rows[struct.unpack_from(">I", data, offset + 4)[0] >> 8][0]
            for offset in range(rel[4], rel[4] + rel[5], 8)]


def section_bytes(data, name):
    section = next(s for s in ss._sections(data) if s[10] == name)
    return data[section[4]:section[4] + section[5]]


class StaticSymbolsTest(unittest.TestCase):
    def test_exported_function_followed_by_statics(self):
        original = build_object()
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["helper", "tail"])
        rows, first_global, _ = symbols(data)
        local = (ss.STB_LOCAL << 4) | ss.STT_FUNC
        self.assertIn(("helper", 0x10, 0x10, local, 1), rows[:first_global])
        self.assertIn(("tail", 0x20, 0x8, local, 1), rows[:first_global])
        # The placeholder is gone; every global follows every local.
        self.assertEqual([row[0] for row in rows[first_global:]], ["entry", "callee"])
        self.assertTrue(all(row[3] >> 4 == ss.STB_LOCAL for row in rows[:first_global]))
        # The relocation still names its symbol, and no section byte moved.
        self.assertEqual(relocation_targets(data), ["callee"])
        for name in (".text", ".mdebug"):
            self.assertEqual(section_bytes(data, name), section_bytes(original, name))
        rel_before, rel_after = (section_bytes(d, ".rel.text") for d in (original, data))
        self.assertEqual(rel_before[:4], rel_after[:4])
        self.assertEqual(rel_before[7], rel_after[7])

    def test_second_run_changes_nothing(self):
        data, _ = ss.add_static_symbols(build_object())
        self.assertEqual(ss.add_static_symbols(data), (data, []))

    def test_object_without_statics_is_byte_identical(self):
        original = build_object(statics=(), placeholder=False)
        self.assertEqual(ss.add_static_symbols(original), (original, []))

    def test_asm_processor_size_zero_local_is_sized(self):
        data, added = ss.add_static_symbols(build_object(asmproc_local=True))
        self.assertEqual(added, ["helper", "tail"])
        rows, first_global, _ = symbols(data)
        self.assertEqual([row[:3] for row in rows[:first_global] if row[0] == "helper"],
                         [("helper", 0x10, 0x10)])

    def test_global_disagreeing_with_mdebug_refuses(self):
        # entry's st_size says 0x14 but its procedure ends at 0x10.
        data = bytearray(build_object())
        sections = ss._sections(bytes(data))
        symtab = next(s for s in sections if s[1] == ss.SHT_SYMTAB)
        for offset in range(symtab[4], symtab[4] + symtab[5], 16):
            if struct.unpack_from(">I", data, offset + 8)[0] == 0x10 and data[offset + 12] == 0x12:
                struct.pack_into(">I", data, offset + 8, 0x14)
        with self.assertRaisesRegex(ss.Failure, "disagrees"):
            ss.add_static_symbols(bytes(data))

    def test_overlapping_static_refuses(self):
        with self.assertRaisesRegex(ss.Failure, "overlap"):
            ss.add_static_symbols(build_object(statics=(("helper", 0x8, 0x10),)))


    def test_file_scope_bss_anchors_without_static_functions(self):
        original = build_object(statics=(), placeholder=False,
                                bss=(("pool", 0), ("head", 0x10)))
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["head", "pool"])
        rows, first_global, _ = symbols(data)
        self.assertIn(("pool", 0, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertIn(("head", 0x10, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertEqual([row[0] for row in rows[first_global:]], ["entry", "callee"])
        self.assertEqual(ss.add_static_symbols(data), (data, []))
        for name in (".text", ".mdebug"):
            self.assertEqual(section_bytes(data, name), section_bytes(original, name))
        self.assertEqual(relocation_targets(data), relocation_targets(original))
        before = section_bytes(original, ".rel.text")
        after = section_bytes(data, ".rel.text")
        for index in range(0, len(before), 8):
            self.assertEqual(before[index:index + 4], after[index:index + 4])
            self.assertEqual(before[index + 7], after[index + 7])

    def test_functions_and_bss_are_restored_together(self):
        original = build_object(bss=(("pool", 0),))
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["helper", "pool", "tail"])
        rows, first_global, _ = symbols(data)
        self.assertIn(("helper", 0x10, 0x10, ss.STT_FUNC, 1), rows[:first_global])
        self.assertIn(("pool", 0, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertEqual(relocation_targets(data), relocation_targets(original))
        self.assertEqual(ss.add_static_symbols(data), (data, []))

    def test_unbalanced_bss_scope_and_linked_input_refuse(self):
        original = bytearray(build_object(bss=(("pool", 0),)))
        struct.pack_into(">H", original, 0x10, 2)
        with self.assertRaisesRegex(ss.Failure, "relocatable"):
            ss.add_static_symbols(bytes(original))
        original = bytearray(build_object(bss=(("pool", 0),)))
        mdebug = next(s for s in ss._sections(original) if s[10] == ".mdebug")
        words = struct.unpack_from(">24I", original, mdebug[4])
        # Remove the compiler's file-scope opener without altering any symbol value.
        struct.pack_into(">I", original, words[9] + 8, ss.SC_TEXT << 21)
        with self.assertRaisesRegex(ss.Failure, "ambiguous .mdebug scope"):
            ss.add_static_symbols(bytes(original))

    def test_nested_and_unemitted_bss_are_not_restored(self):
        original = build_object(statics=(), placeholder=False,
                                nested_bss=(("cache", 4),), nil_bss=(("unemitted", 8),))
        self.assertEqual(ss.add_static_symbols(original), (original, []))

    def test_ambiguous_file_and_nested_names_refuse(self):
        with self.assertRaisesRegex(ss.Failure, "ambiguous nested"):
            ss.add_static_symbols(build_object(bss=(("cache", 0),),
                                               nested_bss=(("cache", 4),)))
        with self.assertRaisesRegex(ss.Failure, "recorded twice"):
            ss.add_static_symbols(build_object(bss=(("cache", 0), ("cache", 4))))

    def test_bss_anchors_require_real_allocated_nobits_and_interior_offset(self):
        for options in ({"bss_kind": 1}, {"bss_flags": 0}, {"emit_bss": False}):
            with self.subTest(options=options):
                with self.assertRaisesRegex(ss.Failure, "allocated NOBITS|unambiguous .bss"):
                    ss.add_static_symbols(build_object(bss=(("pool", 0),), **options))
        for offset in (0x20, 0xFFFFFFFF):
            with self.subTest(offset=offset):
                with self.assertRaisesRegex(ss.Failure, "outside .bss"):
                    ss.add_static_symbols(build_object(bss=(("pool", offset),)))

    def test_existing_valid_local_bss_definition_is_preserved(self):
        for size in (0, 4):
            with self.subTest(size=size):
                original = build_object(statics=(), placeholder=False, bss=(("pool", 0),),
                                        existing_bss=(("pool", 0, size, "local"),))
                self.assertEqual(ss.add_static_symbols(original), (original, []))

    def test_bss_collisions_do_not_convert_imports_globals_or_functions(self):
        for kind in ("import", "global"):
            with self.subTest(kind=kind):
                with self.assertRaisesRegex(ss.Failure, "conflicts"):
                    ss.add_static_symbols(build_object(bss=(("pool", 0),),
                                                       existing_bss=(("pool", 0, 0, kind),)))
        with self.assertRaisesRegex(ss.Failure, "conflicts"):
            ss.add_static_symbols(build_object(bss=(("entry", 0),)))
        with self.assertRaisesRegex(ss.Failure, "names collide"):
            ss.add_static_symbols(build_object(bss=(("helper", 0),)))
        for value, size in ((4, 0), (0, 0x24)):
            with self.subTest(value=value, size=size):
                with self.assertRaisesRegex(ss.Failure, "conflicts"):
                    ss.add_static_symbols(build_object(bss=(("pool", 0),),
                                                       existing_bss=(("pool", value, size, "local"),)))


    def test_file_scope_data_without_functions_is_unsized_and_idempotent(self):
        original = build_object(statics=(), placeholder=False,
                                data_anchors=(("values", 0), ("mode", 0x1F)))
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["mode", "values"])
        rows, first_global, _ = symbols(data)
        self.assertIn(("values", 0, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertIn(("mode", 0x1F, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertEqual(ss.add_static_symbols(data), (data, []))
        self.assertEqual(relocation_targets(data), relocation_targets(original))
        for section in (".text", ".data", ".mdebug"):
            self.assertEqual(section_bytes(data, section), section_bytes(original, section))

    def test_data_bss_and_functions_restore_together_preserving_old_symbols(self):
        original = build_object(bss=(("pool", 0),), data_anchors=(("values", 4),))
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["helper", "pool", "tail", "values"])
        rows, first_global, _ = symbols(data)
        self.assertIn(("pool", 0, 0, ss.STT_OBJECT, 7), rows[:first_global])
        self.assertIn(("values", 4, 0, ss.STT_OBJECT, 8), rows[:first_global])
        self.assertEqual(relocation_targets(data), relocation_targets(original))
        old, old_first_global, _ = symbols(original)
        self.assertEqual(rows[first_global:], [r for r in old[old_first_global:] if r[0] != "helper"])
        self.assertEqual(ss.add_static_symbols(data), (data, []))

    def test_existing_sized_data_preserved_without_inventing_an_extent(self):
        for size in (0, 4):
            with self.subTest(size=size):
                original = build_object(statics=(), placeholder=False,
                    data_anchors=(("values", 4),), existing_data=(("values", 4, size, "local"),))
                self.assertEqual(ss.add_static_symbols(original), (original, []))

    def test_data_requires_real_zero_based_allocated_writable_nonexec_progbits(self):
        options = ({"data_kind": ss.SHT_NOBITS}, {"data_flags": 0},
                   {"data_flags": 2}, {"data_flags": 7}, {"data_addr": 0x1000},
                   {"emit_data": False})
        for option in options:
            with self.subTest(option=option):
                with self.assertRaises(ss.Failure):
                    ss.add_static_symbols(build_object(data_anchors=(("values", 0),), **option))
        for offset in (0x20, 0xFFFFFFFF):
            with self.subTest(offset=offset):
                with self.assertRaisesRegex(ss.Failure, "outside .data"):
                    ss.add_static_symbols(build_object(data_anchors=(("values", offset),)))
        linked = bytearray(build_object(data_anchors=(("values", 0),)))
        struct.pack_into(">H", linked, 0x10, 2)
        with self.assertRaisesRegex(ss.Failure, "relocatable"):
            ss.add_static_symbols(linked)

    def test_duplicate_data_section_and_unbalanced_data_scope_refuse(self):
        original = bytearray(build_object(data_anchors=(("values", 0),),
                                         existing_bss=(("other", 0, 0, "local"),)))
        sections = ss._sections(original)
        data_section = next(s for s in sections if s[10] == ".data")
        bss_index = next(i for i, s in enumerate(sections) if s[10] == ".bss")
        shoff, = struct.unpack_from(">I", original, 0x20)
        struct.pack_into(">I", original, shoff + bss_index * 40, data_section[0])
        with self.assertRaisesRegex(ss.Failure, "unambiguous .data"):
            ss.add_static_symbols(original)
        original = bytearray(build_object(data_anchors=(("values", 0),)))
        mdebug = next(s for s in ss._sections(original) if s[10] == ".mdebug")
        words = struct.unpack_from(">24I", original, mdebug[4])
        struct.pack_into(">I", original, words[9] + 8, ss.SC_TEXT << 21)
        with self.assertRaisesRegex(ss.Failure, "ambiguous .mdebug scope"):
            ss.add_static_symbols(original)

    def test_nested_data_excluded_and_ambiguous_or_conflicting_data_refused(self):
        original = build_object(statics=(), placeholder=False, nested_data=(("values", 4),))
        self.assertEqual(ss.add_static_symbols(original), (original, []))
        for options in ({"nested_data": (("values", 8),)},
                        {"nested_bss": (("values", 8),)},
                        {"data_anchors": (("values", 0), ("values", 4))}):
            with self.subTest(options=options):
                with self.assertRaisesRegex(ss.Failure, "ambiguous|recorded twice"):
                    ss.add_static_symbols(build_object(**({"data_anchors": (("values", 0),)} | options)))
        for kind in ("global", "import"):
            with self.subTest(kind=kind):
                with self.assertRaisesRegex(ss.Failure, "conflicts"):
                    ss.add_static_symbols(build_object(data_anchors=(("values", 0),),
                        existing_data=(("values", 0, 0, kind),)))
        for value, size in ((4, 0), (0, 0x24)):
            with self.subTest(value=value, size=size):
                with self.assertRaisesRegex(ss.Failure, "conflicts"):
                    ss.add_static_symbols(build_object(data_anchors=(("values", 0),),
                        existing_data=(("values", value, size, "local"),)))
        with self.assertRaisesRegex(ss.Failure, "names collide"):
            ss.add_static_symbols(build_object(data_anchors=(("helper", 0),)))
        with self.assertRaisesRegex(ss.Failure, "names collide|ambiguous"):
            ss.add_static_symbols(build_object(data_anchors=(("values", 0),), bss=(("values", 0),)))
        with self.assertRaisesRegex(ss.Failure, "conflicts"):
            ss.add_static_symbols(build_object(data_anchors=(("entry", 0),)))


if __name__ == "__main__":
    unittest.main()
