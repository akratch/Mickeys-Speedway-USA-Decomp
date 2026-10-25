#!/usr/bin/env python3
"""Focused pass-side fixtures for clean-room decoder discriminators."""

import contextlib
import hashlib
import io
import json
import unittest
from unittest import mock

import cleanroom_detectors as D


class AggregatePinExemptionTests(unittest.TestCase):
    path = "config/lane-reopen-authorizations.us.json"

    def fixture(self, *, payload=False):
        rows = {}
        for i in range(100):
            reason = "Reviewed bounded source experiment."
            if payload:
                reason = " ".join(
                    "0x" + hashlib.sha256(f"word-{i}-{j}".encode()).hexdigest()[:8]
                    for j in range(15)
                )
            rows[f"function_{i}"] = {
                "source_commit": hashlib.sha1(f"source-{i}".encode()).hexdigest(),
                "ledger_commit": hashlib.sha1(f"ledger-{i}".encode()).hexdigest(),
                "reason": reason,
            }
        return json.dumps({"schema_version": 1, "authorizations": rows}).encode()

    def check(self, path, data):
        with contextlib.redirect_stderr(io.StringIO()):
            return D.check_content(path, data)

    def driver(self, paths, data, labels=None):
        labels = labels or ["staged"] * len(paths)
        entries = "".join(
            f"blob\tfixture\t{label}\t{path}\n"
            for path, label in zip(paths, labels)
        )
        output = io.StringIO()
        with mock.patch.object(D, "read_blobs", return_value={"fixture": data}), \
                mock.patch.object(D.sys, "stdin", io.StringIO(entries)), \
                contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            code = D.main()
        return code, output.getvalue()

    def test_only_declared_hashes_are_removed_from_aggregate(self):
        found, metrics = self.check(self.path, self.fixture())
        self.assertEqual(found, [])
        self.assertGreaterEqual(metrics["scored"], D.AGGREGATE_WORD_BUDGET)
        self.assertEqual(metrics["aggregate_scored"], 0)
        self.assertEqual(self.driver([self.path], self.fixture())[0], 0)

    def test_reason_payload_still_counts(self):
        _, metrics = self.check(self.path, self.fixture(payload=True))
        self.assertGreaterEqual(metrics["aggregate_scored"], D.AGGREGATE_WORD_BUDGET)
        code, output = self.driver([self.path], self.fixture(payload=True))
        self.assertEqual(code, 1)
        self.assertIn("FAIL [aggregate-word-budget]", output)

    def test_malformed_schema_and_duplicate_keys_fail_closed(self):
        for data in (b"null", b"[]", b"{", self.fixture().replace(
                b'"schema_version": 1', b'"schema_version": 1, "schema_version": 1')):
            with self.subTest(data=data[:40]):
                code, output = self.driver([self.path], data)
                self.assertEqual(code, 1)
                self.assertIn("reopen-authorization-schema", output)

    def test_hash_shaped_reason_is_not_a_hash_leaf(self):
        document = json.loads(self.fixture())
        for i, row in enumerate(document["authorizations"].values()):
            row["reason"] = " ".join([
                row["source_commit"], row["ledger_commit"],
                hashlib.sha1(f"reason-{i}-a".encode()).hexdigest(),
                hashlib.sha1(f"reason-{i}-b".encode()).hexdigest(),
            ])
        _, metrics = self.check(self.path, json.dumps(document).encode())
        self.assertGreater(metrics["aggregate_scored"], 0)

    def test_foreign_identical_blob_counts_in_either_order(self):
        for paths in ([self.path, "config/foreign.json"],
                      ["config/foreign.json", self.path]):
            code, output = self.driver(paths, self.fixture())
            self.assertEqual(code, 1)
            self.assertIn("FAIL [aggregate-word-budget]", output)
            self.assertIn("FAIL [word-table] config/foreign.json", output)

    def test_path_is_exact_and_other_word_table_exemptions_do_not_apply(self):
        for path in ("./" + self.path, "other/" + self.path, "Makefile"):
            _, metrics = self.check(path, self.fixture())
            self.assertNotIn("aggregate_scored", metrics)
            self.assertGreaterEqual(metrics["scored"], D.AGGREGATE_WORD_BUDGET)

    def test_duplicate_bytes_count_once_per_label_using_maximum(self):
        data = self.fixture()
        _, metrics = self.check("foreign.json", data)
        code, output = self.driver([self.path, "foreign.json", "copy.json"], data)
        self.assertEqual(code, 1)
        self.assertIn(f"{metrics['scored']} machine words from", output)
        self.assertEqual(output.count("FAIL [aggregate-word-budget]"), 1)
        code, output = self.driver([self.path, "foreign.json"], data,
                                   ["commit clean", "commit foreign"])
        self.assertEqual(code, 1)
        self.assertEqual(output.count("FAIL [aggregate-word-budget]"), 1)


class RelocationProofSizeExemptionTests(unittest.TestCase):
    def test_only_exact_path_size_is_exempt_other_detectors_remain(self):
        text=b"# authored proof comment\n"*(D.SIZE_LIMIT//20)
        with contextlib.redirect_stderr(io.StringIO()):
            found,_=D.check_content("tools/reloc_surface.py",text)
            other,_=D.check_content("other/reloc_surface.py",text)
            payload=" ".join("0x"+hashlib.sha256(f"synthetic-{i}".encode()).hexdigest()[:8]
                             for i in range(300)).encode()
            words,_=D.check_content("tools/reloc_surface.py",payload)
        self.assertNotIn("oversize",[name for name,_ in found])
        self.assertIn("oversize",[name for name,_ in other])
        self.assertIn("word-table",[name for name,_ in words])


class DecoderExclusionTests(unittest.TestCase):
    def test_address_shaped_identifiers_are_not_numeric_tokens(self) -> None:
        text = "\n".join(
            [
                "extern void func_80012340(void);",
                "extern int D_80081898;",
                "O101TailAB4CNode *gO101TailAB4CNode;",
            ]
            * 80
        )

        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["hex-run"], [])
        self.assertEqual(by_stage["halves-pair"], [])

    def test_consecutive_source_lines_are_not_base_n_blocks(self) -> None:
        text = "\n".join(
            [
                "rumbleTick(updateRate);",
                "osContStartReadData(&D_800CF340);",
                "mmFree(D_800C95A8);",
                "mmFree(D_800C9D2C);",
            ]
        )

        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["base-block"], [])

    def test_supported_numeric_token_shapes_remain_decodable(self) -> None:
        by_stage = D.normalize_words_by_stage(
            r"0x27bdffe0u 27bd_ffe0L \u27bd\uffe0 0x27bdffe0-0x27bdffe4."
        )

        self.assertIn(0x27BDFFE0, by_stage["hex-run"])
        self.assertGreaterEqual(by_stage["hex-run"].count(0x27BDFFE0), 3)

    def test_unprefixed_decimal_is_not_also_decoded_as_hex(self) -> None:
        text = "timestamp 1788409073 recorded"
        by_stage = D.normalize_words_by_stage(text)

        self.assertEqual(by_stage["hex-run"], [])
        self.assertEqual(by_stage["dec-token"], [int(text.split()[1])])


if __name__ == "__main__":
    unittest.main()
