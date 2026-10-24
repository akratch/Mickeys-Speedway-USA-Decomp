#include "PR/ultratypes.h"

/* Tier B: real mutable transition gates, badge phase, sound handle and
 * progress scalar. The sound API writes an actual CPU handle pointer. */
s32 o58TransitionStarted = 0;
s32 o58TransitionRequested = 0;
s32 o58ResultBadgeState = 0;
void *o58TransitionSound = 0;
f32 o58TransitionProgress = 0.0f;
