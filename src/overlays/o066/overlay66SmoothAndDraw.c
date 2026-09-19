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

#define O66_FILTER_ONE(destination, source, leftRed, leftGreen, leftBlue, \
                       centerRed, centerGreen, centerBlue)               \
do {                                                                    \
    s32 redExtract;                                                     \
    s32 greenExtract;                                                   \
    s32 blueExtract;                                                    \
    pixel = *(source) >> 1;                                             \
    redExtract = (pixel & 0x7800) >> 8;                                \
    greenExtract = (pixel & 0x03C0) >> 3;                              \
    blueExtract = (pixel & 0x001E) << 2;                               \
    red2 = redExtract;                                                  \
    green2 = greenExtract;                                              \
    blue2 = blueExtract;                                                \
    *(destination) = ((((leftRed) + (centerRed) +                       \
                       (redExtract >> 1)) << 8) & 0xF800) |             \
                     ((((leftGreen) + (centerGreen) +                   \
                        (greenExtract >> 1)) << 3) &                    \
                      0x07C0) |                                       \
                     ((((leftBlue) + (centerBlue) +                     \
                        (blueExtract >> 1)) >> 2) & 0x003E) | 1;        \
    red0 = red1 >> 1;                                                   \
    green0 = green1 >> 1;                                               \
    blue0 = blue1 >> 1;                                                 \
    red1 = red2;                                                        \
    green1 = green2;                                                    \
    blue1 = blue2;                                                      \
} while (0)

/* PLATEAU (2026-08-26): workbench structure-mismatch; 249 masked/251 raw of 296 words, first +0x84.
 * Flag lattice confirmed the retained exact-size candidate; pointer/filter and register-form hypotheses remain eliminated.
 * Exact size/frame remain with 73 structural and 209 register differences. */
#ifdef NON_MATCHING
void func_overlay_066_F0000040_18C64A8(Overlay66Gfx **commands) {
    u16 *pixels;
    u16 *pixelBase;
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
        pixelBase = gOverlay66SharedPixels;
        remaining = 0x12BFC;
        pixels = pixelBase;
        pixels += 3;

        pixel = pixels[-3] >> 2;
        { s32 component; component = (pixel & 0x3800) >> 8; red0 = component; }
        { s32 component; component = (pixel & 0x01C0) >> 3; green0 = component; }
        { s32 component; component = (pixel & 0x000E) << 2; blue0 = component; }

        pixel = pixels[-2] >> 1;
        { s32 component; component = (pixel & 0x7800) >> 8; red1 = component; }
        { s32 component; component = (pixel & 0x03C0) >> 3; green1 = component; }
        { s32 component; component = (pixel & 0x001E) << 2; blue1 = component; }

        O66_FILTER_ONE(&pixels[-2], &pixels[-1],
                       red0, green0, blue0, red1, green1, blue1);
        O66_FILTER_ONE(&pixels[-1], &pixels[0],
                       red0, green0, blue0, red1, green1, blue1);

        do {
            O66_FILTER_ONE(&pixels[0], &pixels[1],
                           red0, green0, blue0, red1, green1, blue1);
            O66_FILTER_ONE(&pixels[1], &pixels[2],
                           red0, green0, blue0, red1, green1, blue1);
            O66_FILTER_ONE(&pixels[2], &pixels[3],
                           red0, green0, blue0, red1, green1, blue1);
            O66_FILTER_ONE(&pixels[3], &pixels[4],
                           red0, green0, blue0, red1, green1, blue1);
            pixels += 4;
            remaining -= 4;
        } while (remaining != 0);

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
 * summary: Colour floor 247 on 117 probes (w21=c13). Comma-assign, empty-if, OR-zero inert; L160 index +20. Five insertion pairs remain; not colour.
 * PLATEAU-HANDOFF:func_overlay_066_F0000040_18C64A8:end
 */
