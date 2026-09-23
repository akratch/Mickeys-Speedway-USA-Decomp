#!/usr/bin/env bash
# One decomp-workbench comparison for one function.
#
#   ./tools/wb_compare.sh [--diagnose] [--summary-json] [--no-build] <symbol> [extra workbench args...]
#
# Friendly/generated aliases, the owning TU, and the canonical versus
# NON_MATCHING candidate tree are resolved automatically by
# function_preflight.py. The WB_CANDIDATE_BUILD_DIR and WB_CANDIDATE_SYMBOL
# environment overrides remain available for deliberately copied scratch
# objects; ordinary use should not need them.
#
# The target object is the project's own disassembly of the function
# (asm/nonmatchings/**/<symbol>.s) assembled with the project's assembler; the
# candidate is the function as it came out of the normal, full-translation-unit
# C build. That pairing is the one the workbench documents: no isolation
# harness, so the candidate's codegen is the codegen that actually ships.
#
# Note the ordering constraint this creates. splat stops emitting a function's
# nonmatchings .s the moment the C file implements it, so the target object can
# only be built while the function is still unmatched. Run this *during*
# iteration, not after; `--rom` below is the fallback for a function that has
# already matched.
#
#   ./tools/wb_compare.sh --rom <symbol> [args...]
#
# compares the baserom's bytes against the built ROM's bytes over the symbol's
# address range instead. Fully relocated on both sides, so it can only ever
# report instruction-words-identical or a real difference -- useful as a final
# oracle, useless for diagnosing relocation questions. By default this forces
# only the cheap ELF -> BIN -> ROM derivation so a same-timestamp stale ROM
# cannot masquerade as current. --no-build instead requires the ROM to be
# strictly newer than its linked ELF.
#
# Build provenance: decomp-workbench's scoring commands (compare,
# compare-dumps, score, diagnose, diagnose-dumps, rank) only claim `match` on
# an exact result when the build is declared `stock`. This wrapper declares
# `--build-env stock` automatically on every workbench invocation, unless:
#   - the caller's own workbench args already spell --build-env or
#     --build-env-file (their declaration wins, whatever it says);
#   - WB_CANDIDATE_BUILD_DIR or WB_CANDIDATE_SYMBOL is set, because those
#     point the comparison at a scratch object the caller must declare the
#     provenance of themselves; or
#   - the shell has any CDX_* or DKWB_* variable exported and gave no
#     declaration -- that shell may hold a forced or traced build, in which
#     case the wrapper refuses (exit 2) rather than silently claiming stock.
#     Pass --build-env forced (or --build-env-file) explicitly to proceed.
set -euo pipefail
cd "$(dirname "$0")/.."

WB=.venv/bin/decomp-workbench
OBJDUMP=tools/binutils/mips64-elf-objdump
OBJCOPY=tools/binutils/mips64-elf-objcopy
AS=tools/binutils/mips64-elf-as
ASFLAGS="-march=vr4300 -32 -mabi=32 -G0 -I include"
OUT=build/wb
mkdir -p "$OUT"
PROVENANCE=.venv/bin/python
PROVENANCE_TOOL=tools/proof_provenance.py
PREFLIGHT_TOOL=tools/function_preflight.py

mode=asm
wb_command=compare
no_build=0
summary_json=0
while [ $# -gt 0 ]; do
    case "$1" in
        --rom) mode=rom; shift ;;
        --diagnose) wb_command=diagnose; shift ;;
        --summary-json) summary_json=1; shift ;;
        --no-build) no_build=1; shift ;;
        --) shift; break ;;
        *) break ;;
    esac
