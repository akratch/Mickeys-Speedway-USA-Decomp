#include "PR/ultratypes.h"

typedef f32 Overlay69Matrix[4][4];

typedef struct Overlay69AnchorState {
    u8 reserved00[0x44];
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
} Overlay69AnchorState;

typedef struct Overlay69TransformSet {
    u8 reserved00[8];
    s16 direct;
    s16 matrixIndex;
    Overlay69Matrix *matrices[1];
} Overlay69TransformSet;

typedef struct Overlay69AnchorObject {
    s16 angle;
    u8 reserved02[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x4C];
    Overlay69AnchorState *anchorState;
    Overlay69TransformSet **transformSets;
    u8 reserved6C[0x27];
    u8 transformSetIndex;
} Overlay69AnchorObject;

extern void func_8002B040(Overlay69Matrix *matrix, f32 x, f32 y,
                                       f32 z, f32 *outX, f32 *outY,
                                       f32 *outZ);
extern s32 Arctanf(f32 x, f32 z);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
void overlay69UpdateAnchor(Overlay69AnchorObject *object, s32 updateContext) {
    Overlay69TransformSet *set;
    Overlay69AnchorState *state;
    Overlay69Matrix *matrix;
    f32 forwardX;
    f32 forwardY;
    f32 forwardZ;
    s16 angle;

    set = object->transformSets[object->transformSetIndex];
    state = object->anchorState;
    if (set->direct == 0) {
        matrix = set->matrices[set->matrixIndex];
        func_8002B040(matrix, 0.0f, 0.0f, 1.0f,
                                   &forwardX, &forwardY, &forwardZ);
        angle = Arctanf(forwardX, forwardZ);
        state->x = (*matrix)[3][0] - func_8002A8C0(angle) * 18.0f;
        state->y = object->y;
        state->z = (*matrix)[3][2] - func_8002A8BC(angle) * 18.0f;
        state->angle = angle;
    } else {
        state->x = object->x;
        state->y = object->y;
        state->z = object->z;
        state->angle = object->angle;
    }
}
