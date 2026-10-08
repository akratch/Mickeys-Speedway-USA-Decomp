/*
 * Weather tail -- ROM 0x3D370-0x3D5F0.
 *
 * This is the C-owned tail of main/weather after its hand-written snow
 * assembly island.  The final helper remains descriptively unresolved.
 */

#include "PR/ultratypes.h"

extern s32 D_8007C854;
extern s32 D_8007C858;
extern s32 D_8007C85C;
extern s32 D_8007C860;
extern s32 D_8007C864;
extern s32 D_8007C868;

void func_8003C80C();

void func_8003C770(s32 timeOfDay, s32 duration) {
    s32 index;

    index = timeOfDay;
    if ((timeOfDay < 0) || (timeOfDay >= 24)) {
        index = 0;
    }
    if (duration < 0) {
        duration = 0;
    }
    if (duration == 0) {
        D_8007C854 = 0;
        D_8007C858 = 255;
        D_8007C85C = 255;
        return;
    }
    D_8007C854 = 1;
    D_8007C860 = index >> 2;
    D_8007C864 = ((index & 3) * duration) >> 2;
    D_8007C868 = duration;
    func_8003C80C(0, duration, index);
}

typedef struct WeatherColor {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} WeatherColor;

typedef struct WeatherKey {
    WeatherColor colors[9];
    s32 unk24;
    s32 unk28;
} WeatherKey;

extern WeatherKey *D_8007C838[];
extern WeatherColor D_800D40F0[];
extern void func_8002EBD4(WeatherColor *colors);

/* 2026-10-07 (lane i-1): matched by the natural body. The time global is
 * advanced and wrapped in place (no carrier, one parameter, so nothing is
 * homed), and the fraction is computed before the two key lookups, which
 * gives the wrap's ring draws and the dead copy in its delay slot. The one
 * zero-cost block around the call keeps the counter in t0 and the fraction
 * in t1 (brief item 18). */
void func_8003C80C(s32 updateRate) {
    s32 i;
    s32 frac;
    WeatherColor *dst;
    WeatherColor *src0;
    WeatherColor *src1;
    WeatherKey *from;
    WeatherKey *to;

    if (D_8007C854 != 0) {
        D_8007C864 += updateRate;
        while (D_8007C864 >= D_8007C868) {
            D_8007C864 -= D_8007C868;
            D_8007C860++;
            if (D_8007C860 >= 6) {
                D_8007C860 = 0;
            }
        }
        frac = (D_8007C864 << 16) / D_8007C868;
        from = D_8007C838[D_8007C860];
        to = D_8007C838[D_8007C860 + 1];
        src0 = from->colors;
        src1 = to->colors;
        dst = D_800D40F0;
        i = 8;
        do {
            dst->r = (((src1->r - src0->r) * frac) >> 16) + src0->r;
            dst->g = (((src1->g - src0->g) * frac) >> 16) + src0->g;
            dst->b = (((src1->b - src0->b) * frac) >> 16) + src0->b;
            dst->a = src0->a;
            src0++;
            src1++;
            dst++;
        } while (i--);
        do { func_8002EBD4(D_800D40F0); } while (0);
        D_8007C858 = from->unk24 + (((to->unk24 - from->unk24) * frac) >> 16);
        D_8007C85C = from->unk28 + (((to->unk28 - from->unk28) * frac) >> 16);
    }
}
