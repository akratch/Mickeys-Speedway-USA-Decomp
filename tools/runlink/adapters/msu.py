"""Mickey's Speedway USA front end: MSU's build in, kit Refs out.

What is MSU-specific -- finding the raw overlay objects from the atlas,
resolving the project's placeholder spellings (``*_oNNNReloc``, generated
names), placing object data sections in blob-owned modules, the tracked ORT
name list and the resident trap sidecars -- is loaded by
``tools/runlink_pack.py``'s own classes, used read-only.  Everything that is a
*rule* (record order, table split, operation, LOCAL base, stale JUMP index,
main-table order, encoding, comparison) is the kit's.

``parity()`` runs both and requires the same residual set, the same region
bytes and the same region proof.
"""

from __future__ import annotations

import collections
import hashlib
import json
import sys
from pathlib import Path

from .. import codec, compare, config, rules

REPO = Path(__file__).resolve().parents[3]
_LOADED_FRONTENDS = {Path(m.__file__): Path(m.__file__).read_bytes() for m in (codec, compare, config, rules)}
_LOADED_FRONTENDS[Path(__file__)] = Path(__file__).read_bytes()


def _rp():
    sys.path.insert(0, str(REPO / "tools"))
    import runlink_pack  # noqa: E402
    return runlink_pack


def world_for(cfg):
    rp = _rp()
    paths = {k: cfg.root / v for k, v in rp.DEFAULTS.items()}
    paths["rom"] = cfg.rom
    if cfg.name_list is not None:
        paths["ort"] = cfg.name_list
    world = rp.World(paths)
    ort = rp.OrtNames.load(paths["ort"])
    return rp, world, ort


def runtime_local_base(rp, target, module, text):
    """Carry a proved loader role without reclassifying writable compiler data."""
    if target.kind != "section" or target.obj.module != module:
        return None
    role = getattr(target.obj, "runtime_role", None)
    if role is None:
        return None
    if target.secname != ".data":
        raise ValueError("ordinary runtime role targets a non-data section")
    if role == "data":
        return text
    if role == "ro-origin":
        origin = rp.section_offset(target.obj, ".data")
    elif role == "ro-member":
        origin = getattr(target.obj, "runtime_origin", None)
    else:
        raise ValueError("unsupported authenticated runtime role")
    if origin is None:
        raise ValueError("authenticated runtime RO role lacks its origin")
    return origin