done
if [ $# -lt 1 ]; then
    echo "usage: $0 [--rom] [--diagnose] [--summary-json] [--no-build] <symbol> [workbench args...]" >&2
    exit 2
fi
if [ "$summary_json" -eq 1 ] && [ "$wb_command" = diagnose ]; then
    echo "$0: --summary-json summarizes comparison evidence, not diagnosis reports." >&2
    exit 2
fi
sym=$1; shift
target_sym=$sym
candidate_sym=${WB_CANDIDATE_SYMBOL:-$sym}
source_path=
tu=
cand_build_dir=${WB_CANDIDATE_BUILD_DIR:-build}
asmfile=
boundary_evidence=extracted-fallback-symbol
target_boundary_size=

if [ "$mode" = asm ]; then
    preflight_args=("$sym" --resolve-wb)
    if [ "$no_build" -eq 1 ]; then preflight_args+=(--no-build); fi
    resolution=$($PROVENANCE "$PREFLIGHT_TOOL" "${preflight_args[@]}") || exit $?
    IFS=$'\t' read -r resolved_target resolved_candidate resolved_source \
        resolved_tu resolved_build_dir resolved_asm <<< "$resolution"
    target_sym=$resolved_target
    candidate_sym=${WB_CANDIDATE_SYMBOL:-$resolved_candidate}
    source_path=$resolved_source
    tu=$resolved_tu
    cand_build_dir=${WB_CANDIDATE_BUILD_DIR:-$resolved_build_dir}
    asmfile=$resolved_asm
fi

# ROM mode replaces this with the uniquely resolved linked symbol returned by
# the boundary preflight. Assembly mode compares the extracted target directly.
linked_sym=$sym

proof_manifest="$OUT/$sym.provenance.json"

run_provenance() {
    local result=0
    # The comparator owns stdout. Keep provenance receipts visible on stderr
    # so callers requesting --json receive exactly one parseable JSON value.
    "$PROVENANCE" "$PROVENANCE_TOOL" "$@" \
        --manifest "$proof_manifest" >&2 || result=$?
    case "$result" in
        0) return 0 ;;
        3)
            # Keep fallback/unknown comparisons useful as diagnostics, but
            # make the comparator itself reject an exact result.  The census
            # is part of decomp-workbench's public CLI and composes with any
            # caller-provided assertions.
            set -- "${wb_extra_args[@]}" --census exact=false
            wb_extra_args=("$@")
            return 0
            ;;
        *) return "$result" ;;
    esac
}

wb_extra_args=("$@")

# Decide whether to append --build-env stock to a workbench invocation. See
# the "Build provenance" note in the header comment for the guard rules.
# Sets the global build_env_reply array (empty when declaration is left to
# the caller); exits 2 with a one-line refusal under a forcing/tracing
# environment that declared nothing.
compute_build_env_args() {
    local arg
    for arg in "$@"; do
        case "$arg" in
            --build-env|--build-env=*|--build-env-file|--build-env-file=*)
                return 0
                ;;
        esac
    done
    if [ -n "${WB_CANDIDATE_BUILD_DIR:-}" ] || [ -n "${WB_CANDIDATE_SYMBOL:-}" ]; then
        return 0
    fi
    local forcing=() var
    while IFS= read -r var; do
        case "$var" in
            CDX_*|DKWB_*) forcing+=("$var") ;;
        esac
    done < <(compgen -e)
    if [ "${#forcing[@]}" -gt 0 ]; then
        echo "$0: refusing to declare --build-env stock: ${forcing[*]} exported" \
             "with no --build-env/--build-env-file given; pass --build-env forced" \
             "(or --build-env-file) explicitly." >&2
        exit 2
    fi
    build_env_reply=(--build-env stock)
}

