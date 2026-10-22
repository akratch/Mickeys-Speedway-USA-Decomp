#include "PR/ultratypes.h"

typedef struct Overlay62Root {
    u8 pad0[0x25C];
    void *value;
} Overlay62Root;

extern f32 gOverlay62Float0;
extern s32 gOverlay62Value4Initial;
extern s32 gOverlay62Value4;
extern s32 gOverlay62Value8;
extern s32 gOverlay62ValueC;
extern s32 gOverlay62Value10;
extern void *gOverlay62Handle14;
extern void *gOverlay62Handle18;
extern Overlay62Root *D_8007C0B8;

extern void loadFrontEndList(void *);
extern void fontUseFont(s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern void *overlay45CreateDescriptor(void *, s32, s32, s32);
extern void *texLoadSprite(s32, s32);
extern void overlay45SetMode(void *, s32);

/* DKR v77/v80 and JFG have no exact donor for this resource initializer. */
void overlay62Initialize(void) {
    loadFrontEndList(&gOverlay62Float0);
    gOverlay62Value4Initial = (s32)gOverlay62Handle18;
    gOverlay62Float0 = 0.0f;
    gOverlay62Value4 = 13;
    fontUseFont(3);
    fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
    gOverlay62Handle14 = overlay45CreateDescriptor(
        D_8007C0B8->value, 0x28, 0x28, 0);
    gOverlay62Handle18 = texLoadSprite(0x93, 0);
    gOverlay62Value8 = 0x20;
    gOverlay62ValueC = 0;
    gOverlay62Value10 = 0;
    overlay45SetMode(gOverlay62Handle14, 0);
}
