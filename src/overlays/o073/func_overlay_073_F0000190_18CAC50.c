/* Fresh current-base workbench: structure-mismatch, 755 differing words, first mismatch +0x00;
 * 766 vs 760 instructions with the exact 0x98 frame. The full 119-row flag
 * lattice was nonexact. The candidate has 48 text relocation sites versus 46
 * shipped records, so its +0x18 extent and relocation schedule must be closed
 * before allocation work or a permuter batch is useful. The linked promotion
 * trial stops at relocation schedule divergence. */
#ifdef NON_MATCHING
#include "PR/ultratypes.h"

extern s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode, void *hits);
extern s16 Arctanf(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern void func_80008118(void);
extern void func_80008128(void *object, f32 x, f32 y, f32 z);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);

extern f32 D_20;
extern f32 D_24;
extern f32 D_28;
extern f32 D_2C;
extern f32 D_30;
extern f32 D_34;
extern f32 D_38;
extern f32 D_3C;
extern f32 D_40;
extern f32 D_44;
extern f32 D_48;
extern f32 D_4C;
extern f32 D_50;
extern f32 D_54;

typedef struct Func073State {
    u8 pad00[0x7C];
    u8 vertexBank;
    u8 mode;
    u16 flags;
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 timer;
    s16 angle;
    u16 countdown;
    void *target;
} Func073State;

typedef struct Func073Object {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 timer;
    u8 pad2C[0x20];
    f32 *output;
    u8 pad50[0x14];
    Func073State *state;
} Func073Object;

typedef struct Func073TargetData {
    u8 pad00[0x18];
    f32 x;
    f32 y;
    f32 z;
} Func073TargetData;

typedef struct Func073Target {
    u8 pad00[0x48];
    Func073TargetData *data;
} Func073Target;

