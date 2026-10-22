#include "PR/ultratypes.h"
typedef struct Overlay101ByteSlot17 { u8 pad00[0x17]; u8 value; u8 pad18[4]; } Overlay101ByteSlot17;
typedef struct Overlay101ByteEntry17 {
    Overlay101ByteSlot17 *output; s32 owner; s32 start; u8 pad0C[0xC];
    s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry17;
extern Overlay101ByteSlot17 D_0[];
extern Overlay101ByteEntry17 *overlay101AllocateEntry(void);
extern void overlay101UpdateByte17(Overlay101ByteEntry17 *entry, s32 value);

void overlay101ScheduleByte17(s32 index, s32 value, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101ByteEntry17 *entry;
    Overlay101ByteSlot17 *slot;

    slot = &D_0[index];
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0[index];
            entry->output = slot;
            entry->owner = 6;
            entry->start = slot->value;
            entry->end = value;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) overlay101UpdateByte17(entry, submitValue);
        }
    } else {
        slot->value = value;
    }
}
