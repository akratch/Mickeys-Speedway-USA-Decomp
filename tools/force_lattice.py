#!/usr/bin/env python3
"""Search COMBINATIONS of allocator forces, not one force at a time.

    tools/force_lattice.py <symbol> --proc N \
        --force p1:w27=c17 --force p1:w75=c16 [...] [--max-size N]

WHY THIS EXISTS.

Every force receipt this campaign has taken on a large function was measured
ALONE. On the tree's biggest function that produced five separately accepted
colours, each improving the score in isolation:

    web 27 -> s3   227 -> 224      web 379 -> s6  227 -> 217
    web 75 -> s2   227 -> 212      web 956 -> s5  227 -> 220
    web 225 -> s0  227 -> 220

Seven passes recorded those singly and none measured them together, while the
handoff hedged that they "are global effects and are not additive credits".
That is a reason to MEASURE the combination, not to assume it.

The campaign already has the counterexample that makes this urgent: L100's
receipt is a function that went 21 -> 5 on a PAIR of edits where *each edit
alone was a 31-word regression*. Measuring one decision at a time does not
merely under-report a combination -- it can invert its sign. A lattice search
is the only honest way to price a set of interacting decisions.

WHAT IT REPORTS, and why each part matters.

* the singles, re-measured here, so an inherited receipt is confirmed rather
  than trusted -- a force taken on an older source may not reproduce;
* the best combination found, with the subset that reaches it;
* the INTERACTION of each pair: additive, synergistic, or antagonistic.
  Antagonism is the interesting verdict. Two colours that each help alone but
  hurt together have interacting allocation effects. Inspect the decision
  records before attributing that interaction to a particular register.

ACCEPTANCE IS CHECKED, NOT ASSUMED. `CDX_FORCE` is silently ignored unless
`CDX_PROC` is also set, recording `forced=-2` and returning a byte-identical
object that is indistinguishable from a legitimate decline (Trap 20). Every
cell here reads the forced value back out of the records and refuses to score
a cell whose forces did not all apply.

DECISION ORDER, NOT COLOUR. `CDX_BIAS="web=delta,..."` (with `CDX_PROC`) adds
delta to a p1 web's save for globalcolor's selection only, so a decision order
can be imposed and scored where a colour force cannot reach a split; this tool
does not drive it (see docs/LANE_BRIEF.md, Instruments).

THE OBJECT IS SCORED DIRECTLY. score_symbol/align_symbol/frame_census all
recompile the TU with the *configured* command before measuring, which
overwrites the forced object and reports the unforced build -- one lane read
the same number for twelve consecutive forces before noticing. This reads the
forced object's own text.
"""
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import itertools
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

# Window width for the blast radius, in bytes. 0x80 is calibrated, not chosen:
# on the overlay 58 lattice all ten pairs measured an interaction of exactly
# zero, and at 0x200 the radii of the two forces nearest the entry still
# overlapped -- one false collision out of ten. At 0x80 and below the radii
# separate and the prediction matches all ten measurements; at word granularity
# no two of the five forces move the same word at all. A window wider than the
# distance between two forces' footprints merges them and reports contention
# that the scores do not show, so widen this only to read a coarse map, never
# to judge an interaction.
WINDOW = 0x80

ROOT = pathlib.Path(__file__).resolve().parents[1]
INSTRUMENTED = pathlib.Path.home() / "Desktop" / "dev" / "ido-instrumented"
# A force is `p1:w27=c17`. A JOINT force is `p1:w27+w75=c17`: two webs driven
# onto ONE colour in the same compile. The distinction matters because the
# overlay 58 blocker is a coupled pair -- "removing a capture range fixes the
# first split, but a later split then takes the restore" -- and a landscape of
# single forces is structurally unable to find a coupled repair, however many
# probes it runs.
# `=cN` forces a colour; `=s` forces the SPLIT path, the compiler's own grammar
# (its refusal message names both). A split receipt is one `p1dec` row with
# `forced=-1` and NO `p1color` row for that web (a split may leave several
# decision rows for one web number): the web went to memory.
# The joint form `wA+wB=cN` is accepted here for the lattice's bookkeeping but
# the instrumented uopt refuses it outright ("not a phase-qualified force
# control", exit 55) -- measured on overlay 58, lane wv-p. Two webs driven
# together are two single forces in one cell, nothing else.
FORCE_RE = re.compile(r"^p[12]:w\d+(\+w\d+)*=(c\d+|s)$")


