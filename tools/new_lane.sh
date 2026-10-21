#!/usr/bin/env bash
# Create an isolated worktree ("lane") for one worker.
#
#   tools/new_lane.sh <name> [--no-extract] [--no-cache] [--no-prebuild]
#                     [base-branch]
#
# Creates ../mickey-lane-<name> on branch lane/<name> from base-branch
# (default: freshest linear local/remote campaign integration ref), shares the
# untracked toolchain, baserom, venv and
# vendored tool checkouts with this repository by symlink. When a verified
# commit-keyed bootstrap exists below Git's common directory, it copy-on-write
# clones the ignored split/build prerequisites into this lane; otherwise it
# runs the splat extract. Each lane still owns its build/ and asm/. Prints the
# lane path.
set -euo pipefail
name=${1:?lane name}; shift
extract=1; cache=1; prebuild=1; base=
for a in "$@"; do
  case "$a" in
    --no-extract) extract=0 ;;
    --no-cache) cache=0 ;;
    --no-prebuild) prebuild=0 ;;
    *) base=$a ;;
  esac
done
# Resolve an explicitly supplied relative ref (especially HEAD) in the calling
# worktree before switching Git operations to the primary checkout.
caller=$(git rev-parse --show-toplevel)
# Always anchor lane creation in the primary checkout. When this helper is
# invoked from an existing linked worktree, --show-toplevel names that lane and
# its .git is a file, so "$root/.git/modules" cannot be the shared submodule
# store. The common directory is stable from every worktree.
common=$(git rev-parse --path-format=absolute --git-common-dir)
if [ "$(basename "$common")" != .git ]; then
  echo "expected a non-bare repository with a .git common directory: $common" >&2
  exit 2
fi
root=$(dirname "$common")
if [ -z "$base" ]; then
  base=$(python3 "$caller/tools/integration_base.py" --repo "$root")
  base_commit=$(git -C "$root" rev-parse --verify "$base^{commit}")
else
  base_commit=$(git -C "$caller" rev-parse --verify "$base^{commit}")
fi
display_dest=$(dirname "$root")/mickey-lane-$name
dest=$display_dest
# Spotlight reliably excludes the `.noindex` directory suffix;
# `.metadata_never_index` did not stop per-lane mdworker storms on this
# workstation. Keep the established path as a symlink while registering the
# real macOS worktree below an
# excluded directory name. Other platforms retain the ordinary path.
if [ "$(uname -s)" = Darwin ]; then
  dest=$display_dest.noindex
fi
if [ -e "$display_dest" ] || [ -L "$display_dest" ] || [ -e "$dest" ]; then
  echo "lane exists: $display_dest" >&2
  exit 2
fi
# Creating several full worktrees can make macOS Spotlight index every copied
# source/build path at once. Create the registered worktree without populating
# it, install the marker, and only then materialize the tracked tree. Other
# platforms harmlessly ignore this git-ignored empty file.
git -C "$root" worktree add -q --no-checkout \
  -b "lane/$name" "$dest" "$base_commit"
: > "$dest/.metadata_never_index"
git -C "$dest" read-tree "$base_commit"
git -C "$dest" checkout-index --all
for p in baseroms tools/ido tools/binutils .venv tools/objdiff; do
  [ -e "$root/$p" ] && ln -s "$root/$p" "$dest/$p"
done
# Submodules: clone from this repository's own module store (no network),
# so the lane's git status stays clean. A symlink here makes git complain
# that it "expected submodule path not to be a symbolic link".
for p in tools/asm-processor tools/asm-differ tools/m2c; do
  if ! git -C "$dest" -c protocol.file.allow=always \
      -c "submodule.$p.url=$common/modules/$p" \
      submodule update --init --quiet "$p"; then
    echo "submodule init failed for $p; refusing a dirty symlink fallback" >&2
    exit 1
  fi
done
# The permuter checkout is outside the repository; tools/permute.sh expects
# tools/permuter to point at it (git-ignored, machine-specific).
[ -e "$root/tools/permuter" ] && ln -s "$(readlink "$root/tools/permuter" || echo "$root/tools/permuter")" "$dest/tools/permuter"
restored=0
if [ "$cache" = 1 ] && [ "$extract" = 1 ]; then
  set +e
  (cd "$dest" && python3 tools/lane_cache.py restore --quiet)
  cache_status=$?
  set -e
  case "$cache_status" in
    0) restored=1 ;;
    3) ;; # no exact-commit cache: use the ordinary extraction path below
    *) echo "verified lane-cache restore failed" >&2; exit "$cache_status" ;;
  esac
fi
if [ "$extract" = 1 ] && [ "$restored" = 0 ]; then
  (cd "$dest" && gmake extract >"$dest/.lane-extract.log" 2>&1) || {
    echo "extract failed, see $dest/.lane-extract.log" >&2; exit 1; }
fi
if [ "$dest" != "$display_dest" ]; then
  ln -s "$(basename "$dest")" "$display_dest"
fi
# Prebuild the lane once, where the cost is its setup rather than its budget.
# A clean tree links in a single pass: every resident-call rename is a
# declared rule (mk/overlay_resident_rebinds.generated.mk) and the relocation
# surface is tracked, so nothing has to run between compile and link.
#
# A base that predates those declared rules still needs the old dance (build,
# `gmake overlay-syms`, build); fall back to it when the single pass fails and
# say so in the log, because on a current base a failure here is a finding.
#
# Advisory: a lane can still build. If this fails the worktree is fine, so the
# failure is reported and not fatal.
if [ "$prebuild" = 1 ]; then
  jobs=${MICKEY_BUILD_JOBS:-8}
  if ! (cd "$dest" \
        && { gmake -j"$jobs" \
             || { echo "new_lane: single-pass build failed; retrying with overlay-syms" \
                  && gmake overlay-syms && gmake -j"$jobs"; }; }) \
        >"$dest/.lane-prebuild.log" 2>&1; then
    echo "prebuild failed; run 'gmake -j8' in the lane (see $dest/.lane-prebuild.log)" >&2
  fi
fi
echo "$display_dest"
