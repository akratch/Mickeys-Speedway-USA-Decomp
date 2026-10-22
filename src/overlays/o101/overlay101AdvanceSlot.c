#include "PR/ultratypes.h"

typedef struct Overlay101Slot {
    u8 pad00[8];
    u8 state;
    u8 value;
    u8 pad0A[0x12];
} Overlay101Slot;

extern Overlay101Slot D_0[];

void overlay101AdvanceSlot(s32 index) {
    Overlay101Slot *slot;
    s32 state;

    slot = &D_0[index];
    state = slot->state;
    if (state == 2) {
        slot->state = 3;
        slot->value = 0x40;
    } else if (state == 1) {
        slot->state = 3;
    }
}
