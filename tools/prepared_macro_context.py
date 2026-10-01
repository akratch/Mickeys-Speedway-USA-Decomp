#!/usr/bin/env python3
"""Retain compiler-preprocessed context evidence for active prepared macros."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import shutil

import candidate_context


def needs_preprocessing(report: dict) -> bool:
    return (report.get("status") == "unverifiable" and report.get("reason") ==
            "ContextError: active prepared macro requires independent preprocessing/context review")


def compare_stock(baseline, winner, symbol, directory, compiler, compiler_args, capture):
    """Use the runner's bounded subprocess helper with its validated recipe."""
    if any(arg in ("-E", "-S", "-o") or arg.startswith("-o") for arg in compiler_args):
        raise ValueError("unsupported output mode in stock preprocessing recipe")
    forbidden = candidate_context.LOCATION_MACROS | candidate_context.PRAGMA_OPERATORS
    if any("##" in arg or "%:%:" in arg or
           forbidden.intersection(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", arg)) for arg in compiler_args):
        raise ValueError("unsupported dynamic expansion in stock preprocessing recipe")
    arguments = [arg for arg in compiler_args if arg != "-c"] + ["-DNON_MATCHING", "-E"]
    compiler_hash = hashlib.sha256(compiler.read_bytes()).hexdigest()
    def preprocess(source_path, output_path):
        # Positional arguments only: separate diagnostic output from C stdout.
        argv = [str(compiler), *arguments, str(source_path)]
        command_path = source_path.with_name(source_path.stem + "-command.json")
        record = {"argv": argv, "returncode": None, "compiler_sha256": compiler_hash}
        command_path.write_text(json.dumps(record, indent=2) + "\n")
        try:
            process = capture(["bash", "-c", 'prep_output=$1; shift; exec "$@" > "$prep_output"',
                               "stock-preprocess", str(output_path), *argv])
        except BaseException as error:
            record["error"] = type(error).__name__ + ": " + str(error)
            command_path.write_text(json.dumps(record, indent=2) + "\n")
            diagnostics = getattr(error, "output", None) or getattr(error, "stderr", None) or str(error)
            output_path.with_suffix(".log").write_bytes(
                diagnostics if isinstance(diagnostics, bytes) else str(diagnostics).encode())
            raise
        output_path.with_suffix(".log").write_text(process.stdout)
        if hashlib.sha256(compiler.read_bytes()).hexdigest() != compiler_hash:
            raise ValueError("stock compiler changed during preprocessing")
        return {"argv": argv, "returncode": process.returncode, "compiler_sha256": compiler_hash}
    return compare(baseline, winner, symbol, directory, preprocess)


def retain(report: dict, directory: Path, build_permuter: Path) -> None:
    """Include successful and failed replay artifacts in the context receipt."""
    preprocessing = (report.get("comparison") or {}).get("preprocessing")
    path = preprocessing["path"] if preprocessing else report.get("preprocessing_directory")
    if path and Path(path).is_dir():
        source = Path(path)
        if source.parent != build_permuter / "macro-context" or source.is_symlink():
            raise ValueError("stock preprocessing evidence is outside its owned directory")
        shutil.copytree(source, directory / "preprocessing", dirs_exist_ok=True)


def compare(baseline: bytes, winner: bytes, symbol: str, directory: Path, preprocess) -> dict:
    """Compare only actual successful -E outputs, retaining raw input binding.

    The runner supplies its configured stock compiler invocation after validating
    the capture, recipe, dependencies and tool identities. The callback writes
    the output file and returns command/exit metadata. Macro definitions remain
    ordered context in addition to the expanded non-target C context.
    """
    directory.mkdir(parents=True, exist_ok=False)
    old = candidate_context.preprocessing_macro_context(baseline)
    new = candidate_context.preprocessing_macro_context(winner)
    if not old:
        raise ValueError("compiler preprocessing route requires a prepared macro prelude")
    if old != new:
        raise ValueError("prepared macro definition context changed")
    outputs = []
    records = []
    for label, source in (("baseline", baseline), ("winner", winner)):
        source_path = directory / (label + ".c")
        output_path = directory / (label + ".i")
        source_path.write_bytes(source)
        record = preprocess(source_path, output_path)
        (directory / (label + "-command.json")).write_text(json.dumps(record, indent=2) + "\n")
        if type(record.get("returncode")) is not int or record["returncode"] != 0:
            raise ValueError("stock preprocessing failed for " + label)
        if source_path.read_bytes() != source:
            raise ValueError("prepared input changed during stock preprocessing")
        if not output_path.is_file() or output_path.is_symlink():
            raise ValueError("stock preprocessing did not produce an owned output")
        if output_path.stat().st_size > candidate_context.MAX_SOURCE_BYTES:
            raise ValueError("stock preprocessing output exceeds comparison byte limit")
        expanded = output_path.read_bytes()
        if not expanded:
            raise ValueError("stock preprocessing produced an empty output")
        record.update(input_sha256=hashlib.sha256(source).hexdigest(),
                      output_sha256=hashlib.sha256(expanded).hexdigest())
        records.append(record)
        outputs.append(expanded)
    expanded_report = candidate_context.compare_context(*outputs, symbol)
    (directory / "expanded-comparison.json").write_text(json.dumps(expanded_report, indent=2) + "\n")
    metadata = {"schema": "mickey-stock-preprocessed-context-v1", "path": str(directory),
                "macros": old, "commands": records, "expanded_comparison": expanded_report}
    (directory / "preprocessing.json").write_text(json.dumps(metadata, indent=2) + "\n")
    report = dict(expanded_report)
    report.update(baseline_sha256=hashlib.sha256(baseline).hexdigest(),
                  winner_sha256=hashlib.sha256(winner).hexdigest(), preprocessing=metadata)
    return report
