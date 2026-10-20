#!/usr/bin/env python3
"""Read-only branch-layout diagnostic; never a linker or matching proof.

    tools/branch_layout_report.py --symbol NAME --target-object target.o \
        --target-listing target.s --candidate-object candidate.o [--regions spans.json]

No compiler is invoked. The official aligner's output is retained separately.
Only internal R_MIPS_PC16 payloads are resolved in an in-memory diagnostic copy,
using exact label bindings and instruction VMAs from a function-only extracted
listing. Every resolved payload must agree with that listing. Unknown symbols,
unsupported relocations, stale listings and unresolved candidate PC16s refuse
rather than being masked. No object is written.

Optional regions are a JSON list of {name, target_span:[start,end],
candidate_span:[start,end]} covering both owned streams without gaps/overlap.
They can describe dispatch handlers, but their identity is supplied by the
caller, not inferred by this tool. Output is private evidence when its inputs
are ROM-derived. A paired-destination result describes an alignment, not
semantic equivalence, target allocator history, or acceptance for promotion.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import align_symbol as al  # noqa: E402
import nm_ranking as nr  # noqa: E402

ROW = re.compile(r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+\*/")
LABEL = re.compile(r"^\s*(?:glabel\s+([^\s]+)|([\w.$]+):)\s*$")


def branch_destination(word: int, pc: int) -> int | None:
    """Owned-stream-relative destination of a supported PC-relative branch."""
    op, rt = word >> 26, (word >> 16) & 31
    supported = (op in (4, 5, 20, 21)
                 or (op in (6, 7, 22, 23) and rt == 0)
                 or (op == 1 and rt in (0, 1, 2, 3, 16, 17, 18, 19))
                 or (op == 17 and ((word >> 21) & 31) == 8))
    if not supported:
        return None
    displacement = word & 0xFFFF
    if displacement & 0x8000:
        displacement -= 0x10000
    return pc + 4 + displacement * 4


def parse_listing(text: str, symbol: str) -> dict:
    """Bind exact label names to the next recorded instruction, never name hex."""
    active = False
    pending: list[str] = []
    labels: dict[str, int] = {}
    rows: list[tuple[int, int]] = []
    for line in text.splitlines():
        label = LABEL.match(line)
        if label:
            name = label.group(1) or label.group(2)
            if name == symbol:
                if active:
                    raise ValueError("duplicate function label")
                active = True
            if active:
                if name in labels or name in pending:
                    raise ValueError(f"duplicate listing label: {name}")
                pending.append(name)
        if not active:
            continue
        if re.match(r"\s*endlabel\s+" + re.escape(symbol) + r"\s*$", line):
            break
        row = ROW.search(line)
        if row:
            address, word = int(row[1], 16), int(row[2], 16)
            if address % 4 or (rows and address != rows[-1][0] + 4):
                raise ValueError("listing instructions must be aligned and contiguous")
            labels.update((name, address) for name in pending)
            pending.clear()
            rows.append((address, word))
    if not rows or symbol not in labels:
        raise ValueError("function listing is absent or empty")
    return {"base": rows[0][0], "words": [word for _, word in rows], "labels": labels}


def resolve_internal_pc16(words: list[int], relocs: dict, listing: dict) -> tuple[list[int], list[dict]]:
    """Authenticate the target surface and resolve supported internal branches."""
    if len(words) != len(listing["words"]):
        raise ValueError("listing and object owned extents differ")
    base, end = listing["base"], listing["base"] + len(words) * 4
    resolved, records = list(words), []
    for index, (word, expected) in enumerate(zip(words, listing["words"])):
        offset = index * 4
        relocation = relocs.get(offset)
        if relocation and relocation[0] == "R_MIPS_PC16":
            if branch_destination(word, offset) is None:
                raise ValueError(f"PC16 is not a supported branch at {offset}")
            if (word & 0xFFFF0000) != (expected & 0xFFFF0000):
                raise ValueError(f"PC16 opcode/register mismatch at {offset}")
            symbol = relocation[1]
            if symbol not in listing["labels"]:
                raise ValueError(f"PC16 symbol has no exact listing binding: {symbol}")
            address = listing["labels"][symbol]
            if not base <= address < end or (address - base) % 4:
                raise ValueError("PC16 symbol is outside the owned instruction range")
            addend = word & 0xFFFF
            if addend & 0x8000:
                addend -= 0x10000
            addend *= 4
            delta = address + addend - (base + offset)
            if delta % 4 or not -0x8000 <= delta // 4 <= 0x7FFF:
                raise ValueError("PC16 resolved displacement is unaligned or out of range")
            payload = (delta // 4) & 0xFFFF
            if payload != (expected & 0xFFFF):
                raise ValueError(f"PC16 formula disagrees with target listing at {offset}")
            linked = (word & 0xFFFF0000) | payload
            destination = branch_destination(linked, offset)
            if destination is None or not 0 <= destination < len(words) * 4:
                raise ValueError("PC16 destination is outside the owned instruction range")
            resolved[index] = linked
            records.append({"offset": offset, "symbol": symbol,
                            "symbol_vma": address, "encoded_addend_bytes": addend,
                            "resolved_payload": payload, "destination": destination})
        else:
            mask = 0
            if relocation:
                if relocation[0] not in nr.RELOC_VALUE_MASKS:
                    raise ValueError(f"unsupported target relocation: {relocation[0]}")
                mask = nr.RELOC_VALUE_MASKS[relocation[0]]
            if ((word ^ expected) & (~mask & 0xFFFFFFFF)) != 0:
                raise ValueError(f"listing/object instruction mismatch at {offset}")
    return resolved, records


def pair_script(streams: nr.WordStreams) -> list[tuple[str, int, int]]:
    def equal_at(i: int, j: int) -> bool:
        a, b = al._reloc_pair_masked(streams.base_words[i], streams.target_words[j],
                                    i * 4, j * 4, streams.base_reloc, streams.target_reloc)
        return nr.instr_reg_mask(a) == nr.instr_reg_mask(b)
    return al._banded_edit_script(streams.base_words, streams.target_words, equal_at=equal_at)


def branch_pairs(streams: nr.WordStreams) -> list[dict]:
    """Describe branch destinations through the actual diagnostic alignment."""
    script = pair_script(streams)
    mapping = {j * 4: i * 4 for op, i, j in script if op in ("equal", "replace")}
    reverse = {v: k for k, v in mapping.items()}
    rows = []
    for j, word in enumerate(streams.target_words):
        td = branch_destination(word, j * 4)
        if td is None:
            continue
        pc = mapping.get(j * 4)
        cd = None if pc is None else branch_destination(streams.base_words[pc // 4], pc)
        expected = mapping.get(td)
        if pc is None or cd is None:
            status = "no_paired_candidate_branch"
        elif expected is None:
            status = "destination_has_no_instruction_pair"
        elif cd == expected:
            status = "destination_matches_instruction_pair"
        else:
            status = "destination_differs_from_instruction_pair"
        candidate_word = None if pc is None else streams.base_words[pc // 4]
        op = word >> 26
        # REGIMM/COP1 encode the condition in additional fixed fields.
        kind_mask = 0xFC1F0000 if op == 1 else (0xFFFF0000 if op == 17 else 0xFC000000)
        rows.append({"target_offset": j * 4, "candidate_offset": pc,
                     "target_destination": td, "candidate_destination": cd,
                     "paired_expected_destination": expected, "status": status,
                     "branch_kind_matches": candidate_word is not None and cd is not None
                     and (word & kind_mask) == (candidate_word & kind_mask),
                     "condition_register_fields_match": candidate_word is not None and cd is not None
                     and (word & 0xFFFF0000) == (candidate_word & 0xFFFF0000)})
    for i, word in enumerate(streams.base_words):
        destination = branch_destination(word, i * 4)
        target_pc = reverse.get(i * 4)
        if destination is not None and (target_pc is None or branch_destination(
                streams.target_words[target_pc // 4], target_pc) is None):
            rows.append({"target_offset": target_pc, "candidate_offset": i * 4,
                         "candidate_destination": destination, "status": "candidate_branch_without_target_branch"})
    return rows


def immediate_rows(streams: nr.WordStreams, branches: list[dict]) -> list[dict]:
    """Separate residual branch distances from stack, data and literal fields."""
    by_target = {row["target_offset"]: row for row in branches if row["target_offset"] is not None}
    rows = []
    for op, i, j in pair_script(streams):
        if op not in ("equal", "replace"):
            continue
        a, b = al._reloc_pair_masked(streams.base_words[i], streams.target_words[j],
                                    i * 4, j * 4, streams.base_reloc, streams.target_reloc)
        if (a == b or nr.instr_reg_mask(a) == nr.instr_reg_mask(b)
                or al.instr_imm_mask(a) != al.instr_imm_mask(b)):
            continue
        branch = by_target.get(j * 4)
        opcode = b >> 26
        if branch is not None:
            category = "branch_displacement"
        elif ((b >> 21) & 31) == 29 and (opcode in (8, 9, 24, 25) or opcode >= 32):
            category = "stack_displacement"
        elif opcode >= 32:
            category = "nonstack_memory_displacement"
        else:
            category = "literal_or_arithmetic_immediate"
        row = {"target_offset": j * 4, "candidate_offset": i * 4, "category": category}
        if branch is not None:
            row["branch_status"] = branch["status"]
        rows.append(row)
    return rows


def validate_regions(regions: list[dict], target_size: int, candidate_size: int) -> None:
    target_end = candidate_end = 0
    names = set()
    for region in regions:
        if set(region) != {"name", "target_span", "candidate_span"} or not isinstance(region["name"], str):
            raise ValueError("region requires name, target_span and candidate_span")
        if not region["name"] or region["name"] in names:
            raise ValueError("region names must be nonempty and unique")
        names.add(region["name"])
        for key, previous, size in (("target_span", target_end, target_size),
                                    ("candidate_span", candidate_end, candidate_size)):
            span = region[key]
            if (not isinstance(span, list) or len(span) != 2
                    or any(type(value) is not int or value % 4 for value in span)
                    or span[0] != previous or not span[0] < span[1] <= size):
                raise ValueError("regions must partition both streams in address order")
        target_end, candidate_end = region["target_span"][1], region["candidate_span"][1]
    if target_end != target_size or candidate_end != candidate_size:
        raise ValueError("regions do not cover both owned streams")


def sliced(streams: nr.WordStreams, ts: list[int], cs: list[int]) -> nr.WordStreams:
    return nr.WordStreams(
        base_words=streams.base_words[cs[0] // 4:cs[1] // 4],
        target_words=streams.target_words[ts[0] // 4:ts[1] // 4],
        base_reloc={k - cs[0]: v for k, v in streams.base_reloc.items() if cs[0] <= k < cs[1]},
        target_reloc={k - ts[0]: v for k, v in streams.target_reloc.items() if ts[0] <= k < ts[1]},
        base_size=cs[1] - cs[0], target_size=ts[1] - ts[0])


def validate_relocations(relocations: dict, size: int, surface: str) -> None:
    for offset, relocation in relocations.items():
        if type(offset) is not int or offset % 4 or not 0 <= offset < size:
            raise ValueError(f"{surface} relocation offset is unaligned or outside owned extent")
        if (not isinstance(relocation, (tuple, list)) or len(relocation) != 2
                or any(not isinstance(value, str) or not value for value in relocation)):
            raise ValueError(f"malformed {surface} relocation record")
        if relocation[0] != "R_MIPS_PC16" and relocation[0] not in nr.RELOC_VALUE_MASKS:
            raise ValueError(f"unsupported {surface} relocation: {relocation[0]}")


def report(streams: nr.WordStreams, listing: dict, regions: list[dict] | None = None) -> dict:
    if streams.base_size != len(streams.base_words) * 4 or streams.target_size != len(streams.target_words) * 4:
        raise ValueError("owned extents do not match instruction streams")
    validate_relocations(streams.base_reloc, streams.base_size, "candidate")
    validate_relocations(streams.target_reloc, streams.target_size, "target")
    if any(value[0] == "R_MIPS_PC16" for value in streams.base_reloc.values()):
        raise ValueError("candidate has unresolved PC16; provide a resolved candidate surface")
    resolved, records = resolve_internal_pc16(streams.target_words, streams.target_reloc, listing)
    diagnostic = nr.WordStreams(base_words=streams.base_words, target_words=resolved,
                               base_reloc=streams.base_reloc, target_reloc=streams.target_reloc,
                               base_size=streams.base_size, target_size=streams.target_size)
    pairs = branch_pairs(diagnostic)
    immediates = immediate_rows(diagnostic, pairs)
    result = {"schema_version": 1, "diagnostic_only": True, "matching_proof": False,
              "official_alignment": al.align(streams),
              "resolved_layout_alignment": al.align(diagnostic), "pc16_resolutions": records,
              "branch_status_counts": dict(Counter(row["status"] for row in pairs)),
              "branches": pairs, "resolved_immediate_rows": immediates,
              "resolved_immediate_categories": dict(Counter(row["category"] for row in immediates)),
              "regions": [],
              "limits": ["No object bytes are changed and no compiler/linker is invoked.",
                         "Alignment and paired destinations do not prove semantic or CFG equivalence.",
                         "A differing pair can reflect scheduling or basic-block-entry materialization.",
                         "External relocation identities and final linked bytes remain separate proof obligations.",
                         "Region identities, if supplied, are caller claims; only their extent partition is validated.",
                         "Region buckets align within each supplied span; branch statuses use whole-function pairing."]}
    if regions is not None:
        validate_regions(regions, streams.target_size, streams.base_size)
        for region in regions:
            ts, cs = region["target_span"], region["candidate_span"]
            subset = [row for row in pairs if row["target_offset"] is not None and ts[0] <= row["target_offset"] < ts[1]]
            result["regions"].append({**region, "start_shift_bytes": cs[0] - ts[0],
                                      "size_delta_bytes": cs[1] - cs[0] - ts[1] + ts[0],
                                      "resolved_alignment": al.align(sliced(diagnostic, ts, cs)),
                                      "resolved_immediate_categories": dict(Counter(
                                          row["category"] for row in immediates
                                          if ts[0] <= row["target_offset"] < ts[1])),
                                      "branch_status_counts": dict(Counter(row["status"] for row in subset))})
    return result


def object_stream(path: Path, symbol: str) -> tuple[list[int], dict, int]:
    span = nr.func_symbol_span(path, symbol)
    if span is None:
        raise ValueError(f"owned .text symbol is absent: {symbol}")
    return nr.words_of(nr.text_bytes(path, *span)), nr.relocations(path, *span), span[1]


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--target-object", required=True, type=Path)
    parser.add_argument("--target-listing", required=True, type=Path)
    parser.add_argument("--candidate-object", required=True, type=Path)
    parser.add_argument("--regions", type=Path)
    args = parser.parse_args(argv)
    try:
        tw, tr, ts = object_stream(args.target_object, args.symbol)
        cw, cr, cs = object_stream(args.candidate_object, args.symbol)
        listing = parse_listing(args.target_listing.read_text(), args.symbol)
        regions = json.loads(args.regions.read_text()) if args.regions else None
        result = report(nr.WordStreams(base_words=cw, target_words=tw, base_reloc=cr,
                                       target_reloc=tr, base_size=cs, target_size=ts), listing, regions)
        paths = {"target_object": args.target_object, "target_listing": args.target_listing,
                 "candidate_object": args.candidate_object}
        if args.regions:
            paths["regions"] = args.regions
        result["inputs"] = {key: {"sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
                            for key, path in paths.items()}
        result["symbol"] = args.symbol
        result["target_listing_vma"] = listing["base"]
        print(json.dumps(result, indent=2))
        return 0
    except (ValueError, OSError, KeyError, TypeError, subprocess.SubprocessError) as error:
        print(json.dumps({"diagnostic_only": True, "matching_proof": False,
                          "status": "refused", "reason": str(error)}))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
