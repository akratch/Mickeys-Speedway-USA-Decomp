#include "PR/ultratypes.h"

extern void *gOverlay62Resource18;
extern void *gOverlay62Resource14;
extern u8 gOverlay62InlineResource[];
extern void overlay62ReleaseReloc(void *resource);
extern void func_8002C5F4(void);

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay62ReleaseAll(void) {
    overlay62ReleaseReloc(gOverlay62Resource18);
    overlay62ReleaseReloc(gOverlay62Resource14);
    overlay62ReleaseReloc(gOverlay62InlineResource);
    func_8002C5F4();
}
