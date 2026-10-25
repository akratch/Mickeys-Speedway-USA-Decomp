#ifndef GAME_FRAMEBUFFER_STATE_H
#define GAME_FRAMEBUFFER_STATE_H

#include "PR/ultratypes.h"

/* Tier B/D: VI producer and CPU/RDP consumers establish mutable sixteen-bit
 * image elements. The depth/snapshot cell is reused by screen effects.
 * These are pointer cells, not declarations of framebuffer capacities. */
extern u16 *D_800D2FA0;
extern u16 *D_800D2FA4;
extern u16 *D_800D2FA8;
extern u16 *D_800D2FAC;

#endif
