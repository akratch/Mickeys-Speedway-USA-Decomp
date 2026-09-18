#include "PR/ultratypes.h"

typedef struct Overlay98Vertex {
    u8 reserved0[2];
    s16 y;
    u8 reserved4[6];
} Overlay98Vertex;

typedef struct Overlay98PointRef {
    u8 reserved0;
    u8 vertexIndex;
    u8 reserved2[14];
} Overlay98PointRef;

typedef struct Overlay98Span {
    u8 reserved0[6];
    s16 vertexBase;
    s16 pointRefIndex;
    u8 reservedA[2];
    u32 flags;
} Overlay98Span;

typedef struct Overlay98Block {
    Overlay98Vertex *vertices;
    Overlay98PointRef *pointRefs;
    u8 reserved08[4];
    Overlay98Span *spans;
    u8 reserved10[0x14];
    s16 spanCount;
    u8 reserved26[0x1A];
} Overlay98Block;

typedef struct Overlay98Group {
    u8 reserved00[4];
    Overlay98Block *blocks;
    u8 reserved08[0x12];
    s16 blockCount;
} Overlay98Group;

extern s32 overlay98UniqueCountReloc;
extern s16 overlay98UniqueYReloc[15];

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
/*
 * Plateau (2026-08-25 plus lever-51 reopen 2026-09-19): the best safe C is
 * still the walking uniqueEnd scan, exact-size with the retail frame. Masked
 * 32 of 81, first at +0x68. Identity-gated proc-0 is p2, 18 decisions; force
 * of web 112 (type-1 count address, s0) onto a1 is declined (forced=-2, a1
 * absent from its cost table), and web 58 (t3) cannot take v1. Accepted
 * forces are 32 or worse. Forward unique indexing with a local bound unrolls
 * +26 words; -Wo,-loopunroll,0 is 78 words. Live-global forward index is +16
 * of 49 and unroll-inert. An indexed scan whose bound is != uniqueEnd
 * strength-reduces back to this walking object at 32. Countdown of the live
 * global is 31 at delta 0 and still uses the three 0x10 slots, but it is a
 * backward scan: aligned structural 7 to 12 against the target's forward
 * walk, so it is not adopted. L131 UniqueY spellings, L97 regions, volatile
 * pointers, count-pointer, vertex-index split, and setup-before-value are
 * flat or worse. Span subscripts stay inert; dropping the block pointer
 * still grows to 88. Lever 50 does not apply: raw equals masked at 32.
 */
#ifdef NON_MATCHING
void overlay98CollectUniqueY(Overlay98Group *group) {
    Overlay98Block *block;
    Overlay98Span *span;
    s16 *unique;
    s16 *uniqueEnd;
    s16 value;
    s32 blockIndex;
    s32 spanIndex;
    s32 oldUniqueCount;
    s32 nextUniqueCount;
    s32 pointRefIndex;
    register s32 vertexBase;
    s32 isNew;
    u8 destinationIndex;

    overlay98UniqueCountReloc = 0;
    for (blockIndex = 0; blockIndex < group->blockCount; blockIndex++) {
        block = &group->blocks[blockIndex];
        for (spanIndex = 0; spanIndex < block->spanCount; spanIndex++) {
            span = &block->spans[spanIndex];
            if (span->flags & 0x8000) {
                pointRefIndex = span->pointRefIndex;
                vertexBase = span->vertexBase;
                value = *(s16 *)((u8 *)block->vertices +
                                  ((vertexBase +
                                    block->pointRefs[pointRefIndex]
                                        .vertexIndex) *
                                       10) +
                                   2);

                oldUniqueCount = overlay98UniqueCountReloc;
                nextUniqueCount = oldUniqueCount + 1;
                unique = overlay98UniqueYReloc;
                uniqueEnd = &overlay98UniqueYReloc[oldUniqueCount];
                isNew = 1;
                if (oldUniqueCount > 0) {
                    do {
                        if (*unique == value) {
                            isNew = 0;
                        }
                        unique++;
                    } while (unique < uniqueEnd);
                }

                destinationIndex = oldUniqueCount;
                if (isNew) {
                    overlay98UniqueYReloc[destinationIndex] = value;
                    overlay98UniqueCountReloc = nextUniqueCount;
                    if (nextUniqueCount >= 15) {
                        spanIndex = block->spanCount;
                        blockIndex = group->blockCount;
                    }
                }
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o098/overlay98CollectUniqueY/func_overlay_098_F0000000_18D89C0.s")
#endif

/* PLATEAU-HANDOFF:overlay98CollectUniqueY:start
 * symbol: overlay98CollectUniqueY
 * score: 32/81 words
 * frame: 0x10
 * relocations: 8
 * first-mismatch: +0x68
 * summary: Lever 51 stall. Indexed unique scan unrolls +26 or 78-80 without uniqueEnd; != uniqueEnd SRs to walking 32. Countdown 31/0 is backward. p2 w112 cannot take a1.
 * PLATEAU-HANDOFF:overlay98CollectUniqueY:end
 */
