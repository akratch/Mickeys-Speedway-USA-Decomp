#include "PR/ultratypes.h"

/* Tier B: the resident load/free and setup helpers traverse signed-halfword
 * IDs through -1. These complete mutable lists supply the load/free group
 * and the separate object-setup group; the original capacities are unknown. */
s16 o50LoadResourceIds[] = {
    32, 33, 34, 35, 36, 37, 1, 38, 39, 41, 42, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 80, 0, 91, 92, 93, 94, 95, 96, 97, 98, -1,
};

s16 o50SetupObjectIds[] = {
    0, 1, 3, 4, 7, -1,
};
