#!/usr/bin/env bash
# Run the final link, and on failure say what the failure usually means.
#
#   tools/link_diagnose.sh <ld> <ldflags...> -o <out>
#
# Overlays are linked at 0xF0000000 and cannot `jal` a 0x8000xxxx entry point
# directly, so they reach resident functions through the generated *Reloc
# surface: overlay_undefined_syms.us.txt values each placeholder, and
# mk/overlay_resident_rebinds.generated.mk renames the calls adopted C spells
# with the resident name. Both are tracked and the renames are ordinary
# compile rules, so a clean tree links in one pass. When the link reports
# undefined resident references or R_MIPS_26 overflows anyway, the usual cause
# is that the tree changed (a promotion, a new resident call) and the surface
# was not regenerated -- which reads like a defect in someone's source and is
# not.
#
# The success path is untouched: this passes the linker's own output through
# and exits with its status.
set -uo pipefail

log=$(mktemp -t link-diagnose)
trap 'rm -f "$log"' EXIT

"$@" 2>&1 | tee "$log"
status=${PIPESTATUS[0]}
[ "$status" -eq 0 ] && exit 0

# Only advise when the failure carries this signature; a genuine link error
# should not be buried under a guess about build state.
if grep -qE 'R_MIPS_26|relocation truncated|undefined reference' "$log"; then
    cat >&2 <<'MSG'

link failed with undefined resident references or R_MIPS_26 overflows.

This is usually a stale relocation surface rather than a source defect.
Overlays reach resident entry points through *Reloc placeholders that
`gmake overlay-syms` derives from the compiled objects: the values in
overlay_undefined_syms.us.txt and the renames in
mk/overlay_resident_rebinds.generated.mk. A promotion or a new resident call
needs it rerun:

    gmake overlay-syms && gmake

If that fixes it, commit both generated files. See "Promoting an overlay
function" in CLAUDE.md.
MSG
fi
exit "$status"
