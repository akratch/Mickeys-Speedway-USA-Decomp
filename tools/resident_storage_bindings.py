#!/usr/bin/env python3
"""Fixed, independently witnessed bindings for three R8 storage carriers.

This module is deliberately not a resolver.  It accepts a registry key only,
proves the corresponding source and physical-storage witness, then returns a
fixed original identity from the reviewed Q69 namespace.  It never accepts a
candidate address, owner, source, or witness path from its caller.
"""
from __future__ import annotations

import hashlib
import json
import re
import shlex
import shutil
import subprocess
import tempfile
import time
from pathlib import Path

import function_preflight as fp
import candidate_context as cc
import permute_batch as batch
import proof_provenance as pp
import reloc_surface as rs
import resident_storage_view as view
import source_symbol_fidelity as sf

ROOT = Path(__file__).resolve().parent.parent
FUNCTION = 'func_overlay_008_F0001294_185EFEC'
SOURCE_REL = 'src/overlays/o008/overlay_008.c'
SOURCE_SHA256 = 'a785d82a0c344cb7758e796c1e2f38cd9eaaa6fed9918d979754eb2b7915e57e'
_COMMON = Path(subprocess.check_output(['git','rev-parse','--git-common-dir'],cwd=ROOT,text=True).strip()).resolve()
AUTHORITY = _COMMON/'campaign-handoffs/luna-65/Q69-STORAGE-ADAPTER-ARCHITECTURE-20261001/proposed-bindings.json'
AUTHORITY_SHA256 = '171ecb9e7d918a514da23a8cea40fd2405f72621edd2b08215f2b3a3ea737c31'

# Each row is a reviewed fixed binding, never an input supplied by the caller.
BINDINGS = {
    'impact_gate': {'carrier':'gO8P1294ImpactGateReloc','ctype':'u8','size':1,
        'owner':'D_8007BF04','identity':(0xFFD,0x31A4),'section':'main/anim',
        'view':('u8',0,1,4)},
    'color_gate': {'carrier':'gO8P1294ColorGateReloc','ctype':'u8','size':1,
        'owner':'D_8007BF0C','identity':(0xFFD,0x31AC),'section':'main/objects',
        'view':('u8',0,1,4)},
    'gravity': {'carrier':'gO8P1294MotionScalarReloc','ctype':'f32','size':4,
        'owner':'D_800CB304','identity':(0xFFF,0x458C4),'section':'main/charControl',
        'view':('f32',0,4,4)},
}
_LOADED = {Path(m.__file__):pp.sha256_file(Path(m.__file__)) for m in
           (fp,cc,batch,pp,rs,view,sf)}
_LOADED[Path(__file__)] = pp.sha256_file(Path(__file__))


class BindingError(RuntimeError):
    """A fixed binding failed closed because its evidence was insufficient."""


def _need(ok, message):
    if not ok:
        raise BindingError(message)


def _json_stable(value):
    """Compare the same JSON-safe shape that is written into receipts."""
    return json.loads(json.dumps(value,sort_keys=True))


def _verify_loaded():
    _need(all(path.is_file() and pp.sha256_file(path)==digest
              for path,digest in _LOADED.items()),
          'loaded binding evidence implementation changed')
    _need(AUTHORITY.is_file() and pp.sha256_file(AUTHORITY)==AUTHORITY_SHA256,
          'released fixed-identity authority is missing or changed')
    doc=json.loads(AUTHORITY.read_text())
    entries={row['carrier']:row for row in doc['entries']}
    for key,spec in BINDINGS.items():
        row=entries.get(spec['carrier'])
        _need(row is not None and tuple(row['expected_namespace_identity'])==spec['identity'] and
              row['owner_symbol']==spec['owner'] and
              row['physical_extent']==4 and row['route'].startswith('explicit-resident-') and
              row['view_type']==spec['ctype'] and row.get('view_extent',spec['size'])==spec['size'],
              f'fixed authority disagrees with registry for {key}')
    _need(doc['candidate_scope']=={'function':FUNCTION,'overlay':8,'source':'overlays/o008/overlay_008'},
          'released authority candidate scope changed')


