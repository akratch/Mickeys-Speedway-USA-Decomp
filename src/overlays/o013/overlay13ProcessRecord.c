#include "PR/ultratypes.h"

typedef struct Overlay13Record {
    u8 pad00[6]; u8 state; u8 timer;
    f32 scale, x, z, y, targetZ, velocityX, velocityZ, velocityY, phase;
    s32 vertexIndex;
    u8 vertices[0x140];
} Overlay13Record;

extern f32 gOverlay13Gravity;
extern s32 gOverlay13ActiveCount;
extern void SetLinkSlot(s32, u16, u16);

/* Reconstructed from the listing 2026-09-16 (lane s1-a): the earlier candidate
 * tested the tick count after decrementing and returned a pointer; the ROM
 * tests the old count (`if (ticks--)`, `while (ticks--)` -- every
 * `or v0,s0,zero` is that test's dead copy, not a result snapshot), reloads
 * `state` on the count exit only, promotes `record->timer` in the fade loop,
 * flips `vertexIndex` through the field (a ring temp, no `index` local), and
 * fills the four vertices through a walking pointer that uopt folds to its
 * final value (the +48 in one block, the +30 in the next). The caller
 * discards the result, so the function is void. 70 masked words remain at
 * delta 0: the preheader load order, the fade exit's delay slot, and the
 * vertex block's schedule from the 0.1875f constant onward. */
#ifdef NON_MATCHING
void overlay13UpdateRecord(Overlay13Record *record, s32 ticks) {
    f32 radius;
    f32 gravity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    s32 y;
    u8 state;
    u8 timer;
    s16 *result;

    SetLinkSlot(13, 50, 10);
    state = record->state;
    if (state == 1) {
        if (ticks--) {
            velocityX = record->velocityX;
            velocityY = record->velocityY;
            gravity = gOverlay13Gravity;
            do {
                velocityZ = record->velocityZ;
                record->x += velocityX;
                record->z += velocityZ;
                record->y += velocityY;
                record->velocityZ = velocityZ - gravity;
                if (record->z < record->targetZ) {
                    record->z = record->targetZ;
                    state = record->state = 2;
                    goto fall_done;
                }
            } while (ticks--);
            state = record->state;
        }
fall_done:;
    }

    if (state == 2) {
        if (ticks--) {
            timer = record->timer;
            do {
                record->timer -= 2;
                timer = record->timer;
                record->phase += 0.5f;
                if (timer == 0) {
                    record->state = 0;
                    gOverlay13ActiveCount--;
                    break;
                }
            } while (ticks--);
            state = record->state;
        }
        if (state != 0) {
            record->vertexIndex = 1 - record->vertexIndex;
            result = (s16 *)&record->vertices[record->vertexIndex * 0x28];
            radius = record->scale * (24.0f - 0.1875f * (f32)record->timer);
            y = (s32)record->targetZ;

            result[1] = y;
            result[0] = (s32)(record->x - radius);
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[1] = y;
            result[0] = (s32)(record->x + radius);
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[1] = y;
            result[0] = (s32)(record->x - radius);
            result[2] = (s32)(record->y - radius);
            result += 5;
            result[1] = y;
            result[0] = (s32)(record->x + radius);
            result[2] = (s32)(record->y - radius);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13ProcessRecord/func_overlay_013_F0000284_186ED9C.s")
#endif

/* PLATEAU-HANDOFF:overlay13UpdateRecord:start
 * symbol: overlay13UpdateRecord
 * score: 70/161 words
 * frame: 0x20
 * relocations: 5
 * first-mismatch: +0x4C
 * summary: Countdown semantics reconstructed from the listing (old-count tests, count-exit reload, promoted timer, walking vertex pointer, void); 70 at delta 0 remain in the preheader load order, the fade exit's delay slot and the vertex block's schedule.
 * PLATEAU-HANDOFF:overlay13UpdateRecord:end
 */
