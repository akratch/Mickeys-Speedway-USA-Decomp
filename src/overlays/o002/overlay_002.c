#include "overlays/overlay_002.h"

/*
 * Overlay 2: collision shapes. Builds a binary partition of each shape's
 * outline and answers point and segment queries against it. One translation
 * unit; the partition builder's helpers and the query walk are unexported and
 * static.
 */

static s32 overlay2ValidateRegion(Overlay2Region *region);
static void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                               u16 value2);
static s32 overlay2ClassifyBoundary(f32 x1, f32 y1, f32 x2, f32 y2,
                                    s32 *side1, s32 *side2);
static void overlay2IntersectBoundary(f32 x0, f32 y0, f32 x1, f32 y1,
                                      f32 *outX, f32 *outY);
static void overlay2ClipLines(Overlay2Region *input, Overlay2Region *output,
                              s32 wantedSide);
static void overlay2ChooseBoundary(Overlay2Region *region);
static void overlay2SplitRegion(Overlay2Region *previous, Overlay2Region *region);
static void overlay2AdjacentIndices(Overlay2Shape *entry, u16 index,
                                    s16 *previous, s16 *next);
static int overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1, Overlay2Node *node);

/* Pinned DKR v77/v80 and JFG scans found no exact region-validator donor. */
static s32 overlay2ValidateRegion(Overlay2Region *region) {
    Overlay2Line *line;
    Overlay2Line *next;
    s32 remaining;
    s32 angle;
    s32 nextAngle;
    s16 connectorAngle;

    if (region->count != 1) {
        line = &gOverlay2Lines[region->start];
        remaining = region->count;
        while (remaining--) {
            angle = (s16)func_8002A910(line->y2 - line->y1,
                                       line->x2 - line->x1);
            if (remaining != 0) {
                next = line + 1;
            } else {
                next = &gOverlay2Lines[region->start];
            }

            nextAngle = (s16)func_8002A910(next->y2 - next->y1,
                                           next->x2 - next->x1);
            if ((line->x2 != next->x1) || (line->y2 != next->y1)) {
                connectorAngle =
                    (s16)func_8002A910(next->y1 - line->y2,
                                       next->x1 - line->x2);
                if ((func_8002AA0C(angle, connectorAngle) < 0) ||
                    (func_8002AA0C(connectorAngle, nextAngle) < 0)) {
                    return 0;
                }
            } else if (func_8002AA0C(angle, nextAngle) < 0) {
                return 0;
            }

            line = next;
        }
    }

    return 1;
}

/* Pinned DKR v77/v80 and JFG scans found no line-buffer donor. */
static void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                        u16 value2) {
    if ((x1 != x2) || (y1 != y2)) {
        /* IDO schedules the endpoint load before the cached count address only when this original macro-like group shares a source line. */
        do { if (gOverlay2LineCount >= 0xC00) { while (1) { } } gOverlay2Lines[gOverlay2LineCount].x1 = x1; gOverlay2Lines[gOverlay2LineCount].y1 = y1; gOverlay2Lines[gOverlay2LineCount].x2 = x2; gOverlay2Lines[gOverlay2LineCount].y2 = y2; } while (0);
        gOverlay2Lines[gOverlay2LineCount].value1 = value1;
        gOverlay2Lines[gOverlay2LineCount].value2 = value2;
        gOverlay2LineCount++;
    }
}

static s32 overlay2ClassifyBoundary(f32 x1, f32 y1, f32 x2, f32 y2, s32 *side1,
                             s32 *side2) {
    if (gOverlay2BoundaryAxis == 0) {
        *side1 = y1 < gOverlay2BoundaryValue;
        *side2 = y2 < gOverlay2BoundaryValue;
        if (y1 == gOverlay2BoundaryValue) {
            *side1 = *side2;
        } else if (y2 == gOverlay2BoundaryValue) {
            *side2 = *side1;
        }
    } else {
        *side1 = x1 < gOverlay2BoundaryValue;
        *side2 = x2 < gOverlay2BoundaryValue;
        if (x1 == gOverlay2BoundaryValue) {
            *side1 = *side2;
        } else if (x2 == gOverlay2BoundaryValue) {
            *side2 = *side1;
        }
    }
    return 1;
}

