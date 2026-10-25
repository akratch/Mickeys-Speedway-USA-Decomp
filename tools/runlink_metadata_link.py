#!/usr/bin/env python3
"""Consume exact runlink metadata in the fixed-layout canonical GNU ld graph.

The ordinary Splat script remains the retained-input bootstrap. Only its named
metadata selectors change in the final script; code/data inputs stay intact.
"""
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess
import sys

import runlink_pack as rp

REPO = Path(__file__).resolve().parent.parent
_LOADED = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
BOOT_ELF = "build/runlink/bootstrap.elf"
BOOT_MAP = "build/runlink/bootstrap.map"
BOOT_LOG = "build/runlink/bootstrap-link.log"
FINAL_LOG = "build/runlink/final-link.log"
META = "build/runlink/metadata"
FINAL_SCRIPT = "build/runlink/final.ld"
RECEIPT = "build/runlink/metadata-link.json"
FINAL_PROOF = "build/runlink/final-proof.json"
MANIFEST = META + "/manifest.json"
MARKER = "RUNLINK_METADATA_MANIFEST :="
SCHEMA = "mickey-generated-metadata-link-v1"
TOP = {"main_reloc_table.bin", "overlay_rom_table.bin", "overlay_header_table.bin"}


class MetadataLinkError(RuntimeError):
    pass


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_changed(path, data):
    if isinstance(data, str):
        data = data.encode()
    if path.is_file() and path.read_bytes() == data:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(data)
    temporary.replace(path)


def json_bytes(value):
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode()


def metadata_name(name):
    return name in TOP or re.fullmatch(r"overlays/o(\d{3})/overlay_\1_reloc[12]\.bin", name) is not None


def mapped_script(source, names):
    """Replace exactly one .data selector per metadata object, nothing else."""
    names = set(names)
    if not names or not TOP <= names or any(not metadata_name(n) for n in names):
        raise MetadataLinkError("invalid metadata input set")
    expected = {f"build/assets/{n}.o": f"build/runlink/generated/{n}.o" for n in names}
    counts = dict.fromkeys(expected, 0)
    pattern = re.compile(r"(?m)^(\s*)(build/assets/[\w./-]+\.bin\.o)(\(\.data\);)[ \t]*$")
    def replace(match):
        old = match[2]
        if old not in expected:
            return match[0]
        counts[old] += 1
        return match[0].replace(old, expected[old])
    result = pattern.sub(replace, source)
    if any(n != 1 or source.count(old) != 1 for old, n in counts.items()):
        raise MetadataLinkError("metadata selector missing, repeated, or not an ordinary .data input")
    for found in re.findall(r"build/assets/([\w./-]+\.bin)\.o", source):
        if metadata_name(found) and found not in names:
            raise MetadataLinkError("unexpected retained metadata input")
    restored = result
    for old, new in expected.items():
        if result.count(new) != 1:
            raise MetadataLinkError("generated metadata selector is ambiguous")
        restored = restored.replace(new, old)
    if restored != source:
        raise MetadataLinkError("script mapping changed a non-metadata byte")
    return result


def safe_relative(name):
    path = Path(name)
    if path.is_absolute() or ".." in path.parts:
        raise MetadataLinkError("non-repository metadata input")
    return path


def validate_manifest(root):
    if digest(Path(__file__)) != _LOADED:
        raise MetadataLinkError("loaded metadata consumer changed")
    implementations = rp.metadata_implementations()
    if digest(root / "tools/runlink_metadata_link.py") != _LOADED:
        raise MetadataLinkError("metadata consumer is not the loaded implementation")
    manifest = json.loads((root / MANIFEST).read_text())
    if not isinstance(manifest, dict):
        raise MetadataLinkError("invalid metadata manifest")
    proof = manifest.get("proof", {})
    if (manifest.get("schema") != "mickey-generated-metadata-v1"
            or proof.get("raw_equal") is not True or proof.get("raw_differing_bytes") != 0
            or proof.get("differences") != 0 or proof.get("source_only") is not False
            or proof.get("relayout") is not False):
        raise MetadataLinkError("metadata lacks the exact hybrid raw-region proof")
    inputs = manifest["inputs"]
    if BOOT_ELF not in inputs or BOOT_MAP not in inputs or "build/mickey.us.elf" in inputs:
        raise MetadataLinkError("metadata was not produced from the distinct bootstrap")
    for name, info in inputs.items():
        data = (root / safe_relative(name)).read_bytes()
        if info != {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}:
            raise MetadataLinkError(f"stale producer input: {name}")
    for name, sha in manifest["implementation"].items():
        if digest(root / safe_relative(name)) != sha:
            raise MetadataLinkError(f"changed metadata producer: {name}")
    if set(manifest["implementation"]) != {"tools/runlink_pack.py", "tools/overlay_tables.py", "tools/trap_names.py"}:
        raise MetadataLinkError("unbound metadata producer implementation")
    if manifest["implementation"] != implementations:
        raise MetadataLinkError("metadata producer differs from the loaded implementation")
    names = manifest["files"]
    if (len(names) != manifest["metadata_files"] or not TOP <= set(names)
            or any(not metadata_name(n) for n in names)):
        raise MetadataLinkError("invalid generated metadata inventory")
    for name, info in names.items():
        data = (root / META / name).read_bytes()
        if info != {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}:
            raise MetadataLinkError(f"changed generated metadata: {name}")
    borrowing = manifest["borrowing"]
    if borrowing["path"] != "borrowing.json" or digest(root / META / borrowing["path"]) != borrowing["sha256"]:
        raise MetadataLinkError("changed named borrowing ledger")
    ledger = json.loads((root / META / borrowing["path"]).read_text())
    if (ledger != json.loads((root / "config/runlink-residuals.us.json").read_text())
            or borrowing["borrowed_records"] != ledger["totals"]["borrowed_records"]
            or borrowing["entries_by_kind"] != {k: v["count"] for k, v in ledger["borrowed"].items()}):
        raise MetadataLinkError("metadata misreports named borrowing")
    return manifest


