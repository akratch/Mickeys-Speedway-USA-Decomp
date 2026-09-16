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
 * s2-b (2026-09-16), 70 to 4 at delta 0: no `state` local (its symbol web
 * was a phantom holding v0 in the break block, and every test reads
 * `record->state`, whose reload on each loop exit is uopt's PRE of that
 * load); `s32 timer` so the once-masked value is tested without a second
 * `andi`, coupled with the fade loop leaving through a `goto` like the fall
 * loop; `s16 y`, whose dropped narrowing spends the two folded draws that
 * put the vertex temps on the ROM's ring positions; and the vertex stores
 * in x, y, z order. The four words left are the preheader's load order
 * (the ROM emits gravity, velocityX, z, velocityY and colours gravity
 * last). See docs/lastmile-forwarding-kill.md. */
#ifdef NON_MATCHING
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13ProcessRecord/func_overlay_013_F0000284_186ED9C.s")
#endif

/* PLATEAU-HANDOFF:overlay13UpdateRecord:start
 * symbol: overlay13UpdateRecord
 * score: 4/161 words
 * frame: 0x20
 * relocations: 5
 * first-mismatch: +0x4C
 * summary: No state local (a phantom v0 web), s32 timer with a goto exit, s16 y (two folded draws) and x/y/z store order take 70 to 4; the preheader's four loads remain, gravity emitted first but coloured last.
 * PLATEAU-HANDOFF:overlay13UpdateRecord:end
 */
