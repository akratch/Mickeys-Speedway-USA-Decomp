#include "PR/ultratypes.h"

typedef struct Overlay44State {
    s8 active;
    u8 pad01[0x13];
    void *handles[4];
} Overlay44State;

void mmFree(void *handle);

/* DKR v77/v80 has generic fixed-handle cleanup, but no exact donor. */
void overlay44ReleaseHandles(Overlay44State *state) {
    s32 index;

    if (state != 0) {
        if (state->active != -1) {
            state->active = -1;
            index = 0;
            do {
                if (state->handles[index] != 0) {
                    mmFree(state->handles[index]);
                    state->handles[index] = 0;
                }
                index++;
            } while (index != 4);
        }
    }
}
