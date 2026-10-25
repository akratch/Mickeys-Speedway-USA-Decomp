#include "PR/ultratypes.h"

/* Tier B: Mickey's SpriteTriangle grammar is four byte fields followed by
 * three signed UV pairs. The renderer uploads four vertices and two
 * triangles; the initializer updates these mutable UV coordinates from
 * the loaded texture dimensions. This is not original-capacity recovery. */
typedef struct Overlay13QuadTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    s16 uv0U;
    s16 uv0V;
    s16 uv1U;
    s16 uv1V;
    s16 uv2U;
    s16 uv2V;
} Overlay13QuadTriangle;

Overlay13QuadTriangle gOverlay13QuadTriangles[2] = {
    {0x40, 0, 1, 3, 0, 0, 0, 0, 0, 0},
    {0x40, 0, 3, 2, 0, 0, 0, 0, 0, 0},
};
