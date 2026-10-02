#include "PR/ultratypes.h"

/*
 * Overlay 43 ("clone.c"): render each queued clone into its own 64x64 8-bit
 * intensity image.  The two trace calls carry the source file name and the
 * original line numbers 596 and 675.
 *
 * Matched 2026-10-02 by rewriting the inherited m2c shape from the listing:
 * one GBI packet macro per command on the address-taken list cursor, a
 * counted loop over the entries indexed from the top, the K0-to-physical
 * conversion as libultra's subtraction, and the per-file -mips1 override
 * removed (the target fills no load-delay slots, so it is MIPS II).  The
 * last ring draw was the display-list start: the target holds it in a
 * register local (`start`) and copies it into the cursor, so the trace
 * call's argument draws nothing.  The frame's two extra cells are `start`
 * and one unused local below the matrices.
 *
 * PROVENANCE: no code adapted.  The GBI macro bodies follow the published
 * libultra gbi.h encodings (F3D movewords as gImmp21) that the shipped words
 * decode to.
 */

typedef struct {
    u32 w0;
    u32 w1;
} Gwords;

typedef union {
    Gwords words;
    long long force_structure_alignment;
} Gfx;

typedef f32 MtxF[4][4];

typedef struct {
    long m[4][4];
} Mtx;

#define _SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define OS_K0_TO_PHYSICAL(x) (u32)(((char *)(x) - 0x80000000))

#define gDma0p(pkt, c, s, l) {                                              \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8) | _SHIFTL((l), 0, 24);               \
    _g->words.w1 = (u32)(s);                                                \
}
#define gDma1p(pkt, c, s, l, p) {                                           \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p), 16, 8) |              \
                    _SHIFTL((l), 0, 16));                                   \
    _g->words.w1 = (u32)(s);                                                \
}
#define gImmp1(pkt, c, p0) {                                                \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8);                                     \
    _g->words.w1 = (u32)(p0);                                               \
}
#define gImmp21(pkt, c, p0, p1, dat) {                                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p0), 8, 16) |             \
                    _SHIFTL((p1), 0, 8));                                   \
    _g->words.w1 = (u32)(dat);                                              \
}
#define gMoveWd(pkt, index, offset, data) \
    gImmp21((pkt), 0xBC, (offset), (index), (data))
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
#define gDPSetFillColor(pkt, d) gDPSetColor(pkt, 0xF7, d)
#define gSPSetOtherMode(pkt, cmd, sft, len, data) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((cmd), 24, 8) | _SHIFTL((sft), 8, 8) |          \
                    _SHIFTL((len), 0, 8));                                  \
    _g->words.w1 = (u32)(data);                                             \
}
#define G_CYC_FILL 0x00300000
#define gDPSetCycleType(pkt, type) gSPSetOtherMode(pkt, 0xBA, 20, 2, type)
#define G_IM_FMT_I 4
#define G_IM_SIZ_8b 1
#define gDPSetColorImage(pkt, fmt, siz, width, i) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xFF, 24, 8) | _SHIFTL((fmt), 21, 3) |           \
                   _SHIFTL((siz), 19, 2) | _SHIFTL((width) - 1, 0, 12);     \
    _g->words.w1 = (u32)(i);                                                \
}
#define G_SC_NON_INTERLACE 0
#define gDPSetScissor(pkt, mode, ulx, uly, lrx, lry) {                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xED, 24, 8) |                                   \
                   _SHIFTL((int)((float)(ulx) * 4.0F), 12, 12) |            \
                   _SHIFTL((int)((float)(uly) * 4.0F), 0, 12);              \
    _g->words.w1 = _SHIFTL((mode), 24, 2) |                                 \
                   _SHIFTL((int)((float)(lrx) * 4.0F), 12, 12) |            \
                   _SHIFTL((int)((float)(lry) * 4.0F), 0, 12);              \
}
#define gDPFillRectangle(pkt, ulx, uly, lrx, lry) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xF6, 24, 8) | _SHIFTL((lrx), 14, 10) |         \
                    _SHIFTL((lry), 2, 10));                                 \
    _g->words.w1 = (_SHIFTL((ulx), 14, 10) | _SHIFTL((uly), 2, 10));        \
}
#define gSPClearGeometryMode(pkt, word) gImmp1(pkt, 0xB6, word)
#define gSPTexture(pkt, s, t, level, tile, on) {                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xBB, 24, 8) | _SHIFTL((level), 11, 3) |        \
                    _SHIFTL((tile), 8, 3) | _SHIFTL((on), 0, 8));           \
    _g->words.w1 = (_SHIFTL((s), 16, 16) | _SHIFTL((t), 0, 16));            \
}
#define gSPViewport(pkt, v) gDma1p((pkt), 0x03, (v), 16, 0x80)
#define gSPMatrix(pkt, m, p) gDma1p((pkt), 0x01, (m), 64, (p))
#define gSPDisplayList(pkt, dl) gDma1p(pkt, 0x06, dl, 0, 0)
#define gDPFullSync(pkt) gDPNoParam(pkt, 0xE9)
#define gSPEndDisplayList(pkt) gDPNoParam(pkt, 0xB8)
#define FRUSTRATIO_2 2
#define gSPClipRatio(pkt, r) {                                              \
    gMoveWd(pkt, 0x04, 0x04, (u32)(r) & 0xFFFF);                            \
    gMoveWd(pkt, 0x04, 0x0C, (u32)(r) & 0xFFFF);                            \
    gMoveWd(pkt, 0x04, 0x14, (u32)(-(r)) & 0xFFFF);                         \
    gMoveWd(pkt, 0x04, 0x1C, (u32)(-(r)) & 0xFFFF);                         \
}

