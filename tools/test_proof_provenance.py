#!/usr/bin/env python3
"""Focused source-level tests for wb proof provenance."""

from __future__ import annotations

import pathlib
import tempfile
import sys
import unittest
from unittest import mock

import proof_provenance as provenance


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
        for module in ("parser","lexical"):
            with self.subTest(module=module),tempfile.TemporaryDirectory() as directory:
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
                    "try: proof.literal_array_facts(b'char label[]=\"x\";',b'char label[]=\"x\";',root)",
                    "except proof.MetadataProofError as error: print(error)",
                    "else: raise SystemExit('accepted changed preloaded code')",
                ])
                result=subprocess.run([sys.executable,"-c",script],capture_output=True,text=True)
                self.assertEqual(result.returncode,0,result.stderr+result.stdout)
                self.assertIn("preloaded" if module=="parser" else "lexical",result.stdout)

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

if __name__ == "__main__":
    unittest.main()
