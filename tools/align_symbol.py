#!/usr/bin/env python3
"""Align a candidate against its target by shape, and split the residual by cause.

    tools/align_symbol.py <symbol> [<symbol> ...] [--json] [--context N]

`tools/score_symbol.py` answers "how many words differ, positionally". That is
the right number near zero and the wrong one everywhere else, for two reasons
this tool exists to remove:

**On a size mismatch the positional number is noise.** One extra or missing
instruction shifts every word after it, so a function four bytes too long can
report hundreds of differing words and none of them is the defect. The
question worth asking is *where the insertion is*, and a positional diff
cannot answer it because it never aligns across the shift.

**At several hundred words displacement dominates the count.** A candidate can
score worse while being structurally closer -- measured on this project's own
whale, one spelling read 894 positionally against another's 988 while losing
2,905 byte-exact rows to 2,872 under alignment. Adopting on the positional
number alone reverses that decision.

So: align both instruction streams on a **register-erased shape**, then sort
each aligned pair into one of three buckets.

    byte-exact          agreed, once linker-controlled bits are masked
    register-naming     same instruction, different register -- an ALLOCATION
                        problem: colour, save ratio, web numbering
    immediate only      same instruction and registers, different displacement
                        or constant -- usually a FRAME problem, sometimes a
                        wrong literal; never reconstruction
    really different    different instruction, or present on one side only --
                        a STRUCTURE problem: spelling, control flow

Those three point at different levers, and the split says which one you have.
Three whales measured this way came apart as 53/43/4, 16/77/9 and 7/84/9
percent; two were naming problems where chasing the first divergence could not
have paid more than a sixth of the residual.

**Displacement tax** is what the positional count exceeds the aligned
disagreement by. It can come out slightly *negative*, and that is not a fault:
the alignment is free to accept a few more disagreeing pairs in one place to
win many more exact matches elsewhere, and the positional count has no such
freedom. Measured across the 40 largest queued functions, one came out at -13
and the rest were zero or positive. Read a small negative as "no displacement
to recover", not as an error. A large tax means the stream is shifted, not wrong, and the
lever is whatever caused the shift -- usually one instruction, named in the
`insertions`/`deletions` this tool prints.

Both streams are read through `nm_ranking.word_streams`, the same path the
ranking scores by, so the positional figure here agrees with `score_symbol.py`
and `docs/nm-ranking.md` by construction. Relocation payload bits are masked at
the positional union of both relocation surfaces exactly as the ranking does --
a hand-rolled comparison that skips this reports differences that do not
survive linking.
"""
from __future__ import annotations

import argparse
import contextlib
import difflib
import json
import pathlib
import shutil
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import nm_ranking as nr  # noqa: E402
import permute_batch as pb  # noqa: E402


@contextlib.contextmanager
def _isolated_workdir():
    """Own scratch, so this can run in a tight loop beside a build.

    Same reasoning as `score_symbol.py`: `nm_ranking` keeps a fixed directory
    under `build/`, which is right for a whole-queue pass and wrong for a tool
    a lane calls repeatedly while `gmake` is touching the same paths.
    """
    previous = nr.WORK_DIR
    scratch = pathlib.Path(tempfile.mkdtemp(prefix="align-symbol-"))
    nr.WORK_DIR = scratch
    try:
        yield
    finally:
        nr.WORK_DIR = previous
        shutil.rmtree(scratch, ignore_errors=True)


def instr_imm_mask(word: int) -> int:
    """Drop the immediate/displacement, keeping opcode, function and registers.

    The complement of `nm_ranking.instr_reg_mask`, and the pair of them sorts an
    aligned disagreement into *which field* differs. Without this the third
    bucket over-reads badly: a lane measuring three functions found 62 of 101,
    177 of 292 and 172 of 265 "really different" rows were the same instruction
    on the same registers at a different displacement. Two thirds of a
    structural bucket being frame displacement changes what a lane does next.
    """
    op = (word >> 26) & 0x3F
    if op in (0x02, 0x03):  # j, jal: the whole payload is the target
        return word & 0xFC000000
    if op == 0x00:  # SPECIAL: shamt is the immediate here
        return word & 0xFFFF083F
    if op == 0x11:  # COP1: register format carries no immediate
        if ((word >> 21) & 0x1F) == 0x08:  # BC1: low half is a branch offset
            return word & 0xFFFF0000
        return word
    return word & 0xFFFF0000


