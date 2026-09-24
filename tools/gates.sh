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
# Logs go to build/gates/<gate>.log. Exits nonzero if any gate failed, and
# prints the failing gates again at the end so the verdict is the last thing
# on screen.
set -uo pipefail
cd "$(dirname "$0")/.."

DEFAULT_GATES=(verify cleanroom check-docs check-tooling)
PROMOTION_GATES=(check-scoreboard check-overlay-syms check-nonmatching-builds check-promotion-proofs)

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

logdir=build/gates
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
  mkdir -p build/gates
  if git diff --cached --check > build/gates/whitespace.log 2>&1; then
    printf 'PASS  %-26s %s\n' whitespace build/gates/whitespace.log
    extra_gates=$((extra_gates + 1))
  else
    printf 'FAIL  %-26s %s\n' whitespace build/gates/whitespace.log
    sed 's/^/      | /' build/gates/whitespace.log | tail -6
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
  if [ "$status" -eq 0 ]; then
    printf 'PASS  %-26s %s\n' "$g" "$logdir/$g.log"
  else
    printf 'FAIL  %-26s exit=%d  %s\n' "$g" "$status" "$logdir/$g.log"
    sed 's/^/      | /' "$logdir/$g.log" | tail -6
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
