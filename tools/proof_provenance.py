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


def _effective_parser(root):
    """Reuse the bound vendored effective-C parser, never an unbound preload."""
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
    return c_ast, c_parser, before, lexical_identity


def literal_array_facts(source, cpp, root):
    """Closed source syntax, corroborated by the existing effective-C parser."""
    import candidate_context as lexical
    c_ast, c_parser, before, lexical_identity = _effective_parser(root)
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



_STATIC_ANCHOR_PIN = None


def static_anchor_module(root):
    """Bind existing metadata reader code on first loading, never a stale preload."""
    global _STATIC_ANCHOR_PIN
    path = pathlib.Path(root)/'tools/static_symbols.py'
    before = sha256_file(path)
    if _STATIC_ANCHOR_PIN is None:
        if 'static_symbols' in sys.modules:
            raise MetadataProofError('preloaded static anchor reader lacks a bound identity')
        import static_symbols
        if pathlib.Path(static_symbols.__file__).resolve() != path.resolve() or sha256_file(path) != before:
            raise MetadataProofError('static anchor reader changed during loading')
        _STATIC_ANCHOR_PIN = (static_symbols, before)
    module, identity = _STATIC_ANCHOR_PIN
    if pathlib.Path(module.__file__).resolve() != path.resolve() or before != identity:
        raise MetadataProofError('loaded static anchor reader changed')
    return module


def local_ro_owner_profile(command, root):
    """Explicit ordinary DATA-group selection is not source/type authority."""
    words = shlex.split(command)
    flag = '--local-ro-owner-profile'
    if flag not in words:
        return None
    if (words.count(flag) != 1 or words.index(flag)+1 >= len(words)
            or words[words.index(flag)+1] != 'pointer-array-three-s32-v1'
            or '--local-ro-provider-group' not in words
            or any(x in words for x in ('--local-ro-origin-object', '--local-ro-mixed-provider-group',
                                        '--local-data-aggregate', '--local-data-objects'))):
        raise MetadataProofError('unsupported or conflicting LOCAL owner profile')
    local_ro_provider_inputs(command, root)
    return words[words.index(flag)+1]


def pointer_scalar_facts(cpp, flags, root):
    """Closed effective declarations, independent of ELF allocation and names."""
    import candidate_context as lexical
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('pointer/scalar source requires actual o32 flags')
    astmod, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('pointer/scalar effective source is too large')
    try:
        tree = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('pointer/scalar effective source cannot be parsed') from error
    aliases = collections.defaultdict(list)
    declarations = collections.defaultdict(list)
    for node in tree.ext:
        if isinstance(node, astmod.Typedef):
            aliases[node.name].append(node.type)
        elif isinstance(node, astmod.Decl):
            declarations[node.name].append(node)
        else:
            raise MetadataProofError('unsupported pointer/scalar top-level source')
    def primitive(node, kind, seen=()):
        if (not isinstance(node, astmod.TypeDecl) or node.quals
                or getattr(node, 'align', None) or not isinstance(node.type, astmod.IdentifierType)):
            return False
        names = node.type.names
        choices = ([['unsigned','char']] if kind == 'byte' else
                   [['long'], ['signed','long'], ['long','int'], ['signed','long','int']])
        if any(sorted(names) == sorted(x) for x in choices):
            return True
        return (len(names)==1 and names[0] not in seen and len(aliases[names[0]])==1
                and primitive(aliases[names[0]][0], kind, seen+(names[0],)))
    def plain(node):
        return not (node.quals or node.funcspec or node.bitsize or getattr(node,'align',None))
    definitions = [node for nodes in declarations.values() for node in nodes if node.storage != ['extern']]
    if len(definitions)!=4 or any(len(nodes)!=1 for nodes in declarations.values()):
        raise MetadataProofError('pointer/scalar definitions are missing or ambiguous')
    array, *scalars = definitions
    t = array.type
    if (array.storage or not plain(array) or not isinstance(t, astmod.ArrayDecl)
            or t.dim_quals or not isinstance(t.dim,astmod.Constant) or t.dim.type!='int'
            or not re.fullmatch(r'[1-9][0-9]{0,8}',t.dim.value)
            or not isinstance(t.type,astmod.PtrDecl) or t.type.quals
            or not primitive(t.type.type,'byte') or not isinstance(array.init,astmod.InitList)):
        raise MetadataProofError('pointer/scalar source lacks its mutable byte-pointer array')
    count = int(t.dim.value)
    initializers = array.init.exprs
    if (not 0<count<=((1<<31)-13)//4 or len(initializers)!=count
            or any(not isinstance(x,astmod.ID) for x in initializers)):
        raise MetadataProofError('pointer array lacks its complete named-base initializer set')
    targets = [x.name for x in initializers]
    for name in targets:
        nodes = declarations.get(name,())
        if len(nodes)!=1:
            raise MetadataProofError('pointer initializer has no unique extern array')
        node = nodes[0]
        if (node.storage!=['extern'] or not plain(node) or node.init is not None
                or not isinstance(node.type,astmod.ArrayDecl) or node.type.dim is not None
                or node.type.dim_quals or not primitive(node.type.type,'byte')):
            raise MetadataProofError('pointer initializer is not a compatible mutable byte array')
    for node in scalars:
        if (node.storage or not plain(node) or not primitive(node.type,'long')
                or not isinstance(node.init,astmod.Constant) or node.init.type!='int' or node.init.value!='0'):
            raise MetadataProofError('pointer/scalar source requires three signed-long zero objects')
    if set(declarations) != {array.name, *(s.name for s in scalars), *targets}:
        raise MetadataProofError('pointer/scalar source contains an extra declaration')
    if before != literal_profile_inputs(root) or lexical.identity()!=lexical_identity:
        raise MetadataProofError('pointer/scalar parser or lexical implementation changed')
    return dict(array=array.name, count=count, targets=targets, scalars=[s.name for s in scalars])


def callback_data_facts(cpp, flags, root):
    """Closed o32 callback-record syntax; not a general C layout evaluator."""
    import candidate_context as lexical
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('callback DATA requires actual o32 flags')
    ca, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('callback CPP exceeds the source bound')
    try:
        tree = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('callback CPP cannot be parsed') from error
    aliases, declarations = collections.defaultdict(list), collections.defaultdict(list)
    for node in tree.ext:
        if isinstance(node, ca.Typedef):
            aliases[node.name].append(node.type)
        elif isinstance(node, ca.Decl):
            declarations[node.name].append(node)
        else:
            raise MetadataProofError('callback producer contains unsupported top-level source')
    def resolve(node, seen=()):
        if isinstance(node, ca.TypeDecl) and isinstance(node.type, ca.IdentifierType):
            names = node.type.names
            if len(names) == 1 and names[0] in aliases:
                if node.quals or names[0] in seen or len(aliases[names[0]]) != 1:
                    raise MetadataProofError('callback typedef is qualified or ambiguous')
                return resolve(aliases[names[0]][0], seen+(names[0],))
        return node
    def primitive(node, kind):
        node = resolve(node)
        if (not isinstance(node, ca.TypeDecl) or node.quals
                or getattr(node, 'align', None) or not isinstance(node.type, ca.IdentifierType)):
            return False
        words = sorted(node.type.names)
        choices = {'long': [['long'], ['long','signed'], ['int','long'], ['int','long','signed']],
                   'short': [['short','unsigned'], ['int','short','unsigned']],
                   'void': [['void']]}
        return words in choices[kind]
    def plain(node):
        return not (node.quals or node.funcspec or node.bitsize or getattr(node,'align',None))
    def signature(node):
        node = resolve(node)
        if (not isinstance(node, ca.FuncDecl) or node.args is None or len(node.args.params)!=1
                or not isinstance(node.args.params[0], ca.Typename)
                or not primitive(node.args.params[0].type, 'void')):
            raise MetadataProofError('callback needs one explicit void parameter list')
        if primitive(node.type, 'long'):
            return 's32(void)'
        if primitive(node.type, 'void'):
            return 'void(void)'
        raise MetadataProofError('unsupported callback return type')
    definitions = [node for rows in declarations.values() for node in rows if node.storage != ['extern']]
    if len(definitions)!=1 or any(len(rows)!=1 for rows in declarations.values()):
        raise MetadataProofError('callback DATA requires one unique complete array definition')
    owner = definitions[0]; array = owner.type
    if (owner.storage or not plain(owner) or not isinstance(array, ca.ArrayDecl)
            or array.dim_quals or not isinstance(array.dim, ca.Constant)
            or array.dim.type!='int' or not re.fullmatch(r'[1-9][0-9]{0,8}',array.dim.value)
            or not isinstance(owner.init,ca.InitList)):
        raise MetadataProofError('callback DATA needs a positive literal-bound record array')
    record = resolve(array.type)
    if not isinstance(record,ca.TypeDecl) or record.quals or not isinstance(record.type,ca.Struct):
        raise MetadataProofError('callback array element is not its unique complete struct')
    fields=record.type.decls
    if fields is None or len(fields)!=3 or len({field.name for field in fields})!=3:
        raise MetadataProofError('callback record has unsupported fields')
    kinds=[]
    for field in fields[:2]:
        if not plain(field) or field.storage or not isinstance(field.type,ca.PtrDecl) or field.type.quals:
            raise MetadataProofError('callback field is not an unqualified function pointer')
        kinds.append(signature(field.type.type))
    if kinds!=['s32(void)','void(void)'] or not plain(fields[2]) or not primitive(fields[2].type,'short'):
        raise MetadataProofError('callback record must contain test/update/unsigned-short mask')
    count=int(array.dim.value)
    if not 0<count<=(2**31-1)//12 or len(owner.init.exprs)!=count:
        raise MetadataProofError('callback array has incomplete initializer rows')
    def integer(node):
        if isinstance(node,ca.Constant) and re.fullmatch(r'(?:0[xX][0-9a-fA-F]+|0|[1-9][0-9]*)[uU]?',node.value):
            value=int(node.value.rstrip('uU'),0)
        elif isinstance(node,ca.BinaryOp) and node.op in ('|','<<'):
            left,right=integer(node.left),integer(node.right)
            if node.op=='<<' and not 0<=right<=15:
                raise MetadataProofError('callback mask shift exceeds its scalar type')
            value=left|right if node.op=='|' else left<<right
        else:
            raise MetadataProofError('callback mask is not a closed unsigned integer constant')
        if not 0<=value<=65535:
            raise MetadataProofError('callback mask exceeds its actual unsigned-short type')
        return value
    def null(node):
        if isinstance(node,ca.Constant):
            return node.type=='int' and node.value=='0'
        return (isinstance(node,ca.Cast) and isinstance(node.to_type,ca.Typename)
                and isinstance(node.to_type.type,ca.PtrDecl) and not node.to_type.type.quals
                and primitive(node.to_type.type.type,'void')
                and isinstance(node.expr,ca.Constant) and node.expr.type=='int' and node.expr.value=='0')
    callbacks=[]; masks=[]
    for index,row in enumerate(owner.init.exprs):
        if not isinstance(row,ca.InitList) or len(row.exprs)!=3:
            raise MetadataProofError('callback row has missing or extra initialized fields')
        for column,node in enumerate(row.exprs[:2]):
            if null(node):continue
            if not isinstance(node,ca.ID):
                raise MetadataProofError('callback pointer is not a direct named function base')
            choices=declarations.get(node.name,())
            if len(choices)!=1 or choices[0].storage!=['extern'] or not plain(choices[0]) or choices[0].init is not None:
                raise MetadataProofError('callback target lacks one compatible extern prototype')
            if signature(choices[0].type)!=kinds[column]:
                raise MetadataProofError('callback initializer function signature disagrees')
            callbacks.append(dict(offset=index*12+column*4,name=node.name,signature=kinds[column]))
        masks.append(integer(row.exprs[2]))
    if set(declarations)!={owner.name,*(item['name'] for item in callbacks)}:
        raise MetadataProofError('callback source has extra declarations')
    if before!=literal_profile_inputs(root) or lexical.identity()!=lexical_identity:
        raise MetadataProofError('callback loaded parser or lexical code changed')
    return dict(owner=owner.name,count=count,record_stride=12,logical_extent=count*12,
                declared_field_bytes=count*10,internal_padding_bytes=count*2,callbacks=callbacks,masks=masks)


CALLBACK_PROFILE = 'typed-callback-records-v1'
_CALLBACK_SEAL = object()


def callback_selection(command, root):
    """Explicit source/input graph, never numeric function bindings."""
    words = shlex.split(command)
    flags = ('--callback-data-profile', '--callback-source', '--callback-text-inputs')
    if not any(flag in words for flag in flags):
        return None
    if (any(words.count(flag) != 1 for flag in flags)
            or any(flag in words for flag in ('--local-ro-origin-object', '--local-ro-provider-group',
                '--local-ro-mixed-provider-group', '--local-data-aggregate', '--local-data-objects',
                '--local-ro-owner-profile'))):
        raise MetadataProofError('conflicting callback DATA selection')
    def argument(flag):
        index = words.index(flag)+1
        if index >= len(words):
            raise MetadataProofError('missing callback DATA argument')
        return words[index]
    source = argument('--callback-source')
    inputs = words[words.index('--callback-text-inputs')+1:]
    if (argument('--callback-data-profile') != CALLBACK_PROFILE
            or not re.fullmatch(r'(?:src|build)/[A-Za-z_0-9/.-]+\.c', source)
            or '..' in pathlib.PurePosixPath(source).parts
            or not inputs or len(set(inputs)) != len(inputs)
            or any(not re.fullmatch(r'build/src/overlays/o[0-9]{3}/[A-Za-z_0-9]+\.c\.o', n) for n in inputs)
            or len({pathlib.PurePosixPath(n).parent for n in inputs}) != 1):
        raise MetadataProofError('invalid callback source or ordered TEXT input graph')
    return source, tuple(inputs)


def callback_function_facts(cpp, flags, root, wanted):
    """Actual effective definitions, not the producer's prototypes."""
    import candidate_context as lexical
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('callback FUNCTION requires actual o32 flags')
    ca, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('callback FUNCTION CPP exceeds bound')
    try:
        tree = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('callback FUNCTION CPP cannot be parsed') from error
    aliases = collections.defaultdict(list)
    for node in tree.ext:
        if isinstance(node, ca.Typedef):
            aliases[node.name].append(node.type)
    def primitive(node, seen=()):
        if (not isinstance(node, ca.TypeDecl) or node.quals or getattr(node, 'align', None)
                or not isinstance(node.type, ca.IdentifierType)):
            raise MetadataProofError('unsupported callback FUNCTION type')
        words = sorted(node.type.names)
        if words == ['void']:
            return 'void'
        if words in (['long'], ['long', 'signed'], ['int', 'long'], ['int', 'long', 'signed']):
            return 's32'
        if len(words) == 1 and words[0] not in seen and len(aliases[words[0]]) == 1:
            return primitive(aliases[words[0]][0], seen+(words[0],))
        raise MetadataProofError('unsupported callback FUNCTION primitive')
    result = {}
    for name in wanted:
        rows = [node for node in tree.ext if isinstance(node, ca.FuncDef) and node.decl.name == name]
        if len(rows) != 1:
            raise MetadataProofError('callback lacks one actual source function: '+name)
        decl = rows[0].decl
        function = decl.type
        if (decl.storage or decl.quals or decl.funcspec or decl.bitsize or getattr(decl, 'align', None)
                or not isinstance(function, ca.FuncDecl) or function.args is None
                or len(function.args.params) != 1
                or not isinstance(function.args.params[0], ca.Typename)
                or primitive(function.args.params[0].type) != 'void' or rows[0].param_decls):
            raise MetadataProofError('callback function has unsupported linkage/parameters: '+name)
        result[name] = primitive(function.type)+'(void)'
    if before != literal_profile_inputs(root) or lexical.identity() != lexical_identity:
        raise MetadataProofError('callback FUNCTION loaded parser changed')
    return result


def _callback_recipe(root, source, deadline):
    import permute_batch as batch
    relative = source.relative_to(root).as_posix()
    target = 'build/'+relative+'.o'
    output = batch.bounded_capture(['gmake', '--no-print-directory', '-n', '-W', relative, target],
                                   deadline, check=True).stdout.replace('\\\n', ' ').splitlines()
    rows = [line.strip() for line in output if target in line and 'tools/ido/cc' in line]
    if len(rows) != 1:
        raise MetadataProofError('callback source lacks one actual compiler vector')
    command = rows[0]
    return command, batch.compiler_arguments(command, relative, target), tuple(output)


def _callback_frontend(root, source, directory, deadline):
    """Same configured wrapper frontend/include ordering, held before parsing."""
    import permute_batch as batch
    import sweep_receipts as receipts
    root, source = pathlib.Path(root).resolve(), pathlib.Path(source).resolve()
    command, flags, expanded = _callback_recipe(root, source, deadline)
    held = {root/name: digest for name, digest in literal_profile_inputs(root).items()}
    held.update({root/name: digest for name, digest in literal_frontend_inputs(root, command).items()})
    held.update({path: sha256_file(path) for path in (source, root/'Makefile', *sorted((root/'mk').rglob('*.mk')),
                    root/'tools/ido/cc')})
    include_tree = receipts.tree_digest(root/'include')
    source_tree = receipts.tree_digest(source.parent)
    tool_identity = batch.checked_tool_identity()
    if _all_pragmas(source.read_text()):
        raise MetadataProofError('callback source contains an assembly fallback')
    words = shlex.split(command)
    wrapped = len(words)>1 and words[1]=='tools/asm-processor/build.py'
    actual_flags = list(flags)
    if wrapped:
        options = [x for x in flags if x in {'-g3','-g','-O0','-O1','-O2','-framepointer','-KPIC'}]
        if '-mips2' not in flags:
            options.append('-mips1')
        transformed = batch.bounded_capture(['.venv/bin/python','tools/asm-processor/asm_processor.py',
            *options,source.relative_to(root).as_posix()], deadline, check=True).stdout.encode()
        actual_flags += ['-I', str(source.parent)]
    else:
        raise MetadataProofError('callback source requires its configured wrapper frontend')
    frontend = directory/'compiler-input.c'
    frontend.write_bytes(transformed)
    held[frontend] = hashlib.sha256(transformed).hexdigest()
    dependencies = batch.source_dependencies(frontend, tuple(actual_flags), deadline)
    if any(name.startswith('missing:') for name in dependencies):
        raise MetadataProofError('callback source dependency is missing')
    held.update({root/name: digest for name, digest in dependencies.items()})
    import candidate_context as lexical
    lexical.identity()
    for path in (source, *(root/name for name in dependencies)):
        text = lexical._strip_comments(path.read_text()).replace('\\\n','')
        if re.search(r'^\s*#\s*(?:define|undef)\s+sizeof\b',text,re.M):
            raise MetadataProofError('callback layout operator is redefined')
    if any(re.match(r'-[DU]sizeof(?:=|$|\()',flag) for flag in actual_flags):
        raise MetadataProofError('callback compiler redefines the layout operator')
    held.update({root/name: sha256_file(root/name) for name in
                 ('tools/link_overlay_module.py','tools/runlink_pack.py')})
    cpp_command = ['tools/ido/cc','-E',*actual_flags,str(frontend)]
    cpp = batch.bounded_capture(cpp_command, deadline, check=True).stdout.encode()
    cpp_path = directory/'actual-cpp.c'
    cpp_path.write_bytes(cpp)
    held[cpp_path] = hashlib.sha256(cpp).hexdigest()
    def current(*, operation_deadline=None):
        if operation_deadline is None:
            operation_deadline = time.monotonic()+120
        if (sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF
                or batch.checked_tool_identity() != tool_identity
                or batch.source_dependencies(frontend, tuple(actual_flags), operation_deadline) != dependencies
                or _callback_recipe(root, source, operation_deadline) != (command, flags, expanded)
                or receipts.tree_digest(root/'include') != include_tree
                or receipts.tree_digest(source.parent) != source_tree
                or any(sha256_file(path) != digest for path,digest in held.items())):
            raise MetadataProofError('callback source/frontend/tool context changed')
        return True
    current(operation_deadline=deadline)
    return cpp, tuple(actual_flags), frontend, held, current, command


