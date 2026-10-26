#ifndef GAME_MENU_CURSORS_H
#define GAME_MENU_CURSORS_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/* Front-end cursor cells. The existing opaque vertex/triangle transport
 * view does not establish their backing layouts, capacity or lifetime. */
extern Gfx *D_800D3140;
extern Mtx *D_800D3144;
extern void *D_800D3148;
extern void *D_800D314C;

#endif
