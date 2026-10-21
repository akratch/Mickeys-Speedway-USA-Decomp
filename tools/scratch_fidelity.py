#!/usr/bin/env python3
"""Permuter scratch fidelity: restore what the importer drops, then prove the rest.

    tools/scratch_fidelity.py import [import.py args...]
    tools/scratch_fidelity.py check <symbol> <scratch-dir> --source <tu.c> [--json PATH]
    tools/scratch_fidelity.py compare <symbol> <scratch.o> <tree.o>

A permuter scratch (`base.c` + `compile.sh`) is a *copy* of the function's
translation unit, re-printed by decomp-permuter's importer. If the copy does not
compile to the same words as the tree does, every permuter score is measured
against the wrong base: a score of 0 does not transfer, and a "floor" is a false
ceiling. On 2026-10-21 the func_800517E0 scratch was one word off the tree for
exactly that reason and nothing said so.

**The cause, and the restoration (`import`).** The importer turns every function
other than the target into a bare declaration (`ast_util.extract_fn`). Two
things are lost on the way:

  - A block-scope prototype inside a stripped body. IDO keeps such a prototype
    in force for the rest of the file (anim.c declares
    `void func_8005055C(u8 pathIndex);` inside func_800511C4, and every later
    call converts its argument to u8). Stripped with the body, the target's
    calls become unprototyped and the u8 conversion disappears.
  - A K&R definition (`void f(a) u8 a; {...}`) becomes `void f(a);`, which cfe
    rejects with Warning 731 and treats as `void f();`.

`import` runs import.py unchanged except for a wrapper around `extract_fn`: a
prototype declared at block scope in a function that precedes the target is
re-applied at file scope, by rewriting the file's unprototyped declarations of
that name into it (a prototype *beside* an unprototyped declaration is a cfe
error, "prototype and non-prototype declaration found"), or by inserting it
before the target when the file declares the name nowhere else. A K&R
identifier list left on a stripped declaration is dropped (`f()`), which is what
cfe already does with it. The target function's own text is never touched, so
a permuter winner transplants back unchanged. Every restoration is printed.

**The proof (`check`).** Compile the scratch once through its own `compile.sh`,
compile the tracked TU through the tree's configured recipe (fast_score's
`gmake -n NON_MATCHING=1` expansion; never a typed cc line), apply any
post-compile objcopy chain `compile.sh` carries to the tree object too, and
compare the symbol's `.text` words and relocation records:

    SCRATCH FIDELITY: identical (func_x: 1808 words, 412 relocation records)
    SCRATCH FIDELITY: 1 words off at +0x6a4 (func_x: 1808 vs 1808 words)

Words are compared with relocation fields masked (a section-relative addend
moves when the importer prunes the TU's other data, and that is not codegen),
relocation records by site, type and target: a named symbol by name, a section
symbol by section name, and a `.text` jump by the function it lands on. Exit
status: 0 identical, 3 different, 2 on a failure to build either side.
`tools/permute.sh` runs `check` before every search and refuses an unfaithful
scratch unless given `--allow-unfaithful`.
"""

from __future__ import annotations

import argparse
import contextlib
import copy
import json
import os
import runpy
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
PERMUTER_DIR = ROOT / "tools" / "permuter"
IMPORT_PY = PERMUTER_DIR / "import.py"

STT_FUNC, STT_SECTION = 2, 3
SHT_REL = 9
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
# Bits a relocation owns in the word it patches. A type not listed owns the
# whole word, which is conservative: the record itself is still compared.
FIELD_MASKS = {R_MIPS_26: 0x03FFFFFF, R_MIPS_HI16: 0x0000FFFF, R_MIPS_LO16: 0x0000FFFF}


# --------------------------------------------------------------- restoration

def _is_prototype(funcdecl, c_ast) -> bool:
    args = funcdecl.args
    return args is not None and not any(isinstance(p, c_ast.ID) for p in args.params)


def _is_unprototyped(funcdecl, c_ast) -> bool:
    args = funcdecl.args
    return args is None or any(isinstance(p, c_ast.ID) for p in args.params)


