#include "overlays/overlay_088.h"

void overlay88Init(Overlay88Object *object, void *entry) {
    Overlay88State *state = object->state;
    state->field14 = 0; state->byte35 = 0; state->field2 = 0; state->fieldA = 0;
    state->field18 = 0; state->byte36 = 0; state->field4 = 0; state->fieldC = 0;
    state->field1C = 0; state->byte37 = 0; state->field6 = 0; state->fieldE = 0;
    state->field10 = 0; state->byte34 = 0; state->field0 = 0; state->field8 = 0;
}

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
void overlay88UpdateAnchor(Overlay88Object *object, s32 updateContext) {
    Overlay88Node *node;
    Overlay88State *anchor;
    Overlay88Descriptor *descriptor;
    Overlay88Transform *transform;
    f32 forwardX;
    f32 forwardY;
    f32 forwardZ;
    s16 angle;

    node = object->nodes[object->nodeIndex];
    anchor = object->state;
    descriptor = node->descriptor;
    func_8005AF14(node, descriptor, object);
    lightObject(object, node, object->context,
                             node->transforms[node->transformIndex]);
    if (descriptor->needsRefresh != 0) {
        func_80007C68(object, descriptor, node, node->selector);
    }

    node->selector = 0;
    transform = node->transforms[node->transformIndex];
    func_8002B040(transform, 0.0f, 0.0f, 1.0f,
                                &forwardX, &forwardY, &forwardZ);
    angle = Arctanf(forwardX, forwardZ);
    anchor->x = transform->x - func_8002A8C0(angle) * 18.0f;
    anchor->y = object->y;
    anchor->z = transform->z - func_8002A8BC(angle) * 18.0f;
    anchor->angle = angle;
}

/*
 * Overlay 88 and overlay 69 ship the same renderer body. Overlay 69's
 * source is the one copy: renamed here, it is this module's third
 * function. Its callees keep overlay 69's spellings; the object's rule
 * gives them this module's runtime-relocation names.
 */
#define overlay69DrawSortedGeometry overlay88DrawSortedGeometry
#include "src/overlays/o069/overlay69DrawSortedGeometry.c"
#undef overlay69DrawSortedGeometry
