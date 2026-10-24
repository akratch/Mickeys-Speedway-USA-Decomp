#include "PR/ultratypes.h"

/* Tier B: loadFrontEndList/freeFrontEndList consume signed halfwords until
 * -1. setupFrontEndList consumes the second group by the same convention.
 * Array bounds here follow the actual sentinel and next independent owner;
 * compiler alignment is separate. Original names remain inferred (Tier D). */
s16 o57FrontEndResourceGroup[] = {
    3, 4, 5, 7, 62, 63, 64, 65, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    90, 99, 123, 125, 74, -1,
};
s16 o57FrontEndObjectGroup[] = {8, -1};

/* Tier B: the signed-word path loops terminate at -1. The primary sequence
 * contains only IDs 0 and 2..8, so every indexed argument read below is within
 * these genuine nine-element tables. The other sequence is started/stopped
 * directly and does not index these argument tables. The original grouping
 * and names are inferred; unused intermediate entries keep their real values. */
s32 o57PrimaryPathIds[] = {0, 2, 3, 4, 5, 6, 7, 8, -1};
s32 o57SelectionNodeArguments[] = {1, -1, 3, 1, 0, 0, 0, 0, 0};
s32 o57PrimaryNodeArguments[] = {0, -1, 0, 0, 0, 0, 0, 1, 0};
s32 o57SecondaryPathIds[] = {
    9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, 28, 29, 30, 31, -1,
};
