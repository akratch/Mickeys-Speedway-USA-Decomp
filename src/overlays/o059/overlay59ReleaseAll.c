#include "ultra64.h"

typedef struct {
    u8 bytes[0x44];
} Overlay59Entry;

extern Overlay59Entry gOverlay59Entries[];
extern Overlay59Entry gOverlay59EntriesEnd[];
extern void overlay59ReleaseReloc(Overlay59Entry *entry);

/* DKR v77/v80 and JFG contain only generic fixed-array cleanup relatives. */
void overlay59ReleaseAll(void) {
    Overlay59Entry *entry;
    Overlay59Entry *end;

    entry = gOverlay59Entries; end = gOverlay59EntriesEnd;
    do {
        overlay59ReleaseReloc(entry);
        entry++;
    } while (entry != end);
}
