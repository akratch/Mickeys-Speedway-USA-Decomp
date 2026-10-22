#include "ultra64.h"

typedef struct Overlay36QueueEntry {
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    s32 arg4;
} Overlay36QueueEntry;

extern s32 gOverlay36QueueCount;
extern Overlay36QueueEntry *gOverlay36QueueNext;
extern Overlay36QueueEntry gOverlay36QueueBase[];
extern void overlay1ReadSelection();

/* Queue flushing is title-specific; DKR v77/v80 and JFG have no donor. */
void overlay36FlushQueue(void) {
    s32 count;

    count = gOverlay36QueueCount;
    while (count--) {
        gOverlay36QueueNext--;
        overlay1ReadSelection(gOverlay36QueueNext->arg0,
                                  gOverlay36QueueNext->arg1,
                                  gOverlay36QueueNext->arg2,
                                  gOverlay36QueueNext->arg3,
                                  gOverlay36QueueNext->arg4);
    }
    gOverlay36QueueCount = 0;
    gOverlay36QueueNext = gOverlay36QueueBase;
}
