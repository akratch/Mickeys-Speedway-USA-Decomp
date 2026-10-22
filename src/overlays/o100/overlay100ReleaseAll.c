#include "ultra64.h"

extern s32 D_80079278;
extern void *gOverlay100Entries[];
extern void overlay100RemoveEntry(void *entry);

/* DKR v77/v80 and JFG contain only generic object-list cleanup relatives. */
void overlay100ReleaseAll(void) {
    s32 count;

    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100RemoveEntry(gOverlay100Entries[count]);
        }
    }
}