def reloc_masked(word: int, offset: int, a_reloc: dict, b_reloc: dict) -> int:
    """Clear the bits the linker controls at this offset, from either surface.

    The union matters: splat writes a literal where it has no symbol, so one
    side can carry a relocation the other does not, and comparing unmasked
    reports a difference that linking removes.
    """
    value_mask = 0
    for relocation in (a_reloc.get(offset), b_reloc.get(offset)):
        if relocation is not None:
            value_mask |= nr.RELOC_VALUE_MASKS.get(
                relocation[0], nr.UNKNOWN_RELOC_VALUE_MASK
            )
    return word & ((~value_mask) & 0xFFFFFFFF)


def _banded_edit_script(a: list[int], b: list[int]) -> list[tuple[str, int, int]]:
    """Align two instruction streams allowing substitution, in a diagonal band.

    `difflib.SequenceMatcher` is the obvious tool and it is the wrong one here.
    It has no substitution operation, so one changed instruction in the middle
    of an otherwise identical stream comes back as an `insert` at one offset
    and a `delete` at a completely different one -- measured on a 20-word
    probe, a single substitution at index 10 produced an insert at target 10
    and a delete at candidate 19. That double-counts the difference, and it
    reports a missing instruction at an offset where nothing is missing. Since
    "where is the extra instruction" is exactly what this tool is for on a size
    mismatch, that output would send a lane to a false location.

    So: unit-cost edit distance with a real substitution, restricted to a band
    around the diagonal. The band only has to cover the net length difference
    plus whatever local reordering exists, and outside it the alignment is
    never competitive; `BAND_SLACK` is generous against that. Cost is
    O(n x band) rather than O(n x m), which keeps a several-thousand-word
    function well under a second.

    Returns a list of (op, index_in_a, index_in_b) with op in
    {"equal", "replace", "delete", "insert"}; "delete" is present in a only
    (an extra candidate instruction), "insert" is present in b only.
    """
    n, m = len(a), len(b)
    band = abs(n - m) + BAND_SLACK
    INF = float("inf")

    # row[k] is the cost at column j = i + k - band, for k in [0, 2*band].
    width = 2 * band + 1

    def col(i: int, k: int) -> int:
        return i + k - band

    previous = [INF] * width
    # i = 0: cost of consuming j columns of b is j insertions.
    for k in range(width):
        j = col(0, k)
        if 0 <= j <= m:
            previous[k] = j
    choices: list[bytes] = []

    for i in range(1, n + 1):
        current = [INF] * width
        row_choice = bytearray(width)
        for k in range(width):
            j = col(i, k)
            if j < 0 or j > m:
                continue
            best, pick = INF, 0
            # deletion: consume a[i-1], stay at j. previous row, column j
            # sits at offset k+1 there because the diagonal shifted by one.
            if k + 1 < width and previous[k + 1] != INF:
                best, pick = previous[k + 1] + 1, 1  # delete
            if j > 0:
                if previous[k] != INF:
                    cost = previous[k] + (0 if a[i - 1] == b[j - 1] else 1)
                    # Prefer substitution on a tie: an equal-length difference
                    # is one changed instruction, not an insert plus a delete.
                    if cost <= best:
                        best, pick = cost, 2  # replace/equal
                if k > 0 and current[k - 1] != INF:
                    if current[k - 1] + 1 < best:
                        best, pick = current[k - 1] + 1, 3  # insert
            current[k] = best
            row_choice[k] = pick
        previous = current
        choices.append(bytes(row_choice))

    # Traceback from (n, m).
    script: list[tuple[str, int, int]] = []
    i, j = n, m
    while i > 0 or j > 0:
        k = j - i + band
        if i == 0:
            script.append(("insert", i, j - 1)); j -= 1; continue
        if j == 0:
            script.append(("delete", i - 1, j)); i -= 1; continue
        pick = choices[i - 1][k] if 0 <= k < width else 0
        if pick == 1:
            script.append(("delete", i - 1, j)); i -= 1
        elif pick == 3:
            script.append(("insert", i, j - 1)); j -= 1
        else:
            op = "equal" if a[i - 1] == b[j - 1] else "replace"
            script.append((op, i - 1, j - 1)); i -= 1; j -= 1
    script.reverse()
    return script


