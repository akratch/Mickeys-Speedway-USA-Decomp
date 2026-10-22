#include "PR/ultratypes.h"

typedef struct O70State O70State;

typedef struct O70Object {
    s16 angle;
    u8 pad02[2];
    s16 phase;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 value28;
    u8 pad2C[2];
    s16 facing;
    u8 pad30[0x34];
    O70State *state;
    void **targetList;
} O70Object;

struct O70State {
    u8 active;
    u8 type;
    u8 timer;
    s8 countdown;
    s16 angle;
    s16 angleStep;
    s16 verticalStep;
    u8 pad0A[2];
    s32 flags;
    f32 threshold;
    O70Object *related;
};

typedef struct O70RelatedState {
    u8 pad00[0x154];
    f32 verticalOffset;
} O70RelatedState;

typedef struct O70Pair {
    s16 x;
    s16 z;
} O70Pair;

extern s16 gOverlay70PairTableReloc[];
extern s16 gOverlay70HeightTableReloc[];
extern void func_80006EA0(O70Object *object);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void texAnimateSprite(void *target, s32 *flags, s32 mode,
                           f32 *coordinates, s32 ticks);

void func_overlay_070_F00000D8_18C92A0(O70Object *object, s32 ticks) {
    O70State *state;
    O70Object *related;
    O70RelatedState *relatedState;
    O70Pair *pair;
    f32 sine;
    f32 cosine;
    f32 pairX;
    f32 pairZ;
    s32 angle;

    state = object->state;
    related = state->related;
    relatedState = (O70RelatedState *)related->state;

    if (state->active != 0) {
        object->y += (f32)ticks;
        if (state->threshold < object->y) {
            if ((ticks * 8) < state->timer) {
                state->timer -= ticks * 8;
            } else {
                func_80006EA0(object);
            }
        }
    } else {
        if (state->countdown != 0) {
            state->countdown -= ticks;
            if (state->countdown < 0) {
                state->countdown = 0;
            }
        } else {
            angle = state->angle;
            angle += state->angleStep * ticks;
            if (angle >= 0x8000) {
                state->angle = 0x7FFF;
                state->angleStep = mathRnd(-0x200, -0x100);
                state->countdown = mathRnd(0, 0x3C);
            } else if (angle < -0x8000) {
                state->angle = -0x8000;
                state->angleStep = mathRnd(0x100, 0x200);
                state->countdown = mathRnd(0, 0x3C);
            } else {
                state->angle = angle;
            }
        }

        sine = func_8002A8BC(-related->angle);
        cosine = func_8002A8C0(-related->angle);
        pair = (O70Pair *)&gOverlay70PairTableReloc[state->type * 2];
        pairX = (f32)pair->x;
        pairZ = (f32)pair->z;
        object->x = (related->x + (pairX * sine)) - (pairZ * cosine);
        object->y = (f32)gOverlay70HeightTableReloc[state->type + 12] +
                    (related->y + 20.0f) + relatedState->verticalOffset;
        object->z = related->z + (pairZ * sine) + (pairX * cosine);
        object->facing = related->facing;
    }

    object->phase += state->verticalStep * ticks;
    angle = object->phase;
    if (angle >= 0x301) {
        object->phase = 0x600 - angle;
        state->verticalStep = -state->verticalStep;
    } else if (angle < -0x300) {
        object->phase = -0x600 - angle;
        state->verticalStep = -state->verticalStep;
    }
    texAnimateSprite(*object->targetList, &state->flags, 2, &object->value28,
                   ticks);
}
