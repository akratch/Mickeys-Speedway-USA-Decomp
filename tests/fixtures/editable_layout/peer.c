/* Deliberate diagnostic module, never linked into the retail target. */
#include "PR/ultratypes.h"

extern void overlay75MarkSlot(s32 slot);
u32 layout108State[8];
u32 layout108Seed = 13;

void layout108Init(void) {
    layout108State[0] = layout108Seed;
}

void layout108Resume(void) {
    layout108State[1] = (layout108State[1] + layout108State[0]) & 1023;
}

s32 layout108Step(s32 value) {
    u32 index = (u32) value & 7;
    layout108State[index] = (layout108State[index] + layout108Seed) & 1023;
    overlay75MarkSlot((s32) (index & 3));
    return (s32) layout108State[index];
}
