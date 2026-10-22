#include "overlays/patch_indices.h"

extern void *D_800D31C8[];

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
void overlay52PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) D_800D31C8[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) D_800D31C8[entry->second];
        }
        entry++;
    }
}
