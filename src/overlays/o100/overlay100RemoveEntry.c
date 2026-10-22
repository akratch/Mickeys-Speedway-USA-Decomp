#include "PR/ultratypes.h"

extern s32 D_80079278;
extern void *gOverlay100Entries[];
extern void mmFree(void *entry);

/*
 * DKR v77/v80 and JFG contain generic array-compaction relatives, but no
 * exact object donor for this exact registry removal routine.
 */
void overlay100RemoveEntry(void *entry) {
    s32 i;

    if (D_80079278 > 0) {
        for (i = 0; i < D_80079278; i++) {
            if (gOverlay100Entries[i] == entry) {
                D_80079278--;
                break;
            }
        }

        while (i < D_80079278) {
            gOverlay100Entries[i] = gOverlay100Entries[i + 1];
            i++;
        }
    }
    mmFree(entry);
}
