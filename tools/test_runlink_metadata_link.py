#!/usr/bin/env python3
"""Narrow fixed-layout script, wrapper, and provenance refusal regressions."""
import json
from pathlib import Path
from types import SimpleNamespace
import subprocess
import tempfile
import unittest
from unittest import mock

import runlink_metadata_link as link
import proof_provenance as pp
import function_preflight as fp
from test_runlink_pack import words


class MetadataLinkTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.names = sorted(link.TOP | {"overlays/o001/overlay_001_reloc1.bin"})

    def script(self, names=None):
        return ('SECTIONS {\n .code : { build/src/real.c.o(.text); }\n' +
                ''.join(f'  build/assets/{n}.o(.data);\n' for n in (names or self.names)) +
                '  build/assets/real_data.bin.o(.data);\n}\n')

    def test_mapping_changes_only_each_expected_metadata_selector(self):
        source = self.script()
        actual = link.mapped_script(source, self.names)
        for name in self.names:
            actual = actual.replace(f'build/runlink/generated/{name}.o', f'build/assets/{name}.o')
        self.assertEqual(actual, source)
        self.assertIn('build/src/real.c.o(.text)', actual)
        self.assertIn('build/assets/real_data.bin.o(.data)', actual)

    def test_missing_duplicate_wrong_section_or_extra_metadata_refuses(self):
        source = self.script()
        old = 'build/assets/main_reloc_table.bin.o'
        cases = [source.replace(old, 'build/assets/missing.bin.o'),
                 source + f'  {old}(.data);\n', source.replace(old+'(.data)', old+'(.text)'),
                 source + '  build/assets/overlays/o002/overlay_002_reloc2.bin.o(.data);\n']
        for text in cases:
            with self.subTest(text=text), self.assertRaises(link.MetadataLinkError):
                link.mapped_script(text, self.names)

    def test_binary_wrapper_preserves_payload_and_all_symbol_names(self):
        tool = link.REPO / 'tools/binutils/mips64-elf-objcopy'
        if not tool.is_file():
            self.skipTest('configured binutils not installed')
        directory = self.root / 'tools/binutils'; directory.mkdir(parents=True)
        (directory / tool.name).symlink_to(tool)
        name = 'overlays/o001/overlay_001_reloc1.bin'
        data = words(0x12, 0x34, 0x56, 0x78)
        for prefix in ('assets', link.META):
            p = self.root / prefix / name;p.parent.mkdir(parents=True, exist_ok=True);p.write_bytes(data)
        old = self.root / ('build/assets/' + name + '.o');old.parent.mkdir(parents=True)
        subprocess.run([str(tool), '-I', 'binary', '-O', 'elf32-bigmips', '-B', 'mips',
                        'assets/'+name, str(old)], cwd=self.root, check=True)
        link.wrapper(self.root, name)
        new = self.root / ('build/runlink/generated/' + name + '.o')
        self.assertEqual(old.read_bytes(), new.read_bytes())

    def manifest(self):
        for name in (link.BOOT_ELF, link.BOOT_MAP):
            p=self.root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'fixture')
        for name in ('tools/runlink_pack.py', 'tools/overlay_tables.py', 'tools/trap_names.py', 'tools/runlink_metadata_link.py'):
            p=self.root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes((link.REPO/name).read_bytes())
        files={}
        for name in self.names:
            p=self.root/link.META/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'table')
            files[name]={'size':5,'sha256':link.digest(p)}
        ledger=self.root/link.META/'borrowing.json'
        ledger.write_text(json.dumps({'totals':{'borrowed_records':1},'borrowed':{'placeholder':{'count':1}}}))
        canonical=self.root/'config/runlink-residuals.us.json';canonical.parent.mkdir(parents=True);canonical.write_bytes(ledger.read_bytes())
        return {'schema':'mickey-generated-metadata-v1',
                'proof':{'raw_equal':True,'raw_differing_bytes':0,'differences':0,'source_only':False,'relayout':False},
                'inputs':{name:{'size':7,'sha256':link.digest(self.root/name)} for name in (link.BOOT_ELF,link.BOOT_MAP)},
                'implementation':{name:link.digest(self.root/name) for name in ('tools/runlink_pack.py','tools/overlay_tables.py','tools/trap_names.py')},
                'files':files,'metadata_files':len(files),
                'borrowing':{'path':'borrowing.json','sha256':link.digest(ledger),'borrowed_records':1,'entries_by_kind':{'placeholder':1}}}

    def test_manifest_requires_bootstrap_and_actual_hashes(self):
        original=self.manifest();marker=self.root/link.MANIFEST
        marker.write_text(json.dumps(original))
        self.assertEqual(link.validate_manifest(self.root),original)
        for mutate in ('canonical','payload','input','implementation','borrowing'):
            with self.subTest(mutate=mutate):
                m=json.loads(json.dumps(original))
                if mutate=='canonical':m['inputs']['build/mickey.us.elf']=m['inputs'].pop(link.BOOT_ELF)
                elif mutate=='payload':m['files'][self.names[0]]['sha256']='bad'
                elif mutate=='input':m['inputs'][link.BOOT_ELF]['sha256']='bad'
                elif mutate=='implementation':m['implementation']['tools/runlink_pack.py']='bad'
                else:m['borrowing']['sha256']='bad'
                marker.write_text(json.dumps(m))
                with self.assertRaises(link.MetadataLinkError):link.validate_manifest(self.root)

    def test_a_foreign_producer_cannot_authenticate_itself_with_a_hash(self):
        m=self.manifest();path=self.root/'tools/runlink_pack.py'
        path.write_bytes(path.read_bytes()+b'\n# different producer\n')
        m['implementation']['tools/runlink_pack.py']=link.digest(path)
        (self.root/link.MANIFEST).write_text(json.dumps(m))
        with self.assertRaisesRegex(link.MetadataLinkError,'differs from the loaded'):
            link.validate_manifest(self.root)

    def test_provenance_snapshots_missing_fixed_artifacts_and_new_payloads(self):
        (self.root/'Makefile').write_text(link.MARKER+' build/runlink/metadata/manifest.json\n')
        paths=link.provenance_paths(self.root)
        self.assertIn(self.root/link.FINAL_PROOF,paths)
        self.assertIn(self.root/link.RECEIPT,paths)
        m=self.manifest();(self.root/link.MANIFEST).write_text(json.dumps(m))
        paths=link.provenance_paths(self.root)
        self.assertIn(self.root/link.META/self.names[0],paths)
        self.assertIn(self.root/('build/runlink/generated/'+self.names[0]+'.o'),paths)
        self.assertIn(self.root/link.BOOT_ELF,paths)

    def test_mutation_during_link_proof_is_refused(self):
        for change in ('bootstrap', 'final', 'map', 'log', 'prepared', 'manifest'):
            with self.subTest(change=change):
                paths={self.root/n for n in (link.BOOT_ELF, 'build/mickey.us.elf',
                    'build/mickey.us.map', link.RECEIPT, link.BOOT_LOG, link.FINAL_LOG)}
                for path in paths:
                    path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(b'same')
                manifest={'files':{n:{'size':4} for n in self.names},
                          'inputs':{link.BOOT_ELF:{'sha256':link.digest(self.root/link.BOOT_ELF)}}}
                if change=='manifest':manifest['inputs'][link.BOOT_ELF]['sha256']='wrong'
                rows=[SimpleNamespace(file='build/runlink/generated/'+n+'.o',insec='.data',size=4)
                      for n in self.names]
                def read_map(path):
                    if change=='map':path.write_bytes(b'replaced')
                    return rows
                def read_elf(path):
                    data=path.read_bytes()
                    if path==self.root/'build/mickey.us.elf':
                        target={'bootstrap':self.root/link.BOOT_ELF,'final':path,
                                'log':self.root/link.FINAL_LOG,'prepared':self.root/link.RECEIPT}.get(change)
                        if target is not None:target.write_bytes(b'replaced')
                    return SimpleNamespace(data=data,sections=[],symbols=[])
                with mock.patch.object(link,'provenance_paths',return_value=paths), \
                        mock.patch.object(link,'check_prepared',return_value=(manifest,{'borrowed_records':1})), \
                        mock.patch.object(link,'link_command',return_value=['ld']), \
                        mock.patch.object(link.rp,'parse_map',side_effect=read_map), \
                        mock.patch.object(link.rp,'Elf',side_effect=read_elf):
                    with self.assertRaisesRegex(link.MetadataLinkError,'changed during proof'):
                        link.prove(self.root)

    def test_provenance_returns_the_validated_snapshot_without_rehashing(self):
        path=self.root/'input';path.write_bytes(b'changed after validation')
        with mock.patch.object(link,'enabled',return_value=True), \
                mock.patch.object(link,'_prove',return_value=({}, {path:'validated digest'})):
            self.assertEqual(link.provenance_context(self.root),{'input':'validated digest'})

    def test_freshness_hash_failure_cannot_be_hidden_by_make_question_success(self):
        target=self.root/'target.elf';target.write_bytes(b'fixture')
        with mock.patch.object(fp,'TARGET_ELF',target), mock.patch.object(fp,'REPO',self.root), \
                mock.patch.object(pp,'generated_metadata_context',side_effect=pp.MetadataProofError('tampered')), \
                mock.patch.object(fp,'_run') as query:
            with self.assertRaisesRegex(fp.StaleEvidenceError,'tampered'):
                fp._require_fresh_target(target,label='canonical',non_matching=False,build_logic_inputs=())
            query.assert_not_called()


