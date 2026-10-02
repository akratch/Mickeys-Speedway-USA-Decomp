#include "PR/ultratypes.h"

/*
 * Overlay 44: draw the animation's two 16-bit frames as one two-tile
 * texture rectangle per TMEM-sized strip.
 *
 * Rewritten 2026-10-02 from the listing (lane w2-capbuf): libultra-style
 * gDPLoadMultiBlockS / gSPTextureRectangle packet macros on (*dl)++, the
 * strip loop as in JFG screen.c's screenDraw (a semantic relative, no code
 * adapted), the frame-source fields read through the global at each use,
 * and an unsigned stride.  304 -> 13 masked words at size delta 0.
 *
 * Open: the stride's conversion copy is a type-4 temp whose preheader web
 * ties xh and dsdx at save 1.0 and loses on web number (97/105 < 117), so
 * it takes a3 where the shipped code has it in a0 with xh in a2 and dsdx in
 * a3.  Forcing p1:w117=c3,w97=c5,w105=c6 scores 5 (schedule-only: the
 * scale *= 65536 multiply issues three slots later).
 */

typedef struct {
    u32 w0;
    u32 w1;
} Gwords;

typedef union {
    Gwords words;
    long long force_structure_alignment;
} Gfx;

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    s32 dataOffset;
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
    u8 *handles[4];
} Overlay44AnimationState;

#define _SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#ifndef NULL
#define NULL 0
#endif
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define gDma1p(pkt, c, s, l, p) {                                           \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p), 16, 8) |              \
                    _SHIFTL((l), 0, 16));                                   \
    _g->words.w1 = (u32)(s);                                                \
}
#define gSPDisplayList(pkt, dl) gDma1p(pkt, 0x06, dl, 0, 0)
#define gImmp1(pkt, c, p0) {                                                \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8);                                     \
    _g->words.w1 = (u32)(p0);                                               \
}
#define gDPNoParam(pkt, cmd) {                                              \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((cmd), 24, 8);                                   \
    _g->words.w1 = 0;                                                       \
}
#define gDPSetColor(pkt, c, d) {                                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8);                                     \
    _g->words.w1 = (u32)(d);                                                \
}
#define DPRGBColor(pkt, cmd, r, g, b, a)                                    \
    gDPSetColor(pkt, cmd, (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) |          \
                           _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8)))
#define gDPSetEnvColor(pkt, r, g, b, a) DPRGBColor(pkt, 0xFB, r, g, b, a)
#define gDPSetPrimColor(pkt, m, l, r, g, b, a) {                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xFA, 24, 8) | _SHIFTL((m), 8, 8) |             \
                    _SHIFTL((l), 0, 8));                                    \
    _g->words.w1 = (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) |                 \
                    _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8));                   \
}
#define gDPLoadSync(pkt) gDPNoParam(pkt, 0xE6)
#define gDPPipeSync(pkt) gDPNoParam(pkt, 0xE7)
#define gDPSetTextureImage(pkt, f, s, w, i) {                               \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xFD, 24, 8) | _SHIFTL((f), 21, 3) |             \
                   _SHIFTL((s), 19, 2) | _SHIFTL((w) - 1, 0, 12);           \
    _g->words.w1 = (u32)(i);                                                \
}
#define gDPSetTile(pkt, fmt, siz, line, tmem, tile, palette, cmt,           \
                   maskt, shiftt, cms, masks, shifts) {                     \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xF5, 24, 8) | _SHIFTL((fmt), 21, 3) |           \
                   _SHIFTL((siz), 19, 2) | _SHIFTL((line), 9, 9) |          \
                   _SHIFTL((tmem), 0, 9);                                   \
    _g->words.w1 = _SHIFTL((tile), 24, 3) | _SHIFTL((palette), 20, 4) |     \
                   _SHIFTL((cmt), 18, 2) | _SHIFTL((maskt), 14, 4) |        \
                   _SHIFTL((shiftt), 10, 4) | _SHIFTL((cms), 8, 2) |        \
                   _SHIFTL((masks), 4, 4) | _SHIFTL((shifts), 0, 4);        \
}
#define gDPLoadBlock(pkt, tile, uls, ult, lrs, dxt) {                       \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xF3, 24, 8) | _SHIFTL((uls), 12, 12) |         \
                    _SHIFTL((ult), 0, 12));                                 \
    _g->words.w1 = (_SHIFTL((tile), 24, 3) |                                \
                    (_SHIFTL(MIN((lrs), 0x7FF), 12, 12)) |                  \
                    _SHIFTL((dxt), 0, 12));                                 \
}
#define gDPSetTileSize(pkt, t, uls, ult, lrs, lrt) {                        \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xF2, 24, 8) | _SHIFTL((uls), 12, 12) |         \
                    _SHIFTL((ult), 0, 12));                                 \
    _g->words.w1 = (_SHIFTL((t), 24, 3) | _SHIFTL((lrs), 12, 12) |          \
                    _SHIFTL((lrt), 0, 12));                                 \
}
#define G_IM_FMT_RGBA 0
#define G_IM_SIZ_16b 2
#define G_TX_LOADTILE 7
#define G_TX_CLAMP 2
#define gDPLoadMultiBlockS(pkt, timg, tmem, rtile, fmt, siz, width, height, \
                           pal, cms, cmt, masks, maskt, shifts, shiftt) {   \
    gDPSetTextureImage(pkt, fmt, siz, 1, timg);                             \
    gDPSetTile(pkt, fmt, siz, 0, tmem, G_TX_LOADTILE, 0, cmt, maskt,        \
               shiftt, cms, masks, shifts);                                 \
    gDPLoadSync(pkt);                                                       \
    gDPLoadBlock(pkt, G_TX_LOADTILE, 0, 0, ((width) * (height)) - 1, 0);    \
    gDPPipeSync(pkt);                                                       \
    gDPSetTile(pkt, fmt, siz, ((((width) * 2) + 7) >> 3), tmem, rtile, pal, \
               cmt, maskt, shiftt, cms, masks, shifts);                     \
    gDPSetTileSize(pkt, rtile, 0, 0, ((width) - 1) << 2,                    \
                   ((height) - 1) << 2);                                    \
}
#define gSPTextureRectangle(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy) {  \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xE4, 24, 8) | _SHIFTL((xh), 12, 12) |          \
                    _SHIFTL((yh), 0, 12));                                  \
    _g->words.w1 = (_SHIFTL((tile), 24, 3) | _SHIFTL((xl), 12, 12) |        \
                    _SHIFTL((yl), 0, 12));                                  \
    gImmp1(pkt, 0xB3, (_SHIFTL((s), 16, 16) | _SHIFTL((t), 0, 16)));        \
    gImmp1(pkt, 0xB2, (_SHIFTL((dsdx), 16, 16) | _SHIFTL((dtdy), 0, 16)));  \
}

