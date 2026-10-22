#include "overlays/overlay_091.h"

/* DKR v77/v80 and JFG checks found no exact donor for this object initializer. */
void overlay91Init(Overlay91InitObject *object, void *entry) {
    Overlay91InitState *state = object->state;

    state->timer = 0;
    state->mode = 0;
    object->value0 = 0;
    object->flags6 |= 0x400;
    object->index2E = -1;
    object->minValue = -225.0f;
    object->currentValue = 0.0f;
    object->maxValue = 144.0f;
}
