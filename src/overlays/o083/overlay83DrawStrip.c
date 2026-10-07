#include "PR/ultratypes.h"

typedef struct Overlay83Command { u32 w0; u32 w1; } Overlay83Command;
typedef struct Overlay83Strip {
    u8 pad00[0x20];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad23[0xB];
    u8 count;
    u8 vertexIndex;
    u8 pad30[0x84];
} Overlay83Strip;

extern u8 D_80000000[];

/*
 * PROVENANCE: the packet macros follow Jet Force Gemini's published
 * decompilation (include/PR/gbi.h _SHIFTL, gDma1p, gDPSetPrimColor and
 * DPRGBColor; include/f3ddkr.h gSPVertexJFG and gSPPolygon), adapted to
 * this file's command type.
 */
#define _SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define gDma1p(pkt, c, s, l, p) { \
    Overlay83Command *_g = (Overlay83Command *)(pkt); \
    _g->w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p), 16, 8) | _SHIFTL((l), 0, 16)); \
    _g->w1 = (unsigned int)(s); \
}
#define gSPVertexJFG(pkt, v, n, v0) \
    gDma1p(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define gSPPolygon(dl, ptr, numTris, texEnabled) { \
    Overlay83Command *_g = (Overlay83Command *)(dl); \
    _g->w0 = _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(5, 24, 8) | \
             _SHIFTL(((numTris)*16), 0, 16); \
    _g->w1 = (unsigned int)(ptr); \
}
#define gDPSetPrimColor(pkt, m, l, r, g, b, a) { \
    Overlay83Command *_g = (Overlay83Command *)(pkt); \
    _g->w0 = (_SHIFTL(0xFA, 24, 8) | _SHIFTL(m, 8, 8) | _SHIFTL(l, 0, 8)); \
    _g->w1 = (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) | _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8)); \
}
#define gDPSetEnvColor(pkt, r, g, b, a) { \
    Overlay83Command *_g = (Overlay83Command *)(pkt); \
    _g->w0 = _SHIFTL(0xFB, 24, 8); \
    _g->w1 = (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) | _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8)); \
}

/*
 * Matched 2026-10-07 (lane a-ovl1). One packet macro per command, written
 * with the SDK/JFG shapes above. Two edits closed the old 58-word residual:
 * the colour words packed through _SHIFTL, and the vertex address written
 * as byte arithmetic on the strip base (index times record size first), which
 * gives the target's base-first add at both evaluations. The saved
 * display-list pointer is taken after the first packet; taken before it, the
 * parameter keeps a2 instead of the target's a3.
 */
void overlay83DrawStrip(Overlay83Command **displayList, Overlay83Strip *strip) {
    Overlay83Command **savedDisplayList;
    s32 count;
    s32 doubledCount;
    s32 vertexCount;

    count = strip->count;
    if (count != 0) {
        doubledCount = count * 2;
        vertexCount = doubledCount + 2;
        gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);
        savedDisplayList = displayList;
        gDPSetEnvColor((*displayList)++, strip->red, strip->green, strip->blue, 255);
        gSPVertexJFG((*displayList)++,
                     (u8 *)strip + strip->vertexIndex * sizeof(Overlay83Strip) + 0x800000F0,
                     vertexCount, 0);
        gSPPolygon((*savedDisplayList)++, D_80000000, doubledCount, 1);
    }
}
