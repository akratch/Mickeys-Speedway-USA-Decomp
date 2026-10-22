#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58LargePointVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58LargePointVertex;

typedef struct Overlay58LargePointGfx {
    u32 w0;
    u32 w1;
} Overlay58LargePointGfx;

typedef struct Overlay58LargePointRenderState {
    u8 pad0[0x1E8];
    void *resource;
} Overlay58LargePointRenderState;

extern Overlay58LargePointGfx *D_800D3140;
extern Overlay58LargePointVertex *D_800D3148;
extern Overlay58LargePointRenderState D_800D31C8;
extern u8 D_80000098[];
/*
 * The shipped table-1 SYMBOL record names the resident texDPTextureX (its
 * `jal` stores 0, the runtime patches it).  Its first argument is the display
 * list head, the same identity as D_800D3140 below.
 */
extern void texDPTextureX(Overlay58LargePointGfx **displayList, void *texture,
                          s32 flags, s32 arg3);

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDma1p and include/PR/mbi.h _SHIFTL,
 * include/f3ddkr.h gSPVertexJFG and gSPPolygon, include/PR/os_convert.h
 * OS_PHYSICAL_TO_K0), a permitted source under docs/CLEANROOM.md.  The two
 * commands are gSPVertexJFG(dl++, OS_PHYSICAL_TO_K0(cursor), 4, 0) and
 * gSPPolygon(dl++, payload, 2, TRIN_ENABLE_TEXTURE): the vertex command's
 * `v` argument is used twice, which is why the cursor is loaded twice, and
 * each macro is one physical line, which is what retires the store-order
 * tie at +0x88.  Each macro's block-scoped `_g` is a phantom symbol web
 * (decided after the shared `dl++` expression web once both appends sit in
 * one block, which is what gives that web v0).
 */
#define O58_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O58_DMA1P(pkt, c, s, l, p) { Overlay58LargePointGfx *_g = (Overlay58LargePointGfx *)(pkt); _g->w0 = (O58_SHIFTL((c), 24, 8) | O58_SHIFTL((p), 16, 8) | O58_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O58_VERTEX(pkt, v, n, v0) O58_DMA1P(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O58_POLYGON(dl, ptr, numTris, texEnabled) { Overlay58LargePointGfx *_g = (Overlay58LargePointGfx *)(dl); _g->w0 = O58_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O58_SHIFTL(5, 24, 8) | O58_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O58_PHYSICAL_TO_K0(x) (void *)(((u32)(x)+0x80000000))

/*
 * Matched by lane w1-b (2026-09-16) from the 21-word register-only plateau;
 * see docs/lastmile-block-budget-globals.md.  Three things had to hold at
 * once, all of them block structure rather than colour:
 *
 *  1. Both packet appends in ONE uopt block, and `vertices = cursor` in that
 *     same block: the `dl++` expression web then has one occurrence set
 *     (save 6.0) and is coloured v0 before the two `_g` phantoms (3.0 each,
 *     a1/a2), and `vertices` spanning that block and the colour block keeps
 *     v1 busy there so the display-list address falls to a3.
 *  2. The colour fill written as the four-vertex loop it is.  IDO unrolls
 *     the constant-count loop (vertices 1-3 at positive offsets, the cursor
 *     advanced by three, vertex 0 at -3), and the loop's entry and exit
 *     are the block boundaries the match needs: the 0xFF constant is a
 *     single-block web (save 15, v0) that shares no block with the `dl++`
 *     web, and the four coordinate expressions are single-block webs
 *     coloured v0/v1/a1/a2 in first-occurrence order.  An earlier
 *     straight-line spelling needed two empty `if (vertices != 0);`
 *     probes to cut those blocks (lane c-1, 2026-10-08, byte-identical).
 *  3. The call in its own `if (1)` region, arguments in source order.
 *     Spelled plainly the display-list `lui` is scheduled first (4 words
 *     differ).  Reading the render-state field in a region of its own
 *     before the call also matches the bytes, but it reverses the two
 *     HI16/LO16 pairs in the relocation table against the shipped order
 *     (display list first), so the region holds the whole call instead.
 */
void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z) {
    Overlay58LargePointVertex *vertices;
    s32 i;

    if (1) { texDPTextureX(&D_800D3140, D_800D31C8.resource, 5, 0); }
    O58_VERTEX(D_800D3140++, O58_PHYSICAL_TO_K0(D_800D3148), 4, 0);
    O58_POLYGON(D_800D3140++, D_80000098, 2, 1);
    vertices = D_800D3148;
    for (i = 0; i < 4; i++) {
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
    }
    D_800D3148->x = (s16)(x - 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x - 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 18);
    D_800D3148++;
}
