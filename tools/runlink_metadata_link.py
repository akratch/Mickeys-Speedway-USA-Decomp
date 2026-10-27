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

# The capture helper imports this module by name. Keep its loaded-code seals and
# projection pin identical when this file is the CLI entry point.
if __name__ == '__main__':
    sys.modules.setdefault('runlink_metadata_link', sys.modules[__name__])

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
            "config/runlink-edited.us.json", "config/runlink-ort.us.txt",
            "tools/link_diagnose.sh", "tools/run_logged.py",
            "tools/binutils/mips64-elf-ld", "tools/binutils/mips64-elf-objcopy",
            *[p.name for p in (root / "undefined_funcs_auto.us.txt", root / "undefined_syms_auto.us.txt",
                              root / "libultra_undefined_syms.us.txt", root / "overlay_undefined_syms.us.txt")]]


def prepare(root):
    abi_current = canonical_registry_current(root)
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
    abi_current()
    write_changed(root / RECEIPT, json_bytes(receipt))
    abi_current()


def allocated(elf):
    return sorted((s["name"], s["type"], s["flags"], s["addr"], s["size"], s["align"],
                   elf.bytes_of(s["name"])) for s in elf.sections if s["flags"] & 2)


def symbols(elf):
    return sorted((s.name, s.value, s.size, s.bind, s.type, s.section)
                  for s in elf.symbols if s.type != rp.STT_FILE)


def check_prepared(root):
    abi_current = canonical_registry_current(root)
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
    abi_current()
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
    import proof_provenance as provenance
    try:
        encoded_current = provenance.canonical_encoded_word_readback(root)
    except BaseException as error:
        if write:
            (root / FINAL_PROOF).unlink(missing_ok=True)
        if isinstance(error, provenance.MetadataProofError):
            raise MetadataLinkError("encoded-address final readback: " + str(error)) from error
        raise
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
    if encoded_current is not None:
        try:
            encoded_current()
        except BaseException as error:
            if write:
                (root / FINAL_PROOF).unlink(missing_ok=True)
            if isinstance(error, provenance.MetadataProofError):
                raise MetadataLinkError("encoded-address prepublication hold: " + str(error)) from error
            raise
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
                          'subsegments': [[0, 'c', edited_input_declarations(declaration)[0]['source'].removesuffix('.c')]]}]
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


def registry_surface(profiles, exports):
    """Primitive observation seal; object identity is not parsed authority."""
    return json_bytes({'modules': profiles, 'rows': exports.rows, 'by_name': exports.by_name,
                       'entries': sorted((number, role, name)
                                         for (number, role), name in exports.entries.items())})


def edited_input_declarations(declaration):
    """Authored order, never object locations or numeric section origins."""
    if 'inputs' in declaration:
        return declaration['inputs']
    return ({'source': declaration['source'], 'build': declaration['build']},)


def source_registry(root, path, expected=None):
    """One authored ABI; compatibility has names/rows, never inferred homes."""
    require = rp.edited_require
    def unique(items):
        result = {}
        for name, value in items:
            require(name not in result, 'duplicate registry declaration key')
            result[name] = value
        return result
    def declaration_path_current():
        lookup = root
        for part in path.relative_to(root).parts:
            lookup = lookup / part
            require(not lookup.is_symlink(), 'symlink registry declaration')
    declaration_path_current()
    payload = path.read_bytes()
    observed = hashlib.sha256(payload).hexdigest()
    require(expected is None or observed == expected, 'loaded registry differs from prehold')
    document = json.loads(payload, object_pairs_hook=unique)
    require(isinstance(document, dict)
            and set(document) == {'schema', 'default_selected', 'compatibility', 'modules'}
            and document['schema'] == 'mickey-source-module-abi-v1', 'unsupported source ABI registry')
    compatibility = document['compatibility']
    require(isinstance(compatibility, dict) and set(compatibility) == {'exports', 'entries'}
            and isinstance(compatibility['exports'], list)
            and isinstance(compatibility['entries'], list), 'invalid compatibility ABI declarations')
    profiles, names, rows, entries = {}, set(), {}, {}
    identifier = lambda value: isinstance(value, str) and re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', value)
    def export(owner, *, typed):
        require(isinstance(owner, dict) and set(owner) == ({'name', 'row', 'kind'} if typed else {'name', 'row'}),
                'invalid export declaration fields')
        name, row = owner['name'], owner['row']
        require(identifier(name) and type(row) is int and row >= 0
                and name not in names and row not in rows, 'duplicate/invalid ABI name or row')
        if typed:
            require(owner['kind'] in ('function', 'object'), 'unsupported export kind')
        names.add(name); rows[row] = name
    for owner in compatibility['exports']:
        export(owner, typed=False)
    require([owner['row'] for owner in compatibility['exports']] == sorted(rows),
            'compatibility rows are not ordered')
    for entry in compatibility['entries']:
        require(isinstance(entry, dict) and set(entry) == {'module', 'role', 'name'},
                'invalid compatibility entry fields')
        number, role, name = entry['module'], entry['role'], entry['name']
        require(type(number) is int and 0 < number < 0xffc and role in ('init', 'resume')
                and identifier(name) and (number, role) not in entries, 'duplicate/invalid compatibility entry')
        entries[number, role] = name
    def source_path(number, source):
        require(isinstance(source, str), 'invalid source declaration')
        relative = safe_relative(source)
        lookup = root
        for part in relative.parts:
            lookup = lookup / part
            require(not lookup.is_symlink(), 'symlink source ownership')
        require(relative.parts[:3] == ('src', 'overlays', f'o{number:03d}')
                and relative.suffix == '.c' and lookup.is_file(), 'source/module ownership mismatch')
    require(isinstance(document['modules'], list), 'module declarations must be a list')
    for item in document['modules']:
        fields = {'module', 'source', 'exports', 'entries', 'build'}
        require(isinstance(item, dict) and set(item) in (fields, fields | {'integer_word_profile'},
                                                        {'module', 'inputs', 'exports', 'entries'}),
                'unknown/incomplete module declaration')
        word_profile = item.get('integer_word_profile')
        require('integer_word_profile' not in item or isinstance(word_profile, str)
                and word_profile == 'isolated-full-expression', 'unsupported integer-word profile')
        number = item['module']
        members = item.get('inputs', ({'source': item.get('source'), 'build': item.get('build')},))
        require(isinstance(members, (list, tuple)) and bool(members), 'empty/invalid source input vector')
        require(all(isinstance(member, dict) and set(member) == {'source', 'build'}
                    and member['build'] in ('registered', 'complete-overlay') for member in members),
                'unsupported edited input recipe')
        require(type(number) is int and 0 < number < 0xffc and number not in profiles,
                'duplicate/reserved module identity')
        for member in members:
            source_path(number, member['source'])
        require(len({member['source'] for member in members}) == len(members), 'duplicate source input')
        if 'inputs' in item:
            require(all(member['source'] == safe_relative(member['source']).as_posix() for member in members),
                    'noncanonical source input path')
        require(isinstance(item['entries'], dict) and set(item['entries']) == {'init', 'resume'}
                and all(value is None or identifier(value) for value in item['entries'].values()),
                'invalid entry declarations')
        require(not any(owner == number for owner, role in entries),
                'compatibility entry overlaps source ownership')
        for role, name in item['entries'].items():
            if name is not None:
                entries[number, role] = name
        require(isinstance(item['exports'], list), 'invalid exports declaration')
        declared, kinds, declared_rows = [], {}, {}
        for owner in item['exports']:
            export(owner, typed=True)
            name = owner['name']
            declared.append(name); kinds[name] = owner['kind']; declared_rows[name] = owner['row']
        profiles[number] = {'exports': declared, 'kinds': kinds, 'rows': declared_rows,
                            'entries': item['entries'], 'integer_word_profile': word_profile}
        if 'inputs' in item:
            profiles[number]['inputs'] = tuple(dict(member) for member in members)
        else:
            profiles[number].update(source=item['source'], build=item['build'])
    require(sorted(rows) == list(range(len(rows))), 'authored ABI rows must be dense')
    default = document['default_selected']
    registry_selection(profiles, default)
    exports = rp.OrtNames([rows[i] for i in range(len(rows))], entries)
    seal = registry_surface(profiles, exports)
    default_seal = tuple(default)
    def current():
        declaration_path_current()
        require(path.read_bytes() == payload and registry_surface(profiles, exports) == seal
                and tuple(default) == default_seal, 'source ABI declaration/vector changed')
        for number, profile in profiles.items():
            for member in edited_input_declarations(profile):
                source_path(number, member['source'])
    current()
    return profiles, exports, default, observed, current


