#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay4AttachObject();
extern void *gOverlay36CallTarget;

void overlay36CallGlobal(s32 param, s32 unused1, s32 unused2, s32 unused3) {
    overlay4AttachObject(param, gOverlay36CallTarget);
}