def refs_for_module(rp, world, resolver, ov, relocs, expected, rodata_base):
    """Kit Refs for one module, plus the hybrid-data Gens (shipped records for blob-owned data)."""
    info = world.module_info[ov]
    ts = info["text_size"]
    refs = []
    for c in relocs:
        site = c.site()
        t = c.target
        sec = c.section
        if site is None:
            refs.append(rules.Ref(sec, None, c.type, name=t.name, obj=c.obj.label, route=t.route,
                                  problem="unplaced-site"))
            refs[-1].site = c.offset   # section-relative, for the report
            continue
        if t.kind == "encoded-word":
            import proof_provenance as provenance
            witness = getattr(world, "_encoded_word", None)
            try:
                if not isinstance(witness, provenance.EncodedAddressWordWitness) or ov != 73:
                    raise provenance.MetadataProofError("kit encoded-address role lacks its sealed module")
                full = witness.operand(world, c.obj, sec, c, c.addend)
                if (t.name != provenance.ENCODED_WORD_MARKER or t.ident != full
                        or t.value != witness.origin_offset):
                    raise provenance.MetadataProofError("kit encoded-address target/origin differs")
            except (OSError, ValueError, provenance.MetadataProofError, rp.PackError) as error:
                rp.invalidate_data_import_inventory(world)
                raise rp.PackError("kit encoded-address operand refused: " + str(error)) from error
            refs.append(rules.Ref(sec, site, c.type, base=full, full=full, row=None,
                                  name=t.name, obj=c.obj.label, route=t.route,
                                  runtime_local_base=witness.origin_offset))
            continue
        if t.kind == "placeholder":
            e = expected.get((site, c.type))
            if e is None:
                refs.append(rules.Ref(sec, site, c.type, name=t.name, obj=c.obj.label, route=t.route,
                                      problem="placeholder-without-record"))
                continue
            refs.append(rules.Ref(sec, site, c.type, full=e["full"], name=t.name, obj=c.obj.label,
                                  route=t.route, verbatim=e["rec"], verbatim_table=e["table"],
                                  provenance="placeholder"))
            continue
        base = resolver.target_base(t)
        if base is None:
            refs.append(rules.Ref(sec, site, c.type, name=t.name, obj=c.obj.label, route=t.route,
                                  problem="unplaced-target"))
            continue
        full = rp.ident_add(base, c.addend)
        row = resolver.export_row(t, base)
        name = t.name if t.named else resolver.name_of(full)
        tsec = t.secname if t.kind == "section" else ""
        refs.append(rules.Ref(sec, site, c.type, base=base, full=full, row=row, target_section=tsec,
                              name=name, obj=c.obj.label, route=t.route,
                              runtime_local_base=runtime_local_base(rp, t, ov, ts)))
    produced = {(r.site, r.type) for r in refs if r.site is not None and not r.problem}
    owned = []
    for o in world.objects(ov):
        for secname in (".data", ".rodata"):
            if secname in o.placement:
                owned.append((o.placement[secname][0], o.placement[secname][0] + o.elf.size(secname)))
    extra = []
    for key, e in expected.items():
        site = key[0]
        if key in produced or site < ts or any(a <= site < b for a, b in owned):
            continue
        extra.append(rules.Gen(e["table"], codec.Record(e["rec"].symidx, site, key[1], e["rec"].op),
                               e["full"], resolver.name_of(e["full"]), "hybrid-data", "shipped", "blob"))
    return refs, extra


def header_for(world, resolver, ov, shipped_mod):
    info = world.module_info[ov]
    init = resume = -1
    for kind in ("init", "resume"):
        n = resolver.ort.entries.get((ov, kind))
        if n is None:
            continue
        ident = resolver.name_identity(n, ov)
        if ident and ident[0] == "ov" and ident[1] == ov:
            if kind == "init":
                init = ident[2]
            else:
                resume = ident[2]
    bss, _source_owned = _rp().header_bss(world, ov)
    return codec.Header(0, 0, info["text_size"], info["data_size"], bss, 0, 0, init, resume)