typedef struct Overlay43Model {
    u8 pad00[0x0A];
    s16 partIndex;
    void *parts[1];
} Overlay43Model;

typedef struct Overlay43VertexData {
    u8 pad00[4];
    void *vertices;
} Overlay43VertexData;

typedef struct Overlay43RenderState {
    u8 pad00[0x20];
    void *framebuffer;
    u8 pad24[4];
    Gfx *displayList;
    Mtx *matrixHeap;
    Gfx *displayListEnd;
    void *modelDisplayList;
    u8 pad38[0x28];
    Overlay43Model *model;
    Overlay43VertexData *vertexData;
    u8 pad68[4];
    MtxF transform;
    u8 padAC[0xD];
    u8 pending;
} Overlay43RenderState;

typedef struct Overlay43RenderLink {
    u8 pad00[0x1C];
    Overlay43RenderState *state;
} Overlay43RenderLink;

typedef struct Overlay43RenderInput {
    u8 pad00[0x4C];
    Overlay43RenderLink *link;
} Overlay43RenderInput;

typedef struct Overlay43RenderEntry {
    MtxF matrix;
    u8 pad40[4];
    s32 alpha;
} Overlay43RenderEntry;

/* Resident callees: diRcpTrace (func_80044BC8), rsp_segment,
 * rcpInitDpNoSize, mtxf_mul and mtxf_to_mtx. */
extern void overlay43RcpTraceReloc(Gfx *dl, char *file, s32 line);
extern void overlay43RspSegmentReloc(Gfx **dl, s32 segment, void *base);
extern void overlay43InitDpReloc(Gfx **dl);
extern void overlay43MtxMulReloc(MtxF a, MtxF b, MtxF out);
extern void overlay43MtxToMtxReloc(MtxF src, Mtx *dest);

/* Module-relative bases: .rodata (the two "clone.c" strings), .data (the
 * two fixed matrices at +0x38/+0x78 and the viewport at +0xB8) and .bss
 * (the queue at +0x120).  The queue count at .data+0xC8 is addressed as
 * its own symbol: the target materialises its full address once and
 * reads and writes it at displacement 0.  Two resident data words are
 * reached through the reserved data selector. */
extern char gO43RodataBaseReloc[];
extern u8 gO43DataBaseReloc[];
extern u8 gO43BssBaseReloc[];
extern u8 gO43VertexSourceReloc[];
extern s32 gO43RenderPendingReloc;
extern s8 gO43QueueCountReloc;

