#!/usr/bin/env python3
"""Original-coordinate preparation adapter regressions (no search)."""
import sys
import unittest
import json
import tempfile
import hashlib
import subprocess
from types import SimpleNamespace
import contextlib
from unittest.mock import patch
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as pb
sys.path.insert(0, str(pb.PERMUTER_DIR))
from src import ast_util
from perm_pycparser import c_ast


class IdoImportDialect(unittest.TestCase):
    def test_explicit_type_operators_round_trip_without_evaluation(self):
        types = ["int", "unsigned long long", "T", "const T *", "int[3]",
                 "int (*)(T)", "struct S", "double"]
        for operator in sorted(pb.IDO_IMPORT_TYPE_OPERATORS):
            for operand in types:
                source = ("typedef int T; struct S { int x; };\n"
                          f"int helper(void) {{ return {operator}({operand}); }}\n")
                with self.subTest(operator=operator, operand=operand), pb.ido_import_parser():
                    ast = ast_util.parse_c(source)
                    expression = ast.ext[-1].body.block_items[0].expr
                    self.assertIsInstance(expression, c_ast.UnaryOp)
                    self.assertEqual(expression.op, operator)
                    self.assertIsInstance(expression.expr, c_ast.Typename)
                    emitted = ast_util.to_c_raw(ast)
                    self.assertIn(operator + "(", emitted)
                    self.assertEqual(ast_util.to_c_raw(ast_util.parse_c(emitted)), emitted)

    def test_type_shadowing_malformed_and_reserved_identifier_refuse(self):
        sources = [
            "int f(void) { return __builtin_classof(); }",
            "int f(void) { return __builtin_classof(int, int); }",
            "int f(int x) { return __builtin_alignof(x); }",
            "typedef int T; int f(void) { int T; return __builtin_classof(T); }",
            "int __builtin_classof;",
            "int __builtin_alignof(int x);",
        ]
        for source in sources:
            with self.subTest(source=source), pb.ido_import_parser():
                with self.assertRaises(ast_util.CandidateConstructionFailure):
                    ast_util.parse_c(source)

    def test_ordinary_identifiers_strings_and_normal_operators_unchanged(self):
        source = ('int __builtin_classof_value; char *s = "__builtin_alignof(int)"; '
                  'int f(int x) { return sizeof(int) + -x + __builtin_classof_value; }')
        expected = ast_util.to_c_raw(ast_util.parse_c(source))
        with pb.ido_import_parser():
            ast = ast_util.parse_c(source)
            pb.reject_retained_ido_operators(ast)
            self.assertEqual(ast_util.to_c_raw(ast), expected)

    def test_normal_extraction_removes_helper_body_not_declarations(self):
        source = ('typedef int T; int helper(T x) { return __builtin_classof(T); }\n'
                  'int f(T x) { return helper(x); }')
        with pb.ido_import_parser():
            ast = ast_util.parse_c(source)
            fn, _ = ast_util.extract_fn(ast, "f")
            ast_util.prune_ast(fn, ast)
            pb.reject_retained_ido_operators(ast)
            emitted = ast_util.to_c_raw(ast)
        self.assertIn("int helper(T x);", emitted)
        self.assertIn("typedef int T;", emitted)
        self.assertIn("return helper(x);", emitted)
        self.assertNotIn("__builtin_classof", emitted)
        ast_util.parse_c(emitted)

    def test_retained_target_initializer_and_inline_helper_refuse(self):
        sources = [
            'int f(void) { return __builtin_alignof(int); }',
            'int n = __builtin_alignof(int); int f(void) { return n; }',
            'static inline int g(void) { return __builtin_classof(int); } int f(void) { return g(); }',
        ]
        for source in sources:
            with self.subTest(source=source), pb.ido_import_parser():
                ast = ast_util.parse_c(source)
                fn, _ = ast_util.extract_fn(ast, "f")
                ast_util.prune_ast(fn, ast)
                with self.assertRaisesRegex(ValueError, "retained target/context"):
                    pb.reject_retained_ido_operators(ast)

    def test_coordinates_and_group_selection_survive_typed_helper(self):
        source = ('#line 10 "actual.c"\n'
                  'int helper(void) { return __builtin_classof(int); }\n'
                  'void f(int *p) {\n p[0]=1; p[1]=2;\n}\n')
        with pb.ido_import_parser():
            ast = ast_util.parse_c(source)
            helper = ast.ext[0]
            self.assertEqual((helper.coord.file, helper.coord.line), ("actual.c", 10))
            prepared, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
            self.assertEqual(plan["status"], "preserved")
            self.assertEqual(plan["groups"][0]["line"], 12)
            self.assertEqual(ast_util.to_c_raw(helper), ast_util.to_c_raw(prepared.ext[0]))

    def test_adapter_and_import_hooks_restore_on_success_and_exception(self):
        originals = (ast_util.CParser, ast_util.parse_c, ast_util.to_c, ast_util.to_c_raw, sys.argv)
        from perm_pycparser.c_lexer import CLexer
        keywords = dict(CLexer.keyword_map)
        for fail in (False, True):
            with tempfile.TemporaryDirectory() as tmp:
                def importer(*args, **kwargs):
                    ast = ast_util.parse_c('int helper(void) { return __builtin_classof(int); } '
                                          'int f(void) { return 1; }', from_import=True)
                    if fail:
                        raise RuntimeError("fixture failure")
                    fn, _ = ast_util.extract_fn(ast, "f")
                    ast_util.prune_ast(fn, ast)
                    self.assertNotIn("__builtin_classof", ast_util.to_c(ast))
                with patch('runpy.run_path', side_effect=importer):
                    if fail:
                        with self.assertRaisesRegex(RuntimeError, "fixture failure"):
                            pb.grouped_import_main('f', str(Path(tmp)/'plan.json'), [])
                    else:
                        pb.grouped_import_main('f', str(Path(tmp)/'plan.json'), [])
            self.assertEqual((ast_util.CParser, ast_util.parse_c, ast_util.to_c,
                              ast_util.to_c_raw, sys.argv), originals)
            self.assertEqual(CLexer.keyword_map, keywords)
        with self.assertRaises(ast_util.CandidateConstructionFailure):
            ast_util.parse_c('int f(void) { return __builtin_classof(int); }')

    def test_import_emission_refusal_restores_hooks(self):
        original = (ast_util.CParser, ast_util.parse_c, ast_util.to_c, ast_util.to_c_raw, sys.argv)
        for emission in ("to_c", "to_c_raw"):
            with tempfile.TemporaryDirectory() as tmp:
                def importer(*args, **kwargs):
                    ast = ast_util.parse_c('int f(void) { return __builtin_alignof(int); }',
                                           from_import=True)
                    getattr(ast_util, emission)(ast)
                with patch('runpy.run_path', side_effect=importer):
                    with self.assertRaisesRegex(ValueError, "retained target/context"):
                        pb.grouped_import_main('f', str(Path(tmp)/'plan.json'), [])
            self.assertEqual((ast_util.CParser, ast_util.parse_c, ast_util.to_c,
                              ast_util.to_c_raw, sys.argv), original)


