#include "overlays/overlay_001.h"

typedef struct O1RankState {
    s8 index;
    s8 id;
    u8 pad002[0x1A6];
    u16 flags;
    u8 pad1AA[0x1D9];
    s8 lap;
    u8 pad384;
    u8 rank;
    u8 pad386[0x16];
    f32 offset;
    f32 spacing;
    u8 pad3A4[0x5C];
    s32 total;
    s32 split[20];
    s16 gap;
    s16 gapTimer;
} O1RankState;

typedef struct O1RankObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x4C];
    O1RankState *state;
} O1RankObject;

typedef struct O1RankPair {
    f32 distance;
    f32 angle;
    s32 valid;
} O1RankPair;

extern s32 G_o1_83e0;
extern s32 G_o1_83e4;
extern f32 gO1RankTime;
extern s32 gO1RankWord800C947C;
extern u8 gO1RankOrder[];
extern f32 gO1RankWeights[];
extern u8 gO1RankMode;
extern s32 D_1D74;
extern s32 D_1D78;
extern s32 D_1D8C;
extern s32 D_1D94;
extern O1RankObject *D_1D98;
extern O1RankState *D_1DA0_State;
extern f32 D_1DAC;
extern f32 D_1DB0;
extern f32 D_1DB4;
extern f32 D_1DB8;
extern s8 D_1DC0[];
extern u8 D_1DC8[];
extern u8 D_1DD0[];
extern O1RankPair D_1BA8[][6];

extern void *levelGetLevel(void);
extern O1RankObject **func_80005750(s32 *count);
extern void func_8005830C(s32 elapsed);
extern f32 sqrtf(f32 value);
extern void func_800291D8(s32 elapsed);
extern O1RankObject *func_80005820(s32 index);
extern f32 func_overlay_001_F00000E4_184C4C4(f32 first, f32 second);
extern void *func_overlay_001_F00004B4_184C894(O1RankObject *object);
extern void func_overlay_001_F0001D78_184E158(s32 index, void *level, s32 count);
extern void func_overlay_001_F00028D4_184ECB4(s32 index, void *level, s32 count);
extern void func_overlay_001_F000293C_184ED1C(s32 index, void *level, s32 count);
extern void func_overlay_001_F000296C_184ED4C(void *level, s32 elapsed);
extern void func_overlay_001_F0002AA4_184EE84(s32 elapsed);

/* 2026-10-02 o-ovl7: rewritten from the target listing in the overlay's
 * `while (i--)` loop shape (every loop's `or v0,s7` copy is the post-decrement
 * value), with the relocation identities read per site: the resident rank
 * order and weight tables, the per-object pair table, the mode byte, and the
 * resident callees (levelGetLevel, func_80005750, func_8005830C, sqrtf,
 * func_800291D8, func_80005820). 433 to 186 masked words, size -16 to 0,
 * frame 0xB0 exact.
 * 2026-10-02 p-ovl8: 186 to 6 by giving each region its own locals (L131
 * and checklist 14): the pair loop alone uses `object`; the anchor, rank and
 * leader regions read their object into `other` and its state into
 * `otherState` (the leader too, held in s4 like the pair loop's); the sort
 * reads `swap` before the compare; the bottom loop's previous-rank state is
 * its own `prev` (a0) and its split index its own `k`, with `dz` inlined so
 * the local count (and the +0x54 spill cell) is unchanged.
 * 2026-10-02 w6-o001b: matched by deleting the separate `sum = 0` and
 * writing `for (sum = 0, k = 0; ...)`. That keeps the zero with the
 * induction init, so as1 fills the func_80005820 delay slot with the
 * argument copy. The reversed comma is 2 words. `-Wo,-loopunroll,0` was
 * not load-bearing (still 6 words) and is gone. */