@dataclasses.dataclass(frozen=True)
class CallbackCapture:
    raw: object = dataclasses.field(repr=False)
    digest: str
    surface: tuple
    facts_json: str
    directory: pathlib.Path
    held: tuple
    context: object = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def current(self, raw, *, operation_deadline=None):
        self.context(**({"operation_deadline":operation_deadline} if operation_deadline is not None else {}))
        if (self.seal is not _CALLBACK_SEAL or raw is not self.raw
                or hashlib.sha256(raw.data).hexdigest() != self.digest
                or _static_parsed_surface(raw) != self.surface
                or any(sha256_file(path) != digest for path,digest in self.held)):
            raise MetadataProofError('callback compiler/layout observation changed')
        return True


def own_ro_prefix_facts(cpp, flags, root):
    """Complete literal signed-o32 arrays in a mixed C input, not an ELF extent guess."""
    import candidate_context as lexical
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('own RO prefix requires actual o32 flags')
    ca, parser, before, identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('own RO CPP exceeds its bound')
    try:
        tree = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('own RO CPP cannot be parsed') from error
    aliases = collections.defaultdict(list)
    for node in tree.ext:
        if isinstance(node, ca.Typedef):
            aliases[node.name].append(node.type)
    def element(node, seen=(), quals=()):
        if (not isinstance(node, ca.TypeDecl) or getattr(node, 'align', None)
                or not isinstance(node.type, ca.IdentifierType)):
            raise MetadataProofError('own RO element is not a scalar typedef')
        quals = (*quals, *node.quals)
        names = node.type.names
        if len(names) == 1 and names[0] in aliases:
            name = names[0]
            if name in seen or len(aliases[name]) != 1:
                raise MetadataProofError('own RO typedef is ambiguous')
            return element(aliases[name][0], (*seen, name), quals)
        if sorted(names) not in (['long'], ['long','signed'], ['int','long'], ['int','long','signed']) or quals != ('const',):
            raise MetadataProofError('own RO element is not const signed o32 long')
    def integer(node):
        sign = 1
        if isinstance(node, ca.UnaryOp) and node.op == '-':
            sign, node = -1, node.expr
        if (not isinstance(node, ca.Constant) or node.type != 'int'
                or not re.fullmatch(r'(?:0[xX][0-9a-fA-F]+|0|[1-9][0-9]*)', node.value)):
            raise MetadataProofError('own RO initializer is not a signed integer literal')
        value = sign * int(node.value, 0)
        if not -(1 << 31) <= value < 1 << 31:
            raise MetadataProofError('own RO initializer exceeds s32')
        return value
    rows = []
    for node in tree.ext:
        if not isinstance(node, ca.Decl) or node.init is None:
            continue
        array = node.type
        if (node.storage or node.funcspec or node.bitsize or getattr(node, 'align', None)
                or getattr(node, 'gcc_attributes', None) or getattr(node, 'asmlabel', None)
                or node.quals != ['const'] or not isinstance(array, ca.ArrayDecl)
                or array.dim_quals or not isinstance(node.init, ca.InitList)):
            raise MetadataProofError('own RO has an unsupported initialized definition')
        element(array.type)
        values = [integer(x) for x in node.init.exprs]
        if not values or len(values) > lexical.MAX_SOURCE_BYTES // 4:
            raise MetadataProofError('own RO array extent is empty or excessive')
        if array.dim is not None and integer(array.dim) != len(values):
            raise MetadataProofError('own RO array is not completely initialized')
        if len([x for x in tree.ext if isinstance(x, ca.Decl) and x.name == node.name]) != 1:
            raise MetadataProofError('own RO declaration is ambiguous')
        rows.append(dict(name=node.name, count=len(values), values=values))
    if not rows or len({x['name'] for x in rows}) != len(rows):
        raise MetadataProofError('own RO lacks unique initialized arrays')
    if before != literal_profile_inputs(root) or lexical.identity() != identity:
        raise MetadataProofError('own RO loaded parser changed')
    return rows


@dataclasses.dataclass(frozen=True)
class OwnRoCapture:
    original: object = dataclasses.field(repr=False)
    surface: tuple
    facts_json: str
    held: tuple
    context: object = dataclasses.field(repr=False)
    directory: pathlib.Path
    seal: object = dataclasses.field(repr=False)

    def current(self):
        self.context()
        if (self.seal is not _OWN_RO_SEAL or _static_parsed_surface(self.original) != self.surface
                or any(sha256_file(path) != digest for path, digest in self.held)):
            raise MetadataProofError('own RO source/compiler observation changed')
        return True


_OWN_RO_SEAL = object()


def capture_own_ro_prefix(source, root):
    """Fresh unchanged compiler observation plus zero-storage compiler size checks."""
    import permute_batch as batch
    import runlink_pack as rp
    import source_module_link_proof as proof
    root, source = pathlib.Path(root).resolve(), pathlib.Path(source).resolve()
    parent = root/'build/own-ro-source-proof'; parent.mkdir(parents=True, exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix='capture-', dir=parent))
    deadline = time.monotonic()+120
    cpp, flags, frontend, held, context, recipe = _callback_frontend(root, source, directory, deadline)
    facts = own_ro_prefix_facts(cpp, flags, root)
    original_path = directory/'original.o'
    command = ['tools/ido/cc', *flags, '-o', str(original_path), str(frontend)]
    result = batch.bounded_capture(command, deadline, check=True)
    (directory/'compile.log').write_text(result.stdout)
    original = rp.Elf(original_path)
    held[original_path] = hashlib.sha256(original.data).hexdigest()
    checks = ['sizeof(long)==4']
    for row in facts:
        checks += [f'sizeof({row["name"]})=={row["count"]*4}', f'sizeof({row["name"]}[0])==4']
    tag = '__mickey_own_ro_'+uuid.uuid4().hex
    assertions = ''.join(f'typedef struct {{ unsigned check : (({x})?1:-1); }} {tag}{i};\n' for i,x in enumerate(checks))
    checked_source, checked_path = directory/'layout-input.c', directory/'layout.o'
    checked_source.write_bytes(frontend.read_bytes()+b'\n'+assertions.encode())
    result = batch.bounded_capture(['tools/ido/cc',*flags,'-o',str(checked_path),str(checked_source)],deadline,check=True)
    (directory/'layout.log').write_text(result.stdout)
    checked = rp.Elf(checked_path)
    if (proof.allocated(original) != proof.allocated(checked) or proof.symbols(original) != proof.symbols(checked)
            or [(s.index,s.shndx) for s in original.symbols] != [(s.index,s.shndx) for s in checked.symbols]
            or [(s,r.offset,r.type,r.sym.index,a) for s in ('.text','.data','.rodata') for r,a in zip(original.rels(s),rp.in_place_addends(original,s,original.rels(s)))]
               != [(s,r.offset,r.type,r.sym.index,a) for s in ('.text','.data','.rodata') for r,a in zip(checked.rels(s),rp.in_place_addends(checked,s,checked.rels(s)))]):
        raise MetadataProofError('own RO layout check changed original compiler surface')
    held.update({checked_source:sha256_file(checked_source),checked_path:hashlib.sha256(checked.data).hexdigest()})
    static_tool = root/'tools/static_symbols.py'
    held[static_tool] = sha256_file(static_tool)
    static_path = directory/'raw.o'
    static_path.write_bytes(original.data)
    result = batch.bounded_capture(['.venv/bin/python',str(static_tool),str(static_path)],deadline,check=True)
    (directory/'static.log').write_text(result.stdout)
    held[static_path] = sha256_file(static_path)
    context(operation_deadline=deadline)
    result = OwnRoCapture(original,_static_parsed_surface(original),json.dumps(facts,sort_keys=True),tuple(held.items()),context,directory,_OWN_RO_SEAL)
    result.current()
    (directory/'source-proof.json').write_text(json.dumps(dict(facts=facts,recipe=recipe,command=command,checks=checks),indent=2)+'\n')
    return result


def own_ro_prefix_sources(root, dump=None):
    """Exact configured source selection; no caller-supplied numeric bindings."""
    import postprocess_audit as audit
    root=pathlib.Path(root)
    dump=audit.run_make_database() if dump is None else dump
    selected={}
    for target,command in audit.postprocess_commands(dump).items():
        if '--own-ro-prefix-source' not in command:
            continue
        for a,b in {'$(HOST_PYTHON)':'python3','$(TOOLS_DIR)':'tools','$(BUILD_DIR)':'build','$@':target}.items():
            command=command.replace(a,b)
        words=shlex.split(command)
        if (len(words)!=7 or not pathlib.Path(words[0]).name.startswith('python')
                or words[1:4]!=['tools/link_overlay_module.py',target,'--own-ro-prefix-source']
                or words[5]!='--directory' or not re.fullmatch(r'build/[A-Za-z0-9_./-]+',words[6])
                or '..' in pathlib.PurePosixPath(words[6]).parts
                or not re.fullmatch(r'src/[A-Za-z0-9_/]+\.c',words[4])
                or target!='build/'+words[4]+'.o'):
            raise MetadataProofError('own RO selection is not one exact source normalization')
        selected[target]=words[4]
    return selected


def prove_own_ro_definition_join(root, source, linked_path, map_path):
    """Fresh same-input owner proof, independent of ROM operands or export rows."""
    import link_overlay_module as lm
    import runlink_pack as rp
    import source_module_link_proof as proof
    import permute_batch as batch
    root, source = pathlib.Path(root).resolve(), pathlib.Path(source).resolve()
    relative=source.relative_to(root).as_posix(); target='build/'+relative+'.o'
    graph_path=root/'config/overlays.us.json'
    loaded={graph_path:graph_path.read_bytes(),root/target:(root/target).read_bytes(),
            pathlib.Path(linked_path):pathlib.Path(linked_path).read_bytes(),
            pathlib.Path(map_path):pathlib.Path(map_path).read_bytes()}
    graph=json.loads(loaded[graph_path]); configured=rp.Elf(root/target,loaded[root/target])
    linked=rp.Elf(linked_path,loaded[pathlib.Path(linked_path)])
    mapping=rp.parse_map(map_path,loaded[pathlib.Path(map_path)])
    capture=capture_own_ro_prefix(source,root)
    original=rp.Elf(capture.directory/'raw.o')
    plan=lm.own_ro_prefix_plan(original,capture,relative,graph)
    replay_dir=capture.directory/'projection'; replay_dir.mkdir()
    replay_path=replay_dir/'ordinary.o'
    receipt=lm.project_own_ro_prefix(original.path,replay_path,capture,relative,graph,replay_dir,
        objcopy=root/'tools/binutils/mips64-elf-objcopy',rebind=root/'tools/rebind_elf_relocations.py')
    deadline=time.monotonic()+120
    _command,_flags,lines=_callback_recipe(root,source,deadline)
    own=[]; surfaces=[]
    for line in lines:
        words=shlex.split(line)
        if len(words)>2 and words[1]=='tools/link_overlay_module.py' and words[2]==target:
            own.append(words)
        if words and words[0]=='tools/binutils/mips64-elf-objcopy' and words[-1]==target:
            operations=metadata_filter_plan(line,target,root)
            if any(kind!='rename' for kind,_ in operations):
                raise MetadataProofError('own RO permits only ordinary downstream symbol renames')
            surfaces.append(words)
    if (len(own)!=1 or len(own[0])!=7 or own[0][3]!='--own-ro-prefix-source'
            or own[0][4]!=relative or own[0][5]!='--directory' or len(surfaces)>1):
        raise MetadataProofError('own RO lacks its exact selected normalization recipe')
    for words in surfaces:
        result=batch.bounded_capture([str(replay_path) if x==target else x for x in words],deadline,check=True)
        (replay_dir/'surface.log').write_text(result.stdout)
    replay=rp.Elf(replay_path)
    if _static_parsed_surface(configured)!=_static_parsed_surface(replay):
        raise MetadataProofError('own RO configured object differs from actual source normalization replay')
    own_section=f'.overlay_{plan["module"]:03d}'
    inputs=[m for m in mapping if m.file==target and m.size]
    initialized=[m for m in inputs if m.insec in ('.text','.data','.rodata')]
    expected=[('.text',rp.OVERLAY_VMA,original.size('.text')),('.rodata',rp.OVERLAY_VMA+plan['origin'],plan['extent'])]
    if ([(m.insec,m.addr,m.size) for m in initialized]!=expected
            or any(m.outsec!=own_section for m in initialized)):
        raise MetadataProofError('own RO actual map disagrees with complete source prefix')
    body=linked.section(own_section)
    if not body or body['addr']!=rp.OVERLAY_VMA or body['type']!=1 or not body['flags']&2:
        raise MetadataProofError('own RO linked allocation is missing')
    image=linked.bytes_of(own_section)
    if image[plan['origin']:plan['origin']+plan['extent']]!=original.bytes_of('.rodata'):
        raise MetadataProofError('own RO linked source payload differs')
    identities={}
    for index,name,value,size in plan['owners']:
        definitions=[s for s in linked.symbols if s.name==name and s.shndx!=rp.SHN_UNDEF]
        if (len(definitions)!=1 or (definitions[0].bind,definitions[0].type,definitions[0].section,
                definitions[0].value,definitions[0].size)!=(rp.STB_GLOBAL,rp.STT_OBJECT,own_section,
                    rp.OVERLAY_VMA+plan['origin']+value,size)):
            raise MetadataProofError('own RO lacks its unique allocated GLOBAL definition')
        identities[name]=(plan['module'],plan['origin']+value)
        identities[f'__runlink_own_ro_{index}']=identities[name]
    pending={}; expected_sites={(x[1],x[2]):x for x in plan['sites']}
    for rel in original.rels('.text'):
        key=(rel.offset,rel.type)
        if key not in expected_sites:
            continue
        _,offset,kind,index,name,addend=expected_sites[key]
        immediate=int.from_bytes(image[offset:offset+4],'big')&65535
        if kind==rp.R_MIPS_HI16:
            pending.setdefault(index,[]).append((immediate,addend))
        else:
            high=pending.pop(index,[])
            if not high or any(((h<<16)+(immediate-65536 if immediate&32768 else immediate))&0xffffffff
                    != (rel.sym.value+a)&0xffffffff for h,a in high):
                raise MetadataProofError('own RO linked stored operand differs from its source-relative binding')
    if pending:
        raise MetadataProofError('own RO linked operand pair is incomplete')
    # Keep primitive identities distinct from the live observations they bind.
    expected_identities=tuple(sorted(identities.items())); expected_plan=json.dumps(plan,sort_keys=True)
    observed=[(obj,_static_parsed_surface(obj)) for obj in (original,configured,replay,linked)]
    file_holds={path:hashlib.sha256(data).hexdigest() for path,data in loaded.items()}
    file_holds.update({p:hashlib.sha256(p.read_bytes()).hexdigest() for p in replay_dir.iterdir() if p.is_file()})
    # Bind the parsed replay observation, not a later observation of its pathname.
    file_holds[replay_path]=hashlib.sha256(replay.data).hexdigest()
    file_holds[replay_dir/'bindings.o']=receipt['configured_sha256']
    expected_mapping=tuple(tuple(vars(m).items()) for m in mapping)
    stable_held={path:hashlib.sha256(data).hexdigest() for path,data in loaded.items()}
    result={}
    members=dict(identities=identities,plan=plan,raw=original,configured=configured,linked=linked,
                 mapping=mapping,capture=capture,receipt=receipt,held=file_holds,stable_held=stable_held)
    expected_holds=(tuple(sorted(file_holds.items())),tuple(sorted(stable_held.items())))
    expected_receipt=json.dumps(receipt,sort_keys=True)
    def current():
        if (set(result)!=set(members)|{'recheck'} or result.get('recheck') is not current
                or any(result.get(key) is not value for key,value in members.items())
                or (tuple(sorted(file_holds.items())),tuple(sorted(stable_held.items())))!=expected_holds
                or json.dumps(receipt,sort_keys=True)!=expected_receipt):
            raise MetadataProofError('own RO returned witness container changed')
        capture.current()
        if (any(sha256_file(path)!=digest for path,digest in file_holds.items())
                or any(_static_parsed_surface(obj)!=surface for obj,surface in observed)
                or tuple(tuple(vars(m).items()) for m in mapping)!=expected_mapping
                or tuple(sorted(identities.items()))!=expected_identities
                or json.dumps(plan,sort_keys=True)!=expected_plan
                or json.loads(graph_path.read_bytes())!=graph):
            raise MetadataProofError('own RO loaded definition/graph/identity observation changed')
        return True
    result.update(members,recheck=current)
    current()
    return result


