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
 * Plateau (2026-08-25 plus lever-51 reopen 2026-09-17): the best safe C is
 * still exact-size with the retail frame. Flag lattice 32 of 81, first at
 * +0x68. Named uniqueEnd is load-bearing for 81 words and three stack slots;
 * deleting it to index overlay98UniqueYReloc is 78-80 and misses slot +0x4.
 * The historic +26 indexed scan was an extra s32 uniqueIndex beside the u8
 * destination. Span subscripts are byte-identical to the named span pointer.
 * Dropping the named block pointer rematerializes and grows to 88. Inner
 * pointer scope, removing the byte destination, volatile array, unsequenced
 * permuter, vertex array indexing, outer while, unique-before-vertex order,
 * and live-global uniqueEnd remain nonexact. Identity-gated proc-0 is p2
 * only (18 decisions); force of the s0 web onto a1 is declined as forbidden.
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
 * summary: Lever 51 stall. Named uniqueEnd is required for 81 words and three stack slots; unique indexing without it is 78-80, with an extra s32 index +26. Span subscripts are inert; dropping the block pointer grows to 88. Identity-gated proc-0 is p2, 18 decisions; the s0 web cannot take a1. Aligned 52 exact, 19 naming, 7 structural plus 3/3 insertions. Best remains the walking unique scan.
 * PLATEAU-HANDOFF:overlay98CollectUniqueY:end
 */