def block_function_decls(node, c_ast) -> list:
    """Function declarations (Decl of a FuncDecl) anywhere inside `node`."""
    found = []
    stack = [node]
    while stack:
        cur = stack.pop()
        if cur is None:
            continue
        for _name, child in cur.children():
            if isinstance(child, c_ast.Decl) and isinstance(child.type, c_ast.FuncDecl):
                found.append(child)
                continue  # its parameters are not block-scope declarations
            stack.append(child)
    found.sort(key=lambda d: (getattr(d.coord, "line", 0) or 0))
    return found


def leaked_prototypes(ast, fn_name: str, c_ast) -> list[tuple[str, object]]:
    """(enclosing function, Decl) for each block-scope prototype before the target."""
    target = next((i for i, n in enumerate(ast.ext)
                   if isinstance(n, c_ast.FuncDef) and n.decl.name == fn_name), None)
    if target is None:
        return []
    out = []
    for node in ast.ext[:target]:
        if not isinstance(node, c_ast.FuncDef):
            continue
        for decl in block_function_decls(node.body, c_ast):
            if decl.name and _is_prototype(decl.type, c_ast):
                out.append((node.decl.name, decl))
    return out


def knr_names(ast, fn_name: str, c_ast) -> set[str]:
    """Names of non-target K&R definitions (their declarations keep an identifier list)."""
    return {n.decl.name for n in ast.ext
            if isinstance(n, c_ast.FuncDef) and n.decl.name != fn_name and n.param_decls}


def apply_restorations(ast, fn, index: int, leaked, knr: set[str], c_ast) -> tuple[int, list[dict]]:
    """Re-apply leaked prototypes and normalise K&R declarations after extract_fn.

    Returns the target's (possibly shifted) index and one note per action.
    """
    notes: list[dict] = []
    done: set[str] = set()
    for owner, proto in leaked:
        name = proto.name
        if name in done:
            continue
        done.add(name)
        rewritten = 0
        before = False
        for i, node in enumerate(ast.ext):
            if node is fn or not (isinstance(node, c_ast.Decl) and node.name == name
                                  and isinstance(node.type, c_ast.FuncDecl)):
                continue
            if _is_unprototyped(node.type, c_ast):
                node.type = copy.deepcopy(proto.type)
                rewritten += 1
            if i < index:
                before = True
        inserted = False
        if not before:
            new = copy.deepcopy(proto)
            new.storage = [s for s in new.storage if s not in ("auto", "register")]
            ast.ext.insert(index, new)
            index += 1
            inserted = True
        notes.append({"name": name, "from": owner, "rewritten": rewritten, "inserted": inserted,
                      "prototype": _render(proto)})
    for node in ast.ext:
        if (isinstance(node, c_ast.Decl) and node.name in knr - done
                and isinstance(node.type, c_ast.FuncDecl) and node.type.args is not None
                and any(isinstance(p, c_ast.ID) for p in node.type.args.params)):
            node.type.args = None
            notes.append({"name": node.name, "knr": True})
    return index, notes


def _render(decl) -> str:
    try:
        from perm_pycparser import c_generator
        return c_generator.CGenerator().visit(decl).strip() + ";"
    except Exception:  # pragma: no cover - rendering is only for the report
        return decl.name or "?"


def describe(note: dict) -> str:
    if note.get("knr"):
        return (f"scratch fidelity: K&R declaration of {note['name']} printed without its "
                "identifier list (cfe ignores it; Warning 731)")
    how = []
    if note["rewritten"]:
        how.append(f"rewrote {note['rewritten']} unprototyped file-scope declaration(s)")
    if note["inserted"]:
        how.append("inserted before the target")
    return (f"scratch fidelity: restored block-scope prototype `{note['prototype']}` "
            f"(declared inside {note['from']}): " + ", ".join(how))


