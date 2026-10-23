#!/usr/bin/env python3
"""Render pure overlay symbol-alias normalizations as a Make include."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import stat
import sys
import tempfile
from typing import Any


REPO = Path(__file__).resolve().parent.parent
DEFAULT_MANIFEST = (
    REPO / "config" / "normalizations" / "overlay-symbol-aliases.us.json"
)
DEFAULT_OUTPUT = REPO / "mk" / "overlay_aliases.generated.mk"

SYMBOL_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
OBJECT_RE = re.compile(
    r"overlays/o(?P<overlay>[0-9]{3})/[A-Za-z_][A-Za-z0-9_]*\.c\.o\Z"
)


class ManifestError(ValueError):
    """The alias manifest is malformed or ambiguous."""


class StaleOutputError(ValueError):
    """The checked-in Make include does not match the manifest."""


def _strict_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ManifestError(f"duplicate JSON key: {key!r}")
        result[key] = value
    return result


def _require_exact_keys(
    value: dict[str, Any], expected: set[str], location: str
) -> None:
    actual = set(value)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        details = []
        if missing:
            details.append(f"missing {missing}")
        if extra:
            details.append(f"unknown {extra}")
        raise ManifestError(f"{location} has invalid keys: {', '.join(details)}")


def _require_symbol(value: Any, location: str) -> str:
    if not isinstance(value, str) or SYMBOL_RE.fullmatch(value) is None:
        raise ManifestError(
            f"{location} must be a non-empty C-style symbol, got {value!r}"
        )
    return value


def load_manifest(path: Path, root: Path | None = None) -> list[dict[str, Any]]:
    """Load and fail-closed validate a version-one alias manifest."""
    try:
        document = json.loads(
            path.read_text(encoding="utf-8"), object_pairs_hook=_strict_object
        )
    except json.JSONDecodeError as exc:
        raise ManifestError(f"invalid JSON: {exc}") from exc

    if not isinstance(document, dict):
        raise ManifestError("manifest root must be an object")
    _require_exact_keys(document, {"schema_version", "rules"}, "manifest")
    if type(document["schema_version"]) is not int or document["schema_version"] != 1:
        raise ManifestError("schema_version must be the integer 1")

    rules = document["rules"]
    if not isinstance(rules, list) or not rules:
        raise ManifestError("rules must be a non-empty array")

    validated: list[dict[str, Any]] = []
    targets: set[str] = set()
    for rule_index, rule in enumerate(rules):
        location = f"rules[{rule_index}]"
        if not isinstance(rule, dict):
            raise ManifestError(f"{location} must be an object")
        _require_exact_keys(rule, {"object", "aliases"}, location)

        object_path = rule["object"]
        if not isinstance(object_path, str) or OBJECT_RE.fullmatch(object_path) is None:
            raise ManifestError(
                f"{location}.object must match overlays/oNNN/<name>.c.o"
            )
        if object_path in targets:
            raise ManifestError(f"duplicate object target: {object_path}")
        targets.add(object_path)

        aliases = rule["aliases"]
        if not isinstance(aliases, list) or not aliases:
            raise ManifestError(f"{location}.aliases must be a non-empty array")

        validated_aliases: list[dict[str, str]] = []
        sources: set[str] = set()
        destinations: set[str] = set()
        for alias_index, alias in enumerate(aliases):
            alias_location = f"{location}.aliases[{alias_index}]"
            if not isinstance(alias, dict):
                raise ManifestError(f"{alias_location} must be an object")
            _require_exact_keys(alias, {"from", "to"}, alias_location)
            source = _require_symbol(alias["from"], f"{alias_location}.from")
            destination = _require_symbol(alias["to"], f"{alias_location}.to")
            if source == destination:
                raise ManifestError(f"{alias_location} cannot rename a symbol to itself")
            if source in sources:
                raise ManifestError(
                    f"duplicate source symbol for {object_path}: {source}"
                )
            if destination in destinations:
                raise ManifestError(
                    f"duplicate destination symbol for {object_path}: {destination}"
                )
            sources.add(source)
            destinations.add(destination)
            validated_aliases.append({"from": source, "to": destination})

        overlap = sorted(sources & destinations)
        if overlap:
            raise ManifestError(
                f"chained aliases are not allowed for {object_path}: {overlap}"
            )
        validated.append({"object": object_path, "aliases": validated_aliases})

    root = REPO if root is None else root
    for rule in validated:
        source = root / "src" / rule["object"].removesuffix(".o")
        if not source.is_file():
            raise ManifestError(
                f"alias owner no longer exists: src/{rule['object'].removesuffix('.o')}"
            )
    return validated


def render_makefile(rules: list[dict[str, Any]]) -> str:
    """Return canonical Make syntax independent of manifest row order."""
    lines = [
        "# Generated by tools/render_overlay_aliases.py; do not edit.",
        "# Source: config/normalizations/overlay-symbol-aliases.us.json",
        "# Pure objcopy --redefine-sym POSTPROCESS rules only.",
        "",
    ]
    for rule in sorted(rules, key=lambda item: item["object"]):
        target = f"$(BUILD_DIR)/$(SRC_DIR)/{rule['object']}"
        aliases = sorted(
            rule["aliases"], key=lambda item: (item["from"], item["to"])
        )
        lines.append(f"{target}: POSTPROCESS = \\")
        if len(aliases) == 1:
            alias = aliases[0]
            lines.append(
                "\t$(OBJCOPY) --redefine-sym "
                f"{alias['from']}={alias['to']} $@"
            )
        else:
            lines.append("\t$(OBJCOPY) \\")
            for index, alias in enumerate(aliases):
                suffix = " \\" if index + 1 < len(aliases) else " $@"
                lines.append(
                    f"\t\t--redefine-sym {alias['from']}={alias['to']}{suffix}"
                )
        lines.append("")
    return "\n".join(lines)


def write_atomic(path: Path, content: str) -> None:
    """Replace the generated include atomically while preserving its mode."""
    path.parent.mkdir(parents=True, exist_ok=True)
    mode = stat.S_IMODE(path.stat().st_mode) if path.exists() else 0o644
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        os.fchmod(descriptor, mode)
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(content)
        temporary.replace(path)
    except BaseException:
        try:
            os.close(descriptor)
        except OSError:
            pass
        temporary.unlink(missing_ok=True)
        raise


def check_output(path: Path, expected: str) -> None:
    if not path.is_file():
        raise StaleOutputError(f"generated include is missing: {path}")
    if path.read_text(encoding="utf-8") != expected:
        raise StaleOutputError(
            f"generated include is stale: run {Path(__file__).name} --write"
        )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--write", action="store_true")
    action.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)

    try:
        rendered = render_makefile(load_manifest(args.manifest))
        if args.write:
            write_atomic(args.output, rendered)
            print(f"wrote {args.output}")
        else:
            check_output(args.output, rendered)
            print(f"overlay alias include is current: {args.output}")
    except (ManifestError, StaleOutputError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
