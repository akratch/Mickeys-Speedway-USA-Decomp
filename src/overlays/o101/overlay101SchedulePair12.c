#include "PR/ultratypes.h"

typedef struct Overlay101PairSlot12 { u8 pad00[0x12]; s16 x; s16 y; u8 pad16[6]; } Overlay101PairSlot12;
typedef struct Overlay101PairEntry12 {
    Overlay101PairSlot12 *output; s32 owner; s32 startX; s32 startY;
    u8 pad10[8]; s32 endX; s32 endY; u8 pad20[8]; s32 elapsed; s32 duration;
} Overlay101PairEntry12;
extern Overlay101PairSlot12 D_0[];
extern Overlay101PairEntry12 *overlay101AllocateEntry(void);
extern void overlay101UpdateEntry12(Overlay101PairEntry12 *entry, s32 value);

void overlay101SchedulePair12(s32 index, s32 x, s32 y, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101PairEntry12 *entry;
    Overlay101PairSlot12 *slot;

    slot = &D_0[index];
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0[index];
            entry->output = slot;
            entry->owner = 2;
            entry->startX = slot->x;
            entry->startY = slot->y;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateEntry12(entry, submitValue);
            }
        }
    } else {
        slot->x = x;
        slot->y = y;
    }
}