def restoring_extract_fn(original, c_ast, notes: list[dict]):
    """Wrap ast_util.extract_fn so the stripped bodies' prototypes survive.

    Shared by `import` here and tools/permute_batch.py's grouped importer.
    """
    def extract_fn(ast, fn_name):
        leaked = leaked_prototypes(ast, fn_name, c_ast)
        knr = knr_names(ast, fn_name, c_ast)
        fn, index = original(ast, fn_name)
        index, applied = apply_restorations(ast, fn, index, leaked, knr, c_ast)
        notes.extend(applied)
        return fn, index
    return extract_fn


def report(notes: list[dict], quiet_when_empty: bool = False) -> None:
    seen = set()
    for note in notes:
        line = describe(note)
        if line not in seen:  # import.py may extract more than once
            seen.add(line)
            print(line)
    if not notes and not quiet_when_empty:
        print("scratch fidelity: no block-scope prototypes or K&R declarations to restore")


@contextlib.contextmanager
def restoring(ast_util, c_ast, quiet_when_empty: bool = True):
    """Install the restoring extract_fn for the duration; print what it restored.

    tools/permute_batch.py's grouped importer enters this beside its own parser
    dialect, so both permuter runners import the same scratch.
    """
    original = ast_util.extract_fn
    notes: list[dict] = []
    ast_util.extract_fn = restoring_extract_fn(original, c_ast, notes)
    try:
        yield notes
    finally:
        ast_util.extract_fn = original
    report(notes, quiet_when_empty)


def run_import(argv: list[str]) -> int:
    """Run decomp-permuter's import.py with the restoration wrapper installed."""
    if not IMPORT_PY.is_file():
        print(f"scratch_fidelity: no importer at {IMPORT_PY}", file=sys.stderr)
        return 2
    sys.path.insert(0, str(PERMUTER_DIR))
    from perm_pycparser import c_ast
    from src import ast_util
    saved_argv = sys.argv
    try:
        with restoring(ast_util, c_ast, quiet_when_empty=False):
            sys.argv = [str(IMPORT_PY), *argv]
            runpy.run_path(str(IMPORT_PY), run_name="__main__")
    except SystemExit as stop:
        if stop.code not in (None, 0):
            return int(stop.code) if isinstance(stop.code, int) else 1
    finally:
        sys.argv = saved_argv
    return 0


# --------------------------------------------------------------- comparison

class FidelityError(RuntimeError):
    pass


def function_view(path: Path, symbol: str) -> dict:
    """The symbol's words and relocation records, read straight from the ELF."""
    elf = reloc_surface.Elf(Path(path))
    syms = elf.symbols()
    rows = [s for s in syms if s[0] == symbol and s[3] & 0xF == STT_FUNC and s[4] != 0]
    if not rows:
        raise FidelityError(f"{symbol}: no defined function symbol in {path}")
    name, start, size, _info, shndx = rows[0]
    section = elf.names[shndx]
    data = elf.section_bytes(section)
    if size == 0:  # sizeless symbol: run to the next function or the section end
        later = sorted(s[1] for s in syms if s[4] == shndx and s[3] & 0xF == STT_FUNC and s[1] > start)
        size = (later[0] if later else len(data)) - start
    words = [w for (w,) in struct.iter_unpack(">I", data[start:start + size - size % 4])]
    funcs_at = {s[1]: s[0] for s in syms if s[4] == shndx and s[3] & 0xF == STT_FUNC and s[0]}
    relocs: dict[int, tuple[int, str]] = {}
    for sh_index, sh in enumerate(elf.sh):
        if sh[1] != SHT_REL or sh[7] != shndx:
            continue
        for pos in range(sh[4], sh[4] + sh[5], 8):
            off, info = struct.unpack_from(">II", elf.data, pos)
            if not start <= off < start + size:
                continue
            kind, symidx = info & 0xFF, info >> 8
            relocs[off - start] = (kind, _target(elf, syms, symidx, kind, data, off, funcs_at))
    return {"symbol": name, "words": words, "relocs": relocs, "section": section}


