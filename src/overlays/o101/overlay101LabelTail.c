#include "PR/ultratypes.h"

/* Tier B/C: the label-pointer table and three window builders select these
 * distinct writable strings. NUL terminators establish the natural lengths;
 * object/section alignment belongs to IDO, not explicit array capacity.
 * Tier C/D: the intervening printf-style action-limit format is recovered
 * initialized storage with no active reference found. Its name is inferred;
 * it is absent from the main compiler pool and has no invented caller. */

u8 o101Label112[] = "new secrets along the way!";
u8 o101UnreferencedActionLimitFormat[] = "MAX number of actions exceeded (%d)\n";
u8 o101WindowLabel00[] = "Invisible Window";
u8 o101WindowLabel01[] = "Invisible Window";
u8 o101WindowLabel02[] = "Invisible Window";
