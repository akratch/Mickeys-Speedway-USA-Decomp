#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

extern Gfx D_0[];
extern Gfx D_50[];
extern Gfx D_88[];

extern s32 viGetVideoMode(void);
extern void texDPInit(Gfx **displayList);
extern void rsp_segment(Gfx **displayList, s32 segment, void *base);
extern void viGetCurrentSize(u32 *width, u32 *height);

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
void func_overlay_066_F00004E0_18C6948(Gfx **displayList, u16 *framebuffer,
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
