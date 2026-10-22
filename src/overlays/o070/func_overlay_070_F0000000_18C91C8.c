#include "PR/ultratypes.h"

typedef struct O70State {
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
    void *related;
} O70State;

typedef struct O70Object {
    u8 pad00[4];
    s16 angle;
    u8 pad06[0x22];
    f32 value28;
    u8 pad2C[0x38];
    O70State *state;
} O70Object;

typedef struct O70Input {
    u8 pad00[0xA];
    u8 type;
    u8 pad0B;
    void *related;
} O70Input;

typedef struct O70FloatTable {
    u8 pad00[0x20];
    f32 values[1];
} O70FloatTable;

typedef struct O70VerticalStepTable {
    u8 pad00[0xC];
    s8 values[1];
} O70VerticalStepTable;

typedef struct O70AngleTable {
    u8 pad00[0x10];
    s16 values[1];
} O70AngleTable;

extern O70FloatTable gOverlay70FloatTableReloc;
extern O70VerticalStepTable gOverlay70VerticalStepReloc;
extern O70AngleTable gOverlay70AngleReloc;
extern s32 mathRnd(s32 lower, s32 upper);

void func_overlay_070_F0000000_18C91C8(O70Object *object, O70Input *input) {
    O70State *state;
    u8 type;

    state = object->state;
    state->related = input->related;
    state->type = input->type;
    state->angle = mathRnd(-0x8000, 0x7FFF);
    if (mathRnd(0, 1) == 0) {
        state->angleStep = mathRnd(-0x200, -0x180);
    } else {
        state->angleStep = mathRnd(0x180, 0x200);
    }
    state->countdown = 0;
    state->active = 0;
    state->timer = 0xBE;
    state->flags = 0xD;
    object->value28 = gOverlay70FloatTableReloc.values[state->type];
    type = state->type;
    state->verticalStep = gOverlay70VerticalStepReloc.values[type];
    object->angle = gOverlay70AngleReloc.values[type];
}
