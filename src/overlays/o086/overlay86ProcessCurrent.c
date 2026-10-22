#include "PR/ultratypes.h"

typedef struct Overlay86Current {
    void *resource;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *values[1];
} Overlay86Current;

typedef struct Overlay86Owner {
    u8 pad00[0x50];
    void *context;
    u8 pad54[0x14];
    Overlay86Current **currentSlot;
} Overlay86Owner;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this flow. */
extern void func_8005AF14(Overlay86Current *current, void *resource,
                                  Overlay86Owner *owner);
extern void lightObject();

void overlay86ProcessCurrent(Overlay86Owner *owner) {
    Overlay86Current *current;

    current = *owner->currentSlot;
    if (current != NULL && current->resource != NULL && current->active != 0) {
        func_8005AF14(current, current->resource, owner);
        lightObject(owner, current, owner->context,
                             current->values[current->index]);
        current->active = 0;
    }
}
