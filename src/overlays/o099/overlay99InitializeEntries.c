#include "PR/ultratypes.h"

typedef struct Overlay99Object Overlay99Object;

typedef struct Overlay99Entry {
    f32 value00;
    f32 value04;
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    f32 value24;
    s16 kind28;
    s8 mode2A;
    s8 spawnMode2B;
    Overlay99Object *object2C;
} Overlay99Entry;

typedef struct Overlay99Storage {
    Overlay99Entry entries[3];
    s32 count;
} Overlay99Storage;

typedef struct Overlay99SpawnDescriptor {
    s16 kind0;
    u8 reserved2[2];
    s16 x4;
    s16 y6;
    s16 height8;
    s8 modeA;
    u8 flagsB;
    u8 zeroC;
    u8 zeroD;
} Overlay99SpawnDescriptor;

struct Overlay99Object {
    u8 pad00[6];
    s16 flags6;
    u8 pad08[0x34];
    void *owner3C;
};

extern Overlay99Storage gOverlay99Storage;
extern void func_80037414(s32 mode, f32 x, f32 y, s32 zero0,
                                         s32 zero1, s32 zero2, s32 zero3);
extern Overlay99Object *func_8000590C(
    Overlay99SpawnDescriptor *descriptor, s32 one);
extern void viFrameRateReset(void);

/* Declaring the address-taken descriptor after the entry cursor gives it the
 * retail sp+0x54 home. The resulting 110 words and all seven linked runtime
 * relocation roles are exact. */
void overlay99InitializeEntries(s32 count, Overlay99Entry *source,
                                       f32 x, f32 y) {
    u16 reservedStack[3];
    Overlay99Entry *entry;
    Overlay99SpawnDescriptor descriptor;
    s32 created;
    s32 i;

    func_80037414(4, x, y, 0, 0, 0, 0);
    i = 0;
    created = 0;
    entry = gOverlay99Storage.entries;
    if (count > 0) {
        do {
            entry->value00 = source->value00;
            entry->value04 = source->value04;
            entry->value08 = source->value08;
            entry->value0C = source->value0C;
            entry->value10 = source->value10;
            entry->value14 = source->value14;
            entry->value18 = source->value18;
            entry->value1C = source->value1C;
            entry->value20 = source->value20;
            entry->value24 = source->value24;
            entry->kind28 = source->kind28;
            entry->mode2A = source->mode2A;
            entry->object2C = 0;
            if (entry->kind28 != -1) {
                descriptor.kind0 = entry->kind28;
                descriptor.x4 = (s32)source->value00;
                descriptor.y6 = (s32)source->value04;
                descriptor.height8 = 5;
                descriptor.modeA = source->spawnMode2B;
                descriptor.flagsB = 0x40;
                descriptor.zeroC = 0;
                descriptor.zeroD = 0;
                entry->object2C = func_8000590C(&descriptor, 1);
                if (entry->object2C != 0) {
                    entry->object2C->owner3C = 0;
                    entry->object2C->flags6 |= 0x400;
                }
                created++;
            }
            i++;
            source++;
            entry++;
        } while ((i < count) && (i != 3));
    }
    gOverlay99Storage.count = i;
    if (created > 0) {
        viFrameRateReset();
    }
}