def capture_callback_data(raw, source, root):
    """Compiler-evaluated layout; original storage/symbols remain unchanged."""
    import permute_batch as batch
    import runlink_pack as rp
    import source_module_link_proof as proof
    root = pathlib.Path(root).resolve()
    deadline = time.monotonic()+120
    parent = root/'build/callback-source-proof'
    parent.mkdir(parents=True, exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix='capture-', dir=parent))
    digest, surface = hashlib.sha256(raw.data).hexdigest(), _static_parsed_surface(raw)
    cpp, flags, frontend, held, context, recipe = _callback_frontend(root, pathlib.Path(source), directory, deadline)
    facts = callback_data_facts(cpp, flags, root)
    owner = facts['owner']
    prefix = '__mickey_callback_'+uuid.uuid4().hex
    assertions = ''.join('typedef struct { unsigned check : (('+expression+')?1:-1); } '+prefix+str(index)+';\n'
        for index,expression in enumerate((f'sizeof({owner})=={facts["logical_extent"]}',
            f'sizeof({owner}[0])==12', 'sizeof(void (*)(void))==4', 'sizeof(unsigned short)==2')))
    checked_source = directory/'layout-input.c'
    checked_source.write_bytes(frontend.read_bytes()+b'\n'+assertions.encode())
    checked_output = directory/'layout-checked.o'
    command = ['tools/ido/cc',*flags,'-o',str(checked_output),str(checked_source)]
    result = batch.bounded_capture(command, deadline, check=True)
    (directory/'layout.log').write_text(result.stdout)
    checked = rp.Elf(checked_output)
    def rows(elf):
        return [(section,r.offset,r.type,r.sym.index,a) for section in ('.text','.data','.rodata')
                for r,a in zip(elf.rels(section),rp.in_place_addends(elf,section,elf.rels(section)))]
    if (proof.allocated(raw) != proof.allocated(checked) or proof.symbols(raw) != proof.symbols(checked)
            or [(s.index,s.shndx) for s in raw.symbols] != [(s.index,s.shndx) for s in checked.symbols]
            or rows(raw) != rows(checked)):
        raise MetadataProofError('callback layout compile changed original compiler output')
    held.update({checked_source: sha256_file(checked_source), checked_output: hashlib.sha256(checked.data).hexdigest()})
    context(operation_deadline=deadline)
    capture = CallbackCapture(raw,digest,surface,json.dumps(facts,sort_keys=True),directory,
                              tuple(held.items()),context,_CALLBACK_SEAL)
    capture.current(raw, operation_deadline=deadline)
    (directory/'proof.json').write_text(json.dumps({'facts':facts,'original_sha256':digest,
        'compiler_recipe':recipe,'layout_command':command,'layout_sha256':hashlib.sha256(checked.data).hexdigest(),
        'original_symbol_sizes_unchanged':True},indent=2))
    return capture


@dataclasses.dataclass(frozen=True)
class CallbackFunctionCapture:
    configured: object = dataclasses.field(repr=False)
    original: object = dataclasses.field(repr=False)
    configured_surface: tuple
    original_surface: tuple
    signatures_json: str
    held: tuple
    context: object = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def current(self, *, operation_deadline=None):
        self.context(**({"operation_deadline":operation_deadline} if operation_deadline is not None else {}))
        if (self.seal is not _CALLBACK_SEAL
                or _static_parsed_surface(self.configured) != self.configured_surface
                or _static_parsed_surface(self.original) != self.original_surface
                or any(sha256_file(path) != digest for path,digest in self.held)):
            raise MetadataProofError('callback FUNCTION loaded proof changed')
        return True


def capture_callback_functions(configured, source, root, wanted):
    """Only FUNCTION/TEXT evidence; unrelated configured RO is compatibility."""
    import permute_batch as batch
    import runlink_pack as rp
    root = pathlib.Path(root).resolve()
    deadline = time.monotonic()+120
    parent = root/'build/callback-function-proof'
    parent.mkdir(parents=True, exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix='capture-',dir=parent))
    loaded_digest = hashlib.sha256(configured.data).hexdigest()
    if sha256_file(configured.path) != loaded_digest:
        raise MetadataProofError('callback FUNCTION input changed before capture')
    cpp, flags, frontend, held, context, command = _callback_frontend(root,pathlib.Path(source),directory,deadline)
    held[configured.path] = loaded_digest
    original_path = directory/'compiler-original.o'
    words = shlex.split(command)
    if words.count('-o') != 1:
        raise MetadataProofError('callback FUNCTION compiler output is ambiguous')
    words[words.index('-o')+1] = str(original_path)
    result = batch.bounded_capture(words,deadline,check=True)
    (directory/'compile.log').write_text(result.stdout)
    original = rp.Elf(original_path)
    held[original_path] = hashlib.sha256(original.data).hexdigest()
    signatures = callback_function_facts(cpp,flags,root,wanted) if wanted else {}
    a,b = original.section('.text'),configured.section('.text')
    if (not a or not b or a['type']!=1 or b['type']!=1 or a['flags']&6!=6 or b['flags']&6!=6
            or a['addr'] or b['addr'] or not 0 <= a['size']-b['size'] < 16
            or original.bytes_of('.text')[:b['size']] != configured.bytes_of('.text')
            or any(original.bytes_of('.text')[b['size']:])
            or original.bytes_of('.reginfo') != configured.bytes_of('.reginfo')):
        raise MetadataProofError('callback original/configured TEXT differs')
    for symbol in configured.symbols:
        if symbol.section!='.text' or symbol.type!=rp.STT_FUNC or symbol.bind!=rp.STB_GLOBAL:
            continue
        choices = [s for s in original.symbols if s.name==symbol.name and s.section=='.text'
                   and s.type==rp.STT_FUNC and s.bind==rp.STB_GLOBAL]
        if (len(choices)!=1 or choices[0].value!=symbol.value or choices[0].size!=symbol.size
                or not symbol.size or symbol.value%4 or symbol.size%4
                or not 0 <= symbol.value < symbol.value+symbol.size <= b['size']):
            raise MetadataProofError('callback FUNCTION original/configured tuple differs')
    context(operation_deadline=deadline)
    proof = CallbackFunctionCapture(configured,original,_static_parsed_surface(configured),
        _static_parsed_surface(original),json.dumps(signatures,sort_keys=True),tuple(held.items()),
        context,_CALLBACK_SEAL)
    proof.current(operation_deadline=deadline)
    (directory/'proof.json').write_text(json.dumps({'signatures':signatures,
        'scope':'FUNCTION/TEXT only; configured nontext normalization not reproduced',
        'compiler_command':words,'original_sha256':held[original_path],
        'configured_sha256':loaded_digest},indent=2))
    return proof


_POINTER_SCALAR_SEAL = object()


@dataclasses.dataclass(frozen=True)
class PointerScalarCapture:
    cpp: bytes = dataclasses.field(repr=False)
    flags: tuple
    path: pathlib.Path
    compiler_digest: str
    seal: object = dataclasses.field(repr=False)

    def current(self):
        if (self.seal is not _POINTER_SCALAR_SEAL
                or hashlib.sha256(self.path.read_bytes()).digest()!=hashlib.sha256(self.cpp).digest()):
            raise MetadataProofError('pointer/scalar internal CPP observation changed')
        return True


@dataclasses.dataclass(frozen=True)
class PointerScalarWitness:
    observation: object = dataclasses.field(repr=False)
    digest: str
    surface: tuple
    facts_json: str
    capture: PointerScalarCapture = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def current(self, raw):
        self.capture.current()
        if (self.seal is not _POINTER_SCALAR_SEAL or raw is not self.observation
                or hashlib.sha256(raw.data).hexdigest()!=self.digest
                or _static_parsed_surface(raw)!=self.surface):
            raise MetadataProofError('pointer/scalar loaded metadata changed after witnessing')
        return True


def pointer_scalar_witness(raw, capture, root):
    """Only the internal captured CPP observation can authorize source roles."""
    import link_overlay_module as module
    if not isinstance(capture,PointerScalarCapture) or not capture.current():
        raise MetadataProofError('pointer/scalar source lacks an internal capture')
    if hashlib.sha256(raw.data).hexdigest()!=capture.compiler_digest:
        raise MetadataProofError('pointer/scalar compiler observation differs from source capture')
    facts = pointer_scalar_facts(capture.cpp, capture.flags, root)
    shape = module.pointer_scalar_shape(raw)
    objects = shape['objects']
    if ([o['name'] for o in objects] != [facts['array'], *facts['scalars']]
            or objects[0]['size'] != facts['count']*4
            or [r.sym.name for r in raw.rels('.data')] != facts['targets']):
        raise MetadataProofError('pointer/scalar source and original compiler objects differ')
    return PointerScalarWitness(raw, hashlib.sha256(raw.data).hexdigest(),
        _static_parsed_surface(raw), json.dumps(facts,sort_keys=True), capture, _POINTER_SCALAR_SEAL)


def static_source_facts(cpp, flags, root, names):
    """Closed effective o32 declarations; no allocation-derived sizeof."""
    import candidate_context as lexical
    if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
        raise MetadataProofError("loaded static source helper changed")
    if "-32" not in flags or any(x in flags for x in ("-64", "-n32", "-n64")):
        raise MetadataProofError("static source extent requires the actual o32 compiler flags")
    c_ast, c_parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError("static effective C exceeds source proof bound")
    try:
        ast = c_parser.CParser().parse(cpp.decode("utf-8"))
    except Exception as error:
        raise MetadataProofError("effective static C cannot be parsed") from error
    aliases = collections.defaultdict(list)
    declarations = collections.defaultdict(list)
    for node in ast.ext:
        if isinstance(node, c_ast.Typedef):
            aliases[node.name].append(node.type)
    class Declarations(c_ast.NodeVisitor):
        def visit_Pragma(self, node):
            raise MetadataProofError("static source extent does not support pragmas")
        def visit_Decl(self, node):
            if node.name in names:
                declarations[node.name].append(node)
            self.generic_visit(node)
    Declarations().visit(ast)
    def signed_long(node, seen=()):
        if (not isinstance(node, c_ast.TypeDecl) or node.quals or getattr(node, 'align', None)
                or not isinstance(node.type, c_ast.IdentifierType)):
            return False
        words = node.type.names
        if sorted(words) in (["long"], ["long", "signed"], ["int", "long"], ["int", "long", "signed"]):
            return True
        if len(words) != 1 or words[0] in seen or len(aliases[words[0]]) != 1:
            return False
        return signed_long(aliases[words[0]][0], seen + (words[0],))
    facts = []
    for name in sorted(names):
        matches = declarations[name]
        if len(matches) != 1 or matches[0] not in ast.ext:
            raise MetadataProofError("static declaration missing/duplicate/nested: " + name)
        node = matches[0]
        if node.storage != ["static"] or node.quals or getattr(node, "align", None) or node.funcspec or node.bitsize:
            raise MetadataProofError("unsupported static storage declaration: " + name)
        element, count, section = node.type, 1, ".data"
        if isinstance(element, c_ast.ArrayDecl):
            if (element.dim_quals or node.init is not None
                    or not isinstance(element.dim, c_ast.Constant) or element.dim.type != "int"
                    or not re.fullmatch(r"[1-9][0-9]{0,8}", element.dim.value)):
                raise MetadataProofError("static array requires a positive literal bound and no initializer")
            count = int(element.dim.value)
            element, section = element.type, ".bss"
        elif not (isinstance(node.init, c_ast.Constant) and node.init.type == "int" and node.init.value == "0"):
            raise MetadataProofError("static scalar requires integer-zero initialization")
        if not signed_long(element) or not 0 < count <= ((1 << 31)-1)//4:
            raise MetadataProofError("unsupported static element type or extent")
        facts.append(dict(name=name, section=section, element="signed-long-o32", count=count, size=4*count))
    if (before != literal_profile_inputs(root) or lexical.identity() != lexical_identity):
        raise MetadataProofError("loaded static source parser/helper changed")
    return facts


def static_function_correspondence(compiler, prepared, cpp, flags, root):
    """Closed FUNCTION-only preparation; removed placeholders are not targets."""
    import candidate_context as lexical
    import runlink_pack as rp
    if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
        raise MetadataProofError('loaded static FUNCTION proof changed')
    statics = static_anchor_module(root)
    try:
        replayed = statics.add_static_symbols(compiler.data)[0]
    except statics.Failure as error:
        raise MetadataProofError('static FUNCTION compiler metadata refused') from error
    if replayed != prepared.data:
        raise MetadataProofError('static FUNCTION preparation is not the exact normalizer output')
    sections = statics._sections(compiler.data)
    debug = [section for section in sections if section[10] == '.mdebug']
    if len(debug) != 1:
        raise MetadataProofError('static FUNCTION lacks unique compiler debug metadata')
    if statics.bss_anchors(compiler.data, debug[0]) or statics.data_anchors(compiler.data, debug[0]):
        raise MetadataProofError('static storage recovery is not a FUNCTION correspondence')
    procedures = statics.procedures(compiler.data, debug[0])
    wanted = {name: row for name, row in procedures.items() if row[2]}
    if not wanted or '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('static FUNCTION requires compiler procedures and actual o32 flags')
    ca, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('static FUNCTION CPP exceeds bound')
    try:
        tree = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('static FUNCTION effective C cannot be parsed') from error
    def shape(node):
        if node is None:
            return None
        attrs = tuple((key, tuple(value) if isinstance(value, list) else value)
            for key in node.attr_names
            if not (isinstance(node, (ca.Decl, ca.TypeDecl)) and key in ('name', 'declname'))
            for value in (getattr(node, key),))
        return (type(node).__name__, attrs, tuple((key, shape(child)) for key, child in node.children()))
    class NoPragmas(ca.NodeVisitor):
        def visit_Pragma(self, node):
            raise MetadataProofError('static FUNCTION pragmas are unsupported')
    NoPragmas().visit(tree)
    definitions = {}
    for name in wanted:
        rows = [node for node in tree.ext if isinstance(node, ca.FuncDef) and node.decl.name == name]
        if len(rows) != 1:
            raise MetadataProofError('static FUNCTION lacks one effective definition: '+name)
        node = rows[0]; decl = node.decl; function = decl.type
        if (decl.storage != ['static'] or decl.quals or decl.funcspec or decl.bitsize
                or getattr(decl, 'align', None) or node.param_decls
                or not isinstance(function, ca.FuncDecl) or function.args is None
                or any(not isinstance(arg, (ca.Decl, ca.Typename)) for arg in function.args.params)):
            raise MetadataProofError('unsupported static FUNCTION declaration: '+name)
        prototypes = [row for row in tree.ext if isinstance(row, ca.Decl) and row.name == name]
        if len(prototypes) > 1 or any(shape(row) != shape(decl) for row in prototypes):
            raise MetadataProofError('static FUNCTION prototype differs: '+name)
        definitions[name] = hashlib.sha256(repr(shape(decl)).encode()).hexdigest()
    debug_rows = [row for rows in statics._debug_files(compiler.data, debug[0]) for row in rows]
    for name, (start, size, _) in wanted.items():
        if (sum(row == (name, start, statics.ST_STATIC_PROC, statics.SC_TEXT) for row in debug_rows) != 1
                or sum(row[0] == name and row[2] == statics.ST_END for row in debug_rows) != 1):
            raise MetadataProofError('ambiguous static FUNCTION compiler records')
    for symbol in compiler.symbols:
        if symbol.bind == 1 and symbol.type == 2 and symbol.section == '.text':
            if procedures.get(symbol.name) != (symbol.value, symbol.size, False):
                raise MetadataProofError('GLOBAL FUNCTION lacks matching compiler debug extent')
    allocated = lambda image: tuple((row['name'], tuple(row[k] for k in
        ('index','type','flags','addr','size','align','link','info')), image.bytes_of(row['name']))
        for row in image.sections if row['flags'] & 2)
    if allocated(compiler) != allocated(prepared):
        raise MetadataProofError('static FUNCTION changed allocated observation')
    def symbol_rows(image):
        tables = [row for row in statics._sections(image.data) if row[1] == statics.SHT_SYMTAB]
        if len(tables) != 1:
            raise MetadataProofError('static FUNCTION needs one symbol table')
        return statics._symbols(image.data, tables[0])
    original_rows, prepared_rows = symbol_rows(compiler), symbol_rows(prepared)
    for image, rows in ((compiler, original_rows), (prepared, prepared_rows)):
        table = next(section for section in image.sections if section['type'] == 2)
        strings = image.sections[table['link']]['offset']
        if len(rows) != len(image.symbols) or any(symbol.index != index
                or (symbol.name,symbol.value,symbol.size,symbol.bind,symbol.type,symbol.shndx,symbol.section)
                != (statics._cstring(image.data,strings+row[0]),row[1],row[2],row[3]>>4,row[3]&15,row[5],image.secname(row[5]))
                for index,(symbol,row) in enumerate(zip(image.symbols,rows))):
            raise MetadataProofError('static FUNCTION parsed symbol observation differs from loaded bytes')
    key = lambda symbol, rows: (tuple((k,v) for k,v in vars(symbol).items() if k != 'index'),
                               tuple(rows[symbol.index]))
    referenced = {rel.sym.index for section in compiler.sections for rel in compiler.rels(section['name'])}
    pairs, removed, used = [], [], set()
    for symbol in compiler.symbols:
        matches = [row for row in prepared.symbols if key(row, prepared_rows) == key(symbol, original_rows)]
        if len(matches) == 1 and matches[0].index not in used:
            pairs.append((symbol.index, matches[0].index)); used.add(matches[0].index)
        elif (not matches and symbol.name in wanted and symbol.shndx == 0
                and symbol.bind == 1 and symbol.type == 0 and original_rows[symbol.index][4] == 0 and symbol.value == 0
                and symbol.size == wanted[symbol.name][1] and symbol.index not in referenced):
            removed.append((symbol.index, symbol.name, symbol.size))
        else:
            raise MetadataProofError('static FUNCTION original row has no unique correspondence')
    added = []
    for symbol in prepared.symbols:
        if symbol.index in used:
            continue
        if (symbol.name not in wanted or symbol.bind != 0 or symbol.type != 2 or prepared_rows[symbol.index][4] != 0
                or symbol.section != '.text' or (symbol.value, symbol.size, True) != wanted[symbol.name]):
            raise MetadataProofError('unproved added static FUNCTION row')
        added.append((symbol.index, symbol.name, symbol.value, symbol.size, definitions[symbol.name]))
    if len(added) != len(wanted) or len({row[1] for row in added}) != len(wanted):
        raise MetadataProofError('static FUNCTION additions are incomplete or ambiguous')
    mapping = dict(pairs)
    for section in compiler.sections:
        name = section['name']; old = compiler.rels(name); new = prepared.rels(name)
        if (len(old) != len(new) or any((a.offset,a.type,mapping.get(a.sym.index))
                != (b.offset,b.type,b.sym.index) for a,b in zip(old,new))
                or rp.in_place_addends(compiler,name,old) != rp.in_place_addends(prepared,name,new)):
            raise MetadataProofError('static FUNCTION changed ordered original relocation identity')
    if before != literal_profile_inputs(root) or lexical.identity() != lexical_identity:
        raise MetadataProofError('static FUNCTION loaded parser changed')
    static_anchor_module(root)
    return tuple(pairs), tuple(removed), tuple(added)


