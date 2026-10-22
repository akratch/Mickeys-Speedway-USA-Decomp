#!/usr/bin/env python3
"""Rewrite overlay C placeholders to the real names of their targets.

Overlay C used to reach most relocation targets through placeholder externs
(`overlay11CreateReloc`, `gO57Descriptor00Reloc`, `D_1D68`) whose only meaning
was the value `overlay_undefined_syms.us.txt` gave them.  The runlink packer
(`tools/runlink_pack.py`) cannot resolve such a name from the objects, so it
borrows the identity from the shipped tables (docs/overlays.md section 5.3.1).
This tool gives each placeholder the name of the symbol it actually stands
for, so the packer reads the identity from the object instead.

The identity of a placeholder is measured, never guessed: at every site the
compiled raw object references it, the shipped record's target minus the
object's own addend is the symbol the C means (the same subtraction
`reloc_surface.synthesize` performs).  A placeholder is converted only when

* every one of its sites in the module agrees on that target, and
* a real symbol is defined there: a resident function or data symbol, an
  overlay function, or overlay data a C object defines.

Everything else is refused and counted, and nothing is renamed for it:

* `blob-data` / `blob-bss` -- the target is overlay data or BSS still owned by
  a ROM blob, which no object defines.  These wait for phase 3 (data into C);
  they are deliberately *not* renamed to generated names.
* `unnamed` -- a resident or overlay address no symbol starts at (a field in
  the middle of an object), `static` -- a target another file defines
  `static`, `disagree` -- sites naming different targets, `no-record` -- no
  shipped record names the site.
* `export-offset` -- a `SYMBOL` record whose export lies before the target
  (the C carries the field offset in the name, not in the instruction), so
  the real spelling needs an expression change, not a rename.
* `shared` -- the placeholder is also spelled in a file outside the module
  (a shared header, another module's source), so a per-module rename would
  leave its declaration behind; `normalization` -- a normalization spec names
  it.

The rewrite is purely a spelling change in C.  `--redefine-sym <old>=` steps in
the module's own POSTPROCESS rules are respelled `<new>=` so the normal
build's objects stay as they were; any other rename the new name needs is
planned by `reloc_surface.py generate` (`<name>_oNNNReloc`, docs/reloc-surface.md
section 5.7).  The tool cannot prove the ROM is unchanged -- `gmake verify`
does that -- but it never edits anything but identifiers.

    tools/real_names.py report                 # census by class and kind
    tools/real_names.py plan --module 1 61     # what would change, per module
    tools/real_names.py apply --module 1 61    # rewrite the sources

Needs the baserom, a linked build and `gmake runlink-raw`, like the packer.
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

REPO = Path(__file__).resolve().parent.parent
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
GENERIC_DATA_RE = re.compile(r"^D_[0-9A-Fa-f]{1,5}$")
INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)
CONVERTIBLE = ("resident-func", "resident-data", "overlay-func", "overlay-data")


# ---------------------------------------------------------------- measuring

@dataclass
class Placeholder:
    overlay: int
    name: str
    sites: int
    cls: str
    real: str | None = None
    kind: str = ""
    sources: list = field(default_factory=list)


def placeholder_kind(name):
    """The borrowed-identity kinds docs/overlays.md counts."""
    if name.endswith("Reloc"):
        return "Reloc"
    if GENERIC_DATA_RE.match(name):
        return "D_short"
    if name.startswith("ext_o"):
        return "ext"
    if re.match(r"^D_8[0-9A-F]{7}$", name):
        return "D_8"
    return "friendly"


def classify_target(base, ov, tables):
    """(class, real name) for one placeholder's agreed target identity.

    ``tables`` supplies ``overlay_defs`` (identity -> [names] the raw overlay
    objects define globally), ``overlay_statics`` (identity -> [names] defined
    static), ``resident`` (address -> [names] the resident objects define,
    best first) and ``module_info`` (overlay -> text_size/prefix).
    """
    if base is None:
        return "no-record", None
    if base[0] == "res":
        names = tables["resident"].get(base[1], [])
        if not names:
            return "unnamed", None
        return ("resident-func" if names[0][1] else "resident-data"), names[0][0]
    _, m, off = base
    info = tables["module_info"][m]
    names = tables["overlay_defs"].get(base, [])
    if names:
        return ("overlay-func" if off < info["text_size"] else "overlay-data"), names[0]
    if tables["overlay_statics"].get(base):
        return "static", None
    if off < info["text_size"]:
        return "unnamed", None
    return ("blob-bss" if off >= info["prefix"] else "blob-data"), None


def classify_groups(groups, tables):
    """[Placeholder] from {(overlay, name): [(base identity | None, source, op)]}.

    ``tables["definers"]`` maps a real overlay name to the sources defining
    it.  A `SYMBOL` site in the object that defines its target is refused as
    `self-export`: the normal build must store the export's addend there,
    and the only per-object tool, an objcopy rename, would rename the
    definition with the reference.
    """
    out = []
    for (ov, name), sites in sorted(groups.items()):
        bases = {site[0] for site in sites if site[0] is not None}
        if len(bases) > 1:
            cls, real = "disagree", None
        else:
            cls, real = classify_target(next(iter(bases)) if bases else None, ov, tables)
        if cls in CONVERTIBLE and any(op == "SYMBOL-offset" for _b, _s, op in sites):
            # The record names an export plus an offset the object does not
            # carry: the C names a field, not the export.  Renaming it to
            # whatever starts at the field would name an unexported symbol.
            cls = "export-offset"
        if cls.startswith("overlay-"):
            definers = tables.get("definers", {}).get(real, set())
            if any(src in definers and op == "SYMBOL" for _b, src, op in sites):
                cls = "self-export"
        out.append(Placeholder(ov, name, len(sites), cls, real, placeholder_kind(name),
                               sorted({site[1] for site in sites})))
    return out


def measure(args):
    """Placeholder census of the whole tree, from the packer's own model."""
    import runlink_pack as rp
    ns = argparse.Namespace(root=args.root, **{k: None for k in rp.DEFAULTS})
    _paths, world, packed = rp.load_and_pack(ns)
    overlay_defs = collections.defaultdict(list)
    for name, lst in world.defs().items():
        for o, s in lst:
            base = rp.section_offset(o, s.section)
            if base is not None:
                overlay_defs[("ov", o.module, base + s.value)].append(name)
    for k in overlay_defs:
        overlay_defs[k].sort(key=lambda n: (rp.is_generated(n), n))
    statics = collections.defaultdict(list)
    for ov in world.amods:
        for o in world.objects(ov):
            for s in o.elf.symbols:
                if (s.name and s.bind == rp.STB_LOCAL and s.shndx != rp.SHN_UNDEF
                        and s.shndx < 0xFF00 and s.type != rp.STT_SECTION):
                    base = rp.section_offset(o, s.section)
                    if base is not None:
                        statics[("ov", o.module, base + s.value)].append(s.name)
    resident = collections.defaultdict(list)
    for name, syms in world.by_name.items():
        if rp.LINKER_NAME_RE.search(name):
            continue
        for s in syms:
            if s.section in rp.RESIDENT_SECTIONS and world.layout.contains(s.value):
                resident[s.value].append(s)
    resident = {a: [(s.name, s.type == rp.STT_FUNC) for s in rp.best_name(v)]
                for a, v in resident.items()}
    groups = collections.defaultdict(list)
    for ov, relocs in packed.relocs.items():
        exp = world.expected_cache(ov)
        for c in relocs:
            if c.target.kind != "placeholder":
                continue
            site = c.site()
            e = exp.get((site, c.type)) if site is not None else None
            base = rp.ident_add(e["full"], -c.addend) if e and e["full"] else None
            op = rp.OP_NAMES.get(e["op"]) if e else None
            if op == "SYMBOL" and base is not None:
                row = world.shipped.ort[e["symbol_index"]]
                if base != world.layout.row_identity(row["overlay"], row["offset"]):
                    op = "SYMBOL-offset"
            groups[(ov, c.target.name)].append((base, c.obj.source, op))
    definers = {name: {o.source for o, _s in lst} for name, lst in world.defs().items()}
    tables = {"overlay_defs": overlay_defs, "overlay_statics": statics,
              "resident": resident, "module_info": world.module_info,
              "definers": definers}
    return classify_groups(groups, tables)


