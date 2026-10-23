#ifndef OVERLAY_079_H
#define OVERLAY_079_H

#include "PR/ultratypes.h"
#include "game/math.h"

/* Overlay 79: a roaming creature with a spawner and a timed link object.
 * Pinned DKR v77/v80 and JFG object scans found no exact donor. */

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

typedef struct Overlay79Object Overlay79Object;
typedef struct Overlay79MotionState Overlay79MotionState;

struct Overlay79MotionState {
    /* 0x00 */ s16 targetAngle;
    /* 0x02 */ s8 mode;
    /* 0x03 */ s8 active;
    /* 0x04 */ s16 event;
    /* 0x06 */ s16 step;
    /* 0x08 */ u32 collisionFlags;
    /* 0x0C */ s32 effectTimer;
    /* 0x10 */ f32 targetX;
    /* 0x14 */ f32 targetZ;
    /* 0x18 */ f32 acceleration;
    /* 0x1C */ f32 speed;
    /* 0x20 */ f32 heightOffset;
    /* 0x24 */ f32 homeX;
    /* 0x28 */ f32 homeY;
    /* 0x2C */ f32 homeZ;
    /* 0x30 */ f32 radiusSquared;
    /* 0x34 */ f32 travelRadius;
    /* 0x38 */ f32 previousHeight;
    /* 0x3C */ void *effect;
    /* 0x40 */ Overlay79Object *target;
};

typedef struct Overlay79Scale {
    f32 value;
} Overlay79Scale;

struct Overlay79Object {
    /* 0x00 */ s16 angle;
    u8 pad02[6];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    u8 pad18[4];
    /* 0x1C */ f32 velocityX;
    /* 0x20 */ f32 velocityY;
    /* 0x24 */ f32 velocityZ;
    /* 0x28 */ f32 floorHeight;
    u8 pad2C[0xF];
    /* 0x3B */ s8 mode;
    /* 0x3C */ s32 field3C;
    /* 0x40 */ Overlay79Scale *scaleSource;
    u8 pad44[0x20];
    /* 0x64 */ Overlay79MotionState *state;
    u8 pad68[0x18];
    /* 0x80 */ u32 flags;
};

/* Spawn entry shared by the creature's initializer and the timer object. */
typedef struct Overlay79Config {
    u8 pad00[0xA];
    /* 0x0A */ s16 angle;
    /* 0x0C */ s16 scale;
    /* 0x0E */ s16 range;
} Overlay79Config;

/* The link object's entry reads one word where the creature's reads two
 * halves, so it keeps its own layout. */
typedef struct Overlay79LinkConfig {
    u8 pad00[0xC];
    /* 0x0C */ s32 link;
} Overlay79LinkConfig;

/* The timer/link object keeps two words behind its state pointer. */
typedef struct Overlay79TimerObject {
    /* 0x00 */ s16 angle;
    u8 pad02[0x62];
    /* 0x64 */ s32 *timers;
} Overlay79TimerObject;

typedef struct Overlay79SpawnDesc {
    /* 0x00 */ s16 objectId;
    /* 0x02 */ u8 kind;
    /* 0x03 */ u8 flags;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 angle;
    /* 0x0C */ Overlay79Object *parent;
    /* 0x10 */ f32 scale;
    u8 pad14[4];
} Overlay79SpawnDesc;

/* The spawner (func_overlay_079_F0001290_18CE230) and its linked node. */
typedef struct Overlay79Node Overlay79Node;
typedef struct Overlay79Spawned Overlay79Spawned;

struct Overlay79Node {
    s32 active;
    Overlay79Node *next;
    u8 pad8[0x5C];
    Overlay79Spawned *state;
};

typedef struct Overlay79SpawnerObject {
    u8 pad0[0xC];
    Overlay79Vector position;
    u8 pad18[0x4C];
    Overlay79Node *node;
} Overlay79SpawnerObject;

struct Overlay79Spawned {
    u8 pad0[0x3C];
    s32 field3C;
    s32 field40;
};

