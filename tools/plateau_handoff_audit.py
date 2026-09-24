#!/usr/bin/env python3
"""Audit structured source plateau markers and reconcile their exact shards.

Only fields already present in tracked C source markers are projected.  The
tool never reads assembly, build products, a baserom, or the legacy prose
ledger when deciding what to write.
"""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys
import tempfile

import finalize_plateau


COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
MARKER_TOKEN_RE = re.compile(r"PLATEAU-HANDOFF")
KEYED_START_RE = re.compile(
    r"^/\* PLATEAU-HANDOFF:(?P<symbol>[A-Za-z_][A-Za-z0-9_]*):start$"
)
KEYED_END_RE = re.compile(
    r"^ \* PLATEAU-HANDOFF:(?P<symbol>[A-Za-z_][A-Za-z0-9_]*):end$"
)
FIELD_RE = re.compile(r"^ \* (?P<key>[a-z][a-z0-9-]*): (?P<value>[^\r\n]+)$")
DEFINITION_TEMPLATE = (
    r"^[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t*]+)+"
    r"{symbol}\s*\([^;{{}}]*\)\s*\{{"
)
REQUIRED_FIELDS = ("symbol", "score", "frame", "relocations", "first-mismatch")
ALLOWED_FIELDS = frozenset((*REQUIRED_FIELDS, "summary"))
SHARD_DIR = Path(finalize_plateau.HANDOFF_SHARD_DIR)


class AuditFailure(RuntimeError):
    """An operational failure rather than audit drift."""


@dataclass(frozen=True)
class MetricFields:
    score: str
    frame: str
    relocations: str
    first_mismatch: str
    summary: str = ""


@dataclass(frozen=True)
class Marker:
    symbol: str
    source: str
    line: int
    style: str
    metrics: MetricFields


@dataclass(frozen=True)
class Shard:
    symbol: str
    path: str
    source: str
    metrics: MetricFields
    details: str
    tracked: bool


@dataclass(frozen=True)
class Issue:
    code: str
    path: str
    message: str
    symbol: str | None = None
    line: int | None = None


@dataclass(frozen=True)
class Item:
    symbol: str
    source: str
    shard: str
    status: str
    newer: str = ""


@dataclass
class Audit:
    markers: list[Marker]
    shards: dict[str, Shard]
    items: list[Item]
    issues: list[Issue]
    unstructured: list[Issue]
    unbacked_shards: list[str]

    @property
    def actionable(self) -> list[Item]:
        return [item for item in self.items if item.status in {"missing", "stale"}]


