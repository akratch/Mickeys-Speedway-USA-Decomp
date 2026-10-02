#include "overlays/overlay_001.h"

/* ---- overlay1InitMotionScale ---- */

typedef struct O1Point2 { f32 x; f32 y; } O1Point2;
typedef struct O1Reference { u8 pad00[0xC]; f32 x; u8 pad10[4]; f32 y; } O1Reference;
typedef struct O1MotionWorld { u8 pad00[0x37C]; s16 angle; u8 pad37E[0x1A]; f32 scale; f32 heading; } O1MotionWorld;
extern O1Point2 *D_20C;
extern O1Point2 *D_210;
extern f32 overlay1SquareRoot(f32 value);
extern s32 overlay1AngleFromIndex(s16 value);
void overlay1InitMotionScale(void) {
    f32 dx;
    f32 dy;
    f32 firstDistance;
    f32 secondDistance;
    dx = D_20C->x - D_210->x;
    dy = D_20C->y - D_210->y;
    firstDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    dx = D_20C->x - ((O1Reference *)D_1D9C)->x;
    dy = D_20C->y - ((O1Reference *)D_1D9C)->y;
    secondDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    ((O1MotionWorld *)D_1DA0)->scale = secondDistance / firstDistance;
    ((O1MotionWorld *)D_1DA0)->heading =
        (f32)overlay1AngleFromIndex(((O1MotionWorld *)D_1DA0)->angle) +
        ((O1MotionWorld *)D_1DA0)->scale;
}

/* ---- overlay1InterpolatePath ---- */


typedef struct O1ControlPoint { f32 x; f32 z; u8 pad08[8]; } O1ControlPoint;
typedef struct O1ControlTable { u8 pad00[0x14]; O1ControlPoint points[1]; } O1ControlTable;
typedef struct O1PathOffsetOwner { u8 pad00[0x398]; f32 pathOffset; } O1PathOffsetOwner;

extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D6C;
/* Overlay 1's own `overlay1NextPointer` (module offset 0x28), reached through
 * the module's SYMBOL relocation record rather than an intra-module JUMP:
 * the shipped word stores immediate zero and `runlinkDownloadCode` supplies
 * the target, so the reference has to stay undefined in this object. */
extern u8 *overlay1NextPointerReloc(u8 *pointer);
extern f32 splinePos(f32 a, f32 b, f32 c, f32 d, f32 t);

/* The last two words were one spill slot: `originalWhole` homed at sp+0x38
 * against the target's sp+0x40, with all 83 instructions, the 0x68 frame and
 * every allocator lane already identical.  uopt's `spilltemps` lays each
 * register temporary at `frame_top - 4*(k+1)` for its slot index k, so the
 * home is a function of how many pooled temporaries precede it, and no
 * declaration permutation moves it -- all 90 were flat, and all 196 legal
 * statement orders reach only the two adjacent slots 0x38 and 0x3C.  Two edits
 * compose (neither works alone, L88): computing the integral position before
 * the four control-point addresses gives its web the earlier of the two slots,
 * and spelling the fraction as the expression at both call sites instead of a
 * tenth declared local removes one cell from the pool ahead of it, which lifts
 * the pair by four bytes onto sp+0x40 and sp+0x3C.  Each declared local costs
 * exactly one pool cell here: adding an unused one moves every home down by
 * one slot and the frame to 0x70. */
void overlay1InterpolatePath(f32 *outX, f32 *outZ, s32 path, f32 offset) {
    f32 position;
    O1ControlTable *table3Base;
    O1ControlPoint *point1;
    O1ControlPoint *point0;
    O1ControlPoint *point2;
    O1ControlPoint *point3;
    s32 originalWhole;
    s32 whole;
    s32 remaining;

    position = ((O1PathOffsetOwner *)D_1DA0)->pathOffset + offset;
    whole = (s32) position;
    originalWhole = whole;
    point0 = &D_1D60->points[path];
    point1 = &((O1ControlTable *)D_1D64)->points[path];
    point2 = &D_1D68->points[path];
    point3 = &D_1D6C->points[path];
    table3Base = D_1D6C;
    remaining = whole - 1;

    if (whole != 0) {
        do {
            table3Base = (O1ControlTable *)overlay1NextPointerReloc((u8 *)table3Base);
            point0 = point1;
            point1 = point2;
            point2 = point3;
            point3 = &table3Base->points[path];
            whole = remaining;
            remaining--;
        } while (whole != 0);
    }

    *outX = splinePos(point0->x, point1->x, point2->x,
                                  point3->x, position - (f32) originalWhole);
    *outZ = splinePos(point0->z, point1->z, point2->z,
                                  point3->z, position - (f32) originalWhole);
}

