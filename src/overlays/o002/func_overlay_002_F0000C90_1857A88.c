#include "PR/ultratypes.h"

typedef struct Overlay2BuildPoint {
    s16 x;
    s16 y;
} Overlay2BuildPoint;

typedef struct Overlay2BuildLine {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2BuildLine;

typedef struct Overlay2BuildRegion {
    s32 boundaryAxis;
    f32 boundaryValue;
    struct Overlay2BuildRegion *side1;
    struct Overlay2BuildRegion *side0;
    u16 start;
    u16 count;
} Overlay2BuildRegion;

typedef struct Overlay2BuildNode {
    u16 type;
    s16 value;
    union {
        f32 boundaryValue;
        struct {
            u16 count;
            u16 pad06;
        } leaf;
    } data;
    struct Overlay2BuildNode *side1;
    struct Overlay2BuildNode *side0;
} Overlay2BuildNode;

typedef struct Overlay2BuildObject {
    Overlay2BuildPoint *points;
    s32 pointCount;
    u8 pad08[4];
    struct Overlay2BuildObject *next;
    s32 flags;
    Overlay2BuildNode *nodes;
    Overlay2BuildLine *lines;
} Overlay2BuildObject;

extern Overlay2BuildLine *gOverlay2Lines;
extern Overlay2BuildRegion *gOverlay2Regions;
extern void *gOverlay2BoundaryCandidates;
extern s32 gOverlay2LineCount;
extern s32 gOverlay2RegionCount;
extern s32 gOverlay2BuiltNodeCount;
extern s32 gOverlay2BuiltLineCount;

/* Cross-overlay callee, reached through a runtime relocation record. */
extern u16 overlay2GetBuildValueReloc(Overlay2BuildObject *object);
extern void *func_8002B280(s32 size, s32 tag);
extern void _bzero(void *memory, s32 size);
extern void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                               u16 value2);
extern void overlay2SplitRegion(Overlay2BuildRegion *previous,
                                Overlay2BuildRegion *region);
extern s32 mmGetDelay(void);
extern void mmSetDelay(s32 state);
extern void mmFree(void *memory);
extern void func_8002B524(s32 size, void *memory, s32 tag);

/*
 * PROVENANCE: Jet Force Gemini src/overlays/o142/overlay_142.c identifies the
 * close assembly-backed sibling as CreateBSP. No donor C body exists there;
 * this body is reconstructed from Mickey's types, calls, and object code.
 *
 * Matched 2026-10-01 by replacing the inherited carriers with what the
 * listing shows the author wrote:
 * - the two point loops are plain `while (remaining-- > 0)`, so the test is
 *   the compiler's own post-decrement temporary, the same register the two
 *   later countdowns use, not a declared flag;
 * - one unsigned count serves as the region countdown and then as the output
 *   line count. Sharing the symbol gives the countdown its callee-saved
 *   register, and the unsigned type keeps the final literal multiply by 20
 *   from joining the signed divisor's register;
 * - the side test reads the region field directly, and the node type is an
 *   unsigned 16-bit field stored from the literals 0 and 1, which hoists
 *   the 1 into the loop preheader;
 * - the child links are index-then-base sums;
 * - the linked flag is an ordinary parameter (no volatile).
 */
