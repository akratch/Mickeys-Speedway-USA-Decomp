#include "ultra64.h"

typedef struct Overlay36TickObject {
    u8 pad000[0x16A];
    s16 timer;
    u8 pad16C[0x2E];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    s32 owner;
    u8 pad1A4[4];
    u16 flags;
} Overlay36TickObject;

typedef struct Overlay36TickSource {
    u8 pad00[0x64];
    Overlay36TickObject *object;
} Overlay36TickSource;

extern void overlay7CommitSelection(s32 value);

/* State countdown semantics are title-specific; no DKR/JFG donor exists. */
void overlay36TickState(Overlay36TickSource *source) {
    Overlay36TickObject *object;

    object = source->object;
    object->timer = 600;
    object->countdown--;
    if (object->countdown == 0) {
        object->state = 255;
        object->owner = 0;
    }
    if (!(object->flags & 1)) {
        overlay7CommitSelection(7);
    }
}
