#!/usr/bin/env python3
"""Fail-closed per-TU impact analysis for one compiler-flag change.

The command resolves a seed NON_MATCHING symbol to its owning translation
unit, compiles that complete TU with its configured flags and with one explicit
flag delta, then scores *every* guarded candidate in the TU against its own
canonical target.  It prints counts and offsets only, never instruction words.

Example:
    tools/tu_flag_impact.py func_80048080 --add-flags=-Wab,-r4300_mul

The report is diagnostic.  Its scores use flag_sweep.py's relocation-masked
unlinked-object comparison and cannot promote a function or justify a Make
policy change by themselves.
"""

from __future__ import annotations

import argparse
import dataclasses
import json
import re
import shlex
import signal
import subprocess
import sys
import time
from pathlib import Path
from typing import Callable, Sequence


REPO = Path(__file__).resolve().parent.parent
TOOLS = REPO / "tools"
sys.path.insert(0, str(TOOLS))

import flag_sweep as fs  # noqa: E402
import function_preflight as fp  # noqa: E402
import permute_batch as pb  # noqa: E402


class ImpactError(RuntimeError):
    """The requested TU-wide comparison cannot be proved completely."""


class ImpactTimeout(ImpactError):
    """The command exhausted its outer wall-clock budget."""


@dataclasses.dataclass(frozen=True)
class Consumer:
    candidate_symbol: str
    target_symbol: str
    target_asm: Path


@dataclasses.dataclass(frozen=True)
class ImpactRow:
    candidate_symbol: str
    target_symbol: str
    baseline: fs.Score
    trial: fs.Score
    changed_words: int
    target_relocations: int
    baseline_relocations: int
    trial_relocations: int
    verdict: str


@dataclasses.dataclass(frozen=True)
class ImpactReport:
    seed_symbol: str
    source: str
    baseline_flags: tuple[str, ...]
    trial_flags: tuple[str, ...]
    consumers: tuple[ImpactRow, ...]
    cache: str
    compiled: int


_SIMPLE_CODEGEN = re.compile(
    r"(?:-O[0-3]|-g[0-9]?|-mips[123]|-(?:32|n32|64)|"
    r"-W(?:o|ab),[A-Za-z0-9_+.,=-]+)"
)
_WOFF_VALUE = re.compile(r"[0-9]+(?:,[0-9]+)*")
_SAFE_SYMBOL = re.compile(r"[^A-Za-z0-9_.-]+")


