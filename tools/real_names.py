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
  leave its declaration behind; `normalization` -- a spec file this module's
  rules name is also named by another module's rule; `normalization (recipe)`
  -- one of the module's own rules spells the placeholder and already spells
  the real name.  A spec file no rule names takes no part in the build and
  does not count.

The rewrite is purely a spelling change in C.  The module's own POSTPROCESS
rules, and the spec files they name, are respelled with it -- `--redefine-sym
<old>=`, a rebind's or filter's asserted name -- so the normal build's objects
stay as they were; a rename's new name (`<real>=<placeholder>`, which the rule
produces from another spelling) is left alone; any other rename the new name needs is
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


def rewrite_text(text, mapping, dedupe=True):
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
    text = "".join(out)
    return dedupe_declarations(text, set(mapping.values())) if dedupe else text


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


RULE_START_RE = re.compile(r"^(\S+\.o|\$\([A-Za-z0-9_]+\)):\s+POSTPROCESS\s*[:+]?=")
OBJ_VAR_RE = re.compile(r"^([A-Za-z0-9_]+)\s*:=\s*(?:\\\n\s*)?(\S+\.o)\s*$", re.M)
REDEFINE_RE = re.compile(r"(--redefine-sym\s+)([A-Za-z_]\w*)=")


def _rule_blocks(lines):
    """Yield (start, end, target) for each POSTPROCESS rule's line range.

    A rule written against a one-object variable (`O28_MERGED_OBJ := <obj>`,
    defined in the same file) yields that object as its target.
    """
    objs = {m.group(1): m.group(2)
            for m in OBJ_VAR_RE.finditer("".join(l for l in lines if l is not None))}
    i = 0
    while i < len(lines):
        m = RULE_START_RE.match(lines[i] or "")
        if not m:
            i += 1
            continue
        j = i
        while lines[j].rstrip("\n").endswith("\\") and j + 1 < len(lines):
            j += 1
        target = m.group(1)
        if target.startswith("$("):
            target = objs.get(target[2:-1], target)
        yield i, j + 1, target
        i = j + 1


SPLIT_RENAME_RE = re.compile(r"--redefine-sym \\\n\s*([A-Za-z_]\w*)=")


def join_split_renames(text, overlay, mapping):
    """Join `--redefine-sym \\` + `<old>=<new>` onto one line for a renamed old.

    Some recipes wrap each rename after the option; respelling (or dropping)
    the entry needs the option and its argument on one line.
    """
    prefix = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o%03d/" % overlay
    lines = text.splitlines(keepends=True)
    for a, b, target in list(_rule_blocks(lines)):
        if not target.startswith(prefix):
            continue
        rule_map = mapping(target.replace("$(SRC_DIR)", "src")) if callable(mapping) else mapping
        for k in range(a, b - 1):
            if lines[k] is None or not lines[k].rstrip("\n").endswith("--redefine-sym \\"):
                continue
            m = re.match(r"\s*([A-Za-z_]\w*)=", lines[k + 1] or "")
            if m and m.group(1) in rule_map:
                lines[k] = lines[k].rstrip("\n")[:-1] + lines[k + 1].lstrip()
                lines[k + 1] = None
    return "".join(l for l in lines if l is not None)


def rewrite_mk(text, overlay, mapping):
    """Respell `--redefine-sym <old>=` in module `overlay`'s POSTPROCESS rules.

    The normal build's object then carries exactly what it carried before.  A
    respelled entry whose new name the same rule already renames is dropped
    instead: objcopy refuses two renames of one name in one command, and a
    second command's rename of an already-renamed name is inert.
    """
    prefix = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o%03d/" % overlay
    text = join_split_renames(text, overlay, mapping)
    lines = text.splitlines(keepends=True)
    drop = set()
    for a, b, target in list(_rule_blocks(lines)):
        if not target.startswith(prefix):
            continue
        rule_map = mapping(target.replace("$(SRC_DIR)", "src")) if callable(mapping) else mapping
        olds = {mm.group(2) for k in range(a, b) for mm in REDEFINE_RE.finditer(lines[k])}
        claimed = set(olds) - set(rule_map)
        for k in range(a, b):
            def sub(mm, rule_map=rule_map):
                old = mm.group(2)
                if old not in rule_map:
                    return mm.group(0)
                new = rule_map[old]
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
            # A rebind's or filter's spelling is respelled too; a rename's
            # new name (`<old>=<name>`) is what the rule produces and stays.
            lines[k] = respell_rest(line, rule_map)
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


