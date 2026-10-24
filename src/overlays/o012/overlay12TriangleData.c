#include "PR/ultratypes.h"

/* Tier B: each actual polygon path consumes two sixteen-byte triangles
 * over four vertices. The resident SpriteTriangle grammar establishes
 * flags/three vertex indices and three signed texture-coordinate pairs.
 * The ready word is written by initialization/release and read by both
 * constructors. Original names/declarations and flag bit meaning are
 * Tier D; this does not establish resource/graphics/native lifetime safety.
 */
typedef struct O12Triangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} O12Triangle;

O12Triangle o12TrianglesA[2] = {
    { 64, 0, 1, 2, 0, 480, 0, 0, 2016, 512 },
    { 64, 1, 3, 2, 0, 0, 2016, 0, 2016, 512 },
};

O12Triangle o12TrianglesB[2] = {
    { 64, 0, 1, 2, 0, 672, 0, 0, 1504, 704 },
    { 64, 1, 3, 2, 0, 0, 1504, 0, 1504, 704 },
};

O12Triangle o12QuadTriangles[2] = {
    { 64, 0, 1, 2, 0, 0, 992, 0, 0, 992 },
    { 64, 2, 3, 1, 0, 992, 992, 992, 992, 0 },
};

s32 o12Ready = 0;
