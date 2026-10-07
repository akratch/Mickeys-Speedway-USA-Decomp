#include "PR/ultratypes.h"
/* The SDK GBI header takes _SHIFTL/_SHIFTR from mbi.h; spelled as the SDK does. */
#define _SHIFTL(v, s, w) \
    ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define _SHIFTR(v, s, w) \
    ((unsigned int)(((unsigned int)(v) >> (s)) & ((0x01 << (w)) - 1)))
#include "n_audio/gbi.h"

typedef struct Overlay101ClipNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 intensity;
    u8 alpha;
} Overlay101ClipNode;

typedef struct Overlay101Texture {
    s16 width;
    s16 height;
    u8 pad04[0xC];
    u16 pixels[1];
} Overlay101Texture;

typedef struct Overlay101TextureElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[4];
    Overlay101Texture *texture;
} Overlay101TextureElement;

extern Gfx D_230[];

/* Tier B: this function's four runtime R_MIPS_26 records are all SYMBOL
 * operations. Two name overlay 101 +0x1F80 (overlay101SetScissor), one names
 * overlay 101 +0x2118 (overlay101GetBounds) and one names the resident
 * func_80034920 at +0x344D0 past the resident base. The extracted assembly
 * shows all four as a jump to overlay offset 0 because a SYMBOL record ships
 * the 0xF0000000 addend rather than offset >> 2, so they must be routed
 * through the generated surface. The two overlay callees are ROM-exact, so
 * their prototypes are the matched ones. */
void overlay101GetBoundsReloc(Overlay101ClipNode *node, s32 *leftOut,
                              s32 *topOut, s32 *rightOut, s32 *bottomOut);
void overlay101SetScissorReloc(Gfx **displayList, s32 left, s32 top,
                               s32 right, s32 bottom);
void func_80034920(Gfx **displayList);

/*
 * NON_MATCHING reconstruction. Lane p11-o101 (2026-09-12) decoded the four
 * callees; lane x-o101 (2026-10-02) rewrote the display-list body with the
 * SDK's own GBI macros (gSPDisplayList, gDPSetPrimColor,
 * gDPLoadTextureBlockS, gSPTextureRectangle -- every command word, mask and
 * the MIN() branch of the target is that macro set's expansion, including the
 * LoadBlock dxt of 0 that only the S variant has), the texture rows as a
 * u16 pointer stepped by the stride, and the edge sums as locals ahead of
 * the clip chain. 291 -> 263 masked, size delta +8 -> -8, frame 0xF8 -> 0xE8
 * (exact): every declared local takes a frame cell top-down in declaration
 * order here, so eleven locals precede the four bounds (target homes 0xB8..
 * 0xAC with the stride spill at 0xA8 below them) and dropping the nextY
 * carrier removed the one cell too many. Early returns for the type, null
 * and clip tests, and the 0x800 / width quotient held in `rows` itself (the
 * target's s7), took it to 257 at -8 with 228 of the aligned words exact.
 * Lane a-ovl3 (2026-10-07): the clip tests read the y origin from its own
 * local and `y` is copied from it after the tests, which is the target's
 * split of y (v1 for the tests, a callee-saved copy across the scissor
 * call); 257 at -8 -> 90 at size delta 0. What is left is allocation and
 * the clip block's schedule: the target computes the bottom edge before the
 * tests and holds left/top in s3/s1 for the scissor call.
 */
#ifdef NON_MATCHING
void func_overlay_101_F0002510_18DDD30(Gfx **dList, Overlay101ClipNode *node,
                                       Overlay101TextureElement *element) {
    Overlay101Texture *texture;
    Gfx *gfx;
    u16 *source;
    s32 x;
    s32 y;
    s32 shift;
    s32 rows;
    s32 sourceX;
    s32 sourceY;
    s32 drawX;
    s32 drawY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 stride;
    s32 drawWidth;
    s32 drawHeight;
    s32 chunkRows;
    s32 rowOffset;
    s32 edgeX;
    s32 originY;

    if ((node->type != 2) && (node->type != 4)) {
        return;
    }
    texture = element->texture;
    if (texture == NULL) {
        return;
    }
    overlay101GetBoundsReloc(node, &left, &top, &right, &bottom);
    x = node->x + element->x;
    originY = node->y + element->y;
    edgeX = x + texture->width;
    if ((right < x) || (bottom < originY) || (edgeX < left) ||
        (originY + texture->height < top)) {
        return;
    }
    y = originY;
    overlay101SetScissorReloc(dList, left, top, right, bottom);
    rows = 0x800 / texture->width;
    if (rows >= 8) {
        shift = 3;
    } else if (rows >= 4) {
        shift = 2;
    } else {
        shift = 1;
    }
    rows = 1 << shift;
    stride = texture->width * rows;
    gfx = *dList;
    if (x < left) {
        drawX = left;
        sourceX = left - x;
    } else {
        drawX = x;
        sourceX = 0;
    }
    drawWidth = texture->width - sourceX;
    if ((right - drawX) < drawWidth) {
        drawWidth = right - drawX;
    }
    if (y < top) {
        drawY = top;
        sourceY = top - y;
    } else {
        drawY = y;
        sourceY = 0;
    }
    drawHeight = texture->height - sourceY;
    if ((bottom - drawY) < drawHeight) {
        drawHeight = bottom - drawY;
    }
    source = &texture->pixels[stride * (sourceY >> shift)];
    rowOffset = (sourceY & (rows - 1)) << 5;
    drawY *= 4;
    drawX *= 4;
    drawWidth *= 4;
    sourceX <<= 5;
    gSPDisplayList(gfx++, D_230);
    gDPSetPrimColor(gfx++, 0, 0, node->intensity, node->intensity, node->intensity, node->alpha);
    while (drawHeight > 0) {
        gDPLoadTextureBlockS(gfx++, source, G_IM_FMT_RGBA, G_IM_SIZ_16b, texture->width, rows, 0,
                             G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        chunkRows = rows - (rowOffset >> 5);
        if (drawHeight < chunkRows) {
            chunkRows = drawHeight;
        }
        gSPTextureRectangle(gfx++, drawX, drawY, drawX + drawWidth, drawY + chunkRows * 4, G_TX_RENDERTILE,
                            sourceX, rowOffset, 1 << 10, 1 << 10);
        drawHeight -= chunkRows;
        rowOffset = 0;
        drawY += chunkRows * 4;
        source += stride;
    }
    *dList = gfx;
    func_80034920(dList);
    overlay101SetScissorReloc(dList, 0, 0, 1000, 1000);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F0002510_18DDD30/func_overlay_101_F0002510_18DDD30.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_101_F0002510_18DDD30:start
 * symbol: func_overlay_101_F0002510_18DDD30
 * score: 90/293 words
 * frame: 0xE8
 * relocations: 6
 * first-mismatch: +0x70
 * summary: y origin local tested, y copied after the clip tests (target's v1/s4 split): delta 0. Left: clip-block schedule, left/top in s3/s1.
 * PLATEAU-HANDOFF:func_overlay_101_F0002510_18DDD30:end
 */
