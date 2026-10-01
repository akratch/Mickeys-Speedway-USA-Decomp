"""Small fail-closed helpers for reviewed N64 typed storage declarations."""

import re


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
