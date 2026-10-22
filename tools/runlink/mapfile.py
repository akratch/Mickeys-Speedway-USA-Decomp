"""GNU ld ``-Map`` reader: every placed input section, in link order."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class MapInput:
    outsec: str
    insec: str
    addr: int
    size: int
    file: str


@dataclass(frozen=True)
class MapOutput:
    name: str
    addr: int
    size: int


def parse(path):
    """(outputs {name: MapOutput}, inputs [MapInput] in map order)."""
    inputs, outputs = [], {}
    started = False
    cur = None
    pending = None
    head = re.compile(r"^(\.\S+)(?:\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+))?")
    one = re.compile(r"^ (\.\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    name_only = re.compile(r"^ (\.\S+)\s*$")
    cont = re.compile(r"^\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    out_cont = re.compile(r"^\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)")
    awaiting_out = None
    for line in Path(path).read_text(errors="replace").splitlines():
        if not started:
            started = line.startswith("Linker script and memory map")
            continue
        if line and not line[0].isspace():
            m = head.match(line)
            cur = m.group(1) if m else None
            pending = None
            awaiting_out = None
            if m and m.group(2):
                outputs[cur] = MapOutput(cur, int(m.group(2), 16), int(m.group(3), 16))
            elif m:
                awaiting_out = cur
            continue
        if cur is None:
            continue
        if awaiting_out:
            m = out_cont.match(line)
            awaiting_out_name, awaiting_out = awaiting_out, None
            if m:
                outputs[awaiting_out_name] = MapOutput(awaiting_out_name, int(m.group(1), 16), int(m.group(2), 16))
                continue
        m = one.match(line)
        if m:
            inputs.append(MapInput(cur, m.group(1), int(m.group(2), 16), int(m.group(3), 16), m.group(4)))
            pending = None
            continue
        m = name_only.match(line)
        if m:
            pending = m.group(1)
            continue
        m = cont.match(line)
        if m and pending:
            inputs.append(MapInput(cur, pending, int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
        pending = None
    return outputs, inputs
