#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this scaled-value scheduler. */

typedef struct Overlay101LinkedScaledSlot {
    u8 pad00[0x10];
    s16 value;
    u8 pad12[0xA];
} Overlay101LinkedScaledSlot;

typedef struct Overlay101LinkedScaledEntry {
    Overlay101LinkedScaledSlot *output;
    s32 owner;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedScaledEntry;

extern Overlay101LinkedScaledSlot D_0[];
extern f32 gOverlay101TimedScale;
extern f32 gOverlay101ImmediateScale;
extern Overlay101LinkedScaledSlot *overlay101FindEntry(
    Overlay101LinkedScaledSlot *slot, s32 mode, s32 key);
extern Overlay101LinkedScaledEntry *overlay101AllocateEntry(s32 duration);
extern void overlay101SubmitLinkedPairReloc(Overlay101LinkedScaledEntry *entry,
                                             s32 value);

void overlay101ScheduleLinkedScaled(s32 index, s32 key, f32 value, f32 seconds,
                                    s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedScaledSlot *slot;
    Overlay101LinkedScaledEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry(duration);
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 11;
                entry->start = slot->value;
                entry->end = value * gOverlay101TimedScale;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->value = value * gOverlay101ImmediateScale;
        }
    }
}
