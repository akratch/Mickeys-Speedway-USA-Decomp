#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no exact donor for this resource path. */

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    s32 assetOffsets[4];
    s32 assetSizes[4];
    s16 values[4];
} Overlay68Probe;

typedef struct Overlay68EntryHeader {
    s32 active;
    u8 field4;
    u8 timer;
    s16 index;
    s16 width;
    s16 generation;
    void *payload;
} Overlay68EntryHeader;

typedef struct Overlay68ResidentEntry {
    s32 thresholdNumerator;
    u8 pad04[0x1C];
} Overlay68ResidentEntry;

extern const Overlay68KindPair gOverlay68KindMap[];
extern Overlay68EntryHeader *gOverlay68SecondaryEntry;

extern s32 overlay68PayloadLimit(void);
extern void *mmAlloc(s32 size, s32 tag);
extern s32 piRomLoadSection(u32 assetIndex, u32 address,
                                       s32 assetOffset, s32 size);
extern Overlay68ResidentEntry *overlay68GetResidentEntriesReloc(void);
extern s32 levelGetBlurEffect(s32 kind);
extern void mmFree(void *resource);

void overlay68RebuildSecondaryEntry(s32 kind) {
    s32 amount;
    const Overlay68KindPair *mapping;
    s32 entrySize;
    Overlay68ResidentEntry *selectedEntry;
    void *payload;

    gOverlay68SecondaryEntry = 0;
    amount = -1;
    mapping = gOverlay68KindMap;

    while (mapping->kind != -1) {
        if (kind == mapping->kind) {
            amount = mapping->amount;
            break;
        }
        mapping++;
    }

    if (amount != -1) {
        Overlay68EntryHeader *entry;

        entrySize = overlay68PayloadLimit();
        entry = mmAlloc(entrySize, 0x85);
        if (entry != 0) {
            Overlay68Probe *probe;

            probe = mmAlloc(sizeof(*probe), 0x85);
            if (probe != 0) {
                Overlay68ResidentEntry *entries;
                s32 threshold;
                s32 index;

                piRomLoadSection(0x3F, (u32)probe,
                                             amount * (s32)sizeof(*probe),
                                             sizeof(*probe));
                entries = overlay68GetResidentEntriesReloc();
                index = levelGetBlurEffect(kind);
                selectedEntry = entries;
                selectedEntry += index;
                threshold = selectedEntry->thresholdNumerator / 5;
                if (threshold == 0) {
                    threshold = 0x7080;
                }

                index = 0;
                while ((index < 4) && (probe->assetSizes[index] != 0) &&
                       (threshold < probe->values[index])) {
                    index++;
                }

                if (index >= 4) {
                    index = 3;
                } else if (probe->assetSizes[index] == 0) {
                    index--;
                }

                if (index >= 0) {
                    piRomLoadSection(0x40, (u32)entry,
                                                 probe->assetOffsets[index],
                                                 probe->assetSizes[index]);
                    payload = entry + 1;
                    entry->payload = payload;
                    gOverlay68SecondaryEntry = entry;
                }
                mmFree(probe);
            }
            if (gOverlay68SecondaryEntry == 0) {
                mmFree(entry);
            }
        }
    }
}
