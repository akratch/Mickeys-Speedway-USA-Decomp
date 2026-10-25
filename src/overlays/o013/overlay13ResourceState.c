#include "PR/ultratypes.h"

struct Sprite;
struct TextureHeader;

/* Tier B: the loader/free calls establish two sprite handles and one texture
 * handle; active-record walks use a distinct signed count. Tier D: original
 * declarations and grouping. Explicit initialization preserves module state;
 * resource/cache lifetime and the existing consumer bounds remain open. */
struct Sprite *gOverlay13Resource0 = 0;
struct Sprite *gOverlay13Resource1 = 0;
struct TextureHeader *gOverlay13Resource2 = 0;
s32 gOverlay13ActiveCount = 0;