def _static_parsed_surface(elf):
    # Keep the compact audit shape without rendering whole ELF payloads as text.
    # Ordered metadata and length-framed bytes bind the same parsed observation.
    import source_module_link_proof as proof
    allocated = proof.allocated(elf)
    surface = (tuple((name, *row[:-1]) for name, row in allocated.items()),
               proof.symbols(elf),
               [(s["name"], r.offset, r.type, r.sym.name)
                for s in elf.sections for r in elf.rels(s["name"])])
    metadata = repr(surface).encode()
    digest = hashlib.sha256(b"mickey-static-parsed-surface-v1\0")
    digest.update(struct.pack(">Q", len(metadata)))
    digest.update(metadata)
    for row in allocated.values():
        payload = row[-1]
        digest.update(struct.pack(">Q", len(payload)))
        digest.update(payload)
    return digest.hexdigest(), tuple((s.index, s.shndx) for s in elf.symbols)


_STATIC_SOURCE_SEAL = object()


@dataclasses.dataclass(frozen=True)
class StaticSourceExtent:
    name: str
    section: str
    offset: int
    semantic_size: int
    count: int
    allocation: int
    compiler_symbol_size: int | None
    restored_symbol_size: int


@dataclasses.dataclass(frozen=True)
class StaticSourceWitness:
    """Internal immutable source fact; audit JSON is never loadable authority."""
    compiler_sha256: str
    raw_sha256: str
    linked_sha256: str
    raw_surface: tuple
    linked_surface: tuple
    source_sha256: str
    cpp_sha256: str
    extents: tuple[StaticSourceExtent, ...]
    inputs: tuple[tuple[str, str], ...]
    root: pathlib.Path
    toolchain_sha256: str
    raw_observation: object = dataclasses.field(repr=False)
    linked_observation: object = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def exact_anchor(self, raw, linked, section, offset):
        if (self.seal is not _STATIC_SOURCE_SEAL or raw is not self.raw_observation
                or linked is not self.linked_observation
                or hashlib.sha256(raw.data).hexdigest() != self.raw_sha256
                or hashlib.sha256(linked.data).hexdigest() != self.linked_sha256):
            raise MetadataProofError("static source witness belongs to different object observations")
        if (_static_parsed_surface(raw) != self.raw_surface
                or _static_parsed_surface(linked) != self.linked_surface):
            raise MetadataProofError('static parsed object surface changed after witnessing')
        if any(sha256_file(self.root/name) != value for name,value in self.inputs):
            raise MetadataProofError('stale static source/header/parser inputs')
        static_anchor_module(self.root)
        choices = [x for x in self.extents if x.section == section]
        if len(choices) != 1 or offset != choices[0].offset:
            raise MetadataProofError("static source operand must equal its unique exact object anchor")
        return choices[0]


def static_source_witness(compiler, raw, linked, facts, source_sha256, cpp_sha256, inputs, root, toolchain_sha256):
    """Join actual compiler anchors/unsized symbols to held semantic extents."""
    raw_surface, linked_surface = _static_parsed_surface(raw), _static_parsed_surface(linked)
    statics = static_anchor_module(root)
    import runlink_pack as rp
    sections = statics._sections(compiler.data)
    debug = [x for x in sections if x[-1] == ".mdebug"]
    if len(debug) != 1:
        raise MetadataProofError("static source requires one actual compiler mdebug")
    anchors = {".data": statics.data_anchors(compiler.data, debug[0]),
               ".bss": statics.bss_anchors(compiler.data, debug[0])}
    extents = []
    if len({row['name'] for row in facts}) != len(facts):
        raise MetadataProofError("duplicate static source facts")
    for row in facts:
        if (set(row) != {'name','section','element','count','size'}
                or row['element'] != 'signed-long-o32' or type(row['count']) is not int
                or not 0 < row['count'] <= ((1 << 31)-1)//4 or row['size'] != 4*row['count']
                or row['section'] not in ('.data','.bss')
                or row['section'] == '.data' and row['count'] != 1):
            raise MetadataProofError('malformed static source fact')
    for section in ('.data', '.bss'):
        if not any(row['section'] == section for row in facts):
            continue
        rows = [row for row in facts if row['section'] == section]
        if len(rows) != 1 or set(anchors[section]) != {rows[0]['name']}:
            raise MetadataProofError("static target section requires one authentic source storage owner")
        row = rows[0]; name = row['name']; offset = anchors[section][name]
        if offset % 4:
            raise MetadataProofError('static signed-long anchor is not word aligned')
        if not compiler.size(section) == raw.size(section) == linked.size(section):
            raise MetadataProofError('static allocation changed across ordinary objects')
        for image in (compiler, raw, linked):
            sec = image.section(section)
            if (not sec or sec['addr'] or sec['flags'] != 3
                    or sec['type'] != (8 if section == '.bss' else 1)
                    or not 0 <= offset < offset + row['size'] <= sec['size']):
                raise MetadataProofError("static semantic extent escapes actual ordinary allocation")
            if section == '.data' and any(image.bytes_of(section)):
                raise MetadataProofError("static zero scalar allocation has unexplained nonzero bytes")
        for image in (raw, linked):
            definitions = [s for s in image.symbols if s.section == section and s.type not in (3,4)]
            named = [s for s in image.symbols if s.name == name and s.shndx != rp.SHN_UNDEF]
            if (len(definitions) != 1 or len(named) != 1 or definitions[0].name != name
                    or (definitions[0].bind, definitions[0].type, definitions[0].value, definitions[0].size)
                       != (rp.STB_LOCAL, rp.STT_OBJECT, offset, 0)):
                raise MetadataProofError("static anchor/ordinary unsized LOCAL definition differs or overlaps")
        originals = [s for s in compiler.symbols if s.name == name and s.shndx != rp.SHN_UNDEF]
        if len(originals) > 1 or any((s.section,s.bind,s.type,s.value,s.size) != (section,rp.STB_LOCAL,rp.STT_OBJECT,offset,0) for s in originals):
            raise MetadataProofError("original static symbol differs from its address-only anchor")
        extents.append(StaticSourceExtent(name,section,offset,row['size'],row['count'],raw.size(section),
                                          originals[0].size if originals else None,0))
    if _static_parsed_surface(raw) != raw_surface or _static_parsed_surface(linked) != linked_surface:
        raise MetadataProofError('static parsed object metadata changed during validation')
    return StaticSourceWitness(hashlib.sha256(compiler.data).hexdigest(),hashlib.sha256(raw.data).hexdigest(),
        hashlib.sha256(linked.data).hexdigest(),raw_surface,linked_surface,source_sha256,cpp_sha256,tuple(extents),tuple(sorted(inputs)),
        pathlib.Path(root),toolchain_sha256,raw,linked,_STATIC_SOURCE_SEAL)

@dataclasses.dataclass(frozen=True)
class IntegerWordRole:
    function: str
    target: str
    carrier: str
    occurrences: int


def integer_word_facts(cpp, flags, root, *, profile=None):
    """Closed o32 sign-bit word expressions; no pointer-access/sink analysis."""
    import candidate_context as lexical
    if profile not in (None, 'isolated-full-expression'):
        raise MetadataProofError('unsupported integer-word source profile')
    isolated = profile == 'isolated-full-expression'
    if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
        raise MetadataProofError('loaded integer-word proof changed')
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('integer-word encoding requires actual o32 flags')
    ca, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('integer-word effective C exceeds proof bound')
    try:
        ast = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('integer-word effective C cannot be parsed') from error
    def nodes(node):
        yield node
        for _, child in node.children():
            yield from nodes(child)
    aliases, definitions = collections.defaultdict(list), collections.defaultdict(list)
    for node in nodes(ast):
        if isinstance(node, ca.Pragma) or isinstance(node, ca.Typedef) and node not in ast.ext:
            raise MetadataProofError('integer-word source has unsupported pragma/local typedef')
    for node in ast.ext:
        if isinstance(node, ca.Typedef):
            aliases[node.name].append(node.type)
        if isinstance(node, ca.Decl) and node.init is not None:
            definitions[node.name].append(node)
    def words(node, seen=()):
        if not isinstance(node, ca.TypeDecl) or not isinstance(node.type, ca.IdentifierType):
            return None
        names = node.type.names
        if node.quals and not (names == ['void'] and node.quals == ['const']):
            return None
        if getattr(node, 'align', None):
            return None
        if len(names) == 1 and names[0] in aliases:
            if names[0] in seen or len(aliases[names[0]]) != 1:
                raise MetadataProofError('ambiguous integer-word typedef')
            return words(aliases[names[0]][0], seen + (names[0],))
        return sorted(names)
    def scalar(node, signed):
        return (isinstance(node, ca.TypeDecl) and not node.quals and not getattr(node, 'align', None)
                and words(node) in ((['long'], ['long','signed'], ['int','long'], ['int','long','signed'])
                                   if signed else (['long','unsigned'], ['int','long','unsigned'])))
    def pointer(node):
        return (isinstance(node, ca.PtrDecl) and not node.quals
                and isinstance(node.type, ca.TypeDecl) and node.type.quals == ['const']
                and not getattr(node.type, 'align', None) and words(node.type) == ['void'])
    def field(node):
        if (isinstance(node, ca.StructRef) and node.type == '.'
                and isinstance(node.name, ca.ID) and isinstance(node.field, ca.ID)):
            return node.name.name, node.field.name
        return None
    def signbit(node):
        return (isinstance(node, ca.BinaryOp) and node.op == '-'
                and isinstance(node.left, ca.UnaryOp) and node.left.op == '-'
                and isinstance(node.left.expr, ca.Constant) and node.left.expr.type == 'int'
                and node.left.expr.value == '2147483647'
                and isinstance(node.right, ca.Constant) and node.right.type == 'int' and node.right.value == '1')
    parents = {id(child): node for node in nodes(ast) for _,child in node.children()}
    expressions = (ca.Assignment, ca.BinaryOp, ca.Cast, ca.Constant, ca.ExprList,
                   ca.FuncCall, ca.ID, ca.StructRef, ca.UnaryOp, ca.ArrayRef, ca.TernaryOp)
    def full_expression(node):
        """Find a C evaluation boundary, never its containing control statement."""
        while id(node) in parents:
            parent = parents[id(node)]
            boundary = (isinstance(parent, ca.Compound)
                or isinstance(parent, ca.Decl) and parent.init is node
                or isinstance(parent, ca.Return) and parent.expr is node
                or isinstance(parent, (ca.If, ca.While, ca.DoWhile, ca.Switch)) and parent.cond is node
                or isinstance(parent, ca.For) and any(value is node for value in (parent.init, parent.cond, parent.next))
                or isinstance(parent, ca.If) and any(value is node for value in (parent.iftrue, parent.iffalse))
                or isinstance(parent, (ca.For, ca.While, ca.DoWhile, ca.Label)) and parent.stmt is node
                or isinstance(parent, (ca.Case, ca.Default)) and any(value is node for value in parent.stmts or []))
            if boundary:
                if isinstance(node, expressions):
                    return node
                raise MetadataProofError('unsupported integer-word full-expression boundary')
            if not isinstance(parent, expressions):
                raise MetadataProofError('unsupported integer-word full-expression context')
            # FuncCall.args is an argument list, not a comma or evaluation boundary.
            node = parent
        raise MetadataProofError('missing integer-word full-expression boundary')
    structures = collections.defaultdict(list)
    for declaration in ast.ext:
        if isinstance(declaration, (ca.Decl, ca.Typedef)):
            kind = declaration.type
            while isinstance(kind, (ca.TypeDecl, ca.PtrDecl, ca.ArrayDecl)):
                kind = kind.type
            # Function-prototype parameters and initializer expressions do not
            # complete a tag in the enclosing file scope.
            if isinstance(kind, ca.Struct) and kind.name and kind.decls is not None:
                structures[kind.name].append(kind)
    def object_struct(node, seen=()):
        if not isinstance(node, ca.TypeDecl) or node.quals or getattr(node, 'align', None):
            return None
        kind = node.type
        if isinstance(kind, ca.IdentifierType) and len(kind.names) == 1:
            name = kind.names[0]
            choices = aliases.get(name, ())
            if name in seen or len(choices) != 1:
                return None
            return object_struct(choices[0], seen + (name,))
        if isinstance(kind, ca.Struct):
            if kind.name:
                choices = structures.get(kind.name, ())
                return choices[0] if len(choices) == 1 and choices[0].decls else None
            return kind if kind.decls else None
        return None  # Other complete C types require a separate reviewed case.
    def extra_pointer(node):
        return isinstance(node, ca.PtrDecl) and not node.quals and object_struct(node.type) is not None
    def scalar_read(member, use, structure):
        fields = [declaration for declaration in structure.decls if declaration.name == use.field.name]
        if len(fields) != 1 or fields[0].bitsize:
            return False
        kind = words(fields[0].type)
        if not kind or any(word not in ('char','short','int','long','signed','unsigned','float','double') for word in kind):
            return False  # No pointer, array or aggregate field can hide a nested lvalue.
        outer = parents[id(use)]
        if isinstance(outer, ca.Assignment):
            return outer.rvalue is use
        if isinstance(outer, ca.UnaryOp):
            return outer.op in ('+','-','!','~')
        if isinstance(outer, ca.Cast):
            cast = words(outer.to_type.type)
            return bool(cast) and all(word in ('char','short','int','long','signed','unsigned','float','double') for word in cast)
        return (isinstance(outer, (ca.BinaryOp, ca.Return))
                or isinstance(outer, ca.Decl) and outer.init is use
                or isinstance(outer, (ca.If,ca.While,ca.DoWhile,ca.Switch)) and outer.cond is use
                or isinstance(outer, ca.For) and outer.cond is use)
    roles = []
    for function in (n for n in ast.ext if isinstance(n, ca.FuncDef)):
        local = collections.defaultdict(list)
        for node in nodes(function):
            if isinstance(node, ca.Decl):
                local[node.name].append(node)
        carriers, extras, extra_types = {}, {}, {}
        if isolated and any(isinstance(node, ca.Struct) and node.decls is not None
                            for node in nodes(function.body)):
            raise MetadataProofError('unsupported local integer-word object type/shadow')
        for name, declarations in local.items():
            for declaration in declarations:
                kind = declaration.type
                if not isinstance(kind, ca.TypeDecl) or not isinstance(kind.type, ca.Union):
                    continue
                members = kind.type.decls or []
                if len(members) == 2 and pointer(members[0].type) and scalar(members[1].type, True):
                    conversion = members[0].name, members[1].name
                elif isolated and len(members) == 3:
                    pointers = [m for m in members if pointer(m.type)]
                    scalars = [m for m in members if scalar(m.type, True)]
                    ordinary = [m for m in members if extra_pointer(m.type)]
                    if len(pointers) != 1 or len(scalars) != 1 or len(ordinary) != 1:
                        continue
                    if any(not m.name for m in members) or len({m.name for m in members}) != 3:
                        raise MetadataProofError('ambiguous integer-word carrier members')
                    conversion = pointers[0].name, scalars[0].name
                    extras[name] = ordinary[0].name
                    extra_types[name] = object_struct(ordinary[0].type.type)
                else:
                    continue
                if (len(declarations) != 1 or declaration.init is not None or declaration.storage
                        or declaration.quals or getattr(declaration, 'align', None)
                        or any(m.storage or m.bitsize for m in members)):
                    raise MetadataProofError('unsupported/shadowed integer-word carrier')
                carriers[name] = conversion
        covered, occurrences = set(), collections.Counter()
        for expression in nodes(function.body):
            if not isinstance(expression, ca.ExprList) or len(expression.exprs) != 2:
                continue
            parent = parents.get(id(expression))
            if isinstance(parent, ca.FuncCall) and parent.args is expression:
                continue  # Argument separators are not a sequenced comma expression.
            assignment, cast = expression.exprs
            if not isinstance(assignment, ca.Assignment) or field(assignment.lvalue) is None:
                continue
            carrier, member = field(assignment.lvalue)
            if carrier not in carriers:
                continue
            valid = (assignment.op == '=' and member == carriers[carrier][0]
                     and isinstance(assignment.rvalue, ca.Cast) and pointer(assignment.rvalue.to_type.type)
                     and isinstance(assignment.rvalue.expr, ca.ID) and isinstance(cast, ca.Cast)
                     and scalar(cast.to_type.type, False) and isinstance(cast.expr, ca.BinaryOp)
                     and cast.expr.op == '^' and signbit(cast.expr.right)
                     and field(cast.expr.left) == (carrier, carriers[carrier][1]))
            if not valid:
                raise MetadataProofError('unsupported integer-word transform/direct base')
            if isolated:
                boundary = full_expression(expression)
                own_ids = {id(node) for node in nodes(expression)
                           if isinstance(node, ca.ID) and node.name in carriers}
                boundary_ids = {id(node) for node in nodes(boundary)
                                if isinstance(node, ca.ID) and node.name in carriers}
                if len(own_ids) != 2 or boundary_ids != own_ids:
                    raise MetadataProofError('integer-word conversion is not isolated in its full expression')
            target = assignment.rvalue.expr.name
            choices = definitions[target]
            if (local[target] or len(choices) != 1 or choices[0].storage or choices[0].quals
                    or not isinstance(choices[0].type, ca.ArrayDecl)
                    or not isinstance(choices[0].type.dim, ca.Constant)
                    or choices[0].type.dim.type != 'int'
                    or not re.fullmatch(r'[1-9][0-9]*', choices[0].type.dim.value)):
                raise MetadataProofError('integer-word target requires an unshadowed initialized global array')
            occurrences[(target, carrier)] += 1
            covered.update(id(node) for node in nodes(expression))
        for name in set(carriers) | {target for target, carrier in occurrences}:
            uses = [node for node in nodes(function.body) if isinstance(node, ca.ID) and node.name == name]
            for node in uses:
                if id(node) in covered:
                    continue
                if isolated and name in extras:
                    member = parents[id(node)]
                    if field(member) != (name, extras[name]):
                        raise MetadataProofError('uncovered integer-word target/carrier use')
                    full_expression(node)
                    use = parents[id(member)]
                    if isinstance(use, ca.Assignment) and use.op == '=' and use.lvalue is member:
                        if any(isinstance(n, ca.ID) and n.name in carriers for n in nodes(use.rvalue)):
                            raise MetadataProofError('integer-word carrier/member alias or copy')
                        continue
                    if isinstance(use, ca.StructRef) and use.type == '->' and use.name is member:
                        if scalar_read(member, use, extra_types[name]):
                            continue
                    raise MetadataProofError('unsupported integer-word extra-pointer use or escape')
                raise MetadataProofError('uncovered integer-word target/carrier use')
        roles.extend(IntegerWordRole(function.decl.name, target, carrier, count)
                     for (target, carrier), count in sorted(occurrences.items()))
    if not roles or len({(r.function,r.target) for r in roles}) != len(roles):
        raise MetadataProofError('missing/ambiguous integer-word source roles')
    if before != literal_profile_inputs(root) or lexical.identity() != lexical_identity:
        raise MetadataProofError('integer-word parser/helper changed')
    return tuple(roles)


