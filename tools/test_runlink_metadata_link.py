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
        for name in ('tools/runlink_pack.py', 'tools/overlay_tables.py', 'tools/runlink_metadata_link.py'):
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
                'implementation':{name:link.digest(self.root/name) for name in ('tools/runlink_pack.py','tools/overlay_tables.py')},
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


if __name__=='__main__':unittest.main()
