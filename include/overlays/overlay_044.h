#ifndef OVERLAY_044_H
#define OVERLAY_044_H

#include "PR/ultratypes.h"

/*
 * Overlay 44: one translation unit (ADR 0006). Three cached animation
 * states streamed from ROM into four frame handles each, and the renderer
 * that draws two 16-bit frames as texture rectangles. The release helper's
 * private view of the state is the same block: its first byte is the
 * source index, -1 when the slot is free.
 */

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
    u8 storageMode;
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

extern Overlay44AnimationState D_800D76D8[3];

extern Overlay44FrameSource *D_800D76D0;

extern void *mmAlloc(s32 size, s32 tag);

extern void overlay44ReleaseHandles(Overlay44AnimationState *state);
/* The release helper is exported through the module's runtime table, so the
 * allocator's call to it is a SYMBOL record that stores zero: it names the
 * module placeholder, not the definition, until the runlink packer writes the
 * tables (docs/OVERLAY_STRUCTURE_SPRINT.md, phase 4). */
extern void overlay44ReleaseHandles_o044Reloc(Overlay44AnimationState *state);

extern s32 piRomLoadSection(u32 assetIndex, void *address,
                                     s32 assetOffset, s32 size);

extern void viFrameRateReset(void);


void mmFree(void *handle);


typedef struct {
    u32 w0;
    u32 w1;
} Gwords;

typedef union {
    Gwords words;
    long long force_structure_alignment;
} Gfx;


#define _SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))


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

extern Gfx D_0[];
extern Gfx D_28[];
extern void texDPInit(Gfx **dl);

#endif
