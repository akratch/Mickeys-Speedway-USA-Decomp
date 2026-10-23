#include "overlays/overlay_008.h"

/* Tier B: F0894 masks the two selectors to four bits, word-loads these
 * independent tables, and ORs the selected bits into the owner's flags.
 * Preserve signed-word storage and the existing writable data contract. */

s32 gO8Table360[16] = {
    0, ((s32)1 << 7), 0, ((s32)1 << 9),
    ((s32)1 << 11), ((s32)1 << 11), ((s32)1 << 13), ((s32)1 << 13),
    ((s32)1 << 15), 0, ((s32)1 << 17), ((s32)1 << 17),
    ((s32)1 << 19), 0, ((s32)1 << 19), 0,
};

s32 gO8Table3A0[16] = {
    0, ((s32)1 << 6), 0, ((s32)1 << 8),
    ((s32)1 << 10), ((s32)1 << 10), ((s32)1 << 12), ((s32)1 << 12),
    ((s32)1 << 14), 0, ((s32)1 << 16), ((s32)1 << 16),
    ((s32)1 << 18), 0, ((s32)1 << 18), 0,
};

typedef char O8SelectorFlagWidth[(sizeof(s32) == 4) ? 1 : -1];
