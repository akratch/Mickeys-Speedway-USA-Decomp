#include "PR/ultratypes.h"

typedef struct Overlay68EntryHeader {
    s32 active;
    u8 field4;
    u8 timer;
    s16 index;
    s16 width;
    s16 generation;
    void *payload;
} Overlay68EntryHeader;

extern Overlay68EntryHeader *D_8007A1F4;
extern Overlay68EntryHeader *gOverlay68SecondaryEntry;
extern s32 overlay68PayloadLimit(void);
extern Overlay68EntryHeader *mmAlloc(s32 size, s32 tag);

void overlay68CreatePrimary(void) {
    Overlay68EntryHeader *entry;

    if (D_8007A1F4 == 0) {
        entry = mmAlloc(overlay68PayloadLimit(), 0x8F);
        if (entry != 0) {
            entry->active = 0;
            entry->field4 = 0;
            entry->timer = 0;
            entry->index = -1;
            entry->width = 0;
            entry->generation = 0;
            entry->payload = entry + 1;
            D_8007A1F4 = entry;
        }
    }
}

void overlay68CreateSecondary(void) {
    Overlay68EntryHeader *entry;

    if (gOverlay68SecondaryEntry == 0) {
        entry = mmAlloc(overlay68PayloadLimit(), 0x85);
        if (entry != 0) {
            entry->active = 0;
            entry->field4 = 0;
            entry->timer = 0;
            entry->index = -1;
            entry->width = 0;
            entry->generation = 0;
            entry->payload = entry + 1;
            gOverlay68SecondaryEntry = entry;
        }
    }
}