# ---------------------------------------------------------------- rewriting

def _code_spans(text):
    """[(start, end, kind)] covering text: 'code', 'comment' or 'string'."""
    spans, i, n, start = [], 0, len(text), 0
    while i < n:
        c = text[i]
        if c == "/" and text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif c == "/" and text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
        else:
            i += 1
            continue
        if start < i:
            spans.append((start, i, "code"))
        spans.append((i, j, "string" if c in "\"'" else "comment"))
        i = start = j
    if start < n:
        spans.append((start, n, "code"))
    return spans


def rewrite_text(text, mapping):
    """Rename identifiers in C text; comments too, except for short `D_` names.

    String literals are never touched.  A short `D_<hex>` spelling is too
    generic to rename inside prose, so a comment keeps it.
    """
    out = []
    for a, b, kind in _code_spans(text):
        chunk = text[a:b]
        if kind != "string":
            def sub(m, kind=kind):
                new = mapping.get(m.group(0))
                if new is None or (kind == "comment" and GENERIC_DATA_RE.match(m.group(0))):
                    return m.group(0)
                return new
            chunk = IDENT_RE.sub(sub, chunk)
        out.append(chunk)
    return dedupe_declarations("".join(out), set(mapping.values()))


def dedupe_declarations(text, names):
    """Drop a repeated identical `extern` line naming one of ``names``.

    Two placeholders that stood for one symbol become two identical
    declarations after the rename; only those are collapsed.
    """
    seen, out = set(), []
    for line in text.splitlines(keepends=True):
        key = line.strip()
        if (key.startswith("extern ") and key.endswith(";")
                and names.intersection(IDENT_RE.findall(key))):
            if key in seen:
                continue
            seen.add(key)
        out.append(line)
    return "".join(out)