_INTEGER_WORD_SEAL = object()


@dataclasses.dataclass(frozen=True)
class IntegerWordPair:
    function: str
    target: str
    high: int
    low: int
    target_offset: int
    target_size: int


@dataclasses.dataclass(frozen=True)
class IntegerWordWitness:
    compiler_sha256: str
    raw_sha256: str
    linked_sha256: str
    compiler_surface: tuple
    raw_surface: tuple
    linked_surface: tuple
    source_sha256: str
    cpp_sha256: str
    roles: tuple[IntegerWordRole, ...]
    pairs: tuple[IntegerWordPair, ...]
    inputs: tuple[tuple[str, str], ...]
    root: pathlib.Path
    toolchain_sha256: str
    compiler_observation: object = dataclasses.field(repr=False)
    raw_observation: object = dataclasses.field(repr=False)
    linked_observation: object = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def operand(self, raw, linked, rel, addend, exported):
        if (self.seal is not _INTEGER_WORD_SEAL or raw is not self.raw_observation
                or linked is not self.linked_observation
                or hashlib.sha256(self.compiler_observation.data).hexdigest() != self.compiler_sha256
                or _static_parsed_surface(self.compiler_observation) != self.compiler_surface
                or hashlib.sha256(raw.data).hexdigest() != self.raw_sha256
                or hashlib.sha256(linked.data).hexdigest() != self.linked_sha256
                or _static_parsed_surface(raw) != self.raw_surface
                or _static_parsed_surface(linked) != self.linked_surface):
            raise MetadataProofError('integer-word object observations changed')
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF or any(
                sha256_file(self.root/name) != value for name,value in self.inputs):
            raise MetadataProofError('stale integer-word source/header/parser inputs')
        _effective_parser(self.root)
        actual = [(r.offset,r.type,r.sym.index) for r in raw.rels('.text')]
        if (rel.offset,rel.type,rel.sym.index) not in actual or rel.sym is not raw.symbols[rel.sym.index]:
            raise MetadataProofError('integer-word operand is not an actual raw relocation')
        choices = [p for p in self.pairs if p.target == rel.sym.name and rel.offset in (p.high,p.low)]
        if (exported or addend != 0x80000000 or len(choices) != 1
                or rel.type != (5 if rel.offset == choices[0].high else 6)):
            raise MetadataProofError('not an authenticated unexported integer-word HI/LO operand')
        return choices[0]


def integer_word_witness(compiler, raw, linked, roles, exports, source_sha256, cpp_sha256,
                         inputs, root, toolchain_sha256):
    """Bind complete closed source roles to actual allocated targets/raw pairs."""
    import runlink_pack as rp
    surfaces = tuple(_static_parsed_surface(image) for image in (compiler,raw,linked))
    if (not roles or any(not isinstance(r, IntegerWordRole) or r.occurrences < 1 for r in roles)
            or len({(r.function,r.target) for r in roles}) != len(roles)):
        raise MetadataProofError('untrusted/ambiguous integer-word source roles')
    for section in rp.EDITED_SECTIONS:
        old, new = compiler.rels(section), raw.rels(section)
        if (compiler.bytes_of(section) != raw.bytes_of(section) or compiler.size(section) != raw.size(section)
                or [(r.offset,r.type) for r in old] != [(r.offset,r.type) for r in new]
                or rp.in_place_addends(compiler,section,old) != rp.in_place_addends(raw,section,new)):
            raise MetadataProofError('integer-word raw compiler records changed')
    obj = rp.edited_prepare_object(raw, linked)
    source_roles = {(r.function,r.target) for r in roles}
    pairs = []
    for section in rp.EDITED_SECTIONS[:-1]:
        rels = raw.rels(section)
        adds = rp.in_place_addends(raw,section,rels)
        for index, (high, addend) in enumerate(zip(rels,adds)):
            if addend != 0x80000000 or high.type == rp.R_MIPS_LO16:
                continue
            if section != '.text' or high.type != rp.R_MIPS_HI16 or index + 1 >= len(rels):
                raise MetadataProofError('integer-word encoding requires a text HI/LO pair')
            low = rels[index+1]
            if (low.type != rp.R_MIPS_LO16 or high.sym.index != low.sym.index
                    or adds[index+1] != addend or high.sym.bind != rp.STB_GLOBAL
                    or high.sym.name in exports):
                raise MetadataProofError('integer-word pair/target/export differs')
            original = [(r.offset,r.type,r.sym.name,r.sym.bind,r.sym.type) for r in compiler.rels(section)]
            if any((r.offset,r.type,r.sym.name,r.sym.bind,r.sym.type) not in original for r in (high,low)):
                raise MetadataProofError('integer-word original compiler target identity changed')
            target = rp.edited_global_target(obj, high.sym.name)
            original_target = rp.edited_global_target({'raw':compiler,'linked':linked,'bases':obj['bases']},high.sym.name)
            if original_target != target:
                raise MetadataProofError('integer-word original compiler definition changed')
            if target[0] != '.data' or target[3] != rp.STT_OBJECT:
                raise MetadataProofError('integer-word target is not allocated GLOBAL DATA')
            rp.edited_object_operand(target,0)
            function = rp.edited_caller(obj,section,high.offset)
            if (function != rp.edited_caller(obj,section,low.offset)
                    or (function,high.sym.name) not in source_roles):
                raise MetadataProofError('integer-word pair lacks its qualified source role')
            owner = rp.edited_global_target(obj,function)
            original_obj = {'raw':compiler,'linked':linked,'bases':obj['bases']}
            if (rp.edited_caller(original_obj,section,high.offset) != function
                    or rp.edited_caller(original_obj,section,low.offset) != function):
                raise MetadataProofError('integer-word original compiler function ownership changed')
            original_owner = rp.edited_global_target(original_obj,function)
            if owner[0] != '.text' or owner[3] != rp.STT_FUNC or original_owner != owner:
                raise MetadataProofError('integer-word original GLOBAL FUNCTION geometry differs')
            if (high.offset % 4 or low.offset % 4 or min(high.offset,low.offset) < 0
                    or max(high.offset,low.offset)+4 > raw.size(section)):
                raise MetadataProofError('integer-word relocation word escapes text')
            hiword, loword = rp.u32(raw.bytes_of(section),high.offset), rp.u32(raw.bytes_of(section),low.offset)
            if (hiword >> 26 != 15 or loword >> 26 != 9
                    or (hiword >> 16)&31 != (loword >> 21)&31
                    or (hiword >> 16)&31 != (loword >> 16)&31):
                raise MetadataProofError('integer-word pair is not same-register LUI/ADDIU')
            pairs.append(IntegerWordPair(function,high.sym.name,high.offset,low.offset,target[1],target[2]))
    if {(p.function,p.target) for p in pairs} != source_roles:
        raise MetadataProofError('integer-word source/raw role set differs')
    covered = {offset for p in pairs for offset in (p.high,p.low)}
    if any(a == 0x80000000 and (s != '.text' or r.offset not in covered)
           for s in rp.EDITED_SECTIONS[:-1]
           for r,a in zip(raw.rels(s),rp.in_place_addends(raw,s,raw.rels(s)))):
        raise MetadataProofError('uncovered integer-word raw operand')
    if tuple(_static_parsed_surface(image) for image in (compiler,raw,linked)) != surfaces:
        raise MetadataProofError('integer-word parsed metadata changed during witnessing')
    return IntegerWordWitness(hashlib.sha256(compiler.data).hexdigest(),hashlib.sha256(raw.data).hexdigest(),
        hashlib.sha256(linked.data).hexdigest(),*surfaces,source_sha256,cpp_sha256,tuple(roles),tuple(pairs),
        tuple(sorted(inputs)),pathlib.Path(root),toolchain_sha256,compiler,raw,linked,_INTEGER_WORD_SEAL)


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


def local_data_objects(command, root):
    import shlex
    words = shlex.split(command)
    flag = "--local-data-objects"
    selected = flag in words
    if selected and (words.count(flag) != 1 or "--local-data-aggregate" in words
                     or not local_ro_mixed_group(command, root)):
        raise MetadataProofError("LOCAL objects need one explicit mixed provider group")
    return selected


def capture_data_module_producer_raw(root, source, configured, linked, map_path):
    """Fresh producer capture, distinct from and inadmissible as promotion proof."""
    return _capture_configured_raw(root, source, configured, linked, "", module_link=True,
                                   producer_map=map_path)


def capture_nonexec_producer_raw(root, source, configured, linked, map_path):
    """Fresh original compiler output for a declared nonexec metadata recipe."""
    return _capture_configured_raw(root, source, configured, linked, "", producer_map=map_path)


def _keep_rodata_plan(command, target, root):
    """The closed whole-pool recipe, not a generic missing-marker fallback."""
    plan = metadata_filter_plan(command, target, root)
    parts = [shlex.split(part) for part in command.split("&&")]
    if (len(parts) != 2 or len(parts[0]) != 7
            or not pathlib.Path(parts[0][0]).name.startswith("python")
            or parts[0][1:4] != ["tools/externalize_elf_section.py", target, ".rodata"]
            or not re.fullmatch(r"sha256:[0-9a-f]{64}", parts[0][4])
            or parts[0][5:] != ["0", "keep"]
            or parts[1] != ["tools/binutils/mips64-elf-objcopy", "--remove-section", ".rel.rodata", target]
            or plan != [("externalize", (".rodata", parts[0][4], 0)),
                        ("remove-section", [".rel.rodata"]) ]):
        raise MetadataProofError("unsupported keep-only RO recipe")
    return plan


def _validate_kept_rodata(original, replayed, configured):
    """Keep the actual allocated pool and its original outgoing source records."""
    import runlink_pack as rp
    import source_module_link_proof as proof
    for elf in (original, replayed, configured):
        fresh = rp.Elf(elf.path, elf.data)
        if (_static_parsed_surface(elf) != _static_parsed_surface(fresh)
                or elf.section(".rodata") != fresh.section(".rodata")):
            raise MetadataProofError("keep-only parsed metadata changed")
    section = original.section(".rodata")
    if (not section or not section["size"] or section["type"] != 1
            or section["flags"] != 2 or section["addr"] != 0
            or not original.rels(".rodata")):
        raise MetadataProofError("keep-only RO lacks its original allocated pool/records")
    if proof.allocated(original) != proof.allocated(replayed):
        raise MetadataProofError("keep-only RO replay changed allocated bytes or geometry")
    if len(original.rels(".text")) != len(replayed.rels(".text")) or replayed.rels(".rodata"):
        raise MetadataProofError("keep-only RO replay changed the wrong relocation surface")
    for rel, addend in zip(original.rels(".rodata"),
                           rp.in_place_addends(original, ".rodata", original.rels(".rodata"))):
        target = rel.sym.value + addend
        functions = [s for s in original.symbols if s.section == ".text"
                     and s.type == rp.STT_FUNC and s.size and s.value <= target < s.value+s.size]
        if (rel.type != rp.R_MIPS_32 or rel.offset % 4
                or not 0 <= rel.offset <= section["size"]-4
                or rel.sym.section != ".text" or target % 4 or len(functions) != 1):
            raise MetadataProofError("keep-only RO has an unsupported original R32 target")
    try:
        rp.validate_fresh_data_input(replayed, configured)
    except rp.PackError as error:
        raise MetadataProofError("keep-only configured replay differs: " + str(error)) from error


_EDITED_PREFIX_SEAL = object()


@dataclasses.dataclass(frozen=True)
class EditedPrefixCapture:
    root: pathlib.Path
    provider: object
    command: tuple
    inputs: tuple
    surface: object
    mapping: tuple
    projection_tools: object
    seal: object = dataclasses.field(repr=False)

    def current(self):
        if (self.seal is not _EDITED_PREFIX_SEAL
                or _static_parsed_surface(self.provider['elf']) != self.surface
                or tuple(self.provider['map']) != self.mapping
                or metadata_link.edited_projection_tools() != self.projection_tools
                or any(sha256_file(self.root/name) != value for name,value in self.inputs)):
            raise MetadataProofError('edited provider prefix changed')
        return {'role': 'edited-provider-prefix', 'command': list(self.command),
                'inputs': dict(self.inputs)}


def bind_edited_prefix(root, provider, directory, command, prefix, commands, input_hashes):
    """Bind the existing freshly produced projection, never an arbitrary ELF path."""
    import runlink_pack as rp
    root, directory = pathlib.Path(root), pathlib.Path(directory)
    expected = ['tools/binutils/mips64-elf-ld', '-r', '-T',
                str((directory/'providers.ld').relative_to(root)), '-Map',
                str((directory/'providers.map').relative_to(root)), '-o',
                str((directory/'providers.elf').relative_to(root))]
    if list(map(str,command)) != expected or not commands or commands[-1] != expected:
        raise MetadataProofError('edited prefix lacks its actual ordinary link command')
    if provider['elf'].path != directory/'providers.elf':
        raise MetadataProofError('edited prefix loaded object differs')
    with tempfile.TemporaryDirectory(prefix='prefix-bind-', dir=root/'build') as temporary:
        actual, _ = metadata_link.edited_projection(root, pathlib.Path(temporary))
    if actual != prefix or (directory/'providers.ld').read_text() != prefix:
        raise MetadataProofError('edited prefix differs from current source projection')
    selectors = metadata_link.edited_program_selectors(prefix)
    if not selectors or any('/overlays/' in name for name in selectors):
        raise MetadataProofError('edited provider projection has selected inputs')
    held = {name: input_hashes[name] for name in selectors}
    held.update({str(path.relative_to(root)): value for path,value in provider['holds'].items()})
    held[str((directory/'providers.ld').relative_to(root))] = hashlib.sha256(prefix.encode()).hexdigest()
    for name in ('mickey.us.yaml','tools/binutils/mips64-elf-ld',
                 'tools/proof_provenance.py','tools/runlink_metadata_link.py','tools/runlink_pack.py'):
        held[name] = sha256_file(root/name)
    result = EditedPrefixCapture(root, provider, tuple(expected), tuple(sorted(held.items())),
        _static_parsed_surface(provider['elf']), tuple(provider['map']),
        metadata_link.edited_projection_tools(), _EDITED_PREFIX_SEAL)
    result.current()
    return result


def resident_cell_declarations(cpp, flags, root, names, *, definition):
    """Closed o32 unsigned-short pointer cells, not a general C layout engine."""
    import candidate_context as lexical
    if '-32' not in flags or any(flag in flags for flag in ('-64','-n32','-n64')):
        raise MetadataProofError('resident cell requires actual o32 flags')
    ca, parser, before, lexical_identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('resident cell CPP exceeds bound')
    try:
        ast = parser.CParser().parse(cpp.decode('utf-8'))
    except Exception as error:
        raise MetadataProofError('resident cell effective C cannot be parsed') from error
    aliases, declarations = collections.defaultdict(list), collections.defaultdict(list)
    for node in ast.ext:
        if isinstance(node, ca.Typedef):
            aliases[node.name].append(node.type)
    class Visitor(ca.NodeVisitor):
        def visit_Pragma(self, node):
            raise MetadataProofError('resident cell source has a pragma')
        def visit_Decl(self, node):
            if node.name in names:
                declarations[node.name].append(node)
            self.generic_visit(node)
    Visitor().visit(ast)
    def unsigned_short(node, seen=()):
        if not isinstance(node, ca.TypeDecl) or node.quals or getattr(node,'align',None):
            return False
        if not isinstance(node.type, ca.IdentifierType):
            return False
        words = node.type.names
        if sorted(words) in (['short','unsigned'], ['int','short','unsigned']):
            return True
        return (len(words) == 1 and words[0] not in seen and len(aliases[words[0]]) == 1
                and unsigned_short(aliases[words[0]][0], seen+(words[0],)))
    for name in names:
        rows = declarations[name]
        if not rows or any(node not in ast.ext or node.quals or node.funcspec or node.bitsize
                or getattr(node,'align',None) or node.init is not None
                or node.storage not in ([], ['extern']) or not isinstance(node.type,ca.PtrDecl)
                or node.type.quals or not unsigned_short(node.type.type) for node in rows):
            raise MetadataProofError('unsupported resident pointer-cell declaration: '+name)
        if sum(not node.storage for node in rows) != int(definition):
            raise MetadataProofError('resident pointer-cell definition count differs: '+name)
        if not definition and any(node.storage != ['extern'] for node in rows):
            raise MetadataProofError('resident cell consumer is not external')
    if before != literal_profile_inputs(root) or lexical.identity() != lexical_identity:
        raise MetadataProofError('resident cell parser changed')
    return tuple(sorted(names))


def capture_edited_provider_raw(root, source, configured, prefix):
    if not isinstance(prefix, EditedPrefixCapture):
        raise MetadataProofError('resident cell lacks an authenticated edited prefix')
    prefix.current()
    return _capture_configured_raw(root, source, configured, prefix.provider['elf'].path, '',
        producer_map=next(path for path in prefix.provider['holds'] if path.suffix == '.map'),
        ordinary_provider=True, edited_prefix=prefix)


def capture_data_provider_raw(root, source, configured, linked, map_path, *, preserve_compiler=False):
    """Fresh ordinary-provider C using the same bound producer capture lifecycle."""
    return _capture_configured_raw(root, source, configured, linked, "", producer_map=map_path,
                                   ordinary_provider=True,
                                   **({"preserve_compiler": True} if preserve_compiler else {}))