def main_refs(rp, world, resolver, cfg, shipped):
    """The resident references the main table patches, in link order."""
    layout = world.layout
    ship_by_site = {}
    for e in shipped.main:
        base = layout.data if e.op == codec.OP_DATA else layout.text
        ship_by_site[base + e.site] = e
    out, seen = [], set()
    for mi in world.map:
        if not (mi.outsec in (".main", ".entry") and mi.insec in (".text", ".data") and mi.size):
            continue
        path = world.paths["build"].parent / mi.file
        if (mi.file in seen and mi.insec == ".text") or not path.exists() or not mi.file.endswith(".o"):
            continue
        elf = rp.Elf(path)
        rels = elf.rels(mi.insec)
        sidecar = {}
        addresses = {}
        export = getattr(world,"_address_targets",{}).get(mi.file)
        if export and mi.insec == ".text":
            addresses = rp.resident_address_names(world,mi,path,elf,export)
        if rels and mi.insec == ".text" and mi.file.endswith(".c.o"):
            sc = Path(str(path) + ".trapsites")
            if sc.exists():
                rec = json.loads(sc.read_text())
                if rec.get("text_sha256") == hashlib.sha256(elf.bytes_of(".text")).hexdigest():
                    sidecar = {s["offset"]: s["name"] for s in rec.get("sites", [])}
        for r in rels:
            name, addr = r.sym.name, mi.addr + r.offset
            if mi.insec == ".text" and r.type == codec.R_MIPS_26 and name == cfg.trap_symbol:
                callee = sidecar.get(r.offset)
                row = None
                if callee is not None:
                    row = resolver.ort.by_name.get(callee)
                    ident = resolver.name_identity(callee)
                    if row is None and ident is not None:
                        rows = resolver.rows_by_ident().get(ident, [])
                        row = rows[0] if rows else None
                m = rules.MainRef(addr, r.type, False, row, callee or name, mi.file)
            elif (mi.insec == ".text" and r.type in (codec.R_MIPS_HI16, codec.R_MIPS_LO16)
                  and name == rp.SENTINEL and mi.file not in rp.RUNTIME_OBJECTS):
                if r.offset in addresses:
                    source_name=addresses[r.offset]
                    row=rp.address_export_row(world,resolver.ort,source_name)
                    m=rules.MainRef(addr,r.type,False,row,source_name,mi.file)
                else:
                    m = rules.MainRef(addr, r.type, False, None, name, mi.file, "placeholder")
            elif mi.insec == ".data" and r.type == codec.R_MIPS_32 and name == cfg.trap_symbol:
                m = rules.MainRef(addr, r.type, True, None, name, mi.file, "placeholder")
            else:
                continue
            if m.row is None:
                m.provenance = "placeholder"
                e = ship_by_site.get(addr)
                m.verbatim_row = e.symidx if e is not None else None
            out.append(m)
        if mi.insec == ".text":
            seen.add(mi.file)
    return out


def generate(cfg, rom, shipped):
    """(rp, world, packed-by-runlink_pack-inputs, kit results) for MSU."""
    rp, world, ort = world_for(cfg)
    return _generate_world(cfg, rom, shipped, rp, world, ort)


def _generate_world(cfg, rom, shipped, rp, world, ort):
    # Fresh kit Resolver/CRelocs/Refs/Gens; never use the core generated records.
    resolver = rp.Resolver(world, ort)
    relocs = {ov: [c for o in world.objects(ov) for c in rp.collect_relocs(world, resolver, o)]
              for ov in sorted(world.amods)}
    rp.place_sections(world, resolver, relocs)
    mods = []
    residuals = []
    for ov in sorted(world.amods):
        sm = shipped.module(ov)
        expected = compare.shipped_identities(sm, shipped, cfg)
        rodata = world.shipped.rodata_base(ov)
        refs, extra = refs_for_module(rp, world, resolver, ov, relocs[ov], expected, rodata)
        hdr = header_for(world, resolver, ov, sm)
        gm = rules.build_module(ov, refs, hdr, world.module_image_prefix(ov), rodata, extra)
        if ov == 73 and getattr(world, "_encoded_word", None) is not None:
            import proof_provenance as provenance
            try:
                world._encoded_word.runtime(gm.image,
                    [(g.site, g.rec.type, g.rec.op, g.rec.symidx) for g in gm.gens()])
            except (OSError, ValueError, provenance.MetadataProofError, rp.PackError) as error:
                rp.invalidate_data_import_inventory(world)
                raise rp.PackError("kit encoded-address runtime readback: " + str(error)) from error
        mods.append(gm)
        residuals += compare.compare_module(gm, expected, resolver.name_of)
        residuals += compare.compare_header(gm, sm)
        residuals += compare.compare_unresolved(gm)
    ort_rows = []
    for i in range(len(ort.rows)):
        ident = resolver.ort_identity(i)
        ort_rows.append(None if ident is None else codec.OrtRow(*rules.identity_row(ident, cfg)))
    residuals += compare.compare_ort(ort_rows, shipped.ort, lambda i: ort.rows[i])
    main = rules.build_main(main_refs(rp, world, resolver, cfg, shipped), cfg)
    residuals += compare.compare_main(main, shipped.main, lambda i: ort.rows[i] if 0 <= i < len(ort.rows) else str(i))
    gt = compare.gen_tables(mods, main, ort_rows, shipped)
    rp.check_data_import_holds(world)
    return rp, world, ort, mods, main, residuals, gt


