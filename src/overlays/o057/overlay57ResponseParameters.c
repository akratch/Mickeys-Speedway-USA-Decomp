#include "PR/ultratypes.h"

/* Tier B: both smoothing paths compare squared x/y/z distances with these
 * float limits; selection compares its child's float value with the threshold,
 * and the node update copies the scale into its float output field.
 * Their four initialized storage identities are distinct from the BSS words
 * reached by some legacy aliases at other sites. Tier D: original names and
 * qualifiers are unknown; keep the genuine objects independently writable. */
f32 o57TailSquaredDistanceLimit = 500000.0f;
f32 o57TransitionSquaredDistanceLimit = 500000.0f;
f32 o57SelectionThreshold = 0.92f;
f32 o57NodeUpdateScale = 0.002f;
