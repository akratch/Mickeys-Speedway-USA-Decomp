#include "game/menu_resources.h"

/* Tier B: initFront clears 180 pointer entries. The cache holds several
 * resource kinds; historical declaration capacity and resource lifetime
 * require separate evidence. */
void *D_800D31C8[MENU_RESOURCE_CACHE_COUNT];
