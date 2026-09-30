#!/usr/bin/env python3
"""Narrow, read-only proof of one resident IDO switch table for search admission.

This does not change promotion, resolve arbitrary local data, or infer identity
from an anonymous symbol's spelling. The named linked table, its canonical
owner object, ROM bytes, source case domain and both dispatches are authority.
"""
from __future__ import annotations

import collections
import hashlib
import re
import struct
from pathlib import Path

import reloc_surface as rs


class UnsupportedLocalBinding(RuntimeError):
    def __init__(self, route):
        self.route = route
        super().__init__('unsupported-proof-route: ' + route)


def need(condition, reason):
    if not condition:
        raise RuntimeError('local table proof: ' + reason)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def unique_payload(elf, name):
    """Authenticate name-based section reads before using their bytes."""
    need(elf.names.count(name) == 1, 'ambiguous section name: ' + name)
    index, header = elf.section(name)
    need(header[1] == 1 and header[4] + header[5] <= len(elf.data),
         'missing or truncated PROGBITS storage: ' + name)
    need(not any(sh[1] == 4 and sh[7] == index for sh in elf.sh),
         'unsupported RELA storage: ' + name)
    tables = [i for i, sh in enumerate(elf.sh) if sh[1] == 2]
    for sh in elf.sh:
        if sh[1] != 9 or sh[7] != index:
            continue
        need(len(tables) == 1 and sh[6] == tables[0] and sh[9] in (0, 8)
             and sh[5] % 8 == 0 and sh[4] + sh[5] <= len(elf.data),
             'malformed or foreign REL storage: ' + name)
    return index, header


def integer(node):
    kind = type(node).__name__
    if kind == 'Constant' and node.type in ('int', 'long', 'unsigned int'):
        value = re.sub(r'[uUlL]+$', '', node.value)
        return int(value, 16 if value.lower().startswith('0x') else 8 if len(value) > 1 and value[0] == '0' else 10)
    if kind == 'UnaryOp' and node.op in ('+', '-'):
        return integer(node.expr) * (-1 if node.op == '-' else 1)
    raise UnsupportedLocalBinding('switch-nonliteral-case-domain')


def walk(node):
    pending, count = [node], 0
    while pending:
        value = pending.pop()
        count += 1
        need(count <= 100000, 'source AST exceeds proof bound')
        yield value
        pending.extend(child for _, child in reversed(list(value.children())))


def case_groups(switch):
    """Group stacked source labels, without requiring case-body spelling."""
    groups, labels, default = [], [], False
    for node in walk(switch.stmt):
        kind = type(node).__name__
        if kind not in ('Case', 'Default'):
            continue
        if kind == 'Case':
            labels.append(integer(node.expr))
        else:
            default = True
        if any(type(stmt).__name__ not in ('Case', 'Default', 'EmptyStatement', 'Pragma')
               for stmt in node.stmts):
            groups.append({'cases': sorted(labels), 'default': default})
            labels, default = [], False
    if labels or default:
        groups.append({'cases': sorted(labels), 'default': default})
    return sorted(groups, key=lambda row: (row['default'], row['cases']))


