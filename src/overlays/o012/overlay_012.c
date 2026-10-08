#include "overlays/overlay_012.h"

/* Overlay 12, ADR 0006 consolidation: loop-unroll-disabled prefix. */

/* DKR v77/v80 and JFG contain no exact donor for this resource initializer. */
void overlay12Initialize(void) {
    s32 i;
    Overlay12Entry *entry;

    gOverlay12ResourceF3 = overlay12LoadReloc(0xF3);
    gOverlay12ResourceF2 = overlay12LoadReloc(0xF2);
    gOverlay12ResourceF4 = overlay12LoadReloc(0xF4);
    gOverlay12ResourceF5 = overlay12LoadReloc(0xF5);
    gOverlay12ResourceF6 = overlay12LoadReloc(0xF6);
    gOverlay12Resource39 = overlay12LoadReloc(0x39, 1);

    for (i = 0, entry = gOverlay12Entries; i < 64; i++, entry++) {
        entry->active = 0;
    }

    gOverlay12Flag1536 = 0;
    gOverlay12Ready = 1;
    gOverlay12Count = 0;
    gOverlay12Selection = 0;
    /* The zero goes through the loop counter: a literal store loads it in a
     * different order (4 words). */
    i = 0;
    gOverlay12Value1598 = i;
}
