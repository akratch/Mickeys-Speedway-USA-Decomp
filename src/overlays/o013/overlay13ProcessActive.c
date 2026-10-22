#include "PR/ultratypes.h"

typedef struct Overlay13Record {
    u8 pad0[6];
    u8 active;
    u8 pad7[0x79];
} Overlay13Record;

extern s32 gOverlay13Enabled;
extern Overlay13Record gOverlay13Records[];
extern void overlay13UpdateRecord(Overlay13Record *record, s32 arg);

/* DKR v77/v80 contain only generic fixed-pool loops, not this record walk. */
void overlay13ProcessActive(s32 arg) {
    s32 i;
    Overlay13Record *record;

    if (gOverlay13Enabled != 0) {
        record = gOverlay13Records;
        for (i = 0; i < 32; i++, record++) {
            if (record->active != 0) {
                overlay13UpdateRecord(record, arg);
            }
        }
    }
}
