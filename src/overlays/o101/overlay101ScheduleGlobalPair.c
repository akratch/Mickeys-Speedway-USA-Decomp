#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this global-pair scheduler. */

typedef struct Overlay101GlobalPairEntry {
    void *output;
    s32 owner;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101GlobalPairEntry;

extern s32 gOverlay101GlobalX;
extern s32 gOverlay101GlobalY;
extern Overlay101GlobalPairEntry *overlay101AllocateEntry(void);
extern void overlay101UpdateGlobalPair(Overlay101GlobalPairEntry *entry,
                                             s32 value);

void overlay101ScheduleGlobalPair(s32 x, s32 y, f32 seconds,
                                  s32 submitValue) {
    Overlay101GlobalPairEntry *entry;
    s32 duration;

    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            entry->output = NULL;
            entry->owner = 16;
            entry->startX = gOverlay101GlobalX;
            entry->startY = gOverlay101GlobalY;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateGlobalPair(entry, submitValue);
            }
        }
    } else {
        gOverlay101GlobalX = x;
        gOverlay101GlobalY = y;
    }
}
