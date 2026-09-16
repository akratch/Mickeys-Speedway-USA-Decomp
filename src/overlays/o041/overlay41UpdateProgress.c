#include "PR/ultratypes.h"

typedef struct Overlay41Descriptor {
    u8 reserved00[0x14];
    u16 flags14;
    s8 reserved16[0xBE];
    f32 scaleD4;
} Overlay41Descriptor;

typedef struct Overlay41State {
    u8 reserved00[6];
    s16 flags06;
    u8 reserved08[0x31];
    u8 value39;
    u8 reserved3A[6];
    Overlay41Descriptor *descriptor40;
    u8 reserved44[0x4F];
    u8 selector93;
} Overlay41State;

typedef struct Overlay41Input {
    u8 reserved00[0x0C];
    f32 amount0C;
    u8 reserved10[4];
    u8 negate14;
    u8 reserved15;
    u8 flags16;
    u8 reserved17[0x0D];
    u8 start24;
    u8 end25;
    u8 limit26;
    u8 current27;
} Overlay41Input;

extern Overlay41State *gOverlay41CurrentState;
extern void overlay41ApplyAmount(Overlay41State *state, f32 amount, f32 step);

/* Matched 2026-09-16 (lane lm-c), 14 -> 0 masked words at delta 0 in four
 * measured batches; every step is a carrier deletion, none a colour force:
 *
 *   - `limit` is `u8`, so `divisor = limit` is a widening conversion node
 *     rather than a plain copy, and uopt keeps it as the real `move` the
 *     target emits in the first branch's delay slot instead of coalescing
 *     divisor into limit (14 -> 8, with the start read moved after the
 *     clamp, where the target reads it).
 *   - the compare reads the stored field back (`state->value39 < 0xFF`);
 *     uopt forwards the store into that reload and masks it, which is the
 *     target's `sb` of the unmasked sum beside an `andi` of it (8 -> 4).
 *   - no `value`, `start` or `delta` local at all (L160): the sum is a ring
 *     temporary in the target, the start load is one CSE'd temp used twice,
 *     and a declared carrier for either takes a pool colour that pushes
 *     `divisor` off `a0` (4 -> 1).
 *   - the redundant `& 0xFFFF` on the flags06 read-modify-write was a ring
 *     phase device fitted to the old shape (L146); on this shape it is the
 *     one surplus draw (1 -> 0).
 *
 * Twelve of the 32 cells in the final lattice are at 0 to 5 words and the
 * only zero is this one: the operand order `start + quotient` matters at the
 * last word, the mask device and the value local are each worth 16 to 20
 * when present. */
void func_overlay_041_F0001298_18885D0(Overlay41Input *input,
                                        Overlay41State *state, s32 step) {
    Overlay41Descriptor *descriptor;
    s32 active;
    s32 current;
    u8 limit;
    s32 divisor;

    descriptor = state->descriptor40;
    active = descriptor->scaleD4 != 0.0f;
    if ((active && descriptor->reserved16[state->selector93 + 8] == 0) ||
        (!active && descriptor->reserved16[8] == 0)) {
        if (input->negate14 != 0) {
            overlay41ApplyAmount(state, -input->amount0C, (f32)step);
        } else {
            overlay41ApplyAmount(state, input->amount0C, (f32)step);
        }
    }

    current = input->current27;
    limit = input->limit26;
    if (current < limit) {
        divisor = limit;
        input->current27 = current + step;
        current = input->current27;
        if (current >= limit) {
            input->current27 = limit;
            current = limit & 0xFF;
        }
        state->value39 = input->start24 + ((input->end25 - input->start24) * current) / divisor;
        if (state->value39 < 0xFF) {
            state->flags06 |= 4;
        } else if (!(state->descriptor40->flags14 & 4)) {
            state->flags06 &= ~4;
        }
    } else {
        state->value39 = input->end25;
    }

    if (input->flags16 & 2) {
        gOverlay41CurrentState = state;
    }
}
