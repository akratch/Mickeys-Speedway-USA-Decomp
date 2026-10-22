#include "PR/ultratypes.h"

/* DKR v77/v80 source checks found no matching state-allocation wrapper. */

typedef struct Overlay97Object {
    u8 pad0[0x64];
    void *state;
} Overlay97Object;

extern void *addRomdefLight(Overlay97Object *object, void *entry);

void overlay97AssignState(Overlay97Object *object, void *entry, s32 preserveState) {
    object->state = addRomdefLight(object, entry);
}
