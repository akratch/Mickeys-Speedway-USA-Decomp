#include "PR/ultratypes.h"

typedef struct Overlay79TimerObject {
    u8 pad0[0x64];
    s32 *timers;
} Overlay79TimerObject;

extern void func_80006EA0(Overlay79TimerObject *object);

/* DKR v77/v80 and JFG checks found only broad timer/update semantics. */
void overlay79UpdateTimers(Overlay79TimerObject *object, s32 step) {
    s32 *timers = object->timers;

    timers[0] -= step;
    if (timers[0] <= 0) {
        timers[1] -= step;
        if (timers[1] <= 0) {
            timers[1] = 0;
            func_80006EA0(object);
        }
    }
}
