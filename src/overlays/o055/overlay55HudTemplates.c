#include "PR/ultratypes.h"

/* Tier B: resident -1-terminated resource lists; mutable cache-index words
 * patched to CPU pointers, copied/rendered in sixteen-byte nodes; actual
 * kind-zero callers and screen-mode selection bound eight offset cells.
 * Tier D: original declarations/names and unread null terminal members;
 * complete null-node format and independent next-object bounds support the
 * terminal records. Legacy integer helper views and native pointer/lifetime
 * contracts remain open. No BSS or player-alpha capacity is reconstructed.
 */
typedef union O55HudResource {
    s32 cacheIndex;
    void *resource;
} O55HudResource;
typedef struct O55HudNode {
    O55HudResource texture;
    O55HudResource alternate;
    u32 packedGlyph;
    s16 x;
    s16 y;
} O55HudNode;
#ifdef TARGET_N64
typedef char O55HudResourceTargetSize[
    (sizeof(O55HudResource) == 4 && sizeof(void *) == 4) ? 1 : -1];
typedef char O55HudNodeTargetSize[(sizeof(O55HudNode) == 16) ? 1 : -1];
#endif

s16 o55LoadResourceIds[11] = { 2, 25, 26, 4, 20, 21, 22, 23, 24, 53, -1 };
s16 o55SetupObjectIds[3] = { 4, 2, -1 };

O55HudNode o55TimeTemplate[10] = {
    { { 20 }, { 21 }, 0, 0, 0 },
    { { 20 }, { 21 }, 0, 7, 0 },
    { { 20 }, { 21 }, 0xb0000, 14, 0 },
    { { 20 }, { 21 }, 0, 20, 0 },
    { { 20 }, { 21 }, 0, 27, 0 },
    { { 20 }, { 21 }, 0xa0000, 33, 0 },
    { { 20 }, { 21 }, 0, 40, 0 },
    { { 20 }, { 21 }, 0, 47, 0 },
    { { 23 }, { 24 }, 0, -25, -8 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

O55HudNode o55ClockTemplate[2] = {
    { { 25 }, { 0 }, 0, -25, -6 },
    { { 0 }, { 0 }, 0, 0, 0 },
};

s16 o55HudOffsets[8] = { 76, 33, 215, 141, 76, 25, 215, 145 };
