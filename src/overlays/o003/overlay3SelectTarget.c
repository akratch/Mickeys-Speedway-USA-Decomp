#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found no exact selector donor. */

typedef struct Overlay3Object {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x2C];
    s16 type;
} Overlay3Object;

typedef struct Overlay3State {
    u8 pad000[1];
    s8 group;
    u8 pad002[0x199];
    u8 useWeightedSearch;
    u8 pad19C[0x1F1];
    u8 state;
    s16 timer;
    u8 pad390[0x3E];
    s16 sameObjectTimer;
} Overlay3State;

extern Overlay3Object *gOverlay3Objects[];
extern void overlay3TouchObject(s32 group);
extern Overlay3Object *func_overlay_003_F000027C_1859FAC(
    Overlay3Object *anchor, Overlay3State *state);
extern Overlay3Object *func_overlay_003_F00003B0_185A0E0(
    Overlay3Object *anchor, Overlay3State *state, s32 updateRate);

void overlay3SelectTarget(
    f32 *outX, f32 *outZ, Overlay3State *state,
    Overlay3Object *anchor, s32 updateRate) {
    Overlay3Object *selected;

    if (state->useWeightedSearch == 0) {
        selected = func_overlay_003_F000027C_1859FAC(anchor, state);
    } else {
        selected = func_overlay_003_F00003B0_185A0E0(anchor, state, updateRate);
        if (selected == 0) {
            selected = func_overlay_003_F000027C_1859FAC(anchor, state);
        }
    }

    if (selected == gOverlay3Objects[state->group]) {
        state->sameObjectTimer += updateRate;
        if (state->sameObjectTimer > 300.0f) {
            overlay3TouchObject(state->group);
            selected = func_overlay_003_F000027C_1859FAC(anchor, state);
            state->sameObjectTimer = 0;
        }
    } else {
        state->sameObjectTimer = 0;
    }

    gOverlay3Objects[state->group] = selected;
    if (selected != 0 && selected->type != 1) {
        state->state = 0x7F;
        state->timer = 0;
    }

    if (selected != 0) {
        *outX = selected->x;
        *outZ = selected->z;
    } else {
        *outX = anchor->x;
        *outZ = anchor->z;
    }
}
