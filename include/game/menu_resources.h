#ifndef _GAME_MENU_RESOURCES_H_
#define _GAME_MENU_RESOURCES_H_

/* Tier B: initFront clears this span of resource pointers. Entries hold
 * textures, sprites, models or objects according to the asset directory.
 * The original declaration capacity and resource lifetimes remain unknown. */
#define MENU_RESOURCE_CACHE_COUNT 180
extern void *D_800D31C8[MENU_RESOURCE_CACHE_COUNT];

#endif
