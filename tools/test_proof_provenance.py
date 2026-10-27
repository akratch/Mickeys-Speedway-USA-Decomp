#!/usr/bin/env python3
"""Focused source-level tests for wb proof provenance."""

from __future__ import annotations

import pathlib
import contextlib
import tempfile
import sys
import unittest
import hashlib
import struct
import dataclasses
from unittest import mock
from types import SimpleNamespace

import proof_provenance as provenance


class StaticFunctionCorrespondenceTests(unittest.TestCase):
    def fixture(self, **kwargs):
        import runlink_pack as rp
        import test_static_symbols as fixture
        original = fixture.build_object(**kwargs)
        prepared = fixture.ss.add_static_symbols(original)[0]
        return rp.Elf('original', original), rp.Elf('prepared', prepared), fixture.ss

    def prove(self, original, prepared, statics, cpp=None):
        cpp = cpp or b'void entry(void) {} static void helper(void); static void helper(void) {} static void tail(void) {}'
        # Synthetic ELF/debug fixture; production separately binds the real normalizer.
        with mock.patch.object(provenance, 'static_anchor_module', return_value=statics):
            return provenance.static_function_correspondence(original, prepared, cpp, ['-32'],
                pathlib.Path(__file__).resolve().parents[1])

    def test_removed_placeholder_is_not_the_new_function_or_a_relocation_target(self):
        original, prepared, statics = self.fixture()
        pairs, removed, added = self.prove(original, prepared, statics)
        placeholder = next(s for s in original.symbols if s.name == 'helper')
        self.assertNotIn(placeholder.index, dict(pairs))
        self.assertEqual(removed, ((placeholder.index, 'helper', 16),))
        self.assertEqual({row[1] for row in added}, {'helper', 'tail'})
        self.assertNotEqual(original.rels('.text')[0].sym.index, prepared.rels('.text')[0].sym.index)
        self.assertEqual(dict(pairs)[original.rels('.text')[0].sym.index], prepared.rels('.text')[0].sym.index)

    def test_nonstatic_missing_source_and_storage_recovery_refuse(self):
        original, prepared, statics = self.fixture()
        for cpp in (b'void entry(void) {} void helper(void) {} static void tail(void) {}',
                    b'void entry(void) {} static void tail(void) {}',
                    b'void entry(void) {} static void helper(int x); static void helper(void) {} static void tail(void) {}'):
            with self.subTest(cpp=cpp), self.assertRaises(provenance.MetadataProofError):
                self.prove(original, prepared, statics, cpp)
        original, prepared, statics = self.fixture(bss=(('storage',0),))
        with self.assertRaisesRegex(provenance.MetadataProofError, 'storage recovery'):
            self.prove(original, prepared, statics)

    def test_prepared_other_bits_and_parsed_pair_reindex_refuse(self):
        import runlink_pack as rp
        original, prepared, statics = self.fixture()
        payload = bytearray(prepared.data)
        payload[prepared.section('.symtab')['offset'] + 16 + 13] = 2
        with self.assertRaisesRegex(provenance.MetadataProofError, 'exact normalizer'):
            self.prove(original, rp.Elf('wrong-other', bytes(payload)), statics)
        for image in (original, prepared):
            image.symbols.reverse()
        with self.assertRaisesRegex(provenance.MetadataProofError, 'parsed symbol'):
            self.prove(original, prepared, statics)

    def test_referenced_undefined_placeholder_refuses(self):
        import runlink_pack as rp
        original, prepared, statics = self.fixture()
        placeholder = next(s for s in original.symbols if s.name == 'helper')
        payload = bytearray(original.data); rel = original.section('.rel.text')
        info = struct.unpack_from('>I', payload, rel['offset']+4)[0]
        struct.pack_into('>I', payload, rel['offset']+4, placeholder.index << 8 | info & 255)
        with self.assertRaisesRegex(provenance.MetadataProofError, 'compiler metadata refused'):
            self.prove(rp.Elf('referenced', bytes(payload)), prepared, statics)


class ParsedSurfaceTests(unittest.TestCase):
    @staticmethod
    def image():
        import runlink_pack as rp
        from runlink.synth import ElfBuilder
        return rp.Elf('surface', ElfBuilder().section('.text', b'abcdefgh')
            .section('.data', b'payload!').section('.bss', nobits=16)
            .sym('owner', 0, 8, 1, 1, '.data')
            .rel('.text', 0, 5, 'owner').rel('.text', 4, 6, 'owner').build())

    def test_allocated_payload_and_every_geometry_field_remain_observed(self):
        for field, value in (('name', '.changed'), ('type', 8), ('flags', 2),
                             ('addr', 4), ('size', 4), ('align', 8)):
            with self.subTest(field=field):
                image = self.image()
                before = provenance._static_parsed_surface(image)
                image.section('.data')[field] = value
                self.assertNotEqual(before, provenance._static_parsed_surface(image))
                self.assertEqual(before, provenance._static_parsed_surface(self.image()))
        image = self.image()
        before = provenance._static_parsed_surface(image)
        offset = image.section('.data')['offset']
        image.data = image.data[:offset] + b'P' + image.data[offset + 1:]
        self.assertNotEqual(before, provenance._static_parsed_surface(image))

    def test_symbol_fields_indices_and_ordered_relocations_remain_observed(self):
        for field, value in (('name', 'changed'), ('value', 4), ('size', 4),
                             ('bind', 0), ('type', 2), ('section', '.bss'),
                             ('index', 99), ('shndx', 3)):
            with self.subTest(field=field):
                image = self.image()
                before = provenance._static_parsed_surface(image)
                owner = next(s for s in image.symbols if s.name == 'owner')
                image.symbols[owner.index] = dataclasses.replace(owner, **{field: value})
                self.assertNotEqual(before, provenance._static_parsed_surface(image))
        image = self.image()
        before = provenance._static_parsed_surface(image)
        image.symbols.reverse()
        self.assertNotEqual(before, provenance._static_parsed_surface(image))
        image = self.image()
        before = provenance._static_parsed_surface(image)
        records = image.rels('.text')
        variants = (records[::-1], records + records[:1],
                    [dataclasses.replace(records[0], offset=4), records[1]],
                    [dataclasses.replace(records[0], type=2), records[1]],
                    [dataclasses.replace(records[0], sym=dataclasses.replace(records[0].sym,
                        name='another')), records[1]])
        for rows in variants:
            with self.subTest(rows=rows), mock.patch.object(image, 'rels',
                    side_effect=lambda section: rows if section == '.text' else []):
                self.assertNotEqual(before, provenance._static_parsed_surface(image))

    def test_snapshot_is_detached_from_mutable_loaded_bytes_and_parsed_lists(self):
        image = self.image()
        image.data = bytearray(image.data)
        before = provenance._static_parsed_surface(image)
        image.data[image.section('.data')['offset']] ^= 1
        self.assertNotEqual(before, provenance._static_parsed_surface(image))
        image.symbols.clear()
        image.sections.reverse()
        unchanged = self.image()
        unchanged.data = bytearray(unchanged.data)
        self.assertEqual(before, provenance._static_parsed_surface(unchanged))

    def test_payload_boundaries_and_allocation_order_are_distinct(self):
        image = self.image()
        snapshots = []
        for text, data in ((b'ab', b'c'), (b'a', b'bc')):
            with mock.patch.object(image, 'bytes_of', side_effect=lambda name:
                    {'.text': text, '.data': data}.get(name, b'')):
                snapshots.append(provenance._static_parsed_surface(image))
        self.assertNotEqual(*snapshots)
        before = provenance._static_parsed_surface(image)
        image.sections[1:3] = reversed(image.sections[1:3])
        self.assertNotEqual(before, provenance._static_parsed_surface(image))

    def test_audit_json_shape_and_persisted_literal_digest_are_unchanged(self):
        import json
        import runlink_metadata_link as link
        image = self.image()
        surface = provenance._static_parsed_surface(image)
        audit = json.loads(link.json_bytes({'prepared_surface': surface}))
        digest, indices = audit['prepared_surface']
        self.assertRegex(digest, r'^[0-9a-f]{64}$')
        self.assertEqual(indices, [[s.index, s.shndx] for s in image.symbols])
        self.assertEqual(provenance.literal_surface(image),
            '51b656da18831707b0154e039d337f156cefc7a5c4794c5a4ca9064b74b64c2c')


class PointerScalarSourceTests(unittest.TestCase):
    source = ('typedef unsigned char u8; typedef long s32; extern u8 first[]; extern u8 second[]; '
              'u8 *pointers[2]={first,second}; s32 one=0; s32 two=0; s32 three=0;')

    def test_closed_source_types_extern_targets_and_profile(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        facts=provenance.pointer_scalar_facts(self.source.encode(),['-32'],root)
        self.assertEqual(facts['count'],2)
        mutations=[self.source.replace('s32 one','float one'),self.source.replace('s32 one','unsigned long one'),
                   self.source.replace('first[]','*first'),self.source.replace('extern u8 first[]','extern const u8 first[]'),
                   self.source.replace('{first,second}','{0,second}'),self.source.replace('[2]','[3]'),
                   self.source+' s32 extra=0;',self.source.replace('extern u8 first[]','extern u8 first[]; extern u8 first[]')]
        for source in mutations:
            with self.subTest(source=source),self.assertRaises(provenance.MetadataProofError):
                provenance.pointer_scalar_facts(source.encode(),['-32'],root)
        group=' --local-ro-provider-group build/src/overlays/o001/a.c.o build/src/overlays/o001/b.c.o'
        command='helper --local-ro-owner-profile pointer-array-three-s32-v1'+group
        self.assertEqual(provenance.local_ro_owner_profile(command,root),'pointer-array-three-s32-v1')
        self.assertIsNone(provenance.local_ro_owner_profile('helper'+group,root))
        for bad in [command.replace('three-s32-v1','unknown'),command+' --local-data-objects']:
            with self.assertRaises(provenance.MetadataProofError):provenance.local_ro_owner_profile(bad,root)

    def test_witness_rejects_late_parsed_metadata_and_cpp_mutation(self):
        import runlink_pack as rp
        from test_link_overlay_module import pointer_scalars_fixture
        root=pathlib.Path(__file__).resolve().parent.parent
        with tempfile.TemporaryDirectory(dir=root/'build') as directory:
            cpp=pathlib.Path(directory)/'source.i';cpp.write_text(self.source)
            raw=rp.Elf('synthetic',pointer_scalars_fixture())
            record=provenance.PointerScalarCapture(cpp.read_bytes(),('-32',),cpp,
                hashlib.sha256(raw.data).hexdigest(),provenance._POINTER_SCALAR_SEAL)
            witness=provenance.pointer_scalar_witness(raw,record,root);self.assertTrue(witness.current(raw))
            with self.assertRaises(provenance.MetadataProofError):
                provenance.pointer_scalar_witness(raw,{'cpp':str(cpp),'flags':['-32']},root)
            raw.symbols[2]=dataclasses.replace(raw.symbols[2],size=12)
            with self.assertRaises(provenance.MetadataProofError):witness.current(raw)
            cpp.write_text(self.source.replace('first,second','second,first'))
            with self.assertRaises(provenance.MetadataProofError):
                provenance.pointer_scalar_witness(rp.Elf('fresh',pointer_scalars_fixture()),record,root)


class CaptureDeadlineTests(unittest.TestCase):
    """Defined clock advances test budgets, not real compiler performance."""

    def setUp(self):
        import permute_batch as batch
        self.batch = batch
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)
        self.source = self.root / "src/overlays/o059/source.c"
        self.header = self.source.with_name("dependency.h")
        self.configured = self.root / "build/src/overlays/o059/source.c.o"
        self.linked = self.root / "build/image.elf"
        for name in ("Makefile", "mk/objects.mk", "baseroms/mickey.us.z64",
                     "config/overlays.us.json", "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt",
                     "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
                     "tools/rebind_elf_relocations.py", "tools/externalize_elf_section.py",
                     "tools/postprocess_guard.py", "tools/binutils/mips64-elf-objcopy",
                     "config/runlink-ort.us.txt", "tools/link_overlay_module.py",
                     "tools/runlink_pack.py", "tools/binutils/mips64-elf-ld",
                     "include/fixture.h", "tools/asm-processor/build.py"):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture\n")
        for path in (self.source, self.header, self.configured, self.linked):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("original fixture\n")
        self.clock = 1000.0
        self.operations = []
        self.compile_delay = self.recipe_delay = 0
        self.recipe_extra = ""
        stack = contextlib.ExitStack()
        self.addCleanup(stack.close)
        stack.enter_context(mock.patch.object(batch, "ROOT", self.root))
        stack.enter_context(mock.patch.object(batch, "checked_tool_identity"))
        stack.enter_context(mock.patch.object(batch, "sweep_tool_identity", return_value={"compiler": "fixture"}))
        stack.enter_context(mock.patch.object(batch, "bounded_capture", side_effect=self.command))
        stack.enter_context(mock.patch.object(batch, "source_dependencies", side_effect=self.dependencies))
        stack.enter_context(mock.patch.object(provenance, "producer_link_context", return_value={"inputs": {}}))
        stack.enter_context(mock.patch.object(provenance.time, "monotonic", side_effect=lambda: self.clock))
        stack.enter_context(mock.patch.dict(provenance.os.environ, {"PROMOTION_TRIAL": "0"}))

    def command(self, arguments, deadline, **_keywords):
        kind = "recipe" if arguments[0] == "gmake" else "compiler"
        self.operations.append((kind, deadline))
        self.batch.remaining_timeout(deadline)
        if kind == "recipe":
            target = self.configured.relative_to(self.root).as_posix()
            source = self.source.relative_to(self.root).as_posix()
            output = (f"tools/ido/cc -c -32 {self.recipe_extra} -o {target} {source}\n"
                      f"python3 tools/link_overlay_module.py {target}\n")
            self.clock += self.recipe_delay
        else:
            pathlib.Path(arguments[arguments.index("-o") + 1]).write_text("untouched compiler fixture\n")
            output = ""
            self.clock += self.compile_delay
        return SimpleNamespace(stdout=output)

    def dependencies(self, _source, _arguments, deadline):
        self.operations.append(("dependencies", deadline))
        self.batch.remaining_timeout(deadline)
        return {self.header.relative_to(self.root).as_posix(): provenance.sha256_file(self.header)}

    def capture(self):
        return provenance.capture_data_module_producer_raw(
            self.root, self.source, self.configured, self.linked, self.root / "build/image.map")

    def test_completed_capture_age_does_not_expire_a_new_bounded_revalidation(self):
        _raw, _plan, receipt, current = self.capture()
        self.assertEqual({deadline for _kind, deadline in self.operations}, {1120.0})
        self.assertEqual([kind for kind, _deadline in self.operations],
                         ["recipe", "recipe", "dependencies", "compiler", "recipe",
                          "dependencies", "recipe", "dependencies"])
        self.clock = 1121.0
        self.operations.clear()
        self.assertEqual(receipt["inputs"], current())
        self.assertEqual(self.operations, [("recipe", 1241.0), ("dependencies", 1241.0)])

    def test_aged_context_still_refuses_changed_source_dependency_or_recipe(self):
        import runlink_pack as rp
        for changed in ("source", "dependency", "recipe"):
            with self.subTest(changed=changed):
                self.clock = 1000.0
                self.recipe_extra = ""
                self.source.write_text("original fixture\n")
                self.header.write_text("original fixture\n")
                _raw, _plan, receipt, current = self.capture()
                self.clock = 1121.0
                if changed == "source": self.source.write_text("changed source\n")
                if changed == "dependency": self.header.write_text("changed dependency\n")
                if changed == "recipe": self.recipe_extra = "-DCHANGED"
                world = SimpleNamespace(data_import_contexts=[(receipt["inputs"], current)])
                with self.assertRaises((rp.PackError, provenance.MetadataProofError)):
                    rp.check_data_import_holds(world)
                self.assertIsNone(world._guarded_data_objects)

    def test_original_capture_never_renews_its_internal_budget(self):
        self.compile_delay = 121
        with self.assertRaisesRegex(TimeoutError, "whole-batch deadline"):
            self.capture()
        self.assertEqual({deadline for _kind, deadline in self.operations}, {1120.0})
        self.assertGreater(self.clock, 1120)

    def test_later_recipe_and_dependency_checks_share_one_new_budget(self):
        _raw, _plan, _receipt, current = self.capture()
        self.clock = 1121.0
        self.recipe_delay = 121
        self.operations.clear()
        with self.assertRaisesRegex(TimeoutError, "whole-batch deadline"):
            current()
        self.assertEqual(self.operations, [("recipe", 1241.0), ("dependencies", 1241.0)])

    def test_keep_capture_holds_original_replay_and_exact_recipe_through_late_checks(self):
        original, prepared = KeepOnlyRecipeTests.objects()
        self.configured.write_bytes(prepared.data)
        target = self.configured.relative_to(self.root).as_posix()
        metadata = KeepOnlyRecipeTests().command().replace("build/src/overlays/o073/source.c.o", target)
        metadata = metadata.replace("a"*64, hashlib.sha256(original.bytes_of(".rodata")).hexdigest())
        changed = {"suffix": "", "metadata": metadata}
        old_command = self.command
        compiled_output = {}
        def command(arguments, deadline, **keywords):
            result = old_command(arguments, deadline, **keywords)
            if arguments[0] == "gmake":
                result.stdout = result.stdout.splitlines()[0] + "\n" + changed["metadata"] + changed["suffix"] + "\n"
            else:
                output = pathlib.Path(arguments[arguments.index("-o")+1])
                output.write_bytes(original.data)
                compiled_output["path"] = output
            return result
        def replay(_raw, _command, _target, output, *_args, **_keywords):
            output.write_bytes(prepared.data)
            return output
        with mock.patch.object(self.batch, "bounded_capture", side_effect=command), mock.patch.object(
                provenance, "replay_metadata", side_effect=replay):
            raw, _plan, receipt, current = provenance._capture_configured_raw(
                self.root, self.source, self.configured, self.linked, "",
                producer_map=self.root/"build/image.map", preserve_compiler=True)
            self.assertEqual(receipt["kept_rodata"]["original_ro_records"], 1)
            self.assertEqual(receipt["raw_sha256"], receipt["kept_rodata"]["original"])
            self.assertEqual(receipt["inputs"], current())
            untouched = self.root/receipt["compiler_original"]
            self.assertEqual(untouched.read_bytes(), original.data)
            untouched.write_bytes(b"stale original compiler")
            with self.assertRaisesRegex(provenance.MetadataProofError, "observation changed"):
                current()
            untouched.write_bytes(original.data)
            raw.write_bytes(b"stale")
            with self.assertRaisesRegex(provenance.MetadataProofError, "observation changed"):
                current()
            raw.write_bytes(original.data)
            replayed = self.root/receipt["kept_rodata"]["prepared_object"]
            replayed.write_bytes(b"stale")
            with self.assertRaisesRegex(provenance.MetadataProofError, "observation changed"):
                current()
            replayed.write_bytes(prepared.data)
            for suffix, recipe in (("\nunknown "+target, metadata), ("\n"+metadata, metadata),
                                   ("", metadata.replace("0 keep", "0")), ("", "")):
                changed.update(suffix=suffix, metadata=recipe)
                with self.subTest(suffix=suffix, recipe=recipe), self.assertRaises(provenance.MetadataProofError):
                    current()
            changed.update(suffix="", metadata=metadata)
            self.assertEqual(receipt["inputs"], current())
            compiled_output.clear()
            def changed_after_compile(*args, **keywords):
                result = self.dependencies(*args, **keywords)
                if "path" in compiled_output:
                    output = compiled_output["path"]
                    output.write_bytes(output.read_bytes() + b"changed")
                return result
            with mock.patch.object(self.batch, "source_dependencies", side_effect=changed_after_compile):
                with self.assertRaisesRegex(provenance.MetadataProofError, "compiler output changed"):
                    provenance.capture_nonexec_producer_raw(
                        self.root, self.source, self.configured, self.linked, self.root/"build/image.map")


