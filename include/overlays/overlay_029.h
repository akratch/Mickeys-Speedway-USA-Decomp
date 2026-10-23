#ifndef OVERLAY_029_H
#define OVERLAY_029_H

#include "ultra64.h"
#include "n_audio/mbi.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

/* The update's private vector was this one. */
typedef Vec3f Overlay29TailVec3f;

/* One record of the four parallel path tables. The selector steps through
 * them 16 bytes at a time; the sampler reads x/y, the update the selection
 * (it was a padded private view in each file). */
typedef struct Overlay29PathPoint {
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
    u16 selection;
} Overlay29PathPoint;

/* Every function reads the object through its own view; where two views
 * share a name in the per-function files but not a layout, the later one is
 * renamed (Chain, Ratio, Projection, Effect) rather than merged. */
typedef struct Overlay29ChainResource {
    u8 pad0[4];
    s16 first;
    u8 pad6[2];
    u8 last;
} Overlay29ChainResource;

typedef struct Overlay29ChainOwner {
    u8 pad00[0x64];
    Overlay29ChainResource *resource;
} Overlay29ChainOwner;

typedef struct Overlay29Point {
    f32 x;
    f32 y;
} Overlay29Point;

typedef struct Overlay29RatioObject {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay29RatioObject;

typedef struct Overlay29Ratio {
    u8 pad0[4];
    s16 angle;
    u8 pad6[6];
    f32 ratio;
    f32 value;
} Overlay29Ratio;

typedef struct Overlay29SampleObject {
    u8 pad0[0xC];
    f32 frame;
} Overlay29SampleObject;

typedef struct Overlay29SourceState {
    u8 pad000[0x1A8];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 recordIndex;
    u8 selector0;
    u8 selector1;
    u8 selector2;
} Overlay29SourceState;

typedef struct Overlay29Entity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x68];
    void *owner;
} Overlay29Entity;

typedef struct Overlay29Contact {
    void *owner;
    s16 recordIndex;
    u8 pad06[2];
    u8 selector0;
    u8 selector1;
    u8 selector2;
    u8 pad0B[0xF];
    u8 flags;
} Overlay29Contact;

typedef struct Overlay29Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[6];
    Vec3f position;
    u8 pad18[0x30];
    Overlay29Entity *entity;
    u8 pad4C[0x18];
    Overlay29Contact *contact;
    u8 pad68[0x18];
    s32 active;
} Overlay29Object;

typedef struct Overlay29Init {
    u8 pad00[0xA];
    s16 angle0;
    s16 angle1;
    u8 pad0E[2];
    Overlay29Object *object;
} Overlay29Init;

typedef struct Overlay29TailRecord {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[6];
    Overlay29TailVec3f position;
    Overlay29TailVec3f velocity;
    s16 angularVelocity0;
    s16 angularVelocity1;
    s16 angularVelocity2;
    s16 timer;
} Overlay29TailRecord;

typedef struct Overlay29TailEntity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x1C];
    Overlay29TailVec3f position;
    u8 pad30[0x40];
    s32 owner;
} Overlay29TailEntity;

typedef Overlay29PathPoint Overlay29TailDirection;

typedef struct Overlay29TailLinkedState {
    s8 index;
    u8 pad001[0x39B];
    f32 angle;
} Overlay29TailLinkedState;

typedef struct Overlay29TailObject Overlay29TailObject;

typedef struct Overlay29TailState {
    Overlay29TailObject *owner;
    u16 selection0;
    u16 selection1;
    u8 pad08[3];
    s8 recordsActive;
    f32 ratio;
    f32 referenceAngle;
    u32 collisionFlags;
    s16 recordsFinished;
    u8 skipTargeting;
    s8 pendingEffect;
    u8 pad1C[0xC];
    Overlay29TailRecord records[4];
} Overlay29TailState;

struct Overlay29TailObject {
    s16 angle0;
    s16 angle1;
    s16 angularVelocity0;
    s16 flags06;
    f32 scale;
    Overlay29TailVec3f position;
    u8 pad18[4];
    Overlay29TailVec3f velocity;
    u8 pad28[0x20];
    Overlay29TailEntity *entity;
    u8 pad4C[0x18];
    Overlay29TailState *state;
};

typedef struct Overlay29TailLinkedObject {
    u8 pad00[0x48];
    Overlay29TailEntity *entity;
    u8 pad4C[0x18];
    Overlay29TailLinkedState *state;
} Overlay29TailLinkedObject;

typedef struct Overlay29PathState {
    u8 pad00[0x14];
    u32 flags;
    u8 pad18[4];
    Vec3f direction;
} Overlay29PathState;

typedef struct Overlay29Transform {
    Vec3f direction;
    u8 pad0C[4];
    Vec3f position;
    f32 height;
    u32 flags;
} Overlay29Transform;

