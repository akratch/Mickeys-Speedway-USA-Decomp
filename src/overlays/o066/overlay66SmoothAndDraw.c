#include "PR/ultratypes.h"

typedef struct Overlay66Gfx Overlay66Gfx;

extern s32 gOverlay66Timer;
extern s32 gOverlay66Flag;
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
void osInvalDCache(void *vaddr, s32 nbytes);
void osWritebackDCacheAll(void);

/* Matched (lane i-7, 2026-10-07). The pixel local is a u16: as an s32 its
 * web outranks every loop value and takes v0, which keeps the timer's
 * address out of v0; as a u16 it ranks below the channel webs, v0 stays
 * free through the loop and as1 lifts the address's lui into the preheader
 * as shipped. The cursor is the output pixel (base + 1), and the countdown
 * is a while loop decremented at the end of the body, which gives the
 * unrolled copies the shipped schedule. */
void func_overlay_066_F0000040_18C64A8(Overlay66Gfx **commands) {
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
        func_overlay_066_F00004E0_18C6948(commands, gOverlay66FirstPrimary,
                                          gOverlay66SharedFirst);
        gOverlay66BssControl = 1;
    } else if (gOverlay66BssControl > 0) {
        gOverlay66BssControl--;
    } else if (gOverlay66Timer > 0) {
        osInvalDCache(gOverlay66SharedMutation, 0x25800);
        /* One rolled 76798-step filter. IDO unrolls it itself: the two
         * remainder steps come first, then the 4x body counting 76796 down,
         * which is the shipped shape. Bare statements, not a do-while(0)
         * macro, or the unroller declines (the call after the loop). */
        pixels = gOverlay66SharedPixels + 1;
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
    func_overlay_066_F00004E0_18C6948(commands, gOverlay66SharedFinal,
                                      gOverlay66FinalSecondary);
}
