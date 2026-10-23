#ifndef OVERLAY_026_H
#define OVERLAY_026_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "game/math.h"

typedef struct O26Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O26Vec3f;

/* The update's and the plane projection's private vectors were this one. */
typedef O26Vec3f O26Vec3fUpdate;
typedef O26Vec3f O26ProjectionVec3f;

/* Each function reads the object through its own view; the views agree on
 * every field they share but pad over different ones, so they stay separate
 * types (no two name the same layout). */
typedef struct O26Entity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x68];
    s32 owner70;
} O26Entity;

typedef struct O26State {
    s32 word00;
    s32 word04;
    u8 pad08[0x1C];
    f32 value24;
    s32 flags28;
    s16 short2C;
    u16 short2E;
} O26State;

typedef struct O26Object {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    O26Vec3f position;
    u8 pad18[4];
    O26Vec3f direction;
    u8 pad28[6];
    s16 result2E;
    u8 pad30[0x18];
    O26Entity *entity;
    u8 pad4C[0x18];
    O26State *state;
    u8 pad68[0x18];
    s32 active80;
} O26Object;

typedef struct O26Config {
    u8 pad00[0xA];
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    s32 word10;
    s32 word14;
} O26Config;

typedef struct O26MotionRecord {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    O26Vec3fUpdate position;
    O26Vec3fUpdate velocity;
    s16 angularVelocityX;
    s16 angularVelocityY;
    s16 angularVelocityZ;
    s16 timer;
} O26MotionRecord;

typedef struct O26EntityUpdate {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x1C];
    O26Vec3fUpdate position;
    u8 pad30[0x40];
    s32 owner70;
} O26EntityUpdate;

typedef struct O26TrackUpdate {
    s8 index;
    u8 pad001[0x3F9];
    s16 finished3FA;
} O26TrackUpdate;

typedef struct O26LinkedUpdate {
    u8 pad00[0x48];
    O26EntityUpdate *entity48;
    u8 pad4C[0x18];
    O26TrackUpdate *track64;
} O26LinkedUpdate;

typedef struct O26StateUpdate {
    s32 word00;
    O26LinkedUpdate *linked04;
    O26Vec3fUpdate direction08;
    u8 pad14[0xC];
    f32 phase20;
    f32 amplitude24;
    s32 collisionFlags28;
    s16 heading2C;
    s16 wobbleActive2E;
    s8 recordsActive30;
    s8 pendingEffect31;
    s16 recordsFinished32;
    O26MotionRecord records[4];
} O26StateUpdate;

typedef struct O26ObjectUpdate {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    s16 flags06;
    f32 scale08;
    O26Vec3fUpdate position;
    u8 pad18[4];
    O26Vec3fUpdate velocity;
    u8 pad28[6];
    s16 collisionResult2E;
    u8 pad30[0x18];
    O26EntityUpdate *entity48;
    u8 pad4C[0x18];
    O26StateUpdate *state64;
} O26ObjectUpdate;

typedef struct O26ProjectionPlane {
    O26ProjectionVec3f normal;
    f32 constant;
    O26ProjectionVec3f point;
    f32 distance;
    u32 flags;
} O26ProjectionPlane;

typedef struct O26ProjectionResult {
    u8 pad00[8];
    O26ProjectionVec3f normal08;
    O26ProjectionVec3f normal14;
    u8 pad20[8];
    u32 flags28;
} O26ProjectionResult;

typedef struct O26ProjectionOwner {
    u8 pad00[0x64];
    O26ProjectionResult *result64;
} O26ProjectionOwner;

typedef struct O26EffectRecord {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[2];
    f32 scale;
    O26Vec3f position;
    O26Vec3f direction;
    s16 randomX;
    s16 randomY;
    s16 randomZ;
    s16 alpha;
} O26EffectRecord;

typedef struct O26StateD24 {
    u8 pad00[0x14];
    O26Vec3f sourceDirection;
    u8 pad20[0x10];
    u8 active30;
    u8 pad31[3];
    O26EffectRecord effects[4];
} O26StateD24;

typedef struct O26EntityD24 {
    u8 pad00[6];
    u16 flags;
} O26EntityD24;

typedef struct O26ObjectD24 {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    s16 flags06;
    f32 scale;
    O26Vec3f position;
    u8 pad18[0x30];
    O26EntityD24 *entity;
    u8 pad4C[0x18];
    O26StateD24 *state;
    u8 pad68[0x18];
    s32 active80;
} O26ObjectD24;

typedef struct O26Angles2 {
    s16 x;
    s16 y;
} O26Angles2;

typedef struct Overlay26Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay26Group;

typedef struct Overlay26RenderState {
    u8 pad00[0x30];
    s8 enabled;
    u8 pad31[3];
    Overlay26Group group3;
    Overlay26Group group2;
    Overlay26Group group1;
    Overlay26Group group0;
} Overlay26RenderState;

typedef struct Overlay26Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay26Node;

typedef struct Overlay26ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay26ResourceChoice;

typedef struct Overlay26Context {
    u8 pad00[0x50];
    void *drawData;
    u8 pad54[0x10];
    Overlay26RenderState *render;
    Overlay26Node **nodeTable;
} Overlay26Context;

typedef void (*O26CollisionCallback)(O26ProjectionVec3f *start,
                                     O26ProjectionVec3f *end,
                                     O26ProjectionVec3f *direction, f32 distance,
                                     O26ProjectionPlane *plane,
                                     O26ProjectionOwner *owner);

extern void func_overlay_026_F0000D24_187B11C(O26ObjectD24 *object, s32 mode);

extern void mathOneFloatRPY(O26Object *object, O26Vec3f *vector);
/* Accepts either the object angle prefix or a standalone x/y angle record. */
extern void mathOneFloatPY(void *angles, O26Vec3f *direction);
extern void trackMakePolylist(s32 count, O26Vec3f *position,
                              O26Vec3f *origin, f32 *radius,
                              s32 arg4, s32 arg5);
extern s32 func_80010900(O26Vec3f *position, O26Vec3f *origin,
                         f32 radius, void *object, O26CollisionCallback callback);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern void func_80006EA0(void *object);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern f32 sqrtf(f32 value);
extern s32 mathDiffAngle(s16 first, s16 second, s32 step);
extern s32 func_80008128(O26ObjectUpdate *object, f32 arg1, f32 arg2, f32 arg3);
extern void func_8001DCD0(s16 rotation, O26Vec3f *direction,
                          s16 *pitch, s16 *yaw);
extern void overlay37RecordMinimum(s32 index, f32 value);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 first, s16 second, f32 amount);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z,
                          s32 priority, s32 unused);
extern void lightObject(Overlay26Context *, Overlay26Node *, void *, s32);
extern void camPushModelMtx(Gfx **, void *, Overlay26Group *, f32, f32);
extern void camRestoreModelMtx(Gfx **);
extern void texDPInit(Gfx **);

#endif
