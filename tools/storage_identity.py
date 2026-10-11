"""Merge independently authenticated storage namespaces without hiding conflicts."""

import collections


def capture_bss(rs, candidate, linked, source, overlay, enabled):
    """Explicit reviewed consumer scope; never infer a namespace from sites."""
    if not enabled or overlay != 64:
        return None
    import canonical_bss_view as view
    names = {row[0] for row in candidate.symbols()} & set(view.OWNER_NAMES)
    if not names:
        return None
    try:
        return view.collect(candidate, {name: 0xFFF for name in sorted(names)}, linked,
                            candidate_source=source)
    except view.BssViewError as error:
        raise rs.SurfaceComparisonError(str(error)) from error


def scoped_resident_identity(rs, name, identity, proof):
    """Interpret a proved physical address in its explicitly requested namespace.

    This is performed before any resident identity proposal is made. It does
    not delete an independent conflict later in the resolution graph.
    """
    if proof is None or name not in proof["identities"]:
        return identity
    physical = proof["receipt"]["physical_addresses"][name]
    reserved = tuple(proof["identities"][name])
    if (identity != (0, physical - rs.ot.RESIDENT_VRAM_BASE)
            or reserved[0] != 0xFFF or physical != 0x80085A40 + reserved[1]):
        raise rs.SurfaceComparisonError("reserved BSS namespace conflicts with physical address")
    return reserved


def merge_independent_resident_bindings(rs, imported, identities, ambiguous,
                                         evidence, values_path,
                                         redefine_aliases):
    """Resolve fixed resident witnesses together with every independent alias.

    Correlation-only proposals are intentionally excluded.  A real independent
    disagreement anywhere in an imported alias component remains a hard error.
    """
    ri = rs.ri
    SurfaceComparisonError = rs.SurfaceComparisonError
    proposed = collections.defaultdict(set)
    for name, rows in (evidence or {}).items():
        for row in rows:
            if row.get("independent") and row.get("base_identity") is not None:
                proposed[name].add(tuple(row["base_identity"]))
    for name, identity in imported.items():
        proposed[name].add(tuple(identity))
    equality_aliases = ri.parse_linker_aliases(values_path.read_text()) \
        if values_path.is_file() else []
    redefine_pairs = [(source, destination)
                      for destination, source in (redefine_aliases or {}).items()]
    try:
        resolved = ri.resolve_identities(
            proposed, equality_aliases=equality_aliases,
            redefine_aliases=redefine_pairs)
    except ri.RelocationIdentityError as error:
        raise SurfaceComparisonError(str(error)) from error

    graph = collections.defaultdict(set)
    for first, second in equality_aliases + redefine_pairs:
        graph[first].add(second)
        graph[second].add(first)
    affected = set(imported)
    pending = list(imported)
    while pending:
        for other in graph[pending.pop()]:
            if other not in affected:
                affected.add(other)
                pending.append(other)
    if affected.intersection(resolved.ambiguous):
        raise SurfaceComparisonError(
            "fixed resident binding conflicts with an independent alias identity")
    for name in affected:
        identity = resolved.resolved.get(name)
        if identity is None:
            continue
        direct_independent = {
            tuple(row["base_identity"])
            for row in (evidence or {}).get(name, [])
            if row.get("independent") and row.get("base_identity") is not None
        }
        if any(value != identity for value in direct_independent):
            raise SurfaceComparisonError(
                "fixed resident binding conflicts with an independent alias identity")
        existing = identities.get(name)
        if existing is not None and existing != identity and direct_independent:
            raise SurfaceComparisonError(
                "fixed resident binding conflicts with an independent resolved identity")
        identities[name] = identity
        # This can clear only ambiguity represented by discarded correlation
        # routes; all independent proposals participated in resolve_identities.
        ambiguous.discard(name)

