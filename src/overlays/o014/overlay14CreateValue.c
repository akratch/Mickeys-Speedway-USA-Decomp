#include "PR/ultratypes.h"

typedef struct Overlay14ValueSlot {
    s32 key;
    void *value;
} Overlay14ValueSlot;

extern Overlay14ValueSlot gOverlay14Slots28[32];
extern s32 gOverlay14SlotCountE8;

extern s32 frontGetLanguage(void);
extern void *overlay14LoadRelocatedValue(s32 key, s32 kind);
extern void *func_overlay_014_F00009F4_18702CC(s32 key, s32 kind);

/* Matched by subscripting the one slot array everywhere and declaring no slot
 * pointer. The chosen slot's address is then the compiler's own temporary:
 * it is spilled around the calls to a temporary cell, and with it the counter
 * load schedules above the key store as shipped. A declared pointer does not;
 * the earlier listing replay points at the assembler's no-alias fact for an
 * address derived from the array, which is inferred here, not dumped. The
 * reloads of the value after each store come from the joins, not from a
 * volatile field. The free-slot scan clears its index on a line of its own.
 */
void *overlay14CreateValue(s32 key, s32 alternate) {
    s32 i;
    s32 kind;

    for (i = 0; i < 32; i++) {
        if ((gOverlay14Slots28[i].value != 0) &&
            (gOverlay14Slots28[i].key == key)) {
            return gOverlay14Slots28[i].value;
        }
    }
    i = 0;
    while ((i < 32) && (gOverlay14Slots28[i].value != 0)) {
        i++;
    }
    if (i >= 32) {
        return 0;
    }
    switch (frontGetLanguage()) {
        case 1:
            kind = 0xC;
            break;
        case 2:
            kind = 0xE;
            break;
        case 3:
            kind = 0x10;
            break;
        case 5:
            kind = 0x12;
            break;
        default:
            kind = 0xA;
            break;
    }
    if (alternate != 1) {
        gOverlay14Slots28[i].value = overlay14LoadRelocatedValue(key, kind);
    } else {
        gOverlay14Slots28[i].value =
            func_overlay_014_F00009F4_18702CC(key, kind);
    }
    if (gOverlay14Slots28[i].value != 0) {
        gOverlay14Slots28[i].key = key;
        gOverlay14SlotCountE8++;
    }
    return gOverlay14Slots28[i].value;
}
