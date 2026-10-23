#include "overlays/overlay_065.h"

/* Generic double-release wrapper only; no exact DKR donor. */
extern TextureHeader *gOverlay65Resource208;
extern void *gOverlay65Resource2980;
extern void texFreeTexture(TextureHeader *texture);
extern void mmFree(void *data);

void overlay65Release(void) {
    texFreeTexture(gOverlay65Resource208);
    mmFree(gOverlay65Resource2980);
}
