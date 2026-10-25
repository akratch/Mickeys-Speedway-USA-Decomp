#include "PR/ultratypes.h"

/* Tier B: the renderer submits eight SpriteTriangle-grammar records over
 * six vertices; initialization reads twelve signed XYZ triples. These are
 * mutable storage views, with no original qualifier or declaration claim.
 * Tier D: names, grouping and flag meanings are inferred. The stock compiler
 * supplies eight end-alignment bytes after the 200 bytes of real objects.
 * This does not close legacy imports, graphics lifetimes or native views.
 */
typedef struct Overlay73Triangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} Overlay73Triangle;

typedef struct Overlay73VertexSeed {
    s16 x, y, z;
} Overlay73VertexSeed;

Overlay73Triangle o73Triangles[8] = {
    { 0, 0, 3, 1, 0, 0, 1023, 1023, 0, 1023 },
    { 0, 0, 2, 3, 0, 0, 1023, 0, 1023, 1023 },
    { 0, 2, 5, 3, 1023, 0, 0, 1023, 1023, 1023 },
    { 0, 2, 4, 5, 1023, 0, 0, 0, 0, 1023 },
    { 0, 0, 1, 3, 0, 0, 0, 1023, 1023, 1023 },
    { 0, 0, 3, 2, 0, 0, 1023, 1023, 1023, 0 },
    { 0, 2, 3, 5, 1023, 0, 1023, 1023, 0, 1023 },
    { 0, 2, 5, 4, 1023, 0, 0, 1023, 0, 0 },
};

Overlay73VertexSeed o73VertexSeeds[12] = {
    { -64, 0, 32 },
    { -64, 0, -32 },
    { 0, 0, 32 },
    { 0, 0, -32 },
    { 64, 0, 32 },
    { 64, 0, -32 },
    { -64, 0, 32 },
    { -64, 0, -32 },
    { 0, 0, 32 },
    { 0, 0, -32 },
    { 64, 0, 32 },
    { 64, 0, -32 },
};