RULE_START_RE = re.compile(r"^(\S+\.o):\s+POSTPROCESS\s*[:+]?=")
REDEFINE_RE = re.compile(r"(--redefine-sym\s+)([A-Za-z_]\w*)=")


def _rule_blocks(lines):
    """Yield (start, end, target) for each POSTPROCESS rule's line range."""
    i = 0
    while i < len(lines):
        m = RULE_START_RE.match(lines[i])
        if not m:
            i += 1
            continue
        j = i
        while lines[j].rstrip("\n").endswith("\\") and j + 1 < len(lines):
            j += 1
        yield i, j + 1, m.group(1)
        i = j + 1


def rewrite_mk(text, overlay, mapping):
    """Respell `--redefine-sym <old>=` in module `overlay`'s POSTPROCESS rules.

    The normal build's object then carries exactly what it carried before.  A
    respelled entry whose new name the same rule already renames is dropped
    instead: objcopy refuses two renames of one name in one command, and a
    second command's rename of an already-renamed name is inert.
    """
    prefix = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o%03d/" % overlay
    lines = text.splitlines(keepends=True)
    drop = set()
    for a, b, target in list(_rule_blocks(lines)):
        if not target.startswith(prefix):
            continue
        olds = {mm.group(2) for k in range(a, b) for mm in REDEFINE_RE.finditer(lines[k])}
        claimed = set(olds) - set(mapping)
        for k in range(a, b):
            def sub(mm):
                old = mm.group(2)
                if old not in mapping:
                    return mm.group(0)
                new = mapping[old]
                if new in claimed:
                    return "\0DROP\0"
                claimed.add(new)
                return mm.group(1) + new + "="
            line = REDEFINE_RE.sub(sub, lines[k])
            if "\0DROP\0" in line:
                line = re.sub(r"\0DROP\0[A-Za-z_]\w*[ \t]*", "", line)
                body = line.strip()
                if body in ("\\", ""):
                    drop.add(k)
                    continue
                if re.fullmatch(r"\$\(OBJCOPY\)\s+\$@ && \\", body):
                    drop.add(k)
                    continue
            lines[k] = line
    return "".join(l for k, l in enumerate(lines) if k not in drop)


def tree_files(root):
    """Every tracked-source file the rename may need to look at."""
    files = sorted((root / "src").rglob("*.c")) + sorted((root / "src").rglob("*.h"))
    files += sorted((root / "include").rglob("*.h"))
    return files


