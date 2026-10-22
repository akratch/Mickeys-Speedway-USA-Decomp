#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

extern s32 D_8007D6B0;
extern Overlay40Entry D_800D6BF8[8];

/* The counted walk is overlay40AddEntry's: a `while (remaining--)` from 8
 * gives the counter a0 and moves id to a1 as shipped. The function reads only
 * a0; an earlier spelling reached the same allocation through a second formal
 * overwritten before use, which the single resident caller never passes. */
void overlay40RemoveEntry(s32 id) {
    Overlay40Entry *entry;
    s32 remaining;

    entry = D_800D6BF8;
    if (D_8007D6B0 > 0) {
        remaining = 8;
        while (remaining--) {
            if (entry->state != -1 && entry->id == id) {
                entry->state = -1;
                D_8007D6B0--;
            }
            entry++;
        }
    }
}
