"""Small fail-closed helpers for reviewed N64 typed storage declarations."""

import hashlib
from pathlib import Path
import re


_LOADED_IMPLEMENTATION_SHA256 = hashlib.sha256(
    Path(__file__).read_bytes()).hexdigest()


def require_loaded_implementation(error_type):
    """Reject an extracted helper changed after this module was imported."""
    if hashlib.sha256(Path(__file__).read_bytes()).hexdigest() != _LOADED_IMPLEMENTATION_SHA256:
        raise error_type("loaded typed-storage implementation changed on disk")


def validate_initialized_array_owner(source_text, symbol, owner_type,
                                     element_count, error_type):
    """Validate one reviewed fixed-size initialized array's natural C type."""
    def fail(message):
        raise error_type(message)

    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_text, flags=re.S)
    if owner_type == "s32":
        element_width, element_alignment = SCALAR_LAYOUT[owner_type]
    elif owner_type == "Overlay8EffectColor":
        tag = r"(?:\s+Overlay8EffectColor)?"
        match = re.search(
            r"typedef\s+struct" + tag + r"\s*\{(?P<body>.*?)\}\s*"
            r"Overlay8EffectColor\s*;", clean, re.S)
        if not match:
            fail("typed color array struct declaration is missing or ambiguous")
        member_re = re.compile(r"\s*u8\s+([A-Za-z_][A-Za-z0-9_]*)\s*;")
        members, consumed, cursor = [], 0, 0
        for member in member_re.finditer(match.group("body")):
            if match.group("body")[consumed:member.start()].strip():
                fail("unsupported typed color array struct member syntax")
            members.append((member.group(1), cursor))
            cursor += 1
            consumed = member.end()
        if (match.group("body")[consumed:].strip()
                or members != [("red", 0), ("green", 1),
                               ("blue", 2), ("alpha", 3)]):
            fail("typed color array struct does not have the reviewed RGBA byte layout")
        element_width, element_alignment = 4, 1
    else:
        fail("unsupported typed initialized array element type")

    declaration = re.compile(
        r"\b" + re.escape(owner_type) + r"\s+" + re.escape(symbol)
        + r"\s*\[\s*" + str(element_count) + r"\s*\]\s*=\s*\{", re.S)
    matches = list(declaration.finditer(clean))
    if len(matches) != 1:
        fail("typed initialized array definition is missing or ambiguous")
    return {"element_type": owner_type, "element_count": element_count,
            "element_width": element_width,
            "element_alignment": element_alignment,
            "extent": element_width * element_count}


def _storage_hex(value, description, error_type):
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9A-Fa-f]+", value):
        raise error_type("invalid %s %r" % (description, value))
    return int(value, 16)