class KeepOnlyRecipeTests(unittest.TestCase):
    def command(self):
        target = "build/src/overlays/o073/source.c.o"
        return ("python3 tools/externalize_elf_section.py " + target + " .rodata sha256:"
                + "a"*64 + " 0 keep && tools/binutils/mips64-elf-objcopy --remove-section .rel.rodata " + target)

    def test_closed_keep_command_requires_keep_zero_digest_and_exact_removal(self):
        target = "build/src/overlays/o073/source.c.o"
        command = self.command()
        self.assertEqual(["externalize", "remove-section"],
                         [op for op, _args in provenance._keep_rodata_plan(command, target, pathlib.Path("."))])
        for changed in (command.replace("0 keep", "0"), command.replace("0 keep", "4 keep"),
                        command.replace(".rel.rodata", ".rodata"), command.replace("sha256:", "sha257:"),
                        command + " && unknown " + target, command + " && " + command):
            with self.subTest(changed=changed), self.assertRaises(provenance.MetadataProofError):
                provenance._keep_rodata_plan(changed, target, pathlib.Path("."))

    @staticmethod
    def objects():
        import runlink_pack as rp
        from test_runlink_pack import ObjBuilder, GLOBAL, FUNC
        data = (ObjBuilder().section(".text", bytes(8)).section(".rodata", bytes(4))
                .sym("function", 0, 8, GLOBAL, FUNC, ".text")
                .rel(".text", 0, rp.R_MIPS_HI16, ("section", ".rodata"))
                .rel(".text", 4, rp.R_MIPS_LO16, ("section", ".rodata"))
                .rel(".rodata", 0, rp.R_MIPS_32, ("section", ".text")).build())
        original = rp.Elf("original", data)
        readonly = bytearray(data)
        shoff = struct.unpack_from(">I", data, 32)[0]
        struct.pack_into(">I", readonly, shoff+original.section(".rodata")["index"]*40+8, 2)
        data = bytes(readonly)
        original = rp.Elf("original", data)
        prepared = bytearray(data)
        symbols = next(x for x in original.sections if x["type"] == 2)
        symbol = next(x for x in original.symbols if x.type == 3 and x.section == ".rodata")
        offset = symbols["offset"] + symbol.index*16
        struct.pack_into(">II", prepared, offset+4, 0, 0)
        struct.pack_into(">H", prepared, offset+14, rp.SHN_ABS)
        shoff = struct.unpack_from(">I", data, 32)[0]
        struct.pack_into(">I", prepared, shoff+original.section(".rel.rodata")["index"]*40+20, 0)
        return original, rp.Elf("prepared", bytes(prepared))

    def test_original_pool_records_and_loaded_symbol_surface_are_required(self):
        import runlink_pack as rp
        original, prepared = self.objects()
        provenance._validate_kept_rodata(original, prepared, rp.Elf("configured", prepared.data))
        configured = rp.Elf("configured", prepared.data)
        index = next(i for i,s in enumerate(configured.symbols) if s.name == "function")
        configured.symbols[index] = dataclasses.replace(configured.symbols[index], size=4)
        with self.assertRaisesRegex(provenance.MetadataProofError, "parsed metadata"):
            provenance._validate_kept_rodata(original, prepared, configured)
        data = bytearray(original.data)
        shoff = struct.unpack_from(">I", data, 32)[0]
        struct.pack_into(">I", data, shoff+original.section(".rel.rodata")["index"]*40+20, 0)
        with self.assertRaisesRegex(provenance.MetadataProofError, "original allocated pool/records"):
            provenance._validate_kept_rodata(rp.Elf("lost", bytes(data)), prepared, prepared)
        data = bytearray(prepared.data)
        struct.pack_into(">I", data, shoff+prepared.section(".rodata")["index"]*40+20, 0)
        with self.assertRaisesRegex(provenance.MetadataProofError, "allocated bytes"):
            provenance._validate_kept_rodata(original, rp.Elf("dropped", bytes(data)), prepared)


class EditedPrefixCaptureTests(unittest.TestCase):
    def test_actual_projection_command_and_loaded_holds_are_required(self):
        import dataclasses, hashlib
        import runlink_pack as rp
        from test_runlink_pack import ObjBuilder, GLOBAL, OBJECT
        link=provenance.metadata_link
        with tempfile.TemporaryDirectory() as temporary:
            root=pathlib.Path(temporary);directory=root/'build/edited-runlink/control-one'
            directory.mkdir(parents=True)
            source='build/src/main/provider.c.o'
            script='SECTIONS { .main_bss : { '+source+'(.bss); } }'
            for name in (source,'mickey.us.yaml','tools/binutils/mips64-elf-ld',
                         'tools/proof_provenance.py','tools/runlink_metadata_link.py','tools/runlink_pack.py'):
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text('held')
            (directory/'providers.ld').write_text(script)
            path=directory/'providers.elf';path.write_bytes(ObjBuilder().section('.main_bss',nobits=16)
                .sym('cell',0,4,GLOBAL,OBJECT,'.main_bss').build())
            map_path=directory/'providers.map';map_path.write_text('held map')
            image=rp.Elf(path);item=rp.MapInput('.main_bss','.bss',0,16,source)
            context={'elf':image,'map':[item],'holds':{path:provenance.sha256_file(path),map_path:provenance.sha256_file(map_path)}}
            command=['tools/binutils/mips64-elf-ld','-r','-T',str((directory/'providers.ld').relative_to(root)),
                '-Map',str(map_path.relative_to(root)),'-o',str(path.relative_to(root))]
            inputs={source:provenance.sha256_file(root/source)}
            with mock.patch.object(link,'edited_projection',return_value=(script,'')), \
                 mock.patch.object(link,'edited_program_selectors',return_value=[source]), \
                 mock.patch.object(link,'edited_projection_tools',return_value={}):
                capture=provenance.bind_edited_prefix(root,context,directory,command,script,[command],inputs)
                self.assertEqual(capture.current()['role'],'edited-provider-prefix')
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.bind_edited_prefix(root,context,directory,command+['extra'],script,[command],inputs)
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.bind_edited_prefix(root,context,directory,command,script+'changed',[command],inputs)
                for held in (root/source,map_path,directory/'providers.ld'):
                    old=held.read_bytes();held.write_bytes(b'changed')
                    with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                    held.write_bytes(old);capture.current()
                row=next(s for s in image.symbols if s.name=='cell');index=image.symbols.index(row)
                image.symbols[index]=dataclasses.replace(row,size=8)
                with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                image.symbols[index]=row;capture.current()
                context['map']=[dataclasses.replace(item,addr=4)]
                with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                context['map']=[item]
                with mock.patch.object(provenance,'_capture_configured_raw',return_value='fresh') as raw:
                    self.assertEqual(provenance.capture_edited_provider_raw(root,root/'source.c',root/source,capture),'fresh')
                    self.assertIs(raw.call_args.kwargs['edited_prefix'],capture)
                    self.assertTrue(raw.call_args.kwargs['ordinary_provider'])
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.capture_edited_provider_raw(root,root/'source.c',root/source,{'hash':'forged'})


class SourceClassificationTests(unittest.TestCase):
    def classify(self, text: str, *, defines: tuple[str, ...] = ()) -> tuple[str, str]:
        return provenance.classify_source_selection(
            text,
            candidate_symbol="friendly",
            target_symbol="func_1234",
            defines=defines,
        )

    def test_ordinary_c_definition_is_admissible_class(self) -> None:
        text = """
        extern void friendly(int value);
        /* friendly( does not count. */
        void friendly(int value) { if (value) { friendly(value - 1); } }
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_nonmatching_define_selects_guarded_c(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )

    def test_default_build_selects_nonmatching_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_friendly_candidate_pairs_with_auto_named_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/different_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_promoted_symbol_ignores_another_functions_mixed_tu_fallback(self) -> None:
        text = """
        void promoted(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        kind, _reason = provenance.classify_source_selection(
            text,
            candidate_symbol="promoted",
            target_symbol="promoted",
            defines=(),
        )
        self.assertEqual(kind, provenance.ORDINARY_C)

    def test_unconditional_friendly_definition_ignores_another_fallback(self) -> None:
        # A promoted friendly-named body in a multi-function TU, beside another
        # function's guarded fallback: the fallback is not its alternative.
        text = """
        void friendly(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_bare_global_asm_is_fallback(self) -> None:
        text = '#pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")\n'
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_missing_symbol_fails_closed(self) -> None:
        kind, reason = self.classify("void another(void) {}\n")
        self.assertEqual(kind, provenance.UNKNOWN)
        self.assertIn("no active C definition", reason)

    def test_if_not_defined_selects_fallback_when_enabled(self) -> None:
        text = """
        #if !defined(NON_MATCHING)
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #else
        void friendly(void) { }
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )


