#include "PR/ultratypes.h"

/* Tier B: opcode-5 polygon DMA consumes sixteen-byte records. The same
 * Mickey packet/record contract is used by overlay38's owned triangles.
 * The trail loads at most eighteen vertices and submits at most sixteen
 * triangles; these strip indices reference vertices 0..17. This is geometry,
 * with real flags/indices/texture coordinates, not texture or pixel storage.
 * Tier D: original names/qualifiers remain unknown. */
typedef struct Overlay46TrailUV { s16 u, v; } Overlay46TrailUV;
typedef struct Overlay46TrailTriangle {
    u8 flags, firstIndex, secondIndex, thirdIndex;
    Overlay46TrailUV coordinates[3];
} Overlay46TrailTriangle;

Overlay46TrailTriangle gOverlay46TrailTriangles[16] = {
    { 0x40, 0, 1, 3, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 0, 3, 2, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 2, 3, 5, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 2, 5, 4, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 4, 5, 7, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 4, 7, 6, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 6, 7, 9, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 6, 9, 8, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 8, 9, 11, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 8, 11, 10, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 10, 11, 13, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 10, 13, 12, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 12, 13, 15, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 12, 15, 14, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 14, 15, 17, {{0, 0}, {0, 0}, {0, 0}} },
    { 0x40, 14, 17, 16, {{0, 0}, {0, 0}, {0, 0}} },
};
