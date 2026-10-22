#include "PR/ultratypes.h"

typedef struct O26Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O26Vec3f;

typedef struct O26Entity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x68];
    s32 owner70;
} O26Entity;

typedef struct O26State {
    s32 word00;
    s32 word04;
    u8 pad08[0x1C];
    f32 value24;
    s32 flags28;
    s16 short2C;
    u16 short2E;
} O26State;

typedef struct O26Object {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    O26Vec3f position;
    u8 pad18[4];
    O26Vec3f direction;
    u8 pad28[6];
    s16 result2E;
    u8 pad30[0x18];
    O26Entity *entity;
    u8 pad4C[0x18];
    O26State *state;
    u8 pad68[0x18];
    s32 active80;
} O26Object;

typedef struct O26Config {
    u8 pad00[0xA];
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    s32 word10;
    s32 word14;
} O26Config;

extern u8 func_overlay_026_F0000B18_187AF10[];
extern void mathOneFloatRPY(O26Object *object, O26Vec3f *vector);
extern void trackMakePolylist(s32 count, O26Vec3f *position,
                          O26Vec3f *origin, f32 *radius,
                          s32 arg4, s32 arg5);
extern s32 func_80010900(O26Vec3f *position, O26Vec3f *origin,
                         f32 radius, O26Object *object, void *callback);
extern void func_overlay_026_F0000D24_187B11C(O26Object *object, s32 mode);
extern void mathOneFloatPY(O26Object *object, O26Vec3f *direction);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);

void func_overlay_026_F0000000_187A3F8(O26Object *object, O26Config *config) {
    O26State *state;
    f32 radius;
    O26Vec3f position;
    O26Vec3f motion;

    state = object->state;
    radius = 10.0f;
    object->entity->owner70 = config->word10;
    object->entity->flags |= 2;
    object->angleX = config->angleX;
    object->angleY = config->angleY;
    object->angleZ = config->angleZ;
    object->active80 = 1;
    state->short2C = (u16)config->angleX;
    state->word00 = config->word10;
    state->word04 = config->word14;
    state->short2E = 1;
    state->value24 = 12288.0f;

    position.x = object->position.x;
    position.y = object->position.y;
    position.z = object->position.z;
    motion.x = 0.0f;
    motion.y = 0.0f;
    motion.z = -38.0f;
    mathOneFloatRPY(object, &motion);
    object->position.x += motion.x;
    object->position.y += motion.y;
    object->position.z += motion.z;
    trackMakePolylist(1, &position, &object->position, &radius, 0, 0);

    if ((func_80010900(&position, &object->position, radius, object, func_overlay_026_F0000B18_187AF10) != 0) &&
        ((state->flags28 & 4) != 0)) {
        func_overlay_026_F0000D24_187B11C(object, 5);
    } else {
        object->direction.z = -33.0f;
        mathOneFloatPY(object, &object->direction);
        object->result2E = func_8000FAE0(object->position.x,
                                        object->position.y,
                                        object->position.z);
    }
}
