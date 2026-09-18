#include "PR/ultratypes.h"

typedef struct Overlay44Gfx {
    u32 w0;
    u32 w1;
} Overlay44Gfx;

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    u8 *data;
    s32 frameSize;
} Overlay44FrameSource;

typedef struct Overlay44AnimationState {
    s8 sourceIndex;
    u8 mode;
    u8 flags;
    u8 subtype;
    s32 phase;
    s16 value8;
    s16 valueA;
    u8 pad0C[2];
    s8 protectedSlot0;
    s8 protectedSlot1;
    s8 cachedFrame[4];
    void *handles[4];
} Overlay44AnimationState;

#define OVERLAY44_CMD(pkt, a, b)         \
    {                                    \
        Overlay44Gfx *_g = (pkt);        \
        _g->w0 = (u32)(a);               \
        _g->w1 = (u32)(b);               \
    }

extern Overlay44FrameSource *gOverlay44FrameSources;
extern u8 D_0[];
extern u8 D_28[];
extern void func_overlay_044_F0000000_188B860();

/* Frame closed to 0x100 by dissolving extra mips_to_c s32s (L134) and placing
 * five used-but-colored working s32s between the width and y-prev homes (L99).
 * Size is one word short; incoming pointer stays in a0 (web 2 cost 0 vs a2 0.1). */
/* No external donor body was used. */
#ifdef NON_MATCHING
void func_overlay_044_F0000580_188BDE0(
    Overlay44AnimationState *arg0,
    Overlay44Gfx **arg1,
    f32 arg2) {
    s32 spFC;
    s32 var_t2;
    s32 var_t3;
    s32 temp_t4_2;
    s32 var_t5;
    s32 temp_t0;
    s32 spE4;
    u8 *var_s0;
    u8 *var_s1;
    s32 var_a3;
    Overlay44AnimationState *state;

    state = (Overlay44AnimationState *)((u32)arg0 | 0);
    if (state != 0) {
        if (state->sourceIndex != -1) {
            var_s0 = state->handles[state->protectedSlot0];
            spFC = gOverlay44FrameSources[state->sourceIndex].dimension0;
            var_t3 = gOverlay44FrameSources[state->sourceIndex].dimension1;
            var_s1 = state->handles[state->protectedSlot1];
            if (arg2 == 1.0f) {
                OVERLAY44_CMD((*arg1)++, 0x06000000, D_0);
            } else {
                OVERLAY44_CMD((*arg1)++, 0x06000000, D_28);
            }

            var_t2 = state->subtype;
            OVERLAY44_CMD((*arg1)++, 0xFA000000,
                (var_t2 << 24) | (var_t2 << 16) | (var_t2 << 8) | 0xFF);
            var_t2 = state->phase & 0xFF;
            OVERLAY44_CMD((*arg1)++, 0xFB000000,
                (var_t2 << 24) | (var_t2 << 16) | (var_t2 << 8) | var_t2);

            var_t2 = state->value8 * 4;
            spE4 = state->valueA << 16;
            if (var_t3 != 0) {
                s32 sp64;
                s32 sp58;
                s32 sp54;
                s32 sp50;
                s32 sp4C;
                s32 sp48;
                s32 sp44;
                s32 sp40;

                sp64 = spFC * 2;
                temp_t4_2 = (0x800 / sp64) & ~1;
                sp58 = ((((sp64 + 7) >> 3) & 0x1FF) << 9) | 0xF5100000;
                sp54 = sp58 | 0x100;
                sp50 = (((spFC - 1) * 4) & 0xFFF) << 12;
                sp4C = sp50 | 0x01000000;
                sp48 = ((((s32)((f32)spFC * arg2 * 4.0f) + var_t2) & 0xFFF)
                        << 12) | 0xE4000000;
                sp44 = (var_t2 & 0xFFF) << 12;
                var_a3 = (s32)(1024.0f / arg2);
                sp40 = ((var_a3 & 0xFFFF) << 16) | (var_a3 & 0xFFFF);
                var_t5 = spE4;
                arg2 *= 65536.0f;

                do {
                    var_t2 = var_t3;
                    if (temp_t4_2 < var_t3) {
                        var_t2 = temp_t4_2;
                        var_t3 -= temp_t4_2;
                    } else {
                        var_t3 = 0;
                    }

                    OVERLAY44_CMD((*arg1)++, 0xFD100000, var_s1);
                    OVERLAY44_CMD((*arg1)++, 0xF5100100, 0x07080200);
                    var_t5 += (s32)((f32)var_t2 * arg2);
                    OVERLAY44_CMD((*arg1)++, 0xE6000000, 0);
                    temp_t0 = (spFC * var_t2) - 1;
                    var_a3 = 0x7FF;
                    if (temp_t0 < 0x7FF) {
                        var_a3 = temp_t0;
                    }
                    OVERLAY44_CMD((*arg1)++, 0xF3000000,
                        ((var_a3 & 0xFFF) << 12) | 0x07000000);
                    OVERLAY44_CMD((*arg1)++, 0xE7000000, 0);
                    OVERLAY44_CMD((*arg1)++, sp54, 0x01080200);
                    OVERLAY44_CMD((*arg1)++, 0xF2000000,
                        sp4C | (((var_t2 - 1) * 4) & 0xFFF));

                    OVERLAY44_CMD((*arg1)++, 0xFD100000, var_s0);
                    OVERLAY44_CMD((*arg1)++, 0xF5100000, 0x07080200);
                    OVERLAY44_CMD((*arg1)++, 0xE6000000, 0);
                    var_a3 = 0x7FF;
                    if (temp_t0 < 0x7FF) {
                        var_a3 = temp_t0;
                    }
                    OVERLAY44_CMD((*arg1)++, 0xF3000000,
                        ((var_a3 & 0xFFF) << 12) | 0x07000000);
                    OVERLAY44_CMD((*arg1)++, 0xE7000000, 0);
                    OVERLAY44_CMD((*arg1)++, sp58, 0x00080200);
                    OVERLAY44_CMD((*arg1)++, 0xF2000000,
                        sp50 | (((var_t2 - 1) * 4) & 0xFFF));

                    OVERLAY44_CMD((*arg1)++,
                        sp48 | ((var_t5 >> 14) & 0xFFF),
                        sp44 | ((spE4 >> 14) & 0xFFF));
                    OVERLAY44_CMD((*arg1)++, 0xB3000000, 0);
                    OVERLAY44_CMD((*arg1)++, 0xB2000000, sp40);

                    spE4 = var_t5;
                    var_s0 += var_t2 * sp64;
                    var_s1 += var_t2 * sp64;
                } while (var_t3 != 0);
            }

            func_overlay_044_F0000000_188B860(arg1);
            OVERLAY44_CMD((*arg1)++, 0xFA000000, 0xFFFFFFFF);
            OVERLAY44_CMD((*arg1)++, 0xFB000000, 0xFFFFFFFF);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o044/func_overlay_044_F0000580_188BDE0/func_overlay_044_F0000580_188BDE0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_044_F0000580_188BDE0:start
 * symbol: func_overlay_044_F0000580_188BDE0
 * score: 338 differing words
 * frame: 0x100
 * relocations: 7
 * first-mismatch: +0x8
 * summary: Frame 0x100 exact. Size -4 (348 vs 349). Incoming pointer web stays in a0; a2 copy and a0 home store are the missing word plus the 0x100 slot.
 * PLATEAU-HANDOFF:func_overlay_044_F0000580_188BDE0:end
 */
