#!/usr/bin/env python3
"""Compare prepared C context without authorizing declaration adoption.

Inputs must be the initial actual compiler input and a saved candidate, not
raw canonical C versus preprocessed C. Only the requested function body may
change. This is a conservative syntactic contract, not a C equivalence proof.
Reports and source inputs are private, ignored runtime evidence.
"""
from __future__ import annotations

import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import sys

SCHEMA = "mickey-candidate-context-v1"
MAX_SOURCE_BYTES = 4 * 1024 * 1024
MAX_NODES = 100_000
MAX_DEPTH = 128
MAX_DECLARATIONS = 4096
MAX_CHANGES = 32
MAX_SNIPPET = 512
SYMBOL = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
LOCATION_MACROS = {"__LINE__", "__FILE__", "__BASE_FILE__", "__FILE_NAME__",
                   "__COUNTER__", "__DATE__", "__TIME__", "__TIMESTAMP__",
                   "__INCLUDE_LEVEL__"}
PRAGMA_OPERATORS = {"_Pragma", "__pragma"}
# Literals precede comments, so comment-looking text inside a string survives.
LEXICAL = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*', re.S)


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _disk_identity() -> dict:
    """Fingerprint the comparator and parser sources currently on disk."""
    result = {"schema": SCHEMA, "comparator_sha256": _sha(Path(__file__).read_bytes())}
    try:
        import pycparser
        package = Path(pycparser.__file__).parent
        files = [(p.relative_to(package).as_posix(), _sha(p.read_bytes()))
                 for p in sorted(package.rglob("*.py"))]
        result.update(parser="pycparser", parser_version=pycparser.__version__,
                      parser_sha256=_sha(json.dumps(files, separators=(",", ":")).encode()))
    except (ImportError, OSError, AttributeError) as error:
        result.update(parser="unavailable", error=type(error).__name__)
    return result


# Pin when these modules are loaded, not when a long-lived caller first asks
# for an identity. Otherwise cached code could claim subsequently edited files.
_LOADED_IDENTITY_DIGEST = _sha(json.dumps(_disk_identity(), sort_keys=True).encode())


def identity() -> dict:
    """Return identity only while the loaded implementation remains current."""
    current = _disk_identity()
    if _sha(json.dumps(current, sort_keys=True).encode()) != _LOADED_IDENTITY_DIGEST:
        raise RuntimeError("loaded comparator/parser changed on disk; restart the caller")
    return current


class ContextError(ValueError):
    pass


def _strip_comments(text: str) -> str:
    def replace(match: re.Match) -> str:
        token = match.group()
        if token.startswith(("/*", "//")):
            return " " + "\n" * token.count("\n")
        return token
    return LEXICAL.sub(replace, text)


