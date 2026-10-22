#include "PR/ultratypes.h"

/* Per-frame effect update; exact DKR and JFG scans are negative. */
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
    u8 pad0[0x1C];
    f32 velocityX;
    f32 velocityY;
} Overlay23Transform;

typedef struct Overlay23Object {
    u8 pad0[0x39];
    s8 alpha;
    u8 pad3A[0x2A];
    Overlay23State *state;
    u8 pad68[0x10];
    Overlay23Transform *transform;
} Overlay23Object;

extern f32 gOverlay23VelocityFactor;
f32 Powerf(f32 factor);
void func_80006EA0(Overlay23Object *object, s32 updateRate);

void overlay23Update(Overlay23Object *object, s32 updateRate) {
    Overlay23State *state;
    f32 damping;
    f32 updateRateF;
    s16 timer;

    state = object->state;
    state->angle0 = state->angle0 + (state->randomStep * updateRate);
    state->timer = state->timer - updateRate;
    damping = Powerf(gOverlay23VelocityFactor);
    state->velocityX = state->velocityX * damping;
    state->velocityY = state->velocityY * damping;
    updateRateF = (f32)updateRate;
    object->transform->velocityX = state->velocityX * updateRateF;
    object->transform->velocityY = state->velocityY * updateRateF;

    timer = state->timer;
    if (timer <= 0) {
        state->alpha = 0.0f;
        func_80006EA0(object, updateRate);
        return;
    }
    if (timer < state->halfTimer) {
        state->alpha -= state->alphaStep * updateRateF;
        if (state->alpha < 0.0f) {
            state->alpha = 0.0f;
        }
        object->alpha = state->alpha;
    }
}