def _capture_configured_raw(root, source, configured, linked, postprocess, *,
                            module_link=False, producer_map=None, ordinary_provider=False, edited_prefix=None,
                            preserve_compiler=False):
    """Fresh reproduction only: never authenticate an externally supplied raw object."""
    import permute_batch as batch
    import sweep_receipts as receipts
    if root.resolve() != batch.ROOT.resolve() or os.environ.get("PROMOTION_TRIAL", "") not in ("", "0"):
        raise MetadataProofError("unsupported raw-proof root or relaxed trial environment")
    if edited_prefix is not None:
        if not isinstance(edited_prefix, EditedPrefixCapture) or edited_prefix.root != root:
            raise MetadataProofError("foreign edited provider capture")
        edited_prefix.current()
    target = configured.relative_to(root).as_posix()
    deadline = time.monotonic() + 120
    static_lines = []
    keep_observations = {}
    def recipe(operation_deadline):
        output = batch.bounded_capture(
            ["gmake", "--no-print-directory", "-n", "-W", source.relative_to(root).as_posix(), target],
            operation_deadline, check=True).stdout.replace("\\\n", " ").splitlines()
        compilers = [line.strip() for line in output if target in line and "tools/ido/cc" in line]
        helper = ("tools/link_overlay_module.py" if module_link else
                  "tools/trim_elf_section.py" if ordinary_provider or producer_map is not None
                  else "tools/filter_elf_relocations.py")
        metadata = [line.strip() for line in output if target in line and helper in line]
        keep_only = False
        if not metadata and not module_link and not ordinary_provider:
            candidates = [line.strip() for line in output
                          if target in line and "tools/externalize_elf_section.py" in line]
            if len(candidates) == 1:
                _keep_rodata_plan(candidates[0], target, root)
                metadata = candidates
                keep_only = True
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
        if keep_only and any(target in line and line.strip() not in
                             set(compilers + metadata + static_lines + rebinds) for line in output):
            raise MetadataProofError("unknown keep-only recipe line")
        if edited_prefix is not None:
            trap = [line.strip() for line in output if target in line and 'tools/trap_names.py' in line]
            expected_trap = ['python3','tools/trap_names.py','rename',target,'config/overlay_funcs_to_trap.us.txt']
            if (metadata or rebinds or static_lines or len(trap) != 1
                    or shlex.split(trap[0]) != expected_trap
                    or any(target in line and line.strip() not in set(compilers+trap) for line in output)):
                raise MetadataProofError('unsupported edited resident provider recipe')
            return compilers[0], trap[0], ''
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
    command, expanded_postprocess, surface_rebind = recipe(deadline)
    args = batch.compiler_arguments(command, source.relative_to(root).as_posix(), target)
    words = shlex.split(command)
    wrapped = len(words) > 1 and words[1] == "tools/asm-processor/build.py"
    dependency_args = args + (("-I", str(source.parent)) if wrapped else ())
    if words.count("-o") != 1:
        raise MetadataProofError("ambiguous configured output argument")
    plan = ([] if module_link or edited_prefix is not None or not expanded_postprocess else
            metadata_filter_plan(expanded_postprocess, target, root))
    if surface_rebind:
        plan = plan + metadata_filter_plan(surface_rebind, target, root)
    literal_prepared = (prepare_literal_input(source, root, command)
                        if any(operation == "literal-end" for operation, arguments in plan) else None)
    pointer_profile = local_ro_owner_profile(expanded_postprocess, root) if module_link else None
    if module_link and callback_selection(expanded_postprocess, root):
        preserve_compiler = True
    pointer_parser_inputs = literal_profile_inputs(root) if pointer_profile else {}
    if pointer_profile:
        _effective_parser(root)
    def context(*, operation_deadline=None):
        if operation_deadline is None:
            operation_deadline = time.monotonic() + 120
        if any(sha256_file(path) != digest for path, digest in keep_observations.items()):
            raise MetadataProofError("kept RO loaded observation changed")
        batch.checked_tool_identity()
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
            raise MetadataProofError("loaded metadata proof implementation changed")
        if recipe(operation_deadline) != (command, expanded_postprocess, surface_rebind):
            raise MetadataProofError("configured compiler recipe changed during raw proof")
        inputs = {"source": sha256_file(source), "configured": sha256_file(configured),
                  "linked": sha256_file(linked), "command": command, "postprocess": expanded_postprocess, "surface_rebind": surface_rebind,
                  "dependencies": (dict(literal_prepared["dependencies"]) if literal_prepared else
                                   batch.source_dependencies(source, dependency_args, operation_deadline)),
                  "wrapper_source_directory": receipts.tree_digest(source.parent) if wrapped else None,
                  "include_tree": receipts.tree_digest(root / "include"),
                  "tools": batch.sweep_tool_identity(),
                  "asm_processor": receipts.tree_digest(root / "tools/asm-processor"),
                  "makefiles": {p.relative_to(root).as_posix(): sha256_file(p)
                                for p in [root / "Makefile", *sorted((root / "mk").glob("**/*.mk"))]},
                  "proof": _LOADED_METADATA_PROOF,
                  "target_inputs": ({"config/overlay_funcs_to_trap.us.txt": sha256_file(root/"config/overlay_funcs_to_trap.us.txt"),
                                     "tools/trap_names.py": sha256_file(root/"tools/trap_names.py")} if edited_prefix is not None else
                    {name: sha256_file(root / name) for name in
                    ("baseroms/mickey.us.z64", "config/overlays.us.json",
                     "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt")}),
                  "generated_metadata": (edited_prefix.current() if edited_prefix is not None else
                                         generated_metadata_context(root) if producer_map is None
                                         else producer_link_context(root, linked, producer_map)),
                  "filter_specs": ({} if edited_prefix is not None else filter_spec_files(expanded_postprocess, root)),
                  "metadata_tools": {name: sha256_file(root / name) for name in
                    ("tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
                     "tools/rebind_elf_relocations.py", "tools/externalize_elf_section.py",
                     "tools/postprocess_guard.py", "tools/binutils/mips64-elf-objcopy")}}
        inputs['metadata_tools'].update(pointer_parser_inputs)
        if literal_prepared:
            _literal_preparation_current(literal_prepared, root)
            inputs["metadata_tools"].update(literal_profile_inputs(root))
            inputs["metadata_tools"].update(literal_frontend_inputs(root, command))
        if module_link:
            local_data_objects(expanded_postprocess, root)
            callback = callback_selection(expanded_postprocess, root)
            inputs["module_link"] = expanded_postprocess
            inputs["module_link_inputs"] = {name: sha256_file(root / name) for name in (
                "config/runlink-ort.us.txt", "tools/link_overlay_module.py", "tools/runlink_pack.py",
                "tools/rebind_elf_relocations.py", "tools/binutils/mips64-elf-ld")}
            for provider in (*local_ro_provider_inputs(expanded_postprocess, root), *(callback[1] if callback else ())):
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
    before = context(operation_deadline=deadline)
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
    original_compiler = None
    if preserve_compiler:
        original_compiler = directory / "compiler-original.o"
        original_compiler.write_bytes(raw.read_bytes())
        keep_observations[original_compiler] = sha256_file(original_compiler)
    if static_lines:
        import static_symbols as statics
        try:
            raw.write_bytes(statics.add_static_symbols(raw.read_bytes())[0])
        except statics.Failure as error:
            raise MetadataProofError(f"static-symbol pass failed: {error}") from error
    compiled_digest = sha256_file(raw)
    pointer_source = None
    if pointer_profile:
        keep_observations[raw] = compiled_digest
        options = [x for x in args if x in {'-g3','-g','-O0','-O1','-O2','-framepointer','-KPIC'}]
        if '-mips2' not in args:
            options.append('-mips1')
        if wrapped:
            frontend = ['.venv/bin/python','tools/asm-processor/asm_processor.py',*options,
                        source.relative_to(root).as_posix()]
            transformed = batch.bounded_capture(frontend, deadline, check=True).stdout.encode('utf-8')
            cpp_input = directory/'compiler-input.c'
            cpp_input.write_bytes(transformed)
            cpp_flags = list(args)+['-I',str(source.parent)]
        else:
            cpp_input, cpp_flags = source, list(args)
        cpp_command = ['tools/ido/cc','-E',*cpp_flags,str(cpp_input)]
        cpp = batch.bounded_capture(cpp_command, deadline, check=True).stdout.encode('utf-8')
        cpp_path = directory/'actual-cpp.c'
        cpp_path.write_bytes(cpp)
        facts = pointer_scalar_facts(cpp, cpp_flags, root)
        keep_observations[cpp_path] = hashlib.sha256(cpp).hexdigest()
        if wrapped:
            keep_observations[cpp_input] = hashlib.sha256(transformed).hexdigest()
        context.pointer_scalar_capture = PointerScalarCapture(cpp, tuple(cpp_flags), cpp_path,
            compiled_digest, _POINTER_SCALAR_SEAL)
        pointer_source = dict(cpp=cpp_path.relative_to(root).as_posix(),
            cpp_sha256=hashlib.sha256(cpp).hexdigest(), flags=cpp_flags, facts=facts,
            command=cpp_command, profile=pointer_profile)
    if before != context(operation_deadline=deadline):
        raise MetadataProofError("source, input, recipe, object or tool changed during raw proof")
    kept = None
    if any(operation == "externalize" for operation, _arguments in plan) and not any(
            operation in ("filter", "rebind", "nonexec-end", "literal-end") for operation, _arguments in plan):
        _keep_rodata_plan(expanded_postprocess, target, root)
        import runlink_pack as rp
        original = rp.Elf(raw)
        keep_observations[raw] = hashlib.sha256(original.data).hexdigest()
        if keep_observations[raw] != compiled_digest:
            raise MetadataProofError("loaded kept RO compiler output changed")
        if "sha256:" + hashlib.sha256(original.bytes_of(".rodata")).hexdigest() != plan[0][1][1]:
            raise MetadataProofError("loaded kept RO differs from its declared payload")
        replay = raw.with_name("kept-replay.o")
        replay_command = expanded_postprocess + (" && " + surface_rebind if surface_rebind else "")
        replay_metadata(raw, replay_command, target, replay, root, skip_filters=False,
                        deadline=deadline, plan=plan)
        prepared_digest = sha256_file(replay)
        prepared, loaded_configured = rp.Elf(replay), rp.Elf(configured)
        if hashlib.sha256(prepared.data).hexdigest() != prepared_digest:
            raise MetadataProofError("loaded kept RO replay output changed")
        configured_digest = hashlib.sha256(loaded_configured.data).hexdigest()
        if configured_digest != before["configured"]:
            raise MetadataProofError("loaded kept RO configured object changed")
        _validate_kept_rodata(original, prepared, loaded_configured)
        keep_observations[replay] = hashlib.sha256(prepared.data).hexdigest()
        kept = {"original": keep_observations[raw], "prepared": keep_observations[replay],
                "configured": configured_digest, "original_ro_records": len(original.rels(".rodata")),
                "original_text_records": len(original.rels(".text")), "size": original.size(".rodata"),
                "prepared_object": replay.relative_to(root).as_posix()}
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
    if before != context(operation_deadline=deadline):
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
    if kept:
        receipt["kept_rodata"] = kept
    if pointer_source:
        receipt["pointer_scalar_source"] = pointer_source
    if original_compiler is not None:
        receipt["compiler_original"] = original_compiler.relative_to(root).as_posix()
        receipt["compiler_original_sha256"] = keep_observations[original_compiler]
    return raw, plan, receipt, context


# ADR0030 is a target linker-value role, not an object or an integer-word alias.
ENCODED_WORD_MARKER = 'o73TriangleCommandWord'
ENCODED_WORD_OWNER = 'o73Triangles'
ENCODED_WORD_ORIGIN = 'overlay_073_DATA_START'
ENCODED_WORD_FUNCTION = 'func_overlay_073_F0000D70_18CB830'
ENCODED_WORD_SOURCE = 'src/overlays/o073/overlay_073.c'
ENCODED_WORD_PROVIDER = 'src/overlays/o073/overlay73Geometry.c'
ENCODED_WORD_TARGET = 'build/' + ENCODED_WORD_SOURCE + '.o'
ENCODED_WORD_ASSOCIATION = ':'.join((ENCODED_WORD_MARKER, ENCODED_WORD_OWNER,
    ENCODED_WORD_ORIGIN, ENCODED_WORD_FUNCTION, 'Overlay73Command.w1'))
_ENCODED_WORD_SEAL = object()


def encoded_word_selection(root, dump=None):
    """A closed program declaration, never evidence for its own semantics."""
    root = pathlib.Path(root)
    if not (root/'Makefile').is_file():
        return False
    if dump is None:
        import postprocess_audit as audit
        dump = audit.run_make_database()
    # Recursive Make databases may repeat the same complete target assignment.
    rows = set()
    for line in dump.splitlines():
        if line.lstrip().startswith('#') or not re.search(r'\bENCODED_ADDRESS_WORD\b', line):
            continue
        row = re.fullmatch(r'(\S+): ENCODED_ADDRESS_WORD = (.*)', line)
        if row is None:
            raise MetadataProofError('malformed encoded-address program association')
        rows.add(row.groups())
    if rows and rows != {(ENCODED_WORD_TARGET, ENCODED_WORD_ASSOCIATION)}:
        raise MetadataProofError('unsupported encoded-address program association')
    present = any(ENCODED_WORD_MARKER in (root/name).read_text()
                  for name in (ENCODED_WORD_SOURCE, 'include/overlays/overlay_073.h')
                  if (root/name).is_file())
    if bool(rows) != present:
        raise MetadataProofError('encoded-address carrier/held opt-in disagree')
    return bool(rows)


def encoded_word_expression():
    return (ENCODED_WORD_MARKER + ' = ABSOLUTE((' + ENCODED_WORD_OWNER + ' - '
            + ENCODED_WORD_ORIGIN + ' + 0x80000000) & 0xffffffff);')



def encoded_word_generation_inputs(root, objects):
    """Bind real input roles before rendering; fresh admission still follows linking."""
    import runlink_pack as rp
    root = pathlib.Path(root)
    wanted = {ENCODED_WORD_TARGET, 'build/'+ENCODED_WORD_PROVIDER+'.o'}
    images = {}; observed = {}
    for number, path in objects:
        path = pathlib.Path(path)
        image = rp.Elf(path)
        observed[path] = hashlib.sha256(image.data).hexdigest()
        relative = path.relative_to(root).as_posix()
        if relative in wanted:
            if number != 73 or relative in images:
                raise MetadataProofError('encoded-address input source identity is ambiguous')
            images[relative] = image
        for symbol in image.symbols:
            if symbol.name == ENCODED_WORD_MARKER and (relative != ENCODED_WORD_TARGET
                    or symbol.shndx != rp.SHN_UNDEF):
                raise MetadataProofError('encoded-address marker has another input home')
    if set(images) != wanted:
        raise MetadataProofError('encoded-address link lacks actual consumer/owner inputs')
    consumer = images[ENCODED_WORD_TARGET]
    encoded_word_pair(consumer, consumer)
    owner = images['build/'+ENCODED_WORD_PROVIDER+'.o']
    definitions = [symbol for symbol in owner.symbols if symbol.name == ENCODED_WORD_OWNER]
    section = owner.section('.data')
    if (len(definitions) != 1 or definitions[0].section != '.data' or definitions[0].bind != rp.STB_GLOBAL
            or definitions[0].type != rp.STT_OBJECT or definitions[0].size <= 0
            or not section or section['type'] != 1 or section['flags'] != 3
            or definitions[0].value < 0 or definitions[0].value + definitions[0].size > section['size']):
        raise MetadataProofError('encoded-address generation lacks its real sized DATA owner')
    for name in (ENCODED_WORD_SOURCE, ENCODED_WORD_PROVIDER, 'include/overlays/overlay_073.h',
                 'Makefile', *[str(p.relative_to(root)) for p in sorted((root/'mk').glob('**/*.mk'))]):
        path = root/name
        observed[path] = sha256_file(path)
    def current():
        if (not encoded_word_selection(root) or any(sha256_file(path) != digest for path,digest in observed.items())):
            raise MetadataProofError('encoded-address generation input/selection changed')
    current()
    return current


def encoded_word_scripts(scripts):
    """Close the expression and its definitions, not merely its numeric result."""
    clean = {name: re.sub(r'/\*.*?\*/|//[^\n]*|#[^\n]*', '', text, flags=re.S)
             for name, text in scripts.items()}
    if any(re.search(r'\b(?:INCLUDE|INPUT|GROUP|AS_NEEDED|SEARCH_DIR|EXTERN)\b', text) for text in clean.values()):
        raise MetadataProofError('encoded-address nested linker script is unsupported')
    carrying = [(name, text) for name, text in clean.items()
                if re.search(r'\b' + ENCODED_WORD_MARKER + r'\b', text)]
    if len(carrying) != 1:
        raise MetadataProofError('encoded-address expression is missing or ambiguous')
    name, text = carrying[0]
    line = encoded_word_expression()
    if (text.count(line) != 1 or len(re.findall(r'\b' + ENCODED_WORD_MARKER + r'\b', text)) != 1
            or any(re.search(r'\b' + ENCODED_WORD_OWNER + r'\s*(?:<<|>>|[+*/%&|^-])?=', value) for value in clean.values())):
        raise MetadataProofError('encoded-address expression is not the fixed owner dependency')
    # The canonical origin is an actual location-counter assignment, once.
    origins = [(m.group(1), m.group(2).strip()) for text in clean.values()
               for m in re.finditer(r'\b' + ENCODED_WORD_ORIGIN + r'\s*((?:<<|>>|[+*/%&|^-])?=)\s*([^;]+);', text)]
    if origins != [('=', '.')]:
        raise MetadataProofError('encoded-address origin is not the actual DATA boundary')
    return name


