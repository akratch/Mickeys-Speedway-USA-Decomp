#include "overlays/overlay_099.h"

extern void *D_3120;
extern s32 D_3124;
extern s32 D_3128[];
extern void *D_4D56C;
extern u8 D_80000000[];
extern void camStandardPersp(Gfx **displayList, Mtx **matrices);
extern void func_800344D0(Gfx **displayList);
extern void overlay99RenderSegments(Gfx **displayList, Mtx **matrices,
                                    void *vertices, f32 scale);

/* promotion_trial: text-differs, 353 differing in-range words; frame 0xF0 and
 * 13 target relocation sites agree, but linked text is not exact (first
 * mismatch +0x0); structural setup/loop divergence remains. */
#ifdef NON_MATCHING
void func_overlay_099_F0000DDC_18DA38C(Gfx **displayList, Mtx **matrices,
                                       void *vertices, f32 scale, s32 arg4,
                                       s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    Gfx *command;
    s32 alpha;
    s32 outer;
    s32 inner;
    s32 x0;
    s32 x1;
    s32 y0;
    s32 y1;
    s32 right;
    s32 bottom;
    s32 address;
    s32 nextAddress;
    s32 textureOffset;
    s32 nextTextureOffset;
    s32 row;
    s32 rowEnd;
    s32 column;
    s32 columnEnd;
    s32 rectangleWidth;
    s32 rectangleHeight;
    s32 tileWidth;
    s32 tileHeight;
    s32 textureBase;
    s32 width;
    volatile u8 frame_pad[0x38];

    frame_pad[0] = 0;

    if (D_3120 == NULL) {
        return;
    }

    width = arg4 + 1;
    alpha = (s32)((1.0f - scale) * 768.0f);
    if (alpha >= 0x100) {
        alpha = 0xFF;
    }

    camStandardPersp(displayList, matrices);

    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xE7000000;
    command->words.w1 = 0;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xED000000;
    command->words.w1 = 0x005003C0;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xEF30000F;
    command->words.w1 = 0;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xF7000000;
    command->words.w1 = 0x00010001;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xF64FC3BC;
    command->words.w1 = 0;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xE7000000;
    command->words.w1 = 0;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFB000000;
    command->words.w1 = 0xFFFFFF00;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFA000000;
    command->words.w1 = ((alpha & 0xFF) << 24) | ((alpha & 0xFF) << 16) |
                        ((alpha & 0xFF) << 8) | 0xFF;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xB6000000;
    command->words.w1 = 0x00010001;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFC45FE03;
    command->words.w1 = 0x1FFCFDFE;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xEF182C0F;
    command->words.w1 = 0x0F0A4000;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFD10013F;
    command->words.w1 = (u32)D_4D56C;

    outer = 1;
    row = 0;
    if (arg5 > 0) {
        do {
            column = 0;
            if (arg4 > 0) {
                rowEnd = row + arg8;
                y0 = row - 1;
                textureOffset = row * width * 10;
                nextTextureOffset = outer * width * 10;
                do {
                    x0 = column - 1;
                    if (x0 < 0) {
                        x0 = 0;
                    }
                    x1 = column + arg7;
                    y1 = y0;
                    if (y1 < 0) {
                        y1 = 0;
                    }
                    right = 0x13F;
                    if (x1 < right) {
                        right = x1;
                    }
                    rectangleWidth = right - x0;
                    bottom = 0xEF;
                    if (rowEnd < bottom) {
                        bottom = rowEnd;
                    }
                    rectangleHeight = bottom;
                    tileWidth = ((((rectangleWidth * 2) + 9) >> 3) & 0x1FF);

                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xF5100000 | (tileWidth << 9);
                    command->words.w1 = 0x07080200;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xE6000000;
                    command->words.w1 = 0;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xF4000000 | ((x0 * 4) & 0xFFF) << 12 |
                                        ((y1 * 4) & 0xFFF);
                    command->words.w1 = 0x07000000 |
                                        (((right * 4) & 0xFFF) << 12) |
                                        ((bottom * 4) & 0xFFF);
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xE7000000;
                    command->words.w1 = 0;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xF5100000 | (tileWidth << 9);
                    command->words.w1 = 0x00080200;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0xF2000000;
                    command->words.w1 = (((rectangleWidth - 1) * 4) & 0xFFF) << 12 |
                                        (((rectangleHeight - y1) - 1) * 4) & 0xFFF;

                    address = D_3128[D_3124] + textureOffset + 0x80000000;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0x04000026 |
                                        (((address & 6) | 0x18) << 16);
                    command->words.w1 = address;
                    nextAddress = D_3128[D_3124] + nextTextureOffset +
                                  0x80000000;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0x04000626 |
                                        (((nextAddress & 6) | 0x18) << 16);
                    command->words.w1 = nextAddress;
                    command = *displayList;
                    *displayList = command + 1;
                    command->words.w0 = 0x05000000 |
                                        ((((arg6 - 1) * 0x10) | 1) << 16) |
                                        ((arg6 * 0x10) & 0xFFFF);
                    command->words.w1 = (u32)D_80000000;

                    textureOffset += 0x14;
                    nextTextureOffset += 0x14;
                    column = x1;
                } while ((column * 2) < arg4);
            }
            row += arg8;
            outer++;
        } while (outer != arg5);
    }

    func_800344D0(displayList);
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFA000000;
    command->words.w1 = 0xFFFFFFFF;
    overlay99RenderSegments(displayList, matrices, vertices, scale);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o099/func_overlay_099_F0000DDC_18DA38C/func_overlay_099_F0000DDC_18DA38C.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_099_F0000DDC_18DA38C:start
 * symbol: func_overlay_099_F0000DDC_18DA38C
 * score: 331 differing words
 * frame: 0xF0
 * relocations: 13
 * first-mismatch: +0x4
 * summary: 352 candidate instructions; linked text differs and setup/loop structure remains unresolved
 * PLATEAU-HANDOFF:func_overlay_099_F0000DDC_18DA38C:end
 */
