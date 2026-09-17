#include "PR/ultratypes.h"

typedef struct Overlay31PoolRecord {
    s32 state[16];
    u8 reserved40[0x64];
    u8 active;
    u8 reservedA5[0x1B];
} Overlay31PoolRecord;

extern void *overlay31AllocateReloc(s32 size, s32 tag);
extern void *overlay31CreateConfig(s32 kind, void *source, s32 width, s32 height,
                                   s32 slotCount);
extern void *D_10;

/* DKR v77/v80 and JFG contain no exact donor for this pool allocator. */
/* Matched 2026-09-17 (lane w6-o031), 11 -> 0 masked words at delta 0,
 * frame 0x38, four relocations, unforced. An inner-loop OR-zero on a
 * last-declared s32 is an L109 probe: it creates a phantom a0 web
 * (save 100.5) so record is forbidden c3 and takes a1, and i takes a2.
 * Declaration order is load-bearing (`s32 i; s32 j;` first, `zero` last).
 * The inert OR-zero is tracked in docs/cleanup-queue.md. */
Overlay31PoolRecord *overlay31CreatePool(s32 count) {
    s32 i;
    s32 j;
    Overlay31PoolRecord *records;
    Overlay31PoolRecord *record;
    s32 *state;
    s32 zero;

    records = (Overlay31PoolRecord *)overlay31AllocateReloc(
        count * sizeof(Overlay31PoolRecord), 0x8C);
    record = records;
    zero = 0;

    i = 0;
    if (count > 0) {
        do {
            record->state[15] = 0;
            record->active = 0;
            record->state[0] = 0;
            record->state[1] = 0;
            record->state[2] = 0;

            j = 3;
            state = &record->state[3];
            do {
                zero |= 0;
                j += 4;
                state[1] = 0;
                state[2] = 0;
                state[3] = 0;
                state += 4;
                state[-4] = 0;
            } while (j != 15);
            i++;
            record++;
        } while (i != count);
    }

    D_10 = overlay31CreateConfig(0, 0, 0, 0, count * 15);
    return records;
}
