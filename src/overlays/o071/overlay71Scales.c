#include "PR/ultratypes.h"

/* Tier B: the initializer only reads these two coefficients. IDO emits
 * ordinary .data; the input's actual placement declares its runtime RO
 * origin independently. Original names/declarations are Tier D. */
const f32 o71ObjectScale = 0.1f;
const f32 o71InputScale = 0.01f;
