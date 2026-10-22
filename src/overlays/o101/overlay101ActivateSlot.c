#include "PR/ultratypes.h"

typedef struct Overlay101Slot {
    u8 pad00[8];
    u8 state;
    u8 value;
    u8 pad0A[0x12];
} Overlay101Slot;

extern Overlay101Slot D_0[];
extern void amSndPlay(s32 soundId, s32 arg1);

void overlay101ActivateSlot(s32 index) {
    Overlay101Slot *slot;
    slot = &D_0[index];
    if (slot->state == 0) {
        amSndPlay(0x1F7, 0);
        slot->state = 1;
        slot->value = 0;
    } else if (slot->state == 3) {
        slot->state = 1;
    }
}