@dataclasses.dataclass(frozen=True)
class Cell:
    """One measured point: a set of forces and what it scored."""

    forces: tuple[str, ...]
    score: int | None
    accepted: bool
    note: str = ""
    size_delta: int | None = None
    raw_score: int | None = None
    first_mismatch: int | None = None
    artifact: str = ""
    windows: dict[int, int] = dataclasses.field(default_factory=dict, compare=False)

    @property
    def label(self) -> str:
        return "+".join(f.split(":", 1)[1] for f in self.forces) or "(base)"


def webs_of(force: str) -> list[int]:
    """Every web a force drives, so a joint force is not read as one web."""
    return [int(w.lstrip("w")) for w in force.split(":", 1)[1].split("=", 1)[0].split("+")]


def is_split(force: str) -> bool:
    """A `=s` force asks the allocator to split the web instead of colouring it."""
    return force.split("=", 1)[1] == "s"


def colour_of(force: str) -> int:
    if is_split(force):
        raise ValueError(f"{force} is a split force and names no colour")
    return int(force.split("=", 1)[1].lstrip("c"))


def validate_forces(forces: list[str]) -> None:
    """Refuse a malformed spec before spending a compile on it."""
    for force in forces:
        if not FORCE_RE.match(force):
            raise SystemExit(
                f"force {force!r} is not of the form p1:wN=cM. "
                f"A malformed spec is silently ignored by the compiler, which "
                f"reads as a declined force and proves nothing."
            )
    # Compare WEBS, not whole specs: a joint force `p1:w27+w75=c17` contains two
    # of them, and comparing the spec string would miss w27 also appearing in a
    # single force elsewhere in the same cell.
    seen: dict[tuple[str, int], str] = {}
    duplicated = set()
    for force in forces:
        phase = force.split(":", 1)[0]
        for web in webs_of(force):
            if (phase, web) in seen and seen[(phase, web)] != force:
                duplicated.add(f"{phase}:w{web}")
            seen[(phase, web)] = force
    if duplicated:
        raise SystemExit(
            f"web(s) {', '.join(sorted(duplicated))} appear twice with different "
            f"colours; the compiler takes one and the result would be attributed "
            f"to both."
        )


def subsets(forces: list[str], max_size: int | None = None) -> list[tuple[str, ...]]:
    """Every non-empty subset, smallest first.

    Smallest first so the singles are measured before any combination: a single
    that fails to reproduce its inherited receipt invalidates every combination
    containing it, and finding that out first saves the rest of the sweep.
    """
    limit = len(forces) if max_size is None else min(max_size, len(forces))
    out: list[tuple[str, ...]] = []
    for size in range(1, limit + 1):
        out.extend(itertools.combinations(forces, size))
    return out


def interaction(pair_score: int, a_score: int, b_score: int, base: int) -> str:
    """Classify how two forces combine, relative to acting independently.

    `expected` is what the pair would score if the two were independent: each
    contributes its own delta from the base. The verdict is about the gap
    between that and what the pair actually scored.
    """
    expected = base - ((base - a_score) + (base - b_score))
    if pair_score < expected:
        return "SYNERGISTIC"
    if pair_score > expected:
        return "ANTAGONISTIC"
    return "additive"


def compile_command(symbol: str) -> list[str]:
    """The unchanged configured command for the symbol's TU.

    Derived from the build rather than retyped: a hand-written line that drops
    a per-file flag makes BOTH sides of an identity gate wrong in the same way,
    so they agree with each other and disagree with the tree (one such slip
    read 33 against the configured 31).
    """
    import nm_ranking as nr
    import permute_batch as pb

    items = [i for i in pb.discover_queue() if i.func == symbol]
    if not items:
        raise SystemExit(f"{symbol} is not in the NON_MATCHING queue")
    commands = nr.configured_compile_commands(items)
    command = commands.get(items[0].rel_c_file)
    if not command:
        raise SystemExit(f"no configured compile command for {symbol}")
    return list(command)


def replace_compiler(command: list[str], compiler: pathlib.Path) -> list[str]:
    """Keep the asm-processor interpreter, wrapper, assembler and flags intact."""
    actual = list(command)
    # Resolve against ROOT, not the cwd: these are build-command tokens written
    # relative to the repository root, and run_cell runs them with cwd=ROOT.
    # Resolving against the cwd made the swap refuse from any other directory.
    matches = [i for i, token in enumerate(actual)
               if (ROOT / token).resolve() == (ROOT / "tools/ido/cc").resolve()]
    if len(matches) != 1:
        raise ValueError("configured command must name exactly one stock IDO compiler")
    actual[matches[0]] = str(compiler)
    return actual


