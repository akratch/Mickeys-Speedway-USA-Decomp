#!/usr/bin/env python3
"""Bounded real-source75/108 layout check; never relaxes the retail producer.

All changed source, metadata, linker scripts and images live below build/.
The unchanged catalog and its named borrowing remain explicit dependencies.
"""
from __future__ import annotations
import argparse
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

import link_overlay_module as module_link
import runlink_metadata_link as metadata
import runlink_pack as rp
import source_module_link_proof as source_proof
import sweep_receipts
from runlink import codec, compare, config, rules

ROOT = Path(__file__).resolve().parent.parent
FIXTURE = "tests/fixtures/editable_layout"
OUT = "build/editable-layout"
SECTIONS = (".text", ".data", ".rodata", ".bss")

class PilotError(RuntimeError):
    pass

def require(condition, message):
    if not condition:
        raise PilotError(message)

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

LOADED = {Path(p.__file__): sha(p.__file__) for p in (rp, codec, compare, config, rules, module_link, source_proof, source_proof.pp, source_proof.rs, metadata, sweep_receipts)}
LOADED[Path(__file__)] = sha(__file__)

def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    payload=(json.dumps(value, indent=2, sort_keys=True) + "\n").encode()
    path.write_bytes(payload)
    require(path.read_bytes()==payload,"generated receipt changed during write")
    return hashlib.sha256(payload).hexdigest()

