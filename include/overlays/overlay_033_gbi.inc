/* Official N64 SDK GBI macro subset used by the O33 source candidate. */
#ifndef O33_GBI_SUBSET_H
#define O33_GBI_SUBSET_H

typedef union Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
    long long force_structure_alignment;
} Gfx;

#define _SHIFTL(v, s, w) \
    ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))

#define gDPNoParam(pkt, cmd) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(cmd, 24, 8); \
    _g->words.w1 = 0; \
}

#define gDPSetColor(pkt, cmd, data) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(cmd, 24, 8); \
    _g->words.w1 = (u32)(data); \
}

#define gMoveWd(pkt, index, offset, data) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xBC, 24, 8) | \
                   _SHIFTL(offset, 8, 16) | _SHIFTL(index, 0, 8); \
    _g->words.w1 = (u32)(data); \
}

#define gSPClearGeometryMode(pkt, word) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xB6, 24, 8); \
    _g->words.w1 = (u32)(word); \
}

#define gDPSetColorImage(pkt, fmt, siz, width, image) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFF, 24, 8) | _SHIFTL(fmt, 21, 3) | \
                   _SHIFTL(siz, 19, 2) | _SHIFTL((width) - 1, 0, 12); \
    _g->words.w1 = (u32)(image); \
}

#define gDPSetScissor(pkt, mode, ulx, uly, lrx, lry) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xED, 24, 8) | \
                   _SHIFTL((s32)((f32)(ulx) * 4.0F), 12, 12) | \
                   _SHIFTL((s32)((f32)(uly) * 4.0F), 0, 12); \
    _g->words.w1 = _SHIFTL(mode, 24, 2) | \
                   _SHIFTL((s32)((f32)(lrx) * 4.0F), 12, 12) | \
                   _SHIFTL((s32)((f32)(lry) * 4.0F), 0, 12); \
}

#define gDPSetFillColor(pkt, data) gDPSetColor(pkt, 0xF7, (data))

#define gDPSetCycleType(pkt, type) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xBA, 24, 8) | \
                   _SHIFTL(20, 8, 8) | _SHIFTL(2, 0, 8); \
    _g->words.w1 = (u32)(type); \
}

#define gDPFillRectangle(pkt, ulx, uly, lrx, lry) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xF6, 24, 8) | \
                   _SHIFTL(lrx, 14, 10) | _SHIFTL(lry, 2, 10); \
    _g->words.w1 = _SHIFTL(ulx, 14, 10) | _SHIFTL(uly, 2, 10); \
}

#define gDPPipeSync(pkt) gDPNoParam(pkt, 0xE7)

#define O33_RGBA(r, g, b, a) \
    (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) | \
     _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8))
#define gDPSetBlendColor(pkt, r, g, b, a) \
    gDPSetColor(pkt, 0xF9, O33_RGBA(r, g, b, a))
#define gDPSetEnvColor(pkt, r, g, b, a) \
    gDPSetColor(pkt, 0xFB, O33_RGBA(r, g, b, a))
#define gDPSetFogColor(pkt, r, g, b, a) \
    gDPSetColor(pkt, 0xF8, O33_RGBA(r, g, b, a))

#define gDPSetPrimColor(pkt, m, l, r, g, b, a) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFA, 24, 8) | \
                   _SHIFTL(m, 8, 8) | _SHIFTL(l, 0, 8); \
    _g->words.w1 = O33_RGBA(r, g, b, a); \
}

#define gDPFullSync(pkt) gDPNoParam(pkt, 0xE9)
#define gSPEndDisplayList(pkt) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xB8, 24, 8); \
    _g->words.w1 = 0; \
}

#define G_IM_FMT_RGBA 0
#define G_IM_SIZ_16b 2
#define G_CYC_FILL 0x00300000

#endif
