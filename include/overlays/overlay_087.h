#ifndef OVERLAY_087_H
#define OVERLAY_087_H

#include "PR/ultratypes.h"
#include "game/math.h"

/* Overlay 87: a lifting, turning object that reacts to a nearby racer.
 * Pinned DKR v77/v80 and JFG object scans found no exact donor. */

typedef struct Overlay87MotionState {
    /* 0x00 */ f32 liftOffset;
    /* 0x04 */ f32 radius;
    /* 0x08 */ f32 radiusSquared;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 verticalVelocity;
    /* 0x1C */ f32 targetHeight;
    /* 0x20 */ f32 verticalAcceleration;
    /* 0x24 */ f32 bounceAcceleration;
    /* 0x28 */ s16 turning;
    /* 0x2A */ s16 phase;
    /* 0x2C */ s16 angle;
    /* 0x2E */ s16 targetAngle;
    u8 pad30[2];
    /* 0x32 */ s16 angularVelocity;
    /* 0x34 */ s16 angularAcceleration;
} Overlay87MotionState;

typedef struct Overlay87Object {
    /* 0x00 */ s16 angle;
    /* 0x02 */ s16 turnAmount;
    /* 0x04 */ s16 liftAmount;
    u8 pad06[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    u8 pad18[8];
    /* 0x20 */ f32 verticalVelocity;
    u8 pad24[0x17];
    /* 0x3B */ s8 updateMode;
    u8 pad3C[4];
    /* 0x40 */ f32 *scaleSource;
    u8 pad44[0x20];
    /* 0x64 */ Overlay87MotionState *state;
} Overlay87Object;

typedef struct Overlay87Config {
    u8 pad00[0xA];
    /* 0x0A */ s16 angle;
    /* 0x0C */ s16 scale;
    /* 0x0E */ s16 value04;
    /* 0x10 */ s16 value00;
    /* 0x12 */ s16 value08;
} Overlay87Config;

extern f32 gOverlay87InitScaleReloc;
extern void *gOverlay87Current;
extern void *gOverlay87UpdateTable[];
extern f32 gOverlay87TurnFactor;
extern f32 gOverlay87StraightFactor;
extern f32 gOverlay87ApproachFactor;
extern f32 gOverlay87PositiveDivisor;
extern f32 gOverlay87PositiveAcceleration;
extern f32 gOverlay87NegativeDivisor;
extern f32 gOverlay87NegativeAcceleration;
extern f32 gOverlay87Gravity;

/* Resident callees. */
extern Overlay87Object **func_80005750(s32 *count);
extern s32 func_80008128(Overlay87Object *object, f32 x, f32 y, f32 z);
extern s32 func_8005ABA8(Overlay87Object *object, void *update,
                         f32 updateRate);
extern void func_8005AD64(Overlay87Object *object, s32 mode, s32 index,
                          f32 value);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 sqrtf(f32 value);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z, s32 priority,
                         void **handle);
extern void amSndStopXYZ(void *resource);

/* Exported functions. */
void overlay87InitializeObject(Overlay87Object *object,
                               Overlay87Config *config, s32 preserveState);
void func_overlay_087_F0000128_18D3090(Overlay87Object *object,
                                       s32 updateRate);
void overlay87ReleaseCurrent(void);

#endif /* OVERLAY_087_H */
