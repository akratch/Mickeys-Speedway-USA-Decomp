"""Merge independently authenticated storage namespaces without hiding conflicts."""

import collections


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