extern Overlay44FrameSource *gOverlay44FrameSources;
extern Gfx D_0[];
extern Gfx D_28[];
extern void func_overlay_044_F0000000_188B860(Gfx **dl);

#ifdef NON_MATCHING
void func_overlay_044_F0000580_188BDE0(Overlay44AnimationState *state,
                                       Gfx **dl, f32 scale) {
    s32 width;
    s32 height;
    s32 rows;
    s32 maxRows;
    s32 x;
    s32 xh;
    s32 yPrev;
    s32 y;
    s32 dsdx;
    s32 stride;
    u8 *tex0;
    u8 *tex1;
    s32 alpha;

    if (state == NULL || state->sourceIndex == -1) {
        return;
    }
    width = gOverlay44FrameSources[state->sourceIndex].dimension0;
    height = gOverlay44FrameSources[state->sourceIndex].dimension1;
    tex0 = state->handles[state->protectedSlot0];
    tex1 = state->handles[state->protectedSlot1];
    alpha = state->phase & 0xFF;
    if (scale == 1.0f) {
        gSPDisplayList((*dl)++, D_0);
    } else {
        gSPDisplayList((*dl)++, D_28);
    }
    gDPSetPrimColor((*dl)++, 0, 0, state->subtype, state->subtype,
                    state->subtype, 255);
    gDPSetEnvColor((*dl)++, alpha, alpha, alpha, alpha);
    x = state->value8 * 4;
    y = yPrev = state->valueA << 16;
    xh = (s32)(width * scale * 4.0f) + x;
    dsdx = (s32)(1024.0f / scale);
    scale *= 65536.0f;
    if (height != 0) {
        stride = (u32)width * 2;
        maxRows = (0x800 / stride) & ~1;
        do {
            if (maxRows < height) {
                rows = maxRows;
                height -= maxRows;
            } else {
                rows = height;
                height = 0;
            }
            y += (s32)(rows * scale);
            gDPLoadMultiBlockS((*dl)++, tex1, 0x100, 1, G_IM_FMT_RGBA,
                               G_IM_SIZ_16b, width, rows, 0, G_TX_CLAMP,
                               G_TX_CLAMP, 0, 0, 0, 0);
            gDPLoadMultiBlockS((*dl)++, tex0, 0, 0, G_IM_FMT_RGBA,
                               G_IM_SIZ_16b, width, rows, 0, G_TX_CLAMP,
                               G_TX_CLAMP, 0, 0, 0, 0);
            gSPTextureRectangle((*dl)++, x, yPrev >> 14, xh, y >> 14, 0, 0, 0,
                                dsdx, dsdx);
            yPrev = y;
            tex0 += rows * stride;
            tex1 += rows * stride;
        } while (height != 0);
    }
    func_overlay_044_F0000000_188B860(dl);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o044/func_overlay_044_F0000580_188BDE0/func_overlay_044_F0000580_188BDE0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_044_F0000580_188BDE0:start
 * symbol: func_overlay_044_F0000580_188BDE0
 * score: 13 differing words
 * frame: 0x100
 * relocations: 7
 * first-mismatch: +0x188
 * summary: Rewrite from listing, 304 to 13 at delta 0. Stride temp loses a0 to xh/dsdx on web number; forced colours score 5 (schedule only).
 * PLATEAU-HANDOFF:func_overlay_044_F0000580_188BDE0:end
 */
