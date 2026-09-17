#include "overlays/overlay_056.h"

/*
 * Overlay 56, ADR 0006 consolidation: one translation unit in ROM order.
 * Exact DKR v77/v80 and JFG scans are negative for the matched C functions;
 * the unresolved middle function remains GLOBAL_ASM.
 */

void overlay56OffsetCoordinates(u32 *x, u32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x += width >> 1;
    *y = (height >> 1) - *y;
}

void overlay56CenterCoordinates(s32 *x, s32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x -= width >> 1;
    *y = (height >> 1) - *y;
}

void overlay56SplitTime(s32 value, s32 *minutes, s32 *seconds,
                        s32 *centiseconds) {
    s32 wholeMinutes;
    wholeMinutes = value / 18000;
    *minutes = wholeMinutes;
    *seconds = (value / 300) - (wholeMinutes * 60);
    *centiseconds = (value / 3) % 100;
}

void overlay56SetMode(s32 mode) {
    gOverlay56Mode = mode;
}

void overlay56LoadResource(void) {
    Overlay56Context *context;

    context = overlay56GetContextReloc();
    if (context->resourceId != -1) {
        gOverlay56Resource = overlay56LoadResourceReloc(context->resourceId);
    } else {
        gOverlay56Resource = 0;
    }
    gOverlay56ResourceState = 0;
}

void overlay56ReleaseResource(void) {
    if (gOverlay56Resource != 0) {
        overlay56ReleaseResourceReloc(gOverlay56Resource);
        gOverlay56Resource = 0;
    }
}

/* Workbench: structure-mismatch; 538 words differ, first mismatch +0x50. */
/* Candidate is not shape-exact: 591/581 instructions; frame 0x1F8 exact. */
/* Colour table is a data-section field at +0x50; minimap sprite is gOverlay56Resource. */
#ifdef NON_MATCHING
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((u8 *)(expr) + (offset)))

#define O56_RAW58 (*(void **)(D_8004DC30 + 0x58))

extern u8 *func_overlay_056_F0000000_18A2D78();
extern u8 *func_80028F54(void);
extern void viGetCurrentSize(u32 *width, u32 *height);
extern void camStandardOrtho(void **displayList, s32 *matrixCursor);
extern u8 *func_80005750(u32 *count);
extern u8 *levelGetLevel(void);
extern s32 frontGet2PlayerSplit(void);
extern void func_8002FB34(void **displayList, void *state, f32 x, f32 y,
                         f32 scaleX, f32 scaleY, s32 flags, s32 mode);
extern void func_8002F618(void **displayList, void *state, s32 x, s32 y,
                         u8 red, u8 green, u8 blue, u8 alpha);
extern void func_800349A4(void **displayList, void *texture, s32 mode,
                         s32 flags);
extern void func_8002A82C(void *matrix);
extern void matrixTranslate(f32 x, f32 y, f32 z, void *matrix);
extern void func_8002A604(s32 angle, void *matrix);
extern void func_80024978(void *matrix);
extern void mtxf_mul(void *lhs, void *rhs, void *dest);
extern void mtxf_to_mtx(void *src, void *dest);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern u8 D_8004DC30[];
extern s32 D_800C3A3C;
extern s32 D_800D3450;
extern s16 D_78[];
extern s16 D_84[];
extern u8 D_80000004[];
extern u8 D_80000030[];

typedef struct {
    s32 value0;
    s32 value4;
    s32 value8;
    s16 valueC;
    s16 valueE;
    s32 value10;
    u8 pad[112];
} Overlay56CallState;

/* Packed minimap colours sit at overlay data +0x50, stride 4. */
typedef struct Overlay56Data {
    u8 pad[0x50];
    u32 colors[1];
} Overlay56Data;

extern Overlay56Data gOverlay56Data;