def force_acceptance(trace: str, proc: int, forces: tuple[str, ...]) -> str | None:
    """Require a fresh decision for every requested phase/procedure/web/colour.

    Unrequested decisions normally say forced=-2. A missing log or a force
    applied to a different web is not acceptance. Profiles report acceptance
    as either the requested positive colour or -1; the chosen colour must also
    agree. Duplicate records indicate a mixed/stale capture and fail closed.
    """
    rows = []
    for line in trace.splitlines():
        match = re.match(r"^\[CDX\]\s+(p[12](?:dec|color))\s+(.*)$", line.strip())
        if match:
            rows.append((match[1], dict(re.findall(r"(\w+)=([^\s]+)", match[2]))))
    if not any(event.endswith("dec") and row.get("proc") == str(proc)
               for event, row in rows):
        return f"no allocator decisions for procedure {proc}"
    for force in forces:
        phase = force.split(":", 1)[0]
        if is_split(force):
            for web_number in webs_of(force):
                web = str(web_number)
                decs = [r for event, r in rows if event == phase + "dec"
                        and r.get("proc") == str(proc) and r.get("web") == web]
                cols = [r for event, r in rows if event == phase + "color"
                        and r.get("proc") == str(proc) and r.get("web") == web]
                # A split can leave one decision row or several (the
                # fragments keep the web number); every one must say -1.
                if not decs:
                    return f"{force}: no decision record for web {web}"
                if any(r.get("forced") != "-1" for r in decs) or cols:
                    return (f"{force}: not applied to web {web} "
                            f"(forced={[r.get('forced', 'missing') for r in decs]}, "
                            f"colour rows={len(cols)})")
            continue
        colour = str(colour_of(force))
        # Every web a force names must be accepted. A joint force that lands on
        # one of its two webs is not the intervention that was requested, and
        # reading its score as the pair's would attribute a single force's
        # result to a coupling that never happened.
        for web_number in webs_of(force):
            web = str(web_number)
            found = {kind: [r for event, r in rows if event == phase + kind
                           and r.get("proc") == str(proc) and r.get("web") == web]
                     for kind in ("dec", "color")}
            if any(len(matches) != 1 for matches in found.values()):
                return (f"{force}: expected one decision and one final colour "
                        f"record for web {web}")
            # bestcolor is the pre-force proposal. Only the later colour row
            # records the assignment actually emitted by the instrumented pass.
            row = found["color"][0]
            if row.get("forced") not in {colour, "-1"} or row.get("color") != colour:
                return (f"{force}: not applied to web {web} "
                        f"(forced={row.get('forced', 'missing')})")
    return None


def window_residual(symbol: str, object_path: pathlib.Path,
                    width: int = WINDOW) -> dict[int, int]:
    """Bin this object's residual words into fixed windows of the function.

    Bins the *masked* positions, the same ones the score counts, so the bins
    sum to the cell's score rather than to some larger raw number. Returns an
    empty histogram rather than raising: a cell that scored is a real result
    even when its radius could not be read.
    """
    import nm_ranking as nr
    import permute_batch as pb

    try:
        items = [i for i in pb.discover_queue() if i.func == symbol]
        if not items:
            return {}
        streams, _ = nr.word_streams(items[0], object_path)
        if streams is None:
            return {}
        positions = nr.masked_mismatch_positions(
            streams.base_words, streams.target_words,
            streams.base_reloc, streams.target_reloc,
        )
    except Exception:  # noqa: BLE001 - a missing radius must not void a score
        return {}
    counts: dict[int, int] = {}
    for index in positions:
        key = (index * 4) // width * width
        counts[key] = counts.get(key, 0) + 1
    return counts


def blast_radius(base: Cell, cell: Cell) -> dict[int, int]:
    """Which windows this force moved, and by how many words, against the base.

    Signed: negative is residual removed, positive is residual introduced. A
    force that saves words in one window and spends them in another is a very
    different object from one that only saves, and the total hides that.
    """
    if not base.windows or not cell.windows:
        return {}
    keys = set(base.windows) | set(cell.windows)
    moved = {k: cell.windows.get(k, 0) - base.windows.get(k, 0) for k in keys}
    return {k: v for k, v in sorted(moved.items()) if v}


