#include "PR/ultratypes.h"

/* Tier B: the background's opcode-5 packet transfers twelve sixteen-byte
 * triangles from either bank. Its seven-vertex column cache uses indices
 * 0..13. The same record fields are used by Mickey's owned overlay45 and
 * overlay38 geometry. Tier D: names/original declarations are inferred. */
typedef struct Overlay11TriangleUV { s16 u, v; } Overlay11TriangleUV;
typedef struct Overlay11BackgroundTriangle {
    u8 flags, firstIndex, secondIndex, thirdIndex;
    Overlay11TriangleUV coordinates[3];
} Overlay11BackgroundTriangle;

Overlay11BackgroundTriangle gOverlay11StorageTriangles[2][12] = {
    {
        { 0x40, 0, 7, 1, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 7, 8, 1, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 1, 8, 2, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 8, 9, 2, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 2, 9, 3, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 9, 10, 3, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 3, 10, 4, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 10, 11, 4, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 4, 11, 5, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 11, 12, 5, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 5, 12, 6, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 12, 13, 6, {{0, 0}, {0, 0}, {0, 0}} },
    },
    {
        { 0x40, 7, 0, 8, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 0, 1, 8, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 8, 1, 9, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 1, 2, 9, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 9, 2, 10, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 2, 3, 10, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 10, 3, 11, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 3, 4, 11, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 11, 4, 12, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 4, 5, 12, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 12, 5, 13, {{0, 0}, {0, 0}, {0, 0}} },
        { 0x40, 5, 6, 13, {{0, 0}, {0, 0}, {0, 0}} },
    },
};