def _display(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def _parse_flag_values(values: Sequence[str], label: str) -> tuple[str, ...]:
    tokens: list[str] = []
    for value in values:
        try:
            tokens.extend(shlex.split(value))
        except ValueError as error:
            raise ImpactError(f"invalid {label}: {error}") from error
    index = 0
    while index < len(tokens):
        token = tokens[index]
        if token == "-woff":
            if index + 1 >= len(tokens) or not _WOFF_VALUE.fullmatch(tokens[index + 1]):
                raise ImpactError(f"{label} -woff requires a comma-separated numeric value")
            index += 2
            continue
        if not _SIMPLE_CODEGEN.fullmatch(token):
            raise ImpactError(
                f"unsupported {label} token {token!r}; only code-generation flags are allowed"
            )
        index += 1
    return tuple(tokens)


def _exclusive_family(token: str) -> str | None:
    if re.fullmatch(r"-O[0-3]", token):
        return "optimization"
    if re.fullmatch(r"-mips[123]", token):
        return "ISA"
    if token in {"-32", "-n32", "-64"}:
        return "ABI"
    if re.fullmatch(r"-g[0-9]?", token):
        return "debug"
    if token.startswith("-Wo,-loopunroll,"):
        return "loop-unroll"
    return None


def apply_flag_delta(
    baseline: Sequence[str], add: Sequence[str], remove: Sequence[str]
) -> tuple[str, ...]:
    """Apply an exact-token delta and reject ambiguous compiler families."""
    current = list(baseline)
    for token in remove:
        if token not in current:
            raise ImpactError(f"cannot remove absent configured flag {token!r}")
        current.remove(token)
    for token in add:
        if token in current:
            raise ImpactError(f"trial already contains flag {token!r}")
        current.append(token)
    if tuple(current) == tuple(baseline):
        raise ImpactError("flag delta has no effect")

    families: dict[str, list[str]] = {}
    for token in current:
        family = _exclusive_family(token)
        if family:
            families.setdefault(family, []).append(token)
    conflicts = {family: flags for family, flags in families.items() if len(flags) > 1}
    if conflicts:
        rendered = "; ".join(
            f"{family}={','.join(flags)}" for family, flags in sorted(conflicts.items())
        )
        raise ImpactError(
            f"ambiguous trial flag families ({rendered}); remove the configured flag first"
        )
    return tuple(current)


def _fallback_path(value: str, root: Path) -> Path:
    path = (root / value).resolve()
    expected = (root / "asm" / "nonmatchings").resolve()
    try:
        path.relative_to(expected)
    except ValueError as error:
        raise ImpactError(f"fallback escapes asm/nonmatchings: {value}") from error
    if not path.is_file():
        raise ImpactError(f"missing fallback assembly: {_display(path)}")
    return path


def collect_consumers(
    seed: fp.Resolution,
    *,
    root: Path = REPO,
    resolver: Callable[[str], fp.Resolution] = fp.resolve,
) -> tuple[Consumer, ...]:
    """Enumerate and prove every complete guarded function in the owning TU."""
    source = seed.source.resolve()
    if seed.candidate_build_dir != "build_non_matching":
        raise ImpactError(
            f"{seed.requested_symbol!r} is not selected from a NON_MATCHING build"
        )
    text = source.read_text(encoding="utf-8", errors="replace")
    consumers: list[Consumer] = []
    seen_candidates: set[str] = set()
    seen_targets: set[str] = set()

    for block in pb.iter_nonmatching_blocks(text):
        candidate = pb.block_function_name(text, block)
        fallbacks = pb.GLOBAL_ASM_RE.findall(block.fallback)
        if candidate is None:
            if fallbacks:
                raise ImpactError(
                    f"GLOBAL_ASM fallback has no unique candidate definition in {_display(source)}"
                )
            # Balanced declaration-only guards are not compiler-policy consumers.
            continue
        if len(fallbacks) != 1:
            raise ImpactError(
                f"{candidate} has {len(fallbacks)} GLOBAL_ASM fallbacks; expected exactly one"
            )
        fallback = _fallback_path(fallbacks[0], root)
        try:
            resolved = resolver(candidate)
        except (fp.PreflightError, OSError) as error:
            raise ImpactError(f"cannot resolve {candidate}: {error}") from error
        if resolved.source.resolve() != source:
            raise ImpactError(
                f"{candidate} resolves to {_display(resolved.source)}, not owning TU {_display(source)}"
            )
        if resolved.candidate_build_dir != "build_non_matching":
            raise ImpactError(f"{candidate} does not select guarded C under -DNON_MATCHING")
        if resolved.target_asm is None or resolved.target_asm.resolve() != fallback:
            actual = "none" if resolved.target_asm is None else _display(resolved.target_asm)
            raise ImpactError(
                f"{candidate} fallback identity disagrees: source={_display(fallback)}, preflight={actual}"
            )
        if resolved.candidate_symbol in seen_candidates:
            raise ImpactError(f"duplicate candidate symbol {resolved.candidate_symbol}")
        if resolved.target_symbol in seen_targets:
            raise ImpactError(f"duplicate target identity {resolved.target_symbol}")
        seen_candidates.add(resolved.candidate_symbol)
        seen_targets.add(resolved.target_symbol)
        consumers.append(
            Consumer(resolved.candidate_symbol, resolved.target_symbol, fallback)
        )

    if not consumers:
        raise ImpactError(f"{_display(source)} has no complete NON_MATCHING consumers")
    if seed.candidate_symbol not in seen_candidates:
        raise ImpactError(
            f"seed candidate {seed.candidate_symbol} is not one of the owning TU's guarded consumers"
        )
    return tuple(consumers)


def configured_recipe(source: Path) -> pb.BuildRecipe:
    """Recover the real TU recipe; static fallback and postprocessing fail closed."""
    try:
        recipe = pb.build_recipe_for(source)
    except (OSError, subprocess.SubprocessError, ValueError) as error:
        raise ImpactError(f"cannot recover configured compile recipe: {error}") from error
    if not recipe.from_dry_run:
        raise ImpactError("gmake dry-run did not prove the configured code-generation flags")
    if not recipe.flags:
        raise ImpactError("configured compile recipe has no code-generation flags")
    if recipe.objcopy_steps or recipe.skipped_postproc:
        raise ImpactError(
            "owning TU has post-compile policy that this diagnostic cannot reproduce; "
            "refusing a pre-postprocess impact report"
        )
    return recipe


def _prepare_cache(
    source: Path,
    baseline: fs.Combo,
    trial: fs.Combo,
    recipe: pb.BuildRecipe,
) -> tuple[Path, str]:
    try:
        key, manifest = fs.compilation_cache_identity(
            source, ("NON_MATCHING",), (baseline, trial), recipe
        )
    except LookupError as error:
        raise ImpactError(str(error)) from error
    cache = REPO / "build" / "tu_flag_impact" / "cache" / key
    cache.mkdir(parents=True, exist_ok=True)
    manifest_path = cache / "manifest.json"
    encoded = json.dumps(manifest, sort_keys=True, indent=2) + "\n"
    if manifest_path.is_file() and manifest_path.read_text() != encoded:
        raise ImpactError("cache manifest disagrees with its content key")
    manifest_path.write_text(encoded)
    return cache, key


def compile_variants(
    source: Path,
    recipe: pb.BuildRecipe,
    trial_flags: Sequence[str],
    *,
    rescore: bool,
) -> tuple[fs.CompileResult, fs.CompileResult, Path, int]:
    baseline = fs.Combo("configured", (), (), tuple(recipe.flags))
    trial = fs.Combo("trial", (), (), tuple(trial_flags))
    cache, _key = _prepare_cache(source, baseline, trial, recipe)
    results: list[fs.CompileResult] = []
    compiled = 0
    for combo in (baseline, trial):
        outdir = cache / combo.id
        result = fs.load_cached_result(combo, outdir)
        if result is None:
            if rescore:
                raise ImpactError(
                    f"cache is incomplete for {combo.id}; rerun without --rescore"
                )
            result = fs.compile_combo(
                source, combo, outdir, ("NON_MATCHING",),
                fs.lattice_base_arguments(recipe),
            )
            fs.write_cached_result(result, outdir)
            compiled += 1
        if not result.ok or result.obj_path is None:
            detail = result.error.strip().splitlines()
            raise ImpactError(
                f"{combo.id} TU compile failed: {detail[-1] if detail else 'unknown error'}"
            )
        results.append(result)
    return results[0], results[1], cache, compiled


def _verdict(baseline: fs.Score, trial: fs.Score, changed_words: int) -> str:
    before = fs.rank_key(baseline)
    after = fs.rank_key(trial)
    if after < before:
        return "improved"
    if after > before:
        return "regressed"
    return "score-tie/changed" if changed_words else "unchanged"


def score_consumer(
    consumer: Consumer,
    source: Path,
    baseline_object: Path,
    trial_object: Path,
    workdir: Path,
    elf_path: Path,
) -> ImpactRow:
    workdir.mkdir(parents=True, exist_ok=True)
    target_dir = workdir / "target"
    baseline_dir = workdir / "configured"
    trial_dir = workdir / "trial"
    target_dir.mkdir(exist_ok=True)
    baseline_dir.mkdir(exist_ok=True)
    trial_dir.mkdir(exist_ok=True)
    try:
        target_words, target_relocs, _mode, _owner = fs.resolve_target(
            source,
            consumer.target_symbol,
            consumer.target_asm,
            target_dir,
            elf_path,
        )
        baseline_words, baseline_relocs, _ = fs.get_candidate(
            baseline_object, consumer.candidate_symbol, baseline_dir
        )
        trial_words, trial_relocs, _ = fs.get_candidate(
            trial_object, consumer.candidate_symbol, trial_dir
        )
    except (LookupError, OSError, subprocess.SubprocessError) as error:
        raise ImpactError(f"cannot extract {consumer.candidate_symbol}: {error}") from error

    baseline_masks = dict(target_relocs)
    for index, mask in baseline_relocs.items():
        baseline_masks[index] = baseline_masks.get(index, 0) | mask
    trial_masks = dict(target_relocs)
    for index, mask in trial_relocs.items():
        trial_masks[index] = trial_masks.get(index, 0) | mask

    before = fs.score_words(target_words, baseline_words, baseline_masks)
    after = fs.score_words(target_words, trial_words, trial_masks)
    changed = fs.score_words(baseline_words, trial_words).diff_words
    return ImpactRow(
        candidate_symbol=consumer.candidate_symbol,
        target_symbol=consumer.target_symbol,
        baseline=before,
        trial=after,
        changed_words=changed,
        target_relocations=len(target_relocs),
        baseline_relocations=len(baseline_relocs),
        trial_relocations=len(trial_relocs),
        verdict=_verdict(before, after, changed),
    )


def score_all(
    consumers: Sequence[Consumer],
    scorer: Callable[[Consumer, int], ImpactRow],
) -> tuple[ImpactRow, ...]:
    """Collect all rows before rendering; one failed consumer yields no report."""
    rows: list[ImpactRow] = []
    for index, consumer in enumerate(consumers):
        rows.append(scorer(consumer, index))
    if len(rows) != len(consumers):  # defensive against future filtered collectors
        raise ImpactError("consumer report is incomplete")
    return tuple(rows)


def run_analysis(args: argparse.Namespace) -> ImpactReport:
    try:
        seed = fp.resolve(args.symbol)
    except (fp.PreflightError, OSError) as error:
        raise ImpactError(f"cannot resolve seed {args.symbol!r}: {error}") from error
    consumers = collect_consumers(seed)
    recipe = configured_recipe(seed.source)
    add = _parse_flag_values(args.add_flags, "--add-flags")
    remove = _parse_flag_values(args.remove_flags, "--remove-flags")
    trial_flags = apply_flag_delta(recipe.flags, add, remove)
    baseline, trial, cache, compiled = compile_variants(
        seed.source, recipe, trial_flags, rescore=args.rescore
    )
    work_root = cache / "analysis"
    elf_path = fs.repo_cli_path(args.elf)

    def scorer(consumer: Consumer, index: int) -> ImpactRow:
        safe = _SAFE_SYMBOL.sub("_", consumer.candidate_symbol)
        return score_consumer(
            consumer,
            seed.source,
            baseline.obj_path,
            trial.obj_path,
            work_root / f"{index:03d}-{safe}",
            elf_path,
        )

    rows = score_all(consumers, scorer)
    return ImpactReport(
        seed_symbol=args.symbol,
        source=_display(seed.source),
        baseline_flags=tuple(recipe.flags),
        trial_flags=trial_flags,
        consumers=rows,
        cache=_display(cache),
        compiled=compiled,
    )


def _score_text(score: fs.Score) -> str:
    first = "-" if score.first_mismatch is None else f"+0x{score.first_mismatch:X}"
    return f"{score.diff_words}/{score.size_delta:+d}/{first}"


def _report_dict(report: ImpactReport) -> dict[str, object]:
    return {
        "schema": 1,
        "seed_symbol": report.seed_symbol,
        "source": report.source,
        "baseline_flags": list(report.baseline_flags),
        "trial_flags": list(report.trial_flags),
        "cache": report.cache,
        "compiled_variants": report.compiled,
        "consumer_count": len(report.consumers),
        "consumers": [
            {
                "candidate_symbol": row.candidate_symbol,
                "target_symbol": row.target_symbol,
                "baseline": dataclasses.asdict(row.baseline),
                "trial": dataclasses.asdict(row.trial),
                "changed_words": row.changed_words,
                "relocation_sites": {
                    "target": row.target_relocations,
                    "baseline": row.baseline_relocations,
                    "trial": row.trial_relocations,
                },
                "verdict": row.verdict,
            }
            for row in report.consumers
        ],
    }


def render_human(report: ImpactReport) -> None:
    print(f"TU: {report.source}")
    print(f"seed: {report.seed_symbol}")
    print(f"configured: {' '.join(report.baseline_flags)}")
    print(f"trial:      {' '.join(report.trial_flags)}")
    print(f"consumers proved: {len(report.consumers)}/{len(report.consumers)}")
    print()
    print(
        f"{'candidate':<34} {'base d/s/first':>18} {'trial d/s/first':>18} "
        f"{'changed':>8} {'reloc t/b/v':>11}  verdict"
    )
    for row in report.consumers:
        relocs = (
            f"{row.target_relocations}/{row.baseline_relocations}/"
            f"{row.trial_relocations}"
        )
        print(
            f"{row.candidate_symbol:<34} {_score_text(row.baseline):>18} "
            f"{_score_text(row.trial):>18} {row.changed_words:>8} "
            f"{relocs:>11}  {row.verdict}"
        )
    counts: dict[str, int] = {}
    for row in report.consumers:
        counts[row.verdict] = counts.get(row.verdict, 0) + 1
    new_exact = sum(row.trial.exact and not row.baseline.exact for row in report.consumers)
    lost_exact = sum(row.baseline.exact and not row.trial.exact for row in report.consumers)
    summary = ", ".join(f"{key}={counts[key]}" for key in sorted(counts))
    print()
    print(f"summary: {summary}; new masked-exact={new_exact}; lost masked-exact={lost_exact}")
    print(f"compiled variants: {report.compiled}; cache: {report.cache}")
    print("verdict: diagnostic only; confirm any candidate with canonical linked-ROM proof")


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("symbol", help="one guarded candidate in the owning TU")
    parser.add_argument(
        "--add-flags",
        action="append",
        default=[],
        metavar="FLAGS",
        help="quoted code-generation flags to append; repeatable (use = before leading -)",
    )
    parser.add_argument(
        "--remove-flags",
        action="append",
        default=[],
        metavar="FLAGS",
        help="quoted configured flags to remove before additions; repeatable",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=float,
        default=120.0,
        help="hard outer wall-clock cap for resolution, two builds, and all scoring (default: 120)",
    )
    parser.add_argument(
        "--rescore",
        action="store_true",
        help="require and reuse a complete content-addressed two-object cache",
    )
    parser.add_argument("--json", action="store_true", help="emit one complete JSON report")
    parser.add_argument(
        "--elf",
        type=Path,
        default=REPO / "build" / "mickey.us.elf",
        help="linked ELF used by target ownership resolution",
    )
    args = parser.parse_args(argv)
    if not args.add_flags and not args.remove_flags:
        parser.error("at least one --add-flags or --remove-flags delta is required")
    if not 1.0 <= args.timeout_seconds <= 600.0:
        parser.error("--timeout-seconds must be between 1 and 600")
    return args


def main(argv: Sequence[str] | None = None) -> int:
    args = _parse_args(list(argv) if argv is not None else sys.argv[1:])
    started = time.monotonic()

    def expire(_signum: int, _frame: object) -> None:
        raise ImpactTimeout(
            f"outer wall-clock cap expired after {args.timeout_seconds:g} seconds"
        )

    previous = signal.signal(signal.SIGALRM, expire)
    signal.setitimer(signal.ITIMER_REAL, args.timeout_seconds)
    try:
        report = run_analysis(args)
    except ImpactTimeout as error:
        print(f"tu_flag_impact: {error}; no report emitted", file=sys.stderr)
        return 124
    except ImpactError as error:
        print(f"tu_flag_impact: {error}; no report emitted", file=sys.stderr)
        return 2
    finally:
        signal.setitimer(signal.ITIMER_REAL, 0)
        signal.signal(signal.SIGALRM, previous)

    if args.json:
        print(json.dumps(_report_dict(report), sort_keys=True, indent=2))
    else:
        render_human(report)
        print(f"elapsed: {time.monotonic() - started:.1f}s")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