def parse_explicit_storage_registry(registry, error_type, header_count):
    """Parse the fixed reviewed BSS, scalar, and table-owner registry."""
    if (not isinstance(registry, dict)
            or set(registry) != {"schema_version", "groups"}
            or type(registry.get("schema_version")) is not int
            or registry.get("schema_version") != 1
            or not isinstance(registry.get("groups"), list)):
        raise error_type("invalid explicit overlay-storage registry schema")
    groups, ids, aliases = [], set(), set()
    common_keys = {"id", "kind", "overlay", "candidate_sources", "owner_source",
                   "owner_source_sha256", "owner_section", "section_size",
                   "original_namespace", "original_base", "bindings"}
    scalar_owners = None
    array_layout = {
        "D_3E0": ("o8EffectBit4LeftTriggerMasks", 0x000, "s32", 4),
        "D_420": ("o8EffectBit8RightTriggerMasks", 0x040, "s32", 4),
        "D_460": ("o8EffectBit1LeftTriggerMasks", 0x080, "s32", 4),
        "D_4A0": ("o8EffectBit2RightTriggerMasks", 0x0C0, "s32", 4),
        "D_4E0": ("o8EffectColorBySelector", 0x100, "Overlay8EffectColor", 1),
    }
    table_hash = "679c100dc39f0021d7c14fd4ffe49ade502d4ee23c60022cc4eb5cc4a97a51fb"
    table_owners = [row[0] for row in array_layout.values()]
    for raw_group in registry["groups"]:
        if not isinstance(raw_group, dict):
            raise error_type("invalid explicit overlay-storage group schema")
        group = dict(raw_group)
        kind = group.get("kind")
        if kind == "overlay-local-bss-fields":
            group_keys = common_keys | {"owner_symbol", "owner_size", "struct_type"}
            binding_keys = {"candidate", "addend", "member", "type", "width", "member_offset"}
        elif kind in ("overlay-local-data-scalars", "overlay-local-data-arrays"):
            group_keys = common_keys | {"owner_symbols"}
            binding_keys = {"candidate", "addend", "owner_symbol", "owner_offset", "type", "width"}
        else:
            raise error_type("unsupported explicit overlay-storage group kind")
        if set(group) != group_keys:
            raise error_type("invalid explicit overlay-storage group schema")
        if (not isinstance(group["id"], str) or not group["id"] or group["id"] in ids
                or type(group["overlay"]) is not int
                or not 1 <= group["overlay"] <= header_count
                or group["original_namespace"] != "overlay-local"):
            raise error_type("invalid explicit overlay-storage group identity")
        ids.add(group["id"])
        if (not isinstance(group["candidate_sources"], list)
                or not group["candidate_sources"]
                or (kind == "overlay-local-bss-fields"
                    and group["owner_source"] not in group["candidate_sources"])):
            raise error_type("invalid explicit storage source allowlist")
        owner_relative = Path(group["owner_source"]) if isinstance(group["owner_source"], str) else None
        if (owner_relative is None or owner_relative.is_absolute()
                or ".." in owner_relative.parts or not group["owner_source"]):
            raise error_type("invalid explicit owner source path")
        for source in group["candidate_sources"]:
            relative = Path(source) if isinstance(source, str) else None
            if relative is None or relative.is_absolute() or ".." in relative.parts or not source:
                raise error_type("invalid explicit candidate source path")
        if (not isinstance(group["owner_source_sha256"], str)
                or not re.fullmatch(r"[0-9a-f]{64}", group["owner_source_sha256"])):
            raise error_type("invalid explicit storage source pin")
        group["_section_size"] = _storage_hex(group["section_size"], "storage section size", error_type)
        group["_original_base"] = _storage_hex(group["original_base"], "original storage base", error_type)
        if kind == "overlay-local-bss-fields":
            if (not isinstance(group["owner_symbol"], str)
                    or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", group["owner_symbol"])
                    or group["owner_section"] != ".bss"
                    or not isinstance(group["struct_type"], str)
                    or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", group["struct_type"])):
                raise error_type("invalid explicit typed BSS owner")
            group["_owner_size"] = _storage_hex(group["owner_size"], "typed owner size", error_type)
            if group["_owner_size"] <= 0 or group["_section_size"] < group["_owner_size"]:
                raise error_type("typed owner extent exceeds its original BSS section")
        else:
            owners = group["owner_symbols"]
            if (group["owner_section"] != ".data" or not isinstance(owners, list)
                    or not owners or any(not isinstance(name, str) or not re.fullmatch(
                        r"[A-Za-z_][A-Za-z0-9_]*", name) for name in owners)
                    or len(set(owners)) != len(owners)):
                raise error_type("invalid explicit typed data owner list")
            group["_owner_symbols"] = set(owners)
            if kind == "overlay-local-data-scalars":
                if (group["overlay"] != 8 or len(owners) != 40
                        or group["_section_size"] != 0xA0
                        or group["_original_base"] != 0x73B0):
                    raise error_type("unsupported reviewed Overlay 8 scalar storage extent")
                group["_candidate_bias"] = 0xF8
            else:
                if (group["id"] != "o008-original-local-effect-tables-v1"
                        or group["overlay"] != 8
                        or group["candidate_sources"] != ["overlays/o008/overlay_008"]
                        or group["owner_source"] != "overlays/o008/overlay8EffectTables"
                        or group["owner_source_sha256"] != table_hash
                        or owners != table_owners
                        or group["_section_size"] != 0x140
                        or group["_original_base"] != 0x5130):
                    raise error_type("unsupported reviewed Overlay 8 initialized table owner")
                group["_owner_layout"] = {name: {"offset": row[1], "size": 0x40}
                                          for name, row in zip(owners, array_layout.values())}
                group["_candidate_bias"] = 0x3E0
        if not isinstance(group["bindings"], list) or not group["bindings"]:
            raise error_type("explicit storage group has no bindings")
        copied_bindings = []
        for raw_binding in group["bindings"]:
            if not isinstance(raw_binding, dict) or set(raw_binding) != binding_keys:
                raise error_type("invalid explicit storage binding schema")
            binding = dict(raw_binding)
            name = binding["candidate"]
            if (not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name)
                    or (group["overlay"], name) in aliases):
                raise error_type("duplicate or invalid explicit storage candidate name")
            aliases.add((group["overlay"], name))
            binding["_addend"] = _storage_hex(binding["addend"], "candidate storage addend", error_type)
            if kind == "overlay-local-bss-fields":
                for field in ("member", "type"):
                    if not isinstance(binding[field], str) or not re.fullmatch(
                            r"[A-Za-z_][A-Za-z0-9_]*", binding[field]):
                        raise error_type("invalid explicit BSS member identity")
                binding["_width"] = _storage_hex(binding["width"], "typed BSS member width", error_type)
                binding["_member_offset"] = _storage_hex(binding["member_offset"], "typed BSS member offset", error_type)
            else:
                if (not isinstance(binding["owner_symbol"], str)
                        or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", binding["owner_symbol"])
                        or binding["owner_symbol"] not in group["_owner_symbols"]
                        or not isinstance(binding["type"], str)):
                    raise error_type("invalid explicit typed data owner binding")
                binding["_width"] = _storage_hex(binding["width"], "typed data view width", error_type)
                binding["_owner_offset"] = _storage_hex(binding["owner_offset"], "typed data owner offset", error_type)
                if kind == "overlay-local-data-scalars":
                    if (binding["type"] != "f32" or binding["_width"] != 4
                            or binding["_owner_offset"] % 4
                            or binding["_owner_offset"] >= group["_section_size"]
                            or binding["_addend"] != group["_candidate_bias"] + binding["_owner_offset"]):
                        raise error_type("unsupported typed scalar layout")
                else:
                    expected = array_layout.get(name)
                    if (expected is None or binding["owner_symbol"] != expected[0]
                            or binding["_owner_offset"] != expected[1]
                            or binding["type"] != ("u8" if name == "D_4E0" else "s32")
                            or binding["_width"] != expected[3]
                            or binding["_addend"] != 0x3E0 + expected[1]):
                        raise error_type("unsupported reviewed table carrier/type/offset mapping")
            copied_bindings.append(binding)
        if kind == "overlay-local-data-scalars":
            if ({b["owner_symbol"] for b in copied_bindings} != group["_owner_symbols"]
                    or {b["_owner_offset"] for b in copied_bindings}
                    != set(range(0, group["_section_size"], 4))):
                raise error_type("explicit scalar bindings do not cover the exact owner extent")
        elif kind == "overlay-local-data-arrays":
            if ({b["owner_symbol"] for b in copied_bindings} != group["_owner_symbols"]
                    or {b["candidate"] for b in copied_bindings} != set(array_layout)
                    or len(copied_bindings) != 5):
                raise error_type("explicit table bindings do not cover the exact owner extent")
        group["bindings"] = copied_bindings
        groups.append(group)
    return groups