class ManifestPrimitiveTests(unittest.TestCase):
    def test_hash_records_exact_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "candidate.o"
            path.write_bytes(b"compiled-c")
            self.assertEqual(
                provenance.sha256_file(path),
                "c5983ffc917d4649c1e0728fd58ff6be043dd548241563c16a8c0e7b43a082d7",
            )

    def test_source_facts_ignore_calls_and_comments(self) -> None:
        facts = provenance.source_facts(
            """
            // void friendly(void) {}
            void caller(void) { friendly(); }
            void friendly(void) { }
            """,
            "friendly",
        )
        self.assertEqual(len(facts.definitions), 1)

    def test_source_facts_see_kr_definitions(self) -> None:
        text = """
            void reset();
            void reset(a);
            void reset(state, count)
            State *state;
            s32 count;
            {
                reset(state, 1);
            }
            """
        facts = provenance.source_facts(text, "reset")
        self.assertEqual([row.line for row in facts.definitions], [4])
        # A prototype, a call, an initializer or a non-identifier parameter
        # list is never read as a K&R definition.
        for other in ("void reset(a); int x; {",
                      "int y = reset(a) ; int x = 1; {",
                      "void reset(int a) int a; {",
                      "void reset(a) ; {"):
            with self.subTest(other=other):
                self.assertEqual(provenance.source_facts(other, "reset").definitions, ())

    def test_source_view_reads_the_preprocessor_only_for_macro_spelled_definitions(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "src").mkdir()
            body = root / "src/entry.c"
            body.write_text("void entry(int a) { }\n")
            alias = root / "src/entryB.c"
            alias.write_text('#define entry entryB\n#include "entry.c"\n')
            other = root / "src/other.c"
            other.write_text("/* #define entryB */ void other(void) { }\n")
            expanded = '# 1 "src/entryB.c"\n# 1 "src/entry.c"\nvoid entryB(int a) { }\n'
            with mock.patch.object(provenance, "preprocessed_text", return_value=expanded) as cpp:
                self.assertEqual("source", provenance.source_view(body, "entry", root)[1])
                self.assertEqual(("preprocessed"), provenance.source_view(alias, "entryB", root)[1])
                self.assertEqual(1, len(provenance.source_facts_for(alias, "entryB", root).definitions))
                # A commented-out #define is not a spelling.
                self.assertEqual("source", provenance.source_view(other, "entryB", root)[1])
                self.assertEqual(alias, provenance._find_source(root, ("entryB",)))
                self.assertEqual({alias}, {call.args[0] for call in cpp.call_args_list})

    def test_guarded_include_uses_candidate_mode_and_retains_candidate_status(self):
        wrapper = ('#define shared friendly\n#ifdef NON_MATCHING\n'
                   '#include "shared.c"\n#else\n'
                   '#pragma GLOBAL_ASM("asm/func_1234.s")\n#endif\n')
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "wrapper.c"
            source.write_text(wrapper)
            for expanded, expected in (
                ("void friendly(void) {}\n", provenance.NON_MATCHING_C),
                ("void other(void) {}\n", provenance.UNKNOWN),
                ("void friendly(void) {}\nvoid friendly(void) {}\n", provenance.UNKNOWN),
                ('void friendly(void) {}\n#pragma GLOBAL_ASM("asm/func_1234.s")\n', provenance.UNKNOWN),
            ):
                with self.subTest(expanded=expanded), mock.patch.object(
                        provenance, "preprocessed_text", return_value=expanded) as cpp:
                    result = provenance.classify_source_file(
                        source, candidate_symbol="friendly", target_symbol="func_1234",
                        root=root, non_matching=True)
                    self.assertEqual(expected, result[0])
                    cpp.assert_called_once_with(source, root, non_matching=True)
            with mock.patch.object(provenance, "preprocessed_text",
                                   side_effect=provenance.MetadataProofError("missing include")):
                with self.assertRaisesRegex(provenance.MetadataProofError, "missing include"):
                    provenance.classify_source_file(
                        source, candidate_symbol="friendly", target_symbol="func_1234",
                        root=root, non_matching=True)
            source.write_text(wrapper.replace("func_1234.s", "wrong.s"))
            with mock.patch.object(provenance, "preprocessed_text") as cpp:
                self.assertEqual(provenance.UNKNOWN, provenance.classify_source_file(
                    source, candidate_symbol="friendly", target_symbol="func_1234",
                    root=root, non_matching=True)[0])
                cpp.assert_not_called()

    def test_preprocessed_text_runs_the_configured_compiler(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "tools/ido").mkdir(parents=True)
            cc = root / "tools/ido/cc"
            cc.write_text('#!/bin/sh\necho "$@"\n')
            cc.chmod(0o755)
            source = root / "src/a.c"
            source.parent.mkdir()
            source.write_text("")
            command = "tools/ido/cc -c -DX -I include -O2 -o build/src/a.c.o src/a.c"
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(command, ["-DX", "-I", "include", "-O2"], None)):
                self.assertEqual("-E -DX -I include -O2 src/a.c\n",
                                 provenance.preprocessed_text(source, root))
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(command, ["-DNON_MATCHING", "-I", "include"], None)) as discover:
                self.assertEqual("-E -DNON_MATCHING -I include src/a.c\n",
                                 provenance.preprocessed_text(source, root, non_matching=True))
                discover.assert_called_once_with(root, source, non_matching=True)
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(None, [], "gmake dry-run failed")):
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.preprocessed_text(source, root)

    def build_fixture_manifest(self, source_text: str, *, emit_symbol: bool,
                               non_matching: bool = False) -> dict[str, object]:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "src" / "fixture.c"
            source.parent.mkdir(parents=True)
            source.write_text(source_text)
            build_dir = root / ("build_non_matching" if non_matching else "build")
            candidate = build_dir / "src" / "fixture.c.o"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"candidate-object")
            target = root / "target.o"
            target.write_bytes(b"target-object")
            fake_objdump = root / "objdump"
            symbol_line = "00000000 g F .text 00000004 friendly\\n" if emit_symbol else ""
            fake_objdump.write_text(
                "#!/bin/sh\n"
                "test \"$1\" = -t || exit 2\n"
                f"printf '{symbol_line}'\n"
            )
            fake_objdump.chmod(0o755)
            return provenance.build_manifest(
                root=root,
                mode="asm",
                source=source,
                symbol="friendly",
                candidate_symbol="friendly",
                candidate_build_dir=build_dir,
                candidate_object=candidate,
                target_object=target,
                candidate_artifact=candidate,
                target_artifact=target,
                objdump=fake_objdump,
            )

    def test_manifest_indirect_candidate_and_preprocessing_failure(self):
        wrapper = ('#define shared friendly\n#ifdef NON_MATCHING\n'
                   '#include "shared.c"\n#else\n'
                   '#pragma GLOBAL_ASM("asm/friendly.s")\n#endif\n')
        with mock.patch.object(provenance, "_discover_compile_command", return_value=(
                "tools/ido/cc -DNON_MATCHING", ["-DNON_MATCHING"], None)):
            with mock.patch.object(provenance, "preprocessed_text",
                                   return_value="void friendly(void) {}\n"):
                manifest = self.build_fixture_manifest(wrapper, emit_symbol=True, non_matching=True)
                self.assertTrue(manifest["exact_claim_allowed"])
                self.assertEqual(provenance.NON_MATCHING_C, manifest["selection"]["classification"])
            with mock.patch.object(provenance, "preprocessed_text",
                                   side_effect=provenance.MetadataProofError("cpp failed")):
                manifest = self.build_fixture_manifest(wrapper, emit_symbol=True, non_matching=True)
                self.assertFalse(manifest["exact_claim_allowed"])
                self.assertEqual(provenance.UNKNOWN, manifest["selection"]["classification"])
                self.assertIn("cpp failed", manifest["reasons"])

    def test_manifest_allows_tied_ordinary_c(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=True
        )
        self.assertTrue(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["verdict"], "c_evidence")
        self.assertIsNotNone(manifest["candidate_object"]["sha256"])
        self.assertIsNotNone(manifest["target_object"]["sha256"])

    def test_manifest_fails_closed_when_object_symbol_is_missing(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=False
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["exact_guard"], "assert_exact_false")
        self.assertIn("does not define", " ".join(manifest["reasons"]))

    def test_manifest_fails_closed_for_global_asm(self) -> None:
        manifest = self.build_fixture_manifest(
            '#pragma GLOBAL_ASM("asm/nonmatchings/x/friendly.s")\n',
            emit_symbol=True,
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(
            manifest["selection"]["classification"], provenance.GLOBAL_ASM
        )


class ProducerCaptureLifecycleTests(unittest.TestCase):
    def test_stale_final_output_does_not_block_bound_bootstrap_producer_inputs(self):
        import runlink_metadata_link as metadata
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            names = [metadata.BOOT_ELF, metadata.BOOT_MAP, metadata.BOOT_LOG,
                     "mickey.us.ld", "tools/link_diagnose.sh", "tools/run_logged.py",
                     "tools/binutils/mips64-elf-ld", "undefined_funcs_auto.us.txt",
                     "undefined_syms_auto.us.txt", "libultra_undefined_syms.us.txt",
                     "overlay_undefined_syms.us.txt"]
            for name in names:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("synthetic producer input\n")
            with mock.patch.object(metadata, "provenance_context", side_effect=
                    metadata.MetadataLinkError("stale final output")), mock.patch.object(
                    metadata, "link_command", return_value=["ordinary-bootstrap-link"]) as link:
                producer = provenance.producer_link_context(
                    root, root / metadata.BOOT_ELF, root / metadata.BOOT_MAP)
                self.assertEqual(producer["role"], "metadata-producer-input")
                self.assertIn(metadata.BOOT_ELF, producer["inputs"])
                link.assert_called_once_with(root, final=False)
                with self.assertRaisesRegex(provenance.MetadataProofError, "stale final output"):
                    provenance.generated_metadata_context(root)
                (root / metadata.BOOT_MAP).write_text("changed map\n")
                self.assertNotEqual(producer, provenance.producer_link_context(
                    root, root / metadata.BOOT_ELF, root / metadata.BOOT_MAP))
                with self.assertRaisesRegex(provenance.MetadataProofError, "ELF-map pair"):
                    provenance.producer_link_context(root, root / metadata.BOOT_ELF,
                                                     root / "build/mickey.us.map")

    def test_local_provider_declaration_is_one_stable_ordinary_object(self):
        root = pathlib.Path("/synthetic")
        name = "build/src/overlays/o001/labels.c.o"
        self.assertEqual(provenance.local_ro_provider_input("helper --local-ro-origin-object " + name, root), name)
        for value in ("/tmp/provider.o", "build/src/overlays/o001/../labels.c.o", "build/fresh/provider.o"):
            with self.subTest(value=value), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_input("helper --local-ro-origin-object " + value, root)

    def test_nonexec_end_profile_is_explicit_and_does_not_accept_generic_crop(self):
        target="build/src/overlays/o101/tail.c.o"
        command="python3 tools/trim_elf_section.py "+target+" .data 44"
        with self.assertRaises(provenance.MetadataProofError):
            provenance.metadata_filter_plan(command,target)
        self.assertEqual([("nonexec-end",(".data",44))],provenance.metadata_filter_plan(
            command+" --compiler-end-padding",target))

    def test_ordered_provider_group_uses_only_distinct_stable_real_input_paths(self):
        root = pathlib.Path("/synthetic")
        a, b = "build/src/overlays/o001/labels.c.o", "build/src/overlays/o001/tail.c.o"
        self.assertEqual((a, b), provenance.local_ro_provider_inputs(
            "helper --local-ro-provider-group " + a + " " + b, root))
        for args in (a, a + " " + a, a + " /tmp/tail.o", a + " " + b + " --output x",
                     a + " " + b + " --local-ro-origin-object " + a):
            with self.subTest(args=args), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_inputs("helper --local-ro-provider-group " + args, root)

    def test_mixed_provider_group_is_explicit_distinct_and_excludes_old_profiles(self):
        root=pathlib.Path("/synthetic")
        a,b="build/src/overlays/o057/menu.c.o","build/src/overlays/o057/literals.c.o"
        command="helper --local-ro-mixed-provider-group "+a+" "+b
        self.assertTrue(provenance.local_ro_mixed_group(command,root))
        self.assertEqual((a,b),provenance.local_ro_provider_inputs(command,root))
        for suffix in (" --local-data-aggregate", " --local-ro-origin-object "+a,
                       " --local-ro-provider-group "+a+" "+b):
            with self.subTest(suffix=suffix), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_inputs(command+suffix,root)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.local_ro_mixed_group("helper --local-ro-mixed-provider-group "+a+" "+b+" build/src/overlays/o057/extra.c.o",root)

    def test_source_objects_require_the_explicit_mixed_storage_case(self):
        root=pathlib.Path("/synthetic")
        group="--local-ro-mixed-provider-group build/src/overlays/o057/menu.c.o build/src/overlays/o057/literals.c.o"
        self.assertTrue(provenance.local_data_objects("helper --local-data-objects "+group,root))
        self.assertFalse(provenance.local_data_objects("helper "+group,root))
        for command in ("helper --local-data-objects", "helper --local-data-objects --local-data-objects "+group,
                        "helper --local-data-aggregate --local-data-objects "+group):
            with self.subTest(command=command),self.assertRaises(provenance.MetadataProofError):
                provenance.local_data_objects(command,root)

    def test_capture_roles_are_explicit_and_promotion_keeps_the_strict_default(self):
        arguments = [pathlib.Path("root") / name for name in
                     (".", "source.c", "configured.o", "build/mickey.us.elf")]
        with mock.patch.object(provenance, "_capture_configured_raw", return_value="captured") as capture:
            self.assertEqual(provenance.capture_configured_raw(*arguments, "", module_link=True),
                             "captured")
            self.assertNotIn("producer_map", capture.call_args.kwargs)
            capture.reset_mock()
            provenance.capture_data_module_producer_raw(*arguments, pathlib.Path("root/build/mickey.us.map"))
            self.assertEqual(capture.call_args.kwargs["producer_map"], pathlib.Path("root/build/mickey.us.map"))
            self.assertTrue(capture.call_args.kwargs["module_link"])
            capture.reset_mock()
            provenance.capture_data_provider_raw(*arguments, pathlib.Path("root/build/mickey.us.map"))
            self.assertTrue(capture.call_args.kwargs["ordinary_provider"])
            self.assertNotIn("module_link", capture.call_args.kwargs)



class MixedLocalDeclarationTests(unittest.TestCase):
    def test_explicit_one_provider_profile_is_required(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        name="build/src/overlays/o058/labels.c.o"
        self.assertFalse(provenance.local_data_aggregate("helper --local-ro-origin-object "+name,root))
        self.assertTrue(provenance.local_data_aggregate("helper --local-data-aggregate --local-ro-origin-object "+name,root))
        for command in ("helper --local-data-aggregate", "helper --local-data-aggregate --local-data-aggregate --local-ro-origin-object "+name,
                        "helper --local-data-aggregate --local-ro-provider-group "+name):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.local_data_aggregate(command,root)


class LiteralArrayEndTests(unittest.TestCase):
    SOURCE = b'char first[] = "x";\nchar second[] = "yz";\n'

    @staticmethod
    def raw(*, tail=0, size=0, bad_type=False):
        import runlink_pack as rp
        from runlink.synth import ElfBuilder
        import struct
        data = bytearray(ElfBuilder().section(".data", b"x\0\0\0yz\0"+bytes(8)+bytes([tail]))
            .sym("first",0,size,1,2 if bad_type else 1,".data")
            .sym("second",4,0,1,1,".data").build())
        obj = rp.Elf("literal", data)
        struct.pack_into(">I",data,struct.unpack_from(">I",data,32)[0]+obj.section(".data")["index"]*40+8,3)
        return rp.Elf("literal", data)

    def test_closed_source_cpp_and_actual_unsized_arrays_derive_end(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        rows = provenance.literal_array_facts(self.SOURCE,self.SOURCE,root)
        raw = self.raw()
        section,end,bounds = provenance.literal_raw_end(raw,rows)
        self.assertEqual((section,end),(".data",7))
        self.assertEqual(bounds,[["first",0,2,0],["second",4,3,0]])
        import trim_elf_section as trim
        with self.assertRaises(ValueError):trim.compiler_end(raw,".data")

    def test_unsupported_source_and_effective_cpp_fail_closed(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        cases = [self.SOURCE+b'float carrier = 1.0f;',self.SOURCE.replace(b'"x"',b'"x" "y"'),
                 self.SOURCE.replace(b'"x"',b'L"x"'),self.SOURCE.replace(b'"x"',b'"x\\0"'),
                 self.SOURCE.replace(b'first[]',b'first[2]'),b'#define X "x"\nchar first[] = X;',
                 self.SOURCE.replace(b'char first',b'char *first')]
        for source in cases:
            with self.subTest(source=source),self.assertRaises(provenance.MetadataProofError):
                provenance.literal_array_facts(source,self.SOURCE,root)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.literal_array_facts(self.SOURCE,self.SOURCE.replace(b'char',b'unsigned char'),root)

    def test_actual_symbol_or_nonzero_tail_changes_refuse(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        rows = provenance.literal_array_facts(self.SOURCE,self.SOURCE,root)
        for options in ({"tail":1},{"size":2},{"bad_type":True}):
            with self.subTest(options=options),self.assertRaises((provenance.MetadataProofError,ValueError)):
                provenance.literal_raw_end(self.raw(**options),rows)

    def test_literal_plan_keeps_source_identity_and_explicit_mode(self):
        target="build/src/overlays/o057/strings.c.o"
        command="python3 tools/trim_elf_section.py "+target+" .data 7 --literal-array-end "
        self.assertEqual(provenance.metadata_filter_plan(command+"src/overlays/o057/strings.c",target),
                         [("literal-end",(".data",7,"src/overlays/o057/strings.c"))])
        for source in ("src/other.c","/tmp/source.c","src/../source.c"):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.metadata_filter_plan(command+source,target)

    def test_preloaded_parser_or_lexical_then_first_call_disk_mutation_refuses(self):
        import shutil,subprocess,json
        canonical=pathlib.Path(provenance.__file__).resolve().parent.parent
        for profile in ("literal", "static"):
          for module in ("parser","lexical"):
            with self.subTest(module=module, profile=profile),tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);tools=root/"tools";tools.mkdir()
                for name in ("proof_provenance.py","trim_elf_section.py","candidate_context.py"):
                    shutil.copy2(canonical/"tools"/name,tools/name)
                shutil.copytree(canonical/"tools/permuter/perm_pycparser",tools/"permuter/perm_pycparser",
                                ignore=shutil.ignore_patterns("__pycache__","*.pyc"))
                script = "\n".join([
                    "import sys, pathlib, hashlib",
                    "sys.path.insert(0,"+repr(str(canonical/"tools"))+")",
                    "import proof_provenance as proof",
                    "root=pathlib.Path("+repr(str(root))+")",
                    "sys.path.insert(0,str(root/'tools'))",
                    "sys.path.insert(0,str(root/'tools/permuter'))",
                    ("import perm_pycparser.c_parser" if module=="parser" else "import candidate_context"),
                    "path=root/"+repr("tools/permuter/perm_pycparser/c_parser.py" if module=="parser" else "tools/candidate_context.py"),
                    "before=path.read_bytes(); path.write_bytes(before+b'\\n# changed after loading\\n')",
                    "assert path.read_bytes()!=before",
                    ("try: proof.literal_array_facts(b'char label[]=\"x\";',b'char label[]=\"x\";',root)" if profile=="literal" else "try: proof.static_source_facts(b'static long x=0;',['-32'],root,{'x'})"),
                    "except proof.MetadataProofError as error: print(error)",
                    "else: raise SystemExit('accepted changed preloaded code')",
                ])
                result=subprocess.run([sys.executable,"-c",script],capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr+result.stdout)
                self.assertIn("preloaded" if module=="parser" else "lexical",result.stdout)

    def test_wrapped_quoted_header_shadow_matches_compiler_and_holds_chosen_input(self):
        import runlink_pack as rp
        import subprocess, shlex
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        if not (root/"tools/ido/cc").is_file() or not (root/".venv/bin/python").is_file():
            self.skipTest("actual IDO/wrapper unavailable")
        (root/"build").mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="literal-shadow-",dir=root/"build") as directory:
            base=pathlib.Path(directory);local=base/"source";global_=base/"include"
            local.mkdir();global_.mkdir()
            source=local/"probe.c";source.write_text('#include "choice.h"\nchar label[]="ok";\n')
            template="typedef {type} chosen; typedef char Check[(sizeof(chosen)==EXPECTED_WIDTH)?1:-1];\n"
            (local/"choice.h").write_text(template.format(type="short"))
            chosen=global_/"choice.h";chosen.write_text(template.format(type="int"))
            records=[]
            for wrapped,width,expected in ((True,4,"int"),(False,2,"short")):
                output=base/("wrapped.o" if wrapped else "bare.o")
                command=([".venv/bin/python","tools/asm-processor/build.py","tools/ido/cc","--",
                          "tools/binutils/mips64-elf-as","--"] if wrapped else ["tools/ido/cc"])
                command += ["-c","-G","0","-non_shared","-mips2","-O2","-32","-nostdinc",
                            "-I",str(global_),"-DEXPECTED_WIDTH="+str(width),"-o",str(output),
                            source.relative_to(root).as_posix()]
                result=subprocess.run(command,cwd=root,capture_output=True)
                self.assertEqual(result.returncode,0,result.stderr.decode(errors="replace"))
                record=provenance.prove_literal_arrays(rp.Elf(output),source,root,
                                                       compiler_command=shlex.join(command))
                self.assertIn("typedef "+expected+" chosen",record["cpp_text"])
                selected=(chosen if wrapped else local/"choice.h").relative_to(root).as_posix()
                rejected=(local/"choice.h" if wrapped else chosen).relative_to(root).as_posix()
                self.assertIn(selected,record["inputs"])
                self.assertNotIn(rejected,record["inputs"])
                self.assertEqual(record["end"],3)
                records.append((rp.Elf(output),record))
            chosen.write_text(chosen.read_text()+"/* selected header changed */\n")
            with self.assertRaises(provenance.MetadataProofError):
                provenance.validate_literal_proof(*records[0],root)
            self.assertEqual(provenance.validate_literal_proof(*records[1],root),3)

    def test_cpp_boundary_source_raw_and_parser_mutations_refuse(self):
        import runlink_pack as rp
        from types import SimpleNamespace
        import hashlib
        canonical = pathlib.Path(provenance.__file__).resolve().parent.parent
        for mutation in ("source","raw","parser"):
            with self.subTest(mutation=mutation),tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);(root/"src").mkdir();(root/"tools/ido").mkdir(parents=True)
                source=root/"src/strings.c";source.write_bytes(self.SOURCE)
                (root/"Makefile").write_text("fixture\n");(root/"tools/ido/cc").write_bytes(b"compiler")
                rawpath=root/"raw.o";rawpath.write_bytes(self.raw().data);raw=rp.Elf(rawpath)
                parser=root/"parser.py";parser.write_bytes(b"parser")
                def tool_inputs(_root):return {"parser.py":hashlib.sha256(parser.read_bytes()).hexdigest()}
                def cpp(*args,**kwargs):
                    path={"source":source,"raw":rawpath,"parser":parser}[mutation]
                    path.write_bytes(path.read_bytes()+b"changed")
                    return SimpleNamespace(returncode=0,stdout=self.SOURCE,stderr=b"")
                rows=[dict(name="first",const=False,literal="x"),dict(name="second",const=False,literal="yz")]
                with mock.patch.object(provenance,"literal_profile_inputs",side_effect=tool_inputs), \
                     mock.patch.object(provenance,"literal_array_facts",return_value=rows), \
                     mock.patch("permute_batch.source_dependencies",return_value={}), \
                     mock.patch.object(provenance.subprocess,"run",side_effect=cpp):
                    with self.assertRaises(provenance.MetadataProofError):
                        provenance.prove_literal_arrays(raw,source,root,
                          compiler_command="tools/ido/cc -c -nostdinc -o raw.o src/strings.c")


class StaticSourceExtentTests(unittest.TestCase):
    ROOT = pathlib.Path(provenance.__file__).resolve().parent.parent
    CPP = b'typedef long Base; typedef Base Counter; static Counter index=0; static Counter states[8];'

    def test_typedef_resolved_extent_is_independent_of_allocation(self):
        facts = provenance.static_source_facts(self.CPP, ['-32'], self.ROOT, {'index','states'})
        self.assertEqual([(r['name'],r['size']) for r in facts], [('index',4),('states',32)])
        short = provenance.static_source_facts(self.CPP.replace(b'[8]',b'[7]'), ['-32'], self.ROOT, {'states'})
        self.assertEqual(short[0]['size'],28)
        self.assertEqual(short[0]['count'],7)

    def test_missing_duplicate_nested_and_unsupported_types_fail_closed(self):
        cases = [b'typedef int Counter; static Counter index=0;',
                 b'typedef unsigned long Counter; static Counter index=0;',
                 b'static long *index=0;',b'static const long index=0;',
                 b'#pragma unknown_extent\nstatic long index=0;',
                 b'struct S { long x; }; static struct S index={0};',
                 b'static long index;',b'static long index=1;',
                 b'static long index[0];',b'static long index[];',
                 b'static long index[8][2];',b'static long index[8]={0};',
                 b'static long index[4+4];',b'static long index[99999999999];',
                 b'static long index=0; static long index=0;',
                 b'static long index=0; void f(void) { long index; }',
                 b'typedef long Counter; typedef long Counter; static Counter index=0;',
                 b'static long other=0;']
        for cpp in cases:
            with self.subTest(cpp=cpp), self.assertRaises(provenance.MetadataProofError):
                provenance.static_source_facts(cpp,['-32'],self.ROOT,{'index'})
        with self.assertRaises(provenance.MetadataProofError):
            provenance.static_source_facts(self.CPP,['-64'],self.ROOT,{'index'})

    def test_anchor_reader_preload_and_after_load_mutation_refuse(self):
        import shutil,subprocess
        for preload in (True,False):
            with self.subTest(preload=preload), tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);(root/'tools').mkdir()
                path=root/'tools/static_symbols.py';shutil.copy2(self.ROOT/'tools/static_symbols.py',path)
                script='\n'.join([
                    'import sys,pathlib',
                    'sys.path.insert(0,'+repr(str(self.ROOT/'tools'))+')',
                    'import proof_provenance as pp',
                    'root=pathlib.Path('+repr(str(root))+')',
                    'sys.path.insert(0,str(root/"tools"))',
                    'import static_symbols' if preload else 'pp.static_anchor_module(root)',
                    'path=root/"tools/static_symbols.py"; path.write_bytes(path.read_bytes()+b"\\n# changed\\n")',
                    'try: pp.static_anchor_module(root)',
                    'except pp.MetadataProofError as error: print(error)',
                    'else: raise SystemExit("accepted changed anchor code")'])
                result=subprocess.run([sys.executable,'-c',script],capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr+result.stdout)
                self.assertIn('preloaded' if preload else 'changed',result.stdout)

    def fixture(self, **options):
        import runlink_pack as rp
        ss = provenance.static_anchor_module(self.ROOT)
        from test_static_symbols import build_object
        settings=dict(statics=(),placeholder=False,bss=(('states',0),),bss_flags=3,bss_size=32,
                      data_anchors=(('index',0),),data_size=16)
        settings.update(options)
        compiler=rp.Elf('compiler',build_object(**settings))
        raw=rp.Elf('prepared',ss.add_static_symbols(compiler.data)[0])
        return compiler,raw

    def test_exact_anchor_only_preserves_zero_sizes_and_refuses_stale_observations(self):
        import runlink_pack as rp
        compiler,raw=self.fixture()
        facts=provenance.static_source_facts(self.CPP,['-32'],self.ROOT,{'index','states'})
        with tempfile.TemporaryDirectory(dir=self.ROOT/'build') as directory:
            source=pathlib.Path(directory)/'input.c';source.write_bytes(self.CPP)
            inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            witness=provenance.static_source_witness(compiler,raw,raw,facts,'source','cpp',inputs,self.ROOT,'tool')
            self.assertEqual(witness.exact_anchor(raw,raw,'.data',0).semantic_size,4)
            self.assertEqual(witness.exact_anchor(raw,raw,'.data',0).allocation,16)
            for offset in (4,8,12,15,16,28,32):
                with self.subTest(offset=offset), self.assertRaises(provenance.MetadataProofError):
                    witness.exact_anchor(raw,raw,'.data',offset)
            with self.assertRaises(provenance.MetadataProofError):
                witness.exact_anchor(rp.Elf('same-bytes-new-observation',raw.data),raw,'.data',0)
            for symbol in raw.symbols:
                if symbol.name in ('index','states'):self.assertEqual(symbol.size,0)
            source.write_bytes(self.CPP.replace(b'[8]',b'[7]'))
            with self.assertRaisesRegex(provenance.MetadataProofError,'stale'):
                witness.exact_anchor(raw,raw,'.data',0)
            shorter=provenance.static_source_facts(source.read_bytes(),['-32'],self.ROOT,{'index','states'})
            fresh_inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            fresh=provenance.static_source_witness(compiler,raw,raw,shorter,'source7','cpp7',fresh_inputs,self.ROOT,'tool')
            self.assertEqual((fresh.exact_anchor(raw,raw,'.bss',0).semantic_size,raw.size('.bss')),(28,32))

    def test_missing_competing_or_changed_linked_anchor_refuses(self):
        import runlink_pack as rp
        facts=provenance.static_source_facts(self.CPP,['-32'],self.ROOT,{'index','states'})
        for options in ({'data_anchors':()}, {'data_anchors':(('index',0),('extra',4))}, {'data_anchors':(('index',0),('extra',0))},
                        {'data_anchors':(('index',0),),'existing_data':(('other',4,0,'local'),)}):
            with self.subTest(options=options):
                compiler,raw=self.fixture(**options)
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.static_source_witness(compiler,raw,raw,facts,'source','cpp',(),self.ROOT,'tool')
        compiler,raw=self.fixture()
        linked=rp.Elf('linked-changed',raw.data)
        linked.symbols[next(i for i,symbol in enumerate(linked.symbols) if symbol.name=='index')].__dict__['size']=4
        with self.assertRaises(provenance.MetadataProofError):
            provenance.static_source_witness(compiler,raw,linked,facts,'source','cpp',(),self.ROOT,'tool')


class IntegerWordSourceTests(unittest.TestCase):
    ROOT = pathlib.Path(provenance.__file__).resolve().parent.parent
    CPP = b'''typedef long s32; typedef unsigned long u32;
long vertices[4]={1,2,3,4};
u32 pack(void) { union { const void *pointer; s32 bits; } address;
return ((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1))); }
'''
    ISOLATED = b'''typedef long s32; typedef unsigned long u32;
typedef struct Particle { float x; } Particle;
long vertices[4]={1,2,3,4};
u32 pack(Particle *pool) {
union { Particle *particle; const void *pointer; s32 bits; } address;
u32 word; address.particle=pool;
if(address.particle->x) word=0;
word=((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)));
address.particle=pool; word+=(u32)address.particle->x; return word; }
'''

    def isolated(self, cpp):
        return provenance.integer_word_facts(cpp, ['-32'], self.ROOT,
                                             profile='isolated-full-expression')

    def test_three_member_carrier_is_explicit_and_all_unrelated_uses_are_separate(self):
        with self.assertRaises(provenance.MetadataProofError):
            provenance.integer_word_facts(self.ISOLATED, ['-32'], self.ROOT)
        roles = self.isolated(self.ISOLATED)
        self.assertEqual([(r.function,r.target,r.carrier,r.occurrences) for r in roles],
                         [('pack','vertices','address',1)])
        for invalid in ('unknown', False, [], 1):
            with self.subTest(profile=invalid), self.assertRaises(provenance.MetadataProofError):
                provenance.integer_word_facts(self.ISOLATED,['-32'],self.ROOT,profile=invalid)

    def test_isolated_conversion_full_expression_roots_include_noncompound_bodies(self):
        head = self.ISOLATED.split(b'u32 pack')[0]
        union = b'union { Particle *particle; const void *pointer; s32 bits; } address;'
        expression = b'((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)))'
        statements = (b'u32 word; word=E; return word;', b'u32 word=E; return word;', b'return E;',
            b'if(E) return 1; return 0;', b'while(E) break; return 0;', b'do {} while(E); return 0;',
            b'switch(E){case 0:break;default:break;} return 0;',
            b'u32 word; for(word=E;0;) {} return 0;', b'for(;E;) break;return 0;',
            b'u32 word=1;for(;word;word=E) break;return 0;',
            b'u32 word;if(1) word=E;return 0;', b'u32 word;while(0) word=E;return 0;',
            b'u32 word;for(;0;) word=E;return 0;', b'u32 word;do word=E;while(0);return 0;',
            b'u32 word;label:word=E;return 0;',
            b'u32 word;switch(0){case 0:word=E;break;default:break;}return 0;',
            b'u32 word;switch(0){default:word=E;}return 0;')
        for statement in statements:
            with self.subTest(statement=statement):
                self.assertEqual(len(self.isolated(head+b'u32 pack(void){'+union+
                    statement.replace(b'E',expression)+b'}')),1)
        unsupported = head+b'u32 pack(void){'+union+b'switch(0){case '+expression+b':break;} return 0;}'
        with self.assertRaisesRegex(provenance.MetadataProofError,'full-expression'):
            self.isolated(unsupported)

    def test_full_expression_identity_sets_reject_other_arguments_and_second_conversions(self):
        expression = b'((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)))'
        for replacement in (b'combine('+expression+b', (u32)address.particle->x)',
                            b'combine('+expression+b','+expression+b')',
                            b'('+expression+b',address.pointer=0)',
                            b'combine(address.pointer=(const void *)vertices,(u32)(address.bits ^ (-2147483647-1)))'):
            changed = self.ISOLATED.replace(b'long vertices',b'u32 combine(u32,u32); long vertices')
            changed = changed.replace(expression,replacement)
            with self.subTest(replacement=replacement), self.assertRaises(provenance.MetadataProofError):
                self.isolated(changed)
        # The second carrier has different names, but its actual ID node still
        # shares the first conversion's full expression and must be rejected.
        changed=self.ISOLATED.replace(b'u32 word;',
            b'union { Particle *particle; const void *pointer; s32 bits; } other; u32 word;other.particle=pool;')
        changed=changed.replace(expression,b'combine('+expression+b',(u32)other.particle->x)')
        with self.assertRaisesRegex(provenance.MetadataProofError,'not isolated'):
            self.isolated(changed)

    def test_isolated_extra_pointer_types_escapes_shadows_and_uncovered_uses_refuse(self):
        changes = ((b'Particle *particle',b'void *particle'),
            (b'Particle *particle',b'Particle * const particle'),
            (b'Particle *particle',b'const Particle *particle'),
            (b'} address;',b'} address = {0};'),
            (b'} address;',b'} address; union { Particle *particle; const void *pointer; s32 bits; } address;'),
            (b'u32 word;',b'long vertices[4]; u32 word;'),
            (b'u32 word;',b'Particle **escape=&address.particle; u32 word;'),
            (b'u32 word;',b'void *escape=&address; u32 word;'),
            (b'u32 word;',b'u32 word; address.pointer=0;'),
            (b'address.particle=pool; word+=',b'address.particle=address.particle; word+='),
            (b'address.particle=pool; word+=',b'address.particle->x=0; word+='),
            (b'^',b'|'),(b'(const void *)vertices',b'(const void *)(vertices+1)'))
        for old,new in changes:
            with self.subTest(change=(old,new)), self.assertRaises(provenance.MetadataProofError):
                self.isolated(self.ISOLATED.replace(old,new))
        incomplete=self.ISOLATED.replace(b'typedef struct Particle { float x; } Particle;',
                                          b'typedef struct Particle Particle;')
        with self.assertRaises(provenance.MetadataProofError):self.isolated(incomplete)
        prototype_only = incomplete.replace(b'typedef struct Particle Particle;',
            b'typedef struct Particle Particle; void declared(struct Particle { float x; } *arg);')
        ca,parser,_pins,_lex = provenance._effective_parser(self.ROOT)
        parser.CParser().parse(prototype_only.decode())  # This is valid parser grammar.
        with self.assertRaisesRegex(provenance.MetadataProofError,'missing/ambiguous'):
            self.isolated(prototype_only)

    def test_extra_member_reads_cannot_hide_nested_aggregate_lvalues_or_addresses(self):
        for field,statement in ((b'float array[2];',b'address.particle->array[0]=1;'),
                                (b'float array[2];',b'float *escape=&address.particle->array[0];'),
                                (b'struct Inner {float x;} nested;',b'address.particle->nested.x=1;'),
                                (b'float *pointer;',b'*address.particle->pointer=1;')):
            source=self.ISOLATED.replace(b'float x; } Particle;',b'float x; '+field+b' } Particle;')
            source=source.replace(b'u32 word;',b'u32 word; address.particle=pool; '+statement)
            with self.subTest(statement=statement), self.assertRaises(provenance.MetadataProofError):
                self.isolated(source)

    def test_closed_expression_and_complete_carrier_target_coverage(self):
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        self.assertEqual([(r.function,r.target,r.carrier,r.occurrences) for r in roles],
                         [('pack','vertices','address',1)])
        # The role is an integer value, independently of a graphics sink.
        changed=self.CPP.replace(b'return ((',b'u32 word=((').replace(b')); }',b')); return word; }')
        self.assertEqual(provenance.integer_word_facts(changed,['-32'],self.ROOT),roles)
        for old,new in [(b'^',b'|'),(b'2147483647',b'2147483646'),
                        (b'(const void *)vertices',b'(const void *)(vertices+1)'),
                        (b's32 bits',b'u32 bits'),(b'return ((',b'address.pointer=0; return (('),
                        (b'return ((',b'long vertices[4]; return (('),
                        (b'const void *pointer',b'void *pointer'),
                        (b'(u32)(address.bits',b'(s32)(address.bits')]:
            with self.subTest(change=(old,new)), self.assertRaises(provenance.MetadataProofError):
                provenance.integer_word_facts(self.CPP.replace(old,new),['-32'],self.ROOT)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.integer_word_facts(self.CPP,['-64'],self.ROOT)

    def test_argument_list_is_not_a_comma_but_one_comma_argument_is(self):
        positive=self.CPP.replace(b'long vertices[4]',b'u32 consume(u32); long vertices[4]')
        positive=positive.replace(b'return ((',b'return consume(((').replace(b'))); }',b')))); }')
        self.assertEqual(len(provenance.integer_word_facts(positive,['-32'],self.ROOT)),1)
        negative=self.CPP.replace(b'long vertices[4]',b'u32 consume(const void *,u32); long vertices[4]')
        negative=negative.replace(b'return ((address.pointer=',b'return consume(address.pointer=')
        negative=negative.replace(b'(const void *)vertices),',b'(const void *)vertices,')
        with self.assertRaisesRegex(provenance.MetadataProofError,'uncovered'):
            provenance.integer_word_facts(negative,['-32'],self.ROOT)

    def fixture(self):
        import struct,runlink_pack as rp
        from runlink.synth import ElfBuilder,words,GLOBAL,OBJECT,FUNC
        def image(linked):
            text=words(0x3c028000,0,0x24420010 if linked else 0x24420000,0)
            builder=(ElfBuilder().section('.text',text).section('.data',bytes(48))
                     .sym('pack',0,16,GLOBAL,FUNC,'.text').sym('vertices',16,16,GLOBAL,OBJECT,'.data'))
            if not linked:builder.rel('.text',0,5,'vertices').rel('.text',8,6,'vertices')
            data=bytearray(builder.build());shoff=struct.unpack_from('>I',data,32)[0]
            struct.pack_into('>I',data,shoff+2*40+8,3)
            return rp.Elf('linked' if linked else 'raw',bytes(data))
        return image(False),image(True)

    def test_pair_witness_holds_metadata_inputs_and_zero_object_operand(self):
        import runlink_pack as rp
        raw,linked=self.fixture();roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        with tempfile.TemporaryDirectory(dir=self.ROOT/'build') as temporary:
            source=pathlib.Path(temporary)/'source.c';source.write_bytes(self.CPP)
            inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            witness=provenance.integer_word_witness(raw,raw,linked,roles,(), 'source','cpp',inputs,self.ROOT,'tools')
            high=raw.rels('.text')[0]
            self.assertEqual(witness.operand(raw,linked,high,0x80000000,False).target_offset,16)
            self.assertEqual(rp.edited_object_operand(rp.edited_global_target(rp.edited_prepare_object(raw,linked),'vertices'),0),('.data',16))
            for addend,exported in ((0,False),(0x80000004,False),(0x80000000,True)):
                with self.assertRaises(provenance.MetadataProofError):witness.operand(raw,linked,high,addend,exported)
            symbol=next(s for s in linked.symbols if s.name=='vertices');original=symbol.size
            object.__setattr__(symbol,'size',4)
            with self.assertRaises(provenance.MetadataProofError):witness.operand(raw,linked,high,0x80000000,False)
            object.__setattr__(symbol,'size',original)
            source.write_bytes(self.CPP+b'\n')
            with self.assertRaisesRegex(provenance.MetadataProofError,'stale'):
                witness.operand(raw,linked,high,0x80000000,False)

    def test_original_compiler_target_and_late_metadata_remain_bound(self):
        import runlink_pack as rp
        raw,linked=self.fixture();compiler=rp.Elf('compiler',raw.data)
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        witness=provenance.integer_word_witness(compiler,raw,linked,roles,(),'source','cpp',(),self.ROOT,'tools')
        original=next(s for s in compiler.symbols if s.name=='vertices')
        object.__setattr__(original,'value',20)
        with self.assertRaisesRegex(provenance.MetadataProofError,'observations'):
            witness.operand(raw,linked,raw.rels('.text')[0],0x80000000,False)
        object.__setattr__(original,'value',16)
        for image in (raw,linked):
            object.__setattr__(next(s for s in image.symbols if s.name=='vertices'),'name','other')
        wrong=provenance.integer_word_facts(self.CPP.replace(b'vertices',b'other'),['-32'],self.ROOT)
        with self.assertRaisesRegex(provenance.MetadataProofError,'original compiler target'):
            provenance.integer_word_witness(compiler,raw,linked,wrong,(),'source','cpp',(),self.ROOT,'tools')

    def test_original_compiler_function_name_and_size_join(self):
        import runlink_pack as rp
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        for field,value in (('name','other_function'),('size',12)):
            raw,linked=self.fixture();compiler=rp.Elf('compiler',raw.data)
            function=next(s for s in compiler.symbols if s.name=='pack')
            object.__setattr__(function,field,value)
            with self.subTest(field=field), self.assertRaises((rp.PackError,provenance.MetadataProofError)):
                provenance.integer_word_witness(compiler,raw,linked,roles,(),'source','cpp',(),self.ROOT,'tools')

    def test_missing_unauthorized_pair_section_type_and_export_refuse(self):
        import runlink_pack as rp
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        for mutation in ('export','type','section','offset','register','low-type','wrong-name','extra-role'):
            raw,linked=self.fixture();exports=()
            target=next(s for s in raw.symbols if s.name=='vertices')
            if mutation=='export':exports=('vertices',)
            if mutation=='type':object.__setattr__(target,'type',rp.STT_FUNC)
            if mutation=='section':object.__setattr__(target,'section','.text')
            if mutation=='offset':object.__setattr__(target,'value',48)
            if mutation=='wrong-name':object.__setattr__(target,'name','other')
            if mutation in ('register','low-type'):
                data=bytearray(raw.data)
                import struct
                if mutation=='register':struct.pack_into('>I',data,raw.section('.text')['offset']+8,0x24620000)
                else:
                    rel=raw.section('.rel.text');info=struct.unpack_from('>I',data,rel['offset']+12)[0]
                    struct.pack_into('>I',data,rel['offset']+12,(info&~255)|2)
                raw=rp.Elf('mutated',bytes(data))
            use=roles+(provenance.IntegerWordRole('pack','unknown','address',1),) if mutation=='extra-role' else roles
            with self.subTest(mutation=mutation), self.assertRaises((rp.PackError,provenance.MetadataProofError)):
                provenance.integer_word_witness(raw,raw,linked,use,exports,'source','cpp',(),self.ROOT,'tools')

class ResidentPointerCellSourceTests(unittest.TestCase):
    ROOT = pathlib.Path(__file__).resolve().parent.parent

    def facts(self, text, definition=True, flags=('-32',)):
        return provenance.resident_cell_declarations(text.encode(), flags, self.ROOT, {'cell'}, definition=definition)

    def test_actual_pointer_type_with_header_extern_and_tentative_definition(self):
        text='typedef unsigned short u16; extern u16 *cell; u16 *cell;'
        self.assertEqual(self.facts(text), ('cell',))
        self.assertEqual(self.facts('typedef unsigned short u16; extern u16 *cell;',False), ('cell',))

    def test_source_size_or_a_name_does_not_supply_object_authority(self):
        for text in ('unsigned short cell[2];','unsigned long cell;','void *cell;',
                     'unsigned short *cell=0;','static unsigned short *cell;',
                     'unsigned short *cell; unsigned short *cell;',
                     'extern unsigned short *cell;', 'const unsigned short *cell;',
                     'unsigned short *cell; void f(void){unsigned short *cell;}',
                     '#pragma GLOBAL_ASM("proxy.s")\nunsigned short *cell;'):
            with self.subTest(text=text), self.assertRaises(provenance.MetadataProofError):
                self.facts(text)
        with self.assertRaises(provenance.MetadataProofError):
            self.facts('unsigned short *cell;',flags=('-n32',))
        with self.assertRaises(provenance.MetadataProofError):
            self.facts('unsigned short *cell;',False)



class EncodedAddressSourceTests(unittest.TestCase):
    ROOT = pathlib.Path(__file__).resolve().parents[1]
    SOURCE = b'''typedef unsigned long u32;
typedef struct Overlay73Command { u32 w0; u32 w1; } Overlay73Command;
extern unsigned char o73TriangleCommandWord[];
void func_overlay_073_F0000D70_18CB830(Overlay73Command **commands) {
 { Overlay73Command *_g=(Overlay73Command *)((*commands)++);
   _g->w0=1; _g->w1=(unsigned int)o73TriangleCommandWord; }
}'''
    OWNER = b'''typedef unsigned char u8; typedef short s16;
typedef struct Overlay73Triangle {u8 flags,vertex0,vertex1,vertex2; s16 s0,t0,s1,t1,s2,t2;} Overlay73Triangle;
Overlay73Triangle o73Triangles[8]={{0}};'''

    def facts(self, source=None, flags=None):
        return provenance.encoded_word_consumer_facts(source or self.SOURCE, flags or ['-32'], self.ROOT)

    def test_actual_distinct_unsigned_types_and_owner_schema(self):
        facts=self.facts()
        self.assertEqual(facts[-2:],(('int','unsigned'),('long','unsigned')))
        self.assertEqual(provenance.encoded_word_owner_facts(self.OWNER,['-32'],self.ROOT),128)
        self.assertEqual(self.facts(self.SOURCE.replace(b'(unsigned int)',b'(u32)'))[-2],('long','unsigned'))

    def test_complete_carrier_and_receiver_use_closure(self):
        variants=[
            self.SOURCE.replace(b'extern unsigned char',b'unsigned char').replace(b'[];',b'[4];'),
            self.SOURCE.replace(b'[];',b'[4];'),
            self.SOURCE.replace(b'unsigned char',b'char'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)&o73TriangleCommandWord'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)o73TriangleCommandWord[0]'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)(o73TriangleCommandWord+1)'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)(void *)o73TriangleCommandWord'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)sizeof(o73TriangleCommandWord)'),
            self.SOURCE.replace(b'(unsigned int)o73TriangleCommandWord',b'(unsigned int)o73TriangleCommandWord + 1'),
            self.SOURCE.replace(b'_g->w1=',b'_g->w0='),
            self.SOURCE.replace(b'u32 w1',b'long w1'),
            self.SOURCE.replace(b'(unsigned int)',b'(unsigned short)'),
            self.SOURCE.replace(b'void func_',b'static void func_'),
            self.SOURCE.replace(b'_g->w0=1;',b'unsigned char o73TriangleCommandWord[1]; _g->w0=1;'),
            self.SOURCE.replace(b'_g->w0=1;',b'_g->w0=(unsigned int)o73TriangleCommandWord;'),
            self.SOURCE.replace(b'_g->w0=1;',b'escape(_g); _g->w0=1;'),
            self.SOURCE.replace(b'_g->w0=1;',b'{ Overlay73Command *_g; } _g->w0=1;'),
            self.SOURCE.replace(b'_g->w1=(unsigned int)o73TriangleCommandWord;',b'call(_g->w1=(unsigned int)o73TriangleCommandWord,1);'),
            self.SOURCE.replace(b'_g->w0=1;',b'_g->w0=1; _g->w1=0;'),
            self.SOURCE.replace(b'_g->w0=1;',b'_g->w0+=1;'),
            self.SOURCE.replace(b'_g->w0=1;',b'_g->w0=1; unrelated();'),
        ]
        for source in variants:
            with self.subTest(source=source),self.assertRaises(provenance.MetadataProofError): self.facts(source)
        for flags in (['-64'],['-32','-n32']):
            with self.assertRaises(provenance.MetadataProofError): self.facts(flags=flags)

    def test_owner_source_is_the_initialized_writable_record_array(self):
        for source in (self.OWNER.replace(b'[8]',b'[9]'), self.OWNER.replace(b'={{0}}',b''),
                       self.OWNER.replace(b'Overlay73Triangle o73Triangles',b'const Overlay73Triangle o73Triangles'),
                       self.OWNER.replace(b'typedef short s16',b'typedef int s16'),
                       self.OWNER.replace(b'u8 flags',b'u8 other'),
                       self.OWNER.replace(b'Overlay73Triangle o73Triangles',b'static Overlay73Triangle o73Triangles')):
            with self.subTest(source=source),self.assertRaises(provenance.MetadataProofError):
                provenance.encoded_word_owner_facts(source,['-32'],self.ROOT)

    def test_expression_rejects_baseline_numeric_coincidence_and_overrides(self):
        good={'aliases':provenance.encoded_word_expression(),'main':'overlay_073_DATA_START = .;'}
        self.assertEqual(provenance.encoded_word_scripts(good),'aliases')
        for scripts in ({'aliases':'o73TriangleCommandWord = 0x80000000;','main':good['main']},
            dict(good,late=good['aliases']), dict(good,late='o73Triangles = 0;'),
            dict(good,late='o73Triangles <<= 0;'), dict(good,late='overlay_073_DATA_START += 0;'),
            dict(good,main='overlay_073_DATA_START = 0;'),dict(good,main=good['main']+' INCLUDE extra'),
            dict(good,aliases=good['aliases'].replace('0x80000000','0x40000000')),
            dict(good,aliases=good['aliases'].replace('o73Triangles -','o73VertexSeeds -')),
            dict(good,aliases=good['aliases']+' o73TriangleCommandWord = 0;')):
            with self.subTest(scripts=scripts),self.assertRaises(provenance.MetadataProofError):
                provenance.encoded_word_scripts(scripts)

    def test_optin_is_closed_and_missing_optin_does_not_hide_the_carrier(self):
        with tempfile.TemporaryDirectory() as td:
            root=pathlib.Path(td); (root/'Makefile').touch()
            source=root/provenance.ENCODED_WORD_SOURCE; source.parent.mkdir(parents=True);source.write_bytes(self.SOURCE)
            good=provenance.ENCODED_WORD_TARGET+': ENCODED_ADDRESS_WORD = '+provenance.ENCODED_WORD_ASSOCIATION+'\n'
            self.assertTrue(provenance.encoded_word_selection(root,good))
            self.assertTrue(provenance.encoded_word_selection(root,good*3))
            for dump in ('',good.replace('o73Triangles:','other:'),good.replace('o073/','o013/'),
                         good+good.replace('o73Triangles:','other:'),good+good.replace('o073/','o013/'),
                         good+good.replace(' = ', ' := '),good+'ENCODED_ADDRESS_WORD = unknown\n',
                         good+good.replace(': ENCODED_ADDRESS_WORD', ': private ENCODED_ADDRESS_WORD')):
                with self.subTest(dump=dump),self.assertRaises(provenance.MetadataProofError):
                    provenance.encoded_word_selection(root,dump)
            source.write_text('void unrelated(void) {}')
            with self.assertRaises(provenance.MetadataProofError): provenance.encoded_word_selection(root,good)
            self.assertFalse(provenance.encoded_word_selection(root,''))


