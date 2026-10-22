#include "PR/ultratypes.h"

typedef struct Overlay86Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay86Vec3f;

typedef struct Overlay86Resource {
    u8 pad00[0x2D];
    u8 active;
} Overlay86Resource;

typedef struct Overlay86Current {
    Overlay86Resource *resource;
    u8 pad04[0x3C];
    Overlay86Vec3f *position;
} Overlay86Current;

typedef struct Overlay86Object {
    s16 angle;
    u8 pad02[0x0A];
    Overlay86Vec3f position;
    u8 pad18[0x50];
    Overlay86Current **currentSlot;
} Overlay86Object;

typedef struct Overlay86State {
    u8 pad00;
    s8 vectorIndex;
} Overlay86State;

typedef struct Overlay86Transform {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[2];
    f32 scale;
    Overlay86Vec3f position;
    u8 pad18[0x16];
    s16 resultAngle;
    u8 pad30[0x34];
    Overlay86State *state;
} Overlay86Transform;

extern Overlay86Vec3f gOverlay86Vectors[10];
extern void overlay86ProcessCurrent(Overlay86Object *object);
extern void overlay86TransformVectorReloc(s32 mode,
                                          Overlay86Transform *transform,
                                          Overlay86Vec3f *input,
                                          Overlay86Vec3f *output);
extern s16 func_8000FAE0(f32 x, f32 y, f32 z);

void overlay86BuildTransform(Overlay86Object *object,
                             Overlay86Transform *transform) {
    s32 index;
    volatile Overlay86Vec3f target;
    Overlay86Vec3f vector;
    Overlay86Current *current;
    s16 angle;

    index = transform->state->vectorIndex;
    if ((index < 0) || (index >= 10)) {
        index = 0;
    }
    current = *object->currentSlot;
    if ((current != 0) && (current->resource != 0) &&
        (current->resource->active != 0)) {
        overlay86ProcessCurrent(object);
        target.x = current->position->x;
        target.y = current->position->y;
        target.z = current->position->z;
    } else {
        target.x = object->position.x;
        target.y = object->position.y;
        target.z = object->position.z;
    }
    transform->angle = object->angle + 0x8000;
    transform->tilt = 0;
    transform->roll = 0;
    vector.x = gOverlay86Vectors[index].x * transform->scale;
    vector.y = gOverlay86Vectors[index].y * transform->scale;
    vector.z = gOverlay86Vectors[index].z * transform->scale;
    overlay86TransformVectorReloc(1, transform, &vector, &vector);
    transform->position.x = target.x - vector.x;
    transform->position.y = target.y - vector.y;
    transform->position.z = target.z - vector.z;
    angle = func_8000FAE0(transform->position.x,
                                          transform->position.y,
                                          transform->position.z);
    if (angle != -1) {
        transform->resultAngle = angle;
    }
}