def run(root, command, log, commands):
    command = [str(x) for x in command]
    result = subprocess.run(command, cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(result.stdout)
    output_hash=hashlib.sha256(result.stdout.encode()).hexdigest()
    require(sha(log)==output_hash,"command log changed during write")
    commands.append(command)
    require(result.returncode == 0, f"command failed; see {log.relative_to(root)}")
    return output_hash

def real_symbol(elf, name, *, section=None):
    choices = [s for s in elf.symbols if s.name == name and s.bind == rp.STB_GLOBAL
               and s.shndx != rp.SHN_UNDEF and (section is None or s.section == section)]
    require(len(choices) == 1, f"missing or ambiguous actual definition: {name}")
    return choices[0]

def module_record(pm):
    def rec(r): return codec.Record(r.symidx, r.site, r.type, r.op)
    return codec.Module(pm.overlay, codec.Header(0, 0, pm.text_size, pm.data_size, pm.bss_size,
                        0, 0, pm.init, pm.resume), pm.image_prefix,
                        [rec(r) for r in pm.table1], [rec(r) for r in pm.table2])

def checked_layout(main, ort, modules, cfg):
    require([m.number for m in modules] == list(range(1, len(modules)+1)), "module IDs must stay dense; tail only")
    require(len(modules) < min(cfg.reserved_selectors), "module selector is reserved")
    for row in ort:
        require(0 <= row.selector < 1 << 12, "ORT selector overflow")
        row.encode()  # existing 20-bit refusal
    for record in main:
        require(0 <= record.symidx < len(ort), "main ORT index out of range")
        record.encode()
    for m in modules:
        require(0 <= m.header.text and 0 <= m.header.data and 0 <= m.header.bss, "negative module size")
        # runlinkDownloadCode adds (u16)relocTableSize to the three s32
        # extents before mmAlloc; RT2 has its own allocation.
        allocation=m.header.text+m.header.data+m.header.bss+codec.RECORD_SIZE*len(m.table1)
        require(allocation < 1 << 31, "module allocation overflow including RT1")
        require(8*len(m.table1) < 1 << 15 and 8*len(m.table2) < 1 << 16, "runtime relocation size overflow")
        seen=set()
        for table in (m.table1, m.table2):
            for r in table:
                require((r.site,r.type) not in seen,"duplicate relocation site/type")
                seen.add((r.site,r.type))
                require(r.type in rp.TYPE_NAMES and 0 <= r.op <= 3, "unsupported relocation")
                require(0 <= r.symidx < 1 << 32, "relocation index overflow")
                # Every PatchInstruction mode writes the full u32 word.
                require(0 <= r.site and r.site+4 <= len(m.image), "relocation word escapes body")
                if r.op == codec.OP_SYMBOL: require(r.symidx < len(ort), "module ORT index out of range")
                r.encode()
        for entry in (m.header.init, m.header.resume):
            require(entry == -1 or 0 <= entry < m.header.text, "entrypoint escapes text")
    modules = codec.layout_modules(modules)
    main_size = (4 + 8*len(main) + 7) & -8
    ort_start = cfg.main_table_rom + main_size
    headers_start = ort_start + 4*len(ort)
    headers_end = headers_start + 32*len(modules)
    module_start = (headers_end + 7) & -8
    end = module_start + sum(m.header.image_size for m in modules)
    require(end <= 0x2000000, "payload exceeds diagnostic ROM")
    dynamic = replace(cfg, main_table_block=main_size, ort_rom=ort_start, ort_count=len(ort),
                      headers_rom=headers_start, module_count=len(modules), modules_rom=module_start,
                      modules_end=end)
    return codec.Tables(main, ort, modules), dynamic, headers_end

def removed_targets(tables, removed, names, callers=None):
    callers = callers or {}
    failures = []
    for record in tables.main:
        row = tables.ort[record.symidx]
        if row.selector in removed:
            failures.append(f"resident {callers.get((0,record.site,record.type), 'input')} site+{record.site:#x} -> {names[record.symidx]} (module {row.selector})")
    for m in tables.modules:
        if m.number in removed: continue
        for table in (m.table1, m.table2):
            for r in table:
                if r.op == codec.OP_SYMBOL and tables.ort[r.symidx].selector in removed:
                    failures.append(f"module {m.number} {callers.get((m.number,r.site,r.type), 'input')} site+{r.site:#x} -> {names[r.symidx]}")
    require(not failures, "referenced module removal: " + "; ".join(failures))

def compiler(root):
    target = "build/src/overlays/o075/overlay_075.c.o"
    source = "src/overlays/o075/overlay_075.c"
    expanded = subprocess.check_output(["gmake", "-n", "-W", source, target], cwd=root, text=True).replace("\\\n", " ")
    lines = [s.strip() for s in expanded.splitlines() if "tools/ido/cc" in s and target in s]
    require(len(lines) == 1, "ambiguous actual source75 compiler recipe")
    words = shlex.split(lines[0]); args = words[max(i for i,w in enumerate(words) if w == "--")+1:]
    require(args.count("-o") == 1 and args[-1] == source, "unexpected source75 compiler command")
    return args, lines[0]

def compile_module(root, directory, source_text, exports, commands, label, args):
    source = directory / (label+".c"); raw = directory / (label+".o")
    source.write_text(source_text)
    source_sha = sha(source)
    flags = list(args); flags[flags.index("-o")+1] = raw.relative_to(root).as_posix(); flags[-1] = source.relative_to(root).as_posix()
    log_hash=run(root, ["tools/ido/cc", *flags], directory/(label+"-compile.log"), commands)
    require(sha(source) == source_sha, "source changed during compiler execution")
    raw_sha = sha(raw)
    output = directory/(label+"-linked.o")
    receipt = module_link.link(raw, output, exports, directory/(label+"-module"),
        ld=root/"tools/binutils/mips64-elf-ld", objcopy=root/"tools/binutils/mips64-elf-objcopy",
        rebind=root/"tools/rebind_elf_relocations.py")
    require(sha(source) == source_sha and sha(raw) == raw_sha, "compiler input changed during link")
    require(receipt["compiler_sha256"] == raw_sha and receipt["configured_sha256"] == sha(output), "unbound module receipt")
    expected={source.name:source_sha,raw.name:raw_sha,output.name:receipt["configured_sha256"],
              label+"-compile.log":log_hash}
    for name,field in (("compiler.o","compiler_sha256"),("bindings.o","prepared_sha256"),
                       ("module.ld","script_sha256"),("module.elf","linked_sha256"),
                       ("localized.elf","localized_sha256"),("module.o","configured_sha256"),("body.bin","body_sha256")):
        expected[f"{label}-module/{name}"]=receipt[field]
    expected[f"{label}-module/receipt.json"]=hashlib.sha256((json.dumps(receipt,indent=2)+"\n").encode()).hexdigest()
    require(all(sha(directory/name)==value for name,value in expected.items()),"compiler/module generation snapshot changed")
    return raw, output, receipt, expected

def script_for(source, names, generated, module75, peer108):
    """Declared script changes only, with exact reconstruction of the original."""
    original = source
    names = tuple(names)
    result = metadata.mapped_script(source, names)
    substitutions = []
    for name in names:
        old = f"build/runlink/generated/{name}.o"; new = f"{generated}/{name}.o"
        require(result.count(old) == 1, "ambiguous generated selector")
        result = result.replace(old,new); substitutions.append((new,old))
    old75 = "build/src/overlays/o075/overlay_075.c.o"
    require(result.count(old75) == 4, "source75 must have four ordinary section inputs")
    result = result.replace(old75,module75)
    first = "    overlay_001_ROM_START = __romPos;"
    require(result.count(first) == 1, "missing module base boundary")
    aligned = "    __romPos = ALIGN(__romPos, 8);\n"+first
    result = result.replace(first,aligned)
    start = result.index("\n    rom_fill_ROM_START = __romPos;")
    end = result.index("\n    /DISCARD/",start)
    fill = result[start:end]
    require(fill.count("build/assets/rom_fill.bin.o(.data);") == 1 and "overlay_" not in fill, "unexpected fill section")
    result = result[:start]+result[end:]
    appended = ""
    if peer108:
        # Reuse the existing ordinary Splat code and standalone-table blocks.
        begin = source.index("\n    overlay_075_ROM_START = __romPos;")
        finish = source.index("\n    overlay_076_ROM_START = __romPos;",begin)
        appended = source[begin:finish].replace("075","108")
        appended = appended.replace("build/src/overlays/o108/overlay_108.c.o",peer108)
        for t in (1,2):
            appended = appended.replace(f"build/assets/overlays/o108/overlay_108_reloc{t}.bin.o",f"{generated}/overlays/o108/overlay_108_reloc{t}.bin.o")
        result = result.replace("\n    /DISCARD/",appended+"\n    /DISCARD/",1)
    reverse = result
    if appended:
        require(reverse.count(appended) == 1, "ambiguous peer block")
        reverse = reverse.replace(appended,"",1)
    reverse = reverse.replace("\n    /DISCARD/",fill+"\n    /DISCARD/",1)
    reverse = reverse.replace(aligned,first,1).replace(module75,old75)
    for new,old in substitutions: reverse = reverse.replace(new,old)
    for name in names: reverse = reverse.replace(f"build/runlink/generated/{name}.o",f"build/assets/{name}.o")
    require(reverse == original, "diagnostic mapping changed undeclared script bytes")
    require("rom_fill_" not in result and "assets/rom_fill.bin" not in result, "misleading fill consumer/symbol")
    return result

def wrappers(root, directory, files, commands):
    generated = directory/"generated"
    for name,data in files.items():
        source=directory/"metadata"/name;source.parent.mkdir(parents=True,exist_ok=True);source.write_bytes(data)
        target=generated/(name+".o");target.parent.mkdir(parents=True,exist_ok=True)
        relative=source.relative_to(root).as_posix()
        command=["tools/binutils/mips64-elf-objcopy","-I","binary","-O","elf32-bigmips","-B","mips"]
        for suffix in ("start","end","size"):
            command += ["--redefine-sym",metadata.binary_prefix(relative)+"_"+suffix+"="+metadata.binary_prefix("assets/"+name)+"_"+suffix]
        run(root,command+[relative,target.relative_to(root)],directory/"wrapper.log",commands)
    return generated

def loader_replay(root,directory,elf,mapping,commands,catalog):
    inputs=[i for i in mapping if i.file=="build/src/main/runlink.c.o" and i.insec==".text" and i.size]
    require(len(inputs)==1,"ambiguous real loader input")
    item=inputs[0]; raw=rp.Elf(root/item.file)
    require(hashlib.sha256(raw.data).hexdigest()==catalog[item.file],"loaded loader input changed")
    bindings=[]
    for name in sorted({r.sym.name for r in raw.rels(".text") if r.sym.shndx==rp.SHN_UNDEF}):
        bindings.append(f"{name} = {real_symbol(elf,name).value:#x};")
    script=directory/"loader-replay.ld"
    script_bytes=("\n".join(bindings)+f'\nSECTIONS {{ .text {item.addr:#x} : {{ {item.file}(.text) }} /DISCARD/ : {{ *(*) }} }}\n').encode()
    script.write_bytes(script_bytes)
    output=directory/"loader-replay.elf"
    log_hash=run(root,["tools/binutils/mips64-elf-ld","--no-check-sections","-e","0","-T",script.relative_to(root),"-o",output.relative_to(root)],directory/"loader-replay.log",commands)
    actual=elf.bytes_of(".main"); section=elf.section(".main");offset=item.addr-section["addr"]
    replay=rp.Elf(output)
    require(replay.bytes_of(".text")==actual[offset:offset+item.size],"loader bytes not explained by ordinary link")
    expected={"loader-replay.ld":hashlib.sha256(script_bytes).hexdigest(),
              "loader-replay.elf":hashlib.sha256(replay.data).hexdigest(),"loader-replay.log":log_hash}
    require(sha(root/item.file)==catalog[item.file] and all(sha(directory/n)==value for n,value in expected.items()),
            "loader replay inputs/outputs changed during validation")
    return offset,item.size,expected

def validate_final(root,directory,tables,cfg,headers_end,selected,expected_refs,exports,base_elf,commands,catalog,incoming):
    snapshot=artifact_hashes(directory)
    require(all(snapshot.get(name)==value for name,value in incoming.items()),"ordinary final-link inputs changed")
    require(not any(n.startswith("loader-replay.") for n in snapshot),"unexpected existing loader replay")
    require(all(snapshot[name]==value for product in selected.values() for name,value in product[3].items()),
            "compiler/module snapshot changed before final validation")
    elf=rp.Elf(directory/"final.elf");mapping=rp.parse_map(directory/"final.map");rom=(directory/"final.z64").read_bytes()
    require(hashlib.sha256(elf.data).hexdigest()==snapshot["final.elf"] and hashlib.sha256(rom).hexdigest()==snapshot["final.z64"],"loaded final bytes changed")
    decoded=codec.decode(rom,cfg)
    runtime_modules = {m.number: dict(overlay=m.number,rom_start=cfg.modules_rom+m.header.rom,
        rom_end=cfg.modules_rom+m.header.rom+m.header.image_size,text_size=m.header.text,
        data_size=m.header.data,bss_size=m.header.bss,reloc1_size=m.header.rel1,reloc2_size=m.header.rel2)
        for m in decoded.modules}
    require(codec.encode_region(decoded,cfg,gap_fill=255)==codec.encode_region(tables,cfg,gap_fill=255),"actual final metadata/body differs from proposed source layout")
    require(len(rom)==0x2000000 and all(b==255 for b in rom[cfg.modules_end:]),"wrong actual image size/fill")
    require(decoded.module(32).empty and decoded.module(32).header.rom==decoded.module(33).header.rom,"empty module32 geometry changed")
    require(len(decoded.main)==375,"pilot must not claim resident source growth")
    anchors={"main_reloc_table_ROM_START":cfg.main_table_rom,"main_reloc_table_ROM_END":cfg.ort_rom,
             "overlay_rom_table_ROM_START":cfg.ort_rom,"overlay_rom_table_ROM_END":cfg.headers_rom,
             "overlay_header_table_ROM_START":cfg.headers_rom,"overlay_header_table_ROM_END":headers_end,
             "overlay_001_ROM_START":cfg.modules_rom}
    for name,value in anchors.items(): require(real_symbol(elf,name).value==value,"wrong actual loader boundary: "+name)
    inventory={i.file for i in mapping if i.size and i.file.startswith(directory.relative_to(root).as_posix()+"/generated/")}
    wanted={f"{directory.relative_to(root)}/generated/{name}.o" for name in output_files(tables)}
    require(inventory==wanted,"incomplete actual generated input inventory")
    require(not any(i.size and i.file.startswith("build/assets/") and (metadata.metadata_name(i.file[13:-2]) or i.file=="build/assets/rom_fill.bin.o") for i in mapping),"retained metadata/fill final consumer")
    ends=[];selected_sections={}
    for m in decoded.modules:
        if m.empty:continue
        actual=elf.bytes_of(f".overlay_{m.number:03d}")
        require(actual==m.image,"final source body/input layout differs")
        begin=real_symbol(elf,f"overlay_{m.number:03d}_ROM_START").value
        require(begin==cfg.modules_rom+m.header.rom,"downstream ROM location differs")
        ends.append((begin,begin+m.header.image_size))
        if m.number in selected:
            raw=rp.Elf(selected[m.number][0]); obj=rp.Elf(selected[m.number][1])
            raw_key=selected[m.number][0].relative_to(directory).as_posix()
            linked_key=selected[m.number][1].relative_to(directory).as_posix()
            require(hashlib.sha256(raw.data).hexdigest()==snapshot[raw_key]==selected[m.number][2]["compiler_sha256"]
                    and hashlib.sha256(obj.data).hexdigest()==snapshot[linked_key]==selected[m.number][2]["configured_sha256"],
                    "loaded compiler/module product changed")
            selected_sections[m.number]={section:obj.size(section) for section in SECTIONS}
            require(actual==b"".join(obj.bytes_of(s) for s in SECTIONS[:-1]),"selected payload differs from ordinary module link")
            bss=elf.section(f".overlay_{m.number:03d}_bss")
            require(bss and bss["type"]==rp.SHT_NOBITS and bss["flags"]&3==3 and bss["size"]==obj.size(".bss") and bss["addr"]==rp.OVERLAY_VMA+len(actual),"selected NOBITS differs")
            identities=compare.shipped_identities(m,decoded,cfg)
            strict = source_proof.source_identities(raw,obj,elf,exports,m.number,
                                                    rom=rom,runtime_modules=runtime_modules)
            strict_ids = {key: rules.row_identity(codec.OrtRow(*value),cfg) for key,value in strict.items()}
            require({k:v["full"] for k,v in identities.items()} == strict_ids,
                    "final identities lack actual unique source/canonical definitions")
            wanted_ids={(r.site,r.type):r.full for r in expected_refs[m.number]}
            require({k:v["full"] for k,v in identities.items()}==wanted_ids,"final decoded identities disagree with raw source namespace/addends")
            for s in obj.symbols:
                if s.bind==rp.STB_GLOBAL and s.section in SECTIONS and s.name:
                    actual_symbol=real_symbol(elf,s.name)
                    base={".text":0,".data":obj.size(".text"),".rodata":obj.size(".text")+obj.size(".data"),".bss":len(actual)}[s.section]
                    require((actual_symbol.value,actual_symbol.size)==(rp.OVERLAY_VMA+base+s.value,s.size),"source symbol geometry differs: "+s.name)
        else:
            for name in (f".overlay_{m.number:03d}",f".overlay_{m.number:03d}_bss"):
                old,new=base_elf.section(name),elf.section(name)
                require((old is None)==(new is None),"changed unselected section presence")
                if old:require(all(old[k]==new[k] for k in ("type","flags","addr","size","align")) and base_elf.bytes_of(name)==elf.bytes_of(name),"changed unselected payload/VMA")
    require(all(a[1]<=b[0] for a,b in zip(ends,ends[1:])),"overlapping module ROM ranges")
    # The declared mapping may move LMAs; every other allocated payload/VMA
    # stays exact. The removed fill is replaced only by objcopy's FF tail.
    for old in base_elf.sections:
        name=old["name"]
        if (not old["flags"] & 2 or name in (".main", ".rom_fill", ".main_reloc_table",
                ".overlay_rom_table", ".overlay_header_table")
                or re.fullmatch(r"\.overlay_\d{3}(?:_bss|_reloc[12])?",name)):
            continue
        new=elf.section(name)
        require(new and all(old[k]==new[k] for k in ("type","flags","addr","size","align"))
                and base_elf.bytes_of(name)==elf.bytes_of(name),"changed other allocated payload/VMA: "+name)
    program = lambda e: sorted((s.name,s.value,s.size,s.bind,s.type,s.section) for s in e.symbols
        if s.shndx != rp.SHN_UNDEF and s.type not in (rp.STT_FILE,rp.STT_SECTION)
        and (s.section in rp.RESIDENT_SECTIONS or re.fullmatch(r"\.overlay_\d{3}(?:_bss)?",s.section))
        and not rp.LINKER_NAME_RE.search(s.name)
        and s.section not in {f".overlay_{n:03d}{suffix}" for n in selected for suffix in ("","_bss")})
    require(program(base_elf)==program(elf),"unselected program symbol changed")
    off,size,replay_snapshot=loader_replay(root,directory,elf,mapping,commands,catalog)
    snapshot.update(replay_snapshot)
    old,new=base_elf.bytes_of(".main"),elf.bytes_of(".main")
    require(old[:off]==new[:off] and old[off+size:]==new[off+size:],"changed non-loader resident payload")
    require(artifact_hashes(directory)==snapshot,"diagnostic inputs changed while validating final image")
    return {"artifacts":snapshot,"generated_inputs":len(inventory),"retained_metadata_inputs":0,"retained_fill_inputs":0,
            "module_headers":len(decoded.modules),"runtime_slots":len(decoded.modules)+1,"main_records":len(decoded.main),
            "ort_rows":len(decoded.ort),"header_dma_bytes":headers_end-cfg.headers_rom,
            "header_alignment_gap":cfg.modules_rom-headers_end,"module_base":cfg.modules_rom,"payload_end":cfg.modules_end,
            "rom_sha1":hashlib.sha1(rom).hexdigest(),"selected_records":{n:len(expected_refs[n]) for n in selected},
            "selected_sections":selected_sections}

def output_files(tables):
    files={"main_reloc_table.bin":codec.encode_main(tables.main, (4+8*len(tables.main)+7)&-8),
           "overlay_rom_table.bin":codec.encode_ort(tables.ort),
           "overlay_header_table.bin":codec.encode_modules(tables.modules)[0]}
    for m in tables.modules:
        for t,rows in ((1,m.table1),(2,m.table2)):
            if rows:files[f"overlays/o{m.number:03d}/overlay_{m.number:03d}_reloc{t}.bin"]=b"".join(r.encode() for r in rows)
    return files

def one_variant(root,run_dir,label,world,baseline,original_names,cfg,spec,args,catalog,*,grow=False,peer=False,use_peer=False):
    directory=run_dir/label;directory.mkdir()
    commands=[]
    names=list(original_names.rows)
    if grow:names.append(spec["new_export"])
    if peer:names.extend(spec["peer"]["exports"])
    require(len(set(names))==len(names),"diagnostic export names must be unique")
    exports=rp.OrtNames(names,dict(original_names.entries))
    if peer:
        exports.entries[(108,"init")]=spec["peer"]["init"]
        exports.entries[(108,"resume")]=spec["peer"]["resume"]
    export_bytes=exports.render().encode()
    (directory/"exports.txt").write_bytes(export_bytes)
    source=(root/spec["source"]).read_text()
    if grow:
        anchor="\n"+spec["mutation_anchor"]+"\n"
        require(source.count(anchor)==1,"existing source75 mutation anchor changed")
        call="layout75Step((s32) state->slot)"
        declarations="s32 layout75Step(s32 value);\n"
        if use_peer:
            call+=" + layout75UsePeer((s32) state->slot)"
            declarations+="s32 layout75UsePeer(s32 value);\n"
        source=source.replace('#include "overlays/overlay_075.h"\n', '#include "overlays/overlay_075.h"\n'+declarations, 1).replace(anchor,anchor.rstrip("\n")+f"\n    gOverlay75SlotFlags[state->slot] += {call};\n")
        source+="\n"+(root/FIXTURE/"growth.c").read_text()
        if use_peer:source+="\n"+(root/FIXTURE/"peer-use.c").read_text()
    selected={75:compile_module(root,directory,source,exports,commands,"overlay75",args)}
    if peer:selected[108]=compile_module(root,directory,(root/spec["peer"]["source"]).read_text(),exports,commands,"overlay108",args)
    # Resolve from the untouched actual compiler objects. Remove superseded
    # canonical75 definitions; a missing new definition cannot use the old one.
    world._objects={n:list(objects) for n,objects in world._objects.items()}
    world.by_name={name:[s for s in syms if s.section not in {".overlay_075",".overlay_075_bss"}]
                   for name,syms in world.by_name.items()}
    world.amods = dict(world.amods)
    if peer: world.amods[108] = {"overlay": 108}
    for n,(raw,linked,_receipt,_generated) in selected.items():
        e=rp.Elf(raw);linked_elf=rp.Elf(linked)
        require(hashlib.sha256(e.data).hexdigest()==_receipt["compiler_sha256"]
                and hashlib.sha256(linked_elf.data).hexdigest()==_receipt["configured_sha256"],"loaded module-generation input changed")
        require(all(e.size(s)==linked_elf.size(s) for s in SECTIONS),"ordinary module changed section extent")
        obj=rp.ObjRef(n,raw,f"diagnostic/{label}/overlay{n}",0,e.size(".text"),"c",e)
        bases={".data":e.size(".text"),".rodata":e.size(".text")+e.size(".data"),
               ".bss":sum(e.size(s) for s in SECTIONS[:-1])}
        obj.placement={s:(v,"ordinary-module-link") for s,v in bases.items()}
        world._objects[n]=[obj]
        world.module_info[n]={"text_size":e.size(".text"),"data_size":e.size(".data")+e.size(".rodata"),
                              "prefix":bases[".bss"]}
    world._defs=None
    resolver=rp.Resolver(world,exports)
    base_resolver=baseline.resolver
    identities=list(base_resolver._pilot_identities)
    for row in spec["existing_exports"]:
        require(original_names.rows[row["row"]]==row["name"],"retained row/name identity changed")
        identities[row["row"]]=resolver.name_identity(row["name"],75)
    for name in names[len(original_names.rows):]:
        owner=75 if name==spec["new_export"] else 108
        ident=resolver.name_identity(name,owner)
        require(ident is not None and ident[0]=="ov" and ident[1]==owner,"new export lacks a real owner: "+name)
        identities.append(ident)
    resolver._ort_ident=identities
    ort=[codec.OrtRow(*rules.identity_row(i,cfg)) for i in identities]
    modules={n:module_record(pm) for n,pm in baseline.modules.items()}
    expected={};callers={}
    for n,(raw,linked,_receipt,_generated) in selected.items():
        obj=world.objects(n)[0];e=rp.Elf(linked)
        require(hashlib.sha256(e.data).hexdigest()==_receipt["configured_sha256"],"loaded module layout changed")
        rels=rp.collect_relocs(world,resolver,obj)
        refs=[]
        for c in rels:
            target=c.target; base=resolver.target_base(target)
            require(target.kind != "placeholder" and base is not None and c.site() is not None,
                    "selected source needs an identity/placement oracle: "+target.name)
            refs.append(rules.Ref(c.section,c.site(),c.type,base=base,full=rp.ident_add(base,c.addend),
                        row=resolver.export_row(target,base),target_section=target.secname,
                        name=target.name,obj=obj.label,route=target.route))
        entries=[]
        for kind in ("init","resume"):
            name=exports.entries.get((n,kind))
            entries.append(-1 if name is None else real_symbol(e,name,section=".text").value)
        header=codec.Header(0,0,e.size(".text"),e.size(".data")+e.size(".rodata"),e.size(".bss"),0,0,*entries)
        generated=rules.build_module(n,refs,header,b"".join(e.bytes_of(s) for s in SECTIONS[:-1]),e.size(".text")+e.size(".data"))
        require(not generated.unresolved and not generated.notes,"selected source has unresolved or malformed references")
        modules[n]=codec.Module(n,header,generated.image,[g.rec for g in generated.table1],[g.rec for g in generated.table2])
        expected[n]=refs
        for r in refs:
            functions=[s for s in obj.elf.symbols if s.type==rp.STT_FUNC and s.size and s.value<=r.site<s.value+s.size]
            callers[(n,r.site,r.type)]=functions[0].name if functions else r.obj
    main=[codec.Record(e.row,e.offset,e.type,e.op) for e in baseline.main]
    tables,dynamic,header_end=checked_layout(main,ort,[modules[n] for n in sorted(modules)],cfg)
    files=output_files(tables)
    generated=wrappers(root,directory,files,commands)
    source_script=(root/"mickey.us.ld").read_text()
    script=script_for(source_script,root_catalog_names(root),generated.relative_to(root).as_posix(),
                      selected[75][1].relative_to(root).as_posix(),
                      selected[108][1].relative_to(root).as_posix() if peer else None)
    final_script=directory/"final.ld";final_script.write_text(script)
    require(sha(final_script)==hashlib.sha256(script.encode()).hexdigest()
            and sha(directory/"exports.txt")==hashlib.sha256(export_bytes).hexdigest(),"generated link script/exports changed")
    for name,data in files.items():
        wrapper=rp.Elf(generated/(name+".o"))
        require(wrapper.bytes_of(".data")==data and sha(wrapper.path)==hashlib.sha256(wrapper.data).hexdigest()
                and sha(directory/"metadata"/name)==hashlib.sha256(data).hexdigest(),"generated metadata input changed")
    incoming=artifact_hashes(directory)
    command=["tools/link_diagnose.sh","tools/binutils/mips64-elf-ld","-T",final_script.relative_to(root)]
    for f in ("undefined_funcs_auto.us.txt","undefined_syms_auto.us.txt","libultra_undefined_syms.us.txt","overlay_undefined_syms.us.txt"):
        command += ["-T",f]
    command += ["-Map",(directory/"final.map").relative_to(root),"--no-check-sections","-o",(directory/"final.elf").relative_to(root)]
    run(root,command,directory/"final-link.log",commands)
    run(root,["tools/binutils/mips64-elf-objcopy","-O","binary","--pad-to=0x2000000","--gap-fill=0xFF",
              (directory/"final.elf").relative_to(root),(directory/"final.z64").relative_to(root)],directory/"image.log",commands)
    run(root,["tools/n64crc",(directory/"final.z64").relative_to(root)],directory/"crc.log",commands)
    checksum=sha(directory/"final.z64")
    run(root,["tools/n64crc",(directory/"final.z64").relative_to(root)],directory/"crc-check.log",commands)
    require(sha(directory/"final.z64")==checksum,"computed diagnostic checksum is not stable")
    base_elf=world.elf
    result=validate_final(root,directory,tables,dynamic,header_end,selected,expected,exports,base_elf,commands,catalog,incoming)
    original_raw=world.paths["raw"]/"src/overlays/o075/overlay_075.c.o"
    old=rp.Elf(original_raw);new=rp.Elf(selected[75][0])
    require(hashlib.sha256(old.data).hexdigest()==catalog[original_raw.relative_to(root).as_posix()]
            and hashlib.sha256(new.data).hexdigest()==result["artifacts"][selected[75][0].name],"loaded growth evidence changed")
    result["existing_function_bytes"]={"baseline":real_symbol(old,spec["mutated_function"]).size,
                                      "diagnostic":real_symbol(new,spec["mutated_function"]).size}
    selected_rows={r["row"] for r in spec["existing_exports"]}
    require(all((row.selector,row.offset)==(baseline.ort_words[i]>>20,baseline.ort_words[i]&0xFFFFF)
                for i,row in enumerate(ort[:len(original_names.rows)]) if i not in selected_rows),
            "unselected ORT row identity changed")
    result["existing_export_offsets"]={r["name"]:{"baseline":real_symbol(old,r["name"]).value,
        "diagnostic":real_symbol(new,r["name"]).value} for r in spec["existing_exports"]}
    if grow:
        require(any(v["baseline"]!=v["diagnostic"] for v in result["existing_export_offsets"].values()),
                "existing export offsets did not move")
        require(result["existing_function_bytes"]["diagnostic"]>result["existing_function_bytes"]["baseline"],"existing function did not actually grow")
        require(new.size(".data")>old.size(".data") and new.size(".bss")>old.size(".bss"),"actual initialized/BSS source growth absent")
        require(result["selected_records"][75]>31 and result["ort_rows"]>2004,"record/export growth absent")
        require(any(r.op==codec.OP_JUMP for r in modules[75].table1+modules[75].table2),"private source JUMP use disappeared")
        require(result["header_alignment_gap"]>0,"odd export did not exercise header/module alignment")
    else:
        require(result["rom_sha1"]=="507341c0a40ca3e9a7cee969b396ee53facfb548","unchanged control is not retail exact")
    if peer:
        peer_raw=rp.Elf(selected[108][0])
        require(hashlib.sha256(peer_raw.data).hexdigest()==result["artifacts"][selected[108][0].name],"loaded peer evidence changed")
        init=real_symbol(peer_raw,spec["peer"]["init"],section=".text")
        require(init.value==0 and init.type==rp.STT_FUNC and init.size>0,"peer offset-zero function absent")
        if use_peer:
            state=real_symbol(peer_raw,"layout108State",section=".bss")
            require(state.size>0 and state.value+state.size<=peer_raw.size(".bss"),"peer BSS definition escapes allocation")
            state_base=sum(peer_raw.size(section) for section in SECTIONS[:-1])+state.value
            for name,kind,target in ((init.name,rp.R_MIPS_26,("ov",108,init.value)),
                                     (state.name,rp.R_MIPS_32,("ov",108,state_base))):
                row=names.index(name)
                sites={r.site for r in expected[75] if r.type==kind and r.full==target and r.row==row}
                require(sites and any(r.site in sites and r.type==kind and r.op==codec.OP_SYMBOL and r.symidx==row
                        for r in modules[75].table1+modules[75].table2),
                        "peer reference lacks actual source/export/SYMBOL identity: "+name)
            result["peer_offset_zero_call_and_bss_export"]=True
    result.update(schema="mickey-editable-layout-diagnostic-v1",nonmatching_diagnostic=True,
                  source_only=False,general_relayout=False,commands=commands,
                  compiler_receipts={str(n):r[2] for n,r in selected.items()},
                  named_borrowing_sha256=sha(root/"config/runlink-residuals.us.json"),
                  borrowed_records=json.loads((root/"config/runlink-residuals.us.json").read_text())["totals"]["borrowed_records"])
    receipt_hash=publish_receipt(directory,result,result["artifacts"])
    print(label,json.dumps({k:result[k] for k in ("generated_inputs","runtime_slots","ort_rows","header_alignment_gap","selected_records","rom_sha1")}),flush=True)
    return tables,names,result,callers,receipt_hash

def root_catalog_names(root):
    return json.loads((root/metadata.MANIFEST).read_text())["files"]

def tool_context(root):
    return {"ido":sweep_receipts.tree_digest(root/"tools/ido"),
            "includes":sweep_receipts.tree_digest(root/"include"),
            "python":sha(sys.executable),"version":sys.version}

def artifact_hashes(directory):
    require(not any(p.is_symlink() for p in directory.rglob("*")),"symlink diagnostic artifact")
    return {p.relative_to(directory).as_posix():sha(p) for p in sorted(directory.rglob("*"))
            if p.is_file() and p != directory/"receipt.json"}

def publish_receipt(directory,result,validated):
    # Never replace the semantically validated hashes with a later observation.
    require(artifact_hashes(directory)==validated,"validated artifacts changed before receipt publication")
    result["artifacts"]=dict(validated)
    receipt_hash=save(directory/"receipt.json",result)
    require(artifact_hashes(directory)==validated and sha(directory/"receipt.json")==receipt_hash,
            "validated artifacts changed during receipt publication")
    return receipt_hash

def check_receipt(root,path):
    path=(path if path.is_absolute() else root/path).resolve()
    require(path.is_relative_to((root/OUT).resolve()),"receipt is outside the diagnostic output tree")
    require(all(sha(p)==value for p,value in LOADED.items()),"loaded pilot implementation changed")
    loaded=path.read_bytes(); receipt=json.loads(loaded)
    require(receipt.get("schema")=="mickey-editable-layout-check-v1" and receipt.get("nonmatching_diagnostic") is True,
            "not a diagnostic layout receipt")
    artifacts=artifact_hashes(path.parent)
    require(receipt["artifacts"]==artifacts,"missing/tampered diagnostic artifact")
    inputs={n:sha(root/metadata.safe_relative(n)) for n in receipt["inputs"]}
    require(receipt["inputs"]==inputs,"stale diagnostic input")
    tools=tool_context(root)
    require(receipt["tool_context"]==tools,"stale diagnostic toolchain")
    context=metadata.provenance_context(root)
    require(receipt["canonical_context"]==context,"stale canonical catalog")
    require(path.read_bytes()==loaded and artifact_hashes(path.parent)==artifacts
            and inputs=={n:sha(root/n) for n in inputs}
            and context=={n:metadata.digest(root/n) for n in context}
            and tools==tool_context(root) and all(sha(p)==value for p,value in LOADED.items()),
            "diagnostic proof inputs changed during receipt validation")

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--only",choices=("control","all"),default="all")
    parser.add_argument("--out")
    parser.add_argument("--check-receipt",type=Path)
    options=parser.parse_args()
    root=ROOT
    if options.check_receipt:
        check_receipt(root,options.check_receipt)
        print("editable-layout receipt current",options.check_receipt)
        return
    if options.out:
        relative=metadata.safe_relative(options.out)
        require(relative.parts[:2]==("build","editable-layout"),"diagnostic outputs must stay under build/editable-layout")
        directory=root/relative
        require(not directory.exists(),"diagnostic output already exists; preserve it and choose a new --out")
        directory.mkdir(parents=True)
    else:
        (root/OUT).mkdir(parents=True,exist_ok=True)
        directory=Path(tempfile.mkdtemp(prefix="check-",dir=root/OUT))
    before=metadata.provenance_context(root)
    tools_before=tool_context(root)
    require(all(sha(p)==value for p,value in LOADED.items()),"loaded pilot implementation changed")
    print("editable-layout artifacts",directory.relative_to(root),flush=True)
    spec=json.loads((root/FIXTURE/"manifest.json").read_text())
    require(spec["schema"]=="mickey-editable-layout-pilot-v1" and spec["module"]==75 and spec["peer"]["module"]==108,"unexpected deliberate pilot identity")
    require(spec["source"]=="src/overlays/o075/overlay_075.c" and spec["peer"]["source"]==FIXTURE+"/peer.c","pilot source identities changed")
    inputs={str(p.relative_to(root)):sha(p) for p in [Path(__file__),root/spec["source"],root/"Makefile",*(root/FIXTURE).glob("*"),
            root/"tools/link_overlay_module.py",root/"tools/runlink_pack.py",root/"tools/runlink/codec.py",
            root/"tools/runlink/rules.py",root/"tools/runlink/compare.py",root/"tools/source_module_link_proof.py",root/"tools/proof_provenance.py",root/"tools/reloc_surface.py",
            root/"tools/rebind_elf_relocations.py",root/"tools/binutils/mips64-elf-ld",
            root/"tools/binutils/mips64-elf-objcopy",root/"tools/ido/cc",root/"tools/n64crc"] if p.is_file()}
    cfg=config.load("msu-us",root=root)
    paths={k:root/v for k,v in rp.DEFAULTS.items()};paths["elf"]=root/metadata.BOOT_ELF;paths["map"]=root/metadata.BOOT_MAP
    world=rp.World(paths)
    require(hashlib.sha256(world.elf.data).hexdigest()==before[metadata.BOOT_ELF],"loaded canonical catalog bytes changed")
    ort=rp.OrtNames.load(paths["ort"]);packed=rp.pack_all(world,ort)
    require(not packed.problems and not rp.compare_all(packed,world),"canonical outside catalog is not exact")
    require(rp.header_bss(world,75)[1] and not packed.modules[75].borrowed,"source75 is not a complete unborrowed pilot")
    packed.resolver._pilot_identities=[packed.resolver.ort_identity(i) for i in range(len(ort.rows))]
    original_objects={n:list(world.objects(n)) for n in world.amods}
    original_names={n:list(v) for n,v in world.by_name.items()}
    original_info=dict(world.module_info)
    original_amods=dict(world.amods)
    compiler_args,command=compiler(root)
    resident_callers={}
    base_elf=world.elf
    for e in packed.main:
        functions=[s for s in base_elf.symbols if s.type==rp.STT_FUNC and s.size and s.value<=e.site<s.value+s.size]
        resident_callers[(0,e.offset,e.type)]=(functions[0].name if functions else e.obj)
    results={};products={}
    require(artifact_hashes(directory)=={},"unexpected diagnostic root artifact")
    validated={}
    variants=[("control",False,False,False)]
    if options.only=="all":variants += [("grow",True,False,False),("peer",True,True,True),("peer-unreferenced",True,True,False),("removed-unreferenced",True,False,False)]
    for name,grow,peer,use_peer in variants:
        world._objects=original_objects;world.by_name=original_names;world.module_info=dict(original_info);world.amods=dict(original_amods)
        products[name]=one_variant(root,directory,name,world,packed,ort,cfg,spec,compiler_args,before,grow=grow,peer=peer,use_peer=use_peer)
        results[name]=products[name][2]
        validated.update({f"{name}/{path}":value for path,value in results[name]["artifacts"].items()})
        validated[f"{name}/receipt.json"]=products[name][4]
    refusals={}
    for label,product,removed in [("resident-to75",products["control"],{75})]+([("source75-to108",products["peer"],{108})] if "peer" in products else []):
        try: removed_targets(product[0],removed,product[1],{**resident_callers,**product[3]})
        except PilotError as error:
            refusals[label]=str(error)
        else:raise PilotError("referenced removal was accepted")
    if "removed-unreferenced" in products:
        removed_targets(products["peer-unreferenced"][0],{108},products["peer-unreferenced"][1])
        require(results["grow"]["rom_sha1"]==results["removed-unreferenced"]["rom_sha1"],"unreferenced removal did not return the genuine grow image")
    require(artifact_hashes(directory)==validated,"validated diagnostic artifacts changed before publication")
    require(before==metadata.provenance_context(root),"canonical catalog changed during diagnostic build")
    require(inputs=={n:sha(root/n) for n in inputs} and tools_before==tool_context(root),"pilot implementation/input/toolchain changed during check")
    require(all(sha(p)==value for p,value in LOADED.items()),"loaded pilot implementation changed")
    data_only=[]
    for m in world.atlas["modules"]:
        code={r["source"] for r in m["text_ownership"] if r["type"]=="c"}
        data_only.extend(r["source"] for r in m.get("data_rodata_ownership",[]) if r.get("canonical") and not r.get("externalized") and r["source"] not in code)
    publish_receipt(directory,{"schema":"mickey-editable-layout-check-v1","nonmatching_diagnostic":True,
         "source_only":False,"general_relayout":False,"inputs":inputs,"canonical_context":before,"tool_context":tools_before,
         "canonical_recipe":command,"named_borrowing":json.loads((root/"config/runlink-residuals.us.json").read_text()),
         "data_only_inventory_not_enabled":sorted(set(data_only)),"variants":results,"removal_refusals":refusals},validated)
    require(inputs=={n:sha(root/n) for n in inputs} and tools_before==tool_context(root)
            and before==metadata.provenance_context(root) and all(sha(p)==value for p,value in LOADED.items()),
            "pilot inputs changed during aggregate publication")
    print("editable-layout check PASS",directory.relative_to(root),flush=True)

if __name__=="__main__":
    try:main()
    except (PilotError,rp.PackError,module_link.LinkError,source_proof.pp.MetadataProofError,codec.FormatError,OSError,ValueError,KeyError) as error:
        raise SystemExit("editable-layout refused: "+str(error))
