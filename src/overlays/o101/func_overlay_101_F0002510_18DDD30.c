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
 * texDPInit at +0x344D0 past the resident base. The extracted assembly
 * shows all four as a jump to overlay offset 0 because a SYMBOL record ships
 * the 0xF0000000 addend rather than offset >> 2, so they must be routed
 * through the generated surface. The two overlay callees are ROM-exact, so
 * their prototypes are the matched ones. */
s32 overlay101GetBoundsReloc(Overlay101ClipNode *node, s32 *leftOut,
                              s32 *topOut, s32 *rightOut, s32 *bottomOut);
void overlay101SetScissorReloc(Gfx **displayList, s32 left, s32 top,
                               s32 right, s32 bottom);
void texDPInit(Gfx **displayList);

/*
 * Lanes p11-o101, x-o101, a-ovl3, k-2 and m-4 decoded the callees and wrote
 * the body with the SDK GBI macros, the y origin tested from its own local
 * and copied after the clip tests, and the bounds call's result consumed by
 * x (times zero: the call then delivers v0 into the clip block, so bottom
 * takes v0 and left s3, checklist item 38). Lane p-3 (2026-10-08) closed the
 * last 60 words: the display list read before the stride, an empty test of
 * y at the right clamp (y outranks node for s4), the row mask carried by
 * originY from right after rows (it outranks sourceX and drawWidth), the
 * scaling statements in the target's order, and each edge copied from its
 * origin and then grown, which gives the sums the target's operand order.
 */
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
    s32 edgeX;
    s32 edgeY;
    s32 originY;

    if ((node->type != 2) && (node->type != 4)) {
        return;
    }
    texture = element->texture;
    if (texture == NULL) {
        return;
    }
    x = overlay101GetBoundsReloc(node, &left, &top, &right, &bottom) * 0 + node->x + element->x;
    originY = node->y + element->y;
    y = originY;
    edgeX = x;
    edgeY = originY;
    edgeX += texture->width;
    edgeY += texture->height;
    if ((right < x) || (bottom < originY) || (edgeX < left) || (edgeY < top)) {
        return;
    }
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
    originY = rows - 1;
    gfx = *dList;
    stride = texture->width * rows;
    if (x < left) {
        drawX = left;
        sourceX = left - x;
    } else {
        drawX = x;
        sourceX = 0;
    }
    drawWidth = texture->width - sourceX;
    if ((right - drawX) < drawWidth) {
        drawWidth = right - drawX; if (y) {}
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
    drawX *= 4;
    drawY *= 4;
    drawWidth *= 4;
    sourceX <<= 5;
    sourceY = (sourceY & originY) << 5;
    gSPDisplayList(gfx++, D_230);
    gDPSetPrimColor(gfx++, 0, 0, node->intensity, node->intensity, node->intensity, node->alpha);
    while (drawHeight > 0) {
        gDPLoadTextureBlockS(gfx++, source, G_IM_FMT_RGBA, G_IM_SIZ_16b, texture->width, rows, 0,
                             G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        chunkRows = rows - (sourceY >> 5);
        if (drawHeight < chunkRows) {
            chunkRows = drawHeight;
        }
        gSPTextureRectangle(gfx++, drawX, drawY, drawX + drawWidth, drawY + chunkRows * 4, G_TX_RENDERTILE,
                            sourceX, sourceY, 1 << 10, 1 << 10);
        drawHeight -= chunkRows;
        sourceY = 0;
        drawY += chunkRows * 4;
        source += stride;
    }
    *dList = gfx;
    texDPInit(dList);
    overlay101SetScissorReloc(dList, 0, 0, 1000, 1000);
}
