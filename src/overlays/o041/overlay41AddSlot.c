#include "PR/ultratypes.h"

typedef struct Overlay41Slot {
    u32 object;
    u8 color0;
    u8 value1;
    u8 color1;
    u8 value3;
    u8 color2;
    u8 value5;
    u8 color3;
    u8 value7;
    s16 amount;
    s16 previousAmount;
} Overlay41Slot;

/* The module's zero-based initialized-data placeholder. Reaching the scale
 * through a member of the module block, rather than through an index into an
 * `f32 D_0[]`, is load-bearing: IDO canonicalizes a commutative multiply so
 * that an *indexed* array reference becomes the left operand, which emits the
 * two operand loads in the opposite order. A member reference is not indexed,
 * so the written operand order survives. Both spellings relocate identically
 * (D_0 HI16/LO16 with a +0x54 addend). */
typedef struct Overlay41ModuleData {
    u8 pad00[0x54];
    f32 amountScale;
} Overlay41ModuleData;

extern Overlay41Slot D_800D6C58[];
extern Overlay41ModuleData D_0;

/* The twelve-slot scan is a top-tested `while (remaining--)`, not the m2c
 * `do { } while (remaining--)`: both run twelve iterations, but only the
 * top-tested form gives the loop-exit copy a lower uopt web number than the
 * counter, so the counter is coloured a1 and its dead copy v1 rather than the
 * reverse. */
void func_overlay_041_F0001650_1888988(void *object, volatile s32 value1,
                                       s32 value3, s32 value5, s32 value7,
                                       f32 amount, s32 alternateColors) {
    register s32 remaining;
    Overlay41Slot *slot;
    u8 *bytes;

    if (object == 0) {
        return;
    }

    slot = D_800D6C58;
    remaining = 12;
    while (remaining--) {
        if (slot->object == 0) {
            bytes = object;
            if (alternateColors != 0) {
                slot->object = (u32)object;
                slot->color0 = bytes[0x40];
                slot->color1 = bytes[0x41];
                slot->color2 = bytes[0x42];
                slot->color3 = bytes[0x43];
            } else {
                slot->object = (u32)object & 0x7FFFFFFF;
                slot->color0 = bytes[0x38];
                slot->color1 = bytes[0x39];
                slot->color2 = bytes[0x3A];
                slot->color3 = bytes[0x3B];
            }
            slot->value1 = value1;
            slot->value3 = value3;
            slot->value5 = value5;
            slot->value7 = value7;
            slot->amount = amount * D_0.amountScale;
            slot->previousAmount = slot->amount;
            return;
        }
        slot++;
    }
}
