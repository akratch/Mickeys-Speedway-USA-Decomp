#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this linked color scheduler. */

typedef struct Overlay101LinkedColorSlot {
    u8 pad00[0xF];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 pad13[9];
} Overlay101LinkedColorSlot;

typedef struct Overlay101LinkedColorEntry {
    Overlay101LinkedColorSlot *output;
    s32 owner;
    s16 startRed;
    s16 startGreen;
    s16 startBlue;
    s16 startAlpha;
    u8 pad10[8];
    s16 endRed;
    s16 endGreen;
    s16 endBlue;
    s16 endAlpha;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101LinkedColorEntry;

extern Overlay101LinkedColorSlot D_0[];
extern Overlay101LinkedColorSlot *overlay101FindEntry(
    Overlay101LinkedColorSlot *slot, s32 mode, s32 key);
extern Overlay101LinkedColorEntry *overlay101AllocateEntry(s32 duration);
extern void overlay101SubmitLinkedPairReloc(Overlay101LinkedColorEntry *entry,
                                             s32 value);

void overlay101ScheduleLinkedColor(s32 index, s32 key, s32 red, s32 green,
                                   s32 blue, s32 alpha, f32 seconds,
                                   s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedColorSlot *slot;
    Overlay101LinkedColorEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 3, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry(duration);
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 14;
                entry->startRed = slot->red;
                entry->startGreen = slot->green;
                entry->startBlue = slot->blue;
                entry->startAlpha = slot->alpha;
                entry->endRed = red;
                entry->endGreen = green;
                entry->endBlue = blue;
                entry->endAlpha = alpha;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->red = red;
            slot->green = green;
            slot->blue = blue;
            slot->alpha = alpha;
        }
    }
}
