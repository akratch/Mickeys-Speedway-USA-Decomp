#include "PR/ultratypes.h"

/* Effect initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay23State {
    f32 velocityX;
    f32 velocityY;
    f32 scale;
    s16 timer;
    s16 halfTimer;
    s16 angle0;
    s16 angle1;
    s16 randomStep;
    u8 pad16[2];
    f32 alpha;
    f32 alphaStep;
    void *asset;
} Overlay23State;

typedef struct Overlay23Transform {
    s16 flags;
    u8 pad2[2];
    f32 scale;
} Overlay23Transform;

typedef struct Overlay23Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad6[2];
    f32 scale;
    u8 padC[0x10];
    f32 x;
    f32 y;
    f32 z;
    u8 pad28[0x11];
    u8 alpha;
    u8 pad3A[0x16];
    s32 renderFlags;
    u8 pad54[0x10];
    Overlay23State *state;
    u8 pad68[0x10];
    Overlay23Transform *transform;
} Overlay23Object;

typedef struct Overlay23InitData {
    u8 pad0[0xA];
    s16 angle0;
    s16 angle1;
    s16 angle2;
    s16 assetId;
    s16 timer;
    s16 stateAngle0;
    s16 stateAngle1;
    f32 stateScale;
    f32 objectScale;
    f32 x;
    f32 y;
    f32 z;
    f32 velocityX;
    f32 velocityY;
} Overlay23InitData;

s32 mathRnd(s32 minimum, s32 maximum);
void *texLoadSprite(s32 assetId, s32 mode);

void overlay23Init(Overlay23Object *object, Overlay23InitData *init) {
    Overlay23State *state;
    Overlay23Transform *transform;
    f32 combinedScale;

    state = object->state;
    state->velocityX = init->velocityX;
    state->velocityY = init->velocityY;
    state->scale = init->stateScale;
    state->timer = init->timer;
    state->halfTimer = init->timer >> 1;
    state->angle0 = init->stateAngle0;
    state->angle1 = init->stateAngle1;
    state->randomStep = mathRnd(0x20, 0x40);
    state->alpha = 255.0f;
    state->randomStep *= 8;
    state->alphaStep = 255.0f / (f32)state->halfTimer;
    state->asset = texLoadSprite(init->assetId, 1);
    if (mathRnd(0, 99) < 50) {
        state->randomStep = -state->randomStep;
    }

    object->angle0 = init->angle0;
    object->angle1 = init->angle1;
    object->angle2 = init->angle2;
    object->x = init->x;
    object->y = init->y;
    object->z = init->z;
    object->scale = init->objectScale;
    transform = object->transform;
    combinedScale = object->scale * state->scale;
    transform->scale *= combinedScale;
    transform = object->transform;
    transform->flags &= ~2;
}