def parity(cfg, rom, shipped, verbose=False):
    """Run tools/runlink_pack.py and the kit on the same tree; require identical results."""
    rp, world, ort, mods, main, residuals, gt = generate(cfg, rom, shipped)
    packed = rp.pack_all(world, ort)
    return _parity_results(cfg, rom, shipped, rp, world, packed, residuals, gt)


def _parity_results(cfg, rom, shipped, rp, world, packed, residuals, gt):
    theirs = rp.compare_all(packed, world)
    a = {(r.category, r.module, r.offset, r.expected, r.got) for r in theirs}
    b = {(r.category, r.module, r.offset, r.expected, r.got) for r in residuals}
    ok = True
    print(f"runlink_pack.py: {len(theirs)} differences; kit: {len(residuals)} differences")
    cats_a = collections.Counter(r.category for r in theirs)
    cats_b = collections.Counter(r.category for r in residuals)
    print("  runlink_pack.py: " + ", ".join(f"{k} {v}" for k, v in sorted(cats_a.items())))
    print("  kit:             " + ", ".join(f"{k} {v}" for k, v in sorted(cats_b.items())))
    if a != b:
        ok = False
        for k in sorted(a - b, key=str)[:30]:
            print("  only runlink_pack.py:", k)
        for k in sorted(b - a, key=str)[:30]:
            print("  only kit:            ", k)
    else:
        print("OK   residual sets are identical (category, module, offset, expected, got)")
    rp_region = rp.encode_region(packed, world)
    kit_region = codec.encode_region(gt, cfg)
    if rp_region != kit_region:
        ok = False
        n = sum(1 for x, y in zip(rp_region, kit_region) if x != y) + abs(len(rp_region) - len(kit_region))
        print(f"FAIL generated regions differ in {n} bytes")
    else:
        print(f"OK   generated regions are byte-identical ({len(kit_region)} bytes)")
    proof = compare.region_proof(gt, shipped, cfg, rom, residuals)
    theirs_proof = rp.region_proof(packed, world, theirs)
    same = (proof["raw_differing_bytes"] == theirs_proof["raw_differing_bytes"]
            and proof["patched_equal"] == theirs_proof["patched_equal"])
    print(("OK  " if same else "FAIL") + f" region proof: {proof['raw_differing_bytes']} raw bytes differ "
          f"(runlink_pack.py {theirs_proof['raw_differing_bytes']}); with the residuals replaced: "
          f"{'identical' if proof['patched_equal'] else 'differs'} "
          f"(runlink_pack.py {'identical' if theirs_proof['patched_equal'] else 'differs'})")
    ok &= same
    return ok, residuals, proof


