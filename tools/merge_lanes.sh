#!/usr/bin/env bash
# Integrate several lane branches under ONE run of the heavy gates.
#
#   tools/merge_lanes.sh <lane-name> [lane-name ...]
#
# tools/merge_lane.sh runs the full gate suite -- splat extract, a complete
# build, byte-exact ROM verification, docs, scoreboard, overlay symbols and
# clean-room -- once per lane. That is the right shape for a single
# integration, but a campaign wave finishes six or more lanes at a time and
# then pays six full verifications to answer one question: is the resulting
# tree exact?
#
# This driver merges each lane with the same conflict rules and the same
# per-commit hooks, then runs the heavy gates ONCE over the combined result.
# The gates that decide whether anything ships are unchanged and still have to
# pass; what changes is how often the identical build is repeated.
#
# The tradeoff is attribution: if the batch fails its gates, the offending lane
# is not named. That is why the pre-batch commit is printed up front and on
# failure -- recovery is `git reset --hard <that commit>` followed by
# integrating the lanes one at a time with tools/merge_lane.sh. Use this for
# waves of plateau lanes; a lane carrying a new match is worth integrating
# alone so its ROM proof stands on its own.
set -euo pipefail
[ "$#" -gt 0 ] || { echo "usage: $0 <lane-name> [lane-name ...]" >&2; exit 2; }
root=$(git rev-parse --show-toplevel); cd "$root"
build_jobs=${MICKEY_BUILD_JOBS:-6}
base=$(git rev-parse HEAD)
echo "== batch base $base (recovery: git reset --hard $base)"

merged=()
for name in "$@"; do
  branch=lane/$name
  tip=$(git rev-parse --verify "$branch^{commit}") || { echo "no such lane: $branch" >&2; exit 2; }
  if git merge-base --is-ancestor "$tip" HEAD; then echo "== $branch already integrated"; continue; fi
  echo "== merge $branch"
  tools/cleanroom_check.sh --range "HEAD..$tip" 2>&1 | tail -1
  if ! git merge --no-commit --no-ff "$tip" >/dev/null 2>&1; then
    for f in $(git diff --name-only --diff-filter=U); do
      case "$f" in
        README.md|config/overlays.us.json|config/overlay-donors.us.json|config/postprocess-audit.us.json|docs/nm-ranking.md)
          git checkout --theirs "$f" && git add "$f" ;;
        docs/modules.md|docs/overlays.md)
          .venv/bin/python tools/resolve_modules_split.py || { echo "unresolved: $f" >&2; exit 1; } ;;
        mickey.us.yaml|docs/resident.md|*.c|*.h)
          .venv/bin/python tools/resolve_comment_hunks.py "$f" && git add "$f" || true ;;
      esac
    done
    if git diff --name-only --diff-filter=U | grep -q .; then
      .venv/bin/python tools/resolve_lane_conflicts.py "$tip" \
        || { echo "unresolved conflicts in $branch; batch left in progress" >&2; exit 1; }
    fi
  fi
  git rev-parse --verify MERGE_HEAD >/dev/null 2>&1 || { echo "== $branch produced no merge"; continue; }
  if git grep -q '^<<<<<<< ' -- . ':!*.md'; then
    echo "conflict markers left by $branch" >&2; exit 1
  fi
  git commit -q --no-edit
  merged+=("$name")
done

[ "${#merged[@]}" -gt 0 ] || { echo "== nothing merged"; exit 0; }
echo "== merged ${#merged[@]} lane(s): ${merged[*]}"
echo "== integration gates (once for the batch)"
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
gmake extract 2>&1 | tail -1
.venv/bin/python tools/fix_stale_externs.py | tail -1
# Two lanes that each retire a ranking row auto-merge the JSON's rows cleanly
# but keep one header count, so the merged file says one more function than it
# lists and --write-doc refuses it. Re-measure the whole queue instead of
# trusting the merge (CLAUDE.md, "A clean auto-merge of a generated file").
.venv/bin/python tools/nm_ranking.py --out config/nonmatching-ranking.us.json --no-table 2>&1 | tail -1
.venv/bin/python tools/nm_ranking.py --write-doc >/dev/null
# Plateau shards are projected from the source markers. A batch that lands
# several lanes can leave one shard behind its marker, which fails
# check-docs below after the merges are already committed. Reconcile first;
# --write only projects fields already present in tracked source markers.
.venv/bin/python tools/plateau_handoff_audit.py --write | tail -1
.venv/bin/python tools/check_match_regression.py HEAD \
  || { echo "a matched function regressed to GLOBAL_ASM; recover with git reset --hard $base" >&2; exit 1; }
.venv/bin/python tools/check_duplicate_bodies.py \
  || { echo "duplicate body after merge; recover with git reset --hard $base" >&2; exit 1; }
gmake overlay-syms 2>&1 | tail -1
verify_log=$(mktemp -t mickey-batch-verify)
set +e
tools/with_verify_lock.sh gmake -j"$build_jobs" verify >"$verify_log" 2>&1
set -e
out=$(tail -1 "$verify_log"); echo "$out"
case "$out" in
  OK*) rm -f "$verify_log" ;;
  *) echo "verify FAILED for the batch; recover with git reset --hard $base" >&2
     grep -iE 'error|undefined ref|defined twice|unreviewed|truncated|refus' "$verify_log" | head -8 >&2
     echo "full log: $verify_log" >&2
     exit 1 ;;
esac
# Banked improvements leave shards ahead of the ranking rows and POSTPROCESS
# edits leave the audit stale; re-measure both before the docs gate, as
# tools/merge_lane.sh does. Gates run unpiped so their status is their own.
.venv/bin/python tools/nm_ranking.py --refresh-stale 2>&1 | tail -1
.venv/bin/python tools/nm_ranking.py --write-doc >/dev/null
.venv/bin/python tools/postprocess_audit.py --write 2>&1 | tail -1
gmake scoreboard 2>&1 | tail -1
run_gate() {
  local log; log=$(mktemp -t mickey-batch-gate)
  if ! "$@" >"$log" 2>&1; then
    echo "gate FAILED: $*  (recover with git reset --hard $base, or repair and commit)" >&2
    tail -40 "$log" >&2; exit 1
  fi
  tail -1 "$log"; rm -f "$log"
}
run_gate gmake check-docs
run_gate gmake check-scoreboard
run_gate gmake check-overlay-syms
run_gate gmake cleanroom
# Lane commits pass check-tooling one at a time; a property of the combined
# tree (the live queue's size, for one) only shows up here.
run_gate gmake check-tooling
if ! git diff --quiet || ! git diff --cached --quiet; then
  git add -A README.md docs/nm-ranking.md docs/matching-triage-handoffs config overlay_undefined_syms.us.txt mickey.us.yaml 2>/dev/null || true
  git diff --cached --quiet || git commit -q -m "Regenerate derived artifacts after batch integration"
fi
echo "== batch integrated"; git log --oneline -1
