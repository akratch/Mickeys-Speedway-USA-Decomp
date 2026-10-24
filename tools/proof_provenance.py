#!/usr/bin/env python3
"""Prove what source selected bytes in a workbench comparison came from.

The workbench can compare any two objects, including an ordinary build object
whose selected function was inserted by asm-processor from GLOBAL_ASM.  Equal
bytes in that case are useful diagnostic evidence, but they are not evidence
that C reproduced the function.  This tool records the distinction before
``wb_compare.sh`` invokes the comparator.

The manifest deliberately lives under ``build/wb``.  It contains hashes and
build provenance, can mention ROM-derived artifacts, and must never be tracked.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import dataclasses
import hashlib
import json
import os
import pathlib
import re
import shlex
import subprocess
import struct
import sys
import tempfile
import time
import uuid
from typing import Iterable, Sequence

import runlink_metadata_link as metadata_link


class MetadataProofError(RuntimeError):
    """Declared metadata steps do not explain the configured object."""


def literal_profile_inputs(root):
    """Stable existing parser/helper sources, declared before proof snapshots."""
    root = pathlib.Path(root)
    names = ["tools/proof_provenance.py", "tools/trim_elf_section.py", "tools/candidate_context.py"]
    package = root / "tools/permuter/perm_pycparser"
    files = sorted(package.rglob("*.py"))
    if not files or not (package / "c_parser.py").is_file():
        raise MetadataProofError("literal profile requires the existing vendored parser")
    return {name: sha256_file(root / name) for name in names} | {
        path.relative_to(root).as_posix(): sha256_file(path) for path in files}


_LITERAL_PARSER_PIN = None


def _literal_declarations(source):
    """Close the owner syntax before the frontend can read any assembly input."""
    import candidate_context as lexical
    try:
        lexical.identity()
    except (RuntimeError, OSError, ValueError) as error:
        raise MetadataProofError("loaded literal lexical helper changed") from error
    if len(source) > lexical.MAX_SOURCE_BYTES or b"\\\n" in source or b"??" in source:
        raise MetadataProofError("unsupported literal source size/splicing/trigraphs")
    text = lexical._strip_comments(source.decode("utf-8"))
    include = re.compile(r'\s*#\s*include\s*(?:"(?P<quoted>[^"\n]+)"|<(?P<angle>[^>\n]+)>)\s*\n')
    while match := include.match(text):
        name = match["quoted"] or match["angle"]
        if pathlib.PurePosixPath(name).is_absolute() or ".." in pathlib.PurePosixPath(name).parts or "\\" in name:
            raise MetadataProofError("unsupported literal root include path")
        text = text[match.end():]
    declaration = re.compile(r'\s*(?P<const>const\s+)?char\s+(?P<name>[A-Za-z_][A-Za-z_0-9]*)\s*\[\s*\]\s*=\s*"(?P<literal>[\x20-\x21\x23-\x5B\x5D-\x7E]*)"\s*;')
    rows = []
    while text.strip():
        match = declaration.match(text)
        if not match:
            raise MetadataProofError("not complete plain ASCII literal-array declarations")
        rows.append(dict(name=match["name"], const=bool(match["const"]), literal=match["literal"]))
        text = text[match.end():]
    if (not rows or len(rows) > lexical.MAX_DECLARATIONS
            or len({row["name"] for row in rows}) != len(rows)
            or len({row["const"] for row in rows}) != 1):
        raise MetadataProofError("mixed/duplicate/empty literal definitions")
    return rows


def literal_array_facts(source, cpp, root):
    """Closed source syntax, corroborated by the existing effective-C parser."""
    global _LITERAL_PARSER_PIN
    import candidate_context as lexical
    before = literal_profile_inputs(root)
    try:
        lexical_identity = lexical.identity()
    except (RuntimeError, OSError, ValueError) as error:
        raise MetadataProofError("loaded literal lexical helper changed") from error
    if lexical_identity["comparator_sha256"] != before["tools/candidate_context.py"]:
        raise MetadataProofError("literal lexical input differs from loaded helper")
    if _LITERAL_PARSER_PIN is None and any(name == "perm_pycparser" or name.startswith("perm_pycparser.")
                                           for name in sys.modules):
        raise MetadataProofError("preloaded literal parser lacks a bound loading identity")
    if _LITERAL_PARSER_PIN is not None and before != _LITERAL_PARSER_PIN:
        raise MetadataProofError("loaded literal parser/helper sources changed")
    package = pathlib.Path(root) / "tools/permuter"
    existing = sys.modules.get("perm_pycparser")
    if existing and pathlib.Path(existing.__file__).resolve().parent != (package / "perm_pycparser").resolve():
        raise MetadataProofError("literal parser namespace differs from the declared tool")
    if str(package) not in sys.path:
        sys.path.insert(0, str(package))
    from perm_pycparser import c_ast, c_parser
    if before != literal_profile_inputs(root):
        raise MetadataProofError("literal parser changed during loading")
    if _LITERAL_PARSER_PIN is None:
        _LITERAL_PARSER_PIN = before
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError("literal CPP exceeds the source proof bound")
    rows = _literal_declarations(source)
    try:
        ast = c_parser.CParser().parse(cpp.decode("utf-8"))
    except Exception as error:
        raise MetadataProofError("effective literal C cannot be parsed") from error
    actual = []
    for node in ast.ext:
        if isinstance(node, c_ast.Typedef):
            continue
        if (not isinstance(node, c_ast.Decl) or node.storage or node.funcspec or node.bitsize
                or not isinstance(node.type, c_ast.ArrayDecl) or node.type.dim is not None or node.type.dim_quals
                or not isinstance(node.type.type, c_ast.TypeDecl)
                or not isinstance(node.type.type.type, c_ast.IdentifierType)
                or node.type.type.type.names != ["char"]
                or node.type.type.quals not in ([], ["const"])
                or not isinstance(node.init, c_ast.Constant) or node.init.type != "string"):
            raise MetadataProofError("unsupported effective literal declaration")
        actual.append((node.name, node.type.type.quals == ["const"], node.init.value))
    if actual != [(row["name"], row["const"], '"' + row["literal"] + '"') for row in rows]:
        raise MetadataProofError("source/effective literal declaration disagreement")
    if before != literal_profile_inputs(root):
        raise MetadataProofError("literal parser/helper changed during parsing")
    try:
        if lexical.identity() != lexical_identity:
            raise RuntimeError("lexical identity differs")
    except (RuntimeError, OSError, ValueError) as error:
        raise MetadataProofError("loaded literal lexical helper changed during parsing") from error
    return rows


def literal_raw_end(elf, rows):
    """Actual unsized definitions bind exact C bytes, including every NUL."""
    import runlink_pack as rp
    section = ".rodata" if rows[0]["const"] else ".data"
    sec = elf.section(section)
    if (struct.unpack_from(">HH", elf.data, 16) != (1, 8) or not sec
            or sec["type"] != 1 or sec["addr"] or sec["align"] != 16
            or sec["flags"] & 3 != (2 if section == ".rodata" else 3) or sec["flags"] & 4
            or any(s["size"] and s["flags"] & 2 and s["name"] not in
                   (section, ".reginfo", ".MIPS.abiflags") for s in elf.sections)
            or any(elf.rels(s["name"]) for s in elf.sections)
            or any(s.shndx == rp.SHN_COMMON for s in elf.symbols)):
        raise MetadataProofError("literal end requires complete ordinary nonexec storage")
    definitions = [s for s in elf.symbols if s.section == section and s.type != 3]
    allocated = {s["name"] for s in elf.sections if s["flags"] & 2 and s["size"]}
    if (len(definitions) != len(rows) or len({s.name for s in definitions}) != len(rows)
            or any(s.name and s.section in allocated and s.section != section and s.type not in (3, 4)
                   for s in elf.symbols)):
        raise MetadataProofError("literal actual definition set differs")
    by_name = {s.name: s for s in definitions}
    payload = elf.bytes_of(section)
    bounds = []
    for row in rows:
        symbol = by_name.get(row["name"])
        data = row["literal"].encode("ascii") + b"\0"
        if (symbol is None or symbol.bind != rp.STB_GLOBAL or symbol.type != 1 or symbol.size != 0
                or not 0 <= symbol.value <= len(payload)-len(data)
                or payload[symbol.value:symbol.value+len(data)] != data):
            raise MetadataProofError("literal actual definition or complete bytes differ")
        bounds.append([symbol.name, symbol.value, len(data), symbol.size])
    if bounds[0][1] or any(a[1]+a[2] > b[1] for a, b in zip(bounds, bounds[1:])):
        raise MetadataProofError("literal definitions overlap or differ in order")
    end = max(value+size for name, value, size, reported in bounds)
    import trim_elf_section as trim
    trim._compiler_storage(elf, section)  # validates the loaded shared helper too
    trim._zero_compiler_suffix(elf, section, sec, definitions, [end])
    return section, end, bounds


def literal_surface(elf):
    import source_module_link_proof as proof
    surface = (proof.allocated(elf), proof.symbols(elf),
               [(s["name"], r.offset, r.type, r.sym.name) for s in elf.sections for r in elf.rels(s["name"])])
    return hashlib.sha256(repr(surface).encode()).hexdigest()


def _literal_flags(command, source):
    words = shlex.split(command)
    if "tools/ido/cc" not in words or words.count("-o") != 1 or words[-1] != source:
        raise MetadataProofError("literal CPP requires its actual IDO source command")
    if "--" in words:
        words = words[words.index("--", words.index("--")+1)+1:]
    else:
        words = words[words.index("tools/ido/cc")+1:]
    flags = []; skip = False
    for word in words:
        if skip:
            skip = False; continue
        if word == "-o":
            skip = True
        elif word not in ("-c", source):
            flags.append(word)
    return flags


def literal_frontend_inputs(root, command):
    words = shlex.split(command)
    if len(words) < 2 or words[1] != "tools/asm-processor/build.py":
        return {}
    if words[:4] != [".venv/bin/python", "tools/asm-processor/build.py", "tools/ido/cc", "--"]:
        raise MetadataProofError("unsupported literal wrapper invocation")
    root = pathlib.Path(root)
    paths = [root/".venv/bin/python", *sorted((root/"tools/asm-processor").rglob("*.py"))]
    if not (root/"tools/asm-processor/asm_processor.py").is_file():
        raise MetadataProofError("literal wrapper frontend is missing")
    return {path.relative_to(root).as_posix(): sha256_file(path) for path in paths}


def _literal_preparation_current(prepared, root):
    import permute_batch as batch
    if batch.checked_tool_identity() != prepared["dependency_tools"]:
        raise MetadataProofError("literal dependency machinery changed")
    expected = prepared["inputs"] | prepared["dependencies"]
    if any(sha256_file(pathlib.Path(root)/name) != digest for name, digest in expected.items()):
        raise MetadataProofError("literal frontend source/tool/header context changed")
    return expected


def prepare_literal_input(source, root, command):
    """Replay the existing wrapper frontend and discover its actual headers."""
    import permute_batch as batch
    root, source = pathlib.Path(root), pathlib.Path(source)
    dependency_tools = batch.checked_tool_identity()
    relative = source.relative_to(root).as_posix()
    flags = _literal_flags(command, relative)
    tools = literal_profile_inputs(root) | literal_frontend_inputs(root, command)
    for path in (pathlib.Path(batch.__file__), pathlib.Path(batch.sweep_receipts.__file__)):
        if path.is_relative_to(root):
            tools[path.relative_to(root).as_posix()] = sha256_file(path)
    tools[relative] = sha256_file(source)
    loaded = source.read_bytes()
    if hashlib.sha256(loaded).hexdigest() != tools[relative]:
        raise MetadataProofError("literal source changed before frontend")
    _literal_declarations(loaded)
    wrapped = len(shlex.split(command)) > 1 and shlex.split(command)[1] == "tools/asm-processor/build.py"
    frontend = None
    compiler_input = loaded
    if wrapped:
        # These are exactly build.py's frontend options and final -I ordering.
        options = [x for x in flags if x in {"-g3", "-g", "-O0", "-O1", "-O2", "-framepointer", "-KPIC"}]
        if "-mips2" not in flags:
            options.append("-mips1")
        frontend = [".venv/bin/python", "tools/asm-processor/asm_processor.py", *options, relative]
        result = subprocess.run(frontend, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                timeout=120, check=False)
        if result.returncode:
            raise MetadataProofError("literal wrapper frontend failed")
        compiler_input = result.stdout
        flags = flags + ["-I", str(source.resolve().parent)]
        # The trusted derivative is only a dependency-discovery input. Like
        # the real wrapper input, its otherwise empty directory cannot shadow
        # normal root include names. It is never a manifest/build dependency.
        (root/"build").mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="literal-deps-", dir=root/"build") as directory:
            dependency_source = pathlib.Path(directory)/("preprocessed_"+uuid.uuid4().hex+source.suffix)
            dependency_source.write_bytes(compiler_input)
            dependencies = batch.source_dependencies(dependency_source, tuple(flags))
    else:
        dependencies = batch.source_dependencies(source, tuple(flags))
    if any(name.startswith("missing:") for name in dependencies):
        raise MetadataProofError("literal include closure has missing inputs")
    prepared = dict(source=relative, source_bytes=loaded, compiler_input=compiler_input,
                    wrapped=wrapped, flags=flags, frontend_command=frontend,
                    inputs=tools, dependencies=dependencies, dependency_tools=dependency_tools)
    _literal_preparation_current(prepared, root)
    return prepared


def prove_literal_arrays(elf, source, root, *, compiler_command=None, prepared=None):
    """Capture source/actual compiler-input/CPP/raw observations and holds."""
    root, source = pathlib.Path(root), pathlib.Path(source)
    relative = source.relative_to(root).as_posix()
    supplied = compiler_command is not None
    if not supplied:
        compiler_command, _flags, error = _discover_compile_command(root, source, non_matching=False)
        if error:
            raise MetadataProofError("literal compiler discovery failed: " + error)
    prepared = prepare_literal_input(source, root, compiler_command) if prepared is None else prepared
    if prepared["source"] != relative:
        raise MetadataProofError("literal preparation belongs to another source")
    def context():
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
            raise MetadataProofError("loaded literal proof helper changed")
        result = dict(_literal_preparation_current(prepared, root))
        result.update({p.relative_to(root).as_posix(): sha256_file(p)
                       for p in [root/"Makefile", *sorted((root/"mk").glob("**/*.mk"))]})
        result.update({p.relative_to(root).as_posix(): sha256_file(p)
                       for p in (root/"tools/ido").rglob("*") if p.is_file()})
        if not supplied:
            current, _flags, error = _discover_compile_command(root, source, non_matching=False)
            if error or current != compiler_command:
                raise MetadataProofError("literal CPP recipe changed")
        return result
    before = context()
    source_bytes = prepared["source_bytes"]
    with tempfile.TemporaryDirectory(prefix="literal-cpp-") as directory:
        if prepared["wrapped"]:
            cpp_input = pathlib.Path(directory)/("preprocessed_"+uuid.uuid4().hex+source.suffix)
            cpp_input.write_bytes(prepared["compiler_input"])
            argument = str(cpp_input)
        else:
            argument = relative
        cpp_command = ["tools/ido/cc", "-E", *prepared["flags"], argument]
        result = subprocess.run(cpp_command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                timeout=120, check=False)
    if result.returncode:
        raise MetadataProofError("literal configured CPP failed")
    cpp = result.stdout
    rows = literal_array_facts(source_bytes, cpp, root)
    section, end, bounds = literal_raw_end(elf, rows)
    if before != context() or (elf.path.is_file() and elf.path.read_bytes() != elf.data):
        raise MetadataProofError("literal source/CPP/raw/tool context changed during proof")
    return dict(schema="mickey-literal-array-end-v1", section=section, end=end,
                input_size=elf.size(section), arrays=bounds, source_facts=rows,
                source_sha256=hashlib.sha256(source_bytes).hexdigest(),
                compiler_input_sha256=hashlib.sha256(prepared["compiler_input"]).hexdigest(),
                frontend_command=prepared["frontend_command"],
                cpp_sha256=hashlib.sha256(cpp).hexdigest(), cpp_text=cpp.decode("utf-8"),
                source=relative, compiler_command=compiler_command, preprocessor_command=cpp_command,
                raw_sha256=hashlib.sha256(elf.data).hexdigest(),
                raw_surface=literal_surface(elf), inputs=before)


def validate_literal_proof(elf, record, root):
    """Revalidate an internally generated fresh proof and loaded raw surface."""
    root = pathlib.Path(root)
    def current():
        return {name: sha256_file(root/name) for name in record["inputs"]}
    if (record.get("schema") != "mickey-literal-array-end-v1" or literal_surface(elf) != record.get("raw_surface")
            or current() != record["inputs"]):
        raise MetadataProofError("stale literal source/raw/tool proof")
    source = (root/record["source"]).read_bytes()
    cpp = record["cpp_text"].encode("utf-8")
    if (hashlib.sha256(source).hexdigest() != record["source_sha256"]
            or hashlib.sha256(cpp).hexdigest() != record["cpp_sha256"]
            or literal_array_facts(source, cpp, root) != record["source_facts"]):
        raise MetadataProofError("loaded literal source/CPP facts differ")
    section, end, bounds = literal_raw_end(elf, record["source_facts"])
    if (section != record["section"] or end != record["end"] or bounds != record["arrays"]
            or elf.size(section) != record["input_size"] or current() != record["inputs"]):
        raise MetadataProofError("literal proof extent/definitions/context differ")
    return end


def generated_metadata_context(root):
    try:
        return metadata_link.provenance_context(root)
    except (metadata_link.MetadataLinkError, metadata_link.rp.PackError, OSError, ValueError, KeyError, TypeError) as error:
        raise MetadataProofError("generated final metadata proof failed: " + str(error)) from error


def _filter_spec_file(argument: str, root: pathlib.Path) -> pathlib.Path:
    """Resolve a filter ``@SPEC_FILE`` exactly as filter_elf_relocations.py does."""
    relative = pathlib.PurePosixPath(argument[1:])
    if not argument[1:] or relative.is_absolute() or ".." in relative.parts:
        raise MetadataProofError("unsafe filter specification file")
    path = root / relative
    if path.is_symlink() or not path.is_file():
        raise MetadataProofError("missing filter specification file")
    return path


_SPEC_HELPERS = ("filter_elf_relocations.py", "rebind_elf_relocations.py")


def filter_spec_files(command: str, root: pathlib.Path) -> dict[str, str]:
    """Digest every ``@SPEC_FILE`` a metadata recipe names (content is a build input).

    Both spec-reading helpers count: a relocation filter and a relocation
    rebind read their ``@SPEC_FILE`` the same way.
    """
    specs = {}
    for segment in command.split("&&"):
        words = shlex.split(segment)
        if len(words) >= 2 and words[1].endswith(_SPEC_HELPERS):
            for word in words[4:]:
                if word.startswith("@"):
                    specs[word[1:]] = sha256_file(_filter_spec_file(word, root))
    return specs


def _expand_specs(arguments: Sequence[str], root: pathlib.Path) -> list[str]:
    """Expand ``@SPEC_FILE`` arguments exactly as the filter/rebind helpers do."""
    expanded = []
    for word in arguments:
        if not word.startswith("@"):
            expanded.append(word)
            continue
        for line in _filter_spec_file(word, root).read_text().splitlines():
            expanded.extend(line.partition("#")[0].split())
    return expanded


# Sections whose removal would discard the proved function or its identity.
_PROTECTED_SECTIONS = {".text", ".rel.text", ".symtab", ".strtab", ".shstrtab"}


def _objcopy_plan(args: list[str]) -> list[tuple[str, object]]:
    """One objcopy invocation: renames, added symbols and removed sections."""
    pairs, added, removed = [], [], []
    while args:
        option = args[0]
        if option.startswith("--remove-section="):
            value, args = option.split("=", 1)[1], args[1:]
            option = "--remove-section"
        elif option in {"--redefine-sym", "--add-symbol", "--remove-section"} and len(args) >= 2:
            value, args = args[1], args[2:]
        else:
            raise MetadataProofError("unsupported objcopy metadata operation")
        if option == "--redefine-sym":
            if value.count("=") != 1:
                raise MetadataProofError("unsupported objcopy metadata operation")
            old, new = value.split("=")
            if not old or not new:
                raise MetadataProofError("empty rename identity")
            pairs.append((old, new))
        elif option == "--add-symbol":
            name, equals, rest = value.partition("=")
            if not name or not equals or not rest:
                raise MetadataProofError("malformed added symbol")
            added.append(value)
        else:
            if not value or value in _PROTECTED_SECTIONS:
                raise MetadataProofError("metadata step removes a protected section")
            removed.append(value)
    plan: list[tuple[str, object]] = []
    if pairs:
        plan.append(("rename", pairs))
    if added:
        plan.append(("add-symbol", added))
    if removed:
        plan.append(("remove-section", removed))
    if not plan:
        raise MetadataProofError("unsupported objcopy metadata operation")
    return plan


def metadata_filter_plan(command: str, target: str,
                         root: pathlib.Path | None = None) -> list[tuple[str, object]]:
    """Parse a POSTPROCESS chain into its ordered, closed set of metadata steps.

    Accepted steps, each one ``&&``-separated simple command:

    * ``objcopy`` with ``--redefine-sym``, ``--add-symbol`` and
      ``--remove-section`` (never of ``.text``, its relocations or the symbol
      tables);
    * ``filter_elf_relocations.py OBJ .text SPEC...``;
    * ``rebind_elf_relocations.py OBJ SECTION OFFSET:EXPECTED:REPLACEMENT...``;
    * ``externalize_elf_section.py OBJ SECTION EXPECTED [ANCHOR] [keep]`` of a
      section other than ``.text``. ``keep`` retains the proved payload and
      is not an anchor; an integer anchor is independent of that flag;
    * ``trim_elf_section.py OBJ .text SIZE [EXPECTED_DISCARDED]``.

    A filter or rebind argument may be ``@SPEC_FILE``, read the way the
    helpers read it: whitespace-separated specifications, ``#`` comments
    ignored.  Anything else, including any shell syntax, is refused.
    """
    root = pathlib.Path(__file__).resolve().parent.parent if root is None else root
    # These are precisely the existing canonical Makefile substitutions, not
    # arbitrary shell expansion or executable lookup.
    for token, replacement in {"$(OBJCOPY)": "tools/binutils/mips64-elf-objcopy",
                               "$(HOST_PYTHON)": ".venv/bin/python",
                               "$(TOOLS_DIR)": "tools", "$@": target}.items():
        command = command.replace(token, replacement)
    plan: list[tuple[str, object]] = []
    for segment in command.split("&&"):
        words = shlex.split(segment)
        if not words or any(token in {";", "|", "||", ">", "<"} or "$" in token for token in words):
            raise MetadataProofError("unsupported metadata command syntax")
        if words[0] == "tools/binutils/mips64-elf-objcopy" and words[-1] == target:
            plan.extend(_objcopy_plan(words[1:-1]))
        elif (len(words) >= 5 and pathlib.Path(words[0]).name.startswith("python")
              and words[2] == target):
            helper = words[1]
            if helper == "tools/filter_elf_relocations.py" and words[3] == ".text":
                requests = []
                for spec in _expand_specs(words[4:], root):
                    try:
                        offset, kind, name = spec.split(":", 2)
                        row = (int(offset, 0), int(kind, 0), name)
                    except ValueError as error:
                        raise MetadataProofError("unsupported filter specification") from error
                    if row[0] < 0 or row[0] % 4 or row[1] not in {4, 5, 6} or not row[2] or row in requests:
                        raise MetadataProofError("invalid or duplicate filter specification")
                    requests.append(row)
                plan.append(("filter", requests))
            elif helper == "tools/rebind_elf_relocations.py":
                requests = []
                for spec in _expand_specs(words[4:], root):
                    try:
                        offset, expected, replacement = spec.split(":", 2)
                        row = (int(offset, 0), expected, replacement)
                    except ValueError as error:
                        raise MetadataProofError("unsupported rebind specification") from error
                    if (row[0] < 0 or row[0] % 4 or not expected or not replacement
                            or expected == replacement
                            or row[0] in {other[0] for other in requests}):
                        raise MetadataProofError("invalid or duplicate rebind specification")
                    requests.append(row)
                if not requests:
                    raise MetadataProofError("empty rebind specification")
                plan.append(("rebind", (words[3], requests)))
            elif (helper == "tools/externalize_elf_section.py"
                  and 5 <= len(words) <= 7
                  and words[3] not in _PROTECTED_SECTIONS):
                anchor = 0
                saw_anchor = False
                saw_keep = False
                for extra in words[5:]:
                    if extra == "keep":
                        if saw_keep:
                            raise MetadataProofError("unsupported metadata helper")
                        saw_keep = True
                        continue
                    if saw_anchor:
                        raise MetadataProofError("unsupported metadata helper")
                    try:
                        anchor = int(extra, 0)
                    except ValueError as error:
                        raise MetadataProofError("unsupported metadata helper") from error
                    if not 0 <= anchor < 0x8000:
                        raise MetadataProofError("unsupported metadata helper")
                    saw_anchor = True
                plan.append(("externalize", (words[3], words[4], anchor)))
            elif (helper == "tools/trim_elf_section.py" and words[3] in (".data", ".rodata")
                  and len(words) == 7 and words[5] == "--literal-array-end"):
                source = words[6]
                if (not re.fullmatch(r"src/[A-Za-z_0-9/]+\.c", source)
                        or target != "build/"+source+".o"):
                    raise MetadataProofError("literal profile lacks its actual configured source")
                plan.append(("literal-end", (words[3], int(words[4], 0), source)))
            elif (helper == "tools/trim_elf_section.py" and words[3] in (".data", ".rodata")
                  and len(words) == 6 and words[5] == "--compiler-end-padding"):
                plan.append(("nonexec-end", (words[3], int(words[4], 0))))
            elif (helper == "tools/trim_elf_section.py" and words[3] == ".text"
                  and len(words) in (5, 6)):
                plan.append(("trim", int(words[4], 0)))
            else:
                raise MetadataProofError("unsupported metadata helper")
        else:
            raise MetadataProofError("unsupported ordered metadata operation")
    import reloc_identity as ri
    closure = ri.canonicalize_redefine_aliases([pair for operation, pairs in plan
                                               if operation == "rename" for pair in pairs])
    if closure.ambiguous or closure.cycles:
        raise MetadataProofError("ambiguous metadata symbol rename identities")
    return plan


def deferred_sections(plan: Sequence[tuple[str, object]]) -> set[str]:
    """Sections a later step removes that a declared filter still references.

    ``objcopy --remove-section`` refuses a section a relocation still names, so
    a recipe that filters a section-symbol relocation (``0x78:5:.rodata``) and
    then removes that section cannot be replayed without its filter.  The
    replay keeps such a section, and the accounting removes it after the
    filters instead, as the build does.
    """
    named = {name for operation, arguments in plan if operation == "filter"
             for _offset, _kind, name in arguments if name.startswith(".")}
    return {section for operation, arguments in plan if operation == "remove-section"
            for section in arguments if section in named}


def filter_accounting_plan(plan: Sequence[tuple[str, object]]) -> list[tuple[str, object]]:
    """The declared filters, restated against the object with every other step applied.

    The proof compares the configured object with a *replay* of the recipe that
    skips only the filters (``replay_metadata``).  Filters commute with every
    other accepted step at the sites they name -- a rebind of a filtered site
    after the filter would itself fail, and one before it is already reflected
    in the filter's spelling -- with two exceptions this restatement carries:
    a rename *after* a filter changes the name the filtered record carries in
    the replay, so each filter's symbol is carried through the renames that
    follow it; and a section the filters free for a later removal
    (``deferred_sections``) is removed here, after them.
    """
    def forward(name: str, later: Sequence[tuple[str, object]]) -> str:
        # One objcopy invocation renames simultaneously; invocations compose.
        for other, pairs in later:
            if other == "rename":
                name = dict(pairs).get(name, name)
        return name

    result: list[tuple[str, object]] = []
    for index, (operation, arguments) in enumerate(plan):
        if operation == "filter":
            result.append(("filter", [(offset, kind, forward(name, plan[index + 1:]))
                                      for offset, kind, name in arguments]))
    deferred = deferred_sections(plan)
    if deferred:
        result.append(("remove-section", sorted(deferred)))
    return result


def _replay_words(words: list[str], deferred: set[str]) -> list[str] | None:
    """An objcopy invocation without its removals of ``deferred`` sections."""
    kept, index = [], 0
    while index < len(words):
        word = words[index]
        if word.startswith("--remove-section=") and word.split("=", 1)[1] in deferred:
            index += 1
            continue
        if word == "--remove-section" and index + 1 < len(words) and words[index + 1] in deferred:
            index += 2
            continue
        kept.append(word)
        index += 1
    return kept if any(word.startswith("--") for word in kept) else None


def replay_metadata(raw: pathlib.Path, command: str, target: str, output: pathlib.Path,
                    root: pathlib.Path, *, skip_filters: bool, deadline: float,
                    plan: Sequence[tuple[str, object]] = ()) -> pathlib.Path:
    """Run the configured POSTPROCESS steps on a copy of ``raw``, in order.

    ``command`` is the Make-expanded recipe ``metadata_filter_plan`` accepted,
    so every segment is one simple command naming ``target``; each runs as an
    argument vector (no shell) with ``target`` replaced by ``output``.  With
    ``skip_filters`` the relocation filters are left out, and so is the removal
    of any section they free (``deferred_sections`` of ``plan``).
    """
    import shutil
    shutil.copyfile(raw, output)
    deferred = deferred_sections(plan) if skip_filters else set()
    environment = {key: value for key, value in os.environ.items() if key != "PROMOTION_TRIAL"}
    for segment in command.split("&&"):
        words = shlex.split(segment)
        if skip_filters and len(words) >= 2 and words[1].endswith("filter_elf_relocations.py"):
            continue
        if deferred and words and words[0].endswith("objcopy"):
            replayed = _replay_words(words, deferred)
            if replayed is None:
                continue
            words = replayed
        argv = [str(output) if word == target else word for word in words]
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise MetadataProofError("metadata replay exceeded its deadline")
        result = subprocess.run(argv, cwd=root, env=environment, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                timeout=remaining, check=False)
        if result.returncode:
            raise MetadataProofError("metadata replay step failed: %s" % (
                result.stdout.strip().splitlines() or ["exit %d" % result.returncode])[-1])
    return output


def validate_metadata_objects(raw, configured, plan, symbol: str) -> dict:
    """Account for every text relocation and all allocated bytes, without editing either object."""
    import reloc_surface as rs
    def rows(elf):
        syms = elf.symbols()
        result = []
        for section, offset, kind, index in elf.relocations(r".*"):
            name, value, size, info, shndx = syms[index]
            owner = elf.names[shndx] if shndx < len(elf.names) else shndx
            result.append((section, offset, kind, name, value, size, info, owner))
        return result
    expected = rows(raw)
    symbols = lambda elf: [(name, value, size, info, elf.names[index] if index < len(elf.names) else index)
                           for name, value, size, info, index in elf.symbols()]
    expected_symbols = symbols(raw)
    removed = []
    removed_sections: set[str] = set()
    text = raw.section_bytes(".text")
    start, size = rs._function_text_symbol(raw, [symbol])
    if rs._function_text_symbol(configured, [symbol]) != (start, size):
        raise MetadataProofError("metadata changed owned function geometry")
    for operation, arguments in plan:
        if operation == "rename":
            for old, new in arguments:
                expected = [row[:3] + (new if row[3] == old else row[3],) + row[4:] for row in expected]
                expected_symbols = [(new if row[0] == old else row[0],) + row[1:] for row in expected_symbols]
        elif operation == "filter":
            for offset, kind, name in arguments:
                matches = [row for row in expected if row[:4] == (".text", offset, kind, name)]
                if len(matches) != 1:
                    raise MetadataProofError("declared filter lacks exactly one raw relocation")
                expected.remove(matches[0])
                removed.append(matches[0])
        elif operation == "remove-section":
            for name in arguments:
                if any(row[7] == name for row in expected if row[0] != name):
                    raise MetadataProofError("removed section is still a relocation target")
                expected = [row for row in expected if row[0] != name]
                expected_symbols = [row for row in expected_symbols if row[4] != name]
                removed_sections.add(name)
        elif operation == "trim":
            if arguments < start + size or arguments > len(text) or any(text[arguments:]):
                raise MetadataProofError("trim would remove owned instructions or nonzero bytes")
            text_index = raw.section(".text")[0]
            if any(index == text_index and info & 15 == 2 and value + extent > arguments
                   for _name, value, extent, info, index in raw.symbols()):
                raise MetadataProofError("trim would remove a neighboring function extent")
            text = text[:arguments]
        else:
            raise MetadataProofError("unsupported metadata plan operation")
    if text != configured.section_bytes(".text"):
        raise MetadataProofError("raw/configured instruction bytes differ")
    def allocated(elf):
        result = {}
        for name, header in zip(elf.names, elf.sh):
            if not header[2] & 2:
                continue
            if name in result:
                raise MetadataProofError("duplicate allocated section ownership")
            result[name] = header
        return result
    raw_sections, configured_sections = allocated(raw), allocated(configured)
    raw_sections = {name: header for name, header in raw_sections.items()
                    if name not in removed_sections}
    if set(raw_sections) != set(configured_sections):
        raise MetadataProofError("allocated section ownership changed")
    for name, header in raw_sections.items():
        other = configured_sections[name]
        if name == ".text":
            if (header[1], header[2], header[3], header[8]) != (other[1], other[2], other[3], other[8]) or other[5] != len(text):
                raise MetadataProofError("text section geometry changed outside declared trim")
            continue
        if (header[1], header[2], header[3], header[5], header[8]) != (other[1], other[2], other[3], other[5], other[8]):
            raise MetadataProofError("allocated section geometry changed")
        if header[1] != 8 and raw.section_bytes(name) != configured.section_bytes(name):
            raise MetadataProofError("allocated data bytes changed")
    if collections.Counter(expected) != collections.Counter(rows(configured)):
        raise MetadataProofError("unlisted relocation removal, addition or retargeting")
    if collections.Counter(expected_symbols) != collections.Counter(symbols(configured)):
        raise MetadataProofError("unlisted symbol table change")
    owned = [row for row in rows(raw) if row[0] == ".text" and start <= row[1] < start + size]
    filtered = [row for row in removed if start <= row[1] < start + size]
    if not filtered:
        raise MetadataProofError("no declared filters belong to the requested function")
    return {"raw_count": len(owned), "retained_count": len(owned) - len(filtered),
            "filtered": [{"offset": row[1] - start, "rtype": row[2], "symbol": row[3]}
                         for row in filtered], "start": start, "size": size}


def linked_bss_base(raw, configured, linked, overlay: int, runtime_base: int, runtime_size=None) -> tuple[int, int]:
    """Derive a TU BSS base from unique matching raw/configured/linked definitions."""
    section = f".overlay_{overlay:03d}_bss"
    matches = [(i, sh) for i, (name, sh) in enumerate(zip(linked.names, linked.sh)) if name == section]
    if len(matches) != 1:
        raise MetadataProofError("ambiguous linked BSS section ownership")
    linked_index, header = matches[0]
    if header[1] != 8 or not header[2] & 2 or (runtime_size is not None and header[5] > runtime_size):
        raise MetadataProofError("linked BSS exceeds canonical runtime ownership")
    raw_index, raw_header = raw.section(".bss")
    configured_index, configured_header = configured.section(".bss")
    if raw_index is None or configured_index is None or raw_header[5] != configured_header[5]:
        raise MetadataProofError("missing or changed canonical BSS extent")
    definitions = [row for row in raw.symbols() if row[4] == raw_index and row[0] and row[0] != ".bss"]
    if not definitions:
        raise MetadataProofError("raw BSS has no named canonical ownership witness")
    bases = set()
    for name, value, size, info, _section in definitions:
        current = [row for row in configured.symbols() if row[0] == name and row[4] == configured_index]
        # Absolute diagnostic aliases are not physical section definitions.
        # Multiple real section owners remain ambiguous, even across overlays.
        final = [row for row in linked.symbols() if row[0] == name and 0 < row[4] < len(linked.sh)]
        if len(current) != 1 or len(final) != 1 or current[0][1:4] != (value, size, info) or final[0][2:] != (size, info, linked_index):
            raise MetadataProofError("ambiguous or inconsistent canonical BSS symbol witness")
        base = final[0][1] - header[3] - value
        if base < 0 or base + raw_header[5] > header[5]:
            raise MetadataProofError("raw BSS definition escapes linked section ownership")
        bases.add(base)
    if len(bases) != 1:
        raise MetadataProofError("canonical BSS definitions disagree on section placement")
    return overlay, runtime_base + next(iter(bases))


ORDINARY_C = "ordinary_c"
NON_MATCHING_C = "non_matching_candidate"
GLOBAL_ASM = "global_asm_fallback"
UNKNOWN = "unknown"

_PRAGMA_RE = re.compile(
    r'^\s*#\s*pragma\s+GLOBAL_ASM\s*\(\s*"(?P<path>[^"]+\.s)"\s*\)',
    re.MULTILINE,
)
_DIRECTIVE_RE = re.compile(
    r"^\s*#\s*(?P<kind>ifdef|ifndef|if|elif|else|endif)\b(?P<arg>.*)$"
)
_DEFINED_NM_RE = re.compile(
    r"(?:defined\s*\(\s*NON_MATCHING\s*\)|defined\s+NON_MATCHING|\bNON_MATCHING\b)"
)


@dataclasses.dataclass(frozen=True)
class Occurrence:
    line: int
    non_matching_state: bool | None


@dataclasses.dataclass(frozen=True)
class PragmaOccurrence(Occurrence):
    path: str
    symbol: str


@dataclasses.dataclass(frozen=True)
class SourceFacts:
    definitions: tuple[Occurrence, ...]
    pragmas: tuple[PragmaOccurrence, ...]


@dataclasses.dataclass
class _Conditional:
    mentions_non_matching: bool
    true_when_non_matching: bool | None


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


_LOADED_METADATA_PROOF = sha256_file(pathlib.Path(__file__))


def producer_link_context(root, linked, map_path):
    """Bind producer inputs without requiring the final metadata output yet."""
    pairs = {
        (metadata_link.BOOT_ELF, metadata_link.BOOT_MAP): False,
        ("build/mickey.us.elf", "build/mickey.us.map"): True,
    }
    pair = (linked.relative_to(root).as_posix(), map_path.relative_to(root).as_posix())
    if pair not in pairs:
        raise MetadataProofError("producer capture requires the actual bootstrap/final ELF-map pair")
    final = pairs[pair]
    try:
        command = metadata_link.link_command(root, final=final)
    except (metadata_link.MetadataLinkError, OSError, ValueError) as error:
        raise MetadataProofError("unbound metadata producer input link: " + str(error)) from error
    files = [pair[0], pair[1], metadata_link.FINAL_LOG if final else metadata_link.BOOT_LOG,
             "mickey.us.ld", "tools/link_diagnose.sh", "tools/run_logged.py",
             "tools/binutils/mips64-elf-ld", "undefined_funcs_auto.us.txt",
             "undefined_syms_auto.us.txt", "libultra_undefined_syms.us.txt",
             "overlay_undefined_syms.us.txt"]
    if final:
        files.append(metadata_link.FINAL_SCRIPT)
    return {"role": "metadata-producer-input", "command": command,
            "inputs": {name: sha256_file(root / name) for name in files}}


def capture_configured_raw(root, source, configured, linked, postprocess, *, module_link=False):
    """Promotion capture always requires the current generated-final proof."""
    return _capture_configured_raw(root, source, configured, linked, postprocess,
                                   module_link=module_link)


def local_ro_provider_input(command, root):
    """One stable ordinary provider object explicitly declares a runtime origin."""
    import shlex
    words = shlex.split(command)
    flag = "--local-ro-origin-object"
    if flag not in words:
        return None
    if words.count(flag) != 1 or words.index(flag) + 1 >= len(words):
        raise MetadataProofError("ambiguous LOCAL data provider declaration")
    name = words[words.index(flag) + 1]
    path = pathlib.Path(name)
    if (path.is_absolute() or ".." in path.parts
            or not re.fullmatch(r"build/src/overlays/o[0-9]{3}/[^/]+\.c\.o", name)
            or (root / path).resolve().parent != (root / path.parent).resolve()):
        raise MetadataProofError("LOCAL data provider is not a stable ordinary source object")
    return name


def local_data_aggregate(command, root):
    """An explicit one-provider mixed profile; defaults retain their old shape."""
    import shlex
    words = shlex.split(command)
    flag = "--local-data-aggregate"
    if flag not in words:
        return False
    if words.count(flag) != 1 or any(flag in words for flag in ("--local-ro-provider-group", "--local-ro-mixed-provider-group")) or not local_ro_provider_input(command, root):
        raise MetadataProofError("mixed LOCAL aggregate needs one explicit provider")
    return True


def local_ro_provider_inputs(command, root):
    """Distinct explicit ordered ordinary paths; never a generated group C path."""
    import shlex
    words = shlex.split(command)
    group_flags = [flag for flag in ("--local-ro-provider-group", "--local-ro-mixed-provider-group") if flag in words]
    if len(group_flags) > 1:
        raise MetadataProofError("conflicting LOCAL provider profiles")
    flag = group_flags[0] if group_flags else "--local-ro-provider-group"
    if flag not in words:
        one = local_ro_provider_input(command, root)
        return (one,) if one else ()
    if (words.count(flag) != 1 or "--local-ro-origin-object" in words
            or "--local-data-aggregate" in words):
        raise MetadataProofError("conflicting LOCAL provider profiles")
    names = words[words.index(flag) + 1:]
    if len(names) < 2 or len(set(names)) != len(names) or any(n.startswith("--") for n in names):
        raise MetadataProofError("ambiguous ordered LOCAL provider group")
    return tuple(local_ro_provider_input("helper --local-ro-origin-object " + name, root)
                 for name in names)


def local_ro_mixed_group(command, root):
    import shlex
    mixed = "--local-ro-mixed-provider-group" in shlex.split(command)
    if mixed:
        names = local_ro_provider_inputs(command, root)
        if len(names) != 2:
            raise MetadataProofError("mixed LOCAL group requires exactly two complete providers")
    return mixed


def capture_data_module_producer_raw(root, source, configured, linked, map_path):
    """Fresh producer capture, distinct from and inadmissible as promotion proof."""
    return _capture_configured_raw(root, source, configured, linked, "", module_link=True,
                                   producer_map=map_path)


def capture_nonexec_producer_raw(root, source, configured, linked, map_path):
    """Fresh original compiler output for a declared nonexec metadata recipe."""
    return _capture_configured_raw(root, source, configured, linked, "", producer_map=map_path)


def capture_data_provider_raw(root, source, configured, linked, map_path):
    """Fresh ordinary-provider C using the same bound producer capture lifecycle."""
    return _capture_configured_raw(root, source, configured, linked, "", producer_map=map_path,
                                   ordinary_provider=True)


def _capture_configured_raw(root, source, configured, linked, postprocess, *,
                            module_link=False, producer_map=None, ordinary_provider=False):
    """Fresh reproduction only: never authenticate an externally supplied raw object."""
    import permute_batch as batch
    import sweep_receipts as receipts
    if root.resolve() != batch.ROOT.resolve() or os.environ.get("PROMOTION_TRIAL", "") not in ("", "0"):
        raise MetadataProofError("unsupported raw-proof root or relaxed trial environment")
    target = configured.relative_to(root).as_posix()
    deadline = time.monotonic() + 120
    static_lines = []
    def recipe():
        output = batch.bounded_capture(
            ["gmake", "--no-print-directory", "-n", "-W", source.relative_to(root).as_posix(), target],
            deadline, check=True).stdout.replace("\\\n", " ").splitlines()
        compilers = [line.strip() for line in output if target in line and "tools/ido/cc" in line]
        helper = ("tools/link_overlay_module.py" if module_link else
                  "tools/trim_elf_section.py" if ordinary_provider else "tools/filter_elf_relocations.py")
        metadata = [line.strip() for line in output if target in line and helper in line]
        # The generated resident-call renames (mk/overlay_resident_rebinds.generated.mk)
        # run as their own recipe line after POSTPROCESS; replay them too, in that order.
        rebinds = [line.strip() for line in output if target in line and "--redefine-sym" in line
                   and "tools/filter_elf_relocations.py" not in line and "tools/ido/cc" not in line]
        if (len(compilers) != 1 or (len(metadata) not in (0, 1) if ordinary_provider else len(metadata) != 1)
                or len(rebinds) > (0 if module_link or ordinary_provider else 1)):
            raise MetadataProofError("ambiguous expanded compiler/metadata recipe")
        # An overlay object gets its static functions' symbols right after the
        # compile (tools/static_symbols.py): part of the raw object, not of the
        # declared metadata.
        static_lines[:] = [line.strip() for line in output
                           if target in line and "tools/static_symbols.py" in line]
        if len(static_lines) > 1:
            raise MetadataProofError("ambiguous static-symbol recipe")
        if ordinary_provider:
            if metadata:
                provider_plan = metadata_filter_plan(metadata[0], target, root)
                if len(provider_plan) != 1 or provider_plan[0][0] not in ("nonexec-end", "literal-end"):
                    raise MetadataProofError("ordinary provider has an unsupported end profile")
            allowed = set(compilers + static_lines + metadata)
            if any(target in line and line.strip() not in allowed for line in output):
                raise MetadataProofError("ordinary provider has an unsupported metadata recipe")
            return compilers[0], (metadata[0] if metadata else ""), ""
        return compilers[0], metadata[0], (rebinds[0] if rebinds else "")
    command, expanded_postprocess, surface_rebind = recipe()
    args = batch.compiler_arguments(command, source.relative_to(root).as_posix(), target)
    words = shlex.split(command)
    wrapped = len(words) > 1 and words[1] == "tools/asm-processor/build.py"
    dependency_args = args + (("-I", str(source.parent)) if wrapped else ())
    if words.count("-o") != 1:
        raise MetadataProofError("ambiguous configured output argument")
    plan = ([] if module_link or not expanded_postprocess else
            metadata_filter_plan(expanded_postprocess, target, root))
    if surface_rebind:
        plan = plan + metadata_filter_plan(surface_rebind, target, root)
    literal_prepared = (prepare_literal_input(source, root, command)
                        if any(operation == "literal-end" for operation, arguments in plan) else None)
    def context():
        batch.checked_tool_identity()
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
            raise MetadataProofError("loaded metadata proof implementation changed")
        if recipe() != (command, expanded_postprocess, surface_rebind):
            raise MetadataProofError("configured compiler recipe changed during raw proof")
        inputs = {"source": sha256_file(source), "configured": sha256_file(configured),
                  "linked": sha256_file(linked), "command": command, "postprocess": expanded_postprocess, "surface_rebind": surface_rebind,
                  "dependencies": (dict(literal_prepared["dependencies"]) if literal_prepared else
                                   batch.source_dependencies(source, dependency_args, deadline)),
                  "wrapper_source_directory": receipts.tree_digest(source.parent) if wrapped else None,
                  "include_tree": receipts.tree_digest(root / "include"),
                  "tools": batch.sweep_tool_identity(),
                  "asm_processor": receipts.tree_digest(root / "tools/asm-processor"),
                  "makefiles": {p.relative_to(root).as_posix(): sha256_file(p)
                                for p in [root / "Makefile", *sorted((root / "mk").glob("**/*.mk"))]},
                  "proof": _LOADED_METADATA_PROOF,
                  "target_inputs": {name: sha256_file(root / name) for name in
                    ("baseroms/mickey.us.z64", "config/overlays.us.json",
                     "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt")},
                  "generated_metadata": (generated_metadata_context(root) if producer_map is None
                                         else producer_link_context(root, linked, producer_map)),
                  "filter_specs": filter_spec_files(expanded_postprocess, root),
                  "metadata_tools": {name: sha256_file(root / name) for name in
                    ("tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
                     "tools/rebind_elf_relocations.py", "tools/externalize_elf_section.py",
                     "tools/postprocess_guard.py", "tools/binutils/mips64-elf-objcopy")}}
        if literal_prepared:
            _literal_preparation_current(literal_prepared, root)
            inputs["metadata_tools"].update(literal_profile_inputs(root))
            inputs["metadata_tools"].update(literal_frontend_inputs(root, command))
        if module_link:
            inputs["module_link"] = expanded_postprocess
            inputs["module_link_inputs"] = {name: sha256_file(root / name) for name in (
                "config/runlink-ort.us.txt", "tools/link_overlay_module.py", "tools/runlink_pack.py",
                "tools/rebind_elf_relocations.py", "tools/binutils/mips64-elf-ld")}
            for provider in local_ro_provider_inputs(expanded_postprocess, root):
                inputs["module_link_inputs"][provider] = sha256_file(root / provider)
        if static_lines:
            inputs["static_symbols"] = {"command": static_lines[0], "tool": sha256_file(
                root / "tools/static_symbols.py")}
        assembly = {}
        for pragma in _all_pragmas(source.read_text()):
            path = root / pragma.path
            data = path.read_text()
            # This bounded capture does not guess nested assembler dependency lookup.
            for directive, name in re.findall(r'^\s*\.(include|incbin)\s+"([^"]+)"', data, re.M):
                if directive == "include" and (root / "include" / name).is_file():
                    continue  # entire canonical include tree is pinned above
                raise MetadataProofError("unsupported nested assembly dependency in raw proof")
            assembly[pragma.path] = sha256_file(path)
        inputs["assembly"] = assembly
        return inputs
    before = context()
    parent = root / ("build/data-provider-proof" if ordinary_provider else
                     "build/data-module-producer-proof" if producer_map is not None else
                     "build/source-module-link-proof" if module_link else "build/metadata-filter-proof")
    parent.mkdir(parents=True, exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix="capture-", dir=parent))
    raw = directory / "raw.o"
    words[words.index("-o") + 1] = str(raw)
    (directory / "inputs.json").write_text(json.dumps(before, sort_keys=True, indent=2))
    try:
        result = batch.bounded_capture(words, deadline, check=True)
    except BaseException as failure:
        (directory / "compile.log").write_text(str(getattr(failure, "output", "")) + "\n" + repr(failure))
        raise
    (directory / "compile.log").write_text(result.stdout)
    if static_lines:
        import static_symbols as statics
        try:
            raw.write_bytes(statics.add_static_symbols(raw.read_bytes())[0])
        except statics.Failure as error:
            raise MetadataProofError(f"static-symbol pass failed: {error}") from error
    if before != context():
        raise MetadataProofError("source, input, recipe, object or tool changed during raw proof")
    literal_arrays = {}
    for operation, arguments in plan:
        if operation == "literal-end":
            section, size, source_name = arguments
            if source_name != source.relative_to(root).as_posix():
                raise MetadataProofError("literal profile/source capture differs")
            import runlink_pack as rp
            literal = prove_literal_arrays(rp.Elf(raw), source, root, compiler_command=command, prepared=literal_prepared)
            if literal["section"] != section or literal["end"] != size:
                raise MetadataProofError("literal recipe differs from source-derived end")
            literal_arrays[section] = literal
    if before != context():
        raise MetadataProofError("literal capture context changed during source proof")
    receipt = {"schema": ("mickey-data-provider-capture-v1" if ordinary_provider
                          else "mickey-data-module-producer-capture-v1" if producer_map is not None
                          else "mickey-source-module-link-proof-v1" if module_link
                          else "mickey-declared-metadata-proof-v1"), "inputs": before,
               "raw_object": raw.relative_to(root).as_posix(), "raw_sha256": sha256_file(raw),
               "configured_sha256": sha256_file(configured),
               "linked_sha256": sha256_file(linked), "directory": directory.relative_to(root).as_posix()}
    if literal_arrays:
        receipt["literal_arrays"] = literal_arrays
    return raw, plan, receipt, context


def _mask_c(text: str) -> str:
    """Mask comments and literals while preserving offsets and newlines."""

    out = list(text)
    i = 0
    state = "code"
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line_comment"
                continue
            if char == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block_comment"
                continue
            if char == '"':
                out[i] = " "
                i += 1
                state = "string"
                continue
            if char == "'":
                out[i] = " "
                i += 1
                state = "char"
                continue
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                out[i] = " "
            i += 1
            continue
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
                continue
            if char != "\n":
                out[i] = " "
            i += 1
            continue
        else:
            delimiter = '"' if state == "string" else "'"
            if char == "\\":
                out[i] = " "
                if i + 1 < len(text):
                    if text[i + 1] != "\n":
                        out[i + 1] = " "
                    i += 2
                    continue
            if char == delimiter:
                out[i] = " "
                state = "code"
            elif char != "\n":
                out[i] = " "
            i += 1
            continue
        i += 1
    return "".join(out)


def _directive_condition(kind: str, argument: str) -> _Conditional:
    argument = argument.strip()
    if kind == "ifdef":
        mentions = argument.split(None, 1)[0] == "NON_MATCHING" if argument else False
        return _Conditional(mentions, True if mentions else None)
    if kind == "ifndef":
        mentions = argument.split(None, 1)[0] == "NON_MATCHING" if argument else False
        return _Conditional(mentions, False if mentions else None)
    mentions = bool(_DEFINED_NM_RE.search(argument))
    if not mentions:
        return _Conditional(False, None)
    compact = re.sub(r"\s+", "", argument)
    negative = (
        "!defined(NON_MATCHING)" in compact
        or compact.startswith("!NON_MATCHING")
        or compact in {"NON_MATCHING==0", "0==NON_MATCHING"}
    )
    return _Conditional(True, not negative)


def _line_non_matching_states(text: str) -> list[bool | None]:
    states: list[bool | None] = []
    stack: list[_Conditional] = []
    for line in text.splitlines(keepends=True):
        selected = [
            frame.true_when_non_matching
            for frame in stack
            if frame.mentions_non_matching
        ]
        states.append(selected[-1] if selected else None)
        match = _DIRECTIVE_RE.match(line)
        if not match:
            continue
        kind = match.group("kind")
        argument = match.group("arg")
        if kind in {"ifdef", "ifndef", "if"}:
            stack.append(_directive_condition(kind, argument))
        elif kind == "else" and stack:
            frame = stack[-1]
            if frame.mentions_non_matching and frame.true_when_non_matching is not None:
                frame.true_when_non_matching = not frame.true_when_non_matching
        elif kind == "elif" and stack:
            stack[-1] = _directive_condition("if", argument)
        elif kind == "endif" and stack:
            stack.pop()
    if not text.endswith("\n"):
        # splitlines() already emitted the final unterminated line.
        return states
    return states


def _line_starts(text: str) -> list[int]:
    starts = [0]
    starts.extend(match.end() for match in re.finditer("\n", text))
    return starts


def _line_number(starts: Sequence[int], offset: int) -> int:
    return bisect.bisect_right(starts, offset)


def _state_at(states: Sequence[bool | None], line: int) -> bool | None:
    return states[line - 1] if 0 < line <= len(states) else None


def _matching_paren(masked: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "(":
            depth += 1
        elif masked[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    return None


def _brace_depth(masked: str, stop: int) -> int:
    depth = 0
    for char in masked[:stop]:
        if char == "{":
            depth += 1
        elif char == "}" and depth:
            depth -= 1
    return depth


_KR_IDENTIFIER_LIST_RE = re.compile(r"\s*[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*\s*")


def _kr_parameter_declarations(parameters: str, masked: str, tail: int) -> bool:
    """True when ``tail`` opens a K&R parameter-declaration list ending at ``{``.

    ``f(a, b) T a; U b; {`` is a definition in old-style C. The parameter
    list must be bare identifiers, and everything before the body's ``{``
    must be non-empty ``;``-terminated declarations, which a prototype
    (``f(a);``) or a call can never satisfy.
    """
    if not _KR_IDENTIFIER_LIST_RE.fullmatch(parameters):
        return False
    body = masked.find("{", tail)
    if body < 0:
        return False
    declarations = masked[tail:body]
    if any(char in declarations for char in "}=#") or not declarations.rstrip().endswith(";"):
        return False
    return all(segment.strip() for segment in declarations.rstrip()[:-1].split(";"))


def source_facts(text: str, symbol: str) -> SourceFacts:
    """Find global definitions and GLOBAL_ASM fallbacks for ``symbol``."""

    states = _line_non_matching_states(text)
    starts = _line_starts(text)
    masked = _mask_c(text)
    definitions: list[Occurrence] = []
    pattern = re.compile(rf"\b{re.escape(symbol)}\s*\(")
    for match in pattern.finditer(masked):
        if _brace_depth(masked, match.start()) != 0:
            continue
        opening = masked.find("(", match.start(), match.end())
        closing = _matching_paren(masked, opening)
        if closing is None:
            continue
        tail = closing + 1
        while tail < len(masked) and masked[tail].isspace():
            tail += 1
        if tail >= len(masked):
            continue
        if masked[tail] != "{" and not _kr_parameter_declarations(
                masked[opening + 1:closing], masked, tail):
            continue
        line = _line_number(starts, match.start())
        definitions.append(Occurrence(line, _state_at(states, line)))

    pragmas: list[PragmaOccurrence] = []
    for match in _PRAGMA_RE.finditer(text):
        path = match.group("path")
        pragma_symbol = pathlib.PurePosixPath(path).stem
        if pragma_symbol != symbol:
            continue
        line = _line_number(starts, match.start())
        pragmas.append(
            PragmaOccurrence(line, _state_at(states, line), path, pragma_symbol)
        )
    return SourceFacts(tuple(definitions), tuple(pragmas))


def _macro_spells(text: str, symbol: str) -> bool:
    """True when a ``#define`` in ``text`` names or expands to ``symbol``."""
    return any(re.search(rf"\b{re.escape(symbol)}\b", line)
               for line in _mask_c(text).splitlines()
               if re.match(r"\s*#\s*define\b", line))


_CONDITIONAL_RE = re.compile(r"^\s*#\s*(?:if|ifdef|ifndef|elif)\b", re.MULTILINE)


def needs_preprocessed_view(text: str, symbol: str) -> bool:
    """True when only the compiler's view can say which definition is compiled.

    The written source answers whenever it holds a ``GLOBAL_ASM`` for the
    symbol, exactly one definition, or any ``NON_MATCHING``-guarded one (the
    guard is the fact being read). Otherwise it cannot: a ``#define`` may
    spell the definition, or ordinary conditionals may hold two (libultra's
    ``#if BUILD_VERSION >= VERSION_J`` in ``crc.c``) or unbalance the braces
    the reader counts (``#if 0`` blocks in ``n_env.c``).
    """
    facts = source_facts(text, symbol)
    if facts.pragmas or len(facts.definitions) == 1 or any(
            row.non_matching_state is not None for row in facts.definitions):
        return False
    if _macro_spells(text, symbol):
        return True
    return bool(_CONDITIONAL_RE.search(text)
                and re.search(rf"\b{re.escape(symbol)}\s*\(", _mask_c(text)))


def preprocessed_text(source: pathlib.Path, root: pathlib.Path, *,
                      non_matching: bool = False) -> str:
    """The configured compiler's own preprocessed view of ``source``.

    The selected build's compile command (``gmake -n``, every define and include
    in order) is rerun as ``tools/ido/cc -E``. The result
    is what the compiler parsed: conditionals resolved, macros expanded,
    quoted includes inlined, ``# LINE "FILE"`` markers left in place.
    """
    command, flags, error = _discover_compile_command(root, source, non_matching=non_matching)
    if command is None or error is not None:
        raise MetadataProofError("cannot discover the configured compile command: %s" % error)
    words = shlex.split(command)
    if "tools/ido/cc" not in words:
        raise MetadataProofError("configured compile command is not IDO")
    result = subprocess.run(
        ["tools/ido/cc", "-E", *flags, _relative(source, root)], cwd=root, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120, check=False)
    if result.returncode:
        raise MetadataProofError("configured preprocessor failed: %s" % (
            result.stderr.strip().splitlines() or ["exit %d" % result.returncode])[-1])
    return result.stdout


def source_view(source: pathlib.Path, symbol: str,
                root: pathlib.Path | None = None, *,
                non_matching: bool = False) -> tuple[str, str]:
    """The text ``source_facts`` should read for ``symbol``, and which view it is.

    The source as written is the view unless ``needs_preprocessed_view``: a
    definition spelled through the preprocessor is not visible there
    (``overlay101UpdateEntry8B.c`` is
    ``#define overlay101UpdateEntry8 overlay101UpdateEntry8B`` followed by
    ``#include "overlay101UpdateEntry8.c"``), and ordinary conditionals can
    hold two definitions or hide one. Then the view is the configured
    compiler's preprocessed output (``preprocessed_text``), so the facts are
    the ones the ordinary build compiles. Explicit ``non_matching=True``
    selects the configured candidate command for indirect guarded definitions.
    """
    root = pathlib.Path(__file__).resolve().parent.parent if root is None else root
    text = source.read_text(encoding="utf-8", errors="replace")
    # A body in an included .c (a shared .inc.c) is visible only once inlined.
    if not source_facts(text, symbol).definitions and (
            _macro_spells(text, symbol) or re.search(
                r'^\s*#\s*include\s+"[^"\n]+\.(?:c|inc)"', text, re.MULTILINE)):
        return preprocessed_text(source, root, non_matching=non_matching), "preprocessed"
    if not needs_preprocessed_view(text, symbol):
        return text, "source"
    if non_matching:
        return preprocessed_text(source, root, non_matching=True), "preprocessed"
    return preprocessed_text(source, root), "preprocessed"


def source_facts_for(source: pathlib.Path, symbol: str,
                     root: pathlib.Path | None = None) -> SourceFacts:
    """``source_facts`` over ``source_view`` (the preprocessed view when needed)."""
    return source_facts(source_view(source, symbol, root)[0], symbol)


def _all_pragmas(text: str) -> tuple[PragmaOccurrence, ...]:
    states = _line_non_matching_states(text)
    starts = _line_starts(text)
    result: list[PragmaOccurrence] = []
    for match in _PRAGMA_RE.finditer(text):
        path = match.group("path")
        line = _line_number(starts, match.start())
        result.append(
            PragmaOccurrence(
                line,
                _state_at(states, line),
                path,
                pathlib.PurePosixPath(path).stem,
            )
        )
    return tuple(result)


def classify_source_selection(
    text: str,
    *,
    candidate_symbol: str,
    target_symbol: str,
    defines: Iterable[str],
) -> tuple[str, str]:
    """Classify the bytes selected by the effective preprocessor mode."""

    defined = set(defines)
    candidate = source_facts(text, candidate_symbol)
    target = source_facts(text, target_symbol)
    definitions = candidate.definitions
    pragmas = tuple({p.path: p for p in (*candidate.pragmas, *target.pragmas)}.values())
    # Overlay sources commonly give the C body a friendly name while the
    # fallback keeps splat's auto-name.  A single opposite NON_MATCHING branch
    # in the same one-function TU is the selected function even though the two
    # labels differ. Only a guarded definition can be that branch: an
    # unconditional definition is never the alternative to a GLOBAL_ASM, so in
    # a multi-function TU another function's guarded fallback is not paired
    # with it.
    if (definitions and not pragmas and candidate_symbol != target_symbol
            and all(item.non_matching_state is True for item in definitions)):
        opposite = [
            pragma
            for pragma in _all_pragmas(text)
            if pragma.non_matching_state is False
        ]
        if len(opposite) == 1:
            pragmas = tuple(opposite)
    nm_enabled = "NON_MATCHING" in defined

    active_definitions = [
        item
        for item in definitions
        if item.non_matching_state is None or item.non_matching_state == nm_enabled
    ]
    active_pragmas = [
        item
        for item in pragmas
        if item.non_matching_state is None or item.non_matching_state == nm_enabled
    ]

    if len(active_definitions) == 1 and not active_pragmas:
        if active_definitions[0].non_matching_state is True:
            return NON_MATCHING_C, "-DNON_MATCHING selects the guarded C body"
        return ORDINARY_C, "the selected symbol has one active ordinary C definition"
    if active_pragmas and not active_definitions:
        return GLOBAL_ASM, "the effective preprocessor branch selects GLOBAL_ASM"
    if active_definitions and active_pragmas:
        return UNKNOWN, "both C and GLOBAL_ASM appear active for the selected symbol"
    if definitions:
        return UNKNOWN, "the C definition is inactive under the discovered compiler defines"
    return UNKNOWN, "no active C definition or matching GLOBAL_ASM could be tied to the symbol"


def classify_source_file(
    source: pathlib.Path, *, candidate_symbol: str, target_symbol: str,
    root: pathlib.Path, non_matching: bool, defines: Iterable[str] | None = None,
) -> tuple[str, str]:
    """Classify an indirect definition using its configured compiler mode.

    Preprocessing removes NON_MATCHING guards. Preserve candidate status only
    when the written source independently ties this target to its opposite
    fallback; an expanded definition alone cannot establish that pairing.
    """
    raw = source.read_text(encoding="utf-8", errors="replace")
    effective = tuple(defines) if defines is not None else (
        ("NON_MATCHING",) if non_matching else ())
    kind, reason = classify_source_selection(
        raw, candidate_symbol=candidate_symbol, target_symbol=target_symbol,
        defines=effective)
    if non_matching and kind != UNKNOWN:
        return kind, reason
    if non_matching:
        if source_facts(raw, candidate_symbol).definitions:
            return kind, reason
        fallbacks = [p for p in _all_pragmas(raw)
                     if p.symbol in {candidate_symbol, target_symbol}]
        if len(fallbacks) != 1 or fallbacks[0].non_matching_state is not False:
            return UNKNOWN, "indirect candidate lacks one opposite target fallback"
    text, view = source_view(source, candidate_symbol, root,
                             **({"non_matching": True} if non_matching else {}))
    selected, detail = classify_source_selection(
        text, candidate_symbol=candidate_symbol, target_symbol=target_symbol,
        defines=effective)
    if non_matching and view == "preprocessed" and selected == ORDINARY_C:
        return NON_MATCHING_C, "configured -DNON_MATCHING preprocessing selects one C definition"
    return selected, detail


def _relative(path: pathlib.Path, root: pathlib.Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def _artifact(path: pathlib.Path | None, root: pathlib.Path, kind: str) -> dict[str, object] | None:
    if path is None:
        return None
    record: dict[str, object] = {"path": _relative(path, root), "kind": kind}
    if path.is_file():
        record.update(sha256=sha256_file(path), size=path.stat().st_size)
    else:
        record.update(sha256=None, size=None)
    return record


def _find_source(root: pathlib.Path, symbols: Sequence[str]) -> pathlib.Path | None:
    matches: set[pathlib.Path] = set()
    spelled: list[pathlib.Path] = []
    for path in (root / "src").rglob("*.c"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for symbol in symbols:
            facts = source_facts(text, symbol)
            if facts.definitions or facts.pragmas:
                matches.add(path)
                break
        else:
            if any(needs_preprocessed_view(text, symbol) for symbol in symbols):
                spelled.append(path)
    if not matches:
        # Only the preprocessor spells the definition (#define + #include).
        for path in spelled:
            try:
                if any(source_facts_for(path, symbol, root).definitions for symbol in symbols):
                    matches.add(path)
            except MetadataProofError:
                continue
    return next(iter(matches)) if len(matches) == 1 else None


def _discover_compile_command(
    root: pathlib.Path,
    source: pathlib.Path,
    *,
    non_matching: bool,
) -> tuple[str | None, list[str], str | None]:
    rel_source = _relative(source, root)
    build_dir = "build_non_matching" if non_matching else "build"
    target = f"{build_dir}/{rel_source}.o"
    command = [
        "gmake",
        "--no-print-directory",
        "-n",
        "-W",
        rel_source,
    ]
    if non_matching:
        command.append("NON_MATCHING=1")
    command.append(target)
    try:
        result = subprocess.run(
            command,
            cwd=root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=120,
            check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        return None, [], str(error)
    if result.returncode != 0:
        return None, [], (result.stderr.strip() or "gmake dry-run failed")
    logical = result.stdout.replace("\\\n", " ").splitlines()
    candidates = [
        line.strip()
        for line in logical
        if rel_source in line
        and ("tools/asm-processor/build.py" in line or "tools/ido/cc" in line)
    ]
    if len(candidates) != 1:
        return None, [], f"expected one compile command, found {len(candidates)}"
    compile_command = candidates[0]
    try:
        tokens = shlex.split(compile_command)
    except ValueError as error:
        return compile_command, [], str(error)
    if "--" in tokens:
        tokens = tokens[tokens.index("--", tokens.index("--") + 1) + 1 :]
    flags: list[str] = []
    skip_next = False
    for token in tokens:
        if skip_next:
            skip_next = False
            continue
        if token == "-o":
            skip_next = True
            continue
        if token == "-c" or token == rel_source:
            continue
        flags.append(token)
    return compile_command, flags, None


def _object_symbols(path: pathlib.Path, objdump: pathlib.Path) -> tuple[set[str], str | None]:
    if not path.is_file():
        return set(), "object does not exist"
    result = subprocess.run(
        [str(objdump), "-t", str(path)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        return set(), result.stderr.strip() or "objdump symbol-table read failed"
    symbols: set[str] = set()
    for line in result.stdout.splitlines():
        fields = line.split()
        if len(fields) < 6:
            continue
        # GNU objdump's function row is ADDRESS BIND F SECTION SIZE NAME.
        # An undefined reference with the requested spelling must not tie the
        # candidate bytes to a C definition.
        if fields[-4] != "F" or fields[-3] == "*UND*":
            continue
        try:
            size = int(fields[-2], 16)
        except ValueError:
            continue
        if size:
            symbols.add(fields[-1])
    return symbols, None


def build_manifest(
    *,
    root: pathlib.Path,
    mode: str,
    source: pathlib.Path | None,
    symbol: str,
    candidate_symbol: str,
    candidate_build_dir: pathlib.Path,
    candidate_object: pathlib.Path | None,
    target_object: pathlib.Path | None,
    candidate_artifact: pathlib.Path,
    target_artifact: pathlib.Path,
    objdump: pathlib.Path,
) -> dict[str, object]:
    reasons: list[str] = []
    if source is None:
        source = _find_source(root, (candidate_symbol, symbol))
    if source is None or not source.is_file():
        classification = UNKNOWN
        classification_reason = "source path could not be resolved uniquely"
        compile_command = None
        compiler_flags: list[str] = []
        compile_error = "source unavailable"
    else:
        requested_nm = candidate_build_dir.name == "build_non_matching"
        compile_command, compiler_flags, compile_error = _discover_compile_command(
            root, source, non_matching=requested_nm
        )
        defines = {
            flag[2:].split("=", 1)[0]
            for flag in compiler_flags
            if flag.startswith("-D") and len(flag) > 2
        }
        # A failed dry-run must not silently turn the standard NON_MATCHING
        # tree into a fallback classification; its directory is part of the
        # public build contract and supplies the conservative define here.
        if requested_nm:
            defines.add("NON_MATCHING")
        try:
            classification, classification_reason = classify_source_file(
                source, candidate_symbol=candidate_symbol, target_symbol=symbol,
                root=root, non_matching=requested_nm, defines=defines)
        except MetadataProofError as error:
            classification, classification_reason = UNKNOWN, str(error)


    if candidate_object is None and source is not None:
        rel_source = _relative(source, root)
        candidate_object = candidate_build_dir / f"{rel_source}.o"
        if not candidate_object.is_absolute():
            candidate_object = root / candidate_object

    candidate_symbols: set[str] = set()
    symbol_error = None
    if candidate_object is not None:
        candidate_symbols, symbol_error = _object_symbols(candidate_object, objdump)
    if candidate_symbol not in candidate_symbols:
        reasons.append("candidate object does not define the selected candidate symbol")
        if symbol_error:
            reasons.append(symbol_error)

    if classification not in {ORDINARY_C, NON_MATCHING_C}:
        reasons.append(classification_reason)
    if source is None:
        reasons.append("candidate symbol cannot be tied to one source path")
    elif candidate_object is not None and candidate_object.is_file():
        if candidate_object.stat().st_mtime_ns < source.stat().st_mtime_ns:
            reasons.append("candidate object predates its source path")
    else:
        reasons.append("candidate object is unavailable for hashing")

    # A guarded candidate is C evidence only when the actual Make expansion
    # confirms that the defining branch was enabled.
    if classification == NON_MATCHING_C:
        if compile_command is None or "-DNON_MATCHING" not in compiler_flags:
            reasons.append("NON_MATCHING C selection lacks a discovered -DNON_MATCHING compile")

    admissible = not reasons
    source_record = None
    if source is not None:
        source_record = {
            "path": _relative(source, root),
            "sha256": sha256_file(source) if source.is_file() else None,
        }
    manifest: dict[str, object] = {
        "schema": "mickey-wb-proof-provenance-v1",
        "mode": mode,
        "symbol": symbol,
        "candidate_symbol": candidate_symbol,
        "source": source_record,
        "selection": {
            "classification": classification,
            "reason": classification_reason,
        },
        "compiler": {
            "command": compile_command,
            "flags": compiler_flags,
            "discovery_error": compile_error,
        },
        "candidate_object": _artifact(candidate_object, root, "elf_object"),
        "target_object": _artifact(
            target_object if target_object is not None else target_artifact,
            root,
            "elf_object" if target_object is not None else "rom_objdump",
        ),
        "candidate_artifact": _artifact(candidate_artifact, root, "comparison_input"),
        "target_artifact": _artifact(target_artifact, root, "comparison_input"),
        "exact_claim_allowed": admissible,
        "exact_guard": "none" if admissible else "assert_exact_false",
        "verdict": "c_evidence" if admissible else "not_c_evidence",
        "reasons": reasons,
    }
    return manifest


def _parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=pathlib.Path, default=pathlib.Path.cwd())
    parser.add_argument("--mode", choices=("asm", "rom"), required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--candidate-symbol", required=True)
    parser.add_argument("--source", type=pathlib.Path)
    parser.add_argument("--candidate-build-dir", type=pathlib.Path, required=True)
    parser.add_argument("--candidate-object", type=pathlib.Path)
    parser.add_argument("--target-object", type=pathlib.Path)
    parser.add_argument("--candidate-artifact", type=pathlib.Path, required=True)
    parser.add_argument("--target-artifact", type=pathlib.Path, required=True)
    parser.add_argument("--objdump", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    args = _parse_args(argv)
    root = args.root.resolve()

    def rooted(path: pathlib.Path | None) -> pathlib.Path | None:
        if path is None or path.is_absolute():
            return path
        return root / path

    manifest = build_manifest(
        root=root,
        mode=args.mode,
        source=rooted(args.source),
        symbol=args.symbol,
        candidate_symbol=args.candidate_symbol,
        candidate_build_dir=rooted(args.candidate_build_dir),
        candidate_object=rooted(args.candidate_object),
        target_object=rooted(args.target_object),
        candidate_artifact=rooted(args.candidate_artifact),
        target_artifact=rooted(args.target_artifact),
        objdump=rooted(args.objdump),
    )
    manifest_path = rooted(args.manifest)
    assert manifest_path is not None
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = manifest_path.with_suffix(manifest_path.suffix + ".tmp")
    temporary.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, manifest_path)

    selection = manifest["selection"]
    assert isinstance(selection, dict)
    source = manifest["source"]
    source_path = source["path"] if isinstance(source, dict) else "unresolved"
    verdict = "C-EVIDENCE" if manifest["exact_claim_allowed"] else "NOT-C-EVIDENCE"
    print(
        "proof-provenance: "
        f"{verdict} {selection['classification']} source={source_path} "
        f"symbol={args.candidate_symbol} manifest={_relative(manifest_path, root)}",
        file=sys.stderr,
    )
    if not manifest["exact_claim_allowed"]:
        print(
            "proof-provenance: exact claim blocked; "
            + "; ".join(str(reason) for reason in manifest["reasons"]),
            file=sys.stderr,
        )
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