/* Exact at +0x400; DKR v77/v80 and JFG have no exact donor for this helper. */
static void overlay2IntersectBoundary(f32 x0, f32 y0, f32 x1, f32 y1,
                               f32 *outX, f32 *outY) {
    f32 ratio;

    if (gOverlay2BoundaryAxis == 0) {
        ratio = (gOverlay2BoundaryValue - y0) / (y1 - y0);
        *outX = ((x1 - x0) * ratio) + x0;
        *outY = gOverlay2BoundaryValue;
    } else {
        ratio = (gOverlay2BoundaryValue - x0) / (x1 - x0);
        *outX = gOverlay2BoundaryValue;
        *outY = ((y1 - y0) * ratio) + y0;
    }
}

/* Pinned DKR v77/v80 and JFG scans found no matching clipping body. */
static void overlay2ClipLines(Overlay2Region *input, Overlay2Region *output,
                       s32 wantedSide) {
    Overlay2Line *line;
    s32 remaining;
    s32 side1;
    s32 side2;
    f32 intersectionX;
    f32 intersectionY;

    remaining = input->count;
    line = &gOverlay2Lines[input->start];
    output->start = gOverlay2LineCount;
    while (remaining--) {
        if (overlay2ClassifyBoundary(line->x1, line->y1, line->x2, line->y2,
                                     &side1, &side2)) {
            if (side1 != side2) {
                overlay2IntersectBoundary(line->x1, line->y1, line->x2,
                                          line->y2, &intersectionX,
                                          &intersectionY);
                if (side1 == wantedSide) {
                    overlay2AppendLine(line->x1, line->y1, intersectionX,
                                       intersectionY, line->value1,
                                       line->value2);
                } else {
                    overlay2AppendLine(intersectionX, intersectionY, line->x2,
                                       line->y2, line->value1, line->value2);
                }
            } else if (side1 == wantedSide) {
                overlay2AppendLine(line->x1, line->y1, line->x2, line->y2,
                                   line->value1, line->value2);
            }
        } else {
            if (gOverlay2BoundaryAxis == 0) {
                side1 = line->x1 < line->x2;
            } else {
                side1 = line->y1 < line->y2;
            }
            if (side1 == wantedSide) {
                overlay2AppendLine(line->x1, line->y1, line->x2, line->y2,
                                   line->value1, line->value2);
            }
        }
        line++;
    }
    output->count = gOverlay2LineCount - output->start;
}

#define CANDIDATE_COUNT gOverlay2BoundaryCandidateCount[0]

