"""Fresh compiler/linker reproduction for selected section-relative modules."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess
import time

import overlay_tables as ot
import proof_provenance as pp
import reloc_surface as rs
import runlink_pack as rp

_LOADED = pp.sha256_file(Path(__file__))


def selected(target, make_dump):
    return re.search(r"^" + re.escape(target) + r": MODULE_LINK = .*link_overlay_module\.py", make_dump, re.M) is not None


def allocated(elf):
    return {s["name"]: (s["type"], s["flags"], s["addr"], s["size"], s["align"],
                        elf.bytes_of(s["name"]))
            for s in elf.sections if s["flags"] & 2}


def symbols(elf):
    return sorted((s.name, s.value, s.size, s.bind, s.type, s.section)
                  for s in elf.symbols if s.type != rp.STT_FILE)


def source_identities(raw, linked, canonical, exports, overlay):
    """Resolve source operands without taking an identity from the shipped site."""
    bases = {".text": 0, ".data": linked.size(".text"),
             ".rodata": linked.size(".text") + linked.size(".data"),
             ".bss": sum(linked.size(s) for s in (".text", ".data", ".rodata"))}
    by_name = {}
    for symbol in canonical.symbols:
        if symbol.bind == rp.STB_GLOBAL and symbol.shndx != rp.SHN_UNDEF:
            by_name.setdefault(symbol.name, []).append(symbol)
    layout = rp.ResidentLayout.from_elf(by_name)
    result = {}
    linked_local = {s.name: s for s in linked.symbols if s.section in bases}
    for section in (".text", ".rodata", ".data"):
        rels = raw.rels(section)
        for rel, addend in zip(rels, rp.in_place_addends(raw, section, rels)):
            symbol = rel.sym
            if symbol.shndx == rp.SHN_UNDEF:
                if symbol.name not in exports.by_name:
                    raise pp.MetadataProofError("module reference has no declared runtime export")
                choices = by_name.get(symbol.name, [])
                if len(choices) != 1:
                    raise pp.MetadataProofError(f"runtime export has no unique canonical definition: {symbol.name}")
                target = choices[0]
                if target.section in rp.RESIDENT_SECTIONS or target.section == "ABS":
                    if not layout.contains(target.value):
                        raise pp.MetadataProofError("export address escapes resident storage")
                    kind, offset = layout.selector_of(target.value)
                    identity = (kind, offset + addend)
                else:
                    match = re.fullmatch(r"\.overlay_(\d{3})", target.section)
                    if not match:
                        raise pp.MetadataProofError("unsupported exported section")
                    identity = (int(match.group(1)), target.value - rp.OVERLAY_VMA + addend)
            elif symbol.shndx == rp.SHN_COMMON:
                target = linked_local.get(symbol.name)
                if target is None or target.section != ".bss":
                    raise pp.MetadataProofError("COMMON storage was not allocated by the module link")
                identity = (overlay, bases[".bss"] + target.value + addend)
            elif symbol.section in bases:
                identity = (overlay, bases[symbol.section] + symbol.value + addend)
            else:
                raise pp.MetadataProofError("unsupported source-module relocation target")
            key = (bases[section] + rel.offset, rel.type)
            if key in result:
                raise pp.MetadataProofError("duplicate source-module relocation site")
            result[key] = identity
    return result


def prove(root, resolution, context, records, target_elf, linked_name, input_snapshot):
    if resolution.resolution_mode != "post_promotion" or context["kind"] != "overlay":
        raise pp.MetadataProofError("source module link requires canonical overlay ownership")
    if pp.sha256_file(Path(__file__)) != _LOADED:
        raise pp.MetadataProofError("loaded module-link proof changed")
    configured = resolution.candidate_object
    raw_path, _plan, receipt, current_context = pp.capture_configured_raw(
        root, resolution.source, configured, root / "build/mickey.us.elf", "", module_link=True)
    if (receipt["linked_sha256"] != input_snapshot["linked"]
            or receipt["inputs"].get("generated_metadata", {}) != input_snapshot.get("generated_metadata", {})
            or receipt["inputs"]["target_inputs"] != input_snapshot["target_inputs"]
            or hashlib.sha256(target_elf.data).hexdigest() != input_snapshot["linked"]):
        raise pp.MetadataProofError("canonical module inputs changed before reproduction")
    words = shlex.split(receipt["inputs"]["module_link"])
    target = configured.relative_to(root).as_posix()
    if words.count(target) != 1 or "--output" in words:
        raise pp.MetadataProofError("ambiguous source-module output recipe")
    expected = {"--ort": "config/runlink-ort.us.txt", "--ld": "tools/binutils/mips64-elf-ld",
                "--objcopy": "tools/binutils/mips64-elf-objcopy"}
    for flag, value in expected.items():
        if words.count(flag) != 1 or words[words.index(flag) + 1] != value:
            raise pp.MetadataProofError("source-module recipe uses unbound tools or exports")
    if words.count("--directory") != 1:
        raise pp.MetadataProofError("ambiguous source-module artifact directory")
    directory = root / receipt["directory"] / "module"
    words[words.index("--directory") + 1] = str(directory)
    words[words.index(target)] = str(raw_path)
    replayed = raw_path.with_name("module-replayed.o")
    words += ["--output", str(replayed)]
    result = subprocess.run(words, cwd=root, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=120)
    if result.returncode:
        raise pp.MetadataProofError("ordinary module link failed: " + result.stdout.strip())
    link_receipt = json.loads((directory / "receipt.json").read_text())
    if (link_receipt.get("schema") != "mickey-source-module-link-v1"
            or link_receipt.get("compiler_sha256") != pp.sha256_file(raw_path)
            or link_receipt.get("configured_sha256") != pp.sha256_file(replayed)
            or link_receipt.get("container_conversion") != "zero-based-exec-to-rel-header-only"):
        raise pp.MetadataProofError("fresh module-link receipt does not bind its products")
    checks = {"module.elf": "linked_sha256", "bindings.o": "prepared_sha256",
              "module.ld": "script_sha256", "localized.elf": "localized_sha256"}
    for filename, field in checks.items():
        if link_receipt.get(field) != pp.sha256_file(directory / filename):
            raise pp.MetadataProofError("ordinary linker product changed")
    commands = link_receipt.get("commands", [])
    if not any(Path(argv[0]).name == "mips64-elf-ld" and "--no-check-sections" in argv
               and str(directory / "module.elf") in argv for argv in commands if argv):
        raise pp.MetadataProofError("missing ordinary linker invocation")
    actual, reproduced = rp.Elf(configured), rp.Elf(replayed)
    if (allocated(actual) != allocated(reproduced) or symbols(actual) != symbols(reproduced)
            or any(actual.rels(s) for s in (".text", ".data", ".rodata"))):
        raise pp.MetadataProofError("fresh source-module link does not reproduce configured bytes/symbols")
    raw = rp.Elf(raw_path)
    exports = rp.OrtNames.load(root / "config/runlink-ort.us.txt")
    canonical = rp.Elf(root / "build/mickey.us.elf")
    overlay = context["overlay"]
    raw_geometry = rs._function_text_symbol(rs.Elf(raw_path), [resolution.candidate_symbol])
    if context["offset"] != raw_geometry[0]:
        raise pp.MetadataProofError("source-module link is not one whole translation unit")
    body = b"".join(reproduced.bytes_of(s) for s in (".text", ".data", ".rodata"))
    rom = (root / "baseroms/mickey.us.z64").read_bytes()
    module = ot.build_modules(ot.read_headers(rom))[overlay - 1]
    if (canonical.bytes_of(f".overlay_{overlay:03d}") != body
            or rom[module["rom_start"]:module["rom_start"] + len(body)] != body
            or len(body) != module["text_size"] + module["data_size"]
            or reproduced.size(".bss") != module["bss_size"]):
        raise pp.MetadataProofError("source module does not own the exact linked/retail body and BSS extent")
    identities = source_identities(raw, reproduced, canonical, exports, overlay)
    all_records = rs._target_runtime_records(rom, {"kind": "overlay", "overlay": overlay, "module": module}, 0, len(body))
    expected_identities = {(record.offset, record.rtype): record.identity for record in all_records}
    if identities != expected_identities or len(all_records) != len(identities):
        raise pp.MetadataProofError("source module relocation identities disagree with runtime records")
    comparison = rs.function_surface_comparison(
        resolution.requested_symbol, raw_path, root / "build/mickey.us.elf",
        rom_path=root / "baseroms/mickey.us.z64", atlas_path=root / "config/overlays.us.json",
        values_path=root / "overlay_undefined_syms.us.txt", candidate_symbol=resolution.candidate_symbol,
        target_symbol=linked_name, source=resolution.translation_unit)
    start, size = raw_geometry
    owned = {key: value for key, value in identities.items() if start <= key[0] < start + size}
    if len(owned) != len(records) or comparison["offset_type_exact"] is not True:
        raise pp.MetadataProofError("source module function relocation shape is not exact")
    comparison.update(rs.compare_record_sets(records, [
        rs.SurfaceRecord(offset - start, kind, identity)
        for (offset, kind), identity in owned.items()]))
    comparison["candidate_surface_source"] = "fresh-source-module-link"
    receipt.update(source_selection=pp.classify_source_selection(
        resolution.source.read_text(), candidate_symbol=resolution.candidate_symbol,
        target_symbol=resolution.target_symbol,
        defines={token[2:].split("=", 1)[0] for token in shlex.split(receipt["inputs"]["command"])
                 if token.startswith("-D") and len(token) > 2})[0],
        raw_count=len(records), retained_count=0, replayed_sha256=pp.sha256_file(replayed),
        link=link_receipt, allocated_sections_exact=True, symbols_exact=True,
        module_body_exact=True, module_relocations=len(identities),
        identities=[{"offset": offset - start, "rtype": kind, "identity": identity}
                    for (offset, kind), identity in sorted(owned.items())])
    if receipt["source_selection"] != pp.ORDINARY_C:
        raise pp.MetadataProofError("module proof does not select ordinary C")
    if receipt["inputs"] != current_context() or receipt["raw_sha256"] != pp.sha256_file(raw_path):
        raise pp.MetadataProofError("module-link proof inputs changed")
    (root / receipt["directory"] / "proof.json").write_text(json.dumps(receipt, sort_keys=True, indent=2))
    return comparison, receipt, current_context