def plan_module(root, overlay, placeholders, files, texts, inc, normalization_texts=None,
                mk_texts=()):
    """({placeholder: real}, [(placeholder, reason)]) for one module.

    ``mk_texts`` are the recipe files' texts.  Only this module's own
    POSTPROCESS rules, and the spec files they name, can make a rename
    unsafe: every spelling of a placeholder there is respelled with the C
    (`rewrite_mk`, `rewrite_spec`) except a rename's *new* name, which is a
    spelling another object's rule produces and stays.  A spec another
    module's rule also names, or a rule that already spells the real name,
    refuses the placeholder.  ``normalization_texts`` is unused (a spec no
    rule names does not take part in the build).
    """
    own = module_files(root, overlay, files, texts, inc)
    visible = sorted(h for h, users in inc.items() if h not in own and users & own)
    rules = module_rules(mk_texts, overlay)
    specs, foreign = rule_specs(root, mk_texts, overlay)
    mapping, refusals = {}, []
    for p in placeholders:
        if p.overlay != overlay:
            continue
        if p.cls not in CONVERTIBLE:
            refusals.append((p.name, p.cls))
            continue
        word = re.compile(r"\b%s\b" % re.escape(p.name))
        # Only a header this module's sources include can carry a declaration
        # the per-module rename would leave behind; another module's own
        # spelling of the same placeholder is that module's business.
        outside = [f for f in visible if word.search(texts[f])]
        if outside:
            refusals.append((p.name, "shared (%s)" % outside[0].relative_to(root).as_posix()))
            continue
        if any(word.search(specs[s]) for s in foreign):
            refusals.append((p.name, "normalization"))
            continue
        real = re.compile(r"\b%s\b" % re.escape(p.real))
        clash = False
        for block, names in rules:
            body = block + "".join(specs.get(n, "") for n in names)
            if recipe_uses(body, p.name) and real.search(body):
                clash = True
                break
        if clash:
            refusals.append((p.name, "normalization (recipe)"))
            continue
        mapping[p.name] = p.real
    return mapping, refusals


SPEC_REF_RE = re.compile(r"@?(config/normalizations/[A-Za-z0-9_.\-]+)")


def recipe_uses(text, name):
    """Spellings of ``name`` a rename must respell (all but `<old>=<name>`)."""
    return len(re.findall(r"(?<![=\w])%s\b" % re.escape(name), text))


def module_rules(mk_texts, overlay):
    """[(rule text, [spec paths it names])] for module ``overlay``'s POSTPROCESS rules."""
    prefix = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o%03d/" % overlay
    out = []
    for text in mk_texts:
        lines = text.splitlines(keepends=True)
        for a, b, target in _rule_blocks(lines):
            if target.startswith(prefix):
                block = "".join(lines[a:b])
                out.append((block, SPEC_REF_RE.findall(block)))
    return out


def rule_specs(root, mk_texts, overlay):
    """({spec path: text} this module's rules name, {those another module's rules name})."""
    mine = {n for _b, names in module_rules(mk_texts, overlay) for n in names}
    specs = {}
    for n in sorted(mine):
        f = Path(root) / n
        if f.is_file() and f.suffix != ".py":
            specs[n] = f.read_text(errors="replace")
    foreign = set()
    for text in mk_texts:
        lines = text.splitlines(keepends=True)
        for a, b, target in _rule_blocks(lines):
            if "/overlays/o%03d/" % overlay in target:
                continue
            foreign |= set(SPEC_REF_RE.findall("".join(lines[a:b]))) & set(specs)
    return specs, foreign


def respell_rest(text, mapping):
    """Respell every remaining spelling of a mapped name but `<old>=<name>`."""
    for old, new in mapping.items():
        text = re.sub(r"(?<![=\w])%s\b" % re.escape(old), new, text)
    return text


def rewrite_spec(text, mapping):
    """Respell a normalization spec's names (spec files carry no renames)."""
    return respell_rest(text, mapping)


def mk_files(root):
    return ([root / "Makefile"] + sorted((root / "mk").glob("*.mk"))
            + sorted((root / "config" / "normalizations").glob("*.mk")))


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


PREVIOUS_RE = re.compile(r"previous declaration at line (\d+) in file '([^']+)'")


def declares(text, name):
    """Whether ``text`` carries an `extern` declaration of ``name``."""
    return bool(re.search(r"\bextern\b[^;{}]*\b%s\b[^;{}]*;" % re.escape(name), text))


