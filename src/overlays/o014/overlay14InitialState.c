#include "overlays/overlay_014.h"

/* Tier B: fixed nine-byte signed-coordinate/alternate-axis/intensity records
 * consumed by overlay14BuildRects. Seventeen drawable records precede the
 * negative first-field terminator; its remaining fields are not drawn.
 * Tier D: the full terminal record is inferred from this fixed-width format
 * and the next object boundary; only its first byte is consumed. No original
 * declaration or draw-capacity/index-safety claim follows from this storage. */
Overlay14PackedRect o14WindowRects[18] = {
    { 0, -4, 0, -4, 1, 4, 0, -3, 26 },
    { 0, -3, 0, -3, 1, 3, 0, -2, 51 },
    { 0, -2, 0, -2, 1, 2, 0, -1, 77 },
    { 0, -1, 0, -1, 1, 1, 0, 0, 102 },
    { 0, -4, 0, -3, 0, -3, 1, 3, 26 },
    { 0, -3, 0, -2, 0, -2, 1, 2, 51 },
    { 0, -2, 0, -1, 0, -1, 1, 1, 77 },
    { 0, -1, 0, 0, 0, 0, 1, 0, 102 },
    { 1, 0, 0, 0, 1, 1, 1, 0, 102 },
    { 1, 1, 0, -1, 1, 2, 1, 1, 77 },
    { 1, 2, 0, -2, 1, 3, 1, 2, 51 },
    { 1, 3, 0, -3, 1, 4, 1, 3, 26 },
    { 0, -1, 1, 0, 1, 1, 1, 1, 102 },
    { 0, -2, 1, 1, 1, 2, 1, 2, 77 },
    { 0, -3, 1, 2, 1, 3, 1, 3, 51 },
    { 0, -4, 1, 3, 1, 4, 1, 4, 26 },
    { 0, 0, 0, 0, 1, 0, 1, 0, 127 },
    { -1, -1, -1, -1, -1, -1, -1, -1, 0 },
};

/* Tier B: preconverted font strings. Mickey's resident conversion table
 * D_8007D594 maps these lowercase letters uniquely to the glyph codes below;
 * the font stream carries each glyph as marker 0x80 then its code. The asset
 * constructor supplies these fallbacks and the line renderer temporarily
 * writes/restores a delimiter, so both arrays are mutable. */
#define O14_GLYPH_r "\x80\xa6"
#define O14_GLYPH_e "\x80\x99"
#define O14_GLYPH_t "\x80\xa8"
#define O14_GLYPH_u "\x80\xa9"
#define O14_GLYPH_n "\x80\xa2"
#define O14_GLYPH_x "\x80\xac"
#define O14_GLYPH_i "\x80\x9d"
char o14ReturnLabel[] = O14_GLYPH_r O14_GLYPH_e O14_GLYPH_t
                        O14_GLYPH_u O14_GLYPH_r O14_GLYPH_n;
char o14ExitLabel[] = O14_GLYPH_e O14_GLYPH_x O14_GLYPH_i O14_GLYPH_t;
#undef O14_GLYPH_r
#undef O14_GLYPH_e
#undef O14_GLYPH_t
#undef O14_GLYPH_u
#undef O14_GLYPH_n
#undef O14_GLYPH_x
#undef O14_GLYPH_i


/* Tier B: mutable menu controls and CPU pointers written by the initializer,
 * rule loader and command updater. The address words retain their legacy
 * target-width representations; other offset-based views are still open.
 * The final eight bytes are ordinary compiler alignment, not capacity. */
s32 o14Opacity = 0;
s32 o14Active = 0;
s32 o14State = 0;
s32 o14ExitRequested = 0;
s32 o14PendingValue = -1;
s32 o14PendingMode = -1;
s32 o14Transition = 0;
s32 o14Cursor = 0;
s32 o14TextHasMore = 0;
s32 o14Mode = 3;
s32 o14ValueCount = 0;
s32 o14CommandCount = 0;
Overlay14RuleGroup * o14RuleGroups = 0;
u32 * o14Ranges = 0;
s32 o14ResultAddress = 0;
s32 o14SelectionAddress = 0;
Overlay14Owner * o14Owner = 0;
s32 o14Current = -1;
s32 o14Next = -1;
s32 o14Context = 0;
s32 o14RuleField110 = 0;
s32 o14RuleField114 = 0;
