#!/usr/bin/env python3
"""Safely preserve one bounded NON_MATCHING plateau.

The command records concise measured evidence, runs source-only repository
gates, and commits only when --commit is supplied.  It never reads or records
target instruction rows.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import subprocess
import sys


SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
HANDOFF_SHARD_DIR = "docs/matching-triage-handoffs"
SCORE_RE = re.compile(r"^(?:\d+/\d+ (?:words|instructions|bytes)|\d+ differing words)$")
FRAME_RE = re.compile(r"^(?:-?0x[0-9A-Fa-f]+|frameless|unknown)$")
MISMATCH_RE = re.compile(r"^(?:\+?0x[0-9A-Fa-f]+|none|unknown)$")
DIRECTIVE_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")
LEGACY_HANDOFF_RE = re.compile(
    r"/\* PLATEAU-HANDOFF\n"
    r"(?: \* [^\n]*\n)*?"
    r" \*/\n",
)
KEYED_HANDOFF_RE = re.compile(
    r"/\* PLATEAU-HANDOFF:(?P<symbol>[A-Za-z_][A-Za-z0-9_]*):start\n"
    r"(?: \* [^\n]*\n)*?"
    r" \* PLATEAU-HANDOFF:(?P=symbol):end\n"
    r" \*/\n?",
)
GENERATED_OVERLAY_FALLBACK_RE = re.compile(
    r"^func_overlay_[0-9]{3}_F[0-9A-Fa-f]{7}_[0-9A-Fa-f]+\.s$"
)
OVERLAY_RULES_PATH = "mk/overlays.mk"
OBJECT_RULE_RE = re.compile(r"^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/(?P<source>\S+?\.c)\.o\s*:")
REDEFINE_SYM_RE = re.compile(
    r"--redefine-sym\s+(?P<old>[A-Za-z_][A-Za-z0-9_]*)=(?P<new>[A-Za-z_][A-Za-z0-9_]*)"
)
GENERATED_RESIDENT_FALLBACK_RE = re.compile(r"^func_8[0-9A-Fa-f]{7}\.s$")
DEFINITION_TEMPLATE = (
    r"^[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t*]+)+"
    r"{symbol}\s*\([^;{{}}]*\)\s*\{{"
)


class PlateauError(RuntimeError):
    """A concise refusal that leaves the candidate uncommitted."""


@dataclass(frozen=True)
class GuardedCandidate:
    ifdef_line: int
    else_line: int
    endif_line: int
    fallback: str


@dataclass(frozen=True)
class Metrics:
    score: str
    frame: str
    relocations: int
    first_mismatch: str
    summary: str = ""


def run_git(root: Path, *args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", *args], cwd=root, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "git command failed"
        raise PlateauError(f"git {' '.join(args)}: {detail}")
    return result


def repository_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise PlateauError("run this command inside a Git worktree")
    return Path(result.stdout.strip()).resolve()


def validate_one_line(value: str, label: str, limit: int = 160) -> str:
    """Enforce the shard grammar, and say which rule was broken.

    The single combined message ("must be one concise line without table
    separators") left a caller guessing whether the length or a character was
    at fault, and never named the limit. Each rule now reports itself.
    """
    if not value:
        raise PlateauError(f"{label} must not be empty")
    if len(value) > limit:
        raise PlateauError(
            f"{label} is {len(value)} characters; the limit is {limit}. "
            f"Put the longer argument in the shard's evidence prose, which "
            f"is free-form and is now retained across refreshes."
        )
    for character, name in (("\r", "carriage return"), ("\n", "newline"),
                            ("\t", "tab"), ("|", "'|' (the ledger's column separator)")):
        if character in value:
            raise PlateauError(f"{label} must be one line and may not contain a {name}")
    return " ".join(value.split())


def validate_metrics(args: argparse.Namespace) -> Metrics:
    score = validate_one_line(args.score, "score", 64)
    frame = validate_one_line(args.frame, "frame", 24)
    first_mismatch = validate_one_line(args.first_mismatch, "first mismatch", 24)
    if not SCORE_RE.fullmatch(score):
        raise PlateauError("score must look like '98/101 words' or '43 differing words'")
    if not FRAME_RE.fullmatch(frame):
        raise PlateauError("frame must be hexadecimal, 'frameless', or 'unknown'")
    if not MISMATCH_RE.fullmatch(first_mismatch):
        raise PlateauError("first mismatch must be a hexadecimal offset, 'none', or 'unknown'")
    if args.relocations < 0:
        raise PlateauError("relocations must be non-negative")
    summary = validate_one_line(args.summary, "summary") if args.summary else ""
    return Metrics(score, frame, args.relocations, first_mismatch, summary)


def directive(line: str) -> tuple[str, str] | None:
    match = DIRECTIVE_RE.match(line)
    if not match:
        return None
    return match.group(1), match.group(2).strip()


def fallback_aliases(rules_text: str | None) -> dict[tuple[str, str], frozenset[str]]:
    """Map (source path, built symbol) to the splat names the build renames to it.

    A renamed overlay function keeps splat's generated fallback file:
    ``overlay20UpdateGrid`` falls back to
    ``.../overlay20UpdateGrid/func_overlay_020_F0000A68_1877040.s``, and the
    build turns that name into the friendly one with an ``objcopy
    --redefine-sym generated=friendly`` in the object's POSTPROCESS rule in
    mk/overlays.mk. That rule is the build's own record of which fallback file
    is which symbol, so it is the mapping read here, not the symbol's spelling.
    Paths are repository-relative (``src/...``).
    """
    found: dict[tuple[str, str], set[str]] = {}
    if not rules_text:
        return {}
    logical = re.sub(r"\\\n", " ", rules_text)
    for line in logical.splitlines():
        rule = OBJECT_RULE_RE.match(line)
        if rule is None:
            continue
        source = "src/" + rule.group("source")
        for pair in REDEFINE_SYM_RE.finditer(line):
            found.setdefault((source, pair.group("new")), set()).add(pair.group("old"))
    return {key: frozenset(value) for key, value in found.items()}


def fallback_names_symbol(
    path: str, symbol: str, aliases: frozenset[str] | set[str] = frozenset(),
) -> bool:
    """True when one GLOBAL_ASM path is this symbol's own fallback.

    Accepted, in order: the file is ``<symbol>.s`` (resident renames and
    ``<symbol>/<symbol>.s`` per-function directories alike); the file is a
    splat name the build redefines to ``symbol`` (``aliases``, from
    `fallback_aliases`); or the file carries a splat-generated name, overlay
    (``func_overlay_NNN_...``) or resident (``func_8XXXXXXX``). The last is
    what a candidate written under a descriptive C name looks like when the
    ROM symbol was never renamed (``MatrixMultiplyVec4`` over
    ``func_8002AF6C.s``) or was renamed without a redefine rule; the guard's
    own structure (exactly one definition, exactly one fallback) is then the
    pairing, as it is for tools/progress.py.
    """
    name = Path(path).name
    if name == f"{symbol}.s":
        return True
    if name.endswith(".s") and name[:-2] in aliases:
        return True
    return bool(
        GENERATED_OVERLAY_FALLBACK_RE.fullmatch(name)
        or GENERATED_RESIDENT_FALLBACK_RE.fullmatch(name)
    )


def guarded_candidates(
    text: str, symbol: str, aliases: frozenset[str] | set[str] = frozenset(),
) -> list[GuardedCandidate]:
    lines = text.splitlines(keepends=True)
    found: list[GuardedCandidate] = []
    definition = re.compile(
        DEFINITION_TEMPLATE.format(symbol=re.escape(symbol)),
        re.DOTALL | re.MULTILINE,
    )
    fallback_re = re.compile(r'#\s*pragma\s+GLOBAL_ASM\s*\(\s*"([^"]+)"\s*\)')

    depth_before: list[int] = []
    file_depth = 0
    for line in lines:
        depth_before.append(file_depth)
        parsed = directive(line)
        if not parsed:
            continue
        kind, _ = parsed
        if kind in ("if", "ifdef", "ifndef"):
            file_depth += 1
        elif kind == "endif" and file_depth > 0:
            file_depth -= 1

    for start, line in enumerate(lines):
        parsed = directive(line)
        if parsed != ("ifdef", "NON_MATCHING"):
            continue
        depth = 1
        branch_lines: list[tuple[str, int]] = []
        end_line: int | None = None
        for index in range(start + 1, len(lines)):
            nested = directive(lines[index])
            if not nested:
                continue
            kind, _ = nested
            if kind in ("if", "ifdef", "ifndef"):
                depth += 1
            elif kind == "endif":
                depth -= 1
                if depth == 0:
                    end_line = index
                    break
            elif kind in ("elif", "else") and depth == 1:
                branch_lines.append((kind, index))

        first_branch = branch_lines[0][1] if branch_lines else end_line
        candidate_end = first_branch if first_branch is not None else len(lines)
        candidate_text = "".join(lines[start + 1:candidate_end])
        if not definition.search(candidate_text):
            continue

        if end_line is None:
            raise PlateauError(
                f"unterminated target NON_MATCHING guard for {symbol} after line {start + 1}"
            )
        if depth_before[start] != 0:
            raise PlateauError(f"nested target NON_MATCHING guard for {symbol} at line {start + 1}")
        if any(kind == "elif" for kind, _ in branch_lines):
            raise PlateauError(f"ambiguous target NON_MATCHING branches for {symbol}")
        else_lines = [index for kind, index in branch_lines if kind == "else"]
        if len(else_lines) != 1:
            raise PlateauError(
                f"{symbol} target NON_MATCHING guard must have exactly one top-level #else"
            )
        else_line = else_lines[0]
        fallback_text = "".join(lines[else_line + 1:end_line])
        fallbacks = fallback_re.findall(fallback_text)
        valid = [
            path for path in fallbacks
            if fallback_names_symbol(path, symbol, aliases)
        ]
        if len(fallbacks) != 1 or len(valid) != 1:
            raise PlateauError(
                f"{symbol} must have exactly one matching or generated-overlay "
                "#pragma GLOBAL_ASM fallback"
            )
        found.append(GuardedCandidate(start, else_line, end_line, valid[0]))
    return found


def defined_without_fallback(
    text: str, symbol: str, aliases: frozenset[str] | set[str] = frozenset(),
) -> bool:
    """True when the symbol is defined and has no GLOBAL_ASM fallback of its own.

    Deliberately NOT called "is matched". From the file's text alone a matched
    function and a candidate that was never wrapped in a guard are identical:
    both are a plain definition with no fallback. This reports the observation;
    the caller offers both readings rather than picking one.
    """
    definition = re.compile(
        DEFINITION_TEMPLATE.format(symbol=re.escape(symbol)),
        re.DOTALL | re.MULTILINE,
    )
    if not definition.search(text):
        return False
    fallbacks = re.findall(r'#\s*pragma\s+GLOBAL_ASM\s*\(\s*"([^"]+)"\s*\)', text)
    return not any(
        Path(path).name == f"{symbol}.s"
        or (path.endswith(".s") and Path(path).name[:-2] in aliases)
        for path in fallbacks
    )


def require_guarded_candidate(
    text: str, symbol: str, aliases: frozenset[str] | set[str] = frozenset(),
) -> GuardedCandidate:
    candidates = guarded_candidates(text, symbol, aliases)
    if len(candidates) != 1:
        if not candidates:
            # Keep the original sentence -- callers and tests rely on it --
            # and append what the shape of the file narrows the cause to.
            # The frequent cause is a merge that kept a plateau record from a
            # branch predating the function's match: both sides edit different
            # regions, so the merge reports no conflict and only this audit
            # sees the result is inconsistent. It has landed twice.
            detail = ""
            if defined_without_fallback(text, symbol, aliases):
                detail = (
                    f"; {symbol} is defined here with no GLOBAL_ASM fallback of"
                    " its own, so either it is already matched and this"
                    " PLATEAU-HANDOFF block is dead and should be removed"
                    " (usually a merge that kept a plateau record from a branch"
                    " predating the match), or it is a candidate that was never"
                    " wrapped in #ifdef NON_MATCHING. The file cannot tell those"
                    " apart; check whether the function still appears in the"
                    " NON_MATCHING queue"
                )
            raise PlateauError(
                f"{symbol} is not an unambiguous #ifdef NON_MATCHING candidate "
                f"with a GLOBAL_ASM fallback{detail}"
            )
        raise PlateauError(f"{symbol} appears in more than one guarded candidate")
    return candidates[0]


def source_handoff(symbol: str, metrics: Metrics) -> str:
    fields = [
        f"symbol: {symbol}",
        f"score: {metrics.score}",
        f"frame: {metrics.frame}",
        f"relocations: {metrics.relocations}",
        f"first-mismatch: {metrics.first_mismatch}",
    ]
    if metrics.summary:
        fields.append(f"summary: {metrics.summary}")
    return (
        f"/* PLATEAU-HANDOFF:{symbol}:start\n"
        + "".join(f" * {field}\n" for field in fields)
        + f" * PLATEAU-HANDOFF:{symbol}:end\n"
        + " */\n"
    )


def update_source(text: str, symbol: str, handoff: str) -> str:
    if LEGACY_HANDOFF_RE.search(text):
        raise PlateauError(
            "legacy inline PLATEAU-HANDOFF would move measured source lines; "
            "migrate it manually and re-prove the candidate"
        )

    blocks = list(KEYED_HANDOFF_RE.finditer(text))
    marker_count = text.count("PLATEAU-HANDOFF:")
    if marker_count != 2 * len(blocks):
        raise PlateauError("malformed symbol-keyed PLATEAU-HANDOFF metadata")
    if blocks:
        metadata_start = blocks[0].start()
        remainder = KEYED_HANDOFF_RE.sub("", text[metadata_start:])
        if remainder.strip():
            raise PlateauError("PLATEAU-HANDOFF metadata must be a contiguous EOF suffix")

    owned = [block for block in blocks if block.group("symbol") == symbol]
    if len(owned) > 1:
        raise PlateauError(f"duplicate PLATEAU-HANDOFF metadata for {symbol}")
    if owned:
        block = owned[0]
        return text[:block.start()] + handoff + text[block.end():]

    if not text or text.endswith("\n\n"):
        separator = ""
    elif text.endswith("\n"):
        separator = "\n"
    else:
        separator = "\n\n"
    return text + separator + handoff


def markdown_handoff(symbol: str, source: str, metrics: Metrics) -> str:
    marker = f"plateau-handoff:{symbol}"
    rows = [
        f"<!-- {marker}:start -->",
        f"### `{symbol}` plateau handoff",
        "",
        f"- source: `{source}`",
        f"- score: {metrics.score}",
        f"- frame: {metrics.frame}",
        f"- relocations: {metrics.relocations}",
        f"- first mismatch: {metrics.first_mismatch}",
    ]
    if metrics.summary:
        rows.append(f"- summary: {metrics.summary}")
    rows.extend((f"<!-- {marker}:end -->", ""))
    return "\n".join(rows)


def handoff_shard_path(symbol: str) -> str:
    """Return the fixed, traversal-safe ledger path for one exact symbol."""
    if not SYMBOL_RE.fullmatch(symbol):
        raise PlateauError(f"invalid exact symbol {symbol!r}")
    return f"{HANDOFF_SHARD_DIR}/{symbol}.md"


def shard_pattern(symbol: str) -> re.Pattern[str]:
    """The rigid metric header, plus the free-form evidence a worker appends."""
    marker = re.escape(f"plateau-handoff:{symbol}")
    return re.compile(
        rf"\A<!-- {marker}:start -->\n"
        rf"### `{re.escape(symbol)}` plateau handoff\n\n"
        r"- source: `(?P<source>src/[A-Za-z0-9_./-]+\.c)`\n"
        r"- score: [^\n|]+\n"
        r"- frame: [^\n|]+\n"
        r"- relocations: [0-9]+\n"
        r"- first mismatch: [^\n|]+\n"
        r"(?:- summary: [^\n|]+\n)?"
        r"(?P<details>(?:[^\r\n|]*\n)*)"
        rf"<!-- {marker}:end -->\n?\Z"
    )


def parse_shard(text: str, symbol: str) -> tuple[str, str]:
    """Return one shard's (exact source identity, appended evidence).

    The generated metric header stays deliberately rigid so scheduling can
    trust the source identity and bounded-result fields.  A worker may append
    richer, symbol-specific evidence before the closing marker; forcing that
    useful evidence into a separate document made otherwise valid handoffs
    look foreign to ``lane_status``.
    """
    match = shard_pattern(symbol).fullmatch(text)
    if match is None:
        # shard_rejection_reason names the rule that was broken. Raising the
        # flat string here left it unreached on this path, which is the one
        # `finalize_plateau --commit` takes -- so a worker whose evidence
        # table contains a `|` still saw only "malformed or foreign".
        raise PlateauError(shard_rejection_reason(text, symbol))
    details = match.group("details")
    if "plateau-handoff:" in details:
        raise PlateauError(
            f"shard for {symbol} contains a second plateau-handoff marker in "
            f"its evidence; one shard owns exactly one symbol"
        )
    source = match.group("source")
    if any(part in {".", ".."} for part in Path(source).parts):
        raise PlateauError(f"non-canonical source path in handoff shard for {symbol}")
    return source, details


def shard_rejection_reason(text: str, symbol: str) -> str:
    """Say WHY a shard failed the grammar, not merely that it did.

    "malformed or foreign symbol handoff shard" describes every failure
    identically, and the most common one in practice is invisible: a `|`
    anywhere in the free-form evidence. Workers write measurement tables --
    it is the clearest way to present a before/after -- and markdown tables
    are made of pipes, so a good shard is rejected with a message that reads
    like the symbol is wrong. That cost a lane a cycle on 2026-09-09.
    """
    marker = f"plateau-handoff:{symbol}"
    start, end = f"<!-- {marker}:start -->", f"<!-- {marker}:end -->"
    if start not in text or end not in text:
        return (
            f"shard for {symbol} is missing its {'start' if start not in text else 'end'} "
            f"marker ({start if start not in text else end})"
        )
    if text.count(start) != 1 or text.count(end) != 1:
        return f"shard for {symbol} repeats its start or end marker"
    body = text.split(start, 1)[1].split(end, 1)[0]
    if "|" in body:
        lines = [
            number for number, line in enumerate(body.splitlines(), 1) if "|" in line
        ]
        return (
            f"shard for {symbol} contains '|' on line(s) "
            f"{', '.join(str(n) for n in lines[:5])} of its block. The grammar "
            f"forbids the ledger's column separator anywhere in a shard, so a "
            f"markdown table invalidates it; write the measurements as prose "
            f"or an indented list"
        )
    if "\r" in body:
        return f"shard for {symbol} contains a carriage return; write LF endings"
    return (
        f"malformed or foreign symbol handoff shard for {symbol}: the metric "
        f"header must be source, score, frame, relocations, first mismatch, "
        f"then an optional summary, each on its own line"
    )


def handoff_shard_source(text: str, symbol: str) -> str:
    """Validate one symbol-owned shard and return its exact source identity."""
    return parse_shard(text, symbol)[0]


def update_handoff_shard(text: str, symbol: str, block: str) -> str:
    """Refresh a shard's measured header, keeping the evidence under it.

    The header is regenerated because it IS the measurement and every field
    in it has just been re-derived. The prose below it is not: it is the
    record of what a worker eliminated, and it is often the most expensive
    thing in the shard. Returning the bare new block discarded 84 lines of
    committed evidence from overlay1FindBestRecord.md on 2026-09-09, and the
    only reason it was noticed is that the worker read the diff.
    """
    if not text:
        handoff_shard_source(block, symbol)
        return block
    _source, retained = parse_shard(text, symbol)
    handoff_shard_source(block, symbol)
    if not retained.strip():
        return block
    end = f"<!-- plateau-handoff:{symbol}:end -->"
    head, separator, tail = block.rpartition(end)
    if not separator:
        raise PlateauError(f"generated handoff shard for {symbol} has no end marker")
    merged = head + retained + separator + tail
    # The result must still satisfy the same grammar the reader enforces.
    parse_shard(merged, symbol)
    return merged


def update_markdown(text: str, symbol: str, block: str) -> str:
    marker = re.escape(f"plateau-handoff:{symbol}")
    pattern = re.compile(
        rf"<!-- {marker}:start -->.*?<!-- {marker}:end -->\n?",
        re.DOTALL,
    )
    if pattern.search(text):
        return pattern.sub(block, text, count=1)
    return text.rstrip() + "\n\n" + block


def changed_paths(root: Path) -> set[str]:
    paths: set[str] = set()
    for args in (
        ("diff", "--name-only"),
        ("diff", "--cached", "--name-only"),
        ("ls-files", "--others", "--exclude-standard"),
    ):
        output = run_git(root, *args).stdout
        paths.update(line for line in output.splitlines() if line)
    return paths


def require_only_allowed_dirt(root: Path, allowed: set[str]) -> None:
    unrelated = sorted(changed_paths(root) - allowed)
    if unrelated:
        raise PlateauError("unrelated worktree/index dirt: " + ", ".join(unrelated))


def tracked_relative_path(root: Path, value: str, kind: str, suffix: str) -> tuple[Path, str]:
    path = (root / value).resolve()
    try:
        relative = path.relative_to(root).as_posix()
    except ValueError as error:
        raise PlateauError(f"{kind} must stay inside this worktree") from error
    if path.suffix != suffix or not path.is_file():
        raise PlateauError(f"{kind} must be an existing {suffix} file: {relative}")
    if run_git(root, "ls-files", "--error-unmatch", relative, check=False).returncode != 0:
        raise PlateauError(f"{kind} must already be tracked: {relative}")
    return path, relative


def default_handoff_path(root: Path, symbol: str) -> tuple[Path, str]:
    """Resolve the one creatable default shard without widening write scope."""
    relative = handoff_shard_path(symbol)
    shard_dir = (root / HANDOFF_SHARD_DIR).resolve()
    expected_dir = root / HANDOFF_SHARD_DIR
    if shard_dir != expected_dir or not shard_dir.is_dir():
        raise PlateauError(
            f"default handoff shard directory is missing or unsafe: {HANDOFF_SHARD_DIR}"
        )
    unresolved = root / relative
    if unresolved.is_symlink():
        raise PlateauError(f"handoff shard must not be a symlink: {relative}")
    path = unresolved.resolve()
    if path.parent != shard_dir:
        raise PlateauError("default handoff shard escaped its fixed directory")
    if path.exists() and not path.is_file():
        raise PlateauError(f"handoff shard must be a regular Markdown file: {relative}")
    return path, relative


def run_source_gates(root: Path) -> None:
    for target in ("cleanroom", "check-docs"):
        result = subprocess.run(["gmake", target], cwd=root)
        if result.returncode != 0:
            raise PlateauError(f"source-only gate failed: gmake {target}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol", help="Exact C/assembly symbol")
    parser.add_argument("source", help="Tracked source file containing the guarded candidate")
    parser.add_argument("--score", required=True, help="Measured score, e.g. '98/101 words'")
    parser.add_argument("--frame", required=True, help="Measured frame, e.g. 0x8 or frameless")
    parser.add_argument("--relocations", required=True, type=int, help="Measured relocation count")
    parser.add_argument("--first-mismatch", required=True, help="Measured offset, e.g. +0xC")
    parser.add_argument("--summary", default="", help="One-line blocker or next lever")
    parser.add_argument(
        "--handoff-doc",
        default=None,
        help=(
            "existing tracked Markdown handoff ledger; by default write the "
            f"symbol-owned {HANDOFF_SHARD_DIR}/<symbol>.md shard"
        ),
    )
    parser.add_argument("--commit", action="store_true", help="Commit only the source and handoff doc")
    parser.add_argument("--message", help="Commit subject; requires --commit")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    try:
        if not SYMBOL_RE.fullmatch(args.symbol):
            raise PlateauError(f"invalid exact symbol {args.symbol!r}")
        if args.message and not args.commit:
            raise PlateauError("--message requires --commit")
        metrics = validate_metrics(args)
        root = repository_root()
        source_path, source_rel = tracked_relative_path(root, args.source, "source", ".c")
        if not source_rel.startswith("src/"):
            raise PlateauError("source must be under src/")
        custom_handoff = args.handoff_doc is not None
        if custom_handoff:
            doc_path, doc_rel = tracked_relative_path(
                root, args.handoff_doc, "handoff document", ".md"
            )
            if not doc_rel.startswith("docs/"):
                raise PlateauError("handoff document must be under docs/")
            if doc_rel.startswith(f"{HANDOFF_SHARD_DIR}/"):
                raise PlateauError(
                    "the symbol handoff shard directory is reserved; omit "
                    "--handoff-doc to use the safe default"
                )
        else:
            doc_path, doc_rel = default_handoff_path(root, args.symbol)
        allowed = {source_rel, *([doc_rel] if doc_rel else [])}
        require_only_allowed_dirt(root, allowed)

        source_text = source_path.read_text(encoding="utf-8")
        rules = root / OVERLAY_RULES_PATH
        aliases = fallback_aliases(
            rules.read_text(encoding="utf-8") if rules.is_file() else None
        ).get((source_rel, args.symbol), frozenset())
        candidate = require_guarded_candidate(source_text, args.symbol, aliases)
        doc_text = doc_path.read_text(encoding="utf-8") if doc_path.exists() else ""
        block = markdown_handoff(args.symbol, source_rel, metrics)
        updated_doc = (
            update_markdown(doc_text, args.symbol, block)
            if custom_handoff
            else update_handoff_shard(doc_text, args.symbol, block)
        )
        source_path.write_text(
            update_source(source_text, args.symbol, source_handoff(args.symbol, metrics)),
            encoding="utf-8", newline="\n",
        )
        doc_path.write_text(updated_doc, encoding="utf-8", newline="\n")

        require_guarded_candidate(
            source_path.read_text(encoding="utf-8"), args.symbol, aliases,
        )
        require_only_allowed_dirt(root, allowed)
        run_source_gates(root)
        require_only_allowed_dirt(root, allowed)

        commit = "not requested"
        if args.commit:
            message = validate_one_line(args.message or f"Plateau {args.symbol}", "commit message", 100)
            run_git(root, "add", "--", *sorted(allowed))
            staged = set(run_git(root, "diff", "--cached", "--name-only").stdout.splitlines())
            if staged - allowed:
                raise PlateauError("refusing to commit unrelated staged paths")
            if not staged:
                commit = "unchanged"
            else:
                run_git(root, "commit", "-m", message)
                commit = run_git(root, "rev-parse", "HEAD").stdout.strip()

        print(f"symbol: {args.symbol}")
        print(f"source: {source_rel}")
        print(f"fallback: {candidate.fallback}")
        print(f"score: {metrics.score}")
        print(f"frame: {metrics.frame}")
        print(f"relocations: {metrics.relocations}")
        print(f"first-mismatch: {metrics.first_mismatch}")
        print(f"handoff-doc: {doc_rel}")
        print("source-only-gates: cleanroom, check-docs")
        print(f"commit: {commit}")
    except (OSError, PlateauError) as error:
        print(f"finalize-plateau: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