class EncodedAddressObjectTests(unittest.TestCase):
    @staticmethod
    def objects(origin=16, owner_delta=0):
        import runlink_pack as rp
        from test_runlink_pack import ObjBuilder, GLOBAL, OBJECT, FUNC
        marker=provenance.ENCODED_WORD_MARKER; owner=provenance.ENCODED_WORD_OWNER; function=provenance.ENCODED_WORD_FUNCTION
        words=struct.pack('>IIII',15<<26|8<<16,9<<26|8<<21|8<<16,0,0)
        raw=rp.Elf(pathlib.Path('raw.o'),ObjBuilder().section('.text',words)
            .sym(function,0,16,GLOBAL,FUNC,'.text').sym(marker,0,0,GLOBAL,OBJECT,'UND')
            .rel('.text',0,rp.R_MIPS_HI16,marker).rel('.text',4,rp.R_MIPS_LO16,marker).build())
        data=ObjBuilder().section('.data',bytes(208)).sym(owner,0,128,GLOBAL,OBJECT,'.data')
        data.sym('o73VertexSeeds',128,72,GLOBAL,OBJECT,'.data')
        blob=bytearray(data.build()); shoff=struct.unpack_from('>I',blob,32)[0];struct.pack_into('>I',blob,shoff+40+8,3)
        provider=rp.Elf(pathlib.Path('owner.o'),bytes(blob))
        # Independent origin movement changes the text allocation, not an invented marker object.
        if origin!=16:
            larger=words+bytes(origin-16)
            raw=rp.Elf(pathlib.Path('raw.o'),ObjBuilder().section('.text',larger)
                .sym(function,0,16,GLOBAL,FUNC,'.text').sym(marker,0,0,GLOBAL,OBJECT,'UND')
                .rel('.text',0,rp.R_MIPS_HI16,marker).rel('.text',4,rp.R_MIPS_LO16,marker).build())
        value=(owner_delta+0x80000000)&0xffffffff
        linked_words=bytearray(raw.bytes_of('.text'))
        struct.pack_into('>I',linked_words,0,(15<<26|8<<16)|((value+0x8000)>>16)&0xffff)
        struct.pack_into('>I',linked_words,4,(9<<26|8<<21|8<<16)|(value&0xffff))
        image=bytes(linked_words)+bytes(owner_delta)+provider.bytes_of('.data')
        final=rp.Elf(pathlib.Path('final.elf'),ObjBuilder().section('.overlay_073',image,addr=0xF0000000)
            .sym(function,0xF0000000,16,GLOBAL,FUNC,'.overlay_073')
            .sym(owner,0xF0000000+origin+owner_delta,128,GLOBAL,OBJECT,'.overlay_073')
            .sym(provenance.ENCODED_WORD_ORIGIN,0xF0000000+origin,0,GLOBAL,0,'.overlay_073')
            .sym(marker,value,0,GLOBAL,OBJECT,'ABS').build())
        mapping=[rp.MapInput('.overlay_073','.text',0xF0000000,origin,provenance.ENCODED_WORD_TARGET),
                 rp.MapInput('.overlay_073','.data',0xF0000000+origin+owner_delta,208,'build/'+provenance.ENCODED_WORD_PROVIDER+'.o')]
        return raw,provider,final,mapping

    def test_original_pair_and_independent_origin_owner_carry_readback(self):
        for origin,delta in ((16,0),(32,0),(32,32),(48,0x8000)):
            with self.subTest(origin=origin,delta=delta):
                raw,provider,final,mapping=self.objects(origin,delta)
                pair,owner,start=provenance.encoded_word_readback(raw,provider,final,mapping,128)
                self.assertEqual((owner,start),(origin+delta,origin))
                self.assertEqual(pair[1:3],(0,4))

    def test_actual_final_roles_and_original_pair_mutations_refuse(self):
        import runlink_pack as rp
        cases=[('provider',provenance.ENCODED_WORD_OWNER,{'size':0}),
               ('provider',provenance.ENCODED_WORD_OWNER,{'bind':rp.STB_LOCAL}),
               ('provider',provenance.ENCODED_WORD_OWNER,{'shndx':rp.SHN_COMMON,'section':'COMMON'}),
               ('final',provenance.ENCODED_WORD_OWNER,{'shndx':rp.SHN_ABS,'section':'ABS'}),
               ('final',provenance.ENCODED_WORD_MARKER,{'size':4}),
               ('final',provenance.ENCODED_WORD_MARKER,{'type':rp.STT_FUNC}),
               ('final',provenance.ENCODED_WORD_MARKER,{'value':0}),
               ('final',provenance.ENCODED_WORD_ORIGIN,{'value':0}),
               ('raw',provenance.ENCODED_WORD_MARKER,{'size':4}),
               ('raw',provenance.ENCODED_WORD_FUNCTION,{'bind':rp.STB_LOCAL})]
        for which,name,change in cases:
            raw,provider,final,mapping=self.objects();obj={'raw':raw,'provider':provider,'final':final}[which]
            i=next(i for i,s in enumerate(obj.symbols) if s.name==name)
            obj.symbols[i]=dataclasses.replace(obj.symbols[i],**change)
            with self.subTest(which=which,change=change),self.assertRaises(provenance.MetadataProofError):
                provenance.encoded_word_readback(raw,provider,final,mapping,128)
        raw,provider,final,mapping=self.objects();mapping.append(mapping[-1])
        with self.assertRaises(provenance.MetadataProofError): provenance.encoded_word_readback(raw,provider,final,mapping,128)
        raw,provider,final,mapping=self.objects(); i=next(i for i,s in enumerate(provider.symbols) if s.name=='o73VertexSeeds')
        provider.symbols[i]=dataclasses.replace(provider.symbols[i],value=64)
        with self.assertRaises(provenance.MetadataProofError): provenance.encoded_word_readback(raw,provider,final,mapping,128)


