#include "PR/ultratypes.h"

/* Tier B: the grid draw submits four two-triangle opcode-5 DMA packets,
 * following its nine-vertex load. Each record contains flags, three vertex
 * indices and three signed texture-coordinate pairs; this is the same Mickey
 * record contract as the owned trail triangles. The packets use vertices
 * 0..8 and texture coordinates 0..992. Tier D: names and original declaration
 * are inferred; conservative writable storage preserves the existing view. */
typedef struct Overlay57GridUV { s16 u, v; } Overlay57GridUV;
typedef struct Overlay57GridTriangle {
    u8 flags, firstIndex, secondIndex, thirdIndex;
    Overlay57GridUV coordinates[3];
} Overlay57GridTriangle;

Overlay57GridTriangle o57GridTriangleRecords[8] = {
    { 0x40, 0, 1, 3, {{0, 992}, {992, 992}, {0, 0}} },
    { 0x40, 1, 4, 3, {{992, 992}, {992, 0}, {0, 0}} },
    { 0x40, 1, 2, 4, {{0, 992}, {992, 992}, {0, 0}} },
    { 0x40, 2, 5, 4, {{992, 992}, {992, 0}, {0, 0}} },
    { 0x40, 3, 4, 6, {{0, 992}, {992, 992}, {0, 0}} },
    { 0x40, 4, 7, 6, {{992, 992}, {992, 0}, {0, 0}} },
    { 0x40, 4, 5, 7, {{0, 992}, {992, 992}, {0, 0}} },
    { 0x40, 5, 8, 7, {{992, 992}, {992, 0}, {0, 0}} },
};
