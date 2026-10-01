#!/usr/bin/env python3
"""Synthetic tests for overlay-atlas exact-C release deltas."""

import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

import overlay_atlas  # noqa: E402


def ownership(start, end, *, exact, source="overlays/o001/example"):
    return {
        "offset": hex(start),
        "end_offset": hex(end),
        "size": hex(end - start),
        "type": "c",
        "source": source,
        "matched": True,
        "nonmatching": not exact,
    }


def module(overlay, rows, mixed=None):
    result = {"overlay": overlay, "text_ownership": rows}
    if mixed is not None:
        result["mixed_tu_exact_c_ranges"] = mixed
    return result


def atlas(modules):
    exact_bytes = 0
    for item in modules:
        exact_bytes += sum(
            int(row["size"], 0)
            for row in item["text_ownership"]
            if row["type"] == "c"
            and row["matched"] is True
            and row["nonmatching"] is False
        )
        exact_bytes += sum(
            int(row["size"], 0)
            for row in item.get("mixed_tu_exact_c_ranges", [])
        )
    return {
        "schema_version": 1,
        "totals": {"matched_overlay_c_bytes": exact_bytes},
        "modules": modules,
    }


class ExactCDeltaTests(unittest.TestCase):
    def test_reports_promotions_retractions_and_net_bytes(self):
        base = atlas(
            [
                module(1, [ownership(0x10, 0x20, exact=True)]),
                module(2, [ownership(0x30, 0x48, exact=False)]),
            ]
        )
        target = atlas(
            [
                module(1, [ownership(0x10, 0x20, exact=False)]),
                module(2, [ownership(0x30, 0x48, exact=True)]),
            ]
        )

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual([row["key"] for row in delta["promotions"]], [
            "overlay:2:text:0x30"
        ])
        self.assertEqual([row["key"] for row in delta["retractions"]], [
            "overlay:1:text:0x10"
        ])
        self.assertEqual(delta["totals"]["promotion_bytes"], 0x18)
        self.assertEqual(delta["totals"]["retraction_bytes"], 0x10)
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], 8)

    def test_mixed_tu_exact_range_is_a_promotion(self):
        container = ownership(0, 0x40, exact=False)
        base = atlas([module(7, [container])])
        exact_range = {
            "offset": "0x10",
            "end_offset": "0x28",
            "size": "0x18",
            "label": "overlay7Promoted",
            "source": "overlays/o007/overlay_007",
            "evidence": "synthetic exact proof",
        }
        target = atlas([module(7, [container], [exact_range])])

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual(delta["promotions"][0]["kind"], "mixed_tu_range")
        self.assertEqual(delta["promotions"][0]["label"], "overlay7Promoted")
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], 0x18)


    def test_duplicate_overlay_module_fails_closed(self):
        state = atlas([
            module(1, [ownership(0, 4, exact=True)]),
            module(1, [ownership(4, 8, exact=True)]),
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "duplicate overlay 1"
        ):
            overlay_atlas.exact_c_index(state)

    def test_duplicate_overlay_offset_fails_closed(self):
        state = atlas([
            module(1, [
                ownership(0, 0x20, exact=True, source="overlays/o001/a"),
                ownership(0, 0x10, exact=True, source="overlays/o001/b"),
            ])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "ambiguous exact-C identity"
        ):
            overlay_atlas.exact_c_index(state)

    def test_overlapping_ranges_with_different_offsets_fail_closed(self):
        state = atlas([
            module(1, [
                ownership(0, 0x20, exact=True, source="overlays/o001/a"),
                ownership(0x10, 0x30, exact=True, source="overlays/o001/b"),
            ])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "overlapping exact-C identities"
        ):
            overlay_atlas.exact_c_index(state)

    def test_same_identity_with_changed_extent_fails_closed(self):
        base = atlas([module(1, [ownership(0x10, 0x20, exact=True)])])
        target = atlas([module(1, [ownership(0x10, 0x24, exact=True)])])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "ambiguous identity.*extent"
        ):
            overlay_atlas.compare_exact_c_atlases(base, target)

    def test_reviewed_whole_owner_to_mixed_ranges_reports_only_uncovered_bytes(self):
        source = "overlays/o001/example"
        base = atlas([module(1, [ownership(0, 0x40, exact=True, source=source)])])
        container = ownership(0, 0x40, exact=False, source=source)
        mixed = [
            {
                "offset": "0x0",
                "end_offset": "0x10",
                "size": "0x10",
                "label": "firstExact",
                "source": source,
            },
            {
                "offset": "0x30",
                "end_offset": "0x40",
                "size": "0x10",
                "label": "lastExact",
                "source": source,
            },
        ]
        target = atlas([module(1, [container], mixed)])

        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        self.assertEqual(delta["promotions"], [])
        self.assertEqual(
            [
                (row["offset"], row["end_offset"], row["size"])
                for row in delta["retractions"]
            ],
            [(0x10, 0x30, 0x20)],
        )
        self.assertEqual(delta["totals"]["net_exact_c_bytes"], -0x20)

    def test_declared_total_must_match_exact_rows(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        state["totals"]["matched_overlay_c_bytes"] = 12

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "atlas declares 12"
        ):
            overlay_atlas.exact_c_index(state)

    def test_fractional_numeric_field_fails_closed(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        state["modules"][0]["text_ownership"][0]["size"] = 8.5

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "invalid size 8.5"
        ):
            overlay_atlas.exact_c_index(state)

    def test_missing_total_fails_closed(self):
        state = atlas([module(1, [ownership(0, 8, exact=True)])])
        del state["totals"]["matched_overlay_c_bytes"]

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "lacks an exact-C byte total"
        ):
            overlay_atlas.exact_c_index(state)

    def test_orphan_mixed_tu_range_fails_closed(self):
        exact_range = {
            "offset": "0x10",
            "end_offset": "0x20",
            "size": "0x10",
            "label": "orphan",
            "source": "overlays/o001/example",
        }
        state = atlas([
            module(1, [ownership(0, 0x40, exact=True)], [exact_range])
        ])

        with self.assertRaisesRegex(
            overlay_atlas.AtlasDeltaError, "not inside exactly one nonmatching"
        ):
            overlay_atlas.exact_c_index(state)

    def test_json_result_uses_numeric_offsets_and_sizes(self):
        base = atlas([module(3, [ownership(4, 12, exact=False)])])
        target = atlas([module(3, [ownership(4, 12, exact=True)])])
        delta = overlay_atlas.compare_exact_c_atlases(base, target)

        decoded = json.loads(json.dumps(delta, sort_keys=True))

        self.assertEqual(decoded["schema_version"], 1)
        self.assertEqual(decoded["promotions"][0]["overlay"], 3)
        self.assertEqual(decoded["promotions"][0]["offset"], 4)
        self.assertEqual(decoded["promotions"][0]["size"], 8)


class NonmatchingSourceTests(unittest.TestCase):
    def test_guarded_candidate_is_nonmatching(self):
        self.assertTrue(
            overlay_atlas.is_nonmatching_text("#ifdef NON_MATCHING\nvoid f(void) {}\n#endif\n")
        )

    def test_bare_global_asm_is_nonmatching(self):
        self.assertTrue(
            overlay_atlas.is_nonmatching_text(
                '#pragma GLOBAL_ASM("asm/nonmatchings/example.s")\n'
            )
        )

    def test_exact_c_only_is_matching(self):
        self.assertFalse(overlay_atlas.is_nonmatching_text("void f(void) {}\n"))


class AtlasStateLoadingTests(unittest.TestCase):
    def test_loads_manifest_and_checkout_paths(self):
        state = atlas([module(1, [ownership(0, 4, exact=True)])])
        with tempfile.TemporaryDirectory() as temporary:
            tree = Path(temporary)
            manifest = tree / "config" / "overlays.us.json"
            manifest.parent.mkdir()
            manifest.write_text(json.dumps(state))

            from_manifest, manifest_info = overlay_atlas.load_atlas_state(
                str(manifest)
            )
            from_tree, tree_info = overlay_atlas.load_atlas_state(str(tree))

        self.assertEqual(from_manifest, state)
        self.assertEqual(from_tree, state)
        self.assertEqual(manifest_info["kind"], "manifest")
        self.assertEqual(tree_info["kind"], "tree")

    def test_loads_git_ref_without_checkout(self):
        state = atlas([module(9, [ownership(0x20, 0x30, exact=True)])])
        with tempfile.TemporaryDirectory() as temporary:
            repo = Path(temporary)
            subprocess.run(["git", "init", "-q", str(repo)], check=True)
            subprocess.run(
                ["git", "config", "user.name", "Atlas Test"], cwd=repo, check=True
            )
            subprocess.run(
                ["git", "config", "user.email", "atlas@example.invalid"],
                cwd=repo,
                check=True,
            )
            manifest = repo / "config" / "overlays.us.json"
            manifest.parent.mkdir()
            manifest.write_text(json.dumps(state))
            subprocess.run(["git", "add", str(manifest)], cwd=repo, check=True)
            subprocess.run(
                ["git", "commit", "-qm", "Synthetic atlas"], cwd=repo, check=True
            )

            loaded, info = overlay_atlas.load_atlas_state("HEAD", repo=repo)

        self.assertEqual(loaded, state)
        self.assertEqual(info["kind"], "git")
        self.assertRegex(info["resolved"], r"^[0-9a-f]{40}$")


def trial_module(overlay=1):
    """One module with a mid-region fixed data/rodata carve.

    The carve ends before the data section does, so the raw remainder behind it
    needs its own `bin` row -- the case that regressed: both rows were named
    `overlay_001_data_rodata`, splat wrote both to that one asset path, and the
    shorter one won. The module then lost 0x274 bytes and every module behind
    it slid, which the promotion trial reported as ~528,000 out-of-range bytes
    on every candidate it tried.
    """
    return {
        "overlay": overlay,
        "rom": {"start": "0x184C3E0", "end": "0x1856DF8", "size": "0xAA18"},
        "bss_size": "0x1DE0",
        "sections": {
            "text": {"start": "0x184C3E0", "end": "0x1854500", "size": "0x8120"},
            "data_rodata": {
                "start": "0x1854500",
                "end": "0x18547C0",
                "size": "0x2C0",
            },
            "reloc1": {"start": "0x18547C0", "end": "0x1855020", "size": "0x860"},
            "reloc2": {"start": "0x1855020", "end": "0x1856DF8", "size": "0x1DD8"},
        },
        "text_ownership": [
            {
                "offset": "0x0",
                "end_offset": "0x8120",
                "size": "0x8120",
                "type": "c",
                "source": "overlays/o001/example_tail",
                "matched": True,
                "nonmatching": True,
            }
        ],
        "data_rodata_ownership": [
            {
                "offset": "0x274",
                "end_offset": "0x294",
                "size": "0x20",
                "section": ".rodata",
                "source": "overlays/o001/example_tail",
                "trial_function": "example_owner",
            }
        ],
    }


class CanonicalDataOnlyOwnershipTests(unittest.TestCase):
    @staticmethod
    def text_ownership():
        return [{"type": "c", "source": "overlays/o008/overlay_008"}]

    def test_registered_data_only_owner_is_a_canonical_atlas_row(self):
        rows = overlay_atlas.data_rodata_ownership_rows(
            8, 0x2500, self.text_ownership()
        )
        self.assertEqual(
            rows[-1],
            {
                "offset": "0x2378",
                "end_offset": "0x2418",
                "size": "0xA0",
                "type": "c",
                "section": ".data",
                "source": "overlays/o008/overlay8MotionConstants",
                "canonical": True,
            },
        )
        self.assertEqual(
            rows[-2],
            {
                "offset": "0x3E0",
                "end_offset": "0x520",
                "size": "0x140",
                "type": "c",
                "section": ".data",
                "source": "overlays/o008/overlay8EffectTables",
                "canonical": True,
            },
        )
        self.assertEqual(len(rows), 3)

    def test_unallowlisted_data_only_owner_fails_closed(self):
        with mock.patch.dict(
            overlay_atlas.CANONICAL_FIXED_DATA_RODATA_OWNERSHIP,
            {8: [(0x100, 0x110, "unreviewedOwner", ".data")]},
            clear=True,
        ):
            with self.assertRaisesRegex(ValueError, "not explicitly allowed"):
                overlay_atlas.data_rodata_ownership_rows(
                    8, 0x2500, self.text_ownership()
                )

    def test_canonical_owner_carves_without_trial_and_preserves_raw_slices(self):
        module_row = trial_module()
        part = module_row["data_rodata_ownership"][0]
        part["source"] = "overlays/o001/storageOnly"
        part["canonical"] = True
        block = overlay_atlas.render_yaml_block({"modules": [module_row]})

        self.assertIn("subalign: 0x4", block)
        self.assertIn("- [0x1854500, bin, overlay_001_data_rodata]", block)
        self.assertIn("- [0x1854774, .rodata, storageOnly]", block)
        self.assertIn("- [0x1854794, bin, overlay_001_data_rodata_294]", block)

    def test_canonical_owner_overlap_with_trial_owner_fails_closed(self):
        module_row = trial_module()
        module_row["data_rodata_ownership"].append(
            {
                "offset": "0x280",
                "end_offset": "0x2A0",
                "size": "0x20",
                "type": "c",
                "section": ".data",
                "source": "overlays/o001/storageOnly",
                "canonical": True,
            }
        )
        with self.assertRaisesRegex(ValueError, "overlaps"):
            overlay_atlas.render_yaml_block(
                {"modules": [module_row]},
                trial_ownership=True,
                trial_sources=frozenset({"example_tail"}),
            )

    def test_multiple_canonical_rows_carve_in_order_without_losing_raw_slices(self):
        module_row = trial_module()
        module_row["sections"]["data_rodata"] = {
            "start": "0x1854500",
            "end": "0x1856A00",
            "size": "0x2500",
        }
        module_row["data_rodata_ownership"] = [
            {
                "offset": "0x3E0",
                "end_offset": "0x520",
                "size": "0x140",
                "type": "c",
                "section": ".data",
                "source": "overlays/o008/overlay8EffectTables",
                "canonical": True,
            },
            {
                "offset": "0x2378",
                "end_offset": "0x2418",
                "size": "0xA0",
                "type": "c",
                "section": ".data",
                "source": "overlays/o008/overlay8MotionConstants",
                "canonical": True,
            },
        ]
        block = overlay_atlas.render_yaml_block({"modules": [module_row]})

        self.assertIn("- [0x1854500, bin, overlay_001_data_rodata]", block)
        self.assertIn("- [0x18548E0, .data, overlay8EffectTables]", block)
        self.assertIn("- [0x1854A20, bin, overlay_001_data_rodata_520]", block)
        self.assertIn("- [0x1856878, .data, overlay8MotionConstants]", block)
        self.assertIn("- [0x1856918, bin, overlay_001_data_rodata_2418]", block)


class TrialProjectionTests(unittest.TestCase):
    """The promotion trial's temporary ownership projection."""

    def render(self, **kwargs):
        return overlay_atlas.render_yaml_block(
            {"modules": [trial_module()]}, **kwargs
        )

    def test_projection_without_a_trial_source_is_the_canonical_yaml(self):
        # A carve is only correct while the owning TU emits those bytes, which
        # only its own promotion does. Naming no source must leave the module
        # exactly as the tracked yaml spells it, or every trial of every other
        # candidate silently loses the carved range.
        canonical = self.render()
        self.assertEqual(canonical, self.render(trial_ownership=True))
        self.assertIn("- [0x1854500, bin, overlay_001_data_rodata]", canonical)
        self.assertNotIn(".rodata", canonical)
        self.assertIn("subalign: 0x1", canonical)

    def test_a_named_trial_source_carves_only_its_own_range(self):
        carved = self.render(
            trial_ownership=True, trial_sources=frozenset({"example_tail"})
        )
        self.assertIn("- [0x1854774, .rodata, example_tail]", carved)
        self.assertIn("subalign: 0x4", carved)

    def test_a_named_trial_function_carves_only_its_owned_range(self):
        carved = self.render(
            trial_ownership=True,
            trial_sources=frozenset({"example_tail"}),
            trial_functions=frozenset({"example_owner"}),
        )
        self.assertIn("- [0x1854774, .rodata, example_tail]", carved)

    def test_an_externalized_owner_keeps_the_retained_raw_slice(self):
        module = trial_module()
        module["data_rodata_ownership"][0]["externalized"] = True
        projected = overlay_atlas.render_yaml_block(
            {"modules": [module]},
            trial_ownership=True,
            trial_sources=frozenset({"example_tail"}),
            trial_functions=frozenset({"example_owner"}),
        )
        self.assertNotIn(".rodata", projected)
        self.assertIn("- [0x1854500, bin, overlay_001_data_rodata]", projected)
        self.assertIn("subalign: 0x1", projected)

    def test_another_function_in_the_same_tu_carves_nothing(self):
        uncarved = self.render(
            trial_ownership=True,
            trial_sources=frozenset({"example_tail"}),
            trial_functions=frozenset({"other_function"}),
        )
        self.assertEqual(self.render(), uncarved)

    def test_each_raw_slice_gets_its_own_asset_name(self):
        carved = self.render(
            trial_ownership=True, trial_sources=frozenset({"example_tail"})
        )
        names = re.findall(r"- \[0x[0-9A-F]+, bin, (\S+?)\]", carved)
        self.assertEqual(len(names), len(set(names)), names)
        self.assertIn("overlay_001_data_rodata", names)
        self.assertIn("overlay_001_data_rodata_294", names)

    def test_an_unrelated_trial_source_carves_nothing(self):
        self.assertEqual(
            self.render(),
            self.render(
                trial_ownership=True,
                trial_sources=frozenset({"some_other_tu"}),
            ),
        )

    def test_trial_sources_reads_the_environment_and_takes_basenames(self):
        with mock.patch.dict(
            os.environ,
            {overlay_atlas.TRIAL_SOURCE_ENV: "overlays/o001/example_tail"},
        ):
            self.assertEqual(
                overlay_atlas.trial_sources(), frozenset({"example_tail"})
            )
            self.assertEqual(
                overlay_atlas.trial_sources(["overlays/o002/other"]),
                frozenset({"other"}),
            )
        with mock.patch.dict(os.environ, {overlay_atlas.TRIAL_SOURCE_ENV: ""}):
            self.assertEqual(overlay_atlas.trial_sources(), frozenset())

    def test_trial_functions_reads_the_environment(self):
        with mock.patch.dict(
            os.environ,
            {overlay_atlas.TRIAL_FUNCTION_ENV: "example_owner other_function"},
        ):
            self.assertEqual(
                overlay_atlas.trial_functions(),
                frozenset({"example_owner", "other_function"}),
            )
        with mock.patch.dict(os.environ, {overlay_atlas.TRIAL_FUNCTION_ENV: ""}):
            self.assertEqual(overlay_atlas.trial_functions(), frozenset())

    def test_canonical_externalized_owner_fails_closed(self):
        module_row = trial_module()
        part = module_row["data_rodata_ownership"][0]
        part["canonical"] = True
        part["externalized"] = True

        with self.assertRaisesRegex(ValueError, "canonical.*cannot be externalized"):
            overlay_atlas.render_yaml_block({"modules": [module_row]})


if __name__ == "__main__":
    unittest.main()