def _target(elf, syms, symidx: int, kind: int, data: bytes, off: int, funcs_at: dict) -> str:
    if symidx >= len(syms):
        return f"<bad symbol {symidx}>"
    name, _value, _size, info, shndx = syms[symidx]
    if info & 0xF == STT_SECTION or not name:
        section = elf.names[shndx] if 0 < shndx < len(elf.names) else f"<section {shndx}>"
        if kind == R_MIPS_26 and section == ".text":
            dest = (struct.unpack_from(">I", data, off)[0] & 0x03FFFFFF) << 2
            return funcs_at.get(dest, f".text+{dest:#x}")
        return section
    return name


def masked_word(word: int, reloc: tuple[int, str] | None) -> int:
    if reloc is None:
        return word
    return word & ~FIELD_MASKS.get(reloc[0], 0xFFFFFFFF) & 0xFFFFFFFF


def _rows(view: dict) -> list[tuple[int, tuple[int, str] | None]]:
    return [(masked_word(w, view["relocs"].get(i * 4)), view["relocs"].get(i * 4))
            for i, w in enumerate(view["words"])]


def compare_views(scratch: dict, tree: dict) -> dict:
    """Tree offsets where the two views disagree, by masked word or relocation record.

    At equal length the comparison is positional. When the lengths differ, one
    missing or extra instruction would make every later word "differ", so the
    two streams are aligned first and only the unmatched words are counted
    (a one-sided word is reported at the tree offset where it falls).
    """
    sw, tw = scratch["words"], tree["words"]
    off_words: set[int] = set()
    off_relocs: set[int] = set()
    if len(sw) == len(tw):
        for i in range(len(tw)):
            at = i * 4
            sr, tr = scratch["relocs"].get(at), tree["relocs"].get(at)
            if masked_word(sw[i], sr) != masked_word(tw[i], tr):
                off_words.add(at)
            if sr != tr:
                off_relocs.add(at)
        count = len(off_words | off_relocs)
    else:
        import difflib
        srows, trows = _rows(scratch), _rows(tree)
        count = 0
        matcher = difflib.SequenceMatcher(None, srows, trows, autojunk=False)
        for tag, i1, i2, j1, j2 in matcher.get_opcodes():
            if tag == "equal":
                continue
            count += max(i2 - i1, j2 - j1)
            for j in range(j1, max(j2, j1 + 1)):
                off_words.add(j * 4)
            if [r[1] for r in srows[i1:i2]] != [r[1] for r in trows[j1:j2]]:
                off_relocs.add(j1 * 4)
    sites = sorted(off_words | off_relocs)
    return {"identical": not sites, "count": count, "sites": sites, "word_sites": sorted(off_words),
            "reloc_sites": sorted(off_relocs), "scratch_words": len(sw), "tree_words": len(tw),
            "aligned": len(sw) != len(tw), "relocations": len(tree["relocs"])}


def render(symbol: str, result: dict) -> str:
    if result["identical"]:
        return (f"SCRATCH FIDELITY: identical ({symbol}: {result['tree_words']} words, "
                f"{result['relocations']} relocation records)")
    sites = result["sites"]
    shown = ", ".join(f"+{s:#x}" for s in sites[:8]) + (", ..." if len(sites) > 8 else "")
    extra = ""
    if result["reloc_sites"]:
        extra = f"; relocation records differ at {len(result['reloc_sites'])} site(s)"
    if result["aligned"]:
        extra += "; sizes differ, counted after alignment"
    return (f"SCRATCH FIDELITY: {result['count']} words off at {shown} "
            f"({symbol}: scratch {result['scratch_words']} vs tree {result['tree_words']} words{extra})")


def compare_objects(symbol: str, scratch_obj: Path, tree_obj: Path) -> dict:
    return compare_views(function_view(scratch_obj, symbol), function_view(tree_obj, symbol))


# --------------------------------------------------------------- building

def postcompile_lines(compile_sh: str) -> list[str]:
    """objcopy lines `compile.sh` runs after its cc line (tools/permute.sh appends them)."""
    lines = compile_sh.splitlines()
    cc_at = next((i for i, line in enumerate(lines) if "$INPUT" in line and "-o" in line), None)
    if cc_at is None:
        return []
    return [line for line in lines[cc_at + 1:] if "objcopy" in line and '"$OUTPUT"' in line]


