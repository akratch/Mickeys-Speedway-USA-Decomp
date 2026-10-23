#!/usr/bin/env python3
"""Focused tests for the declarative overlay symbol-alias renderer."""

from __future__ import annotations

import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import render_overlay_aliases as renderer


def manifest(rules: list[dict[str, object]]) -> dict[str, object]:
    return {"schema_version": 1, "rules": rules}


def rule(
    object_path: str,
    aliases: list[tuple[str, str]],
) -> dict[str, object]:
    return {
        "object": object_path,
        "aliases": [
            {"from": source, "to": destination}
            for source, destination in aliases
        ],
    }


class ManifestValidationTests(unittest.TestCase):
    def load(self, document: object) -> list[dict[str, object]]:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "aliases.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            if isinstance(document, dict) and isinstance(document.get("rules"), list):
                for item in document["rules"]:
                    if isinstance(item, dict) and isinstance(item.get("object"), str):
                        source = Path(directory) / "src" / item["object"].removesuffix(".o")
                        source.parent.mkdir(parents=True, exist_ok=True)
                        source.touch()
            return renderer.load_manifest(path, root=Path(directory))

    def test_rejects_deleted_alias_owner(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / "aliases.json"
            path.write_text(json.dumps(manifest([
                rule("overlays/o034/deleted.c.o", [("old", "new")])
            ])))
            with self.assertRaisesRegex(renderer.ManifestError, "alias owner no longer exists"):
                renderer.load_manifest(path, root=root)

    def test_rejects_malformed_source_and_destination_symbols(self) -> None:
        for field, value in (("from", "bad=name"), ("to", "9bad")):
            with self.subTest(field=field, value=value):
                alias = {"from": "source", "to": "destination"}
                alias[field] = value
                document = manifest([
                    {
                        "object": "overlays/o034/example.c.o",
                        "aliases": [alias],
                    }
                ])
                with self.assertRaisesRegex(renderer.ManifestError, "C-style symbol"):
                    self.load(document)

    def test_rejects_duplicate_object_targets(self) -> None:
        item = rule("overlays/o034/example.c.o", [("source", "destination")])
        with self.assertRaisesRegex(renderer.ManifestError, "duplicate object target"):
            self.load(manifest([item, item]))

    def test_rejects_duplicate_alias_destinations(self) -> None:
        document = manifest([
            rule(
                "overlays/o034/example.c.o",
                [("sourceA", "destination"), ("sourceB", "destination")],
            )
        ])
        with self.assertRaisesRegex(renderer.ManifestError, "duplicate destination"):
            self.load(document)

    def test_rejects_chained_aliases(self) -> None:
        document = manifest([
            rule(
                "overlays/o034/example.c.o",
                [("sourceA", "middle"), ("middle", "destination")],
            )
        ])
        with self.assertRaisesRegex(renderer.ManifestError, "chained aliases"):
            self.load(document)

    def test_rejects_unknown_fields_and_duplicate_json_keys(self) -> None:
        document = manifest([
            rule("overlays/o034/example.c.o", [("source", "destination")])
        ])
        document["unexpected"] = True
        with self.assertRaisesRegex(renderer.ManifestError, "unknown"):
            self.load(document)

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "aliases.json"
            path.write_text(
                '{"schema_version":1,"schema_version":1,"rules":[]}',
                encoding="utf-8",
            )
            with self.assertRaisesRegex(renderer.ManifestError, "duplicate JSON key"):
                renderer.load_manifest(path)


class RenderingTests(unittest.TestCase):
    def test_rendering_is_canonical_regardless_of_manifest_order(self) -> None:
        rules = [
            rule("overlays/o058/zeta.c.o", [("sourceZ", "destinationZ")]),
            rule(
                "overlays/o034/alpha.c.o",
                [("sourceB", "destinationB"), ("sourceA", "destinationA")],
            ),
        ]
        forward = renderer.render_makefile(rules)
        reverse = renderer.render_makefile(list(reversed(rules)))
        self.assertEqual(forward, reverse)
        self.assertLess(forward.index("o034/alpha"), forward.index("o058/zeta"))
        self.assertLess(forward.index("sourceA="), forward.index("sourceB="))
        self.assertTrue(forward.endswith("\n"))

    def test_write_and_check_detect_stale_output(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest_path = root / "aliases.json"
            output_path = root / "aliases.mk"
            manifest_path.write_text(
                json.dumps(
                    manifest([
                        rule(
                            "overlays/o034/example.c.o",
                            [("source", "destination")],
                        )
                    ])
                ),
                encoding="utf-8",
            )

            source = root / "src/overlays/o034/example.c"
            source.parent.mkdir(parents=True)
            source.touch()
            with mock.patch.object(renderer, "REPO", root), contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(
                    renderer.main([
                        "--manifest",
                        str(manifest_path),
                        "--output",
                        str(output_path),
                        "--write",
                    ]),
                    0,
                )
                self.assertEqual(
                    renderer.main([
                        "--manifest",
                        str(manifest_path),
                        "--output",
                        str(output_path),
                        "--check",
                    ]),
                    0,
                )

            output_path.write_text("stale\n", encoding="utf-8")
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(
                    renderer.main([
                        "--manifest",
                        str(manifest_path),
                        "--output",
                        str(output_path),
                        "--check",
                    ]),
                    1,
                )


if __name__ == "__main__":
    unittest.main()
