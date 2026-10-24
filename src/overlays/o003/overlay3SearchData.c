#include "PR/ultratypes.h"

/* Tier C: the complete retained NUL-terminated module label. No current
 * reader has been established, so mutable storage preserves that limit. */
char gOverlay3RetainedLabel[] = "NO BARRELS\n";

/* Tier B: the actual scalar loaded by overlay3FindClosestObject. The former
 * four-float view encoded its location after the preceding string. */
f32 gOverlay3SearchMaxDistance = 3.402823466e38f;