class EncodedAddressOrdinaryReplayTests(unittest.TestCase):
    def setUp(self):
        import runlink_pack as rp
        import permute_batch as batch
        import runlink_metadata_link as link
        import shlex
        from test_runlink_pack import ObjBuilder, GLOBAL, OBJECT
        self.rp,self.batch,self.link=rp,batch,link
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=pathlib.Path(self.temp.name)
        (self.root/'tools').symlink_to(EncodedAddressSourceTests.ROOT/'tools',target_is_directory=True)
        if not (self.root/'tools/binutils/mips64-elf-ld').is_file():self.skipTest('configured GNU ld absent')
        self.raw,self.provider,_unused,_mapping=EncodedAddressObjectTests.objects()
        def write(name,data):
            path=self.root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data);return path
        self.write=write
        write(provenance.ENCODED_WORD_TARGET,self.raw.data)
        write('build/'+provenance.ENCODED_WORD_PROVIDER+'.o',self.provider.data)
        self.extra=write('build/other.o',ObjBuilder().section('.data',bytes(4))
            .sym('other',0,4,GLOBAL,OBJECT,'.data').build())
        self.main='SECTIONS\n{\n .overlay_073 0xF0000000 : AT(0) SUBALIGN(4)\n {\n  build/src/overlays/o073/overlay_073.c.o(.text);\n  overlay_073_DATA_START = .;\n  build/src/overlays/o073/overlay73Geometry.c.o(.data);\n }\n .extra 0xF0010000 : AT(4096) SUBALIGN(4)\n {\n  build/other.o(.data);\n }\n /DISCARD/ :\n {\n  *(*);\n }\n}\n'
        self.script_names=['mickey.us.ld','undefined_funcs_auto.us.txt','undefined_syms_auto.us.txt',
                           'libultra_undefined_syms.us.txt','overlay_undefined_syms.us.txt']
        for name in self.script_names:write(name,b'')
        (self.root/'mickey.us.ld').write_text(self.main)
        (self.root/'overlay_undefined_syms.us.txt').write_text(provenance.encoded_word_expression()+'\n')
        self.linked=self.root/link.BOOT_ELF;self.map_path=self.root/link.BOOT_MAP
        self.linked.parent.mkdir(parents=True,exist_ok=True)
        self.command=['tools/link_diagnose.sh','tools/binutils/mips64-elf-ld']
        for name in self.script_names:self.command+=['-T',name]
        self.command+=['-Map',link.BOOT_MAP,'--no-check-sections','-o',link.BOOT_ELF]
        (self.root/link.BOOT_LOG).write_text('command: '+shlex.join(self.command)+'\n')
        self.baseline()

    def baseline(self):
        self.batch.bounded_capture(self.command,provenance.time.monotonic()+20,check=True,cwd=self.root)

    def replay(self):
        scripts={name:(self.root/name).read_text() for name in self.script_names}
        context=provenance.producer_link_context(self.root,self.linked,self.map_path)
        return provenance.replay_encoded_word_link(self.root,self.linked,self.map_path,scripts,context)

    def test_actual_bound_ld_replay_runs_preserves_canonical_outputs_and_holds_private(self):
        before={p:p.read_bytes() for p in (self.linked,self.map_path)}
        run=self.batch.bounded_capture
        with mock.patch.object(self.batch,'bounded_capture',wraps=run) as invoked:
            image,mapping,rows,held,current=self.replay()
        self.assertEqual(invoked.call_count,1)
        actual=invoked.call_args.args[0];expected=self.command.copy()
        expected[expected.index('-Map')+1]=str(mapping)
        expected[expected.index('-o')+1]=str(image.path)
        self.assertEqual(actual,expected)
        self.assertEqual(invoked.call_args.kwargs['cwd'],self.root)
        self.assertNotEqual(image.path,self.linked)
        self.assertEqual(before,{p:p.read_bytes() for p in before})
        self.assertEqual(tuple(self.rp.parse_map(self.map_path)),rows);self.assertTrue(current())
        image.path.write_bytes(image.path.read_bytes()+b'changed')
        with self.assertRaises(provenance.MetadataProofError):current()

    def test_actual_final_command_uses_its_bound_script_and_private_outputs(self):
        import shlex
        self.script_names[0]=self.link.FINAL_SCRIPT
        self.write(self.link.FINAL_SCRIPT,self.main.encode())
        self.linked=self.root/'build/mickey.us.elf';self.map_path=self.root/'build/mickey.us.map'
        self.command[3]=self.link.FINAL_SCRIPT
        self.command[self.command.index('-Map')+1]='build/mickey.us.map'
        self.command[self.command.index('-o')+1]='build/mickey.us.elf'
        (self.root/self.link.FINAL_LOG).write_text('command: '+shlex.join(self.command)+'\n')
        self.baseline()
        run=self.batch.bounded_capture
        with mock.patch.object(self.batch,'bounded_capture',wraps=run) as invoked:
            image,mapping,_rows,_held,current=self.replay()
        self.assertEqual(invoked.call_count,1)
        self.assertEqual(invoked.call_args.args[0][3],self.link.FINAL_SCRIPT)
        self.assertNotEqual(image.path,self.linked);self.assertNotEqual(mapping,self.map_path)
        self.assertTrue(current())

    def test_zero_allocation_direct_input_remains_held_during_and_after_replay(self):
        from test_runlink_pack import ObjBuilder
        hidden=self.write('build/empty.o',ObjBuilder().section('.text',b'').build())
        (self.root/'mickey.us.ld').write_text(self.main.replace('  build/other.o(.data);',
            '  build/other.o(.data);\n  build/empty.o(.text);'))
        self.baseline()
        original=hidden.read_bytes()
        _image,_mapping,_rows,held,current=self.replay()
        self.assertIn(hidden,held)
        hidden.write_bytes(original+b'changed')
        with self.assertRaises(provenance.MetadataProofError):current()
        hidden.write_bytes(original)
        run=self.batch.bounded_capture
        def changed(*args,**kwargs):
            result=run(*args,**kwargs);hidden.write_bytes(original+b'changed during link');return result
        with mock.patch.object(self.batch,'bounded_capture',side_effect=changed):
            with self.assertRaisesRegex(provenance.MetadataProofError,'closure changed'):self.replay()

    def test_numeric_baseline_then_correct_script_still_requires_actual_reproduction(self):
        alias=self.root/'overlay_undefined_syms.us.txt'
        alias.write_text('o73TriangleCommandWord = 0x80000000;\n');self.baseline()
        alias.write_text(provenance.encoded_word_expression()+'\n')
        # All old local role/readback checks still pass; only an unrelated real
        # direct input changed since that numerically coincident old link.
        provenance.encoded_word_readback(self.raw,self.provider,self.rp.Elf(self.linked),
                                        self.rp.parse_map(self.map_path),128)
        obj=self.rp.Elf(self.extra);blob=bytearray(obj.data)
        blob[obj.section('.data')['offset']]=1;self.extra.write_bytes(blob)
        run=self.batch.bounded_capture
        with mock.patch.object(self.batch,'bounded_capture',wraps=run) as invoked:
            with self.assertRaisesRegex(provenance.MetadataProofError,'replay differs'):self.replay()
        self.assertEqual(invoked.call_count,1)

    def test_substituted_canonical_elf_cannot_hide_mismatching_reproduction(self):
        obj=self.rp.Elf(self.extra);data=bytearray(obj.data)
        data[obj.section('.data')['offset']]=1;self.extra.write_bytes(data)
        context=provenance.producer_link_context(self.root,self.linked,self.map_path)
        actual_elf=self.rp.Elf;seen=[]
        def substitute(path,*args,**kwargs):
            if pathlib.Path(path)==self.linked:
                outputs=list((self.root/'build/encoded-word-link-proof').glob('capture-*/linked.elf'))
                self.assertEqual(len(outputs),1);seen.append(path)
                return actual_elf(path,outputs[0].read_bytes())
            return actual_elf(path,*args,**kwargs)
        run=self.batch.bounded_capture
        with mock.patch.object(self.rp,'Elf',side_effect=substitute), \
                mock.patch.object(self.batch,'bounded_capture',wraps=run) as invoked:
            with self.assertRaisesRegex(provenance.MetadataProofError,'loaded canonical ELF'):
                self.replay()
        self.assertEqual(len(seen),1);self.assertEqual(invoked.call_count,1)
        self.assertEqual(context,provenance.producer_link_context(self.root,self.linked,self.map_path))

    def test_substituted_canonical_map_bytes_cannot_hide_wrong_physical_input(self):
        good=self.map_path.read_bytes()
        bad=good.replace(b'build/other.o',b'build/unlisted.o')
        self.assertNotEqual(good,bad);self.map_path.write_bytes(bad)
        context=provenance.producer_link_context(self.root,self.linked,self.map_path)
        read=pathlib.Path.read_bytes;seen=[]
        def substitute(path):
            if path==self.map_path:seen.append(path);return good
            return read(path)
        with mock.patch.object(pathlib.Path,'read_bytes',substitute), \
                mock.patch.object(self.batch,'bounded_capture') as invoked:
            with self.assertRaisesRegex(provenance.MetadataProofError,'loaded canonical map'):
                self.replay()
            invoked.assert_not_called()
        self.assertEqual(len(seen),1);self.assertEqual(self.map_path.read_bytes(),bad)
        self.assertEqual(context,provenance.producer_link_context(self.root,self.linked,self.map_path))

    def test_loaded_replay_bytes_are_held_from_the_observations_not_later_file_hashes(self):
        actual_elf=self.rp.Elf;read=pathlib.Path.read_bytes
        for case in ('elf','map'):
            seen=[]
            def substitute_elf(path,*args,**kwargs):
                value=actual_elf(path,*args,**kwargs)
                if case=='elf' and pathlib.Path(path).parent.parent.name=='encoded-word-link-proof':
                    value.data+=b' substituted loaded trailing bytes';seen.append(path)
                return value
            def substitute_map(path):
                value=read(path)
                if case=='map' and path.name=='linked.map' and path.parent.parent.name=='encoded-word-link-proof':
                    seen.append(path);return value+b'\n# substituted loaded map bytes\n'
                return value
            with self.subTest(case=case),mock.patch.object(self.rp,'Elf',side_effect=substitute_elf), \
                    mock.patch.object(pathlib.Path,'read_bytes',substitute_map):
                with self.assertRaisesRegex(provenance.MetadataProofError,'closure changed'):self.replay()
            self.assertEqual(len(seen),1)

    def test_zero_sized_collision_inputs_and_unsupported_spellings_refuse(self):
        from test_runlink_pack import ObjBuilder, GLOBAL, OBJECT
        for name in (provenance.ENCODED_WORD_MARKER,provenance.ENCODED_WORD_ORIGIN,provenance.ENCODED_WORD_OWNER):
            self.write('build/hidden.o',ObjBuilder().section('.text',b'').sym(name,0,0,GLOBAL,OBJECT,'ABS').build())
            (self.root/'mickey.us.ld').write_text(self.main.replace('  build/other.o(.data);','  build/other.o(.data);\n  build/hidden.o(.text);'))
            with self.subTest(name=name),mock.patch.object(self.batch,'bounded_capture') as invoked:
                with self.assertRaises(provenance.MetadataProofError):self.replay()
                invoked.assert_not_called()
        for spelling in ('"build/hidden.o"(.text);','extra/alias.o(.text);','./build/hidden.o(.text);',
                         'build/hidden.o','INPUT(build/hidden.o)','STARTUP(build/hidden.o)',
                         'build/hidden.a(.text);','unknown(build/hidden.o)'):
            (self.root/'mickey.us.ld').write_text(self.main.replace('  build/other.o(.data);','  build/other.o(.data);\n  '+spelling))
            with self.subTest(spelling=spelling),mock.patch.object(self.batch,'bounded_capture') as invoked:
                with self.assertRaises(provenance.MetadataProofError):self.replay()
                invoked.assert_not_called()

    def test_current_canonical_scripts_have_a_complete_explicit_grammar(self):
        root=EncodedAddressSourceTests.ROOT
        scripts={name:(root/name).read_text() for name in self.script_names}
        names=provenance.encoded_word_link_inputs(scripts)
        self.assertIn(provenance.ENCODED_WORD_TARGET,names)
        self.assertIn('build/'+provenance.ENCODED_WORD_PROVIDER+'.o',names)


