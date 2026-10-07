#!/usr/bin/env bash
# Integrate a batch of lanes, and optionally land it, as ONE serialized run.
#
#   tools/integrate.sh [--land] <lane> [lane ...]
#
# Replaces the hand-written chain "wait for the previous batch, stash the
# owner's edit, merge_lanes.sh, land.sh --release-ref campaign/unchain, check
# out back, pop the stash". Steps:
#
#   1. Take a dedicated lock (<git-common-dir>/integrate.lock, a pid-stamped
#      directory). A second invocation queues and waits; it never overlaps. A
#      lock whose pid is dead is reclaimed. It is NOT with_verify_lock.sh:
#      merge_lanes.sh takes that one itself for verify, so wrapping it here
#      would deadlock.
#   2. Record the recovery base (HEAD).
#   3. Stash uncommitted tracked changes to files the batch does not touch;
#      refuse, naming the file, if a dirty file IS touched (a lane's diff, or
#      a generated file the gates rewrite).
#   4. tools/merge_lanes.sh <lanes>. If it dies at check-docs, run the
#      documented repair and re-run the gate once: check_shard_metrics.py
#      --write for header drift. A derived-number mismatch is printed
#      verbatim and the run stops; that comment is never guessed at.
#   5. With --land: tools/land.sh --release-ref campaign/unchain (the campaign
#      branch itself cannot be pushed: historical commits fail the pre-push
#      scan).
#   6. Restore the branch and the stash. Final line is one of
#        == integrated <lanes> landed <sha>
#        == FAILED at <step>: <reason>; recover with git reset --hard <base>
#
# Test hooks: MICKEY_INTEGRATE_{PYTHON,MERGE_LANES,LAND,BRANCH,POLL}.
set -uo pipefail

land=0
if [ "${1:-}" = "--land" ]; then land=1; shift; fi
[ "$#" -gt 0 ] || { echo "usage: $0 [--land] <lane> [lane ...]" >&2; exit 2; }
lanes=("$@")

root=$(git rev-parse --show-toplevel) || exit 2
cd "$root"
py=${MICKEY_INTEGRATE_PYTHON:-.venv/bin/python}
merge_lanes=${MICKEY_INTEGRATE_MERGE_LANES:-tools/merge_lanes.sh}
land_sh=${MICKEY_INTEGRATE_LAND:-tools/land.sh}
branch=${MICKEY_INTEGRATE_BRANCH:-campaign/unchain}
poll=${MICKEY_INTEGRATE_POLL:-2}
lock_dir=$(git rev-parse --path-format=absolute --git-common-dir)/integrate.lock
generated=(README.md docs/nm-ranking.md docs/matching-triage-handoffs config
           overlay_undefined_syms.us.txt mickey.us.yaml)

step=lock; base=""; held=0; stash_ref=""; start_branch=""
logs=$(mktemp -d -t mickey-integrate.XXXXXX)

release() {
  [ "$held" -eq 1 ] || return 0
  rm -f "$lock_dir/pid"; rmdir "$lock_dir" 2>/dev/null || true; held=0
}
restore_stash() {
  [ -n "$stash_ref" ] || return 0
  if git stash pop -q "$stash_ref" >/dev/null 2>&1; then stash_ref=""
  else echo "integrate: could not pop $stash_ref; your edits are still in that stash" >&2; fi
}
fail() {
  echo "== FAILED at $step: $1; recover with git reset --hard ${base:-HEAD}"
  exit 1
}
cleanup() {
  local rc=$?
  if [ -n "$start_branch" ] && [ "$(git rev-parse --abbrev-ref HEAD 2>/dev/null)" != "$start_branch" ]; then
    git checkout -q "$start_branch" 2>/dev/null || true
  fi
  restore_stash
  release
  exit "$rc"
}
trap cleanup EXIT
trap 'exit 130' INT TERM HUP

# 1. lock ------------------------------------------------------------------
announced=0
while :; do
  if mkdir "$lock_dir" 2>/dev/null; then
    printf '%s\n' "$$" > "$lock_dir/pid"; held=1; break
  fi
  owner=$(sed -n 1p "$lock_dir/pid" 2>/dev/null || true)
  case "$owner" in
    ''|*[!0-9]*) ;;   # mid-creation lock: leave it alone
    *) if ! kill -0 "$owner" 2>/dev/null; then
         echo "== reclaiming stale integrate lock (pid $owner is gone)"
         rm -f "$lock_dir/pid"; rmdir "$lock_dir" 2>/dev/null || true
         continue
       fi ;;
  esac
  if [ "$announced" -eq 0 ]; then echo "== queued behind integrate pid ${owner:-?}"; announced=1; fi
  sleep "$poll"
