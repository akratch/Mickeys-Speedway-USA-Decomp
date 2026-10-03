"""Opt-in ownership proofs for named overlay binary data, never target sites.

An ignored request has schema_version=1 and bindings naming symbol,
candidate_source, owner_source, element_type, element_count and bias ("0x0").
Offsets come only from canonical YAML/atlas placement. Probe binaries are
compiled and inspected, never executed; all captured inputs stay under build/.
"""

from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import tempfile

import yaml


def add_option(parser):
    parser.add_argument("--binary-storage-witnesses", type=Path,
                        help="opt-in local named-bin owner requests; compiles layout-only sizeof probes")


def read_option(args):
    path = args.binary_storage_witnesses
    return json.loads(path.read_text()) if path else None


def digest(data):
    return hashlib.sha256(data).hexdigest()


def snapshot(path):
    return {"sha256": digest(path.read_bytes()), "mtime_ns": path.stat().st_mtime_ns}


def merge_owner_identities(imported, identities, ambiguous, evidence, error):
    """Shared owner-proof merge: clear correlation ambiguity, never a conflict."""
    for name, identity in imported.items():
        independent = {tuple(row["base_identity"])
                       for row in evidence.get(name, [])
                       if row.get("independent") and row.get("base_identity") is not None}
        if any(proposed != identity for proposed in independent):
            raise error("explicit typed-storage proof conflicts with independent identity")
        identities[name] = identity
        ambiguous.discard(name)


def relative(value, prefix, error):
    if (not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9_/]+", value)
            or not value.startswith(prefix) or ".." in Path(value).parts):
        raise error("unsafe binary witness source path")
    return value


def array_declaration(text, binding, error):
    """Admit only literal fixed extern arrays of an unconditionally defined POD.

    This checks syntax, not layout size. Actual extents are read from stock
    compiler-emitted sizeof probe symbols below.
    """
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
    if re.search(r"^\s*#\s*pragma\s+pack\b", clean, re.M):
        raise error("packed binary consumer layout is unsupported")
    typ, name, count = (binding[k] for k in ("element_type", "symbol", "element_count"))
    structs = list(re.finditer(r"typedef\s+struct\s+(?:" + re.escape(typ)
                              + r"\s*)?\{([^{}]*)\}\s*" + re.escape(typ)
                              + r"\s*;", clean, re.S))
    declarations = list(re.finditer(r"\bextern\s+(?:const\s+)?" + re.escape(typ)
                                   + r"\s+" + re.escape(name) + r"\s*\[\s*"
                                   + str(count) + r"\s*\]\s*;", clean))
    if len(structs) != 1 or len(declarations) != 1:
        raise error("binary consumer needs one POD type and one fixed extern array")
    members = structs[0].group(1)
    if not members.strip() or re.sub(
            r"\b(?:s8|u8|s16|u16|s32|u32|f32)\s+[A-Za-z_][A-Za-z0-9_]*\s*;",
            "", members).strip():
        raise error("unsupported binary consumer POD member syntax")
    names = re.findall(r"\b(?:s8|u8|s16|u16|s32|u32|f32)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", members)
    if len(names) != len(set(names)):
        raise error("duplicate binary consumer POD member")
    for position in (structs[0].start(), declarations[0].start()):
        depth = 0
        for directive in re.finditer(r"^\s*#\s*(if|ifdef|ifndef|endif)\b", clean[:position], re.M):
            depth += -1 if directive.group(1) == "endif" else 1
            if depth < 0:
                raise error("malformed binary consumer conditional context")
        if depth:
            raise error("conditional binary consumer declaration is unsupported")


