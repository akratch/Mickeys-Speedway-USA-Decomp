#!/usr/bin/env python3
"""Compile small probe TUs with a tracked TU's own recipe and summarise the code.

    tools/microlab.py --template T.c --vars spec.json --flags-from src/main/anim.c
                      [--function NAME] [--listing] [--jobs N] [--out DIR] [--instrumented]

A lane asking "what does IDO emit for this shape?" writes a ten-line probe and a
`cc` line. The probe is cheap; the cc line is where the measurement goes wrong:
a typed command that drops `-Wab,-r4300_mul` or `-mips2` measures a different
compiler (2026-10-01: 197 words against the true 13). So this tool never takes a
compile command. `--flags-from` names a tracked TU, and the probe is compiled
with exactly that TU's configured recipe (fast_score's `gmake -n NON_MATCHING=1`
expansion), only the source and output paths replaced, from the repository root
so the TU's includes resolve.

The template is C with `{{NAME}}` placeholders. The spec says what fills them:

    {"vars": {"TYPE": ["s16", "s32"], "MASK": ["0xFF", "0xFFU"]}}   # product: 4 variants
    {"variants": [{"TYPE": "s16"}, {"TYPE": "u8", "MASK": "0xFF"}]}  # explicit list
    {"TYPE": ["s16", "s32"]}                                          # shorthand for "vars"

Each variant is written to `<out>/v<k>.c` (default `build/microlab/<template>/`,
never tracked), compiled, and summarised per function: instruction count, the
registers it names, and its opcode counts. A variant whose function matches
variant 0 word for word says so; otherwise the opcodes whose counts moved are
named. `--listing` prints each function's disassembly too (probe code only; it
is your synthetic C, and the output goes to the terminal, never into a file the
tree tracks). Any `CDX_*`/`DKWB_*` variable or `--instrumented` compiles with
the instrumented driver, as fast_score does.
"""

from __future__ import annotations

import argparse
import itertools
import json
import os
import re
import subprocess
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fast_score  # noqa: E402

ROOT = fast_score.ROOT
PLACEHOLDER_RE = re.compile(r"\{\{\s*([A-Za-z_][A-Za-z0-9_]*)\s*\}\}")
REGISTER_RE = re.compile(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra|f[0-9]|f[12][0-9]|f3[01])\b")
FUNC_RE = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")
INSN_RE = re.compile(r"^\s*[0-9a-f]+:\t([0-9a-f]{8}) \t(\S+)\s*(.*)$")


class LabError(RuntimeError):
    pass


def variants_from_spec(spec) -> list[dict[str, str]]:
    """The list of {placeholder: text} assignments the spec describes."""
    if isinstance(spec, dict) and "variants" in spec:
        rows = spec["variants"]
        if not isinstance(rows, list) or not all(isinstance(r, dict) for r in rows):
            raise LabError('"variants" must be a list of objects')
        return [{k: str(v) for k, v in r.items()} for r in rows]
    if isinstance(spec, dict):
        table = spec.get("vars", spec)
        if not isinstance(table, dict) or not table:
            raise LabError('spec needs "vars" (name -> list of values) or "variants"')
        names = list(table)
        for name in names:
            if not isinstance(table[name], list) or not table[name]:
                raise LabError(f"spec value for {name!r} must be a non-empty list")
        return [dict(zip(names, (str(v) for v in combo)))
                for combo in itertools.product(*(table[n] for n in names))]
    if isinstance(spec, list):
        return variants_from_spec({"variants": spec})
    raise LabError("spec must be a JSON object or list")


def render(template: str, values: dict[str, str]) -> str:
    missing = sorted({m.group(1) for m in PLACEHOLDER_RE.finditer(template)} - set(values))
    if missing:
        raise LabError("template placeholders with no value: " + ", ".join(missing))
    return PLACEHOLDER_RE.sub(lambda m: values[m.group(1)], template)


def parse_disassembly(text: str) -> dict[str, list[tuple[int, str, str]]]:
    """{function: [(word, mnemonic, operands)]} from `objdump -d -z` output."""
    out: dict[str, list[tuple[int, str, str]]] = {}
    current = None
    for line in text.splitlines():
        m = FUNC_RE.match(line)
        if m:
            current = m.group(1)
            out[current] = []
            continue
        m = INSN_RE.match(line)
        if m and current is not None:
            out[current].append((int(m.group(1), 16), m.group(2), m.group(3).strip()))
    return out


def summarise(rows: list[tuple[int, str, str]]) -> dict:
    ops = Counter(op for _w, op, _a in rows)
    regs = sorted({r for _w, _op, args in rows for r in REGISTER_RE.findall(args)},
                  key=lambda r: (r[0], len(r), r))
    return {"count": len(rows), "ops": dict(sorted(ops.items())), "registers": regs,
            "words": [w for w, _op, _a in rows]}


