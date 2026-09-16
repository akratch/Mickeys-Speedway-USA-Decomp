#!/usr/bin/env bash
# Integrate one lane branch into the current branch (campaign/unchain).
#
#   tools/merge_lane.sh <lane-name>
#
# Reads the lane's committed tip without touching its worktree, merges it,
# resolves generated files that conflict,
# conflict by regenerating them (README scoreboard block, overlay atlas), then
# re-runs verify/check-docs/overlay-atlas/check-scoreboard here. Exits non-zero
# and leaves the merge in progress if anything else conflicts or a gate fails.
set -euo pipefail
name=${1:?lane name}
root=$(git rev-parse --show-toplevel)
cd "$root"
branch=lane/$name
tip=$(git rev-parse --verify "$branch^{commit}")
build_jobs=${MICKEY_BUILD_JOBS:-6}
build_nice=${MICKEY_BUILD_NICE:-15}
case "$build_jobs" in ''|*[!0-9]*|0) echo "invalid MICKEY_BUILD_JOBS: $build_jobs" >&2; exit 2 ;; esac
case "$build_nice" in ''|*[!0-9]*) echo "invalid MICKEY_BUILD_NICE: $build_nice" >&2; exit 2 ;; esac
low_gmake() { nice -n "$build_nice" gmake -j"$build_jobs" "$@"; }
.venv/bin/python tools/merge_transaction.py clean
# Never pipe a gate.  `| tail -1` reports tail's status, which is always 0, so
# a failing scan read as a pass here and its findings scrolled away, leaving one
# line of remediation advice with no finding above it.  Capture the output,
# branch on the real status, and print everything when it fails.
scan=$(mktemp -t mickey-merge-cleanroom)
if tools/cleanroom_check.sh --range "HEAD..$tip" >"$scan" 2>&1; then
	tail -1 "$scan"
	rm -f "$scan"
else
	cat "$scan" >&2
	rm -f "$scan"
	echo "merge_lane: clean-room range scan failed; $branch not merged" >&2
	exit 1
fi
# Advisory only. A superseded lane still merges if the caller wants its
# documentation, but each attempt costs a full build and gate cycle, and
# the resulting tree usually fails several gates later for reasons that
# read like defects in the lane's own work rather than in its age.
.venv/bin/python tools/check_lane_superseded.py "$branch" --base HEAD || true
echo "== merge $branch"
# --no-commit: the merge is committed only after every gate below passes.
if ! git merge --no-commit --no-ff "$tip" >/dev/null 2>&1; then
regenerate_ranking_doc=0
  conflicts=$(git diff --name-only --diff-filter=U)
  for f in $conflicts; do
    case "$f" in
      README.md|config/overlays.us.json|config/overlay-donors.us.json|config/postprocess-audit.us.json) git checkout --theirs "$f" && git add "$f" ;;
      # docs/nm-ranking.md is generated from config/nonmatching-ranking.us.json.
      # Three-way merging it produces a document that matches neither side and
      # fails `nm_ranking.py --check-doc` in the gates below, which is how two
      # match integrations stalled before this rule existed. Regenerate it from
      # the merged ranking instead of merging its text.
      docs/nm-ranking.md) git checkout --theirs "$f" && git add "$f" && regenerate_ranking_doc=1 ;;
      docs/modules.md|docs/overlays.md) .venv/bin/python tools/resolve_modules_split.py || { echo "unresolved conflict: $f" >&2; exit 1; } ;;
      mickey.us.yaml|docs/resident.md|*.c|*.h) .venv/bin/python tools/resolve_comment_hunks.py "$f" && git add "$f" || echo "deferring $f to tools/resolve_lane_conflicts.py" ;;
      *) echo "deferring $f to tools/resolve_lane_conflicts.py" ;;
    esac
  done
fi
# Whatever the per-file rules above left unresolved goes through the
# hunk-level/three-way resolver (README theirs, Makefile keep-both, lane-owned
# new files theirs, ledgers theirs-per-hunk when ours is a subset, everything
# else three-way against the lane's merge-base). It never takes a shared
# file whole.
if git diff --name-only --diff-filter=U | grep -q .; then
  .venv/bin/python tools/resolve_lane_conflicts.py "$tip" || { echo "unresolved conflicts remain; merge left in progress" >&2; exit 1; }
fi
if ! git rev-parse --verify MERGE_HEAD >/dev/null 2>&1; then
  git merge-base --is-ancestor "$tip" HEAD || { echo "merge did not start" >&2; exit 1; }
  echo "$branch is already integrated"
  exit 0