def encoded_word_link_inputs(scripts):
    """Closed current ordinary-script grammar: every input selector is explicit.

    Quoted/non-build paths, archives, bare inputs, includes and other clauses
    are deliberately unsupported, including zero-sized symbol-only inputs.
    """
    inputs = set()
    atom = r'(?:[A-Za-z_][A-Za-z_0-9]*|0x[0-9A-Fa-f]+|[0-9]+)'
    calls = {'ABSOLUTE','ADDR','ALIGN','SIZEOF','LOADADDR','HIDDEN','FILL'}
    for text in scripts.values():
        clean = re.sub(r'/\*.*?\*/|//[^\n]*|#[^\n]*', '', text, flags=re.S)
        for line in clean.splitlines():
            line = line.strip()
            if not line:
                continue
            match = re.fullmatch(r'(build/[A-Za-z0-9_./-]+\.o)\((\.[A-Za-z0-9_]+)\);', line)
            if match:
                name = match[1]
                if '..' in pathlib.PurePosixPath(name).parts or pathlib.PurePosixPath(name).as_posix() != name:
                    raise MetadataProofError('encoded-address input path is not canonical')
                inputs.add(name)
                continue
            if line in ('SECTIONS','{','}','/DISCARD/ :','*(*);'):
                continue
            if re.fullmatch(r'\.[A-Za-z0-9_]+(?: '+atom+r'| \(NOLOAD\))? : (?:AT\('+atom+r'\) )?SUBALIGN\([0-9]+\)',line):
                continue
            assignment = re.fullmatch(r'[A-Za-z_.][A-Za-z_0-9.]*\s*(?:\+?=)\s*[^;]+;',line)
            intrinsic = re.fullmatch(r'(?:HIDDEN|FILL)\([^;]+\);',line)
            if ((assignment or intrinsic) and re.fullmatch(r'[A-Za-z_0-9. \t()+*/%&|^<>=!~,-]+;',line)
                    and set(re.findall(r'([A-Za-z_][A-Za-z_0-9.]*)\s*\(',line)) <= calls):
                continue
            raise MetadataProofError('encoded-address unsupported linker input/clause: '+line)
    if not inputs:
        raise MetadataProofError('encoded-address link has no explicit input closure')
    return frozenset(inputs)


def replay_encoded_word_link(root, linked, map_path, scripts, link_context):
    """Replay the actual bound ld invocation; only private output paths differ."""
    import runlink_pack as rp
    import permute_batch as batch
    encoded_word_scripts(scripts)
    names = encoded_word_link_inputs(scripts)
    command = list(link_context['command'])
    if set(scripts) != {command[i+1] for i,value in enumerate(command) if value == '-T'}:
        raise MetadataProofError('encoded-address replay script/command closure differs')
    if command.count('-Map') != 1 or command.count('-o') != 1:
        raise MetadataProofError('encoded-address replay lacks unique actual outputs')
    if (root/command[command.index('-Map')+1] != map_path
            or root/command[command.index('-o')+1] != linked):
        raise MetadataProofError('encoded-address replay output/source context differs')
    held = {root/name:sha256_file(root/name) for name in names}
    for name,text in scripts.items():
        path = root/name
        held[path] = hashlib.sha256(text.encode()).hexdigest()
    for name in names:
        path = root/name; image = rp.Elf(path)
        if hashlib.sha256(image.data).hexdigest() != held[path]:
            raise MetadataProofError('encoded-address loaded direct input changed')
        for symbol in image.symbols:
            if symbol.name == ENCODED_WORD_MARKER and name != ENCODED_WORD_TARGET:
                raise MetadataProofError('encoded-address marker has another input home')
            if symbol.shndx == rp.SHN_UNDEF:
                continue
            if symbol.name in (ENCODED_WORD_MARKER, ENCODED_WORD_ORIGIN):
                raise MetadataProofError('encoded-address zero-storage value/boundary has an input definition')
            if symbol.name == ENCODED_WORD_OWNER and (name != 'build/'+ENCODED_WORD_PROVIDER+'.o'
                    or symbol.bind != rp.STB_GLOBAL or symbol.type != rp.STT_OBJECT
                    or symbol.section != '.data' or symbol.size <= 0):
                raise MetadataProofError('encoded-address owner has a competing input definition')
    canonical_map_data = map_path.read_bytes()
    if hashlib.sha256(canonical_map_data).hexdigest() != link_context['inputs'][map_path.relative_to(root).as_posix()]:
        raise MetadataProofError('encoded-address loaded canonical map differs from bound context')
    mapping = tuple(rp.parse_map(map_path,canonical_map_data))
    if any(row.file not in names for row in mapping):
        raise MetadataProofError('encoded-address map input is outside the complete script closure')
    def current():
        if ({path:sha256_file(path) for path in held} != held
                or producer_link_context(root,linked,map_path) != link_context):
            raise MetadataProofError('encoded-address replay input/command closure changed')
        return True
    current()
    parent = root/'build/encoded-word-link-proof';parent.mkdir(parents=True,exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix='capture-',dir=parent))
    output, output_map = directory/'linked.elf', directory/'linked.map'
    replay = command.copy()
    replay[replay.index('-Map')+1] = str(output_map)
    replay[replay.index('-o')+1] = str(output)
    (directory/'inputs.json').write_text(json.dumps({
        'command':command,'replay':replay,'inputs':{str(p.relative_to(root)):h for p,h in held.items()}},indent=2)+'\n')
    try:
        result = batch.bounded_capture(replay,time.monotonic()+120,check=True,cwd=root)
    except BaseException as error:
        (directory/'link.log').write_text(str(getattr(error,'output',''))+'\n'+repr(error))
        raise
    (directory/'link.log').write_text(result.stdout)
    current()
    image, original = rp.Elf(output), rp.Elf(linked)
    if hashlib.sha256(original.data).hexdigest() != link_context['inputs'][linked.relative_to(root).as_posix()]:
        raise MetadataProofError('encoded-address loaded canonical ELF differs from bound context')
    map_data = output_map.read_bytes()
    reproduced = tuple(rp.parse_map(output_map,map_data))
    if (metadata_link.allocated(image) != metadata_link.allocated(original)
            or metadata_link.symbols(image) != metadata_link.symbols(original)
            or reproduced != mapping):
        raise MetadataProofError('encoded-address actual ordinary link replay differs from existing ELF/map')
    held.update({path:sha256_file(path) for path in directory.iterdir() if path.is_file()})
    held[output] = hashlib.sha256(image.data).hexdigest()
    held[output_map] = hashlib.sha256(map_data).hexdigest()
    current()
    return image, output_map, reproduced, held, current


def _encoded_ast(cpp, flags, root):
    import candidate_context as lexical
    if '-32' not in flags or any(x in flags for x in ('-64', '-n32', '-n64')):
        raise MetadataProofError('encoded-address requires the actual o32 compiler ABI')
    ca, parser, pin, identity = _effective_parser(root)
    if len(cpp) > lexical.MAX_SOURCE_BYTES:
        raise MetadataProofError('encoded-address CPP exceeds the existing source bound')
    try:
        tree = parser.CParser().parse(cpp.decode())
    except Exception as error:
        raise MetadataProofError('encoded-address effective C cannot be parsed') from error
    def nodes(node):
        yield node
        for _, child in node.children():
            yield from nodes(child)
    all_nodes = list(nodes(tree))
    if any(isinstance(node, ca.Pragma) for node in all_nodes):
        raise MetadataProofError('encoded-address source contains unsupported pragmas')
    aliases = collections.defaultdict(list)
    for node in all_nodes:
        if isinstance(node, ca.Typedef):
            if node not in tree.ext:
                raise MetadataProofError('encoded-address local typedef is unsupported')
            aliases[node.name].append(node.type)
    def resolve(node, seen=()):
        if isinstance(node, ca.TypeDecl):
            if node.quals or getattr(node, 'align', None):
                raise MetadataProofError('encoded-address qualified type is unsupported')
            return resolve(node.type, seen)
        if isinstance(node, ca.IdentifierType) and len(node.names) == 1 and node.names[0] in aliases:
            name = node.names[0]
            if name in seen or len(aliases[name]) != 1:
                raise MetadataProofError('encoded-address ambiguous typedef')
            return resolve(aliases[name][0], seen + (name,))
        return node
    def words(node):
        node = resolve(node)
        return tuple(sorted(node.names)) if isinstance(node, ca.IdentifierType) else None
    if pin != literal_profile_inputs(root) or lexical.identity() != identity:
        raise MetadataProofError('encoded-address parser changed')
    return ca, tree, all_nodes, resolve, words


def encoded_word_consumer_facts(cpp, flags, root, *, function=ENCODED_WORD_FUNCTION,
                                marker=ENCODED_WORD_MARKER):
    """Close one actual unsigned32 cast/store; no general pointer-flow inference."""
    ca, tree, nodes, resolve, words = _encoded_ast(cpp, flags, root)
    declarations = [n for n in nodes if isinstance(n, ca.Decl) and n.name == marker]
    if len(declarations) != 1:
        raise MetadataProofError('encoded-address marker is missing/defined/shadowed')
    declaration = declarations[0]
    if (declaration not in tree.ext or declaration.storage != ['extern'] or declaration.init is not None
            or declaration.quals or declaration.funcspec or declaration.bitsize
            or not isinstance(declaration.type, ca.ArrayDecl) or declaration.type.dim is not None
            or declaration.type.dim_quals or words(declaration.type.type) != ('char','unsigned')):
        raise MetadataProofError('encoded-address needs one incomplete unsigned-char extern')
    functions = [n for n in tree.ext if isinstance(n, ca.FuncDef) and n.decl.name == function]
    if len(functions) != 1 or functions[0].decl.storage:
        raise MetadataProofError('encoded-address needs its GLOBAL source function')
    parents = {id(child): node for node in nodes for _, child in node.children()}
    if any(isinstance(n, ca.ID) and n.name == 'D_80000000' for n in nodes):
        raise MetadataProofError('encoded-address consumer retains old numeric carrier use')
    uses = [n for n in nodes if isinstance(n, ca.ID) and n.name == marker]
    if len(uses) != 1:
        raise MetadataProofError('encoded-address requires exactly one evaluated carrier use')
    use = uses[0]; cast = parents.get(id(use)); assignment = parents.get(id(cast))
    unsigned32 = {('int','unsigned'), ('unsigned',), ('long','unsigned'), ('int','long','unsigned')}
    if (not isinstance(cast, ca.Cast) or cast.expr is not use or words(cast.to_type.type) not in unsigned32
            or not isinstance(assignment, ca.Assignment) or assignment.op != '=' or assignment.rvalue is not cast
            or not isinstance(assignment.lvalue, ca.StructRef) or assignment.lvalue.type != '->'
            or not isinstance(assignment.lvalue.name, ca.ID) or not isinstance(assignment.lvalue.field, ca.ID)
            or assignment.lvalue.field.name != 'w1'):
        raise MetadataProofError('encoded-address use is not a direct unsigned32 field store')
    block = parents.get(id(assignment))
    if not isinstance(block, ca.Compound) or assignment not in block.block_items:
        raise MetadataProofError('encoded-address store needs its actual macro block')
    receiver = assignment.lvalue.name.name
    locals_ = [n for n in block.block_items if isinstance(n, ca.Decl) and n.name == receiver]
    if len(locals_) != 1:
        raise MetadataProofError('encoded-address receiver declaration is ambiguous')
    local = locals_[0]
    items = block.block_items
    first = items[1] if len(items) == 3 else None
    if (items[0] is not local or items[-1] is not assignment
            or not isinstance(first, ca.Assignment) or first.op != '='
            or not isinstance(first.lvalue, ca.StructRef) or first.lvalue.type != '->'
            or not isinstance(first.lvalue.name, ca.ID) or first.lvalue.name.name != receiver
            or not isinstance(first.lvalue.field, ca.ID) or first.lvalue.field.name != 'w0'):
        raise MetadataProofError('encoded-address macro is not its complete two-word store')
    if (local.storage or local.quals or not isinstance(local.type, ca.PtrDecl) or local.type.quals
            or block.block_items.index(local) >= block.block_items.index(assignment)
            or not isinstance(local.init, ca.Cast) or not isinstance(local.init.to_type.type, ca.PtrDecl)
            or local.init.to_type.type.quals):
        raise MetadataProofError('encoded-address receiver is not its initialized command pointer')
    record = resolve(local.type.type)
    if (not isinstance(record, ca.Struct) or record.name != 'Overlay73Command'
            or record is not resolve(local.init.to_type.type.type.type)
            or not record.decls or [n.name for n in record.decls] != ['w0','w1']
            or any(n.bitsize or n.storage or n.quals or words(n.type) not in unsigned32 for n in record.decls)):
        raise MetadataProofError('encoded-address actual command field is not unsigned32')
    ancestor = block
    while ancestor is not None and not isinstance(ancestor, ca.FuncDef):
        ancestor = parents.get(id(ancestor))
    if ancestor is not functions[0]:
        raise MetadataProofError('encoded-address store belongs to another source function')
    # Every same-name receiver occurrence in this macro block must be a field base.
    for n in nodes:
        if isinstance(n, ca.Decl) and n.name == receiver and n is not local:
            p = n
            while p is not None and p is not block:
                p = parents.get(id(p))
            if p is block:
                raise MetadataProofError('encoded-address receiver is shadowed')
        if isinstance(n, ca.ID) and n.name == receiver:
            p = n
            while p is not None and p is not block:
                p = parents.get(id(p))
            if p is block:
                owner = parents.get(id(n))
                if not isinstance(owner, ca.StructRef) or owner.name is not n:
                    raise MetadataProofError('encoded-address receiver escapes its macro block')
    return ('unsigned32-store-o32', function, marker, words(cast.to_type.type), words(record.decls[1].type))


def encoded_word_owner_facts(cpp, flags, root):
    ca, tree, nodes, resolve, words = _encoded_ast(cpp, flags, root)
    definitions = [n for n in nodes if isinstance(n, ca.Decl) and n.name == ENCODED_WORD_OWNER]
    if len(definitions) != 1:
        raise MetadataProofError('encoded-address owner source is missing/shadowed')
    node = definitions[0]
    if (node not in tree.ext or node.storage or node.quals or node.init is None
            or not isinstance(node.type, ca.ArrayDecl) or node.type.dim_quals
            or not isinstance(node.type.dim, ca.Constant) or node.type.dim.value != '8'):
        raise MetadataProofError('encoded-address owner is not the actual initialized triangle array')
    record = resolve(node.type.type)
    names = ['flags','vertex0','vertex1','vertex2','s0','t0','s1','t1','s2','t2']
    if (not isinstance(record, ca.Struct) or record.name != 'Overlay73Triangle' or not record.decls
            or [n.name for n in record.decls] != names
            or any(n.storage or n.bitsize or n.quals for n in record.decls)
            or any(words(n.type) != ('char','unsigned') for n in record.decls[:4])
            or any(words(n.type) not in {('short',),('short','signed'),('int','short'),('int','short','signed')}
                   for n in record.decls[4:])):
        raise MetadataProofError('encoded-address owner record widths differ from actual source')
    return 8 * (4 + 6*2)  # Target primitive widths, corroborated by original OBJECT size.


def encoded_word_pair(compiler, prepared, *, function=ENCODED_WORD_FUNCTION, marker=ENCODED_WORD_MARKER):
    import runlink_pack as rp
    if compiler.data != prepared.data:
        statics = static_anchor_module(pathlib.Path(__file__).resolve().parents[1])
        if statics.add_static_symbols(compiler.data)[0] != prepared.data:
            raise MetadataProofError('encoded-address prepared input is not the original static-symbol pass')
    for section in rp.EDITED_SECTIONS:
        if compiler.bytes_of(section) != prepared.bytes_of(section):
            raise MetadataProofError('encoded-address original allocated bytes changed')
        a,b = compiler.rels(section),prepared.rels(section)
        if ([(r.offset,r.type,r.sym.name,r.sym.bind,r.sym.type) for r in a] !=
                [(r.offset,r.type,r.sym.name,r.sym.bind,r.sym.type) for r in b]
                or rp.in_place_addends(compiler,section,a) != rp.in_place_addends(prepared,section,b)):
            raise MetadataProofError('encoded-address original compiler indices/records changed')
    symbols = [s for s in compiler.symbols if s.name == marker]
    funcs = [s for s in compiler.symbols if s.name == function]
    if (len(symbols) != 1 or symbols[0].shndx != rp.SHN_UNDEF or symbols[0].bind != rp.STB_GLOBAL
            or symbols[0].type != rp.STT_OBJECT or symbols[0].size != 0 or symbols[0].value != 0
            or len(funcs) != 1 or funcs[0].section != '.text' or funcs[0].bind != rp.STB_GLOBAL
            or funcs[0].type != rp.STT_FUNC or funcs[0].size <= 0):
        raise MetadataProofError('encoded-address original marker/function roles differ')
    refs = [(section,r,a) for section in rp.EDITED_SECTIONS[:-1]
            for r,a in zip(compiler.rels(section),rp.in_place_addends(compiler,section,compiler.rels(section)))
            if r.sym.index == symbols[0].index]
    if (len(refs) != 2 or [x[0] for x in refs] != ['.text','.text']
            or [x[1].type for x in refs] != [rp.R_MIPS_HI16,rp.R_MIPS_LO16] or any(x[2] for x in refs)):
        raise MetadataProofError('encoded-address requires its complete zero-addend text pair')
    high,low = [x[1] for x in refs]; owner = funcs[0]
    if (high.offset >= low.offset or any(r.offset%4 or not owner.value <= r.offset
            or r.offset+4 > owner.value+owner.size or r.offset+4 > compiler.size('.text') for r in (high,low))):
        raise MetadataProofError('encoded-address pair escapes original function')
    h,l = [rp.u32(compiler.bytes_of('.text'),r.offset) for r in (high,low)]
    if h>>26 != 15 or l>>26 != 9 or (h>>16)&31 != (l>>21)&31 or (h>>16)&31 != (l>>16)&31:
        raise MetadataProofError('encoded-address pair is not the complete LUI/ADDIU carrier')
    return (symbols[0].index,high.offset,low.offset,owner.value,owner.size)