BAND_SLACK = 48


def align(streams: "nr.WordStreams") -> dict:
    """Align candidate against target by shape and bucket every aligned pair."""
    base, target = streams.base_words, streams.target_words
    b_reloc, t_reloc = streams.base_reloc, streams.target_reloc

    # The alignment key erases registers AND linker-controlled bits, so that a
    # rotated allocation still aligns and only real shape drives the matching.
    def key(words, reloc, other_reloc):
        return [
            nr.instr_reg_mask(reloc_masked(w, i * 4, reloc, other_reloc))
            for i, w in enumerate(words)
        ]

    base_key = key(base, b_reloc, t_reloc)
    target_key = key(target, t_reloc, b_reloc)
    script = _banded_edit_script(base_key, target_key)

    exact = naming = immediate = different = 0
    insertions: list[dict] = []
    deletions: list[dict] = []
    naming_sites: list[int] = []
    immediate_sites: list[int] = []
    different_sites: list[int] = []

    for op, i, j in script:
        if op == "equal":
            bw = reloc_masked(base[i], i * 4, b_reloc, t_reloc)
            tw = reloc_masked(target[j], j * 4, t_reloc, b_reloc)
            if bw == tw:
                exact += 1
            elif nr.instr_reg_mask(bw) == nr.instr_reg_mask(tw):
                naming += 1
                naming_sites.append(j * 4)
            elif instr_imm_mask(bw) == instr_imm_mask(tw):
                immediate += 1
                immediate_sites.append(j * 4)
            else:
                different += 1
                different_sites.append(j * 4)
        elif op == "replace":
            # A substitution still has to be sorted by *which field* differs.
            # The alignment key keeps the immediate, so two loads on the same
            # registers at different displacements arrive here rather than as
            # an "equal" pair -- which is exactly how the third bucket came to
            # over-read frame displacement as structure.
            bw = reloc_masked(base[i], i * 4, b_reloc, t_reloc)
            tw = reloc_masked(target[j], j * 4, t_reloc, b_reloc)
            if nr.instr_reg_mask(bw) == nr.instr_reg_mask(tw):
                naming += 1
                naming_sites.append(j * 4)
            elif instr_imm_mask(bw) == instr_imm_mask(tw):
                immediate += 1
                immediate_sites.append(j * 4)
            else:
                different += 1
                different_sites.append(j * 4)
        elif op == "delete":  # present in candidate, absent from target
            different += 1
            if insertions and insertions[-1]["candidate_offset"] + \
                    insertions[-1]["words"] * 4 == i * 4:
                insertions[-1]["words"] += 1
            else:
                insertions.append({"candidate_offset": i * 4, "words": 1})
        elif op == "insert":  # present in target, absent from candidate
            different += 1
            if deletions and deletions[-1]["target_offset"] + \
                    deletions[-1]["words"] * 4 == j * 4:
                deletions[-1]["words"] += 1
            else:
                deletions.append({"target_offset": j * 4, "words": 1})

    return {
        "aligned_exact": exact,
        "aligned_register_naming": naming,
        "aligned_immediate_only": immediate,
        "aligned_really_different": different,
        "insertions": insertions,
        "deletions": deletions,
        "first_naming_offset": naming_sites[0] if naming_sites else None,
        "first_immediate_offset": immediate_sites[0] if immediate_sites else None,
        "first_different_offset": different_sites[0] if different_sites else None,
    }


