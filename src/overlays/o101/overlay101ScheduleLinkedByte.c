#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this linked byte scheduler. */

typedef struct Overlay101LinkedByteSlot {
    u8 pad00[0x12];
    u8 value;
    u8 pad13[9];
} Overlay101LinkedByteSlot;

typedef struct Overlay101LinkedByteEntry {
    Overlay101LinkedByteSlot *output;
    s32 owner;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedByteEntry;

extern Overlay101LinkedByteSlot D_0[];
extern Overlay101LinkedByteSlot *overlay101FindEntry(
    Overlay101LinkedByteSlot *slot, s32 mode, s32 key);
extern Overlay101LinkedByteEntry *overlay101AllocateEntry(s32 duration);
extern void overlay101SubmitLinkedPairReloc(Overlay101LinkedByteEntry *entry,
                                             s32 value);

void overlay101ScheduleLinkedByte(s32 index, s32 key, s32 value, f32 seconds,
                                  s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedByteSlot *slot;
    Overlay101LinkedByteEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry(duration);
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 12;
                entry->start = slot->value;
                entry->end = value;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->value = value;
        }
    }
}