def _source_declaration(source, spec):
    text=pp._mask_c(source.read_text())
    pattern=re.compile(rf'^\s*extern\s+{spec["ctype"]}\s+{re.escape(spec["carrier"])}\s*;\s*$',re.M)
    matches=list(pattern.finditer(text))
    _need(len(matches)==1,'carrier declaration is absent, mistyped, or ambiguous')
    line=text.count('\n',0,matches[0].start())+1
    # The declared object must be unconditional source, not a hidden macro branch.
    depth=0
    for directive in re.findall(r'^\s*#\s*(if|ifdef|ifndef|endif)\b',text[:matches[0].start()],re.M):
        depth += 1 if directive in ('if','ifdef','ifndef') else -1
    _need(depth==0,'carrier declaration is conditionally compiled')
    return line


def _carrier_record(elf, spec):
    rows=[row for row in elf.symbols() if row[0]==spec['carrier']]
    _need(len(rows)==1,'carrier symbol is missing or duplicated in the full-TU object')
    name,value,size,info,shndx=rows[0][:5]
    _need((info>>4)==1 and (info&15)==1 and shndx==0 and size==spec['size'],
          'carrier is not the exact external GLOBAL UND STT_OBJECT of reviewed width')
    return {'name':name,'binding':'GLOBAL','type':'OBJECT','section':'UND','size':size}


def _r8_recipe(source, configured, deadline):
    sr=source.relative_to(ROOT).as_posix(); tr=configured.relative_to(ROOT).as_posix()
    raw=batch.bounded_capture(['gmake','NON_MATCHING=1','--no-print-directory','-n','-W',sr,tr],deadline,check=True).stdout
    lines=[line.strip() for line in raw.replace('\\\n',' ').splitlines() if line.strip()]
    compiler=[]
    for line in lines:
        try: tokens=shlex.split(line)
        except ValueError as exc: raise BindingError('R8 recipe has invalid shell tokenization') from exc
        if tr in line:
            _need(tr in tokens,'R8 target appears only as an attached recipe token')
            if 'tools/ido/cc' in tokens:
                compiler.append((line,tokens))
            else:
                # Unknown postprocessors that mention the candidate are part
                # of the footprint and cannot be silently discarded.
                raise BindingError('unreviewed R8 postprocessor names the candidate object')
    _need(len(compiler)==1,'R8 NON_MATCHING compiler recipe is absent or ambiguous')
    line,tokens=compiler[0]
    args=batch.compiler_arguments(line,sr,tr)
    return raw,line,args,hashlib.sha256(raw.encode()).hexdigest()


