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
/* Matched by indexing the unique list and bounding the scan by the count
 * global itself: the compiler reduces the subscript to a walking pointer and
 * an end pointer of its own, each with its own address load, and reads the
 * count once. No cursor, end pointer, count copy or next count is declared.
 * The point-reference index, the vertex base and the vertex pointer are named
 * locals; those three webs are what push the hoisted addresses and the stride
 * constant into saved registers.
 */
void overlay98CollectUniqueY(Overlay98Group *group) {
    Overlay98Block *block;
    Overlay98Span *span;
    s16 value;
    s32 blockIndex;
    s32 spanIndex;
    s32 k;
    s32 pointRefIndex;
    s32 vertexBase;
    s32 isNew;
    Overlay98Vertex *vertex;

    overlay98UniqueCountReloc = 0;
    for (blockIndex = 0; blockIndex < group->blockCount; blockIndex++) {
        block = &group->blocks[blockIndex];
        for (spanIndex = 0; spanIndex < block->spanCount; spanIndex++) {
            span = &block->spans[spanIndex];
            if (span->flags & 0x8000) {
                pointRefIndex = span->pointRefIndex;
                vertexBase = span->vertexBase;
                vertex = &block->vertices[
                    block->pointRefs[pointRefIndex].vertexIndex + vertexBase];
                value = vertex->y;
                isNew = 1;
                for (k = 0; k < overlay98UniqueCountReloc; k++) {
                    if (overlay98UniqueYReloc[k] == value) {
                        isNew = 0;
                    }
                }
                if (isNew) {
                    overlay98UniqueYReloc[overlay98UniqueCountReloc++] = value;
                    if (overlay98UniqueCountReloc >= 15) {
                        spanIndex = block->spanCount;
                        blockIndex = group->blockCount;
                    }
                }
            }
        }
    }
}
