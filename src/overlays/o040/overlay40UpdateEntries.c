#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

typedef struct Overlay40Node {
    u8 pad00[0x64];
    void *effect64;
} Overlay40Node;

typedef struct Overlay40Object {
    u8 pad00[8];
    Overlay40Node *node;
} Overlay40Object;

extern Overlay40Entry D_800D6BF8[8];
extern Overlay40Object **D_800D6B00;

/* The counted walk is overlay40AddEntry's `while (remaining--)` from 8. The
 * function reads only a0. An earlier spelling made the counter a second
 * formal, `remaining = 7; do { ... } while (remaining--)`, and needed the
 * count and the hoisted `D_800D6B00` address on one source line to get
 * as1's order; as a local with this loop shape it needs neither, and the
 * resident caller passes the one argument (2026-10-22). */
void overlay40UpdateEntries(s32 amount) {
    Overlay40Entry *entry;
    s8 previous;
    Overlay40Object *object;
    s32 remaining;

    entry = D_800D6BF8;
    remaining = 8;
    while (remaining--) {
        if (entry->state != -1) {
            if (entry->state < 8) {
                entry->state++;
            }
            previous = entry->scales[0];
            entry->scales[2] = entry->scales[1];
            entry->scales[0] = previous - amount;
            entry->scales[1] = previous;
            if (entry->scales[0] < 0) {
                entry->scales[0] = 0;
            }
            object = D_800D6B00[entry->id];
            if (object != 0 && object->node != 0) {
                if (entry->state & 1) {
                    object->node->effect64 = &entry->red;
                } else {
                    object->node->effect64 = 0;
                }
            }
        }
        entry++;
    }
}