void func_overlay_001_F0002B4C_184EF2C(s32 elapsed) {
    void *level;
    s32 count;
    O1RankObject *object;
    O1RankObject *other;
    O1RankPair *pair;
    O1RankPair *reverse;
    s32 i;
    s32 j;
    O1RankObject **list;
    s32 changed;
    s32 sum;
    f32 anchor;
    O1RankState *state;
    s32 k;
    O1RankState *otherState;
    f32 step;
    f32 swap;
    u8 swapIndex;
    s32 delta;
    O1RankState *prev;

    level = levelGetLevel();
    list = func_80005750(&count);
    if (G_o1_83e4 == 3) {
        func_overlay_001_F0002AA4_184EE84(elapsed);
    } else {
        func_overlay_001_F000296C_184ED4C(level, elapsed);
    }
    func_8005830C(elapsed);
    if (count == 0) {
        return;
    }
    if (G_o1_83e4 == 0) {
        return;
    }
    D_1D94 = elapsed;
    gO1RankTime = elapsed;
    if (gO1RankWord800C947C != 0) {
        D_1D78 = 1;
    } else {
        D_1D78 = 0;
    }
    i = 5;
    while (i--) {
        D_1DD0[i] = D_1DC8[i];
    }

    i = count;
    while (i--) {
        D_1DC0[i] = 0;
        if (func_overlay_001_F00004B4_184C894(list[i]) == NULL) {
            return;
        }
        switch (G_o1_83e4) {
            case 0:
                break;
            case 1:
                func_overlay_001_F0001D78_184E158(i, level, count);
                break;
            case 2:
                func_overlay_001_F00028D4_184ECB4(i, level, count);
                break;
            case 3:
                func_overlay_001_F000293C_184ED1C(i, level, count);
                break;
        }
    }

    if (G_o1_83e4 == 1 && count >= 2) {
        pair = &D_1BA8[0][0];
        i = count * count;
        while (i--) {
            pair->valid = 0;
            pair++;
        }
        i = count;
        while (i--) {
            object = list[i];
            state = object->state;
            j = count;
            while (j--) {
                other = list[j];
                otherState = other->state;
                pair = &D_1BA8[state->index][otherState->index];
                reverse = &D_1BA8[otherState->index][state->index];
                if (i != j) {
                    if (pair->valid == 0) {
                        pair->distance = sqrtf((object->x - other->x) * (object->x - other->x) +
                                               (object->z - other->z) * (object->z - other->z));
                        pair->angle = func_overlay_001_F00000E4_184C4C4(D_1DA0_State->offset, otherState->offset);
                        reverse->distance = pair->distance;
                        reverse->angle = -pair->angle;
                        pair->valid = 1;
                    }
                } else {
                    pair->distance = 0.0f;
                    reverse->distance = 0.0f;
                    pair->angle = 0.0f;
                    reverse->angle = 0.0f;
                }
            }
        }

        do {
            changed = 0;
            i = count - 1;
            while (i--) {
                swap = gO1RankWeights[i];
                if (swap < gO1RankWeights[i + 1]) {
                    gO1RankWeights[i] = gO1RankWeights[i + 1];
                    gO1RankWeights[i + 1] = swap;
                    swapIndex = gO1RankOrder[i];
                    gO1RankOrder[i] = gO1RankOrder[i + 1];
                    gO1RankOrder[i + 1] = swapIndex;
                    changed = 1;
                }
            }
        } while (changed);

        anchor = gO1RankWeights[0];
        i = count;
        while (i--) {
            other = list[gO1RankOrder[i]];
            otherState = other->state;
            if (!(otherState->flags & 1)) {
                anchor = (f32)(otherState->lap * D_1D8C) + otherState->offset;
                break;
            }
        }

        step = D_1DB8 / (f32)D_1D8C;
        i = count;
        while (i--) {
            other = list[gO1RankOrder[i]];
            otherState = other->state;
            if (gO1RankMode == 2) {
                otherState->spacing = (gO1RankWeights[0] - gO1RankWeights[i]) * step + D_1DB0;
            } else {
                otherState->spacing = (anchor - gO1RankWeights[i]) * step + D_1DB0;
            }
            otherState->rank = i;
            if (!(otherState->flags & 1) || (otherState->flags & 0x10)) {
                otherState->spacing = 1.0f;
            } else if (otherState->spacing < D_1DAC) {
                otherState->spacing = D_1DAC;
            } else if (D_1DB4 < otherState->spacing) {
                otherState->spacing = D_1DB4;
            }
        }

        other = list[gO1RankOrder[0]];
        otherState = other->state;
        if (D_1D74 != otherState->id) {
            D_1D74 = otherState->id;
            D_1D98 = other;
        }
    }

    if (G_o1_83e0 == 0) {
        func_800291D8(10);
    }
    i = count;
    while (i--) {
        if (D_1DC0[i] != 0) {
            state = func_80005820(i)->state;
            if (state->rank != 0) {
                prev = func_80005820(gO1RankOrder[state->rank - 1])->state;
                /* sum then k: the other comma order leaves sum in the jal slot. */
                for (sum = 0, k = 0; k < state->lap; k++) {
                    sum += prev->split[k];
                }
                delta = state->total - sum;
                if (delta >= -0x7FFE && delta < 0x7FFF) {
                    if (state->rank == 1 && prev->lap < 3) {
                        prev->gap = delta;
                        prev->gapTimer = 0xB4;
                    }
                    state->gap = -delta;
                    state->gapTimer = 0xB4;
                }
            }
        }
    }
}