/* ---- overlay1ResolveMotionPoint ---- */

typedef struct O1PathOwner { s16 angle; u8 pad02[0xA]; f32 x; f32 y; f32 z; } O1PathOwner;
extern s32 D_0;
extern f32 D_B4;
extern f32 D_B8;
extern s32 overlay1ActivateObject(void *owner);
extern void overlay1InterpolatePath(f32 *x, f32 *z, s32 path, f32 offset);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);
/* Assigning the measured distance through scale preserves the original FP
 * carrier web; the configured build emits all 100 instruction words exactly. */
void overlay1ResolveMotionPoint(O1PathOwner *owner, s32 path, f32 *outX,
                                f32 *outY, f32 *outZ) {
    f32 dx;
    f32 dz;
    f32 distance;
    f32 scale;
    if (overlay1ActivateObject(owner) == 0) {
        *outX = 0.0f;
        *outY = 0.0f;
        *outZ = 0.0f;
        return;
    }
    if (D_0 == 1) {
        overlay1InterpolatePath(outX, outZ, path, 1.0f);
        dx = *outX - owner->x;
        dz = *outZ - owner->z;
        distance = sqrtf((dx * dx) + (dz * dz));
        scale = distance;
        if (scale > 0.0f) {
            scale = 1.0f / scale;
            dx *= scale;
            dz *= scale;
        }
        *outX = owner->x + (dx * 150.0f);
        *outY = owner->y + D_B4;
        *outZ = owner->z + (dz * 150.0f);
    } else {
        *outX = owner->x + (func_8002A8C0(owner->angle) * 150.0f);
        *outY = owner->y + D_B8;
        *outZ = owner->z + (func_8002A8BC(owner->angle) * 150.0f);
    }
}

/* ---- overlay1MeasureCurves ---- */

extern f32 overlay1EvaluateCurve(f32, f32, s32, s32, f32);
extern f32 overlay1SquareRoot(f32);
/* Matched 2026-09-12. The residual was never statement shape: it was the
 * parameter model. The four integer controls have to be reloaded from their
 * argument homes at every call WITHOUT carrying volatile scheduling edges,
 * and `volatile` gives the reloads and the edges together -- as1's trace
 * chains every volatile reference in a block to the next, which pinned the
 * call-result copy fourth where the target places it first. Taking each
 * control's address instead makes it memory class, so every read is a load
 * from its home, and no edge is emitted; the copy then ties at aftercycles 0
 * with the four loads and the block collapses to ugen's emission order, which
 * is the target's. The same `*(s32 *)&x` bit-pattern idiom is used by
 * overlay1FindClosestSample in the sibling TU. Declaring the four controls
 * `f32` and passing their bit patterns is byte-identical, so the bytes do not
 * decide the parameter type; the narrower s32 reading is kept. */
f32 overlay1MeasureCurves(f32 startX, f32 startY,
                          f32 endX, f32 endY,
                          s32 controlX1, s32 controlY1,
                          s32 controlX2, s32 controlY2,
                          s32 segmentCount) {
    f32 t = 0.0f;
    volatile f64 unusedLocal;
    volatile f32 total = 0.0f;
    f32 previousX = overlay1EvaluateCurve(startX, endX, *(s32 *)&controlX1, *(s32 *)&controlX2, 0.0f);
    f32 previousY = overlay1EvaluateCurve(startY, endY, *(s32 *)&controlY1, *(s32 *)&controlY2, 0.0f);
    f32 step;
    f32 x;
    f32 y;
    f32 dx;
    f32 dy;
    s32 remaining = segmentCount - 1;
    if (segmentCount != 0) {
        step = 1.0f / (f32)segmentCount;
        do {
            t += step;
            x = overlay1EvaluateCurve(startX, endX, *(s32 *)&controlX1, *(s32 *)&controlX2, t);
            y = overlay1EvaluateCurve(startY, endY, *(s32 *)&controlY1, *(s32 *)&controlY2, t);
            dx = x - previousX;
            dy = y - previousY;
            total += overlay1SquareRoot((dx * dx) + (dy * dy));
            previousX = x;
            previousY = y;
        } while (remaining--);
    }
    return total;
}

