#include "overlays/overlay019.h"

#define O19_LOAD_SPAN_FIELDS \
    firstItem = ((O19Span *)((u8 *)group->spans + frame.offset.spanOffset))->itemStart; \
    itemEnd = (((O19Span *)((u8 *)group->spans + frame.offset.spanOffset)) + 1)->itemStart; \
    vertexBase = ((O19Span *)((u8 *)group->spans + frame.offset.spanOffset))->vertexBase; \
    bit = ((O19Span *)((u8 *)group->spans + frame.offset.spanOffset))->flags
#define O19_ADD_REVERSED(a, b) ((b) + (a))
#define O19_ADVANCE_SPAN \
    frame.index.spanIndex = frame.index.vSpanIndex + 1; \
    frame.offset.spanOffset += sizeof(O19Span)

typedef struct O19SpatialMaskFrame {
    union {
        s32 spanOffset;
        volatile s32 vSpanOffset;
    } offset;
    u32 suppressed;
    u8 unused38[0x30];
    union {
        s32 spanIndex;
        volatile s32 vSpanIndex;
    } index;
    u8 unused6C[0x14];
} O19SpatialMaskFrame;

/* s1-b (2026-09-16): a fresh s16 bin counter (numbered between binEnd and
 * binStart, 9300/5 like both), an xMax probe in the vertex loop (4100/8 puts
 * xMax ahead of xMin) and the two loop-1 inits on one physical line: 63 to 17. */
/* s2-a (2026-09-16): the span fields are read through the expression itself
 * via the NON-volatile union member (one CSE'd load), so the base is a type-4
 * web numbered after firstItem's type-3 web and takes a0 where a declared
 * `span` took v1: 17 to 9. */
/* lm-o019 (2026-09-17): loop 1's tail is as1 lineno. bit <<= on its own
 * line; #line 49 on binIndex++; #line 48 on the trailing ;. 9 to 2. */
/* w2-o019 (2026-09-17): ugen still emits mask=0 (t0) then selector=0 (a2).
 * Comma, 0|0, xor-self, assign-as-if, last-use, literal type and earlier
 * def do not emit a2 first at unchanged colours; reversing statements
 * recolours selector off a2 and still emits t0 first (L87 lock). */
#ifdef NON_MATCHING
void overlay19BuildSpatialMasks(O19Context *context, O19Group *group, O19Output *output) {
    O19SpatialMaskFrame frame; s32 item, itemEnd, selector; s16 vertexBase; O19Span *span; O19Point *point; O19Vertex *vertices, *vertex; s16 x, y, z, xMax, xMin, yMax, yMin, zMax, zMin; s16 spanCount, lower, upper, step, binStart, binEnd, firstItem, binIndex; u32 bit, mask; ;
    frame.index.spanIndex = 0; spanCount = group->spanCount; if ((spanCount > 0)) { ; frame.offset.spanOffset = 0; do { O19_LOAD_SPAN_FIELDS;
    if ((firstItem < itemEnd)) { ; item = firstItem; ; frame.suppressed = bit & 0x1080; do { ; xMax = -32000; yMax = -32000; zMax = -32000; if (frame.suppressed != 0) output->masks[item] = 0; else { ; xMin = 32000; if (xMin); yMin = 32000; zMin = 32000; mask = 0; if (mask); selector = 0; if (selector); point = &group->points[item]; vertices = context->vertices;
    do { vertex = (O19Vertex *)((u8 *)vertices + (u32)(point->selectors[selector] + vertexBase) * 10); x = vertex->x; y = vertex->y; z = vertex->z; if ((xMax < x)) { ; xMax = x; } if ((x < xMin)) { ; xMin = x; } if (yMax < y) yMax = y; if (y < yMin) yMin = y; if (zMax < z) zMax = z; if (z < zMin) zMin = z; if (xMax); selector++; } while (selector != 3); ; bit = 1;
    lower = group->xLower;
    upper = group->xUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    binIndex = 0; binStart = lower;

    ;
    do {
        if (!(binEnd < xMin || xMax < binStart)) { ; mask |= bit; }
        binEnd += step; binStart += step;
        bit <<= 1;
#line 49
        binIndex++;
#line 48
        ;
#line 53
    } while (binIndex < 8);

    ;
    lower = group->zLower;
    upper = group->zUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    binStart = lower;
    for (binIndex = 0; binIndex < 8; binIndex++) {
        if (!(binEnd < zMin || zMax < binStart)) { ; mask |= bit; }
        binEnd += step;
        binStart += step;
        bit <<= 1;
        ;
    }
    lower = group->yLower;
    upper = group->yUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    binStart = lower;
    for (binIndex = 0; binIndex < 8; binIndex++) {
        if (!(binEnd < yMin || yMax < binStart)) { ; mask |= bit; }
        binEnd += step;
        binStart += step;
        bit <<= 1;
        ;
    }
    ; output->masks[item] = mask; ; } item++; ; } while (item < itemEnd);
    spanCount = ((volatile O19Group *)group)->spanCount; }
    O19_ADVANCE_SPAN;
    } while ((frame.index.spanIndex < spanCount)); } }
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o019/overlay19BuildSpatialMasks/func_overlay_019_F0000F58_18761B0.s")
#endif

/* PLATEAU-HANDOFF:overlay19BuildSpatialMasks:start
 * symbol: overlay19BuildSpatialMasks
 * score: 2/227 words
 * frame: 0x80
 * relocations: 0
 * first-mismatch: +0xC0
 * summary: ugen emits mask=0 (t0) then selector=0 (a2); no tested C form emits a2 first at unchanged colours (L87 lock).
 * PLATEAU-HANDOFF:overlay19BuildSpatialMasks:end
 */