def matched_target_object(symbol: str) -> pathlib.Path:
    """The built object of a matched symbol's TU (`build/<tracked source>.o`)."""
    import fast_score
    source = fast_score.tracked_source_for(symbol)
    obj = nr.ROOT / "build" / (source + ".o")
    if not obj.is_file():
        raise SystemExit(f"{symbol}: not in the NON_MATCHING queue and {obj.relative_to(nr.ROOT)} "
                         "does not exist; run gmake to build the matched object first")
    return obj


class AlignedScorer:
    """Score many candidate objects of ONE symbol against a cached target.

    `nr.word_streams` re-assembles the target listing on every call, which is
    the whole cost when a product of hundreds of cells is ranked. The target
    stream does not depend on the candidate, so it is read once here and each
    `score(obj)` only reads the candidate's own stream. Thread-safe after
    construction (no shared mutable state, nothing touches `nr.WORK_DIR`).
    """

    def __init__(self, symbol: str, workdir: pathlib.Path):
        queue = {item.func: item for item in pb.discover_queue()}
        item = queue.get(symbol)
        self.symbol = symbol
        self.matched = item is None
        if item is None:
            # An already-matched (promoted) symbol: its target is the built
            # object of its own TU, which `gmake verify` proves byte-identical
            # to the ROM for this function. That lets post-match cleanup be
            # measured: a candidate is exact when it still equals that object.
            target_o = matched_target_object(symbol)
        else:
            out_dir = pathlib.Path(workdir) / "aligned-target"
            out_dir.mkdir(parents=True, exist_ok=True)
            target_o = out_dir / "target.o"
            target_asm = pb.prepare_target_asm(item, out_dir)
            nr.assemble_target(target_asm, target_o)
        span = nr.func_symbol_span(target_o, symbol)
        if span is None:
            raise SystemExit(f"{symbol}: no .text symbol in the target object")
        self.target_size = span[1]
        self.target_words = nr.words_of(nr.text_bytes(target_o, *span))
        self.target_reloc = nr.relocations(target_o, *span)

    def score(self, obj: pathlib.Path) -> dict | None:
        span = nr.func_symbol_span(obj, self.symbol)
        if span is None:
            return None
        streams = nr.WordStreams(
            base_words=nr.words_of(nr.text_bytes(obj, *span)),
            target_words=self.target_words,
            base_reloc=nr.relocations(obj, *span),
            target_reloc=self.target_reloc,
            base_size=span[1], target_size=self.target_size,
        )
        _cat, raw, _first, masked, first = nr.classify(
            streams.base_size, streams.target_size, streams.base_words,
            streams.target_words, streams.base_reloc, streams.target_reloc)
        row = {"masked": masked, "raw": raw, "delta": span[1] - self.target_size,
               "first": first}
        row.update(align(streams))
        row["residual"] = (row["aligned_register_naming"]
                           + row["aligned_immediate_only"]
                           + row["aligned_really_different"])
        return row


def render_buckets(row: dict) -> str:
    """One line: the four buckets and the one-sided spans of an aligned row."""
    ins = sum(s["words"] for s in row["insertions"])
    dele = sum(s["words"] for s in row["deletions"])
    return (f"exact {row['aligned_exact']}  naming {row['aligned_register_naming']}  "
            f"imm {row['aligned_immediate_only']}  different {row['aligned_really_different']}  "
            f"(candidate-only {ins}, target-only {dele})  residual {row['residual']}")