/* ---- overlay1Noop ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor; this is a generic no-op. */
void overlay1Noop(void) {
}

/* ---- overlay1LoadBuildRecords ---- */


/* Canonical typed owner of Overlay 1 text +0x10C8..+0x19B8. */

typedef struct Overlay1PackedRecord {
    s16 type;
    u8 size;
    u8 pad03;
    s16 value4;
    u8 pad06[2];
    s16 value8;
    u8 group;
    u8 slot;
    union {
        u16 index;
        u8 byte;
    } link;
} Overlay1PackedRecord;

typedef struct Overlay1Point {
    s16 first;
    s16 second;
} Overlay1Point;

typedef struct Overlay1Group {
    Overlay1Point *points;
    s32 count;
    struct Overlay1Group *previous;
    struct Overlay1Group *next;
    s32 selector;
    s32 field14;
    s32 field18;
} Overlay1Group;

typedef struct Overlay1Metric {
    f32 x;
    f32 y;
    f32 score;
    s8 rank;
    u8 pad0D[3];
} Overlay1Metric;

typedef struct Overlay1LargeRecord {
    u8 pad00[0x14];
    Overlay1Metric metrics[8];
} Overlay1LargeRecord;

typedef struct Overlay1MetricCursor {
    u8 pad00[0x14];
    Overlay1Metric metric;
} Overlay1MetricCursor;

extern void overlay1LoadPackedRecordsReloc(
    Overlay1PackedRecord **records, s32 *size, s32 resource);
extern s32 D_1D7C;
extern s32 D_1D80;
extern s32 D_1D8C;
extern s32 D_0;
extern s32 D_1D90;
extern s32 D_1DBC;
extern s32 D_1D98;
extern s32 D_1D98Read;
extern u8 gOverlay1RankBase;
extern u8 gOverlay1RankLimit;
extern s32 gOverlay1RankDelta;
extern s32 gOverlay1ModeConstant;
extern u8 gOverlay1ConfigMode;
extern f32 D_BC;
extern f32 D_C0;
extern f32 D_C4;
extern f32 D_C8;
extern f32 D_CC;
extern f32 D_D0;
extern f32 D_D4;
extern f32 D_1DAC;
extern f32 D_1DB0;
extern f32 D_1DB4;
extern f32 D_1DB8;
extern u8 D_0_Clear[];
extern Overlay1Group *D_1BA0;
extern Overlay1Point *D_1D70;
extern void *D_1BA4;
extern void overlay1ReleaseBuildMemoryReloc(void *memory);
extern void *overlay1AllocateBuildMemoryReloc(s32 size, s32 tag);
extern void overlay1RejectBuildCycleReloc(Overlay1Group *group);
extern void overlay1FinalizeBuildGroupReloc(Overlay1Group *group);
extern void *overlay1SubmitBuildReloc(s32 value);
extern Overlay1LargeRecord *D_1D58;
extern Overlay1LargeRecord *D_1D58Read;
extern Overlay1LargeRecord *D_1D5C;
extern u8 *gOverlay1MissingLargeContext;
extern f32 D_1DA8;
extern u8 D_1DCC[];
extern void overlay1ClearLargeRecordsReloc(void *records, s32 size);
extern void overlay1DecodeLargeRecordReloc(
    Overlay1LargeRecord *record, Overlay1PackedRecord *packed, s32 mode);
extern void overlay1ReportMissingLargeReloc(s32 value, s32 type, s32 severity);
extern Overlay1LargeRecord *overlay1GetMetricSourceAReloc(
    Overlay1LargeRecord *record);
extern Overlay1LargeRecord *overlay1GetMetricSourceBReloc(
    Overlay1LargeRecord *record);
extern Overlay1LargeRecord *overlay1GetMetricSourceCReloc(
    Overlay1LargeRecord *record);
extern f32 func_overlay_001_F0000F84_184D364(
    f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3,
    s32 scale);

/* 2026-10-02 x-o058: rewritten in the overlay's while (i--) / record-walk
 * shape from the listing: 469 masked at -52 bytes -> 20 at size delta 0,
 * frame 0xD8 exact. The notes below the body (inside the guard, standing in
 * for the line padding that keeps later functions on their numbers) say how. */
