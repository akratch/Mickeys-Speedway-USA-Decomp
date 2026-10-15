#!/usr/bin/env python3
"""Tests for tools/archive_shard.py and the audits' handling of archives."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import archive_shard  # noqa: E402
import finalize_plateau as plateau  # noqa: E402

SYM = "demo_symbol"
SHARD = f"""<!-- plateau-handoff:{SYM}:start -->
### `{SYM}` plateau handoff

- source: `src/main/demo.c`
- score: 1/2 words
- frame: 0x10
- relocations: 0
- first mismatch: +0x4
- summary: Head.

### Standing section

Body.

### 2026-10-01: first

one

#### 2026-10-02: nested

nested body

### 2026-10-03: second

two

### Undated top

kept

### 2026-10-04: third

three
<!-- plateau-handoff:{SYM}:end -->
"""


class ArchiveShardTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        d = self.root / plateau.HANDOFF_SHARD_DIR
        d.mkdir(parents=True)
        self.shard = d / f"{SYM}.md"
        self.shard.write_text(SHARD)

    def tearDown(self):
        self.tmp.cleanup()

    def run_tool(self, *args):
        return archive_shard.main([SYM, "--root", str(self.root), *args])

    def test_keep_sections_moves_oldest_in_order(self):
        self.assertEqual(self.run_tool("--keep-sections", "1"), 0)
        live = self.shard.read_text()
        arch = (self.root / plateau.HANDOFF_SHARD_DIR / f"{SYM}.archive-1.md").read_text()
        plateau.parse_shard(live, SYM)
        self.assertIn("2026-10-04: third", live)
        self.assertNotIn("2026-10-01: first", live)
        self.assertIn("Undated top", live)
        self.assertIn(f"{SYM}.archive-1.md", live)
        self.assertLess(arch.index("first"), arch.index("nested body"))
        self.assertLess(arch.index("nested body"), arch.index("two"))
        self.assertNotIn("plateau-handoff:", arch)
        self.assertNotIn("Undated top", arch)

    def test_archive_before_and_next_number(self):
        self.assertEqual(self.run_tool("--archive-before", "2026-10-03"), 0)
        self.assertEqual(self.run_tool("--archive-before", "2026-10-04"), 0)
        d = self.root / plateau.HANDOFF_SHARD_DIR
        self.assertTrue((d / f"{SYM}.archive-1.md").exists())
        self.assertTrue((d / f"{SYM}.archive-2.md").exists())
        self.assertIn("2026-10-04: third", self.shard.read_text())

    def test_nothing_to_archive_and_dry_run(self):
        self.assertEqual(self.run_tool("--keep-sections", "9"), 1)
        before = self.shard.read_text()
        self.assertEqual(self.run_tool("--keep-sections", "0", "--dry-run"), 0)
        self.assertEqual(self.shard.read_text(), before)

    def test_archive_name_is_recognised(self):
        self.assertTrue(plateau.is_archive_name("func_800517E0.archive-1.md"))
        self.assertFalse(plateau.is_archive_name("func_800517E0.md"))


if __name__ == "__main__":
    unittest.main()
