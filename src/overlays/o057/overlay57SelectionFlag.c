#include "PR/ultratypes.h"

/* Tier B: the selection update writes 1 while changing its path and clears
 * this signed flag after the child's float value passes the threshold.
 * Tier D: original name/qualifier unknown. The neighboring unread word is
 * not included; only the actual four-byte object is supplied. */
s32 o57SelectionChanging = 0;
