#!/usr/bin/env bash
# Land integrated campaign work on master, the canonical branch.
#
#   tools/land.sh [--release-ref REF]
#
# A curated release ref must already be an ancestor of campaign/unchain.
# It is pinned before publication and must contain both local and remote master.
# This publishes only master with the reviewed release tree; campaign work
# remains local. Local integration is required even when campaign is not pushed.
#
# By default, pushes campaign/unchain, merges it into master, re-verifies the ROM from
# the merge result, and pushes master. Run it after every integration batch;
# leaving master behind means someone merges it by hand later.
#
# Why the ROM is re-verified AFTER the merge and not before: the merge can
# combine two lanes' generated artifacts (the ranking, the atlas, the
# authorizations) in ways neither side produced alone, and `gmake verify` on
# campaign/unchain does not cover the merge commit's own tree.
#
# Never force-pushes. Never touches the `upstream` remote, which is Rare's
# public Jet Force Gemini repository and not ours. `origin` is public, so a
# push here publishes; the content rules in CLAUDE.md are the protection.
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

release_ref=""
if [ "$#" -ne 0 ]; then
    if [ "$#" -ne 2 ] || [ "$1" != "--release-ref" ] || [ -z "$2" ]; then
        echo "usage: tools/land.sh [--release-ref REF]" >&2
        exit 2
    fi
    release_ref=$2
fi

start=$(git rev-parse --abbrev-ref HEAD)
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    echo "tools/land.sh: tracked changes present; commit them first" >&2
    exit 1
fi
restore() { git checkout -q "$start" 2>/dev/null || true; }
trap restore EXIT

merge_ref=campaign/unchain
if [ -n "$release_ref" ]; then
    # Resolve once: neither a moving branch nor a tag can replace reviewed input.
    merge_ref=$(git rev-parse --verify --end-of-options "${release_ref}^{commit}")
    campaign_oid=$(git rev-parse --verify refs/heads/campaign/unchain)
    git merge-base --is-ancestor "$merge_ref" "$campaign_oid" || {
        echo "tools/land.sh: release ref is not integrated into campaign/unchain" >&2
        exit 1
    }
    git fetch -q origin master
    for master_ref in refs/heads/master refs/remotes/origin/master; do
        git merge-base --is-ancestor "$master_ref" "$merge_ref" || {
            echo "tools/land.sh: release ref does not contain $master_ref" >&2
            exit 1
        }
    done
fi

if [ -z "$release_ref" ]; then
    echo "== push campaign/unchain"
    git push origin campaign/unchain
fi

echo "== merge $merge_ref into master"
git checkout -q master
if [ -z "$release_ref" ]; then git fetch -q origin master; fi
git merge -q --ff-only origin/master
git merge --no-edit "$merge_ref"
if [ -n "$release_ref" ]; then
    landed_oid=$(git rev-parse --verify HEAD)
    git diff --quiet "$merge_ref" "$landed_oid" || {
        echo "tools/land.sh: merged tree differs from the pinned release tree" >&2
        exit 1
    }
fi

echo "== regenerate the overlay alias list"
# and fail if the committed one was stale. reloc_surface derives the list from
# the COMPILED overlay objects, so a promotion whose POSTPROCESS rule is
# incomplete can still pass its own gates: POSTPROCESS is a Make *variable*,
# not a prerequisite, so editing a rule rebuilds nothing and a warm build/
# keeps an object that predates it. Overlay 60 shipped 8 of its 51 resident
# rebinds that way and no fresh worktree could link afterwards.
#
# Regenerating and then diffing is the cheap detector: if the committed file
# moves, the tree that produced it was not the tree being committed.
# reloc_surface derives the alias list from the COMPILED overlay objects, so a
# build directory carrying state from before an overlay promotion links against
# stale aliases and dies with `R_MIPS_26 relocation truncated` -- even when the
# committed overlay_undefined_syms.us.txt is already correct. Regenerating is
# idempotent and cheap next to the verify it protects.
gmake overlay-syms >/dev/null
if ! git diff --quiet -- overlay_undefined_syms.us.txt; then
    echo "tools/land.sh: overlay_undefined_syms.us.txt was stale in the commit." >&2
    echo "  Regenerating it from the built objects changed it, which means some" >&2
    echo "  overlay object was compiled without the POSTPROCESS rule that is now" >&2
    echo "  committed -- usually an incomplete set of --redefine-sym rebinds." >&2
    echo "  Check mk/overlays.mk names every resident call the promoted object" >&2
    echo "  makes, then rebuild that object and re-run." >&2
    git --no-pager diff --stat -- overlay_undefined_syms.us.txt >&2
    exit 1
fi

echo "== verify the merge result"
gmake verify

echo "== push master"
if [ -n "$release_ref" ]; then
    gmake cleanroom check-docs check-scoreboard
    if [ "$(git rev-parse --verify HEAD)" != "$landed_oid" ] ||
            ! git diff --quiet || ! git diff --cached --quiet; then
        echo "tools/land.sh: verified release checkout changed before publication" >&2
        exit 1
    fi
    git push origin "$landed_oid:refs/heads/master"
else
    git push origin master
fi
# ADR 0011: a source or handoff commit consumes its reopening authorization.
# Landing must not rearm that attempt by copying the old reason to new pins.
# The coordinator may authorize a genuinely new mechanism separately.

echo "== landed: $(git log --oneline -1)"