void func_overlay_002_F0000C90_1857A88(Overlay2BuildObject *object,
                                        s32 includeLinked) {
    void *pad0; /* two unused homes hold the 0x68 frame (L99) */
    void *pad1;
    Overlay2BuildPoint *point;
    Overlay2BuildPoint *nextPoint;
    Overlay2BuildPoint *firstPoint;
    Overlay2BuildPoint *lastPoint;
    Overlay2BuildObject *linked;
    Overlay2BuildRegion *rootRegion;
    s32 savedState;
    s32 rootValue;
    s32 linkedValue;
    s32 remaining;
    u32 count;
    s32 size;
    s32 stride;

    remaining = object->pointCount;
    point = object->points;
    firstPoint = point;
    lastPoint = point + remaining;
    lastPoint--;
    linked = object;
    rootValue = overlay2GetBuildValueReloc(object);

    gOverlay2Lines = func_8002B280(0xF000, 0x85);
    gOverlay2Regions = func_8002B280(0x2800, 0x85);
    gOverlay2BoundaryCandidates = func_8002B280(0x2000, 0x85);
    _bzero(gOverlay2Lines, 0xF000);
    _bzero(gOverlay2Regions, 0x2800);
    _bzero(gOverlay2BoundaryCandidates, 0x2000);
    gOverlay2LineCount = 0;
    gOverlay2RegionCount = 1;
    rootRegion = gOverlay2Regions;

    while (remaining-- > 0) {
        if (point < lastPoint) {
            nextPoint = point + 1;
        } else {
            nextPoint = firstPoint;
        }
        overlay2AppendLine((f32)point->x, (f32)point->y,
                           (f32)nextPoint->x, (f32)nextPoint->y,
                           point - firstPoint, rootValue);
        point = nextPoint;
    }
    rootRegion->count = gOverlay2LineCount;

    if (includeLinked != 0) {
        linked = linked->next;
        while (linked != NULL) {
            if (linked->flags & 1) {
                linkedValue = overlay2GetBuildValueReloc(linked);
                remaining = linked->pointCount;
                point = linked->points;
                firstPoint = point;
                lastPoint = point + remaining;
                lastPoint--;
                while (remaining-- > 0) {
                    if (point < lastPoint) {
                        nextPoint = point + 1;
                    } else {
                        nextPoint = firstPoint;
                    }
                    overlay2AppendLine((f32)point->x, (f32)point->y,
                                       (f32)nextPoint->x, (f32)nextPoint->y,
                                       point - firstPoint, linkedValue);
                    point = nextPoint;
                }
                rootRegion->count = gOverlay2LineCount;
            }
            linked = linked->next;
        }
    }

    overlay2SplitRegion(NULL, rootRegion);
    stride = 0x14;
    size = (gOverlay2RegionCount * 0x10) +
           (gOverlay2LineCount * stride);
    object->nodes = func_8002B280(size, 0x85);
    object->lines = (Overlay2BuildLine *)((u8 *)object->nodes +
                                          (gOverlay2RegionCount *
                                           0x10));

    {
        Overlay2BuildRegion *region;
        Overlay2BuildNode *node;
        Overlay2BuildLine *line;
        Overlay2BuildLine *outputLine;
        s32 lineRemaining;

        region = gOverlay2Regions;
        node = object->nodes;
        outputLine = object->lines;
        count = gOverlay2RegionCount;
        if (count--) {
            do {
                if (region->side0 != NULL) {
                    node->type = 0;
                    node->side1 = (Overlay2BuildNode *)(
                        ((((s32)region->side1 - (s32)gOverlay2Regions) / stride) * 0x10) +
                        (s32)object->nodes);
                    node->side0 = (Overlay2BuildNode *)(
                        ((((s32)region->side0 - (s32)gOverlay2Regions) / stride) * 0x10) +
                        (s32)object->nodes);
                    node->value = region->boundaryAxis;
                    node->data.boundaryValue = region->boundaryValue;
                } else {
                    node->type = 1;
                    node->value =
                        ((s32)outputLine - (s32)object->lines) / stride;
                    node->data.leaf.count = region->count;
                    line = (Overlay2BuildLine *)(
                        (region->start * stride) +
                        (s32)gOverlay2Lines);
                    lineRemaining = region->count;
                    if (lineRemaining--) {
                        do {
                            outputLine->x1 = line->x1;
                            outputLine->y1 = line->y1;
                            outputLine->x2 = line->x2;
                            outputLine->y2 = line->y2;
                            outputLine->value1 = line->value1;
                            outputLine->value2 = line->value2;
                            outputLine++;
                            line++;
                        } while (lineRemaining--);
                    }
                }
                region++;
                node++;
            } while (count--);
        }

        gOverlay2BuiltNodeCount = gOverlay2RegionCount;
        gOverlay2BuiltLineCount =
            ((s32)outputLine - (s32)object->lines) / stride;
        count = ((s32)outputLine - (s32)object->lines) / stride;
    }

    savedState = mmGetDelay();
    mmSetDelay(0);
    mmFree(object->nodes);
    mmSetDelay(savedState);
    size = (gOverlay2RegionCount * 0x10) +
           (count * 0x14);
    func_8002B524(size, object->nodes, 0x85);

    savedState = mmGetDelay();
    mmSetDelay(0);
    mmFree(gOverlay2Lines);
    mmFree(gOverlay2Regions);
    mmFree(gOverlay2BoundaryCandidates);
    mmSetDelay(savedState);
}
