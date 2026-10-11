"""Prove direct overlay-local initialized C storage without a proxy alias.

This route consumes no target relocation sites. It is deliberately restricted
to separate data-only translation units whose complete storage reproduces the
canonical linked section and retail range with the configured stock compiler.
"""

import hashlib
import json
import re
import tempfile
from pathlib import Path
from types import SimpleNamespace

LOADED_SHA256 = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()

def yaml_config(text, error):
    import yaml

    class UniqueLoader(yaml.SafeLoader):
        pass

    def mapping(loader, node, deep=False):
        result = {}
        for key_node, value_node in node.value:
            key = loader.construct_object(key_node, deep=deep)
            if key in result:
                raise error("duplicate YAML ownership key")
            result[key] = loader.construct_object(value_node, deep=deep)
        return result

    UniqueLoader.add_constructor(yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG, mapping)
    return yaml.load(text, Loader=UniqueLoader)


def collect(rs, module, candidate, name, linked, rom, *, root=None):
    root = Path(root or rs.REPO)
    error = rs.SurfaceComparisonError

    def require(ok, why):
        if not ok:
            raise error("direct canonical data owner: " + why)

    overlay = module["overlay"]
    section = ".overlay_%03d" % overlay
    definitions = [row for row in linked.symbols()
                   if row[0] == name and row[4] not in (rs.SHN_UNDEF, rs.SHN_ABS)]
    if not definitions:
        return None
    # A same-name definition in a different module is never borrowed.
    selected = [row for row in definitions if row[4] < len(linked.names)
                and linked.names[row[4]] == section]
    if not selected:
        return None
    require(len(definitions) == len(selected) == 1, "ambiguous named definition")
    definition = selected[0]
    require(definition[2] > 0 and definition[3] & 15 == rs.STT_OBJECT,
            "definition is not a sized object")
    require(not any(row[0] == name and row[4] == rs.SHN_ABS
                    for row in linked.symbols()), "unexpected absolute alias")
    carriers = [row for row in candidate.symbols() if row[0] == name]
    require(len(carriers) == 1 and carriers[0][4] == rs.SHN_UNDEF
            and carriers[0][3] & 15 in (rs.STT_NOTYPE, rs.STT_OBJECT)
            and carriers[0][2] in (0, definition[2]), "candidate shadows the external owner")
    choices = []
    for row in module.get("data_rodata_ownership") or []:
        if row.get("type") != "c" or row.get("section") != ".data":
            continue
        source = Path(row.get("source", ""))
        require(not source.is_absolute() and ".." not in source.parts,
                "unsafe owner source")
        path = root / "build/src" / (str(source) + ".c.o")
        if not path.is_file():
            continue
        obj = rs.Elf(path)
        matches = [symbol for symbol in obj.symbols()
                   if symbol[0] == name and symbol[4] not in (rs.SHN_UNDEF, rs.SHN_ABS)]
        if matches:
            require(len(matches) == 1, "duplicate input symbol")
            choices.append((row, source, obj, matches[0]))
    require(len(choices) == 1, "missing or ambiguous canonical input owner")
    row, source, obj, symbol = choices[0]
    require(symbol[4] < len(obj.names) and obj.names[symbol[4]] == ".data"
            and symbol[3] & 15 == rs.STT_OBJECT and symbol[2] == definition[2],
            "input object type or extent differs")
    start, end, size = [rs._atlas_hex(row, key, "direct data owner")
                        for key in ("offset", "end_offset", "size")]
    text_size = rs._atlas_hex(module["sections"]["text"], "size", "overlay text")
    data_size = rs._atlas_hex(module["sections"]["data_rodata"], "size", "overlay data")
    require(0 <= start < end <= data_size and end - start == size,
            "invalid atlas extent")
    require(0 <= symbol[1] and symbol[1] + symbol[2] <= size,
            "symbol escapes owned input")
    base = rs.SYNTHETIC_VMA + text_size + start
    require(definition[1] == base + symbol[1], "linked definition contradicts atlas placement")
    _, header = obj.section(".data")
    require(header[1] == 1 and header[5] == size, "input is not complete initialized storage")
    for other in (".text", ".rodata", ".bss"):
        require(other not in obj.names or obj.section(other)[1][5] == 0,
                "only a data-only translation unit is supported")
    require(not list(obj.relocations(r".*")), "data owner has relocations")
    object_rel = Path(obj.path).relative_to(root).as_posix()
    placement = rs.linked_input_sections(root / "build/mickey.us.map").get((object_rel, ".data"))
    require(placement == (base, size), "linked map disagrees with input ownership")
    # Reject overlapping object definitions, but permit padding between them.
    intervals = sorted((value, value + extent, other) for other, value, extent, info, index
                       in obj.symbols() if index == symbol[4] and info & 15 == rs.STT_OBJECT)
    require(all(a[1] <= b[0] for a, b in zip(intervals, intervals[1:])),
            "overlapping storage definitions")
    input_bytes = obj.section_bytes(".data")
    linked_header = linked.section(section)[1]
    linked_start = base - linked_header[3]
    rom_start = rs._atlas_hex(module["rom"], "start", "overlay ROM") + text_size + start
    require(len(input_bytes) == size and linked_start >= 0
            and input_bytes == linked.section_bytes(section)[linked_start:linked_start + size]
            == rom[rom_start:rom_start + size], "input, linked and retail storage differ")

    import storage_view as sv
    require(root.resolve() == sv.ROOT.resolve(), "proof root differs from loaded tools")
    source_path = root / "src" / (str(source) + ".c")
    source_bytes = source_path.read_bytes()
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_bytes.decode(), flags=re.S)
    declaration = r"\b(unsigned\s+char|signed\s+int)\s+([A-Za-z_]\w*)\s*=\s*0\s*;"
    declarations = re.findall(declaration, clean)
    require(declarations and not re.sub(declaration, "", clean).strip(),
            "only ordinary zero-initialized char/int scalar C definitions are supported")
    typed = [(typ, variable) for typ, variable in declarations if variable == name]
    require(len(typed) == 1 and definition[2] == (1 if typed[0][0].split() == ["unsigned", "char"] else 4),
            "C declaration width differs from the actual storage")
    require(rom == (root / "baseroms/mickey.us.z64").read_bytes(),
            "supplied ROM is not canonical")
    paths = [source_path, Path(obj.path), Path(candidate.path), Path(linked.path),
             root / "mickey.us.yaml", root / "config/overlays.us.json", Path(__file__),
             root / "baseroms/mickey.us.z64"]

    def snapshot():
        return {str(path): hashlib.sha256(path.read_bytes()).hexdigest() for path in paths}

    before = snapshot()
    require(before[str(Path(__file__))] == LOADED_SHA256, "loaded owner proof changed")
    require(before[str(source_path)] == hashlib.sha256(source_bytes).hexdigest()
            and before[str(root / "baseroms/mickey.us.z64")] == hashlib.sha256(rom).hexdigest(),
            "source or ROM changed before capture")
    canonical = json.loads((root / "config/overlays.us.json").read_text())
    require([row for row in canonical["modules"] if row.get("overlay") == overlay] == [module],
            "atlas module is not current")
    segments = [seg for seg in yaml_config((root / "mickey.us.yaml").read_text(), error)["segments"]
                if isinstance(seg, dict) and seg.get("name") == "overlay_%03d" % overlay]
    require(len(segments) == 1 and segments[0].get("dir") == source.parent.as_posix(),
            "YAML overlay owner differs")
    rows = segments[0].get("subsegments", [])
    expected_row = [rom_start, ".data", source.name]
    require(rows.count(expected_row) == 1, "YAML does not own the named data source")
    index = rows.index(expected_row)
    require(index + 1 < len(rows) and isinstance(rows[index + 1], list)
            and rows[index + 1][0] == rom_start + size, "YAML data boundary differs")
    parent = root / "build/direct-data-owner"
    parent.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix="proof-", dir=parent))
    raw, receipt, current, _ = sv.capture(
        SimpleNamespace(source=source_path, candidate_object=Path(obj.path)), directory)
    # No metadata transformation is needed or accepted for this narrow route.
    require(not receipt["metadata_plan"], "data owner unexpectedly requires metadata edits")
    rs._reserved_witness_fidelity(raw, obj)

    def recheck():
        require(receipt["inputs"] == current() and snapshot() == before,
                "owner authority changed during proof")
        require(hashlib.sha256(Path(raw.path).read_bytes()).hexdigest() == receipt["raw_sha256"],
                "captured stock object changed")
        require(getattr(candidate, "data", None) == Path(candidate.path).read_bytes()
                and getattr(obj, "data", None) == Path(obj.path).read_bytes()
                and getattr(linked, "data", None) == Path(linked.path).read_bytes(),
                "loaded ELF differs from captured file")

    recheck()
    return {"identity": (overlay, text_size + start + symbol[1]),
            "owner": str(source), "size": symbol[2],
            "capture_sha256": hashlib.sha256(json.dumps(receipt, sort_keys=True).encode()).hexdigest(),
            "recheck": recheck}
