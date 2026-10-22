#include "overlays/o100/motion.h"

/* The unloaded JAL field stays zero; the runtime table binds this call to
 * overlay100RemoveEntry at module offset 0x278. */
extern void overlay100RemoveEntry(Overlay100Motion *motion);
extern f32 gOverlay100GravityReloc[];

Overlay100Motion *overlay100UpdateMotion(Overlay100Motion *motion, s32 step) {
    s32 oldPhase, nextPhase, count;
    Overlay100Vec3 *velocity, *oldFrame, *newFrame;
    f32 timeStep, blendScale, gravityStep, verticalBias;

    if (motion == 0) return 0;
    motion->remaining -= step;
    if (motion->remaining <= 0) {
        overlay100RemoveEntry(motion);
        return 0;
    }
    blendScale = step;
    oldPhase = motion->phase;
    nextPhase = oldPhase + 1;
    if (nextPhase >= 3) nextPhase = 0;
    timeStep = blendScale;
    motion->phase = nextPhase;
    if (motion->bank < 3) motion->bank++;
    else {
        motion->nextBank++;
        if (motion->nextBank >= 3) motion->nextBank = 0;
    }
    count = motion->count;
    velocity = motion->velocity;
    oldFrame = motion->frames[oldPhase];
    newFrame = motion->frames[nextPhase];
    if (count--) {
        gravityStep = -(gOverlay100GravityReloc[1] * timeStep);
        verticalBias = ((f32)(step + 1) * gravityStep) * 0.5f;
        do {
            newFrame->x = oldFrame->x + velocity->x * blendScale;
            newFrame->y = oldFrame->y + velocity->y * blendScale + verticalBias;
            newFrame->z = oldFrame->z + velocity->z * blendScale;
            velocity->y += gravityStep;
            velocity++;
            oldFrame++;
            newFrame++;
        } while (count--);
    }
    return motion;
}
