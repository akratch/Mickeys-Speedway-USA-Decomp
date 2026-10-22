#include "PR/ultratypes.h"

typedef struct {
    u8 pad000[0x50];
    void *resource50;
    u8 pad054[0x148];
    void *resource19C;
    u8 pad1A0[0x24];
    void *resource1C4;
} Overlay11Resources;

extern Overlay11Resources *D_8007C0B8;
extern void *gOverlay11Created0;
extern void *gOverlay11Created1;
extern void *gOverlay11Created2;
extern s32 gOverlay11CreatedActive;
extern void *overlay45CreateDescriptor(void *resource, s32 x, s32 y, s32 mode);

/* DKR v77/v80 and JFG have only generic fixed-resource creation idioms. */
void overlay11InitializeThreeB(void) {
    gOverlay11Created0 = overlay45CreateDescriptor(D_8007C0B8->resource1C4, 160, 40, 4);
    gOverlay11Created1 = overlay45CreateDescriptor(D_8007C0B8->resource19C, 160, 110, 4);
    gOverlay11Created2 = overlay45CreateDescriptor(D_8007C0B8->resource50, 160, 140, 4);
    gOverlay11CreatedActive = 1;
}
