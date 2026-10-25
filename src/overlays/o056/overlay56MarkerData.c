#include "PR/ultratypes.h"

/* Tier B: the live resource cell is loaded/released as a CPU pointer.
 * The marker vertex command consumes four ten-byte position/RGBA records.
 * Tier D: original names/declarations and their shared source grouping.
 * IDO must naturally emit the remaining end alignment; no padding object.
 * Legacy operand aliases, physical GPU addresses, API and lifetime contracts
 * are not repaired by these physical source definitions. */
typedef struct O56MarkerVertex {
    s16 x, y, z;
    u8 red, green, blue, alpha;
} O56MarkerVertex;
#ifdef TARGET_N64
typedef char O56MarkerVertexSize[(sizeof(O56MarkerVertex) == 10) ? 1 : -1];
#endif

void *o56MinimapResource = 0;
O56MarkerVertex o56MarkerVertices[4] = {
    { 0, 4, 0, 255, 255, 255, 255 },
    { 4, -6, 0, 255, 255, 255, 255 },
    { 0, -3, 0, 255, 255, 255, 255 },
    { -4, -6, 0, 255, 255, 255, 255 },
};
