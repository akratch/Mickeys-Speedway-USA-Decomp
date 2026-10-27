#include "PR/ultratypes.h"

/* Tier B: the opcode-5 strip renderer submits up to sixteen real triangle
 * records over eighteen generated vertices. Mickey's SpriteTriangle fields
 * establish flags, three indices and three signed texture-coordinate pairs.
 * Tier D: the original name, qualifier, TU and capacity remain unknown.
 * This is the minimum complete currently consumed strip, not a reserve. */
typedef struct Overlay83StripUV { s16 u, v; } Overlay83StripUV;
typedef struct Overlay83StripTriangle {
    u8 flags, firstIndex, secondIndex, thirdIndex;
    Overlay83StripUV coordinates[3];
} Overlay83StripTriangle;

Overlay83StripTriangle gOverlay83StripTriangles[16] = {
    {0x40, 0, 1, 2, {{0, 0}, {0, 480}, {256, 0}}},
    {0x40, 1, 2, 3, {{0, 480}, {256, 0}, {256, 480}}},
    {0x40, 2, 3, 4, {{256, 0}, {256, 480}, {512, 0}}},
    {0x40, 3, 4, 5, {{256, 480}, {512, 0}, {512, 480}}},
    {0x40, 4, 5, 6, {{512, 0}, {512, 480}, {768, 0}}},
    {0x40, 5, 6, 7, {{512, 480}, {768, 0}, {768, 480}}},
    {0x40, 6, 7, 8, {{768, 0}, {768, 480}, {1024, 0}}},
    {0x40, 7, 8, 9, {{768, 480}, {1024, 0}, {1024, 480}}},
    {0x40, 8, 9, 10, {{1024, 0}, {1024, 480}, {1280, 0}}},
    {0x40, 9, 10, 11, {{1024, 480}, {1280, 0}, {1280, 480}}},
    {0x40, 10, 11, 12, {{1280, 0}, {1280, 480}, {1536, 0}}},
    {0x40, 11, 12, 13, {{1280, 480}, {1536, 0}, {1536, 480}}},
    {0x40, 12, 13, 14, {{1536, 0}, {1536, 480}, {1792, 0}}},
    {0x40, 13, 14, 15, {{1536, 480}, {1792, 0}, {1792, 480}}},
    {0x40, 14, 15, 16, {{1792, 0}, {1792, 480}, {2048, 0}}},
    {0x40, 15, 16, 17, {{1792, 480}, {2048, 0}, {2048, 480}}},
};
