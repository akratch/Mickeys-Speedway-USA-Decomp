#include "PR/ultratypes.h"

/* Tier B: the normal character selector preserves slots0..9; the renderer
 * and colour export read packed RGB, while the primitive-colour word ORs
 * dynamic alpha into the stored zero low byte. The layout selector has
 * four count-derived positions plus the explicit split-screen slot4.
 * Tier D: original names/declarations/grouping; these minimal logical
 * bounds do not validate arbitrary character/count inputs or close the
 * legacy caller views and native/GPU lifetime contracts. IDO alone supplies
 * interobject/end alignment; no sixth coordinate cell is defined. */
#define O56_RGBA(r, g, b, a) \
    (((u32)(r) << 24) | ((u32)(g) << 16) | ((u32)(b) << 8) | (u32)(a))
#ifdef TARGET_N64
typedef char O56MapStyleFieldWidths[
    (sizeof(u32) == 4 && sizeof(s16) == 2) ? 1 : -1];
#endif

u32 o56CharacterColors[10] = {
    O56_RGBA(255, 0, 0, 0),
    O56_RGBA(255, 160, 160, 0),
    O56_RGBA(255, 255, 0, 0),
    O56_RGBA(0, 255, 255, 0),
    O56_RGBA(0, 255, 0, 0),
    O56_RGBA(255, 128, 0, 0),
    O56_RGBA(255, 64, 64, 0),
    O56_RGBA(64, 64, 255, 0),
    O56_RGBA(64, 255, 64, 0),
    O56_RGBA(128, 0, 255, 0),
};

s16 o56LayoutXOffsets[5] = { 0, 0, -20, -100, -100 };
s16 o56LayoutYOffsets[5] = { 0, -60, 0, -60, 0 };