class SourceGroups(unittest.TestCase):
    def test_standalone_inline_do_preserves_whole_line_and_live_ast_twice(self):
        from src.candidate import Candidate
        from src.perm.perm import EvalState
        import tomllib
        weights = tomllib.loads((pb.PERMUTER_DIR / "default_weights.toml").read_text())["base"]
        source = ("void f(int *p, int n) {\n int x;\n p[1]=701;\n"
                  " do { x = 0; p[0] = 703; x++; } while (0);\n p[2]=709;\n}\n")
        for text in (source, source.replace("\n", "\r\n")):
            with self.subTest(newlines=repr(text[:40])):
                prepared, plan = pb.group_seed_source(text.encode(), "f")
                self.assertEqual(plan["status"], "preserved")
                self.assertEqual(plan["groups"], [{"line": 4, "statements": 1, "control": "DoWhile"}])
                candidate = Candidate.from_source(prepared.decode(), EvalState(), "f", weights, 1)
                emitted = candidate.get_source()
                second, plan2 = pb.group_seed_source(emitted.encode(), "f")
                self.assertEqual(plan2["status"], "preserved")
                emitted2 = Candidate.from_source(second.decode(), EvalState(), "f", weights, 2).get_source()
                for result in (emitted, emitted2):
                    line = next(line for line in result.splitlines() if "703" in line)
                    self.assertIn("do {", line)
                    self.assertIn("} while (0);", line)
                    self.assertNotIn("701", line)
                    self.assertNotIn("709", line)
                    self.assertEqual(ast_util.to_c_raw(ast_util.parse_c(result)),
                                     ast_util.to_c_raw(ast_util.parse_c(source)))
                pending, constants = [candidate.ast], []
                while pending:
                    node = pending.pop()
                    self.assertFalse(isinstance(node, c_ast.Pragma) and "b64literal" in node.string)
                    if isinstance(node, c_ast.Constant) and node.value == "703":
                        constants.append(node)
                    pending.extend(child for _, child in node.children())
                self.assertEqual(len(constants), 1)
                constants[0].value = "1703"
                candidate._cache_source = None
                self.assertIn("1703", candidate.get_source())

    def test_standalone_inline_do_nested_statements_scopes_and_literal_braces(self):
        for body in (
            "int x = 0; { int x = 1; p[0] = x; } p[1] = x;",
            "if (n) { p[0] = 1; } else { p[0] = 2; }",
            "do { p[0]++; } while (n); p[1]++;",
            "for (i = 0; i < n; i++) { p[i]++; }",
            "if (n) { break; } p[0]++; continue;",
            'puts("}; while (0); //"); p[0]++;',
            "",
        ):
            source = f"void f(int *p, int n) {{\n int i;\n do {{ {body} }} while (0);\n p[2]++;\n}}\n"
            with self.subTest(body=body):
                _, plan, emitted = self.emit_seed(source.encode())
                self.assertEqual(plan["status"], "preserved")
                self.assertEqual(ast_util.to_c_raw(ast_util.parse_c(emitted.decode())),
                                 ast_util.to_c_raw(ast_util.parse_c(source)))
                _, plan2, emitted2 = self.emit_seed(emitted)
                self.assertEqual(plan2["status"], "preserved")
                self.assertEqual(ast_util.to_c_raw(ast_util.parse_c(emitted2.decode())),
                                 ast_util.to_c_raw(ast_util.parse_c(source)))

    def test_standalone_inline_do_comment_punctuation_does_not_select_braces(self):
        source = 'void f(int *p) {\n do { p[0]++; /* } while(0); */ p[1]++; } while (0);\n}\n'
        parsed_source = source.replace('/* } while(0); */', ' ' * len('/* } while(0); */'))
        ast = ast_util.parse_c(parsed_source)
        grouped, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertEqual(plan["status"], "preserved")
        emitted = ast_util.to_c(grouped)
        self.assertIn('do { p[0]++; p[1]++; } while (0);', emitted)

    def test_standalone_inline_do_unsupported_endpoints_keep_original_ast(self):
        for statement in (
            "do { p[0]++; } while (\n 0);",
            "do { p[0]++;\n } while (0);",
            "do { p[0]++; } while (0); p[1]++;",
            "do { label: p[0]++; } while (0);",
            "do { switch(n) { case 1: p[0]++; break; } } while (0);",
            "do {\n#pragma _permuter sameline start\n p[0]++; } while (0);",
        ):
            source = f"void f(int *p, int n) {{\n {statement}\n}}\n"
            ast = ast_util.parse_c(source)
            before = ast_util.to_c_raw(ast)
            result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
            with self.subTest(statement=statement):
                self.assertEqual(plan["status"], "measurement-required")
                self.assertIs(result, ast)
                self.assertEqual(ast_util.to_c_raw(result), before)

    def test_standalone_inline_do_selected_function_and_exact_token_witness(self):
        source = ("void g(int *p) { do { p[0] = 2; } while (0); }\n"
                  "void f(int *p) {\n do { p[0] = 1; } while (0);\n}\n")
        ast = ast_util.parse_c(source)
        before_g = ast_util.to_c_raw(ast.ext[0])
        grouped, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertEqual(plan["status"], "preserved")
        self.assertEqual(ast_util.to_c_raw(grouped.ext[0]), before_g)
        for incompatible in (
            source.replace("p[0] = 1", "p[0] = 3"),
            source.replace("while (0);\n}", "while (0); } void h(void) {\n}"),
        ):
            result, plan = pb.prepare_source_groups(ast, incompatible, "f", c_ast)
            self.assertEqual(plan["status"], "measurement-required")
            self.assertIs(result, ast)

    WIDE = ("void f(int *p, int n) {\n int i=0;\n do\n {\n"
            " p[i]=701; i++; } while(i<n); i=702; do { p[i]=703; i++; } while(i<n); "
            "i=704; do { p[i]=705; i++; } while(i<n); i=706; do {\n"
            " p[i]=707;\n i++;\n } while(i<n);\n p[0]=708;\n}\n")

    def test_wide_do_span_exact_line_and_live_ast_over_two_emissions(self):
        from src.candidate import Candidate
        from src.perm.perm import EvalState
        import tomllib
        weights = tomllib.loads((pb.PERMUTER_DIR / "default_weights.toml").read_text())["base"]
        for source in (self.WIDE, self.WIDE.replace("\n", "\r\n")):
            prepared, plan = pb.group_seed_source(source.encode(), "f")
            self.assertEqual(plan["status"], "preserved")
            self.assertTrue(any(g["control"] == "LexicallyBoundDoSpan" for g in plan["groups"]))
            candidate = Candidate.from_source(prepared.decode(), EvalState(), "f", weights, 1)
            emitted = candidate.get_source()
            second, plan2 = pb.group_seed_source(emitted.encode(), "f")
            self.assertEqual(plan2["status"], "preserved")
            emitted2 = ast_util.to_c(Candidate._cached_shared_ast(second.decode(), "f")[2])
            for text in (emitted, emitted2):
                line = next(line for line in text.splitlines() if "701" in line)
                self.assertTrue(all(str(value) in line for value in range(701,707)))
                self.assertNotIn("707", line)
                self.assertNotIn("708", line)
                parsed = ast_util.parse_c(text)
                function, _ = ast_util.extract_fn(parsed, "f")
                baseline = ast_util.parse_c(source.replace("\r\n", "\n"))
                original, _ = ast_util.extract_fn(baseline, "f")
                self.assertEqual(ast_util.to_c_raw(function), ast_util.to_c_raw(original))
            pending, found = [candidate.ast], []
            while pending:
                node = pending.pop()
                self.assertFalse(isinstance(node,c_ast.Pragma) and "b64literal" in node.string)
                if isinstance(node,c_ast.Constant) and node.value == "703":
                    found.append(node)
                pending.extend(child for _,child in node.children())
            self.assertEqual(len(found),1)
            found[0].value = "1703"
            candidate._cache_source = None
            self.assertIn("1703",candidate.get_source())

    def test_wide_span_nested_scopes_literals_and_adjacent_bridges(self):
        for source in (
            self.WIDE.replace("do { p[i]=703;", "do { int x=i; p[x]=703;"),
            self.WIDE.replace("i=704;", 'i=704+"};do{;"[0];'),
            "#define UNUSED 1\n" + self.WIDE.replace("i=704;", "i=704; /* }; do { */"),
            self.WIDE.replace("do { p[i]=703; i++; }", "do { }"),
            self.WIDE.replace("p[i]=703; i++;", "p[i]=703; break;"),
            self.WIDE.replace(" p[i]=707;", " p[i]=707; i++; } while(i<n); i=709; do {\n p[i]=710;"),
        ):
            with self.subTest(source=source):
                _, plan, emitted = self.emit_seed(source.encode())
                self.assertEqual(plan["status"],"preserved")
                self.assertEqual(pb.group_seed_source(emitted,"f")[1]["status"],"preserved")

    def test_wide_span_missing_token_or_slot_witness_keeps_original_ast(self):
        for source in (
            self.WIDE.replace("i=704;", "i=(704);"),
            self.WIDE.replace("i=704;", "i=704;\n#line 99\n"),
            self.WIDE.replace("p[i]=703;", "label: p[i]=703;"),
            self.WIDE.replace("p[i]=703;", "if(n) { p[i]=703; }"),
            self.WIDE.replace(" p[i]=707;", "#pragma _permuter sameline start\n p[i]=707;"),
            self.WIDE.replace("i=704;", "i=704;\n i=\n704;"),
        ):
            ast = ast_util.parse_c(source)
            before = ast_util.to_c_raw(ast)
            result, plan = pb.prepare_source_groups(ast,source,"f",c_ast)
            with self.subTest(source=source):
                self.assertEqual(plan["status"],"measurement-required")
                self.assertEqual(ast_util.to_c_raw(result),before)

    def test_do_tail_composite_preserves_exact_line_and_ast_twice(self):
        source = (b"void f(void) {\n int x=0;\n do {\n x++;\n"
                  b" } while(x<2); x=0; do { x++; x+=2; } while(x<4);\n x++;\n}\n")
        import candidate_context as context
        prepared, plan, emitted = self.emit_seed(source)
        self.assertEqual(plan["status"], "preserved")
        self.assertEqual(plan["groups"], [{"line": 5, "statements": 3,
                                          "control": "DoWhileTailComposite"}])
        expected = b" } while (x < 2); x = 0; do { x++; x += 2; } while (x < 4);"
        self.assertIn(expected, emitted.splitlines())
        # Strip only generated markers; every original semantic AST node stays.
        from perm_pycparser import c_generator
        baseline_ast = ast_util.parse_c(source.decode())
        emitted_ast = ast_util.parse_c(emitted.decode())
        generator = c_generator.CGenerator()
        self.assertEqual(generator.visit(baseline_ast), generator.visit(emitted_ast))
        self.assertEqual(context.compare_context(source, emitted, "f")["status"], "unchanged")
        _, second_plan, second = self.emit_seed(emitted)
        self.assertEqual(second_plan["status"], "preserved")
        self.assertIn(expected, second.splitlines())
        self.assertIn(b"_permuter sameline start", prepared)

    def test_do_tail_composite_nested_and_adjacent_groups(self):
        source = (b"void f(void) {\n int x=0;\n {\n do {\n x++; x+=2;\n"
                  b" } while(x<2); x=0; do { x++; } while(x<4);\n"
                  b" do {\n x++;\n } while(x<2); x=1;\n }\n}\n")
        _, plan, emitted = self.emit_seed(source)
        self.assertEqual(plan["status"], "preserved")
        self.assertEqual(sum(g["control"] == "DoWhileTailComposite" for g in plan["groups"]), 2)
        self.assertIn(b"x++; x += 2;", emitted)
        self.assertIn(b"} while (x < 2); x = 1;", emitted)

    def test_do_tail_composite_ignores_literal_punctuation(self):
        source = (b"void f(void) {\n int x=0;\n do {\n x++;\n"
                  b' } while(x<2); puts("};while(;)"); do { x++; } while(x<4);\n}\n')
        _, plan, emitted = self.emit_seed(source)
        self.assertEqual(plan["status"], "preserved")
        self.assertIn(b'puts("};while(;)"); do {', emitted)

    def test_do_tail_composite_partial_or_ambiguous_shapes_keep_original_ast(self):
        template = "void f(void) {\n int x=0;\n do {\n x++;\n%s\n}\n"
        for group in (
            " } while(x<2\n ); x=0; do { x++; } while(x<4);",
            " } while(x<2); x=0; do { x++; } while(x<4\n );",
            " } while(x<2); x=0; do { do { x++; } while(x<3); } while(x<4);",
            " } while(x<2); label: x=0;",
            " } while(x<2); if(x) { x++; } else { x--; }",
        ):
            source = template % group
            ast = ast_util.parse_c(source)
            before = ast_util.to_c_raw(ast)
            result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
            with self.subTest(group=group):
                self.assertEqual(plan["status"], "measurement-required")
                self.assertEqual(ast_util.to_c_raw(result), before)

    def emit_seed(self, source):
        prepared, plan = pb.group_seed_source(source, "f")
        ast = ast_util.parse_c(prepared.decode())
        function, _ = ast_util.extract_fn(ast, "f")
        ast_util.normalize_ast(function, ast)
        return prepared, plan, ast_util.to_c(ast).encode()

    def test_inactive_seed_macros_survive_vendor_parse_and_two_emissions(self):
        import candidate_context as context
        source = (b"#define JOIN(a,b) a ## b\n#define UNUSED 7\n"
                  b"void f(void) {\n int x;\n x=0; do {\n x++;\n } while(x<2);\n}\n")
        prepared, plan, emitted = self.emit_seed(source)
        self.assertNotIn(b"#define", prepared)
        self.assertIn(b"#pragma _permuter latedefine start", prepared)
        self.assertEqual(plan["status"], "preserved")
        self.assertEqual(plan["inactive_macro_prelude"]["source_sha256"], hashlib.sha256(source).hexdigest())
        self.assertEqual(len(plan["inactive_macro_prelude"]["definitions"]), 2)
        self.assertIn(b"x = 0; do {", emitted)
        self.assertIn(b"#define JOIN(a,b) a ## b", emitted)
        self.assertEqual(context.compare_context(source, emitted, "f")["status"], "unchanged")
        _, _, second = self.emit_seed(emitted)
        self.assertEqual(context.compare_context(emitted, second, "f")["status"], "unchanged")

    def test_seed_macro_prelude_keeps_physical_group_coordinates(self):
        import candidate_context as context
        source = b"#define UNUSED(a) \\\n ((a)+1)\nvoid f(void) {\n int x;\n x=0; x++;\n}\n"
        text, definitions, _ = context.inactive_seed_prelude(source)
        self.assertEqual(text.count("\n"), source.count(b"\n"))
        self.assertEqual(text.index("void"), source.index(b"void"))
        self.assertEqual(definitions, ["define UNUSED(a)  ((a)+1)"])
        _, plan, emitted = self.emit_seed(source)
        self.assertEqual(plan["groups"][0]["line"], 5)
        self.assertIn(b"x = 0; x++;", emitted)
        self.assertEqual(context.compare_context(source, emitted, "f")["status"], "unchanged")

    def test_inactive_seed_macro_comments_literals_and_line_endings(self):
        import candidate_context as context
        for newline in (b"\n", b"\r\n"):
            source = newline.join((b'#define UNUSED /* note */ "x"',
                b'void f(void) {', b' const char *s = "UNUSED // not a use";',
                b' int x; /* UNUSED */ x=0; x++;', b'}', b''))
            _, _, emitted = self.emit_seed(source)
            self.assertEqual(context.compare_context(source, emitted, "f")["status"], "unchanged")

    def test_seed_macro_vt_ff_do_not_create_directive_boundaries(self):
        import candidate_context as context
        for space in (b"\v", b"\f"):
            source = b"#define UNUSED 1" + space + b" + 2\nvoid f(void) { return; }\n"
            _, _, emitted = self.emit_seed(source)
            self.assertEqual(context.compare_context(source, emitted, "f")["status"], "unchanged")
            with self.assertRaises(Exception):
                self.emit_seed(b"#define UNUSED 1" + space + b"int secret;\nvoid f(void) { UNUSED; }\n")

    def test_seed_macro_unsafe_preprocessing_refuses_before_emission(self):
        samples = [
            b"#define M 1\nvoid f(void) { M; }\n",
            b"int x;\n#define M 1\nvoid f(void) {}\n",
            b"#define M 1\n#define M 1\nvoid f(void) {}\n",
            b"#if 0\n#define M 1\n#endif\nvoid f(void) {}\n",
            b"#define M 1\n#undef M\nvoid f(void) {}\n",
            b"#define M 1\n#include \"x.h\"\nvoid f(void) {}\n",
            b"void f(void) {\n#define M 1\n}\n",
            b"#define M 1\n#pragma M\nvoid f(void) {}\n",
            b"#define M 1 /* cross\nline */\nvoid f(void) {}\n",
            b"#define M 1\nvoid f(void) { int x = __LINE__; }\n",
            b"#define M 1\nvoid f(void) { \\\n return; }\n",
            b"#define M 1\\ \nvoid f(void) {}\n",
            b"#define M 1\n#line 50\nvoid f(void) {}\n",
            b"#define M 1\n# 50 \"x.c\"\nvoid f(void) {}\n",
            b"#define M 1\n#pragma _permuter latedefine start\nvoid f(void) {}\n",
            b"#define M 1\n#pragma _permuter b64literal I2RlZmluZSBYIDE=\nvoid f(void) {}\n",
        ]
        for source in samples:
            with self.subTest(source=source), self.assertRaises(Exception):
                self.emit_seed(source)

    def test_macro_seed_plan_and_definitions_remain_hash_bound(self):
        source = b"#define UNUSED 1\nvoid f(void) { return; }\n"
        prepared, plan = pb.group_seed_source(source, "f")
        raw = json.dumps(plan).encode()
        baseline = {"seed/prepared.c": prepared, "seed/plan.json": raw}
        identity = {"preparation_contract": pb.SEED_FIDELITY_CONTRACT,
            "source_sha256": hashlib.sha256(source).hexdigest(),
            "prepared_source_sha256": hashlib.sha256(prepared).hexdigest(),
            "plan_sha256": hashlib.sha256(raw).hexdigest()}
        item = SimpleNamespace(func="f")
        inputs = {"search": {"seed": identity}}
        pb.validate_seed_layout(item, baseline, inputs, source)
        plan["inactive_macro_prelude"]["definitions"][0]["sha256"] = "0" * 64
        changed = dict(baseline, **{"seed/plan.json": json.dumps(plan).encode()})
        with self.assertRaisesRegex(RuntimeError, "layout"):
            pb.validate_seed_layout(item, changed, inputs, source)
        changed = dict(baseline, **{"seed/prepared.c": prepared.replace(b"UNUSED 1", b"UNUSED 2")})
        with self.assertRaisesRegex(RuntimeError, "layout"):
            pb.validate_seed_layout(item, changed, inputs, source)

    def test_saved_seed_reconstructs_consumed_markers_for_both_emissions(self):
        source = b"void f(void) {\n int x;\n x=0; do {\n x++;\n } while(x<2);\n}\n"
        prepared, plan = pb.group_seed_source(source, "f")
        self.assertEqual(plan["status"], "preserved")
        self.assertIn(b"#pragma _permuter sameline start", prepared)
        def emit(raw):
            # Candidate._cached_shared_ast/get_source's exact AST sequence,
            # without importing optional search-only toml/randomizer modules.
            ast = ast_util.parse_c(raw)
            function, _ = ast_util.extract_fn(ast, "f")
            ast_util.normalize_ast(function, ast)
            return ast_util.to_c(ast)
        emitted = emit(prepared.decode())
        self.assertIn("x = 0; do {", emitted)
        self.assertNotIn("_permuter", emitted)
        second, _ = pb.group_seed_source(emitted.encode(), "f")
        self.assertIn("x = 0; do {", emit(second.decode()))
        self.assertNotIn("x = 0; do {", emit(emitted))

    def test_seed_layout_hashes_and_unknown_plan_fail_closed(self):
        source = b"void f(void) {\n return;\n}\n"
        prepared, plan = pb.group_seed_source(source, "f")
        raw_plan = json.dumps(plan).encode()
        baseline = {"seed/prepared.c": prepared, "seed/plan.json": raw_plan}
        identity = {"preparation_contract": pb.SEED_FIDELITY_CONTRACT,
            "source_sha256": hashlib.sha256(source).hexdigest(),
            "prepared_source_sha256": hashlib.sha256(prepared).hexdigest(),
            "plan_sha256": hashlib.sha256(raw_plan).hexdigest()}
        item = SimpleNamespace(func="f")
        inputs = {"search": {"seed": identity}}
        pb.validate_seed_layout(item, baseline, inputs, source)
        for name in baseline:
            changed = dict(baseline, **{name: baseline[name] + b" "})
            with self.subTest(name=name), self.assertRaisesRegex(RuntimeError, "layout"):
                pb.validate_seed_layout(item, changed, inputs, source)
        plan["status"] = "unknown"
        baseline["seed/plan.json"] = json.dumps(plan).encode()
        identity["plan_sha256"] = hashlib.sha256(baseline["seed/plan.json"]).hexdigest()
        with self.assertRaisesRegex(RuntimeError, "layout"):
            pb.validate_seed_layout(item, baseline, inputs, source)

    def test_actual_elf_seed_fidelity_checks_instructions_and_identity_not_score(self):
        assembler = pb.ROOT / "tools/binutils/mips64-elf-as"
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            objects = []
            for index, (symbol, register) in enumerate((("one", "$2"), ("one", "$2"),
                                                       ("two", "$2"), ("one", "$3"))):
                source, obj = root / f"{index}.s", root / f"{index}.o"
                source.write_text(f".set noreorder\n.text\n.globl f\n.ent f\nf:\n"
                    f"lui {register},%hi({symbol})\naddiu {register},{register},%lo({symbol})\n"
                    "jr $31\nnop\n.end f\n")
                subprocess.run([str(assembler), "-32", "-o", str(obj), str(source)], check=True)
                objects.append(obj)
            item = SimpleNamespace(func="f", rel_c_file="src/fixture.c", overlay=None)
            report = pb.seed_object_fidelity(item, objects[0], objects[1])
            self.assertTrue(report["source_fidelity_exact"])
            self.assertEqual(report["relocation_count"], 2)
            with patch.object(pb.reloc_surface, "function_surface_comparison",
                              return_value={"stable_identity_exact": False}):
                with self.assertRaisesRegex(RuntimeError, "identity"):
                    pb.seed_object_fidelity(item, objects[0], objects[2])
            with self.assertRaisesRegex(RuntimeError, "instruction"):
                pb.seed_object_fidelity(item, objects[0], objects[3])
            # Exercise the real production capture-review seam with actual ELF
            # objects. Both authenticated captures have identical C and score;
            # only their owned object fields/identities differ.
            source = b"void f(void) { return; }\n"
            seed = SimpleNamespace(source=source, object=objects[0].read_bytes(),
                object_sha256=hashlib.sha256(objects[0].read_bytes()).hexdigest())
            authorities = [root / name for name in ("build/mickey.us.elf", "rom", "atlas", "links")]
            for path in authorities:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"synthetic authority")
            for index in (1, 2, 3):
                out = root / f"run-{index}"
                out.mkdir()
                (out / "base.c").write_bytes(source)
                (out / "target.o").write_bytes(b"target")
                actual = SimpleNamespace(source=source, object=objects[index].read_bytes(),
                    object_sha256=hashlib.sha256(objects[index].read_bytes()).hexdigest())
                result = pb.RunResult(func="f", c_file="src/fixture.c", overlay=None, ok=False)
                result.seed_score = 10
                result.seed_proof = {"status": "pending-search"}
                inputs = {"search": {"seed": {"target_object_sha256": hashlib.sha256(b"target").hexdigest()}}}
                artifacts = {"seed/prepared.c": source}
                with contextlib.ExitStack() as stack:
                    for name, value in (("ROOT", root), ("BASEROM", authorities[1]), ("ATLAS_PATH", authorities[2])):
                        stack.enter_context(patch.object(pb, name, value))
                    stack.enter_context(patch.object(pb.reloc_surface, "LINK_SYMS", authorities[3]))
                    stack.enter_context(patch.object(pb.reloc_surface, "function_surface_comparison", return_value={"stable_identity_exact": False}))
                    for name in ("wait_for_headroom", "validate_baseline", "checked_tool_identity", "retain_context",
                                 "require_search_bindings", "validate_search_binding_authority"):
                        stack.enter_context(patch.object(pb, name))
                    stack.enter_context(patch.object(pb, "run_permuter", return_value=(10, 0, False, False)))
                    stack.enter_context(patch.object(pb, "captured_baseline", return_value=actual))
                    stack.enter_context(patch.object(pb, "_best", return_value=(None, None)))
                    stack.enter_context(patch.object(pb, "review_context", return_value={"status": "unchanged"}))
                    promotion = stack.enter_context(patch.object(pb, "promote"))
                    pb.run_prepared(item, out, out, result, 1, 1, 1, False, [], 0, 0, False, 0, None,
                        prepared_inputs=inputs, canonical_evidence=seed, seed_evidence=seed, seed_artifacts=artifacts)
                promotion.assert_not_called()
                with self.subTest(search_object=index):
                    self.assertEqual(result.ok, index == 1, result.error)
                    self.assertEqual(result.seed_proof["status"] == "validated", index == 1)
                    self.assertEqual("seed/search-fidelity.json" in artifacts, index == 1)

    def plan_inputs(self, directory):
        return {"context": {"source_group_plan": hashlib.sha256(
            (directory / "source-groups.json").read_bytes()).hexdigest()}}
    def convert(self, source, symbol="f"):
        ast = ast_util.parse_c(source, from_import=True)
        plan = pb.preserve_source_groups(ast, source, symbol, c_ast)
        raw = ast_util.to_c_raw(ast)
        # Same two passes as prune_source and Candidate.get_source.
        return plan, raw, ast_util.to_c(ast_util.parse_c(raw))

    def test_simple(self):
        plan, raw, result = self.convert("void f(void) {\n int x;\n x=1; x++;\n}\n")
        self.assertEqual(len(plan["groups"]), 1)
        self.assertIn("#pragma _permuter sameline start", raw)
        self.assertIn("x = 1; x++;", result)
        self.assertNotIn("_permuter", result)

    def test_do_cast(self):
        plan, _, result = self.convert("void f(void) {\n int x;\n x=(int)0; do {\n x++;\n } while(x<2);\n}\n")
        self.assertEqual(plan["groups"][0]["control"], "DoWhile")
        self.assertIn("x = (int) 0; do {\n", result)
        self.assertNotIn("do { x++", result)

    def test_if(self):
        _, _, result = self.convert("void f(void) {\n int x;\n x=0; if(x) {\n x++;\n }\n}\n")
        self.assertIn("x = 0; if (x) {\n", result)

    def test_nested(self):
        plan, _, result = self.convert("void f(void) {\n int x;\n if(x) {\n x=0; do {\n x++; x--;\n } while(x);\n }\n}\n")
        self.assertEqual(len(plan["groups"]), 2)
        self.assertIn("x = 0; do {\n", result)
        self.assertIn("x++; x--;", result)

    def test_whole_line_standalone_compound_preserves_declarations_and_scope(self):
        source = "void f(int *p) {\n { int *q = p++; *q = 1; *q += 2; };\n p++;\n}\n"
        plan, raw, result = self.convert(source)
        self.assertEqual(plan["groups"], [{"line": 2, "statements": 2, "control": "Compound"}])
        self.assertIn("{ int *q = p++; *q = 1; *q += 2; } ;\n", result)
        self.assertIn("\n  p++;", result)
        # Markers only: deleting them restores the original declaration scope,
        # statement ordering and every expression in the parser's AST.
        ast = ast_util.parse_c(raw)
        ast.ext[0].body.block_items = [n for n in ast.ext[0].body.block_items
                                     if not isinstance(n, c_ast.Pragma)]
        self.assertEqual(ast_util.to_c_raw(ast),
                         ast_util.to_c_raw(ast_util.parse_c(source, from_import=True)))

    def test_whole_line_compound_inside_multiline_control(self):
        source = "void f(int *p) {\n if(p) {\n { int x = (int)*p; *p = x; }\n p++;\n }\n}\n"
        _, _, result = self.convert(source)
        self.assertIn("{ int x = (int) (*p); *p = x; }\n", result)
        self.assertNotIn("if (p) { {", result)

    def test_partial_or_ambiguous_compound_retains_original_for_measurement(self):
        for body in ("{ int x;\n x=1;\n }", "{ int x; } { int y; }",
                     "{ if(p) *p=1; }", "if(p) { *p=1; }"):
            source = "void f(int *p) {\n " + body + "\n}\n"
            original = ast_util.parse_c(source, from_import=True)
            before = ast_util.to_c_raw(original)
            result, plan = pb.prepare_source_groups(original, source, "f", c_ast)
            with self.subTest(body=body):
                self.assertEqual(plan["status"], "measurement-required")
                self.assertIs(result, original)
                self.assertEqual(ast_util.to_c_raw(result), before)

    def test_compound_literal_braces_do_not_supply_a_fake_boundary(self):
        source = 'void f(void) {\n { char *p = "}"; foo(p); }\n}\n'
        _, _, result = self.convert(source)
        self.assertIn('{ char *p = "}"; foo(p); }\n', result)

    def test_only_selected_function(self):
        plan, raw, _ = self.convert("void g(void) { int y; y=1; y++; }\nvoid f(void) {\n int x;\n x=1;\n x++;\n}\n")
        self.assertEqual(plan["groups"], [])
        self.assertNotIn("sameline", raw)

    def test_line_directives(self):
        plan, _, result = self.convert('#line 90 "original.c"\nvoid f(void) {\n int x;\n x=1; x++;\n}\n')
        self.assertEqual(plan["groups"][0]["line"], 92)
        self.assertIn("x = 1; x++;", result)

    def test_duplicate_line_identity(self):
        with self.assertRaisesRegex(ValueError, "ambiguous"):
            self.convert('#line 4 "a.c"\nvoid f(void) {\n int x;\n#line 5 "a.c"\n x=1; x++;\n}\n')

    def test_unsupported_groups(self):
        for body in ("x=0; do x++; while(x);", "x=0; if(x) x++;",
                     "x=0; while(x) {\n x++;\n }", "x=0; do { x++; } while(x);"):
            with self.subTest(body=body), self.assertRaisesRegex(ValueError, "unsupported"):
                self.convert("void f(void) {\n int x;\n" + body + "\n}\n")

    def test_multiline_statement_not_silently_joined(self):
        with self.assertRaises(ValueError):
            self.convert("void f(void) {\n int x;\n x=\n 1; x++;\n}\n")

    def test_preexisting_pragma(self):
        with self.assertRaisesRegex(ValueError, "preexisting"):
            self.convert("void f(void) {\n#pragma _permuter sameline start\n return;\n}\n")

    def test_missing_function(self):
        with self.assertRaisesRegex(ValueError, "one selected"):
            self.convert("void g(void) {}")

    def test_non_sibling_group_detection(self):
        for source in ("void f(void) { return; }", "void f(int x) {\n if(x) x++;\n}",
                       "void f(int x) {\n if(x) { x++; }\n}",
                       "void f(int x) {\n switch(x) {\n case 1: x++;\n }\n}"):
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, "unsupported"):
                self.convert(source)

    def test_unsupported_original_ast_is_measured_not_rewritten(self):
        source = "void f(void) { return; }"
        ast = ast_util.parse_c(source, from_import=True)
        result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertIs(result, ast)
        self.assertEqual(plan["status"], "measurement-required")

    def test_nested_trial_mutation_never_leaks_on_outer_refusal(self):
        source = "void f(int x) {\n if(x) {\n x=1; x++;\n }\n x=0; while(x) {\n x++;\n }\n}\n"
        ast = ast_util.parse_c(source, from_import=True)
        before = ast_util.to_c_raw(ast)
        result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertEqual(plan["status"], "measurement-required")
        self.assertIs(result, ast)
        self.assertEqual(ast_util.to_c_raw(result), before)

    def test_closing_control_boundary_requires_measurement(self):
        source = "void f(int x) {\n if(x) {\n x=1; x++;\n } x++;\n}\n"
        ast = ast_util.parse_c(source, from_import=True)
        result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertIs(result, ast)
        self.assertEqual(plan["status"], "measurement-required")

    def test_multiline_lexical_endpoints_require_measurement(self):
        for body in ("foo(\n ); x++;", "x=(1\n ); x++;", "foo(\n ); /* ; */ x++;"):
            source = "void f(void) {\n int x;\n" + body + "\n}\n"
            ast = ast_util.parse_c(source.replace("/* ; */", "       "), from_import=True)
            result, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
            self.assertIs(result, ast)
            self.assertEqual(plan["status"], "measurement-required")

    def test_literal_semicolon_is_not_a_statement_boundary(self):
        source = 'void f(void) {\n foo(";");\n return;\n}\n'
        ast = ast_util.parse_c(source, from_import=True)
        _, plan = pb.prepare_source_groups(ast, source, "f", c_ast)
        self.assertEqual(plan["status"], "ungrouped")

    def test_plan_status_and_digest_fail_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            item = pb.QueueItem("f", root / "src/f.c")
            plan = {"contract": pb.SOURCE_GROUP_CONTRACT, "symbol": "f", "groups": []}
            for status in (None, "unknown", "preserved", "measurement-required"):
                plan["status"] = status
                (root / "source-groups.json").write_text(json.dumps(plan))
                with self.subTest(status=status), self.assertRaisesRegex(RuntimeError, "status"):
                    pb.grouped_baseline_fidelity(item, root, root, self.plan_inputs(root), None)
            plan["status"] = "ungrouped"
            (root / "source-groups.json").write_text(json.dumps(plan))
            old = self.plan_inputs(root)
            (root / "source-groups.json").write_text(json.dumps({**plan, "reason": "replaced"}))
            with self.assertRaisesRegex(RuntimeError, "changed"):
                pb.grouped_baseline_fidelity(item, root, root, old, None)

    def test_importer_recipe_normalizes_only_generated_lane_plumbing(self):
        first = b'#!/bin/sh\ncd /lane-a\ntools/ido/cc -O2 -DNAME="/lane-a" "$INPUT" -o "$OUTPUT"\n'
        second = first.replace(b"cd /lane-a", b"cd /lane-b")
        with patch.object(pb, "ROOT", Path("/lane-a")):
            expected = pb.compile_script_digest(first)
        with patch.object(pb, "ROOT", Path("/lane-b")):
            self.assertEqual(expected, pb.compile_script_digest(second))
            self.assertNotEqual(expected, pb.compile_script_digest(second.replace(b"-O2", b"-O1")))
            self.assertNotEqual(expected, pb.compile_script_digest(second.replace(b'DNAME="/lane-a"', b'DNAME="/lane-b"')))

    def test_fidelity_missing_plan_refuses_before_build(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            item = pb.QueueItem("f", root / "src/f.c", overlay=1)
            with patch.object(pb, "ROOT", root), patch.object(pb, "bounded_capture") as run:
                with self.assertRaisesRegex(RuntimeError, "missing"):
                    pb.grouped_baseline_fidelity(item, root, root, {}, None)
                run.assert_not_called()

    def test_ungrouped_requires_full_tu_fidelity(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            item = pb.QueueItem("f", root / "src/f.c")
            (root / "source-groups.json").write_text(json.dumps({
                "contract": pb.SOURCE_GROUP_CONTRACT, "symbol": "f", "groups": [], "status": "ungrouped"}))
            with patch.object(pb, "ROOT", root), patch.object(pb, "bounded_capture",
                    side_effect=RuntimeError("full-TU build required")) as run:
                with self.assertRaisesRegex(RuntimeError, "full-TU build required"):
                    pb.grouped_baseline_fidelity(item, root, root, self.plan_inputs(root), None)
                run.assert_called_once()

    def test_symlinked_object_parent_refuses_before_build(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            out = root / "run"
            out.mkdir()
            (root / "outside").mkdir()
            (root / "build_non_matching").symlink_to(root / "outside", target_is_directory=True)
            item = pb.QueueItem("f", root / "src/f.c")
            (out / "source-groups.json").write_text(json.dumps({
                "contract": pb.SOURCE_GROUP_CONTRACT, "symbol": "f", "groups": [{}], "status": "preserved"}))
            with patch.object(pb, "ROOT", root), patch.object(pb, "bounded_capture") as run:
                with self.assertRaisesRegex(RuntimeError, "symlinked"):
                    pb.grouped_baseline_fidelity(item, out, root, self.plan_inputs(out), None)
                run.assert_not_called()

    def normalized(self, word, records):
        with patch.object(pb.reloc_surface, "Elf") as elf, patch.object(
                pb.reloc_surface, "_unique_symbol", return_value=(0, 4, ".text")):
            elf.return_value.section_bytes.return_value = word.to_bytes(4, "big")
            elf.return_value.relocations.return_value = records
            return pb.normalized_owned_instructions(Path("synthetic.o"), "f")

    def test_normalization_preserves_registers_and_nonrelocation_fields(self):
        record = [(".text", 0, 6, 1)]
        self.assertEqual(self.normalized(123, record), self.normalized(456, record))
        self.assertNotEqual(self.normalized(123, record), self.normalized((1 << 16) + 123, record))
        self.assertNotEqual(self.normalized(123, []), self.normalized(456, []))

    def test_duplicate_unknown_and_misaligned_relocations_refuse(self):
        for records in ([(".text", 0, 2, 1)], [(".text", 1, 6, 1)],
                        [(".text", 0, 6, 1), (".text", 0, 6, 1)]):
            with self.subTest(records=records), self.assertRaisesRegex(RuntimeError, "unsupported"):
                self.normalized(123, records)

    def source_records(self, name="a", value=0, info=16, section=0, word=4, duplicate=False):
        with patch.object(pb.reloc_surface, "Elf") as elf, patch.object(
                pb.reloc_surface, "_unique_symbol", return_value=(0, 4, ".text")):
            row = (name, value, 0, info, section)
            elf.return_value.symbols.return_value = [row, row] if duplicate else [row]
            elf.return_value.section_bytes.return_value = word.to_bytes(4, "big")
            elf.return_value.relocations.return_value = [(".text", 0, 4, 0)]
            return pb.raw_source_relocations(Path("synthetic.o"), "f")

    def test_source_correspondence_binds_name_type_binding_addend(self):
        original = self.source_records()
        self.assertEqual(original, self.source_records())
        for kwargs in ({"name": "b"}, {"info": 18}, {"info": 32}, {"word": 5}):
            self.assertNotEqual(original, self.source_records(**kwargs))
        self.assertNotEqual(self.source_records(section=0xFFF1, value=5),
                            self.source_records(section=0xFFF1, value=6))

    def test_defined_and_duplicate_source_names_refuse(self):
        for kwargs in ({"section": 1}, {"duplicate": True}):
            with self.assertRaisesRegex(RuntimeError, "independent"):
                self.source_records(**kwargs)

    def test_partial_target_allows_faithful_unsupported_source_but_not_wrong_addend(self):
        for wrong in (False, True):
            with self.subTest(wrong_addend=wrong), tempfile.TemporaryDirectory() as tmp, contextlib.ExitStack() as stack:
                root = Path(tmp)
                out, scratch = root / "run", root / "run/scratch"
                scratch.mkdir(parents=True)
                item = pb.QueueItem("f", root / "src/f.c", overlay=1)
                target = root / "build_non_matching/src/f.c.o"
                target.parent.mkdir(parents=True)
                (out / "source-groups.json").write_text(json.dumps({"contract": pb.SOURCE_GROUP_CONTRACT,
                    "symbol": "f", "status": "measurement-required", "reason": "unsupported fixture", "groups": []}))
                script = b"current local importer recipe"
                (out / "importer-compile.sh").write_bytes(script)
                for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml"):
                    (scratch / name).write_bytes(b"fixture")
                authorities = [root / name for name in ("build/mickey.us.elf", "rom", "atlas", "values")]
                for path in authorities:
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes(b"authority")
                def run(args, deadline):
                    if "--analysis-only" in args:
                        target.write_bytes(bytes(4))
                        text = json.dumps({"candidate_object": "build_non_matching/src/f.c.o",
                                           "linked_symbol": "f", "preflight": {"status": "partial"}})
                    else:
                        Path(args[-1]).write_bytes(int(wrong).to_bytes(4, "big"))
                        text = "compiled"
                    return subprocess.CompletedProcess(args, 0, text)
                class Elf:
                    def __init__(self, path): self.data = path.read_bytes()
                    def section_bytes(self, name): return self.data
                    def symbols(self): return [("external", 0, 0, 16, 0)]
                    def relocations(self): return [(".text", 0, 4, 0)]
                for name, value in (("ROOT", root), ("BASEROM", authorities[1]), ("ATLAS_PATH", authorities[2])):
                    stack.enter_context(patch.object(pb, name, value))
                stack.enter_context(patch.object(pb.reloc_surface, "LINK_SYMS", authorities[3]))
                stack.enter_context(patch.object(pb.reloc_surface, "Elf", Elf))
                stack.enter_context(patch.object(pb.reloc_surface, "_unique_symbol", return_value=(0, 4, ".text")))
                stack.enter_context(patch.object(pb.reloc_surface, "function_surface_comparison", return_value={"stable_identity_exact": False}))
                stack.enter_context(patch.object(pb, "bounded_capture", side_effect=run))
                stack.enter_context(patch.object(pb, "validate_baseline"))
                stack.enter_context(patch.object(pb, "checked_tool_identity"))
                stack.enter_context(patch.object(pb, "measure_seed_stage", return_value=(
                    SimpleNamespace(source=b"void f(void) { return; }", object=bytes(4), object_sha256="fixture"), 10)))
                inputs = self.plan_inputs(out)
                inputs["context"]["importer_recipe"] = pb.compile_script_digest(script)
                if wrong:
                    with self.assertRaisesRegex(RuntimeError, "source-symbol"):
                        pb.grouped_baseline_fidelity(item, out, scratch, inputs, None)
                else:
                    pb.grouped_baseline_fidelity(item, out, scratch, inputs, None)
                    report = json.loads((out / "source-fidelity/report.json").read_text())
                    self.assertTrue(report["source_fidelity_exact"])
                    self.assertEqual(report["identity_route"], "raw-source-symbols-not-runtime-proof")


if __name__ == "__main__":
    unittest.main()