def apply_module(root, overlay, mapping, files, texts, inc, compile=True):
    """Rewrite one module; return (changed paths, refusals, renamed mapping).

    With ``compile``, the module is compiled after the rewrite.  A real name
    the rewrite left declared with two different types in one translation
    unit -- two placeholders that spelled one callee with different
    prototypes, which their call sites' codegen depends on -- is refused
    where it conflicts: in that source file when the file declares the
    placeholder itself, otherwise in the whole module.  The module is then
    rewritten again from its original text, until it compiles.
    """
    own = sorted(module_files(root, overlay, files, texts, inc))
    originals = {f: texts[f] for f in own}
    mks = [f for f in mk_files(root) if not f.name.endswith(".generated.mk")]
    mk_originals = {f: f.read_text() for f in mks}
    # Spec files this module's rules name, each with the object whose rule names it.
    spec_owner = {}
    prefix = "$(BUILD_DIR)/$(SRC_DIR)/overlays/o%03d/" % overlay
    for text in mk_originals.values():
        lines = text.splitlines(keepends=True)
        for a, b, target in _rule_blocks(lines):
            if target.startswith(prefix):
                for n in SPEC_REF_RE.findall("".join(lines[a:b])):
                    f = root / n
                    if f.is_file() and f.suffix != ".py":
                        spec_owner.setdefault(f, target)
    spec_originals = {f: f.read_text() for f in spec_owner}
    sources = [f for f in own if f.suffix == ".c"]
    dropped_all, dropped = set(), collections.defaultdict(set)
    reasons = {}

    def file_map(f):
        return {o: n for o, n in mapping.items() if o not in dropped_all and o not in dropped[f]}

    def object_map(target):
        src = root / (target[len("$(BUILD_DIR)/"):-len(".o")])
        return file_map(src) if src in originals else {}

    unexplained = []
    while True:
        changed = []
        for f, old in originals.items():
            new = rewrite_text(old, file_map(f), dedupe=False)
            if f.read_text() != new:
                f.write_text(new)
            if new != old:
                changed.append(f)
        for f, old in mk_originals.items():
            new = rewrite_mk(old, overlay, object_map)
            if f.read_text() != new:
                f.write_text(new)
            if new != old:
                changed.append(f)
        for f, old in spec_originals.items():
            new = rewrite_spec(old, object_map(spec_owner[f].replace("$(SRC_DIR)", "src")))
            if f.read_text() != new:
                f.write_text(new)
            if new != old:
                changed.append(f)
        if not compile:
            break
        _names, located, rest = compile_module(root, overlay, sources)
        if rest:
            unexplained = rest
            break
        unexplained, progress = [], False
        for path, line, msg in located:
            spots = [(root / path, line)]
            m = PREVIOUS_RE.search(msg)
            if m:
                spots.append((root / m.group(2), int(m.group(1))))
            hit = False
            for f, ln in spots:
                if f not in originals:
                    continue
                lines = originals[f].splitlines()
                here = set(IDENT_RE.findall(lines[ln - 1])) if 0 < ln <= len(lines) else set()
                for o in sorted(here & set(file_map(f))):
                    hit = progress = True
                    reasons[o] = "prototype-conflict (%s)" % mapping[o]
                    if f.suffix == ".c" and declares(originals[f], o):
                        dropped[f].add(o)
                    else:
                        dropped_all.add(o)
                if hit:
                    break
            if not hit:
                unexplained.append("%s:%d: %s" % (path, line, msg))
        if not located or not progress:
            break
    if compile and unexplained:
        # Not a prototype conflict: put the module back as it was and refuse it.
        for f, old in (list(originals.items()) + list(mk_originals.items())
                       + list(spec_originals.items())):
            if f.read_text() != old:
                f.write_text(old)
        refused = [(o, "module does not compile: %s" % unexplained[0][:160])
                   for o in sorted(mapping)]
        for f in own:
            texts[f] = f.read_text()
        return [], refused, {}
    # The collapse of identical declarations is cosmetic; it runs last so the
    # compile loop's line numbers stay those of the original text.
    for f in originals:
        text = f.read_text()
        new = dedupe_declarations(text, set(file_map(f).values()))
        if new != text:
            f.write_text(new)
        texts[f] = f.read_text()
    renamed = {o: n for o, n in mapping.items() if o not in dropped_all}
    refused = [(o, reasons[o] + (" in %d file(s)" % sum(o in d for d in dropped.values())
                                 if o not in dropped_all else ""))
               for o in sorted(reasons)]
    return changed, refused, renamed


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
