"""ORT names: the ROM's symbol-name table (JFG), or a tracked name list (MSU).

The tracked list is ``<row> <name>`` lines in row order, plus optional
``entry <module> init|resume <name>`` lines; ``#`` starts a comment.  It holds
names only.
"""

from __future__ import annotations

import struct
from pathlib import Path


class NameList:
    def __init__(self, rows, entries=None):
        self.rows = list(rows)
        self.entries = dict(entries or {})
        self.by_name = {}
        for i, n in enumerate(self.rows):
            self.by_name.setdefault(n, i)

    @classmethod
    def load(cls, path):
        rows, entries = [], {}
        for ln, line in enumerate(Path(path).read_text().splitlines(), 1):
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            parts = line.split()
            if parts[0] == "entry" and len(parts) == 4:
                entries[(int(parts[1]), parts[2])] = parts[3]
                continue
            if len(parts) != 2 or not parts[0].isdigit() or int(parts[0]) != len(rows):
                raise ValueError(f"{path}:{ln}: expected `<row> <name>` in row order")
            rows.append(parts[1])
        return cls(rows, entries)


def ort_names(cfg, tables):
    """One name per ORT row: the ROM's own, else the configured list's."""
    if tables.names is not None:
        return tables.names
    if cfg.name_list is not None and cfg.name_list.exists():
        return NameList.load(cfg.name_list).rows
    return [f"ort_{i}" for i in range(len(tables.ort))]


def encode_names(rows, cfg):
    """Re-encode a symbol-name table the way JFG ships it: offsets, then the blob.

    The offsets block is followed (after any gap) by the blob of
    NUL-terminated names in row order.  A name already stored is not stored
    again: its row points at the first copy (JFG US has one such row)."""
    offs = []
    blob = b""
    first = {}
    for n in rows:
        if n in first:
            offs.append(first[n])
            continue
        first[n] = len(blob)
        offs.append(len(blob))
        blob += n.encode("latin-1") + b"\0"
    head = b"".join(struct.pack(">I", o) for o in offs)
    gap = cfg.names_rom - (cfg.name_offsets_rom + len(head))
    return head + b"\0" * gap + blob
