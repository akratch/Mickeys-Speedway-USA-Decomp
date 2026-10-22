#include "PR/ultratypes.h"

extern void func_overlay_002_F0000C90_1857A88(void *object, s32 enabled);

/* DKR v77/v80 and JFG checks found no exact donor for this mode wrapper. */
void overlay2Enable(void *object) {
    func_overlay_002_F0000C90_1857A88(object, 1);
}