@dataclasses.dataclass(frozen=True)
class EncodedAddressWordWitness:
    root: pathlib.Path
    pair: tuple
    owner_offset: int
    origin_offset: int
    owner_size: int
    compiler: object = dataclasses.field(repr=False)
    raw: object = dataclasses.field(repr=False)
    configured: object = dataclasses.field(repr=False)
    provider: object = dataclasses.field(repr=False)
    linked: object = dataclasses.field(repr=False)
    replay: object = dataclasses.field(repr=False)
    replay_mapping: tuple = dataclasses.field(repr=False)
    replay_map_path: pathlib.Path = dataclasses.field(repr=False)
    observations: tuple = dataclasses.field(repr=False)
    contexts: tuple = dataclasses.field(repr=False)
    holds: tuple = dataclasses.field(repr=False)
    mapping: tuple = dataclasses.field(repr=False)
    map_path: pathlib.Path = dataclasses.field(repr=False)
    seal: object = dataclasses.field(repr=False)

    def current(self):
        import runlink_pack as rp
        if self.seal is not _ENCODED_WORD_SEAL:
            raise MetadataProofError('untrusted encoded-address witness')
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
            raise MetadataProofError('loaded encoded-address proof changed')
        if not encoded_word_selection(self.root):
            raise MetadataProofError('encoded-address opt-in was removed')
        if any(expected != context() for expected,context in self.contexts):
            raise MetadataProofError('encoded-address fresh capture context changed')
        for obj, digest, surface in self.observations:
            if (hashlib.sha256(obj.data).hexdigest() != digest or sha256_file(obj.path) != digest
                    or _static_parsed_surface(obj) != surface):
                raise MetadataProofError('encoded-address loaded object/parsed observation changed')
        # The production CLI and imported helper have distinct MapInput classes.
        # Compare every semantic field, retaining row order and multiplicity.
        def map_rows(rows):
            return tuple((r.outsec,r.insec,r.addr,r.size,r.file) for r in rows)
        if (map_rows(rp.parse_map(self.map_path)) != map_rows(self.mapping)
                or map_rows(rp.parse_map(self.replay_map_path)) != map_rows(self.replay_mapping)):
            raise MetadataProofError('encoded-address loaded map changed')
        if any(sha256_file(path) != digest for path,digest in self.holds):
            raise MetadataProofError('encoded-address held source/expression/input changed')
        return True

    def operand(self, world, obj, section, rel, addend):
        import runlink_pack as rp
        self.current()
        rp.validate_fresh_data_input(obj.elf, self.raw)
        if (obj.module != 73 or obj.source != ENCODED_WORD_SOURCE[4:-2] or section != '.text'
                or rel.sym.name != ENCODED_WORD_MARKER or rel.sym.shndx != rp.SHN_UNDEF
                or addend or (rel.offset,rel.type) not in ((self.pair[1],rp.R_MIPS_HI16),(self.pair[2],rp.R_MIPS_LO16))):
            raise MetadataProofError('encoded-address operand is outside its complete source pair')
        # Bind the caller's loaded parse and original-index mapping as well.
        actual = [r for r in self.raw.rels(section) if (r.offset,r.type)==(rel.offset,rel.type)]
        def symbol_row(s):
            return (s.index,s.name,s.value,s.size,s.bind,s.type,s.shndx,s.section)
        if len(actual)!=1 or symbol_row(rel.sym) != symbol_row(actual[0].sym):
            raise MetadataProofError('encoded-address caller raw index/symbol differs')
        observed = getattr(world, '_encoded_word_consumers', {})
        if id(obj.elf) not in observed:
            digest = hashlib.sha256(obj.elf.data).hexdigest()
            if sha256_file(obj.path) != digest:
                raise MetadataProofError('encoded-address loaded caller differs from its file')
            surface = _static_parsed_surface(obj.elf)
            def current():
                return (hashlib.sha256(obj.elf.data).hexdigest(), sha256_file(obj.path),
                        _static_parsed_surface(obj.elf))
            world.data_import_contexts.append(((digest,digest,surface),current))
            rp.hold_data_import_inputs(world,{obj.path:digest})
            observed[id(obj.elf)] = (obj.elf,digest,surface)
            world._encoded_word_consumers = observed
        return ('ov',73,(self.owner_offset + 0x80000000)&0xffffffff)

    def runtime(self, image, records):
        import runlink_pack as rp
        self.current()
        keys = [(self.pair[1],rp.R_MIPS_HI16),(self.pair[2],rp.R_MIPS_LO16)]
        selected = [(i,row) for i,row in enumerate(records) if row[:2] in keys]
        if (len(selected)!=2 or [r[:2] for _,r in selected]!=keys or selected[1][0]!=selected[0][0]+1
                or any(r[2:] != (rp.OP_LOCAL,self.origin_offset) for _,r in selected)):
            raise MetadataProofError('encoded-address actual runtime pair/base differs')
        stored = ((rp.u32(image,self.pair[1])&0xffff)<<16) + rp.sext16(rp.u32(image,self.pair[2]))
        if (self.origin_offset+stored)&0xffffffff != (self.owner_offset+0x80000000)&0xffffffff:
            raise MetadataProofError('encoded-address decoded runtime scalar differs from real owner')
        return True


def encoded_word_readback(compiler, provider, linked, mapping, owner_size):
    """Independent loaded ELF/map roles, word decode and untouched instruction bits."""
    import runlink_pack as rp
    pair = encoded_word_pair(compiler,compiler)
    def unique(image,name):
        rows=[s for s in image.symbols if s.name==name]
        if len(rows)!=1:
            raise MetadataProofError('encoded-address symbol is missing/ambiguous: '+name)
        return rows[0]
    owner=unique(provider,ENCODED_WORD_OWNER); actual=unique(linked,ENCODED_WORD_OWNER)
    marker=unique(linked,ENCODED_WORD_MARKER); origin=unique(linked,ENCODED_WORD_ORIGIN)
    text=linked.section('.overlay_073'); data=provider.section('.data')
    if (not data or data['type']!=1 or data['flags']!=3 or data['addr']!=0
            or owner.bind!=rp.STB_GLOBAL or owner.type!=rp.STT_OBJECT or owner.section!='.data'
            or owner.size!=owner_size or owner.size<=0 or owner.value<0 or owner.value+owner.size>data['size']
            or provider.rels('.data') or any(provider.size(s) for s in ('.text','.rodata','.bss'))):
        raise MetadataProofError('encoded-address owner is not the real bounded writable C DATA')
    objects=sorted((s.value,s.value+s.size) for s in provider.symbols
                   if s.section=='.data' and s.type==rp.STT_OBJECT)
    if any(a[1]>b[0] for a,b in zip(objects,objects[1:])):
        raise MetadataProofError('encoded-address owner objects overlap')
    homes=[m for m in mapping if m.file=='build/'+ENCODED_WORD_PROVIDER+'.o' and m.insec=='.data' and m.size]
    texts=[m for m in mapping if m.outsec=='.overlay_073' and m.insec=='.text' and m.size]
    if (len(homes)!=1 or len(texts)!=1 or texts[0].file!=ENCODED_WORD_TARGET
            or not text or text['type']!=1 or text['flags']&2!=2
            or texts[0].addr!=text['addr'] or texts[0].size!=compiler.size('.text')
            or homes[0].outsec!='.overlay_073' or homes[0].size!=data['size']
            or origin.section!='.overlay_073' or origin.value!=texts[0].addr+texts[0].size
            or homes[0].addr<origin.value
            or actual.bind!=rp.STB_GLOBAL or actual.type!=rp.STT_OBJECT or actual.size!=owner.size
            or actual.section!='.overlay_073' or actual.value!=homes[0].addr+owner.value):
        raise MetadataProofError('encoded-address real owner/DATA-origin physical map differs')
    home=homes[0]
    if any(m is not home and m.size and m.outsec==home.outsec and m.addr<home.addr+home.size
           and home.addr<m.addr+m.size for m in mapping):
        raise MetadataProofError('encoded-address physical owner overlaps another input')
    if (marker.shndx!=rp.SHN_ABS or marker.bind!=rp.STB_GLOBAL or marker.type!=rp.STT_OBJECT or marker.size!=0
            or marker.value!=(actual.value-origin.value+0x80000000)&0xffffffff):
        raise MetadataProofError('encoded-address final marker role/value differs')
    image=linked.bytes_of('.overlay_073'); begin=home.addr-text['addr']
    if image[begin:begin+home.size]!=provider.bytes_of('.data'):
        raise MetadataProofError('encoded-address full original owner allocation changed')
    function=unique(compiler,ENCODED_WORD_FUNCTION); final_function=unique(linked,ENCODED_WORD_FUNCTION)
    if (final_function.bind!=rp.STB_GLOBAL or final_function.type!=rp.STT_FUNC
            or final_function.section!='.overlay_073' or final_function.size!=function.size
            or final_function.value!=text['addr']+function.value):
        raise MetadataProofError('encoded-address original GLOBAL function geometry changed')
    masks={}
    for r in compiler.rels('.text'):
        if r.offset%4 or r.offset<0 or r.offset+4>compiler.size('.text') or r.offset in masks:
            raise MetadataProofError('encoded-address original relocation word is invalid')
        if r.type not in (rp.R_MIPS_HI16,rp.R_MIPS_LO16,rp.R_MIPS_26,rp.R_MIPS_32):
            raise MetadataProofError('encoded-address unsupported original relocation type')
        masks[r.offset]={rp.R_MIPS_HI16:0xffff,rp.R_MIPS_LO16:0xffff,rp.R_MIPS_26:0x3ffffff,rp.R_MIPS_32:0xffffffff}[r.type]
    original=compiler.bytes_of('.text')
    if len(image)<len(original) or any((rp.u32(original,o)^rp.u32(image,o))&(~masks.get(o,0)&0xffffffff)
                                      for o in range(0,len(original),4)):
        raise MetadataProofError('encoded-address final nonrelocation instruction bits changed')
    stored=((rp.u32(image,pair[1])&0xffff)<<16)+rp.sext16(rp.u32(image,pair[2]))
    if stored&0xffffffff != marker.value:
        raise MetadataProofError('encoded-address final signed-low pair does not encode marker')
    return pair,actual.value-text['addr'],origin.value-text['addr']


def capture_encoded_word(world, dump=None):
    """One canonical role using the existing producer capture and held inventory."""
    import runlink_pack as rp
    import permute_batch as batch
    import postprocess_audit as audit
    root=world.paths['build'].parent
    dump=audit.run_make_database() if dump is None else dump
    if not encoded_word_selection(root,dump):
        return None
    context_start = len(world.data_import_contexts)
    statics = static_anchor_module(root)
    if rp.plain_data_targets(dump).get('build/'+ENCODED_WORD_PROVIDER+'.o')!='data':
        raise MetadataProofError('encoded-address lacks its existing ordinary DATA owner declaration')
    if ENCODED_WORD_MARKER in rp.OrtNames.load(world.paths['ort']).by_name:
        raise MetadataProofError('encoded-address marker must not be an ORT export')
    script_names=['mickey.us.ld','undefined_funcs_auto.us.txt','undefined_syms_auto.us.txt',
                  'libultra_undefined_syms.us.txt','overlay_undefined_syms.us.txt']
    link_context=producer_link_context(root,world.paths['elf'],world.paths['map'])
    if link_context['command'][3]!= 'mickey.us.ld':
        script_names[0]=metadata_link.FINAL_SCRIPT
    scripts={n:(root/n).read_text() for n in script_names}
    encoded_word_scripts(scripts)
    raw_path,plan,receipt,context=_capture_configured_raw(root,root/ENCODED_WORD_SOURCE,
        root/ENCODED_WORD_TARGET,world.paths['elf'],'',producer_map=world.paths['map'],preserve_compiler=True)
    if 'kept_rodata' not in receipt or receipt['inputs']['assembly']:
        raise MetadataProofError('encoded-address consumer lacks its current keep-only C capture')
    compiler=rp.Elf(root/receipt['compiler_original']); raw=rp.Elf(raw_path)
    configured=rp.Elf(root/ENCODED_WORD_TARGET)
    if (hashlib.sha256(compiler.data).hexdigest()!=receipt['compiler_original_sha256']
            or hashlib.sha256(raw.data).hexdigest()!=receipt['raw_sha256']
            or hashlib.sha256(configured.data).hexdigest()!=receipt['configured_sha256']):
        raise MetadataProofError('encoded-address loaded capture differs from receipts')
    pair=encoded_word_pair(compiler,raw)
    rp.validate_fresh_data_input(rp.Elf(world.paths['raw']/(ENCODED_WORD_SOURCE+'.o')),raw)
    provider_source=ENCODED_WORD_PROVIDER[4:-2]
    provider=rp.reproduce_data_provider(world,provider_source,preserve_compiler=True)
    provider_receipt=world.data_import_provider_receipts[provider_source]
    prepared_provider=provider
    provider=rp.Elf(root/provider_receipt['compiler_original'])
    if (hashlib.sha256(provider.data).hexdigest()!=provider_receipt['compiler_original_sha256']
            or hashlib.sha256(prepared_provider.data).hexdigest()!=provider_receipt['raw_sha256']
            or provider_receipt['inputs']['assembly'] or provider_receipt.get('effective_sections')
            or statics.add_static_symbols(provider.data)[0]!=prepared_provider.data):
        raise MetadataProofError('encoded-address original owner/receipt/prepared join differs')
    rp.hold_data_capture(world,ENCODED_WORD_SOURCE[4:-2],receipt,context)
    cpp_holds={}; private_artifacts=set(); facts=[]
    for source,cap in ((ENCODED_WORD_SOURCE,receipt),(ENCODED_WORD_PROVIDER,provider_receipt)):
        command=cap['inputs']['command']
        flags=_literal_flags(command,source)
        result=batch.bounded_capture(['tools/ido/cc','-E',*flags,source],time.monotonic()+120,check=True)
        cpp=result.stdout.encode(); path=root/cap['directory']/'encoded-effective.i';path.write_bytes(cpp)
        cpp_holds[path]=hashlib.sha256(cpp).hexdigest()
        private_artifacts.add(path)
        facts.append(encoded_word_consumer_facts(cpp,flags,root) if source==ENCODED_WORD_SOURCE
                     else encoded_word_owner_facts(cpp,flags,root))
    checked,owner_offset,origin_offset=encoded_word_readback(compiler,provider,world.elf,world.map,facts[1])
    if checked!=pair:
        raise MetadataProofError('encoded-address original pair changed during readback')
    replay,replay_map,replay_mapping,replay_holds,replay_current = replay_encoded_word_link(
        root,world.paths['elf'],world.paths['map'],scripts,link_context)
    if encoded_word_readback(compiler,provider,replay,replay_mapping,facts[1]) != (pair,owner_offset,origin_offset):
        raise MetadataProofError('encoded-address replayed owner/word/function roles differ')
    cpp_holds.update(replay_holds)
    private_artifacts.update(set(replay_holds) & {replay.path,replay_map,
        replay.path.parent/'inputs.json',replay.path.parent/'link.log'})
    world.data_import_contexts.append((True,replay_current))
    cpp_holds.update({root/n:hashlib.sha256(t.encode()).hexdigest() for n,t in scripts.items()})
    cpp_holds.update({root/n:d for n,d in literal_profile_inputs(root).items()})
    cpp_holds[world.paths['map']]=hashlib.sha256(world.map_data).hexdigest()
    # Fresh proof artifacts stay live-held, but are not deterministic producer inputs.
    # These exact paths came from this capture, never a prefix-based exclusion.
    private_holds={path:cpp_holds[path] for path in private_artifacts}
    def private_current():
        if any(sha256_file(path)!=digest for path,digest in private_holds.items()):
            raise MetadataProofError('encoded-address private proof artifact changed')
        return True
    private_current()
    world.data_import_contexts.append((True,private_current))
    rp.hold_data_import_inputs(world,{path:digest for path,digest in cpp_holds.items()
                                      if path not in private_artifacts})
    observations=tuple((o,hashlib.sha256(o.data).hexdigest(),_static_parsed_surface(o))
                       for o in (compiler,raw,configured,provider,prepared_provider,world.elf,replay))
    result=EncodedAddressWordWitness(root,pair,owner_offset,origin_offset,facts[1],compiler,raw,
        configured,provider,world.elf,replay,replay_mapping,replay_map,observations,
        tuple(world.data_import_contexts[context_start:]),
        tuple((dict(world.data_import_inputs)|private_holds).items()),tuple(world.map),world.paths['map'],_ENCODED_WORD_SEAL)
    result.current()
    return result


def canonical_encoded_word_capture(root):
    """Producer-only context avoids recursively asking for generated-final proof."""
    from types import SimpleNamespace
    import runlink_pack as rp
    if not encoded_word_selection(root):
        raise MetadataProofError('encoded-address canonical role is not selected')
    world=rp.World(rp.resolve_paths(SimpleNamespace(root=str(root))),need_raw=False)
    world.data_import_inputs={}; world.data_import_contexts=[]; world.data_import_provider_receipts={}
    return capture_encoded_word(world)


def encoded_word_surface(root, overlay, candidate_symbol, target_symbol, row, candidate_object, target_path):
    """Strict comparison uses the same fresh canonical role, before numeric aliases."""
    import runlink_pack as rp
    if (overlay != 73 or candidate_symbol != ENCODED_WORD_FUNCTION or target_symbol != ENCODED_WORD_FUNCTION
            or row.get("source") != ENCODED_WORD_SOURCE[4:-2]
            or pathlib.Path(target_path).resolve() != (root/"build/mickey.us.elf").resolve()):
        raise MetadataProofError("encoded-address strict proof has a different source home")
    witness = canonical_encoded_word_capture(root)
    candidate = rp.Elf(candidate_object)
    try:
        try:
            rp.validate_fresh_data_input(candidate, witness.configured)
        except rp.PackError:
            rp.validate_fresh_data_input(candidate, witness.raw)
    except rp.PackError as error:
        raise MetadataProofError("encoded-address compared input differs from fresh capture") from error
    digest = hashlib.sha256(candidate.data).hexdigest()
    observation = (digest, digest, _static_parsed_surface(candidate))
    def current():
        witness.current()
        if (sha256_file(pathlib.Path(candidate_object)), hashlib.sha256(candidate.data).hexdigest(),
                _static_parsed_surface(candidate)) != observation:
            raise MetadataProofError("encoded-address compared candidate changed before report")
    identity = (73, (witness.owner_offset+0x80000000)&0xffffffff)
    return {"identity":identity, "current":current,
        "evidence":{"underlying_owner":[73,witness.owner_offset], "owner_size":witness.owner_size,
                    "marker_allocated_bytes":0},
        "report":{"role":"source-encoded-address-word", "marker":ENCODED_WORD_MARKER,
            "function":ENCODED_WORD_FUNCTION, "owner":ENCODED_WORD_OWNER,
            "underlying_identity":[73,witness.owner_offset], "encoded_identity":list(identity),
            "owner_size":witness.owner_size, "marker_allocated_bytes":0,
            "pair":list(witness.pair[1:3]), "fresh_held_source_and_final_roles":True}}


def canonical_encoded_word_readback(root):
    """Final-link role proof cannot inherit a numeric alias or serialized receipt."""
    if not encoded_word_selection(root):
        return
    witness=canonical_encoded_word_capture(root)
    records=[]
    for table in (1,2):
        data=witness.linked.bytes_of(f'.overlay_073_reloc{table}')
        if len(data)%8:
            raise MetadataProofError('encoded-address final runtime table is truncated')
        for offset in range(0,len(data),8):
            index,bits=struct.unpack_from('>II',data,offset)
            records.append((bits>>8,(bits>>4)&15,bits&15,index))
    witness.runtime(witness.linked.bytes_of('.overlay_073'),records)
    return witness.current


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