typedef struct Overlay79Plane {
    Overlay79Vector normal;
    f32 constant;
    Overlay79Vector origin;
    f32 distance;
    u32 flags;
} Overlay79Plane;

/*
 * `D_800D3128` is the reserved-BSS selector 0xFFF entry at +0x4D6E8;
 * ROM-table index 1579 covers both of this module's uses, so it is one
 * object (Tier A).
 */
typedef struct Overlay79Flags {
    /* 0x00 */ u32 unused0 : 5;
    u32 stage : 4;
    u32 unused9 : 4;
    u32 disabled : 1;
    u32 unused14 : 18;
    u8 pad04[0xA];
    /* 0x0E */ u16 progress;
} Overlay79Flags;

extern Overlay79Flags D_800D3128;
extern f32 gOverlay79InitScaleReloc;
extern s32 gOverlay79CounterReloc;
extern f32 gOverlay79ModeFactors[8];
extern f32 gOverlay79Constants[16];

/* Resident callees. */
extern void *func_80005750(s32 *count);
extern void *func_8000590C(Overlay79SpawnDesc *desc, s32 count);
extern void func_80006EA0(void *object);
extern s32 func_80008128(Overlay79Object *object, f32 x, f32 y, f32 z);
extern s32 func_80010900(f32 *start, f32 *end, f32 height,
                         Overlay79Object *object, void *callback);
extern void func_800291B4(void);
extern s32 func_8005ABA8(Overlay79Object *object, f32 factor, f32 updateRate);
extern void func_8005AD64(Overlay79Object *object, s32 mode, s32 index,
                          f32 value);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 sqrtf(f32 value);

/* func_overlay_079_F0000134_18CD0D4 passes the static
 * func_overlay_079_F0000FA0_18CDF40 to func_80010900 as a callback. The
 * shipped %hi/%lo pair is a LOCAL record whose words hold the bare module
 * offset (the loader adds the module base), while the linked static would
 * carry the 0xF0000000 link base in its %hi, so the site names the
 * surface's placeholder for that offset until the tables are packer output.
 * The callback therefore stays a global, not static, function: it has no ORT
 * row, but nothing in the C names it, and IDO drops an unreferenced static. */
void func_overlay_079_F0000FA0_18CDF40(void *unused, Overlay79Vector *pos,
                                       Overlay79Vector *vel, f32 radius,
                                       Overlay79Plane *plane,
                                       Overlay79Object *object);
extern void func_overlay_079_F0000FA0_o079Reloc(
    void *unused, Overlay79Vector *pos, Overlay79Vector *vel, f32 radius,
    Overlay79Plane *plane, Overlay79Object *object);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern void mathOneFloatRPY(Overlay79Object *object, f32 *vector);
extern void amSndPlay(s32 id, s32 unused1);
extern void amSndPlayXYZ(u16 id, f32 x, f32 y, f32 z, s32 priority,
                         void **handle);
extern void amSndSetXYZ(void *handle, f32 x, f32 y, f32 z);
extern void amSndStopXYZ(void *handle);
extern void partUpdateTriggers(Overlay79Object *object, s32 updateRate);
extern void trackMakePolylist(s32 mode, f32 *start, f32 *end, f32 *height,
                              void *unused, s32 flags);

/* Exported functions. */
void func_overlay_079_F0000000_18CCFA0(Overlay79Object *object,
                                       Overlay79Config *config,
                                       s32 preserveState);
void func_overlay_079_F0000134_18CD0D4(Overlay79Object *object,
                                       s32 updateRate);
void overlay79SetLink(Overlay79TimerObject *object, Overlay79LinkConfig *entry);
void func_overlay_079_F0001290_18CE230(Overlay79SpawnerObject *object,
                                       s32 unused1);
void overlay79InitState(Overlay79TimerObject *object, Overlay79Config *entry);
void overlay79UpdateTimers(Overlay79TimerObject *object, s32 step);

#endif /* OVERLAY_079_H */
