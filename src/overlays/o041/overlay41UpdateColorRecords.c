#include "PR/ultratypes.h"

typedef struct Overlay41ColorRecord {
    s32 handle;
    u8 targetRed;
    u8 red;
    u8 targetGreen;
    u8 green;
    u8 targetBlue;
    u8 blue;
    u8 targetAlpha;
    u8 alpha;
    s16 remaining;
    s16 duration;
} Overlay41ColorRecord;

extern Overlay41ColorRecord D_800D6C58[12];
extern void changeLightColour(s32 handle, s32 red, s32 green, s32 blue);
extern void changeLightIntensity(s32 handle, s32 alpha);

/* Matched 2026-09-12, lane p9-tight. Fifteen words to zero, and the four
 * `initial*` locals were never there: the target's snapshot is a RE-READ of
 * the record field, which uopt common-subexpressions back to the first load,
 * so the load lands in the CSE temp and the working value is the copy. Both
 * the plateau's direction (load into the working local, copy into the
 * snapshot) and its mirror put the load and the copy in the wrong groups, and
 * no colour reachable from either closes it -- the plateau measured 15 and the
 * mirror 25.
 *
 * Read off an instrumented uopt whose object is byte-identical to the
 * configured build. On the plateau body the four working webs take v1/t0/t1/s2
 * at save 20 and the four snapshot webs a1/a2/a3/t2 at save 10, and `remaining`
 * at save 15 lands between them and steals $a0. Forcing `remaining` to c9 and
 * the snapshots to c3-c6 is accepted and scores 8, not 0, which is how the
 * load/copy half was separated from the colour half: the colour was worth 7
 * and the group membership 8, and only a source change reaches the second.
 * With the re-read the CSE temp is its own web and both halves fall together.
 *
 * The 0x80000000 `lui` and the four direct-write `sb` displacements carry
 * relocations in the extracted target and literals here; they are masked, and
 * `promotion-proof` covers the linked identity. */
void func_overlay_041_F0000124_188745C(s32 amount) {
    Overlay41ColorRecord *record;
    s32 i;
    s32 handle;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 remaining;
    s32 factor;
    u8 *direct;

    record = D_800D6C58;
    i = 11;
    do {
        handle = record->handle;
        if (handle != 0) {
            red = record->red;
            green = record->green;
            blue = record->blue;
            alpha = record->alpha;
            remaining = record->remaining;
            if (amount >= remaining) {
                record->handle = 0;
            } else {
                record->remaining = remaining - amount;
                factor = (record->remaining << 16) / record->duration;
                red += ((record->targetRed - record->red) * factor) >> 16;
                green += ((record->targetGreen - record->green) * factor) >> 16;
                blue += ((record->targetBlue - record->blue) * factor) >> 16;
                alpha += ((record->targetAlpha - record->alpha) * factor) >> 16;
            }
            if (handle < 0) {
                changeLightColour(handle, red & 0xFF, green & 0xFF, blue & 0xFF);
                changeLightIntensity(handle, alpha & 0xFF);
            } else {
                direct = (u8 *)((u32)handle | 0x80000000U);
                direct[0x38] = red;
                direct[0x39] = green;
                direct[0x3A] = blue;
                direct[0x3B] = alpha;
            }
        }
        record++;
    } while (i--);
}
