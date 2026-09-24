#include "PR/ultratypes.h"

typedef struct O22Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O22Vec3f;

typedef struct O22Model {
    u8 pad00[6];
    u16 flags06;
    u8 pad08[0x68];
    void *owner70;
} O22Model;

typedef struct O22State {
    s8 mode;
    u8 flags;
    u8 pad02[2];
    f32 speed;
    O22Vec3f up;
    O22Vec3f normal;
    O22Vec3f contact;
    O22Vec3f previousPosition;
    u8 pad38[4];
    void *soundHandle;
} O22State;

typedef struct O22Object {
    u8 pad00[0xC];
    O22Vec3f position;
    u8 pad18[4];
    O22Vec3f velocity;
    u8 animationState28[6];
    s16 result2E;
    u8 animationState30[0x18];
    O22Model *model;
    u8 pad4C[0x18];
    O22State *state;
    void **animationEntries;
    u8 pad6C[0x14];
    u32 flags80;
} O22Object;

extern f32 D_4;
extern f32 D_8;
extern f32 D_C;
extern u8 D_A7C[];

extern s16 Arctanf(f32 y, f32 x);
extern f32 sqrtf(f32 value);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void trackMakePolylist(s32 count, O22Vec3f *start, O22Vec3f *end,
                              f32 *distance, void *arg4, s32 arg5);
extern s32 func_80010900(O22Vec3f *start, O22Vec3f *end, f32 distance,
                         O22Object *object, void *callback);
extern s32 func_80008128(O22Object *object, f32 x, f32 y, f32 z);
extern void func_overlay_022_F0000D30_1878E38(O22Object *object, s32 flags);
extern void partUpdateTriggers(O22Object *object, s32 updateRate);
extern u32 func_80001620(u16 soundId);
extern void func_800031E8(void *handle);
extern void func_80002FE0(u16 soundId, f32 x, f32 y, f32 z, u8 priority,
                          void **handle);
extern void func_8000309C(void *handle, u8 volume);
extern void func_80036544(void *entry, s32 *mode, s32 animationId,
                          void *state, s32 updateRate);

/* Nested exclusive-block f32 carriers produced the 0xE0 frame. Dropping them
 * closes the frame to 0x90; an unused pointer restores the 8-byte rounding
 * cell. $f20 went away once speed was stored to state before the bounce
 * calls. Size is still 3 words short. */
