#include "overlays/overlay_069.h"


/* Exact DKR and JFG scans are negative. */
void overlay69Init(Overlay69Object *object, void *entry) {
    Overlay69State *state = object->state;
    state->field14 = 0; state->byte35 = 0; state->field2 = 0; state->fieldA = 0;
    state->field18 = 0; state->byte36 = 0; state->field4 = 0; state->fieldC = 0;
    state->field1C = 0; state->byte37 = 0; state->field6 = 0; state->fieldE = 0;
    state->field10 = 0; state->byte34 = 0; state->field0 = 0; state->field8 = 0;
}

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

/* +0x170. Overlay 88 ships the same function and includes this file. */
#include "overlay69DrawSortedGeometry.inc"