def validate_explicit_array_owner(group, binding, source_text, error_type):
    layout = {
        "o8EffectBit4LeftTriggerMasks": "s32",
        "o8EffectBit8RightTriggerMasks": "s32",
        "o8EffectBit1LeftTriggerMasks": "s32",
        "o8EffectBit2RightTriggerMasks": "s32",
        "o8EffectColorBySelector": "Overlay8EffectColor",
    }
    expected_type = layout.get(binding.get("owner_symbol"))
    if (group.get("kind") != "overlay-local-data-arrays"
            or expected_type is None):
        raise error_type("unsupported explicit initialized array binding")
    parsed = validate_initialized_array_owner(
        source_text, binding["owner_symbol"], expected_type, 16, error_type)
    if parsed["extent"] != group["_owner_layout"][binding["owner_symbol"]]["size"]:
        raise error_type("typed initialized array extent differs from reviewed owner")
    return parsed


SCALAR_LAYOUT = {
    "s8": (1, 1), "u8": (1, 1), "s16": (2, 2), "u16": (2, 2),
    "s32": (4, 4), "u32": (4, 4), "f32": (4, 4),
}


def natural_c_struct_layout(source_text, struct_name, error_type):
    """Compute a natural N64 layout for simple named scalar members."""
    def fail(message):
        raise error_type(message)

    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_text, flags=re.S)
    pattern = re.compile(
        r"typedef\s+struct\s*\{(?P<body>.*?)\}\s*"
        + re.escape(struct_name) + r"\s*;", re.S)
    matches = list(pattern.finditer(clean))
    if len(matches) != 1:
        fail("typed storage struct declaration is missing or ambiguous")
    body = matches[0].group("body")
    member_re = re.compile(
        r"\s*(s8|u8|s16|u16|s32|u32|f32)\s*(\*)?\s*"
        r"([A-Za-z_][A-Za-z0-9_]*)\s*;")
    members, consumed, cursor, struct_align = {}, 0, 0, 1
    for match in member_re.finditer(body):
        if body[consumed:match.start()].strip():
            fail("unsupported typed storage struct member syntax")
        base_type, pointer, name = match.groups()
        if name in members:
            fail("duplicate typed storage struct member")
        width, alignment = (4, 4) if pointer else SCALAR_LAYOUT[base_type]
        cursor = (cursor + alignment - 1) & ~(alignment - 1)
        members[name] = {"type": base_type + (" *" if pointer else ""),
                         "width": width, "offset": cursor,
                         "alignment": alignment}
        cursor += width
        struct_align = max(struct_align, alignment)
        consumed = match.end()
    if body[consumed:].strip() or not members:
        fail("unsupported or empty typed storage struct")
    extent = (cursor + struct_align - 1) & ~(struct_align - 1)
    return members, extent, struct_align