class EncodedAddressCaptureTests(unittest.TestCase):
    def setUp(self):
        provenance._effective_parser(EncodedAddressSourceTests.ROOT)
        provenance.static_anchor_module(EncodedAddressSourceTests.ROOT)
        import runlink_pack as rp
        import postprocess_audit as audit
        import permute_batch as batch
        self.rp=rp
        self.actual_replay=False
        self.real_replay=provenance.replay_encoded_word_link
        self.real_link_context=provenance.producer_link_context
        self.real_bounded_capture=batch.bounded_capture
        self.real_ort_load=rp.OrtNames.load
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=pathlib.Path(self.temp.name)
        (self.root/'tools').symlink_to(EncodedAddressSourceTests.ROOT/'tools',target_is_directory=True)
        raw,provider,final,mapping=EncodedAddressObjectTests.objects()
        def write(name,data):
            p=self.root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data);return p
        write('Makefile',b'fixture')
        self.source=write(provenance.ENCODED_WORD_SOURCE,EncodedAddressSourceTests.SOURCE)
        self.owner_source=write(provenance.ENCODED_WORD_PROVIDER,EncodedAddressSourceTests.OWNER)
        self.original=write('build/capture/compiler-original.o',raw.data)
        self.raw=write('build/capture/raw.o',raw.data)
        configured=write(provenance.ENCODED_WORD_TARGET,raw.data)
        self.ordinary=write('build/raw/'+provenance.ENCODED_WORD_SOURCE+'.o',raw.data)
        provider_path=write('build/'+provenance.ENCODED_WORD_PROVIDER+'.o',provider.data)
        final_path=write('build/mickey.us.elf',final.data)
        map_text='Linker script and memory map\n.overlay_073 0xf0000000 0xe0\n'+''.join(
            f' {m.insec} {m.addr:#x} {m.size:#x} {m.file}\n' for m in mapping)
        self.map_path=write('build/mickey.us.map',map_text.encode())
        scripts=['mickey.us.ld','undefined_funcs_auto.us.txt','undefined_syms_auto.us.txt',
                 'libultra_undefined_syms.us.txt','overlay_undefined_syms.us.txt']
        for name in scripts:write(name,b'')
        (self.root/'mickey.us.ld').write_text('overlay_073_DATA_START = .;')
        self.script=self.root/'overlay_undefined_syms.us.txt';self.script.write_text(provenance.encoded_word_expression())
        self.dump=(provenance.ENCODED_WORD_TARGET+': ENCODED_ADDRESS_WORD = '+provenance.ENCODED_WORD_ASSOCIATION+'\n'
            +'build/'+provenance.ENCODED_WORD_PROVIDER+'.o: PLAIN_DATA_PROOF = 1\n'
            +'build/'+provenance.ENCODED_WORD_PROVIDER+'.o: PLAIN_DATA_RUNTIME_ROLE = data\n')
        self.world=SimpleNamespace(paths={'build':self.root/'build','raw':self.root/'build/raw','elf':final_path,
              'map':self.map_path,'ort':self.root/'ort'},elf=rp.Elf(final_path),map=mapping,map_data=map_text.encode(),
              data_import_inputs={},data_import_contexts=[],data_import_provider_receipts={},need_raw=False,
              layout=None,_guarded_data_objects={})
        def inputs(source,target):
            return {'source':provenance.sha256_file(source),'configured':provenance.sha256_file(target),
                'assembly':{},'command':'tools/ido/cc -32 -c -o '+str(target.relative_to(self.root))+' '+str(source.relative_to(self.root)),
                'dependencies':{},'makefiles':{},'module_link_inputs':{},'metadata_tools':{},
                'generated_metadata':{'inputs':{}}}
        self.consumer_inputs=lambda:inputs(self.source,configured)
        self.provider_inputs=lambda:inputs(self.owner_source,provider_path)
        self.receipt={'inputs':self.consumer_inputs(),'raw_sha256':provenance.sha256_file(self.raw),
            'configured_sha256':provenance.sha256_file(configured),'compiler_original':'build/capture/compiler-original.o',
            'compiler_original_sha256':provenance.sha256_file(self.original),'kept_rodata':{'fixture':True},'directory':'build/capture'}
        self.provider_receipt={'inputs':self.provider_inputs(),'directory':'build/provider',
            'compiler_original':str(provider_path.relative_to(self.root)),
            'compiler_original_sha256':provenance.sha256_file(provider_path),
            'raw_sha256':provenance.sha256_file(provider_path),'configured_sha256':provenance.sha256_file(provider_path)}
        (self.root/'build/provider').mkdir()
        def provider_capture(world,source,**options):
            world.data_import_provider_receipts[source]=self.provider_receipt
            rp.hold_data_capture(world,source,self.provider_receipt,self.provider_inputs)
            return rp.Elf(provider_path)
        def replay(root,linked,map_path,scripts,context):
            if self.actual_replay:
                return self.real_replay(root,linked,map_path,scripts,context)
            image_path=write('build/replay/linked.elf',linked.read_bytes())
            replay_map=write('build/replay/linked.map',map_path.read_bytes())
            image=rp.Elf(image_path); rows=tuple(rp.parse_map(replay_map))
            held={p:provenance.sha256_file(p) for p in (image_path,replay_map)}
            def current():
                if any(provenance.sha256_file(p)!=h for p,h in held.items()):
                    raise provenance.MetadataProofError('fixture replay changed')
                return True
            return image,replay_map,rows,held,current
        def cpp(command,*args,**kwargs):
            if command[0]!='tools/ido/cc':
                return self.real_bounded_capture(command,*args,**kwargs)
            return SimpleNamespace(stdout=(self.source if command[-1]==provenance.ENCODED_WORD_SOURCE else self.owner_source).read_text())
        self.stack=contextlib.ExitStack();self.addCleanup(self.stack.close)
        for patch in (mock.patch.object(audit,'run_make_database',side_effect=lambda:self.dump),
            mock.patch.object(provenance,'producer_link_context',side_effect=lambda *args:
                self.real_link_context(*args) if self.actual_replay else {'command':['wrapper','ld','-T','mickey.us.ld']}),
            mock.patch.object(provenance,'_capture_configured_raw',return_value=(self.raw,[],self.receipt,self.consumer_inputs)),
            mock.patch.object(provenance,'replay_encoded_word_link',side_effect=replay),
            mock.patch.object(rp,'reproduce_data_provider',side_effect=provider_capture),
            mock.patch.object(rp.OrtNames,'load',return_value=rp.OrtNames([],{})),
            mock.patch.object(batch,'bounded_capture',side_effect=cpp)):
            self.stack.enter_context(patch)

    def capture(self):
        witness=provenance.capture_encoded_word(self.world,self.dump)
        self.world._encoded_word=witness
        self.world.data_import_contexts.append((True,witness.current))
        return witness

    def test_sealed_factory_uses_actual_final_ld_replay_and_holds_its_parse(self):
        import shlex
        import runlink_metadata_link as link
        import permute_batch as batch
        self.actual_replay=True
        text='\n'.join(['SECTIONS','{',' .overlay_073 0xF0000000 : AT(0) SUBALIGN(4)',' {',
            '  '+provenance.ENCODED_WORD_TARGET+'(.text);','  overlay_073_DATA_START = .;',
            '  build/'+provenance.ENCODED_WORD_PROVIDER+'.o(.data);',' }',' /DISCARD/ :',' {','  *(*);',' }','}'])+'\n'
        (self.root/'mickey.us.ld').write_text(text)
        final_script=self.root/link.FINAL_SCRIPT;final_script.parent.mkdir(parents=True,exist_ok=True);final_script.write_text(text)
        scripts=[link.FINAL_SCRIPT,'undefined_funcs_auto.us.txt','undefined_syms_auto.us.txt',
                 'libultra_undefined_syms.us.txt','overlay_undefined_syms.us.txt']
        command=['tools/link_diagnose.sh','tools/binutils/mips64-elf-ld']
        for name in scripts:command+=['-T',name]
        command+=['-Map','build/mickey.us.map','--no-check-sections','-o','build/mickey.us.elf']
        (self.root/link.FINAL_LOG).write_text('command: '+shlex.join(command)+'\n')
        self.real_bounded_capture(command,provenance.time.monotonic()+20,check=True,cwd=self.root)
        self.world.elf=self.rp.Elf(self.world.paths['elf']);self.world.map=self.rp.parse_map(self.map_path)
        self.world.map_data=self.map_path.read_bytes()
        with mock.patch.object(batch,'bounded_capture',wraps=batch.bounded_capture) as observed:
            witness=self.capture()
        links=[call for call in observed.call_args_list if call.args[0][0]=='tools/link_diagnose.sh']
        self.assertEqual(len(links),1)
        self.assertNotEqual(witness.replay.path,self.world.paths['elf']);self.assertTrue(witness.current())
        witness.replay.symbols=[]
        with self.assertRaisesRegex(provenance.MetadataProofError,'parsed'):witness.current()

    def test_word_context_excludes_unrelated_callback_but_global_hold_keeps_it(self):
        calls=[];valid=[True]
        def unrelated():
            calls.append(True);return valid[0]
        self.world.data_import_contexts.append((True,unrelated))
        witness=self.capture();self.assertTrue(witness.current());self.assertEqual(calls,[])
        resolver=self.rp.Resolver(self.world,self.rp.OrtNames([],{}));resolver.rows_by_ident()
        self.rp.check_data_import_holds(self.world);self.assertEqual(len(calls),1)
        valid[0]=False
        self.assertTrue(witness.current());self.assertEqual(len(calls),1)
        with self.assertRaises(self.rp.PackError):self.rp.check_data_import_holds(self.world)
        self.assertEqual(len(calls),2);self.assertIsNone(self.world._guarded_data_objects)
        with self.assertRaises(self.rp.PackError):resolver.rows_by_ident()

    def test_preserved_owner_cache_still_requires_fresh_capture_and_keeps_failure(self):
        source=provenance.ENCODED_WORD_PROVIDER[4:-2]
        self.world.data_import_providers={source:self.rp.Elf(self.root/self.provider_receipt['compiler_original'])}
        self.world.data_import_provider_receipts[source]=self.provider_receipt
        self.rp.reproduce_data_provider.reset_mock()
        self.capture()
        self.rp.reproduce_data_provider.assert_called_once_with(self.world,source,preserve_compiler=True)
        with mock.patch.object(self.rp,'reproduce_data_provider',
                side_effect=provenance.MetadataProofError('fresh owner capture failed')) as fresh:
            with self.assertRaisesRegex(provenance.MetadataProofError,'fresh owner capture failed'):
                self.capture()
        fresh.assert_called_once_with(self.world,source,preserve_compiler=True)

    def test_probe_fresh_private_artifacts_do_not_change_deterministic_inputs(self):
        self.world.amods={}
        for name in ('atlas','residuals','rom'):
            self.world.paths[name]=self.root/name
        probe=self.capture()
        before=self.rp.metadata_input_paths(self.world.paths,self.world)
        private_before={path for path,digest in probe.holds if path.name=='encoded-effective.i'}
        self.world.data_import_contexts=[];self.world.data_import_inputs={}
        self.world.data_import_provider_receipts={}
        self.receipt['directory']='build/capture-fresh';(self.root/self.receipt['directory']).mkdir()
        self.provider_receipt['directory']='build/provider-fresh';(self.root/self.provider_receipt['directory']).mkdir()
        fresh=self.capture()
        after=self.rp.metadata_input_paths(self.world.paths,self.world)
        private_after={path for path,digest in fresh.holds if path.name=='encoded-effective.i'}
        self.assertEqual(before,after)
        self.assertEqual(len(private_before),2);self.assertEqual(len(private_after),2)
        self.assertTrue(private_before.isdisjoint(private_after))
        for witness,private in ((probe,private_before),(fresh,private_after)):
            self.assertTrue(private.isdisjoint(after))
            self.assertNotIn(witness.replay.path,after);self.assertNotIn(witness.replay_map_path,after)
            self.assertTrue(witness.current())
        private=next(iter(private_after));original=private.read_bytes()
        private.write_bytes(original+b' late private CPP')
        with self.assertRaises(provenance.MetadataProofError):self.rp.check_data_import_holds(self.world)
        self.assertIsNone(self.world._guarded_data_objects)

    def test_cli_map_row_class_identity_is_not_authority_but_all_fields_are(self):
        alternate=dataclasses.make_dataclass('MapInput',
            [(f.name,f.type) for f in dataclasses.fields(self.rp.MapInput)],
            namespace={'__module__':'__main__'},frozen=True)
        self.world.map=[alternate(*dataclasses.astuple(row)) for row in self.world.map]
        witness=self.capture();self.assertTrue(witness.current())
        self.assertNotEqual(witness.mapping[0],self.rp.parse_map(self.map_path)[0])
        for field in ('outsec','insec','addr','size','file'):
            value=getattr(witness.mapping[0],field)
            changed=dataclasses.replace(witness.mapping[0],**{field:value+4 if isinstance(value,int) else value+'-changed'})
            with self.subTest(field=field),self.assertRaisesRegex(provenance.MetadataProofError,'loaded map'):
                dataclasses.replace(witness,mapping=(changed,*witness.mapping[1:])).current()
        for rows in (witness.mapping[::-1],witness.mapping+witness.mapping[:1]):
            with self.assertRaisesRegex(provenance.MetadataProofError,'loaded map'):
                dataclasses.replace(witness,mapping=rows).current()

    def test_sealed_capture_loaded_parses_and_runtime_pair(self):
        w=self.capture();self.assertTrue(w.current())
        records=[(0,self.rp.R_MIPS_HI16,self.rp.OP_LOCAL,16),(4,self.rp.R_MIPS_LO16,self.rp.OP_LOCAL,16)]
        self.assertTrue(w.runtime(w.linked.bytes_of('.overlay_073'),records))
        for changed in (records[::-1],records[:1],records+records,[(*r[:3],0) for r in records]):
            with self.assertRaises(provenance.MetadataProofError):w.runtime(w.linked.bytes_of('.overlay_073'),changed)
        with self.assertRaises(provenance.MetadataProofError):dataclasses.replace(w,seal=object()).current()
        index=next(i for i,s in enumerate(w.provider.symbols) if s.name==provenance.ENCODED_WORD_OWNER)
        original=w.provider.symbols[index];w.provider.symbols[index]=dataclasses.replace(original,size=4)
        with self.assertRaisesRegex(provenance.MetadataProofError,'parsed'):w.current()
        w.provider.symbols[index]=original;self.assertTrue(w.current())

    def test_raw_receipt_original_compiler_and_wrong_owner_authority_refuse(self):
        for field in ('raw_sha256','configured_sha256','compiler_original_sha256'):
            original=self.receipt[field];self.receipt[field]='bad'
            with self.subTest(field=field),self.assertRaises(provenance.MetadataProofError):self.capture()
            self.receipt[field]=original
        for field in ('raw_sha256','compiler_original_sha256'):
            original=self.provider_receipt[field];self.provider_receipt[field]='bad'
            with self.subTest(provider_field=field),self.assertRaises(provenance.MetadataProofError):self.capture()
            self.provider_receipt[field]=original
        self.receipt.pop('kept_rodata')
        with self.assertRaisesRegex(provenance.MetadataProofError,'keep-only'):self.capture()

    def test_late_source_script_original_or_optin_loss_clears_old_resolver_and_recovers(self):
        for case in ('source','script','original','optin'):
            self.world.data_import_contexts=[];self.world.data_import_inputs={};self.world._guarded_data_objects={}
            w=self.capture();resolver=self.rp.Resolver(self.world,self.rp.OrtNames([],{}));resolver.rows_by_ident()
            path={'source':self.source,'script':self.script,'original':self.original}.get(case)
            old=path.read_bytes() if path else self.dump
            if path:path.write_bytes(old+b' changed')
            else:self.dump=''
            with self.subTest(case=case),self.assertRaises((self.rp.PackError,provenance.MetadataProofError)):
                self.rp.check_data_import_holds(self.world)
            self.assertIsNone(self.world._guarded_data_objects);self.assertIsNone(self.world._encoded_word)
            if path:path.write_bytes(old)
            else:self.dump=old
            with self.assertRaisesRegex(self.rp.PackError,'unavailable'):resolver.rows_by_ident()
            self.world.data_import_contexts=[];self.world.data_import_inputs={}
            fresh=self.capture();self.world._guarded_data_objects={}
            self.assertTrue(fresh.current());self.assertEqual(resolver.rows_by_ident(),{})

    def test_generator_requires_actual_input_roles_and_holds_them(self):
        header=self.root/'include/overlays/overlay_073.h';header.parent.mkdir(parents=True);header.write_text('')
        inputs=[(73,self.root/provenance.ENCODED_WORD_TARGET),
                (73,self.root/('build/'+provenance.ENCODED_WORD_PROVIDER+'.o'))]
        current=provenance.encoded_word_generation_inputs(self.root,inputs)
        current()
        with self.assertRaises(provenance.MetadataProofError):
            provenance.encoded_word_generation_inputs(self.root,inputs[:1])
        with self.assertRaises(provenance.MetadataProofError):
            provenance.encoded_word_generation_inputs(self.root,inputs+inputs)
        self.owner_source.write_bytes(self.owner_source.read_bytes()+b' changed')
        with self.assertRaises(provenance.MetadataProofError):current()

    def test_sealed_marker_late_change_withholds_manifest_and_retry_recaptures(self):
        from test_runlink_pack import MetadataEmitterTests
        harness=MetadataEmitterTests();self.addCleanup(harness.doCleanups)
        with mock.patch.object(self.rp.OrtNames,'load',side_effect=self.real_ort_load):
            harness.setUp()
        load=self.rp.load_and_pack;replace=self.rp.os.replace
        for case in ('expression','optin','parsed','private-cpp'):
            self.world.data_import_contexts=[];self.world.data_import_inputs={}
            witness=self.capture()
            old_script=self.script.read_bytes();old_dump=self.dump
            old_symbols=list(witness.provider.symbols)
            private_cpp=self.root/self.receipt['directory']/'encoded-effective.i';old_cpp=private_cpp.read_bytes()
            def loaded(args):
                paths,world,packed=load(args)
                world._encoded_word=witness
                def held():
                    try:return witness.current()
                    except provenance.MetadataProofError as error:raise self.rp.PackError(str(error)) from error
                world.data_import_contexts=getattr(world,'data_import_contexts',[])+[(True,held)]
                return paths,world,packed
            def changed(source,target):
                replace(source,target)
                if pathlib.Path(target).name=='main_reloc_table.bin':
                    if case=='expression':self.script.write_bytes(old_script+b' o73Triangles = 0;')
                    elif case=='optin':self.dump=''
                    elif case=='private-cpp':private_cpp.write_bytes(old_cpp+b' late private CPP')
                    else:witness.provider.symbols=[]
            with self.subTest(case=case),mock.patch.object(self.rp.OrtNames,'load',side_effect=self.real_ort_load), \
                    mock.patch.object(self.rp,'load_and_pack',side_effect=loaded), \
                    mock.patch.object(self.rp.os,'replace',side_effect=changed):
                status,log=harness.emit()
            self.assertEqual(status,2,log)
            self.assertFalse((harness.out/'manifest.json').exists())
            self.script.write_bytes(old_script);self.dump=old_dump;witness.provider.symbols[:]=old_symbols
            private_cpp.write_bytes(old_cpp)
            self.world.data_import_contexts=[];self.world.data_import_inputs={}
            witness=self.capture()
            with mock.patch.object(self.rp.OrtNames,'load',side_effect=self.real_ort_load), \
                    mock.patch.object(self.rp,'load_and_pack',side_effect=loaded):
                status,log=harness.emit()
            self.assertEqual(status,0,log)

    def test_final_prepublication_retains_private_capture_and_loaded_replay_holds(self):
        import json
        import runlink_metadata_link as link
        boot=self.root/link.BOOT_ELF;boot.parent.mkdir(parents=True,exist_ok=True)
        boot.write_bytes(self.world.paths['elf'].read_bytes())
        for name in (link.RECEIPT,link.BOOT_LOG,link.FINAL_LOG):
            path=self.root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text('fixture')
        paths={self.root/n for n in (link.BOOT_ELF,'build/mickey.us.elf','build/mickey.us.map',
                                    link.RECEIPT,link.BOOT_LOG,link.FINAL_LOG)}
        manifest={'files':{},'inputs':{link.BOOT_ELF:{'sha256':provenance.sha256_file(boot)}}}
        marker=self.root/link.FINAL_PROOF
        for case in ('private-original','loaded-replay','private-missing','private-cpp'):
            self.world.data_import_contexts=[];self.world.data_import_inputs={}
            witness=self.capture();original=self.original.read_bytes();symbols=list(witness.replay.symbols)
            private_cpp=self.root/self.receipt['directory']/'encoded-effective.i';old_cpp=private_cpp.read_bytes()
            def readback(root):
                witness.current()
                return witness.current
            calls=[]
            def outer_paths(root):
                calls.append(True)
                if len(calls)==2:
                    if case=='private-original':self.original.write_bytes(original+b'late private capture')
                    elif case=='private-missing':self.original.unlink()
                    elif case=='private-cpp':private_cpp.write_bytes(old_cpp+b' late private CPP')
                    else:witness.replay.symbols=[]
                return paths
            marker.write_text('old completion proof')
            with self.subTest(case=case),mock.patch.object(link,'provenance_paths',side_effect=outer_paths), \
                    mock.patch.object(link,'check_prepared',return_value=(manifest,{'borrowed_records':1})), \
                    mock.patch.object(link,'link_command',return_value=['ld']), \
                    mock.patch.object(provenance,'canonical_encoded_word_readback',side_effect=readback):
                with self.assertRaises(OSError if case=='private-missing' else link.MetadataLinkError):
                    link.prove(self.root)
            self.assertEqual(len(calls),2);self.assertFalse(marker.exists())
            self.original.write_bytes(original);witness.replay.symbols[:]=symbols
            private_cpp.write_bytes(old_cpp)
            self.world.data_import_contexts=[];self.world.data_import_inputs={}
            witness=self.capture()
            with mock.patch.object(link,'provenance_paths',return_value=paths), \
                    mock.patch.object(link,'check_prepared',return_value=(manifest,{'borrowed_records':1})), \
                    mock.patch.object(link,'link_command',return_value=['ld']), \
                    mock.patch.object(provenance,'canonical_encoded_word_readback',side_effect=readback):
                proof=link.prove(self.root)
            self.assertTrue(marker.is_file())
            self.assertNotIn('capture',json.dumps(proof));self.assertNotIn('replay',json.dumps(proof))

    def test_strict_surface_uses_fresh_role_and_holds_actual_compared_image(self):
        witness=self.capture()
        args=(self.root,73,provenance.ENCODED_WORD_FUNCTION,provenance.ENCODED_WORD_FUNCTION,
              {'source':provenance.ENCODED_WORD_SOURCE[4:-2]},self.ordinary,self.world.paths['elf'])
        with mock.patch.object(provenance,'canonical_encoded_word_capture',return_value=witness):
            proof=provenance.encoded_word_surface(*args)
            self.assertEqual(proof['identity'],(73,0x80000010));proof['current']()
            for index,value in ((1,13),(2,'different'),(3,'different'),(4,{'source':'other'}),
                                (6,self.root/'other.elf')):
                changed=list(args);changed[index]=value
                with self.assertRaises(provenance.MetadataProofError):provenance.encoded_word_surface(*changed)
            original=self.ordinary.read_bytes();self.ordinary.write_bytes(original+b' late')
            with self.assertRaises(provenance.MetadataProofError):proof['current']()
            self.ordinary.write_bytes(original)
            candidate=self.rp.Elf(self.ordinary)
            with mock.patch.object(self.rp,'Elf',return_value=candidate):
                proof=provenance.encoded_word_surface(*args)
            candidate.data+=b' late loaded mutation'
            with self.assertRaises(provenance.MetadataProofError):proof['current']()

    def test_cli_symbol_class_identity_preserves_every_original_symbol_field(self):
        witness=self.capture()
        alternate=dataclasses.make_dataclass('Symbol',
            [(f.name,f.type) for f in dataclasses.fields(self.rp.Symbol)],
            namespace={'__module__':'__main__'},frozen=True)
        image=self.rp.Elf(self.ordinary)
        image.symbols=[alternate(*dataclasses.astuple(s)) for s in image.symbols]
        obj=SimpleNamespace(module=73,source=provenance.ENCODED_WORD_SOURCE[4:-2],path=self.ordinary,elf=image)
        rel=image.rels('.text')[0]
        self.assertNotEqual(rel.sym,witness.raw.rels('.text')[0].sym)
        self.assertEqual(witness.operand(self.world,obj,'.text',rel,0),('ov',73,0x80000010))
        for field in ('index','name','value','size','bind','type','shndx','section'):
            value=getattr(rel.sym,field)
            symbol=dataclasses.replace(rel.sym,**{field:value+1 if isinstance(value,int) else value+'-changed'})
            with self.subTest(field=field),self.assertRaises(provenance.MetadataProofError):
                witness.operand(self.world,obj,'.text',dataclasses.replace(rel,sym=symbol),0)
        self.assertTrue(witness.current())

    def test_loaded_caller_retained_through_late_hold(self):
        w=self.capture()
        obj=SimpleNamespace(module=73,source=provenance.ENCODED_WORD_SOURCE[4:-2],path=self.ordinary,elf=self.rp.Elf(self.ordinary))
        rel=obj.elf.rels('.text')[0]
        self.assertEqual(w.operand(self.world,obj,'.text',rel,0),('ov',73,0x80000010))
        index=next(i for i,s in enumerate(obj.elf.symbols) if s.name==provenance.ENCODED_WORD_MARKER)
        obj.elf.symbols[index]=dataclasses.replace(obj.elf.symbols[index],size=4)
        with self.assertRaises(self.rp.PackError):self.rp.check_data_import_holds(self.world)
        self.assertIsNone(self.world._encoded_word)

