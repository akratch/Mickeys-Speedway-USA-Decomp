#!/usr/bin/env python3
"""Durable receipt identity, concurrency and real runner failure regressions."""
from __future__ import annotations

import contextlib
import concurrent.futures
import copy
import dataclasses
import importlib.util
import hashlib
import json
import os
import shlex
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
import tomllib
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import sweep_receipts as receipts

spec = importlib.util.spec_from_file_location("sweep_runner_test", Path(__file__).with_name("permute_batch.py"))
batch = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = batch
spec.loader.exec_module(batch)


class ReceiptTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="sweep-receipt-")
        self.root = Path(self.tmp.name)
        self.store = receipts.ReceiptStore(self.root / "store")
        self.inputs = {"schema": 1, "context": {"source": "fixture",
                       "identity": {"symbol": "fixture", "overlay": 1, "section": ".text", "offset": 16},
                       "tools": {"candidate_context": batch.context_tool_identity()}},
                       "search": {"minutes": 1}}
        self.result = {"func": "fixture", "ok": True, "base_score": 20,
                       "best_score": 10, "zero_found": False, "promoted": False}
        self.artifacts = {name: ("synthetic " + name).encode() for name in receipts.REQUIRED_ARTIFACTS}
        baseline = b"int fixture(void) { return 1; }"
        winner = b"int fixture(void) { return 2; }"
        self.artifacts.update({"baseline/compiled.c": baseline, "context/baseline.c": baseline,
                               "context/winner.c": winner, "best/source.c": winner})
        import candidate_context
        report = {"schema": "mickey-prepared-context-review-v1", "status": "unchanged",
                  "symbol": "fixture", "canonical_source_sha256": "fixture",
                  "baseline_source_sha256": hashlib.sha256(baseline).hexdigest(),
                  "winner_source_sha256": hashlib.sha256(winner).hexdigest(),
                  "comparator_identity": batch.context_tool_identity(),
                  "comparison": candidate_context.compare_context(baseline, winner, "fixture"), "reason": None}
        self.result["context_review"] = report
        self.artifacts["context/report.json"] = json.dumps(report).encode()
        self.artifacts["baseline/measurement.json"] = json.dumps({"returncode": 0,
            "source_sha256": report["baseline_source_sha256"],
            "object_sha256": hashlib.sha256(self.artifacts["baseline/compiled.o"]).hexdigest()}).encode()
        self.inputs["baseline_hashes"] = {name: hashlib.sha256(data).hexdigest()
                                          for name, data in self.artifacts.items() if name.startswith("baseline/")
                                          and name not in {"baseline/measurement.json"}}
        binding = {"inputs_sha256": receipts.digest(self.inputs), "run_id": "synthetic-run"}
        report["capture_binding"] = binding
        self.artifacts["context/report.json"] = json.dumps(report).encode()
        measurement = json.loads(self.artifacts["baseline/measurement.json"])
        measurement["binding"] = binding
        self.artifacts["baseline/measurement.json"] = json.dumps(measurement).encode()
        self.result["artifact_bundle"] = self.store.save_bundle(self.artifacts, complete=True, inputs=self.inputs)

    def tearDown(self):
        self.tmp.cleanup()

    def record(self, result=None, inputs=None):
        inputs = inputs or self.inputs
        key = receipts.digest(inputs)
        with self.store.claim(key) as owned:
            self.assertTrue(owned)
            self.store.record(inputs, result or self.result)
        return key

    def test_completed_and_best_survive_new_store_instance(self):
        key = self.record()
        self.record({**self.result, "best_score": 15})
        store = receipts.ReceiptStore(self.store.root)
        self.assertEqual(store.completed(key)["result"]["best_score"], 15)
        self.assertEqual(json.loads((store.directory(key) / "best.json").read_text())["score"], 10)
        self.assertEqual(len(list((store.directory(key) / "attempts").glob("*.json"))), 2)

    def test_failure_interrupt_and_unpromoted_zero_do_not_complete(self):
        for change in ({"ok": False, "error": "compiler died"}, {"stopped_batch": True},
                       {"zero_found": True, "best_score": 0}, {"promote_error": "proof failed"},
                       {"commit_error": "hook failed"}, {"base_score": None}):
            inputs = copy.deepcopy(self.inputs)
            inputs["search"]["case"] = str(change)
            key = self.record({**self.result, **change}, inputs)
            self.assertIsNone(self.store.completed(key))
            self.assertEqual(len(list((self.store.directory(key) / "attempts").glob("*.json"))), 1)

    def test_changed_identity_and_settings_do_not_resume(self):
        self.record()
        for change in ("overlay", "section", "offset"):
            inputs = copy.deepcopy(self.inputs)
            inputs["context"]["identity"][change] = "different"
            self.assertIsNone(self.store.completed(receipts.digest(inputs)))
        inputs = copy.deepcopy(self.inputs)
        inputs["search"]["minutes"] = 2
        self.assertIsNone(self.store.completed(receipts.digest(inputs)))
        self.assertTrue(self.store.descending(inputs["context"]))
        inputs["context"]["source"] = "changed"
        self.assertFalse(self.store.descending(inputs["context"]))

    def test_corrupt_receipt_is_not_reused(self):
        key = self.record()
        path = self.store.directory(key) / "complete.json"
        path.write_text("{unfinished")
        self.assertIsNone(self.store.completed(key))

    def test_malformed_receipt_container_shapes_fail_closed(self):
        valid = {"inputs": self.inputs, "result": self.result}
        for malformed in (None, []):
            with self.subTest(container="value", malformed=malformed):
                self.assertFalse(self.store.artifacts_valid(malformed))
            for field in ("inputs", "result", "search", "context"):
                value = copy.deepcopy(valid)
                container = value["inputs"] if field in ("search", "context") else value
                container[field] = malformed
                with self.subTest(container=field, malformed=malformed):
                    self.assertFalse(self.store.artifacts_valid(value))

    def test_missing_old_or_unbound_context_report_never_reuses(self):
        for fault in ("missing", "schema", "status", "comparator", "winner", "inner"):
            result = copy.deepcopy(self.result)
            artifacts = dict(self.artifacts)
            report = result["context_review"]
            if fault == "missing":
                result.pop("context_review")
            elif fault == "schema":
                report["schema"] = "old"
            elif fault == "status":
                report["status"] = "unverifiable"
            elif fault == "comparator":
                report["comparator_identity"] = {"old": True}
            elif fault == "winner":
                report["winner_source_sha256"] = "other"
            else:
                report["comparison"]["winner_sha256"] = "other"
            artifacts["context/report.json"] = json.dumps(report).encode()
            result["artifact_bundle"] = self.store.save_bundle(artifacts, complete=True, inputs=self.inputs)
            with self.subTest(fault=fault):
                self.assertFalse(self.store.artifacts_valid({"inputs": self.inputs, "result": result}))

    def test_null_and_list_context_or_capture_are_retryable_not_exceptions(self):
        for member in ("context/report.json", "baseline/measurement.json"):
            for malformed in (None, []):
                artifacts = dict(self.artifacts)
                result = copy.deepcopy(self.result)
                artifacts[member] = json.dumps(malformed).encode()
                if member == "context/report.json":
                    result["context_review"] = malformed
                result["artifact_bundle"] = self.store.save_bundle(artifacts, complete=True, inputs=self.inputs)
                with self.subTest(member=member, malformed=malformed):
                    key = self.record(result)
                    self.assertIsNone(self.store.completed(key))
                    self.assertFalse(self.store.descending(self.inputs["context"]))

    def test_missing_corrupt_and_symlink_bundles_reject_all_reuse(self):
        key = self.record()
        path = self.store.root / "bundles" / (self.result["artifact_bundle"] + ".zip")
        original = path.read_bytes()
        for fault in ("missing", "corrupt", "symlink"):
            with self.subTest(fault=fault):
                path.unlink(missing_ok=True)
                if fault == "corrupt":
                    path.write_bytes(b"corrupt")
                elif fault == "symlink":
                    other = self.root / "foreign.zip"
                    other.write_bytes(original)
                    path.symlink_to(other)
                self.assertIsNone(self.store.completed(key))
                self.assertFalse(self.store.descending(self.inputs["context"]))
        path.unlink()

    def test_immutable_bundle_and_partial_attempt_preserve_best(self):
        key = self.record()
        partial = self.store.save_bundle({"best/source.c": b"partial"}, complete=False, inputs=self.inputs)
        self.record({**self.result, "ok": False, "best_score": 15, "artifact_bundle": partial})
        best = json.loads((self.store.directory(key) / "best.json").read_text())
        self.assertEqual(self.store.read_bundle(best["result"]["artifact_bundle"]), self.artifacts)
        self.assertEqual(self.store.read_bundle(partial, require_complete=False), {"best/source.c": b"partial"})
        self.assertEqual(self.store.save_bundle(self.artifacts, complete=True, inputs=self.inputs), self.result["artifact_bundle"])
        with self.assertRaises(ValueError):
            self.store.save_bundle({"../escape": b"x"}, complete=False, inputs=self.inputs)

    def test_owned_reads_reject_parent_symlink_and_special_files(self):
        foreign = self.root / "foreign"
        foreign.mkdir()
        (foreign / "secret").write_text("not an artifact")
        owned = self.root / "owned"
        owned.mkdir()
        (owned / "escape").symlink_to(foreign, target_is_directory=True)
        with self.assertRaises(OSError):
            receipts.owned_bytes(owned, "escape/secret")
        files, errors = receipts.attempt_files(owned)
        self.assertEqual(files, {})
        self.assertTrue(errors)
        with self.assertRaises(ValueError):
            receipts.owned_bytes(owned, "../foreign/secret")

    def test_valid_foreign_bundle_cannot_be_substituted_in_same_receipt(self):
        key = self.record()
        other = copy.deepcopy(self.inputs)
        other["search"]["minutes"] = 9
        foreign = self.store.save_bundle(self.artifacts, complete=True, inputs=other)
        for path in (self.store.directory(key) / "complete.json",
                     self.store.root / "contexts" / receipts.digest(self.inputs["context"]) / (key + ".json")):
            value = json.loads(path.read_text())
            value["result"]["artifact_bundle"] = foreign
            path.write_text(json.dumps(value))
        self.assertIsNone(self.store.completed(key))
        self.assertFalse(self.store.descending(self.inputs["context"]))
        wrong_baseline = {**self.artifacts, "baseline/base.c": b"different baseline"}
        with self.assertRaisesRegex(ValueError, "baseline input binding"):
            self.store.save_bundle(wrong_baseline, complete=True, inputs=self.inputs)

    def test_lower_partial_failed_and_missing_bundle_cannot_replace_usable_best(self):
        key = self.record()
        best_path = self.store.directory(key) / "best.json"
        original = best_path.read_bytes()
        partial = self.store.save_bundle({"best/source.c": b"partial"}, complete=False, inputs=self.inputs)
        for bundle, ok in ((partial, True), ("f" * 64, True), (self.result["artifact_bundle"], False)):
            self.record({**self.result, "best_score": 5, "artifact_bundle": bundle, "ok": ok})
            self.assertEqual(best_path.read_bytes(), original)
        self.assertEqual(len(list((self.store.directory(key) / "attempts").glob("*.json"))), 4)
        self.assertEqual(json.loads((self.store.directory(key) / "partial-best.json").read_text())["score"], 5)

    def test_attempt_scanning_stops_at_byte_entry_and_deadline_caps(self):
        owned = self.root / "bounded"
        owned.mkdir()
        for index in range(20):
            (owned / str(index)).write_bytes(b"12345678")
        with patch.object(receipts, "owned_bytes", wraps=receipts.owned_bytes) as read:
            files, errors = receipts.attempt_files(owned, byte_budget=8)
        self.assertEqual(read.call_count, 1)
        self.assertEqual(len(files), 1)
        self.assertTrue(errors)
        with patch.object(receipts, "owned_bytes", wraps=receipts.owned_bytes) as read:
            files, errors = receipts.attempt_files(owned, entry_budget=2)
        self.assertEqual(read.call_count, 2)
        self.assertTrue(errors)
        with patch.object(receipts, "owned_bytes", side_effect=AssertionError("must not read")):
            files, errors = receipts.attempt_files(owned, deadline=time.monotonic() - 1)
        self.assertFalse(files)
        self.assertTrue(errors)

    def test_encoded_archive_cap_is_checked_before_publication(self):
        with patch.object(receipts, "MAX_PAYLOAD_BYTES", 100), patch.object(receipts, "MAX_ARTIFACT_BYTES", 200):
            with self.assertRaisesRegex(ValueError, "encoded bundle"):
                self.store.save_bundle({"small": b"x"}, complete=False, inputs=self.inputs)

    def test_deleted_origin_lane_and_concurrent_bundle_writers(self):
        origin = self.root / "origin-lane"
        origin.mkdir()
        files = {}
        for index, name in enumerate(sorted(receipts.REQUIRED_ARTIFACTS)):
            path = origin / str(index)
            path.write_bytes(self.artifacts[name])
            files[name] = receipts.owned_bytes(origin, path.name)
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            keys = list(pool.map(lambda _: self.store.save_bundle(files, complete=True, inputs=self.inputs), range(8)))
        self.assertEqual(len(set(keys)), 1)
        shutil.rmtree(origin)
        self.assertFalse(origin.exists())
        self.assertEqual(receipts.ReceiptStore(self.store.root).read_bundle(keys[0]), files)

    def test_cross_process_claim_and_crash_release(self):
        key = receipts.digest(self.inputs)
        code = ("import sys,time; from pathlib import Path; "
                "from sweep_receipts import ReceiptStore; "
                "s=ReceiptStore(Path(sys.argv[1])); "
                "c=s.claim(sys.argv[2]); print(c.__enter__(),flush=True); time.sleep(20)")
        env = dict(os.environ, PYTHONPATH=str(Path(receipts.__file__).parent))
        proc = subprocess.Popen([sys.executable, "-c", code, str(self.store.root), key],
                                env=env, stdout=subprocess.PIPE, text=True)
        try:
            self.assertEqual(proc.stdout.readline().strip(), "True")
            with self.store.claim(key) as owned:
                self.assertFalse(owned)
        finally:
            proc.kill()
            proc.wait()
            proc.stdout.close()
        with self.store.claim(key) as owned:
            self.assertTrue(owned)

    def test_common_directory_shared_by_linked_worktree(self):
        repo = self.root / "repo"
        repo.mkdir()
        env = dict(os.environ, GIT_CONFIG_GLOBAL=os.devnull, GIT_CONFIG_NOSYSTEM="1")
        def git(*args):
            subprocess.run(["git", "-C", str(repo), *args], check=True, env=env,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        git("init", "-q")
        git("-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
            "commit", "--allow-empty", "-qm", "fixture")
        lane = self.root / "lane"
        git("worktree", "add", "-qb", "lane", str(lane))
        self.assertEqual(receipts.ReceiptStore.for_repo(repo).root,
                         receipts.ReceiptStore.for_repo(lane).root)


class RecipeTests(unittest.TestCase):
    def test_cached_comparator_cannot_claim_changed_on_disk_identity(self):
        import candidate_context
        with patch.object(batch, "_CONTEXT_IDENTITY_PIN", None), \
             patch.object(candidate_context, "identity", side_effect=[{"version": 1}, {"version": 2}]):
            self.assertEqual(batch.context_tool_identity(), {"version": 1})
            with self.assertRaisesRegex(RuntimeError, "restart the runner"):
                batch.context_tool_identity()

    def test_dependency_closure_rechecks_shadowing_transitive_and_missing_headers(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("early", "late"):
                (root / name).mkdir()
            source = root / "fixture.c"
            source.write_text('#include "outer.h"\nint fixture(void) { return 1; }')
            (root / "late/outer.h").write_text('#include "inner.h"\n')
            args = ("-nostdinc", "-I", "early", "-I", "late")
            with patch.object(batch, "ROOT", root):
                first = batch.source_dependencies(source, args)
                (root / "late/inner.h").write_text("typedef int value;\n")
                second = batch.source_dependencies(source, args)
                self.assertNotEqual(first, second)
                (root / "early/outer.h").write_text('#include "inner.h"\n')
                third = batch.source_dependencies(source, args)
                self.assertNotEqual(second, third)
                (root / "late/inner.h").write_text("typedef long value;\n")
                self.assertNotEqual(third, batch.source_dependencies(source, args))

    def test_unverifiable_include_forms_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "fixture.c"
            for text, args in (("#include HEADER\n", ("-nostdinc",)),
                               ('#include "absent.h"\n', ()),
                               ("int fixture;", ("-include", "forced.h")),
                               ("int fixture;", ("-I-",)),
                               ('#include_next "absent.h"\n', ("-nostdinc",)),
                               ('#import "absent.h"\n', ("-nostdinc",)),
                               ('%:include "absent.h"\n', ("-nostdinc",)),
                               ('#include \\ \n"absent.h"\n', ("-nostdinc",)),
                               ("??=include HEADER\n", ("-nostdinc",))):
                source.write_text(text)
                with self.subTest(text=text, args=args), patch.object(batch, "ROOT", root):
                    with self.assertRaises(RuntimeError):
                        batch.source_dependencies(source, args)

    def test_compact_includes_and_symlink_lookup_components(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "real").mkdir()
            header = root / "real/outer.h"
            header.write_text("typedef int value;\n")
            source = root / "fixture.c"
            args = ("-nostdinc", "-I", "real")
            with patch.object(batch, "ROOT", root):
                for spelling in ('"outer.h"', '<outer.h>'):
                    source.write_text("#include" + spelling + "\n")
                    self.assertIn("real/outer.h", batch.source_dependencies(source, args))
                (root / "link").symlink_to(root / "real", target_is_directory=True)
                with self.assertRaises(OSError):
                    batch.source_dependencies(source, ("-nostdinc", "-I", "link"))
                (root / "outer.h").symlink_to(header)
                source.write_text('#include"outer.h"\n')
                with self.assertRaises(OSError):
                    batch.source_dependencies(source, args)

    def test_full_argument_tail_reaches_importer_settings(self):
        source, obj = "src/fixture.c", "build/src/fixture.c.o"
        line = (".venv/bin/python tools/asm-processor/build.py tools/ido/cc -- "
                "tools/binutils/mips64-elf-as -32 -- -c -non_shared -G 0 "
                "-O2 -mips2 -DBUILD_VERSION=7 -DRAREDIFFS -I 'fixture include' "
                f"-Xphase,uopt,+ -Xphase,uopt,-O1 -o {obj} {source}")
        args = batch.compiler_arguments(line, source, obj)
        self.assertIn("-DBUILD_VERSION=7", args)
        self.assertIn("-DRAREDIFFS", args)
        self.assertIn("fixture include", args)
        self.assertIn("-Xphase,uopt,-O1", args)
        self.assertNotIn(source, args)
        self.assertNotIn(obj, args)
        recipe = batch.BuildRecipe(("-O2", "-mips2"), (), (), True, args)
        with tempfile.TemporaryDirectory(prefix="sweep-settings-") as tmp:
            settings = Path(tmp) / "settings.toml"
            batch.write_settings_toml(settings, recipe.flags, recipe=recipe)
            parsed = tomllib.loads(settings.read_text())
        self.assertEqual(batch.shlex.split(parsed["compiler_command"]),
                         ["tools/ido/cc", *args, "-DNON_MATCHING"])
        self.assertTrue(parsed["objdump_command"].startswith("tools/binutils/"))

    def test_unsupported_wrapper_and_shell_fail_closed(self):
        for line in ("ccache tools/ido/cc -c -O2 -mips2 -o out.o fixture.c",
                     "env MODE=test tools/ido/cc -c -O2 -mips2 -o out.o fixture.c",
                     "tools/ido/cc -c -O2 -mips2 -o out.o fixture.c && echo changed"):
            with self.assertRaises(ValueError):
                batch.compiler_arguments(line, "fixture.c", "out.o")

    def test_recipe_is_refreshed_after_same_path_flags_change(self):
        with tempfile.TemporaryDirectory(prefix="sweep-recipe-") as tmp:
            root = Path(tmp)
            source = root / "src/fixture.c"
            source.parent.mkdir()
            source.write_text("int fixture(void);\n")
            mtime = source.stat().st_mtime_ns
            def dry(value):
                return subprocess.CompletedProcess([], 0, stdout=(
                    "tools/ido/cc -c -O2 -mips2 " + value +
                    " -o build/src/fixture.c.o src/fixture.c\n"))
            with patch.object(batch, "ROOT", root), \
                 patch.object(batch, "bounded_capture", side_effect=[dry("-DVALUE=1"), dry("-DVALUE=2")]) as command:
                first = batch.build_recipe_for(source)
                second = batch.build_recipe_for(source)
            self.assertIn("-DVALUE=1", first.compiler_args)
            self.assertIn("-DVALUE=2", second.compiler_args)
            self.assertEqual(source.stat().st_mtime_ns, mtime)
            self.assertEqual(command.call_args.args[0][:4], ["gmake", "-n", "-W", "src/fixture.c"])
            self.assertTrue(command.call_args.kwargs["check"])

    def test_failed_dry_run_rejects_usable_compiler_line(self):
        with tempfile.TemporaryDirectory(prefix="failed-recipe-") as tmp:
            root = Path(tmp)
            source = root / "fixture.c"
            source.write_text("int fixture(void);\n")
            command = root / "gmake"
            command.write_text("#!/bin/sh\necho 'tools/ido/cc -c -O2 -mips2 -o build/fixture.c.o fixture.c'\nexit 2\n")
            command.chmod(0o755)
            with patch.object(batch, "ROOT", root), patch.dict(os.environ, {"PATH": str(root) + os.pathsep + os.environ["PATH"]}):
                with self.assertRaises(subprocess.CalledProcessError):
                    batch.build_recipe_for(source)


class ImportPreservationTests(unittest.TestCase):
    def test_preexisting_and_failed_imports_survive_retry(self):
        for fault in ("exit", "timeout", "cancel", "success"):
            with self.subTest(fault=fault), tempfile.TemporaryDirectory(prefix="import-preserve-") as tmp:
                root = Path(tmp)
                source = root / "fixture.c"
                source.write_text("int fixture(void);\n")
                stale = root / "nonmatchings/fixture"
                stale.mkdir(parents=True)
                (stale / "manual-best.c").write_text("manual candidate\n")
                output = root / "build/run"
                output.mkdir(parents=True)
                def importer(args, deadline):
                    stale.mkdir()
                    (output / "source-groups.json").write_text("{}")
                    (stale / "candidate.c").write_text("new candidate\n")
                    (stale / "compile.sh").write_text("#!/bin/sh\n")
                    if fault == "timeout":
                        raise subprocess.TimeoutExpired(args, 0)
                    if fault == "cancel":
                        raise KeyboardInterrupt()
                    return subprocess.CompletedProcess(args, 0 if fault == "success" else 7, "diagnostic")
                with patch.object(batch, "ROOT", root), patch.object(batch, "bounded_capture", side_effect=importer):
                    item = batch.QueueItem("fixture", source)
                    if fault == "success":
                        batch.run_import(item, output, output / "settings", output / "target")
                    else:
                        with self.assertRaises((RuntimeError, subprocess.TimeoutExpired, KeyboardInterrupt)):
                            batch.run_import(item, output, output / "settings", output / "target")
                self.assertEqual(next(output.glob("preexisting-import-*/manual-best.c")).read_text(), "manual candidate\n")
                pattern = "scratch/candidate.c" if fault == "success" else "failed-import-*/candidate.c"
                self.assertEqual(next(output.glob(pattern)).read_text(), "new candidate\n")
                self.assertFalse(stale.exists())


SYNTHETIC_SEARCH_BASELINE = '''import sys, tempfile, subprocess
from pathlib import Path
scratch = Path(sys.argv[-1])
with tempfile.NamedTemporaryFile(prefix="permuter", suffix=".c") as source, \\
     tempfile.NamedTemporaryFile(prefix="permuter", suffix=".o") as obj:
    source.write(b'#line 1 "synthetic-search"\\n' + (scratch / "base.c").read_bytes())
    source.flush()
    subprocess.run(["bash", str(scratch / "compile.sh"), source.name, "-o", obj.name], check=True)
'''


class RunnerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="sweep-runner-")
        self.root = Path(self.tmp.name)
        self.stack = contextlib.ExitStack()
        self.stack.enter_context(patch.object(batch, "_PROCESS_TOOLS_PIN", None))
        self.store = receipts.ReceiptStore(self.root / "receipts")
        for name, value in (("ROOT", self.root), ("BUILD_PERMUTER", self.root / "build/permuter"),
                            ("ATLAS_PATH", self.root / "config/overlays.us.json"),
                            ("HANDOFF_DIR", self.root / "docs/matching-triage-handoffs"),
                            ("PERMUTER_DIR", self.root / "permuter"),
                            ("PERMUTER_PY", self.root / "permuter/permuter.py"),
                            ("PYTHON", Path(sys.executable))):
            self.stack.enter_context(patch.object(batch, name, value))
        self.write("src/fixture.c", "int fixture(void) { return 1; }\n")
        self.write("tools/ido/cc", "compiler fixture\n")
        self.write("tools/binutils/as", "assembler fixture\n")
        self.write("tools/binutils/mips64-elf-objdump", "#!/bin/sh\nprintf '%s\\n' \"$@\"\n").chmod(0o755)
        self.write("permuter/permuter.py", SYNTHETIC_SEARCH_BASELINE + "print('base score = 20', flush=True)\n")
        self.item = batch.QueueItem("fixture", self.root / "src/fixture.c")
        recipe = batch.BuildRecipe(("-O2", "-mips2"), (), (), True, ("-c", "-O2", "-mips2"))
        self.stack.enter_context(patch.object(batch, "build_recipe_for", return_value=recipe))
        self.stack.enter_context(patch.object(batch, "replicate_objcopy"))
        self.stack.enter_context(patch.object(batch, "prepare_target_asm", side_effect=self.target))
        self.stack.enter_context(patch.object(batch, "run_import", side_effect=self.importer))
        self.stack.enter_context(patch.object(batch, "annotate_overlay_scratch", return_value=0))
        # These orchestration fixtures copy C as their synthetic object. Real
        # parser/ELF fidelity is exercised separately in test_source_fidelity.
        self.stack.enter_context(patch.object(batch, "prepare_seed_layout", side_effect=lambda item, directory, source, deadline:
            (source, json.dumps({"contract": batch.SOURCE_GROUP_CONTRACT, "symbol": item.func,
                                 "groups": [], "status": "ungrouped"}).encode())))
        self.stack.enter_context(patch.object(batch, "prove_seed_emission", side_effect=self.synthetic_seed_fidelity))
        self.stack.enter_context(patch.object(batch, "validate_seed_search", side_effect=lambda item, out, seed, capture, inputs, deadline:
            {"contract": batch.SEED_FIDELITY_CONTRACT, "source_fidelity_exact": True,
             "owned_bytes": 4, "relocation_count": 0, "identity_route": "raw-source-symbols-not-runtime-proof",
             "inputs_sha256": receipts.digest(inputs), "measured_object_sha256": seed.object_sha256,
             "search_object_sha256": capture.object_sha256}))

    def synthetic_seed_fidelity(self, item, directory, baseline, inputs, source, seed, deadline):
        return source, {"contract": batch.SEED_FIDELITY_CONTRACT, "source_fidelity_exact": True,
            "owned_bytes": 4, "relocation_count": 0, "identity_route": "raw-source-symbols-not-runtime-proof",
            "inputs_sha256": receipts.digest(inputs),
            "original_source_sha256": hashlib.sha256(source).hexdigest(),
            "original_object_sha256": hashlib.sha256(source).hexdigest(),
            "emitted_source_sha256": seed.source_sha256, "emitted_object_sha256": seed.object_sha256,
            "recipe_sha256": hashlib.sha256(baseline["baseline/compile.sh"]).hexdigest()}

    def tearDown(self):
        self.stack.close()
        self.tmp.cleanup()

    def write(self, path, text):
        path = self.root / path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def target(self, item, out_dir):
        path = out_dir / "target.s"
        # Synthetic address metadata only, no target machine words.
        path.write_text("/* 100 80000100 fixture */\n")
        return path

    def importer(self, item, out_dir, settings, target, deadline):
        scratch = out_dir / "scratch"
        scratch.mkdir()
        (out_dir / "source-groups.json").write_text(json.dumps({
            "contract": batch.SOURCE_GROUP_CONTRACT, "symbol": item.func, "groups": [], "status": "ungrouped"}))
        (scratch / "base.c").write_text(item.c_file.read_text())
        (scratch / "base.o").write_bytes(b"synthetic baseline object")
        (scratch / "compile.sh").write_text(f'#!/bin/sh\ncd {self.root}\ncp "$1" "$3"\n')
        (out_dir / "importer-compile.sh").write_bytes((scratch / "compile.sh").read_bytes())
        (scratch / "settings.toml").write_text('compiler_type = "ido"\n'
            'objdump_command = "tools/binutils/mips64-elf-objdump -drz -m mips:4300"\n')
        shutil.copy(target, scratch / "target.s")
        (scratch / "target.o").write_bytes(b"synthetic target object")
        return scratch

    def run_one(self, **kwargs):
        return batch.run_one(self.item, 1, 1, 1, False, [], load_threshold=0,
                             annotate_overlays=False, receipt_store=self.store, **kwargs)

    def search_fault_program(self, program):
        return (SYNTHETIC_SEARCH_BASELINE + '\nif "--debug" in sys.argv:\n'
                '    print("base score = 20", flush=True)\n    raise SystemExit(0)\n' + program)

    def test_baseline_readiness_active_macro_refuses_before_random_search(self):
        self.item.c_file.write_text("#define VALUE 1\nint fixture(void) { return VALUE; }\n")
        with patch.object(batch, "run_permuter") as search:
            result = self.run_one()
        search.assert_not_called()
        self.assertFalse(result.ok)
        self.assertIn("baseline context readiness refused", result.error)
        saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        prefix = "attempt/baseline-readiness/"
        self.assertEqual(saved[prefix + "baseline.c"], saved[prefix + "baseline.o"])
        self.assertEqual(json.loads(saved[prefix + "report.json"])["status"], "unverifiable")
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_baseline_readiness_valid_launch_and_seeded_capture_reuse(self):
        parent = self.seed_parent_run()
        with patch.object(batch, "measure_seed_stage", wraps=batch.measure_seed_stage) as measured, \
             patch.object(batch, "run_permuter", wraps=batch.run_permuter) as search:
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertTrue(result.ok, result.error)
        self.assertEqual(search.call_count, 1)
        self.assertEqual([call.args[1].name for call in measured.call_args_list],
                         ["canonical-measurement", "seed-measurement"])
        self.assertIsNotNone(self.store.completed(result.receipt_key))

    def test_baseline_readiness_rechecks_wait_time_input_recipe_and_tool_drift(self):
        real_recipe = batch.build_recipe_for.return_value
        for fault in ("base.c", "target.o", "settings.toml", "recipe", "tool", "source"):
            def mutate(*args):
                scratch = next((self.root / "build/permuter/fixture/runs").glob("*/scratch"))
                if fault == "recipe":
                    batch.build_recipe_for.return_value = dataclasses.replace(real_recipe, flags=("-O1",))
                elif fault == "tool":
                    (self.root / "tools/ido/cc").write_text("changed compiler")
                elif fault == "source":
                    self.item.c_file.write_text("int fixture(void) { return 99; }\n")
                else:
                    (scratch / fault).write_bytes(b"changed input")
            # Each iteration owns a fresh disposable runner, not another lane.
            if fault != "base.c":
                self.tearDown()
                self.setUp()
                real_recipe = batch.build_recipe_for.return_value
            with self.subTest(fault=fault), patch.object(batch, "wait_for_headroom", side_effect=mutate), \
                 patch.object(batch, "run_permuter") as search:
                result = self.run_one()
            self.assertFalse(result.ok, fault)
            search.assert_not_called()

    def test_baseline_readiness_score_drift_refuses_completion(self):
        def search(*args, **kwargs):
            self.improved(*args, **kwargs)
            return 19, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search):
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("differs from context readiness", result.error)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_baseline_readiness_reuses_grouped_measurement(self):
        def grouped(item, out, scratch, inputs, deadline):
            baseline = {"baseline/" + name: (scratch / name).read_bytes()
                        for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml")}
            return batch.measure_seed_stage(item, out / "grouped-measurement", baseline,
                                            inputs, baseline["baseline/base.c"], deadline)
        with patch.object(batch, "grouped_baseline_fidelity", side_effect=grouped), \
             patch.object(batch, "measure_seed_stage", wraps=batch.measure_seed_stage) as measured:
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        self.assertEqual(measured.call_count, 1)
        self.assertEqual(measured.call_args.args[1].name, "grouped-measurement")

    def test_baseline_installed_capture_wrapper_drift_refuses_before_search(self):
        real = batch.run_prepared
        def replace(item, scratch, *args, **kwargs):
            (scratch / "compile.sh").write_bytes(b"changed installed recipe")
            return real(item, scratch, *args, **kwargs)
        with patch.object(batch, "run_prepared", side_effect=replace), \
             patch.object(batch, "run_permuter") as search:
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("prepared search input changed", result.error)
        search.assert_not_called()

    def test_real_search_then_resume_from_durable_receipt(self):
        first = self.run_one()
        self.assertTrue(first.ok, first.error)
        self.assertEqual(first.base_score, 20)
        recorded = self.store.completed(first.receipt_key)
        self.assertEqual(recorded["inputs"]["search"]["mandatory_args"],
                         list(batch.MANDATORY_PERMUTER_ARGS))
        self.assertIn("--no-ignore-branch-targets",
                      recorded["inputs"]["search"]["mandatory_args"])
        with patch.object(batch, "run_permuter", side_effect=AssertionError("must skip search")):
            second = self.run_one(resume=True)
        self.assertTrue(second.resumed, second.error)
        self.assertEqual(first.receipt_key, second.receipt_key)
        self.assertEqual(first.context_review["status"], "unchanged")
        self.assertEqual(second.context_review, first.context_review)
        self.assertTrue(Path(first.scratch_path).exists())

    def test_fidelity_failure_precedes_search_and_completed_resume(self):
        first = self.run_one()
        self.assertTrue(first.ok, first.error)
        for resume in (False, True):
            with self.subTest(resume=resume), patch.object(batch, "grouped_baseline_fidelity",
                    side_effect=RuntimeError("configured instruction fidelity failure")), patch.object(
                    batch, "run_permuter", side_effect=AssertionError("must not search")):
                failed = self.run_one(resume=resume)
            self.assertFalse(failed.ok)
            self.assertFalse(failed.resumed)
            self.assertFalse(failed.promoted)
            self.assertIn("instruction fidelity", failed.error)
            self.assertIsNotNone(self.store.completed(first.receipt_key))

    def seed_parent_run(self, seed_score=10, search_suffix="", debug_setup=""):
        # Fault injection belongs to the seeded stages, not the parent's new
        # independent readiness measurement.
        debug_setup = debug_setup.replace('if "--debug" in sys.argv:',
            'if "--debug" in sys.argv and "baseline-measurement" not in sys.argv[-1]:')
        self.write("permuter/permuter.py", debug_setup + SYNTHETIC_SEARCH_BASELINE + f'''
seeded = b"return 2" in (scratch / "base.c").read_bytes()
print("base score =", {seed_score} if seeded else 20, flush=True)
if "--debug" not in sys.argv and seeded:
    {search_suffix or "pass"}
''')
        with patch.object(batch, "run_permuter", side_effect=self.improved):
            parent = self.run_one()
        self.assertTrue(parent.ok, parent.error)
        self.assertIsNotNone(self.store.completed(parent.receipt_key))
        return parent

    def test_seed_debug_resolves_local_objdump_and_isolates_debug_outputs(self):
        parent = self.seed_parent_run(debug_setup='''
import sys, shlex, tomllib, subprocess
from pathlib import Path
if "--debug" in sys.argv:
    settings = tomllib.loads((Path(sys.argv[-1]) / "settings.toml").read_text())
    output = subprocess.check_output(shlex.split(settings["objdump_command"]), text=True)
    assert output.splitlines() == ["-drz", "-m", "mips:4300"]
    Path("debug_source.c").write_text("synthetic isolated debug source")
    Path("debug_compiled_object.o").write_bytes(b"synthetic isolated debug object")
''')
        result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertTrue(result.ok, result.error)
        out = Path(result.scratch_path).parent
        for name in ("canonical-measurement", "seed-measurement"):
            stage = out / name
            self.assertTrue((stage / "debug_source.c").is_file())
            self.assertTrue((stage / "debug_compiled_object.o").is_file())
            original = tomllib.loads((stage / "settings-original.toml").read_text())
            effective = tomllib.loads((stage / "scratch/settings.toml").read_text())
            self.assertTrue(original["objdump_command"].startswith("tools/"))
            self.assertEqual(shlex.split(effective["objdump_command"])[0],
                             str(self.root / "tools/binutils/mips64-elf-objdump"))
        self.assertFalse((self.root / "debug_source.c").exists())
        self.assertIsNotNone(self.store.completed(result.receipt_key))

    def test_failed_seed_debug_output_is_preserved_before_error(self):
        parent = self.seed_parent_run(debug_setup='''
import sys
if "--debug" in sys.argv:
    print("synthetic scorer failure stdout")
    print("synthetic scorer failure stderr", file=sys.stderr)
    raise SystemExit(7)
''')
        result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        output = (Path(result.scratch_path).parent / "canonical-measurement/debug.log").read_text()
        self.assertIn("failure stdout", output)
        self.assertIn("failure stderr", output)
        files = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        self.assertIn("attempt/canonical-measurement/debug.log", files)
        self.assertIsNotNone(self.store.completed(parent.receipt_key))

    def test_seed_stage_rejects_foreign_objdump_and_preserves_arguments(self):
        raw = b'compiler_type = "ido"\nobjdump_command = "tools/binutils/mips64-elf-objdump -drz -m mips:4300"\n'
        effective, mapping = batch.seed_stage_settings(raw)
        self.assertEqual(mapping["original"][1:], mapping["effective"][1:])
        self.assertEqual(tomllib.loads(effective.decode())["compiler_type"], "ido")
        with self.assertRaisesRegex(RuntimeError, "pinned local tool"):
            batch.seed_stage_settings(raw.replace(b"tools/binutils/mips64-elf-objdump", b"/foreign/objdump"))

    def test_seed_debug_timeout_and_cancel_leave_explicit_failure_log(self):
        parent = self.seed_parent_run()
        real = batch.bounded_capture
        for error in (subprocess.TimeoutExpired(["synthetic-debug"], 1, output=b"partial debug output"),
                      RuntimeError("batch cancelled")):
            def command(*args, **kwargs):
                if kwargs.get("cwd") is not None:
                    raise error
                return real(*args, **kwargs)
            with self.subTest(error=type(error).__name__), patch.object(batch, "bounded_capture", side_effect=command):
                result = self.run_one(seed_receipt=parent.receipt_key)
            self.assertFalse(result.ok)
            log = (Path(result.scratch_path).parent / "canonical-measurement/debug.log").read_text()
            self.assertIn("[seed stage failure] " + type(error).__name__, log)
            if isinstance(error, subprocess.TimeoutExpired):
                self.assertIn("partial debug output", log)
            self.assertIsNotNone(self.store.completed(parent.receipt_key))

    def test_real_seed_debug_child_output_survives_timeout_and_cancel(self):
        parent = self.seed_parent_run(debug_setup='''
import os, sys, time
from pathlib import Path
if "--debug" in sys.argv:
    Path("debug-child.pid").write_text(str(os.getpid()))
    print("real debug stdout sentinel", flush=True)
    print("real debug stderr sentinel", file=sys.stderr, flush=True)
    time.sleep(30)
''')
        for cancelled in (False, True):
            timer = threading.Timer(2, batch.CANCEL_EVENT.set) if cancelled else None
            if timer is not None:
                timer.start()
            try:
                result = self.run_one(seed_receipt=parent.receipt_key,
                    batch_deadline=time.monotonic() + (8 if cancelled else 2))
            finally:
                if timer is not None:
                    timer.cancel()
                    timer.join()
                batch.CANCEL_EVENT.clear()
            with self.subTest(cancelled=cancelled):
                self.assertFalse(result.ok)
                stage = Path(result.scratch_path).parent / "canonical-measurement"
                log = (stage / "debug.log").read_text()
                self.assertIn("real debug stdout sentinel", log)
                self.assertIn("real debug stderr sentinel", log)
                self.assertIn("[seed stage failure]", log)
                saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
                self.assertIn(b"real debug stdout sentinel", saved["attempt/canonical-measurement/debug.log"])
                pid = int((stage / "debug-child.pid").read_text())
                with self.assertRaises(ProcessLookupError):
                    os.kill(pid, 0)
                self.assertIsNotNone(self.store.completed(parent.receipt_key))

    def test_seed_flat_preserves_body_and_validated_durable_resume(self):
        parent = self.seed_parent_run()
        result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertTrue(result.ok, result.error)
        self.assertEqual((result.original_base_score, result.seed_score, result.base_score,
                          result.best_score, result.search_gain), (20, 10, 10, 10, 0))
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertIn(b"return 2", saved["best/source.c"])
        self.assertIn(b"return 1", saved["baseline/compiled.c"])
        self.assertEqual(saved["seed/search.c"], saved["seed/compiled.c"])
        self.assertIsNotNone(self.store.completed(result.receipt_key))
        self.assertNotEqual(parent.receipt_key, result.receipt_key)
        with patch.object(batch, "measure_seed_stage", side_effect=AssertionError("resume must skip")):
            resumed = self.run_one(seed_receipt=parent.receipt_key, resume=True)
        self.assertTrue(resumed.resumed, resumed.error)
        self.assertEqual(resumed.seed_score, 10)
        complete = self.store.completed(result.receipt_key)
        self.assertFalse(self.store.descending(complete["inputs"]["context"]))
        altered = copy.deepcopy(complete)
        altered["result"]["seed_proof"]["seed_score"] = 0
        self.assertFalse(self.store.artifacts_valid(altered))

    def alternate_parent_run(self, score=15, source="int fixture(void) { return 3; }\n"):
        self.write("permuter/permuter.py", SYNTHETIC_SEARCH_BASELINE + f'''
text = (scratch / "base.c").read_bytes()
print("base score =", {score} if b"return 3" in text else 10 if b"return 2" in text else 20, flush=True)
''')
        def outputs(scratch, *args, **kwargs):
            result = self.improved(scratch, *args, **kwargs)
            for ordinal in (1, 2):
                directory = scratch / f"output-15-{ordinal}"
                directory.mkdir()
                (directory / "score.txt").write_text("15\n")
                (directory / "source.c").write_text(source)
            return result
        with patch.object(batch, "run_permuter", side_effect=outputs):
            parent = self.run_one()
        self.assertTrue(parent.ok, parent.error)
        return parent

    def test_alternate_seed_preserves_parent_and_isolates_durable_resume(self):
        parent = self.alternate_parent_run()
        before = {p.relative_to(self.store.directory(parent.receipt_key)): p.read_bytes()
                  for p in self.store.directory(parent.receipt_key).rglob("*.json")}
        original, best = self.store.seed_parent(parent.receipt_key)
        selected_parent, source, selection = self.store.select_seed(parent.receipt_key, "output-15-1")
        self.assertEqual(original, selected_parent)
        self.assertIn(b"return 2", best)
        self.assertIn(b"return 3", source)
        self.assertEqual((selection["selected_score"], selection["parent_best_score"]), (15, 10))
        result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
        self.assertTrue(result.ok, result.error)
        self.assertEqual((result.original_base_score, result.seed_parent_score, result.seed_score,
                          result.best_score, result.search_gain), (20, 15, 15, 15, 0))
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertEqual(json.loads(saved["seed/parent.json"]), original)
        self.assertEqual(json.loads(saved["seed/selection.json"]), selection)
        self.assertEqual(saved["seed/source.c"], source)
        self.assertIsNotNone(self.store.completed(result.receipt_key))
        with patch.object(batch, "measure_seed_stage", side_effect=AssertionError("resume must skip")):
            resumed = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1", resume=True)
        self.assertTrue(resumed.resumed, resumed.error)
        second = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-2", resume=True)
        self.assertTrue(second.ok, second.error)
        self.assertFalse(second.resumed)
        self.assertNotEqual(second.receipt_key, result.receipt_key)
        default = self.run_one(seed_receipt=parent.receipt_key, resume=True)
        self.assertTrue(default.ok, default.error)
        self.assertEqual(default.seed_score, 10)
        self.assertNotEqual(default.receipt_key, result.receipt_key)
        after = {p.relative_to(self.store.directory(parent.receipt_key)): p.read_bytes()
                 for p in self.store.directory(parent.receipt_key).rglob("*.json")}
        self.assertEqual(before, after)

    def test_alternate_selector_and_paired_archive_fail_closed(self):
        parent = self.alternate_parent_run()
        value = self.store.completed(parent.receipt_key)
        files = self.store.read_bundle(parent.artifact_bundle)
        for candidate in ("../output-15-1", "/output-15-1", "output-015-1", "output-15-0",
                          "output-15-1/source.c", "output-15-1\n", "output-15-999", "output-99999999999-1"):
            with self.subTest(candidate=candidate), self.assertRaises((ValueError, KeyError)):
                self.store.select_seed(parent.receipt_key, candidate)
        prefix = "attempt/scratch/output-15-1/"
        for member, data in (("score.txt", None), ("source.c", None), ("source.c", b""),
                ("source.c", b"\xff"), ("score.txt", b"10\n"), ("score.txt", b"True\n"),
                ("score.txt", b"15.0\n"), ("score.txt", b"15\nextra"), ("score.txt", b" 15\n")):
            altered = dict(files)
            if data is None:
                del altered[prefix + member]
            else:
                altered[prefix + member] = data
            with self.subTest(member=member, data=data), self.assertRaises((ValueError, KeyError)):
                receipts.resolve_seed_candidate(value, altered, "output-15-1")
        altered = dict(files)
        altered["attempt/scratch/output-20-1/source.c"] = b"int fixture(void) { return 3; }"
        altered["attempt/scratch/output-20-1/score.txt"] = b"20\n"
        with self.assertRaises(ValueError):
            receipts.resolve_seed_candidate(value, altered, "output-20-1")
        with patch.object(self.store, "read_bundle", side_effect=ValueError("corrupt or incomplete")):
            with self.assertRaises(ValueError):
                self.store.select_seed(parent.receipt_key, "output-15-1")

    def test_alternate_durable_selection_and_parent_mutation_rejected(self):
        parent = self.alternate_parent_run()
        result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
        complete = self.store.completed(result.receipt_key)
        self.assertIsNotNone(complete)
        files = self.store.read_bundle(result.artifact_bundle)
        for name in ("seed/selection.json", "seed/source.c", "seed/plan.json", "seed/prepared.c",
                     "seed/fidelity.json", "seed/search-fidelity.json", "seed/original.o"):
            for data in (None, b"null", b"different"):
                changed = dict(files)
                if data is None:
                    del changed[name]
                else:
                    changed[name] = data
                altered = copy.deepcopy(complete)
                altered["result"]["artifact_bundle"] = self.store.save_bundle(changed, complete=True, inputs=complete["inputs"])
                with self.subTest(name=name, data=data):
                    self.assertFalse(self.store.artifacts_valid(altered))
        for field, new in (("candidate", "output-15-2"), ("selected_score", 10),
                           ("source_sha256", "0" * 64), ("score_member", "best/score.txt"),
                           ("score_sha256", "0" * 64), ("parent_best_score", 15),
                           ("schema", "unknown"), ("parent_receipt_sha256", "0" * 64)):
            altered = copy.deepcopy(complete)
            altered["inputs"]["search"]["seed"]["selection"][field] = new
            self.assertFalse(self.store.artifacts_valid(altered), field)
        changed = dict(files)
        parent_copy = json.loads(changed["seed/parent.json"])
        parent_copy["finished"] += 1
        changed["seed/parent.json"] = json.dumps(parent_copy).encode()
        altered = copy.deepcopy(complete)
        altered["result"]["artifact_bundle"] = self.store.save_bundle(changed, complete=True, inputs=complete["inputs"])
        self.assertFalse(self.store.artifacts_valid(altered))

    def test_alternate_preparation_and_search_failures_keep_parent(self):
        for score, error in ((0, None), (16, None), (15, "seed emission changed owned instruction fields"),
                             (15, "seed emission relocation identity is unproved")):
            batch._PROCESS_TOOLS_PIN = None
            parent = self.alternate_parent_run(score=score)
            original = self.store.completed(parent.receipt_key)
            guard = patch.object(batch, "prove_seed_emission", side_effect=RuntimeError(error)) if error else contextlib.nullcontext()
            with self.subTest(score=score, error=error), guard, patch.object(batch, "run_permuter") as search:
                result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
            self.assertFalse(result.ok)
            self.assertIn(error or "seed score differs", result.error)
            search.assert_not_called()
            self.assertEqual(self.store.completed(parent.receipt_key), original)
            saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
            self.assertIn("seed/selection.json", saved)

    def test_alternate_stale_context_and_changed_declaration_never_search(self):
        for stale in (False, True):
            batch._PROCESS_TOOLS_PIN = None
            parent = self.alternate_parent_run(source="extern int changed; int fixture(void) { return 3; }\n")
            if stale:
                self.item.c_file.write_text("int fixture(void) { return 4; }\n")
            with patch.object(batch, "run_permuter") as search:
                result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
            self.assertFalse(result.ok)
            self.assertIn("stale" if stale else "declarations differ", result.error)
            search.assert_not_called()

    def test_alternate_cli_and_direct_api_require_parent(self):
        with contextlib.redirect_stderr(__import__("io").StringIO()):
            for args in (["--seed-candidate", "output-15-1"],
                         ["--seed-receipt", "a" * 64, "--function", "fixture", "--seed-candidate", "../bad"]):
                with self.assertRaises(SystemExit):
                    batch.parse_args(args)
        parsed = batch.parse_args(["--seed-receipt", "a" * 64, "--function", "fixture",
                                   "--seed-candidate", "output-15-1"])
        self.assertEqual(parsed.seed_candidate, "output-15-1")
        with patch.object(batch, "run_import") as importer:
            failed = self.run_one(seed_candidate="output-15-1")
        self.assertIn("requires --seed-receipt", failed.error)
        importer.assert_not_called()

    def test_alternate_failed_or_cancelled_search_retains_selection_without_completion(self):
        parent = self.alternate_parent_run()
        before = self.store.completed(parent.receipt_key)
        for error in (RuntimeError("child exited 7"), RuntimeError("cancelled")):
            with self.subTest(error=str(error)), patch.object(batch, "run_permuter", side_effect=error), \
                    patch.object(batch, "promote") as promote:
                result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
            self.assertFalse(result.ok)
            self.assertIn(str(error), result.error)
            self.assertIsNone(self.store.completed(result.receipt_key))
            self.assertEqual(self.store.completed(parent.receipt_key), before)
            saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
            self.assertEqual(json.loads(saved["seed/selection.json"])["candidate"], "output-15-1")
            self.assertIn(b"return 3", saved["best/source.c"])
            promote.assert_not_called()

    def test_alternate_actual_capture_and_prelaunch_replacement_still_refuse(self):
        parent = self.alternate_parent_run()
        real = batch.run_prepared
        def replace(item, scratch, *args, **kwargs):
            (scratch / "base.c").write_text("int fixture(void) { return 99; }")
            return real(item, scratch, *args, **kwargs)
        with patch.object(batch, "run_prepared", side_effect=replace), patch.object(batch, "run_permuter") as search:
            result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
        self.assertFalse(result.ok)
        self.assertIn("prepared seed source changed", result.error)
        search.assert_not_called()
        with patch.object(batch, "validate_seed_search", side_effect=RuntimeError("search object relocation drift")), \
                patch.object(batch, "promote") as promote:
            result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
        self.assertFalse(result.ok)
        self.assertIn("search object relocation drift", result.error)
        self.assertIsNone(self.store.completed(result.receipt_key))
        promote.assert_not_called()

    def test_alternate_malformed_seed_parent_result_shapes_fail_closed(self):
        parent = self.alternate_parent_run()
        result = self.run_one(seed_receipt=parent.receipt_key, seed_candidate="output-15-1")
        complete = self.store.completed(result.receipt_key)
        self.assertIsNotNone(complete)
        files = self.store.read_bundle(result.artifact_bundle)
        for malformed in (None, [], "not-a-dict", 7):
            for field in ("seed", "parent", "result"):
                altered, changed = copy.deepcopy(complete), dict(files)
                if field == "seed":
                    altered["inputs"]["search"]["seed"] = malformed
                elif field == "parent":
                    changed["seed/parent.json"] = json.dumps(malformed).encode()
                else:
                    value = json.loads(changed["seed/parent.json"])
                    value["result"] = malformed
                    changed["seed/parent.json"] = json.dumps(value).encode()
                altered["result"]["artifact_bundle"] = self.store.save_bundle(changed, complete=True, inputs=altered["inputs"])
                with self.subTest(field=field, malformed=malformed):
                    self.assertFalse(self.store.artifacts_valid(altered))
            for value in (malformed, {"result": malformed, "inputs": {}}, {"result": {}, "inputs": malformed}):
                with self.assertRaises(ValueError):
                    receipts.resolve_seed_candidate(value, {}, "output-15-1")
        parent_value = self.store.completed(parent.receipt_key)
        parent_files = self.store.read_bundle(parent.artifact_bundle)
        for malformed in (None, [], "not-a-dict", 7):
            changed = {**parent_files, "context/report.json": json.dumps(malformed).encode()}
            with self.assertRaises(ValueError):
                receipts.resolve_seed_candidate(parent_value, changed, "output-15-1")

    def test_seed_unexpected_zero_refuses_before_search_or_promotion(self):
        parent = self.seed_parent_run(seed_score=0)
        with patch.object(batch, "promote") as promote, patch.object(batch, "run_permuter") as search:
            result = batch.run_one(self.item, 1, 1, 1, True, [], annotate_overlays=False,
                receipt_store=self.store, seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("seed score differs", result.error)
        search.assert_not_called()
        promote.assert_not_called()

    def test_seed_regression_and_same_score_fidelity_failure_never_search(self):
        for score, error in ((130, None), (10, "seed emission changed owned instruction fields"),
                             (10, "seed emission relocation identity is unproved")):
            # Each changed synthetic vendor stands for a fresh runner process.
            batch._PROCESS_TOOLS_PIN = None
            parent = self.seed_parent_run(seed_score=score)
            guard = patch.object(batch, "prove_seed_emission", side_effect=RuntimeError(error)) \
                if error else contextlib.nullcontext()
            with self.subTest(score=score, error=error), guard, patch.object(batch, "run_permuter") as search:
                result = self.run_one(seed_receipt=parent.receipt_key)
            self.assertFalse(result.ok)
            self.assertIn(error or "seed score differs", result.error)
            search.assert_not_called()
            self.assertIsNone(self.store.completed(result.receipt_key))
            self.assertIsNotNone(self.store.completed(parent.receipt_key))

    def test_seed_durable_fidelity_missing_corrupt_or_mutated_evidence_rejected(self):
        parent = self.seed_parent_run()
        result = self.run_one(seed_receipt=parent.receipt_key)
        complete = self.store.completed(result.receipt_key)
        self.assertIsNotNone(complete)
        files = self.store.read_bundle(result.artifact_bundle)
        for name in ("seed/fidelity.json", "seed/search-fidelity.json", "seed/plan.json", "seed/prepared.c", "seed/original.o"):
            for mutation in (None, b"null", b"different"):
                changed = dict(files)
                if mutation is None:
                    del changed[name]
                else:
                    changed[name] = mutation
                altered = copy.deepcopy(complete)
                altered["result"]["artifact_bundle"] = self.store.save_bundle(
                    changed, complete=True, inputs=complete["inputs"])
                with self.subTest(name=name, mutation=mutation):
                    self.assertFalse(self.store.artifacts_valid(altered))

    def test_seed_same_score_search_object_failure_refuses_validation_and_promotion(self):
        parent = self.seed_parent_run()
        with patch.object(batch, "validate_seed_search", side_effect=RuntimeError("seed emission changed source relocation identity")) as check, \
             patch.object(batch, "promote") as promote:
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("relocation identity", result.error)
        check.assert_called_once()
        promote.assert_not_called()
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_seed_prelaunch_prepared_source_replacement_refuses_search(self):
        parent = self.seed_parent_run()
        real = batch.run_prepared
        def replace(item, scratch, *args, **kwargs):
            (scratch / "base.c").write_bytes(b"int fixture(void) { return 99; }")
            return real(item, scratch, *args, **kwargs)
        with patch.object(batch, "run_prepared", side_effect=replace), \
             patch.object(batch, "run_permuter") as search:
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("prepared seed source changed", result.error)
        search.assert_not_called()
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_seed_search_failure_preserves_parent_and_is_retryable(self):
        parent = self.seed_parent_run(search_suffix="raise SystemExit(7)")
        result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("exited 7", result.error)
        self.assertIsNone(self.store.completed(result.receipt_key))
        self.assertIsNotNone(self.store.completed(parent.receipt_key))
        saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        self.assertIn(b"return 2", saved["best/source.c"])

    def test_seed_parent_context_rejects_stale_source_before_debug(self):
        parent = self.seed_parent_run()
        self.item.c_file.write_text("int fixture(void) { return 3; }\n")
        with patch.object(batch, "measure_seed_stage", side_effect=AssertionError("must not compile seed")):
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("stale", result.error)

    def test_seed_regressing_search_retains_measured_seed(self):
        parent = self.seed_parent_run()
        def regression(scratch, *args, **kwargs):
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            best = scratch / "output-15-1"
            best.mkdir()
            (best / "score.txt").write_text("15")
            (best / "source.c").write_text("int fixture(void) { return 3; }")
            return 10, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=regression):
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertTrue(result.ok, result.error)
        self.assertEqual(result.best_score, 10)
        files = self.store.read_bundle(result.artifact_bundle)
        self.assertIn(b"return 2", files["best/source.c"])
        self.assertTrue(any(b"return 3" in data for name, data in files.items() if name.endswith("source.c")))

    def test_failed_seed_outputs_keep_best_source_and_score_paired(self):
        parent = self.seed_parent_run()
        for assigned in (False, True):
            for score in (15, 10, 5):
                def search(scratch, *args, **kwargs):
                    subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
                    best = scratch / f"output-{score}-1"
                    best.mkdir()
                    (best / "score.txt").write_text(str(score))
                    (best / "source.c").write_text("int fixture(void) { return 3; }")
                    if not assigned:
                        raise RuntimeError("injected permuter exit 7 before result assignment")
                    return 11, 1, False, False  # assignment followed by seed-baseline proof failure
                with self.subTest(assigned=assigned, score=score), \
                     patch.object(batch, "run_permuter", side_effect=search):
                    result = self.run_one(seed_receipt=parent.receipt_key)
                    self.assertFalse(result.ok)
                    self.assertEqual(result.best_score, min(10, score))
                    files = self.store.read_bundle(result.artifact_bundle, require_complete=False)
                    self.assertIn(b"return 3" if score < 10 else b"return 2", files["best/source.c"])
                    self.assertTrue(any(b"return 3" in data for name, data in files.items()
                                        if name.startswith("attempt/") and name.endswith("source.c")))
                    self.assertIsNone(self.store.completed(result.receipt_key))
                    self.assertIsNotNone(self.store.completed(parent.receipt_key))

    def test_seed_cancellation_at_each_stage_preserves_parent_and_source(self):
        parent = self.seed_parent_run()
        original = self.item.c_file.read_bytes()
        real = batch.measure_seed_stage
        for stage in ("canonical-measurement", "seed-measurement", "search"):
            def measure(item, directory, *args):
                if directory.name == stage:
                    raise TimeoutError("injected stage deadline")
                return real(item, directory, *args)
            search = patch.object(batch, "run_permuter", side_effect=TimeoutError("injected search deadline")) \
                if stage == "search" else contextlib.nullcontext()
            with self.subTest(stage=stage), patch.object(batch, "measure_seed_stage", side_effect=measure), search:
                result = self.run_one(seed_receipt=parent.receipt_key)
                self.assertFalse(result.ok)
                self.assertIn("deadline", result.error)
                self.assertIsNone(self.store.completed(result.receipt_key))
                self.assertIsNotNone(self.store.completed(parent.receipt_key))
                self.assertEqual(self.item.c_file.read_bytes(), original)
                self.assertTrue(result.artifact_bundle)

    def test_seed_search_capture_drift_cannot_promote(self):
        parent = self.seed_parent_run()
        def drift(scratch, *args, **kwargs):
            (scratch / "base.c").write_text("int fixture(void) { return 99; }")
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            return 0, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=drift), patch.object(batch, "promote") as promote:
            result = batch.run_one(self.item, 1, 1, 1, True, [], annotate_overlays=False,
                receipt_store=self.store, seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("prepared search input changed during search", result.error)
        promote.assert_not_called()
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_seed_missing_parent_bundle_rejects_before_preparation(self):
        parent = self.seed_parent_run()
        with patch.object(self.store, "read_bundle", side_effect=ValueError("corrupt bundle")), \
             patch.object(batch, "run_import", side_effect=AssertionError("must not prepare")):
            result = self.run_one(seed_receipt=parent.receipt_key)
        self.assertFalse(result.ok)
        self.assertIn("missing, incomplete or corrupt", result.error)

    def test_seed_context_portability_is_narrow(self):
        parent = self.seed_parent_run()
        old = self.store.completed(parent.receipt_key)["inputs"]
        new = copy.deepcopy(old)
        new["baseline_hashes"]["baseline/compile.sh"] = "different lane cwd"
        new["context"]["baseline_hashes"]["baseline/compile.sh"] = "different lane cwd"
        new["context"]["tools"]["runner"] = "new proof implementation"
        self.assertTrue(batch.seed_context_compatible(old, new))
        for path in (("context", "source"), ("context", "recipe"),
                     ("context", "identity"), ("context", "dependencies"),
                     ("context", "prepared")):
            changed = copy.deepcopy(new)
            changed[path[0]][path[1]] = "stale"
            self.assertFalse(batch.seed_context_compatible(old, changed))
        new["context"]["tools"]["ido"] = "different compiler"
        self.assertFalse(batch.seed_context_compatible(old, new))

    def test_seed_cli_rejects_ambiguous_modes_before_work(self):
        key = "a" * 64
        for extra in ([], ["--function", "a", "--function", "b"],
                      ["--function", "a", "--jobs", "2"],
                      ["--function", "a", "--deep"],
                      ["--function", "a", "--extend-minutes", "1"],
                      ["--function", "a", "--", "--debug"]):
            with self.subTest(extra=extra), contextlib.redirect_stderr(__import__("io").StringIO()):
                with self.assertRaises(SystemExit):
                    batch.parse_args(["--seed-receipt", key, *extra])
        self.assertEqual(batch.parse_args(["--seed-receipt", key, "--function", "a"]).seed_receipt, key)

    def test_loaded_implementation_drift_before_first_run_refuses_preparation(self):
        loaded = {"runner": (Path(batch.__file__), "not-the-imported-digest")}
        with patch.object(batch, "_LOADED_IMPLEMENTATIONS", loaded), \
             patch.object(batch, "run_import", side_effect=AssertionError("must not prepare")):
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("loaded runner implementation changed", result.error)

    def test_deleted_origin_scratch_retains_recoverable_source_and_object(self):
        first = self.run_one()
        shutil.rmtree(self.root / "build")
        second = self.run_one(resume=True)
        self.assertTrue(second.resumed, second.error)
        saved = self.store.read_bundle(second.artifact_bundle)
        self.assertEqual(saved["baseline/base.o"], b"synthetic baseline object")
        self.assertEqual(saved["best/source.c"], saved["baseline/compiled.c"])
        self.assertNotEqual(saved["baseline/base.c"], saved["baseline/compiled.c"])
        self.assertNotEqual(saved["baseline/base.o"], saved["baseline/compiled.o"])
        self.assertEqual(saved["context/baseline.c"], saved["baseline/compiled.c"])
        self.assertEqual(json.loads(saved["context/report.json"]), first.context_review)

    def test_zero_baseline_uses_actual_capture_not_importer(self):
        self.write("permuter/permuter.py", SYNTHETIC_SEARCH_BASELINE + "print('base score = 0', flush=True)\n")
        result = self.run_one()
        self.assertTrue(result.ok, result.error)
        self.assertTrue(result.zero_found)
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertNotEqual(saved["baseline/base.c"], saved["context/winner.c"])
        self.assertEqual(saved["context/winner.c"], saved["baseline/compiled.c"])
        self.assertEqual(result.context_review["status"], "unchanged")

    def test_runner_changed_declaration_zero_blocks_apply_before_source_write(self):
        original = self.item.c_file.read_bytes()
        def search(scratch, *args, **kwargs):
            self.improved(scratch, *args, **kwargs)
            best = scratch / "output-0-1"
            best.mkdir()
            (best / "score.txt").write_text("0")
            (best / "source.c").write_text("extern int added; int fixture(void) { return 2; }")
            return 20, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search):
            result = batch.run_one(self.item, 1, 1, 1, True, [], load_threshold=0,
                                   annotate_overlays=False, receipt_store=self.store)
        self.assertTrue(result.zero_found)
        self.assertFalse(result.promoted)
        self.assertIn("context changed", result.promote_error)
        self.assertEqual(self.item.c_file.read_bytes(), original)
        self.assertEqual(result.context_review["status"], "changed")

    def test_late_capture_replacement_cannot_replace_frozen_bundle_pair(self):
        original = batch.retain_context
        def retain(directory, evidence, winner, report):
            original(directory, evidence, winner, report)
            if directory.name != "context-review":
                return
            capture = directory.parent / "baseline-capture"
            (capture / "compiled.c").write_text("int foreign(void) { return 9; }")
            (capture / "compiled.o").write_bytes(b"foreign object")
        with patch.object(batch, "retain_context", side_effect=retain):
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertNotIn(b"foreign", saved["baseline/compiled.c"])
        self.assertNotIn(b"foreign", saved["baseline/compiled.o"])
        self.assertEqual(saved["baseline/compiled.c"], saved["context/baseline.c"])
        self.assertIsNotNone(self.store.completed(result.receipt_key))

    def test_extension_keeps_initial_context_even_when_seed_has_changed_declaration(self):
        calls = 0
        def search(scratch, *args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 1:
                self.improved(scratch, *args, **kwargs)
                (scratch / "output-10-1/source.c").write_text("extern int added; int fixture(void) { return 2; }\n")
                return 20, 60, False, False
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            return 10, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search):
            result = self.run_one(extend_minutes=1)
        self.assertFalse(result.ok)
        self.assertIn("extension context readiness refused", result.error)
        self.assertEqual(calls, 1)
        self.assertFalse(result.extended)
        self.assertEqual(result.context_review["status"], "changed")
        saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        self.assertNotIn(b"added", saved["context/baseline.c"])
        self.assertIn(b"added", saved["context/winner.c"])
        self.assertIsNone(self.store.completed(result.receipt_key))

    def extension_program(self):
        self.write("permuter/permuter.py", SYNTHETIC_SEARCH_BASELINE +
            'print("base score =", 10 if b"return 2" in (scratch / "base.c").read_bytes() else 20, flush=True)\n')

    def assert_extension_input_drift(self, name, during_measurement=False):
        self.extension_program()
        calls = waits = 0
        def search(scratch, *args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 1:
                self.improved(scratch, *args, **kwargs)
                return 20, 60, False, False
            return 10, 1, False, False
        def mutate():
            scratch = next((self.root / "build/permuter/fixture/runs").glob("*/scratch"))
            if name == "cancel":
                batch.CANCEL_EVENT.set()
            else:
                (scratch / name).write_bytes(b"changed extension input")
        def wait(*args):
            nonlocal waits
            waits += 1
            if waits == 2 and not during_measurement:
                mutate()
        real = batch.measure_seed_stage
        def measured(*args, **kwargs):
            result = real(*args, **kwargs)
            if args[1].name == "extension-measurement" and during_measurement:
                mutate()
            return result
        try:
            with patch.object(batch, "run_permuter", side_effect=search), \
                 patch.object(batch, "wait_for_headroom", side_effect=wait), \
                 patch.object(batch, "measure_seed_stage", side_effect=measured):
                result = self.run_one(extend_minutes=1)
        finally:
            batch.CANCEL_EVENT.clear()
        self.assertEqual(calls, 1)
        self.assertFalse(result.ok)
        self.assertFalse(result.extended)
        self.assertEqual(result.best_score, 10)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_extension_second_wait_input_drift_refuses_launch(self):
        self.assert_extension_input_drift("target.o")

    def test_extension_second_wait_compiler_drift_refuses_launch(self):
        self.assert_extension_input_drift("compile.sh")

    def test_extension_measurement_target_drift_refuses_launch(self):
        self.assert_extension_input_drift("target.o", during_measurement=True)

    def test_extension_second_wait_cancel_preserves_best_without_launch(self):
        self.assert_extension_input_drift("cancel")

    def test_extension_actual_capture_source_or_score_drift_refuses_completion(self):
        self.extension_program()
        calls = 0
        def search(scratch, *args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 1:
                self.improved(scratch, *args, **kwargs)
                return 20, 60, False, False
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            return 9, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search):
            result = self.run_one(extend_minutes=1)
        self.assertEqual(calls, 2)
        self.assertFalse(result.ok)
        self.assertIn("extension baseline differs", result.error)
        self.assertEqual(result.best_score, 10)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_extension_fidelity_and_distinct_capture_preserve_original_baseline(self):
        self.extension_program()
        calls = 0
        def search(scratch, *args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 1:
                self.improved(scratch, *args, **kwargs)
                return 20, 60, False, False
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            return 10, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search), \
             patch.object(batch, "prove_seed_emission", side_effect=self.synthetic_seed_fidelity) as original_proof, \
             patch.object(batch, "validate_seed_search", wraps=batch.validate_seed_search) as search_proof:
            result = self.run_one(extend_minutes=1)
        self.assertTrue(result.ok, result.error)
        self.assertTrue(result.extended)
        original_proof.assert_called_once()
        search_proof.assert_called_once()
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertIn(b"return 1", saved["context/baseline.c"])
        self.assertIn(b"return 2", saved["attempt/extension-search/baseline-capture/compiled.c"])
        first = json.loads(saved["attempt/baseline-capture/capture.json"])["binding"]
        later = json.loads(saved["attempt/extension-search/baseline-capture/capture.json"])["binding"]
        self.assertNotEqual(first["run_id"], later["run_id"])
        self.assertIsNotNone(self.store.completed(result.receipt_key))

    def test_extension_same_score_emission_fidelity_failure_never_relaunches(self):
        self.extension_program()
        def search(scratch, *args, **kwargs):
            self.improved(scratch, *args, **kwargs)
            return 20, 60, False, False
        with patch.object(batch, "run_permuter", side_effect=search) as random, \
             patch.object(batch, "prove_seed_emission", side_effect=RuntimeError("wrong owned register or identity")):
            result = self.run_one(extend_minutes=1)
        random.assert_called_once()
        self.assertFalse(result.ok)
        self.assertFalse(result.extended)
        self.assertIn("wrong owned register or identity", result.error)
        self.assertEqual(result.best_score, 10)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_missing_failed_and_swapped_capture_cannot_be_checked(self):
        for fault in ("missing", "failed", "swapped", "foreign run"):
            def search(scratch, *args, **kwargs):
                result = self.improved(scratch, *args, **kwargs)
                capture = scratch.parent / "baseline-capture"
                if fault == "missing":
                    (capture / "capture.json").unlink()
                elif fault in {"failed", "foreign run"}:
                    metadata = json.loads((capture / "capture.json").read_text())
                    if fault == "failed":
                        metadata["returncode"] = 1
                    else:
                        metadata["binding"]["run_id"] = "another-valid-run"
                    (capture / "capture.json").write_text(json.dumps(metadata))
                else:
                    (capture / "compiled.c").write_text("int different(void) { return 1; }")
                return result
            with self.subTest(fault=fault), patch.object(batch, "run_permuter", side_effect=search):
                result = self.run_one()
            self.assertEqual(result.context_review["status"], "unverifiable")
            self.assertIsNone(self.store.completed(result.receipt_key))

    def test_missing_bundle_forces_real_runner_search_again(self):
        first = self.run_one()
        (self.store.root / "bundles" / (first.artifact_bundle + ".zip")).unlink()
        with patch.object(batch, "run_permuter", wraps=batch.run_permuter) as search:
            second = self.run_one(resume=True)
        self.assertTrue(second.ok, second.error)
        self.assertFalse(second.resumed)
        self.assertEqual(search.call_count, 1)

    def test_importer_object_debug_path_churn_does_not_change_search_identity(self):
        original = self.importer
        count = 0
        def importer(*args):
            nonlocal count
            scratch = original(*args)
            count += 1
            (scratch / "base.o").write_bytes(f"synthetic .mdebug preparation path {count}".encode())
            return scratch
        with patch.object(batch, "run_import", side_effect=importer):
            first = self.run_one()
            with patch.object(batch, "run_permuter", side_effect=AssertionError("must reuse identical inputs")):
                second = self.run_one(resume=True)
        self.assertTrue(second.resumed, second.error)
        self.assertEqual(first.receipt_key, second.receipt_key)
        saved = self.store.read_bundle(second.artifact_bundle)
        self.assertEqual(saved["baseline/base.o"], b"synthetic .mdebug preparation path 1")

    def improved(self, scratch, *args, **kwargs):
        subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
        best = scratch / "output-10-1"
        best.mkdir()
        (best / "score.txt").write_text("10\n")
        (best / "source.c").write_text("int fixture(void) { return 2; }\n")
        return 20, 1, False, False

    def test_best_source_compiled_once_and_original_baseline_preserved(self):
        with patch.object(batch, "run_permuter", side_effect=self.improved), \
             patch.object(batch, "bounded_capture", wraps=batch.bounded_capture) as compile_call:
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        self.assertEqual(compile_call.call_count, 2)  # readiness plus winning source
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertEqual(saved["best/source.c"], saved["best/object.o"])
        self.assertNotEqual(saved["best/source.c"], saved["baseline/base.c"])
        self.assertIn("baseline/recipe.json", saved)

    def test_capture_records_ast_input_and_final_recipe_not_importer_object(self):
        def postprocess(scratch, *args):
            with (scratch / "compile.sh").open("a") as stream:
                stream.write('printf final-postprocess >> "$3"\n')
        with patch.object(batch, "replicate_objcopy", side_effect=postprocess):
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        saved = self.store.read_bundle(result.artifact_bundle)
        source, obj = saved["baseline/compiled.c"], saved["baseline/compiled.o"]
        self.assertTrue(source.startswith(b'#line 1 "synthetic-search"'))
        self.assertEqual(obj, source + b"final-postprocess")
        self.assertEqual(saved["baseline/base.o"], b"synthetic baseline object")
        self.assertEqual(json.loads(saved["baseline/measurement.json"])["score"], 20)

    def test_preservation_failure_never_rescans_attempt_directory(self):
        with patch.object(self.store, "save_bundle", side_effect=OSError("disk full")), \
             patch.object(receipts, "attempt_files", wraps=receipts.attempt_files) as scan:
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertEqual(scan.call_count, 1)
        self.assertIn("refusing a second full scan", result.error)

    def test_real_vendor_compiler_interface_captures_its_exact_temporary_input(self):
        project = Path(batch.__file__).resolve().parent.parent
        vendor, python = project / "tools/permuter", project / ".venv/bin/python"
        if not (vendor / "src/compiler.py").is_file() or not python.exists():
            self.skipTest("optional local permuter checkout and venv unavailable")
        self.write("permuter/permuter.py", SYNTHETIC_SEARCH_BASELINE.replace(
            "synthetic-search", "actual-vendor") + "print('base score = 20', flush=True)\n")
        def search(scratch, *args, **kwargs):
            code = ('import sys,os;sys.path.insert(0,sys.argv[2]);'
                    'from src.compiler import Compiler;'
                    'c=Compiler(sys.argv[1]+"/compile.sh",show_errors=True,debug_mode=False);'
                    'o=c.compile("#line 1 \\\"actual-vendor\\\"\\nint fixture(void) { return 1; }\\n");'
                    'assert o;os.unlink(o)')
            subprocess.run([str(python), "-c", code, str(scratch), str(vendor)], check=True)
            return 20, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=search):
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertTrue(saved["baseline/compiled.c"].startswith(b'#line 1 "actual-vendor"'))
        self.assertEqual(saved["baseline/compiled.c"], saved["baseline/compiled.o"])

    def test_compile_failure_and_deadline_preserve_partial_best_retryably(self):
        real = batch.bounded_capture
        for error in (RuntimeError("compiler failed"), TimeoutError("deadline exhausted")):
            with self.subTest(error=error), patch.object(batch, "run_permuter", side_effect=self.improved), \
                 patch.object(batch, "bounded_capture", side_effect=lambda *a, **k:
                    real(*a, **k) if k.get("cwd") is not None else (_ for _ in ()).throw(error)):
                result = self.run_one()
            self.assertFalse(result.ok)
            self.assertIsNone(self.store.completed(result.receipt_key))
            saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
            self.assertIn("best/source.c", saved)
            self.assertNotIn("best/object.o", saved)
            self.assertIn("baseline/base.o", saved)

    def test_actual_exhausted_deadline_never_launches_best_compilation(self):
        # The deadline must expire after the search and before the best
        # compile. A one-second wall deadline and a 1.1 s sleep did that only
        # when preparation took under a second, which a loaded machine does
        # not guarantee. Exhaust it on the monotonic clock instead: the search
        # moves the clock an hour forward, deterministically.
        real = batch.bounded_capture
        real_clock = time.monotonic
        skew = [0.0]
        def search(*args, **kwargs):
            output = self.improved(*args, **kwargs)
            skew[0] = 3600.0
            return output
        launched = []
        def capture(*a, **k):
            if k.get("cwd") is not None:
                return real(*a, **k)
            launched.append(a[0] if a else k)
            raise AssertionError("must not launch")
        with patch.object(batch, "run_permuter", side_effect=search), \
             patch.object(batch.time, "monotonic", side_effect=lambda: real_clock() + skew[0]), \
             patch.object(batch, "bounded_capture", side_effect=capture):
            result = self.run_one(batch_deadline=real_clock() + 600)
        self.assertEqual(launched, [])
        self.assertFalse(result.ok)
        self.assertIsNone(self.store.completed(result.receipt_key))
        saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        self.assertIn("best/source.c", saved)
        self.assertNotIn("best/object.o", saved)

    def test_real_best_compiler_failure_retains_partial_output_and_log(self):
        original = self.importer
        def importer(*args):
            scratch = original(*args)
            (scratch / "compile.sh").write_text('#!/bin/sh\ncase "$1" in *artifact-compile-*) printf partial > "$3"; echo failure; exit 7;; esac\ncp "$1" "$3"\n')
            return scratch
        with patch.object(batch, "run_import", side_effect=importer), \
             patch.object(batch, "run_permuter", side_effect=self.improved):
            result = self.run_one()
        self.assertFalse(result.ok)
        saved = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        partial = [value for name, value in saved.items()
                   if name.startswith("attempt/artifact-compile-") and name.endswith("/object.o")]
        self.assertEqual(partial, [b"partial"])
        self.assertEqual(saved["attempt/artifact-compile.log"], b"failure\n")
        self.assertNotIn("best/object.o", saved)

    def test_best_recipe_receives_precreated_output_like_vendor_compiler(self):
        original = self.importer
        def importer(*args):
            scratch = original(*args)
            (scratch / "compile.sh").write_text('#!/bin/sh\ntest -f "$3" || exit 9\nOUTPUT="$(realpath "$3")"\ncp "$1" "$OUTPUT"\n')
            return scratch
        with patch.object(batch, "run_import", side_effect=importer), \
             patch.object(batch, "run_permuter", side_effect=self.improved):
            result = self.run_one()
        self.assertTrue(result.ok, result.error)
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertEqual(saved["best/source.c"], saved["best/object.o"])

    def test_import_lock_wait_observes_total_deadline(self):
        batch._IMPORT_LOCK.acquire()
        start = time.monotonic()
        try:
            result = self.run_one(batch_deadline=time.monotonic() + 0.05)
        finally:
            batch._IMPORT_LOCK.release()
        self.assertFalse(result.ok)
        self.assertTrue(result.stopped_batch)
        # Hang guard: the lock is never released, so an unobserved deadline
        # waits forever. Ten seconds separates that from a loaded machine.
        self.assertLess(time.monotonic() - start, 10)

    def test_raw_baseline_recipe_paths_conservatively_separate_lanes(self):
        first = self.run_one()
        original_root = self.root
        original_item = self.item
        lane = self.root / "other-lane"
        for name in ("src", "tools", "permuter"):
            shutil.copytree(self.root / name, lane / name)
        try:
            self.root = lane
            self.item = batch.QueueItem("fixture", lane / "src/fixture.c")
            with patch.object(batch, "ROOT", lane), \
                 patch.object(batch, "BUILD_PERMUTER", lane / "build/permuter"), \
                 patch.object(batch, "PERMUTER_DIR", lane / "permuter"), \
                 patch.object(batch, "PERMUTER_PY", lane / "permuter/permuter.py"):
                second = self.run_one(resume=True)
            self.assertFalse(second.resumed, second.error)
            self.assertTrue(second.ok, second.error)
            self.assertNotEqual(first.receipt_key, second.receipt_key)
        finally:
            self.root = original_root
            self.item = original_item

    def test_changed_source_compiler_permuter_and_prepared_context_retry(self):
        previous = self.run_one()
        for path in ("src/fixture.c", "tools/ido/cc", "permuter/weights.toml"):
            target = self.root / path
            target.write_text((target.read_text() if target.exists() else "") + "\n")
            if path != "src/fixture.c":
                rejected = self.run_one(resume=True)
                self.assertFalse(rejected.ok)
                self.assertIn("restart the runner", rejected.error)
                # A distinct process is needed after tool changes. Reset only
                # the fixture's lifetime pin to model that fresh invocation.
                batch._PROCESS_TOOLS_PIN = None
            result = self.run_one(resume=True)
            self.assertTrue(result.ok, result.error)
            self.assertFalse(result.resumed)
            self.assertNotEqual(result.receipt_key, previous.receipt_key)
            previous = result
        original = self.importer
        def changed_header(*args):
            scratch = original(*args)
            (scratch / "base.c").write_text("int fixture(void) { return 2; }\n")
            return scratch
        with patch.object(batch, "run_import", side_effect=changed_header):
            result = self.run_one(resume=True)
        self.assertNotEqual(result.receipt_key, previous.receipt_key)

    def test_header_expanded_path_literals_do_not_alias_placeholder_text(self):
        original = self.importer
        literal = str(self.root)
        def expanded(*args):
            scratch = original(*args)
            (scratch / "base.c").write_text('const char *header_value = ' + json.dumps(literal) + ';\n')
            return scratch
        with patch.object(batch, "run_import", side_effect=expanded):
            first = self.run_one()
            literal = "<repo>"
            second = self.run_one(resume=True)
        self.assertFalse(second.resumed)
        self.assertNotEqual(first.receipt_key, second.receipt_key)

    def test_extension_failure_retains_primary_best_scalar_and_artifact(self):
        self.extension_program()
        def primary(scratch, *args, **kwargs):
            subprocess.run([sys.executable, "-c", SYNTHETIC_SEARCH_BASELINE, str(scratch)], check=True)
            best = scratch / "output-10-1"
            best.mkdir()
            (best / "score.txt").write_text("10\n")
            (best / "source.c").write_text("int fixture(void) { return 2; }\n")
            return (20, 60, False, False)
        count = 0
        def search(*args, **kwargs):
            nonlocal count
            count += 1
            if count == 1:
                return primary(*args, **kwargs)
            raise RuntimeError("extension failed")
        with patch.object(batch, "run_permuter", side_effect=search):
            result = self.run_one(extend_minutes=1)
        self.assertEqual(count, 2)
        self.assertFalse(result.ok)
        self.assertEqual(result.best_score, 10)
        self.assertIsNone(self.store.completed(result.receipt_key))
        saved = json.loads((self.store.directory(result.receipt_key) / "partial-best.json").read_text())
        self.assertEqual(saved["score"], 10)
        self.assertTrue((Path(result.scratch_path) / "output-10-1/source.c").is_file())
        bundle = self.store.read_bundle(result.artifact_bundle, require_complete=False)
        self.assertEqual(bundle["context/baseline.c"], bundle["baseline/compiled.c"])
        self.assertEqual(bundle["context/winner.c"], bundle["best/source.c"])
        self.assertEqual(result.context_review["status"], "unchanged")

    def test_nonzero_process_exit_is_retryable_even_after_base_score(self):
        self.write("permuter/permuter.py", self.search_fault_program("print('base score = 20', flush=True)\nraise SystemExit(7)\n"))
        for _ in range(2):
            result = self.run_one(resume=True)
            self.assertFalse(result.ok)
            self.assertIn("exited 7", result.error)
            self.assertFalse(result.resumed)
            self.assertIsNone(self.store.completed(result.receipt_key))

    def test_process_exit_124_is_failure_not_runner_cap(self):
        self.write("permuter/permuter.py", self.search_fault_program("print('base score = 20', flush=True)\nraise SystemExit(124)\n"))
        result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("exited 124", result.error)

    def test_failed_parent_does_not_leave_term_ignoring_worker(self):
        self.write("permuter/permuter.py", self.search_fault_program("""import os, signal, time
read_fd, write_fd = os.pipe()
worker = os.fork()
if worker == 0:
    os.close(read_fd)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    os.write(write_fd, b'1')
    time.sleep(20)
    os._exit(0)
os.close(write_fd)
os.read(read_fd, 1)
print('worker =', worker, flush=True)
print('base score = 20', flush=True)
raise SystemExit(7)
"""))
        result = self.run_one()
        self.assertFalse(result.ok)
        log = (Path(result.scratch_path).parent / "permuter.log").read_text()
        worker = int(log.split("worker = ")[1].splitlines()[0])
        # SIGKILL is immediate; the poll only waits for the kernel to reap.
        # Ten seconds keeps that true under load and still far below the
        # worker's own 20 s sleep.
        deadline = time.monotonic() + 10
        while True:
            state = subprocess.run(["ps", "-p", str(worker), "-o", "stat="],
                                   capture_output=True, text=True).stdout.strip()
            if not state or state.startswith("Z"):
                break
            if time.monotonic() >= deadline:
                self.fail(f"our worker {worker} survived cleanup: {state}")
            time.sleep(0.02)

    def test_zero_exit_without_base_score_is_failure(self):
        self.write("permuter/permuter.py", self.search_fault_program("print('no measurement')\n"))
        result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("no base score", result.error)

    def test_batch_cap_is_not_a_completed_receipt(self):
        self.write("permuter/permuter.py", "import time\nprint('base score = 20', flush=True)\ntime.sleep(20)\n")
        start = time.monotonic()
        result = self.run_one(batch_deadline=start + 0.2)
        self.assertLess(time.monotonic() - start, 10)  # the child sleeps 20 s
        self.assertTrue(result.stopped_batch)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_capped_child_unflushed_baseline_print_is_retained(self):
        self.write("permuter/permuter.py", "import time\nprint('base score = 20')\ntime.sleep(20)\n")
        scratch = self.root / "scratch"
        scratch.mkdir()
        out_dir = self.root / "capped"
        out_dir.mkdir()
        start = time.monotonic()
        score, elapsed, flat, stopped = batch.run_permuter(scratch, out_dir, 0.005, 1, [])
        self.assertEqual(score, 20)
        self.assertLess(time.monotonic() - start, 10)  # the child sleeps 20 s
        self.assertFalse(flat)
        self.assertFalse(stopped)
        self.assertIn("base score = 20", (out_dir / "permuter.log").read_text())

    def test_child_failure_after_successful_capture_retains_pair_and_best_without_apply(self):
        original = self.item.c_file.read_bytes()
        self.write("permuter/permuter.py", self.search_fault_program('''
best = scratch / "output-10-1"
best.mkdir()
(best / "score.txt").write_text("10")
(best / "source.c").write_text("int fixture(void) { return 2; }\\n")
raise SystemExit(7)
'''))
        with patch.object(batch, "promote", side_effect=AssertionError("failed search must never promote")):
            result = batch.run_one(self.item, 1, 1, 1, True, [], load_threshold=0,
                                   annotate_overlays=False, receipt_store=self.store)
        self.assertFalse(result.ok)
        self.assertIn("permuter exited 7", result.error)
        self.assertIsNone(result.base_score)  # Never invent the missing log measurement.
        self.assertEqual(result.best_score, 10)  # This score exists in the saved output.
        self.assertFalse(result.promoted)
        self.assertEqual(result.context_review["status"], "unchanged")
        self.assertEqual(self.item.c_file.read_bytes(), original)
        saved = self.store.read_bundle(result.artifact_bundle)
        self.assertEqual(saved["baseline/compiled.c"], saved["context/baseline.c"])
        self.assertEqual(saved["baseline/compiled.o"], saved["baseline/compiled.c"])
        self.assertEqual(saved["context/winner.c"], saved["best/source.c"])
        self.assertIsNone(self.store.completed(result.receipt_key))
        partial = json.loads((self.store.directory(result.receipt_key) / "partial-best.json").read_text())
        self.assertFalse(self.store.descending(partial["inputs"]["context"]))

    def test_failed_later_search_preserves_prior_best_artifact(self):
        first = self.run_one()
        with patch.object(batch, "run_permuter", side_effect=RuntimeError("synthetic failure")):
            second = self.run_one()
        self.assertFalse(second.ok)
        self.assertNotEqual(first.scratch_path, second.scratch_path)
        self.assertTrue(Path(first.scratch_path).exists())
        self.assertIsNotNone(self.store.completed(first.receipt_key))

    def test_search_settings_change_does_not_resume(self):
        first = self.run_one()
        second = batch.run_one(self.item, 2, 1, 1, False, [], load_threshold=0,
                               annotate_overlays=False, receipt_store=self.store, resume=True)
        self.assertTrue(second.ok, second.error)
        self.assertFalse(second.resumed)
        self.assertNotEqual(first.receipt_key, second.receipt_key)

    def test_deep_selection_uses_current_prepared_context(self):
        with patch.object(batch, "run_permuter", side_effect=self.improved):
            first = self.run_one()
        self.assertTrue(first.ok, first.error)
        second = self.run_one(deep=True)
        self.assertFalse(second.deep_skipped)
        self.assertTrue(second.ok, second.error)
        self.item.c_file.write_text("int fixture(void) { return 2; }\n")
        third = self.run_one(deep=True)
        self.assertTrue(third.deep_skipped)

    def test_source_change_during_search_does_not_complete(self):
        def changing_search(*args, **kwargs):
            self.item.c_file.write_text("int fixture(void) { return 3; }\n")
            return 20, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=changing_search):
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("source changed", result.error)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_tools_change_during_search_does_not_complete(self):
        def changing_search(*args, **kwargs):
            self.write("tools/ido/cc", "changed compiler\n")
            return 20, 1, False, False
        with patch.object(batch, "run_permuter", side_effect=changing_search):
            result = self.run_one()
        self.assertFalse(result.ok)
        self.assertIn("tools changed", result.error)
        self.assertIsNone(self.store.completed(result.receipt_key))

    def test_batch_ignores_legacy_symbol_only_resume_and_returns_failure(self):
        self.write("permuter/permuter.py", self.search_fault_program("print('base score = 20', flush=True)\nraise SystemExit(7)\n"))
        summary = self.write("build/permuter/summary.json", json.dumps({"results": [{
            "func": "fixture", "c_file": "src/fixture.c", "overlay": None,
            "ok": True, "base_score": 20}]}))
        with patch.object(batch, "discover_queue", return_value=[self.item]), \
             patch.object(batch, "SUMMARY_JSON", summary), \
             patch.object(batch, "SUMMARY_TXT", summary.with_suffix(".txt")), \
             patch.object(receipts.ReceiptStore, "for_repo", return_value=self.store):
            code = batch.run_batch(["--resume", "--no-integration-ref-filter", "--order", "queue",
                                    "--minutes", "1", "--load-threshold", "0"])
        self.assertEqual(code, 1)
        result = json.loads(summary.read_text())["results"][0]
        self.assertIn("exited 7", result["error"])
        self.assertFalse(result["resumed"])

    def test_resume_limit_counts_searches_after_receipt_skips(self):
        second_source = self.write("src/fixture2.c", "int fixture2(void) { return 2; }\n")
        second_item = batch.QueueItem("fixture2", second_source)
        for jobs in (1, 2):
            with self.subTest(jobs=jobs):
                self.store = receipts.ReceiptStore(self.root / f"receipts-{jobs}")
                first = self.run_one()
                self.assertTrue(first.ok, first.error)
                summary = self.root / f"build/permuter/summary-{jobs}.json"
                with patch.object(batch, "discover_queue", return_value=[self.item, second_item]), \
                     patch.object(batch, "SUMMARY_JSON", summary), \
                     patch.object(batch, "SUMMARY_TXT", summary.with_suffix(".txt")), \
                     patch.object(receipts.ReceiptStore, "for_repo", return_value=self.store):
                    code = batch.run_batch(["--resume", "--limit", "1", "--jobs", str(jobs),
                        "--no-integration-ref-filter", "--order", "queue", "--minutes", "1",
                        "--load-threshold", "0", "--flat-minutes", "0",
                        "--permuter-threads", "1", "--no-overlay-annotate"])
                self.assertEqual(code, 0)
                results = json.loads(summary.read_text())["results"]
                self.assertEqual(len(results), 2)
                self.assertTrue(results[0]["resumed"])
                self.assertFalse(results[1]["resumed"])
                self.assertEqual(results[1]["base_score"], 20)

    def test_preparation_timeout_stops_command(self):
        start = time.monotonic()
        with self.assertRaises(subprocess.TimeoutExpired):
            batch.bounded_capture([sys.executable, "-c", "import time; time.sleep(20)"],
                                   start + 0.1)
        self.assertLess(time.monotonic() - start, 10)  # the child sleeps 20 s



class StopProcessGroupTests(unittest.TestCase):
    """Darwin's killpg reports EPERM for a group holding an unreaped zombie."""

    class Proc:
        pid = 424242

        def __init__(self):
            self.waits = []

        def wait(self, timeout=None):
            self.waits.append(timeout)
            return 0

    def test_eperm_from_a_zombie_member_is_not_a_failed_run(self):
        proc = self.Proc()
        calls = []
        def killpg(pid, sig):
            calls.append(sig)
            raise PermissionError(1, "Operation not permitted")
        with patch.object(batch.os, "killpg", side_effect=killpg):
            batch.stop_process_group(proc)
        # Both the TERM and the KILL sweep were still attempted, and the
        # parent was reaped: EPERM does not skip the cleanup that ends it.
        self.assertEqual(calls, [signal.SIGTERM, signal.SIGKILL])
        self.assertEqual(proc.waits, [15, None])

    def test_a_vanished_group_is_reaped_once(self):
        proc = self.Proc()
        with patch.object(batch.os, "killpg", side_effect=ProcessLookupError):
            batch.stop_process_group(proc)
        self.assertEqual(proc.waits, [None])


if __name__ == "__main__":
    unittest.main()
