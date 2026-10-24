#include "PR/ultratypes.h"

/* Tier B: explicit player<2 loops bound the split positions and alpha cells.
 * The copier selects sixteen signed offset cells from screen mode, list kind
 * and player. The transition word and modulo-ten signed byte are actual
 * mutable controls; IDO supplies their interobject alignment.
 * Original declarations/names are Tier D. Public caller/index/lifetime and
 * the legacy pointer views are not closed by these physical definitions.
 */
s16 o53IconSplitX[2] = { 25, 251 };
s16 o53ItemSplitX[2] = { 31, 257 };
s16 o53ClockSplitX[2] = { -80, 26 };
s16 o53HudOffsets[16] = {
    128, 37, 128, 145, 128, 37, 128, 145, 92, 37, 198, 37, 92, 37, 198, 37
};
s32 o53TransitionDone = 0;
s8 o53BlinkCounter = 0;
s32 o53IconAlpha[2] = { 0, 0 };
