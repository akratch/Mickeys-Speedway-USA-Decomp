#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58PointVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58PointVertex;

typedef struct Overlay58PointGfx {
    u32 w0;
    u32 w1;
} Overlay58PointGfx;

typedef struct Overlay58PointRenderState {
    u8 pad0[0x44];
    void *resource;
} Overlay58PointRenderState;

extern Overlay58PointGfx *gOverlay58PointDisplayListReloc;
extern Overlay58PointVertex *gOverlay58PointVertexCursorReloc;
extern Overlay58PointRenderState gOverlay58PointRenderStateReloc;
extern u8 D_80000078[];
/*
 * The callee is reached through the overlay loader's relocation table, so its
 * `jal` encodes 0 and splat names it after overlay offset 0 for every such
 * call in this overlay; the real callee is not identified.  What the target
 * bytes do decide is its FIRST ARGUMENT: see the note below.
 */
extern void func_overlay_058_F0000000_18AF1E8(
    Overlay58PointVertex **cursor, void *resource, s32 mode, s32 arg3);

/*
 * Plateau (2026-09-10): 26 of 104 relocation-masked words differ, down from 70,
 * at exact 104-instruction geometry and an exact 0x18 frame.  Two mechanisms
 * closed 44 words and both are properties of the allocator, not of spelling:
 *
 *  1. The call's first argument is `&gOverlay58PointVertexCursorReloc`, the
 *     same symbol the body re-reads twenty times afterwards -- NOT the display
 *     list.  p1 splits that one address web around the call and the post-call
 *     piece keeps the a0 argument affinity at cost 0 (ido-5.3 L57/L66), which
 *     is the only construct that puts the long-lived cursor address in a0
 *     instead of v0.  Worth 16 words by itself; all twenty `0(a0)` sites go
 *     register-exact.  1352 spellings measured against the old argument
 *     spelling never reached below 68.
 *  2. Four discarded-expression probes (ido-5.3 L37) at the positions below.
 *     They cost zero instructions and re-order p1's colouring by adding web
 *     occurrences; worth a further 26 words.  Their positions were found by a
 *     frame-constrained hill climb, 7800 variants, and are a local optimum
 *     under single insert/delete/move at 4968 further variants.
 *
 * Falsified here: declaration order is wholly inert (400 permutations, none
 * below the then-best); statement-block order, the pointer/array and cast
 * forms, `physicalBase` inlining, the second-gfx word order, and every shape
 * that drops the early `vertices` assignment all regress.
 *
 * Residual: `gfx` wants v0 and takes a2; the physical-address chain wants ring
 * temps and takes colours; `xPlus` wants a1 and takes t0; the two `lui`s at
 * +0x88 are transposed; the two `lui`s at +0x14 are transposed.
 *
 * Lane nx-a (2026-09-16): 24 -> 21 by transferring the sibling's form -- the
 * first cursor load generated inside the command expression with no declared
 * intermediate and no byte cast (see overlay58DrawLargePointQuad.c, lane g1).
 * The two siblings now carry the identical 21-row residual, window for
 * window, and the allocator records name its cause: the declared `gfx`
 * symbol survives copy propagation as a PHANTOM web (never emitted) that is
 * coloured a1, and the `if (gfx != 0)` probe extends it into the block where
 * `zPlus` is defined.  That phantom denies a1 to `xPlus`, the `0xFF` constant
 * denies v0 to the `dl++` expression web in the colour-store block, and the
 * rest of the residual follows.  See docs/lastmile-phantom-web.md.
 */
#ifdef NON_MATCHING
void overlay58DrawPointQuad(s32 x, s32 y, s32 z) {
    Overlay58PointGfx *gfx;
    Overlay58PointVertex *vertices;
    s32 physicalBase;
    s32 xPlus;
    s32 xMinus;
    s32 zMinus;
    s32 zPlus;
    if (x != 0);

    func_overlay_058_F0000000_18AF1E8(&gOverlay58PointVertexCursorReloc,
                                      gOverlay58PointRenderStateReloc.resource,
                                      5, 0);
    gfx = gOverlay58PointDisplayListReloc++;
    physicalBase = 0x80000000U;
    gfx->w0 = 0x04000000U |
              ((((((u32)gOverlay58PointVertexCursorReloc + physicalBase) & 6U) | 0x20U) & 0xFFU) << 16) |
              0x30U;
    gfx->w1 = (u32)gOverlay58PointVertexCursorReloc + physicalBase;
    if (gOverlay58PointVertexCursorReloc != 0);

    gfx = gOverlay58PointDisplayListReloc++;
    gfx->w0 = 0x05110020U; gfx->w1 = (u32)D_80000078;

    vertices = gOverlay58PointVertexCursorReloc;
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

    xPlus = x + 8;
    zPlus = z + 8;
    if (gfx != 0);
    if (vertices != 0);
    xMinus = x - 8;
    zMinus = z - 8;

    gOverlay58PointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zMinus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xMinus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58PointVertexCursorReloc++;

    gOverlay58PointVertexCursorReloc->x = (s16)xPlus;
    gOverlay58PointVertexCursorReloc->y = (s16)y;
    gOverlay58PointVertexCursorReloc->z = (s16)zPlus;
    gOverlay58PointVertexCursorReloc++;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/overlay58DrawPointQuad/func_overlay_058_F0004F28_18B4110.s")
#endif

/* PLATEAU-HANDOFF:overlay58DrawPointQuad:start
 * symbol: overlay58DrawPointQuad
 * score: 21 differing words
 * frame: 0x18
 * relocations: 11
 * first-mismatch: 0x14
 * summary: Sibling transfer 24 to 21; residual identical to overlay58DrawLargePointQuad and caused by the phantom gfx symbol web on a1 plus 0xFF interference on the dl expression web.
 * PLATEAU-HANDOFF:overlay58DrawPointQuad:end
 */