def measure(symbols: list[str]) -> tuple[list[dict], list[str]]:
    queue = {item.func: item for item in pb.discover_queue()}
    wanted, errors = [], []
    for symbol in symbols:
        item = queue.get(symbol)
        if item is None:
            errors.append(f"{symbol}: not in the NON_MATCHING queue "
                          f"(already matched, or never queued)")
        else:
            wanted.append(item)
    if not wanted:
        return [], errors

    with _isolated_workdir():
        commands = nr.configured_compile_commands(wanted)
        compiled = {source: nr.compile_configured_tu(source, commands[source])
                    for source in commands}
        rows = []
        for item in wanted:
            candidate, error = compiled[item.rel_c_file]
            if candidate is None:
                errors.append(f"{item.func}: {error}")
                continue
            streams, error = nr.word_streams(item, candidate)
            if streams is None:
                errors.append(f"{item.func}: {error}")
                continue
            result, error = nr.process_item(item, candidate)
            if result is None:
                errors.append(f"{item.func}: {error}")
                continue
            row = {
                "symbol": item.func,
                "file": item.rel_c_file,
                "size_bytes": result.size_bytes,
                "size_delta": result.size_delta,
                "positional_masked": result.relocation_masked_differing_words,
                "candidate_words": len(streams.base_words),
                "target_words": len(streams.target_words),
            }
            row.update(align(streams))
            disagree = (row["aligned_register_naming"]
                        + row["aligned_immediate_only"]
                        + row["aligned_really_different"])
            positional = row["positional_masked"]
            row["displacement_tax"] = (
                positional - disagree if positional is not None else None
            )
            rows.append(row)
    return rows, errors


def render(row: dict) -> str:
    out = [f"{row['symbol']}  ({row['file']})",
           f"  {row['size_bytes']} bytes, size delta {row['size_delta']:+d}, "
           f"candidate {row['candidate_words']} words vs target {row['target_words']}"]
    tax = row["displacement_tax"]
    out.append(f"  positional masked  {row['positional_masked']}")
    if tax is not None:
        out.append(f"  displacement tax   {tax}"
                   + ("   <-- the count is mostly shift, not disagreement"
                      if tax > 0 and tax >= 2 * (row["aligned_register_naming"]
                                                 + row["aligned_really_different"])
                      else ""))
    out.append("  aligned:")
    total = (row["aligned_exact"] + row["aligned_register_naming"]
             + row["aligned_immediate_only"]
             + row["aligned_really_different"]) or 1
    for label, field in (("byte-exact", "aligned_exact"),
                         ("register naming", "aligned_register_naming"),
                         ("immediate only", "aligned_immediate_only"),
                         ("really different", "aligned_really_different")):
        value = row[field]
        out.append(f"    {label:<18} {value:>6}  ({100.0 * value / total:.1f}%)")
    if row["insertions"]:
        out.append("  candidate has instructions the target does not:")
        for span in row["insertions"]:
            out.append(f"    +0x{span['candidate_offset']:X}  {span['words']} word(s)")
    if row["deletions"]:
        out.append("  target has instructions the candidate does not:")
        for span in row["deletions"]:
            out.append(f"    +0x{span['target_offset']:X}  {span['words']} word(s)")
    if row["first_naming_offset"] is not None:
        out.append(f"  first naming-only difference   +0x{row['first_naming_offset']:X}")
    if row["first_immediate_offset"] is not None:
        out.append(f"  first immediate-only difference +0x{row['first_immediate_offset']:X}")
    if row["first_different_offset"] is not None:
        out.append(f"  first structural difference    +0x{row['first_different_offset']:X}")
    return "\n".join(out)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Align a candidate against its target and split the residual by cause.")
    parser.add_argument("symbols", nargs="+")
    parser.add_argument("--json", action="store_true", help="machine-readable output")
    args = parser.parse_args(argv)

    rows, errors = measure(args.symbols)
    if args.json:
        print(json.dumps({"functions": rows, "errors": errors}, indent=2))
    else:
        for row in rows:
            print(render(row))
            print()
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
    return 1 if errors and not rows else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
