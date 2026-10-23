#include "overlays/overlay_066.h"

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void func_overlay_066_F00004E0_18C6948(Gfx **displayList, u16 *framebuffer, void *segmentBase);

/* Pinned DKR v77/v80 and JFG scans contain no exact donor. */
void overlay66Select(s32 selection) {
    if (selection != gOverlay66Current) {
        gOverlay66Flag = 0;
        gOverlay66Current = selection;
        gOverlay66Timer = 5;
    }
}

s32 overlay66GetCurrent(void) {
    return gOverlay66Current;
}

/* Matched (lane i-7, 2026-10-07). The pixel local is a u16: as an s32 its
 * web outranks every loop value and takes v0, which keeps the timer's
 * address out of v0; as a u16 it ranks below the channel webs, v0 stays
 * free through the loop and as1 lifts the address's lui into the preheader
 * as shipped. The cursor is the output pixel (base + 1), and the countdown
 * is a while loop decremented at the end of the body, which gives the
 * unrolled copies the shipped schedule. */
void func_overlay_066_F0000040_18C64A8(Gfx **commands) {
    u16 *pixels;
    s32 remaining;
    u16 pixel;
    s32 red0;
    s32 green0;
    s32 blue0;
    s32 red1;
    s32 green1;
    s32 blue1;
    s32 red2;
    s32 green2;
    s32 blue2;

    if (gOverlay66Flag == 0) {
        func_overlay_066_F00004E0_18C6948(commands, D_800D2FA0,
                                          D_800D2FAC);
        gOverlay66BssControl = 1;
    } else if (gOverlay66BssControl > 0) {
        gOverlay66BssControl--;
    } else if (gOverlay66Timer > 0) {
        osInvalDCache(D_800D2FAC, 0x25800);
        /* One rolled 76798-step filter. IDO unrolls it itself: the two
         * remainder steps come first, then the 4x body counting 76796 down,
         * which is the shipped shape. Bare statements, not a do-while(0)
         * macro, or the unroller declines (the call after the loop). */
        pixels = D_800D2FAC + 1;
        pixel = pixels[-1] >> 2;
        red0 = (pixel & 0x3800) >> 8;
        green0 = (pixel & 0x01C0) >> 3;
        blue0 = (pixel & 0x000E) << 2;
        pixel = pixels[0] >> 1;
        red1 = (pixel & 0x7800) >> 8;
        green1 = (pixel & 0x03C0) >> 3;
        blue1 = (pixel & 0x001E) << 2;
        remaining = 76798;
        while (remaining != 0) {
            pixel = pixels[1] >> 1;
            red2 = (pixel & 0x7800) >> 8;
            green2 = (pixel & 0x03C0) >> 3;
            blue2 = (pixel & 0x001E) << 2;
            pixels[0] = (((red0 + red1 + (red2 >> 1)) << 8) & 0xF800) |
                        (((green0 + green1 + (green2 >> 1)) << 3) & 0x07C0) |
                        (((blue0 + blue1 + (blue2 >> 1)) >> 2) & 0x003E) | 1;
            red0 = red1 >> 1;
            green0 = green1 >> 1;
            blue0 = blue1 >> 1;
            red1 = red2;
            green1 = green2;
            blue1 = blue2;
            pixels++;
            remaining--;
        }

        gOverlay66Timer--;
        osWritebackDCacheAll();
    }

    gOverlay66Flag = 1;
    func_overlay_066_F00004E0_18C6948(commands, D_800D2FAC,
                                      D_800D2FA8);
}

/*
 * PROVENANCE: the strip loop is the matched resident fxScreenEffect
 * (src/main/fx.c) with its coordinate arguments as locals: a full-screen
 * 320x240 copy at the origin, coordinates rescaled in place, s taken before
 * the rescale. Mickey's own listing supplies the setup.
 *
 * Matched (202 -> 0 masked words) once the strip limit and s were made
 * opaque to uopt's constant propagation while ugen still folds them: the
 * target keeps the limit, s and the rectangle's x/tile word as loop-invariant
 * registers (a materialised 0x3C0 guarding the loop, a zero shifted into the
 * s word), which every plain constant spelling folds away 13 words short.
 * `width * 0` is an opaque zero to uopt (it does not
 * fold a product by zero) and a constant to ugen (o058 F000138C uses the
 * same fact). It is a stand-in for the author's form, not a claim about it:
 * most likely a macro or shared-routine parameter that is 0 in this overlay
 * and multiplies a value the compiler cannot see. The donor's own
 * `s = x0 << 5` lets uopt fold the zero and re-lays out the function (lane c-1). The two unreferenced s32 locals set the frame to 0xA8 with
 * width and height at 0x7C/0x78.
 */
static void func_overlay_066_F00004E0_18C6948(Gfx **displayList, u16 *framebuffer,
                                       void *segmentBase) {
    Gfx *textureCommands;
    s32 videoMode;
    u16 *screen;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 top;
    s32 s;
    s32 pad;
    u32 width;
    u32 height;
    s32 pad2;

    videoMode = viGetVideoMode();
    if (videoMode != 2) {
        if (videoMode != 3) {
            textureCommands = D_50;
        } else {
            textureCommands = D_88;
        }
    } else {
        textureCommands = D_88;
    }

    gDPPipeSync((*displayList)++);
    texDPInit(displayList);
    rsp_segment(displayList, 0, 0);
    rsp_segment(displayList, 1, segmentBase);
    rsp_segment(displayList, 2, 0);
    rsp_segment(displayList, 4, (u8 *)segmentBase - 0x500);
    gDPSetColorImage((*displayList)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320,
                     (void *)0x01000000);
    viGetCurrentSize(&width, &height);
    gDPSetScissor((*displayList)++, G_SC_NON_INTERLACE, 0, 0,
                  width - 1, height - 1);
    gSPDisplayList((*displayList)++, D_0);
    gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);

    x0 = 0;
    y0 = 0;
    x1 = 320;
    y1 = 960 + width * 0;
    screen = framebuffer;
    s = width * 0;
    x0 <<= 2;
    y0 <<= 2;
    x1 <<= 2;
    while (y0 < y1) {
        (*displayList)->words.w0 = textureCommands->words.w0;
        (*displayList)->words.w1 = (u32)screen;
        (*displayList)++;
        {
            Gfx *_g = (*displayList)++;
            _g->words.w1 = (u32)(textureCommands + 1) + 0x80000000;
            _g->words.w0 = 0x07060030;
        }
        screen += 320 * 4;
        top = y0;
        y0 += 16;
        if (y0 > y1) {
            y0 = y1;
        }
        gSPTextureRectangle((*displayList)++, x0, top, x1, y0, G_TX_RENDERTILE,
                            s, 0, 1 << 10, 1 << 10);
    }

    gDPPipeSync((*displayList)++);
    texDPInit(displayList);
    rsp_segment(displayList, 1, (void *)D_0->words.w0);
    gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*displayList)++, 255, 255, 255, 255);
}
