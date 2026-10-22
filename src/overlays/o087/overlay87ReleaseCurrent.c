#include "PR/ultratypes.h"

extern void *gOverlay87Current;
extern void amSndStopXYZ(void *resource);

/* DKR v77/v80 and JFG checks found only generic null-guarded release logic. */
void overlay87ReleaseCurrent(void) {
    void *resource = gOverlay87Current;

    if (resource != NULL) {
        amSndStopXYZ(resource);
    }
}
