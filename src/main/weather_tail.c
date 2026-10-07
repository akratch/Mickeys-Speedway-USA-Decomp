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
/* 2026-10-07 (lane g-4): 43 -> 35. One zero-cost block around each of the
 * call and the two scalar lerps after the colour loop (brief item 18):
 * frac then spans more blocks, its save falls below the counter's, and the
 * counter takes t0 and frac t1 as shipped. Left: the time wrap (ring phase
 * and the dead copy in the wrap's delay slot) and from->unk28's register. */
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
            dst->r = (((src1->r - src0->r) * frac) >> 16) + src0->r;
            dst->g = (((src1->g - src0->g) * frac) >> 16) + src0->g;
            dst->b = (((src1->b - src0->b) * frac) >> 16) + src0->b;
            dst->a = src0->a;
            src0++;
            src1++;
            dst++;
        } while (i--);
        do { func_8002EBD4(D_800D40F0); } while (0);
        do { D_8007C858 = from->unk24 + (((to->unk24 - from->unk24) * frac) >> 16); } while (0);
        do { D_8007C85C = from->unk28 + (((to->unk28 - from->unk28) * frac) >> 16); } while (0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather_tail/func_8003C80C.s")
#endif

/* PLATEAU-HANDOFF:func_8003C80C:start
 * symbol: func_8003C80C
 * score: 35/118 words
 * frame: 0x38
 * relocations: 21
 * first-mismatch: +0x14
 * summary: Zero-cost blocks on the call and both scalar lerps lower frac's save: counter t0, frac t1 as shipped, 43 to 35 at 0. Left: the time wrap window.
 * PLATEAU-HANDOFF:func_8003C80C:end
 */
