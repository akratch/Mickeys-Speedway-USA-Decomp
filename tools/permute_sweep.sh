#!/usr/bin/env bash
# Explicit, ownership-checked wrapper around the bounded batch runner.
set -euo pipefail

usage() {
    printf '%s\n' \
        'Usage: tools/permute_sweep.sh --report-only|--promote LANE [-- BATCH_ARGS...]' \
        '' \
        '  -h, --help    Help only: no Git calls, filesystem writes or builds.' \
        '  --report-only Verify the base and search; never apply or commit candidates.' \
        '  --promote     Explicitly enable verified --apply --commit promotions.' \
        '' \
        'LANE: 1-64 lowercase letters/digits/_/-, starting with a letter or digit.' \
        'The lane is created if absent, or ownership-checked before fast-forward resync.' \
        'Use only a lane you own exclusively; this wrapper does not arbitrate checkouts.' \
        'Batch options require --. Forwarded --apply, --commit and --list are rejected.' \
        'Defaults: jobs=2, threads=4, build-jobs=machine cores, minutes=20, flat-minutes=6,' \
        '          max-total-minutes=120, extend-minutes=20, load-threshold=13, resume.' \
        'Example: tools/permute_sweep.sh --report-only my-sweep -- --function myFunction --minutes 3'
}
die() { printf 'permute-sweep: %s\n' "$*" >&2; exit 2; }

# Help is recognized anywhere before resolving directories or invoking Git.
for arg in "$@"; do
    case "$arg" in -h|--help) usage; exit 0 ;; esac
done
if [ "$#" -eq 0 ]; then usage; exit 0; fi
case "$1" in
    --report-only) promote=0 ;;
    --promote) promote=1 ;;
    *) die 'choose --report-only or --promote explicitly (see --help)' ;;
esac
shift
[ "$#" -gt 0 ] || die 'missing lane name'
lane=$1; shift
[[ "$lane" =~ ^[a-z0-9][a-z0-9_-]{0,63}$ ]] || die "invalid lane name: $lane"
if [ "$#" -gt 0 ]; then
    [ "$1" = -- ] || die 'batch arguments must follow --'
    shift
fi

