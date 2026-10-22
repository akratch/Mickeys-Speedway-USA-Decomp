#include "ultra64.h"

typedef struct Overlay36NestedFlags {
    s16 flags;
} Overlay36NestedFlags;

typedef struct Overlay36VectorState {
    s16 kind;
    s16 value;
    s16 mode;
    u8 pad06[0x16];
    f32 vector[2];
    f32 amount;
    u8 pad28[0x50];
    Overlay36NestedFlags *nested;
} Overlay36VectorState;

extern void mathOneFloatPY(Overlay36VectorState *state, f32 *vector);

/* Vector-state initialization is Mickey-specific; no DKR/JFG donor exists. */
void overlay36InitVectorState(Overlay36VectorState *state, s16 kind,
                              s16 value, f32 amount) {
    state->value = value;
    state->amount = -amount;
    state->mode = 0;
    state->kind = kind;
    state->vector[0] = 0.0f;
    state->vector[1] = 0.0f;
    mathOneFloatPY(state, state->vector);
    state->nested->flags &= ~2;
}
