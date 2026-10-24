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

    def test_loader_allocation_includes_rt1_but_allocates_rt2_separately(self):
        record=codec.Record(0,0,codec.R_MIPS_26,codec.OP_SYMBOL)
        maximum=(1<<31)-1
        # The ROM-backed body stays tiny; only its declared NOBITS grows.
        exact=replace(self.module,header=replace(self.module.header,bss=maximum-32-codec.RECORD_SIZE),
                      table1=[record])
        pilot.checked_layout([], [codec.OrtRow(1,0)], [exact], self.cfg)
        overflow=replace(exact,header=replace(exact.header,bss=exact.header.bss+1))
        with self.assertRaisesRegex(pilot.PilotError,"allocation overflow including RT1"):
            pilot.checked_layout([], [codec.OrtRow(1,0)], [overflow], self.cfg)
        separate=replace(self.module,header=replace(self.module.header,bss=maximum-32),
                         table2=[record])
        pilot.checked_layout([], [codec.OrtRow(1,0)], [separate], self.cfg)

    def test_every_relocation_requires_its_complete_four_byte_word(self):
        for table in ("table1","table2"):
            for kind in codec.TYPE_NAMES:
                last=codec.Record(0,28,kind,codec.OP_SYMBOL)
                valid=replace(self.module,**{table:[last]})
                pilot.checked_layout([], [codec.OrtRow(1,0)], [valid], self.cfg)
                for site in (29,30,31):
                    partial=replace(valid,**{table:[replace(last,site=site)]})
                    with self.subTest(table=table,type=kind,site=site):
                        with self.assertRaisesRegex(pilot.PilotError,"relocation word escapes body"):
                            pilot.checked_layout([], [codec.OrtRow(1,0)], [partial], self.cfg)

    def test_resident_call_delta_requires_the_actual_declared_replacement(self):
        old=[(".text",0,4,"residentCall"),(".text",8,4,"existingOverlay")]
        new=[(".text",0,4,"sourceOverlay"),(".text",8,4,"existingOverlay")]
        self.assertEqual(0,pilot.resident_call_delta(old,new,"residentCall","sourceOverlay"))
        changed=[new[0],(".text",8,4,"differentOverlay")]
        with self.assertRaisesRegex(pilot.PilotError,"not one call-target replacement"):
            pilot.resident_call_delta(old,changed,"residentCall","sourceOverlay")

    def test_resident_witness_preserves_old_records_and_refuses_placeholder(self):
        entry=pilot.rp.MainEntry
        old=[entry(0,0,4,0,100,"first"),entry(1,8,4,0,108,"second")]
        witness=entry(2,4,4,0,104,"sourceOverlay")
        self.assertIs(witness,pilot.resident_main_delta(old,[old[0],witness,old[1]],104,"sourceOverlay"))
        with self.assertRaisesRegex(pilot.PilotError,"identity/order changed"):
            pilot.resident_main_delta(old,[old[1],witness,old[0]],104,"sourceOverlay")
        with self.assertRaisesRegex(pilot.PilotError,"lacks a raw name"):
            pilot.resident_main_delta(old,[old[0],replace(witness,provenance="placeholder"),old[1]],104,"sourceOverlay")

    @unittest.skipUnless((pilot.ROOT/"tools/binutils/mips64-elf-ld").is_file(),
                         "MIPS linker unavailable")
    def test_resident_replay_links_its_actual_data_definition(self):
        from runlink.synth import ElfBuilder, GLOBAL, OBJECT, words

        build=pilot.ROOT/"build";build.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=build) as name:
            directory=Path(name);root=pilot.ROOT
            path=directory/"resident.o";relative=path.relative_to(root).as_posix()
            builder=ElfBuilder().section(".text",words((15<<26)|(8<<16),
                (35<<26)|(8<<21)|(2<<16))).section(".data",words(7))
            builder.sym("threshold",0,4,GLOBAL,OBJECT,".data")
            builder.rel(".text",0,codec.R_MIPS_HI16,"threshold")
            builder.rel(".text",4,codec.R_MIPS_LO16,"threshold")
            path.write_bytes(builder.build())
            script=directory/"reference.ld"
            script.write_text('SECTIONS { .main 0x80001000 : { '+relative
                +'(.text) . = ALIGN(16); '+relative+'(.data) } /DISCARD/ : { *(*) } }\n')
            commands=[];reference=directory/"reference.elf";mapping=directory/"reference.map"
            pilot.run(root,["tools/binutils/mips64-elf-ld","-e","0","-T",
                script.relative_to(root),"-Map",mapping.relative_to(root),"-o",
                reference.relative_to(root)],directory/"reference.log",commands)
            elf=pilot.rp.Elf(reference);inputs=pilot.rp.parse_map(mapping)
            offset,size,_snapshot=pilot.resident_replay(root,directory,elf,inputs,
                commands,relative,pilot.sha(path),"replay")
            self.assertEqual((offset,size),(0,8))
            linked=pilot.rp.Elf(directory/"replay.elf")
            self.assertEqual(linked.bytes_of(".data"),words(7))
            with self.assertRaisesRegex(pilot.PilotError,"unique complete mapped input"):
                pilot.resident_replay(root,directory,elf,
                    [item for item in inputs if item.insec!=".data"],commands,
                    relative,pilot.sha(path),"missing-data")

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
        for section in pilot.SECTIONS:source+=f"    build/src/main/spranim.c.o({section});\n"
        source+="    build/src/unrelated.c.o(.text);\n"
        source+="\n    rom_fill_ROM_START = __romPos;\n    .rom_fill : { build/assets/rom_fill.bin.o(.data); }\n    rom_fill_ROM_END = __romPos;\n\n    /DISCARD/ : { *(*) }\n}\n"
        result=pilot.script_for(source,pilot.metadata.TOP,"build/editable-layout/test/generated",
                               "build/editable-layout/test/module.o",None)
        self.assertIn("build/src/unrelated.c.o(.text);",result)
        self.assertNotIn("rom_fill",result)
        self.assertEqual(4,result.count("build/editable-layout/test/module.o"))
        resident=pilot.script_for(source,pilot.metadata.TOP,"build/editable-layout/test/generated",
                                  "build/editable-layout/test/module.o",None,"build/editable-layout/test/spranim.c.o")
        self.assertEqual(4,resident.count("build/editable-layout/test/spranim.c.o"))
        self.assertNotIn("build/src/main/spranim.c.o",resident)
        with self.assertRaises(pilot.PilotError):
            pilot.script_for(source.replace("rom_fill_ROM_END", "overlay_extra_ROM_END"),pilot.metadata.TOP,
                             "build/editable-layout/test/generated","build/editable-layout/test/module.o",None)

    def test_mutation_between_validation_and_receipt_never_replaces_validated_hash(self):
        with tempfile.TemporaryDirectory() as temp:
            directory=Path(temp);artifact=directory/"final.elf"
            artifact.write_bytes(b"semantically validated")
            validated=pilot.artifact_hashes(directory)
            artifact.write_bytes(b"replacement after validation")
            self.assertNotEqual(pilot.sha(artifact),validated["final.elf"])
            with self.assertRaisesRegex(pilot.PilotError,"before receipt publication"):
                pilot.publish_receipt(directory,{},validated)
            self.assertFalse((directory/"receipt.json").exists())
            artifact.write_bytes(b"semantically validated")
            actual_save=pilot.save
            def replace_during_publication(path,value):
                artifact.write_bytes(b"replacement during receipt write")
                return actual_save(path,value)
            with mock.patch.object(pilot,"save",side_effect=replace_during_publication):
                with self.assertRaisesRegex(pilot.PilotError,"during receipt publication"):
                    pilot.publish_receipt(directory,{},validated)
            recorded=json.loads((directory/"receipt.json").read_text())["artifacts"]
            self.assertEqual(recorded,validated)
            self.assertNotEqual(recorded["final.elf"],pilot.sha(artifact))

    def test_aggregate_retains_the_generated_variant_receipt_digest(self):
        with tempfile.TemporaryDirectory() as temp:
            directory=Path(temp);variant=directory/"control";variant.mkdir()
            (variant/"final.elf").write_bytes(b"validated final output")
            validated=pilot.artifact_hashes(variant)
            receipt_hash=pilot.publish_receipt(variant,{},validated)
            expected={"control/final.elf":validated["final.elf"],"control/receipt.json":receipt_hash}
            (variant/"receipt.json").write_text('{"unproved":"replacement"}\n')
            self.assertNotEqual(receipt_hash,pilot.sha(variant/"receipt.json"))
            with self.assertRaisesRegex(pilot.PilotError,"before receipt publication"):
                pilot.publish_receipt(directory,{},expected)
            self.assertFalse((directory/"receipt.json").exists())

    def test_toolchain_replacement_during_receipt_validation_refuses(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);directory=root/"build/editable-layout/check";directory.mkdir(parents=True)
            (directory/"final.elf").write_bytes(b"validated output")
            pilot.save(directory/"receipt.json",{
                "schema":"mickey-editable-layout-check-v1","nonmatching_diagnostic":True,
                "artifacts":pilot.artifact_hashes(directory),"inputs":{},"tool_context":{},"canonical_context":{}})
            with mock.patch.object(pilot,"tool_context",side_effect=[{}, {"ido":"replacement"}]), \
                 mock.patch.object(pilot.metadata,"provenance_context",return_value={}):
                with self.assertRaisesRegex(pilot.PilotError,"changed during receipt validation"):
                    pilot.check_receipt(root,directory/"receipt.json")

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
