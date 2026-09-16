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

extern Overlay58LargePointGfx *gOverlay58LargePointDisplayListReloc;
extern Overlay58LargePointVertex *gOverlay58LargePointVertexCursorReloc;
extern Overlay58LargePointRenderState gOverlay58LargePointRenderStateReloc;
extern u8 D_80000098[];
/*
 * The callee is reached through the overlay loader's relocation table, so its
 * `jal` encodes 0 and splat names it after overlay offset 0 for every such
 * call in this overlay; the real callee is not identified.  Its FIRST
 * ARGUMENT is `&gOverlay58LargePointVertexCursorReloc`, the same symbol the body
 * re-reads afterwards -- p1 splits that address web around the call and the
 * post-call piece keeps the a0 argument affinity (ido-5.3 L57/L66).
 */
extern void func_overlay_058_F0000000_18AF1E8(
    Overlay58LargePointVertex **cursor, void *resource, s32 mode, s32 arg3);

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
 *  2. A zero-cost boundary after `vertices = cursor` and another after the
 *     last colour store (the two `if (vertices != 0);` probes, L97), so the
 *     0xFF constant is a single-block web (save 15, v0) that shares no
 *     block with the `dl++` web, and the four coordinate expressions are
 *     single-block webs coloured v0/v1/a1/a2 in first-occurrence order.
 *     uopt closes a straight-line block on its own after twenty loads of
 *     LOCAL variables (global loads are free), so the appends' four loads
 *     never reach that budget and the boundaries must be explicit.
 *  3. The render-state field read in its own region before the call: as1
 *     ties the two argument `lui`s at equal height and breaks the tie on
 *     source line, so the load's line must precede the cursor address's.
 *     The dead `vertices += 3` survives without any probe.
 */
void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z) {
    Overlay58LargePointVertex *vertices;
    void *resource;

    if (1) { resource = gOverlay58LargePointRenderStateReloc.resource; }
    func_overlay_058_F0000000_18AF1E8(&gOverlay58LargePointVertexCursorReloc, resource, 5, 0);
    O58_VERTEX(gOverlay58LargePointDisplayListReloc++, O58_PHYSICAL_TO_K0(gOverlay58LargePointVertexCursorReloc), 4, 0);
    O58_POLYGON(gOverlay58LargePointDisplayListReloc++, D_80000098, 2, 1);
    vertices = gOverlay58LargePointVertexCursorReloc;
    if (vertices != 0);
    vertices[1].r = 0xFF;
    vertices[1].g = 0xFF;
    vertices[1].b = 0xFF;
    vertices[1].a = 0xFF;
    vertices[2].r = 0xFF;
    vertices[2].g = 0xFF;
    vertices[2].b = 0xFF;
    vertices[2].a = 0xFF;
    vertices[3].r = 0xFF;
    vertices[3].g = 0xFF;
    vertices[3].b = 0xFF;
    vertices[3].a = 0xFF;
    vertices += 3;
    vertices[-3].r = 0xFF;
    vertices[-3].g = 0xFF;
    vertices[-3].b = 0xFF;
    vertices[-3].a = 0xFF;
    if (vertices != 0);
    gOverlay58LargePointVertexCursorReloc->x = (s16)(x - 18);
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)(z - 18);
    gOverlay58LargePointVertexCursorReloc++;
    gOverlay58LargePointVertexCursorReloc->x = (s16)(x + 18);
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)(z - 18);
    gOverlay58LargePointVertexCursorReloc++;
    gOverlay58LargePointVertexCursorReloc->x = (s16)(x - 18);
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)(z + 18);
    gOverlay58LargePointVertexCursorReloc++;
    gOverlay58LargePointVertexCursorReloc->x = (s16)(x + 18);
    gOverlay58LargePointVertexCursorReloc->y = (s16)y;
    gOverlay58LargePointVertexCursorReloc->z = (s16)(z + 18);
    gOverlay58LargePointVertexCursorReloc++;
}
