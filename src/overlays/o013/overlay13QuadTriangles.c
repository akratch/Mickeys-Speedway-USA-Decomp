#include "overlays/overlay_013.h"

/* Tier B: Mickey's SpriteTriangle grammar is four byte fields followed by
 * three signed UV pairs. The renderer uploads four vertices and two
 * triangles; the initializer updates these mutable UV coordinates from
 * the loaded texture dimensions. This is not original-capacity recovery. */
Overlay13QuadTriangle gOverlay13QuadTriangles[2] = {
    {0x40, 0, 1, 3, 0, 0, 0, 0, 0, 0},
    {0x40, 0, 3, 2, 0, 0, 0, 0, 0, 0},
};