void func_overlay_056_F00001A0_18A2F18(void **displayList, s32 *vertexCursor,
                                      s32 updateRate) {
    u32 sp1D4;
    f32 sp1B4;
    u32 sp168;
    u32 sp128;
    void *spE8;
    u32 spE4;
    s32 spE0;
    Overlay56CallState spC0State;
    u8 *spBC;
    u8 *spB4;
    s32 spB0;
    u32 spAC;
    s32 spA8;
    s32 sp8C;
    s16 *sp7C;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f12;
    f32 temp_f12_2;
    f32 temp_f14;
    f32 temp_f20;
    f32 temp_f22;
    f32 temp_f24;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 var_f10;
    f32 var_f16;
    f32 var_f16_2;
    f32 var_f6;
    f32 var_f6_2;
    s16 *temp_t5;
    s16 *var_s6;
    s32 temp_t5_2;
    u8 **temp_v0_4;
    u8 **temp_v0_5;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_s1;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s7;
    s32 var_v1;
    u32 temp_t4;
    u32 temp_t7;
    u32 temp_v0_3;
    u32 temp_v0_6;
    u32 var_a2;
    u8 *temp_s1;
    u8 *temp_s3;
    u8 *temp_v0;
    u8 temp_a0;
    u8 temp_v1;
    u8 temp_v1_4;
    u8 **var_s5;
    void *temp_fp;
    u8 *temp_s0;
    u8 *temp_s1_2;
    u8 *temp_t3;
    u8 *temp_t5_3;
    u8 *temp_t6;
    u8 *temp_v0_2;
    u8 *temp_v1_2;
    u8 *temp_v1_5;
    u8 *var_s0;
    temp_v0 = func_80028F54();
    spBC = temp_v0;
    if (D_800C3A3C == 0) {
        temp_a0 = gOverlay56Mode;
        if (temp_a0 == 3) {
            var_v1 = 2;
        } else {
            var_v1 = 1;
        }
        gOverlay56ResourceState = (s16) (gOverlay56ResourceState +
                                                (updateRate << var_v1));
        if ((temp_a0 != 3) && (gOverlay56ResourceState >= 0xA1)) {
            gOverlay56ResourceState = 0xA0;
        } else if (gOverlay56ResourceState >= 0x100) {
            gOverlay56ResourceState = 0xFF;
        }
    }
    if ((gOverlay56Resource != NULL) &&
        ((D_800D3450 == 0) ||
         ((temp_v1 = *temp_v0, (temp_v1 != 0)) &&
          (temp_v1 != 2) && (temp_v1 != 1) && (temp_v1 != 3) &&
          (temp_v1 != 4)))) {
        spB4 = *displayList;
        spB0 = *vertexCursor;
        viGetCurrentSize(&spAC, (u32 *) &spA8);
        temp_t6 = spB4;
        spB4 = temp_t6 + 8;
        M2C_FIELD(temp_t6, u32 *, 4) = 0;
        M2C_FIELD(temp_t6, u32 *, 0) = 0xE7000000;
        temp_v1_2 = spB4;
        spB4 = temp_v1_2 + 8;
        M2C_FIELD(temp_v1_2, u32 *, 0) = 0xED000000;
        var_f6 = (f32) spA8;
        if (spA8 < 0) {
            var_f6 += 4294967296.0f;
        }
        M2C_FIELD(temp_v1_2, u32 *, 4) =
            (u32) ((((s32) ((f32) spAC * 4.0f) & 0xFFF) << 0xC) |
                   ((s32) (var_f6 * 4.0f) & 0xFFF));
        camStandardOrtho((void **) &spB4, &spB0);
        temp_s1 = func_80005750(&sp1D4);
        temp_s3 = levelGetLevel();
        spC0State.value4 = 0;
        spC0State.value0 = (s32) gOverlay56Resource;
        if ((gOverlay56Mode == 2) &&
            (var_a0 = 4,
             frontGet2PlayerSplit() != 0)) {
        } else {
            var_a0 = gOverlay56Mode - 1;
        }
        temp_v0_2 = gOverlay56Resource;
        temp_v1_3 = var_a0;
        temp_t5 = &D_84[temp_v1_3];
        sp7C = temp_t5;
        spC0State.valueE = (*temp_t5 -
                            ((s32) M2C_FIELD(temp_v0_2, u16 *, 8) >> 1)) + 0xB4;
        spC0State.value10 = 0;
        if (gOverlay56Mode != 0) {
            var_s6 = &D_78[temp_v1_3];
            spC0State.valueC = (-*var_s6 -
                    ((s32) M2C_FIELD(temp_v0_2, u16 *, 6) >> 1)) + 0x3C;
            func_8002FB34((void **) &spB4, &spC0State, 320.0f, 0.0f,
                          1.0f, 1.0f,
                          gOverlay56ResourceState | ~0xFF, 0x1002);
        } else {
            var_s6 = &D_78[temp_v1_3];
            spC0State.valueC = (*var_s6 -
                    ((s32) M2C_FIELD(temp_v0_2, u16 *, 6) >> 1)) + 0x104;
            func_8002F618((void **) &spB4, &spC0State, 0, 0, 0xFF, 0xFF,
                          0xFF, (u8) gOverlay56ResourceState);
        }
        viGetCurrentSize(&spE4, (u32 *) &spE0);
        temp_fp = O56_RAW58;
        temp_f22 = func_8002A8BC(M2C_FIELD(temp_s3, s16 *, 0x122));
        temp_f24 = func_8002A8C0(M2C_FIELD(temp_s3, s16 *, 0x122));
        temp_t5_2 = gOverlay56ResourceState * 2;
        var_s7 = temp_t5_2;
        if (temp_t5_2 >= 0x100) {
            var_s7 = 0xFF;
        }
        spC0State.value4 = 0;
        spC0State.value10 = 0;
        var_s2 = sp1D4 - 1;
        if (sp1D4 != 0) {
            var_s5 = (u8 **)(temp_s1 + (var_s2 * 4));
            do {
                temp_s0 = *var_s5;
                temp_f2 = M2C_FIELD(temp_s3, f32 *, 0x128);
                temp_v1_4 = gOverlay56Mode;
                temp_f0 = M2C_FIELD(temp_s0, f32 *, 0xC) * temp_f2;
                temp_f12 = M2C_FIELD(temp_s0, f32 *, 0x14) * temp_f2;
                temp_s1_2 = M2C_FIELD(temp_s0, u8 **, 0x64);
                temp_f14 = (temp_f0 * temp_f22) - (temp_f12 * temp_f24);
                var_f16 = temp_f14;
                temp_f20 = (temp_f12 * temp_f22) + (temp_f0 * temp_f24);
                if (temp_v1_4 != 0) {
                    var_f16 = -temp_f14;
                }
                if (var_s2 < (s32) gOverlay56Mode) {
                    sp1B4 = var_f16;
                    func_800349A4((void **) &spB4, NULL, 5, 0);
                    func_8002A82C(&sp168);
                    if (gOverlay56Mode != 0) {
                        var_f16_2 = sp1B4 +
                            (f32) ((*var_s6 -
                                    M2C_FIELD(temp_s3, s16 *, 0x124)) +
                                   0x208);
                    } else {
                        var_f16_2 = sp1B4 +
                            (f32) (M2C_FIELD(temp_s3, s16 *, 0x124) +
                                   *var_s6);
                    }
                    temp_t7 = spE4 >> 1;
                    temp_t4 = (u32) spE0 >> 1;
                    var_f6_2 = (f32) temp_t7;
                    if ((s32) temp_t7 < 0) {
                        var_f6_2 += 4294967296.0f;
                    }
                    var_f10 = (f32) temp_t4;
                    if ((s32) temp_t4 < 0) {
                        var_f10 += 4294967296.0f;
                    }
                    matrixTranslate(
                        var_f16_2 - var_f6_2,
                        var_f10 -
                            (temp_f20 +
                             (f32) (M2C_FIELD(temp_s3, s16 *, 0x126) +
                                    *sp7C)),
                        0.0f, &sp168);
                    if (gOverlay56Mode != 0) {
                        func_8002A604(
                            (s16) ((M2C_FIELD(temp_s0, s16 *, 0) -
                                    M2C_FIELD(temp_s3, s16 *, 0x122)) * -1),
                            &sp168);
                    } else {
                        func_8002A604(
                            (s16) (M2C_FIELD(temp_s0, s16 *, 0) -
                                   M2C_FIELD(temp_s3, s16 *, 0x122)),
                            &sp168);
                    }
                    func_80024978(&sp128);
                    mtxf_mul(&sp168, &sp128, &spE8);
                    mtxf_to_mtx(&spE8, (void *) spB0);
                    temp_t5_3 = spB4;
                    spB4 = temp_t5_3 + 8;
                    M2C_FIELD(temp_t5_3, u32 *, 0) = 0x01000040;
                    M2C_FIELD(temp_t5_3, u32 *, 4) = spB0 + 0x80000000;
                    temp_v0_2 = spB4;
                    spB0 += 0x40;
                    if (((s32) (M2C_FIELD(temp_s1_2, u8 *, 0x190) *
                               var_s7) >> 8) > 0) {
                        spB4 = temp_v0_2 + 8;
                        M2C_FIELD(temp_v0_2, u32 *, 0) = 0xFA000000;
                        M2C_FIELD(temp_v0_2, u32 *, 4) =
                            (gOverlay56Data.colors[M2C_FIELD(temp_s1_2, s8 *, 1)] |
                             ((s32) (var_s7 *
                                     M2C_FIELD(temp_s1_2, u8 *, 0x190)) >>
                              8));
                        temp_v1_5 = spB4;
                        temp_t3 = temp_v1_5 + 8;
                        spB4 = temp_t3;
                        M2C_FIELD(temp_v1_5, u32 *, 0) =
                            (((((s32) D_80000004 & 6) | 0x20) & 0xFF) <<
                             0x10) | 0x04000000 | 0x30;
                        M2C_FIELD(temp_v1_5, u32 *, 4) = (u32) D_80000004;
                        spB4 = temp_t3 + 8;
                        M2C_FIELD(temp_t3, u32 *, 4) = (u32) D_80000030;
                        M2C_FIELD(temp_t3, u32 *, 0) = 0x05110020;
                    }
                } else {
                    spC0State.value0 = (s32) O56_RAW58;
                    spC0State.valueC = (s16) (s32) (var_f16 -
                                       (f32) ((s32) M2C_FIELD(temp_fp,
                                                               u16 *, 6) >> 1));
                    spC0State.valueE = (s16) (s32) (temp_f20 -
                                       (f32) ((s32) M2C_FIELD(temp_fp,
                                                               u16 *, 8) >> 1));
                    temp_v0_3 = gOverlay56Data.colors[M2C_FIELD(temp_s1_2, s8 *, 1)];
                    if (temp_v1_4 != 0) {
                        var_a2 = (*var_s6 -
                                  M2C_FIELD(temp_s3, s16 *, 0x124)) + 0x208;
                    } else {
                        var_a2 = *var_s6 +
                                 M2C_FIELD(temp_s3, s16 *, 0x124);
                    }
                    func_8002F618(
                        (void **) &spB4, &spC0State, var_a2,
                        *sp7C + M2C_FIELD(temp_s3, s16 *, 0x126),
                        temp_v0_3 >> 0x18, (temp_v0_3 >> 0x10) & 0xFF,
                        (temp_v0_3 >> 8) & 0xFF,
                        (s32) (M2C_FIELD(temp_s1_2, u8 *, 0x190) * var_s7) >>
                            8);
                }
                var_s5 -= 4;
                var_s2 -= 1;
            } while (var_s2 != 0);
        }
        var_s2_2 = 1;
        if (*spBC == 1) {
            var_s1 = sp8C;
            do {
                temp_v0_4 = (u8 **)0;
                if ((var_s2_2 != 0) &&
                    (temp_v0_4 = *(u8 ***)0, temp_v0_4 != NULL)) {
                    var_s0 = *temp_v0_4;
                    var_s1 = 0xFF;
                } else {
                    var_s0 = NULL;
                    if (var_s2_2 == 0) {
                        temp_v0_5 = *(u8 ***)0;
                        if (temp_v0_5 != NULL) {
                            var_s0 = *temp_v0_5;
                            var_s1 = 0x55;
                        }
                    }
                }
                if (var_s0 != NULL) {
                    temp_f2_2 = M2C_FIELD(temp_s3, f32 *, 0x128);
                    temp_f0_2 = M2C_FIELD(var_s0, f32 *, 0xC) * temp_f2_2;
                    temp_f12_2 = M2C_FIELD(var_s0, f32 *, 0x14) * temp_f2_2;
                    spC0State.value0 = (s32) O56_RAW58;
                    temp_v0_6 = var_s1 & 0xFF;
                    spC0State.valueC = (s16) (s32) (((temp_f0_2 * temp_f22) -
                                         (temp_f12_2 * temp_f24)) -
                                        (f32) ((s32) M2C_FIELD(temp_fp,
                                                               u16 *, 6) >> 1));
                    spC0State.valueE = (s16) (s32) (((temp_f12_2 * temp_f22) +
                                         (temp_f0_2 * temp_f24)) -
                                        (f32) ((s32) M2C_FIELD(temp_fp,
                                                               u16 *, 8) >> 1));
                    func_8002F618(
                        (void **) &spB4, &spC0State,
                        *var_s6 + M2C_FIELD(temp_s3, s16 *, 0x124),
                        *sp7C + M2C_FIELD(temp_s3, s16 *, 0x126), temp_v0_6,
                        temp_v0_6, temp_v0_6,
                        (s32) (M2C_FIELD(M2C_FIELD(var_s0, u8 **, 0x64),
                                        u8 *, 0x0F) * var_s7) >> 7);
                }
                var_s2_2 -= 1;
            } while (var_s2_2 != 0);
            sp8C = var_s1;
        }
        *displayList = spB4;
        *vertexCursor = spB0;
    }
}