run_workbench() {
    local command=$1 raw_report result=0
    shift
    local build_env_reply=()
    compute_build_env_args "$@"
    if [ "$summary_json" -eq 0 ]; then
        exec "$WB" "$command" "$@" "${build_env_reply[@]}"
    fi

    raw_report="$OUT/$sym.workbench.json"
    "$WB" "$command" "$@" "${build_env_reply[@]}" --json --color never > "$raw_report" || result=$?
    summary_size=${target_boundary_size:--1}
    if ! "$PROVENANCE" -c '
import json
import pathlib
import sys
sys.path.insert(0, "tools")
import function_preflight as fp
try:
    report = fp.workbench_summary(
        pathlib.Path(sys.argv[1]),
        pathlib.Path(sys.argv[2]),
        requested_symbol=sys.argv[3],
        target_symbol=sys.argv[4],
        candidate_symbol=sys.argv[5],
        comparison_mode=sys.argv[6],
        boundary_evidence=sys.argv[7],
        boundary_size=None if sys.argv[8] == "-1" else int(sys.argv[8]),
    )
except (ValueError, fp.PreflightError) as error:
    print(f"wb_compare.sh: cannot summarize workbench evidence: {error}", file=sys.stderr)
    raise SystemExit(2)
print(json.dumps(report, sort_keys=True, separators=(",", ":")))
' "$raw_report" "$proof_manifest" "$sym" "$target_sym" "$candidate_sym" \
        "$mode" "$boundary_evidence" "$summary_size"
    then
        echo "$0: full report retained at $raw_report" >&2
        return 2
    fi
    return "$result"
}

if [ "$mode" = rom ]; then
    preflight_args=("$sym" --resolve-rom)
    if [ "$no_build" -eq 1 ]; then preflight_args+=(--no-build); fi
    resolution=$($PROVENANCE "$PREFLIGHT_TOOL" "${preflight_args[@]}") || exit $?
    IFS=$'\t' read -r linked_sym vram_hex size_hex section boundary_evidence \
        <<< "$resolution"
    if [ -z "$linked_sym" ] || [ -z "$vram_hex" ] || [ -z "$size_hex" ] \
        || [ -z "$section" ] || [ -z "$boundary_evidence" ]; then
        echo "$0: preflight returned an incomplete linked boundary for '$sym'." >&2
        exit 2
    fi
    vram=$(( 0x$vram_hex ))
    size=$(( 0x$size_hex ))
    target_boundary_size=$size
    rom_build_dir=${WB_ROM_BUILD_DIR:-build}
    candidate_elf=$rom_build_dir/mickey.us.elf
    candidate_rom=$rom_build_dir/mickey.us.z64
    if [ ! -f "$candidate_elf" ]; then
        echo "$0: no linked ELF under '$rom_build_dir' -- run gmake first." >&2
        exit 1
    fi
    if [ "$no_build" -eq 0 ]; then
        # GNU Make can miss this edge when the link and prior ROM happen in the
        # same filesystem timestamp tick.  --what-if marks only the linked ELF
        # as newly changed, forcing the cheap ELF -> BIN -> ROM derivation
        # without recompiling or relinking the project.
        # Build diagnostics are not comparator output; in particular they
        # must not prefix decomp-workbench's JSON response.
        nice -n 10 gmake -j2 --no-print-directory \
            -W "$candidate_elf" "$candidate_rom" >&2
    elif [ ! -f "$candidate_rom" ] || [ ! "$candidate_rom" -nt "$candidate_elf" ]; then
        echo "$0: '$candidate_rom' is not strictly newer than '$candidate_elf';" >&2
        echo "  omit --no-build to refresh the ROM proof artifact." >&2
        exit 2
    fi

    read -r candidate_vram_hex candidate_size_hex candidate_section < <(
        "$OBJDUMP" -t "$candidate_elf" \
        | awk -v s="$sym" '$NF == s && NF >= 5 { print $1, $(NF-1), $(NF-2) }' \
        | head -1
    )
    candidate_vram=$(( 0x${candidate_vram_hex:-0} ))
    candidate_size=$(( 0x${candidate_size_hex:-0} ))
    if [ "$candidate_size" -eq 0 ]; then
        echo "$0: '$sym' not found in $candidate_elf" >&2
        exit 1
    fi

    # A section's LMA is its ROM position and its VMA is its runtime address.
    # Their delta handles both resident code and overlays, whose shared load
    # VMA cannot use the resident segment's fixed ROM bias.
    symbol_rom() {
        local elf=$1 symbol_vram=$2 symbol_section=$3
        local section_vram_hex section_rom_hex section_vram section_rom
        read -r section_vram_hex section_rom_hex < <(
            "$OBJDUMP" -h "$elf" \
            | awk -v s="$symbol_section" '$2 == s { print $4, $5; exit }'
        )
        if [ -z "${section_vram_hex:-}" ] || [ -z "${section_rom_hex:-}" ]; then
            echo "$0: section '$symbol_section' not found in $elf" >&2
            return 1
        fi
        section_vram=$(( 0x$section_vram_hex ))
        section_rom=$(( 0x$section_rom_hex ))
        echo $(( section_rom + symbol_vram - section_vram ))
    }
    target_rom=$(symbol_rom build/mickey.us.elf "$vram" "$section")
    candidate_rom_offset=$(symbol_rom "$candidate_elf" "$candidate_vram" "$candidate_section")

    # The target size is the ownership-checked preflight boundary.  An
    # optional symbol-table size was already required to agree with it; a
    # missing annotation is therefore safe and no candidate length is ever
    # substituted for the target's.
    tsize=$size
    # `set --` here would eat the caller's extra workbench arguments, so the
    # two dumps are written by name rather than by looping over pairs.
    dump() {
        local input=$1 output=$2 dump_vram=$3 dump_rom=$4 dump_size=$5
        "$OBJDUMP" -D -b binary -m mips:4300 -EB \
            --adjust-vma=$(( dump_vram - dump_rom )) \
            --start-address=$(printf 0x%x "$dump_vram") \
            --stop-address=$(printf 0x%x $(( dump_vram + dump_size ))) \
            "$input" > "$output"
    }
    # --start/--stop-address are read in the *adjusted* space, so they are VRAM
    # addresses here, not ROM offsets. Passing ROM offsets silently produces an
    # empty dump, which the workbench then rejects for having no instructions.
    dump baseroms/mickey.us.z64 "$OUT/$sym.target.objdump" \
        "$vram" "$target_rom" "$tsize"
    dump "$candidate_rom" "$OUT/$sym.candidate.objdump" \
        "$candidate_vram" "$candidate_rom_offset" "$candidate_size"
    run_provenance \
        --mode rom \
        --symbol "$sym" \
        --candidate-symbol "$sym" \
        --candidate-build-dir "$rom_build_dir" \
        --candidate-artifact "$OUT/$sym.candidate.objdump" \
        --target-artifact "$OUT/$sym.target.objdump" \
        --objdump "$OBJDUMP"
    rom_command=compare-dumps
    if [ "$wb_command" = diagnose ]; then rom_command=diagnose-dumps; fi
    run_workbench "$rom_command" "$OUT/$sym.target.objdump" \
        "$OUT/$sym.candidate.objdump" "${wb_extra_args[@]}"
    exit $?
