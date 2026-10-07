/* NON_MATCHING: query output storage, ten-byte vertex banks and observed
 * automatic homes are reconstructed. Radius limits and shared zero values
 * have explicit, nonoverlapping local roles. Signed steering comparisons
 * preserve promoted negation, including the signed-halfword minimum.
 * The entry multiply sits in its own region (do/while) so the step is
 * narrowed once, as shipped. 2026-10-07 (lane f-o073), 14 -> 4 at size 0:
 * the case 0 timer add goes through the float `limit` local so the rate
 * product is numbered before the timer read (a timer read numbered first
 * reserves a 4-byte compiler cell and puts the float-rate spill at +0x30
 * instead of +0x34); the case 4 count assignment, index copy and test sit
 * on one physical line so as1's line tie-break schedules the index
 * narrowing first. Remaining 4: `hitIndex - 1` is an a1 web in the target
 * and a ring temp here; see the owned matching-triage handoff. */
#ifdef NON_MATCHING
#include "PR/ultratypes.h"

struct HitCopyState;
extern s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode,
                       struct HitCopyState **hits);
extern s32 Arctanf(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern void func_80008118(void);
extern void func_80008128(void *object, f32 x, f32 y, f32 z);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);


typedef struct Func073Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Func073Vertex;

typedef struct Func073State {
    Func073Vertex vertices[12];
    void *resource;
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
    s32 horizontal;
    s32 vertical;
    s32 phase;
    f32 height;
    s16 absStep;
    s16 delta;
    f32 dx;
    f32 dy;
    f32 dz;
    s16 targetAngle;
    f32 limit;
    s16 hitCount;
    s32 hitIndex;
    Func073TargetData *data;
    Func073Target *target;
    Func073Vertex *vertex;
    Func073State *state;
    struct HitCopyState *hits[8];

    state = object->state;

    absStep = state->angle;
    do {
        absStep *= updateRate;
    } while (0);
    if (absStep < 0) {
        absStep = -absStep;
    }

    switch (state->mode) {
    case 0: {
        state->timer = 0;
        if ((s16)func_8005776C(state->x, 0.0f, state->z, state->radius,
                          1, hits) != 0) {
            state->target = hits[0];
            state->countdown = 0xF0;
            state->mode = 3;
        }
        limit = (f32)updateRate * 0.004f;
        object->timer += limit;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 1:
    case 2: {

        if (state->angle < 0x480) {
            state->angle += updateRate * 0x10;
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        targetAngle = Arctanf(dx, dz);
        delta = targetAngle - object->angle;
        if (delta < 0) {
            if (-absStep < delta) {
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

        height = state->y - object->y;
        if (height < 0) {
            object->velocityY -= 0.1f * (f32)updateRate;
            if (object->velocityY < height) {
                object->velocityY = height;
            }
        } else if (height > 0) {
            object->velocityY += 0.1f * (f32)updateRate;
            if (height < object->velocityY) {
                object->velocityY = height;
            }
        } else {
            object->velocityY = 0;
        }

        if (state->mode == 1 &&
            (s16)func_8005776C(state->x, 0, state->z, state->radius,
                          1, hits) != 0) {
            state->target = hits[0];
            state->countdown = 0x78;
            state->mode = 3;
        }

        if (state->mode != 3) {
            dy = object->y - state->y;
            if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) < 4.0f) {
                func_80008118();
                func_80008128(object, -dx, -dy, -dz);
                object->velocityY = 0;
                state->mode = 0;
                state->timer = 0.0f;
            }
        }

        object->timer += 0.064f * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 3: {

        if (state->angle < 0x480) {
            state->angle += updateRate * 0x10;
            if (state->angle >= 0x481) {
                state->angle = 0x480;
            }
        }

        data = ((Func073Target *)state->target)->data;
        dx = data->x - state->x;
        dz = data->z - state->z;
        limit = state->radius * state->radius * 4.0f;
        if (limit < ((dx * dx) + (dz * dz))) {
            state->target = NULL;
            if ((s16)func_8005776C(state->x, 0.0f, state->z, state->radius,
                              1, hits) != 0) {
                state->target = hits[0];
                state->countdown = 0xF0;
            }
        }

        target = (Func073Target *)state->target;
        if (target != NULL) {
            data = target->data;
            dx = object->x - data->x;
            dz = object->z - data->z;
            targetAngle = Arctanf(dx, dz);
            delta = targetAngle - object->angle;
            if (delta < 0) {
                if (-absStep < delta) {
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

            height = ((Func073Target *)state->target)->data->y - object->y;
            if (height < 0) {
                object->velocityY -= 0.1f * (f32)updateRate;
                if (object->velocityY < height) {
                    object->velocityY = height;
                }
            } else if (height > 0) {
                object->velocityY += 0.1f * (f32)updateRate;
                if (height < object->velocityY) {
                    object->velocityY = height;
                }
            }

            if (((dx * dx) + (dz * dz)) < 256.0f) {
                state->mode = 4;
            }
        } else {
            state->mode = 1;
        }

        object->timer += 0.064f * (f32)updateRate;
        if (object->timer >= 1.0f) {
            object->timer -= 1.0f;
        }
        goto common;
    }

    case 4: {

        if (state->angle >= 0x181) {
            state->angle -= updateRate * 0x10;
            if (state->angle < 0x180) {
                state->angle = 0x180;
            }
        }

        dx = object->x - state->x;
        dz = object->z - state->z;
        limit = state->radius * state->radius * 4.0f;
        if (limit < ((dx * dx) + (dz * dz))) {
            state->mode = 2;
        } else {

            if (updateRate < state->countdown) {
                state->countdown -= updateRate;
            } else {
                state->countdown = 0;
                hitCount = (s16)func_8005776C(object->x, 0.0f, object->z, 150.0f, 1, hits); hitIndex = hitCount; if (hitIndex != 0) {
                    if (hitCount >= 2) {
                        hitIndex = (s16)mathRnd(1, hitCount);
                    }
                    state->target = hits[(s16)(hitIndex - 1)];
                    state->countdown = 0xF0;
                } else {
                    state->target = NULL;
                    state->mode = 1;
                }
            }

            if (state->mode == 4) {
                data = ((Func073Target *)state->target)->data;
                dx = object->x - data->x;
                dz = object->z - data->z;
                if (22500.0f < ((dx * dx) + (dz * dz))) {
                    state->mode = 3;
                } else {
                    if (func_800299E8(0, 0x3FF) >= 0x3EC) {
                        state->angle = -state->angle;
                    }
                    object->angle += updateRate * state->angle;
                    dy = ((Func073Target *)state->target)->data->y;
                    if ((dy + 40.0f) < object->y) {
                        state->flags &= ~1;
                    } else if (object->y < dy) {
                        state->flags |= 1;
                    }
                    if (state->flags & 1) {
                        object->velocityY += 0.1f * (f32)updateRate;
                    } else {
                        object->velocityY -= 0.1f * (f32)updateRate;
                    }
                    object->angle += state->angle;
                }
            }

        }

        object->timer += 0.064f * (f32)updateRate;
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

        if (state->mode != 4) {
            limit = 1.2f;
        } else {
            limit = 1.6f;
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
    vertex = &state->vertices[state->vertexBank * 6];
    phase = (s32)(object->timer * 32768.0f);
    horizontal = (s32)(func_8002A8C0(phase) * 64.0f);
    vertical = (s32)(func_8002A8BC(phase) * 64.0f);
    if (horizontal < 0) {
        horizontal = -horizontal;
    }
    if (vertical < 0) {
        vertical = -vertical;
    }
    vertex[0].x = -horizontal;
    vertex[0].y = vertical;
    vertex[1].x = -horizontal;
    vertex[1].y = vertical;
    vertex[4].x = horizontal;
    vertex[4].y = vertical;
    vertex[5].x = horizontal;
    vertex[5].y = vertical;
    if (object->output != NULL) {
        *object->output = (f32)horizontal * 0.1f * object->scale;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o073/func_overlay_073_F0000190_18CAC50/func_overlay_073_F0000190_18CAC50.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_073_F0000190_18CAC50:start
 * symbol: func_overlay_073_F0000190_18CAC50
 * score: 4/760 words
 * frame: 0x98
 * relocations: 46
 * first-mismatch: +0x7C8
 * summary: 4 masked at size 0, case 4 query: one ring temp drawn and released between the two narrowings with nothing emitted; no test or argument spelling draws it.
 * PLATEAU-HANDOFF:func_overlay_073_F0000190_18CAC50:end
 */