def run(argv, root):
    result = subprocess.run([str(x) for x in argv], cwd=root, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode:
        raise MetadataLinkError(result.stdout.strip())


def binary_prefix(name):
    return "_binary_" + re.sub(r"[^a-zA-Z0-9_]", "_", name)


def wrapper(root, name):
    source = f"{META}/{name}"
    target = root / f"build/runlink/generated/{name}.o"
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = target.with_name(target.name + ".tmp")
    args = ["tools/binutils/mips64-elf-objcopy", "-I", "binary", "-O", "elf32-bigmips", "-B", "mips"]
    for suffix in ("start", "end", "size"):
        args += ["--redefine-sym", f"{binary_prefix(source)}_{suffix}={binary_prefix('assets/' + name)}_{suffix}"]
    run(args + [source, temporary.relative_to(root)], root)
    temporary.replace(target)


def context_paths(root):
    return [BOOT_ELF, BOOT_MAP, BOOT_LOG, "mickey.us.ld", "Makefile", MANIFEST,
            "tools/runlink_metadata_link.py", "tools/runlink_pack.py", "tools/overlay_tables.py", "tools/trap_names.py",
            "tools/link_diagnose.sh", "tools/run_logged.py",
            "tools/binutils/mips64-elf-ld", "tools/binutils/mips64-elf-objcopy",
            *[p.name for p in (root / "undefined_funcs_auto.us.txt", root / "undefined_syms_auto.us.txt",
                              root / "libultra_undefined_syms.us.txt", root / "overlay_undefined_syms.us.txt")]]


def prepare(root):
    try:
        previous = json.loads((root / RECEIPT).read_text())
    except (OSError, ValueError):
        previous = {}
    if not isinstance(previous, dict):
        previous = {}
    try:
        if previous.get("schema") != SCHEMA or previous.get("inputs", {}).get(MANIFEST) != digest(root / MANIFEST):
            raise MetadataLinkError("metadata requires a fresh authenticated emission")
        manifest = validate_manifest(root)
    except (MetadataLinkError, OSError, ValueError, KeyError, TypeError):
        run([sys.executable, "tools/runlink_pack.py", "--elf", BOOT_ELF,
             "--map", BOOT_MAP, "emit-metadata"], root)
        manifest = validate_manifest(root)
    script = mapped_script((root / "mickey.us.ld").read_bytes().decode(), manifest["files"])
    write_changed(root / FINAL_SCRIPT, script)
    inputs = {name: digest(root / name) for name in context_paths(root)}
    wrappers = {}
    for name in manifest["files"]:
        output = f"build/runlink/generated/{name}.o"
        old = previous.get("wrappers", {}).get(output)
        if (previous.get("schema") != SCHEMA or previous.get("inputs") != inputs
                or not (root / output).is_file() or digest(root / output) != old):
            wrapper(root, name)
        # Binary wrappers must preserve the old metadata symbol names and bytes.
        generated, retained = rp.Elf(root / output), rp.Elf(root / f"build/assets/{name}.o")
        if allocated(generated) != allocated(retained) or symbols(generated) != symbols(retained):
            raise MetadataLinkError(f"binary wrapper differs from retained metadata: {name}")
        wrappers[output] = digest(root / output)
    if validate_manifest(root) != manifest or {n: digest(root / n) for n in inputs} != inputs:
        raise MetadataLinkError("metadata changed while preparing final inputs")
    receipt = {"schema": SCHEMA, "inputs": inputs, "script": digest(root / FINAL_SCRIPT),
               "wrappers": wrappers, "producer": manifest["implementation"],
               "borrowed_records": manifest["borrowing"]["borrowed_records"]}
    write_changed(root / RECEIPT, json_bytes(receipt))


def allocated(elf):
    return sorted((s["name"], s["type"], s["flags"], s["addr"], s["size"], s["align"],
                   elf.bytes_of(s["name"])) for s in elf.sections if s["flags"] & 2)


def symbols(elf):
    return sorted((s.name, s.value, s.size, s.bind, s.type, s.section)
                  for s in elf.symbols if s.type != rp.STT_FILE)


def check_prepared(root):
    manifest = validate_manifest(root)
    receipt = json.loads((root / RECEIPT).read_text())
    if not isinstance(receipt, dict):
        raise MetadataLinkError("invalid final metadata preparation")
    if receipt.get("schema") != SCHEMA or receipt.get("inputs") != {n: digest(root / n) for n in context_paths(root)}:
        raise MetadataLinkError("stale final metadata preparation")
    expected = mapped_script((root / "mickey.us.ld").read_bytes().decode(), manifest["files"])
    if ((root / FINAL_SCRIPT).read_bytes().decode() != expected or digest(root / FINAL_SCRIPT) != receipt["script"]
            or set(receipt["wrappers"]) != {f"build/runlink/generated/{n}.o" for n in manifest["files"]}):
        raise MetadataLinkError("changed metadata-only final script/inventory")
    for name, sha in receipt["wrappers"].items():
        if digest(root / safe_relative(name)) != sha:
            raise MetadataLinkError(f"changed final metadata wrapper: {name}")
    return manifest, receipt


def link_command(root, *, final):
    log = FINAL_LOG if final else BOOT_LOG
    lines = (root / log).read_text().splitlines()
    if not lines or not lines[0].startswith("command: "):
        raise MetadataLinkError("missing ordinary linker command record")
    argv = shlex.split(lines[0][len("command: "):])
    expected = ["tools/link_diagnose.sh", "tools/binutils/mips64-elf-ld", "-T",
                FINAL_SCRIPT if final else "mickey.us.ld"]
    for script in ("undefined_funcs_auto.us.txt", "undefined_syms_auto.us.txt",
                   "libultra_undefined_syms.us.txt", "overlay_undefined_syms.us.txt"):
        expected += ["-T", script]
    expected += ["-Map", "build/mickey.us.map" if final else BOOT_MAP,
                 "--no-check-sections", "-o", "build/mickey.us.elf" if final else BOOT_ELF]
    if argv != expected:
        raise MetadataLinkError("unbound ordinary metadata linker command")
    return argv


def _prove(root, *, write):
    # Bind the parsed images and map/log/prepared evidence to one stable input
    # snapshot. The proof output is only an input on the read-only path.
    paths = provenance_paths(root) | {root / "build/mickey.us.elf", root / "build/mickey.us.map"}
    if write:
        paths.discard(root / FINAL_PROOF)
    before = {path: digest(path) for path in paths}
    manifest, receipt = check_prepared(root)
    commands = {"bootstrap": link_command(root, final=False), "final": link_command(root, final=True)}
    final = root / "build/mickey.us.elf"
    mapping = rp.parse_map(root / "build/mickey.us.map")
    wanted = {f"build/runlink/generated/{n}.o": info["size"] for n, info in manifest["files"].items()}
    found = {}
    for item in mapping:
        if not item.size:
            continue
        if item.file.startswith("build/assets/") and metadata_name(item.file[len("build/assets/"):-2]):
            raise MetadataLinkError("final map consumes retained metadata")
        if item.file in wanted:
            if item.file in found or item.insec != ".data" or item.size != wanted[item.file]:
                raise MetadataLinkError("invalid generated metadata map geometry")
            found[item.file] = item.size
    if found != wanted:
        raise MetadataLinkError("final map lacks the complete generated metadata input set")
    bootstrap, linked = rp.Elf(root / BOOT_ELF), rp.Elf(final)
    if (hashlib.sha256(bootstrap.data).hexdigest() != manifest["inputs"][BOOT_ELF]["sha256"]
            or hashlib.sha256(bootstrap.data).hexdigest() != before[root / BOOT_ELF]
            or hashlib.sha256(linked.data).hexdigest() != before[final]):
        raise MetadataLinkError("loaded metadata link images changed during proof")
    if allocated(bootstrap) != allocated(linked) or symbols(bootstrap) != symbols(linked):
        raise MetadataLinkError("generated final link changed allocated bytes or program symbols")
    proof = {"schema": "mickey-generated-metadata-final-v1", "allocated_sections_exact": True,
             "whole_elf_exact": bootstrap.data == linked.data,
             "program_symbols_exact": True, "generated_metadata_inputs": len(found),
             "retained_metadata_inputs": 0, "borrowed_records": receipt["borrowed_records"], "commands": commands,
             "inputs": {n: before[root / n] for n in (RECEIPT, "build/mickey.us.elf", "build/mickey.us.map", FINAL_LOG)}}
    if not write and json.loads((root / FINAL_PROOF).read_text()) != proof:
        raise MetadataLinkError("stale generated final-link proof")
    after_paths = provenance_paths(root) | {root / "build/mickey.us.elf", root / "build/mickey.us.map"}
    if write:
        after_paths.discard(root / FINAL_PROOF)
    if after_paths != paths or {path: digest(path) for path in paths} != before:
        raise MetadataLinkError("metadata link inputs changed during proof")
    if write:
        write_changed(root / FINAL_PROOF, json_bytes(proof))
    return proof, before


def prove(root, *, write=True):
    return _prove(root, write=write)[0]


def enabled(root):
    return (root / "Makefile").is_file() and MARKER in (root / "Makefile").read_text()


def provenance_paths(root):
    if not enabled(root):
        return set()
    paths = {root / n for n in context_paths(root)}
    paths.update(root / n for n in (BOOT_ELF, BOOT_MAP, BOOT_LOG, FINAL_LOG, "build/mickey.us.elf", "build/mickey.us.map", MANIFEST, RECEIPT, FINAL_SCRIPT, FINAL_PROOF,
                                META + "/borrowing.json", "tools/runlink_metadata_link.py"))
    try:
        manifest = json.loads((root / MANIFEST).read_text())
        paths.update(root / META / safe_relative(n) for n in manifest["files"])
        paths.update(root / f"build/runlink/generated/{n}.o" for n in manifest["files"] if metadata_name(n))
        paths.update(root / safe_relative(n) for n in manifest["inputs"])
    except (OSError, ValueError, KeyError, TypeError, MetadataLinkError):
        pass  # Fixed paths remain in the snapshot even before a first build.
    return paths


def provenance_context(root):
    if not enabled(root):
        return {}
    _proof, snapshot = _prove(root, write=False)
    return {p.relative_to(root).as_posix(): sha for p, sha in sorted(snapshot.items())}



_EDITED_RUN_STARTED = False
_EDITED_PROJECT_PIN = None


def edited_projection_tools():
    """Pin the installed linker writer before first loading its code."""
    import importlib.util
    import sweep_receipts
    global _EDITED_PROJECT_PIN
    packages = ('splat', 'yaml', 'pylibyaml', 'spimdisasm', 'intervaltree', 'sortedcontainers', 'tqdm', 'colorama')
    if _EDITED_PROJECT_PIN is None:
        rp.edited_require(not any(name.split('.')[0] in packages for name in sys.modules),
                          'edited linker projection package was preloaded without an identity')
        paths = []
        for name in packages:
            found = importlib.util.find_spec(name)
            rp.edited_require(found is not None and found.origin is not None,
                              'missing installed linker projection dependency: ' + name)
            path = Path(found.origin)
            paths.append(path.parent if found.submodule_search_locations else path)
        _EDITED_PROJECT_PIN = {path: (sweep_receipts.tree_digest(path) if path.is_dir() else digest(path)) for path in paths}
    current = {path: (sweep_receipts.tree_digest(path) if path.is_dir() else digest(path)) for path in _EDITED_PROJECT_PIN}
    rp.edited_require(current == _EDITED_PROJECT_PIN, 'loaded linker projection tool changed')
    return {str(path): value for path, value in current.items()}


def edited_source_context(root):
    """Conservative real resident source/include/ASM dependency closure."""
    import sweep_receipts
    names = ('src/main', 'src/libultra', 'src/libgcc', 'include', 'tools/asm-processor', 'asm/main', 'asm/libultra',
             'asm/data/main', 'asm/nonmatchings/main', 'asm/nonmatchings/libultra')
    return {name: sweep_receipts.tree_digest(root / name) if (root / name).is_dir() else None for name in names}


def edited_projection(root, directory, selected=None, generated=None):
    """Emit provider/outside scripts; selected inputs never initialize as old segments."""
    project_tools = edited_projection_tools()
    import copy
    import yaml
    from splat.scripts import split
    from splat.util import conf
    source = (root / 'mickey.us.yaml').read_bytes()
    original = yaml.safe_load(source)
    segments = original['segments']
    cut = next(i for i, segment in enumerate(segments)
               if isinstance(segment, dict) and segment.get('name') == 'assets')
    require = rp.edited_require

    def emit(label, items, *, source_root=None):
        document = copy.deepcopy(original)
        document['segments'] = items
        options = document['options']
        options.update(base_path=str(root), target_path='NOT-AN-EDITED-INPUT',
                       symbol_addrs_path=[], reloc_addrs_path=[],
                       ld_script_path=str(directory / (label + '.ld')),
                       undefined_funcs_auto_path=str(directory / 'unused-functions.txt'),
                       undefined_syms_auto_path=str(directory / 'unused-symbols.txt'))
        if source_root is not None:
            options['src_path'] = source_root
        path = directory / (label + '.yaml')
        write_changed(path, yaml.safe_dump(document, sort_keys=False))
        loaded = conf.load([path], modes=['ld'])
        split.write_linker_script(split.initialize_segments(loaded['segments']))
        return (directory / (label + '.ld')).read_text()

    prefix = emit('providers', segments[:cut] + [[segments[cut]['start']]])
    template = None
    if selected is not None:
        def outside(segment):
            if not isinstance(segment, dict):
                return True
            match = re.fullmatch(r'overlay_(\d{3})(?:_reloc[12])?', segment.get('name', ''))
            return match is None or int(match[1]) not in selected
        template = emit('outside', [segment for segment in segments if outside(segment)])
        if generated is not None:
            for number, declaration in sorted(selected.items()):
                label = f'overlay_{number:03d}'
                fresh = [{'name': label, 'type': 'code', 'start': 0,
                          'vram': rp.OVERLAY_VMA, 'subalign': 4, 'align': 8,
                          'subsegments': [[0, 'c', declaration['source'].removesuffix('.c')]]}]
                for table in (1, 2):
                    if getattr(generated[number]['module'], f'table{table}'):
                        fresh.append({'name': label + f'_reloc{table}', 'type': 'bin', 'start': 0,
                                      'align': 8, 'subalign': 1, 'dir': f'overlays/o{number:03d}'})
                fragment = emit('selected-' + str(number), fresh + [[0]], source_root='.')
                fragment = fragment[fragment.index('    ' + label + '_ROM_START'):fragment.index('    /DISCARD/')]
                following = [(int(match[1]), match.start()) for match in re.finditer(
                    r'(?m)^    overlay_(\d{3})_ROM_START = __romPos;', template) if int(match[1]) > number]
                position = min(following)[1] if following else template.index('    rom_fill_ROM_START = __romPos;')
                template = template[:position] + fragment + template[position:]
        write_changed(directory / 'template.ld', template)
    require((root / 'mickey.us.yaml').read_bytes() == source, 'loaded projection config changed')
    selectors = sorted(set(re.findall(r'(?m)^\s*(build/[\w./-]+\.o)\([^)]*\);', prefix)))
    write_changed(directory / 'inputs.mk', 'EDITED_PROVIDER_INPUTS := ' + ' '.join(selectors) + '\n')
    require(edited_projection_tools() == project_tools, 'projection tool changed during emission')
    return prefix, template


def edited_catalog(root, path, exports, selection=None):
    """Read explicit source/ABI ownership, without selected numeric layout data."""
    require = rp.edited_require
    def unique(items):
        result = {}
        for name, value in items:
            require(name not in result, 'duplicate edited declaration key')
            result[name] = value
        return result
    payload = path.read_bytes()
    document = json.loads(payload, object_pairs_hook=unique)
    require(isinstance(document, dict) and set(document) == {'schema', 'default_selected', 'modules'}
            and document['schema'] in ('mickey-edited-source-modules-v1', 'mickey-edited-source-modules-v2'),
            'unsupported edited declaration schema')
    extended = document['schema'].endswith('-v2')
    profiles, names, rows = {}, set(), set()
    require(isinstance(document['modules'], list), 'module declarations must be a list')
    for item in document['modules']:
        require(isinstance(item, dict) and set(item) == ({'module', 'source', 'exports', 'entries', 'build'}
                if extended else {'module', 'source', 'exports', 'entries'}),
                'unknown/incomplete module declaration')
        number, source = item['module'], item['source']
        build = item.get('build', 'registered')
        require(build in ('registered', 'complete-overlay'), 'unsupported edited compiler profile')
        require(type(number) is int and 0 < number < 0xffc and number not in profiles, 'duplicate/reserved module identity')
        require(isinstance(source, str), 'invalid source declaration')
        relative = safe_relative(source)
        lookup = root
        for part in relative.parts:
            lookup = lookup / part
            require(not lookup.is_symlink(), 'symlink source ownership')
        require(relative.parts[:3] == ('src', 'overlays', f'o{number:03d}') and relative.suffix == '.c'
                and (root / relative).is_file() and not (root / relative).is_symlink(), 'source/module ownership mismatch')
        require(isinstance(item['entries'], dict) and set(item['entries']) == {'init', 'resume'}
                and all(value is None or isinstance(value, str) and re.fullmatch(r'[A-Za-z_]\w*', value)
                        for value in item['entries'].values()), 'invalid entry declarations')
        require(isinstance(item['exports'], list), 'invalid exports declaration')
        declared, kinds, declared_rows = [], {}, {}
        for owner in item['exports']:
            require(isinstance(owner, dict) and set(owner) == {'name', 'row', 'kind'}, 'invalid export declaration fields')
            name, row, kind = owner['name'], owner['row'], owner['kind']
            require(isinstance(name, str) and re.fullmatch(r'[A-Za-z_]\w*', name)
                    and type(row) is int and row >= 0
                    and (exports.by_name.get(name) == row if row < len(exports.rows)
                         else extended and name not in exports.by_name)
                    and name not in names and row not in rows,
                    'duplicate/wrong symbolic export name/row')
            require(kind in ('function', 'object'), 'unsupported export kind')
            names.add(name); rows.add(row); declared.append(name); kinds[name] = kind; declared_rows[name] = row
        profiles[number] = {'source': source, 'exports': declared, 'kinds': kinds,
                            'rows': declared_rows, 'entries': item['entries'], 'build': build}
    ids = document['default_selected'] if selection is None else selection
    require(isinstance(ids, list) and ids and all(type(number) is int and number in profiles for number in ids)
            and len(set(ids)) == len(ids), 'invalid/undeclared selected module identities')
    appended = sorted(row for number in ids for row in profiles[number]['rows'].values()
                      if row >= len(exports.rows))
    require(appended == list(range(len(exports.rows), len(exports.rows) + len(appended))),
            'new export declarations must be a contiguous row suffix')
    require(path.read_bytes() == payload, 'declaration changed while parsing')
    return {number: profiles[number] for number in sorted(ids)}, hashlib.sha256(payload).hexdigest()


def edited_source_exports(selected, original, module_count):
    """Keep compatibility ABI rows fixed; author only a dense source tail."""
    require = rp.edited_require
    tail_ids = sorted(number for number in selected if number > module_count)
    require(tail_ids == list(range(module_count + 1, module_count + 1 + len(tail_ids))),
            'new modules must form a dense tail after compatibility modules')
    require(all(selected[number]['build'] == 'complete-overlay' for number in tail_ids),
            'new tail module requires explicit complete-overlay compiler profile')
    rows = list(original.rows)
    appended = sorted((row, name) for declaration in selected.values()
                      for name, row in declaration['rows'].items() if row >= len(rows))
    require([row for row, name in appended] == list(range(len(rows), len(rows) + len(appended))),
            'new export declarations must be a contiguous row suffix')
    rows.extend(name for row, name in appended)
    return rp.OrtNames(rows, dict(original.entries))


def edited_validate_imports(raw, exports, label):
    """Name a genuine current caller when a declared target was removed."""
    for section in ('.text', '.data', '.rodata'):
        for rel in raw.rels(section):
            if rel.sym.shndx == rp.SHN_UNDEF and rel.sym.name not in exports.by_name:
                caller = rp.edited_caller({'raw': raw}, section, rel.offset)
                raise MetadataLinkError(label + ' ' + caller + ': undeclared/removed target ' + rel.sym.name)


def edited_recipe(root, source, profile='registered'):
    """Authenticate the existing complete-module recipe, not an arbitrary object."""
    target = 'build/' + source + '.o'
    expanded = subprocess.check_output(['gmake', '--no-print-directory', '-n',
        '--assume-old=build/.splat-stamp', '-W', source, target,
        *(['EDITED_COMPLETE_SOURCE=' + source] if profile == 'complete-overlay' else [])], cwd=root, text=True)
    lines = expanded.replace('\\\n', ' ').splitlines()
    compiler = [line for line in lines if 'tools/ido/cc' in line and target in line]
    links = [shlex.split(line) for line in lines if 'tools/link_overlay_module.py' in line]
    rp.edited_require(len(compiler) == 1 and len(links) == 1
        and links[0][1:3] == ['tools/link_overlay_module.py', target],
        'selected source does not use one registered complete-module recipe')
    words = shlex.split(compiler[0])
    args = words[max(i for i, word in enumerate(words) if word == '--') + 1:]
    rp.edited_require(args.count('-o') == 1 and args[-1] == source, 'invalid actual compiler recipe')
    return args, compiler[0]


def edited_script(root, source, names, generated, selected, products):
    """Map only declared metadata/source selectors and reverse every transformation."""
    require = rp.edited_require
    result = mapped_script(source, names)
    substitutions = []
    for name in names:
        old = f'build/runlink/generated/{name}.o'
        new = f'{generated}/{name}.o'
        require(result.count(old) == 1, 'ambiguous generated selector')
        result = result.replace(old, new); substitutions.append((new, old))
    for number, declaration in selected.items():
        old = 'build/' + declaration['source'] + '.o'
        new = products[number][1].relative_to(root).as_posix()
        require(result.count(old) == 4, 'selected source must have four ordinary section selectors')
        result = result.replace(old, new); substitutions.append((new, old))
    first = '    overlay_001_ROM_START = __romPos;'
    require(result.count(first) == 1, 'missing independent module region boundary')
    aligned = '    __romPos = ALIGN(__romPos, 8);\n' + first
    result = result.replace(first, aligned, 1)
    start = result.index('\n    rom_fill_ROM_START = __romPos;')
    end = result.index('\n    /DISCARD/', start)
    fill = result[start:end]
    require(fill.count('build/assets/rom_fill.bin.o(.data);') == 1 and 'overlay_' not in fill, 'unexpected fill section')
    result = result[:start] + result[end:]
    reverse = result.replace('\n    /DISCARD/', fill + '\n    /DISCARD/', 1).replace(aligned, first, 1)
    for new, old in reversed(substitutions):
        reverse = reverse.replace(new, old)
    for name in names:
        reverse = reverse.replace(f'build/runlink/generated/{name}.o', f'build/assets/{name}.o')
    require(reverse == source, 'edited mapping changed undeclared script bytes')
    require('rom_fill_' not in result and 'assets/rom_fill.bin' not in result, 'misleading retained fill')
    return result


def edited_compile(root, directory, label, text, exports, args, commands):
    """Fresh current source/CPP/compiler/static-symbol/ordinary-link chain."""
    import permute_batch as batch
    import link_overlay_module as module_link
    import check_editable_layout as pilot
    import proof_provenance as pp
    rp.edited_require('GLOBAL_ASM' not in text and not re.search(r'\b(?:asm|__asm__)\b', text),
                      'selected complete C input must not require assembly substitution')
    source = directory / (label + '.c')
    raw = directory / (label + '.o')
    untouched = directory / (label + '.compiler.o')
    write_changed(source, text)
    source_hash = digest(source)
    closure = batch.source_dependencies(source, tuple(args))
    static_inputs = pp.literal_profile_inputs(root) | {'tools/static_symbols.py': digest(root/'tools/static_symbols.py')} | closure | {source.relative_to(root).as_posix(): source_hash}
    static_tools = batch.checked_tool_identity()
    flags = list(args)
    flags[flags.index('-o') + 1] = untouched.relative_to(root).as_posix()
    flags[-1] = source.relative_to(root).as_posix()
    pilot.run(root, ['tools/ido/cc', *flags], directory / (label + '-compile.log'), commands)
    compiler = rp.Elf(untouched)
    compiler_hash = hashlib.sha256(compiler.data).hexdigest()
    write_changed(raw, compiler.data)
    pilot.run(root, [sys.executable, 'tools/static_symbols.py', raw.relative_to(root)],
              directory / (label + '-statics.log'), commands)
    prepared = rp.Elf(raw)
    for section in rp.EDITED_SECTIONS:
        rp.edited_require(compiler.size(section) == prepared.size(section)
                          and compiler.bytes_of(section) == prepared.bytes_of(section),
                          'static-symbol step changed allocated compiler bytes')
        old, new = compiler.rels(section), prepared.rels(section)
        rp.edited_require([(r.offset, r.type) for r in old] == [(r.offset, r.type) for r in new]
                          and rp.in_place_addends(compiler, section, old)
                          == rp.in_place_addends(prepared, section, new), 'static-symbol step changed original records')
    edited_validate_imports(prepared, exports, label)
    linked = directory / (label + '-linked.o')
    receipt = module_link.link(raw, linked, exports, directory / (label + '-module'),
        ld=root / 'tools/binutils/mips64-elf-ld', objcopy=root / 'tools/binutils/mips64-elf-objcopy',
        rebind=root / 'tools/rebind_elf_relocations.py')
    linked_elf = rp.Elf(linked)
    rp.edited_require(digest(source) == source_hash
        and batch.source_dependencies(source, tuple(args)) == closure
        and digest(untouched) == compiler_hash
        and hashlib.sha256(prepared.data).hexdigest() == receipt['compiler_sha256']
        and hashlib.sha256(linked_elf.data).hexdigest() == receipt['configured_sha256'],
        'fresh compiler/CPP/ordinary module receipt changed')
    expected = {source.name: source_hash, untouched.name: compiler_hash,
                raw.name: receipt['compiler_sha256'], linked.name: receipt['configured_sha256']}
    for name, field in (('compiler.o', 'compiler_sha256'), ('bindings.o', 'prepared_sha256'),
            ('module.ld', 'script_sha256'), ('module.elf', 'linked_sha256'),
            ('localized.elf', 'localized_sha256'), ('module.o', 'configured_sha256'), ('body.bin', 'body_sha256')):
        expected[label + '-module/' + name] = receipt[field]
    expected[label + '-module/receipt.json'] = hashlib.sha256((json.dumps(receipt, indent=2) + '\n').encode()).hexdigest()
    rp.edited_require(all(digest(directory / name) == value for name, value in expected.items()),
                      'fresh module receipt products changed')
    obj = rp.edited_prepare_object(prepared, linked_elf)
    target_sections = {rel.sym.section for section in rp.EDITED_SECTIONS[:-1] for rel in prepared.rels(section)
                       if rel.sym.type == rp.STT_SECTION and rel.sym.section in ('.data', '.bss')}
    if target_sections:
        statics = pp.static_anchor_module(root)
        debug = [row for row in statics._sections(compiler.data) if row[-1] == '.mdebug']
        rp.edited_require(len(debug) == 1, 'static source needs actual compiler mdebug')
        anchors = {'.data': statics.data_anchors(compiler.data, debug[0]),
                   '.bss': statics.bss_anchors(compiler.data, debug[0])}
        names = {name for section in target_sections for name in anchors[section]}
        rp.edited_require(names, 'section operand lacks source-object boundary proof')
        def static_current():
            rp.edited_require(digest(root/'tools/proof_provenance.py') == pp._LOADED_METADATA_PROOF,
                              'loaded static source proof helper changed')
            return ({name: digest(root/name) for name in static_inputs}, batch.checked_tool_identity())
        rp.edited_require(static_current() == (static_inputs, static_tools), 'static source context changed before CPP')
        command = ['tools/ido/cc', *flags]
        cpp_command = ['tools/ido/cc', '-E', *pp._literal_flags(shlex.join(command), flags[-1]), flags[-1]]
        result = subprocess.run(cpp_command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)
        rp.edited_require(result.returncode == 0, 'actual edited static CPP failed')
        cpp = result.stdout
        try:
            facts = pp.static_source_facts(cpp, flags, root, names)
            witness = pp.static_source_witness(compiler, prepared, linked_elf, facts, source_hash,
                                              hashlib.sha256(cpp).hexdigest(), static_inputs.items(), root,
                                              hashlib.sha256(json_bytes(static_tools)).hexdigest())
        except pp.MetadataProofError as error:
            raise MetadataLinkError('static source proof: ' + str(error)) from error
        rp.edited_require(static_current() == (static_inputs, static_tools)
            and digest(untouched) == witness.compiler_sha256 and digest(raw) == witness.raw_sha256
            and digest(linked) == witness.linked_sha256, 'static source/CPP/object context changed')
        cpp_path = directory/(label+'-effective.i')
        audit_path = directory/(label+'-static-source.json')
        audit = {'schema': 'mickey-static-source-extent-v1', 'compiler_command': command,
            'effective_cpp_command': cpp_command, 'cpp_artifact': cpp_path.name, 'cpp_sha256': witness.cpp_sha256,
            'source_sha256': witness.source_sha256, 'compiler_sha256': witness.compiler_sha256,
            'prepared_sha256': witness.raw_sha256, 'linked_sha256': witness.linked_sha256,
            'prepared_surface': witness.raw_surface, 'linked_surface': witness.linked_surface,
            'declarations': facts, 'extents': [dataclasses.asdict(x) for x in witness.extents],
            'restored_elf_symbol_size': 0, 'inputs': static_inputs, 'toolchain_sha256': witness.toolchain_sha256}
        audit_bytes = json_bytes(audit)
        write_changed(cpp_path, cpp);write_changed(audit_path, audit_bytes)
        expected[cpp_path.name] = witness.cpp_sha256
        expected[audit_path.name] = hashlib.sha256(audit_bytes).hexdigest()
        rp.edited_require(static_current() == (static_inputs, static_tools)
            and digest(cpp_path) == witness.cpp_sha256 and digest(audit_path) == expected[audit_path.name],
            'static CPP/audit context changed before attachment')
        obj['static_source'] = witness
        obj['static_source_audit'] = audit
    return (obj, (raw, linked, receipt, expected), closure)


def edited_program_selectors(template):
    """Current selected-free program dependencies; never retained metadata/fill."""
    selectors = set(re.findall(r'(?m)^\s*(build/[\w./-]+\.o)\([^)]*\);', template))
    return sorted(name for name in selectors if not (name.startswith('build/assets/')
                  and (metadata_name(name[13:-2]) or name.endswith('/rom_fill.bin.o'))))


def edited_outside_context(root, selectors):
    """Hold actual source/extracted inputs used by ordinary outside production."""
    import sweep_receipts
    files, assembly = {}, {}
    for selector in selectors:
        require = rp.edited_require
        require(selector.startswith('build/') and selector.endswith('.o'), 'invalid outside program input')
        source = safe_relative(selector.removeprefix('build/').removesuffix('.o'))
        path = root / source
        files[source.as_posix()] = digest(path)
        if selector.endswith('.c.o'):
            match = re.search(r'/overlays/(o\d{3})/', selector)
            if match:
                name = 'asm/nonmatchings/overlays/' + match[1]
                directory = root / name
                assembly[name] = sweep_receipts.tree_digest(directory) if directory.is_dir() else None
    return {'files': files, 'assembly': assembly}


def edited_bss_valid(section, expected_size, body_end):
    """A zero-storage GNU ld descriptor is not an invented NOBITS reserve."""
    if section is None:
        return expected_size == 0
    if expected_size == 0 and (section['type'], section['flags'], section['size'], section['align'], section['addr']) \
            == (1, 1, 0, 1, body_end):
        return True
    return (section['type'] == rp.SHT_NOBITS and section['flags'] == 3
            and section['size'] == expected_size and section['addr'] == body_end)


def edited_loaded_identity():
    """The eager encoder imports must retain their actual load-time identities."""
    from runlink import codec, rules
    rp.metadata_implementations()
    rp.edited_require(digest(Path(__file__)) == _LOADED
        and digest(Path(codec.__file__)) == codec.LOADED_SOURCE
        and digest(Path(rules.__file__)) == rules.LOADED_SOURCE,
        'loaded edited encoder/consumer implementation changed')


def edited_directory(root, name):
    relative = safe_relative(name)
    rp.edited_require(relative.parts[:2] == ('build', 'edited-runlink'), 'edited output must be separate')
    path = root
    for part in relative.parts:
        path = path / part
        rp.edited_require(not path.is_symlink(), 'symlink edited output directory')
    return path


def edited_source_closures_current(products, closures, arguments):
    """Rewalk existing literal lookup results, including newly shadowing files."""
    import permute_batch as batch
    try:
        return all(batch.source_dependencies(products[n][0].with_suffix('.c'), tuple(arguments[n])) == closure
                   for n, closure in closures.items())
    except (OSError, ValueError, RuntimeError):
        return False


def edited_compiler_flags(arguments):
    """Stable ordered compiler identity, excluding output/source path plumbing."""
    rp.edited_require(arguments.count('-o') == 1, 'invalid edited compiler output switches')
    output = arguments.index('-o')
    return [word for index, word in enumerate(arguments)
            if index not in (output, output + 1, len(arguments) - 1)]


def edited_current(root, parent, variant, request):
    """Return only a current validated edited receipt; never bless fresh hashes."""
    edited_loaded_identity()
    import permute_batch as batch
    import check_editable_layout as pilot
    marker = parent / (variant + '-current.json')
    try:
        marker_data = marker.read_bytes()
        current = json.loads(marker_data)
        directory = root / safe_relative(current['path'])
        rp.edited_require(directory.resolve().is_relative_to(parent.resolve()), 'foreign edited marker')
        payload = (directory / 'receipt.json').read_bytes()
        receipt = json.loads(payload)
        rp.edited_require(hashlib.sha256(payload).hexdigest() == current['receipt_sha256']
            and receipt['schema'] == 'mickey-edited-source-build-v2'
            and receipt['variant'] == variant and receipt.get('request') == request, 'invalid edited receipt/request')
        expected = receipt['artifacts']
        rp.edited_require(pilot.artifact_hashes(directory) == expected, 'edited artifacts changed')
        rp.edited_require(all(digest(root / safe_relative(name)) == value
            for name, value in {**receipt['inputs'], **receipt['provider_holds']}.items()), 'edited inputs changed')
        rp.edited_require(batch.checked_tool_identity() == receipt['toolchain']
            and edited_projection_tools() == receipt['projection_toolchain']
            and edited_source_context(root) == receipt['resident_source_context']
            and edited_outside_context(root, receipt['outside_selectors']) == receipt['outside_input_context'],
            'edited toolchain/source changed')
        for label, record in receipt['source_cpp'].items():
            source = directory / record['source']
            rp.edited_require(batch.source_dependencies(source, tuple(receipt['arguments'][label])) == record['closure'],
                              'edited source CPP closure changed')
        rp.edited_require(marker.read_bytes() == marker_data
            and (directory / 'receipt.json').read_bytes() == payload
            and pilot.artifact_hashes(directory) == expected
            and all(digest(root / safe_relative(name)) == value
                for name, value in {**receipt['inputs'], **receipt['provider_holds']}.items())
            and batch.checked_tool_identity() == receipt['toolchain']
            and edited_projection_tools() == receipt['projection_toolchain']
            and edited_source_context(root) == receipt['resident_source_context']
            and edited_outside_context(root, receipt['outside_selectors']) == receipt['outside_input_context'],
            'edited receipt inputs changed during check')
        return receipt
    except (OSError, ValueError, KeyError, TypeError, rp.PackError):
        return None  # Stale/missing products rebuild in a new preserved run.


def edited_build(root, out, variant, outside_rom=None, declarations='config/runlink-edited.us.json', selection=None):
    """Explicit nonmatching source-selected build; retail entry points stay strict."""
    global _EDITED_RUN_STARTED
    edited_loaded_identity()
    if not _EDITED_RUN_STARTED:
        rp.edited_require(not any(name in sys.modules for name in
            ('check_editable_layout', 'runlink.compare', 'runlink.config')),
            'unbound edited proof modules were preloaded; use the fresh edited CLI')
        _EDITED_RUN_STARTED = True
    from dataclasses import asdict
    import tempfile
    import permute_batch as batch
    import check_editable_layout as pilot
    from runlink import codec, config, rules, compare
    require = rp.edited_require
    require(variant in ('control', 'peer', 'unreferenced'), 'unsupported edited source variant')
    parent = edited_directory(root, out)
    parent.mkdir(parents=True, exist_ok=True)
    declaration_path = root / safe_relative(declarations)
    early_paths = {declaration_path, root / 'Makefile', root / 'mickey.us.yaml', root / 'tools/runlink_metadata_link.py',
                   root / 'tools/runlink_pack.py', root / 'tools/static_symbols.py', root / 'tools/runlink/configs/msu-us.toml',
                   root / 'config/runlink-ort.us.txt', root / 'config/overlay_funcs_to_trap.us.txt'}
    if variant != 'control':
        early_paths.update((root / pilot.FIXTURE).glob('*'))
    import proof_provenance as pp
    early_paths.update(root/name for name in pp.literal_profile_inputs(root))
    early_paths.update((root / 'mk').glob('*.mk'))
    early_paths.update((root / 'tools').glob('*.py'))
    early_paths.update((root / 'tools').glob('*.sh'))
    early = {str(path.relative_to(root)): digest(path) for path in early_paths}
    tool = batch.checked_tool_identity()
    project_tools = edited_projection_tools()
    source_context = edited_source_context(root)
    require(all(digest(path) == value for path, value in pilot.LOADED.items()), 'loaded edited implementation changed')
    exports0 = rp.OrtNames.load(root / 'config/runlink-ort.us.txt')
    selected, declaration_hash = edited_catalog(root, declaration_path, exports0, selection)
    require(declaration_hash == early[declaration_path.relative_to(root).as_posix()], 'loaded declaration differs from prehold')
    early.update({declaration['source']: digest(root/declaration['source']) for declaration in selected.values()})
    # Derive the current request's outside closure EVERY invocation, before Make
    # and before warm lookup. The resident setup include is selection-independent.
    with tempfile.TemporaryDirectory(prefix='.outside-setup-', dir=parent) as temporary:
        _, outside_script = edited_projection(root, Path(temporary), selected)
        outside_selectors = edited_program_selectors(outside_script)
        projection_hash = hashlib.sha256(outside_script.encode()).hexdigest()
    outside_context = edited_outside_context(root, outside_selectors)
    setup_command = ['gmake', '--no-print-directory', '-j' + str(__import__('os').cpu_count() or 1),
                     'edited-runlink-inputs', 'EDITED_PROVIDER_INPUTS=' + ' '.join(outside_selectors)]
    setup = subprocess.run(setup_command, cwd=root, text=True,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    write_changed(parent / 'input-production.log', setup.stdout)
    require(setup.returncode == 0, 'ordinary outside input production failed; see input-production.log')
    require(all(digest(root / name) == value for name, value in early.items())
            and batch.checked_tool_identity() == tool
            and edited_projection_tools() == project_tools
            and edited_source_context(root) == source_context
            and edited_outside_context(root, outside_selectors) == outside_context,
            'input production recipe/tool/source/extracted inputs changed')
    arguments, recipes = {}, {}
    for number, declaration in selected.items():
        arguments[number], recipes[number] = edited_recipe(root, declaration['source'], declaration['build'])
    require(all(digest(root/name) == value for name,value in early.items())
            and batch.checked_tool_identity() == tool, 'edited recipe/source changed before warm lookup')
    request = {'outside_rom': safe_relative(outside_rom or 'baseroms/mickey.us.z64').as_posix(),
               'config': 'tools/runlink/configs/msu-us.toml',
               'declaration': declaration_path.relative_to(root).as_posix(), 'declaration_sha256': declaration_hash,
               'selected_modules': sorted(selected), 'variant': variant,
               'compiler_flags': {str(number): edited_compiler_flags(args) for number,args in arguments.items()}}
    current = edited_current(root, parent, variant, request)
    if current is not None:
        require(all(digest(root/name) == value for name,value in early.items())
                and batch.checked_tool_identity() == tool, 'edited source/recipe changed during warm lookup')
        print('edited source build current', variant, current['rom_sha1'], flush=True)
        return current
    directory = Path(tempfile.mkdtemp(prefix=variant + '-', dir=parent))
    commands = []
    cfg = config.load('msu-us', root=root)
    if outside_rom is not None:
        from dataclasses import replace
        cfg = replace(cfg, rom=root / safe_relative(outside_rom))
    declared_exports = edited_source_exports(selected, exports0, cfg.module_count)
    selected, exports, overrides, diagnostic = pilot.edited_fixture_inputs(root, selected, declared_exports, variant)
    for number in selected:
        if number not in arguments:
            arguments[number] = arguments[diagnostic['module']]
            recipes[number] = 'approved existing diagnostic peer uses its source module compiler flags'
    write_changed(directory / 'exports.txt', exports.render())
    # Snapshot all implementation/recipe/source/declaration inputs BEFORE parsing.
    inputs = {root / n for n in ('Makefile', 'mickey.us.yaml', 'config/runlink-ort.us.txt',
        'tools/runlink_metadata_link.py', 'tools/runlink_pack.py', 'tools/check_editable_layout.py',
        'tools/link_overlay_module.py', 'tools/static_symbols.py', 'tools/rebind_elf_relocations.py',
        'tools/runlink/codec.py', 'tools/runlink/rules.py', 'tools/runlink/compare.py',
        'tools/binutils/mips64-elf-ld', 'tools/binutils/mips64-elf-objcopy', 'tools/n64crc')}
    inputs.add(declaration_path)
    inputs.update(root / declaration['source'] for declaration in selected.values())
    inputs.update((root / 'mk').glob('*.mk'))
    if variant != 'control':
        inputs.update((root / pilot.FIXTURE).glob('*'))
    inputs.add(cfg.path)
    initial = {str(p.relative_to(root)): digest(p) for p in inputs}
    initial.update({name: digest(root / name) for name in early})
    require(declaration_hash == initial[declaration_path.relative_to(root).as_posix()]
            and all(initial[name] == value for name, value in early.items()), 'loaded recipe/declaration differs from snapshot')
    prefix, template = edited_projection(root, directory, selected)
    require(edited_program_selectors(template) == outside_selectors,
            'current outside selector projection changed after input production')
    outside_inputs = {root / name for name in outside_selectors}
    # Retained outside input objects are explicit compatibility prerequisites.
    # No selected source/image/table/header input or full bootstrap is opened.
    outside_hashes = {str(p.relative_to(root)): digest(p) for p in outside_inputs}
    initial.update(outside_hashes)
    provider_command = ['tools/binutils/mips64-elf-ld', '-r', '-T', (directory / 'providers.ld').relative_to(root),
                        '-Map', (directory / 'providers.map').relative_to(root), '-o', (directory / 'providers.elf').relative_to(root)]
    pilot.run(root, provider_command, directory / 'providers.log', commands)
    provider = rp.edited_provider_context(root, directory / 'providers.elf', directory / 'providers.map')
    objects, products, closures = {}, {}, {}
    for number, declaration in selected.items():
        text = overrides.get(number, (root / declaration['source']).read_text())
        objects[number], products[number], closures[number] = edited_compile(
            root, directory, 'overlay' + str(number), text, exports, arguments[number], commands)
        if 'static_source_audit' in objects[number]:
            held = objects[number]['static_source_audit']['inputs']
            require(all(name not in initial or initial[name] == value for name,value in held.items()),
                    'conflicting held static source input observation')
            initial.update(held)
    for number, obj in objects.items():
        actual_registered = {symbol.name for symbol in obj['linked'].symbols
            if symbol.bind == rp.STB_GLOBAL and symbol.section in rp.EDITED_SECTIONS
            and symbol.shndx not in (rp.SHN_UNDEF, rp.SHN_ABS) and symbol.name in exports.by_name}
        require(actual_registered <= set(selected[number]['exports']),
                'selected allocated registry export lacks explicit owner declaration')
    generated = {n: rp.edited_module_records(n, objects, selected, exports, provider) for n in objects}
    peer_removal = None
    if variant == 'unreferenced':
        preliminary_ort = [None] * len(exports.rows)
        for product in generated.values():
            identities = dict(product['exports'])
            identities.update({ref.row: ref.base for ref in product['refs'] if ref.row is not None})
            for row, identity in identities.items():
                preliminary_ort[row] = codec.OrtRow(*rules.identity_row(identity, cfg))
        preliminary = codec.Tables([], preliminary_ort, [p['module'] for p in generated.values()])
        pilot.removed_targets(preliminary, {diagnostic['peer']}, exports.rows,
            {key: name for product in generated.values() for key, name in product['callers'].items()})
        source_number, peer_number = diagnostic['module'], diagnostic['peer']
        before = generated[source_number]['module']
        exports = rp.OrtNames(exports.rows[:-diagnostic['peer_export_count']], dict(exports0.entries))
        write_changed(directory / 'exports.txt', exports.render())
        del selected[peer_number], objects[peer_number], products[peer_number], closures[peer_number], arguments[peer_number]
        objects[source_number], products[source_number], closures[source_number] = edited_compile(root, directory,
            'overlay-removed', overrides[source_number], exports, arguments[source_number], commands)
        generated = {source_number: rp.edited_module_records(source_number, objects, selected, exports, provider)}
        after = generated[source_number]['module']
        require((before.image, before.table1, before.table2) == (after.image, after.table1, after.table2),
                'unreferenced tail removal changed surviving ordinary source module')
        peer_removal = 'unreferenced tail removed; declarations pruned; fresh surviving compiler/link exact'
    selected_rows = {exports0.by_name[name] for declaration in selected.values()
                     for name in declaration['exports'] if name in exports0.by_name}
    # Decode only OUTSIDE compatibility. Selected header cells, ORT target cells,
    # payloads and runtime records are excluded before interpretation/slicing.
    outside_bytes = cfg.rom.read_bytes()
    outside_hash = hashlib.sha256(outside_bytes).hexdigest()
    initial[cfg.rom.relative_to(root).as_posix()] = outside_hash
    outside = codec.decode(outside_bytes, cfg, exclude_modules=set(selected), exclude_rows=selected_rows)
    require(not any(row is not None and row.selector in selected for row in outside.ort),
            'outside catalog contains an undeclared selected-module ORT row')
    ort = list(outside.ort) + [None] * (len(exports.rows) - len(outside.ort))
    for product in generated.values():
        for row, identity in product['exports'].items():
            ort[row] = codec.OrtRow(*rules.identity_row(identity, cfg))
    require(all(row is not None for row in ort), 'selected export declaration left an ORT hole')
    # Every selected imported resident export is independently checked against its
    # actual allocated provider identity, never accepted from its retained row.
    for product in generated.values():
        for ref in product['refs']:
            if ref.row is not None:
                require(tuple(rules.row_identity(ort[ref.row], cfg)) == ref.base,
                        'declared ORT identity disagrees with actual provider')
    tables, dynamic, header_end = pilot.checked_layout(outside.main, ort,
        sorted(outside.modules + [p['module'] for p in generated.values()], key=lambda m: m.number), cfg)
    callers = {key: name for p in generated.values() for key, name in p['callers'].items()}
    outside_callers = {}
    for record in outside.main:
        if record.symidx not in selected_rows:
            continue
        require(record.type == rp.R_MIPS_26 and record.op == codec.OP_SYMBOL,
                'selected external resident reference is outside supported call profile')
        site = cfg.resident_text + record.site
        homes = [item for item in provider['map'] if item.insec == '.text'
                 and item.addr <= site and site + 4 <= item.addr + item.size]
        require(len(homes) == 1, 'resident selected caller lacks unique actual input')
        item = homes[0]
        raw = rp.Elf(root / item.file)
        sidecar_path = root / (item.file + '.trapsites')
        payload = sidecar_path.read_bytes()
        sidecar = json.loads(payload)
        require(sidecar['text_sha256'] == hashlib.sha256(raw.bytes_of('.text')).hexdigest(),
                'resident selected-call sidecar is stale')
        offset = site - item.addr
        named = [r for r in sidecar['sites'] if r['offset'] == offset]
        rels = [r for r in raw.rels('.text') if r.offset == offset and r.type == record.type]
        functions = [symbol for symbol in raw.symbols if symbol.type == rp.STT_FUNC
                     and symbol.size and symbol.section == '.text'
                     and symbol.value <= offset and offset + 4 <= symbol.value + symbol.size]
        require(len(named) == len(rels) == len(functions) == 1
                and named[0]['name'] == exports.rows[record.symidx]
                and rels[0].sym.name == rp.TRAP, 'resident call/declared selected export differs')
        callers[(0, record.site, record.type)] = functions[0].name
        outside_callers[functions[0].name] = named[0]['name']
        provider['holds'][root / item.file] = hashlib.sha256(raw.data).hexdigest()
        provider['holds'][sidecar_path] = hashlib.sha256(payload).hexdigest()
    removal = {}
    for number in selected:
        try:
            pilot.removed_targets(tables, {number}, exports.rows, callers)
        except pilot.PilotError as error:
            removal[number] = str(error)
    files = pilot.output_files(tables)
    wrappers = pilot.wrappers(root, directory, files, commands)
    _, template = edited_projection(root, directory, selected, generated)
    script = edited_script(root, template, files, wrappers.relative_to(root).as_posix(), selected, products)
    write_changed(directory / 'final.ld', script)
    command = ['tools/link_diagnose.sh', 'tools/binutils/mips64-elf-ld', '-T', (directory / 'final.ld').relative_to(root)]
    for name in ('undefined_funcs_auto.us.txt', 'undefined_syms_auto.us.txt', 'libultra_undefined_syms.us.txt', 'overlay_undefined_syms.us.txt'):
        inputs.add(root / name)
        initial[name] = digest(root / name)
        command += ['-T', name]
    command += ['-Map', (directory / 'final.map').relative_to(root), '--no-check-sections', '-o', (directory / 'final.elf').relative_to(root)]
    incoming = pilot.artifact_hashes(directory)
    require(all(incoming[name] == value for product in products.values() for name, value in product[3].items()),
            'loaded module products changed before final link')
    pilot.run(root, command, directory / 'final-link.log', commands)
    pilot.run(root, ['tools/binutils/mips64-elf-objcopy', '-O', 'binary', '--pad-to=0x2000000', '--gap-fill=0xFF',
        (directory / 'final.elf').relative_to(root), (directory / 'final.z64').relative_to(root)], directory / 'image.log', commands)
    pilot.run(root, ['tools/n64crc', (directory / 'final.z64').relative_to(root)], directory / 'crc.log', commands)
    # Final proof consumes actual outputs, with loaded observations held throughout.
    snapshot = pilot.artifact_hashes(directory)
    require(all(snapshot.get(name) == value for name, value in incoming.items()), 'final link changed its input snapshot')
    elf = rp.Elf(directory / 'final.elf')
    map_data = (directory / 'final.map').read_bytes()
    mapping = rp.parse_map(directory / 'final.map', map_data)
    rom = (directory / 'final.z64').read_bytes()
    loaded = {directory / 'final.elf': hashlib.sha256(elf.data).hexdigest(),
              directory / 'final.map': hashlib.sha256(map_data).hexdigest(),
              directory / 'final.z64': hashlib.sha256(rom).hexdigest()}
    require(all(snapshot[p.name] == value for p, value in loaded.items()), 'loaded final image/map bytes differ from snapshot')
    decoded = codec.decode(rom, dynamic)
    require(codec.encode_region(decoded, dynamic, gap_fill=255) == codec.encode_region(tables, dynamic, gap_fill=255),
            'actual final decoded metadata/body differs')
    require(len(rom) == 0x2000000 and all(b == 255 for b in rom[dynamic.modules_end:]), 'final FF fill differs')
    for number, product in generated.items():
        module = decoded.module(number)
        obj = objects[number]
        require(elf.bytes_of(f'.overlay_{number:03d}') == module.image
                == b''.join(obj['linked'].bytes_of(s) for s in rp.EDITED_SECTIONS[:-1]), 'selected ordinary payload differs')
        bss = elf.section(f'.overlay_{number:03d}_bss')
        bss_size = obj['linked'].size('.bss')
        require(edited_bss_valid(bss, bss_size, rp.OVERLAY_VMA + len(module.image)),
                'selected actual NOBITS differs')
        identities = compare.shipped_identities(module, decoded, dynamic)
        require({k: v['full'] for k, v in identities.items()}
                == {(r.site, r.type): r.full for r in product['refs']}, 'raw→final full identities differ')
        for name, witness in product['export_witnesses'].items():
            final = pilot.real_symbol(elf, name)
            require(final.value == rp.OVERLAY_VMA + witness['identity'][2] and final.size == witness['size'], 'final export differs')
    for name, witness in provider['witnesses'].items():
        actual = pilot.real_symbol(elf, name)
        require(actual.type == rp.STT_FUNC and actual.size == witness['size']
                and actual.value == witness['address'] and elf.section(actual.section)['flags'] & 6 == 6,
                'final allocated resident provider differs: ' + name)
    if variant == 'peer':
        try:
            pilot.removed_targets(decoded, {diagnostic['peer']}, exports.rows, callers)
        except pilot.PilotError as error:
            peer_removal = str(error)
            require(diagnostic['caller'] in peer_removal and diagnostic['init'] in peer_removal, 'peer removal lacks actual caller')
        else:
            raise MetadataLinkError('referenced peer removal was accepted')
    for module in outside.modules:
        require(decoded.module(module.number).image == module.image
                == elf.bytes_of(f'.overlay_{module.number:03d}'), 'unselected compatibility payload changed')
    anchors = {'main_reloc_table_ROM_START': dynamic.main_table_rom,
               'main_reloc_table_ROM_END': dynamic.ort_rom, 'overlay_rom_table_ROM_START': dynamic.ort_rom,
               'overlay_rom_table_ROM_END': dynamic.headers_rom, 'overlay_header_table_ROM_START': dynamic.headers_rom,
               'overlay_header_table_ROM_END': header_end, 'overlay_001_ROM_START': dynamic.modules_rom}
    for name, value in anchors.items():
        require(pilot.real_symbol(elf, name).value == value, 'actual loader boundary differs: ' + name)
    _, _, replay = pilot.resident_replay(root, directory, elf, mapping, commands, 'build/src/main/runlink.c.o',
                          initial['build/src/main/runlink.c.o'], 'loader-replay')
    snapshot.update(replay)
    allowed = [(16, 24)]  # Actual n64crc result, validated by a second no-op run.
    for item in mapping:
        if item.file == 'build/src/main/runlink.c.o' and item.insec in ('.text', '.data', '.rodata') and item.size:
            allowed.append((item.addr - cfg.resident_rom_delta, item.addr - cfg.resident_rom_delta + item.size))
    previous = 0
    for begin, end in sorted(allowed):
        require(previous <= begin <= end <= cfg.main_table_rom and outside_bytes[previous:begin] == rom[previous:begin],
                'unselected resident/assets prefix changed')
        previous = end
    require(outside_bytes[previous:cfg.main_table_rom] == rom[previous:cfg.main_table_rom], 'unselected prefix tail changed')
    checksum = hashlib.sha256(rom).hexdigest()
    crc_log = directory / 'crc-recheck.log'
    crc_hash = pilot.run(root, ['tools/n64crc', (directory / 'final.z64').relative_to(root)], crc_log, commands)
    snapshot[crc_log.name] = crc_hash
    require(digest(directory / 'final.z64') == checksum, 'edited checksum did not stabilize')
    inventory = {item.file for item in mapping if item.size and '/generated/' in item.file}
    require(inventory == {str((wrappers / (name + '.o')).relative_to(root)) for name in files}, 'generated wrapper inventory differs')
    require(not any(item.size and item.file.startswith('build/assets/')
        and (metadata_name(item.file[13:-2]) or item.file.endswith('/rom_fill.bin.o')) for item in mapping), 'retained metadata/fill final input')
    for path, expected in provider['holds'].items():
        require(digest(path) == expected, 'loaded provider changed before publication')
    for path, expected in loaded.items():
        require(digest(path) == expected, 'loaded final output changed before publication')
    edited_loaded_identity()
    require(all(digest(root / n) == expected for n, expected in initial.items())
            and batch.checked_tool_identity() == tool
            and edited_projection_tools() == project_tools
            and edited_source_context(root) == source_context
            and edited_source_closures_current(products, closures, arguments)
            and edited_outside_context(root, outside_selectors) == outside_context, 'edited source/tool/outside inputs changed')
    require(pilot.artifact_hashes(directory) == snapshot, 'validated artifacts changed during final proof')
    artifacts = snapshot
    receipt = {'schema': 'mickey-edited-source-build-v2', 'nonmatching': True, 'source_only': False,
        'general_relayout': False, 'variant': variant, 'request': request, 'inputs': initial, 'toolchain': tool,
        'projection_toolchain': project_tools, 'resident_source_context': source_context,
        'outside_selectors': outside_selectors, 'outside_input_context': outside_context,
        'input_production': {'command': setup_command, 'projection_sha256': projection_hash},
        'commands': commands, 'source_cpp': {str(n): {'source': products[n][0].name.removesuffix('.o') + '.c',
            'closure': closures[n]} for n in products}, 'arguments': arguments, 'compiler_recipe': recipes,
        'source_static': {str(n): obj['static_source_audit'] for n, obj in objects.items()
                          if 'static_source_audit' in obj},
        'selected_raw_records': {n: len(p['refs']) for n, p in generated.items()},
        'selected_header': {n: asdict(p['module'].header) for n, p in generated.items()},
        'selected_joins': {n: p['joins'] for n, p in generated.items()},
        'outside_authority': 'retained unselected headers/payloads/records/ORT identities; resident fixed homes',
        'outside_retained_records': len(outside.main) + sum(len(m.table1) + len(m.table2) for m in outside.modules),
        'generated_metadata_inputs': len(inventory), 'retained_metadata_inputs': 0,
        'retained_fill_inputs': 0, 'selected_removal_refusals': removal, 'peer_removal': peer_removal, 'actual_resident_callers': outside_callers,
        'rom_sha1': hashlib.sha1(rom).hexdigest(), 'provider_holds': {str(p.relative_to(root)): h for p, h in provider['holds'].items()}}
    pilot.publish_receipt(directory, receipt, artifacts)
    require(all(digest(root / n) == expected for n, expected in initial.items())
            and all(digest(path) == value for path, value in provider['holds'].items())
            and batch.checked_tool_identity() == tool
            and edited_projection_tools() == project_tools
            and edited_source_context(root) == source_context
            and edited_source_closures_current(products, closures, arguments)
            and edited_outside_context(root, outside_selectors) == outside_context
            and all(digest(path) == value for path, value in pilot.LOADED.items()),
            'edited inputs changed during receipt publication')
    write_changed(parent / (variant + '-current.json'), json_bytes({'path': str(directory.relative_to(root)),
        'receipt_sha256': digest(directory / 'receipt.json')}))
    print('edited source build PASS', directory.relative_to(root), receipt['selected_raw_records'], receipt['rom_sha1'], flush=True)
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("prepare", "prove", "edited", "edited-project"))
    parser.add_argument("--root", type=Path, default=REPO)
    parser.add_argument("--out", default="build/edited-runlink")
    parser.add_argument("--outside-rom")
    parser.add_argument("--declarations", default="config/runlink-edited.us.json")
    parser.add_argument("--modules")
    parser.add_argument("--variant", choices=("control", "peer", "unreferenced"), default="control")
    args = parser.parse_args()
    try:
        if args.command == "edited-project":
            directory = edited_directory(args.root.resolve(), args.out)
            directory.mkdir(parents=True, exist_ok=True)
            edited_projection(args.root.resolve(), directory)
        elif args.command == "edited":
            edited_build(args.root.resolve(), args.out, args.variant, args.outside_rom, args.declarations,
                         None if args.modules is None else [int(value) for value in args.modules.split(',')])
        else:
            (prepare if args.command == "prepare" else prove)(args.root.resolve())
    except (MetadataLinkError, rp.PackError, OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
