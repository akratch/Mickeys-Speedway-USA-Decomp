#ifndef OVERLAY_009_H
#define OVERLAY_009_H

#include "PR/ultratypes.h"

typedef struct O9Angle {
    u8 pad000[4];
    s16 angle;
} O9Angle;

typedef struct O9Motion {
    u8 pad000[0x24];
    f32 velocity;
    s16 angle;
} O9Motion;

typedef struct O9OutputRecord {
    s16 pad0;
    s16 pitch;
    s16 yaw;
    u8 pad6[6];
    f32 x;
    f32 y;
    f32 z;
} O9OutputRecord;

typedef struct O9OutputControl {
    u8 pad000[4];
    f32 lean;
    u8 pad008[0xB4];
    void *handle;
} O9OutputControl;

typedef struct O9OutputState {
    f32 x;
    f32 y;
    f32 throttle;
    f32 scale;
    f32 minimum;
    u8 pad014[8];
    f32 magnitude;
} O9OutputState;

typedef struct O9InputControl {
    u8 pad000[4];
    f32 lean;
    u8 pad008[0xE8];
    s16 angle;
    u8 pad0F2[0x16];
    s16 angleStep;
    u8 pad10A[0x312];
    u32 flags;
    u8 pad420[8];
    s32 inputY;
    s32 inputX;
} O9InputControl;

typedef struct O9InputState {
    f32 x;
    f32 y;
    f32 throttle;
    f32 position;
    f32 acceleration;
    f32 turnRate;
} O9InputState;

typedef struct O9IntegrateOutput {
    u8 pad000[0xC];
    f32 x;
    u8 pad010[4];
    f32 z;
    u8 pad018[4];
    f32 dx;
    f32 zero;
    f32 dz;
} O9IntegrateOutput;

typedef struct O9IntegrateControl {
    u8 pad000[4];
    f32 scaleX;
    f32 scaleZ;
    u8 pad00C[0x2C];
    f32 originX;
    u8 pad03C[4];
    f32 originZ;
    u8 pad044[0x30];
    f32 dirX;
    f32 dirY;
    f32 dirZ;
    f32 speedLimit;
    f32 velocity;
    f32 acceleration;
    u8 pad08C[0x64];
    s16 angle;
    u8 pad0F2[0x8F];
    u8 active;
    u8 pad182[0x2B6];
    s32 mode;
} O9IntegrateControl;

typedef struct O9Point {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
} O9Point;

typedef struct O9Height {
    u8 pad000[0xC];
    f32 height;
} O9Height;

typedef struct O9Hit {
    f32 height;
} O9Hit;

typedef struct O9MotionResult {
    s16 angle;
    s16 targetAngle;
    s16 bank;
    u8 pad006[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[12];
    f32 smoothX;
    f32 smoothY;
} O9MotionResult;

typedef struct O9MotionState {
    s8 mode;
    u8 pad001[3];
    f32 input;
    u8 pad008[12];
    f32 axisX;
    f32 axisY;
    f32 axisZ;
    u8 pad020[188];
    s16 angle;
    u8 pad0DE[10];
    f32 tilt;
    f32 speed;
    s16 angleTarget;
    u8 pad0F2[16];
    s16 direction;
    u8 pad104[4];
    s16 speedScale;
    u8 pad10A[790];
    s32 flags;
} O9MotionState;

typedef struct O9MotionOwner {
    u8 pad000[4];
    s16 bankLimit;
    u8 pad006[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[76];
    O9MotionState *state;
} O9MotionOwner;

extern s16 *D_0;
extern f32 D_54;
extern f32 D_58;
extern f32 D_70;
extern f32 D_78;
extern f32 D_7C;
extern f32 D_2D0;
extern f32 D_300[];
extern f32 D_340[];
extern s16 D_380[];
extern u8 D_388[];
extern s16 *gOverlay9PacketCursor;
extern f32 G_rt_458c4;

extern void ext_o0_1ee14(void *, s8);
extern void ext_o0_1d4c0(void *, void *);
extern void ext_o0_29adc(s16 *, f32 *);
extern s32 ext_o0_1312c(f32, f32, void *, s32, s32);
extern f32 ext_o0_2a470(s32);
extern f32 ext_o0_2a46c(s32);
/* Overlay 9 +0x540 runtime SYMBOL proxies: resident mathDiffAngle/func_8002A8C0. */
extern s32 o9P540MathDiffAngleReloc(s32, s32);
extern f32 o9P540CosReloc(s32);
extern s32 ext_o0_2a5bc(s32, s32);
extern void ext_o0_5aac4(void *, void *, void *);
extern void ext_o0_19668(void *, void *, void *, void *);
extern void ext_o0_1d510(void *, void *, void *, void *, s32);
extern void ext_o0_2d98(void *);
/* Mickey resident call boundaries and the spatial-audio ABI are authenticated. */
extern void amSndPlayXYZ(u16, f32, f32, f32, u8, void **);
extern void ext_o0_3e99c(void *, s32);
extern s32 mathRnd(s32, s32);
extern void amSndSetXYZ(void *, f32, f32, f32);
extern void amSndSetPitchXYZ(void *, u8);
extern void ext_o0_7cd8(void *, f32, f32, f32);
extern void ext_o0_1d920(O9IntegrateOutput *, O9IntegrateControl *, f32);
extern s32 ext_o0_1353c(f32, f32, s32, O9Hit ***);
extern void ext_o0_210b4(f32, s32);
extern s32 ext_o0_214c8(void);

void func_overlay_009_F0000000_1866678(void *, s32);
void func_overlay_009_F0000540_1866BB8(O9Angle *, void *, O9Motion *, s32);
void func_overlay_009_F0000744_1866DBC(O9OutputRecord *, O9OutputControl *,
                                       O9OutputState *, s32);
void func_overlay_009_F00009BC_1867034(s16 *, O9InputControl *, O9InputState *);
void func_overlay_009_F0000CE4_186735C(O9IntegrateOutput *,
                                       O9IntegrateControl *, void *, f32);
void func_overlay_009_F0000F6C_18675E4(O9Point *, O9Height *, s32);
void overlay9Ignore(volatile s32, volatile s32, volatile s32);
void func_overlay_009_F00010B4_186772C(O9MotionResult *, O9MotionOwner *, f32);

#endif