/* Countdown and source-line scheduling reproduce the owned instruction stream. */
static void overlay2ChooseBoundary(Overlay2Region *region) {
    s32 scanRemaining;
    Overlay2Line *candidateLine;
    Overlay2Line *line;
    Overlay2BoundaryCandidate *candidate;
    s32 lineRemaining;
    s32 side0Count;
    s32 side1;
    s32 side2;
    s32 side1Count;
    s32 bestScore;
    s32 crossingCount;
    s32 axis;
    s32 endpoint;
    s32 rangeResults;
    s32 score;
    s32 savedLineCount;

    scanRemaining = region->count;
    candidateLine = &gOverlay2Lines[region->start];
    bestScore = 0x7FFFFFFF;
    CANDIDATE_COUNT = 0;
    if (scanRemaining--) {
        do {
            axis = 1;
            do {
                endpoint = 1;
                do {
                    gOverlay2BoundaryAxis = axis;
                    if (axis == 0) {
                        if (endpoint != 0) {
                            gOverlay2BoundaryValue = candidateLine->y1;
                        } else {
                            gOverlay2BoundaryValue = candidateLine->y2;
                        }
                    } else if (endpoint != 0) {
                        gOverlay2BoundaryValue = candidateLine->x1;
                    } else {
                        gOverlay2BoundaryValue = candidateLine->x2;
                    }

                    side0Count = 0;
                    side1Count = 0;
                    crossingCount = 0;
                    rangeResults = 0;
                    lineRemaining = region->count;
                    line = &gOverlay2Lines[region->start];

                    if (lineRemaining--) {
                        do {
                            overlay2ClassifyBoundary(line->x1, line->y1,
                                                     line->x2, line->y2,
                                                     &side1, &side2);
                            line++;
                            if (side1 != side2) {
                                crossingCount++;
                            } else if (side1 != 0) {
                                side1Count++;
                            } else {
                                side0Count++;
                            }
                        } while (lineRemaining--);
                    }

                    score = side1Count - side0Count;
                    if (score < 0) {
                        score = -score;
                    }
                    score += crossingCount >> 1;

                    savedLineCount = gOverlay2LineCount;
                    gOverlay2TemporaryMode = 0;
                    overlay2ClipLines(region, &gOverlay2TemporaryRange, 1);
                    if (overlay2ValidateRegion(
                            &gOverlay2TemporaryRange) != 0) {
                        rangeResults = 1;
                    }
                    overlay2ClipLines(region, &gOverlay2TemporaryRange, 0);
                    if (overlay2ValidateRegion(
                            &gOverlay2TemporaryRange) != 0) {
                        rangeResults++;
                    }
                    gOverlay2LineCount = savedLineCount;
                    gOverlay2TemporaryMode = 1;

                    if ((crossingCount == 0) &&
                        ((side1Count == 0) || (side0Count == 0))) {
                        score += 1000;
                    }
                    if (rangeResults == 2) {
                        score = -100;
                    } else if (rangeResults == 1) {
                        score -= 5;
                    }
                    if (score < bestScore) {
                        CANDIDATE_COUNT = 0;
                        bestScore = score;
                    }
                    if (score <= bestScore) {
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].lineIndex =
                            candidateLine - gOverlay2Lines;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].side0Count =
                            side0Count;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].side1Count =
                            side1Count;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT]
                            .crossingCount = crossingCount;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].axis =
                            gOverlay2BoundaryAxis;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].value =
                            gOverlay2BoundaryValue;
                        if (CANDIDATE_COUNT < 0x20) {
                            CANDIDATE_COUNT++;
                        }
                    }
                } while (endpoint--);
            } while (axis--);
            candidateLine++;
        } while (scanRemaining--);
    }

    scanRemaining = CANDIDATE_COUNT;
    /* Keep the cursor definition and loop opener on the same physical line. */
    if (scanRemaining--) {
        candidate = (Overlay2BoundaryCandidate *)((u8 *)gOverlay2BoundaryCandidates + (scanRemaining << 4)); do {

            if (candidate->crossingCount < 0x0FFFFFFF) {
                gOverlay2SelectedBoundary = scanRemaining;
            }
            candidate--;
        } while (scanRemaining--);
    }

    gOverlay2BoundaryAxis =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].axis;
    region->boundaryAxis = gOverlay2BoundaryAxis;
    gOverlay2BoundaryValue =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].value;
    region->boundaryValue = gOverlay2BoundaryValue;
    gOverlay2SelectedBoundary =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].lineIndex;
}


/* Exact 72-word C. IDO 5.3's fidelity-clean as1 scheduler trace showed the
 * prologue's ready nodes split between the function and loop-header source
 * lines. Keeping those token-equivalent headers on one physical line produces
 * the retail order; all nine runtime relocation tuples and identities agree. */
