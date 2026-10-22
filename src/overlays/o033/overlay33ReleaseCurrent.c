#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found only generic lifecycle relatives. */
extern void mmFree();
extern void *gOverlay33CurrentRead;
extern void *gOverlay33Current;

void overlay33ReleaseCurrent(void) {
    void *current;

    current = gOverlay33CurrentRead;
    if (current != 0) {
        mmFree(current);
        gOverlay33Current = 0;
    }
}
