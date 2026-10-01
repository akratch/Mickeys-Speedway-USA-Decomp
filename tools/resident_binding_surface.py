#!/usr/bin/env python3
"""Bind the three reviewed R8 storage carriers to fixed resident identities.

The adapter is deliberately limited to the actual R8 candidate ELF.  Physical
storage authority comes from resident_storage_bindings; this module checks the
candidate's own typed undefined symbols and complete REL pairs before exposing
those identities to the relocation resolver.
"""
from __future__ import annotations

import hashlib
import sys
from collections import defaultdict
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))
import reloc_surface as rs  # noqa: E402
import resident_storage_bindings as adapter  # noqa: E402
import source_symbol_fidelity as sf  # noqa: E402

FUNCTION = "func_overlay_008_F0001294_185EFEC"
SOURCE = "src/overlays/o008/overlay_008.c"
SOURCE_SHA256 = "cf75bc1886302a11dea0183468bca071d2fe7cca3dac8870748a052c4aa8b1a9"
EXPECTED = {
    "impact_gate": ("gO8P1294ImpactGateReloc", (0xFFD, 0x31A4), 1),
    "color_gate": ("gO8P1294ColorGateReloc", (0xFFD, 0x31AC), 1),
    "gravity": ("gO8P1294MotionScalarReloc", (0xFFF, 0x458C4), 4),
}


class BindingSurfaceError(RuntimeError):
    """The supplied candidate lacks the exact, independently bound surface."""


def _need(condition: bool, message: str) -> None:
    if not condition:
        raise BindingSurfaceError(message)


def _sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


_LOADED = {
    Path(__file__).resolve(): _sha(Path(__file__).resolve()),
    Path(rs.__file__).resolve(): _sha(Path(rs.__file__).resolve()),
    Path(adapter.__file__).resolve(): _sha(Path(adapter.__file__).resolve()),
    Path(sf.__file__).resolve(): _sha(Path(sf.__file__).resolve()),
}


def _verify_loaded():
    _need(all(path.is_file() and _sha(path) == digest
              for path, digest in _LOADED.items()),
          "resident binding surface implementation changed since import")
    adapter._verify_loaded()