def _inactive_macro_prelude(text: str, *, allow_active: bool = False) -> tuple[str, list[dict], list[str]]:
    """Retain unused leading definitions as context, never guess expansion.

    Prepared vendor input can carry unused graphics definitions, including
    token pasting. Their exact phase-2/comment-normalized logical lines remain
    ordered context evidence. Any possible use outside the prelude refuses the
    route, as do conditional preprocessing and definitions after C has begun.
    """
    rows, snippets, names = [], [], set()
    output = []
    leading = True
    # Only LF ends a preprocessing directive after CR normalization. Python's
    # splitlines also splits vertical tabs/form feeds, which could incorrectly
    # move a macro replacement token into the parsed declaration context.
    for line in re.findall(r"[^\n]*\n|[^\n]+$", text):
        # Ignore directive-looking text in literals, not actual directives.
        visible = LEXICAL.sub(lambda match: " " if match.group().startswith(('"', "'"))
                              else match.group(), line)
        if not visible.strip():
            output.append(line)
            continue
        if not re.match(r"\s*#", visible):
            leading = False
            output.append(line)
            continue
        if re.fullmatch(r'\s*#\s*(?:line\s+)?\d+(?:\s+"[^"\n]*")?(?:\s+\d+)*\s*', line):
            output.append(line)
            continue
        definition = re.fullmatch(r"\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)([^\n]*)\n?", line)
        if definition:
            if not leading:
                raise ContextError("macro definitions must be a leading prepared prelude")
            name, replacement = definition.groups()
            if name in names:
                raise ContextError("duplicate or redefined prepared macro")
            if replacement.startswith("("):
                parameters = re.match(r"\(([^)]*)\)", replacement)
                if parameters is None:
                    raise ContextError("malformed prepared macro parameters")
                arguments = [value.strip() for value in parameters[1].split(",")] if parameters[1].strip() else []
                if any(not SYMBOL.fullmatch(value) for value in arguments) or len(set(arguments)) != len(arguments):
                    raise ContextError("unsupported prepared macro parameters")
            elif replacement and not replacement[0].isspace():
                raise ContextError("malformed prepared macro definition")
            names.add(name)
            normalized = line.strip()
            rows.append({"kind": "MacroDefinition", "name": name,
                         "sha256": _sha(normalized.encode())})
            snippets.append(normalized[:MAX_SNIPPET])
            output.append("\n" if line.endswith("\n") else "")
            continue
        if re.match(r"\s*#\s*pragma\b", visible):
            leading = False
            output.append(line)
            continue
        raise ContextError("unsupported prepared preprocessing directive")
    prepared = "".join(output)
    without_literals = LEXICAL.sub(lambda match: " " if match.group().startswith(('"', "'"))
                                  else match.group(), prepared)
    if not allow_active and names.intersection(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", without_literals)):
        raise ContextError("active prepared macro requires independent preprocessing/context review")
    if len(rows) > MAX_DECLARATIONS:
        raise ContextError("prepared macro prelude exceeds declaration limit")
    return prepared, rows, snippets


def preprocessing_macro_context(source: bytes) -> list[dict]:
    """Validate and fingerprint definitions for an actual compiler -E replay.

    This never supplies C with its active definitions blanked. Only the stock
    preprocessor output may be passed to the ordinary context comparator.
    """
    text = _prepared_text(source)
    visible = LEXICAL.sub(lambda match: " " if match.group().startswith(('"', "'"))
                         else match.group(), text)
    if "##" in visible or "%:%:" in visible:
        raise ContextError("active token pasting requires independent expansion provenance")
    _text, rows, _snippets = _inactive_macro_prelude(text, allow_active=True)
    return rows


def _prepared_text(source: bytes) -> str:
    """Apply the shared conservative prepared-input lexical checks."""
    if len(source) > MAX_SOURCE_BYTES:
        raise ContextError("prepared source exceeds comparison byte limit")
    text = source.decode("utf-8")
    # Translation phase 1/2 precedes comment recognition. Trigraph behavior
    # depends on compiler mode; refuse it rather than apply the wrong dialect.
    if re.search(r"\?\?[=/'()!<>-]", text):
        raise ContextError("trigraph-bearing prepared input requires preprocessing")
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    if re.search(r"\\[^\S\n]+\n", text):
        raise ContextError("whitespace after continuation backslash is dialect-dependent")
    text = re.sub(r"\\\n", "", text)
    multiline_comment = any(match.group().startswith("/*") and "\n" in match.group()
                            for match in LEXICAL.finditer(text))
    text = _strip_comments(text)
    if multiline_comment and re.search(r"(?m)^\s*#\s*define\b", text):
        raise ContextError("multiline comments with macro definitions require preprocessing")
    without_literals = LEXICAL.sub(lambda match: " " if match.group().startswith(('"', "'"))
                                  else match.group(), text)
    if "/*" in without_literals or '"' in without_literals or "'" in without_literals:
        raise ContextError("unterminated prepared comment or literal")
    if any("\n" in match.group() for match in LEXICAL.finditer(text)
           if match.group().startswith(('"', "'"))):
        raise ContextError("unspliced newline in prepared literal")
    identifiers = set(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", without_literals))
    if LOCATION_MACROS.intersection(identifiers):
        raise ContextError("location/time-dependent macros must be expanded in prepared input")
    if PRAGMA_OPERATORS.intersection(identifiers):
        raise ContextError("pragma operators require independent context review")
    return text


def inactive_seed_prelude(source: bytes) -> tuple[str, list[str], list[dict]]:
    """Blank validated definitions without moving physical C coordinates.

    Definitions are returned separately for vendor latedefine AST carriage.
    This is not preprocessing: every possible active macro use still refuses.
    """
    normalized = _prepared_text(source)
    _body, rows, _snippets = _inactive_macro_prelude(normalized)
    physical = source.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
    if not rows:
        return physical, [], []
    if re.search(r"(?m)^\s*#\s*(?:line\b|\d|pragma\s+_permuter\b)", normalized):
        raise ContextError("seed macro source has line controls or reserved vendor pragmas")
    chunks, definitions = [], []
    for match in re.finditer(r"(?:[^\n]|(?<=\\)\n)*(?:\n|$)", physical):
        chunk = match.group()
        if not chunk:
            continue
        logical = re.sub(r"\\\n", "", chunk)
        if re.match(r"\s*#\s*define\b", logical):
            # Use the same comment-normalized logical text as context hashing,
            # not the abbreviated human-readable snippets.
            definition = re.sub(r"^\s*#\s*", "", _strip_comments(logical)).strip()
            definitions.append(definition)
            chunks.append(re.sub(r"[^\n]", " ", chunk))
        else:
            if "\\\n" in chunk:
                raise ContextError("seed C continuations require coordinate-aware preprocessing")
            # Keep physical columns as well as lines for source grouping.
            chunks.append(LEXICAL.sub(lambda m: re.sub(r"[^\n]", " ", m.group())
                                     if m.group().startswith(("/*", "//")) else m.group(), chunk))
    # Independently validate the complete recovered definitions and their order.
    recovered = "".join("#" + definition + "\n" for definition in definitions)
    _, recovered_rows, _ = _inactive_macro_prelude(recovered)
    if recovered_rows != rows:
        raise ContextError("seed macro prelude recovery changed definition context")
    return "".join(chunks), definitions, rows


def _surface(source: bytes, symbol: str) -> tuple[list[dict], list[str]]:
    import pycparser
    from pycparser import c_ast, c_generator
    text = _prepared_text(source)
    text, macro_rows, macro_snippets = _inactive_macro_prelude(text)
    # A fresh parser per input also prevents typedef state leaking between inputs.
    ast = pycparser.CParser().parse(text, filename="<prepared>")
    if len(ast.ext) + len(macro_rows) > MAX_DECLARATIONS:
        raise ContextError("prepared context exceeds declaration limit")
    targets = [node for node in ast.ext
               if isinstance(node, c_ast.FuncDef) and node.decl.name == symbol]
    if len(targets) != 1:
        raise ContextError("prepared source must define the requested function exactly once")
    target = targets[0]
    pending = [target.body]
    body_nodes = 0
    while pending:
        node = pending.pop()
        body_nodes += 1
        if body_nodes > MAX_NODES:
            raise ContextError("prepared body exceeds comparison structure limit")
        if isinstance(node, c_ast.Pragma):
            raise ContextError("target-body pragma cannot safely be excluded from context")
        pending.extend(child for _name, child in node.children())
    # Exclude only the body: signature and old-style parameter declarations
    # remain compiler input, even though the body is allowed to vary.
    target.body = c_ast.Compound(block_items=[])
    count = body_nodes

    def normalize(node, depth=0):
        nonlocal count
        count += 1
        if depth > MAX_DEPTH or count > MAX_NODES:
            raise ContextError("prepared context exceeds comparison structure limit")
        return {"kind": type(node).__name__,
                "attributes": {name: getattr(node, name) for name in node.attr_names},
                "children": [(name, normalize(child, depth + 1))
                             for name, child in node.children()]}

    generator = c_generator.CGenerator()
    rows, snippets = list(macro_rows), list(macro_snippets)
    for node in ast.ext:
        name = getattr(node, "name", None)
        if isinstance(node, c_ast.FuncDef):
            name = node.decl.name
        normalized = normalize(node)
        rows.append({"kind": type(node).__name__, "name": name,
                     "sha256": _sha(json.dumps(normalized, sort_keys=True,
                                               separators=(",", ":")).encode())})
        snippets.append(generator.visit(node)[:MAX_SNIPPET])
    return rows, snippets


def compare_context(baseline: bytes, winner: bytes, symbol: str) -> dict:
    """Return unchanged/changed/unverifiable; never silently assume equivalence.

    AST equality deliberately preserves declaration order, parameter names,
    qualifiers, pragmas and all non-target helper bodies. Conservative review
    flags are preferable to adopting an unproved context change. Coordinates
    (including preprocessor line markers), comments and formatting are ignored.
    """
    report = {"schema": SCHEMA, "status": "unverifiable", "symbol": symbol,
              "baseline_sha256": _sha(baseline), "winner_sha256": _sha(winner),
              "baseline_context_sha256": None, "winner_context_sha256": None,
              "changes": [], "truncated": False, "reason": None}
    if not isinstance(symbol, str) or not SYMBOL.fullmatch(symbol):
        report["reason"] = "invalid requested function name"
        return report
    try:
        identity()
        old, old_text = _surface(baseline, symbol)
        new, new_text = _surface(winner, symbol)
        for field, value in (("baseline_context_sha256", old), ("winner_context_sha256", new)):
            report[field] = _sha(json.dumps(value, sort_keys=True, separators=(",", ":")).encode())
        report["status"] = "unchanged" if old == new else "changed"
        if old == new:
            return report
        # autojunk=False, matching every other SequenceMatcher call in this
        # tree. The "popular element" heuristic switches on at 200 rows and
        # then refuses to anchor a match on any row occurring in more than 1%
        # of b, so an unrelated growth in context size can change how a given
        # change is reported. Rows here are content hashes, where "popular"
        # carries no signal about anchor quality, so the heuristic has nothing
        # to offer and a size-dependent behaviour change to lose. No output
        # difference was reproducible on this path (tried near-identical and
        # heavily-diverged surfaces with a deliberately popular row, both
        # byte-identical reports): find_longest_match extends blocks across
        # junk, which recovers what the heuristic skipped. This is a
        # consistency and determinism fix, not a repair of an observed fault.
        matcher = difflib.SequenceMatcher(a=[row["sha256"] for row in old],
                                         b=[row["sha256"] for row in new], autojunk=False)
        for operation, a, b, c, d in matcher.get_opcodes():
            if operation == "equal":
                continue
            for i in range(max(b - a, d - c)):
                if len(report["changes"]) >= MAX_CHANGES:
                    report["truncated"] = True
                    break
                before = {**old[a + i], "text": old_text[a + i]} if a + i < b else None
                after = {**new[c + i], "text": new_text[c + i]} if c + i < d else None
                report["changes"].append({"operation": operation, "before": before, "after": after})
        report["reason"] = "prepared context changed; independent ABI/source review required"
    except Exception as error:
        # Parser diagnostics can echo large source lines; keep the scalar
        # error bounded and leave original inputs in their evidence files.
        report["status"] = "unverifiable"
        report["reason"] = (f"{type(error).__name__}: {error}")[:512]
    return report


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("winner", type=Path)
    parser.add_argument("--symbol", required=True)
    args = parser.parse_args(argv)
    from sweep_receipts import owned_bytes
    try:
        report = compare_context(
            owned_bytes(args.baseline.parent, args.baseline.name, limit=MAX_SOURCE_BYTES),
            owned_bytes(args.winner.parent, args.winner.name, limit=MAX_SOURCE_BYTES), args.symbol)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print(json.dumps(report, indent=2, sort_keys=True))
    return {"unchanged": 0, "changed": 1, "unverifiable": 2}[report["status"]]


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
