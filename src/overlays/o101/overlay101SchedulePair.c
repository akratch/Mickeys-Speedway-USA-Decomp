#include "PR/ultratypes.h"

typedef struct Overlay101PairSlot {
    u8 pad00[0xE];
    s16 x;
    s16 y;
    u8 pad12[0xA];
} Overlay101PairSlot;
typedef struct Overlay101PairEntry {
    Overlay101PairSlot *output;
    s32 owner;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101PairEntry;

extern Overlay101PairSlot D_0[];
extern Overlay101PairEntry *overlay101AllocateEntry(void);
extern void overlay101UpdateEntry(Overlay101PairEntry *entry, s32 value);

void overlay101SchedulePair(s32 index, s32 x, s32 y, f32 seconds,
                            s32 submitValue) {
    s32 duration;
    Overlay101PairEntry *entry;
    Overlay101PairSlot *slot;

    slot = &D_0[index];
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0[index];
            entry->output = slot;
            entry->owner = 1;
            entry->startX = slot->x;
            entry->startY = slot->y;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateEntry(entry, submitValue);
            }
        }
    } else {
        slot->x = x;
        slot->y = y;
    }
}
