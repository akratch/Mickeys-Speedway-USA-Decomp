#include "overlays/overlay_077.h"

/* Volatile here: the tail's original declaration, and the one that
 * reproduces its bytes. See overlay_077.c's file comment. */
extern volatile s32 gOverlay77Handle;

/*
 * Overlay 77 state tail (+0x3B8). Exact DKR and JFG scans are negative.
 * Kept as its own translation unit, separate from overlay_077.c: see the
 * comment there for why -Wab,-r4300_mul forces this module into two TUs
 * rather than one.
 */

void overlay77EnsureSelection(void) {
    if (gOverlay77Handle == 0) {
        s32 count = gOverlay77Sequence;

        gOverlay77Handle = count;
        gOverlay77Selection = mathRnd(0, count - 1);
    }
}

void overlay77RunCallback(void) {
    void *argument = gOverlay77CallbackArgument;

    if (argument != 0) {
        amSndStopXYZ(argument);
    }
}