#ifdef NON_MATCHING
extern s32 gO1Finishers;
extern s32 G_o1_83e0;
extern s32 G_o1_83e4;
extern u8 gO1PlayerCount;
extern u8 gO1PlayerBase;
extern u8 gO1RankOrder[];
extern s32 gOverlay1Data[];
extern u8 D_1DC8[];
void overlay1LoadBuildRecords(void) {
    Overlay1LargeRecord *large;
    Overlay1PackedRecord *records;
    s32 size;
    Overlay1PackedRecord *record;
    Overlay1PackedRecord *entry;
    Overlay1PackedRecord *node;
    s32 offset;
    s32 i;
    s32 j;
    s32 k;
    s32 value;
    Overlay1LargeRecord *sourceA;
    Overlay1LargeRecord *sourceB;
    Overlay1LargeRecord *sourceC;
    Overlay1Group *group;
    Overlay1Group *link;
    Overlay1Point *point;
    f32 score;
    f32 maximum;
    f32 minimum;
    f32 scale;
    f32 total;

    overlay1LoadPackedRecordsReloc(&records, &size, 1);
    D_1D7C = 0;
    D_1D80 = 0;
    D_1D8C = 0;
    gO1Finishers = 0;
    D_1D90 = 0;
    D_1DBC = 0;
    D_1D98 = 0;
    G_o1_83e0 = gO1PlayerCount - gO1PlayerBase;
    G_o1_83e4 = 3;
    switch (gOverlay1ConfigMode) {
    case 2:
        D_1DAC = 1.0f;
        D_1DB0 = 1.0f;
        D_1DB4 = 1.25f;
        D_1DB8 = 0.25f;
        break;
    case 1:
        D_1DAC = 0.8f;
        D_1DB0 = 0.98f;
        D_1DB4 = 1.15f;
        D_1DB8 = 0.5f;
        break;
    case 0:
        D_1DAC = 0.65f;
        D_1DB0 = 0.9f;
        D_1DB4 = 1.1f;
        D_1DB8 = 0.75f;
        break;
    }
    i = 6;
    while (i--) {
        gO1RankOrder[i] = 0xFF;
    }

    for (offset = 0, record = records; offset < size; offset += record->size, record = (Overlay1PackedRecord *)((u8 *)record + record->size)) {
        if (record->type == 0xC8) {
            D_1D7C++;
            if (D_1D80 < record->group + 1) {
                D_1D80 = record->group + 1;
            }
        }
    }

    if (D_1D80 == 0) {
        goto large;
    }
    {
        if (D_1BA0 != NULL) {
            overlay1ReleaseBuildMemoryReloc(D_1BA0);
        }
        if (D_1D70 != NULL) {
            overlay1ReleaseBuildMemoryReloc(D_1D70);
        }
        D_1BA0 = overlay1AllocateBuildMemoryReloc(D_1D80 * sizeof(Overlay1Group), 0x85);
        D_1D70 = overlay1AllocateBuildMemoryReloc(D_1D7C * sizeof(Overlay1Point), 0x85);
        k = D_1D80;
        while (k--) {
            D_1BA0[k].points = NULL;
            D_1BA0[k].count = 0;
            D_1BA0[k].previous = NULL;
            D_1BA0[k].next = NULL;
            D_1BA0[k].selector = 0;
            D_1BA0[k].field14 = 0;
            D_1BA0[k].field18 = 0;
        }
        for (offset = 0, entry = records; offset < size; offset += entry->size, entry = (Overlay1PackedRecord *)((u8 *)entry + entry->size)) {
            if (entry->type == 0xC8) {
                D_1BA0[entry->group].count++;
            }
        }
        for (j = 0, k = 0; k < D_1D80; k++) {
            D_1BA0[k].points = &D_1D70[j];
            j += D_1BA0[k].count;
        }
        for (offset = 0, entry = records; offset < size; offset += entry->size, entry = (Overlay1PackedRecord *)((u8 *)entry + entry->size)) {
            if (entry->type == 0xC8) {
                group = &D_1BA0[entry->group];
                point = &group->points[entry->slot];
                point->first = entry->value4;
                point->second = entry->value8;
            }
        }
        for (offset = 0, node = records; offset < size; offset += node->size, node = (Overlay1PackedRecord *)((u8 *)node + node->size)) {
            if (node->type == 0xC9) {
                group = &D_1BA0[node->group];
                group->selector = node->slot;
                if (node->link.byte != 0) {
                    link = &D_1BA0[node->link.byte];
                    while (link->next != NULL) {
                        link = link->next;
                    }
                    if (link == group) {
                        overlay1RejectBuildCycleReloc(link);
                    } else {
                        link->next = group;
                        group->previous = link;
                    }
                }
            }
        }
        j = D_1D80 - 1;
        group = &D_1BA0[1];
        while (j-- > 0) {
            if (group->previous == NULL) {
                overlay1FinalizeBuildGroupReloc(group);
            }
            group++;
        }
        D_1BA4 = overlay1SubmitBuildReloc(1);
        return;
    }
large:
    {
        for (offset = 0, record = records; offset < size; offset += record->size, record = (Overlay1PackedRecord *)((u8 *)record + record->size)) {
            if (record->type == 0xCA) {
                if (D_1D8C < record->link.index + 1) {
                    D_1D8C = record->link.index + 1;
                }
            }
        }
        value = D_1D8C * sizeof(Overlay1LargeRecord);
        if (value != 0) {
            D_1D58 = overlay1AllocateBuildMemoryReloc(value, 0x85);
            overlay1ClearLargeRecordsReloc(D_1D58, value);
            for (offset = 0, record = records; offset < size; offset += record->size, record = (Overlay1PackedRecord *)((u8 *)record + record->size)) {
                if (record->type == 0xCA) {
                    overlay1DecodeLargeRecordReloc(&D_1D58[record->link.index], record, 0);
                }
            }
        } else {
            D_1D58 = NULL;
        }
        if (D_1D58 != NULL) {
            G_o1_83e4 = 1;
        } else {
            G_o1_83e4 = 0;
            overlay1ReportMissingLargeReloc(gOverlay1Data[0], 2, 2);
            return;
        }
        D_1D5C = &D_1D58[D_1D8C - 1];
        for (offset = 0, record = records; offset < size; offset += record->size, record = (Overlay1PackedRecord *)((u8 *)record + record->size)) {
            if (record->type == 0xCA) {
                large = &D_1D58[record->link.index];
                maximum = 0.0f;
                minimum = 3.4028235e38f;
                sourceA = overlay1GetMetricSourceAReloc(large);
                sourceB = overlay1GetMetricSourceBReloc(sourceA);
                sourceC = overlay1GetMetricSourceCReloc(large);
                i = 8;
                while (i--) {
                    score = func_overlay_001_F0000F84_184D364(
                        sourceC->metrics[i].x, sourceC->metrics[i].y,
                        large->metrics[i].x, large->metrics[i].y,
                        sourceA->metrics[i].x, sourceA->metrics[i].y,
                        sourceB->metrics[i].x, sourceB->metrics[i].y, 0x10);
                    large->metrics[i].score = score;
                    if (large->metrics[i].rank != 0) {
                        if (maximum < score) {
                            maximum = score;
                        }
                        if (score < minimum) {
                            minimum = score;
                        }
                    }
                }
                if (maximum != minimum) {
                    scale = 51.0f / (maximum - minimum);
                    i = 8;
                    while (i--) {
                        if (large->metrics[i].rank != 0) {
                            large->metrics[i].rank = (f32)large->metrics[i].rank + (maximum - large->metrics[i].score) * scale;
                            if (i == 3) {
                                large->metrics[i].rank += 5;
                            }
                        }
                    }
                }
            }
        }
        total = 0.0f;
        large = D_1D58;
        i = D_1D8C;
        while (i--) {
            total += large->metrics[3].score;
        }
        D_1DA8 = total / (f32)D_1D8C;
        i = 5;
        while (i--) {
            D_1DC8[i] = 0;
        }
    }
}
/*
 * What moved it, measured with tools/fast_score.py on this TU:
 *  - identities from the runtime relocation table: the "rank delta" and
 *    "mode constant" are G_o1_83e0 and G_o1_83e4 (SYMBOL records into this
 *    overlay's BSS), D_0 is resident gO1Finishers, the 0xFF clear is
 *    gO1RankOrder[0..5], the last clear is D_1DC8[0..4], the report's
 *    argument is the word at this overlay's .data +0, and D_BC..D_D4 are
 *    rodata literals (0.8, 0.98, 1.15, 0.65, 0.9, 1.1, FLT_MAX);
 *  - a switch on the config mode, every loop in the while (i--) idiom or a
 *    plain record walk with no do/while guard copies: 469 at -52 -> 450 at -16;
 *  - the group-less path is reached by goto (the shipped layout jumps over
 *    the group code from a block holding only the records reload): -> 298 at
 *    -12; the missing-large test is an if/else whose else returns: -> 181 at -4;
 *  - the closing average reads D_1D58 once through a pointer local, so the
 *    score load hoists out of its loop: -> 144 at size delta 0;
 *  - one variable per loop role (k for the clear and points index, j for the
 *    point index and the finalize count, entry for the two count/point walks,
 *    node for the link walk): -> 49, frame 0xD8;
 *  - declaration order putting the homes where shipped: -> 20.
 * Open (20): the two running-maximum stores rematerialise their address in
 * the target (lui at) instead of using the held register; the rank-adjust
 * and average float registers are one ring position off; the FLT_MAX load
 * and the large pointer argument trade places around the first source call.
 */



















