script_root=$(cd "$(dirname "$0")/.." && pwd -P)
# Parse, do not run the batch. -B avoids bytecode writes. Parsing also catches
# abbreviations such as --app and --comm: forwarded args cannot override mode.
build_jobs=$("$script_root/.venv/bin/python" -B -c '
import os, sys
sys.path.insert(0, sys.argv[1] + "/tools")
import permute_batch
args = permute_batch.parse_args(["--build-jobs", str(os.cpu_count() or 1), *sys.argv[2:]])
if args.apply or args.commit or args.list:
    sys.exit("permute-sweep: forwarded --apply/--commit/--list are forbidden; choose the wrapper mode")
if args.max_total_minutes <= 0 or args.jobs <= 0 or args.minutes <= 0:
    sys.exit("permute-sweep: jobs, minutes and total budget must be positive")
if args.build_jobs <= 0:
    sys.exit("permute-sweep: build-jobs must be positive")
print(args.build_jobs)
' "$script_root" "$@")

for name in GIT_DIR GIT_WORK_TREE GIT_COMMON_DIR GIT_INDEX_FILE; do
    [ -z "${!name:-}" ] || die "$name overrides are unsupported"
done
common=$(git -C "$script_root" rev-parse --path-format=absolute --git-common-dir)
[ "$(basename "$common")" = .git ] || die 'expected a non-bare common .git directory'
root=$(cd "$(dirname "$common")" && pwd -P)
common="$root/.git"
base=$(git -C "$root" rev-parse --verify 'refs/heads/campaign/unchain^{commit}')
lane_dir="$(dirname "$root")/mickey-lane-$lane"
expected_branch="refs/heads/lane/$lane"

check_owner() {
    [ -d "$lane_dir" ] || die "lane path is not a directory: $lane_dir"
    physical=$(cd "$lane_dir" && pwd -P)
    case "$physical" in
        "$lane_dir"|"$lane_dir.noindex") ;;
        *) die "lane symlink points outside its expected path: $physical" ;;
    esac
    [ -f "$physical/.git" ] || die 'expected a linked worktree, not an ordinary directory/repository'
    [ "$(git -C "$physical" rev-parse --show-toplevel)" = "$physical" ] || die 'wrong worktree root'
    [ "$(git -C "$physical" rev-parse --path-format=absolute --git-common-dir)" = "$common" ] || die 'lane belongs to another repository'
    [ "$(git -C "$physical" symbolic-ref -q HEAD)" = "$expected_branch" ] || die 'lane has an unexpected or detached branch'
    admin=$(git -C "$physical" rev-parse --absolute-git-dir)
    case "$admin" in "$common"/worktrees/*) ;; *) die 'unexpected worktree registration' ;; esac
    [ "$(<"$admin/gitdir")" = "$physical/.git" ] || die 'worktree registration points elsewhere'
    [ ! -e "$admin/index.lock" ] || die 'worktree index is busy; preserve it and retry later'
    tracked_status=$(git -C "$physical" status --porcelain --untracked-files=no) || die 'cannot read lane status; refusing to resync'
    [ -z "$tracked_status" ] || die 'lane has tracked changes; refusing to resync'
}

if [ -e "$lane_dir" ] || [ -L "$lane_dir" ]; then
    check_owner
else
    [ ! -e "$lane_dir.noindex" ] && [ ! -L "$lane_dir.noindex" ] || die 'unmapped .noindex worktree already exists'
    branch_status=0
    git -C "$root" show-ref --verify --quiet "$expected_branch" || branch_status=$?
    if [ "$branch_status" -eq 0 ]; then
        die 'lane branch already exists without its expected worktree; preserve it for its owner'
    fi
    [ "$branch_status" -eq 1 ] || die 'cannot inspect lane branch; refusing to create a worktree'
    "$script_root/tools/new_lane.sh" "$lane" --no-extract "$base" >/dev/null
    check_owner
fi

# Preserve even cherry-equivalent divergent histories rather than reset them.
git -C "$physical" merge-base --is-ancestor HEAD "$base" || die 'lane has unintegrated/divergent commits; preserve it for coordinator review'
printf 'resync: lane/%s -> campaign/unchain %s\n' "$lane" "$base"
git -C "$physical" merge -q --ff-only "$base"
check_owner
cd "$physical"
gmake "-j$build_jobs" extract >/dev/null
gmake overlay-syms >/dev/null
# `overlay-syms` needs the overlay objects, but its resident-name census also
# reads every resident object named by the linker script. On a cold lane those
# objects do not exist yet. The first full build materializes them and may fail
# only at the final resident-call link; regenerate the surface against that
# complete inventory before the authoritative build/ROM proof.
mkdir -p build/permuter
bootstrap_log=$(mktemp build/permuter/sweep-bootstrap.log.XXXXXXXX)
bootstrap_surface=$(mktemp build/permuter/sweep-overlay-syms.XXXXXXXX)
cp overlay_undefined_syms.us.txt "$bootstrap_surface"
bootstrap_status=0
gmake "-j$build_jobs" >"$bootstrap_log" 2>&1 || bootstrap_status=$?
if [ "$bootstrap_status" -ne 0 ]; then
    if ! .venv/bin/python -B - "$bootstrap_log" <<'PY'
import re
import sys
from pathlib import Path

lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
has_link_diagnosis = any(
    line.strip() == "link failed with undefined resident references or R_MIPS_26 overflows."
    for line in lines
)
overflow_lines = [line for line in lines if "relocation truncated to fit:" in line]
only_expected_overflows = bool(overflow_lines) and all(
    re.search(r"relocation truncated to fit: R_MIPS_26 against `[^`']+['`]", line)
    for line in overflow_lines
)
make_errors = [line for line in lines if re.search(r"gmake(?:\[\d+\])?: \*\*\* .* Error \d+", line)]
link_target_failed = any(
    re.fullmatch(r"gmake\[1\]: \*\*\* \[Makefile:\d+: build/mickey\.us\.elf\] Error 1", line)
    for line in make_errors
)
only_expected_make_errors = bool(make_errors) and all(
    re.fullmatch(r"gmake\[1\]: \*\*\* \[Makefile:\d+: build/mickey\.us\.elf\] Error 1|gmake: \*\*\* \[Makefile:\d+: all\] Error 2", line)
    for line in make_errors
)
compiler_error = any(re.search(r"\b(?:fatal error\b|error:)", line, re.I) for line in lines)
sys.exit(0 if (has_link_diagnosis and only_expected_overflows and link_target_failed
               and only_expected_make_errors and not compiler_error) else 1)
PY
    then
        cat "$bootstrap_log" >&2
        printf 'permute-sweep: cold bootstrap failed outside the diagnosed resident R_MIPS_26 link case (log: %s/%s)\n' "$physical" "$bootstrap_log" >&2
        exit "$bootstrap_status"
    fi
    gmake overlay-syms >/dev/null
    if ! .venv/bin/python -B - "$bootstrap_log" "$bootstrap_surface" overlay_undefined_syms.us.txt <<'PY'
import hashlib
import re
import sys
from pathlib import Path

log, before_path, after_path = map(Path, sys.argv[1:])
before, after = before_path.read_bytes(), after_path.read_bytes()
text = log.read_text(errors="replace")
names = set(re.findall(
    r"relocation truncated to fit: R_MIPS_26 against `([^`']+)['`]", text
))
def assignments(data):
    return {
        m.group(1): m.group(2)
        for m in re.finditer(
            r"^\s*([A-Za-z_]\w*)\s*=\s*([^;]+);", data.decode(), re.M
        )
    }

old, new = assignments(before), assignments(after)
replaced = bool(names) and all(
    name in old
    and name not in new
    and any(re.fullmatch(re.escape(name) + r"_o\d{3}Reloc", key) for key in new)
    for name in names
)
changed = hashlib.sha256(before).digest() != hashlib.sha256(after).digest()
sys.exit(0 if changed and replaced else 1)
PY
    then
        printf 'permute-sweep: resident alias surface did not change to replace each failed resident symbol; refusing retry (bootstrap log: %s/%s)\n' "$physical" "$bootstrap_log" >&2
        exit 2
    fi
    printf 'cold bootstrap link needs resident object inventory; regenerated resident aliases (%s)\n' "$bootstrap_log" >&2
else
    gmake overlay-syms >/dev/null
fi
final_build_log=$(mktemp build/permuter/sweep-build.log.XXXXXXXX)
final_build_status=0
gmake "-j$build_jobs" >"$final_build_log" 2>&1 || final_build_status=$?
if [ "$final_build_status" -ne 0 ]; then
    cat "$final_build_log" >&2
    printf 'permute-sweep: authoritative build failed (log: %s/%s)\n' "$physical" "$final_build_log" >&2
    exit "$final_build_status"
fi
gmake "-j$build_jobs" verify | tail -1

# Unique logs preserve sequential launches within the same minute.
log=$(mktemp build/permuter/sweep.log.XXXXXXXX)
# Keep the array nonempty for macOS system Bash 3.2 with nounset enabled.
mode_args=(--order ranking --resume)
if [ "$promote" -eq 1 ]; then mode_args+=(--apply --commit); fi
printf 'sweep log: %s/%s\n' "$physical" "$log"
printf 'promotion enabled: %s; exact-context receipts in Git common directory\n' "$promote"
.venv/bin/python -u tools/permute_batch.py \
    "${mode_args[@]}" \
    --jobs 2 --permuter-threads 4 --build-jobs "$build_jobs" \
    --minutes 20 --max-total-minutes 120 --extend-minutes 20 --flat-minutes 6 --load-threshold 13 \
    "$@" 2>&1 | tee "$log"

if [ "$promote" -eq 1 ]; then
    gmake "-j$build_jobs" extract >/dev/null
    gmake overlay-syms >/dev/null
    gmake "-j$build_jobs" >/dev/null
    gmake "-j$build_jobs" verify | tail -1
    .venv/bin/python tools/progress.py --version us | head -6
    printf 'promotions on lane/%s:\n' "$lane"
    git log --oneline "$base..HEAD"
fi
