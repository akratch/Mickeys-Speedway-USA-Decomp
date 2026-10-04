#!/usr/bin/env python3
"""Resident category accounting; synthetic source and symbol metadata only."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import progress


def guarded(names=("first",), fallbacks=None):
    fallbacks = names if fallbacks is None else fallbacks
    return ("#ifdef NON_MATCHING\n"
            + "\n".join(f"void {name}(void) {{}}" for name in names)
            + "\n#else\n"
            + "\n".join(f'#pragma GLOBAL_ASM("asm/nonmatchings/main/example/{name}.s")'
                        for name in fallbacks)
            + "\n#endif\n")


class GuardTests(unittest.TestCase):
    def test_single_and_multiple_definitions(self):
        for names in (("first",), ("first", "second")):
            self.assertEqual(len(progress.resident_guarded_fallbacks(guarded(names))), len(names))

    def test_renamed_fallback_uses_actual_path(self):
        self.assertEqual(progress.resident_guarded_fallbacks(guarded(("friendly",), ("original",))),
                         ["asm/nonmatchings/main/example/original.s"])

    def test_bare_fallback_and_declarations_are_not_coverage(self):
        text = ('#pragma GLOBAL_ASM("asm/nonmatchings/main/example/bare.s")\n'
                '#ifdef NON_MATCHING\nextern void declared(void);\n#endif\n')
        self.assertEqual(progress.resident_guarded_fallbacks(text), [])

    def test_declaration_only_pair_refuses(self):
        with self.assertRaisesRegex(RuntimeError, "definitions"):
            progress.resident_guarded_fallbacks(guarded().replace("void first(void) {}", "void first(void);"))

    def test_comments_and_literals_cannot_supply_definitions(self):
        fake = '/* void fake(void) {} */\nconst char *text = "void fake(void) {}";'
        with self.assertRaises(RuntimeError):
            progress.resident_guarded_fallbacks(guarded().replace("void first(void) {}", fake))
        self.assertEqual(progress.resident_guarded_fallbacks("/*\n" + guarded() + "*/\n"), [])

    def test_comments_and_nested_body_conditionals(self):
        body = 'void first(void) {\n#if OPTION\nint x = 1;\n#else\nint x = 2;\n#endif\n}\n'
        text = guarded().replace("void first(void) {}", body).replace("#ifdef NON_MATCHING", "#ifdef NON_MATCHING /* candidate */")
        self.assertEqual(len(progress.resident_guarded_fallbacks(text)), 1)

    def test_conditional_function_definitions_refuse(self):
        for body in ('#if 0\nvoid first(void) {}\n#endif',
                     '#ifdef OTHER\nvoid first(void) {}\n#endif',
                     'void first(void)\n#if 0\n{}\n#endif'):
            text = guarded().replace('void first(void) {}', body)
            with self.subTest(body=body), self.assertRaisesRegex(RuntimeError, 'conditional'):
                progress.resident_guarded_fallbacks(text)

    def test_unclosed_or_excess_function_braces_refuse(self):
        for body in ('void first(void) {', '}\nvoid first(void) {}\n{'):
            with self.subTest(body=body), self.assertRaises(RuntimeError):
                progress.resident_guarded_fallbacks(guarded().replace('void first(void) {}', body))

    def test_ambiguous_branches_and_unbalanced_guards_refuse(self):
        for text in (guarded().replace("#else", "#elif OTHER"),
                     guarded().replace("#endif", ""),
                     "#if OTHER\n" + guarded() + "#endif\n",
                     guarded().replace("#else", "#else\n#else")):
            with self.subTest(text=text), self.assertRaises(RuntimeError):
                progress.resident_guarded_fallbacks(text)

    def test_missing_extra_and_duplicate_definitions_refuse(self):
        for names, fallbacks in ((('first',), ()), (('first',), ('first', 'second')),
                                 (('first', 'first'), ('first', 'second'))):
            with self.subTest(names=names), self.assertRaises(RuntimeError):
                progress.resident_guarded_fallbacks(guarded(names, fallbacks))

    def test_unknown_guard_spelling_and_conditional_fallback_refuse(self):
        for text in (guarded().replace('#ifdef NON_MATCHING', '#if defined(NON_MATCHING)'),
                     guarded().replace('#pragma', '#if OTHER\n#pragma').replace('\n#endif', '\n#endif\n#endif', 1)):
            with self.subTest(text=text), self.assertRaises(RuntimeError):
                progress.resident_guarded_fallbacks(text)


class ResidentAccountingTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.source = self.root / 'src/main/example.c'
        self.source.parent.mkdir(parents=True)
        self.source.write_text(guarded())
        self.asm = self.root / 'asm/nonmatchings/main/example/first.s'
        self.asm.parent.mkdir(parents=True)
        self.asm.write_text('glabel first\nendlabel first\n')
        self.funcs = {'first': 16, 'matched': 8, 'manual': 12, 'bare': 4}
        self.addrs = {'first': 4096, 'matched': 4112, 'manual': 4120, 'bare': 4132}
        self.records = [(n, self.addrs[n], size) for n, size in self.funcs.items()]

    def count(self):
        return progress.get_resident_nonmatching_functions(
            self.root, self.funcs, self.addrs, {'matched'}, {'manual'}, self.records)

    def test_partition_preserves_denominator_and_exact_credit(self):
        names = self.count()
        self.assertEqual(names, {'first'})
        total = sum(self.funcs.values())
        matched, manual = self.funcs['matched'], self.funcs['manual']
        before_remaining = total - matched - manual
        nm = sum(self.funcs[n] for n in names)
        after_remaining = before_remaining - nm
        self.assertEqual(nm, 16)
        self.assertEqual(after_remaining, 4)
        self.assertEqual(matched + manual + nm + after_remaining, total)

    def test_renamed_c_does_not_require_a_same_named_elf_symbol(self):
        self.source.write_text(guarded(('friendly',), ('first',)))
        self.assertEqual(self.count(), {'first'})

    def test_multiple_fallbacks_are_each_counted_once(self):
        self.source.write_text(guarded(('first', 'second')))
        self.asm.with_name('second.s').write_text('glabel second\n')
        self.funcs['second'] = 8
        self.addrs['second'] = 4140
        self.records.append(('second', 4140, 8))
        self.assertEqual(self.count(), {'first', 'second'})

    def test_overlay_tus_are_excluded(self):
        overlay = self.root / 'src/overlays/o001/example.c'
        overlay.parent.mkdir(parents=True)
        overlay.write_text(guarded())
        self.assertEqual(self.count(), {'first'})

    def test_duplicate_source_ownership_refuses(self):
        self.source.with_name('other.c').write_text(guarded())
        with self.assertRaisesRegex(RuntimeError, 'duplicate'):
            self.count()

    def test_missing_and_duplicate_elf_identity_refuse(self):
        for records in (self.records[1:], self.records + [self.records[0]]):
            with self.subTest(records=records), self.assertRaisesRegex(RuntimeError, 'ELF identity'):
                with patch.object(self, 'records', records):
                    self.count()

    def test_unaligned_zero_and_mismatched_extent_refuse(self):
        for record in (('first', 4096, 0), ('first', 4096, 3), ('first', 4097, 16), ('first', 4096, 20)):
            with self.subTest(record=record), self.assertRaises(RuntimeError):
                with patch.object(self, 'records', [record, *self.records[1:]]):
                    self.count()

    def test_overlap_and_same_extent_alias_refuse(self):
        for record in (('other', 4100, 16), ('alias', 4096, 16)):
            with self.subTest(record=record), self.assertRaisesRegex(RuntimeError, 'overlapping'):
                with patch.object(self, 'records', [*self.records, record]):
                    self.count()

    def test_matched_or_handwritten_conflict_refuses(self):
        for name in ('matched', 'manual'):
            self.source.write_text(guarded((name,)))
            self.asm.with_name(name + '.s').write_text('glabel ' + name + '\n')
            with self.subTest(name=name), self.assertRaisesRegex(RuntimeError, 'conflicting'):
                self.count()

    def test_missing_wrong_or_ambiguous_fallback_label_refuses(self):
        for text in ('glabel wrong\n', 'glabel first\nglabel first\n', 'glabel first\nglabel bare\n'):
            self.asm.write_text(text)
            with self.subTest(text=text), self.assertRaisesRegex(RuntimeError, 'fallback identity'):
                self.count()

    def test_missing_and_symlink_fallback_refuse(self):
        self.asm.unlink()
        with self.assertRaisesRegex(RuntimeError, 'missing or nonregular'):
            self.count()
        self.asm.symlink_to(self.source)
        with self.assertRaisesRegex(RuntimeError, 'missing or nonregular'):
            self.count()

    def test_path_escape_refuses(self):
        self.source.write_text(guarded().replace('asm/nonmatchings/main/example/first.s', '../foreign.s'))
        with self.assertRaisesRegex(RuntimeError, 'invalid resident fallback path'):
            self.count()

    def test_duplicate_elf_records_survive_existing_dictionary(self):
        listing = '00001000 g F .text 00000010 first\n' * 2
        records = []
        with patch.object(progress.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0, listing.encode(), b'')):
            funcs, addrs, _, _ = progress.get_elf_functions('unused', 'unused', {}, records)
        self.assertEqual(funcs, {'first': 16})
        self.assertEqual(addrs, {'first': 4096})
        self.assertEqual(len(records), 2)



def data_guard(paths=("asm/nonmatchings/main/example/D_10000000.s",)):
    return ("#ifndef NON_MATCHING\n"
            + "\n".join(f'#pragma GLOBAL_ASM("{path}")' for path in paths)
            + "\n#endif\n")


class DataGuardTests(unittest.TestCase):
    def test_requires_authentication_and_never_returns_function(self):
        with self.assertRaisesRegex(RuntimeError, 'requires section authentication'):
            progress.resident_guarded_fallbacks(data_guard())
        seen = []
        def accept(path):
            seen.append(path)
            return True
        self.assertEqual(progress.resident_guarded_fallbacks(data_guard(), validate_data=accept), [])
        self.assertEqual(seen, ['asm/nonmatchings/main/example/D_10000000.s'])
        with self.assertRaisesRegex(RuntimeError, 'not authenticated'):
            progress.resident_guarded_fallbacks(data_guard(), validate_data=lambda _: False)

    def test_ambiguous_inverse_guards_refuse_before_validator(self):
        original = data_guard()
        for text in (original.replace('#endif', '#else\n#endif'),
                     original.replace('#endif', '#elif OTHER\n#endif'),
                     original.replace('#endif', ''),
                     '#if OTHER\n' + original + '#endif\n',
                     original.replace('#pragma', '#if OTHER\n#pragma').replace('#endif', '#endif\n#endif'),
                     original.replace('#endif', 'void function(void) {}\n#endif'),
                     original.replace('#endif', 'extern float value;\n#endif'),
                     original.replace('#ifndef NON_MATCHING', '#if !defined(NON_MATCHING)'),
                     data_guard(('asm/nonmatchings/main/example/D_10000000.s',) * 2)):
            with self.subTest(text=text), self.assertRaises(RuntimeError):
                progress.resident_guarded_fallbacks(text, validate_data=lambda _: self.fail('validator reached'))


class DataAccountingTests(unittest.TestCase):
    count = ResidentAccountingTests.count

    def setUp(self):
        ResidentAccountingTests.setUp(self)
        self.data = self.asm.with_name('D_10000000.s')
        self.valid = '.section .rodata\n.balign 4\ndlabel D_10000000\n.float 1.25\nenddlabel D_10000000\n'
        self.data.write_text(self.valid)
        self.source.write_text(data_guard() + guarded())

    def test_scalar_guards_preserve_function_partition(self):
        self.assertEqual(self.count(), {'first'})
        for ending in ('', 'enddlabel D_10000000\n', '.size D_10000000, . - D_10000000\n'):
            self.data.write_text(self.valid.replace('enddlabel D_10000000\n', ending))
            self.assertEqual(self.count(), {'first'})

    def test_executable_and_unknown_data_syntax_refuse(self):
        for text in (self.valid.replace('.rodata', '.text'),
                     self.valid.replace('.rodata', '.data'),
                     self.valid.replace('dlabel D_10000000', 'glabel D_10000000', 1),
                     self.valid.replace('dlabel D_10000000', 'alabel D_10000000', 1),
                     self.valid.replace('.float 1.25', 'nop'),
                     self.valid.replace('.float 1.25', '.include "foreign.inc"'),
                     self.valid.replace('.float 1.25', '.float 1.25\n.float 2.5'),
                     self.valid + '.section .text\n',
                     self.valid.replace('enddlabel D_10000000', '.type D_10000000, @function')):
            self.data.write_text(text)
            with self.subTest(text=text), self.assertRaises(RuntimeError):
                self.count()

    def test_data_label_cannot_be_an_elf_function_even_if_zero_sized(self):
        self.records.append(('D_10000000', 5000, 0))
        with self.assertRaisesRegex(RuntimeError, 'data fallback identity'):
            self.count()

    def test_wrong_duplicate_and_foreign_labels_refuse(self):
        for text in (self.valid.replace('D_10000000', 'D_10000004'),
                     self.valid + 'dlabel D_10000004\n',
                     self.valid.replace('.float 1.25', 'dlabel D_10000000\n.float 1.25')):
            self.data.write_text(text)
            with self.subTest(text=text), self.assertRaises(RuntimeError):
                self.count()

    def test_foreign_owner_unsafe_and_missing_paths_refuse(self):
        for path in ('asm/nonmatchings/main/foreign/D_10000000.s',
                     'asm/nonmatchings/main/example/first.s',
                     'asm/nonmatchings/main/example/./D_10000000.s',
                     'asm//nonmatchings/main/example/D_10000000.s',
                     '../D_10000000.s', '/tmp/D_10000000.s'):
            self.source.write_text(data_guard((path,)) + guarded())
            with self.subTest(path=path), self.assertRaises(RuntimeError):
                self.count()
        self.source.write_text(data_guard() + guarded())
        self.data.unlink()
        with self.assertRaisesRegex(RuntimeError, 'missing or nonregular'):
            self.count()
        self.data.symlink_to(self.asm)
        with self.assertRaisesRegex(RuntimeError, 'missing or nonregular'):
            self.count()

    def test_duplicate_data_guard_refuses(self):
        self.source.write_text(data_guard() * 2 + guarded())
        with self.assertRaisesRegex(RuntimeError, 'duplicate resident data'):
            self.count()


if __name__ == '__main__':
    unittest.main()

class StaleExtractTests(unittest.TestCase):
    """The scoreboard must refuse a stale extract, not report it backwards."""

    def tree(self, pragmas, labelled):
        import tempfile
        tmp = tempfile.mkdtemp()
        asm = Path(tmp) / "asm" / "nonmatchings" / "main" / "unit"
        asm.mkdir(parents=True)
        for name in labelled:
            (asm / f"{name}.s").write_text(f"glabel {name}\n    /* 1 2 3 */  nop\n")
        src = Path(tmp) / "src" / "main"
        src.mkdir(parents=True)
        body = "".join(
            f'#pragma GLOBAL_ASM("asm/nonmatchings/main/unit/{n}.s")\n' for n in pragmas
        )
        (src / "unit.c").write_text(body)
        return str(Path(tmp) / "asm"), str(Path(tmp) / "src")

    def test_a_promoted_function_with_leftover_asm_is_detected(self):
        # The observed failure: a 1372-byte promotion reported as a 1372-byte
        # regression, with --check-readme confirming it.
        asm, src = self.tree(pragmas=["stillAsm"], labelled=["promoted", "stillAsm"])
        self.assertEqual(progress.stale_extract_names(asm, src), {"promoted"})

    def test_a_freshly_extracted_tree_is_clean(self):
        asm, src = self.tree(pragmas=["a", "b"], labelled=["a", "b"])
        self.assertEqual(progress.stale_extract_names(asm, src), set())

    def test_whole_file_hand_written_dumps_are_not_scanned(self):
        # asm/main/*.s and asm/libultra/*.s are original hand-written assembly
        # that no pragma references and that counts as verified asm. Scanning
        # them reported 235 false positives on a clean tree.
        import tempfile
        tmp = tempfile.mkdtemp()
        (Path(tmp) / "asm" / "main").mkdir(parents=True)
        (Path(tmp) / "asm" / "main" / "dump.s").write_text("glabel handWritten\n")
        (Path(tmp) / "src").mkdir(parents=True)
        self.assertEqual(
            progress.stale_extract_names(
                str(Path(tmp) / "asm"), str(Path(tmp) / "src")
            ),
            set(),
        )

    def test_an_absent_nonmatchings_directory_is_not_an_error(self):
        import tempfile
        tmp = tempfile.mkdtemp()
        self.assertEqual(progress.stale_extract_names(tmp, tmp), set())