#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F00010C8_184D4A8.s")
#endif

/*
 * The adopted clear is four source lines shorter than the pointer walk.
 * These lines keep later matched functions on their original numbers.
 */
/* ---- overlay1InitializeModeState ---- */


typedef struct Overlay1ModeInput {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1ModeInput;

typedef struct Overlay1ModeSource {
    u8 pad00[0xE];
    u16 value;
} Overlay1ModeSource;

typedef struct Overlay1ModeState {
    u8 pad00[0x37C];
    s16 value;
    u8 mode;
    u8 previousMode;
    u8 phase;
    u8 pad381[0x17];
    s32 selector;
} Overlay1ModeState;

extern void func_overlay_001_F0000BD4_184CFB4(void);
extern u8 func_overlay_001_F0000614_184C9F4(
    f32 x, f32 y, Overlay1ModeSource *source, s32 selector);

void overlay1InitializeModeState(s32 value) {
    ((Overlay1ModeState *)D_1DA0)->value = value;
    ((Overlay1ModeState *)D_1DA0)->mode = 3;
    func_overlay_001_F0000BD4_184CFB4();
    ((Overlay1ModeState *)D_1DA0)->pad381[0x31] =
        (u8)((Overlay1ModeSource *)D_1D64)->value;
    ((Overlay1ModeState *)D_1DA0)->mode =
        func_overlay_001_F0000614_184C9F4(
            ((Overlay1ModeInput *)D_1D9C)->x,
            ((Overlay1ModeInput *)D_1D9C)->y,
            (Overlay1ModeSource *)D_1D64,
            ((Overlay1ModeState *)D_1DA0)->selector);
    ((Overlay1ModeState *)D_1DA0)->previousMode =
        ((Overlay1ModeState *)D_1DA0)->mode;
    ((Overlay1ModeState *)D_1DA0)->phase = 0;
}

/* ---- overlay1BuildObjectMappings ---- */


typedef struct Overlay1BuildData {
    s8 rank;
    s8 index;
    u8 pad02[0x1A6];
    u16 flags;
} Overlay1BuildData;

typedef struct Overlay1BuildObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
    u8 pad18[0x4C];
    Overlay1BuildData *data;
    Overlay1BuildData *innerData;
} Overlay1BuildObject;