#undef O56_RAW58
#undef M2C_FIELD
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o056/overlay_056/func_overlay_056_F00001A0_18A2F18.s")
#endif

void overlay56UnpackColor(s32 index, u32 *red, s32 *green, s32 *blue) {
    u32 *color = &gOverlay56Colors[index];
    *red = *color >> 24;
    *green = (*color >> 16) & 0xFF;
    *blue = (*color >> 8) & 0xFF;
}

/* PLATEAU-HANDOFF:func_overlay_056_F00001A0_18A2F18:start
 * symbol: func_overlay_056_F00001A0_18A2F18
 * score: 538 differing words
 * frame: 0x1F8
 * relocations: 75
 * first-mismatch: +0x50
 * summary: Colour table reconstructed as a field at overlay-data +0x50 (gOverlay56Data.colors[index]) so both marker sites emit lui plus scaled addu plus lw 0x50 off the data-section base, the live pointer the target already holds. Indexing gOverlay56Colors by name still costs an extra addiu per site. Minimap sprite identity is gOverlay56Resource, not a NULL-page load. Aligned split 132 naming 244 immediate 28 structural 216 versus the prior 130/237/28/223; size is 591 versus 581 (delta +40) because the colour loads add the four missing base words plus leftover surplus elsewhere. The eight-word hole at target +0x6D8 remains unpack schedule: the target shifts RGB immediately after the load (green, blue, then red in the mode-branch delay slot) while this candidate still unpacks in the call. Identity-gate passed with CDX_PROC=6. Do not colour-landscape until size delta is 0. Ghost slots at D_800D1494 (alpha 255) and D_800D1498 (alpha 85) are identified but naming them did not move the aligned residual. Next: force the AI-site unpack before the mode branch without extra copies, then the unsigned-float surplus around +0x498.
 * PLATEAU-HANDOFF:func_overlay_056_F00001A0_18A2F18:end
 */
