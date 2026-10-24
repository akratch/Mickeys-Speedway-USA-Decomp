#include "PR/ultratypes.h"

/* Tier B: the resident list walkers consume signed IDs through -1. The
 * two HUD lists are patched from cache-slot indices to CPU resource pointers,
 * copied with a sixteen-byte stride, then consumed by the texture-node
 * renderer. Two-player loops independently bound the HUD-row pair.
 * Tier D: original declarations/names and the unread terminal-record members;
 * all terminal fields match the complete null-node format. Legacy helper
 * integer aliases and broader pointer/native/lifetime contracts remain open.
 */
typedef union O53HudResource {
    s32 cacheIndex;
    void *resource;
} O53HudResource;
typedef struct O53HudNode {
    O53HudResource texture;
    O53HudResource alternate;
    u32 packedGlyph;
    s16 x;
    s16 y;
} O53HudNode;
#ifdef TARGET_N64
typedef char O53HudResourceTargetSize[
    (sizeof(O53HudResource) == 4 && sizeof(void *) == 4) ? 1 : -1];
typedef char O53HudNodeTargetSize[(sizeof(O53HudNode) == 16) ? 1 : -1];
#endif

s16 o53LoadResourceIds[10] = { 2, 20, 21, 22, 23, 24, 25, 26, 4, -1 };
s16 o53SetupObjectIds[3] = { 2, 4, -1 };

O53HudNode o53TimeTemplate[10] = {
    { { 20 }, { 21 }, 0, 0, 0 },
    { { 20 }, { 21 }, 0, 7, 0 },
    { { 20 }, { 21 }, 0x000B0000, 14, 0 },
    { { 20 }, { 21 }, 0, 20, 0 },
    { { 20 }, { 21 }, 0, 27, 0 },
    { { 20 }, { 21 }, 0x000A0000, 33, 0 },
    { { 20 }, { 21 }, 0, 40, 0 },
    { { 20 }, { 21 }, 0, 47, 0 },
    { { 23 }, { 24 }, 0, -25, -8 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O53HudNode o53ClockTemplate[2] = {
    { { 25 }, { 0 }, 0, -25, -6 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

s16 o53HudRows[2] = { -12, -120 };
