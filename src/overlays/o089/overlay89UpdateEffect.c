#include "PR/ultratypes.h"

typedef struct Overlay89Transform {
    s16 angleA;
    s16 angleB;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay89Transform;

typedef struct Overlay89EffectState {
    u8 pad00[4];
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 pad08[0x18];
    s32 enabled;
    f32 radius;
    f32 scale;
    void *handle;
} Overlay89EffectState;

extern f32 gOverlay89EffectScale;
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern void trackLightMove(void *handle, f32 x, f32 y, f32 z);
extern void *trackLightAdd(f32 x, f32 y, f32 z, f32 scale,
                                        f32 scaledSize, s32 red, s32 green,
                                        s32 blue);

/*
 * DKR v77/v80 and JFG have no exact donor for this effect update. The
 * per-file R4300 multiply scheduling flag is required for the shipped hazard
 * nop.
 */
void overlay89UpdateEffect(const Overlay89Transform *transform,
                           Overlay89EffectState *effect) {
    f32 x;
    f32 y;
    f32 z;
    f32 trigA0;
    f32 trigB0;
    f32 trigA1;
    f32 trigB1;
    f32 scaleValue;
    volatile f32 scaleArg;

    if (effect->enabled == 0) {
        return;
    }

    trigA0 = func_8002A8C0(transform->angleA);
    trigB0 = func_8002A8BC(transform->angleA);
    trigA1 = func_8002A8C0(transform->angleB);
    trigB1 = func_8002A8BC(transform->angleB);

    scaleValue = (scaleArg = effect->scale);
    scaleValue *= gOverlay89EffectScale;
    z = effect->radius;
    trigB1 = z * trigB1;
    x = transform->x - (trigB1 * trigA0);
    y = transform->y + (effect->radius * trigA1);
    z = transform->z - (trigB1 * trigB0);

    if (effect->handle != NULL) {
        trackLightMove(effect->handle, x, y, z);
    } else {
        effect->handle = trackLightAdd(
            x,
            y,
            z,
            scaleArg,
            scaleValue,
            (effect->red * effect->intensity) >> 8,
            (effect->green * effect->intensity) >> 8,
            (effect->blue * effect->intensity) >> 8);
    }
}
