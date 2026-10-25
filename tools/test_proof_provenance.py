#!/usr/bin/env python3
"""Focused source-level tests for wb proof provenance."""

from __future__ import annotations

import pathlib
import contextlib
import tempfile
import sys
import unittest
from unittest import mock
from types import SimpleNamespace

import proof_provenance as provenance


class CaptureDeadlineTests(unittest.TestCase):
    """Defined clock advances test budgets, not real compiler performance."""

    def setUp(self):
        import permute_batch as batch
        self.batch = batch
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)
        self.source = self.root / "src/overlays/o059/source.c"
        self.header = self.source.with_name("dependency.h")
        self.configured = self.root / "build/src/overlays/o059/source.c.o"
        self.linked = self.root / "build/image.elf"
        for name in ("Makefile", "mk/objects.mk", "baseroms/mickey.us.z64",
                     "config/overlays.us.json", "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt",
                     "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
                     "tools/rebind_elf_relocations.py", "tools/externalize_elf_section.py",
                     "tools/postprocess_guard.py", "tools/binutils/mips64-elf-objcopy",
                     "config/runlink-ort.us.txt", "tools/link_overlay_module.py",
                     "tools/runlink_pack.py", "tools/binutils/mips64-elf-ld",
                     "include/fixture.h", "tools/asm-processor/build.py"):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture\n")
        for path in (self.source, self.header, self.configured, self.linked):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("original fixture\n")
        self.clock = 1000.0
        self.operations = []
        self.compile_delay = self.recipe_delay = 0
        self.recipe_extra = ""
        stack = contextlib.ExitStack()
        self.addCleanup(stack.close)
        stack.enter_context(mock.patch.object(batch, "ROOT", self.root))
        stack.enter_context(mock.patch.object(batch, "checked_tool_identity"))
        stack.enter_context(mock.patch.object(batch, "sweep_tool_identity", return_value={"compiler": "fixture"}))
        stack.enter_context(mock.patch.object(batch, "bounded_capture", side_effect=self.command))
        stack.enter_context(mock.patch.object(batch, "source_dependencies", side_effect=self.dependencies))
        stack.enter_context(mock.patch.object(provenance, "producer_link_context", return_value={"inputs": {}}))
        stack.enter_context(mock.patch.object(provenance.time, "monotonic", side_effect=lambda: self.clock))
        stack.enter_context(mock.patch.dict(provenance.os.environ, {"PROMOTION_TRIAL": "0"}))

    def command(self, arguments, deadline, **_keywords):
        kind = "recipe" if arguments[0] == "gmake" else "compiler"
        self.operations.append((kind, deadline))
        self.batch.remaining_timeout(deadline)
        if kind == "recipe":
            target = self.configured.relative_to(self.root).as_posix()
            source = self.source.relative_to(self.root).as_posix()
            output = (f"tools/ido/cc -c -32 {self.recipe_extra} -o {target} {source}\n"
                      f"python3 tools/link_overlay_module.py {target}\n")
            self.clock += self.recipe_delay
        else:
            pathlib.Path(arguments[arguments.index("-o") + 1]).write_text("untouched compiler fixture\n")
            output = ""
            self.clock += self.compile_delay
        return SimpleNamespace(stdout=output)

    def dependencies(self, _source, _arguments, deadline):
        self.operations.append(("dependencies", deadline))
        self.batch.remaining_timeout(deadline)
        return {self.header.relative_to(self.root).as_posix(): provenance.sha256_file(self.header)}

    def capture(self):
        return provenance.capture_data_module_producer_raw(
            self.root, self.source, self.configured, self.linked, self.root / "build/image.map")

    def test_completed_capture_age_does_not_expire_a_new_bounded_revalidation(self):
        _raw, _plan, receipt, current = self.capture()
        self.assertEqual({deadline for _kind, deadline in self.operations}, {1120.0})
        self.assertEqual([kind for kind, _deadline in self.operations],
                         ["recipe", "recipe", "dependencies", "compiler", "recipe",
                          "dependencies", "recipe", "dependencies"])
        self.clock = 1121.0
        self.operations.clear()
        self.assertEqual(receipt["inputs"], current())
        self.assertEqual(self.operations, [("recipe", 1241.0), ("dependencies", 1241.0)])

    def test_aged_context_still_refuses_changed_source_dependency_or_recipe(self):
        import runlink_pack as rp
        for changed in ("source", "dependency", "recipe"):
            with self.subTest(changed=changed):
                self.clock = 1000.0
                self.recipe_extra = ""
                self.source.write_text("original fixture\n")
                self.header.write_text("original fixture\n")
                _raw, _plan, receipt, current = self.capture()
                self.clock = 1121.0
                if changed == "source": self.source.write_text("changed source\n")
                if changed == "dependency": self.header.write_text("changed dependency\n")
                if changed == "recipe": self.recipe_extra = "-DCHANGED"
                world = SimpleNamespace(data_import_contexts=[(receipt["inputs"], current)])
                with self.assertRaises((rp.PackError, provenance.MetadataProofError)):
                    rp.check_data_import_holds(world)
                self.assertIsNone(world._guarded_data_objects)

    def test_original_capture_never_renews_its_internal_budget(self):
        self.compile_delay = 121
        with self.assertRaisesRegex(TimeoutError, "whole-batch deadline"):
            self.capture()
        self.assertEqual({deadline for _kind, deadline in self.operations}, {1120.0})
        self.assertGreater(self.clock, 1120)

    def test_later_recipe_and_dependency_checks_share_one_new_budget(self):
        _raw, _plan, _receipt, current = self.capture()
        self.clock = 1121.0
        self.recipe_delay = 121
        self.operations.clear()
        with self.assertRaisesRegex(TimeoutError, "whole-batch deadline"):
            current()
        self.assertEqual(self.operations, [("recipe", 1241.0), ("dependencies", 1241.0)])