def run_git(root: Path, *args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", *args], cwd=root, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "git command failed"
        raise AuditFailure(f"git {' '.join(args)}: {detail}")
    return result


def repository_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise AuditFailure("run this command inside a Git worktree")
    return Path(result.stdout.strip()).resolve()


def tracked_paths(root: Path) -> set[str]:
    output = run_git(root, "ls-files", "-z").stdout
    return {path for path in output.split("\0") if path}


def tracked_sources(root: Path, tracked: set[str]) -> dict[str, str]:
    sources: dict[str, str] = {}
    for relative in sorted(path for path in tracked if path.startswith("src/") and path.endswith(".c")):
        path = root / relative
        if path.is_symlink() or not path.is_file():
            raise AuditFailure(f"tracked source is not a regular file: {relative}")
        try:
            sources[relative] = path.read_text(encoding="utf-8")
        except UnicodeDecodeError as error:
            raise AuditFailure(f"tracked source is not UTF-8: {relative}") from error
    return sources


def issue(code: str, path: str, message: str, symbol: str | None = None,
          line: int | None = None) -> Issue:
    return Issue(code=code, path=path, message=message, symbol=symbol, line=line)


def parse_metrics(fields: dict[str, str], path: str, line: int) -> MetricFields:
    missing = [name for name in REQUIRED_FIELDS if name not in fields]
    if missing:
        raise ValueError("missing required fields: " + ", ".join(missing))
    unknown = sorted(set(fields) - ALLOWED_FIELDS)
    if unknown:
        raise ValueError("unknown fields: " + ", ".join(unknown))

    symbol = fields["symbol"]
    if not finalize_plateau.SYMBOL_RE.fullmatch(symbol):
        raise ValueError(f"invalid exact symbol {symbol!r}")
    for name, limit in (("score", 160), ("frame", 160), ("first-mismatch", 160)):
        value = fields[name]
        if finalize_plateau.validate_one_line(value, name, limit) != value:
            raise ValueError(f"{name} is not in canonical one-line form")
    if not re.fullmatch(r"[0-9]+", fields["relocations"]):
        raise ValueError("relocations must be a non-negative decimal integer")
    summary = fields.get("summary", "")
    if summary and finalize_plateau.validate_one_line(summary, "summary", 2048) != summary:
        raise ValueError("summary is not in canonical one-line form")
    return MetricFields(
        fields["score"], fields["frame"], fields["relocations"],
        fields["first-mismatch"], summary,
    )


def parse_field_lines(lines: list[str], path: str, line: int) -> tuple[str, MetricFields]:
    fields: dict[str, str] = {}
    for row in lines:
        if row == " *":
            continue
        match = FIELD_RE.fullmatch(row)
        if match is None:
            raise ValueError(f"non-field content in structured marker: {row!r}")
        key = match.group("key")
        if key in fields:
            raise ValueError(f"duplicate field: {key}")
        fields[key] = match.group("value")
    metrics = parse_metrics(fields, path, line)
    return fields["symbol"], metrics


def parse_source_markers(
    sources: dict[str, str],
) -> tuple[list[Marker], list[Issue], list[Issue]]:
    markers: list[Marker] = []
    issues: list[Issue] = []
    unstructured: list[Issue] = []

    for path, text in sources.items():
        covered_tokens: set[int] = set()
        for comment in COMMENT_RE.finditer(text):
            if "PLATEAU-HANDOFF" not in comment.group(0):
                continue
            block = comment.group(0)
            line = text.count("\n", 0, comment.start()) + 1
            for token in MARKER_TOKEN_RE.finditer(block):
                covered_tokens.add(comment.start() + token.start())
            lines = block.splitlines()
            first = lines[0] if lines else ""
            start = KEYED_START_RE.fullmatch(first)
            try:
                if start is not None:
                    if len(lines) < 4 or lines[-1] != " */":
                        raise ValueError("malformed symbol-keyed comment boundary")
                    end = KEYED_END_RE.fullmatch(lines[-2])
                    if end is None or end.group("symbol") != start.group("symbol"):
                        raise ValueError("missing or mismatched symbol-keyed end marker")
                    if len(MARKER_TOKEN_RE.findall(block)) != 2:
                        raise ValueError("symbol-keyed marker must contain exactly one start/end pair")
                    symbol, metrics = parse_field_lines(lines[1:-2], path, line)
                    if symbol != start.group("symbol"):
                        raise ValueError(
                            f"marker symbol {start.group('symbol')} disagrees with field {symbol}"
                        )
                    markers.append(Marker(symbol, path, line, "keyed", metrics))
                    continue

                if first == "/* PLATEAU-HANDOFF":
                    if not lines or lines[-1] != " */":
                        raise ValueError("malformed legacy comment boundary")
                    known_fields = [
                        match.group("key") for row in lines[1:-1]
                        if (match := FIELD_RE.fullmatch(row)) and match.group("key") in ALLOWED_FIELDS
                    ]
                    if not known_fields:
                        unstructured.append(issue(
                            "unstructured-marker", path,
                            "prose-only marker is not a metric source", line=line,
                        ))
                        continue
                    symbol, metrics = parse_field_lines(lines[1:-1], path, line)
                    markers.append(Marker(symbol, path, line, "legacy", metrics))
                    continue

                if ":start" in block or ":end" in block:
                    raise ValueError("malformed symbol-keyed marker")
                unstructured.append(issue(
                    "unstructured-marker", path,
                    "decorated prose marker is not a metric source", line=line,
                ))
            except (ValueError, finalize_plateau.PlateauError) as error:
                issues.append(issue("malformed-marker", path, str(error), line=line))

        token_positions = {match.start() for match in MARKER_TOKEN_RE.finditer(text)}
        for position in sorted(token_positions - covered_tokens):
            line = text.count("\n", 0, position) + 1
            issues.append(issue(
                "malformed-marker", path,
                "PLATEAU-HANDOFF token is outside a complete C block comment", line=line,
            ))

    by_symbol: dict[str, list[Marker]] = {}
    for marker in markers:
        by_symbol.setdefault(marker.symbol, []).append(marker)
    for symbol, owned in sorted(by_symbol.items()):
        if len(owned) > 1:
            locations = ", ".join(f"{row.source}:{row.line}" for row in owned)
            issues.append(issue(
                "duplicate-marker", owned[0].source,
                f"structured marker appears {len(owned)} times: {locations}", symbol,
            ))
    return markers, issues, unstructured


def definition_paths(sources: dict[str, str], symbol: str) -> list[str]:
    definition = re.compile(
        DEFINITION_TEMPLATE.format(symbol=re.escape(symbol)), re.DOTALL | re.MULTILINE,
    )
    return sorted(path for path, text in sources.items() if definition.search(text))


def validate_source_ownership(
    sources: dict[str, str], markers: list[Marker], issues: list[Issue],
) -> None:
    for marker in markers:
        paths = definition_paths(sources, marker.symbol)
        if paths != [marker.source]:
            detail = ", ".join(paths) if paths else "none"
            issues.append(issue(
                "source-ownership", marker.source,
                f"exact definitions are {detail}; expected only {marker.source}",
                marker.symbol, marker.line,
            ))
            continue
        try:
            finalize_plateau.require_guarded_candidate(
                sources[marker.source], marker.symbol,
            )
        except finalize_plateau.PlateauError as error:
            issues.append(issue(
                "source-ownership", marker.source, str(error),
                marker.symbol, marker.line,
            ))


def shard_pattern(symbol: str) -> re.Pattern[str]:
    marker = re.escape(f"plateau-handoff:{symbol}")
    return re.compile(
        rf"\A<!-- {marker}:start -->\n"
        rf"### `{re.escape(symbol)}` plateau handoff\n\n"
        r"- source: `(?P<source>src/[A-Za-z0-9_./-]+\.c)`\n"
        r"- score: (?P<score>[^\n|]+)\n"
        r"- frame: (?P<frame>[^\n|]+)\n"
        r"- relocations: (?P<relocations>[0-9]+)\n"
        r"- first mismatch: (?P<first_mismatch>[^\n|]+)\n"
        r"(?:- summary: (?P<summary>[^\n|]+)\n)?"
        r"(?P<details>(?:[^\r\n|]*\n)*)"
        rf"<!-- {marker}:end -->\n?\Z"
    )


def parse_shard(path: Path, relative: str, symbol: str, tracked: bool) -> Shard:
    if path.is_symlink() or not path.is_file():
        raise ValueError("shard is not a regular file")
    text = path.read_text(encoding="utf-8")
    finalize_plateau.handoff_shard_source(text, symbol)
    match = shard_pattern(symbol).fullmatch(text)
    if match is None:
        raise ValueError(finalize_plateau.shard_rejection_reason(text, symbol))
    fields = {
        "symbol": symbol,
        "score": match.group("score"),
        "frame": match.group("frame"),
        "relocations": match.group("relocations"),
        "first-mismatch": match.group("first_mismatch"),
    }
    if match.group("summary") is not None:
        fields["summary"] = match.group("summary")
    metrics = parse_metrics(fields, relative, 1)
    return Shard(
        symbol, relative, match.group("source"), metrics,
        match.group("details"), tracked,
    )


def discover_shards(
    root: Path, tracked: set[str], sources: dict[str, str], issues: list[Issue],
) -> dict[str, Shard]:
    directory = root / SHARD_DIR
    if directory.is_symlink() or not directory.is_dir():
        raise AuditFailure(f"missing or unsafe shard directory: {SHARD_DIR.as_posix()}")
    shards: dict[str, Shard] = {}
    for path in sorted(directory.glob("*.md")):
        if path.name == "README.md":
            continue
        relative = path.relative_to(root).as_posix()
        symbol = path.stem
        if not finalize_plateau.SYMBOL_RE.fullmatch(symbol):
            issues.append(issue("invalid-shard-name", relative, "filename is not an exact symbol"))
            continue
        try:
            shard = parse_shard(path, relative, symbol, relative in tracked)
        except (OSError, UnicodeDecodeError, ValueError, finalize_plateau.PlateauError) as error:
            issues.append(issue("malformed-shard", relative, str(error), symbol))
            continue
        paths = definition_paths(sources, symbol)
        if paths != [shard.source]:
            detail = ", ".join(paths) if paths else "none"
            issues.append(issue(
                "shard-source-ownership", relative,
                f"exact definitions are {detail}; shard records {shard.source}", symbol,
            ))
        shards[symbol] = shard
    return shards


def newer_handoff_side(root: Path, source_rel: str, shard_rel: str) -> str:
    """Which of a disagreeing pair was written last.

    ``source`` and ``shard`` are file mtimes. ``same`` means the metrics
    differ but neither file is newer, which a paired finalize does not
    produce.
    """
    source_ns = (root / source_rel).stat().st_mtime_ns
    shard_ns = (root / shard_rel).stat().st_mtime_ns
    if shard_ns > source_ns:
        return "shard"
    if source_ns > shard_ns:
        return "source"
    return "same"


def audit_tree(root: Path) -> Audit:
    tracked = tracked_paths(root)
    sources = tracked_sources(root, tracked)
    markers, issues, unstructured = parse_source_markers(sources)
    validate_source_ownership(sources, markers, issues)
    shards = discover_shards(root, tracked, sources, issues)

    duplicate_symbols = {
        marker.symbol for marker in markers
        if sum(row.symbol == marker.symbol for row in markers) > 1
    }
    invalid_marker_symbols = {
        row.symbol for row in issues
        if row.symbol is not None and row.code in {"duplicate-marker", "source-ownership"}
    }
    malformed_shards = {
        row.symbol for row in issues
        if row.symbol is not None and row.code in {"malformed-shard", "shard-source-ownership"}
    }
    items: list[Item] = []
    marker_symbols: set[str] = set()
    for marker in sorted(markers, key=lambda row: (row.symbol, row.source, row.line)):
        if marker.symbol in duplicate_symbols:
            continue
        marker_symbols.add(marker.symbol)
        shard_path = finalize_plateau.handoff_shard_path(marker.symbol)
        if marker.symbol in invalid_marker_symbols or marker.symbol in malformed_shards:
            status = "invalid"
        elif marker.symbol not in shards:
            status = "missing"
        else:
            shard = shards[marker.symbol]
            status = (
                "current"
                if shard.source == marker.source and shard.metrics == marker.metrics
                else "stale"
            )
        newer = (
            newer_handoff_side(root, marker.source, shard_path)
            if status == "stale" else ""
        )
        items.append(Item(marker.symbol, marker.source, shard_path, status, newer))

    unbacked = sorted(set(shards) - marker_symbols)
    return Audit(
        markers=sorted(markers, key=lambda row: (row.symbol, row.source, row.line)),
        shards=shards,
        items=items,
        issues=sorted(
            issues,
            key=lambda row: (row.path, row.line or 0, row.symbol or "", row.code, row.message),
        ),
        unstructured=sorted(
            unstructured, key=lambda row: (row.path, row.line or 0, row.message),
        ),
        unbacked_shards=unbacked,
    )


def regenerate_stale_shards(root: Path) -> list[str]:
    """Rewrite missing or stale shards from the source blocks in ``root``.

    The merged source is the authority. A lane's older shard is not kept.
    Unlike ``--write``, a shard the merge has staged is not refused: during
    a merge every shard is staged.
    """
    audit = audit_tree(root)
    if audit.issues:
        raise AuditFailure(
            "refusing to regenerate shards while source markers are invalid"
        )
    markers: dict[str, Marker] = {}
    for marker in audit.markers:
        markers.setdefault(marker.symbol, marker)
    written: list[str] = []
    for item in audit.actionable:
        marker = markers.get(item.symbol)
        if marker is None:
            continue
        existing = audit.shards.get(item.symbol)
        details = existing.details if existing is not None else ""
        path = root / item.shard
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(render_shard(marker, details), encoding="utf-8", newline="\n")
        written.append(item.shard)
    return written


def render_shard(marker: Marker, details: str = "") -> str:
    handoff = f"plateau-handoff:{marker.symbol}"
    rows = [
        f"<!-- {handoff}:start -->",
        f"### `{marker.symbol}` plateau handoff",
        "",
        f"- source: `{marker.source}`",
        f"- score: {marker.metrics.score}",
        f"- frame: {marker.metrics.frame}",
        f"- relocations: {marker.metrics.relocations}",
        f"- first mismatch: {marker.metrics.first_mismatch}",
    ]
    if marker.metrics.summary:
        rows.append(f"- summary: {marker.metrics.summary}")
    return "\n".join(rows) + "\n" + details + f"<!-- {handoff}:end -->\n"


def path_is_dirty(root: Path, relative: str) -> bool:
    result = run_git(root, "status", "--porcelain=v1", "--", relative)
    return bool(result.stdout)


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=False, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent,
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        if path.exists():
            mode = stat.S_IMODE(path.stat().st_mode)
            os.chmod(temporary, mode)
        else:
            os.chmod(temporary, 0o644)
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def write_actionable(root: Path, audit: Audit) -> list[str]:
    if audit.issues:
        raise AuditFailure("validation errors must be resolved before --write")
    markers = {marker.symbol: marker for marker in audit.markers}
    planned: list[tuple[Path, str]] = []
    for item in audit.actionable:
        path = root / item.shard
        existing = audit.shards.get(item.symbol)
        if path.is_symlink():
            raise AuditFailure(f"refusing symlink shard destination: {item.shard}")
        if path.exists() and existing is None:
            raise AuditFailure(f"refusing unvalidated shard destination: {item.shard}")
        if path.exists() and path_is_dirty(root, item.shard):
            raise AuditFailure(f"refusing locally modified shard: {item.shard}")
        details = existing.details if existing is not None else ""
        planned.append((path, render_shard(markers[item.symbol], details)))
    for path, text in planned:
        atomic_write(path, text)
    return [path.relative_to(root).as_posix() for path, _text in planned]


def result_dict(audit: Audit, mode: str, written: list[str]) -> dict[str, object]:
    status_counts = {
        status: sum(item.status == status for item in audit.items)
        for status in ("current", "missing", "stale", "invalid")
    }
    style_counts = {
        style: sum(marker.style == style for marker in audit.markers)
        for style in ("keyed", "legacy")
    }
    return {
        "mode": mode,
        "ok": not audit.issues and not audit.actionable,
        "markers": {
            "structured": len(audit.markers),
            "by_style": style_counts,
            "unstructured": len(audit.unstructured),
        },
        "shards": {
            **status_counts,
            "reconcilable": len(audit.actionable),
            "unbacked": len(audit.unbacked_shards),
            "written": len(written),
        },
        "items": [asdict(item) for item in audit.items],
        "issues": [asdict(row) for row in audit.issues],
        "unstructured_markers": [asdict(row) for row in audit.unstructured],
        "unbacked_shards": audit.unbacked_shards,
        "written_paths": written,
    }


def print_text(result: dict[str, object]) -> None:
    markers = result["markers"]
    shards = result["shards"]
    assert isinstance(markers, dict) and isinstance(shards, dict)
    print(
        "plateau-handoffs: "
        f"structured={markers['structured']} current={shards['current']} "
        f"missing={shards['missing']} stale={shards['stale']} "
        f"invalid={shards['invalid']} reconcilable={shards['reconcilable']} "
        f"unstructured={markers['unstructured']} unbacked={shards['unbacked']} "
        f"written={shards['written']} issues={len(result['issues'])}"
    )
    items = result["items"]
    assert isinstance(items, list)
    for status in ("missing", "stale", "invalid"):
        symbols = []
        for row in items:
            if row["status"] != status:
                continue
            if status == "stale" and row.get("newer"):
                side = row["newer"]
                label = "same age" if side == "same" else f"{side} newer"
                symbols.append(f"{row['symbol']} ({label})")
            else:
                symbols.append(row["symbol"])
        if symbols:
            print(f"{status}: " + ", ".join(symbols))
    for row in result["issues"]:
        location = row["path"] + (f":{row['line']}" if row["line"] else "")
        owned = f" [{row['symbol']}]" if row["symbol"] else ""
        print(f"error: {location}{owned}: {row['message']}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="Fail on missing/stale shards or invalid evidence")
    mode.add_argument("--write", action="store_true", help="Atomically write only valid missing/stale shards")
    parser.add_argument("--json", action="store_true", help="Emit deterministic JSON")
    return parser


def main() -> int:
    args = build_parser().parse_args()
    mode = "write" if args.write else "check"
    try:
        root = repository_root()
        audit = audit_tree(root)
        written: list[str] = []
        if args.write:
            written = write_actionable(root, audit)
            audit = audit_tree(root)
        result = result_dict(audit, mode, written)
        if args.json:
            print(json.dumps(result, indent=2, sort_keys=True))
        else:
            print_text(result)
        return 0 if result["ok"] else 1
    except (AuditFailure, OSError) as error:
        if args.json:
            print(json.dumps({"mode": mode, "ok": False, "error": str(error)}, sort_keys=True))
        else:
            print(f"plateau-handoff-audit: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
