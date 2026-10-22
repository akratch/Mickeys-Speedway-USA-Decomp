#include "PR/ultratypes.h"

typedef struct Overlay41Transition {
    s16 remaining;
    s16 value;
    s16 x;
    s16 y;
    s8 red;
    s8 green;
    s8 blue;
    u8 intensity;
} Overlay41Transition;

extern Overlay41Transition D_800D6D18[5];

void overlay41TickTransitions(s32 step) {
    Overlay41Transition *transition;
    s32 i;
    s32 intensity;

    transition = D_800D6D18;
    for (i = 0; i < 5; i++) {
        if ((transition->remaining > 0) || (transition->intensity > 0)) {
            if (transition->remaining > 0) {
                transition->remaining -= step;
                intensity = transition->intensity + (step * 8);
                if (intensity >= 0x100) {
                    transition->intensity = 0xFF;
                } else {
                    transition->intensity = intensity;
                }
            } else if (transition->intensity > 0) {
                intensity = transition->intensity - (step * 8);
                if (intensity < 0) {
                    transition->intensity = 0;
                } else {
                    transition->intensity = intensity;
                }
            }
        }
        transition++;
    }
}
