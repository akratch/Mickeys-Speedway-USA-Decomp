#!/usr/bin/env python3
"""Focused field, removal, mapping and artifact refusals; no game execution."""
from dataclasses import replace
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import check_editable_layout as pilot
from runlink import codec, config

class LayoutTests(unittest.TestCase):
    def setUp(self):
        self.cfg=config.load("msu-us",root=pilot.ROOT)
        self.module=codec.Module(1,codec.Header(text=16,data=16,bss=16,init=0),bytes(32))

    def test_odd_export_count_has_exact_header_dma_and_separate_aligned_base(self):
        tables,cfg,end=pilot.checked_layout([], [codec.OrtRow(1,0)], [self.module], self.cfg)
        self.assertEqual(32,end-cfg.headers_rom)
        self.assertEqual(4,cfg.modules_rom-end)
        self.assertEqual(1,len(tables.modules))
        self.assertEqual(0,tables.modules[0].header.rom)

    def test_fields_and_sparse_module_lists_refuse_before_output(self):
        for row in (codec.OrtRow(1,1<<20),codec.OrtRow(1<<12,0)):
            with self.assertRaises((pilot.PilotError,codec.FormatError)):
                pilot.checked_layout([], [row], [self.module], self.cfg)
        for module in (replace(self.module,number=2),
                       replace(self.module,header=replace(self.module.header,init=16)),
                       replace(self.module,table1=[codec.Record(0,0,4,0)]*4096)):
            with self.assertRaises(pilot.PilotError):
                pilot.checked_layout([], [codec.OrtRow(1,0)], [module], self.cfg)
        with self.assertRaises(pilot.PilotError):
            pilot.checked_layout([], [codec.OrtRow(1,0)],
                [replace(self.module,table2=[codec.Record(0,0,4,1)]*2)], self.cfg)

    def test_removal_reports_actual_caller_and_allows_unreferenced_tail(self):
        tables=codec.Tables([], [codec.OrtRow(2,0)],
            [replace(self.module,table1=[codec.Record(0,0,4,0)]),replace(self.module,number=2)])
        with self.assertRaisesRegex(pilot.PilotError,"caller.*target"):
            pilot.removed_targets(tables,{2},["target"],{(1,0,4):"caller"})
        tables.modules[0].table1=[]
        pilot.removed_targets(tables,{2},["target"])

    def test_mapper_removes_only_declared_fill_and_known_selectors(self):
        source="SECTIONS {\n"
        for name in sorted(pilot.metadata.TOP):source+=f"    build/assets/{name}.o(.data);\n"
        source+="    overlay_001_ROM_START = __romPos;\n"
        for section in pilot.SECTIONS:source+=f"    build/src/overlays/o075/overlay_075.c.o({section});\n"
        source+="    build/src/unrelated.c.o(.text);\n"
        source+="\n    rom_fill_ROM_START = __romPos;\n    .rom_fill : { build/assets/rom_fill.bin.o(.data); }\n    rom_fill_ROM_END = __romPos;\n\n    /DISCARD/ : { *(*) }\n}\n"
        result=pilot.script_for(source,pilot.metadata.TOP,"build/editable-layout/test/generated",
                               "build/editable-layout/test/module.o",None)
        self.assertIn("build/src/unrelated.c.o(.text);",result)
        self.assertNotIn("rom_fill",result)
        self.assertEqual(4,result.count("build/editable-layout/test/module.o"))
        with self.assertRaises(pilot.PilotError):
            pilot.script_for(source.replace("rom_fill_ROM_END", "overlay_extra_ROM_END"),pilot.metadata.TOP,
                             "build/editable-layout/test/generated","build/editable-layout/test/module.o",None)

    def test_missing_or_tampered_artifacts_and_changed_inputs_refuse(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);directory=root/"build/editable-layout/check";directory.mkdir(parents=True)
            artifact=directory/"final.elf";artifact.write_bytes(b"ordinary output")
            input_path=root/"source.c";input_path.write_bytes(b"source")
            receipt={"schema":"mickey-editable-layout-check-v1","nonmatching_diagnostic":True,
                     "artifacts":pilot.artifact_hashes(directory),"inputs":{"source.c":pilot.sha(input_path)},
                     "tool_context":{},"canonical_context":{}}
            path=directory/"receipt.json";pilot.save(path,receipt)
            with mock.patch.object(pilot,"tool_context",return_value={}),mock.patch.object(pilot.metadata,"provenance_context",return_value={}):
                pilot.check_receipt(root,path)
                original=artifact.read_bytes();artifact.write_bytes(b"changed")
                self.assertNotEqual(original,artifact.read_bytes())
                with self.assertRaisesRegex(pilot.PilotError,"tampered"):pilot.check_receipt(root,path)
                artifact.write_bytes(original);artifact.unlink()
                with self.assertRaisesRegex(pilot.PilotError,"missing"):pilot.check_receipt(root,path)
                artifact.write_bytes(original);input_path.write_bytes(b"different source")
                with self.assertRaisesRegex(pilot.PilotError,"stale diagnostic input"):pilot.check_receipt(root,path)

if __name__=="__main__":unittest.main()
