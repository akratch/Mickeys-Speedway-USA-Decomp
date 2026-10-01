#include "overlays/overlay_008.h"

/* Each set bit selects the corresponding ParticleTriggerSlot. The particle
 * updater tests the low bit and shifts once for each slot, so these are
 * one-hot selector-to-trigger maps rather than opaque effect words.
 */
s32 o8EffectBit4LeftTriggerMasks[16] = {
    1 << 0, 1 << 7, 1 << 7, 1 << 9,
    1 << 11, 1 << 11, 1 << 13, 1 << 13,
    1 << 15, 1 << 0, 1 << 17, 1 << 17,
    1 << 19, 1 << 19, 1 << 19, 0,
};

s32 o8EffectBit8RightTriggerMasks[16] = {
    1 << 1, 1 << 6, 1 << 6, 1 << 8,
    1 << 10, 1 << 10, 1 << 12, 1 << 12,
    1 << 14, 1 << 1, 1 << 16, 1 << 16,
    1 << 18, 1 << 18, 1 << 18, 0,
};

s32 o8EffectBit1LeftTriggerMasks[16] = {
    1 << 4, 1 << 7, 1 << 7, 1 << 9,
    1 << 11, 1 << 11, 1 << 13, 1 << 13,
    1 << 15, 1 << 4, 1 << 17, 1 << 17,
    1 << 19, 1 << 19, 1 << 19, 0,
};

s32 o8EffectBit2RightTriggerMasks[16] = {
    1 << 5, 1 << 6, 1 << 6, 1 << 8,
    1 << 10, 1 << 10, 1 << 12, 1 << 12,
    1 << 14, 1 << 5, 1 << 16, 1 << 16,
    1 << 18, 1 << 18, 1 << 18, 0,
};

typedef struct Overlay8EffectColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay8EffectColor;

/* Per-selector RGBA parameters copied to particle effect color fields. */
Overlay8EffectColor o8EffectColorBySelector[16] = {
    /* Selector 0: translucent dark gray. */
    {48, 48, 48, 128},
    /* Selectors 1-2: translucent pale blue. */
    {192, 192, 255, 48},
    {192, 192, 255, 64},
    /* Selectors 3-7 and 10-11: translucent brown/red variants. */
    {96, 64, 32, 128},
    {64, 40, 16, 160},
    {64, 40, 16, 160},
    {96, 64, 32, 128},
    {96, 64, 32, 64},
    /* Selector 8: translucent dark gray. */
    {48, 48, 48, 64},
    /* Selector 9: disabled transparent black. */
    {0, 0, 0, 0},
    {64, 40, 16, 160},
    {64, 40, 16, 160},
    /* Selectors 12-14: translucent pale blue. */
    {192, 192, 255, 128},
    {192, 192, 255, 64},
    {192, 192, 255, 128},
    /* Selector 15: disabled transparent black. */
    {0, 0, 0, 0},
};
