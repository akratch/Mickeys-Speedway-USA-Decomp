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
    s16 type;
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

extern Overlay2BuildRegion *D_4;
extern Overlay2BuildLine *gOverlay2BuildLinesReloc;
extern void *gOverlay2BuildScratchReloc;
extern s32 gOverlay2BuildLineCountReloc;
extern s32 gOverlay2BuildRegionCountReloc;
extern s32 gOverlay2BuiltNodeCountReloc;
extern s32 gOverlay2BuiltLineCountReloc;

extern u16 overlay2GetBuildValueReloc(Overlay2BuildObject *object);
extern void *overlay2BuildAllocateReloc(s32 size, s32 tag);
extern void overlay2BuildClearReloc(void *memory, s32 size);
extern void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                               u16 value2);
extern void overlay2SplitRegion(Overlay2BuildRegion *previous,
                                Overlay2BuildRegion *region);
extern void *overlay2BuildBeginReloc(void);
extern void overlay2BuildPhaseReloc(void *state);
extern void overlay2BuildReleaseReloc(void *memory);
extern void overlay2BuildResizeReloc(s32 size, void *memory, s32 tag);

/* Workbench plateau: 76 masked words; 355/355 instructions, frame exact 0x68.
 * Overlay41 remat of side0 before the test plus L160 countdown 94 to 76.
 * Signed-1 hoist stalled: static load +4/238; preheader and header assigns flat.
 * u32 leafType still the only form that keeps the preheader li, at 121 naming. */
/*
 * PROVENANCE: Jet Force Gemini src/overlays/o142/overlay_142.c identifies the
 * close assembly-backed sibling as CreateBSP. No donor C body exists there;
 * this body is reconstructed from Mickey's types, calls, and object code.
 */
