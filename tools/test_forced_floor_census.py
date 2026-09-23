#!/usr/bin/env python3
"""Forced-floor census: every case is synthetic handoff prose.

The phrasings are the ones lanes actually write; the point of each test is
that a claim is read the way a coordinator would read it, and that nothing
is called exhausted without a stated exhaustion.
"""
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import forced_floor_census as ffc  # noqa: E402


def shard(symbol: str, summary: str, *sections: str) -> str:
    body = [
        f"<!-- plateau-handoff:{symbol}:start -->",
        f"### `{symbol}` plateau handoff",
        "",
        "- source: `src/overlays/o999/example.c`",
        "- score: 10/100 words",
        f"- summary: {summary}",
        "",
    ]
    for section in sections:
        body += [section, ""]
    body.append(f"<!-- plateau-handoff:{symbol}:end -->")
    return "\n".join(body)


def rank(name: str, words: int, delta: int = 0, size: int = 400) -> dict:
    return {"name": name, "relocation_masked_differing_words": words,
            "size_delta": delta, "size_bytes": size}


def run(shards: dict[str, str], ranking: list[dict],
        sources: dict | None = None) -> dict[str, ffc.Row]:
    rows = ffc.census(
        ranking={r["name"]: r for r in ranking},
        shards={k: (f"docs/matching-triage-handoffs/{k}.md", v)
                for k, v in shards.items()},
        sources=sources or {}, commits=False)
    return {r.symbol: r for r in rows}


EXHAUSTIVE = (
    "#### 2026-09-12, lane p23: exhaustive colour floor\n\n"
    "The every-colour footprint sampled 80 probes over 9 coloured webs; no\n"
    "accepted force beat 14, so the winner list is empty and the lattice\n"
    "floor is 14. The named source question is still open."
)


class ClaimParsing(unittest.TestCase):
    def test_wrapped_exhaustive_section_is_a_proved_floor(self):
        row = run({"a": shard("a", "Size 0.", EXHAUSTIVE)}, [rank("a", 14)])["a"]
        self.assertEqual(row.floor, 14)
        self.assertTrue(row.proved)
        self.assertEqual(row.status, "colour-exhausted")

    def test_summary_phrasings(self):
        cases = {
            "Colour floor 28 (202 probes, 0 winners).": (28, True),
            "exhaustive same-kind colour landscape floors at 2 with 0 winners "
            "of 172 probes": (2, True),
            "Floor 89 at delta 0 from p1:w61=c11. No zero-scoring force.": (89, True),
            "the measured single-force colour floor is 51/344 over the sample.": (51, False),
            "Colour floor **224**. No 0-score force.": (224, True),
        }
        for text, (floor, proved) in cases.items():
            with self.subTest(text):
                row = run({"s": shard("s", text)}, [rank("s", floor)])["s"]
                self.assertEqual((row.floor, row.proved), (floor, proved))

    def test_delta_zero_winners_are_not_zero_winners(self):
        """'Four delta-0 winners' lists winners; it is not an exhaustion."""
        text = "Colour: 151 probes. Four delta-0 winners score 224. Colour floor 224."
        row = run({"s": shard("s", text)}, [rank("s", 226)])["s"]
        self.assertFalse(row.proved)
        self.assertEqual(row.status, "unproved")

    def test_floor_without_colour_context_is_ignored(self):
        """A structural 'floor' is not a colour floor."""
        text = "No legal merge reaches 0x38: the legal floor is three declarations."
        self.assertEqual(run({"s": shard("s", text)}, [rank("s", 9)]), {})
        text = "The floor of the whole family is 56."
        self.assertEqual(run({"s": shard("s", text)}, [rank("s", 60)]), {})

    def test_later_section_supersedes_earlier(self):
        first = ("#### 2026-09-10, lane a\n\nThe lattice floor is 40; there is no "
                 "zero-scoring force.")
        second = ("#### 2026-09-19, lane b\n\n--every-colour (202 probes, 31 webs) "
                  "has zero winners of 28.")
        row = run({"s": shard("s", "Colour floor 40.", first, second)},
                  [rank("s", 28)])["s"]
        self.assertEqual(row.floor, 28)
        self.assertEqual(row.status, "colour-exhausted")

    def test_proof_must_be_in_the_section_stating_the_floor(self):
        proof = "#### 2026-09-10, lane a\n\nColour cannot close it."
        floor = "#### 2026-09-12, lane b\n\nThe diagnostic lattice floor of 47 holds."
        row = run({"s": shard("s", "Size 0.", proof, floor)}, [rank("s", 49)])["s"]
        self.assertEqual(row.floor, 47)
        self.assertFalse(row.proved)


class Classification(unittest.TestCase):
    def rows(self, **ranking):
        shards = {name: shard(name, "Colour floor 20. No zero-scoring force.")
                  for name in ranking}
        return run(shards, [rank(n, *v) for n, v in ranking.items() if v])

    def test_statuses(self):
        rows = run(
            {n: shard(n, s) for n, s in {
                "exhausted": "Colour floor 20. No zero-scoring force.",
                "zero": "Colour floor 0: a zero-scoring force exists.",
                "moved": "Colour floor 20. No zero-scoring force.",
                "track_b": "Colour floor 20. No zero-scoring force.",
                "hedged": "Colour floor 20.",
                "gone": "Colour floor 20. No zero-scoring force.",
            }.items()},
            [rank("exhausted", 25), rank("zero", 3), rank("moved", 12),
             rank("track_b", 30, delta=4), rank("hedged", 20)])
        self.assertEqual(
            {k: v.status for k, v in rows.items()},
            {"exhausted": "colour-exhausted", "zero": "zero-floor",
             "moved": "superseded", "track_b": "size-mismatch",
             "hedged": "unproved", "gone": "not-queued"})

    def test_source_block_is_read_when_no_shard_states_a_floor(self):
        body = ("\n * symbol: blk\n * score: 5/50 words\n * summary: Size 0 at 61. "
                "Colour floor 61. Colour cannot close it.\n * PLATEAU-HANDOFF:blk:end\n")
        rows = run({}, [rank("blk", 61)],
                   sources={"blk": [("src/overlays/o999/blk.c", body)]})
        self.assertEqual(rows["blk"].status, "colour-exhausted")
        self.assertEqual(rows["blk"].handoff, "src/overlays/o999/blk.c")


class Document(unittest.TestCase):
    def test_doc_carries_counts_and_no_prose(self):
        rows = run({"a": shard("a", "Colour floor 14. No zero-scoring force: "
                                    "SENTINEL prose.")},
                   [rank("a", 14, size=1234)])
        text = ffc.render_doc(list(rows.values()))
        self.assertIn("1 queued functions, 1,234 bytes, are colour-exhausted", text)
        self.assertIn("| `a` | 1,234 | +0 | 14 | 14 | yes | colour-exhausted |", text)
        self.assertNotIn("SENTINEL", text)

    def test_colour_exhausted_helper(self):
        rows = list(run({"a": shard("a", "Colour floor 14. Zero winners.")},
                        [rank("a", 14)]).values())
        self.assertEqual(list(ffc.colour_exhausted(rows)), ["a"])


if __name__ == "__main__":
    unittest.main()