def source_switch(ast, symbol):
    """Read actual preprocessed AST; require a signed halfword parameter load."""
    functions = [n for n in ast.ext if type(n).__name__ == 'FuncDef' and n.decl.name == symbol]
    need(len(functions) == 1, 'source function is not unique')
    function = functions[0]
    switches = [n for n in walk(function.body) if type(n).__name__ == 'Switch']
    if len(switches) != 1:
        raise UnsupportedLocalBinding('single-switch-source-owner')
    switch = switches[0]
    cases = [integer(n.expr) for n in walk(switch.stmt) if type(n).__name__ == 'Case']
    need(cases and len(set(cases)) == len(cases), 'duplicate or empty source cases')
    need(sum(type(n).__name__ == 'Default' for n in walk(switch.stmt)) == 1, 'source default is not unique')
    typedefs = {n.name: n.type for n in ast.ext if type(n).__name__ == 'Typedef'}
    cond = switch.cond
    if not (type(cond).__name__ == 'UnaryOp' and cond.op == '*'
            and type(cond.expr).__name__ == 'Cast'):
        raise UnsupportedLocalBinding('signed-halfword-switch-selector')
    cast = cond.expr
    typ = cast.to_type.type
    need(type(typ).__name__ == 'PtrDecl', 'selector is not a pointer load')
    typ = typ.type
    seen = set()
    while type(typ).__name__ == 'TypeDecl':
        typ = typ.type
        if type(typ).__name__ == 'IdentifierType' and len(typ.names) == 1 and typ.names[0] in typedefs:
            name = typ.names[0]
            need(name not in seen, 'cyclic selector typedef')
            seen.add(name)
            typ = typedefs[name]
    need(type(typ).__name__ == 'IdentifierType'
         and set(typ.names) in ({'short'}, {'signed', 'short'}, {'signed', 'short', 'int'}, {'short', 'int'}),
         'selector domain is not signed 16-bit')
    expr = cast.expr
    if type(expr).__name__ != 'BinaryOp' or expr.op != '+' or type(expr.left).__name__ != 'ID':
        raise UnsupportedLocalBinding('parameter-plus-literal-switch-selector')
    params = function.decl.type.args.params
    names = [getattr(p, 'name', None) for p in params]
    need(names.count(expr.left.name) == 1 and names.index(expr.left.name) < 4, 'selector parameter is not an argument register')
    parameter = params[names.index(expr.left.name)].type
    need(type(parameter).__name__ == 'PtrDecl', 'selector base parameter is not a pointer')
    # Pointer arithmetic must be byte arithmetic, not scaled by another type.
    ptype = parameter.type
    while type(ptype).__name__ == 'TypeDecl':
        ptype = ptype.type
        if type(ptype).__name__ == 'IdentifierType' and len(ptype.names) == 1 and ptype.names[0] in typedefs:
            ptype = typedefs[ptype.names[0]]
    need(type(ptype).__name__ == 'IdentifierType' and 'char' in ptype.names,
         'selector parameter arithmetic is not byte-sized')
    return {'cases': sorted(cases), 'selector_argument': names.index(expr.left.name),
            'selector_offset': integer(expr.right), 'signed_bits': 16,
            'case_groups': case_groups(switch)}


def fields(word):
    return word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31, word & 63


