#ifndef OVERLAY_022_H
#define OVERLAY_022_H

#include "PR/ultratypes.h"

typedef struct O22Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O22Vec3f;

typedef O22Vec3f Vec3f;

typedef struct O22Model {
    u8 pad00[6];
    u16 flags06;
    u8 pad08[0x68];
    void *owner70;
} O22Model;

typedef struct O22State {
    s8 mode;
    u8 flags;
    u8 pad02[2];
    f32 speed;
    O22Vec3f up;
    O22Vec3f normal;
    O22Vec3f contact;
    O22Vec3f previousPosition;
    u8 pad38[4];
    void *soundHandle;
} O22State;

typedef struct O22Object {
    u8 pad00[0xC];
    O22Vec3f position;
    u8 pad18[4];
    O22Vec3f velocity;
    u8 animationState28[6];
    s16 result2E;
    u8 animationState30[0x18];
    O22Model *model;
    u8 pad4C[0x18];
    O22State *state;
    void **animationEntries;
    u8 pad6C[0x14];
    u32 flags80;
} O22Object;

typedef struct Overlay22Plane {
    Vec3f normal;
    u8 pad0C[4];
    Vec3f point;
    f32 distance;
    u32 flags;
} Overlay22Plane;

typedef struct Overlay22Result {
    u8 pad00;
    u8 flags01;
    s16 mode02;
    u8 pad04[4];
    Vec3f normal08;
    Vec3f source14;
    Vec3f point20;
} Overlay22Result;

typedef struct Overlay22Owner {
    u8 pad00[0x64];
    Overlay22Result *result64;
} Overlay22Owner;

/* Data-relative placeholders: the shipped LOCAL pairs store section-relative
 * offsets, which the 0xF0000000-based link cannot produce from the defined
 * objects: the initializer's distance and the resolver's limit table. */
extern f32 gOverlay22DistanceReloc;
extern f32 D_0[];
/* The address the collision query calls back, stored module-relative. */
extern u8 func_overlay_022_F0000A7C_o022Reloc[];
/* SYMBOL calls to the module's exported remover store zero, so they name a
 * placeholder rather than the in-module definition; the initializer passes a
 * third argument, hence no prototype. */
extern void func_overlay_022_F0000D30_1878E38Reloc();

extern void mathOneFloatRPY(void *, Vec3f *);
extern s32 func_8000FAE0(f32, f32, f32);
extern s32 Arctanf(f32 y, f32 x);
extern f32 sqrtf(f32 value);
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern void trackMakePolylist(s32 count, O22Vec3f *start, O22Vec3f *end,
                              f32 *distance, void *arg4, s32 arg5);
extern s32 func_80010900(O22Vec3f *start, O22Vec3f *end, f32 distance,
                         void *object, void *callback);
extern s32 func_80008128(O22Object *object, f32 x, f32 y, f32 z);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern u32 func_80001620(u16 soundId);
extern void amSndStopXYZ(void *handle);
extern void amSndPlayXYZ(u16 soundId, f32 x, f32 y, f32 z, u8 priority,
                          void **handle);
extern void amSndSetVolXYZ(void *handle, u8 volume);
extern void texAnimateSprite(void *entry, s32 *mode, s32 animationId,
                          void *state, s32 updateRate);
extern void func_80006EA0(void *object);

#endif
