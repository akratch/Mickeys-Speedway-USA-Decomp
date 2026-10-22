#include "PR/ultratypes.h"

typedef struct Overlay44Owner {
    s8 state;
    u8 pad01[0x13];
    void *resources[4];
} Overlay44Owner;

extern void mmFree(void *);

/* DKR v77/v80 and JFG contain no exact donor for this release sweep. */
void overlay44Release(Overlay44Owner *owner) {
    s32 offset;
    u8 *cursor;
    void *resource;

    if (owner != 0) {
        offset = 0;
        cursor = (u8 *)owner;
        if (owner->state != (s8)0xFF) {
            owner->state = (s8)0xFF;
            do {
                resource = *(void **)(cursor + 0x14);
                if (resource != 0) {
                    mmFree(resource);
                    *(void **)(cursor + 0x14) = 0;
                }
                offset += 4;
                cursor += 4;
            } while (offset != 0x10);
        }
    }
}