class EditedPrefixCaptureTests(unittest.TestCase):
    def test_actual_projection_command_and_loaded_holds_are_required(self):
        import dataclasses, hashlib
        import runlink_pack as rp
        from test_runlink_pack import ObjBuilder, GLOBAL, OBJECT
        link=provenance.metadata_link
        with tempfile.TemporaryDirectory() as temporary:
            root=pathlib.Path(temporary);directory=root/'build/edited-runlink/control-one'
            directory.mkdir(parents=True)
            source='build/src/main/provider.c.o'
            script='SECTIONS { .main_bss : { '+source+'(.bss); } }'
            for name in (source,'mickey.us.yaml','tools/binutils/mips64-elf-ld',
                         'tools/proof_provenance.py','tools/runlink_metadata_link.py','tools/runlink_pack.py'):
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text('held')
            (directory/'providers.ld').write_text(script)
            path=directory/'providers.elf';path.write_bytes(ObjBuilder().section('.main_bss',nobits=16)
                .sym('cell',0,4,GLOBAL,OBJECT,'.main_bss').build())
            map_path=directory/'providers.map';map_path.write_text('held map')
            image=rp.Elf(path);item=rp.MapInput('.main_bss','.bss',0,16,source)
            context={'elf':image,'map':[item],'holds':{path:provenance.sha256_file(path),map_path:provenance.sha256_file(map_path)}}
            command=['tools/binutils/mips64-elf-ld','-r','-T',str((directory/'providers.ld').relative_to(root)),
                '-Map',str(map_path.relative_to(root)),'-o',str(path.relative_to(root))]
            inputs={source:provenance.sha256_file(root/source)}
            with mock.patch.object(link,'edited_projection',return_value=(script,'')), \
                 mock.patch.object(link,'edited_program_selectors',return_value=[source]), \
                 mock.patch.object(link,'edited_projection_tools',return_value={}):
                capture=provenance.bind_edited_prefix(root,context,directory,command,script,[command],inputs)
                self.assertEqual(capture.current()['role'],'edited-provider-prefix')
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.bind_edited_prefix(root,context,directory,command+['extra'],script,[command],inputs)
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.bind_edited_prefix(root,context,directory,command,script+'changed',[command],inputs)
                for held in (root/source,map_path,directory/'providers.ld'):
                    old=held.read_bytes();held.write_bytes(b'changed')
                    with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                    held.write_bytes(old);capture.current()
                row=next(s for s in image.symbols if s.name=='cell');index=image.symbols.index(row)
                image.symbols[index]=dataclasses.replace(row,size=8)
                with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                image.symbols[index]=row;capture.current()
                context['map']=[dataclasses.replace(item,addr=4)]
                with self.assertRaisesRegex(provenance.MetadataProofError,'prefix changed'):capture.current()
                context['map']=[item]
                with mock.patch.object(provenance,'_capture_configured_raw',return_value='fresh') as raw:
                    self.assertEqual(provenance.capture_edited_provider_raw(root,root/'source.c',root/source,capture),'fresh')
                    self.assertIs(raw.call_args.kwargs['edited_prefix'],capture)
                    self.assertTrue(raw.call_args.kwargs['ordinary_provider'])
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.capture_edited_provider_raw(root,root/'source.c',root/source,{'hash':'forged'})


class SourceClassificationTests(unittest.TestCase):
    def classify(self, text: str, *, defines: tuple[str, ...] = ()) -> tuple[str, str]:
        return provenance.classify_source_selection(
            text,
            candidate_symbol="friendly",
            target_symbol="func_1234",
            defines=defines,
        )

    def test_ordinary_c_definition_is_admissible_class(self) -> None:
        text = """
        extern void friendly(int value);
        /* friendly( does not count. */
        void friendly(int value) { if (value) { friendly(value - 1); } }
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_nonmatching_define_selects_guarded_c(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )

    def test_default_build_selects_nonmatching_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_friendly_candidate_pairs_with_auto_named_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/different_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_promoted_symbol_ignores_another_functions_mixed_tu_fallback(self) -> None:
        text = """
        void promoted(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        kind, _reason = provenance.classify_source_selection(
            text,
            candidate_symbol="promoted",
            target_symbol="promoted",
            defines=(),
        )
        self.assertEqual(kind, provenance.ORDINARY_C)

    def test_unconditional_friendly_definition_ignores_another_fallback(self) -> None:
        # A promoted friendly-named body in a multi-function TU, beside another
        # function's guarded fallback: the fallback is not its alternative.
        text = """
        void friendly(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_bare_global_asm_is_fallback(self) -> None:
        text = '#pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")\n'
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_missing_symbol_fails_closed(self) -> None:
        kind, reason = self.classify("void another(void) {}\n")
        self.assertEqual(kind, provenance.UNKNOWN)
        self.assertIn("no active C definition", reason)

    def test_if_not_defined_selects_fallback_when_enabled(self) -> None:
        text = """
        #if !defined(NON_MATCHING)
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #else
        void friendly(void) { }
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )


class ManifestPrimitiveTests(unittest.TestCase):
    def test_hash_records_exact_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "candidate.o"
            path.write_bytes(b"compiled-c")
            self.assertEqual(
                provenance.sha256_file(path),
                "c5983ffc917d4649c1e0728fd58ff6be043dd548241563c16a8c0e7b43a082d7",
            )

    def test_source_facts_ignore_calls_and_comments(self) -> None:
        facts = provenance.source_facts(
            """
            // void friendly(void) {}
            void caller(void) { friendly(); }
            void friendly(void) { }
            """,
            "friendly",
        )
        self.assertEqual(len(facts.definitions), 1)

    def test_source_facts_see_kr_definitions(self) -> None:
        text = """
            void reset();
            void reset(a);
            void reset(state, count)
            State *state;
            s32 count;
            {
                reset(state, 1);
            }
            """
        facts = provenance.source_facts(text, "reset")
        self.assertEqual([row.line for row in facts.definitions], [4])
        # A prototype, a call, an initializer or a non-identifier parameter
        # list is never read as a K&R definition.
        for other in ("void reset(a); int x; {",
                      "int y = reset(a) ; int x = 1; {",
                      "void reset(int a) int a; {",
                      "void reset(a) ; {"):
            with self.subTest(other=other):
                self.assertEqual(provenance.source_facts(other, "reset").definitions, ())

    def test_source_view_reads_the_preprocessor_only_for_macro_spelled_definitions(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "src").mkdir()
            body = root / "src/entry.c"
            body.write_text("void entry(int a) { }\n")
            alias = root / "src/entryB.c"
            alias.write_text('#define entry entryB\n#include "entry.c"\n')
            other = root / "src/other.c"
            other.write_text("/* #define entryB */ void other(void) { }\n")
            expanded = '# 1 "src/entryB.c"\n# 1 "src/entry.c"\nvoid entryB(int a) { }\n'
            with mock.patch.object(provenance, "preprocessed_text", return_value=expanded) as cpp:
                self.assertEqual("source", provenance.source_view(body, "entry", root)[1])
                self.assertEqual(("preprocessed"), provenance.source_view(alias, "entryB", root)[1])
                self.assertEqual(1, len(provenance.source_facts_for(alias, "entryB", root).definitions))
                # A commented-out #define is not a spelling.
                self.assertEqual("source", provenance.source_view(other, "entryB", root)[1])
                self.assertEqual(alias, provenance._find_source(root, ("entryB",)))
                self.assertEqual({alias}, {call.args[0] for call in cpp.call_args_list})

    def test_guarded_include_uses_candidate_mode_and_retains_candidate_status(self):
        wrapper = ('#define shared friendly\n#ifdef NON_MATCHING\n'
                   '#include "shared.c"\n#else\n'
                   '#pragma GLOBAL_ASM("asm/func_1234.s")\n#endif\n')
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "wrapper.c"
            source.write_text(wrapper)
            for expanded, expected in (
                ("void friendly(void) {}\n", provenance.NON_MATCHING_C),
                ("void other(void) {}\n", provenance.UNKNOWN),
                ("void friendly(void) {}\nvoid friendly(void) {}\n", provenance.UNKNOWN),
                ('void friendly(void) {}\n#pragma GLOBAL_ASM("asm/func_1234.s")\n', provenance.UNKNOWN),
            ):
                with self.subTest(expanded=expanded), mock.patch.object(
                        provenance, "preprocessed_text", return_value=expanded) as cpp:
                    result = provenance.classify_source_file(
                        source, candidate_symbol="friendly", target_symbol="func_1234",
                        root=root, non_matching=True)
                    self.assertEqual(expected, result[0])
                    cpp.assert_called_once_with(source, root, non_matching=True)
            with mock.patch.object(provenance, "preprocessed_text",
                                   side_effect=provenance.MetadataProofError("missing include")):
                with self.assertRaisesRegex(provenance.MetadataProofError, "missing include"):
                    provenance.classify_source_file(
                        source, candidate_symbol="friendly", target_symbol="func_1234",
                        root=root, non_matching=True)
            source.write_text(wrapper.replace("func_1234.s", "wrong.s"))
            with mock.patch.object(provenance, "preprocessed_text") as cpp:
                self.assertEqual(provenance.UNKNOWN, provenance.classify_source_file(
                    source, candidate_symbol="friendly", target_symbol="func_1234",
                    root=root, non_matching=True)[0])
                cpp.assert_not_called()

    def test_preprocessed_text_runs_the_configured_compiler(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "tools/ido").mkdir(parents=True)
            cc = root / "tools/ido/cc"
            cc.write_text('#!/bin/sh\necho "$@"\n')
            cc.chmod(0o755)
            source = root / "src/a.c"
            source.parent.mkdir()
            source.write_text("")
            command = "tools/ido/cc -c -DX -I include -O2 -o build/src/a.c.o src/a.c"
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(command, ["-DX", "-I", "include", "-O2"], None)):
                self.assertEqual("-E -DX -I include -O2 src/a.c\n",
                                 provenance.preprocessed_text(source, root))
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(command, ["-DNON_MATCHING", "-I", "include"], None)) as discover:
                self.assertEqual("-E -DNON_MATCHING -I include src/a.c\n",
                                 provenance.preprocessed_text(source, root, non_matching=True))
                discover.assert_called_once_with(root, source, non_matching=True)
            with mock.patch.object(provenance, "_discover_compile_command",
                                   return_value=(None, [], "gmake dry-run failed")):
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.preprocessed_text(source, root)

    def build_fixture_manifest(self, source_text: str, *, emit_symbol: bool,
                               non_matching: bool = False) -> dict[str, object]:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "src" / "fixture.c"
            source.parent.mkdir(parents=True)
            source.write_text(source_text)
            build_dir = root / ("build_non_matching" if non_matching else "build")
            candidate = build_dir / "src" / "fixture.c.o"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"candidate-object")
            target = root / "target.o"
            target.write_bytes(b"target-object")
            fake_objdump = root / "objdump"
            symbol_line = "00000000 g F .text 00000004 friendly\\n" if emit_symbol else ""
            fake_objdump.write_text(
                "#!/bin/sh\n"
                "test \"$1\" = -t || exit 2\n"
                f"printf '{symbol_line}'\n"
            )
            fake_objdump.chmod(0o755)
            return provenance.build_manifest(
                root=root,
                mode="asm",
                source=source,
                symbol="friendly",
                candidate_symbol="friendly",
                candidate_build_dir=build_dir,
                candidate_object=candidate,
                target_object=target,
                candidate_artifact=candidate,
                target_artifact=target,
                objdump=fake_objdump,
            )

    def test_manifest_indirect_candidate_and_preprocessing_failure(self):
        wrapper = ('#define shared friendly\n#ifdef NON_MATCHING\n'
                   '#include "shared.c"\n#else\n'
                   '#pragma GLOBAL_ASM("asm/friendly.s")\n#endif\n')
        with mock.patch.object(provenance, "_discover_compile_command", return_value=(
                "tools/ido/cc -DNON_MATCHING", ["-DNON_MATCHING"], None)):
            with mock.patch.object(provenance, "preprocessed_text",
                                   return_value="void friendly(void) {}\n"):
                manifest = self.build_fixture_manifest(wrapper, emit_symbol=True, non_matching=True)
                self.assertTrue(manifest["exact_claim_allowed"])
                self.assertEqual(provenance.NON_MATCHING_C, manifest["selection"]["classification"])
            with mock.patch.object(provenance, "preprocessed_text",
                                   side_effect=provenance.MetadataProofError("cpp failed")):
                manifest = self.build_fixture_manifest(wrapper, emit_symbol=True, non_matching=True)
                self.assertFalse(manifest["exact_claim_allowed"])
                self.assertEqual(provenance.UNKNOWN, manifest["selection"]["classification"])
                self.assertIn("cpp failed", manifest["reasons"])

    def test_manifest_allows_tied_ordinary_c(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=True
        )
        self.assertTrue(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["verdict"], "c_evidence")
        self.assertIsNotNone(manifest["candidate_object"]["sha256"])
        self.assertIsNotNone(manifest["target_object"]["sha256"])

    def test_manifest_fails_closed_when_object_symbol_is_missing(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=False
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["exact_guard"], "assert_exact_false")
        self.assertIn("does not define", " ".join(manifest["reasons"]))

    def test_manifest_fails_closed_for_global_asm(self) -> None:
        manifest = self.build_fixture_manifest(
            '#pragma GLOBAL_ASM("asm/nonmatchings/x/friendly.s")\n',
            emit_symbol=True,
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(
            manifest["selection"]["classification"], provenance.GLOBAL_ASM
        )


class ProducerCaptureLifecycleTests(unittest.TestCase):
    def test_stale_final_output_does_not_block_bound_bootstrap_producer_inputs(self):
        import runlink_metadata_link as metadata
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            names = [metadata.BOOT_ELF, metadata.BOOT_MAP, metadata.BOOT_LOG,
                     "mickey.us.ld", "tools/link_diagnose.sh", "tools/run_logged.py",
                     "tools/binutils/mips64-elf-ld", "undefined_funcs_auto.us.txt",
                     "undefined_syms_auto.us.txt", "libultra_undefined_syms.us.txt",
                     "overlay_undefined_syms.us.txt"]
            for name in names:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("synthetic producer input\n")
            with mock.patch.object(metadata, "provenance_context", side_effect=
                    metadata.MetadataLinkError("stale final output")), mock.patch.object(
                    metadata, "link_command", return_value=["ordinary-bootstrap-link"]) as link:
                producer = provenance.producer_link_context(
                    root, root / metadata.BOOT_ELF, root / metadata.BOOT_MAP)
                self.assertEqual(producer["role"], "metadata-producer-input")
                self.assertIn(metadata.BOOT_ELF, producer["inputs"])
                link.assert_called_once_with(root, final=False)
                with self.assertRaisesRegex(provenance.MetadataProofError, "stale final output"):
                    provenance.generated_metadata_context(root)
                (root / metadata.BOOT_MAP).write_text("changed map\n")
                self.assertNotEqual(producer, provenance.producer_link_context(
                    root, root / metadata.BOOT_ELF, root / metadata.BOOT_MAP))
                with self.assertRaisesRegex(provenance.MetadataProofError, "ELF-map pair"):
                    provenance.producer_link_context(root, root / metadata.BOOT_ELF,
                                                     root / "build/mickey.us.map")

    def test_local_provider_declaration_is_one_stable_ordinary_object(self):
        root = pathlib.Path("/synthetic")
        name = "build/src/overlays/o001/labels.c.o"
        self.assertEqual(provenance.local_ro_provider_input("helper --local-ro-origin-object " + name, root), name)
        for value in ("/tmp/provider.o", "build/src/overlays/o001/../labels.c.o", "build/fresh/provider.o"):
            with self.subTest(value=value), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_input("helper --local-ro-origin-object " + value, root)

    def test_nonexec_end_profile_is_explicit_and_does_not_accept_generic_crop(self):
        target="build/src/overlays/o101/tail.c.o"
        command="python3 tools/trim_elf_section.py "+target+" .data 44"
        with self.assertRaises(provenance.MetadataProofError):
            provenance.metadata_filter_plan(command,target)
        self.assertEqual([("nonexec-end",(".data",44))],provenance.metadata_filter_plan(
            command+" --compiler-end-padding",target))

    def test_ordered_provider_group_uses_only_distinct_stable_real_input_paths(self):
        root = pathlib.Path("/synthetic")
        a, b = "build/src/overlays/o001/labels.c.o", "build/src/overlays/o001/tail.c.o"
        self.assertEqual((a, b), provenance.local_ro_provider_inputs(
            "helper --local-ro-provider-group " + a + " " + b, root))
        for args in (a, a + " " + a, a + " /tmp/tail.o", a + " " + b + " --output x",
                     a + " " + b + " --local-ro-origin-object " + a):
            with self.subTest(args=args), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_inputs("helper --local-ro-provider-group " + args, root)

    def test_mixed_provider_group_is_explicit_distinct_and_excludes_old_profiles(self):
        root=pathlib.Path("/synthetic")
        a,b="build/src/overlays/o057/menu.c.o","build/src/overlays/o057/literals.c.o"
        command="helper --local-ro-mixed-provider-group "+a+" "+b
        self.assertTrue(provenance.local_ro_mixed_group(command,root))
        self.assertEqual((a,b),provenance.local_ro_provider_inputs(command,root))
        for suffix in (" --local-data-aggregate", " --local-ro-origin-object "+a,
                       " --local-ro-provider-group "+a+" "+b):
            with self.subTest(suffix=suffix), self.assertRaises(provenance.MetadataProofError):
                provenance.local_ro_provider_inputs(command+suffix,root)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.local_ro_mixed_group("helper --local-ro-mixed-provider-group "+a+" "+b+" build/src/overlays/o057/extra.c.o",root)

    def test_source_objects_require_the_explicit_mixed_storage_case(self):
        root=pathlib.Path("/synthetic")
        group="--local-ro-mixed-provider-group build/src/overlays/o057/menu.c.o build/src/overlays/o057/literals.c.o"
        self.assertTrue(provenance.local_data_objects("helper --local-data-objects "+group,root))
        self.assertFalse(provenance.local_data_objects("helper "+group,root))
        for command in ("helper --local-data-objects", "helper --local-data-objects --local-data-objects "+group,
                        "helper --local-data-aggregate --local-data-objects "+group):
            with self.subTest(command=command),self.assertRaises(provenance.MetadataProofError):
                provenance.local_data_objects(command,root)

    def test_capture_roles_are_explicit_and_promotion_keeps_the_strict_default(self):
        arguments = [pathlib.Path("root") / name for name in
                     (".", "source.c", "configured.o", "build/mickey.us.elf")]
        with mock.patch.object(provenance, "_capture_configured_raw", return_value="captured") as capture:
            self.assertEqual(provenance.capture_configured_raw(*arguments, "", module_link=True),
                             "captured")
            self.assertNotIn("producer_map", capture.call_args.kwargs)
            capture.reset_mock()
            provenance.capture_data_module_producer_raw(*arguments, pathlib.Path("root/build/mickey.us.map"))
            self.assertEqual(capture.call_args.kwargs["producer_map"], pathlib.Path("root/build/mickey.us.map"))
            self.assertTrue(capture.call_args.kwargs["module_link"])
            capture.reset_mock()
            provenance.capture_data_provider_raw(*arguments, pathlib.Path("root/build/mickey.us.map"))
            self.assertTrue(capture.call_args.kwargs["ordinary_provider"])
            self.assertNotIn("module_link", capture.call_args.kwargs)



class MixedLocalDeclarationTests(unittest.TestCase):
    def test_explicit_one_provider_profile_is_required(self):
        root=pathlib.Path(__file__).resolve().parent.parent
        name="build/src/overlays/o058/labels.c.o"
        self.assertFalse(provenance.local_data_aggregate("helper --local-ro-origin-object "+name,root))
        self.assertTrue(provenance.local_data_aggregate("helper --local-data-aggregate --local-ro-origin-object "+name,root))
        for command in ("helper --local-data-aggregate", "helper --local-data-aggregate --local-data-aggregate --local-ro-origin-object "+name,
                        "helper --local-data-aggregate --local-ro-provider-group "+name):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.local_data_aggregate(command,root)


class LiteralArrayEndTests(unittest.TestCase):
    SOURCE = b'char first[] = "x";\nchar second[] = "yz";\n'

    @staticmethod
    def raw(*, tail=0, size=0, bad_type=False):
        import runlink_pack as rp
        from runlink.synth import ElfBuilder
        import struct
        data = bytearray(ElfBuilder().section(".data", b"x\0\0\0yz\0"+bytes(8)+bytes([tail]))
            .sym("first",0,size,1,2 if bad_type else 1,".data")
            .sym("second",4,0,1,1,".data").build())
        obj = rp.Elf("literal", data)
        struct.pack_into(">I",data,struct.unpack_from(">I",data,32)[0]+obj.section(".data")["index"]*40+8,3)
        return rp.Elf("literal", data)

    def test_closed_source_cpp_and_actual_unsized_arrays_derive_end(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        rows = provenance.literal_array_facts(self.SOURCE,self.SOURCE,root)
        raw = self.raw()
        section,end,bounds = provenance.literal_raw_end(raw,rows)
        self.assertEqual((section,end),(".data",7))
        self.assertEqual(bounds,[["first",0,2,0],["second",4,3,0]])
        import trim_elf_section as trim
        with self.assertRaises(ValueError):trim.compiler_end(raw,".data")

    def test_unsupported_source_and_effective_cpp_fail_closed(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        cases = [self.SOURCE+b'float carrier = 1.0f;',self.SOURCE.replace(b'"x"',b'"x" "y"'),
                 self.SOURCE.replace(b'"x"',b'L"x"'),self.SOURCE.replace(b'"x"',b'"x\\0"'),
                 self.SOURCE.replace(b'first[]',b'first[2]'),b'#define X "x"\nchar first[] = X;',
                 self.SOURCE.replace(b'char first',b'char *first')]
        for source in cases:
            with self.subTest(source=source),self.assertRaises(provenance.MetadataProofError):
                provenance.literal_array_facts(source,self.SOURCE,root)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.literal_array_facts(self.SOURCE,self.SOURCE.replace(b'char',b'unsigned char'),root)

    def test_actual_symbol_or_nonzero_tail_changes_refuse(self):
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        rows = provenance.literal_array_facts(self.SOURCE,self.SOURCE,root)
        for options in ({"tail":1},{"size":2},{"bad_type":True}):
            with self.subTest(options=options),self.assertRaises((provenance.MetadataProofError,ValueError)):
                provenance.literal_raw_end(self.raw(**options),rows)

    def test_literal_plan_keeps_source_identity_and_explicit_mode(self):
        target="build/src/overlays/o057/strings.c.o"
        command="python3 tools/trim_elf_section.py "+target+" .data 7 --literal-array-end "
        self.assertEqual(provenance.metadata_filter_plan(command+"src/overlays/o057/strings.c",target),
                         [("literal-end",(".data",7,"src/overlays/o057/strings.c"))])
        for source in ("src/other.c","/tmp/source.c","src/../source.c"):
            with self.assertRaises(provenance.MetadataProofError):
                provenance.metadata_filter_plan(command+source,target)

    def test_preloaded_parser_or_lexical_then_first_call_disk_mutation_refuses(self):
        import shutil,subprocess,json
        canonical=pathlib.Path(provenance.__file__).resolve().parent.parent
        for profile in ("literal", "static"):
          for module in ("parser","lexical"):
            with self.subTest(module=module, profile=profile),tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);tools=root/"tools";tools.mkdir()
                for name in ("proof_provenance.py","trim_elf_section.py","candidate_context.py"):
                    shutil.copy2(canonical/"tools"/name,tools/name)
                shutil.copytree(canonical/"tools/permuter/perm_pycparser",tools/"permuter/perm_pycparser",
                                ignore=shutil.ignore_patterns("__pycache__","*.pyc"))
                script = "\n".join([
                    "import sys, pathlib, hashlib",
                    "sys.path.insert(0,"+repr(str(canonical/"tools"))+")",
                    "import proof_provenance as proof",
                    "root=pathlib.Path("+repr(str(root))+")",
                    "sys.path.insert(0,str(root/'tools'))",
                    "sys.path.insert(0,str(root/'tools/permuter'))",
                    ("import perm_pycparser.c_parser" if module=="parser" else "import candidate_context"),
                    "path=root/"+repr("tools/permuter/perm_pycparser/c_parser.py" if module=="parser" else "tools/candidate_context.py"),
                    "before=path.read_bytes(); path.write_bytes(before+b'\\n# changed after loading\\n')",
                    "assert path.read_bytes()!=before",
                    ("try: proof.literal_array_facts(b'char label[]=\"x\";',b'char label[]=\"x\";',root)" if profile=="literal" else "try: proof.static_source_facts(b'static long x=0;',['-32'],root,{'x'})"),
                    "except proof.MetadataProofError as error: print(error)",
                    "else: raise SystemExit('accepted changed preloaded code')",
                ])
                result=subprocess.run([sys.executable,"-c",script],capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr+result.stdout)
                self.assertIn("preloaded" if module=="parser" else "lexical",result.stdout)

    def test_wrapped_quoted_header_shadow_matches_compiler_and_holds_chosen_input(self):
        import runlink_pack as rp
        import subprocess, shlex
        root = pathlib.Path(provenance.__file__).resolve().parent.parent
        if not (root/"tools/ido/cc").is_file() or not (root/".venv/bin/python").is_file():
            self.skipTest("actual IDO/wrapper unavailable")
        (root/"build").mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="literal-shadow-",dir=root/"build") as directory:
            base=pathlib.Path(directory);local=base/"source";global_=base/"include"
            local.mkdir();global_.mkdir()
            source=local/"probe.c";source.write_text('#include "choice.h"\nchar label[]="ok";\n')
            template="typedef {type} chosen; typedef char Check[(sizeof(chosen)==EXPECTED_WIDTH)?1:-1];\n"
            (local/"choice.h").write_text(template.format(type="short"))
            chosen=global_/"choice.h";chosen.write_text(template.format(type="int"))
            records=[]
            for wrapped,width,expected in ((True,4,"int"),(False,2,"short")):
                output=base/("wrapped.o" if wrapped else "bare.o")
                command=([".venv/bin/python","tools/asm-processor/build.py","tools/ido/cc","--",
                          "tools/binutils/mips64-elf-as","--"] if wrapped else ["tools/ido/cc"])
                command += ["-c","-G","0","-non_shared","-mips2","-O2","-32","-nostdinc",
                            "-I",str(global_),"-DEXPECTED_WIDTH="+str(width),"-o",str(output),
                            source.relative_to(root).as_posix()]
                result=subprocess.run(command,cwd=root,capture_output=True)
                self.assertEqual(result.returncode,0,result.stderr.decode(errors="replace"))
                record=provenance.prove_literal_arrays(rp.Elf(output),source,root,
                                                       compiler_command=shlex.join(command))
                self.assertIn("typedef "+expected+" chosen",record["cpp_text"])
                selected=(chosen if wrapped else local/"choice.h").relative_to(root).as_posix()
                rejected=(local/"choice.h" if wrapped else chosen).relative_to(root).as_posix()
                self.assertIn(selected,record["inputs"])
                self.assertNotIn(rejected,record["inputs"])
                self.assertEqual(record["end"],3)
                records.append((rp.Elf(output),record))
            chosen.write_text(chosen.read_text()+"/* selected header changed */\n")
            with self.assertRaises(provenance.MetadataProofError):
                provenance.validate_literal_proof(*records[0],root)
            self.assertEqual(provenance.validate_literal_proof(*records[1],root),3)

    def test_cpp_boundary_source_raw_and_parser_mutations_refuse(self):
        import runlink_pack as rp
        from types import SimpleNamespace
        import hashlib
        canonical = pathlib.Path(provenance.__file__).resolve().parent.parent
        for mutation in ("source","raw","parser"):
            with self.subTest(mutation=mutation),tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);(root/"src").mkdir();(root/"tools/ido").mkdir(parents=True)
                source=root/"src/strings.c";source.write_bytes(self.SOURCE)
                (root/"Makefile").write_text("fixture\n");(root/"tools/ido/cc").write_bytes(b"compiler")
                rawpath=root/"raw.o";rawpath.write_bytes(self.raw().data);raw=rp.Elf(rawpath)
                parser=root/"parser.py";parser.write_bytes(b"parser")
                def tool_inputs(_root):return {"parser.py":hashlib.sha256(parser.read_bytes()).hexdigest()}
                def cpp(*args,**kwargs):
                    path={"source":source,"raw":rawpath,"parser":parser}[mutation]
                    path.write_bytes(path.read_bytes()+b"changed")
                    return SimpleNamespace(returncode=0,stdout=self.SOURCE,stderr=b"")
                rows=[dict(name="first",const=False,literal="x"),dict(name="second",const=False,literal="yz")]
                with mock.patch.object(provenance,"literal_profile_inputs",side_effect=tool_inputs), \
                     mock.patch.object(provenance,"literal_array_facts",return_value=rows), \
                     mock.patch("permute_batch.source_dependencies",return_value={}), \
                     mock.patch.object(provenance.subprocess,"run",side_effect=cpp):
                    with self.assertRaises(provenance.MetadataProofError):
                        provenance.prove_literal_arrays(raw,source,root,
                          compiler_command="tools/ido/cc -c -nostdinc -o raw.o src/strings.c")