def registry_selection(profiles, selection):
    import copy
    rp.edited_require(isinstance(selection, list) and selection
        and all(type(number) is int and number in profiles for number in selection)
        and len(set(selection)) == len(selection), 'invalid/undeclared selected module identities')
    return {number: copy.deepcopy(profiles[number]) for number in sorted(selection)}


def edited_catalog(root, path, selection=None):
    profiles, exports, default, observed, current = source_registry(root, path)
    selected = registry_selection(profiles, default if selection is None else selection)
    current()
    return selected, observed


def registry_projection(root, path=None):
    profiles, exports, default, observed, current = source_registry(
        root, path or root / 'config/runlink-edited.us.json')
    text = exports.render()
    current()
    return text, current


def canonical_registry_current(root):
    text, declaration_current = registry_projection(root)
    path = root / 'config/runlink-ort.us.txt'
    payload = text.encode()
    def current():
        declaration_current()
        rp.edited_require(not path.is_symlink() and path.read_bytes() == payload,
                          'canonical symbolic ABI differs from authored registry')
    current()
    return current


def held_registry_projection(path, text):
    """Derived input, never a fallback or a cache authority."""
    rp.edited_require(not path.parent.is_symlink() and not path.is_symlink(), 'symlink ABI projection')
    payload = text.encode()
    if path.exists():
        rp.edited_require(path.is_file() and path.read_bytes() == payload, 'changed derived ABI projection')
    else:
        write_changed(path, payload)
    observed = hashlib.sha256(payload).hexdigest()
    rp.edited_require(digest(path) == observed, 'loaded ABI projection differs')
    return observed


def edited_source_exports(selected, original, module_count):
    """Keep compatibility ABI rows fixed; author only a dense source tail."""
    require = rp.edited_require
    tail_ids = sorted(number for number in selected if number > module_count)
    require(tail_ids == list(range(module_count + 1, module_count + 1 + len(tail_ids))),
            'new modules must form a dense tail after compatibility modules')
    require(all(member['build'] == 'complete-overlay' for number in tail_ids
                for member in edited_input_declarations(selected[number])),
            'new tail module requires explicit complete-overlay compiler profile')
    rows = list(original.rows)
    appended = sorted((row, name) for declaration in selected.values()
                      for name, row in declaration['rows'].items() if row >= len(rows))
    require([row for row, name in appended] == list(range(len(rows), len(rows) + len(appended))),
            'new export declarations must be a contiguous row suffix')
    rows.extend(name for row, name in appended)
    return rp.OrtNames(rows, dict(original.entries))


def edited_validate_imports(raw, exports, label, internal=()):
    """Name a genuine current caller when a declared target was removed."""
    for section in ('.text', '.data', '.rodata'):
        for rel in raw.rels(section):
            if rel.sym.shndx == rp.SHN_UNDEF and rel.sym.name not in exports.by_name and rel.sym.name not in internal:
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
        # The final selector is replaced by the one held aggregate product.
        old = 'build/' + edited_input_declarations(declaration)[0]['source'] + '.o'
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