class CallbackSourceTests(unittest.TestCase):
    source = ('typedef long s32; typedef unsigned short u16; '
              'typedef struct Row { s32 (*test)(void); void (*update)(void); u16 mask; } Row; '
              'extern s32 test(void); extern void update(void); '
              'Row rows[2]={{test,update,(1u<<0)|(1u<<5)},{0,0,1}};')

    def test_real_typed_fields_and_closed_unsigned_masks(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        facts=provenance.callback_data_facts(self.source.encode(),['-32'],root)
        self.assertEqual(facts['logical_extent'],24)
        self.assertEqual(facts['declared_field_bytes'],20)
        self.assertEqual(facts['internal_padding_bytes'],4)
        self.assertEqual(facts['masks'],[33,1])
        self.assertEqual([f['offset'] for f in facts['callbacks']],[0,4])
        mutants=[self.source.replace('s32 test(void)','void test(void)'),
            self.source.replace('(*test)(void)','(*test)()'),
            self.source.replace('u16 mask','u16 mask; u16 filler'),
            self.source.replace('(1u<<0)|(1u<<5)','1u<<32'),
            self.source.replace('(1u<<0)|(1u<<5)','65536u'),
            self.source.replace('{test,update,','{&test,update,'),
            self.source.replace('Row rows[2]','const Row rows[2]'),
            self.source.replace('[2]','[3]'),self.source+' s32 extra=0;',
            '#pragma pack(1)\n'+self.source]
        for source in mutants:
            with self.subTest(source=source), self.assertRaises(provenance.MetadataProofError):
                provenance.callback_data_facts(source.encode(),['-32'],root)

    def test_definition_abi_not_just_the_import_prototype(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        good=b'typedef long s32; s32 test(void){return 1;} void update(void){}'
        self.assertEqual(provenance.callback_function_facts(good,['-32'],root,['test','update']),
                         {'test':'s32(void)','update':'void(void)'})
        for source in (good.replace(b'test(void)',b'test(int x)'),
                       good.replace(b's32 test',b'float test'),good+b' void update(void){}'):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.callback_function_facts(source,['-32'],root,['test','update'])

    def test_explicit_profile_graph_is_not_a_numeric_or_mixed_binding(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        cmd=('helper --callback-data-profile typed-callback-records-v1 '
             '--callback-source src/overlays/o001/actions.c --callback-text-inputs '
             'build/src/overlays/o001/main.c.o build/src/overlays/o001/tail.c.o')
        self.assertEqual(provenance.callback_selection(cmd,root)[0],'src/overlays/o001/actions.c')
        self.assertIsNone(provenance.callback_selection('helper',root))
        for bad in (cmd.replace('-v1','-v2'),cmd+' --local-data-objects',
                    cmd.replace('o001/tail','o002/tail'),cmd+' build/src/overlays/o001/main.c.o'):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.callback_selection(bad,root)

    def test_loaded_original_and_layout_artifacts_cannot_change_after_capture(self):
        import runlink_pack as rp
        from test_link_overlay_module import callback_fixture
        root=pathlib.Path(__file__).resolve().parent.parent
        raw=rp.Elf('original',callback_fixture())
        cap=provenance.CallbackCapture(raw,hashlib.sha256(raw.data).hexdigest(),
            provenance._static_parsed_surface(raw),'{}',root,(),lambda:True,provenance._CALLBACK_SEAL)
        self.assertTrue(cap.current(raw))
        raw.symbols[2]=dataclasses.replace(raw.symbols[2],size=24)
        with self.assertRaises(provenance.MetadataProofError):cap.current(raw)
        with self.assertRaises(provenance.MetadataProofError):
            dataclasses.replace(cap,seal=object()).current(raw)


class CallbackFrontendFreshnessTests(unittest.TestCase):
    @unittest.skipUnless((pathlib.Path(__file__).resolve().parent.parent/'tools/ido/cc').is_file(),'requires pinned compiler')
    def test_actual_wrapper_cpp_uses_held_search_and_new_shadow_refuses(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        with tempfile.TemporaryDirectory(dir=root/'build') as temporary:
            base=pathlib.Path(temporary);source_dir=base/'source';first=base/'first';second=base/'second';output=base/'proof'
            for path in (source_dir,first,second,output):path.mkdir()
            source=source_dir/'unit.c';source.write_text('#include \"callback_types.h\"\nlong object=VALUE;\n')
            (second/'callback_types.h').write_text('#define VALUE 17\n')
            flags=('-c','-nostdinc','-32','-mips2','-O2','-I',str(first),'-I',str(second))
            command='.venv/bin/python tools/asm-processor/build.py tools/ido/cc -- tools/binutils/mips64-elf-as -- '+ ' '.join(flags)+' -o target.o '+str(source)
            with mock.patch.object(provenance,'_callback_recipe',return_value=(command,flags,('held recipe',))):
                cpp,_flags,_frontend,_held,current,_recipe=provenance._callback_frontend(root,source,output,
                                                                                       provenance.time.monotonic()+120)
                self.assertIn(b'object=17',cpp.replace(b' ',b''))
                self.assertTrue(current())
                (first/'callback_types.h').write_text('#define VALUE 29\n')
                with self.assertRaises(provenance.MetadataProofError):current()
                (first/'callback_types.h').unlink()
                self.assertTrue(current())



class OwnRoSourceTests(unittest.TestCase):
    def test_complete_const_signed_arrays_and_no_guessed_extent(self):
        root = pathlib.Path(__file__).resolve().parent.parent
        source = b'typedef signed long scalar; const scalar first[]={1,-2,3}; const scalar second[1]={7};'
        facts = provenance.own_ro_prefix_facts(source, ['-32'], root)
        self.assertEqual([(r['name'], r['count'], r['values']) for r in facts],
                         [('first', 3, [1, -2, 3]), ('second', 1, [7])])
        for changed in (source.replace(b'const scalar', b'scalar'),
                        source.replace(b'signed long', b'unsigned long'),
                        source.replace(b'[1]', b'[2]'),
                        source.replace(b'{7}', b'{2147483648}'),
                        source + b' long unrelated=0;'):
            with self.subTest(source=changed), self.assertRaises(provenance.MetadataProofError):
                provenance.own_ro_prefix_facts(changed, ['-32'], root)

    def test_capture_keeps_parsed_metadata_private_files_and_global_failure_hold(self):
        import runlink_pack as rp
        from test_link_overlay_module import OwnRoProjectionTests
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            raw, capture, _ = OwnRoProjectionTests.fixture(root)
            self.assertTrue(capture.current())
            symbol = next(s for s in raw.symbols if s.name == 'first')
            raw.symbols[symbol.index] = dataclasses.replace(symbol, size=8)
            world = SimpleNamespace(data_import_contexts=[(True, capture.current)],
                                    data_import_inputs={}, _objects={'held': []}, _defs={'held': []})
            with self.assertRaises(provenance.MetadataProofError):
                rp.check_data_import_holds(world)
            self.assertIsNone(world._objects)
            self.assertIsNone(world._defs)
            raw.symbols[symbol.index] = symbol
            self.assertTrue(capture.current())
            (root/'raw.o').write_bytes(raw.data+b'changed')
            with self.assertRaises(provenance.MetadataProofError):
                capture.current()
            with self.assertRaises(provenance.MetadataProofError):
                dataclasses.replace(capture, seal=object()).current()


if __name__ == "__main__":
    unittest.main()