class StaticSourceExtentTests(unittest.TestCase):
    ROOT = pathlib.Path(provenance.__file__).resolve().parent.parent
    CPP = b'typedef long Base; typedef Base Counter; static Counter index=0; static Counter states[8];'

    def test_typedef_resolved_extent_is_independent_of_allocation(self):
        facts = provenance.static_source_facts(self.CPP, ['-32'], self.ROOT, {'index','states'})
        self.assertEqual([(r['name'],r['size']) for r in facts], [('index',4),('states',32)])
        short = provenance.static_source_facts(self.CPP.replace(b'[8]',b'[7]'), ['-32'], self.ROOT, {'states'})
        self.assertEqual(short[0]['size'],28)
        self.assertEqual(short[0]['count'],7)

    def test_missing_duplicate_nested_and_unsupported_types_fail_closed(self):
        cases = [b'typedef int Counter; static Counter index=0;',
                 b'typedef unsigned long Counter; static Counter index=0;',
                 b'static long *index=0;',b'static const long index=0;',
                 b'#pragma unknown_extent\nstatic long index=0;',
                 b'struct S { long x; }; static struct S index={0};',
                 b'static long index;',b'static long index=1;',
                 b'static long index[0];',b'static long index[];',
                 b'static long index[8][2];',b'static long index[8]={0};',
                 b'static long index[4+4];',b'static long index[99999999999];',
                 b'static long index=0; static long index=0;',
                 b'static long index=0; void f(void) { long index; }',
                 b'typedef long Counter; typedef long Counter; static Counter index=0;',
                 b'static long other=0;']
        for cpp in cases:
            with self.subTest(cpp=cpp), self.assertRaises(provenance.MetadataProofError):
                provenance.static_source_facts(cpp,['-32'],self.ROOT,{'index'})
        with self.assertRaises(provenance.MetadataProofError):
            provenance.static_source_facts(self.CPP,['-64'],self.ROOT,{'index'})

    def test_anchor_reader_preload_and_after_load_mutation_refuse(self):
        import shutil,subprocess
        for preload in (True,False):
            with self.subTest(preload=preload), tempfile.TemporaryDirectory() as directory:
                root=pathlib.Path(directory);(root/'tools').mkdir()
                path=root/'tools/static_symbols.py';shutil.copy2(self.ROOT/'tools/static_symbols.py',path)
                script='\n'.join([
                    'import sys,pathlib',
                    'sys.path.insert(0,'+repr(str(self.ROOT/'tools'))+')',
                    'import proof_provenance as pp',
                    'root=pathlib.Path('+repr(str(root))+')',
                    'sys.path.insert(0,str(root/"tools"))',
                    'import static_symbols' if preload else 'pp.static_anchor_module(root)',
                    'path=root/"tools/static_symbols.py"; path.write_bytes(path.read_bytes()+b"\\n# changed\\n")',
                    'try: pp.static_anchor_module(root)',
                    'except pp.MetadataProofError as error: print(error)',
                    'else: raise SystemExit("accepted changed anchor code")'])
                result=subprocess.run([sys.executable,'-c',script],capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr+result.stdout)
                self.assertIn('preloaded' if preload else 'changed',result.stdout)

    def fixture(self, **options):
        import runlink_pack as rp
        ss = provenance.static_anchor_module(self.ROOT)
        from test_static_symbols import build_object
        settings=dict(statics=(),placeholder=False,bss=(('states',0),),bss_flags=3,bss_size=32,
                      data_anchors=(('index',0),),data_size=16)
        settings.update(options)
        compiler=rp.Elf('compiler',build_object(**settings))
        raw=rp.Elf('prepared',ss.add_static_symbols(compiler.data)[0])
        return compiler,raw

    def test_exact_anchor_only_preserves_zero_sizes_and_refuses_stale_observations(self):
        import runlink_pack as rp
        compiler,raw=self.fixture()
        facts=provenance.static_source_facts(self.CPP,['-32'],self.ROOT,{'index','states'})
        with tempfile.TemporaryDirectory(dir=self.ROOT/'build') as directory:
            source=pathlib.Path(directory)/'input.c';source.write_bytes(self.CPP)
            inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            witness=provenance.static_source_witness(compiler,raw,raw,facts,'source','cpp',inputs,self.ROOT,'tool')
            self.assertEqual(witness.exact_anchor(raw,raw,'.data',0).semantic_size,4)
            self.assertEqual(witness.exact_anchor(raw,raw,'.data',0).allocation,16)
            for offset in (4,8,12,15,16,28,32):
                with self.subTest(offset=offset), self.assertRaises(provenance.MetadataProofError):
                    witness.exact_anchor(raw,raw,'.data',offset)
            with self.assertRaises(provenance.MetadataProofError):
                witness.exact_anchor(rp.Elf('same-bytes-new-observation',raw.data),raw,'.data',0)
            for symbol in raw.symbols:
                if symbol.name in ('index','states'):self.assertEqual(symbol.size,0)
            source.write_bytes(self.CPP.replace(b'[8]',b'[7]'))
            with self.assertRaisesRegex(provenance.MetadataProofError,'stale'):
                witness.exact_anchor(raw,raw,'.data',0)
            shorter=provenance.static_source_facts(source.read_bytes(),['-32'],self.ROOT,{'index','states'})
            fresh_inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            fresh=provenance.static_source_witness(compiler,raw,raw,shorter,'source7','cpp7',fresh_inputs,self.ROOT,'tool')
            self.assertEqual((fresh.exact_anchor(raw,raw,'.bss',0).semantic_size,raw.size('.bss')),(28,32))

    def test_missing_competing_or_changed_linked_anchor_refuses(self):
        import runlink_pack as rp
        facts=provenance.static_source_facts(self.CPP,['-32'],self.ROOT,{'index','states'})
        for options in ({'data_anchors':()}, {'data_anchors':(('index',0),('extra',4))}, {'data_anchors':(('index',0),('extra',0))},
                        {'data_anchors':(('index',0),),'existing_data':(('other',4,0,'local'),)}):
            with self.subTest(options=options):
                compiler,raw=self.fixture(**options)
                with self.assertRaises(provenance.MetadataProofError):
                    provenance.static_source_witness(compiler,raw,raw,facts,'source','cpp',(),self.ROOT,'tool')
        compiler,raw=self.fixture()
        linked=rp.Elf('linked-changed',raw.data)
        linked.symbols[next(i for i,symbol in enumerate(linked.symbols) if symbol.name=='index')].__dict__['size']=4
        with self.assertRaises(provenance.MetadataProofError):
            provenance.static_source_witness(compiler,raw,linked,facts,'source','cpp',(),self.ROOT,'tool')


