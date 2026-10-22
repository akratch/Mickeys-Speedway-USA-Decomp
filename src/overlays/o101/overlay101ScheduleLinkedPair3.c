#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this linked-slot scheduler. */

typedef struct Overlay101LinkedSlot3 {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[0x10];
} Overlay101LinkedSlot3;

typedef struct Overlay101LinkedEntry3 {
    Overlay101LinkedSlot3 *output;
    s32 owner;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedEntry3;

extern Overlay101LinkedSlot3 D_0[];
extern Overlay101LinkedSlot3 *overlay101FindEntry(Overlay101LinkedSlot3 *slot,
                                                       s32 mode, s32 key);
extern Overlay101LinkedEntry3 *overlay101AllocateEntry(s32 duration);
extern void overlay101SubmitLinkedPairReloc(Overlay101LinkedEntry3 *entry,
                                             s32 value);

void overlay101ScheduleLinkedPair3(s32 index, s32 key, s32 x, s32 y,
                                   f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedSlot3 *slot;
    Overlay101LinkedEntry3 *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 3, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry(duration);
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 13;
                entry->startX = slot->x;
                entry->startY = slot->y;
                entry->endX = x;
                entry->endY = y;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->x = x;
            slot->y = y;
        }
    }
}