def _r8_snapshot(source, configured, resolution, deadline):
    """Fresh complete full-TU source, compiler, dependency, and link closure."""
    fp.require_fresh_evidence(resolution)
    target=configured.relative_to(ROOT).as_posix(); sr=source.relative_to(ROOT).as_posix()
    raw_recipe,command,_args,recipe_sha=_r8_recipe(source,configured,deadline)
    compiler_args=batch.compiler_arguments(command,sr,target)
    dependencies=batch.source_dependencies(source,compiler_args,deadline)
    _need(not any(name.startswith('missing:') for name in dependencies),
          'R8 source dependency closure contains a missing include')
    dependency_files={}
    for relative,expected in dependencies.items():
        path=ROOT/relative
        _need(path.is_file() and pp.sha256_file(path)==expected,
              f'R8 source dependency changed during evidence capture: {relative}')
        dependency_files[relative]=expected
    target_paths={
      'linked_elf':fp.TARGET_ELF,'link_map':ROOT/'build/mickey.us.map','rom':fp.ROM,
      'candidate_object':configured,'normal_split_stamp':ROOT/'build/.splat-stamp',
      'candidate_split_stamp':ROOT/'build_non_matching/.splat-stamp',
      'overlay_config':ROOT/'config/overlays.us.json','yaml':ROOT/'mickey.us.yaml',
      'linker_script':ROOT/'mickey.us.ld','undefined_symbols':ROOT/'overlay_undefined_syms.us.txt',
      'symbol_addresses':ROOT/'symbol_addrs.us.txt',
    }
    target_hashes={name:pp.sha256_file(path) for name,path in target_paths.items()}
    makefiles=[ROOT/'Makefile',*sorted((ROOT/'mk').glob('**/*.mk'))]
    makefile_hashes={path.relative_to(ROOT).as_posix():pp.sha256_file(path) for path in makefiles}
    tools=batch.checked_tool_identity()
    preprocessed=batch.bounded_capture(['tools/ido/cc',*[a for a in compiler_args if a!='-c'],
      '-E',sr],deadline,check=True).stdout
    snapshot={'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
      'source_sha256':pp.sha256_file(source),'configured_object_sha256':pp.sha256_file(configured),
      'recipe_sha256':recipe_sha,'recipe_raw_sha256':hashlib.sha256(raw_recipe.encode()).hexdigest(),
      'compiler_command':command,'dependencies':dependencies,'dependency_files':dependency_files,
      'target_inputs':target_hashes,'makefiles':makefile_hashes,'tools':tools,
      'preprocessed_sha256':hashlib.sha256(preprocessed.encode()).hexdigest()}
    return snapshot,raw_recipe,command,compiler_args,preprocessed


def _assert_same_candidate_snapshot(before,after):
    _need(before==after,'R8 source/compiler/dependency/link closure changed during joint binding proof')


def _assert_same_joint_view(before,after):
    fields=('key','function','source','source_sha256','configured_recipe_fingerprint',
      'configured_object_sha256','owned_size','owned_bytes_sha256','owner','semantic_view',
      'loader_view','linked_rom_sha256','freshness_before','freshness_after')
    _need(_json_stable({field:before.get(field) for field in fields})==
          _json_stable({field:after.get(field) for field in fields}),
          'physical owner, loader, linked bytes, or input closure changed across the joint proof window')
    _need(_json_stable(before.get('freshness_after'))==_json_stable(after.get('freshness_before')),
          'Q80 physical witness closure changed between pre-candidate and final checks')


def _assert_candidate_view_closure(candidate,witness):
    closure=witness['freshness_after']; targets=candidate['freshness_after']['target_inputs']
    _need(targets['linked_elf']==closure['linked_elf'] and
          targets['link_map']==closure['map'] and targets['rom']==closure['rom'] and
          candidate['freshness_after']['tools']==closure['tools'] and
          candidate['freshness_after']['makefiles']==closure['makefiles'],
          'R8 candidate and named storage witness did not share one final linked/tool/recipe closure')


