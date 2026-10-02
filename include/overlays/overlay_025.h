#ifndef OVERLAY_025_H
#define OVERLAY_025_H

#include "PR/ultratypes.h"

typedef struct Overlay25Object Overlay25Object;

typedef struct Overlay25Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay25Vector;

typedef struct Overlay25OwnerState {
    u8 pad00[4];
    f32 scale;
    u8 pad08[0xE8];
    s16 baseAngle;
    u8 padF2[0x0C];
    s16 relativeAngle;
} Overlay25OwnerState;

typedef struct Overlay25Owner {
    u8 pad00[0x64];
    Overlay25OwnerState *state;
} Overlay25Owner;

typedef struct Overlay25InitState {
    s16 lifetime;
    u8 duration;
    u8 activeDuration;
    f32 currentValue;
    f32 velocityX;
    f32 lift;
    f32 velocityZ;
    u8 pad14[4];
    u8 color[3];
    u8 pad1B;
    Overlay25Owner *owner;
} Overlay25InitState;

typedef struct Overlay25EffectState {
    s16 lifetime;
    s8 duration;
    s8 activeDuration;
    f32 multiplier;
    f32 velocityX;
    f32 lift;
    f32 velocityZ;
    u32 flags;
    u8 pad18[4];
    Overlay25Object *owner;
} Overlay25EffectState;

typedef struct Overlay25EntityState {
    u8 pad000[4];
    f32 height;
    u8 pad008[0x341];
    u8 enabled;
    u8 pad34A[0x6C];
    s16 ownerHitCount;
    s16 selfHitCount;
} Overlay25EntityState;

typedef union Overlay25State {
    Overlay25InitState init;
    Overlay25EffectState effect;
    Overlay25EntityState entity;
} Overlay25State;

typedef struct Overlay25Transform {
    f32 value;
    u8 pad04[0x50];
    f32 scaleX;
    f32 scaleY;
} Overlay25Transform;

struct Overlay25Object {
    u8 pad00[6];
    s16 flags;
    f32 value;
    f32 x;
    f32 y;
    s32 z;
    u8 pad18[0x28];
    Overlay25Transform *transform;
    u8 pad44[8];
    Overlay25Vector *vector;
    u8 pad50[0x14];
    Overlay25State *state;
};

typedef struct Overlay25Init {
    u8 pad00[0x0A];
    s16 useOwner;
    Overlay25Owner *owner;
} Overlay25Init;

typedef struct Overlay25Status {
    u8 type;
} Overlay25Status;

typedef struct Overlay25Source {
    u8 pad0[4];
    f32 value;
    u8 pad8[8];
    Overlay25Vector vector;
    u8 pad1C[4];
    s32 flags;
} Overlay25Source;

typedef struct Overlay25VectorState {
    u8 pad0[0x14];
    s32 flags;
} Overlay25VectorState;

typedef struct Overlay25VectorObject {
    u8 pad0[0x64];
    Overlay25VectorState *state;
} Overlay25VectorObject;

extern u16 gOverlay25GlobalFlagsReloc;
extern const u8 gOverlay25ColorsReloc[];

extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);
extern s32 func_800299E8(s32 lower, s32 upper);
extern void overlay25SetVectorFlagsReloc(void);
extern void overlay25DestroyReloc(Overlay25Object *object);
extern void overlay25MoveReloc(Overlay25Object *object, f32 x, f32 y, f32 z);
extern void overlay25SweepReloc(s32 mode, Overlay25Vector *position,
                                Overlay25Vector *movement, f32 *radius,
                                s32 arg4, s32 arg5);
extern s32 overlay25TraceReloc(Overlay25Vector *position,
                               Overlay25Vector *movement, f32 radius,
                               Overlay25Object *object,
                               void (*callback)(void));
extern s32 overlay25QueryObjectsReloc(f32 x, f32 y, s32 z, f32 radius,
                                      s32 mode, Overlay25Object **objects);
extern s32 overlay25CanHitReloc(Overlay25Object *object,
                                Overlay25EntityState *state);
/* Overlay 25 table-1 record 15 resolves through ORT 1471 to Overlay 7 +0x894. */
extern void overlay7DispatchModesReloc(Overlay25Object *owner,
                                       Overlay25Object *object);
extern Overlay25Status *overlay25GetStatusReloc(void);
extern void overlay25NotifyHitReloc(Overlay25Object *object);

#endif
