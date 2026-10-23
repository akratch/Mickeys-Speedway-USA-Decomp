#!/usr/bin/env python3
"""Source-derived module identities never borrow the target site's identity."""
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest

import proof_provenance as pp
import runlink_pack as rp
import source_module_link_proof as proof
from test_link_overlay_module import fixture


def canonical(extra=()):
    def symbol(name, value, section=".main"):
        return SimpleNamespace(name=name, value=value, bind=rp.STB_GLOBAL,
                               shndx=1, section=section)
    return SimpleNamespace(symbols=[
        symbol("main_TEXT_START", 0x80000450), symbol("runlinkCodeEnd", 0x80001000),
        symbol("main_BSS_START", 0x80002000, ".main_bss"),
        symbol("main_BSS_END", 0x80003000, ".main_bss"),
        *[symbol(name, value, section) for name, value, section in extra],
    ])


class SourceIdentityTests(unittest.TestCase):
    def test_local_sections_and_exports_resolve_without_a_target_table(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "raw.o"
            path.write_bytes(fixture())
            raw = rp.Elf(path)
            actual = proof.source_identities(raw, raw,
                canonical([("external", 0x80000500, ".main")]),
                rp.OrtNames(["exported", "external"], {}), 1)
            self.assertEqual({(0, 4): (0, 0xB0), (4, 4): (1, 12), (8, 4): (1, 16),
                              (20, 5): (1, 48), (24, 6): (1, 48),
                              (32, 2): (1, 16), (36, 2): (1, 12)}, actual)

    def test_missing_or_shadowed_canonical_export_is_refused(self):
        cases = [[], [("external", 0, "ABS")],
                 [("external", 0, "ABS"), ("external", 0x80000500, ".main")]]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "raw.o"
            path.write_bytes(fixture())
            raw = rp.Elf(path)
            for symbols in cases:
                with self.subTest(symbols=symbols), self.assertRaises(pp.MetadataProofError):
                    proof.source_identities(raw, raw, canonical(symbols),
                        rp.OrtNames(["exported", "external"], {}), 1)


if __name__ == '__main__':
    unittest.main()
