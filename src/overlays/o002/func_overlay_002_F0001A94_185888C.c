#include "PR/ultratypes.h"

typedef struct Overlay2Point {
    s16 x;
    s16 y;
} Overlay2Point;

typedef struct Overlay2Entry {
    Overlay2Point *points;
    s32 pointCount;
    u8 pad8[0x14];
} Overlay2Entry;

typedef struct Overlay2Line {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
    u16 sourceIndex;
    u16 ownerIndex;
} Overlay2Line;

typedef struct Overlay2Node Overlay2Node;

typedef struct Overlay2Shape {
    Overlay2Point *points;
    s32 pointCount;
    s32 pad8;
    struct Overlay2Shape *next;
    u32 flags;
    Overlay2Node *nodes;
    Overlay2Line *lines;
} Overlay2Shape;

typedef struct Overlay2Hit {
    s16 x;
    s16 y;
    u16 index;
    s16 previous;
    s16 next;
    u16 owner;
    s32 flags;
} Overlay2Hit;

/* Overlay 1's function, called through a placeholder: a value line for
 * overlay1GetEntry itself would override its definition in every module. */
extern Overlay2Entry *overlay2Overlay1GetEntryReloc(u16 index);
extern void overlay2AdjacentIndices(Overlay2Entry *entry, u16 index,
                                    s16 *previous, s16 *next);
extern s32 overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1,
                             Overlay2Node *node);
/* Both call sites reach overlay2ContainsPoint through the module's own ROM
 * symbol table (a SYMBOL relocation record naming overlay 2 offset 0x123C),
 * not as an intra-module JUMP, so the reference has to stay undefined here
 * and take the 0xF0000000 stored addend the runtime patches. */
extern s32 overlay2ContainsPointReloc(f32 x, f32 y, Overlay2Shape *shape);

extern Overlay2Node *D_38;
extern Overlay2Line *gOverlay2QueryLinesReloc;
extern f32 gOverlay2QueryX0Reloc;
extern f32 gOverlay2QueryY0Reloc;
extern f32 gOverlay2QueryX1Reloc;
extern f32 gOverlay2QueryY1Reloc;
extern Overlay2Hit *gOverlay2QueryHitReloc;
extern f32 gOverlay2QueryHitXReloc;
extern f32 gOverlay2QueryHitYReloc;
extern f32 gOverlay2QueryBestReloc;
extern u16 gOverlay2QueryIndexReloc;
extern u16 D_72;
extern f32 gOverlay2QueryLimitReloc;
extern f32 gOverlay2QueryResultReloc;

/* Matched.  Two independent edits closed it, each a regression on its own.
 * Dropping the `register s16 pointY` carrier removes the fourth declared auto,
 * which shrinks the auto region by one word and lifts the two `&hit->previous`
 * / `&hit->next` spill homes from -0x18/-0x14 to -0x14/-0x10; alone that cost
 * 27 -> 41 masked words by pushing `point->y` out of a pool colour into the
 * ugen ring one slot early.  Writing all three `s16` equality tests with the
 * point member on the left then supplies the emission order the ring wants:
 * ugen evaluates a comparison's right operand first, so `point->x == (s16)g`
 * requests the cast chain before the member load, which is the target's ring
 * order.  Either edit alone is worse than the plateau; together they are
 * exact. */
s32 func_overlay_002_F0001A94_185888C(f32 x0, f32 y0, f32 x1, f32 y1,
                                      Overlay2Shape *shape, Overlay2Hit *hit,
                                      s32 previousIndex, u16 shapeIndex) {
    Overlay2Entry *baseEntry;
    Overlay2Entry *hitEntry;
    Overlay2Point *point;

    baseEntry = overlay2Overlay1GetEntryReloc(shapeIndex);
    D_38 = shape->nodes;
    gOverlay2QueryLinesReloc = shape->lines;
    gOverlay2QueryX0Reloc = x0;
    gOverlay2QueryY0Reloc = y0;
    gOverlay2QueryX1Reloc = x1;
    gOverlay2QueryY1Reloc = y1;
    gOverlay2QueryBestReloc = gOverlay2QueryLimitReloc;
    gOverlay2QueryHitReloc = hit;

    overlay2QueryNode(x0, y0, x1, y1, D_38);
    if (gOverlay2QueryBestReloc == gOverlay2QueryResultReloc) {
        return 0;
    }

    if (hit != 0) {
        hit->x = (s16)gOverlay2QueryHitXReloc;
        hit->y = (s16)gOverlay2QueryHitYReloc;
        hit->index = gOverlay2QueryIndexReloc;
        hit->owner = D_72;

        hitEntry = overlay2Overlay1GetEntryReloc(D_72);
        overlay2AdjacentIndices(hitEntry, hit->index,
                                &hit->previous, &hit->next);

        if (previousIndex != -1) {
            point = &baseEntry->points[previousIndex];
            if (point->x == (s16)gOverlay2QueryX0Reloc) {
                if (point->y == (s16)gOverlay2QueryY0Reloc) {
                    if (overlay2ContainsPointReloc(
                            ((gOverlay2QueryHitXReloc -
                              gOverlay2QueryX0Reloc) *
                             0.5f) +
                                gOverlay2QueryX0Reloc,
                            ((gOverlay2QueryHitYReloc -
                              gOverlay2QueryY0Reloc) *
                             0.5f) +
                                gOverlay2QueryY0Reloc,
                            shape) == 0) {
                        hit->x = point->x;
                        hit->y = point->y;
                        hit->index = previousIndex;
                        hit->owner = shapeIndex;
                        overlay2AdjacentIndices(baseEntry, previousIndex,
                                                &hit->previous, &hit->next);
                        hit->flags = 0;
                        return 1;
                    }
                }
            }
        }

        point = &hitEntry->points[hit->index];
        if ((point->x == (s16)gOverlay2QueryHitXReloc) &&
            (point->y == (s16)gOverlay2QueryHitYReloc)) {
            hit->flags = 1;
        } else {
            if (overlay2ContainsPointReloc(
                    ((gOverlay2QueryHitXReloc - gOverlay2QueryX0Reloc) *
                     0.5f) +
                        gOverlay2QueryX0Reloc,
                    ((gOverlay2QueryHitYReloc - gOverlay2QueryY0Reloc) *
                     0.5f) +
                        gOverlay2QueryY0Reloc,
                    shape) == 0) {
                hit->x = (s16)gOverlay2QueryX0Reloc;
                hit->y = (s16)gOverlay2QueryY0Reloc;
                hit->previous = hit->index;
                hit->index = 0xFF;
                hit->flags = 0;
                return 1;
            } else {
                hit->previous = hit->index;
                hit->index = 0xFF;
                hit->flags = 0;
            }
        }
    }
    return 1;
}
