#ifndef OVERLAY_094_H
#define OVERLAY_094_H

#include "PR/ultratypes.h"

typedef struct Overlay94Record {
    u8 pad00[0x40];
    u8 queryState[1];
} Overlay94Record;

typedef struct Overlay94Entity {
    s32 savedValue;
    u8 pad04[4];
    s16 status;
    s16 recordIndex;
    Overlay94Record *records[1];
} Overlay94Entity;

typedef struct Overlay94State {
    f32 current;
    s8 selector;
    s8 active;
    s16 velocity;
    s32 angle;
    s32 accumulator;
    s16 command;
} Overlay94State;

typedef struct Overlay94Object {
    s16 kind;
    u8 pad02[6];
    f32 scale;
    u8 pad0C[0x44];
    void *renderResource;
    u8 pad54[0x10];
    Overlay94State *state;
    Overlay94Entity **entityRef;
} Overlay94Object;

typedef struct Overlay94Init {
    u8 pad00[0x0A];
    s16 kind;
    s32 selector;
    f32 scale;
} Overlay94Init;

typedef struct Overlay94SavedValue {
    s32 unused[2];
    s32 value;
} Overlay94SavedValue;

extern s32 gO94Value;
extern s32 gOverlay94Value;
extern f32 gO94Const0;
extern f32 gO94Const4;
extern f32 gO94Const8;
extern f32 gO94ConstC;
extern f32 gO94Const10;
extern f32 gO94Const14;
extern f32 gO94Const18;
extern f32 gO94Const1C;
extern f32 gO94Const20;
extern f32 gO94Const24;

extern u32 func_800254FC(s32 selector);
extern s32 func_8002565C(s32 selector);
extern f32 func_8002A878(f32 amount, s32 updateRate);
extern s32 func_8002A910(f32 z, f32 x);
extern void func_8002B040(void *queryState, f32 x, f32 y, f32 z,
                          f32 *out0, f32 *out1, f32 *out2);
extern s32 Arctanf(f32 value0, f32 value1);
extern s32 func_8005ABA8(Overlay94Object *object, f32 current, f32 rate);
extern void func_8005AD64(Overlay94Object *object, s32 arg1, s32 arg2,
                          f32 arg3);
extern void func_8005AF14(Overlay94Entity *entity, s32 savedValue,
                          Overlay94Object *object);
extern void lightObject(Overlay94Object *object, Overlay94Entity *entity,
                        void *renderResource, Overlay94Record *record);

void overlay94InitializeController(Overlay94Object *object,
                                   const Overlay94Init *init);
void overlay94UpdateController(Overlay94Object *object, s32 updateRate);
void overlay94SetValue(s32 value);

#endif