def validate_explicit_bss_member(group, binding, numeric_value, source_text,
                                 error_type):
    def fail(message):
        raise error_type(message)

    if numeric_value != binding["_addend"] or numeric_value != binding["_member_offset"]:
        fail("explicit BSS binding addend does not equal its reviewed member offset")
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_text, flags=re.S)
    if re.search(r"#\s*pragma\s+pack|__attribute__|_Alignas", clean):
        fail("explicit BSS owner uses non-natural layout syntax")
    declaration = re.compile(
        r"\b" + re.escape(group["struct_type"]) + r"\s+"
        + re.escape(group["owner_symbol"]) + r"\s*;")
    if len(declaration.findall(clean)) != 1:
        fail("typed owner object declaration is missing or ambiguous")
    members, natural_size, _align = natural_c_struct_layout(
        clean, group["struct_type"], error_type)
    if natural_size != group["_owner_size"]:
        fail("typed owner natural extent differs from reviewed size")
    member = members.get(binding["member"])
    if member is None:
        fail("typed BSS member is missing")
    if (member["type"] != binding["type"]
            or member["width"] != binding["_width"]
            or member["offset"] != binding["_member_offset"]):
        fail("typed BSS member disagrees with reviewed type/layout")
    if member["offset"] + member["width"] > group["_owner_size"]:
        fail("typed BSS member escapes object extent")
    return member


def has_unique_initialized_f32_scalar(source_text, symbol):
    source_text = re.sub(r"/\*.*?\*/|//[^\n]*", "", source_text, flags=re.S)
    literal = r"[+-]?(?:(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?)(?:[fFlL])?"
    declaration = re.compile(
        r"\bf32\s+" + re.escape(symbol)
        + r"\s*(?:=\s*" + literal + r")?\s*;")
    return len(declaration.findall(source_text)) == 1
