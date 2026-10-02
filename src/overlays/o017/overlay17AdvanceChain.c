#include "PR/ultratypes.h"

typedef struct Overlay17Vertex {
    s16 x, y, z;
    u8 r, g, b, a;
} Overlay17Vertex;

typedef struct Overlay17Chain {
    s16 count;
    u8 selectedBuffer;
    u8 pad03[0x21];
    u8 red, green, blue, alpha;
    u8 pad28[4];
    Overlay17Vertex *buffers[2];
} Overlay17Chain;

extern void func_overlay_017_F0000000_18739B8(Overlay17Chain *chain,
                                               f32 *x0, f32 *y0, f32 *z0,
                                               f32 *x1, f32 *y1, f32 *z1);

/* Plateau, 2026-10-02 (lane x-ovlb): 49 -> 47 masked at delta 0. The
 * buffers are arrays of 10-byte vertices indexed by pair, instead of byte
 * offsets on halfword cursors, and both loops use one counter `n`, which is
 * the target's v0/v1 pair. Still open is the copy count: the target expands
 * (count - 1) * 10 into the counter register with shifts, while the two
 * buffer offsets share the constant 10 in t2. Every source spelling
 * measured either distributes the multiply (count * 10 - 10) or lets the
 * copy count share the constant register. See the handoff shard. */
#ifdef NON_MATCHING
void overlay17AdvanceChain(Overlay17Chain *chain, s32 useAlpha) {
    s32 count;
    s32 alpha;
    f32 x0, y0, z0, x1, y1, z1;
    u16 *src;
    u16 *dst;
    Overlay17Vertex *vtx;
    s32 n;

    if (chain == 0) {
        return;
    }

    count = chain->count;
    src = (u16 *)&chain->buffers[chain->selectedBuffer][(count - 1U) << 1];
    chain->selectedBuffer ^= 1;
    dst = (u16 *)&chain->buffers[chain->selectedBuffer][count << 1];
    n = (count - 1U) * 10;
    while (n--) {
        *--dst = *--src;
    }
    vtx = chain->buffers[chain->selectedBuffer];
    if (useAlpha) {
        alpha = chain->alpha;
    } else {
        alpha = 0;
    }
    func_overlay_017_F0000000_18739B8(chain, &x0, &y0, &z0, &x1, &y1, &z1);
    vtx[0].x = x0;
    vtx[0].y = y0;
    vtx[0].z = z0;
    vtx[0].r = chain->red;
    vtx[0].g = chain->green;
    vtx[0].b = chain->blue;
    vtx[0].a = alpha;
    vtx[1].x = x1;
    vtx[1].y = y1;
    vtx[1].z = z1;
    vtx[1].r = chain->red;
    vtx[1].g = chain->green;
    vtx[1].b = chain->blue;
    vtx[1].a = alpha;
    vtx += 2;
    n = chain->count - 1;
    while (n--) {
        if (vtx[0].a != 0) {
            alpha = (chain->alpha * n) / (chain->count - 1);
            vtx[0].a = alpha;
            vtx[1].a = alpha;
        }
        vtx += 2;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o017/overlay17AdvanceChain/func_overlay_017_F0000668_1874020.s")
#endif

/* PLATEAU-HANDOFF:overlay17AdvanceChain:start
 * symbol: overlay17AdvanceChain
 * score: 47/147 words
 * frame: 0x70
 * relocations: 1
 * first-mismatch: +0x18
 * summary: Vertex-indexed buffers, one shared loop counter: 49 to 47 unforced. Open: copy count needs a shift expansion into v0, not the t2 constant.
 * PLATEAU-HANDOFF:overlay17AdvanceChain:end
 */