def layout_proof(rs, root, source, candidate, binding):
    """Capture actual preprocessed input; compile unique sizeof-only additions.

    The appended probe must preserve the candidate's executable bytes, text
    relocations and symbols. No guessed source-layout size is accepted.
    """
    import fast_score
    if any(key.startswith(("CDX_", "DKWB_")) for key in os.environ):
        raise rs.SurfaceComparisonError("binary layout probe refuses forced/instrumented compiler environment")
    dependencies = rs._callee_build_dependencies(root, source, source.read_text())
    if dependencies is None:
        raise rs.SurfaceComparisonError("binary consumer recipe/dependencies unavailable")
    paths, _ = dependencies
    paths = set(paths) | {source, Path(candidate.path), Path(__file__)}

    def context():
        args = fast_score.configured_cc_args(source.relative_to(root).as_posix())
        if Path(args[0]).resolve() != (root / "tools/ido/cc").resolve():
            raise rs.SurfaceComparisonError("binary layout probe requires the stock compiler")
        return {"recipe": args, "inputs": {str(path.resolve()): snapshot(path)
                                           for path in sorted(paths)}}

    before = context()
    candidate_time = Path(candidate.path).stat().st_mtime_ns
    if any(path.stat().st_mtime_ns > candidate_time for path in paths - {Path(candidate.path), Path(__file__)}):
        raise rs.SurfaceComparisonError("binary consumer object is stale")
    out = root / "build/binary-storage-witnesses"
    out.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix="layout-", dir=out))
    src = source.relative_to(root).as_posix()
    args = before["recipe"]
    pp = list(args)
    output_index = pp.index("-o")
    del pp[output_index:output_index + 2]
    pp.remove("-c")
    pp.insert(1, "-E")
    preprocessed = subprocess.run(pp, cwd=root, capture_output=True, timeout=30)
    if preprocessed.returncode:
        raise rs.SurfaceComparisonError("binary consumer preprocessing failed")
    prepared = directory / "consumer.i"
    prepared.write_bytes(preprocessed.stdout)
    # A unique prefix cannot shadow a consumer definition or another probe.
    prefix = "binary_witness_" + directory.name.replace("-", "_")
    if prefix.encode() in preprocessed.stdout or any(name.startswith(prefix) for name, *_ in candidate.symbols()):
        raise rs.SurfaceComparisonError("binary layout probe symbol collision")
    element, array, alignment = prefix + "_element", prefix + "_array", prefix + "_alignment"
    addition = ("\nunsigned char %s[sizeof(%s)];\nunsigned char %s[sizeof(%s)];\n"
                "struct %s_layout { unsigned char prefix; %s value; };\n"
                "unsigned char %s[sizeof(struct %s_layout) - sizeof(%s)];\n"
                % (element, binding["element_type"], array, binding["symbol"], prefix,
                   binding["element_type"], alignment, prefix, binding["element_type"])).encode()
    probe = directory / "probe.i"
    probe.write_bytes(preprocessed.stdout + addition)
    obj = directory / "probe.o"
    compile_args = fast_score.rewrite_io(args, probe, src, obj)
    compiled = subprocess.run(compile_args, cwd=root, capture_output=True, timeout=30)
    if compiled.returncode:
        raise rs.SurfaceComparisonError("binary consumer sizeof probe compile failed")
    elf = rs.Elf(obj)
    rs._explicit_storage_fidelity(elf, candidate, [".text"], set())
    sizes = []
    for name in (element, array, alignment):
        matches = [row for row in elf.symbols() if row[0] == name]
        if (len(matches) != 1 or matches[0][3] & 0xF != rs.STT_OBJECT
                or matches[0][4] in (rs.SHN_UNDEF, rs.SHN_ABS)
                or elf.names[matches[0][4]] != ".bss" or matches[0][2] <= 0):
            raise rs.SurfaceComparisonError("binary consumer sizeof probe symbol invalid")
        sizes.append(matches[0][2])
    if context() != before:
        raise rs.SurfaceComparisonError("binary consumer inputs changed during layout proof")
    return {"element_bytes": sizes[0], "array_bytes": sizes[1], "alignment_bytes": sizes[2],
            "prepared_sha256": digest(prepared.read_bytes()), "probe_sha256": digest(obj.read_bytes()),
            "compiler_context_sha256": digest(json.dumps(before, sort_keys=True).encode())}


