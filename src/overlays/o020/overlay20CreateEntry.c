#include "PR/ultratypes.h"

typedef struct Overlay20Owner {
    u8 pad0[0xC];
    s32 x;
    u8 pad10[4];
    s32 z;
    u8 pad18[0x6C];
    void *entry;
} Overlay20Owner;

typedef struct Overlay20EntryConfig {
    u8 pad0[0xA];
    s16 heading;
    s16 mode;
    s16 scale;
    s16 value0;
    s16 value1;
} Overlay20EntryConfig;

extern f32 gOverlay20Scale;
extern void *overlay20ConfigureEntry(void *, s32, s32, f32, s32, f32, f32, f32);

/* Exact at +0xF78; DKR v77/v80 and JFG have no exact donor for this wrapper. */
void overlay20CreateEntry(Overlay20Owner *owner, Overlay20EntryConfig *config) {
    owner->entry = overlay20ConfigureEntry(
        owner->entry, owner->x, owner->z, (f32)config->heading, config->mode,
        (f32)config->scale * gOverlay20Scale, (f32)config->value0,
        (f32)config->value1);
}
