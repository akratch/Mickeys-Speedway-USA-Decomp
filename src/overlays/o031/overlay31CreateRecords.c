#include "PR/ultratypes.h"

typedef struct Overlay31Record {
    void *owner;
    u8 *blocks[9];
    u8 data[9][0x1C];
    u8 active;
    u8 pad125[0x23];
} Overlay31Record;

extern void *mmAlloc(s32 size, s32 tag);

/* DKR v77/v80 and JFG contain no exact donor for this record allocator. */
Overlay31Record *overlay31CreateRecords(s32 count) {
    Overlay31Record *records;
    Overlay31Record *record;
    s32 i;
    s32 j;

    records = mmAlloc(count * sizeof(Overlay31Record), 0x8C);
    record = records;
    for (i = 0; i < count; i++) {
        record->owner = NULL;
        record->active = 0;
        for (j = 0; j < 9; j++) {
            record->blocks[j] = record->data[j];
        }
        record++;
    }
    return records;
}
