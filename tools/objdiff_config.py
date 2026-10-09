#!/usr/bin/env python3
"""Derives objdiff.json from the project's tracked inputs.

Sources of truth, none of them the build/ tree:

  * mickey.us.yaml            -- every `c` subsegment is a translation unit
    (src/<dir>/<name>.c); every `asm`/`bin` subsegment names its object.
  * config/overlays.us.json   -- the overlay atlas: each module's
    text_ownership lists the C and asm sources that make up its .text.
  * the split output on disk  -- asm/**/*.s and assets/**/*.bin, discovered
    exactly as the Makefile's S_FILES/BIN_DIRS wildcards discover them
    (splat auto-names its data/rodata files, so the yaml cannot list them).
  * `gmake -nB`               -- the exact per-TU compiler flags the build uses
    (dry run: nothing is compiled).

A source file under src/ that neither the yaml nor the atlas names is an
orphan: the Makefile compiles it but the linker script never places it, so it
gets no unit.

Units that are C get a base_path (what the build produces) and a target_path
(expected/build/, see tools/make_expected.sh). Assembly-only units (asm/*.s,
assets/*.bin) carry no base_path, so objdiff has nothing to score them
against and does not count them as decompiled; they still list the target so
their symbols appear in the project. `metadata.complete` is true only for a C
unit with no `#pragma GLOBAL_ASM`.

    tools/objdiff_config.py               # write objdiff.json
    tools/objdiff_config.py --check       # exit 1 if objdiff.json drifted
    tools/objdiff_config.py --stdout      # print instead of writing
    tools/objdiff_config.py --apply-excludes --stdout   # what objdiff-cli is fed
    tools/objdiff_config.py --verify-built  # also require build/ objects exist
    tools/objdiff_config.py --base-dir build_non_matching --stdout

Needs the split tree (`gmake extract`) but no compiled objects.
"""
import argparse
import difflib
import json
import os
import pathlib
import re
import shlex
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
EXCLUDE_FILE = "tools/objdiff_exclude.txt"
OVERLAY_RE = re.compile(r"(?:^|/)overlays/(o\d{3})/")
GLOBAL_ASM_RE = re.compile(r"^\s*#\s*pragma\s+GLOBAL_ASM\b", re.M)
SCRATCH_COMPILER = "ido5.3"
CATEGORIES = [("resident", "resident"), ("overlay", "overlay"), ("libultra", "libultra")]


def load_yaml(path):
    import yaml

    with open(path) as f:
        return yaml.safe_load(f)


def yaml_units(y):
    """(type, name, dir) for every named c/asm/hasm/bin subsegment."""
    out = []
    for seg in y.get("segments", []):
        if not isinstance(seg, dict):
            continue
        d = seg.get("dir", "") or ""
        for sub in seg.get("subsegments", []) or []:
            if isinstance(sub, dict):
                typ, name = sub.get("type"), sub.get("name")
            else:
                typ = sub[1] if len(sub) > 1 else None
                name = sub[2] if len(sub) > 2 else None
            if typ in ("c", "asm", "hasm", "bin") and isinstance(name, str):
                out.append((typ, name, d))
    return out


def yaml_paths(y):
    """Relative source paths the yaml declares, as {path: type}."""
    opt = y.get("options", {})
    src, asm, assets = (opt.get("src_path", "src"), opt.get("asm_path", "asm"),
                        opt.get("asset_path", "assets"))
    res = {}
    for typ, name, d in yaml_units(y):
        pre, ext = {"c": (src, ".c"), "asm": (asm, ".s"), "hasm": (asm, ".s"),
                    "bin": (assets, ".bin")}[typ]
        res[os.path.join(pre, d, name + ext).replace(os.sep, "/")] = typ
    return res


def atlas_paths(atlas):
    """{path: type} for every source in every module's text_ownership."""
    res = {}
    for mod in atlas.get("modules", []):
        for t in mod.get("text_ownership", []):
            pre, ext = ("src", ".c") if t["type"] == "c" else ("asm", ".s")
            res[f"{pre}/{t['source']}{ext}"] = t["type"]
    return res


