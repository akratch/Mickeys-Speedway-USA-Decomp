#include "PR/ultratypes.h"

/* Exact DKR and JFG scans are negative. */
typedef struct Overlay88State {
    s16 field0, field2, field4, field6;
    s16 field8, fieldA, fieldC, fieldE;
    s32 field10, field14, field18, field1C;
    u8 pad20[0x14];
    u8 byte34, byte35, byte36, byte37;
} Overlay88State;
typedef struct Overlay88Object { u8 pad0[0x64]; Overlay88State *state; } Overlay88Object;

void overlay88Init(Overlay88Object *object, void *entry) {
    Overlay88State *state = object->state;
    state->field14 = 0; state->byte35 = 0; state->field2 = 0; state->fieldA = 0;
    state->field18 = 0; state->byte36 = 0; state->field4 = 0; state->fieldC = 0;
    state->field1C = 0; state->byte37 = 0; state->field6 = 0; state->fieldE = 0;
    state->field10 = 0; state->byte34 = 0; state->field0 = 0; state->field8 = 0;
}
