#include "overlays/overlay_099.h"

/*
 * Overlay 99 +0xDDC: a full-screen pass that uses the framebuffer as its
 * texture.  Each grid cell loads one tile of it and draws it through two
 * three-vertex rows of the current vertex grid.
 *
 * Matched 2026-10-02 by rewriting the inherited m2c shape from the listing:
 * one packet macro per command on (*displayList)++, the tile corners as
 * MIN/MAX expressions, the two vertex rows indexed from the grid
 * (`i * width + j`, `(i + 1) * width + j`) so uopt builds the two offsets
 * and their +20 steps, and the vertex address held in a local so the macro
 * reads the buffer table once per command.  Both loops carry their second
 * induction in the for-header (`i++, y += stepY`; `j += 2, x += stepX`),
 * which is what orders the latch loads.  The frame's last cell is one
 * unused local.  The inherited -Wo,-r4300_mul override was inert and is
 * removed.
 *
 * PROVENANCE: no code adapted.  The vertex and polygon packet encodings
 * follow Jet Force Gemini's public f3ddkr.h (gSPVertexJFG, gSPPolygon),
 * with this game's vertex-slot field in bits 9-15 of the length word.
 */
typedef struct Overlay99Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay99Vertex;

#define O99_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O99_K0_TO_PHYSICAL(x) (u32)(((char *)(x) - 0x80000000))
#define O99_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define O99_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define O99_CMD(pkt, a, b) {                                                \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (u32)(a);                                                \
    _g->words.w1 = (u32)(b);                                                \
}
#define O99_DMA1P(pkt, c, s, l, p) {                                        \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL((c), 24, 8) | O99_SHIFTL((p), 16, 8) |        \
                    O99_SHIFTL((l), 0, 16));                                \
    _g->words.w1 = (u32)(s);                                                \
}
#define gDPPipeSyncO99(pkt) O99_CMD(pkt, 0xE7000000, 0)
#define gDPLoadSyncO99(pkt) O99_CMD(pkt, 0xE6000000, 0)
#define gDPSetPrimColorO99(pkt, m, l, r, g, b, a) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xFA, 24, 8) | O99_SHIFTL((m), 8, 8) |       \
                    O99_SHIFTL((l), 0, 8));                                 \
    _g->words.w1 = (O99_SHIFTL(r, 24, 8) | O99_SHIFTL(g, 16, 8) |           \
                    O99_SHIFTL(b, 8, 8) | O99_SHIFTL(a, 0, 8));             \
}
#define gDPSetTileO99(pkt, line, tmem, tile, cmt, cms) {                    \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = O99_SHIFTL(0xF5, 24, 8) | O99_SHIFTL(0, 21, 3) |         \
                   O99_SHIFTL(2, 19, 2) | O99_SHIFTL((line), 9, 9) |        \
                   O99_SHIFTL((tmem), 0, 9);                                \
    _g->words.w1 = O99_SHIFTL((tile), 24, 3) | O99_SHIFTL((cmt), 18, 2) |   \
                   O99_SHIFTL((cms), 8, 2);                                 \
}
#define gDPLoadTileO99(pkt, tile, uls, ult, lrs, lrt) {                     \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xF4, 24, 8) | O99_SHIFTL((uls), 12, 12) |   \
                    O99_SHIFTL((ult), 0, 12));                              \
    _g->words.w1 = (O99_SHIFTL((tile), 24, 3) | O99_SHIFTL((lrs), 12, 12) | \
                    O99_SHIFTL((lrt), 0, 12));                              \
}
#define gDPSetTileSizeO99(pkt, t, uls, ult, lrs, lrt) {                     \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xF2, 24, 8) | O99_SHIFTL((uls), 12, 12) |   \
                    O99_SHIFTL((ult), 0, 12));                              \
    _g->words.w1 = (O99_SHIFTL((t), 24, 3) | O99_SHIFTL((lrs), 12, 12) |    \
                    O99_SHIFTL((lrt), 0, 12));                              \
}
#define gSPVertexO99(pkt, v, n, v0)                                         \
    O99_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1)) + 8) | ((v0) << 9),  \
              ((n) << 3) | ((u32)(v) & 6))
#define gSPPolygonO99(pkt, ptr, numTris, texEnabled) {                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = O99_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | \
                   O99_SHIFTL(5, 24, 8) | O99_SHIFTL(((numTris) * 16), 0, 16); \
    _g->words.w1 = (u32)(ptr);                                              \
}

