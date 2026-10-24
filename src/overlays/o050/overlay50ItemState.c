#include "PR/ultratypes.h"

/* Tier B: texAnimateSprite consumes mutable enable/loop flag bits; the
 * updater cycles the extra centisecond digit modulo ten, tracks the loaded
 * front-end item (-1 means none), and clamps its display alpha to 0..255.
 * Ordinary scalar alignment supplies the gaps between the byte objects. */
s32 o50FrontAnimationFlags = 1 | 8;
s8 o50CentisecondCycle = 0;
s8 o50LoadedItemSlot = -1;
s32 o50ItemAlpha = 0;
