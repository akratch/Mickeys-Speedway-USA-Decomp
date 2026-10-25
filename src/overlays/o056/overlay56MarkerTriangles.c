#include "PR/ultratypes.h"

/* Tier B: the marker polygon command consumes two sixteen-byte records,
 * whose indices reference its four submitted vertices. Flag bit meanings
 * and original names/declarations remain unknown (Tier D). This definition
 * preserves actual texture-coordinate fields without inventing padding. */
typedef struct O56MarkerTriangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} O56MarkerTriangle;
#ifdef TARGET_N64
typedef char O56MarkerTriangleSize[(sizeof(O56MarkerTriangle) == 16) ? 1 : -1];
#endif

O56MarkerTriangle o56MarkerTriangles[2] = {
    { 64, 0, 1, 2, 0, 0, 0, 0, 0, 0 },
    { 64, 0, 2, 3, 0, 0, 0, 0, 0, 0 },
};
