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

/*
 * Matched 2026-10-02. The count is read from the chain each time, with no
 * local copy. The two buffer indices are unsigned and the copy count is a
 * signed multiply of the same unsigned (count - 1): the two index scalings
 * then share one constant register and multiply by it, while the copy
 * count's constant is a different type, has one use, and is expanded into
 * shifts; the shared unsigned subtraction keeps the multiply from being
 * distributed over it.
 */
void overlay17AdvanceChain(Overlay17Chain *chain, s32 useAlpha) {
    s32 n;
    s32 alpha;
    f32 x0, y0, z0, x1, y1, z1;
    u16 *src;
    u16 *dst;
    Overlay17Vertex *vtx;

    if (chain == 0) {
        return;
    }

    src = (u16 *)&chain->buffers[chain->selectedBuffer][(chain->count - 1U) << 1];
    chain->selectedBuffer ^= 1;
    dst = (u16 *)&chain->buffers[chain->selectedBuffer][(u32)chain->count << 1];
    n = (s32)(chain->count - 1U) * 10;
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