fi

if [ -z "$asmfile" ] || [ ! -f "$asmfile" ]; then
    echo "$0: no unique asm/nonmatchings fallback for '$target_sym'." >&2
    echo "  splat drops it once the C implements the function; try --rom." >&2
    exit 1
fi
cand=$cand_build_dir/src/$tu.c.o
if [ ! -f "$cand" ]; then echo "$0: no candidate object $cand -- run gmake first." >&2; exit 1; fi

printf '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n' > "$OUT/$sym.target.s"
cat "$asmfile" >> "$OUT/$sym.target.s"
$AS $ASFLAGS -o "$OUT/$sym.target.o" "$OUT/$sym.target.s"
if [ "$candidate_sym" != "$target_sym" ]; then
    $OBJCOPY --redefine-sym "$target_sym=$candidate_sym" "$OUT/$sym.target.o"
fi

run_provenance \
    --mode asm \
    --symbol "$target_sym" \
    --candidate-symbol "$candidate_sym" \
    --source "$source_path" \
    --candidate-build-dir "$cand_build_dir" \
    --candidate-object "$cand" \
    --target-object "$OUT/$sym.target.o" \
    --candidate-artifact "$cand" \
    --target-artifact "$OUT/$sym.target.o" \
    --objdump "$OBJDUMP"
run_workbench "$wb_command" "$OUT/$sym.target.o" "$cand" \
    --function "$candidate_sym" --objdump "$OBJDUMP" "${wb_extra_args[@]}"
