#include "overlays/overlay_105.h"

/* Overlay 105: negative DKR/JFG scans; both relocs name libultra PI locks. */
s32 overlay105CheckWord(u32 base) {
    struct {
        u32 unused;
        u32 status;
    } locals;
    volatile u32 *piStatus;

    __osPiGetAccess();
    piStatus = (volatile u32 *)0xA4600010;
    locals.status = *piStatus;
    while ((locals.status & 3) != 0) {
        locals.status = *piStatus;
    }
    locals.status = *(u32 *)((base | 0xB0000A5C) | 0xA0000000);
    __osPiRelAccess();
    if (locals.status != 0x1740FFF9) {
        return 0;
    }
    return 1;
}