done

# 2. base ------------------------------------------------------------------
step=preflight
start_branch=$(git rev-parse --abbrev-ref HEAD)
[ "$start_branch" = "$branch" ] || fail "must run on $branch (on $start_branch)"
base=$(git rev-parse HEAD)
echo "== integrate base $base (recovery: git reset --hard $base)"
git rev-parse -q --verify MERGE_HEAD >/dev/null && fail "a merge is already in progress"

# 3. dirty files -------------------------------------------------------------
touched=$(mktemp "$logs/touched.XXXXXX")
for name in "${lanes[@]}"; do
  tip=$(git rev-parse --verify "lane/$name^{commit}" 2>/dev/null) || fail "no such lane: lane/$name"
  git diff --name-only "HEAD...$tip" >> "$touched" || fail "cannot diff lane/$name"
done
printf '%s\n' "${generated[@]}" >> "$touched"
dirty=()
while IFS= read -r line; do
  [ -n "$line" ] || continue
  p=${line:3}; p=${p##* -> }
  dirty+=("$p")
done < <(git status --porcelain --untracked-files=no)
for p in ${dirty[@]+"${dirty[@]}"}; do
  while IFS= read -r t; do
    [ -n "$t" ] || continue
    if [ "$p" = "$t" ] || [ "${p#"$t"/}" != "$p" ]; then
      fail "uncommitted change to $p, which this batch touches; commit or revert it first"
    fi
  done < "$touched"
done
if [ "${#dirty[@]}" -gt 0 ]; then
  step=stash
  git stash push -q -m "integrate-$$" -- "${dirty[@]}" || fail "git stash failed"
  stash_ref=$(git stash list --format='%gd %s' | awk -v m="integrate-$$" '$NF==m{print $1; exit}')
  [ -n "$stash_ref" ] || fail "stash vanished"
  echo "== stashed ${#dirty[@]} unrelated file(s): ${dirty[*]}"
fi

# 4. merge -------------------------------------------------------------------
step=merge_lanes
mlog="$logs/merge.log"
"$merge_lanes" "${lanes[@]}" 2>&1 | tee "$mlog"
mrc=${PIPESTATUS[0]}
if [ "$mrc" -ne 0 ]; then
  grep -q 'gate FAILED: gmake check-docs' "$mlog" || fail "merge_lanes.sh exited $mrc"
  step=check-docs
  echo "== check-docs failed after the merges; looking for a documented repair"
  if ! "$py" tools/check_derived_numbers.py >"$logs/derived.log" 2>&1; then
    cat "$logs/derived.log" >&2
    fail "check_derived_numbers mismatch (lines printed above); edit them by hand, never guess, then commit"
  fi
  if ! "$py" tools/check_shard_metrics.py >"$logs/shard.log" 2>&1; then
    echo "== repairing shard header drift: check_shard_metrics.py --write"
    "$py" tools/check_shard_metrics.py --write || fail "check_shard_metrics.py --write refused"
  fi
  gmake check-docs >"$logs/docs.log" 2>&1 || { tail -40 "$logs/docs.log" >&2; fail "check-docs still fails after repair"; }
  echo "== check-docs passes after repair"
  # merge_lanes.sh stopped at check-docs; run the gates that follow it.
  for g in check-scoreboard check-overlay-syms cleanroom check-tooling; do
    step=$g
    gmake "$g" >"$logs/$g.log" 2>&1 || { tail -40 "$logs/$g.log" >&2; fail "gate $g failed"; }
    tail -1 "$logs/$g.log"
  done
  step=commit
  if ! git diff --quiet || ! git diff --cached --quiet; then
    for p in "${generated[@]}"; do [ -e "$p" ] && git add -A -- "$p"; done
    git diff --cached --quiet || git commit -q -m "Regenerate derived artifacts after batch integration" \
      || fail "could not commit the regenerated artifacts"
  fi
  if ! git diff --quiet; then fail "regenerated files left uncommitted (see git status)"; fi
fi

# 5. land --------------------------------------------------------------------
landed=""
if [ "$land" -eq 1 ]; then
  step=land
  "$land_sh" --release-ref "$branch" || fail "land.sh failed"
  landed=$(git rev-parse refs/heads/master)
fi

# 6. restore -----------------------------------------------------------------
step=restore
git checkout -q "$start_branch" || fail "cannot return to $start_branch"
restore_stash
[ -z "$stash_ref" ] || fail "stash pop conflicted"
rm -rf "$logs"
if [ "$land" -eq 1 ]; then echo "== integrated ${lanes[*]} landed $landed"
else echo "== integrated ${lanes[*]} (not landed) head $(git rev-parse HEAD)"; fi