def _candidate_carriers(candidate_object: Path, symbol: str, overlay: int,
                        source: str):
    root = adapter.ROOT.resolve()
    _need(symbol == FUNCTION and overlay == 8 and source == SOURCE,
          "fixed resident bindings are scoped to the reviewed R8 candidate")
    path = Path(candidate_object).resolve()
    _need(path.is_file() and path.is_relative_to(root),
          "candidate object is not an existing object in this checkout")
    source_path = root / SOURCE
    _need(_sha(source_path) == SOURCE_SHA256,
          "canonical R8 source changed from the reviewed binding context")

    start_sha = _sha(path)
    elf = rs.Elf(path)
    rows = [row for row in elf.symbols() if row[0] == FUNCTION]
    _need(len(rows) == 1, "candidate function symbol is absent or ambiguous")
    fn = rows[0]
    text_index, text_header = elf.section(".text")
    _need(text_index is not None and fn[4] == text_index and (fn[3] & 0xF) == rs.STT_FUNC,
          "candidate function is not a unique .text function")
    _need(fn[2] == 5036, "candidate owned function size differs from reviewed stock C")
    relocs = [(offset, kind, index) for section, offset, kind, index in elf.relocations()
              if section == ".text" and fn[1] <= offset < fn[1] + fn[2]]
    _need(len(relocs) == 137, "candidate function relocation count differs from reviewed stock C")
    symbols = elf.symbols()
    function = sf._function(elf, symbols, FUNCTION)
    ordered_graph = sf._relocations(
        elf, sf._symtab_index(elf), function, symbols)
    owned = elf.section_bytes(".text")[fn[1]:fn[1] + fn[2]]
    _need(len(owned) == fn[2], "candidate function byte range is truncated")
    by_name = defaultdict(list)
    for index, row in enumerate(symbols):
        by_name[row[0]].append((index, row))

    carrier_rows = {}
    for key, (name, _identity, width) in EXPECTED.items():
        named = by_name[name]
        _need(len(named) == 1, f"candidate carrier {name} is absent or duplicated")
        index, row = named[0]
        _need(row[2] == width and row[3] >> 4 == 1
              and row[3] & 0xF == rs.STT_OBJECT and row[4] == rs.SHN_UNDEF
              and row[1] == 0,
              f"candidate carrier {name} has the wrong ELF type, width, or binding")
        uses = [(offset, kind) for offset, kind, symbol_index in relocs
                if symbol_index == index]
        _need(len(uses) == 2 and sorted(kind for _, kind in uses) == [rs.R_MIPS_HI16, rs.R_MIPS_LO16],
              f"candidate carrier {name} is unused or lacks one complete HI16/LO16 pair")
        carrier_rows[name] = {"key": key, "symbol_index": index,
                              "relocations": [[offset, kind] for offset, kind in uses]}

    # Check REL pairing by symbol-table index, matching the linker's rule.
    pending = defaultdict(list)
    paired = defaultdict(int)
    # Preserve the actual REL table order.  A LO16 before its matching HI16
    # does not acquire authority by sorting sites after the fact.
    for offset, kind, index in relocs:
        if kind == rs.R_MIPS_HI16:
            pending[index].append(offset)
        elif kind == rs.R_MIPS_LO16:
            _need(bool(pending[index]), "candidate contains an unpaired LO16 relocation")
            pending[index].pop()
            paired[index] += 1
    _need(not any(pending.values()), "candidate contains an unpaired HI16 relocation")
    for name, record in carrier_rows.items():
        _need(paired[record["symbol_index"]] == 1,
              f"candidate carrier {name} does not have one complete ordered pair")
    _need(_sha(path) == start_sha, "candidate object changed while its carrier surface was parsed")
    return {"path": path, "sha256": start_sha, "source_path": source_path,
            "source_sha256": SOURCE_SHA256, "function_size": fn[2],
            "function_bytes_sha256": hashlib.sha256(owned).hexdigest(),
            "ordered_relocations": ordered_graph,
            "relocation_count": len(relocs), "carriers": carrier_rows}


def _authenticate_candidate_report(candidate, packet):
    stock = packet.get("candidate_stock_c", {})
    artifacts = packet.get("artifacts", {})
    configured_rel = artifacts.get("configured_object")
    raw_rel = artifacts.get("stock_c_object")
    _need(isinstance(configured_rel, str) and isinstance(raw_rel, str),
          "adapter did not retain both stock-C and configured candidate objects")
    configured = (adapter.ROOT / configured_rel).resolve()
    raw = (adapter.ROOT / raw_rel).resolve()
    _need(configured.is_file() and configured.is_relative_to(adapter.ROOT.resolve())
          and raw.is_file() and raw.is_relative_to(adapter.ROOT.resolve()),
          "adapter candidate object artifacts are unavailable or outside the checkout")
    _need(_sha(configured) == stock.get("configured_object_sha256") == candidate["sha256"],
          "supplied candidate is not the freshly authenticated configured stock-C object")
    _need(_sha(raw) == stock.get("object_sha256"),
          "retained stock-C raw object differs from its authenticated hash")
    _need(stock.get("function_size") == candidate["function_size"]
          and stock.get("function_bytes_sha256") == candidate["function_bytes_sha256"]
          and stock.get("ordered_relocations") == candidate["ordered_relocations"],
          "supplied candidate owned bytes or ordered named REL graph differs from stock C")
    fidelity = stock.get("source_fidelity", {})
    _need(fidelity.get("runtime_identity_proved") is False
          and fidelity.get("promotion_authority") is False,
          "adapter source-fidelity proof carried unexpected identity authority")
    context = stock.get("preprocessed_self_context", {})
    _need(context.get("status") == "unchanged"
          and stock.get("source_sha256") == candidate["source_sha256"]
          and isinstance(stock.get("recipe_fingerprint"), str),
          "adapter candidate compiler context/source is not the reviewed baseline")


