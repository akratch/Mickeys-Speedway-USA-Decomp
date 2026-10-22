#include "ultra64.h"

extern s32 D_80079278;
extern void *gOverlay100Entries[];
extern void overlay100DrawMotion(void *displayList, void *entry);

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyToValue(void *displayList) {
    s32 count;
    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100DrawMotion(displayList, gOverlay100Entries[count]);
        }
    }
}