def _compound_checks(cfg, rom, shipped, check_tables, cli_current, verbose=False):
    """One fresh request-owned inventory; no caller-supplied World or receipt."""
    import copy
    from types import SimpleNamespace
    import proof_provenance as provenance
    import runlink_metadata_link as metadata
    rp, world, ort = world_for(cfg)
    paths = world.paths
    declaration_current = metadata.canonical_registry_current(cfg.root)
    top = {paths[name]: paths[name].read_bytes() for name in ('atlas', 'ort', 'residuals')}
    top.update({paths['elf']: world.elf.data, paths['map']: world.map_data, paths['rom']: world.shipped.rom})
    config_bytes = cfg.path.read_bytes()
    cfg_state = copy.deepcopy(cfg)
    from dataclasses import replace
    if replace(config.load(cfg.path, root=cfg.root), rom=cfg.rom) != cfg:
        raise rp.PackError('compound loaded configuration differs from its source')
    parsed_ort = (tuple(ort.rows), dict(ort.by_name), dict(ort.entries))
    implementation = rp.metadata_implementations()
    frontend_bytes = dict(_LOADED_FRONTENDS)
    def top_parsed():
        return (world.amods, world.module_info, vars(world.layout), world.by_name,
                provenance._static_parsed_surface(world.elf))
    parsed_top = copy.deepcopy(top_parsed())
    # Bind the parsed ELF as well as the immutable buffer used to construct it.
    if provenance._static_parsed_surface(rp.Elf(paths['elf'], top[paths['elf']])) != parsed_top[-1]:
        raise rp.PackError('compound parsed ELF differs from loaded bytes')
    def top_current():
        declaration_current()
        cli_current()
        if (rp.metadata_implementations() != implementation
                or any(p.read_bytes() != value for p, value in frontend_bytes.items())
                or any(hashlib.sha256(Path(m.__file__).read_bytes()).hexdigest() != m.LOADED_SOURCE
                       for m in (codec, rules))):
            raise rp.PackError('compound loaded frontend changed')
        if (cfg != cfg_state or cfg.path.read_bytes() != config_bytes
                or any(path.read_bytes() != value for path, value in top.items())
                or world.elf.data != top[paths['elf']]
                or world.map_data != top[paths['map']]
                or world.map != rp.parse_map(paths['map'], top[paths['map']])
                or world.atlas != json.loads(top[paths['atlas']])
                or world.shipped.rom != top[paths['rom']] or rom != world.shipped.rom
                or codec.decode(rom, cfg) != shipped
                or top_parsed() != parsed_top
                or (tuple(ort.rows), dict(ort.by_name), dict(ort.entries)) != parsed_ort
                or ort.render() != rp.OrtNames.load(paths['ort']).render()):
            raise rp.PackError('compound check loaded top inputs changed')
    try:
        top_current()
        # pack_all retains its unconditional, atomic, fresh producer admission.
        packed = rp.pack_all(world, ort)
        rp.metadata_input_paths(paths, world)  # Finish existing lazy input discovery.
        # Comparisons reuse only this core Resolver's name/row maps. Complete
        # their ordinary lazy discovery before freezing that diagnostic state.
        rp.compare_all(packed, world)
        core_resolver = packed.resolver
        inventory = world._guarded_data_objects
        if inventory is None:
            raise rp.PackError('compound producer inventory is unavailable')
        objects = tuple(o for number in sorted(world.amods) for o in world.objects(number))
        observations = tuple((o, o.elf, getattr(o.elf, 'compiler_original', o.elf), bytes(o.elf.data))
                             for o in objects)
        observed_elfs = tuple({id(e): e for _, view, original, _ in observations
                              for e in (view, original)}.values())
        def parsed_state():
            # Metadata only: no repr of ELF payloads and no copied whole ELF buffers.
            return (world._expected,
                    {k: v for k, v in vars(world.shipped).items() if k != 'rom'},
                    {k: tuple(id(o) for o in v) for k, v in inventory.items()},
                    {k: tuple(id(o) for o in v) for k, v in world._objects.items()},
                    tuple((name, {k: id(v) for k, v in getattr(world, name, {}).items()})
                          for name in ('data_import_providers', '_end_proved_objects')),
                    id(getattr(world, '_encoded_word', None)),
                    {k: id(v) for k, v in getattr(world, '_encoded_word_consumers', {}).items()},
                    tuple((name, getattr(world, name, None)) for name in
                          ('_plain_data_declarations', '_local_prefix_objects', '_address_targets',
                           '_end_padding_plans', '_literal_array_plans')),
                    {k: tuple((id(o), s) for o, s in v) for k, v in world.defs().items()},
                    tuple((id(o), o.module, o.path, o.source, o.text_offset, o.text_size,
                           o.kind, o.placement, o.data_imports, o.runtime_role, o.runtime_origin)
                          for o in objects),
                    tuple(provenance._static_parsed_surface(e) for e in observed_elfs))
        state = copy.deepcopy(parsed_state())
        contexts = [(copy.deepcopy(expected), callback)
                    for expected, callback in getattr(world, 'data_import_contexts', [])]
        held_inputs = dict(getattr(world, 'data_import_inputs', {}))
        inputs = rp.metadata_input_paths(paths, world)
        input_bytes = rp.metadata_digests(inputs, cfg.root)
        def core_product():
            return packed.modules, packed.main, packed.ort_words, packed.borrowed, packed.problems
        # Unresolved CRelocs may point at ObjRefs. Those objects are held above;
        # copying their ELF/witness graphs would retain unnecessary whole inputs.
        core_snapshot = copy.deepcopy(core_product(), {id(o): o for o in objects})
        def resolver_state():
            return (id(packed.resolver), id(core_resolver.w), id(core_resolver.ort),
                    id(core_resolver.layout), id(core_resolver._ort_inventory),
                    tuple((name, getattr(core_resolver, name, None)) for name in
                          ('_ort_ident', '_held_ort_targets', '_name_cache', '_rbi', '_sym_index')))
        resolver_snapshot = copy.deepcopy(resolver_state(), {id(o): o for o in objects})
        def membership_current():
            if (world._guarded_data_objects is not inventory
                    or tuple(id(o) for n in sorted(world.amods) for o in world.objects(n)) != tuple(map(id, objects))
                    or getattr(world, 'data_import_contexts', []) != contexts
                    or getattr(world, 'data_import_inputs', {}) != held_inputs):
                raise rp.PackError('compound parsed producer inventory changed')
        def inventory_current():
            membership_current()
            if parsed_state() != state:
                raise rp.PackError('compound parsed producer inventory changed')
            if core_product() != core_snapshot:
                raise rp.PackError('compound core product changed')
            if resolver_state() != resolver_snapshot:
                raise rp.PackError('compound core comparison namespace changed')
            for obj, view, original, loaded_view in observations:
                if (obj.elf is not view or view.data != loaded_view
                        or original.path.read_bytes() != original.data):
                    raise rp.PackError('compound loaded raw observation changed')
        def current():
            top_current()
            membership_current()
            rp.check_data_import_holds(world)
            # A callback/discovery pass cannot replace the inventory and cause
            # World.objects to silently admit a fresh one during this request.
            membership_current()
            if (rp.metadata_input_paths(paths, world) != inputs
                    or rp.metadata_digests(inputs, cfg.root) != input_bytes):
                raise rp.PackError('compound current inputs changed')
            top_current()
            inventory_current()
        args = SimpleNamespace(check=True, verbose=verbose, json=None, write_residuals=False)
        checks = []
        current()
        def checked(label, call):
            # Previous post-check is also this check's pre-check; no work intervenes.
            result = call()
            current()
            checks.append((label, result))
            if result != 0:
                raise rp.PackError('compound ' + label + ' failed')
        checked('compare', lambda: rp._compare_loaded(args, paths, world, packed))
        checked('region', lambda: rp._region_loaded(world, packed))
        checked('kit-check', lambda: check_tables(cfg, rom, shipped))
        def kit_parity():
            _rp0, _world0, _ort0, mods, main, residuals, gt = _generate_world(
                cfg, rom, shipped, rp, world, ort)
            kit_snapshot = copy.deepcopy((mods, main, residuals, gt))
            current()
            ok, _residuals, _proof = _parity_results(cfg, rom, shipped, rp, world, packed, residuals, gt)
            if (mods, main, residuals, gt) != kit_snapshot:
                raise rp.PackError('parity mutated the independent kit product')
            return 0 if ok else 1
        checked('kit-parity', kit_parity)
        # The last post-check seals the complete request immediately before reporting.
        for label, result in checks:
            print(('PASS ' if result == 0 else 'FAIL ') + 'compound ' + label)
        return 0 if all(result == 0 for _, result in checks) else 1
    except BaseException:
        rp.invalidate_data_import_inventory(world)
        raise
