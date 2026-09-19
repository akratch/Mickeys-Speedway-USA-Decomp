#include "PR/ultratypes.h"
typedef struct Overlay3Object { s16 heading; u8 pad02[0x0A]; f32 x; f32 y; f32 z; } Overlay3Object;
typedef struct Overlay3Control { u8 pad000[1]; s8 chanceIndex; u8 pad002[0x198]; u8 mode; u8 pad19B[0x1F2]; u8 cachedIndex; } Overlay3Control;
extern Overlay3Object **overlay3GetSearchObjectsReloc(s32 *count);
extern s32 overlay3PlanarAngleReloc(f32 dx, f32 dz);
extern s32 overlay3AngleDeltaReloc(s16 heading, s32 angle);
extern s32 overlay1EncodeAngleReloc(s32 one, s32 shiftedAngle);
extern s32 overlay2CheckPathReloc(f32 ax, f32 az, f32 bx, f32 bz, s32 angleValue, s32 zero, s32 minusOne, s32 mask);
extern s32 overlay3RandomRangeReloc(s32 low, s32 high);
extern void overlay36Mode3ActionReloc(Overlay3Object *object);
extern void overlay36Mode4ActionReloc(Overlay3Object *object);
extern void overlay36Mode6ActionReloc(Overlay3Object *object);
extern void overlay36Mode7ActionReloc(Overlay3Object *object);
extern u8 gOverlay3ModeChance[];
/* Plateau (batch 17): exact 0x1C4 size; 34 words differ, first at +0x84.
 * Casting the raw angle and shifting only at encode removed nine differences.
 * Flags tie; the remaining blocker is the valid/mode temporary handoff. */
#ifdef NON_MATCHING
s32 overlay3RunCachedModeAction(Overlay3Object *anchor, Overlay3Control *control) {
    s32 count; Overlay3Object **objects; Overlay3Object *target;
    s32 valid; s32 angle; s32 encoded; s32 random;
    objects = overlay3GetSearchObjectsReloc(&count);
    if (control->cachedIndex == 0x7F) return 0;
    target = objects[control->cachedIndex];
    if (target == anchor) return 0;
    angle = overlay3AngleDeltaReloc(anchor->heading,
        overlay3PlanarAngleReloc(anchor->x - target->x, anchor->z - target->z));
    valid = (s16)angle >= -1999;
    if (valid) {
        valid = (s16)angle < 2000;
        if (valid) {
            encoded = overlay1EncodeAngleReloc(1, angle << 16);
            valid = overlay2CheckPathReloc(anchor->x, anchor->z, target->x, target->z,
                                           encoded, 0, -1, 0xFFFF) == 0;
        }
    }
    switch (control->mode) {
        case 3:
            if (valid) {
                random = overlay3RandomRangeReloc(1, 100);
                if (gOverlay3ModeChance[control->chanceIndex] < random) overlay36Mode3ActionReloc(anchor);
            }
            break;
        case 4:
            if (valid) {
                random = overlay3RandomRangeReloc(1, 100);
                if (gOverlay3ModeChance[control->chanceIndex] < random) overlay36Mode4ActionReloc(anchor);
            }
            break;
        case 6: overlay36Mode6ActionReloc(anchor); break;
        case 7: overlay36Mode7ActionReloc(anchor); break;
    }
    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o003/overlay3RunCachedModeAction/func_overlay_003_F00000B8_1859DE8.s")
#endif

/* PLATEAU-HANDOFF:overlay3RunCachedModeAction:start
 * symbol: overlay3RunCachedModeAction
 * score: 34/113 words
 * frame: 0x58
 * relocations: 15
 * first-mismatch: +0x84
 * summary: Packed 25 still misses the +0xE0 copy. Comma-assign leftover generated-subscript and L109 fold or grow; valid cannot take v1.
 * PLATEAU-HANDOFF:overlay3RunCachedModeAction:end
 */