def includers(root, files, texts):
    """{header path: {files that include it, transitively}}."""
    by_base = collections.defaultdict(list)
    for f in files:
        if f.suffix == ".h":
            by_base[f.name].append(f)
    direct = collections.defaultdict(set)
    for f in files:
        for inc in INCLUDE_RE.findall(texts[f]):
            for h in by_base.get(Path(inc).name, []):
                direct[h].add(f)
    out = {}
    for h in direct:
        seen, todo = set(), [h]
        while todo:
            x = todo.pop()
            for y in direct.get(x, ()):
                if y not in seen:
                    seen.add(y)
                    todo.append(y)
        out[h] = seen
    return out


def module_files(root, overlay, files, texts, inc):
    """The files one module owns: its sources plus headers only it includes."""
    mdir = root / "src" / "overlays" / ("o%03d" % overlay)
    own = {f for f in files if mdir in f.parents}
    for h, users in inc.items():
        if h in own:
            continue
        srcs = {u for u in users if u.suffix == ".c"}
        if srcs and all(mdir in u.parents for u in srcs):
            own.add(h)
    return own


def plan_module(root, overlay, placeholders, files, texts, inc, normalization_texts,
                mk_texts=()):
    """({placeholder: real}, [(placeholder, reason)]) for one module."""
    own = module_files(root, overlay, files, texts, inc)
    mapping, refusals = {}, []
    for p in placeholders:
        if p.overlay != overlay:
            continue
        if p.cls not in CONVERTIBLE:
            refusals.append((p.name, p.cls))
            continue
        word = re.compile(r"\b%s\b" % re.escape(p.name))
        outside = [f for f in files if f not in own and word.search(texts[f])]
        if outside:
            refusals.append((p.name, "shared (%s)" % outside[0].relative_to(root).as_posix()))
            continue
        if any(word.search(t) for t in normalization_texts):
            refusals.append((p.name, "normalization"))
            continue
        # A recipe that names the placeholder other than as a rename's old
        # name (a relocation rebind, an added symbol) would stop matching.
        if any(len(word.findall(t)) > len(re.findall(r"--redefine-sym\s+%s=" % re.escape(p.name), t))
               for t in mk_texts):
            refusals.append((p.name, "normalization (recipe)"))
            continue
        mapping[p.name] = p.real
    return mapping, refusals


def mk_files(root):
    return [root / "Makefile"] + sorted((root / "mk").glob("*.mk"))


REDECL_RE = re.compile(r"redeclaration of '([A-Za-z_]\w*)'")
CFE_ERROR_RE = re.compile(r"cfe: Error: (\S+), line (\d+): (.*)")


def compile_module(root, overlay, sources):
    """(redeclared names, [(file, line, message)], unparsed output) for a module."""
    import subprocess
    objs = ["build/%s.o" % f.relative_to(root).as_posix() for f in sources]
    proc = subprocess.run(["gmake", "-k", "-j8", "--no-print-directory"] + objs, cwd=root,
                          capture_output=True, text=True)
    out = proc.stdout + proc.stderr
    names = set(REDECL_RE.findall(out))
    located = [(m.group(1), int(m.group(2)), m.group(3)) for m in CFE_ERROR_RE.finditer(out)]
    rest = [] if not proc.returncode or located else out.splitlines()[-5:]
    return names, located, rest


def apply_module(root, overlay, mapping, files, texts, inc, compile=True):
    """Rewrite one module; return (changed paths, [(placeholder, reason)]).

    With ``compile``, the module is compiled after the rewrite.  A real name
    the rewrite left declared with two different types in one translation
    unit (two placeholders that spelled one callee with different
    prototypes, which their call sites' codegen depends on) is a refusal:
    every placeholder for it is restored and the module rewritten again.
    """
    mapping = dict(mapping)
    own = sorted(module_files(root, overlay, files, texts, inc))
    originals = {f: texts[f] for f in own}
    for f in mk_files(root):
        if not f.name.endswith(".generated.mk"):
            originals[f] = f.read_text()
    refused = []
    sources = [f for f in own if f.suffix == ".c"]
    unexplained = []
    while True:
        changed = []
        for f, old in originals.items():
            if f.suffix in (".c", ".h"):
                new = rewrite_text(old, mapping)
            else:
                new = rewrite_mk(old, overlay, mapping)
            if f.read_text() != new:
                f.write_text(new)
            if new != old:
                changed.append(f)
        if not compile:
            break
        names, located, rest = compile_module(root, overlay, sources)
        if rest:
            unexplained = rest
            bad = set()
            break
        new_names = set(mapping.values())
        bad = names & new_names
        unexplained = []
        for path, line, msg in located:
            if REDECL_RE.search(msg) and REDECL_RE.search(msg).group(1) in bad:
                continue
            f = root / path
            text = f.read_text().splitlines() if f.is_file() else []
            here = set(IDENT_RE.findall(text[line - 1])) if 0 < line <= len(text) else set()
            hit = here & new_names
            if hit:
                bad |= hit
            else:
                unexplained.append("%s:%d: %s" % (path, line, msg))
        if not bad:
            break
        for old in sorted(mapping):
            if mapping[old] in bad:
                refused.append((old, "prototype-conflict (%s)" % mapping[old]))
                del mapping[old]
    if compile and unexplained:
        # Not a prototype conflict: put the module back as it was and refuse it.
        for f, old in originals.items():
            if f.read_text() != old:
                f.write_text(old)
        refused += [(old, "module does not compile: %s" % unexplained[0][:160])
                    for old in sorted(mapping)]
        mapping, changed = {}, []
    for f in own:
        texts[f] = f.read_text()
    return changed, refused, mapping