def compile_scratch(scratch: Path, out: Path) -> subprocess.CompletedProcess:
    out.parent.mkdir(parents=True, exist_ok=True)
    out.unlink(missing_ok=True)
    out.touch()  # compile.sh realpath()s its output, which BSD realpath needs to exist
    return subprocess.run(["bash", str(scratch / "compile.sh"), str(scratch / "base.c"), "-o", str(out)],
                          cwd=ROOT, capture_output=True, text=True)


def compile_tree(source: str, out: Path, compile_sh: str = "") -> subprocess.CompletedProcess:
    import fast_score
    out.parent.mkdir(parents=True, exist_ok=True)
    args = fast_score.rewrite_io(fast_score.configured_cc_args(source), ROOT / source, source, out)
    proc = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    if proc.returncode:
        return proc
    for line in postcompile_lines(compile_sh):
        step = subprocess.run(["bash", "-c", f'OUTPUT="$1"; {line}', "postcompile", str(out)],
                              cwd=ROOT, capture_output=True, text=True)
        if step.returncode:
            return step
    return proc


def run_check(symbol: str, scratch: Path, source: str, workdir: Path | None) -> tuple[int, dict]:
    scratch = scratch.resolve()
    source = os.path.relpath((ROOT / source).resolve(), ROOT) if Path(source).is_absolute() else source
    workdir = (workdir or scratch.parent / "fidelity").resolve()
    scratch_obj, tree_obj = workdir / "scratch.o", workdir / "tree.o"
    built = compile_scratch(scratch, scratch_obj)
    if built.returncode or scratch_obj.stat().st_size == 0:
        print("SCRATCH FIDELITY: cannot compile the scratch base.c\n" + built.stderr[-2000:], file=sys.stderr)
        return 2, {"error": "scratch compile failed"}
    tree = compile_tree(source, tree_obj, (scratch / "compile.sh").read_text())
    if tree.returncode:
        print(f"SCRATCH FIDELITY: cannot compile {source} through its configured recipe\n"
              + tree.stderr[-2000:], file=sys.stderr)
        return 2, {"error": "tree compile failed"}
    try:
        result = compare_objects(symbol, scratch_obj, tree_obj)
    except FidelityError as error:
        print(f"SCRATCH FIDELITY: {error}", file=sys.stderr)
        return 2, {"error": str(error)}
    result.update(symbol=symbol, source=source, scratch_object=str(scratch_obj), tree_object=str(tree_obj))
    print(render(symbol, result))
    return (0 if result["identical"] else 3), result


def main(argv: list[str] | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "import":
        rest = argv[1:]
        if rest and rest[0] == "--":
            rest = rest[1:]
        return run_import(rest)
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="command", required=True)
    sub.add_parser("import", help="run import.py with block-scope prototype restoration")
    chk = sub.add_parser("check", help="compile scratch and tree once each and compare the symbol")
    chk.add_argument("symbol")
    chk.add_argument("scratch", type=Path)
    chk.add_argument("--source", required=True, help="tracked TU path, e.g. src/main/anim.c")
    chk.add_argument("--workdir", type=Path, help="where the two objects go (default <scratch>/../fidelity)")
    chk.add_argument("--json", metavar="PATH")
    cmp_ = sub.add_parser("compare", help="compare one symbol in two existing objects")
    cmp_.add_argument("symbol")
    cmp_.add_argument("scratch_object", type=Path)
    cmp_.add_argument("tree_object", type=Path)
    ns = ap.parse_args(argv)
    if ns.command == "compare":
        try:
            result = compare_objects(ns.symbol, ns.scratch_object, ns.tree_object)
        except (FidelityError, OSError, SystemExit) as error:
            print(f"SCRATCH FIDELITY: {error}", file=sys.stderr)
            return 2
        print(render(ns.symbol, result))
        return 0 if result["identical"] else 3
    status, result = run_check(ns.symbol, ns.scratch, ns.source, ns.workdir)
    if ns.json:
        Path(ns.json).write_text(json.dumps(result, indent=1) + "\n")
    return status


if __name__ == "__main__":
    sys.exit(main())