def collect_candidate(candidate_object: Path, symbol: str, overlay: int, source: str):
    """Return independently witnessed identities and opaque final-check handles."""
    _verify_loaded()
    candidate = _candidate_carriers(candidate_object, symbol, overlay, source)
    bindings = {}
    reports = {}
    for key, (carrier, identity, width) in EXPECTED.items():
        spec = adapter.BINDINGS.get(key)
        _need(spec is not None and spec.get("carrier") == carrier
              and tuple(spec.get("identity", ())) == identity
              and spec.get("size") == width,
              "loaded fixed resident binding registry differs from reviewed scope")
        packet = adapter.collect(key)
        _need(packet.get("status") == "fixed-independent-binding-complete"
              and packet.get("resolver_admission") is False
              and packet.get("matching_credit") == 0
              and packet.get("carrier") == carrier
              and tuple(packet.get("original_identity", ())) == identity
              and packet.get("candidate_function") == FUNCTION
              and packet.get("candidate_source") == SOURCE,
              f"physical witness for {key} is incomplete or out of scope")
        _authenticate_candidate_report(candidate, packet)
        bindings[carrier] = identity
        reports[carrier] = {
            "key": key,
            "identity": list(identity),
            "recheck_handle": packet["recheck_handle"],
            "adapter_sha256": packet["adapter_sha256"],
            "candidate_stock_c": packet["candidate_stock_c"],
            "artifacts": packet["artifacts"],
        }
    _need(_sha(candidate["path"]) == candidate["sha256"]
          and _sha(candidate["source_path"]) == candidate["source_sha256"],
          "candidate or canonical source changed during physical witness collection")
    _verify_loaded()
    return {"schema": "mickey-r8-resident-binding-surface-v1",
            "candidate": {"path": candidate["path"].relative_to(adapter.ROOT).as_posix(),
                          "sha256": candidate["sha256"],
                          "source": SOURCE, "source_sha256": candidate["source_sha256"],
                          "function_size": candidate["function_size"],
                          "function_bytes_sha256": candidate["function_bytes_sha256"],
                          "ordered_relocations": candidate["ordered_relocations"],
                          "relocation_count": candidate["relocation_count"],
                          "carriers": candidate["carriers"]},
            "bindings": {name: list(identity) for name, identity in bindings.items()},
            "reports": reports,
            "recheck_handles": [reports[name]["recheck_handle"] for name, _, _ in EXPECTED.values()],
            "matching_credit": 0}


def recheck_candidate(receipt):
    """Recheck all fixed witnesses and the supplied candidate's terminal bytes."""
    _verify_loaded()
    _need(isinstance(receipt, dict)
          and receipt.get("schema") == "mickey-r8-resident-binding-surface-v1",
          "invalid internal resident-binding receipt")
    candidate = receipt.get("candidate", {})
    path = (adapter.ROOT / candidate.get("path", "")).resolve()
    _need(path.is_file() and path.is_relative_to(adapter.ROOT.resolve()),
          "candidate receipt path is not an owned object")
    expected_sha = candidate.get("sha256")
    _need(isinstance(expected_sha, str) and _sha(path) == expected_sha,
          "candidate object changed after binding capture")
    source = adapter.ROOT / SOURCE
    _need(candidate.get("source") == SOURCE and _sha(source) == SOURCE_SHA256
          and candidate.get("source_sha256") == SOURCE_SHA256,
          "canonical source changed after binding capture")
    for handle in receipt.get("recheck_handles", []):
        adapter.recheck(handle)
    _verify_loaded()
    _need(_sha(path) == expected_sha and _sha(source) == SOURCE_SHA256,
          "candidate object or source changed during final binding recheck")
    return True
