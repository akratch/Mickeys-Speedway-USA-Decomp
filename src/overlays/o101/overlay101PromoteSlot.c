#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG contain no exact donor for this slot-order update. */

typedef struct Overlay101Slot {
    u8 data[0x1C];
} Overlay101Slot;

extern Overlay101Slot D_0[];
extern Overlay101Slot *gOverlay101Order[];
extern s32 gOverlay101OrderCount;

void overlay101PromoteSlot(s32 index) {
    Overlay101Slot *target;
    Overlay101Slot *previous;
    Overlay101Slot *current;
    s32 count;
    s32 i;

    target = &D_0[index];
    i = 1;
    previous = gOverlay101Order[0];
    gOverlay101Order[0] = target;
    count = gOverlay101OrderCount;
    while (i < count && previous != target) {
            current = gOverlay101Order[i];
            gOverlay101Order[i] = previous;
            previous = current;
            i++;
    }
}
