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
extern u8 gOverlay58StripIndexPayload58Reloc[];
extern f32 overlay58SqrtReloc(f32 value);
extern void overlay58PrepareStripReloc(Overlay58StripGfx **displayList,
                                       void *resource, s32 mode, s32 arg3);

/*
 * Plateau (2026-09-10): 68 of 201 relocation-masked words differ, down from
 * 102, at unchanged 201-word geometry and an unchanged 0x88 frame.  Three
 * moves, found by a frame-constrained hill climb over 25,132 variants:
 * one discarded-expression probe (ido-5.3 L37, zero instructions), the `dx`
 * declaration moved down two slots, and two write-order moves inside the loop
 * body that are semantically inert (the second vertex's `y` written before its
 * `x`, and the cursor advance moved across the independent `endZ` term).
 *
 * Falsified here: the argument-affinity mechanism that is worth 16 words on
 * this overlay's two point-quad draw routines does NOT apply to this call.
 * Passing `&gOverlay58StripVertexCursorReloc` as the first argument instead of
 * the display list regresses 102 -> 152 and breaks the size delta by -4, so
 * this callee really does take the display list.  The mechanism is a property
 * of the call site, not of the overlay.
 */
#ifdef NON_MATCHING
void overlay58DrawSegmentStrip(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                               f32 z1, f32 limit) {
    f32 dy;
    f32 dz;
    f32 distance;
    f32 t;
    f32 dx;
    f32 dummy0;
    f32 dummy1;
    f32 dummy2;
    f32 dummy3;
    f32 zPerpendicular;
    f32 xPerpendicular;
    f32 hole58;
    f32 stripStep;
    f32 quadSpan;

    dx = x1 - x0;
    dy = y1 - y0;
    dz = z1 - z0;
    distance = overlay58SqrtReloc((dx * dx) + (dy * dy) + (dz * dz));

    zPerpendicular = (1.25f * dx) / distance;
    t = 0.0f;
    xPerpendicular = (1.25f * dz) / distance;
    stripStep = 12.0f / distance;
    quadSpan = 8.0f / distance;

    overlay58PrepareStripReloc(&gOverlay58StripDisplayListReloc, (void *)0,
                               5, 0);
    if (limit != 0);

    while (t < limit) {
        Overlay58StripVertex *vertices;
        Overlay58StripVertex *vertex;
        Overlay58StripGfx *command;
        s32 vertexCommand;
        f32 next;
        f32 startX;
        f32 startZ;
        f32 endX;
        f32 endZ;
        f32 currentStep;
        s16 y;

        vertices = gOverlay58StripVertexCursorReloc;
        vertices++;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
        vertices->a = 0xFF;
        vertices->b = 0xFF;
        vertices->g = 0xFF;
        vertices->r = 0xFF;
        vertices[-3].r = 0xFF;
        vertices[-3].g = 0xFF;
        vertices[-3].b = 0xFF;
        vertices[-3].a = 0xFF;

        next = t + quadSpan;
        if (1.0f < next) {
            next = 1.0f;
        }
        currentStep = stripStep;
        if (next <= limit) {
            command = gOverlay58StripDisplayListReloc;
            vertexCommand =
                (((s32)gOverlay58StripVertexCursorReloc + 0x80000000) & 6) |
                0x20;
            gOverlay58StripDisplayListReloc = command + 1;
            command->w0 =
                (((vertexCommand & 0xFF) << 16) | 0x04000000) | 0x30;
            command->w1 =
                (s32)gOverlay58StripVertexCursorReloc + 0x80000000;

            command = gOverlay58StripDisplayListReloc;
            gOverlay58StripDisplayListReloc = command + 1;
            command->w0 = 0x05110020;
            command->w1 = (u32)&gOverlay58StripIndexPayload58Reloc[0];

            startX = x0 + (t * dx);
            startZ = z0 + (t * dz);
            y = (s16)y0;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(startX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = y;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(startZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
            gOverlay58StripVertexCursorReloc->y = y;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(startX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->z =
                (s16)(startZ - zPerpendicular);

            endZ = z0 + (next * dz);
            gOverlay58StripVertexCursorReloc++;
            endX = x0 + (next * dx);
            gOverlay58StripVertexCursorReloc->x =
                (s16)(endX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(endZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(endX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = y;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(endZ - zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
        }

        t += currentStep;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/overlay58DrawSegmentStrip/func_overlay_058_F0004C04_18B3DEC.s")
#endif

/* PLATEAU-HANDOFF:overlay58DrawSegmentStrip:start
 * symbol: overlay58DrawSegmentStrip
 * score: 68/201 words
 * frame: 0x88
 * relocations: 8
 * first-mismatch: +0xF0
 * summary: hypothesis=live-zero address add; spellings=9 stores +16/124, all uses +72/191, 16 ops +32/129 with 0xFF on v1; stall=cross emits reloads, size never 0
 * PLATEAU-HANDOFF:overlay58DrawSegmentStrip:end
 */
