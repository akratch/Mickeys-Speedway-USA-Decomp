"""Command line: ``python3 tools/runlink [--config NAME|PATH] [--root DIR] <command>``.

Commands
--------
  check     decode the tables, re-encode them, require byte equality with the ROM
  census    counts and every generation rule tested against the shipped tables
  decode    dump the decoded tables as JSON (to stdout or --out); ROM-derived, keep it local
  names     print the ORT as `<row> <name>` (from the ROM's name table, or the tracked list)
  generate  regenerate the tables from the build and compare (see docs/runlink-toolkit.md)
  parity    MSU only: the kit and tools/runlink_pack.py must give identical results

Exit status: 0 success; 1 a check failed; 2 unusable inputs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path

if __package__ in (None, ""):
    # Run as `python3 tools/runlink`: make the package importable as `runlink`.
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
    __package__ = "runlink"

from runlink import census, codec, config, names  # noqa: E402


def load(args):
    cfg = config.load(args.config, args.root)
    if args.rom:
        cfg.rom = Path(args.rom).resolve()
    if not cfg.rom or not cfg.rom.exists():
        raise config.ConfigError(f"ROM {cfg.rom} is missing (bring your own, or pass --rom)")
    rom = codec.read_rom(cfg.rom)
    if cfg.rom_sha1 and hashlib.sha1(rom).hexdigest() != cfg.rom_sha1:
        print(f"warning: {cfg.rom} is not the ROM {cfg.name} describes (sha1 {cfg.rom_sha1})",
              file=sys.stderr)
    return cfg, rom, codec.decode(rom, cfg)


def cmd_check(args):
    cfg, rom, tables = load(args)
    lo, hi = codec.region_bounds(tables, cfg)
    ok = True
    again = codec.encode_region(tables, cfg)
    if again != rom[lo:hi]:
        n = sum(1 for a, b in zip(again, rom[lo:hi]) if a != b) + abs(len(again) - (hi - lo))
        print(f"FAIL region {lo:#x}-{hi:#x}: re-encoding differs in {n} bytes")
        ok = False
    else:
        print(f"OK   region {lo:#x}-{hi:#x} ({hi - lo} bytes) re-encodes byte-identically")
    if cfg.modules_end is not None and hi != cfg.modules_end:
        print(f"FAIL last module ends at {hi:#x}, config says {cfg.modules_end:#x}")
        ok = False
    chained = all(m.header.rom == p for m, p in zip(tables.modules, _positions(tables.modules)))
    print(("OK  " if chained else "FAIL") + " headers chain back to back from the module base")
    ok &= chained
    if tables.names is not None:
        blob = names.encode_names(tables.names, cfg)
        a, b = cfg.name_offsets_rom, cfg.names_end
        if blob != rom[a:a + len(blob)]:
            print("FAIL symbol-name table does not re-encode byte-identically")
            ok = False
        else:
            print(f"OK   symbol-name table ({len(tables.names)} names) re-encodes byte-identically")
    return 0 if ok else 1


def _positions(modules):
    pos = 0
    for m in modules:
        yield pos
        pos += m.header.image_size


def cmd_census(args):
    cfg, rom, tables = load(args)
    out = {"config": cfg.name, "counts": census.counts(tables, cfg, rom),
           "rules": census.rule_checks(tables, cfg)}
    if args.json:
        Path(args.json).write_text(json.dumps(out, indent=1, default=str) + "\n")
    for k, v in out["counts"].items():
        if isinstance(v, dict):
            print(f"{k}:")
            for kk, vv in v.items():
                print(f"    {kk:32s} {vv}")
        else:
            print(f"{k:40s} {v}")
    print("rules (holds/total):")
    for k, v in out["rules"].items():
        print(f"    {v['holds']:6d}/{v['total']:<6d} {k}")
        if args.verbose and v["exceptions"]:
            for e in v["exceptions"][:args.limit]:
                print(f"           exception {e}")
    return 0


def cmd_decode(args):
    cfg, rom, tables = load(args)
    doc = {
        "main": [vars(r) for r in tables.main],
        "ort": [vars(r) for r in tables.ort],
        "modules": [{"number": m.number, "header": vars(m.header),
                     "table1": [vars(r) for r in m.table1],
                     "table2": [vars(r) for r in m.table2]} for m in tables.modules],
    }
    text = json.dumps(doc, indent=0)
    if args.out:
        Path(args.out).write_text(text + "\n")
    else:
        print(text)
    return 0


def cmd_names(args):
    cfg, rom, tables = load(args)
    rows = names.ort_names(cfg, tables)
    for i, n in enumerate(rows):
        print(f"{i} {n}")
    return 0


def cmd_generate(args):
    from runlink import generate
    cfg, rom, tables = load(args)
    return generate.main(cfg, rom, tables, args)


def cmd_parity(args):
    cfg, rom, tables = load(args)
    if cfg.build.get("adapter") != "msu":
        raise config.ConfigError(f"parity compares the kit with tools/runlink_pack.py; {cfg.name} has no such tool")
    from runlink.adapters import msu
    ok, _res, _proof = msu.parity(cfg, rom, tables, args.verbose)
    return 0 if ok else 1


def main(argv=None):
    ap = argparse.ArgumentParser(prog="runlink", description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--config", default="msu-us", help="config name (configs/) or path")
    ap.add_argument("--root", help="directory the config's relative paths resolve against")
    ap.add_argument("--rom", help="ROM image (overrides the config)")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("check")
    p = sub.add_parser("census")
    p.add_argument("--json")
    p.add_argument("-v", "--verbose", action="store_true")
    p.add_argument("--limit", type=int, default=20)
    p = sub.add_parser("decode")
    p.add_argument("--out")
    sub.add_parser("names")
    p = sub.add_parser("parity", help="MSU: the kit and tools/runlink_pack.py must agree exactly")
    p.add_argument("-v", "--verbose", action="store_true")
    p = sub.add_parser("generate")
    p.add_argument("--mode", choices=("objects", "hybrid", "oracle"), default="hybrid",
                   help="objects: identities from the build only; hybrid: placeholder identities from "
                        "the shipped records; oracle: every identity from the shipped records")
    p.add_argument("-v", "--verbose", action="store_true")
    p.add_argument("--limit", type=int, default=30)
    p.add_argument("--json", help="write the full comparison here")
    p.add_argument("--out", help="write the generated region here")
    p.add_argument("--unpatched", help="OLD=NEW: rewrite the map's object-path prefix OLD to NEW to find the "
                                       "resident objects as compiled, before trap renaming")
    args = ap.parse_args(argv)
    try:
        return {"check": cmd_check, "census": cmd_census, "decode": cmd_decode,
                "names": cmd_names, "generate": cmd_generate,
                "parity": cmd_parity}[args.cmd](args)
    except (config.ConfigError, codec.FormatError, OSError) as e:
        print(f"runlink: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
