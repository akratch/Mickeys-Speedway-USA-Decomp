#include "overlays/overlay_058.h"

/* Tier B: the path renderer scans signed-halfword pairs, stops at the
 * selection sentinel and uses the paired node index as a geometry bound.
 * The caller's selection, stage and path-resource domains remain required. */
s16 gOverlay58PathMap[] = {
    15, 0,
    8, 5,
    27, 4,
    11, 18,
    25, 16,
    23, 14,
    37, 11,
    28, 6,
    31, 13,
    34, 12,
    10, 12,
    32, 17,
    17, 10,
    41, 7,
    24, 9,
    16, 1,
    38, 2,
    33, 15,
    40, 8,
    26, 3,
    -1, -1,
};