fi
if git grep -q '^<<<<<<< ' -- . ':!*.md'; then echo "conflict markers left in tracked files:" >&2; git grep -l '^<<<<<<< ' -- . >&2; exit 1; fi
.venv/bin/python tools/merge_transaction.py begin
if [ "${regenerate_ranking_doc:-0}" = 1 ]; then
  .venv/bin/python tools/nm_ranking.py --write-doc >/dev/null
  git add docs/nm-ranking.md
  echo "regenerated docs/nm-ranking.md from the merged ranking"
fi
# A promotion retires the matched function's ranking row, so `check-docs`
# fails below until config/nonmatching-ranking.us.json stops listing it. That
# happened on five merges on 2026-09-16, and each cost a full atlas/extract/
# two-pass rebuild plus a measuring `nm_ranking.py` run -- ten to fifteen
# minutes -- before `finish_merge.sh` could be re-run by hand.
#
# `--prune-stale` drops rows whose file/symbol identity has left the queue
# WITHOUT compiling, which is exactly what a promotion needs and all it needs;
# the surviving rows keep their proven measurements. It is a no-op when the
# merge promoted nothing, so it is unconditional here.
pruned=$(.venv/bin/python tools/nm_ranking.py --prune-stale 2>&1 | tail -1)
case "$pruned" in
  "pruned 0 stale row"*) ;;
  *) .venv/bin/python tools/nm_ranking.py --write-doc >/dev/null
     git add config/nonmatching-ranking.us.json docs/nm-ranking.md
     echo "ranking: $pruned" ;;
esac
echo "== integration gates"
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
gmake extract 2>&1 | tail -1
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
.venv/bin/python tools/fix_stale_externs.py | tail -1
.venv/bin/python tools/check_match_regression.py HEAD || { echo "a function matched at HEAD carries GLOBAL_ASM again; merge left uncommitted (resolve hunks, never whole files)" >&2; exit 1; }
.venv/bin/python tools/check_duplicate_bodies.py || { echo "a function has both a C body and a GLOBAL_ASM pragma after the merge; drop the stale pragma line" >&2; exit 1; }
gmake -s check-nonmatching-builds || { echo "a candidate-bearing TU no longer compiles with -DNON_MATCHING (its candidates would drop out of the permuter sweep); merge left uncommitted" >&2; exit 1; }
# Fresh extraction and build: stale objects and stale asm/ have masked real failures twice.
gmake distclean >/dev/null 2>&1; gmake extract 2>&1 | tail -1
low_gmake >/dev/null 2>&1 || true   # warm-up: the first parallel build after a re-split can race
gmake overlay-syms 2>&1 | tail -1   # a merge that changes overlay relocation surfaces needs the generated symbol block before the link
low_gmake >/dev/null 2>&1 || true
# See finish_merge.sh: `set -e` + `pipefail` would abort at this assignment
# when verify fails, suppressing the diagnostic the case below exists to give.
verify_log=$(mktemp -t mickey-merge-verify)
set +e
tools/with_verify_lock.sh nice -n "$build_nice" gmake -j"$build_jobs" verify >"$verify_log" 2>&1
set -e
out=$(tail -1 "$verify_log"); echo "$out"
case "$out" in
  OK*) rm -f "$verify_log" ;;
  *) echo "verify FAILED after merging $branch; merge left uncommitted (git merge --abort to drop it)" >&2
     grep -iE 'error|undefined ref|defined twice|unreviewed|truncated|refus' "$verify_log" | head -8 >&2
     echo "full log: $verify_log" >&2
     exit 1 ;;
esac
gmake scoreboard 2>&1 | tail -1
gmake overlay-atlas 2>&1 | tail -1
.venv/bin/python tools/fix_jumptable_claim.py >/dev/null 2>&1 || true
gmake check-docs 2>&1 | tail -1
gmake check-overlay-syms 2>&1 | tail -1
gmake check-scoreboard 2>&1 | tail -1
gmake cleanroom 2>&1 | tail -1
.venv/bin/python tools/merge_transaction.py stage
git commit -q -m "Merge $branch into $(git rev-parse --abbrev-ref HEAD)

Gates at merge time: verify byte-identical, check-docs, overlay-atlas,
overlay symbols checked, scoreboard regenerated."
.venv/bin/python tools/merge_transaction.py clean
git log --oneline -1
