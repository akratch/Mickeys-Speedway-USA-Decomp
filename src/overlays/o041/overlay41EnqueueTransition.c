#include "PR/ultratypes.h"

typedef struct Overlay41QueueEntry {
    s16 timer;
    s16 value2;
    s16 value4;
    s16 value6;
    s8 value8;
    s8 value9;
    s8 valueA;
    u8 active;
} Overlay41QueueEntry;

extern Overlay41QueueEntry gOverlay41QueueEntries[5];

/* Matched as the loop it is: five queue entries scanned with an index and a
 * walking entry pointer. The compiler peels the first iteration onto the
 * array's own address and unrolls the other four, which is the hand-unrolled
 * shape, the eleven alias names and the volatile cursor the earlier candidate
 * spelled out. The pointer has to be a second induction variable beside the
 * index: indexing the array, deriving the pointer inside the body, or
 * bounding the loop by the pointer all unroll differently.
 */
void func_overlay_041_F000195C_1888C94(s32 value2, s32 timer, s32 value4,
                                       s32 value6, s32 value8, s32 value9,
                                       s32 valueA) {
    s32 i;
    Overlay41QueueEntry *entry;

    for (i = 0, entry = gOverlay41QueueEntries; i < 5; i++, entry++) {
        if (entry->active == 0 && entry->timer <= 0) {
            entry->value2 = value2;
            entry->timer = timer;
            entry->value4 = value4;
            entry->value6 = value6;
            entry->value8 = value8;
            entry->value9 = value9;
            entry->valueA = valueA;
            entry->active = 0;
            return;
        }
    }
}
