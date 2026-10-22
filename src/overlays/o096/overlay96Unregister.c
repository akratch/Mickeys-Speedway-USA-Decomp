#include "PR/ultratypes.h"

extern s32 gO96EntryCountReloc;
extern void *gO96EntriesReloc[16];

/* Matched 2026-09-17 (lane w7-o096), 25 -> 0 masked words at delta 0,
 * frameless, six relocations, unforced. L160: delete the walking entry
 * pointer and subscript gO96EntriesReloc[index] in a while so IDO generates
 * the cursor. Lever 45: repeating the count global buys the retained base
 * and the count-1 copy. Declaration census (lever 46) is the sibling's
 * single index local. */
void overlay96Unregister(void *volume) {
    s32 index;

    index = gO96EntryCountReloc;
    while (index--) {
        if (gO96EntriesReloc[index] == volume) {
            gO96EntryCountReloc--;
            while (index < gO96EntryCountReloc) {
                gO96EntriesReloc[index] = gO96EntriesReloc[index + 1];
                index++;
            }
            return;
        }
    }
}
