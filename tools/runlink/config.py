"""Per-game configuration: where the tables are, and the game's constants.

Everything game-specific lives in one TOML (or JSON) file; the code reads
nothing else about a game.  See ``configs/*.toml`` for the two shipped
configurations and docs/runlink-toolkit.md for the keys.

Relative paths in a config resolve against ``root``: the ``--root`` option,
else the config's own ``root`` key (itself relative to the config file),
else the current directory.
"""

from __future__ import annotations

import json
import os
from dataclasses import dataclass, field
from pathlib import Path

try:
    import tomllib
except ImportError:  # Python < 3.11
    tomllib = None

PACKAGE = Path(__file__).resolve().parent
CONFIGS = PACKAGE / "configs"


class ConfigError(ValueError):
    pass


def _int(v, key):
    if isinstance(v, int):
        return v
    if isinstance(v, str):
        try:
            return int(v, 0)
        except ValueError:
            pass
    raise ConfigError(f"{key}: expected an integer, got {v!r}")


@dataclass
class Config:
    name: str
    path: Path
    root: Path
    rom: Path
    rom_sha1: str
    main_table_rom: int
    main_table_block: int
    ort_rom: int
    ort_count: int
    headers_rom: int
    module_count: int
    modules_rom: int
    modules_end: int
    names_rom: int = None          # name blob start (ROM symbol-name table)
    names_end: int = None
    name_offsets_rom: int = None
    resident_text: int = 0x80000450
    resident_data: int = None
    resident_bss: int = None
    resident_end: int = None
    resident_rom_delta: int = None  # resident VRAM minus ROM offset
    trap_symbol: str = "TrapDanglingJump"
    trap_address: int = None
    data_selectors: tuple = (0xFFD, 0xFFE)
    bss_selector: int = 0xFFF
    reserved_selectors: tuple = (0xFFC, 0xFFD, 0xFFE, 0xFFF)
    absolute_selector: int = None  # a selector whose offsets are from absolute_base
    absolute_base: int = 0x80000000
    name_list: Path = None         # tracked `<row> <name>` list, when the ROM ships none
    build: dict = field(default_factory=dict)   # generator inputs, adapter-specific

    def resolve(self, p):
        if p is None:
            return None
        p = Path(os.path.expanduser(str(p)))
        return p if p.is_absolute() else self.root / p


def load(spec, root=None) -> Config:
    """Load a config by path, or by short name (``msu-us``, ``jfg-us``)."""
    p = Path(spec)
    if not p.exists():
        for ext in (".toml", ".json"):
            cand = CONFIGS / f"{spec}{ext}"
            if cand.exists():
                p = cand
                break
        else:
            raise ConfigError(f"no config {spec!r} (looked in {CONFIGS})")
    raw = p.read_bytes()
    if p.suffix == ".json":
        doc = json.loads(raw)
    else:
        if tomllib is None:
            raise ConfigError("TOML configs need Python 3.11+ (tomllib); use a .json config")
        doc = tomllib.loads(raw.decode())
    if root is not None:
        base = Path(root).resolve()
    elif "root" in doc:
        base = (p.parent / os.path.expanduser(doc["root"])).resolve()
    else:
        base = Path.cwd()
    t = doc.get("tables", {})
    res = doc.get("resident", {})
    sel = doc.get("selectors", {})
    names = doc.get("names", {})

    def req(d, k, section):
        if k not in d:
            raise ConfigError(f"{p}: [{section}] lacks {k}")
        return _int(d[k], f"{section}.{k}")

    def opt(d, k, section, default=None):
        return _int(d[k], f"{section}.{k}") if k in d else default

    cfg = Config(
        name=doc.get("name", p.stem), path=p, root=base,
        rom=None, rom_sha1=doc.get("rom_sha1", ""),
        main_table_rom=req(t, "main_table", "tables"),
        main_table_block=req(t, "main_table_block", "tables"),
        ort_rom=req(t, "ort", "tables"),
        ort_count=req(t, "ort_count", "tables"),
        headers_rom=req(t, "headers", "tables"),
        module_count=req(t, "module_count", "tables"),
        modules_rom=req(t, "modules", "tables"),
        modules_end=opt(t, "modules_end", "tables"),
        names_rom=opt(names, "blob", "names"),
        names_end=opt(names, "blob_end", "names"),
        name_offsets_rom=opt(names, "offsets", "names"),
        resident_text=opt(res, "text", "resident", 0x80000450),
        resident_data=opt(res, "data", "resident"),
        resident_bss=opt(res, "bss", "resident"),
        resident_end=opt(res, "end", "resident"),
        resident_rom_delta=opt(res, "rom_delta", "resident"),
        trap_symbol=res.get("trap_symbol", "TrapDanglingJump"),
        trap_address=opt(res, "trap_address", "resident"),
        data_selectors=tuple(_int(x, "selectors.data") for x in sel.get("data", [0xFFD, 0xFFE])),
        bss_selector=_int(sel.get("bss", 0xFFF), "selectors.bss"),
        reserved_selectors=tuple(_int(x, "selectors.reserved")
                                 for x in sel.get("reserved", [0xFFC, 0xFFD, 0xFFE, 0xFFF])),
        absolute_selector=opt(sel, "absolute", "selectors"),
        absolute_base=opt(sel, "absolute_base", "selectors", 0x80000000),
        build=doc.get("build", {}),
    )
    cfg.rom = cfg.resolve(doc.get("rom", "baserom.z64"))
    if "list" in names:
        cfg.name_list = cfg.resolve(names["list"])
    if (cfg.names_rom is None) != (cfg.name_offsets_rom is None):
        raise ConfigError(f"{p}: [names] needs both offsets and blob, or neither")
    if cfg.names_rom is not None and cfg.names_end is None:
        raise ConfigError(f"{p}: [names] lacks blob_end")
    return cfg
