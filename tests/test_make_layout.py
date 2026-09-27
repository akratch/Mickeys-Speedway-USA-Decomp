#!/usr/bin/env python3
"""Keep the host build graph separate from per-overlay object policy."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parent.parent
MAKEFILE = ROOT / "Makefile"
OVERLAY_POLICY = ROOT / "mk" / "overlays.mk"
NEW_LANE = ROOT / "tools" / "new_lane.sh"


class MakeLayoutTests(unittest.TestCase):
    def test_tooling_gate_uses_discovery_once_without_duplicate_suites(self) -> None:
        text = MAKEFILE.read_text(encoding="utf-8")
        recipe = text.split("check-tooling:\n", 1)[1].split("\n\n", 1)[0]
        self.assertEqual(recipe.strip(),
                         "$(HOST_PYTHON) $(TOOLS_DIR)/run_tool_tests.py "
                         "$(CHECK_TOOLING_ARGS)")

    def test_root_includes_overlay_policy_once(self) -> None:
        text = MAKEFILE.read_text(encoding="utf-8")
        self.assertEqual(1, text.count("include mk/overlays.mk"))
        self.assertNotRegex(
            text,
            re.compile(
                r"^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/overlays/",
                re.MULTILINE,
            ),
        )

    def test_overlay_policy_contains_no_link_graph(self) -> None:
        text = OVERLAY_POLICY.read_text(encoding="utf-8")
        self.assertIn(
            "$(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o: MIPSISET :=",
            text,
        )
        self.assertNotIn("$(TARGET).elf:", text)
        self.assertNotIn("mips64-elf-ld", text)

    def test_root_owns_the_single_final_link(self) -> None:
        text = MAKEFILE.read_text(encoding="utf-8")
        self.assertEqual(1, len(re.findall(r"^\$\(TARGET\)\.elf:", text, re.MULTILINE)))

    def test_lane_is_marked_before_tracked_checkout(self) -> None:
        text = NEW_LANE.read_text(encoding="utf-8")
        noindex = text.index("dest=$display_dest.noindex")
        worktree = text.index("worktree add -q --no-checkout")
        marker = text.index(': > "$dest/.metadata_never_index"')
        read_tree = text.index('read-tree "$base_commit"')
        checkout = text.index("checkout-index --all")
        compatibility_link = text.index('ln -s "$(basename "$dest")"')
        self.assertLess(noindex, worktree)
        self.assertLess(worktree, marker)
        self.assertLess(marker, read_tree)
        self.assertLess(read_tree, checkout)
        self.assertLess(checkout, compatibility_link)


if __name__ == "__main__":
    unittest.main()
