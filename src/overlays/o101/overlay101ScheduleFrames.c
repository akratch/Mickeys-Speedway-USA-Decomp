#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this frame-sequence scheduler. */

typedef struct Overlay101FrameSlot {
    u8 pad00[0xC];
    u8 frameCount;
    u8 frame;
    u8 pad0E[0xE];
} Overlay101FrameSlot;

typedef struct Overlay101FrameSchedule {
    Overlay101FrameSlot *output;
    s32 owner;
    s32 delay;
    u8 pad0C[0x1C];
    s32 elapsed;
    s32 duration;
} Overlay101FrameSchedule;

extern Overlay101FrameSlot D_0[];
extern Overlay101FrameSlot *overlay101FindEntry(Overlay101FrameSlot *slot,
                                                     s32 mode, s32 key);
extern Overlay101FrameSchedule *overlay101AllocateEntry(void);
extern void overlay101UpdateFrames(Overlay101FrameSchedule *entry,
                                        s32 value);

void overlay101ScheduleFrames(s32 index, s32 key, f32 fraction, f32 rate,
                              f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101FrameSlot *slot;
    Overlay101FrameSchedule *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0[index], 3, key);
    if (slot != NULL) {
        if (fraction >= 0.0f && fraction <= 1.0f) {
            slot->frame = (s32)(slot->frameCount * fraction);
        }
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->owner = 15;
                entry->delay = rate * 60.0f;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101UpdateFrames(entry, submitValue);
                }
            }
        }
    }
}