#ifdef NON_MATCHING
void func_overlay_022_F00002B0_18783B8(O22Object *object, s32 updateRate) {
    O22State *state;
    O22Model *model;
    f32 deltaTime;
    f32 step;
    f32 distance;
    f32 accelerationX;
    f32 accelerationY;
    f32 accelerationZ;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 speed;
    void *pad;
    s16 angle;
    s32 collision;
    s32 animationMode;

    state = object->state;
    deltaTime = (f32)updateRate;
    step = (f32)updateRate;
    distance = D_4;
    object->flags80 = 0;
    model = object->model;

    if (model->flags06 & 2) {
        model->owner70 = 0;
        model = object->model;
        model->flags06 = model->flags06 & ~2;
    }

    if (state->mode == 2) {
        return;
    }

    if ((state->mode == 1) && (state->flags & 2)) {
        crossX = 0.0f - ((state->up.x * -1.0f) * state->up.y);
        crossY = state->up.y * -(state->up.z * -1.0f);
        crossZ = (state->up.x * (state->up.x * -1.0f)) -
                 (-(state->up.z * -1.0f) * state->up.z);
        angle = Arctanf(crossX, crossY);
        accelerationY = -func_8002A8C0(Arctanf(crossZ, sqrtf((crossX * crossX) + (crossY * crossY))));
        accelerationX = func_8002A8C0(angle) * accelerationY;
        accelerationZ = func_8002A8BC(angle) * accelerationY;
    } else {
        accelerationX = 0.0f;
        accelerationZ = 0.0f;
        accelerationY = -1.0f;
    }

    object->position.x += (object->velocity.x * deltaTime) +
                          (0.5f * accelerationX * deltaTime * deltaTime);
    object->position.y += (object->velocity.y * deltaTime) +
                          (0.5f * accelerationY * deltaTime * deltaTime);
    object->position.z += (object->velocity.z * step) +
                          (0.5f * accelerationZ * step * step);
    object->velocity.x = object->velocity.x + (accelerationX * deltaTime);
    object->velocity.y = object->velocity.y + (accelerationY * deltaTime);
    object->velocity.z = object->velocity.z + (accelerationZ * step);
    state->flags = 0;

    trackMakePolylist(1, &state->previousPosition, &object->position,
                      &distance, 0, 1);
    collision = func_80010900(&state->previousPosition, &object->position,
                              distance, object, D_A7C);

    if ((func_80008128(object, 0.0f, 0.0f, 0.0f) != 0) ||
        (object->result2E == -1)) {
        object->position.x = state->previousPosition.x;
        object->position.y = state->previousPosition.y;
        object->position.z = state->previousPosition.z;
        func_80008128(object, 0.0f, 0.0f, 0.0f);
        func_overlay_022_F0000D30_1878E38(object, 5);
        return;
    }

    if (collision & 0x40000000) {
        state->mode = 2;
        state->speed = 0.0f;
    } else if (collision != 0) {
        if (state->flags & 4) {
            speed = (object->velocity.x * object->velocity.x) +
                    (object->velocity.y * object->velocity.y) +
                    (object->velocity.z * object->velocity.z);
            if (speed > 0.0f) {
                speed = sqrtf(speed);
                object->velocity.x /= speed;
                object->velocity.y /= speed;
                object->velocity.z /= speed;
                if (state->mode == 0) {
                    state->mode = 1;
                }
                speed *= D_8;
                state->speed = speed;
                crossX = (state->normal.x * object->velocity.x) +
                         (state->normal.y * object->velocity.y) +
                         (state->normal.z * object->velocity.z);
                crossX = -crossX;
                object->velocity.x =
                    (((2.0f * crossX) * state->normal.x) + object->velocity.x) * state->speed;
                object->velocity.y =
                    (((2.0f * crossX) * state->normal.y) + object->velocity.y) * state->speed;
                object->velocity.z =
                    (((2.0f * crossX) * state->normal.z) + object->velocity.z) * state->speed;

                if (state->speed > 10.0f) {
                    accelerationX = object->position.x;
                    accelerationY = object->position.y;
                    accelerationZ = object->position.z;
                    object->position.x = state->contact.x;
                    object->position.y = state->contact.y;
                    object->position.z = state->contact.z;
                    object->flags80 |= 2;
                    partUpdateTriggers(object, 1);
                    object->position.x = accelerationX;
                    object->position.y = accelerationY;
                    object->position.z = accelerationZ;
                }

                {
                    u32 soundVolume;

                    soundVolume = func_80001620(0x20B);
                    crossY = state->speed * 0.03125f * (f32)soundVolume;
                    if ((f32)soundVolume < crossY) {
                        crossY = (f32)soundVolume;
                    }
                    if (state->soundHandle != 0) {
                        func_800031E8(state->soundHandle);
                    }
                    func_80002FE0(0x20B, object->position.x, object->position.y,
                                  object->position.z, 4, &state->soundHandle);
                    func_8000309C(state->soundHandle, (u8)crossY);
                }
            } else {
                state->speed = 0.0f;
                state->mode = 2;
            }
        } else if (state->flags & 2) {
            object->velocity.y =
                (object->position.y - state->previousPosition.y) / step;
            if (state->mode == 1) {
                speed = (object->velocity.x * object->velocity.x) +
                        (object->velocity.y * object->velocity.y) +
                        (object->velocity.z * object->velocity.z);
                if (speed > 0.0f) {
                    speed = sqrtf(speed);
                    object->velocity.x /= speed;
                    object->velocity.y /= speed;
                    object->velocity.z /= speed;
                    speed -= D_C * step;
                    object->velocity.x *= speed;
                    object->velocity.y *= speed;
                    object->velocity.z *= speed;
                    state->speed = speed;
                }
                if (speed <= 0.0f) {
                    state->speed = 0.0f;
                    state->mode = 2;
                }
            }
        }
    } else {
        state->speed = sqrtf((object->velocity.x * object->velocity.x) +
                             (object->velocity.y * object->velocity.y) +
                             (object->velocity.z * object->velocity.z));
    }

    if (state->mode == 0) {
        object->flags80 |= 1;
        partUpdateTriggers(object, updateRate);
    }

    state->previousPosition.x = object->position.x;
    state->previousPosition.y = object->position.y;
    state->previousPosition.z = object->position.z;
    speed = state->speed;
    if (speed < 10.0f) {
        speed = 10.0f;
    }
    animationMode = 9;
    func_80036544(*object->animationEntries, &animationMode, (s32)speed,
                  object->animationState28, updateRate);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o022/func_overlay_022_F00002B0_18783B8/func_overlay_022_F00002B0_18783B8.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_022_F00002B0_18783B8:start
 * symbol: func_overlay_022_F00002B0_18783B8
 * score: 448/499 words
 * frame: 0x90
 * relocations: 29
 * first-mismatch: +0x1C
 * summary: Pair 4 +0x5F0 owns L213 stack-load, L224 move, L237 stack-load. Unowned call gone. f20 if speed crosses calls; volatile reload stays masked 448.
 * PLATEAU-HANDOFF:func_overlay_022_F00002B0_18783B8:end
 */
