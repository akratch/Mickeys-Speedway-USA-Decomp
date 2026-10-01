#!/usr/bin/env bash
# Finish an in-progress merge after manual conflict resolution.
#
#   tools/finish_merge.sh
#
# Refuses to commit unless every gate passes: no conflict markers in tracked
# files, splat re-extract, atlas + digest regeneration, stale-extern rename,
# byte-identical verify, recomputed derived claims, check-docs, scoreboard,
# overlay-atlas. Prints the progress lines on success.
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"
if git grep -q '^<<<<<<< ' -- . ':!*.md'; then echo "conflict markers remain:" >&2; git grep -l '^<<<<<<< ' -- . >&2; exit 1; fi
if git diff --name-only --diff-filter=U | grep -q .; then echo "unmerged paths remain:" >&2; git diff --name-only --diff-filter=U >&2; exit 1; fi
.venv/bin/python tools/merge_transaction.py begin
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
gmake distclean >build-distclean.log 2>&1 || { echo "distclean FAILED (see build-distclean.log); merge left uncommitted" >&2; exit 1; }
rm -f build-distclean.log
extract_log=$(mktemp -t mickey-merge-extract)
gmake extract >"$extract_log" 2>&1 || { echo "extract FAILED; merge left uncommitted" >&2; tail -20 "$extract_log" >&2; exit 1; }
tail -1 "$extract_log"; rm -f "$extract_log"
gmake -j6 >/dev/null 2>&1 || true   # warm-up: the first parallel build after a re-split can race
# A merge that changes overlay relocation surfaces needs the generated
# overlay symbol block regenerated before the link can succeed.
gmake overlay-syms 2>&1 | tail -1
gmake -j6 >/dev/null 2>&1 || true
# `set -e` with `pipefail` would abort at this assignment when verify fails,
# before the case below can report why -- a failure then surfaced only as a
# bare exit status. Keep the whole log and read the reason out of it.
verify_log=$(mktemp -t mickey-finish-verify)
set +e
tools/with_verify_lock.sh gmake -j6 verify >"$verify_log" 2>&1
set -e
out=$(tail -1 "$verify_log"); echo "$out"
case "$out" in
  OK*) rm -f "$verify_log" ;;
  *) echo "verify FAILED; not committing" >&2
     grep -iE 'error|undefined ref|defined twice|unreviewed|truncated|refus' "$verify_log" | head -8 >&2
     echo "full log: $verify_log" >&2
     exit 1 ;;
esac
.venv/bin/python tools/fix_jumptable_claim.py | tail -1
# Re-measure stale ranking rows and the postprocess audit first: a banked
# improvement or a POSTPROCESS edit otherwise fails check-docs here.
# Advisory: a failure here surfaces in check-docs below with its own message.
{ .venv/bin/python tools/nm_ranking.py --refresh-stale 2>&1 | tail -1; } || echo "warning: ranking refresh failed" >&2
.venv/bin/python tools/nm_ranking.py --write-doc >/dev/null 2>&1 || true
{ .venv/bin/python tools/postprocess_audit.py --write 2>&1 | tail -1; } || echo "warning: postprocess audit refresh failed" >&2
git add -- config/nonmatching-ranking.us.json docs/nm-ranking.md config/postprocess-audit.us.json 2>/dev/null || true
docs_log=$(mktemp -t mickey-merge-check-docs)
gmake check-docs >"$docs_log" 2>&1 || { echo "check-docs FAILED; merge left uncommitted" >&2; tail -30 "$docs_log" >&2; exit 1; }
tail -1 "$docs_log"; rm -f "$docs_log"
gmake scoreboard 2>&1 | tail -1
gmake overlay-atlas 2>&1 | tail -1
gmake check-overlay-syms 2>&1 | tail -1
gmake check-scoreboard 2>&1 | tail -1
gmake cleanroom 2>&1 | tail -1
.venv/bin/python tools/merge_transaction.py stage
git commit -q --no-edit
.venv/bin/python tools/merge_transaction.py clean
git log --oneline -1
timeout 240 gmake progress 2>&1 | grep -E '^functions|decompiled|NON_MATCH'
