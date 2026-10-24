#include "PR/ultratypes.h"

/* Tier B: byte loads compare each group's threshold with a random value
 * in 1..100. The ten groups are the same domain explicitly visited by
 * overlay3UpdateTimedEntries. Remaining input bytes are compiler alignment. */
u8 gOverlay3ModeChance[10] = { 5, 3, 7, 2, 100, 5, 8, 8, 8, 2 };
