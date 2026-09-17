#include "PR/ultratypes.h"

typedef struct Overlay14ValueSlot {
    s32 key;
    void *volatile value;
} Overlay14ValueSlot;

extern Overlay14ValueSlot gOverlay14Slots28[];
extern Overlay14ValueSlot gOverlay14FreeSlots28[];
extern Overlay14ValueSlot gOverlay14ChosenSlots28[];
extern Overlay14ValueSlot gOverlay14SlotsEnd128[];
extern void *gOverlay14SlotsActive2C;
extern s32 gOverlay14SlotCountE8;

extern s32 frontGetLanguage(void);
extern void *overlay14LoadRelocatedValue(s32 key, s32 kind);
extern void *func_overlay_014_F00009F4_18702CC(s32 key, s32 kind);

/* Retained candidate: 2 masked words at delta 0, frame 0x28, 15 relocations
 * (lane w1-b, 2026-09-16).  The two rows left are the tail's count load,
 * which the ROM schedules above the key store; that needs as1 to know the
 * slot pointer and &gOverlay14SlotCountE8 cannot alias (a `.noalias` stamp
 * ugen does not emit for a pointer whose defs have three different bases),
 * or the ROM's own emission order.  Read the shard before touching it. */
#ifdef NON_MATCHING
void *overlay14CreateValue(s32 key, s32 alternate) {
    void *value;
    s32 index;
    s32 kind;
    Overlay14ValueSlot *slot;

    slot = gOverlay14Slots28; scan_loop:
    value = slot->value;
    if ((value != 0) && (slot->key == key)) {
        return value;
    }
    slot++;
    if (slot < gOverlay14SlotsEnd128) {
        goto scan_loop;
    }

    index = 0;
    slot = gOverlay14FreeSlots28;
    if (gOverlay14SlotsActive2C != 0) {
        do {
            index++;
            if (index >= 32) {
                break;
            }
            slot = &gOverlay14FreeSlots28[index];
        } while (slot->value != 0);
    }
    if (index >= 32) {
        return 0;
    }
    if (1) {
        /* Lane w1-b (2026-09-16), 13 -> 2 masked at delta 0; see
         * docs/lastmile-block-budget-globals.md.  One pointer symbol for the
         * scan, the free loop and the chosen slot, DECLARED FOURTH: a coloured
         * pointer that is spilled around calls spills to its own reserved
         * home, and homes descend in declaration order (L99), which is what
         * puts the spill at 0x18 and sizes the frame to the target's 0x28.
         * `base + index` is a different IR name from the free loop's
         * `&base[index]`, so the shift is not PRE'd into a web and is drawn
         * from the ring as shipped.  The dead `index = 0` stops uopt
         * rematerialising the pointer from a spilled index after each call,
         * and the or-with-zero read is one def and one use at zero width
         * that lifts the pointer web's net past `value`'s 25/3. */
        slot = gOverlay14ChosenSlots28 + index;
        index = 0;
        slot = (Overlay14ValueSlot *)((u32)slot | 0);
        kind = frontGetLanguage();

    switch (kind) {
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
            slot->value = overlay14LoadRelocatedValue(key, kind);
        } else {
            slot->value = func_overlay_014_F00009F4_18702CC(key, kind);
        }
        value = slot->value;
        if (value != 0) {
            slot->key = key;
            value = slot->value;
            gOverlay14SlotCountE8++;
        }
    }
    return value;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/overlay14CreateValue/func_overlay_014_F00006FC_186FFD4.s")
#endif

/* PLATEAU-HANDOFF:overlay14CreateValue:start
 * symbol: overlay14CreateValue
 * score: 2/96 words
 * frame: 0x28
 * relocations: 15
 * first-mismatch: +0x158
 * summary: one fourth-declared slot pointer spilled to its own home, base-plus-index pointer keeps the shift a ring draw; two rows left are the tail count-load hoist over the key store.
 * PLATEAU-HANDOFF:overlay14CreateValue:end
 */