typedef struct Overlay1BuildState {
    u8 pad00[0x381];
    s8 byte381;
    s8 byte382;
    s8 byte383;
    s8 byte384;
    u8 pad385[0x1B];
    f32 scale;
    u8 pad3A4[0x5C];
    s32 word400;
    s32 word404;
    s32 word408;
    s32 word40C;
    s32 word410;
} Overlay1BuildState;

extern Overlay1BuildObject **overlay1GetBuildObjectsReloc(s32 *count);
extern void overlay1MarkBuildObjectReloc(Overlay1BuildObject *object);
extern void overlay1BuildActivateReloc(Overlay1BuildObject *object);
extern void overlay1BuildInitializeStateReloc(s32 value);
extern s32 gOverlay1BuildGate;
extern u8 gOverlay1RankBase;
extern u8 gOverlay1RankLimit;
extern u8 gOverlay1ObjectMappingTable[][10];
extern void *gOverlay1BuildStateReloc;

void overlay1BuildObjectMappings(volatile s32 unused) {
    s32 count;
    Overlay1BuildObject **base;
    Overlay1BuildObject *object;
    Overlay1BuildData *data;
    Overlay1BuildData *innerData;
    s32 remaining;
    s32 inner;

    base = overlay1GetBuildObjectsReloc(&count);
    if (gOverlay1BuildGate != 0) {
        remaining = count;
        while (remaining--) {
            object = base[remaining];
            data = object->data;
            if (data->rank >= (gOverlay1RankBase - gOverlay1RankLimit)) {
                data->flags |= 1;
                overlay1MarkBuildObjectReloc(object);
            } else {
                data->flags |= 0x20;
            }
            overlay1BuildActivateReloc(object);
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->scale = 1.0f;
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->byte381 = 0;
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->byte382 = 0;
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->byte383 = -1;
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->byte384 = 0;
            ((Overlay1BuildState *)gOverlay1BuildStateReloc)->word400 = 0;
            *(s16 *)((u8 *)gOverlay1BuildStateReloc + 0x3BA) = 0xFF;
            *(f32 *)((u8 *)gOverlay1BuildStateReloc + 0x3D0) = object->x;
            *(f32 *)((u8 *)gOverlay1BuildStateReloc + 0x3D4) = object->y;
            *(f32 *)((u8 *)gOverlay1BuildStateReloc + 0x3D8) = object->x;
            *(f32 *)((u8 *)gOverlay1BuildStateReloc + 0x3DC) = object->y;
            if (gOverlay1BuildGate == 1) {
                overlay1BuildInitializeStateReloc(0);
                for (inner = 0; inner < 5; inner++) {
                    ((s32 *)gOverlay1BuildStateReloc)[0x101 + inner] = 0;
                }
            }
            inner = count;
            while (inner--) {
                data = base[remaining]->data;
                innerData = base[inner]->data;
                *((u8 *)gOverlay1BuildStateReloc + 0x3A8 + inner) =
                    gOverlay1ObjectMappingTable[data->index][innerData->index];
            }
        }
    }
}


