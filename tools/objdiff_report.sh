#!/usr/bin/env bash
# Runs objdiff-cli's progress report over this project's objdiff.json and
# prints per-object match percentages.
#
#   ./tools/objdiff_report.sh              # summary + worst matches
#   ./tools/objdiff_report.sh --full        # every unit, not just the worst
#
# objdiff.json and tools/objdiff_exclude.txt are regenerated from the current
# build/ tree on every run (cheap: well under a second even for this
# project's ~1400 objects), so there's no separate --regen step.
#
# Needs tools/objdiff/objdiff-cli (tools/setup_objdiff.sh) and expected/build/
# (tools/make_expected.sh, itself needs a `gmake verify`-clean build/).
set -eu
cd "$(dirname "$0")/.."

OBJDIFF=tools/objdiff/objdiff-cli
full=0
for a in "$@"; do
    case "$a" in
        --full) full=1 ;;
        *) echo "$0: unknown argument '$a'" >&2; exit 2 ;;
    esac
done

if [ ! -x "$OBJDIFF" ]; then
    echo "$0: $OBJDIFF not found -- run tools/setup_objdiff.sh first." >&2
    exit 1
fi
if [ ! -d expected/build ]; then
    echo "$0: expected/build/ not found -- run tools/make_expected.sh first." >&2
    exit 1
fi
if [ ! -d build ]; then
    echo "$0: build/ not found -- run gmake first." >&2
    exit 1
fi

# Objects with a Makefile POSTPROCESS override (trim_elf_section.py leaves a
# .text section header shorter than the ELF's own symbol table expects;
# normalize_elf_instructions.py similarly rewrites individual words after the
# fact) are excluded via tools/objdiff_exclude.txt -- objdiff-cli's report
# generator aborts the *entire* batch on the first such object it can't parse
# ("Section symbol without section", or for some, an unattributed "Symbol
# data out of bounds" with no file name at all to key a targeted exclusion
# off of). 686 of this project's ~832 C objects carry a POSTPROCESS override
# (`grep -h -c ': POSTPROCESS' Makefile mk/*.mk`), which is most of the overlay
# tree.
#
# tools/objdiff_exclude.txt is derived (a grep over the Makefile, nothing
# ROM-derived) but gitignored rather than committed: it's ~700 lines of
# nothing but object-file basenames, which reads as high-entropy text to
# tools/cleanroom_detectors.py's base64-volume heuristic (a real false
# positive on this specific file shape, not a loophole -- see
# docs/CLEANROOM.md). Regenerated fresh on every run instead, before
# objdiff.json (which reads it).
EXCLUDE_FILE=tools/objdiff_exclude.txt
grep -h -oE '^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[A-Za-z0-9_/]+\.c\.o: POSTPROCESS' \
    Makefile mk/*.mk | sed -E 's#\$\(BUILD_DIR\)/\$\(SRC_DIR\)/#src/#; s/: POSTPROCESS$//' \
    | sort -u > "$EXCLUDE_FILE"
# This is a known scope limit (docs/tools.md): objdiff currently reports on
# the un-postprocessed objects only, which is still the large majority of
# code needing normal (non-normalized) matching. The retry loop below is a
# defensive fallback for any object that slips through with a "Failed to
# open ... .o" error (which, unlike "Symbol data out of bounds", does name
# the file).

echo "Regenerating objdiff.json from the current build/ tree..." >&2
.venv/bin/python tools/gen_objdiff_config.py > objdiff.json

report=$(mktemp "${TMPDIR:-/tmp}/objdiff-report.XXXXXX.json")
errlog=$(mktemp "${TMPDIR:-/tmp}/objdiff-err.XXXXXX.log")
# objdiff-cli reads ./objdiff.json, which the retry loop rewrites with the local
# exclusions applied; put the canonical (exclusion-free) file back on the way
# out so the working tree is not left dirty.
trap 'rm -f "$report" "$errlog"; .venv/bin/python tools/objdiff_config.py >/dev/null 2>&1' EXIT

attempt=0
while [ "$attempt" -lt 20 ]; do
    attempt=$((attempt + 1))
    if "$OBJDIFF" report generate -p . -o "$report" -f json -d 2>"$errlog"; then
        cat "$errlog" >&2
        break
    fi
    bad=$(grep -oE 'Failed to open \./(expected/build/|build/)\S+\.o' "$errlog" \
          | sed -E 's#^Failed to open \./(expected/build/|build/)##' | head -1)
    if [ -z "$bad" ]; then
        cat "$errlog" >&2
        echo "$0: objdiff-cli failed in a way this script doesn't know how to" \
             "recover from (see above)." >&2
        exit 1
    fi
    if grep -qxF "$bad" "$EXCLUDE_FILE"; then
        cat "$errlog" >&2
        echo "$0: '$bad' is already excluded but objdiff-cli still failed on" \
             "it -- giving up." >&2
        exit 1
    fi
    echo "objdiff-cli can't parse $bad (POSTPROCESS-trimmed object); excluding" \
         "it and retrying." >&2
    echo "$bad" >> "$EXCLUDE_FILE"
    .venv/bin/python tools/gen_objdiff_config.py > objdiff.json
done
if [ "$attempt" -ge 20 ]; then
    echo "$0: gave up after 20 exclusions." >&2
    exit 1
fi

.venv/bin/python - "$report" "$full" <<'PYEOF'
import json
import sys

report_path, full = sys.argv[1], sys.argv[2] == "1"
data = json.load(open(report_path))
units = data.get("units", [])

rows = []
grand_total = grand_matched = 0
for u in units:
    m = u.get("measures", {})
    total = int(m.get("total_code") or 0)
    matched = int(m.get("matched_code") or 0)
    grand_total += total
    grand_matched += matched
    if total == 0:
        continue
    pct = 100.0 * matched / total
    rows.append((pct, u.get("name", "?"), matched, total))

rows.sort()

print(f"{len(rows)} units with code, {len(units)} total")
if grand_total:
    print(f"overall: {grand_matched}/{grand_total} bytes matched ({100.0*grand_matched/grand_total:.2f}%)")
print()

shown = rows if full else rows[:40]
label = "all units" if full else "worst 40 (of nonzero-size units; pass --full for all)"
print(f"--- {label} ---")
for p, name, matched, total in shown:
    print(f"{p:6.2f}%  {matched:>7}/{total:<7}  {name}")
PYEOF
