#!/usr/bin/env bash
# Run the pre-commit gates and report each one's TRUE exit status.
#
#   tools/gates.sh                # verify, cleanroom, check-docs, check-tooling
#   tools/gates.sh --promotion    # the above plus the promotion gates
#   tools/gates.sh verify check-docs
#   tools/gates.sh --staged       # cleanroom scans the index, not the worktree
#
# WHY THIS EXISTS. The obvious way to run a gate and see its output is
#
#     gmake check-scoreboard 2>&1 | tail -3; echo "exit=$?"
#
# and that reports the exit status of `tail`, which is always 0. A gate that
# printed its failure three lines up therefore reads as a pass. This has
# happened three times in this campaign and twice it put a commit on top of a
# red gate. Piping a gate's output anywhere is the hazard; this script keeps
# the status and the output separate so they cannot be confused.
#
# WHY check-tooling IS IN THE DEFAULT SET. It is not in CLAUDE.md's
# "before every commit" list, so it was in neither of this script's sets, and a
# per-TU CFLAGS line added to the root Makefile turned it red for several
# commits while every gate that WAS run stayed green. A lane found it, not the
# coordinator. A gate runner whose set is narrower than the tree's checks
# reproduces the exact failure it was written to prevent, so the set is now the
# tool tests too. It needs no build.
#
# Logs go to build/gates/<gate>.log ($GATES_LOG_DIR overrides, for tests). Exits nonzero if any gate failed, and
# prints the failing gates again at the end so the verdict is the last thing
# on screen.
#
# check-runlink-tables (in --promotion) rebuilds every module's relocation
# tables, the ORT and the main table from the objects and fails on any
# difference from the shipped ones that config/runlink-residuals.us.json does
# not list, and on any listed one that has gone (tools/runlink_pack.py). A
# source edit that fixes a residual therefore fails it too, by design: run
# `gmake runlink-residuals` and commit the shorter manifest.
#
# TWO FAILURES THAT ARE NOT THE TREE'S. check-promotion-proofs refuses with
# "proof inputs changed during census" when a source changed while it ran --
# another lane compiling in this tree, typically. It is retried once, and the
# message says what to stop. check-objdiff-config reads the split tree on
# disk, so a leftover extract (an old carve's .bin or .s that the linker
# script no longer places) reads as a stale objdiff.json; the orphans are
# named, with the fix (delete them; never regenerate objdiff.json around one).
set -uo pipefail
cd "$(dirname "$0")/.."

DEFAULT_GATES=(verify cleanroom check-docs check-tooling)
PROMOTION_GATES=(check-scoreboard check-overlay-syms check-objdiff-config check-nonmatching-builds check-promotion-proofs check-runlink-tables check-overlay-graph)

gates=()
staged=0
for a in "$@"; do
  case "$a" in
    --promotion) gates+=("${DEFAULT_GATES[@]}" "${PROMOTION_GATES[@]}") ;;
    --staged)    staged=1 ;;
    -h|--help)   sed -n '2,18p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    -*)          echo "gates.sh: unknown option $a" >&2; exit 2 ;;
    *)           gates+=("$a") ;;
  esac
done
[ ${#gates[@]} -eq 0 ] && gates=("${DEFAULT_GATES[@]}")

logdir=${GATES_LOG_DIR:-build/gates}
mkdir -p "$logdir"
failed=()

extra_gates=0
# Whitespace, when the index is what is being gated. `git diff --cached
# --check` refuses a trailing blank line at EOF and trailing spaces, and it is
# what `merge_transaction.py` runs at integration time -- so a lane that does
# not run it here hands back a branch that stops the merge after every gate has
# already passed. That happened on four merges on 2026-09-16, each caught only
# after a full build. It costs milliseconds and needs no build, so it runs
# first.
if [ "$staged" -eq 1 ]; then
  mkdir -p "$logdir"
  if git diff --cached --check > "$logdir/whitespace.log" 2>&1; then
    printf 'PASS  %-26s %s\n' whitespace "$logdir/whitespace.log"
    extra_gates=$((extra_gates + 1))
  else
    printf 'FAIL  %-26s %s\n' whitespace "$logdir/whitespace.log"
    sed 's/^/      | /' "$logdir/whitespace.log" | tail -6
    echo
    echo "GATES FAILED: whitespace"
    echo "Do NOT commit. Do NOT pass --no-verify. Fix the gate."
    exit 1
  fi
fi

for g in "${gates[@]}"; do
  args=()
  if [ "$g" = cleanroom ] && [ "$staged" -eq 1 ]; then
    args=(CLEANROOM_ARGS=--staged)
  fi
  if [ "$g" = check-promotion-proofs ]; then
    jobs=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
    args+=(PROMOTION_CENSUS_ARGS="-j $jobs --require-coverage")
  fi
  # No pipe. The status below is gmake's own.
  gmake "$g" "${args[@]}" > "$logdir/$g.log" 2>&1
  status=$?
  retried=""
  if [ "$status" -ne 0 ] && [ "$g" = check-promotion-proofs ] \
      && grep -q "proof inputs changed during census" "$logdir/$g.log"; then
    echo "NOTE  $g: a source changed mid-census: stop lanes compiling in this tree and rerun (retrying once)"
    mv "$logdir/$g.log" "$logdir/$g.first.log"
    gmake "$g" "${args[@]}" > "$logdir/$g.log" 2>&1
    status=$?
    retried=" (on retry; first attempt in $logdir/$g.first.log)"
  fi
  if [ "$status" -eq 0 ]; then
    printf 'PASS  %-26s %s%s\n' "$g" "$logdir/$g.log" "$retried"
  else
    printf 'FAIL  %-26s exit=%d  %s%s\n' "$g" "$status" "$logdir/$g.log" "$retried"
    sed 's/^/      | /' "$logdir/$g.log" | tail -6
    if [ "$g" = check-promotion-proofs ] && grep -q "proof inputs changed during census" "$logdir/$g.log"; then
      echo "      a source changed mid-census: stop lanes compiling in this tree and rerun"
    fi
    if [ "$g" = check-objdiff-config ]; then
      # Name split files the linker script never places: a leftover extract
      # makes a clean tree read stale. Status of the helper is informational.
      "${GATES_PYTHON:-.venv/bin/python}" tools/objdiff_config.py --orphans > "$logdir/$g.orphans.log" 2>&1
      sed 's/^/      > /' "$logdir/$g.orphans.log"
    fi
    failed+=("$g")
  fi
done

if [ ${#failed[@]} -ne 0 ]; then
  echo
  echo "GATES FAILED: ${failed[*]}"
  echo "Do NOT commit. Do NOT pass --no-verify. Fix the gate."
  exit 1
fi
echo
echo "all $(( ${#gates[@]} + extra_gates )) gate(s) passed"
