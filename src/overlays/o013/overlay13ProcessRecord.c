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
 * discards the result, so the function is void.
 * s2-b (2026-09-17), matched from 70 at delta 0: no `state` local (its
 * symbol web was a phantom holding v0 in the break block; every test reads
 * `record->state`, whose reload on each loop exit is uopt's PRE of that
 * load); `s32 timer` so the once-masked value is tested without a second
 * `andi`, coupled with the fade loop leaving through a `goto` like the fall
 * loop; `s16 y`, whose dropped narrowing spends the two folded draws that
 * put the vertex temps on the ROM's ring positions; the vertex stores in
 * x, y, z order; and two dead definitions before the fall loop's guard
 * (`velocityX = record->z; velocityY = record->velocityY;`), which uopt
 * deletes after they have numbered the velocities' webs ahead of gravity's
 * and ordered the hoisted loads z-then-velocityY. See
 * docs/lastmile-forwarding-kill.md. */
void overlay13UpdateRecord(Overlay13Record *record, s32 ticks) {
    f32 radius;
    f32 gravity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    s16 y;
    s32 timer;
    s16 *result;

    SetLinkSlot(13, 50, 10);
    if (record->state == 1) {
        /* Dead, and deleted by uopt: these two definitions exist to number the
         * velocities' webs before gravity's and to order the hoisted loads. */
        velocityX = record->z;
        velocityY = record->velocityY;
        if (ticks--) {
            gravity = gOverlay13Gravity;
            velocityX = record->velocityX;
            do {
                velocityZ = record->velocityZ;
                record->x += velocityX;
                record->z += velocityZ;
                record->y += record->velocityY;
                record->velocityZ = velocityZ - gravity;
                if (record->z < record->targetZ) {
                    record->z = record->targetZ;
                    record->state = 2;
                    goto fall_done;
                }
            } while (ticks--);
        }
fall_done:;
    }

    if (record->state == 2) {
        if (ticks--) {
            timer = record->timer;
            do {
                record->timer -= 2;
                timer = record->timer;
                record->phase += 0.5f;
                if (timer == 0) {
                    record->state = 0;
                    gOverlay13ActiveCount--;
                    goto fade_done;
                }
            } while (ticks--);
        }
fade_done:
        if (record->state != 0) {
            record->vertexIndex = 1 - record->vertexIndex;
            result = (s16 *)&record->vertices[record->vertexIndex * 0x28];
            radius = record->scale * (24.0f - 0.1875f * (f32)record->timer);
            y = (s32)record->targetZ;

            result[0] = (s32)(record->x - radius);
            result[1] = y;
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[0] = (s32)(record->x + radius);
            result[1] = y;
            result[2] = (s32)(record->y + radius);
            result += 5;
            result[0] = (s32)(record->x - radius);
            result[1] = y;
            result[2] = (s32)(record->y - radius);
            result += 5;
            result[0] = (s32)(record->x + radius);
            result[1] = y;
            result[2] = (s32)(record->y - radius);
        }
    }
}
