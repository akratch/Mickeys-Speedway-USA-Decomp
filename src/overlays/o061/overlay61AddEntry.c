#include "PR/ultratypes.h"

typedef struct Overlay61Entry {
    s16 field0;
    u8 field2;
    u8 field3;
    u8 field4;
    u8 field5;
    u8 field6;
    u8 field7;
    u8 text0[0x30];
    u8 text1[4];
    s32 field3C;
} Overlay61Entry;

extern s32 gOverlay61EntryCountReloc;
extern Overlay61Entry gOverlay61EntriesReloc[];
extern u8 gOverlay61Text0Reloc[];
extern u8 gOverlay61Text1Reloc[];
extern void overlay61CopyTextReloc(u8 *, u8 *, s32);

void overlay61AddEntry(s32 value0, s32 value1, s32 text0Id, s32 text1Id, s32 value4,
                       s32 slot, s32 value6, s32 totalTime) {
    s32 hours;
    s32 minutes;
    s32 hundredths;
    Overlay61Entry *entry;

    if (gOverlay61EntryCountReloc < 20) {
        entry = &gOverlay61EntriesReloc[gOverlay61EntryCountReloc++];

        if (totalTime != -1) {
            hours = totalTime / 3600;
            minutes = (totalTime - (hours * 3600)) / 60;
            hundredths = (((totalTime - (hours * 3600)) - (minutes * 60)) * 100) / 60;
        }

        if ((slot < 0) || (slot >= 11)) {
            slot = 0;
        }

        if (text0Id != 0) {
            overlay61CopyTextReloc(entry->text0, gOverlay61Text0Reloc, text0Id);
        } else {
            entry->text0[0] = 0;
        }

        if (text1Id != 0) {
            overlay61CopyTextReloc(entry->text1, gOverlay61Text1Reloc, text1Id);
        } else {
            entry->text1[0] = 0;
        }

        entry->field0 = value0;
        entry->field2 = value1;
        entry->field3 = slot;
        entry->field4 = value6;
        entry->field5 = hours;
        entry->field6 = minutes;
        entry->field7 = hundredths;
        entry->field3C = value4;
    }
}
