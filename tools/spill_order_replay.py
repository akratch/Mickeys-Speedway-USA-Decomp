#!/usr/bin/env python3
"""Validate an externally supplied uniform-cell spill-request order.

Input JSON schema spill-order-replay-v1 has requests (id, size, alignment,
regions), a complete order, and optional constraints (id, slot). All requests
must have the same size/alignment. Regions are authenticated externally;
intersection means simultaneous ownership. Slots are zero-based pool cells,
not frame offsets. This replays a witness, never searches for one or proves
source realizability, compiler fidelity, ordinary-temporary behavior or a match.
Exit 0: constraints satisfied; 1: valid witness violates constraints; 2: invalid.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

SCHEMA = "spill-order-replay-v1"


def integer(value, label, minimum=0):
    if type(value) is not int or value < minimum:
        raise ValueError(f"{label} must be an integer >= {minimum}")
    return value


def keys(value, required, optional=()):
    if not isinstance(value, dict) or not set(required) <= value.keys():
        raise ValueError("object is missing required fields")
    if value.keys() - set(required) - set(optional):
        raise ValueError("object has unknown fields")


def unique_integers(value, label):
    if not isinstance(value, list):
        raise ValueError(f"{label} must be a list")
    result = [integer(item, label) for item in value]
    if len(set(result)) != len(result):
        raise ValueError(f"{label} contains duplicates")
    return result


def no_duplicate_keys(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON field: {key}")
        result[key] = value
    return result


def replay(document):
    keys(document, ("schema", "requests", "order"), ("constraints",))
    if document["schema"] != SCHEMA:
        raise ValueError("unsupported schema")
    rows = document["requests"]
    if not isinstance(rows, list) or not rows:
        raise ValueError("requests must be a nonempty list")
    requests = {}
    geometry = set()
    for row in rows:
        keys(row, ("id", "size", "alignment", "regions"))
        ident = integer(row["id"], "request id")
        if ident in requests:
            raise ValueError("duplicate request id")
        size = integer(row["size"], "size", 1)
        alignment = integer(row["alignment"], "alignment", 1)
        if alignment & (alignment - 1) or size % alignment:
            raise ValueError("alignment must be a power of two dividing size")
        geometry.add((size, alignment))
        requests[ident] = set(unique_integers(row["regions"], "regions"))
    if len(geometry) != 1:
        raise ValueError("mixed request size/alignment is unsupported")
    size, alignment = next(iter(geometry))
    order = unique_integers(document["order"], "order")
    if set(order) != requests.keys():
        raise ValueError("order must contain every request exactly once")
    constraints = document.get("constraints", [])
    if not isinstance(constraints, list):
        raise ValueError("constraints must be a list")
    targets = {}
    for row in constraints:
        keys(row, ("id", "slot"))
        ident = integer(row["id"], "constraint id")
        slot = integer(row["slot"], "constraint slot")
        if ident not in requests or ident in targets:
            raise ValueError("unknown or duplicate constrained request")
        targets[ident] = slot

    slots, steps = {}, []
    for ident in order:
        blockers = {}
        for prior, slot in slots.items():
            if requests[ident] & requests[prior]:
                blockers.setdefault(slot, []).append(prior)
        chosen = 0
        while chosen in blockers:
            chosen += 1
        slots[ident] = chosen
        steps.append({"id": ident, "slot": chosen, "blockers": [
            {"slot": slot, "owners": sorted(owners)}
            for slot, owners in sorted(blockers.items())]})
    # Independent check of every conflicting pair in the completed assignment.
    for index, ident in enumerate(order):
        for other in order[:index]:
            if requests[ident] & requests[other] and slots[ident] == slots[other]:
                raise AssertionError("overlapping requests share a cell")
    memberships = {}
    for ident, regions in requests.items():
        for node in regions:
            memberships.setdefault(node, []).append(ident)
    node = min(memberships, key=lambda n: (-len(memberships[n]), n)) if memberships else None
    clique = sorted(memberships[node]) if node is not None else []
    violations = [{"id": ident, "expected": slot, "actual": slots[ident]}
                  for ident, slot in sorted(targets.items()) if slots[ident] != slot]
    pool_slots = max(slots.values()) + 1
    return {"schema": "spill-order-replay-result-v1", "diagnostic_only": True,
            "constraints_satisfied": not violations, "violations": violations,
            "size": size, "alignment": alignment, "pool_slots": pool_slots,
            "pool_bytes": pool_slots * size, "steps": steps,
            "shared_node_lower_bound": {"node": node, "requests": clique,
                                        "slots": len(clique)}}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="external JSON request witness")
    args = parser.parse_args(argv)
    try:
        raw = args.input.read_bytes()
        document = json.loads(raw, object_pairs_hook=no_duplicate_keys)
        result = replay(document)
    except (OSError, ValueError) as error:
        print(f"spill-order-replay: {error}", file=sys.stderr)
        return 2
    result["input_sha256"] = hashlib.sha256(raw).hexdigest()
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["constraints_satisfied"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
