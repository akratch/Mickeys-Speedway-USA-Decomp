#include "PR/ultratypes.h"

typedef struct Overlay86Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay86Vec3f;

typedef struct Overlay86Object {
    u8 pad00[8];
    f32 scale;
    Overlay86Vec3f position;
} Overlay86Object;

typedef struct Overlay86State {
    u8 pad00;
    s8 vectorIndex;
} Overlay86State;

extern Overlay86Vec3f gOverlay86Vectors[10];
extern void pointListRPY(s32 mode, Overlay86Object *object,
                                          f32 *input, f32 *output);

void overlay86ScaledVectorPosition(Overlay86Object *object,
                                   Overlay86State *state, f32 *outX,
                                   f32 *outY, f32 *outZ) {
    s32 index;
    f32 vector[3];

    index = state->vectorIndex;

    if ((index < 0) || (index >= 10)) {
        index = 0;
    }

    vector[0] = gOverlay86Vectors[index].x * object->scale;
    vector[1] = gOverlay86Vectors[index].y * object->scale;
    vector[2] = gOverlay86Vectors[index].z * object->scale;
    pointListRPY(1, object, vector, vector);
    *outX = vector[0] + object->position.x;
    *outY = vector[1] + object->position.y;
    *outZ = vector[2] + object->position.z;
}
