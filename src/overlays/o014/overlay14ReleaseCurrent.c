#include "PR/ultratypes.h"

extern s32 gOverlay14ResourceD0;
extern s32 gOverlay14ValueD4;
extern s32 gOverlay14FlagC8;
extern void overlay14ApplyValues(s32 resource, s32 value);

/* DKR v77/v80 and JFG checks found no exact donor for this cleanup wrapper. */
void overlay14ReleaseCurrent(void) {
    gOverlay14FlagC8 = 0;
    if (gOverlay14ResourceD0 != -1) {
        overlay14ApplyValues(gOverlay14ResourceD0, gOverlay14ValueD4);
    }
}
