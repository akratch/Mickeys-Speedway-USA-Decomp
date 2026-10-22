#include "PR/ultratypes.h"

typedef struct Overlay41StepRecord {
    u8 id;
    u8 residual;
    s16 remaining;
    s32 x;
    s32 dx;
    s32 y;
    s32 dy;
} Overlay41StepRecord;

extern Overlay41StepRecord D_800D6B58[8];
extern void trackAddTextureScroll(s32 id, s32 x, s32 y);

#define gOverlay41StepRecords D_800D6B58
#define overlay41EmitStep trackAddTextureScroll

/* Updating both accumulators before decrementing the independent remaining
 * field preserves the source semantics and reproduces IDO's retail schedule.
 * Pinned exact-overlay scans found no exact DKR/JFG donor. */
void func_overlay_041_F0000000_1887338(s32 amount) {
    Overlay41StepRecord *record;
    s32 i;
    s32 x;
    s32 y;
    s32 step;

    record = gOverlay41StepRecords;
    i = 7;
    do {
        if (record->id != 0xFF) {
            step = record->remaining;
            if (step != 0) {
                if (amount < step) {
                    step = amount;
                }
                record->x += record->dx * step;
                record->y += record->dy * step;
                record->remaining -= step;
            }
            x = ((record->x * amount) >> 8) + (record->residual & 0xF);
            y = ((record->y * amount) >> 8) + (record->residual >> 4);
            overlay41EmitStep(record->id, x >> 3, y >> 3);
            record->residual = x & 7;
            record->residual |= (y & 7) << 4;
        }
        record++;
    } while (i--);
}
