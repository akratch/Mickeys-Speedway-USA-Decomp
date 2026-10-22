#include "ultra64.h"

typedef struct {
    void *handle;
    u8 pad4[0xC];
} Overlay48Entry;

extern Overlay48Entry gOverlay48Entries[];
extern void overlay45ReleaseDescriptor(void *handle);

/* DKR v77/v80 only contain generic fixed-handle cleanup relatives. */
void overlay48ReleaseAll(void) {
    s32 index;
    Overlay48Entry *entry;

    entry = gOverlay48Entries;
    index = 0;
    do {
        if (entry->handle != NULL) {
            overlay45ReleaseDescriptor(entry->handle);
            entry->handle = NULL;
        }
        index++;
        entry++;
    } while (index != 5);
}