static void overlay2SplitRegion(Overlay2Region *previous, Overlay2Region *region) { for (;;) {

        if (overlay2ValidateRegion(region) != 0) {
            break;
        }
        overlay2ChooseBoundary(region);
        if ((previous != NULL) &&
            (previous->boundaryAxis == region->boundaryAxis) &&
            (previous->boundaryValue == region->boundaryValue)) {
            break;
        }

        region->side1 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(region, region->side1, 1);
        region->side0 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(region, region->side0, 0);
        overlay2SplitRegion(region, region->side1);
        previous = region;
        region = region->side0;
    }
}

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
void func_overlay_002_F0000C90_1857A88(Overlay2Shape *object,
                                        s32 includeLinked) {
    void *pad0; /* two unused homes hold the 0x68 frame (L99) */
    void *pad1;
    Overlay2Point *point;
    Overlay2Point *nextPoint;
    Overlay2Point *firstPoint;
    Overlay2Point *lastPoint;
    Overlay2Shape *linked;
    Overlay2Region *rootRegion;
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
    rootValue = overlay1GetEntryIndex(object);

    gOverlay2Lines = mmAlloc(0xF000, 0x85);
    gOverlay2Regions = mmAlloc(0x2800, 0x85);
    gOverlay2BoundaryCandidates = mmAlloc(0x2000, 0x85);
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
                linkedValue = overlay1GetEntryIndex(linked);
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
    object->nodes = mmAlloc(size, 0x85);
    object->lines = (Overlay2Line *)((u8 *)object->nodes +
                                          (gOverlay2RegionCount *
                                           0x10));

    {
        Overlay2Region *region;
        Overlay2Node *node;
        Overlay2Line *line;
        Overlay2Line *outputLine;
        s32 lineRemaining;

        region = gOverlay2Regions;
        node = object->nodes;
        outputLine = object->lines;
        count = gOverlay2RegionCount;
        if (count--) {
            do {
                if (region->side0 != NULL) {
                    node->type = 0;
                    node->side1 = (Overlay2Node *)(
                        ((((s32)region->side1 - (s32)gOverlay2Regions) / stride) * 0x10) +
                        (s32)object->nodes);
                    node->side0 = (Overlay2Node *)(
                        ((((s32)region->side0 - (s32)gOverlay2Regions) / stride) * 0x10) +
                        (s32)object->nodes);
                    node->index = region->boundaryAxis;
                    node->data.boundary = region->boundaryValue;
                } else {
                    node->type = 1;
                    node->index =
                        ((s32)outputLine - (s32)object->lines) / stride;
                    node->data.leaf.count = region->count;
                    line = (Overlay2Line *)(
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
    mmAllocAtAddr(size, object->nodes, 0x85);

    savedState = mmGetDelay();
    mmSetDelay(0);
    mmFree(gOverlay2Lines);
    mmFree(gOverlay2Regions);
    mmFree(gOverlay2BoundaryCandidates);
    mmSetDelay(savedState);
}

/* DKR v77/v80 and JFG checks found no exact donor for this mode wrapper. */
void overlay2Enable(void *object) {
    func_overlay_002_F0000C90_1857A88_o002Reloc(object, 1);
}

/* Pinned DKR v77/v80 and JFG scans found no exact point-containment donor. */
s32 overlay2ContainsPoint(f32 x, f32 y, Overlay2Shape *shape) {
    Overlay2Node *node;
    Overlay2Line *line;
    f32 coordinate;
    s32 count;
    s16 firstAngle;
    s16 secondAngle;
    s32 hasMore;

    node = shape->nodes;
    if (node->type == 0) {
        do {
            if (node->index == 0) {
                coordinate = y;
            } else {
                coordinate = x;
            }
            if (coordinate < node->data.boundary) {
                node = node->side1;
            } else {
                node = node->side0;
            }
        } while (node->type == 0);
    }

    count = node->data.leaf.count;
    line = &shape->lines[node->index];
    hasMore = count;
    count--;
    if (hasMore != 0) {
        do {
            firstAngle = Arctanf(line->y1 - y, line->x1 - x);
            secondAngle = Arctanf(line->y2 - y, line->x2 - x);
            if (mathDiffAngle(firstAngle, secondAngle) < 0) {
                return 0;
            }
            line++;
            hasMore = count;
            count--;
        } while (hasMore != 0);
    }
    return 1;
}

/* Segment intersection: the second copy of overlay2IntersectSegments in this
 * overlay, written the same way and built with -Wab,-r4300_mul
 * (mk/overlays.mk): the shipped code keeps every mul.s out of branch delay
 * slots and separates back-to-back multiplies, which is that flag's
 * signature. From the 123-word inherited candidate, what closed it: the
 * flag (122 -> 18 once the shape below was in place), the parallel test as a
 * difference of the two products compared with zero and the sign test
 * against an integer zero (both keep the denominator as its own value
 * instead of a compare of the two products), one rounded numerator per
 * coordinate instead of two divides each, and the locals in this order (the
 * last one moves every home and spill cell down by one). */
s32 func_overlay_002_F0001364_185815C(f32 x1, f32 y1, f32 x2, f32 y2,
                                      f32 x3, f32 y3, f32 x4, f32 y4,
                                      f32 *x, f32 *y) {
    f32 side3;
    f32 side4;
    f32 a1;
    f32 b1;
    f32 c1;
    f32 a2;
    f32 side1;
    f32 b2;
    f32 c2;
    f32 side2;
    f32 denom;
    f32 offset;
    f32 numeratorX;
    f32 numeratorY;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = (x2 * y1) - (x1 * y2);
    side3 = (a1 * x3) + (b1 * y3) + c1;
    side4 = (a1 * x4) + (b1 * y4) + c1;
    if (side3 && side4 &&
        (((side3 > 0.0f) && (side4 > 0.0f)) ||
         ((side3 < 0.0f) && (side4 < 0.0f)))) {
        return 0;
    }

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = (x4 * y3) - (x3 * y4);
    side1 = (a2 * x1) + (b2 * y1) + c2;
    side2 = (a2 * x2) + (b2 * y2) + c2;
    if (side1 && side2 &&
        (((side1 > 0.0f) && (side2 > 0.0f)) ||
         ((side1 < 0.0f) && (side2 < 0.0f)))) {
        return 0;
    }

    side1 = a2 * b1;
    side2 = a1 * b2;
    denom = side2 - side1;
    if (denom == 0.0f) {
        *x = x1;
        *y = y1;
        return 2;
    }

    if (x != NULL) {
        if (denom < 0) {
            offset = -denom * 0.5f;
        } else {
            offset = denom * 0.5f;
        }
        side1 = b1 * c2;
        side2 = b2 * c1;
        if (side1 < side2) {
            numeratorX = (side1 - side2) - offset;
        } else {
            numeratorX = (side1 - side2) + offset;
        }
        *x = numeratorX / denom;
        side1 = a2 * c1;
        side2 = a1 * c2;
        if (side1 < side2) {
            numeratorY = (side1 - side2) - offset;
        } else {
            numeratorY = (side1 - side2) + offset;
        }
        *y = numeratorY / denom;
    }

    return 1;
}

/* DKR v77/v80 and JFG have no exact donor for this circular-index helper. */
static void overlay2AdjacentIndices(Overlay2Shape *collection, u16 index,
                             s16 *previous, s16 *next) {
    *previous = index - 1;
    *next = index + 1;
    if (index == 0) {
        *previous = collection->pointCount - 1;
        return;
    }
    if (index == collection->pointCount - 1) {
        *next = 0;
    }
}

/* Mickey-local reconstruction; pinned DKR/JFG object scans found no donor.
 *
 * Matched by writing the natural shape with no carriers at all: the leaf scan
 * is `while (remaining--)`, the leaf result, the child selection and the
 * all-hits union are plain expressions, and the first-hit case is `||`. The
 * function returns `int`, not `s32`: with a `long` result the `||` value is
 * converted through a named temporary, which is what every earlier carrier
 * shape was imitating. One unreferenced leading local supplies the frame cell
 * above the scan counter.
 */
static int overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1, Overlay2Node *node) {
    s32 pad;
    s32 remaining;
    s32 leafResult;
    f32 hitX;
    f32 hitY;
    Overlay2Line *line;

    if (node->type == 1) {
        leafResult = 0;
        remaining = node->data.leaf.count;
        line = &D_3C[node->index];
        while (remaining--) {
            if (func_overlay_002_F0001364_185815C_o002Reloc(D_40, D_44, D_48, D_4C,
                                               line->x1, line->y1,
                                               line->x2, line->y2,
                                               &hitX, &hitY) != 0) {
                if (((((D_40 - hitX) * (D_40 - hitX)) +
                      ((D_44 - hitY) * (D_44 - hitY))) > 1.0f) &&
                    ((((D_40 - hitX) * (D_40 - hitX)) +
                      ((D_44 - hitY) * (D_44 - hitY))) < D_6C)) {
                    D_6C = ((D_40 - hitX) * (D_40 - hitX)) +
                           ((D_44 - hitY) * (D_44 - hitY));
                    D_64 = hitX;
                    D_68 = hitY;
                    D_70 = line->value1;
                    D_72 = line->value2;
                }
                if (D_60 != 0) {
                    leafResult = 1;
                } else {
                    return 1;
                }
            }
            line++;
        }
        return (D_60 != 0) ? leafResult : 0;
    }

    D_50 = ((node->index == 0) ? y0 : x0) < node->data.boundary;
    D_54 = ((node->index == 0) ? y1 : x1) < node->data.boundary;

    if (D_50 == D_54) {
        return overlay2QueryNode(x0, y0, x1, y1,
                                 (D_50 != 0) ? node->side1 : node->side0);
    }

    D_30 = node->index;
    D_34 = node->data.boundary;
    overlay2IntersectBoundary(x0, y0, x1, y1, &D_58, &D_5C);

    if (D_50 != 0) {
        if (D_60 != 0) {
            return overlay2QueryNode(x0, y0, D_58, D_5C, node->side1) |
                   overlay2QueryNode(D_58, D_5C, x1, y1, node->side0);
        }
        return overlay2QueryNode(x0, y0, D_58, D_5C, node->side1) ||
               overlay2QueryNode(D_58, D_5C, x1, y1, node->side0);
    }

    if (D_60 != 0) {
        return overlay2QueryNode(x0, y0, D_58, D_5C, node->side0) |
               overlay2QueryNode(D_58, D_5C, x1, y1, node->side1);
    }
    return overlay2QueryNode(x0, y0, D_58, D_5C, node->side0) ||
           overlay2QueryNode(D_58, D_5C, x1, y1, node->side1);
}

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
    Overlay2Shape *baseEntry;
    Overlay2Shape *hitEntry;
    Overlay2Point *point;

    baseEntry = overlay1GetEntry(shapeIndex);
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

        hitEntry = overlay1GetEntry(D_72);
        overlay2AdjacentIndices(hitEntry, hit->index,
                                &hit->previous, &hit->next);

        if (previousIndex != -1) {
            point = &baseEntry->points[previousIndex];
            if (point->x == (s16)gOverlay2QueryX0Reloc) {
                if (point->y == (s16)gOverlay2QueryY0Reloc) {
                    if (overlay2ContainsPoint_o002Reloc(
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
            if (overlay2ContainsPoint_o002Reloc(
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

/* Mickey-only reconstruction. The closest permitted reference skeleton is
 * too weak to establish a donor body (masked 4-gram Jaccard 0.077).
 *
 * Matched from 198 masked words by writing it with fourteen plain locals and
 * early exits. One `candidate` is the scan variable of both loops and of the
 * renumbering loop (the earlier split carrier cost a fifteenth frame cell);
 * the second scan rejects with one `continue`, which is what orders index
 * after the renumbering cursor and the indices base ahead of the unroller's
 * bound; `closestIndex` is reused for the closest object's slot,
 * `closestRoute` and `candidateRoute` for the renumbering loop's two routes;
 * the nonzero group returns early, so the chained group copy reloads route;
 * and the renumbering start sits in the for-init, which keeps the copy after
 * the cursor setup. */
void func_overlay_002_F0001DF8_1858BF0(Overlay2RouteObject *object,
                                        Overlay2RouteInput *input) {
    s32 start;
    s32 end;
    s32 index;
    Overlay2RouteObject **objects;
    u16 indices[0x400];
    Overlay2RouteObject *candidate;
    Overlay2RouteObject *closest;
    Overlay2RouteObject *previous;
    Overlay2RouteObject *next;
    Overlay2RouteState *closestRoute;
    Overlay2RouteState *candidateRoute;
    Overlay2RouteState *route;
    Overlay2ObjectHeader *header;
    s32 closestIndex;
    s32 unused;
    u32 bestDistance;
    u32 distance;
    u32 previousDistance;
    s32 count;

    objects = func_8000572C(&start, &end);
    route = object->route;
    closestIndex = -1;
    bestDistance = (u32)-1;
    route->group = input->group;
    count = 0;
    route->order = input->order;

    if (input->group < 2) {
        for (index = start; index < end; index++) {
            candidate = objects[index];
            if ((candidate->disabled == 0) && (candidate != object) &&
                (candidate->type == 0x2B)) {
                distance = (u32)func_8000BCB0(
                    object->x, object->y, object->z, candidate->x,
                    candidate->y, candidate->z);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    closestIndex = index;
                }
            }
        }

        if (closestIndex != -1) {
            closest = objects[closestIndex];
            closestRoute = closest->route;
            for (index = start; index < end; index++) {
                candidate = objects[index];
                candidateRoute = candidate->route;
                if ((candidate->disabled != 0) || (candidate == object) ||
                    (candidate->type != 0x2B)) {
                    continue;
                }
                if (candidateRoute->group == closestRoute->group) {
                    count++;
                    indices[candidateRoute->order] = (u16)index;
                }
            }
        } else {
            input->group = 1;
            route->group = 1;
            return;
        }

        if (route->group != 0) {
            return;
        }

        route->group = input->group = closestRoute->group;
        if (count == 1) {
            route->order = input->order = 1;
            return;
        }
        if (count == 2) {
            route->order = input->order = 2;
            return;
        }

        if (joyGetButtons(0) & 0x100) {
            /* The closest object's slot in the group's order. */
            closestIndex = closest->header->order;
            if (closestIndex == 0) {
                previous = objects[indices[count - 1]];
                next = objects[indices[1]];
            } else if (closestIndex == count - 1) {
                previous = objects[indices[count - 2]];
                next = objects[indices[0]];
            } else {
                previous = objects[indices[closestIndex - 1]];
                next = objects[indices[closestIndex + 1]];
            }

            previousDistance = (u32)func_8000BCB0(
                previous->x, previous->y, previous->z, candidate->x,
                candidate->y, candidate->z);
            distance = (u32)func_8000BCB0(
                next->x, next->y, next->z, candidate->x, candidate->y,
                candidate->z);
            if (distance < previousDistance) {
                previous = closest;
            }

            closestRoute = previous->route;
            for (index = count - 1; closestRoute->order < index; index--) {
                candidate = objects[indices[index]];
                candidateRoute = candidate->route;
                header = candidate->header;
                candidateRoute->order++;
                header->order++;
            }
            route->order = input->order = closestRoute->order + 1;
            return;
        }

        input->order = (u8)count;
        route->order = (u8)count;
    }
}

/* DKR v77/v80 and JFG checks found only generic three-byte color copies. */
void overlay2CopyColor(Overlay2ColorObject *object, u8 *color) {
    u8 *state = object->state;

    state[0] = color[0xA];
    state[1] = color[0xB];
    state[2] = color[0xC];
}