# ---------------------------------------------------------------- commands

def report(placeholders):
    by_cls = collections.Counter()
    syms = collections.Counter()
    by_kind = collections.Counter()
    for p in placeholders:
        by_cls[p.cls] += p.sites
        syms[p.cls] += 1
        by_kind[(p.kind, "convertible" if p.cls in CONVERTIBLE else p.cls)] += p.sites
    total = sum(by_cls.values())
    print("placeholder records %d (%d symbols)" % (total, len(placeholders)))
    for k, v in by_cls.most_common():
        print("  %-14s %6d records %5d symbols" % (k, v, syms[k]))
    print("by kind:")
    for (k, c), v in sorted(by_kind.items()):
        print("  %-9s %-12s %6d" % (k, c, v))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("command", choices=("report", "plan", "apply"))
    ap.add_argument("--module", type=int, nargs="*", default=[])
    ap.add_argument("--root", default=None)
    ap.add_argument("--no-compile", action="store_true",
                    help="apply: rewrite only; do not compile to refuse prototype conflicts")
    args = ap.parse_args(argv)
    root = Path(args.root).resolve() if args.root else REPO
    placeholders = measure(args)
    if args.command == "report":
        report(placeholders)
        return 0
    if not args.module:
        ap.error("plan/apply need --module")
    files = tree_files(root)
    texts = {f: f.read_text(errors="replace") for f in files}
    inc = includers(root, files, texts)
    norm = [p.read_text(errors="replace")
            for p in sorted((root / "config" / "normalizations").glob("*")) if p.is_file()]
    total_conv = total_ref = 0
    for ov in args.module:
        mapping, refusals = plan_module(root, ov, placeholders, files, texts, inc, norm,
                                        [f.read_text() for f in mk_files(root)])
        sites = {p.name: p.sites for p in placeholders if p.overlay == ov}
        conv = sum(sites[n] for n in mapping)
        ref = collections.Counter()
        for n, why in refusals:
            ref[why.split(" ")[0]] += sites[n]
        total_conv += conv
        total_ref += sum(ref.values())
        print("overlay %d: %d placeholder(s), %d record(s) renamed; refused %s"
              % (ov, len(mapping), conv,
                 ", ".join("%s %d" % kv for kv in sorted(ref.items())) or "none"))
        if args.command == "plan":
            for old in sorted(mapping):
                print("  %s -> %s" % (old, mapping[old]))
            for n, why in refusals:
                if not why.startswith(("blob", )):
                    print("  refused %s: %s" % (n, why))
        else:
            changed, refused, kept = apply_module(root, ov, mapping, files, texts, inc,
                                                  compile=not args.no_compile)
            for n, why in refused:
                print("  refused %s: %s" % (n, why))
                total_conv -= sites[n]
                total_ref += sites[n]
            print("  overlay %d: %d placeholder(s) renamed after compiling"
                  % (ov, len(kept)))
            for f in changed:
                print("  rewrote", f.relative_to(root).as_posix())
    print("total: %d record(s) renamed, %d refused" % (total_conv, total_ref))
    return 0


if __name__ == "__main__":
    sys.exit(main())