def compare(base: dict, other: dict) -> str:
    if base["words"] == other["words"]:
        return "identical to v0"
    moved = []
    for op in sorted(set(base["ops"]) | set(other["ops"])):
        a, b = base["ops"].get(op, 0), other["ops"].get(op, 0)
        if a != b:
            moved.append(f"{op} {a}->{b}")
    regs = ""
    if base["registers"] != other["registers"]:
        regs = f"; registers {' '.join(other['registers'])}"
    size = other["count"] - base["count"]
    return (f"differs from v0 ({size:+d} insns"
            + (f"; {', '.join(moved)}" if moved else "; same opcode counts") + regs + ")")


def objdump(obj: Path) -> str:
    return subprocess.run([str(ROOT / "tools/binutils/mips64-elf-objdump"), "-d", "-z", str(obj)],
                          capture_output=True, text=True, check=True).stdout


def compile_probe(args: list[str], source: str, probe: Path) -> tuple[Path, str]:
    obj = probe.with_suffix(".o")
    obj.unlink(missing_ok=True)
    cmd = fast_score.rewrite_io(args, probe, source, obj)
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if proc.returncode:
        return obj, (proc.stderr.strip().splitlines() or ["compile failed"])[-1]
    return obj, ""


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--template", type=Path, required=True)
    ap.add_argument("--vars", type=Path, required=True, help="JSON spec of placeholder values")
    ap.add_argument("--flags-from", required=True, metavar="TU",
                    help="tracked TU whose configured recipe compiles the probes, e.g. src/main/anim.c")
    ap.add_argument("--function", help="summarise only this function")
    ap.add_argument("--listing", action="store_true", help="also print each function's disassembly")
    ap.add_argument("--jobs", type=int, default=3)
    ap.add_argument("--out", type=Path, help="probe directory (default build/microlab/<template stem>)")
    ap.add_argument("--instrumented", action="store_true")
    ap.add_argument("--json", type=Path, metavar="PATH")
    ns = ap.parse_args(argv)
    try:
        source = Path(ns.flags_from).as_posix()
        if Path(source).is_absolute():
            source = os.path.relpath(source, ROOT)
        if not (ROOT / source).is_file() or not source.startswith("src/"):
            raise LabError(f"--flags-from must name a tracked TU under src/: {ns.flags_from}")
        tracked = subprocess.run(["git", "ls-files", "--error-unmatch", source], cwd=ROOT,
                                 capture_output=True, text=True)
        if tracked.returncode:
            raise LabError(f"--flags-from must name a tracked TU: {source}")
        template = ns.template.read_text()
        variants = variants_from_spec(json.loads(ns.vars.read_text()))
        texts = [render(template, v) for v in variants]
    except (LabError, OSError, ValueError) as error:
        print(f"microlab: {error}", file=sys.stderr)
        return 2
    out = (ns.out or ROOT / "build" / "microlab" / ns.template.stem).resolve()
    out.mkdir(parents=True, exist_ok=True)
    instrumented = fast_score.instrumented_wanted(os.environ, ns.instrumented)
    args = fast_score.use_compiler(fast_score.configured_cc_args(source),
                                   fast_score.compiler_for(instrumented))
    print(f"recipe: {source} ({'instrumented' if instrumented else 'stock'}): "
          + " ".join(a for a in args[1:] if a.startswith(("-O", "-mips", "-W", "-g"))))
    probes = []
    for k, text in enumerate(texts):
        probe = out / f"v{k}.c"
        probe.write_text(text)
        probes.append(probe)
    with ThreadPoolExecutor(max_workers=max(1, min(ns.jobs, 3))) as pool:
        built = list(pool.map(lambda p: compile_probe(args, source, p), probes))
    report = []
    first: dict[str, dict] = {}
    status = 0
    for k, (values, (obj, error)) in enumerate(zip(variants, built)):
        label = ", ".join(f"{n}={v}" for n, v in values.items())
        if error:
            print(f"v{k} [{label}]: COMPILE ERROR: {error}")
            report.append({"variant": k, "values": values, "error": error})
            status = 1
            continue
        functions = parse_disassembly(objdump(obj))
        if ns.function:
            functions = {n: r for n, r in functions.items() if n == ns.function}
            if not functions:
                print(f"v{k} [{label}]: no function {ns.function} in the probe")
                status = 1
                continue
        print(f"v{k} [{label}]")
        row = {"variant": k, "values": values, "functions": {}}
        for name, rows in functions.items():
            info = summarise(rows)
            note = compare(first[name], info) if name in first else "reference"
            first.setdefault(name, info)
            ops = " ".join(f"{op}:{n}" for op, n in info["ops"].items())
            print(f"  {name}: {info['count']} insns; regs {' '.join(info['registers']) or '-'}; {note}")
            print(f"    ops: {ops}")
            if ns.listing:
                for _word, op, operands in rows:
                    print(f"      {op:8s} {operands}")
            row["functions"][name] = {k2: v for k2, v in info.items() if k2 != "words"} | {"note": note}
        report.append(row)
    if ns.json:
        ns.json.write_text(json.dumps({"source": source, "probes": str(out), "variants": report}, indent=1) + "\n")
    print(f"probes: {out}")
    return status


if __name__ == "__main__":
    sys.exit(main())
