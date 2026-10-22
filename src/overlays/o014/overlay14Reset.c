#include "PR/ultratypes.h"

/*
 * Overlay 14 +0xB40. JFG proves the 0x1C-byte instruction sequence but names
 * it only with a generated placeholder, which is not promoted here.
 */
extern s32 gOverlay14Timer1C;
extern s32 gOverlay14Timer20;
extern s32 gOverlay14ResetCC;

void overlay14Reset(void) {
    gOverlay14Timer1C = 0;
    gOverlay14Timer20 = 0;
    gOverlay14ResetCC = 0;
}
