#!/usr/bin/env python3
"""Capture three fixed, independently sourced resident-storage witnesses.

This is a scoped evidence reporter.  It accepts only a registry key, owns its
capture directory, never accepts a candidate object/address, and never changes
relocation identity or promotion verdicts.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import candidate_context as cc
import function_preflight as fp
import overlay_tables as ot
import permute_batch as batch
import proof_provenance as pp
import reloc_surface as rs
import source_symbol_fidelity as sf
import storage_view as sv
import sweep_receipts as receipts

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / 'tools'
LOADER_BODY = sv.LOADER_BODY
LOADER_RECIPE = '0c80ca4b26661342a8eb479d905aa636e31b01980b911b8c4a43f67d4017b306'
SOURCE_PINS = {
    'impact_gate': ('src/main/anim.c', 'e75ea3d245cf5076d769b603ff82fc2a50ce04c7d1a100d91de4ea4d0898dfc3'),
    'color_gate': ('src/main/objects.c', '74081eadd056007b818a6ec79a15ca778307999fda423967692f6b4a68a18416'),
    'gravity': ('src/main/charControl.c', 'ea6052dcc1764229acd9ab1778ecb17a233d2b19e275e3686082dc83a9d8b44f'),
}
WITNESSES = {
    'impact_gate': {'function':'func_80050348','owner':'D_8007BF04','external':'D_8007BF04','selector':0xFFD,'offset':0x31A4,'size':532,'relocs':22,'uses':3,'recipe':'c9ae34a06b5294868c9bb0a05f888ab6b7942dbf9c6861830bf560a1208858e6','renames':('animResetTrap','hitCopyFirstTrap','TrapDanglingJump')},
    'color_gate': {'function':'func_80005548','owner':'D_8007BF0C','external':'D_8007BF0C','selector':0xFFD,'offset':0x31AC,'size':348,'relocs':3,'uses':1,'recipe':'83b348a05f2391a764a7c64885b2439570eb112d049fef3ac8cc4207088bdc16','renames':('objectsSizeDefaultBranch','objectsInitDefaultBranch','objectsControlDefaultBranch','objectsSwitchTablesBase')},
    'gravity': {'function':'', 'owner':'D_800CB304','source':'src/main/charControl.c','address':0x800CB304,'size':4,'recipe':'22174bb0b30080a22443639d4e286bee8d82f88c67afc9bb40e48be8f6dde8bf'},
}
LOADED = {Path(m.__file__): pp.sha256_file(Path(m.__file__)) for m in
          (fp, ot, batch, pp, rs, sf, sv, cc, receipts)}
LOADED[Path(__file__)] = pp.sha256_file(Path(__file__))
EXECUTED_HELPERS = (ROOT / 'tools/trim_elf_bss.py', ROOT / 'tools/trim_elf_section.py',
                    ROOT / 'tools/add_elf_relocations.py', ROOT / 'tools/rebind_elf_relocations.py',
                    ROOT / 'tools/binutils/mips64-elf-objcopy')
for _helper in EXECUTED_HELPERS:
    if _helper.is_file():
        LOADED[_helper] = pp.sha256_file(_helper)


class EvidenceError(RuntimeError):
    pass


def need(test, why):
    if not test:
        raise EvidenceError(why)


def _verify_loaded_proofs():
    need(all(pp.sha256_file(path)==expected for path,expected in LOADED.items()),
         'loaded evidence implementation changed')


def _recipe(source: Path, target: Path, deadline):
    sr = source.relative_to(ROOT).as_posix(); tr = target.relative_to(ROOT).as_posix()
    raw = batch.bounded_capture(['gmake','--no-print-directory','-n','-W',sr,tr],deadline,check=True).stdout
    lines=[line.strip() for line in raw.replace('\\\n',' ').splitlines() if line.strip()]
    target_lines=[]
    for line in lines:
        try: tokens=shlex.split(line)
        except ValueError as error: raise EvidenceError('configured target recipe has invalid shell tokenization') from error
        if tr in line:
            need(tr in tokens,'target path appears only as an attached or shell-adjacent recipe token')
            target_lines.append((line,tokens))
    commands=[(line,tokens) for line,tokens in target_lines if 'tools/ido/cc' in tokens]
    metadata=[line for line,tokens in target_lines if 'tools/ido/cc' not in tokens]
    need(len(commands)==1,'configured compiler recipe is ambiguous')
    # Every other dry-run command that names this object is part of the reviewed
    # footprint. Unknown commands are retained and therefore fail the pin.
    for line in metadata:
        try: tokens=shlex.split(line)
        except ValueError as error: raise EvidenceError('target metadata command has invalid shell tokenization') from error
        need(not any(token!='&&' and any(char in token for char in ';|><$`') for token in tokens),
             'unsupported shell construct in target metadata recipe')
        need(tokens and tokens[0] != '&&' and tokens[-1] != '&&',
             'malformed target metadata command chain')
        segments=line.split('&&')
        need(all(segment.strip() for segment in segments),'empty target metadata command in chain')
        need(all(tr in shlex.split(segment) for segment in segments),
             'metadata command in recipe does not explicitly name the target object')
    compiler_line,words=commands[0]
    normalized=list(words); need(normalized.count('-o')==1,'compiler output argument is ambiguous')
    normalized[normalized.index('-o')+1]='<OUTPUT>'
    postprocess=[shlex.split(line) for line in metadata]
    fingerprint=hashlib.sha256(json.dumps({'compiler':normalized,'postprocess_commands':postprocess},sort_keys=True,separators=(',',':')).encode()).hexdigest()
    # Preserve the complete command sequence in the input snapshot, including
    # commands introduced on a separate Make recipe line.
    metadata_text='\n'.join(metadata)
    return raw,compiler_line,metadata_text,words,fingerprint


def _common_context(source, configured, recipe_text, command, metadata,
                    dependency_specs, proof_paths, extra_paths, deadline,
                    recipe_specs, expected_recipes):
    _verify_loaded_proofs()
    tool_identity=batch.checked_tool_identity()
    live_recipes={name:_recipe(recipe_source,recipe_target,deadline)[0]
                  for name,(recipe_source,recipe_target) in recipe_specs.items()}
    _assert_same_snapshot(expected_recipes,live_recipes,'configured recipes')
    dependencies={name:batch.source_dependencies(dep_source,args,deadline)
                  for name,(dep_source,args) in dependency_specs.items()}
    need(not any(path.startswith('missing:') for closure in dependencies.values() for path in closure),
         'source dependency closure contains a missing input')
    dependency_files={}
    for closure in dependencies.values():
        for relative,expected in closure.items():
            path=ROOT/relative
            need(path.is_file() and pp.sha256_file(path)==expected,
                 f'source dependency changed during evidence capture: {relative}')
            dependency_files[relative]=expected
    pinned_paths={name:pp.sha256_file(path) for name,path in extra_paths.items()}
    return {
      'git_head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
      'source':pp.sha256_file(source),'configured_object':pp.sha256_file(configured),
      'linked_elf':pp.sha256_file(fp.TARGET_ELF),'map':pp.sha256_file(ROOT/'build/mickey.us.map'),
      'rom':pp.sha256_file(fp.ROM),'recipe_text_sha256':{name:hashlib.sha256(value.encode()).hexdigest() for name,value in live_recipes.items()},
      'compiler_command':command,'metadata_commands':metadata.splitlines(),'dependencies':dependencies,
      'dependency_files':dependency_files,'additional_input_files':pinned_paths,
      'include_tree':receipts.tree_digest(ROOT/'include'),'source_directory':receipts.tree_digest(source.parent),
      'tools':tool_identity,'asm_processor':receipts.tree_digest(ROOT/'tools/asm-processor'),
      'makefiles':{p.relative_to(ROOT).as_posix():pp.sha256_file(p) for p in [ROOT/'Makefile',*sorted((ROOT/'mk').glob('**/*.mk'))]},
      'proof_tools':{str(p.relative_to(ROOT)):pp.sha256_file(p) for p in proof_paths},
      'target_inputs':{n:pp.sha256_file(ROOT/n) for n in ('baseroms/mickey.us.z64','config/overlays.us.json','mickey.us.yaml','mickey.us.ld','overlay_undefined_syms.us.txt','symbol_addrs.us.txt')},
    }


def _assert_same_snapshot(before, after, label):
    need(before==after,f'{label} evidence snapshot changed during capture')


def _preprocess(args, source_rel, deadline):
    return batch.bounded_capture(['tools/ido/cc',*[a for a in args if a!='-c'],'-E',source_rel],deadline,check=True).stdout


def _global_object_declaration(text, name, ctype):
    masked=pp._mask_c(text)
    pattern=re.compile(rf'^\s*{re.escape(ctype)}\s+{re.escape(name)}\s*;\s*$',re.M)
    rows=list(pattern.finditer(masked))
    # A data object is a declaration, not the function-only source_facts API.
    need(len(rows)==1,'typed global owner declaration is missing or ambiguous')
    line=masked.count('\n',0,rows[0].start())+1
    prefix='\n'.join(masked.splitlines()[:line-1])
    depth=0
    for directive in re.findall(r'^\s*#\s*(if|ifdef|ifndef|endif)\b',prefix,re.M):
        depth += 1 if directive in ('if','ifdef','ifndef') else -1
    need(depth==0,'typed global owner is conditionally compiled')
    return line


def _require_recipe(key, actual):
    need(actual==WITNESSES[key]['recipe'],'configured recipe does not match reviewed storage footprint')


def _require_byte_zero_load(text, site):
    need(site>=0 and site+4<=len(text) and site%4==0,'named storage use lies outside instruction section')
    word=struct.unpack_from('>I',text,site)[0]
    need(word>>26==36 and (word&0xffff)==0,'named storage use is not LBU at owner offset zero')
    return {'section_offset':site,'opcode':'lbu','owner_offset':0}


def _owned_function(raw, cfg, function, size, expected_relocs, expected_use, external, renamed):
    fidelity=sf.compare_source_symbols(raw.path,cfg.path,function)
    need(fidelity['owned_size']==size and fidelity['relocation_count']==expected_relocs,'owned source-fidelity size or relocation count differs')
    rf=sv.unique_symbol(raw,function); cf=sv.unique_symbol(cfg,function)
    need(rf[:4]==cf[:4] and raw.names[rf[4]]==cfg.names[cf[4]]=='.text','configured function boundary differs from ordinary stock C')
    start=rf[1]; rb=raw.section_bytes('.text')[start:start+size]; cb=cfg.section_bytes('.text')[start:start+size]
    need(len(rb)==size and rb==cb,'owned raw C bytes differ from configured object')
    def graph(elf):
        syms=elf.symbols(); out=[]
        for sec,site,kind,index in elf.relocations():
            if sec!='.text' or not start<=site<start+size: continue
            s=syms[index]; secname=elf.names[s[4]] if 0<s[4]<len(elf.names) else ('UND' if s[4]==0 else 'ABS')
            need(kind!=rs.R_MIPS_PC16 and secname!='.rodata' and s[0] not in renamed,'owned source relocation intersects omitted metadata or unsupported section')
            out.append((site,kind,s[0],secname))
        return out
    raw_graph,cfg_graph=graph(raw),graph(cfg)
    need(raw_graph==cfg_graph and len(raw_graph)==expected_relocs,'complete ordered owned relocation graph changed')
    # Validate the full-TU implicit-addend pairing, not merely a relocation
    # tuple multiset. Any pair crossing the function boundary is disqualifying.
    fidelity_rows={int(r['site'][1:],16):r for r in fidelity['relocations']}
    def pair_graph(elf):
        fn=sv.unique_symbol(elf,function); begin,end=fn[1],fn[1]+fn[2]
        symbols=elf.symbols(); text=elf.section_bytes('.text'); pending={}; groups=[]; covered=[]; sequence=[]
        def owned(site): return begin<=site<end
        for sec,site,kind,index in elf.relocations():
            if sec!='.text': continue
            name=symbols[index][0]; word=struct.unpack_from('>I',text,site)[0]
            if owned(site): sequence.append([site-begin,kind,name])
            if kind==rs.R_MIPS_HI16:
                pending.setdefault(name,[]).append((site,word&0xffff))
            elif kind==rs.R_MIPS_LO16:
                highs=pending.pop(name,[]); sites=[s for s,_ in highs]+[site]
                if not any(owned(s) for s in sites): continue
                need(all(owned(s) for s in sites),'HI16/LO16 relocation group crosses function boundary')
                low=word&0xffff; low=low-65536 if low&0x8000 else low
                addends={((v<<16)+low)&0xffffffff for _,v in highs} if highs else {low&0xffffffff}
                need(len(addends)==1,'paired relocation implicit addends disagree')
                addend=next(iter(addends)); rel=[s-begin for s in sites]
                for site_rel in rel:
                    row=fidelity_rows.get(site_rel)
                    need(row is not None and row['source_name']==name and row['owner_relative_addend']==addend,
                         'source-fidelity owner/addend differs from full-TU pair graph')
                covered.extend(rel)
                groups.append({'type':'HI16-group/LO16' if highs else 'standalone-LO16','name':name,
                    'high_sites':[s-begin for s,_ in highs],'low_site':site-begin,'addend':addend,
                    'use_word_sha256':[hashlib.sha256(text[s:s+4]).hexdigest() for s in sites]})
            elif owned(site):
                need(kind==rs.R_MIPS_26,'unsupported owned relocation kind')
                addend=(word&0x3ffffff)<<2; row=fidelity_rows.get(site-begin)
                need(row is not None and row['owner_relative_addend']==addend,'R_MIPS_26 owner/addend mismatch')
                covered.append(site-begin); groups.append({'type':'R_MIPS_26','name':name,'site':site-begin,'addend':addend,
                    'use_word_sha256':hashlib.sha256(text[site:site+4]).hexdigest()})
        need(not any(owned(site) for rows in pending.values() for site,_ in rows),'owned pending HI16 relocation has no LO16')
        need(sorted(covered)==sorted(fidelity_rows) and len(covered)==len(set(covered)),
             'full-TU pair graph does not cover each source-fidelity record exactly once')
        return {'ordered_owned_tuples':sequence,'groups':groups,'covered_records':len(covered)}
    need(pair_graph(raw)==pair_graph(cfg),'ordered relocation grouping/addends/use words changed')
    sites=[(site,kind) for site,kind,name,sec in raw_graph if name==external and kind==rs.R_MIPS_LO16]
    need(len(sites)==expected_use,'named owner use count differs')
    uses=[]; text=raw.section_bytes('.text')
    for site,kind in sites:
        need(kind==rs.R_MIPS_LO16,'named storage use is not the reviewed paired low relocation')
        use=_require_byte_zero_load(text,site); use['function_offset']=site-start; uses.append(use)
    return fidelity,start,hashlib.sha256(rb).hexdigest(),uses


def _gate(key, out, deadline):
    spec=WITNESSES[key]; function=spec['function']
    resolution=fp.resolve(function); source=resolution.source; cfg=resolution.candidate_object
    need(resolution.resolution_mode=='post_promotion','gate source is not canonical exact C')
    fp.require_fresh_evidence(resolution)
    source_rel=source.relative_to(ROOT).as_posix(); target=cfg.relative_to(ROOT).as_posix()
    recipe_text,command,metadata,words,fp_hash=_recipe(source,cfg,deadline)
    _require_recipe(key,fp_hash)
    compiler_args=batch.compiler_arguments(command,source_rel,target)
    dep_args=compiler_args+(('-I',str(source.parent)) if len(words)>1 and words[1]=='tools/asm-processor/build.py' else ())
    loader_res=fp.resolve('ResolveRelocAddress'); fp.require_fresh_evidence(loader_res)
    loader_source=loader_res.source; loader_cfg=loader_res.candidate_object
    loader_rel=loader_source.relative_to(ROOT).as_posix(); loader_target=loader_cfg.relative_to(ROOT).as_posix()
    loader_recipe_text,loader_command,loader_metadata,loader_words,_=_recipe(loader_source,loader_cfg,deadline)
    need(loader_rel=='src/main/runlink.c' and _recipe(loader_source,loader_cfg,deadline)[4]==LOADER_RECIPE,
         'reviewed loader source or full compiler recipe changed')
    loader_args=batch.compiler_arguments(loader_command,loader_rel,loader_target)
    loader_deps_args=loader_args+(('-I',str(loader_source.parent)) if len(loader_words)>1 and loader_words[1]=='tools/asm-processor/build.py' else ())
    dependency_specs={'target':(source,dep_args),'loader':(loader_source,loader_deps_args)}
    recipe_specs={'target':(source,cfg),'loader':(loader_source,loader_cfg)}
    expected_recipes={'target':recipe_text,'loader':loader_recipe_text}
    owner_before=sv.resident_owner(rs.Elf(fp.TARGET_ELF),fp.ROM.read_bytes(),spec['owner'])
    extra_paths={'loader_source':loader_source,'loader_configured_object':loader_cfg,
                 'initialized_owner_input_object':ROOT/owner_before['input_object']}
    proof_paths=[Path(__file__),Path(fp.__file__),Path(pp.__file__),Path(rs.__file__),Path(sf.__file__),Path(sv.__file__),Path(batch.__file__),Path(cc.__file__),Path(receipts.__file__),*EXECUTED_HELPERS]
    before=_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes)
    need(hashlib.sha256(source.read_bytes()).hexdigest()==SOURCE_PINS[key][1],'source differs from reviewed exact source pin')
    text=source.read_text(); facts=pp.source_facts(text,function)
    need(len(facts.definitions)==1 and not facts.pragmas,'function is not one ordinary C definition')
    cpp_before=_preprocess(compiler_args,source_rel,deadline)
    c_facts=pp.source_facts(cpp_before,function)
    need(len(c_facts.definitions)==1 and not c_facts.pragmas,'preprocessed function is not ordinary C')
    shutil.copyfile(source,out/'source.c'); shutil.copyfile(cfg,out/'configured.o')
    shutil.copyfile(loader_source,out/'loader-source.c'); shutil.copyfile(loader_cfg,out/'loader-configured.o')
    shutil.copyfile(ROOT/owner_before['input_object'],out/'initialized-owner-input.o')
    (out/'preprocessed-before.c').write_text(cpp_before)
    loader_cpp_before=_preprocess(loader_args,loader_rel,deadline)
    (out/'loader-preprocessed-before.c').write_text(loader_cpp_before)
    raw_path=out/'stock-c.o'; run=list(words); run[run.index('-o')+1]=str(raw_path)
    (out/'compiler-command.json').write_text(json.dumps(run,indent=2)+'\n')
    result=batch.bounded_capture(run,deadline,check=True); (out/'compile.log').write_text(result.stdout)
    raw_object_sha=pp.sha256_file(raw_path)
    need(pp.sha256_file(out/'initialized-owner-input.o')==owner_before['object_sha256'],
         'retained initialized-owner object differs from linked proof input')
    _assert_same_snapshot(before,_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes),'source/tool/build')
    raw,cfgelf=rs.Elf(raw_path),rs.Elf(cfg)
    fidelity,start,owned_sha,uses=_owned_function(raw,cfgelf,function,spec['size'],spec['relocs'],spec['uses'],spec['external'],spec['renames'])
    fp.require_fresh_evidence(resolution)
    preflight=fp.collect(resolution,no_build=True)
    need(preflight['preflight']['status']=='complete' and preflight['workbench']['differing_words']==0,'fresh exact linked/ROM function proof failed')
    linked=rs.Elf(fp.TARGET_ELF); rom=fp.ROM.read_bytes(); owner=sv.resident_owner(linked,rom,spec['owner'])
    need(owner==owner_before and owner['size']==4,'gate owner changed or is not the authenticated four-byte initialized object')
    fp.require_fresh_evidence(loader_res)
    loader_pf=fp.collect(loader_res,no_build=True)
    need(loader_pf['preflight']['status']=='complete' and loader_pf['workbench']['differing_words']==0,'fresh exact loader proof failed')
    view=sv.loader_view((spec['selector'],spec['offset']),owner,linked,loader_cpp_before)
    need(view['physical_address']==owner['address'],'loader view did not resolve to named owner')
    sym=sv.unique_symbol(linked,resolution.target_symbol); address=sym[1]; section=sym[4]
    begin=address-linked.sh[section][3]; linked_bytes=linked.section_bytes(linked.names[section])[begin:begin+spec['size']]
    rom_bytes=rom[address-ot.VRAM_ROM_DELTA:address-ot.VRAM_ROM_DELTA+spec['size']]
    need(len(linked_bytes)==spec['size'] and linked_bytes==rom_bytes,'exact function bytes are not resident in retail ROM')
    cpp_after=_preprocess(compiler_args,source_rel,deadline)
    need(cpp_before==cpp_after,'preprocessed source changed during witness')
    (out/'preprocessed-after.c').write_text(cpp_after)
    loader_cpp_after=_preprocess(loader_args,loader_rel,deadline)
    need(loader_cpp_before==loader_cpp_after,'loader preprocessed source changed during witness')
    (out/'loader-preprocessed-after.c').write_text(loader_cpp_after)
    context=cc.compare_context(cpp_before.encode(),cpp_after.encode(),function)
    fp.require_fresh_evidence(resolution); fp.require_fresh_evidence(loader_res)
    after=_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes)
    _assert_same_snapshot(before,after,'fresh input')
    need(pp.sha256_file(raw_path)==raw_object_sha,'retained stock-C object changed after source-fidelity proof')
    report={'schema':'mickey-resident-storage-view-v1','key':key,'status':'scoped-independent-witness-complete',
      'resolver_admission':False,'matching_credit':0,'function':function,'source':source_rel,
      'source_sha256':before['source'],'configured_recipe_fingerprint':fp_hash,'stock_c_object_sha256':raw_object_sha,
      'configured_object_sha256':before['configured_object'],'owned_size':spec['size'],'owned_start':start,
      'owned_stock_configured_bytes_equal':True,'owned_bytes_sha256':owned_sha,'source_fidelity':fidelity,
      'complete_owned_relocation_count':spec['relocs'],'named_live_uses':uses,'owner':owner,
      'semantic_view':{'type':'u8','offset':0,'extent':1,'physical_object_extent':4},'loader_view':view,
      'fresh_target_preflight':preflight,'fresh_loader_preflight':loader_pf,
      'linked_rom_bytes':spec['size'],'linked_rom_sha256':hashlib.sha256(linked_bytes).hexdigest(),
      'preprocessed_self_context':context,'freshness_before':before,'freshness_after':after,
      'full_configured_postprocess_replayed':False,'metadata_footprint_disjoint_from_owned_function':True,
      'loader_preprocessed_source_sha256':hashlib.sha256(loader_cpp_before.encode()).hexdigest(),
      'recipe':{'compiler_command':command,'postprocess_commands':metadata.splitlines(),
                'loader_compiler_command':loader_command,'loader_postprocess_commands':loader_metadata.splitlines()}}
    (out/'report.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
    return report


def _gravity(out,deadline):
    spec=WITNESSES['gravity']; source=ROOT/spec['source']; cfg=ROOT/'build/src/main/charControl.c.o'; sr=source.relative_to(ROOT).as_posix(); tr=cfg.relative_to(ROOT).as_posix()
    recipe_text,command,metadata,words,fp_hash=_recipe(source,cfg,deadline); _require_recipe('gravity',fp_hash)
    args=batch.compiler_arguments(command,sr,tr)
    owner_res=fp.resolve('func_8001BBB4'); need(owner_res.source==source and owner_res.candidate_object==cfg,'gravity owner TU freshness symbol changed')
    fp.require_fresh_evidence(owner_res)
    loader_res=fp.resolve('ResolveRelocAddress'); fp.require_fresh_evidence(loader_res)
    loader_source=loader_res.source; loader_cfg=loader_res.candidate_object
    loader_rel=loader_source.relative_to(ROOT).as_posix(); loader_target=loader_cfg.relative_to(ROOT).as_posix()
    loader_recipe_text,loader_command,loader_metadata,loader_words,_=_recipe(loader_source,loader_cfg,deadline)
    need(loader_rel=='src/main/runlink.c' and _recipe(loader_source,loader_cfg,deadline)[4]==LOADER_RECIPE,
         'reviewed loader source or full compiler recipe changed')
    loader_args=batch.compiler_arguments(loader_command,loader_rel,loader_target)
    loader_deps_args=loader_args+(('-I',str(loader_source.parent)) if len(loader_words)>1 and loader_words[1]=='tools/asm-processor/build.py' else ())
    dependency_specs={'owner_tu':(source,args),'loader':(loader_source,loader_deps_args)}
    recipe_specs={'owner_tu':(source,cfg),'loader':(loader_source,loader_cfg)}
    expected_recipes={'owner_tu':recipe_text,'loader':loader_recipe_text}
    extra_paths={'owner_configured_object':cfg,'loader_source':loader_source,'loader_configured_object':loader_cfg}
    proof_paths=[Path(__file__),Path(pp.__file__),Path(rs.__file__),Path(batch.__file__),Path(sv.__file__),Path(fp.__file__),Path(receipts.__file__),*EXECUTED_HELPERS]
    before=_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes)
    need(before['source']==SOURCE_PINS['gravity'][1],'gravity source differs from reviewed exact pin')
    decl_line=_global_object_declaration(source.read_text(),'D_800CB304','f32')
    cpp_before=_preprocess(args,sr,deadline); _global_object_declaration(cpp_before,'D_800CB304','f32')
    shutil.copyfile(source,out/'source.c'); shutil.copyfile(cfg,out/'configured.o')
    shutil.copyfile(loader_source,out/'loader-source.c'); shutil.copyfile(loader_cfg,out/'loader-configured.o')
    (out/'preprocessed-before.c').write_text(cpp_before)
    loader_cpp_before=_preprocess(loader_args,loader_rel,deadline)
    (out/'loader-preprocessed-before.c').write_text(loader_cpp_before)
    rawpath=out/'stock-c.o'; run=list(words); run[run.index('-o')+1]=str(rawpath); (out/'compiler-command.json').write_text(json.dumps(run,indent=2)+'\n')
    result=batch.bounded_capture(run,deadline,check=True); (out/'compile.log').write_text(result.stdout)
    raw_object_sha=pp.sha256_file(rawpath)
    _assert_same_snapshot(before,_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes),'gravity source/tool/build')
    raw=rs.Elf(rawpath); bss_i=raw.names.index('.bss'); bss_header=raw.sh[bss_i]
    need(bss_header[1]==8 and bss_header[2]&3==3 and bss_header[5]==16 and bss_header[8]==16,'raw gravity BSS geometry is not the reviewed 16-byte aligned tail')
    symbol=sv.unique_symbol(raw,'D_800CB304'); need(symbol[4]==bss_i and symbol[1]==0 and symbol[2]==4 and symbol[3]&15==1,'raw gravity object is not one global four-byte f32 owner')
    need(not any(sec=='.bss' for sec,_,_,_ in raw.relocations()),'raw gravity BSS has relocations')
    replay=out/'normalized.o'; shutil.copyfile(rawpath,replay)
    trim=['python3','tools/trim_elf_bss.py',str(replay),'D_800CB304','4']
    trim_result=batch.bounded_capture(trim,deadline,check=True); (out/'trim.log').write_text(trim_result.stdout)
    normalized_object_sha=pp.sha256_file(replay)
    norm=rs.Elf(replay); nh=norm.sh[norm.names.index('.bss')]
    need(nh[5]==4 and nh[8]==4,'authorized trailing alignment normalization did not produce 4-byte BSS')
    # The comparator intentionally skips bytes for SHT_NOBITS; no NOBITS bytes are read.
    rs._reserved_witness_fidelity(norm,rs.Elf(cfg))
    fp.require_fresh_evidence(owner_res)
    owner_preflight=fp.collect(owner_res,no_build=True)
    need(owner_preflight['preflight']['status']=='complete' and owner_preflight['workbench']['differing_words']==0,'fresh charControl TU exact proof failed')
    fp.require_fresh_evidence(loader_res)
    loader_preflight=fp.collect(loader_res,no_build=True)
    need(loader_preflight['preflight']['status']=='complete' and loader_preflight['workbench']['differing_words']==0,'fresh exact loader/ROM proof failed')
    linked=rs.Elf(fp.TARGET_ELF); owner=sv.unique_symbol(linked,'D_800CB304'); need(owner[4]>0 and linked.names[owner[4]]=='.main_bss' and owner[1]==spec['address'] and owner[2]==4 and owner[3]&15==1,'linked gravity BSS owner differs')
    placements=rs.linked_input_sections(); matches=[(obj,sec,start,size) for (obj,sec),(start,size) in placements.items() if obj=='build/src/main/charControl.c.o' and sec=='.bss']
    need(matches==[('build/src/main/charControl.c.o','.bss',spec['address'],4)],'linked map does not place the normalized owner uniquely')
    need(sv.body_digest(loader_cpp_before,'ResolveRelocAddress')==LOADER_BODY,'reviewed loader BSS semantics changed')
    anchor=sv.unique_symbol(linked,'D_80085A40'); need(anchor[1]==0x80085A40 and anchor[1]+0x458C4==owner[1],'BSS loader anchor equation differs')
    yaml_text=(ROOT/'mickey.us.yaml').read_text()
    need(len(re.findall(r'^\s*- \{ type: \.bss, vram: 0x800CB304, name: main/charControl \}\s*$',yaml_text,re.M))==1,
         'canonical YAML does not name the exact gravity BSS origin')
    # BSS is NOBITS: deliberately no ROM byte access or zero comparison.
    cpp_after=_preprocess(args,sr,deadline); need(cpp_before==cpp_after,'gravity preprocessed source changed')
    (out/'preprocessed-after.c').write_text(cpp_after)
    loader_cpp_after=_preprocess(loader_args,loader_rel,deadline)
    need(loader_cpp_before==loader_cpp_after,'loader preprocessed source changed during gravity witness')
    (out/'loader-preprocessed-after.c').write_text(loader_cpp_after)
    ctx=cc.compare_context(cpp_before.encode(),cpp_after.encode(),'D_800CB304')
    fp.require_fresh_evidence(owner_res); fp.require_fresh_evidence(loader_res)
    after=_common_context(source,cfg,recipe_text,command,metadata,dependency_specs,proof_paths,extra_paths,deadline,recipe_specs,expected_recipes)
    _assert_same_snapshot(before,after,'gravity freshness')
    need(pp.sha256_file(rawpath)==raw_object_sha and pp.sha256_file(replay)==normalized_object_sha,
         'retained gravity raw or normalized object changed after proof')
    report={'schema':'mickey-resident-storage-view-v1','key':'gravity','status':'scoped-independent-witness-complete',
      'resolver_admission':False,'matching_credit':0,'source':sr,'source_sha256':before['source'],
      'configured_recipe_fingerprint':fp_hash,'raw_stock_object_sha256':raw_object_sha,
      'normalized_object_sha256':normalized_object_sha,'configured_object_sha256':before['configured_object'],'source_declaration_line':decl_line,
      'raw_bss':{'type':'SHT_NOBITS','size':16,'alignment':16,'owner_offset':0,'owner_size':4},
      'normalized_bss':{'type':'SHT_NOBITS','size':4,'alignment':4},'linked_owner':{'name':'D_800CB304','address':owner[1],'size':owner[2],'section':'.main_bss','input':matches[0][0]},
      'loader_view':{'selector':0xFFF,'offset':0x458C4,'anchor':'D_80085A40','anchor_address':anchor[1],'physical_address':owner[1],'loader_body_sha256':LOADER_BODY},
      'fresh_owner_tu_preflight':owner_preflight,'fresh_loader_preflight':loader_preflight,
      'preprocessed_self_context':ctx,'freshness_before':before,'freshness_after':after,
      'bss_bytes_read':False,'rom_zero_comparison_performed':False,
      'full_configured_postprocess_replayed':False,'approved_bss_normalization_replayed':True,
      'normalization_command':trim,'loader_preprocessed_source_sha256':hashlib.sha256(loader_cpp_before.encode()).hexdigest(),
      'recipe':{'compiler_command':command,'postprocess_commands':metadata.splitlines(),
                'loader_compiler_command':loader_command,'loader_postprocess_commands':loader_metadata.splitlines()}}
    (out/'report.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
    return report


def collect(key: str):
    if key not in WITNESSES:
        raise EvidenceError('unknown fixed witness key')
    parent=ROOT/'build/resident-storage-views'; parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix=key+'-',dir=parent))
    (out/'key.txt').write_text(key+'\n')
    try:
        deadline=time.monotonic()+240
        result=_gravity(out,deadline) if key=='gravity' else _gate(key,out,deadline)
        result['saved_report']=out.relative_to(ROOT).as_posix()+'/report.json'
        return result
    except BaseException as exc:
        failure={'schema':'mickey-resident-storage-view-failure-v1','key':key,'status':'blocked','error_type':type(exc).__name__,'error':str(exc)}
        (out/'failure.json').write_text(json.dumps(failure,indent=2)+'\n')
        raise


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--key',required=True,choices=sorted(WITNESSES))
    args=parser.parse_args()
    try:
        print(json.dumps(collect(args.key),indent=2,sort_keys=True)); return 0
    except (EvidenceError,fp.PreflightError,pp.MetadataProofError,rs.SurfaceComparisonError,OSError,RuntimeError,subprocess.SubprocessError) as e:
        print(f'resident_storage_view: {e}',file=sys.stderr); return 1

if __name__=='__main__': raise SystemExit(main())
