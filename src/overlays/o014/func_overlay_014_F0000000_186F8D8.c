#include "PR/ultratypes.h"
typedef struct Slot { s32 key; void *value; } Slot;
extern Slot gOverlay14ValueSlots28[], gOverlay14ValueSlotsEnd128[];
extern s32 gOverlay14StateC0, gOverlay14StateC4, gOverlay14StateC8;
extern s32 gOverlay14PendingValueD0, gOverlay14PendingModeD4;
extern s32 gOverlay14TransitionD8, gOverlay14CursorDC, gOverlay14PointerE0;
extern s32 gOverlay14ValueCountE8, gOverlay14CommandCountEC;
extern void *gOverlay14AssetF0, *gOverlay14OffsetsF4;
extern s32 gOverlay14ResultF8, gOverlay14ResultFC;
extern s32 gOverlay14Current104, gOverlay14Next108, gOverlay14Context10C, gOverlay14Field114;
extern void overlay14InitializeReloc(void);
extern void mmFree(void *ptr);

/* The slot cursor's initializer shares the `do {` header line, and the loop
 * bound is spelled inline in the tail test rather than kept in a local. Both
 * are as1 list-scheduler levers: the bound then materializes as a hoisted
 * loop invariant born after the count address, so the three pre-header
 * address pairs issue in the retail order. */
void func_overlay_014_F0000000_186F8D8(void) {
    Slot *slot;
    s32 invalid;
    overlay14InitializeReloc();
    invalid = -1;
    gOverlay14Current104 = invalid; gOverlay14Next108 = invalid;
    gOverlay14Context10C = 0; gOverlay14Field114 = 0;
    if (gOverlay14AssetF0) mmFree(gOverlay14AssetF0);
    if (gOverlay14OffsetsF4) mmFree(gOverlay14OffsetsF4);
    slot = gOverlay14ValueSlots28; do {
        if ((gOverlay14ValueCountE8 != 0) && slot->value) mmFree(slot->value);
        slot++;
        slot[-1].value = 0;
        slot[-1].key = invalid;
    } while (slot != gOverlay14ValueSlotsEnd128);
    gOverlay14ValueCountE8 = 0;
    gOverlay14CommandCountEC = 0;
    gOverlay14StateC0 = 0; gOverlay14StateC4 = 0; gOverlay14StateC8 = 0;
    invalid = -1;
    gOverlay14PendingValueD0 = invalid; gOverlay14PendingModeD4 = invalid;
    gOverlay14TransitionD8 = 0; gOverlay14CursorDC = 0; gOverlay14PointerE0 = 0;
    gOverlay14AssetF0 = 0; gOverlay14OffsetsF4 = 0;
    gOverlay14ResultF8 = 0; gOverlay14ResultFC = 0;
}