class EditedReceiptTests(unittest.TestCase):
    def test_explicit_catalog_validates_owner_rows_source_kind_and_selection(self):
        import copy
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for number in (34, 75):
                source = root / f'src/overlays/o{number:03d}/unit.c'
                source.parent.mkdir(parents=True); source.write_text('void entry(void) {}\n')
            exports = link.rp.OrtNames(['entry34', 'entry75'], {})
            document = {'schema': 'mickey-edited-source-modules-v1', 'default_selected': [75],
                'modules': [{'module': n, 'source': f'src/overlays/o{n:03d}/unit.c',
                             'exports': [{'name': 'entry'+str(n), 'row': i, 'kind': 'function'}],
                             'entries': {'init': None, 'resume': None}} for i, n in enumerate((34, 75))]}
            path = root / 'catalog.json'; path.write_text(json.dumps(document))
            self.assertEqual(set(link.edited_catalog(root, path, exports, [34, 75])[0]), {34, 75})
            self.assertEqual(set(link.edited_catalog(root, path, exports)[0]), {75})
            for kind in ('row', 'duplicate', 'kind', 'source', 'field'):
                changed = copy.deepcopy(document)
                if kind == 'row': changed['modules'][0]['exports'][0]['row'] = 1
                elif kind == 'duplicate': changed['modules'].append(changed['modules'][0])
                elif kind == 'kind': changed['modules'][0]['exports'][0]['kind'] = 'opaque'
                elif kind == 'source': changed['modules'][0]['source'] = 'src/overlays/o075/unit.c'
                else: changed['modules'][0]['bss_size'] = 0
                path.write_text(json.dumps(changed))
                with self.subTest(kind=kind), self.assertRaises(link.rp.PackError):
                    link.edited_catalog(root, path, exports, [34])
            path.write_text(json.dumps(document))
            for selected in ([34, 34], [108], []):
                with self.subTest(selected=selected), self.assertRaises(link.rp.PackError):
                    link.edited_catalog(root, path, exports, selected)

    def test_current_outside_program_closure_omits_metadata_and_fill(self):
        source = ('  build/src/overlays/o034/unit.c.o(.text);\n'
                  '  build/assets/overlays/o034/overlay_034_reloc1.bin.o(.data);\n'
                  '  build/assets/main_reloc_table.bin.o(.data);\n'
                  '  build/assets/rom_fill.bin.o(.data);\n'
                  '  build/assets/opaque.bin.o(.data);\n')
        self.assertEqual(link.edited_program_selectors(source),
                         ['build/assets/opaque.bin.o', 'build/src/overlays/o034/unit.c.o'])

    def test_complete_source_recipe_is_required(self):
        source = 'src/overlays/o034/unit.c'; target = 'build/'+source+'.o'
        compile_line = f'wrapper tools/ido/cc -- as -- -c -o {target} {source}\n'
        complete = compile_line + f'python3 tools/link_overlay_module.py {target} --directory build/module\n'
        with mock.patch.object(link.subprocess, 'check_output', return_value=complete):
            self.assertEqual(link.edited_recipe(Path('.'), source)[0][-1], source)
        for command in (compile_line, complete + complete):
            with mock.patch.object(link.subprocess, 'check_output', return_value=command):
                with self.assertRaisesRegex(link.rp.PackError, 'complete-module'):
                    link.edited_recipe(Path('.'), source)

    def test_actual_zero_storage_write_only_descriptor_needs_no_nobits_reserve(self):
        descriptor = {'type': 1, 'flags': 1, 'size': 0, 'align': 1, 'addr': 2336}
        self.assertTrue(link.edited_bss_valid(None, 0, 2336))
        self.assertTrue(link.edited_bss_valid(descriptor, 0, 2336))
        self.assertFalse(link.edited_bss_valid(descriptor, 1, 2336))
        self.assertFalse(link.edited_bss_valid({**descriptor, 'flags': 3}, 0, 2336))
        self.assertFalse(link.edited_bss_valid({**descriptor, 'flags': 5}, 0, 2336))
        self.assertFalse(link.edited_bss_valid({**descriptor, 'size': 4}, 0, 2336))
        self.assertFalse(link.edited_bss_valid({**descriptor, 'addr': 2340}, 0, 2336))
        allocated = {'type': link.rp.SHT_NOBITS, 'flags': 3, 'size': 32, 'align': 16, 'addr': 2336}
        self.assertTrue(link.edited_bss_valid(allocated, 32, 2336))

    def test_edited_output_cannot_alias_source_or_another_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'build/edited-runlink').mkdir(parents=True)
            (root / 'src').mkdir()
            (root / 'build/edited-runlink/link').symlink_to(root / 'src', target_is_directory=True)
            with self.assertRaises(link.rp.PackError):
                link.edited_directory(root, 'src')
            with self.assertRaisesRegex(link.rp.PackError, 'symlink'):
                link.edited_directory(root, 'build/edited-runlink/link/output')

    def test_eager_encoder_cannot_claim_a_later_first_proof_disk_hash(self):
        from runlink import codec, rules
        link.edited_loaded_identity()
        original = link.digest
        for module in (codec, rules):
            def changed(path):
                return 'replaced after import' if Path(path) == Path(module.__file__) else original(path)
            with self.subTest(module=module.__name__), mock.patch.object(link, 'digest', side_effect=changed):
                with self.assertRaisesRegex(link.rp.PackError, 'loaded edited'):
                    link.edited_loaded_identity()

    def test_selected_profile_rejects_wrong_duplicate_or_missing_owner_rows(self):
        import copy
        spec = json.loads((link.REPO / 'tests/fixtures/editable_layout/manifest.json').read_text())
        exports = link.rp.OrtNames.load(link.REPO / 'config/runlink-ort.us.txt')
        import check_editable_layout as pilot
        pilot.edited_fixture_declaration(spec, exports)
        for kind in ('row', 'duplicate', 'missing', 'schema', 'source', 'function'):
            changed = copy.deepcopy(spec)
            if kind == 'row': changed['existing_exports'][0]['row'] += 1
            elif kind == 'duplicate': changed['existing_exports'].append(changed['existing_exports'][0])
            elif kind == 'missing': changed['existing_exports'].pop()
            elif kind == 'schema': changed['schema'] = 'unknown'
            elif kind == 'source': changed['source'] = 'src/other.c'
            else: changed['mutated_function'] = 'other'
            with self.subTest(kind=kind), self.assertRaises(link.rp.PackError):
                pilot.edited_fixture_declaration(changed, exports)

    def test_preloaded_linker_projection_code_cannot_acquire_a_first_call_pin(self):
        import sys
        with mock.patch.object(link, '_EDITED_PROJECT_PIN', None), \
             mock.patch.dict(sys.modules, {'yaml': SimpleNamespace(__file__='preloaded.py')}):
            with self.assertRaisesRegex(link.rp.PackError, 'preloaded'):
                link.edited_projection_tools()

    def test_current_receipt_refuses_missing_changed_or_mutating_provider(self):
        import hashlib
        import permute_batch as batch
        import check_editable_layout as pilot
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            parent = root / 'build/edited-runlink'; directory = parent / 'control-one'
            directory.mkdir(parents=True)
            provider = root / 'provider.o'; provider.write_bytes(b'actual input')
            output = directory / 'final.bin'; output.write_bytes(b'validated')
            expected = pilot.artifact_hashes(directory)
            request = {'outside_rom': 'first.z64', 'selected_modules': [75], 'declaration_sha256': 'catalogA'}
            receipt = {'schema': 'mickey-edited-source-build-v2', 'variant': 'control', 'request': request,
                       'artifacts': expected, 'inputs': {'provider.o': link.digest(provider)},
                       'provider_holds': {}, 'toolchain': {}, 'projection_toolchain': {},
                       'resident_source_context': {}, 'outside_selectors': [],
                       'outside_input_context': {'files': {}, 'assembly': {}}, 'source_cpp': {}, 'arguments': {}}
            payload = link.json_bytes(receipt); (directory / 'receipt.json').write_bytes(payload)
            (parent / 'control-current.json').write_bytes(link.json_bytes({'path': 'build/edited-runlink/control-one',
                'receipt_sha256': hashlib.sha256(payload).hexdigest()}))
            with mock.patch.object(batch, 'checked_tool_identity', return_value={}), \
                 mock.patch.object(link, 'edited_projection_tools', return_value={}), \
                 mock.patch.object(link, 'edited_source_context', return_value={}):
                self.assertEqual(link.edited_current(root, parent, 'control', request), receipt)
                (root / 'second.z64').write_bytes(b'different valid outside request')
                self.assertIsNone(link.edited_current(root, parent, 'control', {'outside_rom': 'second.z64'}))
                self.assertIsNone(link.edited_current(root, parent, 'control', {**request, 'selected_modules': [34]}))
                self.assertIsNone(link.edited_current(root, parent, 'control', {**request, 'declaration_sha256': 'catalogB'}))
                provider.write_bytes(b'changed')
                self.assertIsNone(link.edited_current(root, parent, 'control', request))
                provider.unlink()
                self.assertIsNone(link.edited_current(root, parent, 'control', request))
                provider.write_bytes(b'actual input')
                output.unlink()
                self.assertIsNone(link.edited_current(root, parent, 'control', request))
                output.write_bytes(b'validated')
                original = pilot.artifact_hashes
                def replace_after_validation(path):
                    result = original(path)
                    if output.read_bytes() == b'validated':
                        output.write_bytes(b'unvalidated replacement')
                    return result
                with mock.patch.object(pilot, 'artifact_hashes', side_effect=replace_after_validation):
                    self.assertIsNone(link.edited_current(root, parent, 'control', request))


if __name__=='__main__':unittest.main()
