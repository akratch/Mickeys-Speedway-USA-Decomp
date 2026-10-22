#include "PR/ultratypes.h"

typedef struct Overlay99Entry {
    u8 pad0[0x2C];
    void *resource;
} Overlay99Entry;

extern Overlay99Entry gOverlay99Entries[];
extern void func_80006EA0(void *resource);

/* DKR v77/v80 and JFG checks found only generic resource-array cleanup. */
void overlay99ReleaseEntries(void) {
    Overlay99Entry *entry = gOverlay99Entries;
    s32 remaining = 2;

    do {
        if (entry->resource != NULL) {
            func_80006EA0(entry->resource);
            entry->resource = NULL;
        }
        entry++;
    } while (remaining--);
}