#define gO43TraceFile0 (gO43RodataBaseReloc)
#define gO43TraceFile1 (gO43RodataBaseReloc + 0x10)
#define gO43CloneMatrixA (*(MtxF *)(gO43DataBaseReloc + 0x38))
#define gO43CloneMatrixB (*(MtxF *)(gO43DataBaseReloc + 0x78))
#define gO43Viewport (gO43DataBaseReloc + 0xB8)
#define gO43QueueCount gO43QueueCountReloc
#define gO43Queue ((Overlay43RenderState **)(gO43BssBaseReloc + 0x120))

void func_overlay_043_F0000BE4_188ABB4(Overlay43RenderInput *input,
                                       Overlay43RenderEntry **entries,
                                       s32 count) {
    Gfx *dl;
    Overlay43Model *model;
    Mtx *mtx;
    s32 i;
    s32 alpha;
    Overlay43VertexData *vertexData;
    Overlay43RenderState *state;
    MtxF matA;
    MtxF matB;
    Gfx *start;
    s32 unused;

    state = input->link->state;
    model = state->model;
    vertexData = state->vertexData;
    if (gO43QueueCount < 15) {
        start = state->displayList;
        mtx = state->matrixHeap;
        dl = start;
        overlay43RcpTraceReloc(start, gO43TraceFile0, 596);
        overlay43RspSegmentReloc(&dl, 0, 0);
        overlay43RspSegmentReloc(&dl, 1, state->framebuffer);
        gDPSetCycleType(dl++, G_CYC_FILL);
        gDPSetColorImage(dl++, G_IM_FMT_I, G_IM_SIZ_8b, 64,
                         OS_K0_TO_PHYSICAL(state->framebuffer));
        gDPSetFillColor(dl++, 0);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, 64, 64);
        gDPFillRectangle(dl++, 0, 0, 63, 63);
        overlay43InitDpReloc(&dl);
        gSPClearGeometryMode(dl++, 0x1F3204);
        gSPTexture(dl++, 0, 0, 0, 0, 0);
        gMoveWd(dl++, 0x02, 0, 0);
        gSPViewport(dl++, OS_K0_TO_PHYSICAL(gO43Viewport));
        gSPClipRatio(dl++, FRUSTRATIO_2);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 1, 1, 63, 63);
        for (i = count - 1; i >= 0; i--) {
            alpha = entries[i]->alpha;
            gDPSetPrimColor(dl++, 0, 0, alpha, alpha, alpha, 255);
            gDPSetEnvColor(dl++, alpha, alpha, alpha, 255);
            overlay43MtxMulReloc(state->transform, entries[i]->matrix, matA);
            overlay43MtxMulReloc(matA, gO43CloneMatrixB, matA);
            overlay43MtxMulReloc(matA, gO43CloneMatrixA, matB);
            overlay43MtxToMtxReloc(matB, mtx);
            gSPMatrix(dl++, OS_K0_TO_PHYSICAL(mtx), 0);
            {
                Gfx *_g = (Gfx *)(dl++);
                _g->words.w0 = _SHIFTL(0xBF, 24, 8) |
                    _SHIFTL(OS_K0_TO_PHYSICAL(model->parts[model->partIndex]),
                            0, 24);
                _g->words.w1 = OS_K0_TO_PHYSICAL(vertexData->vertices);
            }
            gDma0p(dl++, 0x02, OS_K0_TO_PHYSICAL(gO43VertexSourceReloc), 0x50);
            gSPDisplayList(dl++, OS_K0_TO_PHYSICAL(state->modelDisplayList));
            gImmp1(dl++, 0xBF, 0);
            mtx++;
        }
        gMoveWd(dl++, 0x0A, 0, 0);
        gDPFullSync(dl++);
        gSPEndDisplayList(dl++);
        overlay43RcpTraceReloc(dl, gO43TraceFile1, 675);
        state->displayListEnd = dl;
        state->pending = 1;
        gO43RenderPendingReloc = 1;
        gO43Queue[gO43QueueCount] = state;
        gO43QueueCount++;
    }
}