typedef struct Overlay29ProjectionOwner {
    u8 pad00[0x64];
    Overlay29PathState *state;
} Overlay29ProjectionOwner;

typedef struct Overlay29Record {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[2];
    f32 scalar;
    Vec3f position;
    Vec3f vector;
    s16 random0;
    s16 random1;
    s16 random2;
    s16 value;
} Overlay29Record;

typedef struct Overlay29State {
    u8 pad00[0xB];
    u8 active;
    u8 pad0C[0x10];
    Vec3f direction;
    Overlay29Record records[4];
} Overlay29State;

typedef struct Overlay29EffectResource {
    u8 pad00[6];
    u16 flags;
} Overlay29EffectResource;

typedef struct Overlay29EffectObject {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    s16 flags06;
    f32 scalar;
    Vec3f position;
    u8 pad18[0x30];
    Overlay29EffectResource *resource;
    u8 pad4C[0x18];
    Overlay29State *state;
    u8 pad68[0x18];
    u32 flags80;
} Overlay29EffectObject;

typedef struct Overlay29Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay29Group;

typedef struct Overlay29RenderState {
    u8 pad00[0xB];
    s8 enabled;
    u8 pad0C[0x1C];
    Overlay29Group group3;
    Overlay29Group group2;
    Overlay29Group group1;
    Overlay29Group group0;
} Overlay29RenderState;

typedef struct Overlay29Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay29Node;

typedef struct Overlay29ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay29ResourceChoice;

typedef struct Overlay29Context {
    u8 pad00[0x64];
    Overlay29RenderState *render;
    Overlay29Node **nodeTable;
} Overlay29Context;

/* Section-relative placeholders the surface values from the shipped LOCAL
 * pairs; the 0xF0000000-based link cannot produce them from C objects. */
extern s32 gOverlay29Current;
extern Overlay29PathPoint *gOverlay29Base0;
extern Overlay29PathPoint *gOverlay29Base1;
extern Overlay29PathPoint *gOverlay29Base2;
extern Overlay29PathPoint *gOverlay29Base3;
extern Overlay29PathPoint *gOverlay29Selected0;
extern Overlay29PathPoint *gOverlay29Selected1;
extern Overlay29PathPoint *gOverlay29Selected2;
extern Overlay29PathPoint *gOverlay29Selected3;
extern s32 gOverlay29Value0;
extern s32 gOverlay29Value1;
extern s32 gOverlay29Value2;
extern s32 gOverlay29Value3;
extern s32 gOverlay29Node0;
extern s32 gOverlay29Node4;
extern s32 gOverlay29Node8;
extern s32 gOverlay29NodeC;
extern Overlay29Point *gOverlay29PointA;
extern Overlay29Point *gOverlay29PointB;
extern u8 gOverlay29MinimumYReloc[];
/* The collision query calls the projection back through this address,
 * stored module-relative. */
extern u8 func_overlay_029_F0000EE0_o029Reloc[];

/* SYMBOL calls whose shipped word is zero and whose C still names a
 * placeholder rather than the callee (phase 1 naming). */
extern s32 overlay29AdvanceReloc(s32 value);
extern s32 overlay29CreateReloc(s32 value);
extern s32 overlay29AngleReloc(s16);
extern void func_overlay_029_F00010C4_187E374Reloc(void *object, s32 effect);

extern f32 sqrtf(f32 value);
extern f32 splinePos(f32, f32, f32, f32, f32);
extern void mathOneFloatRPY(void *, Vec3f *);
extern void mathOneFloatPY(void *object, Vec3f *vector);
extern void trackMakePolylist(s32 count, Vec3f *start, Vec3f *end,
                              f32 *radius, void *arg4, s32 arg5);
extern s32 func_80010900(Vec3f *start, Vec3f *end, f32 radius, void *object,
                         void *callback);
extern s32 overlay1TestDirection(Overlay29TailDirection *direction,
                                       f32 x, f32 z);
extern Overlay29TailLinkedObject *overlay1FindPreviousAngle(f32 angle);
extern void overlay37RecordMinimum(s32 index, f32 distance);
/* Called with the object by the update and with no argument by the effect
 * handler, so it has no prototype. */
extern void func_80006EA0();
extern void partUpdateTriggers(void *object, s32 updateRate);
/* s16, as the effect handler stores it: the update's s32 view compiles the
 * same, and the shipped records name this callee at all four sites. */
extern s16 Arctanf(f32 x, f32 z);
extern s32 mathDiffAngle(s16 current, s16 target);
extern s32 func_80008128(Overlay29TailObject *object, f32 x, f32 y, f32 z);
extern s16 mathRnd(s32 low, s32 high);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z, s32 type, s32 arg);
extern void camPushModelMtx(Gfx **, void *, Overlay29Group *, f32, f32);
extern void camRestoreModelMtx(Gfx **);
extern void texDPInit(Gfx **);

#endif
