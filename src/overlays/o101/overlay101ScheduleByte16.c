#include "PR/ultratypes.h"
typedef struct Overlay101ByteSlot16 { u8 pad00[0x16]; u8 value; u8 pad17[5]; } Overlay101ByteSlot16;
typedef struct Overlay101ByteEntry16 {
    Overlay101ByteSlot16 *output; s32 owner; s32 start; u8 pad0C[0xC];
    s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry16;
extern Overlay101ByteSlot16 D_0[];
extern Overlay101ByteEntry16 *overlay101AllocateEntry(void);
extern void overlay101UpdateByte16(Overlay101ByteEntry16 *entry, s32 value);

void overlay101ScheduleByte16(s32 index, s32 value, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101ByteEntry16 *entry;
    Overlay101ByteSlot16 *slot;

    slot = &D_0[index];
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0[index];
            entry->output = slot;
            entry->owner = 7;
            entry->start = slot->value;
            entry->end = value;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) overlay101UpdateByte16(entry, submitValue);
        }
    } else {
        slot->value = value;
    }
}
