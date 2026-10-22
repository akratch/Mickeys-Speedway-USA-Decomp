#include "PR/ultratypes.h"

extern u8 gOverlay52Storage;
extern void *gOverlay52Resource;
extern s32 gOverlay52Slots[2];

extern void freeFrontEndList(void *storage);
extern void overlay56ReleaseResource(void);
extern void overlay45ReleaseDescriptor(void *resource);
extern void freeFrontEndItem(s32 slot);

/*
 * DKR us.v77/us.v80 and JFG object scans found no exact donor. Their resource
 * cleanup code shares only the generic release-and-clear pattern used here.
 */
void overlay52Cleanup(void) {
    s32 *slot;
    s32 *end;

    freeFrontEndList(&gOverlay52Storage);
    overlay56ReleaseResource();
    if (gOverlay52Resource != NULL) {
        overlay45ReleaseDescriptor(gOverlay52Resource);
    }
    slot = gOverlay52Slots; end = &gOverlay52Slots[2];
    while (slot != end) {
        if (*slot != -1) {
            freeFrontEndItem(*slot);
            *slot = -1;
        }
        slot++;
    }
}
