#include "overlays/overlay_013.h"

/* Tier B: two distinct four-byte coefficient reads. Tier D: names/grouping
 * and original declarations. Conservatively writable: the runtime RO role
 * does not establish original C constness or exclude indirect mutation. */
f32 gOverlay13Gravity = 0.3f;
f32 gOverlay13SecondaryScale = 0.6f;
