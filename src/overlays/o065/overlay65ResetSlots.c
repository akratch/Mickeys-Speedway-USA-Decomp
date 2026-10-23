#include "overlays/overlay_065.h"

extern Overlay65TrailRecord D_0[50];
extern s16 gOverlay65Ready;
/* Tier B: the LOCAL records address the 50 trail active fields in BSS. */
void overlay65ResetSlots(void) {
    register s32 index;

    for (index = 0; index < 50; index++) {
        D_0[index].active = 0;
    }
    gOverlay65Ready = 1;
}
