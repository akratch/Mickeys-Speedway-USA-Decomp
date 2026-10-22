#include "overlays/overlay_082.h"

/*
 * Overlay 82 initializer. This remains a separate TU because its target-
 * proven -Wo,-loopunroll,2 flag does not apply to the update/accessor tail.
 */

/* DKR v77/v80 and JFG contain no exact donor for this state initializer. */
void overlay82Init(Overlay82Object *object, void *entry) {
    u8 *state;
    s32 *values;
    s32 index;

    state = object->state;
    values = (s32 *) state;
    state[0] = 0;
    state[1] = 0;
    state[2] = 0;
    state[3] = 0;
    for (index = 0; index < 2; index++) values[index + 1] = 0;
    object = (Overlay82Object *)((u8 *)values + index * sizeof(s32));
    ((s32 *)object)[2] = 0;
    ((s32 *)object)[3] = 0;
    ((s32 *)object)[4] = 0;
    ((s32 *)object)[1] = 0;
}
