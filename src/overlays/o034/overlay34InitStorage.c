#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 bytes[0x68];
} Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern void *mmAlloc(s32 size, u32 colourTag);

/* The byte count is a call-crossing common subexpression rather than a named
 * local: that puts its caller-save home one word above a declared local's,
 * which is what fixes the four sp+0x1C sites. */
void overlay34InitStorage(s32 count) {
    s32 *word;
    s32 countdown;
    s32 remaining;

    gOverlay34Records = mmAlloc(count * (s32)sizeof(Overlay34Record), 0x87);
    word = (s32 *)gOverlay34Records;
    remaining = (count * (s32)sizeof(Overlay34Record)) >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }

    gOverlay34Pointers = mmAlloc(count * (s32)sizeof(*gOverlay34Pointers), 0x87);
    word = (s32 *)gOverlay34Pointers;
    remaining = (count * (s32)sizeof(*gOverlay34Pointers)) >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }
    gOverlay34Count = count;
}