def _candidate_capture(out):
    resolution=fp.resolve(FUNCTION)
    source=resolution.source.resolve(); configured=resolution.candidate_object.resolve()
    _need(source==ROOT/SOURCE_REL,'R8 source/TU does not match the fixed candidate scope')
    _need(pp.sha256_file(source)==SOURCE_SHA256,'R8 source differs from the reviewed fixed source pin')
    _need(resolution.translation_unit=='overlays/o008/overlay_008' and
          resolution.candidate_build_dir=='build_non_matching' and
          resolution.selection=='-DNON_MATCHING selects the guarded C body',
          'R8 is not the reviewed guarded-C translation-unit selection')
    deadline=time.monotonic()+240
    before,recipe,command,original,cpp_before=_r8_snapshot(source,configured,resolution,deadline)
    raw=out/'r8-stock-c.o'
    actual=['tools/ido/cc',*original,'-o',str(raw),SOURCE_REL]
    result=batch.bounded_capture(actual,deadline,check=True)
    (out/'r8-stock-c-command.json').write_text(json.dumps(actual,indent=2)+'\n')
    (out/'r8-stock-c-compile.log').write_text(result.stdout)
    (out/'r8-preprocessed-before.c').write_text(cpp_before)
    fidelity=sf.compare_source_symbols(raw,configured,resolution.candidate_symbol)
    _need(fidelity['runtime_identity_proved'] is False and fidelity['promotion_authority'] is False,
          'source-fidelity helper returned unexpected identity/promotion authority')
    raw_elf,cfg_elf=rs.Elf(raw),rs.Elf(configured)
    definitions={}
    source_lines={}
    for key,spec in BINDINGS.items():
        source_lines[key]=_source_declaration(source,spec)
        left,right=_carrier_record(raw_elf,spec),_carrier_record(cfg_elf,spec)
        _need(left==right,'stock-C and configured carrier symbol records differ')
        definitions[key]=left
        _need(any(row['source_name']==spec['carrier'] for row in fidelity['relocations']),
              'carrier is not in the exact ordered function relocation surface')
    # Compare the complete ordered function relocation tuples and bytes.  The
    # fidelity tool additionally checks REL addends and original-name owners.
    raw_fn=sf._function(raw_elf,raw_elf.symbols(),resolution.candidate_symbol)
    cfg_fn=sf._function(cfg_elf,cfg_elf.symbols(),resolution.candidate_symbol)
    _need(raw_fn['size']==cfg_fn['size'],'stock-C and configured R8 function extents differ')
    _need(raw_fn['size']==5036,'R8 owned function no longer has the reviewed 5,036-byte boundary')
    _need(sf._normalized_bytes(raw_elf,raw_fn)==sf._normalized_bytes(cfg_elf,cfg_fn),
          'stock-C and configured R8 function instruction fields differ')
    raw_rows=sf._relocations(raw_elf,sf._symtab_index(raw_elf),raw_fn,raw_elf.symbols())
    cfg_rows=sf._relocations(cfg_elf,sf._symtab_index(cfg_elf),cfg_fn,cfg_elf.symbols())
    _need(raw_rows==cfg_rows,'complete ordered R8 relocation graph differs from stock C')
    _need(len(raw_rows)==137,'R8 complete ordered relocation surface differs from reviewed 137 records')
    text_section=raw_elf.section_bytes(raw_elf.names[raw_fn['section']])
    owned_bytes=text_section[raw_fn['start']:raw_fn['start']+raw_fn['size']]
    _need(len(owned_bytes)==raw_fn['size'],'stock-C owned function bytes are truncated')
    after,fresh_recipe,_,_,cpp_after=_r8_snapshot(source,configured,resolution,deadline)
    _assert_same_candidate_snapshot(before,after)
    _need(cpp_before==cpp_after,'R8 configured NON_MATCHING preprocessing changed during capture')
    context=cc.compare_context(cpp_before.encode(),cpp_after.encode(),FUNCTION)
    _need(context['status']=='unchanged' and
          context['baseline_context_sha256']==context['winner_context_sha256'],
          'R8 compiler self-context is not unchanged')
    (out/'r8-preprocessed-after.c').write_text(cpp_after)
    _need(fresh_recipe==recipe,'R8 full configured compiler recipe changed during source-fidelity capture')
    raw_object_sha=pp.sha256_file(raw)
    _need(pp.sha256_file(raw)==raw_object_sha,'retained R8 stock-C object changed during capture')
    return {'resolution':resolution,'source':source,'configured':configured,'raw':raw,
      'source_sha256':pp.sha256_file(source),'configured_sha256':pp.sha256_file(configured),
      'raw_sha256':raw_object_sha,'recipe_sha256':before['recipe_sha256'],
      'recipe_fingerprint':before['recipe_sha256'],'definitions':definitions,'source_declaration_lines':source_lines,
      'function_size':raw_fn['size'],'function_bytes_sha256':hashlib.sha256(owned_bytes).hexdigest(),
      'function_relocations':len(raw_rows),
      'ordered_relocations':raw_rows,'fidelity':fidelity,'freshness_before':before,
      'freshness_after':after,'preprocessed_self_context':context,
      'preprocessed_sha256':before['preprocessed_sha256']}