class IntegerWordSourceTests(unittest.TestCase):
    ROOT = pathlib.Path(provenance.__file__).resolve().parent.parent
    CPP = b'''typedef long s32; typedef unsigned long u32;
long vertices[4]={1,2,3,4};
u32 pack(void) { union { const void *pointer; s32 bits; } address;
return ((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1))); }
'''
    ISOLATED = b'''typedef long s32; typedef unsigned long u32;
typedef struct Particle { float x; } Particle;
long vertices[4]={1,2,3,4};
u32 pack(Particle *pool) {
union { Particle *particle; const void *pointer; s32 bits; } address;
u32 word; address.particle=pool;
if(address.particle->x) word=0;
word=((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)));
address.particle=pool; word+=(u32)address.particle->x; return word; }
'''

    def isolated(self, cpp):
        return provenance.integer_word_facts(cpp, ['-32'], self.ROOT,
                                             profile='isolated-full-expression')

    def test_three_member_carrier_is_explicit_and_all_unrelated_uses_are_separate(self):
        with self.assertRaises(provenance.MetadataProofError):
            provenance.integer_word_facts(self.ISOLATED, ['-32'], self.ROOT)
        roles = self.isolated(self.ISOLATED)
        self.assertEqual([(r.function,r.target,r.carrier,r.occurrences) for r in roles],
                         [('pack','vertices','address',1)])
        for invalid in ('unknown', False, [], 1):
            with self.subTest(profile=invalid), self.assertRaises(provenance.MetadataProofError):
                provenance.integer_word_facts(self.ISOLATED,['-32'],self.ROOT,profile=invalid)

    def test_isolated_conversion_full_expression_roots_include_noncompound_bodies(self):
        head = self.ISOLATED.split(b'u32 pack')[0]
        union = b'union { Particle *particle; const void *pointer; s32 bits; } address;'
        expression = b'((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)))'
        statements = (b'u32 word; word=E; return word;', b'u32 word=E; return word;', b'return E;',
            b'if(E) return 1; return 0;', b'while(E) break; return 0;', b'do {} while(E); return 0;',
            b'switch(E){case 0:break;default:break;} return 0;',
            b'u32 word; for(word=E;0;) {} return 0;', b'for(;E;) break;return 0;',
            b'u32 word=1;for(;word;word=E) break;return 0;',
            b'u32 word;if(1) word=E;return 0;', b'u32 word;while(0) word=E;return 0;',
            b'u32 word;for(;0;) word=E;return 0;', b'u32 word;do word=E;while(0);return 0;',
            b'u32 word;label:word=E;return 0;',
            b'u32 word;switch(0){case 0:word=E;break;default:break;}return 0;',
            b'u32 word;switch(0){default:word=E;}return 0;')
        for statement in statements:
            with self.subTest(statement=statement):
                self.assertEqual(len(self.isolated(head+b'u32 pack(void){'+union+
                    statement.replace(b'E',expression)+b'}')),1)
        unsupported = head+b'u32 pack(void){'+union+b'switch(0){case '+expression+b':break;} return 0;}'
        with self.assertRaisesRegex(provenance.MetadataProofError,'full-expression'):
            self.isolated(unsupported)

    def test_full_expression_identity_sets_reject_other_arguments_and_second_conversions(self):
        expression = b'((address.pointer=(const void *)vertices),(u32)(address.bits ^ (-2147483647-1)))'
        for replacement in (b'combine('+expression+b', (u32)address.particle->x)',
                            b'combine('+expression+b','+expression+b')',
                            b'('+expression+b',address.pointer=0)',
                            b'combine(address.pointer=(const void *)vertices,(u32)(address.bits ^ (-2147483647-1)))'):
            changed = self.ISOLATED.replace(b'long vertices',b'u32 combine(u32,u32); long vertices')
            changed = changed.replace(expression,replacement)
            with self.subTest(replacement=replacement), self.assertRaises(provenance.MetadataProofError):
                self.isolated(changed)
        # The second carrier has different names, but its actual ID node still
        # shares the first conversion's full expression and must be rejected.
        changed=self.ISOLATED.replace(b'u32 word;',
            b'union { Particle *particle; const void *pointer; s32 bits; } other; u32 word;other.particle=pool;')
        changed=changed.replace(expression,b'combine('+expression+b',(u32)other.particle->x)')
        with self.assertRaisesRegex(provenance.MetadataProofError,'not isolated'):
            self.isolated(changed)

    def test_isolated_extra_pointer_types_escapes_shadows_and_uncovered_uses_refuse(self):
        changes = ((b'Particle *particle',b'void *particle'),
            (b'Particle *particle',b'Particle * const particle'),
            (b'Particle *particle',b'const Particle *particle'),
            (b'} address;',b'} address = {0};'),
            (b'} address;',b'} address; union { Particle *particle; const void *pointer; s32 bits; } address;'),
            (b'u32 word;',b'long vertices[4]; u32 word;'),
            (b'u32 word;',b'Particle **escape=&address.particle; u32 word;'),
            (b'u32 word;',b'void *escape=&address; u32 word;'),
            (b'u32 word;',b'u32 word; address.pointer=0;'),
            (b'address.particle=pool; word+=',b'address.particle=address.particle; word+='),
            (b'address.particle=pool; word+=',b'address.particle->x=0; word+='),
            (b'^',b'|'),(b'(const void *)vertices',b'(const void *)(vertices+1)'))
        for old,new in changes:
            with self.subTest(change=(old,new)), self.assertRaises(provenance.MetadataProofError):
                self.isolated(self.ISOLATED.replace(old,new))
        incomplete=self.ISOLATED.replace(b'typedef struct Particle { float x; } Particle;',
                                          b'typedef struct Particle Particle;')
        with self.assertRaises(provenance.MetadataProofError):self.isolated(incomplete)
        prototype_only = incomplete.replace(b'typedef struct Particle Particle;',
            b'typedef struct Particle Particle; void declared(struct Particle { float x; } *arg);')
        ca,parser,_pins,_lex = provenance._effective_parser(self.ROOT)
        parser.CParser().parse(prototype_only.decode())  # This is valid parser grammar.
        with self.assertRaisesRegex(provenance.MetadataProofError,'missing/ambiguous'):
            self.isolated(prototype_only)

    def test_extra_member_reads_cannot_hide_nested_aggregate_lvalues_or_addresses(self):
        for field,statement in ((b'float array[2];',b'address.particle->array[0]=1;'),
                                (b'float array[2];',b'float *escape=&address.particle->array[0];'),
                                (b'struct Inner {float x;} nested;',b'address.particle->nested.x=1;'),
                                (b'float *pointer;',b'*address.particle->pointer=1;')):
            source=self.ISOLATED.replace(b'float x; } Particle;',b'float x; '+field+b' } Particle;')
            source=source.replace(b'u32 word;',b'u32 word; address.particle=pool; '+statement)
            with self.subTest(statement=statement), self.assertRaises(provenance.MetadataProofError):
                self.isolated(source)

    def test_closed_expression_and_complete_carrier_target_coverage(self):
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        self.assertEqual([(r.function,r.target,r.carrier,r.occurrences) for r in roles],
                         [('pack','vertices','address',1)])
        # The role is an integer value, independently of a graphics sink.
        changed=self.CPP.replace(b'return ((',b'u32 word=((').replace(b')); }',b')); return word; }')
        self.assertEqual(provenance.integer_word_facts(changed,['-32'],self.ROOT),roles)
        for old,new in [(b'^',b'|'),(b'2147483647',b'2147483646'),
                        (b'(const void *)vertices',b'(const void *)(vertices+1)'),
                        (b's32 bits',b'u32 bits'),(b'return ((',b'address.pointer=0; return (('),
                        (b'return ((',b'long vertices[4]; return (('),
                        (b'const void *pointer',b'void *pointer'),
                        (b'(u32)(address.bits',b'(s32)(address.bits')]:
            with self.subTest(change=(old,new)), self.assertRaises(provenance.MetadataProofError):
                provenance.integer_word_facts(self.CPP.replace(old,new),['-32'],self.ROOT)
        with self.assertRaises(provenance.MetadataProofError):
            provenance.integer_word_facts(self.CPP,['-64'],self.ROOT)

    def test_argument_list_is_not_a_comma_but_one_comma_argument_is(self):
        positive=self.CPP.replace(b'long vertices[4]',b'u32 consume(u32); long vertices[4]')
        positive=positive.replace(b'return ((',b'return consume(((').replace(b'))); }',b')))); }')
        self.assertEqual(len(provenance.integer_word_facts(positive,['-32'],self.ROOT)),1)
        negative=self.CPP.replace(b'long vertices[4]',b'u32 consume(const void *,u32); long vertices[4]')
        negative=negative.replace(b'return ((address.pointer=',b'return consume(address.pointer=')
        negative=negative.replace(b'(const void *)vertices),',b'(const void *)vertices,')
        with self.assertRaisesRegex(provenance.MetadataProofError,'uncovered'):
            provenance.integer_word_facts(negative,['-32'],self.ROOT)

    def fixture(self):
        import struct,runlink_pack as rp
        from runlink.synth import ElfBuilder,words,GLOBAL,OBJECT,FUNC
        def image(linked):
            text=words(0x3c028000,0,0x24420010 if linked else 0x24420000,0)
            builder=(ElfBuilder().section('.text',text).section('.data',bytes(48))
                     .sym('pack',0,16,GLOBAL,FUNC,'.text').sym('vertices',16,16,GLOBAL,OBJECT,'.data'))
            if not linked:builder.rel('.text',0,5,'vertices').rel('.text',8,6,'vertices')
            data=bytearray(builder.build());shoff=struct.unpack_from('>I',data,32)[0]
            struct.pack_into('>I',data,shoff+2*40+8,3)
            return rp.Elf('linked' if linked else 'raw',bytes(data))
        return image(False),image(True)

    def test_pair_witness_holds_metadata_inputs_and_zero_object_operand(self):
        import runlink_pack as rp
        raw,linked=self.fixture();roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        with tempfile.TemporaryDirectory(dir=self.ROOT/'build') as temporary:
            source=pathlib.Path(temporary)/'source.c';source.write_bytes(self.CPP)
            inputs=((source.relative_to(self.ROOT).as_posix(),provenance.sha256_file(source)),)
            witness=provenance.integer_word_witness(raw,raw,linked,roles,(), 'source','cpp',inputs,self.ROOT,'tools')
            high=raw.rels('.text')[0]
            self.assertEqual(witness.operand(raw,linked,high,0x80000000,False).target_offset,16)
            self.assertEqual(rp.edited_object_operand(rp.edited_global_target(rp.edited_prepare_object(raw,linked),'vertices'),0),('.data',16))
            for addend,exported in ((0,False),(0x80000004,False),(0x80000000,True)):
                with self.assertRaises(provenance.MetadataProofError):witness.operand(raw,linked,high,addend,exported)
            symbol=next(s for s in linked.symbols if s.name=='vertices');original=symbol.size
            object.__setattr__(symbol,'size',4)
            with self.assertRaises(provenance.MetadataProofError):witness.operand(raw,linked,high,0x80000000,False)
            object.__setattr__(symbol,'size',original)
            source.write_bytes(self.CPP+b'\n')
            with self.assertRaisesRegex(provenance.MetadataProofError,'stale'):
                witness.operand(raw,linked,high,0x80000000,False)

    def test_original_compiler_target_and_late_metadata_remain_bound(self):
        import runlink_pack as rp
        raw,linked=self.fixture();compiler=rp.Elf('compiler',raw.data)
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        witness=provenance.integer_word_witness(compiler,raw,linked,roles,(),'source','cpp',(),self.ROOT,'tools')
        original=next(s for s in compiler.symbols if s.name=='vertices')
        object.__setattr__(original,'value',20)
        with self.assertRaisesRegex(provenance.MetadataProofError,'observations'):
            witness.operand(raw,linked,raw.rels('.text')[0],0x80000000,False)
        object.__setattr__(original,'value',16)
        for image in (raw,linked):
            object.__setattr__(next(s for s in image.symbols if s.name=='vertices'),'name','other')
        wrong=provenance.integer_word_facts(self.CPP.replace(b'vertices',b'other'),['-32'],self.ROOT)
        with self.assertRaisesRegex(provenance.MetadataProofError,'original compiler target'):
            provenance.integer_word_witness(compiler,raw,linked,wrong,(),'source','cpp',(),self.ROOT,'tools')

    def test_original_compiler_function_name_and_size_join(self):
        import runlink_pack as rp
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        for field,value in (('name','other_function'),('size',12)):
            raw,linked=self.fixture();compiler=rp.Elf('compiler',raw.data)
            function=next(s for s in compiler.symbols if s.name=='pack')
            object.__setattr__(function,field,value)
            with self.subTest(field=field), self.assertRaises((rp.PackError,provenance.MetadataProofError)):
                provenance.integer_word_witness(compiler,raw,linked,roles,(),'source','cpp',(),self.ROOT,'tools')

    def test_missing_unauthorized_pair_section_type_and_export_refuse(self):
        import runlink_pack as rp
        roles=provenance.integer_word_facts(self.CPP,['-32'],self.ROOT)
        for mutation in ('export','type','section','offset','register','low-type','wrong-name','extra-role'):
            raw,linked=self.fixture();exports=()
            target=next(s for s in raw.symbols if s.name=='vertices')
            if mutation=='export':exports=('vertices',)
            if mutation=='type':object.__setattr__(target,'type',rp.STT_FUNC)
            if mutation=='section':object.__setattr__(target,'section','.text')
            if mutation=='offset':object.__setattr__(target,'value',48)
            if mutation=='wrong-name':object.__setattr__(target,'name','other')
            if mutation in ('register','low-type'):
                data=bytearray(raw.data)
                import struct
                if mutation=='register':struct.pack_into('>I',data,raw.section('.text')['offset']+8,0x24620000)
                else:
                    rel=raw.section('.rel.text');info=struct.unpack_from('>I',data,rel['offset']+12)[0]
                    struct.pack_into('>I',data,rel['offset']+12,(info&~255)|2)
                raw=rp.Elf('mutated',bytes(data))
            use=roles+(provenance.IntegerWordRole('pack','unknown','address',1),) if mutation=='extra-role' else roles
            with self.subTest(mutation=mutation), self.assertRaises((rp.PackError,provenance.MetadataProofError)):
                provenance.integer_word_witness(raw,raw,linked,use,exports,'source','cpp',(),self.ROOT,'tools')