def collides(base: Cell, a: Cell, b: Cell) -> set[int]:
    """Windows both forces moved -- where their effects can contend.

    An empty result predicts additivity; a non-empty one only says the two
    footprints fall in a shared window at this resolution, which at a coarse
    WINDOW can happen to forces that share no word. Treat a collision as a
    question for a narrower window, not as a verdict.

    Disjoint radii are the mechanism behind an additive pair: two forces that
    recolour different regions of the function cannot fight over a word. This
    is what makes an additive measurement explicable rather than merely
    observed, and it predicts additivity from n measurements instead of
    confirming it with 2**n.
    """
    return set(blast_radius(base, a)) & set(blast_radius(base, b))


def _score(symbol: str, object_path: pathlib.Path):
    """Score the supplied object directly; never discover or rebuild a candidate."""
    import nm_ranking as nr
    import permute_batch as pb

    items = [i for i in pb.discover_queue() if i.func == symbol]
    if not items:
        raise ValueError(f"{symbol} is not in the NON_MATCHING queue")
    item = items[0]
    result, error = nr.process_item(item, object_path)
    if result is None or result.relocation_masked_differing_words is None:
        raise ValueError(error or "object score unavailable")
    return result


def run_cell(symbol: str, proc: int, forces: tuple[str, ...], *,
             command: list[str], directory: pathlib.Path,
             deadline: float, timeout: float, stock: bool = False) -> Cell:
    """Preserve one bounded compile, its own trace, and its direct object score."""
    directory.mkdir()
    output = directory / "candidate.o"
    trace = directory / "allocator.log"
    actual = list(command) if stock else replace_compiler(command, INSTRUMENTED / "cc")
    actual[actual.index("-o") + 1] = str(output)
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(("CDX_", "DKWB_"))}
    if not stock:
        env.update(CDX_LOG="1", CDX_PROC=str(proc), CDX_OUT=str(trace),
                   CDX_DETAIL_WEB="all")
        if forces:
            env["CDX_FORCE"] = ",".join(forces)
    (directory / "command.json").write_text(json.dumps(actual, indent=2) + "\n")
    try:
        if time.time() + timeout >= deadline:
            raise ValueError("compile cap cannot fit before the deadline")
        result = subprocess.run(actual, env=env, capture_output=True, text=True,
                                cwd=ROOT, timeout=timeout)
        (directory / "compile.log").write_text(result.stdout + result.stderr)
        if result.returncode or not output.is_file():
            raise ValueError(f"compile failed (exit {result.returncode}); see compile.log")
        if not stock:
            error = force_acceptance(trace.read_text() if trace.exists() else "", proc, forces)
            if error:
                raise ValueError(error)
        score = _score(symbol, output)
        cell = Cell(forces, score.relocation_masked_differing_words, True,
                    size_delta=score.size_delta, raw_score=score.differing_words,
                    first_mismatch=score.relocation_masked_first_mismatch_offset,
                    artifact=directory.name,
                    windows=window_residual(symbol, output))
    except (OSError, ValueError, subprocess.TimeoutExpired) as exc:
        cell = Cell(forces, None, False, str(exc), artifact=directory.name)
    (directory / "result.json").write_text(json.dumps(dataclasses.asdict(cell), indent=2) + "\n")
    return cell


