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

extern u8 D_A7C[];

extern s32 Arctanf(f32 y, f32 x);
extern f32 sqrtf(f32 value);
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern void trackMakePolylist(s32 count, O22Vec3f *start, O22Vec3f *end,
                              f32 *distance, void *arg4, s32 arg5);
extern s32 func_80010900(O22Vec3f *start, O22Vec3f *end, f32 distance,
                         O22Object *object, void *callback);
extern s32 func_80008128(O22Object *object, f32 x, f32 y, f32 z);
extern void func_overlay_022_F0000D30_1878E38Reloc(O22Object *object,
                                                  s32 flags);
extern void partUpdateTriggers(O22Object *object, s32 updateRate);
extern u32 func_80001620(u16 soundId);
extern void func_800031E8(void *handle);
extern void func_80002FE0(u16 soundId, f32 x, f32 y, f32 z, u8 priority,
                          void **handle);
extern void func_8000309C(void *handle, u8 volume);
extern void func_80036544(void *entry, s32 *mode, s32 animationId,
                          void *state, s32 updateRate);

/* Matched 2026-10-02 (lane w2-ovlc), rewritten from the listing (402 at -12
 * before). The slope is cross(cross(up, down), up) through the x/y/z locals,
 * which reproduces the up.y spill and the two -1.0 materializations; the
 * same locals carry the position deltas, the normalized bounce velocity and
 * the saved position. accelerationY is the un-negated cosine with
 * -accelerationY in the X/Z products; the reflection reads -dot at each use;
 * vel.z, the slide divide and the friction step read (f32)updateRate. The
 * last 70 words were the frame: the elevation's sqrtf result goes through
 * its own local, because func_8002A8C0(Arctanf(slopeY, sqrtf(...))) makes
 * cfe pool a temp the target does not have (frame 0x98 -> 0x90). */
void func_overlay_022_F00002B0_18783B8(O22Object *object, s32 updateRate) {
    O22State *state;
    f32 updateRateF;
    f32 x;
    f32 y;
    f32 z;
    f32 slopeX;
    f32 slopeY;
    f32 slopeZ;
    f32 distance;
    f32 dot;
    f32 speed;
    f32 length;
    f32 accelerationX;
    f32 accelerationY;
    f32 accelerationZ;
    f32 volume;
    u32 soundVolume;
    s32 collision;
    f32 animationSpeed;
    s32 animationMode;
    s16 angle;

    state = object->state;
    updateRateF = updateRate;
    distance = 14.4f;
    object->flags80 = 0;
    if (object->model->flags06 & 2) {
        object->model->owner70 = 0;
        object->model->flags06 &= ~2;
    }
    if (state->mode == 2) {
        return;
    }
    if ((state->mode == 1) && (state->flags & 2)) {
        x = -(state->up.z * -1.0f);
        y = 0.0f;
        z = state->up.x * -1.0f;
        slopeX = y * state->up.z - z * state->up.y;
        slopeY = z * state->up.x - x * state->up.z;
        slopeZ = state->up.y * x;
        angle = Arctanf(slopeX, slopeZ);
        length = sqrtf(slopeX * slopeX + slopeZ * slopeZ);
        accelerationY = func_8002A8C0(Arctanf(slopeY, length));
        accelerationX = func_8002A8C0(angle) * -accelerationY;
        accelerationZ = func_8002A8BC(angle) * -accelerationY;
    } else {
        accelerationX = 0.0f;
        accelerationZ = 0.0f;
        accelerationY = -1.0f;
    }

    x = object->velocity.x * updateRateF +
        0.5f * accelerationX * updateRateF * updateRateF;
    object->velocity.x += accelerationX * updateRateF;
    y = object->velocity.y * updateRateF +
        0.5f * accelerationY * updateRateF * updateRateF;
    object->velocity.y += accelerationY * updateRateF;
    z = object->velocity.z * updateRateF +
        0.5f * accelerationZ * updateRateF * updateRateF;
    object->velocity.z += accelerationZ * (f32)updateRate;
    object->position.x += x;
    object->position.y += y;
    object->position.z += z;
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
        func_overlay_022_F0000D30_1878E38Reloc(object, 5);
        return;
    }

    if (collision & 0x40000000) {
        state->mode = 2;
        state->speed = 0.0f;
    } else if (collision != 0) {
        if (state->flags & 4) {
            speed = object->velocity.x * object->velocity.x +
                    object->velocity.y * object->velocity.y +
                    object->velocity.z * object->velocity.z;
            if (speed > 0.0f) {
                speed = sqrtf(speed);
                x = object->velocity.x / speed;
                y = object->velocity.y / speed;
                z = object->velocity.z / speed;
                if (state->mode == 0) {
                    state->mode = 1;
                }
                speed *= 0.8f;
                dot = x * state->normal.x + y * state->normal.y +
                      z * state->normal.z;
                object->velocity.x = (2.0f * -dot * state->normal.x + x) * speed;
                object->velocity.y = (2.0f * -dot * state->normal.y + y) * speed;
                object->velocity.z = (2.0f * -dot * state->normal.z + z) * speed;
                if (speed > 10.0f) {
                    x = object->position.x;
                    y = object->position.y;
                    z = object->position.z;
                    object->position.x = state->contact.x;
                    object->position.y = state->contact.y;
                    object->position.z = state->contact.z;
                    object->flags80 |= 2;
                    partUpdateTriggers(object, 1);
                    object->position.x = x;
                    object->position.y = y;
                    object->position.z = z;
                }
                soundVolume = func_80001620(0x20B);
                volume = speed * 0.03125f * (f32)soundVolume;
                if ((f32)soundVolume < volume) {
                    volume = (f32)soundVolume;
                }
                if (state->soundHandle != 0) {
                    func_800031E8(state->soundHandle);
                }
                func_80002FE0(0x20B, object->position.x, object->position.y,
                              object->position.z, 4, &state->soundHandle);
                func_8000309C(state->soundHandle, (u8)volume);
                state->speed = speed;
            } else {
                state->speed = 0.0f;
                state->mode = 2;
            }
        } else if (state->flags & 2) {
            object->velocity.y = (object->position.y -
                                  state->previousPosition.y) / (f32)updateRate;
            if (state->mode == 1) {
                speed = object->velocity.x * object->velocity.x +
                        object->velocity.y * object->velocity.y +
                        object->velocity.z * object->velocity.z;
                if (speed > 0.0f) {
                    speed = sqrtf(speed);
                    object->velocity.x /= speed;
                    object->velocity.y /= speed;
                    object->velocity.z /= speed;
                    speed -= 0.03f * (f32)updateRate;
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
        state->speed = sqrtf(object->velocity.x * object->velocity.x +
                             object->velocity.y * object->velocity.y +
                             object->velocity.z * object->velocity.z);
    }

    if (state->mode == 0) {
        object->flags80 |= 1;
        partUpdateTriggers(object, updateRate);
    }
    state->previousPosition.x = object->position.x;
    state->previousPosition.y = object->position.y;
    state->previousPosition.z = object->position.z;
    animationSpeed = state->speed;
    if (animationSpeed < 10.0f) {
        animationSpeed = 10.0f;
    }
    animationMode = 9;
    func_80036544(*object->animationEntries, &animationMode,
                  (s32)animationSpeed, object->animationState28, updateRate);
}