class ResidentPointerCellSourceTests(unittest.TestCase):
    ROOT = pathlib.Path(__file__).resolve().parent.parent

    def facts(self, text, definition=True, flags=('-32',)):
        return provenance.resident_cell_declarations(text.encode(), flags, self.ROOT, {'cell'}, definition=definition)

    def test_actual_pointer_type_with_header_extern_and_tentative_definition(self):
        text='typedef unsigned short u16; extern u16 *cell; u16 *cell;'
        self.assertEqual(self.facts(text), ('cell',))
        self.assertEqual(self.facts('typedef unsigned short u16; extern u16 *cell;',False), ('cell',))

    def test_source_size_or_a_name_does_not_supply_object_authority(self):
        for text in ('unsigned short cell[2];','unsigned long cell;','void *cell;',
                     'unsigned short *cell=0;','static unsigned short *cell;',
                     'unsigned short *cell; unsigned short *cell;',
                     'extern unsigned short *cell;', 'const unsigned short *cell;',
                     'unsigned short *cell; void f(void){unsigned short *cell;}',
                     '#pragma GLOBAL_ASM("proxy.s")\nunsigned short *cell;'):
            with self.subTest(text=text), self.assertRaises(provenance.MetadataProofError):
                self.facts(text)
        with self.assertRaises(provenance.MetadataProofError):
            self.facts('unsigned short *cell;',flags=('-n32',))
        with self.assertRaises(provenance.MetadataProofError):
            self.facts('unsigned short *cell;',False)


if __name__ == "__main__":
    unittest.main()