def collect(key: str):
    """Capture one fixed binding; callers may supply only its registry key."""
    if key not in BINDINGS:
        raise BindingError('unknown fixed binding key')
    _verify_loaded()
    root=ROOT/'build/resident-storage-bindings'; root.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix=key+'-',dir=root))
    try:
        witness=view.collect(key)
        _need(witness.get('status')=='scoped-independent-witness-complete' and
              witness.get('resolver_admission') is False and witness.get('matching_credit')==0,
              'resident-storage witness is incomplete or claims prohibited authority')
        spec=BINDINGS[key]
        _need(witness.get('source','').endswith(spec['section']+'.c'),
              'source witness translation unit does not match fixed authority')
        view_spec=spec['view']; semantic=witness.get('semantic_view',{})
        if key=='gravity':
            lv=witness.get('loader_view',{})
            _need(lv.get('selector')==spec['identity'][0] and lv.get('offset')==spec['identity'][1] and
                  lv.get('physical_address')==witness.get('linked_owner',{}).get('address'),
                  'typed NOBITS loader identity does not resolve to its named owner')
            _need(witness.get('bss_bytes_read') is False and witness.get('rom_zero_comparison_performed') is False,
                  'gravity NOBITS witness improperly treated BSS as ROM bytes')
            owner_record=witness['linked_owner']
            _need(owner_record.get('name')==spec['owner'] and owner_record.get('size')==spec['size'] and
                  owner_record.get('section')=='.main_bss','gravity owner is not the exact typed NOBITS object')
        else:
            _need((semantic.get('type'),semantic.get('offset'),semantic.get('extent'),semantic.get('physical_object_extent'))==view_spec,
                  'resident gate semantic byte view differs from fixed authority')
            lv=witness.get('loader_view',{})
            _need(lv.get('selector')==spec['identity'][0] and lv.get('offset')==spec['identity'][1] and
                  lv.get('physical_address')==witness.get('owner',{}).get('address'),
                  'resident gate loader identity does not resolve to its named owner')
            owner_record=witness['owner']
            _need(owner_record.get('symbol')==spec['owner'] and owner_record.get('size')==4 and
                  owner_record.get('input_section')=='.data',
                  'resident gate is not the exact named initialized four-byte owner')
        candidate=_candidate_capture(out)
        final_witness=view.collect(key)
        _assert_same_joint_view(witness,final_witness)
        _assert_candidate_view_closure(candidate,final_witness)
        _verify_loaded()
        # Opaque recheck IDs are generated here; no caller-provided path or
        # identity can redirect a later check.
        recheck_id=hashlib.sha256((str(time.time_ns())+key+candidate['raw_sha256']).encode()).hexdigest()
        packet={'schema':'mickey-resident-storage-binding-v1','status':'fixed-independent-binding-complete',
          'key':key,'candidate_function':FUNCTION,'candidate_source':SOURCE_REL,
          'carrier':spec['carrier'],'carrier_type':spec['ctype'],'carrier_size':spec['size'],
          'candidate_symbol_records':candidate['definitions'],'source_declaration_line':candidate['source_declaration_lines'][key],
          'original_identity':list(spec['identity']),'original_owner':spec['owner'],
          'physical_owner':owner_record,'view_witness':witness,'final_witness':final_witness,
          'candidate_stock_c':{'object_sha256':candidate['raw_sha256'],'configured_object_sha256':candidate['configured_sha256'],
             'source_sha256':candidate['source_sha256'],'recipe_sha256':candidate['recipe_sha256'],
             'recipe_fingerprint':candidate['recipe_fingerprint'],'function_size':candidate['function_size'],
             'function_bytes_sha256':candidate['function_bytes_sha256'],
             'ordered_relocations':candidate['ordered_relocations'],'source_fidelity':candidate['fidelity']},
          'identity_authority':{'path':'Q69-STORAGE-ADAPTER-ARCHITECTURE-20261001/proposed-bindings.json',
             'sha256':AUTHORITY_SHA256,'route':'fixed original namespace record; no target-site correlation'},
          'adapter_sha256':pp.sha256_file(Path(__file__)),
          'resolver_admission':False,'matching_credit':0,'recheck_handle':recheck_id,
          'artifacts':{'binding_report':None,'stock_c_object':None,'configured_object':None}}
        for name,src in [('stock-c.o',candidate['raw']),('configured.o',candidate['configured'])]:
            shutil.copyfile(src,out/name)
        packet['artifacts']['binding_report']=(out/'binding.json').relative_to(ROOT).as_posix()
        packet['artifacts']['stock_c_object']=(out/'stock-c.o').relative_to(ROOT).as_posix()
        packet['artifacts']['configured_object']=(out/'configured.o').relative_to(ROOT).as_posix()
        (out/'binding.json').write_text(json.dumps(packet,indent=2,sort_keys=True)+'\n')
        return packet
    except BaseException as exc:
        (out/'failure.json').write_text(json.dumps({'schema':'mickey-resident-storage-binding-failure-v1',
             'key':key,'status':'blocked','error_type':type(exc).__name__,'error':str(exc)},indent=2)+'\n')
        raise