void func_overlay_073_F0000190_18CAC50(Func073Object *object, s32 updateRate) {
    s16 absStep;
    s16 stateAngle;
    void *hit;
    s16 *vertex;
    s32 phase;
    s32 horizontal;
    s32 vertical;
    Func073State *state;

    state = object->state;
    stateAngle = state->angle;
    absStep = (s16)(stateAngle * updateRate);
    if (absStep < 0) {
        absStep = -absStep;
    }

    switch (state->mode) {
    case 0: {
        state->timer = 0.0f;
        if (func_8005776C(state->x, 0.0f, state->z, state->radius,
                          1, &hit) != 0) {
            state->countdown = 0xF0;
            state->mode = 3;
            state->target = hit;
        }
        object->timer += (f32)updateRate * D_20;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 1:
    case 2: {
        f32 dx;
        f32 dz;
        f32 dy;
        s16 targetAngle;
        s16 delta;

        if (stateAngle < 0x480) {
            state->angle = stateAngle + (updateRate * 0x10);
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        targetAngle = Arctanf(dx, dz);
        delta = targetAngle - object->angle;
        if (delta < 0) {
            if ((s16)-absStep < delta) {
                object->angle += delta;
            } else {
                object->angle -= absStep;
            }
        } else if (delta > 0) {
            if (delta < absStep) {
                object->angle += delta;
            } else {
                object->angle += absStep;
            }
        }

        dy = state->y - object->y;
        if (dy < 0.0f) {
            object->velocityY -= D_24 * (f32)updateRate;
            if (object->velocityY < dy) {
                object->velocityY = dy;
            }
        } else if (dy > 0.0f) {
            object->velocityY += D_28 * (f32)updateRate;
            if (dy < object->velocityY) {
                object->velocityY = dy;
            }
        } else {
            object->velocityY = 0.0f;
        }

        if (state->mode == 1 &&
            func_8005776C(state->x, 0.0f, state->z, state->radius,
                          1, &hit) != 0) {
            state->countdown = 0x78;
            state->mode = 3;
            state->target = hit;
        }

        if (state->mode != 3) {
            dy = object->y - state->y;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 4.0f) {
                func_80008118();
                func_80008128(object, -dx, -dy, -dz);
                object->velocityY = 0.0f;
                state->mode = 0;
                state->timer = 0.0f;
            }
        }

        object->timer += D_2C * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 3: {
        Func073Target *target;
        Func073TargetData *data;
        f32 dx;
        f32 dz;
        f32 dy;
        s16 targetAngle;
        s16 delta;

        if (stateAngle < 0x480) {
            state->angle = stateAngle + (updateRate * 0x10);
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        target = (Func073Target *)state->target;
        data = target->data;
        dx = data->x - state->x;
        dz = data->z - state->z;
        if ((state->radius * state->radius * 4.0f) <
            ((dx * dx) + (dz * dz))) {
            state->target = NULL;
            if (func_8005776C(state->x, 0.0f, state->z, state->radius,
                              1, &hit) != 0) {
                state->countdown = 0xF0;
                state->target = hit;
            }
        }

        target = (Func073Target *)state->target;
        if (target == NULL) {
            state->mode = 1;
        } else {
            data = target->data;
            dx = object->x - data->x;
            dz = object->z - data->z;
            targetAngle = Arctanf(dx, dz);
            delta = targetAngle - object->angle;
            if (delta < 0) {
                if ((s16)-absStep < delta) {
                    object->angle += delta;
                } else {
                    object->angle -= absStep;
                }
            } else if (delta > 0) {
                if (delta < absStep) {
                    object->angle += delta;
                } else {
                    object->angle += absStep;
                }
            }

            dy = data->y - object->y;
            if (dy < 0.0f) {
                object->velocityY -= D_30 * (f32)updateRate;
                if (object->velocityY < dy) {
                    object->velocityY = dy;
                }
            } else if (dy > 0.0f) {
                object->velocityY += D_34 * (f32)updateRate;
                if (dy < object->velocityY) {
                    object->velocityY = dy;
                }
            }

            if (((dx * dx) + (dz * dz)) < 256.0f) {
                state->mode = 4;
            }
        }

        object->timer += D_38 * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 4: {
        Func073Target *target;
        Func073TargetData *data;
        f32 dx;
        f32 dz;
        f32 dy;
        s32 hitCount;
        s32 hitIndex;

        if (stateAngle >= 0x181) {
            state->angle = stateAngle - (updateRate * 0x10);
            if (state->angle < 0x180) {
                state->angle = 0x180;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        if ((state->radius * state->radius * 4.0f) <
            ((dx * dx) + (dz * dz))) {
            state->mode = 2;
            object->timer += D_48 * (f32)updateRate;
            if (object->timer >= 1.0f) {
                object->timer -= 1.0f;
            }
            goto common;
        }

        if (updateRate < state->countdown) {
            state->countdown -= updateRate;
        } else {
            state->countdown = 0;
            hitCount = func_8005776C(object->x, 0.0f, object->z,
                                     150.0f, 1, &hit);
            if (hitCount == 0) {
                state->target = NULL;
                state->mode = 1;
            } else {
                hitIndex = hitCount;
                if (hitCount >= 2) {
                    hitIndex = mathRnd(1, hitCount);
                }
                state->countdown = 0xF0;
                state->target = ((void **)&hit)[hitIndex - 1];
            }
        }

        if (state->mode == 4) {
            target = (Func073Target *)state->target;
            data = target->data;
            dx = object->x - data->x;
            dz = object->z - data->z;
            if (D_3C < ((dx * dx) + (dz * dz))) {
                state->mode = 3;
            } else {
                if (func_800299E8(0, 0x3FF) >= 0x3EC) {
                    state->angle = -state->angle;
                }
                object->angle += updateRate * state->angle;
                dy = data->y;
                if ((dy + 40.0f) < object->y) {
                    state->flags &= ~1;
                } else if (object->y < dy) {
                    state->flags |= 1;
                }
                if (state->flags & 1) {
                    object->velocityY += D_40 * (f32)updateRate;
                } else {
                    object->velocityY -= D_44 * (f32)updateRate;
                }
                object->angle += state->angle;
            }
        }

        object->timer += D_48 * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    default:
        goto common;
    }

common:
    if (state->mode != 0) {
        if (state->timer < 3.0f) {
            state->timer += 0.25f * (f32)updateRate;
            if (state->timer > 3.0f) {
                state->timer = 3.0f;
            }
        }
    }

    if (state->timer != 0.0f) {
        f32 limit;

        if (state->mode == 4) {
            limit = D_50;
        } else {
            limit = D_4C;
        }
        if (object->velocityY < -limit) {
            object->velocityY = -limit;
        }
        if (limit < object->velocityY) {
            object->velocityY = limit;
        }
        object->velocityX = func_8002A8C0(object->angle) * -state->timer;
        object->velocityZ = func_8002A8BC(object->angle) * -state->timer;
        func_80008128(object, object->velocityX, object->velocityY,
                      object->velocityZ);
    }

    state->vertexBank = 1 - state->vertexBank;
    vertex = (s16 *)((u8 *)state + (state->vertexBank * 0x3C));
    phase = (s32)(object->timer * 32768.0f);
    horizontal = (s32)(func_8002A8C0(phase) * 64.0f);
    vertical = (s32)(func_8002A8BC(phase) * 64.0f);
    if (horizontal < 0) {
        horizontal = -horizontal;
    }
    if (vertical < 0) {
        vertical = -vertical;
    }
    vertex[0] = -horizontal;
    vertex[1] = vertical;
    vertex[5] = -horizontal;
    vertex[6] = vertical;
    vertex[20] = horizontal;
    vertex[21] = vertical;
    vertex[25] = horizontal;
    vertex[26] = vertical;
    if (object->output != NULL) {
        *object->output = (f32)horizontal * D_54 * object->scale;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o073/func_overlay_073_F0000190_18CAC50/func_overlay_073_F0000190_18CAC50.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_073_F0000190_18CAC50:start
 * symbol: func_overlay_073_F0000190_18CAC50
 * score: 755 differing words
 * frame: 0x98
 * relocations: 48
 * first-mismatch: +0x10
 * summary: Fresh current-base frame-exact 760/766 result; promotion stops at relocation schedule divergence (48/46 sites), so close extent before allocator/permuter work.
 * PLATEAU-HANDOFF:func_overlay_073_F0000190_18CAC50:end
 */
