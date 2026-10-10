#!/usr/bin/env python3
"""Adversarial source-only fixtures for explicit raw-reference scheduling."""
import copy
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import raw_reference_gate as g

TOOL = Path(__file__).with_name('lane_status.py').resolve()
OWNER = 'main/gen_anim_data'


def config():
    return {'sha1': g.ROM_SHA1, 'segments': [
        {'name': 'main', 'type': 'code', 'start': 4096, 'vram': 2147483648,
         'subsegments': [[4096, 'asm', 'main/other'], [4112, 'asm', OWNER], [4160, 'c', 'main/next']]},
        {'start': 8192, 'type': 'bin', 'name': 'data'}]}


class RawGate(unittest.TestCase):
    def setUp(self):
        self.old = Path.cwd()
        self.temp = tempfile.TemporaryDirectory()
        os.chdir(self.temp.name)
        self.cmd('init', '-q', '-b', 'main')
        self.cmd('config', 'user.email', 'fixture@example.invalid')
        self.cmd('config', 'user.name', 'Fixture')
        self.write(g.YAML_PATH, json.dumps(config()))
        self.write('docs/prior.md', 'gen_anim_data: prior negative reference search.\n')
        self.source = self.save('Initial raw ownership')
        self.seed()
        self.ledger = self.save('Reconcile raw reference handoff')
        self.auth()
        self.base = self.save('Authorize one reference-only mechanism')

    def tearDown(self):
        os.chdir(self.old)
        self.temp.cleanup()

    def cmd(self, *args):
        return subprocess.check_output(['git', *args], text=True, stderr=subprocess.DEVNULL).strip()

    def write(self, path, text):
        path = Path(path); path.parent.mkdir(parents=True, exist_ok=True); path.write_text(text)

    def save(self, message):
        self.cmd('add', '.'); self.cmd('commit', '-qm', message)
        return self.cmd('rev-parse', 'HEAD')

    def seed(self):
        row = {'schema_version': 1, 'work_class': g.WORK_CLASS, 'owner': OWNER,
               'identity': g.identity_text(Path(g.YAML_PATH).read_text(), OWNER),
               'summary': 'Prior reference coverage exhausted.',
               'evidence': [{'commit': self.source, 'path': 'docs/prior.md', 'summary': 'Reconciled negative search.'}]}
        self.write(g.shard_path(OWNER), json.dumps(row))

    def auth(self, **changes):
        row = {'source_commit': self.source, 'ledger_commit': self.ledger, 'reason': 'New bounded retail revision.'}
        row.update(changes)
        self.write(g.AUTH_PATH, json.dumps({'schema_version': 1, 'work_class': g.WORK_CLASS, 'authorizations': {OWNER: row}}))

    def state(self, base='HEAD'):
        return g.classify(base, OWNER)['state']

    def change_yaml(self, edit):
        doc = json.loads(Path(g.YAML_PATH).read_text()); edit(doc)
        self.write(g.YAML_PATH, json.dumps(doc))

    def test_fresh_authorization(self):
        result = g.classify('HEAD', OWNER)
        self.assertEqual(result['state'], 'base-only')
        self.assertEqual(result['source_commit'], self.source)
        self.assertEqual(result['ledger_commit'], self.ledger)
        self.assertEqual(result['work_class'], g.WORK_CLASS)
        self.assertEqual(result['matching_credit'], 0)

    def test_explicit_cli_and_c_isolation(self):
        p = subprocess.run([sys.executable, str(TOOL), '--base', 'HEAD', '--raw-reference', OWNER, '--json'], capture_output=True, text=True)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertEqual(json.loads(p.stdout)['work_class'], g.WORK_CLASS)
        for selector in ['--symbol', '--symbols']:
            p = subprocess.run([sys.executable, str(TOOL), '--base', 'HEAD', selector, 'gen_anim_data', '--no-cache'], capture_output=True, text=True)
            self.assertNotEqual(p.returncode, 0)
        p = subprocess.run([sys.executable, str(TOOL), '--raw-reference', OWNER, '--symbol', 'gen_anim_data'], capture_output=True, text=True)
        self.assertEqual(p.returncode, 2)
        self.assertFalse(Path('build/cache').exists())

    def test_worktree_cannot_authorize(self):
        self.cmd('checkout', '-q', self.ledger)
        self.auth()
        self.assertEqual(self.state(), 'stale-ledger')

    def test_missing_and_empty_authorization(self):
        self.write(g.AUTH_PATH, json.dumps({'schema_version':1,'work_class':g.WORK_CLASS,'authorizations':{}}))
        self.save('No authorization')
        self.assertEqual(self.state(), 'already-integrated/exhausted')
        Path(g.AUTH_PATH).unlink(); self.save('Missing metadata')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_handoff_change_consumes(self):
        p=Path(g.shard_path(OWNER)); row=json.loads(p.read_text());row['summary']='New completed search.';p.write_text(json.dumps(row))
        self.save('Evidence update')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_source_change_consumes_even_with_updated_shard(self):
        self.change_yaml(lambda d:d['segments'][0]['subsegments'][1].__setitem__(0, 4116))
        self.seed();self.save('Changed boundary')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_sibling_change_preserves_pin(self):
        self.change_yaml(lambda d:d['segments'][0]['subsegments'][0].__setitem__(2, 'main/renamed_other'))
        self.save('Sibling ownership only')
        self.assertEqual(self.state(), 'base-only')
        self.assertEqual(g.source_pin(g.commit('HEAD'), OWNER), self.source)

    def test_source_revert_consumes(self):
        original=Path(g.YAML_PATH).read_text()
        self.change_yaml(lambda d:d['segments'][0].__setitem__('vram', 2147483664));self.save('Mapping changed')
        self.write(g.YAML_PATH, original);self.save('Mapping reverted')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_null_nonexistent_and_noncheckpoint_pins(self):
        for key,value in [('ledger_commit',None),('source_commit','f'*40),('source_commit',self.base),('ledger_commit',self.base)]:
            with self.subTest(key=key,value=value):
                self.auth(**{key:value});self.save('Bad pins')
                self.assertEqual(self.state(), 'stale-ledger')

    def test_malformed_foreign_and_missing_handoff(self):
        original=Path(g.shard_path(OWNER)).read_text()
        for edit in [lambda r:r.__setitem__('owner','main/foreign'),lambda r:r.__setitem__('evidence',[]),lambda r:r.__setitem__('extra',True),lambda r:r['evidence'][0].__setitem__('path','docs/foreign.md')]:
            row=json.loads(original);edit(row);self.write(g.shard_path(OWNER),json.dumps(row));self.save('Invalid handoff')
            self.assertEqual(self.state(), 'stale-ledger')
        Path(g.shard_path(OWNER)).unlink();self.save('Remove handoff')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_duplicate_json_rejected(self):
        with self.assertRaises(g.GateError):g.strict_json('{"a":1,"a":2}')

    def test_yaml_adversaries(self):
        mutations=[
          lambda d:d.__setitem__('sha1','0'*40),
          lambda d:d['segments'][0]['subsegments'].append([4176,'asm',OWNER]),
          lambda d:d['segments'][0]['subsegments'][1].__setitem__(0,True),
          lambda d:d['segments'][0]['subsegments'][1].__setitem__(0,'4112'),
          lambda d:d['segments'][0]['subsegments'][1].__setitem__(0,4113),
          lambda d:d['segments'][0]['subsegments'][2].__setitem__(0,4100),
          lambda d:d['segments'][0]['subsegments'][0].__setitem__(0,4120),
          lambda d:d['segments'][0]['subsegments'][1].__setitem__(1,'c'),
          lambda d:d['segments'][0].__setitem__('name','overlay_001'),
          lambda d:d['segments'][0].__setitem__('vram',True),
          lambda d:d['segments'].pop(),
        ]
        for mutation in mutations:
            d=config();mutation(d)
            with self.subTest(d=d),self.assertRaises(g.GateError):g.identity_text(json.dumps(d),OWNER)
        with self.assertRaises(g.GateError):g.identity_text('sha1: x\nsha1: y\n',OWNER)
        with self.assertRaises(g.GateError):g.identity_text(json.dumps(config()),'main/../gen_anim_data')

    def lane(self, base=None):
        self.cmd('checkout','-qb','lane/test',base or self.base)

    def test_lane_sibling_and_base_only_changes_clear(self):
        self.lane()
        self.change_yaml(lambda d:d['segments'][0]['subsegments'][0].__setitem__(2,'main/sibling'))
        self.save('Sibling lane change')
        self.cmd('checkout','-q','main')
        self.assertEqual(self.state(),'base-only')

    def test_older_divergent_lane_is_active(self):
        self.lane(self.ledger)
        self.change_yaml(lambda d:d['segments'][0]['subsegments'][1].__setitem__(0,4116))
        self.save('Older lane changes owner')
        self.cmd('checkout','-q','main')
        self.change_yaml(lambda d:d['segments'][0].__setitem__('vram',2147483680))
        self.seed();self.save('Independent newer canonical mapping')
        self.assertEqual(self.state(),'active')

    def test_new_lane_seen_after_prior_classification(self):
        self.assertEqual(self.state(),'base-only')
        self.lane();Path(g.shard_path(OWNER)).unlink();self.save('Delete handoff')
        self.cmd('checkout','-q','main')
        self.assertEqual(self.state(),'active')

    def test_lane_corruption_or_deletion_blocks(self):
        for corrupt in (False,True):
            with self.subTest(corrupt=corrupt):
                self.cmd('checkout','-q','main')
                self.cmd('checkout','-qB','lane/test',self.base)
                if corrupt:self.write(g.YAML_PATH,'not: [valid')
                else:self.change_yaml(lambda d:d['segments'][0]['subsegments'].pop(1))
                self.save('Damaged lane ownership')
                self.cmd('checkout','-q','main')
                self.assertEqual(self.state(),'active')

    def test_base_only_change_does_not_reserve_lane(self):
        self.lane();self.write('unrelated.txt','lane');self.save('Unrelated work')
        self.cmd('checkout','-q','main')
        self.change_yaml(lambda d:d['segments'][0].__setitem__('vram',2147483680))
        self.seed();self.save('Canonical mapping only')
        self.assertEqual(self.state(),'stale-ledger')
        self.assertEqual(g.classify('HEAD',OWNER)['active_lanes'],[])

    def test_unrelated_pin_ancestry(self):
        self.cmd('checkout','--orphan','unrelated')
        self.write('other','x');foreign=self.save('Unrelated root')
        self.cmd('checkout','-q','main');self.auth(source_commit=foreign);self.save('Foreign pin')
        self.assertEqual(self.state(),'stale-ledger')

    def test_legacy_change_requires_reconciliation_then_fresh_pins(self):
        self.write('docs/prior.md', 'gen_anim_data: new negative reference search.\n')
        self.assertEqual(g.check_worktree('HEAD'), (1, 1))
        evidence_commit = self.save('Additional legacy finding')
        self.assertEqual(g.check_worktree('HEAD'), (1, 1))
        self.assertEqual(self.state(), 'stale-ledger')
        row = json.loads(Path(g.shard_path(OWNER)).read_text())
        row['evidence'].append({'commit': evidence_commit, 'path': 'docs/prior.md',
                                'summary': 'New finding reconciled explicitly.'})
        self.write(g.shard_path(OWNER), json.dumps(row))
        self.assertEqual(g.check_worktree('HEAD'), (1, 1))
        self.ledger = self.save('Reconcile newer legacy evidence')
        self.assertEqual(self.state(), 'stale-ledger')
        self.auth(reason='A different newly reviewed coverage gap.')
        self.save('Authorize new reference mechanism')
        self.assertEqual(self.state(), 'base-only')

    def test_lane_legacy_change_blocks(self):
        self.lane(); self.write('docs/prior.md', 'gen_anim_data: lane finding.\n')
        self.save('Lane legacy finding'); self.cmd('checkout', '-q', 'main')
        self.assertEqual(self.state(), 'active')

    def test_source_path_conflict_is_not_raw_ready(self):
        self.write('src/main/gen_anim_data.c', 'void gen_anim_data(void) {}\n')
        self.save('Conflicting C ownership')
        self.assertEqual(self.state(), 'stale-ledger')

    def test_mapping_dependency_changes_identity(self):
        d = config()
        d['segments'].insert(0, {'name': 'entry', 'type': 'code', 'start': 0,
                                  'vram': 2147480000, 'subsegments': [[0, 'c', 'entry']]})
        d['segments'][1]['follows_vram'] = 'entry'
        original = g.identity_text(json.dumps(d), OWNER)
        d['segments'][0]['vram'] += 16
        self.assertNotEqual(original, g.identity_text(json.dumps(d), OWNER))
        d['segments'][0]['follows_vram'] = 'main'
        with self.assertRaises(g.GateError): g.identity_text(json.dumps(d), OWNER)

    def test_nonadjacent_reversed_boundary_rejected(self):
        d = config()
        d['segments'][0]['subsegments'].extend([[4200, 'c', 'tail'], [4140, 'c', 'overlap']])
        with self.assertRaises(g.GateError): g.identity_text(json.dumps(d), OWNER)

    def test_fetched_lane_blocks(self):
        self.lane()
        self.change_yaml(lambda d:d['segments'][0]['subsegments'][1].__setitem__(0,4116))
        head = self.save('Fetched lane raw owner change')
        self.cmd('checkout', '-q', 'main')
        self.cmd('update-ref', 'refs/remotes/origin/lane/burn-b-fixture', head)
        self.cmd('branch', '-D', 'lane/test')
        self.assertEqual(self.state(), 'active')

    def test_owner_mapping_overrides_rejected(self):
        for extra in ('vram', 'align', 'unknown'):
            d = config()
            d['segments'][0]['subsegments'][1] = {'start':4112, 'type':'asm', 'name':OWNER, extra:1234}
            with self.subTest(extra=extra), self.assertRaises(g.GateError):
                g.identity_text(json.dumps(d), OWNER)
        d = config()
        d['segments'][0]['subsegments'][1] = {'start':4112, 'type':'asm', 'name':OWNER}
        self.assertEqual(g.identity_text(json.dumps(d),OWNER),g.identity_text(json.dumps(config()),OWNER))

    def test_unrelated_merge_preserves_source_pin(self):
        self.cmd('checkout', '-qb', 'old-branch', self.source)
        self.write('unrelated.txt', 'old side branch'); self.save('Unrelated side work')
        self.cmd('checkout', '-q', 'main')
        self.change_yaml(lambda d:d['segments'][0].__setitem__('vram',2147483680))
        self.source = self.save('New canonical mapping')
        self.seed(); self.ledger = self.save('Reconcile new mapping')
        self.auth(); self.save('Authorize new mapping')
        self.cmd('merge', '--no-ff', '-qm', 'Merge unrelated old branch', 'old-branch')
        self.assertEqual(g.source_pin(g.commit('HEAD'),OWNER),self.source)
        self.assertEqual(self.state(),'base-only')

    def test_old_missing_owner_lane_c_conflict_blocks(self):
        original = Path(g.YAML_PATH).read_text()
        self.change_yaml(lambda d:d['segments'][0]['subsegments'].pop(1))
        common = self.save('Raw owner temporarily absent')
        self.cmd('checkout', '-qb', 'lane/early-c-owner', common)
        self.write('src/main/gen_anim_data.c', 'void gen_anim_data(void) {}\n')
        self.save('Lane claims C owner')
        self.cmd('checkout', '-q', 'main')
        self.write(g.YAML_PATH, original); self.source = self.save('Restore raw owner')
        self.seed(); self.ledger = self.save('Reconcile restored raw owner')
        self.auth(); self.save('New raw reference authorization')
        self.assertEqual(self.state(), 'active')

    def test_nested_owner_layout_rejected_across_parents(self):
        d = config()
        d['segments'].append({'name':'overlay_001','type':'code','start':12288,'vram':4026531840,
                               'subsegments':[{'type':'group','start':12288,
                                               'subsegments':[[12288,'asm',OWNER]]}]})
        with self.assertRaises(g.GateError):g.identity_text(json.dumps(d),OWNER)

    def test_reason_bounds(self):
        for reason in ('x'*241, 'new | old'):
            self.auth(reason=reason); self.save('Invalid reason')
            self.assertEqual(self.state(),'stale-ledger')

    def test_check_worktree_is_not_assignment(self):
        self.assertEqual(g.check_worktree('HEAD'),(1,1))
        self.write(g.shard_path(OWNER),'{}')
        with self.assertRaises(g.GateError):g.check_worktree('HEAD')
        self.assertEqual(self.state(),'base-only')


if __name__ == '__main__':unittest.main()
