#include "PR/ultratypes.h"

/* Tier B: loadFrontEndList reads signed resource IDs through the -1 entry.
 * The loader gives these IDs their resource-domain interpretation. */
s16 o58ResultAssetIds[32] = {
    3, 4, 5, 17, 66, 67, 68, 69,
    70, 71, 72, 73, 75, 76, 77, 78,
    79, 54, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 90, 74, 122, 123, -1,
};
