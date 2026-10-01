#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58StripVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58StripVertex;

typedef struct Overlay58StripGfx {
    u32 w0;
    u32 w1;
} Overlay58StripGfx;

extern Overlay58StripGfx *gOverlay58StripDisplayListReloc;
extern Overlay58StripVertex *gOverlay58StripVertexCursorReloc;
extern u8 D_80000058[];
extern f32 overlay58SqrtReloc(f32 value);
extern void overlay58PrepareStripReloc(Overlay58StripGfx **displayList,
                                       void *resource, s32 mode, s32 arg3);

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDma1p and include/PR/mbi.h _SHIFTL,
 * include/f3ddkr.h gSPVertexJFG and gSPPolygon, include/PR/os_convert.h
 * OS_PHYSICAL_TO_K0), a permitted source under docs/CLEANROOM.md, exactly as
 * in this overlay's two point-quad routines.
 */
#define O58_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O58_DMA1P(pkt, c, s, l, p) { Overlay58StripGfx *_g = (Overlay58StripGfx *)(pkt); _g->w0 = (O58_SHIFTL((c), 24, 8) | O58_SHIFTL((p), 16, 8) | O58_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O58_VERTEX(pkt, v, n, v0) O58_DMA1P(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O58_POLYGON(dl, ptr, numTris, texEnabled) { Overlay58StripGfx *_g = (Overlay58StripGfx *)(dl); _g->w0 = O58_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O58_SHIFTL(5, 24, 8) | O58_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O58_PHYSICAL_TO_K0(x) (void *)(((u32)(x)+0x80000000))

/*
 * Matched 2026-10-01 by shape, from a 68-word plateau whose named blocker was
 * the ranking of the 0xFF constant against the vertex cursor's address.  The
 * four vertex colours are set by a four-pass loop over a walking pointer; the
 * compiler unrolls it completely, which is where the second, third, fourth,
 * first store order, the reversed third group and the dead pointer advance
 * all come from, and the constant keeps its inner-loop weight so it is
 * coloured ahead of the address.  The two packets are the JFG macros, the Y
 * coordinate is the same cast at each use, and the parameter is initialised
 * before the prepare call with no probe statement.
 */
void overlay58DrawSegmentStrip(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                               f32 z1, f32 limit) {
    f32 dy;
    f32 dz;
    f32 distance;
    f32 t;
    f32 dx;
    f32 next;
    f32 startX;
    f32 startZ;
    f32 endX;
    f32 zPerpendicular;
    f32 xPerpendicular;
    f32 endZ;
    f32 stripStep;
    f32 quadSpan;
    Overlay58StripVertex *vertices;
    s32 i;

    dx = x1 - x0;
    dy = y1 - y0;
    dz = z1 - z0;
    distance = overlay58SqrtReloc((dx * dx) + (dy * dy) + (dz * dz));

    zPerpendicular = (1.25f * dx) / distance;
    xPerpendicular = (1.25f * dz) / distance;
    stripStep = 12.0f / distance;
    quadSpan = 8.0f / distance;
    t = 0.0f;

    overlay58PrepareStripReloc(&gOverlay58StripDisplayListReloc, (void *)0,
                               5, 0);
    while (t < limit) {
        vertices = gOverlay58StripVertexCursorReloc;
        for (i = 0; i < 4; i++) {
            vertices->r = 0xFF;
            vertices->g = 0xFF;
            vertices->b = 0xFF;
            vertices->a = 0xFF;
            vertices++;
        }
        next = t + quadSpan;
        if (next > 1.0f) {
            next = 1.0f;
        }
        if (next <= limit) {
            O58_VERTEX(gOverlay58StripDisplayListReloc++, O58_PHYSICAL_TO_K0(gOverlay58StripVertexCursorReloc), 4, 0);
            O58_POLYGON(gOverlay58StripDisplayListReloc++, D_80000058, 2, 1);
            startX = (t * dx) + x0;
            startZ = (t * dz) + z0;
            gOverlay58StripVertexCursorReloc->x = (s16)(startX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z = (s16)(startZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
            gOverlay58StripVertexCursorReloc->x = (s16)(startX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z = (s16)(startZ - zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
            endX = (next * dx) + x0;
            endZ = (next * dz) + z0;
            gOverlay58StripVertexCursorReloc->x = (s16)(endX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z = (s16)(endZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
            gOverlay58StripVertexCursorReloc->x = (s16)(endX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z = (s16)(endZ - zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
        }
        t += stripStep;
    }
}
