#include "PR/ultratypes.h"

typedef struct Overlay66Gfx Overlay66Gfx;

extern s32 gOverlay66Timer;
extern s32 gOverlay66Flag;
extern s32 D_F8;
extern s32 gOverlay66BssControl;
extern u16 *gOverlay66FirstPrimary;
extern u16 *gOverlay66SharedFirst;
extern u16 *gOverlay66SharedMutation;
extern u16 *gOverlay66SharedPixels;
extern u16 *gOverlay66SharedFinal;
extern u16 *gOverlay66FinalSecondary;

void func_overlay_066_F00004E0_18C6948(Overlay66Gfx **commands,
                                       const u16 *primary,
                                       const u16 *secondary);
void func_overlay_066_F0000000_18C6468();

/* PLATEAU (2026-08-26): workbench structure-mismatch; 249 masked/251 raw of 296 words, first +0x84.
 * Flag lattice confirmed the retained exact-size candidate; pointer/filter and register-form hypotheses remain eliminated.
 * Exact size/frame remain with 73 structural and 209 register differences. */
#ifdef NON_MATCHING
void func_overlay_066_F0000040_18C64A8(Overlay66Gfx **commands) {
    u16 *pixels;
    s32 remaining;
    s32 pixel;
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
        func_overlay_066_F00004E0_18C6948(commands, gOverlay66FirstPrimary,
                                          gOverlay66SharedFirst);
        gOverlay66BssControl = 1;
    } else if (gOverlay66BssControl > 0) {
        gOverlay66BssControl--;
    } else if (gOverlay66Timer > 0) {
        func_overlay_066_F0000000_18C6468(gOverlay66SharedMutation, 0x25800);
        /* One rolled 76798-step filter. IDO unrolls it itself: the two
         * remainder steps come first, then the 4x body counting 76796 down,
         * which is the shipped shape. Bare statements, not a do-while(0)
         * macro, or the unroller declines (the call after the loop). */
        pixels = gOverlay66SharedPixels;
        pixel = pixels[0] >> 2;
        red0 = (pixel & 0x3800) >> 8;
        green0 = (pixel & 0x01C0) >> 3;
        blue0 = (pixel & 0x000E) << 2;
        pixel = pixels[1] >> 1;
        red1 = (pixel & 0x7800) >> 8;
        green1 = (pixel & 0x03C0) >> 3;
        blue1 = (pixel & 0x001E) << 2;
        for (remaining = 76798; remaining != 0; remaining--) {
            pixel = pixels[2] >> 1;
            red2 = (pixel & 0x7800) >> 8;
            green2 = (pixel & 0x03C0) >> 3;
            blue2 = (pixel & 0x001E) << 2;
            pixels[1] = (((red0 + red1 + (red2 >> 1)) << 8) & 0xF800) |
                        (((green0 + green1 + (green2 >> 1)) << 3) & 0x07C0) |
                        (((blue0 + blue1 + (blue2 >> 1)) >> 2) & 0x003E) | 1;
            red0 = red1 >> 1;
            green0 = green1 >> 1;
            blue0 = blue1 >> 1;
            red1 = red2;
            green1 = green2;
            blue1 = blue2;
            pixels++;
        }

        D_F8--;
        func_overlay_066_F0000000_18C6468();
    }

    gOverlay66Flag = 1;
    func_overlay_066_F00004E0_18C6948(commands, gOverlay66SharedFinal,
                                      gOverlay66FinalSecondary);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o066/overlay66SmoothAndDraw/func_overlay_066_F0000040_18C64A8.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_066_F0000040_18C64A8:start
 * symbol: func_overlay_066_F0000040_18C64A8
 * score: 249 differing words
 * frame: -0x18
 * relocations: 28
 * first-mismatch: +0x4
 * summary: Rolled loop: IDO unrolls it as shipped (aligned exact 48 to 76). Left: hi(D_F8) held in v0 across the loop rotates the ring.
 * PLATEAU-HANDOFF:func_overlay_066_F0000040_18C64A8:end
 */