def collect(rs, request, candidate, candidate_start, candidate_size, module,
            linked, rom, values_path, *, root=None, elf_loader=None, probe=None, evidence=None):
    """Derive independent identities; no target relocation records are inputs."""
    error = rs.SurfaceComparisonError
    root = Path(root or rs.REPO)
    loader, prove = elf_loader or rs.Elf, probe or layout_proof
    if (not isinstance(request, dict) or set(request) != {"schema_version", "bindings"}
            or type(request["schema_version"]) is not int or request["schema_version"] != 1
            or not isinstance(request["bindings"], list) or not request["bindings"]):
        raise error("invalid binary storage witness schema")
    overlay = module.get("overlay")
    if (type(overlay) is not int or module.get("identity") != "overlay:%d" % overlay
            or rs._atlas_hex(module, "synthetic_vma", "binary owner module") != rs.SYNTHETIC_VMA):
        raise error("binary storage witness needs one authenticated overlay module")
    yaml_path = root / "mickey.us.yaml"
    rom_path = root / "baseroms/mickey.us.z64"
    atlas_path = root / "config/overlays.us.json"
    atlas_bytes = atlas_path.read_bytes()
    canonical_modules = [row for row in json.loads(atlas_bytes)["modules"]
                         if row.get("overlay") == overlay]
    if len(canonical_modules) != 1 or canonical_modules[0] != module or rom_path.read_bytes() != rom:
        raise error("binary witness requires current canonical atlas and ROM")
    policy_paths = [yaml_path, Path(values_path), root / "Makefile", atlas_path]
    linked_path, candidate_path = Path(linked.path), Path(candidate.path)
    if not linked_path.is_file() or not candidate_path.is_file():
        raise error("binary storage witness requires current candidate and linked files")
    for elf, path in ((linked, linked_path), (candidate, candidate_path)):
        if getattr(elf, "data", None) != path.read_bytes():
            raise error("binary witness ELF input changed after loading")
    yaml_bytes = yaml_path.read_bytes()
    values_bytes = Path(values_path).read_bytes()
    try:
        tree = yaml.safe_load(yaml_bytes)
    except yaml.YAMLError as failure:
        raise error("binary owner YAML is malformed") from failure
    if not isinstance(tree, dict) or not isinstance(tree.get("segments"), list):
        raise error("binary owner YAML segments are missing")
    segments = [seg for seg in tree.get("segments", []) if isinstance(seg, dict)
                and seg.get("name") == "overlay_%03d" % overlay]
    if len(segments) != 1:
        raise error("binary owner YAML module missing or duplicated")
    segment = segments[0]
    rom_start = rs._atlas_hex(module["rom"], "start", "binary module ROM")
    data = module["sections"]["data_rodata"]
    data_start, data_end = (rs._atlas_hex(data, key, "binary module data") for key in ("start", "end"))
    if (segment.get("dir") != "overlays/o%03d" % overlay
            or segment.get("start") != rom_start or segment.get("vram") != rs.SYNTHETIC_VMA
            or data_end - data_start != rs._atlas_hex(data, "size", "binary module data")
            or data_start != rom_start + rs._atlas_hex(module["sections"]["text"], "size", "binary module text")):
        raise error("binary owner YAML and atlas placement disagree")
    rows = segment.get("subsegments", [])
    if (not isinstance(rows, list) or any(not isinstance(row, list) or len(row) != 3
            or type(row[0]) is not int for row in rows)
            or any(a[0] >= b[0] for a, b in zip(rows, rows[1:]))):
        raise error("binary owner YAML rows are ambiguous or unsupported")
    try:
        numeric = rs.ri.parse_numeric_assignments(values_bytes.decode())
    except (UnicodeError, rs.ri.RelocationIdentityError) as failure:
        raise error("binary witness linker assignments are malformed") from failure
    result, receipts = {}, []
    used = {candidate.symbols()[index][0] for section, offset, _, index in candidate.relocations()
            if section == ".text" and candidate_start <= offset < candidate_start + candidate_size
            and index < len(candidate.symbols())}
    for binding in request["bindings"]:
        keys = {"symbol", "candidate_source", "owner_source", "element_type", "element_count", "bias"}
        if not isinstance(binding, dict) or set(binding) != keys:
            raise error("invalid binary storage binding fields")
        name, typ, count = (binding[k] for k in ("symbol", "element_type", "element_count"))
        if (not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name)
                or name not in used or name in result or not isinstance(typ, str)
                or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", typ)
                or type(count) is not int or not 0 < count <= 65536 or binding["bias"] != "0x0"):
            raise error("binary witness requires unique used external, POD type/count and zero bias")
        prefix = "overlays/o%03d/" % overlay
        consumer = relative(binding["candidate_source"], prefix, error)
        owner = relative(binding["owner_source"], prefix, error)
        if rs._source_from_object(candidate_path) != consumer:
            raise error("binary consumer object does not own its asserted source")
        definitions = [row for row in module.get("text_ownership", []) if row.get("source") == consumer and row.get("type") == "c"]
        if len(definitions) != 1:
            raise error("binary consumer canonical ownership is missing or ambiguous")
        source = root / "src" / (consumer + ".c")
        source_bytes = source.read_bytes()
        array_declaration(source_bytes.decode(), binding, error)
        external = [row for row in candidate.symbols() if row[0] == name]
        if len(external) != 1 or external[0][4] != rs.SHN_UNDEF or external[0][1] != 0:
            raise error("binary candidate carrier is not one undefined external")
        if numeric.get(name) != 0:
            raise error("binary carrier linker assignment must preserve zero stored bias")
        assigned = [row for row in linked.symbols() if row[0] == name]
        if len(assigned) != 1 or assigned[0][4] != rs.SHN_ABS or assigned[0][1] != 0:
            raise error("binary carrier linked assignment is missing or conflicts")
        owners = [i for i, row in enumerate(rows) if row[1] == "bin" and segment["dir"] + "/" + str(row[2]) == owner]
        if len(owners) != 1 or owners[0] + 1 >= len(rows):
            raise error("binary owner YAML range missing or duplicated")
        i = owners[0]
        start, end = rows[i][0], rows[i + 1][0]
        if not data_start <= start < end <= data_end or end > len(rom):
            raise error("binary owner escapes initialized module data")
        asset = root / "assets" / (owner + ".bin")
        wrapper = root / "build/assets" / (owner + ".bin.o")
        paths = policy_paths + [source, asset, wrapper, candidate_path, linked_path, rom_path]
        before = {str(path): snapshot(path) for path in paths}
        anchors = {atlas_path: atlas_bytes, yaml_path: yaml_bytes, Path(values_path): values_bytes,
                   source: source_bytes, rom_path: rom, candidate_path: candidate.data,
                   linked_path: linked.data}
        if any(before[str(path)]["sha256"] != digest(raw) for path, raw in anchors.items()):
            raise error("binary storage inputs changed before proof")
        if (wrapper.stat().st_mtime_ns < max(asset.stat().st_mtime_ns, *(path.stat().st_mtime_ns for path in policy_paths))
                or linked_path.stat().st_mtime_ns < wrapper.stat().st_mtime_ns
                or candidate_path.stat().st_mtime_ns < source.stat().st_mtime_ns):
            raise error("binary owner or consumer inputs are stale")
        obj = loader(wrapper)
        if digest(obj.data) != before[str(wrapper)]["sha256"]:
            raise error("binary wrapper changed while loading")
        index, header = obj.section(".data")
        size = end - start
        if (index is None or not isinstance(header, tuple) or header[1] != 1 or header[5] != size
                or obj.relocations(r".*") or obj.section_bytes(".data") != rom[start:end]
                or asset.read_bytes() != rom[start:end]):
            raise error("binary wrapper extent, bytes or relocation ownership differs")
        base = "_binary_" + re.sub(r"[^A-Za-z0-9_]", "_", asset.relative_to(root).as_posix())
        for suffix, value in (("_start", 0), ("_end", size)):
            symbols = [row for row in obj.symbols() if row[0] == base + suffix]
            placed = [row for row in linked.symbols() if row[0] == base + suffix]
            if (len(symbols) != 1 or symbols[0][1] != value or symbols[0][4] != index
                    or symbols[0][3] & 0xF != rs.STT_NOTYPE or len(placed) != 1
                    or placed[0][4] in (rs.SHN_UNDEF, rs.SHN_ABS)
                    or linked.names[placed[0][4]] != ".overlay_%03d" % overlay
                    or placed[0][1] != rs.SYNTHETIC_VMA + start - rom_start + value):
                raise error("binary owner endpoints are missing, duplicated or misplaced")
        linked_bytes = linked.section_bytes(".overlay_%03d" % overlay)
        if linked_bytes[start - rom_start:end - rom_start] != rom[start:end]:
            raise error("linked binary owner bytes differ from its wrapper and ROM")
        proof = prove(rs, root, source, candidate, binding)
        if proof["element_bytes"] * count != size or proof["array_bytes"] != size:
            raise error("compiler-proved binary consumer extent differs from owner")
        alignment = proof["alignment_bytes"]
        if (type(alignment) is not int or alignment <= 0 or alignment & (alignment - 1)
                or proof["element_bytes"] % alignment or (start - rom_start) % alignment):
            raise error("compiler-proved binary consumer alignment differs from owner")
        if before != {str(path): snapshot(path) for path in paths}:
            raise error("binary storage inputs changed during proof")
        identity = (overlay, start - rom_start)
        result[name] = identity
        receipts.append({"symbol": name, "owner_source": owner, "identity": list(identity),
                         "owner_bytes": size, "owner_sha256": digest(rom[start:end]),
                         "consumer_layout": proof, "matching_credit": 0})
        rs._identity_witness(evidence, name, "independent-binary-owner", identity,
                             independent=True, owner_source=owner,
                             owner_sha256=receipts[-1]["owner_sha256"], consumer_layout=proof)
    return result, receipts
