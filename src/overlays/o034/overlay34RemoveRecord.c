#include "PR/ultratypes.h"

typedef struct Overlay34Resource Overlay34Resource;

typedef struct Overlay34Record {
    u8 pad00[0x20];
    Overlay34Resource *resource;
    u8 pad24[0x1A];
    u8 active;
    u8 pad3F[0x29];
} Overlay34Record;

extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern void texFreeTexture(Overlay34Resource *resource);

/* Matched 2026-09-12, lane p9-tight. Twelve words to zero. The plateau's
 * `shadow` local was an artefact: the four `move $a1,$v1` the old handoff was
 * trying to buy are the post-decrement temp of `while (remaining--)`, emitted
 * once in each loop's guarded entry and once at each rotated loop top, and
 * they are one web. A source-level `shadow` cannot join that web because the
 * two ranges overlap -- `shadow` is live to the call while the temp is live to
 * the bottom test -- so every spelling of the copy gave the temp its own
 * register. Deleting `shadow` and writing both loops as `while (remaining--)`
 * puts the temp in $a1 and reproduces all four copies for free; the condition
 * must be bare, since `remaining-- != 0` materialises a boolean and emits
 * `sltu` where the target has `move`. texFreeTexture therefore takes one
 * argument: $a1 at the call is this dead temp, not a second parameter. */
void overlay34RemoveRecord(Overlay34Record *record) {
    Overlay34Record **slot;
    s32 remaining;

    slot = gOverlay34Pointers;
    remaining = gOverlay34ActiveCount;
    while (remaining--) {
        if (*slot == record) {
            while (remaining--) {
                *slot = slot[1];
                slot++;
            }
            if (record->resource != NULL) {
                texFreeTexture(record->resource);
            }
            record->active = 0;
            gOverlay34ActiveCount--;
            return;
        }
        slot++;
    }
}