def disk_objects(root, y):
    """asm/**.s and assets/**.bin, the way the Makefile wildcards find them."""
    opt = y.get("options", {})
    found = []
    for top, ext in ((opt.get("asm_path", "asm"), ".s"),
                     (opt.get("asset_path", "assets"), ".bin")):
        base = root / top
        if not base.is_dir():
            continue
        for dp, dns, fns in os.walk(base):
            dns[:] = sorted(d for d in dns if not (dp == str(base) and d == "nonmatchings"))
            for fn in sorted(fns):
                if fn.endswith(ext):
                    found.append(pathlib.Path(dp, fn).relative_to(root).as_posix())
    return found


def categories_for(path):
    if "/libultra/" in path or path.startswith("libultra/"):
        return ["libultra"]
    m = OVERLAY_RE.search(path)
    if m:
        return ["overlay", f"overlay-{m.group(1)}"]
    return ["resident"]


def collect(root, y, atlas):
    """Return (c_sources, other_sources, orphans, problems)."""
    declared = yaml_paths(y)
    atl = atlas_paths(atlas) if atlas else {}
    problems = []
    c = {p for p, t in declared.items() if t == "c"}
    c_atlas = {p for p, t in atl.items() if t == "c"}
    # The atlas covers overlays only; the yaml must agree with it there.
    c_yaml_ov = {p for p in c if OVERLAY_RE.search(p)}
    for p in sorted(c_atlas - c_yaml_ov):
        problems.append(f"atlas names {p} but the yaml has no such c subsegment")
    for p in sorted(c_yaml_ov - c_atlas):
        problems.append(f"yaml names {p} but the atlas does not own it")
    c |= c_atlas
    for p in sorted(c):
        if not (root / p).is_file():
            problems.append(f"missing source file {p}")
    others = set(disk_objects(root, y))
    # Atlas asm entries (e.g. *_padding) can be a few bytes folded into a C
    # object, so only the yaml's own asm/bin names must exist as split files.
    for p, t in declared.items():
        if t != "c" and not (root / p).is_file() and (root / "asm").is_dir():
            problems.append(f"missing split file {p}")
    srcdir = y.get("options", {}).get("src_path", "src")
    orphans = []
    if (root / srcdir).is_dir():
        for dp, dns, fns in os.walk(root / srcdir):
            for fn in fns:
                if fn.endswith(".c"):
                    p = pathlib.Path(dp, fn).relative_to(root).as_posix()
                    if p not in c:
                        orphans.append(p)
    return sorted(c), sorted(others), sorted(orphans), problems


def compiler_flags(toks):
    """Drop include paths and -D defines: they are project-wide, objdiff's
    scratch export does not need them, and they would triple the file size."""
    out, skip = [], False
    for t in toks:
        if skip:
            skip = False
        elif t == "-I":
            skip = True
        elif not (t.startswith("-D") or t.startswith("-I") or t == "-nostdinc"):
            out.append(t)
    return out


def compile_flags(root, c_sources, make="gmake"):
    """{source: flag string} from a dry run of the real build rules."""
    targets = [f"build/{s}.o" for s in c_sources]
    r = subprocess.run([make, "-nB", "--no-print-directory", *targets], cwd=root,
                       capture_output=True, text=True)
    flags = {}
    for line in r.stdout.replace("\t", " ").splitlines():
        if " -c " not in line or " -o build/" not in line:
            continue
        toks = shlex.split(line)
        try:
            i, j = toks.index("-c"), toks.index("-o")
        except ValueError:
            continue
        flags[toks[j + 2]] = " ".join(compiler_flags(toks[i + 1:j]))
    return flags


def load_excludes(root):
    p = root / EXCLUDE_FILE
    if not p.is_file():
        return set()
    return {l.strip() for l in p.read_text().splitlines()
            if l.strip() and not l.startswith("#")}


