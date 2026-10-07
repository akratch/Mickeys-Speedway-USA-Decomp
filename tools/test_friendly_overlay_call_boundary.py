#!/usr/bin/env python3
"""Synthetic regressions for friendly-name canonical callee authentication."""
import os
from pathlib import Path
import tempfile
import unittest

import reloc_surface as rs
import test_reloc_surface as fixtures


class FriendlyOverlayCallBoundaryTests(unittest.TestCase):
    Fixture = fixtures.GeneratedCrossOverlayCIdentityTests
    NAME = Fixture.NAME
    FRIENDLY = "drawFixture"
    Elf = Fixture.Elf
    fixture = Fixture.fixture
    resolve = Fixture.resolve

    def test_extracted_helper_is_pinned_by_search_runner(self):
        import permute_batch as batch
        import overlay_call_context
        path, digest = batch._LOADED_IMPLEMENTATIONS['overlay_call_context']
        self.assertEqual(path.resolve(), Path(overlay_call_context.__file__).resolve())
        self.assertEqual(digest, __import__('hashlib').sha256(path.read_bytes()).hexdigest())

    def friendly_fixture(self, root, *, caller_friendly=False):
        args = self.fixture(root)
        atlas, canonical, linked, caller, source, obj = args
        source.write_text("void %s(void) {}\n" % self.FRIENDLY)
        os.utime(source, ns=(obj.stat().st_mtime_ns - 1000000,) * 2)
        canonical._symbols[0] = (self.FRIENDLY, 16, 8, rs.STT_FUNC, 1)
        linked._symbols[0] = (self.FRIENDLY, rs.SYNTHETIC_VMA + 16, 8, rs.STT_FUNC, 1)
        linked._symbols.append((self.NAME, rs.SYNTHETIC_VMA + 16, 0, 0, rs.SHN_ABS))
        if caller_friendly:
            caller._symbols[0] = (self.FRIENDLY, 0, 0, 0, rs.SHN_UNDEF)
        (root / 'overlay_undefined_syms.us.txt').write_text(
            '%s = %s;\n' % (self.NAME, self.FRIENDLY))
        os.utime(root / 'overlay_undefined_syms.us.txt',
                 ns=(linked.path.stat().st_mtime_ns - 1000000,) * 2)
        return args

    def test_generated_and_friendly_callers_require_real_definition(self):
        for friendly in (False, True):
            with self.subTest(friendly=friendly), tempfile.TemporaryDirectory() as td:
                root = Path(td)
                args = self.friendly_fixture(root, caller_friendly=friendly)
                self.assertEqual({self.FRIENDLY if friendly else self.NAME: (8, 16)},
                                 self.resolve(root, *args))

    def test_alias_does_not_replace_independent_proof(self):
        for defect in ('map', 'definition', 'object_name', 'linked_name', 'registry',
                       'stale', 'stale_map', 'wrong_map', 'rom', 'instruction', 'conditional'):
            with self.subTest(defect=defect), tempfile.TemporaryDirectory() as td:
                root = Path(td); args = self.friendly_fixture(root)
                atlas, canonical, linked, caller, source, obj = args
                rom = None
                if defect == 'map': (root / 'overlay_undefined_syms.us.txt').unlink()
                elif defect == 'stale_map':
                    os.utime(root / 'overlay_undefined_syms.us.txt',
                             ns=(linked.path.stat().st_mtime_ns + 1000000,) * 2)
                elif defect == 'wrong_map':
                    path = root / 'overlay_undefined_syms.us.txt'
                    path.write_text('%s = wrongFriendly;\n' % self.NAME)
                    os.utime(path, ns=(linked.path.stat().st_mtime_ns - 1000000,) * 2)
                elif defect == 'definition': source.write_text('void other(void) {}\n')
                elif defect == 'conditional': source.write_text('#if 0\nvoid drawFixture(void) {}\n#endif\n')
                elif defect == 'object_name': canonical._symbols[0] = (self.NAME, 16, 8, rs.STT_FUNC, 1)
                elif defect == 'linked_name': linked._symbols = [linked._symbols[-1]]
                elif defect == 'registry': atlas['modules'][0]['mixed_tu_exact_c_ranges'] = []
                elif defect == 'stale': os.utime(source, ns=(obj.stat().st_mtime_ns + 1000000,) * 2)
                elif defect == 'rom': rom = bytes(0x110) + b'\x01' + bytes(47)
                elif defect == 'instruction': canonical._text = bytes(16) + b'\x01' + bytes(47)
                if defect in ('definition', 'conditional'):
                    os.utime(source, ns=(obj.stat().st_mtime_ns - 1000000,) * 2)
                self.assertEqual({}, self.resolve(root, *args, rom=rom))

    def test_conflicting_aliases_raise(self):
        for extra in ('{g} = another;\n', '{g} = {f};\n', '{g} = 0;\n',
                      '{f} = other;\n', 'func_overlay_009_F0000010_210 = {f};\n'):
            with self.subTest(extra=extra), tempfile.TemporaryDirectory() as td:
                root = Path(td); args = self.friendly_fixture(root)
                path = root / 'overlay_undefined_syms.us.txt'
                path.write_text(path.read_text() + extra.format(g=self.NAME, f=self.FRIENDLY))
                with self.assertRaises(rs.SurfaceComparisonError):
                    self.resolve(root, *args)

    def test_wrong_linked_alias_value_raises(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); args = self.friendly_fixture(root)
            args[2]._symbols[-1] = (self.NAME, rs.SYNTHETIC_VMA + 20, 0, 0, rs.SHN_ABS)
            with self.assertRaisesRegex(rs.SurfaceComparisonError, 'alias conflicts'):
                self.resolve(root, *args)

    def test_conflicting_canonical_generated_alias_raises(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); args = self.friendly_fixture(root)
            args[1]._symbols.append((self.NAME, 20, 8, rs.STT_FUNC, 1))
            with self.assertRaisesRegex(rs.SurfaceComparisonError, 'alias conflicts'):
                self.resolve(root, *args)

    def test_alias_map_change_during_read_raises(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            atlas, canonical, linked, caller, source, obj = self.friendly_fixture(root)
            def replace(_path):
                (root / 'overlay_undefined_syms.us.txt').write_text('changed')
                return canonical
            with self.assertRaisesRegex(rs.SurfaceComparisonError, 'inputs changed'):
                rs._stable_overlay_call_identities(root / 'missing', caller, 1, linked,
                    atlas, 0, 4, root=root, elf_loader=replace, rom=bytes(0x140))


if __name__ == '__main__':
    unittest.main()