def edited_compile(root, directory, label, text, exports, args, commands, *, integer_word_profile=None, capture_only=False):
    """Fresh current source/CPP/compiler/static-symbol/ordinary-link chain."""
    import permute_batch as batch
    import link_overlay_module as module_link
    import check_editable_layout as pilot
    import proof_provenance as pp
    rp.edited_require(integer_word_profile in (None, 'isolated-full-expression'),
                      'unsupported integer-word source profile')
    rp.edited_require('GLOBAL_ASM' not in text and not re.search(r'\b(?:asm|__asm__)\b', text),
                      'selected complete C input must not require assembly substitution')
    source = directory / (label + '.c')
    raw = directory / (label + '.o')
    untouched = directory / (label + '.compiler.o')
    write_changed(source, text)
    source_hash = digest(source)
    if capture_only:
        rp.edited_require(source_hash == hashlib.sha256(text.encode()).hexdigest(), 'vector source copy differs from input text')
    closure = batch.source_dependencies(source, tuple(args))
    static_inputs = pp.literal_profile_inputs(root) | {'tools/static_symbols.py': digest(root/'tools/static_symbols.py')} | closure | {source.relative_to(root).as_posix(): source_hash}
    static_tools = batch.checked_tool_identity()
    flags = list(args)
    flags[flags.index('-o') + 1] = untouched.relative_to(root).as_posix()
    flags[-1] = source.relative_to(root).as_posix()
    command_records = {}
    def capture_run(command, action, *, cpp=False):
        command = [str(word) for word in command]
        path = directory/(label+'-'+action+'-command.json')
        record = {'command':command, 'events':['planned','started']}
        payload = json_bytes(record); write_changed(path,payload)
        rp.edited_require(path.read_bytes() == payload, 'vector command record changed before start')
        try:
            result = subprocess.run(command,cwd=root,stdout=subprocess.PIPE,
                stderr=subprocess.PIPE if cpp else subprocess.STDOUT,timeout=120)
        except (OSError,subprocess.TimeoutExpired) as error:
            record.update(events=record['events']+['failed'],exit=None,error=str(error))
            write_changed(path,json_bytes(record)); raise
        log = directory/(label+'-'+action+'.log')
        logged = result.stderr if cpp else result.stdout
        write_changed(log,logged)
        rp.edited_require(log.read_bytes() == logged, 'vector command log changed')
        record.update(events=record['events']+['completed'],exit=result.returncode,
            stdout_sha256=hashlib.sha256(result.stdout).hexdigest(),log_sha256=hashlib.sha256(logged).hexdigest())
        payload=json_bytes(record);write_changed(path,payload)
        rp.edited_require(path.read_bytes() == payload, 'vector completed command record changed')
        command_records[path.name]=hashlib.sha256(payload).hexdigest()
        command_records[log.name]=hashlib.sha256(logged).hexdigest()
        commands.append(command)
        rp.edited_require(result.returncode == 0, 'vector command failed; see '+str(log.relative_to(root)))
        return result
    if capture_only:
        capture_run(['tools/ido/cc', *flags], 'compile')
    else:
        pilot.run(root, ['tools/ido/cc', *flags], directory / (label + '-compile.log'), commands)
    compiler = rp.Elf(untouched)
    compiler_hash = hashlib.sha256(compiler.data).hexdigest()
    write_changed(raw, compiler.data)
    if capture_only:
        capture_run([sys.executable, 'tools/static_symbols.py', raw.relative_to(root)], 'statics')
    else:
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
    if not capture_only:
        edited_validate_imports(prepared, exports, label)
    if capture_only:
        rp.edited_require(compiler.data[:24] == prepared.data[:24]
            and compiler.data[36:40] == prepared.data[36:40]
            and all({k:v for k,v in (compiler.section(name) or {}).items() if k != 'offset'}
                    == {k:v for k,v in (prepared.section(name) or {}).items() if k != 'offset'}
                    and compiler.bytes_of(name) == prepared.bytes_of(name)
                    for name in (*rp.EDITED_ABI, '.options')),
            'vector static preparation changed original ABI header/options')
        rp.edited_require(pp._static_parsed_surface(compiler) == pp._static_parsed_surface(prepared)
            and [vars(symbol) for symbol in compiler.symbols] == [vars(symbol) for symbol in prepared.symbols]
            and all([(rel.offset,rel.type,vars(rel.sym)) for rel in compiler.rels(section['name'])]
                == [(rel.offset,rel.type,vars(rel.sym)) for rel in prepared.rels(section['name'])]
                for section in compiler.sections),
            'vector static preparation changed original symbol-index/relocation surface')
        rp.edited_check_sections(compiler); rp.edited_check_sections(prepared)
        rp.edited_require(integer_word_profile is None, 'vector integer-word witness is unsupported')
        rp.edited_bss_allocation(prepared)
        command = ['tools/ido/cc', '-E', *pp._literal_flags(shlex.join(['tools/ido/cc', *flags]), flags[-1]), flags[-1]]
        result = capture_run(command, 'cpp', cpp=True)
        cpp_path = directory/(label+'-effective.i'); write_changed(cpp_path, result.stdout)
        expected = {source.name: source_hash, untouched.name: compiler_hash,
                    raw.name: hashlib.sha256(prepared.data).hexdigest(),
                    cpp_path.name: hashlib.sha256(result.stdout).hexdigest(), **command_records}
        observed = tuple((image, hashlib.sha256(image.data).hexdigest(), pp._static_parsed_surface(image))
                         for image in (compiler, prepared))
        expected_seal, args_seal, flags_seal = tuple(sorted(expected.items())), tuple(args), tuple(flags)
        closure_seal = json_bytes(closure)
        def current():
            rp.edited_require(tuple(sorted(expected.items())) == expected_seal
                and tuple(args) == args_seal and tuple(flags) == flags_seal
                and all(digest(directory/name) == value for name,value in expected_seal)
                and all(hashlib.sha256(image.data).hexdigest() == value
                        and pp._static_parsed_surface(image) == surface for image,value,surface in observed)
                and json_bytes(closure) == closure_seal
                and {name:digest(root/name) for name in static_inputs} == static_inputs
                and batch.source_dependencies(source, tuple(args)) == closure
                and batch.checked_tool_identity() == static_tools, 'vector compiler/CPP observation changed')
        current()
        capture = dict(raw=prepared, compiler=compiler, source_path=source, compiler_flags=flags,
                       source_closure=closure, source_sha256=source_hash, capture_current=current)
        return capture, (raw, None, None, expected), closure
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
    obj.update(compiler=compiler, source_path=source, compiler_flags=flags,
               source_closure=closure, source_sha256=source_hash)
    target_sections = {rel.sym.section for section in rp.EDITED_SECTIONS[:-1] for rel in prepared.rels(section)
                       if rel.sym.type == rp.STT_SECTION and rel.sym.section in ('.data', '.bss')}
    word_operands = any(addend == 0x80000000
        for section in rp.EDITED_SECTIONS[:-1]
        for addend in rp.in_place_addends(prepared, section, prepared.rels(section)))
    rp.edited_require(integer_word_profile is None or word_operands,
                      'declared integer-word profile lacks actual word operands')
    if target_sections or word_operands:
        def source_current():
            rp.edited_require(digest(root/'tools/proof_provenance.py') == pp._LOADED_METADATA_PROOF,
                              'loaded effective source proof helper changed')
            return ({name: digest(root/name) for name in static_inputs}, batch.checked_tool_identity())
        rp.edited_require(source_current() == (static_inputs, static_tools), 'source context changed before CPP')
        command = ['tools/ido/cc', *flags]
        cpp_command = ['tools/ido/cc', '-E', *pp._literal_flags(shlex.join(command), flags[-1]), flags[-1]]
        result = subprocess.run(cpp_command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)
        rp.edited_require(result.returncode == 0, 'actual edited effective CPP failed')
        cpp = result.stdout
    if target_sections:
        statics = pp.static_anchor_module(root)
        debug = [row for row in statics._sections(compiler.data) if row[-1] == '.mdebug']
        rp.edited_require(len(debug) == 1, 'static source needs actual compiler mdebug')
        anchors = {'.data': statics.data_anchors(compiler.data, debug[0]),
                   '.bss': statics.bss_anchors(compiler.data, debug[0])}
        names = {name for section in target_sections for name in anchors[section]}
        rp.edited_require(names, 'section operand lacks source-object boundary proof')
        try:
            facts = pp.static_source_facts(cpp, flags, root, names)
            witness = pp.static_source_witness(compiler, prepared, linked_elf, facts, source_hash,
                                              hashlib.sha256(cpp).hexdigest(), static_inputs.items(), root,
                                              hashlib.sha256(json_bytes(static_tools)).hexdigest())
        except pp.MetadataProofError as error:
            raise MetadataLinkError('static source proof: ' + str(error)) from error
        rp.edited_require(source_current() == (static_inputs, static_tools)
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
        rp.edited_require(source_current() == (static_inputs, static_tools)
            and digest(cpp_path) == witness.cpp_sha256 and digest(audit_path) == expected[audit_path.name],
            'static CPP/audit context changed before attachment')
        obj['static_source'] = witness
        obj['static_source_audit'] = audit
    if word_operands:
        try:
            roles = pp.integer_word_facts(cpp, flags, root, profile=integer_word_profile)
            witness = pp.integer_word_witness(compiler, prepared, linked_elf, roles, exports.by_name,
                source_hash, hashlib.sha256(cpp).hexdigest(), static_inputs.items(), root,
                hashlib.sha256(json_bytes(static_tools)).hexdigest())
        except pp.MetadataProofError as error:
            raise MetadataLinkError('integer-word source proof: ' + str(error)) from error
        rp.edited_require(source_current() == (static_inputs, static_tools)
            and digest(untouched) == witness.compiler_sha256 and digest(raw) == witness.raw_sha256
            and digest(linked) == witness.linked_sha256, 'integer-word source/object context changed')
        cpp_path = directory/(label+'-effective.i')
        audit_path = directory/(label+'-integer-word.json')
        audit = {'schema': 'mickey-integer-word-encoding-v1', 'encoding': 'xor-signbit-word',
            'profile': integer_word_profile,
            'compiler_command': command, 'effective_cpp_command': cpp_command,
            'cpp_artifact': cpp_path.name, 'cpp_sha256': witness.cpp_sha256,
            'source_sha256': source_hash, 'compiler_sha256': witness.compiler_sha256,
            'prepared_sha256': witness.raw_sha256, 'linked_sha256': witness.linked_sha256,
            'compiler_surface': witness.compiler_surface,
            'prepared_surface': witness.raw_surface, 'linked_surface': witness.linked_surface,
            'roles': [dataclasses.asdict(x) for x in witness.roles],
            'pairs': [dataclasses.asdict(x) for x in witness.pairs],
            'inputs': static_inputs, 'toolchain_sha256': witness.toolchain_sha256}
        audit_bytes = json_bytes(audit)
        write_changed(cpp_path, cpp);write_changed(audit_path, audit_bytes)
        expected[cpp_path.name] = witness.cpp_sha256
        expected[audit_path.name] = hashlib.sha256(audit_bytes).hexdigest()
        rp.edited_require(source_current() == (static_inputs, static_tools)
            and digest(cpp_path) == witness.cpp_sha256 and digest(audit_path) == expected[audit_path.name],
            'integer-word CPP/audit changed before attachment')
        obj['integer_word'] = witness
        obj['integer_word_audit'] = audit
    return (obj, (raw, linked, receipt, expected), closure)


def edited_compile_group(root, directory, label, declaration, exports, arguments, commands, initial):
    """Original C captures -> one ordinary ld-r group -> one operand preparation."""
    import link_overlay_module as module_link
    import proof_provenance as pp
    import check_editable_layout as pilot
    require = rp.edited_require
    captures, closures, expected, capture_holds = [], [], {}, []
    require(len(arguments) == len(declaration['inputs']), 'input recipe vector changed')
    for index, (member, args) in enumerate(zip(declaration['inputs'], arguments)):
        payload = (root/member['source']).read_bytes()
        require(hashlib.sha256(payload).hexdigest() == initial[member['source']], 'loaded vector source differs from prehold')
        capture, product, closure = edited_compile(root, directory, label+f'-input{index}',
            payload.decode(), exports, args, commands, capture_only=True)
        captures.append(capture); closures.append(closure); expected.update(product[3])
        capture_holds.append(capture['capture_current'])
    providers = rp.edited_group_providers([capture['raw'] for capture in captures], exports, declaration['exports'])
    internal_names = frozenset(name for name, _input, _symbol in providers)
    for index, capture in enumerate(captures):
        edited_validate_imports(capture['raw'], exports, label+f'-input{index}', internal_names)
    names = [capture['raw'].path.relative_to(root).as_posix() for capture in captures]
    aggregate_path = directory/(label+'.o'); map_path = directory/(label+'-aggregate.map')
    script_path = directory/(label+'-aggregate.ld')
    # Same explicit runtime-section treatment as the ordinary module linker.
    # Keep original ABI observations and check the linker's register-mask merge.
    lines = ['SECTIONS {']
    for section in (*rp.EDITED_SECTIONS, *rp.EDITED_ABI):
        suffix = ' (NOLOAD)' if section == '.bss' else ''
        selectors = ' '.join(json.dumps(name)+'('+section+')' for name in names)
        lines.append(f'    {section} 0{suffix} : {{ {selectors} }}')
    lines += ['    /DISCARD/ : { *(*) }', '}']
    script_data = ('\n'.join(lines)+'\n').encode(); write_changed(script_path,script_data)
    expected[script_path.name] = hashlib.sha256(script_data).hexdigest()
    command = ['tools/binutils/mips64-elf-ld', '-r', '-T', script_path.relative_to(root),
               '-Map', map_path.relative_to(root), '-o', aggregate_path.relative_to(root), *names]
    command_path = directory/(label+'-aggregate-command.json')
    write_changed(command_path,json_bytes({'command':[str(x) for x in command],'events':['planned','started']}))
    for capture,held in zip(captures,capture_holds):
        held(); rp.edited_check_sections(capture['raw'])
    require(digest(script_path) == expected[script_path.name], 'aggregate script changed before link')
    try:
        result = subprocess.run(command,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=120)
    except (OSError,subprocess.TimeoutExpired) as error:
        write_changed(command_path,json_bytes({'command':[str(x) for x in command],
            'events':['planned','started','failed'],'exit':None,'error':str(error)}))
        raise
    log_path=directory/(label+'-aggregate.log'); write_changed(log_path,result.stdout)
    command_data=json_bytes({'command':[str(x) for x in command],
        'events':['planned','started','completed'],'exit':result.returncode,
        'log_sha256':hashlib.sha256(result.stdout).hexdigest()})
    write_changed(command_path,command_data)
    expected[command_path.name]=hashlib.sha256(command_data).hexdigest()
    expected[log_path.name]=hashlib.sha256(result.stdout).hexdigest()
    commands.append([str(x) for x in command])
    require(result.returncode == 0, 'ordinary aggregate link failed; see '+str(log_path.relative_to(root)))
    aggregate = rp.Elf(aggregate_path); map_data = map_path.read_bytes()
    expected[aggregate_path.name] = hashlib.sha256(aggregate.data).hexdigest()
    expected[map_path.name] = hashlib.sha256(map_data).hexdigest()
    mapping = rp.parse_map(map_path, map_data)
    for capture in captures:
        flags=capture['compiler_flags']
        require(('-G0' in flags and '-G' not in flags) or
            (flags.count('-G') == 1 and flags[flags.index('-G')+1] == '0' and '-G0' not in flags),
            'vector ABI requires authentic explicit -G0 recipe')
    abi_audit = rp.edited_group_abi([capture['raw'] for capture in captures], aggregate, mapping, names)
    origins, joins = rp.edited_group_origins([capture['raw'] for capture in captures], aggregate, mapping, names)
    # Owned declarations cannot turn into external zero bindings when a provider is removed.
    for name in declaration['exports']:
        choices = [symbol for capture in captures for symbol in capture['raw'].symbols
                   if symbol.name == name and symbol.bind == rp.STB_GLOBAL
                   and symbol.shndx not in (rp.SHN_UNDEF, rp.SHN_ABS, rp.SHN_COMMON)]
        require(len(choices) == 1 and declaration['kinds'][name]
            == ('function' if choices[0].type == rp.STT_FUNC else 'object'),
            'declared vector export lacks one actual provider: '+name)
    for held in capture_holds: held()
    require(digest(aggregate_path) == expected[aggregate_path.name]
            and digest(map_path) == expected[map_path.name], 'loaded group changed before preparation')
    linked_path = directory/(label+'-linked.o')
    receipt = module_link.link(aggregate_path, linked_path, exports, directory/(label+'-module'),
        ld=root/'tools/binutils/mips64-elf-ld', objcopy=root/'tools/binutils/mips64-elf-objcopy',
        rebind=root/'tools/rebind_elf_relocations.py')
    linked = rp.Elf(linked_path)
    require(receipt['compiler_sha256'] == expected[aggregate_path.name]
            and receipt['configured_sha256'] == hashlib.sha256(linked.data).hexdigest(),
            'ordinary aggregate module receipt differs from loaded inputs')
    expected[linked_path.name] = hashlib.sha256(linked.data).hexdigest()
    for name, field in (('compiler.o','compiler_sha256'), ('bindings.o','prepared_sha256'),
            ('module.ld','script_sha256'), ('module.elf','linked_sha256'),
            ('localized.elf','localized_sha256'), ('module.o','configured_sha256'), ('body.bin','body_sha256')):
        expected[label+'-module/'+name] = receipt[field]
    expected[label+'-module/receipt.json'] = hashlib.sha256((json.dumps(receipt,indent=2)+'\n').encode()).hexdigest()
    obj = rp.edited_prepare_object(aggregate, linked)
    require(all((aggregate.section('.bss') or {}).get(key) == (linked.section('.bss') or {}).get(key)
                for key in ('type', 'flags', 'addr', 'size', 'align')),
            'ordinary module link changed proved NOBITS allocation')
    require(not linked.size('.bss') or obj['bases']['.bss'] % 4 == 0,
            'NOBITS zero-init base is not loader-word aligned')
    members = tuple(captures)
    for index, member in enumerate(members):
        member.update(linked=linked, bases=obj['bases'], input_origins=origins[index],
                      input_path=declaration['inputs'][index]['source'])
    audit = {'authority':'original compiler inputs; aggregate is ordinary linker output',
             'inputs':[dict(source=member['input_path'], compiler_sha256=hashlib.sha256(member['compiler'].data).hexdigest(),
                            prepared_sha256=hashlib.sha256(member['raw'].data).hexdigest(), origins=origins[i])
                       for i,member in enumerate(members)], 'original_records':joins,
             'aggregate_sha256':expected[aggregate_path.name], 'map_sha256':expected[map_path.name],
             'aggregate_stage_module_receipt':receipt, 'abi':abi_audit, 'providers':providers,
             'bss_objects':[rp.edited_bss_allocation(capture['raw']) for capture in captures]}
    closures = tuple(closures)
    obj.update(source_inputs=members, graph_audit=audit, source_closures=closures)
    expected_seal = tuple(sorted(expected.items()))
    argument_members = tuple(arguments)
    map_seal = tuple(tuple(vars(row).items()) for row in mapping)
    primitive = json_bytes(audit)
    bases = dict(obj['bases'])
    observations = tuple((image, hashlib.sha256(image.data).hexdigest(), pp._static_parsed_surface(image))
                         for image in (aggregate, linked))
    member_seals = tuple((member, member['raw'], member['compiler'], member['input_origins'],
                         tuple(member['input_origins'].items()), member['input_path'], member['capture_current'], member['source_closure'])
                        for member in members)
    def current():
        require(obj['raw'] is aggregate and obj['linked'] is linked and obj['source_inputs'] is members
            and obj['graph_audit'] is audit and obj['source_closures'] is closures and obj['bases'] == bases and json_bytes(audit) == primitive
            and tuple(sorted(expected.items())) == expected_seal
            and len(arguments) == len(argument_members)
            and all(args is original for args,original in zip(arguments,argument_members))
            and tuple(tuple(vars(row).items()) for row in mapping) == map_seal,
            'consumed vector graph/origin/map changed')
        require(all(digest(directory/name) == value for name,value in expected_seal)
            and all(hashlib.sha256(image.data).hexdigest() == value and pp._static_parsed_surface(image) == surface
                    for image,value,surface in observations), 'loaded vector group/output changed')
        for index,(member, raw, compiler, origin, offset_seal, source, held, closure) in enumerate(member_seals):
            require(members[index] is member and member['raw'] is raw and member['compiler'] is compiler
                and member['linked'] is linked and member['input_origins'] is origin
                and tuple(origin.items()) == offset_seal and member['input_path'] == source
                and member['capture_current'] is held and member['bases'] == bases
                and member['source_closure'] is closure and closures[index] is closure,
                'consumed original vector member changed')
            held()
    current()
    return obj, (aggregate_path,linked_path,receipt,expected), closures, current


def edited_cell_readback(elf, mapping, provider):
    """Independent final-link allocation and map role checks for C cells."""
    import check_editable_layout as pilot
    require = rp.edited_require
    for name, witness in provider.get('cells', {}).items():
        require(isinstance(witness, rp.EditedResidentCell), 'untrusted final resident cell witness')
        witness.current()
        actual = pilot.real_symbol(elf, name)
        section = elf.section(actual.section)
        homes = [item for item in mapping if item.file == witness.item.file and item.insec == '.bss']
        require(actual.type == rp.STT_OBJECT and actual.size == 4 and actual.section == '.main_bss'
            and actual.shndx not in (rp.SHN_UNDEF, rp.SHN_ABS, rp.SHN_COMMON)
            and section and section['type'] == rp.SHT_NOBITS and section['flags'] == 3
            and actual.value == witness.address and section['addr'] <= actual.value
            and actual.value+4 <= section['addr']+section['size']
            and homes == [witness.item], 'final allocated resident BSS cell differs: '+name)


def edited_word_readback(module, joins):
    """Join decoded stored words to the separately authenticated real object base."""
    from runlink import codec
    encoded = {(row['offset'], row['type']): row for row in joins if row.get('word_encoding')}
    seen = set()
    for rows in (module.table1, module.table2):
        for index, high in enumerate(rows):
            key = (high.site, high.type)
            if key not in encoded or high.type != codec.R_MIPS_HI16:
                continue
            require = rp.edited_require
            require(index + 1 < len(rows), 'decoded integer-word high lacks low')
            low = rows[index+1]
            low_key = (low.site, low.type)
            require(low.type == codec.R_MIPS_LO16 and low.op == high.op == codec.OP_LOCAL
                    and high.symidx == low.symidx == module.header.text and low_key in encoded,
                    'decoded integer-word pair/base differs')
            first, second = encoded[key], encoded[low_key]
            underlying = first['underlying_identity']
            require(underlying == second['underlying_identity'] and underlying[:2] == ('ov',module.number)
                    and first['word_encoding'] == second['word_encoding'], 'decoded integer-word target differs')
            high_bits = rp.u32(module.image,high.site) & 0xffff
            low_bits = rp.sext16(rp.u32(module.image,low.site))
            stored = ((high_bits << 16) + low_bits) & 0xffffffff
            # This equality proves the modulo-32 result for every runtime base,
            # independently of CPU pointer bounds or a chosen runtime address.
            require((high.symidx + stored) & 0xffffffff
                    == (underlying[2] + 0x80000000) & 0xffffffff,
                    'decoded integer-word modular encoding differs from real target')
            seen.update((key,low_key))
    rp.edited_require(seen == set(encoded), 'decoded integer-word operand coverage differs')


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
        for n, closure in closures.items():
            if arguments[n] and isinstance(arguments[n][0], list):
                if not all(batch.source_dependencies(products[n][0].with_name(products[n][0].stem+f'-input{i}.c'),
                    tuple(args)) == item for i,(args,item) in enumerate(zip(arguments[n],closure))): return False
            elif batch.source_dependencies(products[n][0].with_suffix('.c'), tuple(arguments[n])) != closure:
                return False
        return True
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
        for record in receipt.get('provider_cell_cpp', {}).values():
            rp.edited_require(batch.source_dependencies(root / safe_relative(record['source']),
                tuple(record['arguments'])) == record['closure'], 'resident provider CPP closure changed')
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
        for record in receipt.get('provider_cell_cpp', {}).values():
            rp.edited_require(batch.source_dependencies(root / safe_relative(record['source']),
                tuple(record['arguments'])) == record['closure'], 'resident provider CPP closure changed during check')
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
                   root / 'config/overlay_funcs_to_trap.us.txt'}
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
    profiles, registry, default, declaration_hash, registry_current = source_registry(
        root, declaration_path, early[declaration_path.relative_to(root).as_posix()])
    selected = registry_selection(profiles, default if selection is None else selection)
    projection_text = registry.render()
    projection = parent / 'abi' / (hashlib.sha256(projection_text.encode()).hexdigest() + '.txt')
    projection_hash = held_registry_projection(projection, projection_text)
    early[projection.relative_to(root).as_posix()] = projection_hash
    declaration_current = registry_current
    def registry_current():
        declaration_current()
        require(not projection.parent.is_symlink() and not projection.is_symlink()
                and projection.read_bytes() == projection_text.encode(), 'held ABI projection changed')
    registry_current()
    require(declaration_hash == early[declaration_path.relative_to(root).as_posix()], 'loaded declaration differs from prehold')
    require(variant == 'control' or not any('inputs' in d for d in selected.values()), 'vector graph requires production control route')
    early.update({member['source']: digest(root/member['source']) for d in selected.values() for member in edited_input_declarations(d)})
    # Derive the current request's outside closure EVERY invocation, before Make
    # and before warm lookup. The resident setup include is selection-independent.
    with tempfile.TemporaryDirectory(prefix='.outside-setup-', dir=parent) as temporary:
        _, outside_script = edited_projection(root, Path(temporary), selected)
        outside_selectors = edited_program_selectors(outside_script)
        projection_hash = hashlib.sha256(outside_script.encode()).hexdigest()
    outside_context = edited_outside_context(root, outside_selectors)
    setup_command = ['gmake', '--no-print-directory', '-j' + str(__import__('os').cpu_count() or 1),
                     'edited-runlink-inputs', 'EDITED_PROVIDER_INPUTS=' + ' '.join(outside_selectors),
                     'RUNLINK_ORT_NAMES=' + projection.relative_to(root).as_posix()]
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
        pairs = [edited_recipe(root, member['source'], member['build']) for member in edited_input_declarations(declaration)]
        if 'inputs' in declaration:
            arguments[number], recipes[number] = [pair[0] for pair in pairs], [pair[1] for pair in pairs]
        else:
            arguments[number], recipes[number] = pairs[0]
    require(all(digest(root/name) == value for name,value in early.items())
            and batch.checked_tool_identity() == tool, 'edited recipe/source changed before warm lookup')
    request = {'outside_rom': safe_relative(outside_rom or 'baseroms/mickey.us.z64').as_posix(),
               'config': 'tools/runlink/configs/msu-us.toml',
               'declaration': declaration_path.relative_to(root).as_posix(), 'declaration_sha256': declaration_hash,
               'selected_modules': sorted(selected), 'variant': variant,
               'integer_word_profiles': {str(number): declaration['integer_word_profile']
                                         for number, declaration in selected.items()},
               'compiler_flags': {str(number): ([edited_compiler_flags(a) for a in args] if args and isinstance(args[0], list)
                                              else edited_compiler_flags(args)) for number,args in arguments.items()}}
    registry_current()
    current = edited_current(root, parent, variant, request)
    if current is not None:
        require(all(digest(root/name) == value for name,value in early.items())
                and batch.checked_tool_identity() == tool, 'edited source/recipe changed during warm lookup')
        registry_current()
        print('edited source build current', variant, current['rom_sha1'], flush=True)
        return current
    directory = Path(tempfile.mkdtemp(prefix=variant + '-', dir=parent))
    commands = []
    cfg = config.load('msu-us', root=root)
    if outside_rom is not None:
        from dataclasses import replace
        cfg = replace(cfg, rom=root / safe_relative(outside_rom))
    require(len(registry.rows) >= cfg.ort_count, 'authored ABI removed a compatibility row')
    exports0 = rp.OrtNames(registry.rows[:cfg.ort_count],
        {key: value for key, value in registry.entries.items() if key[0] <= cfg.module_count})
    declared_exports = edited_source_exports(selected, exports0, cfg.module_count)
    require(declared_exports.rows == registry.rows, 'new ABI suffix lacks selected source ownership')
    require(all(number <= cfg.module_count or number in selected for number, role in registry.entries),
            'new entry lacks selected source ownership')
    selected, exports, overrides, diagnostic = pilot.edited_fixture_inputs(root, selected, declared_exports, variant)
    plan_seal = registry_surface(selected, exports)
    def plan_current():
        registry_current()
        require(registry_surface(selected, exports) == plan_seal, 'loaded source ABI build plan changed')
    plan_current()
    for number in selected:
        if number not in arguments:
            arguments[number] = arguments[diagnostic['module']]
            recipes[number] = 'approved existing diagnostic peer uses its source module compiler flags'
    write_changed(directory / 'exports.txt', exports.render())
    # Snapshot all implementation/recipe/source/declaration inputs BEFORE parsing.
    inputs = {root / n for n in ('Makefile', 'mickey.us.yaml',
        'tools/runlink_metadata_link.py', 'tools/runlink_pack.py', 'tools/check_editable_layout.py',
        'tools/link_overlay_module.py', 'tools/static_symbols.py', 'tools/rebind_elf_relocations.py',
        'tools/runlink/codec.py', 'tools/runlink/rules.py', 'tools/runlink/compare.py',
        'tools/binutils/mips64-elf-ld', 'tools/binutils/mips64-elf-objcopy', 'tools/n64crc')}
    inputs.add(declaration_path)
    inputs.update(root / member['source'] for declaration in selected.values() for member in edited_input_declarations(declaration))
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
    provider['prefix_capture'] = pp.bind_edited_prefix(root, provider, directory,
        provider_command, prefix, commands, outside_hashes)
    objects, products, closures, graph_holds = {}, {}, {}, []
    for number, declaration in selected.items():
        if 'inputs' in declaration:
            objects[number], products[number], closures[number], held = edited_compile_group(
                root, directory, 'overlay'+str(number), declaration, exports, arguments[number], commands, initial)
            graph_holds.append((number, objects[number], products[number], arguments[number], closures[number], held))
        else:
            text = overrides.get(number, (root / declaration['source']).read_text())
            objects[number], products[number], closures[number] = edited_compile(
                root, directory, 'overlay' + str(number), text, exports, arguments[number], commands,
                integer_word_profile=declaration.get('integer_word_profile'))
        for audit_name in ('static_source_audit', 'integer_word_audit'):
            if audit_name in objects[number]:
                held = objects[number][audit_name]['inputs']
                require(all(name not in initial or initial[name] == value for name,value in held.items()),
                        'conflicting held effective source input observation')
                initial.update(held)
    def graphs_current():
        for number, original, product, args, closure, held in graph_holds:
            require(objects.get(number) is original and products.get(number) is product
                    and arguments.get(number) is args and closures.get(number) is closure
                    and closure is original['source_closures'] and len(closure) == len(args),
                    'consumed vector module/closure replaced')
            held()
    graphs_current()
    for number, obj in objects.items():
        actual_registered = {symbol.name for symbol in obj['linked'].symbols
            if symbol.bind == rp.STB_GLOBAL and symbol.section in rp.EDITED_SECTIONS
            and symbol.shndx not in (rp.SHN_UNDEF, rp.SHN_ABS) and symbol.name in exports.by_name}
        require(actual_registered <= set(selected[number]['exports']),
                'selected allocated registry export lacks explicit owner declaration')
    generated = {n: rp.edited_module_records(n, objects, selected, exports, provider) for n in objects}
    graphs_current()
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
        plan_current()
        source_number, peer_number = diagnostic['module'], diagnostic['peer']
        before = generated[source_number]['module']
        exports = rp.OrtNames(exports.rows[:-diagnostic['peer_export_count']], dict(exports0.entries))
        write_changed(directory / 'exports.txt', exports.render())
        del selected[peer_number], objects[peer_number], products[peer_number], closures[peer_number], arguments[peer_number]
        objects[source_number], products[source_number], closures[source_number] = edited_compile(root, directory,
            'overlay-removed', overrides[source_number], exports, arguments[source_number], commands,
            integer_word_profile=selected[source_number].get('integer_word_profile'))
        plan_seal = registry_surface(selected, exports)
        plan_current()
        generated = {source_number: rp.edited_module_records(source_number, objects, selected, exports, provider)}
        after = generated[source_number]['module']
        require((before.image, before.table1, before.table2) == (after.image, after.table1, after.table2),
                'unreferenced tail removal changed surviving ordinary source module')
        peer_removal = 'unreferenced tail removed; declarations pruned; fresh surviving compiler/link exact'
    plan_current()
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
    graphs_current()
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
    held_final_elf, held_final_map = elf, mapping
    final_surface = pp._static_parsed_surface(elf) if graph_holds else None
    final_map_seal = tuple(tuple(vars(row).items()) for row in mapping) if graph_holds else None
    final_files = tuple(loaded.items())
    def final_graph_current():
        if graph_holds:
            require(elf is held_final_elf and mapping is held_final_map
                and pp._static_parsed_surface(elf) == final_surface
                and tuple(tuple(vars(row).items()) for row in mapping) == final_map_seal
                and hashlib.sha256(elf.data).hexdigest() == dict(final_files)[directory/'final.elf']
                and all(digest(path) == expected for path,expected in final_files),
                'consumed final graph ELF/map observation changed')
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
        edited_word_readback(module, product['joins'])
        if 'source_inputs' in obj:
            graph_providers = rp.edited_group_providers([m['raw'] for m in obj['source_inputs']], exports, selected[number]['exports'])
            require(graph_providers == obj['graph_audit']['providers'], 'consumed internal provider inventory changed')
            for name, _input, _symbol in graph_providers:
                section, offset, size, kind = rp.edited_global_target(obj, name)
                final = pilot.real_symbol(elf, name)
                outsec = f'.overlay_{number:03d}_bss' if section == '.bss' else f'.overlay_{number:03d}'
                contributions = [m for m in mapping if m.file == products[number][1].relative_to(root).as_posix()
                                 and m.outsec == outsec and m.insec == section]
                require(len(contributions) == 1 and final.section == outsec and final.type == kind
                    and final.size == size and final.value == rp.OVERLAY_VMA + obj['bases'][section] + offset
                    and contributions[0].addr + offset == final.value
                    and 0 <= offset < offset + size <= contributions[0].size,
                    'final internal provider allocation/map differs: ' + name)
        for name, witness in product['export_witnesses'].items():
            final = pilot.real_symbol(elf, name)
            require(final.value == rp.OVERLAY_VMA + witness['identity'][2] and final.size == witness['size'], 'final export differs')
    for name, witness in provider['witnesses'].items():
        actual = pilot.real_symbol(elf, name)
        require(actual.type == rp.STT_FUNC and actual.size == witness['size']
                and actual.value == witness['address'] and elf.section(actual.section)['flags'] & 6 == 6,
                'final allocated resident provider differs: ' + name)
    edited_cell_readback(elf, mapping, provider)
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
    rp.edited_check_provider_holds(provider)
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
    graphs_current()
    artifacts = snapshot
    cpp_records, receipt_arguments = {}, {}
    for n in products:
        if arguments[n] and isinstance(arguments[n][0], list):
            for i, closure in enumerate(objects[n]['source_closures']):
                args = arguments[n][i]
                key = f'{n}:{i}'
                cpp_records[key] = {'source': products[n][0].stem+f'-input{i}.c', 'closure': closure}
                receipt_arguments[key] = args
        else:
            cpp_records[str(n)] = {'source': products[n][0].with_suffix('.c').name, 'closure': closures[n]}
            receipt_arguments[str(n)] = arguments[n]
    receipt = {'schema': 'mickey-edited-source-build-v2', 'nonmatching': True, 'source_only': False,
        'general_relayout': False, 'variant': variant, 'request': request, 'inputs': initial, 'toolchain': tool,
        'projection_toolchain': project_tools, 'resident_source_context': source_context,
        'outside_selectors': outside_selectors, 'outside_input_context': outside_context,
        'input_production': {'command': setup_command, 'projection_sha256': projection_hash},
        'commands': commands, 'source_cpp': cpp_records, 'arguments': receipt_arguments, 'compiler_recipe': recipes,
        'source_graphs': {str(n): obj['graph_audit'] for n,obj in objects.items() if 'graph_audit' in obj},
        'source_static': {str(n): obj['static_source_audit'] for n, obj in objects.items()
                          if 'static_source_audit' in obj},
        'source_integer_word': {str(n): obj['integer_word_audit'] for n, obj in objects.items()
                                if 'integer_word_audit' in obj},
        'selected_raw_records': {n: len(p['refs']) for n, p in generated.items()},
        'selected_header': {n: asdict(p['module'].header) for n, p in generated.items()},
        'selected_joins': {n: p['joins'] for n, p in generated.items()},
        'outside_authority': 'retained unselected headers/payloads/records/ORT identities; resident fixed homes',
        'outside_retained_records': len(outside.main) + sum(len(m.table1) + len(m.table2) for m in outside.modules),
        'generated_metadata_inputs': len(inventory), 'retained_metadata_inputs': 0,
        'retained_fill_inputs': 0, 'selected_removal_refusals': removal, 'peer_removal': peer_removal, 'actual_resident_callers': outside_callers,
        'provider_cell_cpp': provider.get('cell_source_cpp', {}),
        'provider_cells': {name: {'address': witness.address, 'size': 4, 'input': witness.item.file}
                           for name,witness in provider.get('cells', {}).items()},
        'rom_sha1': hashlib.sha1(rom).hexdigest(), 'provider_holds': {str(p.relative_to(root)): h for p, h in provider['holds'].items()}}
    plan_current()
    cpp_receipt_seal = json_bytes((cpp_records, receipt_arguments)) if graph_holds else None
    final_graph_current()
    published_receipt_hash = pilot.publish_receipt(directory, receipt, artifacts)
    final_graph_current()
    graphs_current()
    if graph_holds:
        require(receipt['source_cpp'] is cpp_records and receipt['arguments'] is receipt_arguments
            and json_bytes((cpp_records, receipt_arguments)) == cpp_receipt_seal,
            'published vector CPP receipt members changed')
    rp.edited_check_provider_holds(provider)
    require(all(digest(root / n) == expected for n, expected in initial.items())
            and all(digest(path) == value for path, value in provider['holds'].items())
            and batch.checked_tool_identity() == tool
            and edited_projection_tools() == project_tools
            and edited_source_context(root) == source_context
            and edited_source_closures_current(products, closures, arguments)
            and edited_outside_context(root, outside_selectors) == outside_context
            and all(digest(path) == value for path, value in pilot.LOADED.items()),
            'edited inputs changed during receipt publication')
    plan_current()
    final_graph_current()
    if graph_holds:
        require(digest(directory / 'receipt.json') == published_receipt_hash,
                'published vector receipt file changed after validation')
    write_changed(parent / (variant + '-current.json'), json_bytes({'path': str(directory.relative_to(root)),
        'receipt_sha256': published_receipt_hash if graph_holds else digest(directory / 'receipt.json')}))
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