def render(base: Cell, cells: list[Cell], forces: list[str]) -> str:
    out = [f"base (no force): {base.score}", ""]
    singles = {c.forces[0]: c for c in cells if len(c.forces) == 1}
    out.append("singles:")
    for force in forces:
        cell = singles.get(force)
        if cell is None or cell.score is None:
            out.append(f"  {force:<16} UNMEASURED  {cell.note if cell else ''}")
        else:
            delta = (base.score - cell.score) if base.score is not None else 0
            out.append(f"  {force:<16} {cell.score:>5}   {delta:+d}")
    out += ["", "lattice (masked words; size delta in bytes):"]
    for cell in cells:
        value = str(cell.score) if cell.accepted and cell.score is not None else "UNMEASURED"
        delta = f"{cell.size_delta:+d}" if cell.size_delta is not None else "?"
        out.append(f"  {cell.label:<65} {value:>10}  delta {delta}  {cell.note}")
    scored = [c for c in cells if c.accepted and c.score is not None
              and c.size_delta in (None, 0)]
    if scored:
        best = min(scored, key=lambda c: c.score)
        out += ["", f"BEST: {best.score} with {best.label}"]
        if base.score is not None:
            out.append(f"      {base.score - best.score:+d} against the base")
    radii = {f: blast_radius(base, c) for f, c in singles.items()
             if c.accepted and c.score is not None}
    if any(radii.values()):
        out += ["", f"blast radius (windows of {WINDOW:#x} bytes, "
                    "signed words against the base):"]
        for force in forces:
            moved = radii.get(force)
            if not moved:
                out.append(f"  {force.split(':')[1]:<12} (no radius read)")
                continue
            spans = " ".join(f"{k:#06x}{v:+d}" for k, v in moved.items())
            out.append(f"  {force.split(':')[1]:<12} {spans}")

    if base.score is not None and len(forces) > 1:
        out += ["", "pairwise interaction:"]
        by = {c.forces: c for c in cells if c.accepted and c.score is not None}
        for a, b in itertools.combinations(forces, 2):
            pair = by.get((a, b)) or by.get((b, a))
            ca, cb = singles.get(a), singles.get(b)
            label = f"  {a.split(':')[1]:<12} + {b.split(':')[1]:<12} "
            if not (pair and ca and cb and ca.accepted and cb.accepted
                    and ca.score is not None and cb.score is not None):
                out.append(label + "UNMEASURED")
                continue
            if any(c.size_delta not in (None, 0) for c in (base, ca, cb, pair)):
                out.append(label + "SIZE-CHANGED; positional interaction not comparable")
                continue
            verdict = interaction(pair.score, ca.score, cb.score, base.score)
            expected = ca.score + cb.score - base.score
            shared = collides(base, ca, cb)
            if not (radii.get(a) and radii.get(b)):
                why = ""
            elif shared:
                why = "  contend in " + ",".join(f"{w:#06x}" for w in sorted(shared))
            else:
                why = "  disjoint radii"
            out.append(label + f"{pair.score:>5}  expected {expected}, "
                       f"interaction {pair.score - expected:+d}  {verdict}{why}")
    return "\n".join(out)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("symbol")
    parser.add_argument("--proc", type=int, required=True,
                        help="procedure ordinal; CDX_PROC must be an ordinal, "
                             "obtained by first running with a non-numeric "
                             "value to print the procindex table")
    parser.add_argument("--force", action="append", default=[], dest="forces",
                        help="p1:wN=cM, repeatable")
    parser.add_argument("--max-size", type=int, default=None,
                        help="largest subset to measure (default: all)")
    parser.add_argument("--dry-run", action="store_true",
                        help="list the cells without compiling")
    parser.add_argument("--output-dir", type=pathlib.Path,
                        help="new evidence directory outside the checkout (default: temporary)")
    parser.add_argument("--seconds", type=float, default=600,
                        help="total wall-clock cap, including setup (default: 600)")
    parser.add_argument("--compile-timeout", type=float, default=120,
                        help="per-compile cap; must fit in remaining time (default: 120)")
    parser.add_argument("--deadline-unix", type=float,
                        default=os.environ.get("MICKEY_TASK_DEADLINE_UNIX"),
                        help="also honor this absolute soft deadline")
    parser.add_argument("--expect-single", action="append", default=[],
                        help="stop before combinations if FORCE=SCORE does not reproduce")
    args = parser.parse_args(argv)

    if not args.forces:
        parser.error("give at least one --force")
    validate_forces(args.forces)
    if (args.proc < 0 or args.seconds <= 0 or args.compile_timeout <= 0
            or (args.max_size is not None and args.max_size < 1)):
        parser.error("procedure must be nonnegative; sizes and time caps must be positive")
    expected = {}
    for receipt in args.expect_single:
        try:
            force, score = receipt.rsplit("=", 1)
            score = int(score)
        except ValueError:
            parser.error("expected receipt must be FORCE=SCORE")
        if force not in args.forces or score < 0 or force in expected:
            parser.error("expected receipt must name one unique requested force and a nonnegative score")
        expected[force] = score

    plan = subsets(args.forces, args.max_size)
    if args.dry_run:
        print(f"{len(plan)} cell(s) would be measured, smallest first:")
        for forces in plan:
            print("  " + "+".join(f.split(":", 1)[1] for f in forces))
        return 0

    import nm_ranking as nr

    deadline = min(time.time() + args.seconds, args.deadline_unix or float("inf"))
    if args.output_dir:
        directory = args.output_dir.resolve()
        if directory.is_relative_to(ROOT):
            parser.error("evidence directory must be outside the checkout")
        directory.mkdir(parents=True, exist_ok=False)
    else:
        directory = pathlib.Path(tempfile.mkdtemp(prefix="force-lattice-"))
    print(f"private evidence: {directory}", flush=True)
    previous_work = nr.WORK_DIR
    nr.WORK_DIR = directory / "ranking"
    cells = []
    base = Cell((), None, False, "baseline not measured")
    failure = None
    try:
        command = compile_command(args.symbol)
        source = ROOT / command[-1]
        source_bytes = source.read_bytes()
        (directory / "source.c").write_bytes(source_bytes)
        (directory / "source.sha256").write_text(hashlib.sha256(source_bytes).hexdigest() + "\n")
        common = dict(command=command, deadline=deadline, timeout=args.compile_timeout)
        stock = run_cell(args.symbol, args.proc, (), directory=directory / "stock",
                         stock=True, **common)
        base = run_cell(args.symbol, args.proc, (), directory=directory / "base", **common)
        if not stock.accepted or not base.accepted:
            raise ValueError(f"baseline failed: stock {stock.note}; instrumented {base.note}")
        if source.read_bytes() != source_bytes:
            raise ValueError("source changed during baseline capture")
        if time.time() + args.compile_timeout >= deadline:
            raise ValueError("fidelity cap cannot fit before the deadline")
        fidelity_run = subprocess.run(
            [str(ROOT / ".venv/bin/decomp-workbench"), "fidelity",
             str(directory / "stock/candidate.o"), str(directory / "base/candidate.o"),
             "--objdump", str(ROOT / "tools/binutils/mips64-elf-objdump"), "--json"],
            capture_output=True, text=True, cwd=ROOT, timeout=args.compile_timeout)
        (directory / "fidelity.log").write_text(fidelity_run.stdout + fidelity_run.stderr)
        fidelity = json.loads(fidelity_run.stdout)
        if (fidelity_run.returncode or not isinstance(fidelity, dict)
                or fidelity.get("pass") is not True
                or not isinstance(fidelity.get("gates"), dict) or not fidelity["gates"]
                or not all(value is True for value in fidelity["gates"].values())):
            raise ValueError("stock/instrumented object fidelity failed")
        (directory / "fidelity.json").write_text(json.dumps(fidelity, indent=2) + "\n")
        print(f"stock/instrumented fidelity: PASS; baseline {base.score}", flush=True)
        for index, forces in enumerate(plan, 1):
            if source.read_bytes() != source_bytes:
                raise ValueError("source changed during lattice")
            cell = run_cell(args.symbol, args.proc, forces,
                            directory=directory / f"cell-{index:03d}", **common)
            cells.append(cell)
            (directory / "results.json").write_text(json.dumps(
                {"base": dataclasses.asdict(base), "cells": [dataclasses.asdict(c) for c in cells]},
                indent=2) + "\n")
            print(f"{index}/{len(plan)} {cell.label}: {cell.score} {cell.note}", flush=True)
            if source.read_bytes() != source_bytes:
                raise ValueError("source changed during cell capture; result is unauthenticated")
            if len(forces) == 1 and forces[0] in expected:
                if not cell.accepted or cell.score != expected[forces[0]] or cell.size_delta != 0:
                    raise ValueError(f"single receipt failed for {forces[0]}; stopping before combinations")
            if index < len(plan) and time.time() + args.compile_timeout >= deadline:
                raise ValueError("next compile cap cannot fit before the deadline")
    except (OSError, ValueError, RuntimeError, SystemExit, subprocess.TimeoutExpired) as exc:
        failure = str(exc)
    finally:
        nr.WORK_DIR = previous_work
    report = render(base, cells, args.forces)
    if failure:
        report += f"\n\nSTOP: {failure}"
    (directory / "report.txt").write_text(report + "\n")
    print(report)
    refused = [c for c in cells if not c.accepted]
    if refused:
        print(f"\n{len(refused)} cell(s) did not apply and were not scored:",
              file=sys.stderr)
        for cell in refused[:5]:
            print(f"  {cell.label}: {cell.note}", file=sys.stderr)
    return int(bool(failure or refused or not base.accepted))


if __name__ == "__main__":
    raise SystemExit(main())
