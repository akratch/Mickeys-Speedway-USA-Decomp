#include "PR/ultratypes.h"

typedef struct Overlay20RemoveOwner {
    u8 pad0[0x84];
    void *entry;
} Overlay20RemoveOwner;

extern void *gOverlay20Entries[];
extern s32 gOverlay20EntryCount;
extern u8 gOverlay20MarkerEnd;
extern u32 gOverlay20ActiveBits;

/* PROVENANCE: indexed search and list-compaction loops adapted from Diddy Kong
 * Racing's published src/weather.c::lensflare_override_remove. Mickey's owner
 * offset, arrays, marker cleanup, relocations, and target bytes remain
 * authoritative. */
/* Matched 2026-10-01. The count is read from its global at every use and
 * decremented in place, and the search and the compaction index the one entry
 * array. The earlier candidate copied the count into the parameter, carried
 * the decremented bound in a local and named the compaction array separately;
 * that left a count web holding the first colour across the compaction loop,
 * so its limit could not take it. The search stays bottom-tested: the `for`
 * spelling schedules two words differently. The marker walk still reuses the
 * parameter as its cursor; a separate cursor local costs nine words. */
void overlay20RemoveEntry(s32 owner) {
    void *entry;
    s32 i;

    entry = ((Overlay20RemoveOwner *)owner)->entry;
    if (entry == NULL) {
        return;
    }
    i = 0;
    if (gOverlay20EntryCount > 0) {
        do {
            if (entry == gOverlay20Entries[i]) {
                break;
            }
            i++;
        } while (i < gOverlay20EntryCount);
    }
    if (i >= gOverlay20EntryCount) {
        return;
    }
    gOverlay20EntryCount--;
    for (; i < gOverlay20EntryCount; i++) {
        gOverlay20Entries[i] = gOverlay20Entries[i + 1];
    }

    owner = (s32)&gOverlay20MarkerEnd;
    i = 31;
    do {
        if (owner != 0) {
            gOverlay20ActiveBits &= ~(1U << i);
            return;
        }
        owner -= 0x24;
    } while (i--);
}