/* ---- overlay1ReleaseRecords ---- */


typedef struct Overlay1ReleaseRecord {
    u8 pad00[0x14];
    void *resource;
    u8 pad18[4];
} Overlay1ReleaseRecord;

extern s32 gOverlay1ReleaseRecordCount;
extern Overlay1ReleaseRecord *gOverlay1ReleaseRecords;
extern void *gOverlay1ReleaseSecondary;
extern void *gOverlay1ReleaseFinal;
extern void overlay1ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG have generic teardown loops, but no exact donor. */
void overlay1ReleaseRecords(void) {
    s32 remaining;
    Overlay1ReleaseRecord *record;

    if (gOverlay1ReleaseRecordCount != 0) {
        remaining = gOverlay1ReleaseRecordCount - 1;
        record = &gOverlay1ReleaseRecords[1];
        while (remaining-- > 0) {
            if (record->resource != NULL) {
                overlay1ReleaseReloc(record->resource);
            }
            record++;
        }
        overlay1ReleaseReloc(gOverlay1ReleaseRecords);
        overlay1ReleaseReloc(gOverlay1ReleaseSecondary);
        gOverlay1ReleaseRecords = NULL;
        gOverlay1ReleaseSecondary = NULL;
    }
    if (gOverlay1ReleaseFinal != NULL) {
        overlay1ReleaseReloc(gOverlay1ReleaseFinal);
    }
}

/* ---- overlay1CallReset ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor for this wrapper. */
extern void overlay1ResetReloc(void);

void overlay1CallReset(void) {
    overlay1ResetReloc();
}


/* PLATEAU-HANDOFF:overlay1LoadBuildRecords:start
 * symbol: overlay1LoadBuildRecords
 * score: 20/572 words
 * frame: 0xD8
 * relocations: 110
 * first-mismatch: +0x1F0
 * summary: Rewritten in the overlay idiom with relocation identities: 469 at -52 to 20 at size 0, frame exact. Open: max-store address rematerialised, FP ring, arg order.
 * PLATEAU-HANDOFF:overlay1LoadBuildRecords:end
 */
