#include "PR/ultratypes.h"

extern s32 gOverlay33Handle;

void mmFree(void *handle);

/* DKR v77/v80 has the generic optional-release idiom, but no exact donor. */
void overlay33ReleaseGlobal(void) {
    if (gOverlay33Handle != 0) {
        mmFree((void *)gOverlay33Handle);
        gOverlay33Handle = 0;
    }
}
