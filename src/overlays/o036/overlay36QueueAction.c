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

/* Fixed queue insertion is title-specific; no DKR/JFG donor exists. */
void overlay36QueueAction(s32 action, s32 param1, s32 param2, s32 param3, s32 param4) {
    if (gOverlay36QueueCount < 16) {
        gOverlay36QueueNext->arg0 = action;
        gOverlay36QueueNext->arg1 = param1;
        gOverlay36QueueNext->arg2 = param2;
        gOverlay36QueueNext->arg3 = param3;
        gOverlay36QueueNext->arg4 = param4;
        gOverlay36QueueNext++;
        gOverlay36QueueCount++;
    }
}