def recheck(handle: str):
    """Repeat a fixed capture for an internally issued opaque handle."""
    _need(isinstance(handle,str) and re.fullmatch(r'[0-9a-f]{64}',handle) is not None,
          'invalid opaque recheck handle')
    root=ROOT/'build/resident-storage-bindings'
    matches=[]
    for path in root.glob('*/binding.json'):
        try:
            packet=json.loads(path.read_text())
            if packet.get('recheck_handle')==handle: matches.append((path.resolve(),packet))
        except (OSError,json.JSONDecodeError):
            continue
    _need(len(matches)==1,'opaque recheck handle is unknown or ambiguous')
    report_path,old=matches[0]
    _need(report_path==ROOT/old['artifacts']['binding_report'] and
          report_path.parent.parent==root.resolve(),
          'opaque handle receipt is outside the internally owned capture directory')
    retained_raw=report_path.parent/'stock-c.o'; retained_cfg=report_path.parent/'configured.o'
    _need(old['artifacts']['stock_c_object']==retained_raw.relative_to(ROOT).as_posix() and
          old['artifacts']['configured_object']==retained_cfg.relative_to(ROOT).as_posix() and
          pp.sha256_file(retained_raw)==old['candidate_stock_c']['object_sha256'] and
          pp.sha256_file(retained_cfg)==old['candidate_stock_c']['configured_object_sha256'],
          'retained stock-C/configured object artifact changed since capture')
    fresh=collect(old['key'])
    stable_fields=('configured_object_sha256','source_sha256','recipe_sha256','recipe_fingerprint',
                   'function_size','function_bytes_sha256','ordered_relocations','source_fidelity')
    stable_candidate={name:fresh['candidate_stock_c'][name] for name in stable_fields}
    old_candidate={name:old['candidate_stock_c'][name] for name in stable_fields}
    _need(old.get('adapter_sha256')==pp.sha256_file(Path(__file__)) and
          fresh['original_identity']==old['original_identity'] and fresh['carrier']==old['carrier'] and
          fresh['original_owner']==old['original_owner'] and
          fresh['carrier_type']==old['carrier_type'] and fresh['carrier_size']==old['carrier_size'] and
          fresh['physical_owner']==old['physical_owner'] and
          _json_stable(stable_candidate)==old_candidate and
          _json_stable(fresh['view_witness']['freshness_before'])==old['view_witness']['freshness_before'] and
          _json_stable(fresh['view_witness']['freshness_after'])==old['view_witness']['freshness_after'] and
          _json_stable(fresh['final_witness']['freshness_before'])==old['final_witness']['freshness_before'] and
          _json_stable(fresh['final_witness']['freshness_after'])==old['final_witness']['freshness_after'],
          'fresh recheck no longer agrees with captured fixed binding')
    return {'status':'fresh-fixed-binding-recheck-complete','key':old['key'],
            'identity':fresh['original_identity'],'recheck_report':fresh['artifacts']['binding_report'],
            'matching_credit':0,'resolver_admission':False}
