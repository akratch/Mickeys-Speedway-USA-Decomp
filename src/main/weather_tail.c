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

void func_8003C770(s32 arg0, s32 arg1) {
    s32 index;

    index = arg0;
    if ((arg0 < 0) || (arg0 >= 24)) {
        index = 0;
    }
    if (arg1 < 0) {
        arg1 = 0;
    }
    if (arg1 == 0) {
        D_8007C854 = 0;
        D_8007C858 = 255;
        D_8007C85C = 255;
        return;
    }
    D_8007C854 = 1;
    D_8007C860 = index >> 2;
    D_8007C864 = ((index & 3) * arg1) >> 2;
    D_8007C868 = arg1;
    func_8003C80C(0, arg1, index);
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

#ifdef NON_MATCHING
void func_8003C80C(s32 arg0, s32 time) {
    s32 i;
    s32 frac;
    WeatherColor *dst;
    WeatherColor *src0;
    WeatherColor *src1;
    WeatherKey *from;
    WeatherKey *to;

    if (D_8007C854 != 0) {
        time = D_8007C864;
        time += arg0;
        D_8007C864 = time;
        i = 8;
        while ((time < D_8007C868) == 0) {
            D_8007C864 = time - D_8007C868;
            D_8007C860++;
            if (D_8007C860 >= 6) {
                D_8007C860 = 0;
            }
            time = D_8007C864;
        }
        from = D_8007C838[D_8007C860];
        to = D_8007C838[D_8007C860 + 1];
        frac = (time << 16) / D_8007C868;
        src0 = from->colors;
        src1 = to->colors;
        dst = D_800D40F0;
        do {
            dst->r = src0->r + (((src1->r - src0->r) * frac) >> 16);
            dst->g = src0->g + (((src1->g - src0->g) * frac) >> 16);
            dst->b = src0->b + (((src1->b - src0->b) * frac) >> 16);
            dst->a = src0->a;
            src0++;
            src1++;
            dst++;
        } while (i--);
        func_8002EBD4(D_800D40F0);
        D_8007C858 = from->unk24 + (((to->unk24 - from->unk24) * frac) >> 16);
        D_8007C85C = from->unk28 + (((to->unk28 - from->unk28) * frac) >> 16);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather_tail/func_8003C80C.s")
#endif

/* PLATEAU-HANDOFF:func_8003C80C:start
 * symbol: func_8003C80C
 * score: 74/118 words
 * frame: 0x38
 * relocations: 21
 * first-mismatch: +0x14
 * summary: Nine colours (do-while, post-decrement from 8), time in the second parameter: 103 to 74 at delta 0. Defining dst before the source pointers grows the body; the v1 pool already matches.
 * PLATEAU-HANDOFF:func_8003C80C:end
 */
