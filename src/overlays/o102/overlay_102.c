#include "overlays/overlay_102.h"

/* Overlay 102: negative DKR/JFG exact scans; five relocs prove this map. */

s32 overlay102CheckSignature(void) {
    if (D_8007A204 != 0) {
        romCopy(0xF80, D_8007A204, 0x10);
        if (D_8007A204[0] == 0x6E5F923A &&
            D_8007A204[1] == 0xFB0BE5F6) {
            return 1;
        }
    }
    return 0;
}
