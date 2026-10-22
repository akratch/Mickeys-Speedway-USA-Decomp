#include "PR/ultratypes.h"

typedef struct Overlay83Object {
    u8 pad0[0x64];
    void *descriptor;
} Overlay83Object;

typedef struct Overlay83Descriptor {
    u8 count;
    u8 pad1[3];
    u8 *entries;
} Overlay83Descriptor;

extern void overlay83Submit(Overlay83Object *, void *, s32);

/* DKR v77/v80 and JFG contain no exact donor or semantic source lead for this fixed-stride draw loop. */
void overlay83DrawEntries(Overlay83Object *object, s32 updateRate) {
    Overlay83Descriptor *descriptor;
    u8 *entry;
    u8 *current;
    s32 remaining;

    descriptor = object->descriptor;
    remaining = descriptor->count;
    entry = descriptor->entries;
    while (remaining--) {
        current = entry;
        entry += 0x258;
        overlay83Submit(object, current, updateRate);
    }
}