#ifdef NON_MATCHING
void func_overlay_002_F0000C90_1857A88(Overlay2BuildObject *object,
                                        volatile s32 includeLinked) {
    void *unused0; /* L99: unused pointers declared first */
    void *unused1;
    Overlay2BuildPoint *point;
    Overlay2BuildPoint *nextPoint;
    Overlay2BuildPoint *firstPoint;
    Overlay2BuildPoint *lastPoint;
    Overlay2BuildObject *linked;
    Overlay2BuildRegion *rootRegion;
    void *savedState;
    s32 rootValue;
    s32 linkedValue;
    s32 hasRemaining;
    s32 remaining;
    s32 outputLineCount;
    s32 size;
    s32 stride;

    remaining = object->pointCount;
    point = object->points;
    firstPoint = point;
    lastPoint = point + remaining;
    lastPoint--;
    linked = object;
    rootValue = overlay2GetBuildValueReloc(object);

    gOverlay2BuildLinesReloc = overlay2BuildAllocateReloc(0xF000, 0x85);
    D_4 = overlay2BuildAllocateReloc(0x2800, 0x85);
    gOverlay2BuildScratchReloc = overlay2BuildAllocateReloc(0x2000, 0x85);
    overlay2BuildClearReloc(gOverlay2BuildLinesReloc, 0xF000);
    overlay2BuildClearReloc(D_4, 0x2800);
    overlay2BuildClearReloc(gOverlay2BuildScratchReloc, 0x2000);
    gOverlay2BuildLineCountReloc = 0;
    gOverlay2BuildRegionCountReloc = 1;
    rootRegion = D_4;

    hasRemaining = remaining > 0;
    remaining--;
    if (hasRemaining) {
        do {
            if (point < lastPoint) {
                nextPoint = point + 1;
            } else {
                nextPoint = firstPoint;
            }
            overlay2AppendLine((f32)point->x, (f32)point->y,
                               (f32)nextPoint->x, (f32)nextPoint->y,
                               point - firstPoint, rootValue);
            hasRemaining = remaining > 0;
            remaining--;
            point = nextPoint;
        } while (hasRemaining);
    }
    rootRegion->count = gOverlay2BuildLineCountReloc;

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
                hasRemaining = remaining > 0;
                remaining--;
                if (hasRemaining) {
                    do {
                        if (point < lastPoint) {
                            nextPoint = point + 1;
                        } else {
                            nextPoint = firstPoint;
                        }
                        overlay2AppendLine(
                            (f32)point->x, (f32)point->y,
                            (f32)nextPoint->x, (f32)nextPoint->y,
                            point - firstPoint, linkedValue);
                        hasRemaining = remaining > 0;
                        remaining--;
                        point = nextPoint;
                    } while (hasRemaining);
                }
                rootRegion->count = gOverlay2BuildLineCountReloc;
            }
            linked = linked->next;
        }
    }

    overlay2SplitRegion(NULL, rootRegion);
    stride = 0x14;
    size = (gOverlay2BuildRegionCountReloc * 0x10) +
           (gOverlay2BuildLineCountReloc * stride);
    object->nodes = overlay2BuildAllocateReloc(size, 0x85);
    object->lines = (Overlay2BuildLine *)((u8 *)object->nodes +
                                          (gOverlay2BuildRegionCountReloc *
                                           0x10));

    {
        Overlay2BuildRegion *region;
        Overlay2BuildRegion *side0;
        Overlay2BuildNode *node;
        Overlay2BuildLine *line;
        Overlay2BuildLine *outputLine;
        s32 regionRemaining;
        s32 lineRemaining;
        s32 leafType;

        region = D_4;
        node = object->nodes;
        outputLine = object->lines;
        regionRemaining = gOverlay2BuildRegionCountReloc;
        leafType = 1;
        if (regionRemaining--) {
            do {
                side0 = region->side0;
                if (side0 != NULL) {
                    node->type = 0;
                    node->side1 = (Overlay2BuildNode *)(
                        (s32)object->nodes +
                        ((((s32)region->side1 - (s32)D_4) / stride) * 0x10));
                    node->side0 = (Overlay2BuildNode *)(
                        (s32)object->nodes +
                        ((((s32)region->side0 - (s32)D_4) / stride) * 0x10));
                    node->value = region->boundaryAxis;
                    node->data.boundaryValue = region->boundaryValue;
                } else {
                    node->type = leafType;
                    node->value =
                        ((s32)outputLine - (s32)object->lines) / stride;
                    node->data.leaf.count = region->count;
                    line = (Overlay2BuildLine *)(
                        (region->start * stride) +
                        (s32)gOverlay2BuildLinesReloc);
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
            } while (regionRemaining--);
        }

        gOverlay2BuiltNodeCountReloc = gOverlay2BuildRegionCountReloc;
        gOverlay2BuiltLineCountReloc =
            ((s32)outputLine - (s32)object->lines) / stride;
        outputLineCount = ((s32)outputLine - (s32)object->lines) / stride;
    }

    savedState = overlay2BuildBeginReloc();
    overlay2BuildPhaseReloc(NULL);
    overlay2BuildReleaseReloc(object->nodes);
    overlay2BuildReleaseReloc(savedState);
    size = (gOverlay2BuildRegionCountReloc * 0x10) +
           (((outputLineCount * 4) + outputLineCount) * 4);
    overlay2BuildResizeReloc(size, object->nodes, 0x85);

    savedState = overlay2BuildBeginReloc();
    overlay2BuildPhaseReloc(NULL);
    overlay2BuildReleaseReloc(gOverlay2BuildLinesReloc);
    overlay2BuildReleaseReloc(D_4);
    overlay2BuildReleaseReloc(gOverlay2BuildScratchReloc);
    overlay2BuildReleaseReloc(savedState);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/func_overlay_002_F0000C90_1857A88/func_overlay_002_F0000C90_1857A88.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_002_F0000C90_1857A88:start
 * symbol: func_overlay_002_F0000C90_1857A88
 * score: 76/355 words
 * frame: 0x68
 * relocations: 65
 * first-mismatch: +0xDC
 * summary: Signed-1 hoist stalled after 3: static s32 +4/238, if-preheader and do-header assigns flat at 76. Signed const still rematerializes in the leaf.
 * PLATEAU-HANDOFF:func_overlay_002_F0000C90_1857A88:end
 */
