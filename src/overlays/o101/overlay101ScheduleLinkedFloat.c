#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this linked float scheduler. */

typedef struct Overlay101LinkedFloatSlot {
    u8 pad00[0xC];
    f32 value;
    u8 pad10[0xC];
} Overlay101LinkedFloatSlot;

typedef struct Overlay101LinkedFloatEntry {
    Overlay101LinkedFloatSlot *output;
    s32 owner;
    u8 pad08[8];
    f32 start;
    u8 pad14[0xC];
    f32 end;
    u8 pad24[4];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedFloatEntry;

extern Overlay101LinkedFloatSlot D_0[];
extern Overlay101LinkedFloatSlot *overlay101FindEntry(
    Overlay101LinkedFloatSlot *slot, s32 mode, s32 key);
extern Overlay101LinkedFloatEntry *overlay101AllocateEntry(s32 duration);
extern void overlay101SubmitLinkedPairReloc(Overlay101LinkedFloatEntry *entry,
                                             s32 value);

void overlay101ScheduleLinkedFloat(s32 index, s32 key, f32 value, f32 seconds,
                                   s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedFloatSlot *slot;
    Overlay101LinkedFloatEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry(duration);
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 10;
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
