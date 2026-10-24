#!/usr/bin/env python3
"""Consume exact runlink metadata in the fixed-layout canonical GNU ld graph.

The ordinary Splat script remains the retained-input bootstrap. Only its named
metadata selectors change in the final script; code/data inputs stay intact.
"""
from __future__ import annotations

import argparse
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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("prepare", "prove"))
    parser.add_argument("--root", type=Path, default=REPO)
    args = parser.parse_args()
    try:
        (prepare if args.command == "prepare" else prove)(args.root.resolve())
    except (MetadataLinkError, rp.PackError, OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