extern void *gO99BlurSourceReloc;
extern s32 gO99VertexBufferIndexReloc;
extern Overlay99Vertex *gO99VertexBuffersReloc[];
extern u16 *gO99FramebufferReloc;
extern u8 gO99TrianglesReloc[];
extern void overlay99CamStandardPerspReloc(Gfx **displayList, Mtx **matrices);
extern void overlay99Func80034920Reloc(Gfx **displayList);
extern void overlay99RenderSegmentsReloc(Gfx **displayList, Mtx **matrices,
                                         void *vertices, f32 scale);

void func_overlay_099_F0000DDC_18DA38C(Gfx **displayList, Mtx **matrices,
                                       void *vertices, f32 scale, s32 columns,
                                       s32 rows, s32 triangles, s32 stepX,
                                       s32 stepY) {
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 uls;
    s32 ult;
    s32 lrs;
    s32 lrt;
    s32 width;
    s32 alpha;
    Overlay99Vertex *vtx;
    s32 unused;

    if (gO99BlurSourceReloc == NULL) {
        return;
    }
    width = columns + 1;
    alpha = (s32)((1.0f - scale) * 768.0f);
    if (alpha >= 0x100) {
        alpha = 0xFF;
    }
    overlay99CamStandardPerspReloc(displayList, matrices);
    gDPPipeSyncO99((*displayList)++);
    O99_CMD((*displayList)++, 0xED000000, 0x005003C0);
    O99_CMD((*displayList)++, 0xEF30000F, 0);
    O99_CMD((*displayList)++, 0xF7000000, 0x00010001);
    O99_CMD((*displayList)++, 0xF64FC3BC, 0);
    gDPPipeSyncO99((*displayList)++);
    O99_CMD((*displayList)++, 0xFB000000, 0xFFFFFF00);
    gDPSetPrimColorO99((*displayList)++, 0, 0, alpha, alpha, alpha, 255);
    O99_CMD((*displayList)++, 0xB6000000, 0x00010001);
    O99_CMD((*displayList)++, 0xFC45FE03, 0x1FFCFDFE);
    O99_CMD((*displayList)++, 0xEF182C0F, 0x0F0A4000);
    O99_CMD((*displayList)++, 0xFD10013F, gO99FramebufferReloc);
    for (i = 0, y = 0; i < rows; i++, y += stepY) {
        for (j = 0, x = 0; j < columns; j += 2, x += stepX) {
            uls = O99_MAX(x - 1, 0);
            ult = O99_MAX(y - 1, 0);
            lrs = O99_MIN(x + stepX, 319);
            lrt = O99_MIN(y + stepY, 239);
            gDPSetTileO99((*displayList)++, ((((lrs - uls) + 1) * 2) + 7) >> 3,
                          0, 7, 2, 2);
            gDPLoadSyncO99((*displayList)++);
            gDPLoadTileO99((*displayList)++, 7, uls << 2, ult << 2, lrs << 2,
                           lrt << 2);
            gDPPipeSyncO99((*displayList)++);
            gDPSetTileO99((*displayList)++, ((((lrs - uls) + 1) * 2) + 7) >> 3,
                          0, 0, 2, 2);
            gDPSetTileSizeO99((*displayList)++, 0, 0, 0,
                              ((lrs - uls) - 1) << 2, ((lrt - ult) - 1) << 2);
            vtx = &gO99VertexBuffersReloc[gO99VertexBufferIndexReloc][i * width + j];
            gSPVertexO99((*displayList)++, O99_K0_TO_PHYSICAL(vtx), 3, 0);
            vtx = &gO99VertexBuffersReloc[gO99VertexBufferIndexReloc][(i + 1) * width + j];
            gSPVertexO99((*displayList)++, O99_K0_TO_PHYSICAL(vtx), 3, 3);
            gSPPolygonO99((*displayList)++, O99_K0_TO_PHYSICAL(gO99TrianglesReloc),
                          triangles, 1);
        }
    }
    overlay99Func80034920Reloc(displayList);
    O99_CMD((*displayList)++, 0xFA000000, 0xFFFFFFFF);
    overlay99RenderSegmentsReloc(displayList, matrices, vertices, scale);
}
