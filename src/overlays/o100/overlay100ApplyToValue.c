#include "ultra64.h"

extern s32 gOverlay100Count;
extern void *gOverlay100Entries[];
extern void overlay100ApplyToValueReloc(void *displayList, void *entry);

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyToValue(void *displayList) {
    s32 count;
    count = gOverlay100Count;
    if (count != 0) {
        while (count--) {
            overlay100ApplyToValueReloc(displayList, gOverlay100Entries[count]);
        }
    }
}
