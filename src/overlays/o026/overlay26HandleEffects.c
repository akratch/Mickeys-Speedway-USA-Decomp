#include "PR/ultratypes.h"

typedef struct O26Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O26Vec3f;

typedef struct O26EffectRecord {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[2];
    f32 scale;
    O26Vec3f position;
    O26Vec3f direction;
    s16 randomX;
    s16 randomY;
    s16 randomZ;
    s16 alpha;
} O26EffectRecord;

typedef struct O26StateD24 {
    u8 pad00[0x14];
    O26Vec3f sourceDirection;
    u8 pad20[0x10];
    u8 active30;
    u8 pad31[3];
    O26EffectRecord effects[4];
} O26StateD24;

typedef struct O26EntityD24 {
    u8 pad00[6];
    u16 flags;
} O26EntityD24;

typedef struct O26ObjectD24 {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    s16 flags06;
    f32 scale;
    O26Vec3f position;
    u8 pad18[0x30];
    O26EntityD24 *entity;
    u8 pad4C[0x18];
    O26StateD24 *state;
    u8 pad68[0x18];
    s32 active80;
} O26ObjectD24;

typedef struct O26Angles2 {
    s16 x;
    s16 y;
} O26Angles2;

extern void func_80006EA0(O26ObjectD24 *object);
extern s32 func_8002A910(f32 first, f32 second);
extern f32 sqrtf(f32 value);
extern void func_80029FE4(O26Angles2 *angles, O26Vec3f *direction);
extern s32 func_8002997C(s32 minimum, s32 maximum);
extern void func_8003EDEC(O26ObjectD24 *object, s32 mode);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z,
                          s32 priority, s32 unused);

/* Removing both redundant input aliases recovers the local and temporary
 * stack homes together without changing the effect operations. */
void func_overlay_026_F0000D24_187B11C(O26ObjectD24 *object, s32 mode) {
    O26StateD24 *state;
    O26EffectRecord *effect;
    O26Angles2 angles;
    s16 azimuth;
    s16 elevation;

    state = object->state;
    if (mode & 1) {
        func_80006EA0(object);
    } else if (mode & 2) {
        azimuth = func_8002A910(state->sourceDirection.x,
                               state->sourceDirection.z);
        elevation = func_8002A910(
            sqrtf((state->sourceDirection.z * state->sourceDirection.z) +
                  (state->sourceDirection.x * state->sourceDirection.x)),
            state->sourceDirection.y);

        angles.x = azimuth;
        angles.y = elevation + 0x3000;
        effect = &state->effects[0];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.y = elevation + 0x2000;
        angles.x = azimuth;
        effect = &state->effects[1];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.x = azimuth - 0x2000;
        angles.y = elevation + 0x2000;
        effect = &state->effects[2];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.x = azimuth + 0x2000;
        angles.y = elevation + 0x2000;
        effect = &state->effects[3];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;
        state->active30 = 1;
    }

    if (mode & 4) {
        object->active80 |= 2;
        func_8003EDEC(object, 1);
    }
    if (mode & 8) {
        amSndPlayXYZ(0x278, object->position.x, object->position.y,
                      object->position.z, 4, 0);
    }
    if (mode & 0x10) {
        amSndPlayXYZ(0x27A, object->position.x, object->position.y,
                      object->position.z, 4, 0);
    }
    object->entity->flags &= ~1;
    object->flags06 |= 0x400;
}