def build_config(root, base_dir="build", flags=None, make="gmake", apply_excludes=False):
    y = load_yaml(root / "mickey.us.yaml")
    atlas_file = root / "config" / "overlays.us.json"
    atlas = json.loads(atlas_file.read_text()) if atlas_file.is_file() else None
    c_src, others, orphans, problems = collect(root, y, atlas)
    if flags is None:
        flags = compile_flags(root, c_src, make)
    excludes = load_excludes(root) if apply_excludes else set()
    units = []
    used = set()
    for path in sorted(set(c_src) | set(others)):
        rel = f"{path}.o"
        if rel in excludes:
            continue
        is_c = path in set(c_src)
        cats = categories_for(path)
        used.update(cats)
        meta = {"progress_categories": cats}
        unit = {"name": path, "target_path": f"expected/build/{rel}"}
        if is_c:
            unit["base_path"] = f"{base_dir}/{rel}"
            meta["source_path"] = path
            text = (root / path).read_text(errors="replace") if (root / path).is_file() else ""
            meta["complete"] = not GLOBAL_ASM_RE.search(text)
            if path in flags:
                unit["scratch"] = {"platform": "n64", "compiler": SCRATCH_COMPILER,
                                   "c_flags": flags[path]}
        unit["metadata"] = meta
        units.append(unit)
    cats = [c for c in sorted(used, key=lambda c: (c.startswith("overlay-"), c))]
    known = dict(CATEGORIES)
    config = {
        "custom_make": "gmake",
        "build_target": False,
        "build_base": False,
        "progress_categories": [{"id": c, "name": known.get(c, c)} for c in cats],
        "units": units,
    }
    return config, orphans, problems


def render(config):
    return json.dumps(config, indent=2) + "\n"


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--stdout", action="store_true")
    ap.add_argument("--verify-built", action="store_true")
    ap.add_argument("--apply-excludes", action="store_true",
                    help="drop objects listed in tools/objdiff_exclude.txt (a local, "
                    "gitignored list objdiff_report.sh maintains); never used for "
                    "the committed objdiff.json")
    ap.add_argument("--base-dir", default="build")
    ap.add_argument("--root", default=str(ROOT))
    a = ap.parse_args(argv)
    root = pathlib.Path(a.root)
    base_dir = a.base_dir.strip("/")
    config, orphans, problems = build_config(root, base_dir, apply_excludes=a.apply_excludes)
    text = render(config)
    status = 0
    for p in problems:
        print(f"objdiff-config: {p}", file=sys.stderr)
        status = 1
    if a.verify_built:
        missing = [u["base_path"] for u in config["units"]
                   if "base_path" in u and not (root / u["base_path"]).is_file()]
        for m in missing:
            print(f"objdiff-config: unit object not built: {m}", file=sys.stderr)
        status = status or (1 if missing else 0)
    out = root / "objdiff.json"
    if a.check:
        cur = out.read_text() if out.is_file() else ""
        if cur != text:
            diff = list(difflib.unified_diff(cur.splitlines(), text.splitlines(),
                                             "objdiff.json (committed)",
                                             "objdiff.json (derived)", lineterm="", n=0))
            print("\n".join(diff[:40]), file=sys.stderr)
            print(f"objdiff-config: objdiff.json is stale ({len(diff)} diff lines); "
                  "run `gmake objdiff-config`", file=sys.stderr)
            return 1
        print(f"objdiff.json up to date ({len(config['units'])} units"
              f", {len(orphans)} orphan sources skipped)")
        return status
    if a.stdout:
        sys.stdout.write(text)
        return status
    out.write_text(text)
    print(f"wrote objdiff.json: {len(config['units'])} units, "
          f"{len(orphans)} orphan sources skipped", file=sys.stderr)
    return status


if __name__ == "__main__":
    raise SystemExit(main())
