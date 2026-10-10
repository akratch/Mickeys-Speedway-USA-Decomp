#!/usr/bin/env python3
"""Synthetic request geometry only; no game or compiler artifacts."""
import contextlib
import copy
import io
import itertools
import json
from pathlib import Path
import tempfile
import unittest

import spill_order_replay as replay


def fixture():
    # A path has a two-cell solution, but an unfortunate first-fit order uses three.
    return {"schema": replay.SCHEMA, "requests": [
        {"id": 10, "size": 8, "alignment": 8, "regions": [1]},
        {"id": 20, "size": 8, "alignment": 8, "regions": [1, 2]},
        {"id": 30, "size": 8, "alignment": 8, "regions": [2, 3]},
        {"id": 40, "size": 8, "alignment": 8, "regions": [3]}],
        "order": [10, 20, 30, 40]}


class ReplayTests(unittest.TestCase):
    def test_order_changes_pool(self):
        data = fixture()
        self.assertEqual(replay.replay(data)["pool_slots"], 2)
        data["order"] = [10, 40, 20, 30]
        result = replay.replay(data)
        self.assertEqual(result["pool_slots"], 3)
        self.assertEqual(result["pool_bytes"], 24)
        self.assertEqual(result["shared_node_lower_bound"]["slots"], 2)
        self.assertEqual(result["steps"][-1]["blockers"], [
            {"slot": 0, "owners": [40]}, {"slot": 1, "owners": [20]}])

    def test_disjoint_reuse_and_overlap(self):
        result = replay.replay(fixture())
        self.assertEqual([r["slot"] for r in result["steps"]], [0, 1, 0, 1])

    def test_all_orders_respect_conflicts(self):
        data = fixture()
        for order in itertools.permutations(data["order"]):
            data["order"] = list(order)
            slots = {r["id"]: r["slot"] for r in replay.replay(data)["steps"]}
            for a, b in [(10, 20), (20, 30), (30, 40)]:
                self.assertNotEqual(slots[a], slots[b])

    def test_constraints_are_checked_not_forced(self):
        data = fixture()
        data["constraints"] = [{"id": 10, "slot": 0}, {"id": 20, "slot": 0}]
        result = replay.replay(data)
        self.assertFalse(result["constraints_satisfied"])
        self.assertEqual(result["violations"], [{"id": 20, "expected": 0, "actual": 1}])
        data["constraints"][1]["slot"] = 1
        self.assertTrue(replay.replay(data)["constraints_satisfied"])

    def test_empty_regions(self):
        data = fixture()
        for row in data["requests"]:
            row["regions"] = []
        result = replay.replay(data)
        self.assertEqual(result["pool_slots"], 1)
        self.assertEqual(result["shared_node_lower_bound"],
                         {"node": None, "requests": [], "slots": 0})

    def test_shared_node_bound_is_not_claimed_maximum_clique(self):
        data = fixture()
        data["requests"] = data["requests"][:3]
        for row, regions in zip(data["requests"], [[1, 2], [2, 3], [1, 3]]):
            row["regions"] = regions
        data["order"] = [10, 20, 30]
        result = replay.replay(data)
        self.assertEqual(result["pool_slots"], 3)
        self.assertEqual(result["shared_node_lower_bound"]["slots"], 2)

    def test_request_list_order_does_not_change_witness(self):
        data = fixture()
        expected = replay.replay(data)
        data["requests"].reverse()
        self.assertEqual(replay.replay(data), expected)

    def test_strict_schema_and_geometry(self):
        bad = []
        def mutate(change):
            data = fixture(); change(data); bad.append(data)
        mutate(lambda d: d.update(schema="other"))
        mutate(lambda d: d.update(extra=True))
        mutate(lambda d: d.pop("order"))
        mutate(lambda d: d.update(requests=[]))
        mutate(lambda d: d["requests"].append(copy.deepcopy(d["requests"][0])))
        mutate(lambda d: d.update(order=[10, 20, 30]))
        mutate(lambda d: d.update(order=[10, 20, 30, 99]))
        mutate(lambda d: d.update(order=[10, 20, 30, 30]))
        mutate(lambda d: d.update(order=[True, 20, 30, 40]))
        for field, value in [("id", True), ("size", 0), ("size", 16),
                             ("size", True), ("alignment", 3), ("alignment", 16),
                             ("alignment", 4), ("regions", [1, 1]),
                             ("regions", [False]), ("regions", [-1])]:
            mutate(lambda d, f=field, v=value: d["requests"][0].update({f: v}))
        for constraint in [{"id": 99, "slot": 0}, {"id": 10, "slot": True},
                           {"id": 10, "slot": -1}, {"id": 10, "slot": 0, "x": 1}]:
            mutate(lambda d, c=constraint: d.update(constraints=[c]))
        mutate(lambda d: d.update(constraints=[{"id": 10, "slot": 0}] * 2))
        for data in bad:
            with self.subTest(data=data), self.assertRaises(ValueError):
                replay.replay(data)

    def test_cli_exit_codes_hash_and_duplicate_fields(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "input.json"
            data = fixture()
            for expected, constraints in [(0, []), (1, [{"id": 10, "slot": 7}])]:
                data["constraints"] = constraints
                path.write_text(json.dumps(data))
                out = io.StringIO()
                with contextlib.redirect_stdout(out):
                    self.assertEqual(replay.main([str(path)]), expected)
                self.assertEqual(len(json.loads(out.getvalue())["input_sha256"]), 64)
            path.write_text('{"schema":"a","schema":"b"}')
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(replay.main([str(path)]), 2)
                path.unlink()
                self.assertEqual(replay.main([str(path)]), 2)


if __name__ == "__main__":
    unittest.main()
