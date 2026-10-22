#include "ultra64.h"

extern s32 D_80079278;
extern void *gOverlay100Entries[];
extern void overlay100UpdateMotion(void *entry, s32 value);

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyValue(s32 value) {
    s32 count;
    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100UpdateMotion(gOverlay100Entries[count], value);
        }
    }
}