class Code:
    """Bounded def/use on ordinary MIPS control flow, including delay slots."""
    def __init__(self, elf, symbol):
        self.elf = elf
        text_index, _ = unique_payload(elf, '.text')
        self.start, self.size, section = rs._unique_symbol(elf, symbol, require_text=True)
        need(section == '.text' and self.size and self.size % 4 == 0, 'invalid owned function')
        owners = [row for row in elf.symbols() if row[0] == symbol and row[2] > 0]
        need(len(owners) == 1 and owners[0][4] == text_index and owners[0][3] & 15 == 2,
             'function symbol does not own the selected executable section')
        data = elf.section_bytes(section)[self.start:self.start + self.size]
        need(len(data) == self.size and self.size <= 65536, 'invalid function extent')
        self.words = dict(enumerate(struct.unpack('>' + 'I' * (len(data) // 4), data)))
        self.words = {i * 4: w for i, w in self.words.items()}
        self.symbols = elf.symbols()
        self.relocs = {}
        for _, offset, kind, index in elf.relocations():
            if self.start <= offset < self.start + self.size:
                site = offset - self.start
                need(site not in self.relocs and site in self.words and index < len(self.symbols), 'invalid relocation geometry')
                self.relocs[site] = kind, index
        self.pred = collections.defaultdict(set)
        edges, delays = {}, {}
        for site, word in self.words.items():
            op, a, b, d, shift, fn = fields(word)
            if rs._pc16_branch_instruction(word):
                target = self.branch_destination(site)
                need(site + 4 in self.words and target in self.words, 'branch escapes function')
                likely = op in (20, 21, 22, 23) or (op == 1 and b in (2, 3, 18, 19))
                edges[site] = [site + 4, site + 8] if likely else [site + 4]
                delays[site + 4] = [target] if likely else [target, site + 8]
            elif op in (2, 3) or (op == 0 and fn in (8, 9)):
                need(site + 4 in self.words, 'missing jump delay slot')
                edges[site] = [site + 4]
                # Calls return; indirect jumps terminate this local flow.
                if op == 3 or (op == 0 and fn == 9):
                    delays[site + 4] = [site + 8]
                elif op == 2:
                    raise UnsupportedLocalBinding('absolute-local-jump-cfg')
                else:
                    delays[site + 4] = []
            else:
                edges[site] = [site + 4]
        for site, successors in delays.items():
            op, _, _, _, _, fn = fields(self.words[site])
            need(not rs._pc16_branch_instruction(self.words[site]) and op not in (2, 3)
                 and not (op == 0 and fn in (8, 9)), 'control transfer in delay slot')
            edges[site] = successors
        for site, successors in edges.items():
            for dest in successors:
                if dest in self.words:
                    if dest in delays:
                        need(site == dest - 4, 'entry into a delay slot')
                    self.pred[dest].add(site)
        self.reachable = set()
        pending = [0]
        while pending:
            site = pending.pop()
            if site in self.reachable:
                continue
            self.reachable.add(site)
            pending.extend(x for x in edges.get(site, []) if x in self.words)

    def branch_destination(self, site):
        word = self.words[site]
        if site in self.relocs:
            kind, index = self.relocs[site]
            need(kind == 10, 'branch has a non-PC16 relocation')
            _, value, _, _, section = self.symbols[index]
            need(section == self.elf.section('.text')[0], 'PC16 destination namespace differs')
            dest = value + rs.sext16(word & 65535) * 4 + 4 - self.start
        else:
            dest = site + 4 + rs.sext16(word & 65535) * 4
        need(dest in self.words, 'PC16 addend escapes owned function')
        return dest

    def writes(self, site):
        word = self.words[site]
        op, a, b, d, shift, fn = fields(word)
        if op == 0:
            if fn in (8, 12, 13, 16 + 1, 18 + 1, 24, 25, 26, 27):
                return set()
            if fn == 9:
                return set(range(1, 16)) | {24, 25, 31}
            return {d} - {0}
        if op == 3:
            return set(range(1, 16)) | {24, 25, 31}
        if op in (8, 9, 10, 11, 12, 13, 14, 15, 32, 33, 34, 35, 36, 37, 38, 39):
            return {b} - {0}
        if op in (16, 17, 18) and a in (0, 2):
            return {b} - {0}
        if op in (1, 2, 4, 5, 6, 7, 16, 17, 18, 20, 21, 22, 23, 40, 41, 42, 43, 46, 49, 53, 57, 61):
            return set()
        raise UnsupportedLocalBinding('dispatch-instruction-def-use')

    def definitions(self, site, register):
        need(register != 0, 'zero register cannot define dispatch input')
        result, visited = set(), set()
        pending = list(self.pred[site])
        if site == 0:
            result.add(-1)
        while pending:
            pos = pending.pop()
            if pos in visited or pos not in self.reachable:
                continue
            visited.add(pos)
            if register in self.writes(pos):
                result.add(pos)
            elif pos == 0:
                result.add(-1)
            else:
                pending.extend(self.pred[pos])
        need(result, 'dispatch input has no reaching definition')
        return result

    def one(self, site, register):
        values = self.definitions(site, register)
        need(len(values) == 1 and -1 not in values, 'ambiguous dispatch reaching definition')
        return next(iter(values))

    def argument(self, site, register, expected, visited=None):
        visited = set() if visited is None else set(visited)
        need((site, register) not in visited and len(visited) < 64, 'cyclic argument provenance')
        visited.add((site, register))
        for pos in self.definitions(site, register):
            if pos == -1:
                need(register == 4 + expected, 'selector does not originate at source argument')
                continue
            op, a, b, d, shift, fn = fields(self.words[pos])
            need(op == 0 and fn in (33, 37) and (a == 0 or b == 0), 'unsupported selector argument definition')
            self.argument(pos, a or b, expected, visited)


def dispatch(code, hi, lo, shape):
    """Trace the table load back through scale, bound, origin and signed load."""
    words = code.words
    op, base, output, _, _, _ = fields(words[lo])
    need(op == 35, 'table LO is not a word load')
    add = code.one(lo, base)
    op, a, b, d, _, fn = fields(words[add])
    need(op == 0 and fn == 33, 'table address is not an unsigned register sum')
    defs = [(reg, code.one(add, reg)) for reg in (a, b)]
    need(sum(pos == hi for _, pos in defs) == 1, 'HI does not reach table address')
    scale = next(pos for _, pos in defs if pos != hi)
    op, a, index, d, shift, fn = fields(words[scale])
    need(op == 0 and fn == 0 and a == 0 and shift == 2, 'table index is not scaled by four')
    origin = code.one(scale, index)
    op, selector, dest, _, _, _ = fields(words[origin])
    need(op == 9 and dest == index, 'table index lacks subtract/origin definition')
    first = -rs.sext16(words[origin] & 65535)
    signed_loads = code.definitions(origin, selector)
    for pos in signed_loads:
        need(pos >= 0 and fields(words[pos])[0] == 33, 'switch selector is not sign-extended LH')
        _, source_base, _, _, _, _ = fields(words[pos])
        need(rs.sext16(words[pos] & 65535) == shape['selector_offset'], 'selector byte offset differs from source')
        code.argument(pos, source_base, shape['selector_argument'])
    bounds = []
    for pos, word in words.items():
        op, a, b, _, _, _ = fields(word)
        if op != 11 or a != index or code.definitions(pos, a) != {origin}:
            continue
        for branch, bw in words.items():
            bop, ba, bb, _, _, _ = fields(bw)
            if bop != 4 or {ba, bb} != {0, b} or code.definitions(branch, b) != {pos}:
                continue
            # The table path must be dominated by the fallthrough guard. All
            # backward paths from scale/load must pass this branch. Delay-slot
            # scaling is valid; entry from another predecessor is not.
            pending, seen = [lo], set()
            guarded = True
            while pending:
                node = pending.pop()
                if node == branch or node in seen:
                    continue
                seen.add(node)
                if node == 0 or not code.pred[node]:
                    guarded = False
                    break
                pending.extend(p for p in code.pred[node] if p in code.reachable)
            default = code.branch_destination(branch)
            if guarded and default not in seen:
                bounds.append((word & 65535, default, branch))
    need(len(bounds) == 1, 'unsigned range guard is missing or ambiguous')
    count, default, branch = bounds[0]
    need(0 < count <= 4096 and first == min(shape['cases'])
         and first + count - 1 == max(shape['cases']), 'index origin or entry count differs from source domain')
    consumers = [pos for pos, word in words.items() if fields(word)[0] == 0 and fields(word)[5] == 8
                 and fields(word)[1] == output and code.definitions(pos, output) == {lo}]
    need(len(consumers) == 1, 'table load does not feed one owned indirect jump')
    need(fields(words[hi])[0] == 15, 'table HI is not LUI')
    return {'first': first, 'count': count, 'default': default, 'hi': hi, 'lo': lo,
            'origin': origin, 'branch': branch, 'jump': consumers[0]}


def pairs(code):
    pending, result = {}, []
    for site, (kind, index) in sorted(code.relocs.items()):
        if kind == 5:
            pending.setdefault(index, []).append(site)
        elif kind == 6:
            highs = pending.pop(index, [])
            if len(highs) == 1:
                hi = highs[0]
                addend = ((code.words[hi] & 65535) << 16) + rs.sext16(code.words[site] & 65535)
                result.append((hi, site, index, addend))
    return result


def partition(entries, info, shape):
    need(len(entries) == info['count'], 'table entry count differs from dispatch')
    cases = set(shape['cases'])
    for index, value in enumerate(entries):
        need((value == info['default']) == (index + info['first'] not in cases),
             'case/default partition differs from source')
    groups, result = {}, []
    for value in entries:
        if value not in groups:
            groups[value] = len(groups)
        result.append(groups[value])
    return result


def candidate_table(path, symbol, shape):
    elf = rs.Elf(path)
    code = Code(elf, symbol)
    rodata, header = unique_payload(elf, '.rodata')
    local = [(hi, lo, index, addend) for hi, lo, index, addend in pairs(code)
             if code.symbols[index][4] == rodata and rodata is not None]
    if not local:
        raise UnsupportedLocalBinding('resident-single-rodata-switch-table')
    need(len(local) == 1, 'multiple candidate table owners')
    hi, lo, index, addend = local[0]
    _, value, _, info, section = code.symbols[index]
    need(info & 15 == 3, 'candidate table requires a section-symbol relocation')
    start = value + addend
    spec = dispatch(code, hi, lo, shape)
    end = start + spec['count'] * 4
    data = elf.section_bytes('.rodata')
    need(start >= 0 and start % 4 == 0 and end <= len(data), 'HI/LO addend escapes candidate table extent')
    relocations = [(offset, kind, symbol_index) for _, offset, kind, symbol_index in elf.relocations(r'\.rodata')
                   if start <= offset < end]
    need([offset for offset, _, _ in sorted(relocations)] == list(range(start, end, 4)),
         'candidate table lacks exactly one relocation per entry')
    entries = []
    text_index = elf.section('.text')[0]
    for offset, kind, symbol_index in sorted(relocations):
        need(kind == 2 and symbol_index < len(code.symbols), 'candidate entry is not an owned R_MIPS_32')
        _, base, _, _, owner = code.symbols[symbol_index]
        need(owner == text_index, 'candidate table target crosses namespace')
        dest = base + int.from_bytes(data[offset:offset + 4], 'big') - code.start
        need(dest in code.words, 'candidate table target escapes owned function')
        entries.append(dest)
    # Every text reference to this local range must belong to this dispatch.
    # Other tables in the full TU are allowed, but overlapping owners are not.
    text = elf.section_bytes('.text')
    pending = {}
    owners = []
    for _, offset, kind, idx in sorted(elf.relocations(), key=lambda r: r[1]):
        if idx >= len(code.symbols) or code.symbols[idx][4] != rodata:
            continue
        word = int.from_bytes(text[offset:offset + 4], 'big')
        if kind == 5:
            pending.setdefault(idx, []).append((offset, word & 65535))
        elif kind == 6:
            highs = pending.pop(idx, [])
            need(bool(highs), 'unpaired local LO prevents unique ownership proof')
            for pos, high in highs:
                target = code.symbols[idx][1] + (high << 16) + rs.sext16(word & 65535)
                if start <= target < end:
                    owners.append((pos - code.start, offset - code.start, target))
        else:
            raise UnsupportedLocalBinding('non-hilo-local-table-owner')
    need(not pending, 'unpaired local HI prevents unique ownership proof')
    need(owners == [(hi, lo, start)], 'candidate table ownership is shared or ambiguous')
    return {'sites': [[hi, 5], [lo, 6]], 'table_offset': start, 'entries': entries,
            'dispatch': spec, 'partition': partition(entries, spec, shape), 'shape': shape,
            'object_sha256': digest(path)}


def target_table(root, symbol, source, candidate, shape):
    """Authenticate named target table via linker owner, YAML mapping and ROM."""
    import yaml
    root = Path(root)
    linked_path = root / 'build/mickey.us.elf'
    object_path = rs.resolve_resident_target_object(candidate, source)
    linked, obj = rs.Elf(linked_path), rs.Elf(object_path)
    code = Code(obj, symbol)
    value, size, section = rs._unique_symbol(linked, symbol)
    need(section == '.main', 'target function is outside resident namespace')
    unique_payload(linked, section)
    # Existing target adapter authenticates all instruction fields and PC16
    # actual addends/destinations, rather than merely trusting local labels.
    rs._resident_target_records(candidate, source, linked, symbol, value, size, section, rs.LINK_SYMS)
    symbols = linked.symbols()
    possible = []
    for hi, lo, index, addend in pairs(code):
        name, _, _, _, shndx = code.symbols[index]
        if shndx != 0 or addend != 0:
            continue
        matches = [row for row in symbols if row[0] == name and row[2] > 0]
        if len(matches) != 1:
            continue
        row = matches[0]
        if row[4] != linked.section(section)[0] or row[3] & 15 != 1:
            continue
        # Dispatch decoder is selected by the LW/indirect-jump def/use, not
        # by the target symbol's name or a candidate-site correlation.
        if fields(code.words[lo])[0] != 35:
            continue
        output = fields(code.words[lo])[2]
        jumps = [pos for pos, word in code.words.items()
                 if fields(word)[0] == 0 and fields(word)[5] == 8
                 and fields(word)[1] == output and code.definitions(pos, output) == {lo}]
        if not jumps:
            continue
        spec = dispatch(code, hi, lo, shape)
        possible.append((row, spec))
    need(len(possible) == 1, 'named target table owner is missing or ambiguous')
    row, spec = possible[0]
    name, address, extent, _, _ = row
    need(extent == spec['count'] * 4, 'named target table extent differs from dispatch')
    map_path = root / 'build/mickey.us.map'
    owners = []
    for line in map_path.read_text().splitlines():
        match = re.fullmatch(r'\s+\.rodata\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+(build/\S+\.o)\s*', line)
        if match:
            base, length = int(match[1], 16), int(match[2], 16)
            if base <= address and address + extent <= base + length:
                owners.append((base, length, root / match[3]))
    need(len(owners) == 1, 'target table linker owner is ambiguous')
    owner_base, owner_size, owner_path = owners[0]
    owner = rs.Elf(owner_path)
    unique_payload(owner, '.rodata')
    owned = [s for s in owner.symbols() if s[0] == name]
    need(len(owned) == 1 and owned[0][1] + owner_base == address and owned[0][2] == extent
         and owned[0][4] == owner.section('.rodata')[0], 'named table disagrees with canonical owner object')
    yaml_path = root / 'mickey.us.yaml'
    segments = yaml.safe_load(yaml_path.read_text())['segments']
    layouts = [s for s in segments if isinstance(s, dict) and '.' + s.get('name', '') == section]
    need(len(layouts) == 1 and layouts[0].get('type') == 'code', 'target namespace lacks unique YAML owner')
    layout = layouts[0]
    rom_start = address - layout['vram'] + layout['start']
    subsegments = [s for s in layout['subsegments'] if isinstance(s, list) and len(s) >= 2 and isinstance(s[0], int)]
    intervals = [(s, subsegments[i + 1][0]) for i, s in enumerate(subsegments[:-1])]
    owners_yaml = [(s, end) for s, end in intervals if s[0] <= rom_start and rom_start + extent <= end and s[1] in ('rodata', '.rodata')]
    need(len(owners_yaml) == 1, 'table extent lacks YAML rodata ownership')
    sub, end = owners_yaml[0]
    expected = ('build/src/' + sub[2] + '.c.o') if sub[1] == '.rodata' else ('build/asm/data/' + (sub[2] if len(sub) > 2 else format(sub[0], 'X')) + '.rodata.s.o')
    need(owner_path.relative_to(root).as_posix() == expected, 'YAML and linker table owners disagree')
    data = linked.section_bytes(section)
    local = address - linked.section(section)[1][3]
    table_bytes = data[local:local + extent]
    rom_path = root / 'baseroms/mickey.us.z64'
    need(len(table_bytes) == extent and rom_path.read_bytes()[rom_start:rom_start + extent] == table_bytes,
         'linked target table differs from ROM')
    entries = [dest - value for dest in struct.unpack('>' + 'I' * spec['count'], table_bytes)]
    need(all(0 <= dest < size and dest % 4 == 0 for dest in entries), 'target table escapes owned function')
    # Independently reproduce each table pointer from the canonical owner
    # object's symbol/REL addend; the linked ELF alone is not its own witness.
    table_start = owned[0][1]
    rels = [(off, kind, index) for _, off, kind, index in owner.relocations(r'\.rodata') if table_start <= off < table_start + extent]
    need([off for off, _, _ in sorted(rels)] == list(range(table_start, table_start + extent, 4)), 'target table relocation coverage is incomplete')
    raw = owner.section_bytes('.rodata')
    for i, (off, kind, index) in enumerate(sorted(rels)):
        need(kind == 2 and index < len(owner.symbols()), 'unsupported target table entry relocation')
        target_name, target_base, _, _, target_section = owner.symbols()[index]
        need(target_section == 0, 'target table entry needs named external text owner')
        matches = [s for s in symbols if s[0] == target_name]
        need(len(matches) == 1 and matches[0][4] == linked.section(section)[0], 'target table entry identity/namespace is ambiguous')
        expected_address = matches[0][1] + int.from_bytes(raw[off:off + 4], 'big')
        need(expected_address == value + entries[i], 'target table REL addend disagrees with linked entry')
        # The label must independently belong to this canonical function.
        labels = [s for s in code.symbols if s[0] == target_name]
        need(len(labels) == 1 and labels[0][4] == obj.section('.text')[0]
             and labels[0][1] - code.start == entries[i], 'target table label lacks owned text correspondence')
    return {'identity': [0, address - rs.ot.RESIDENT_VRAM_BASE], 'name': name,
            'entries': entries, 'dispatch': spec, 'partition': partition(entries, spec, shape),
            'authority': {str(p.relative_to(root)): digest(p) for p in (linked_path, object_path, owner_path, map_path, yaml_path, rom_path)}}


def authenticate(root, symbol, source, candidate, shape, overlay=None):
    need(overlay is None and not source.startswith('overlays/'), 'local table identity crosses overlay namespace')
    local = candidate_table(candidate, symbol, shape)
    target = target_table(root, symbol, source, candidate, shape)
    need(local['partition'] == target['partition'], 'candidate/target case destination partition differs')
    need(digest(candidate) == local['object_sha256'], 'candidate changed during proof')
    need(all(digest(Path(root) / path) == value for path, value in target['authority'].items()),
         'target authority changed during proof')
    return {'schema': 'mickey-resident-switch-table-v1', 'candidate': local, 'target': target,
            'records': [[site, kind, target['identity']] for site, kind in local['sites']]}


def fidelity(left, right):
    """Exact function-relative local fields, independent of full-TU bases."""
    for key in ('sites', 'entries', 'dispatch', 'shape', 'partition'):
        need(left[key] == right[key], 'raw/full-TU table fidelity differs: ' + key)
