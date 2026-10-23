#ifndef OVERLAY_036_H
#define OVERLAY_036_H

#include "ultra64.h"

/* Every function reads the module's objects and states through its own
 * view; where two per-function views share a name but not a layout, the
 * later one carries its function's suffix rather than being merged. */
typedef struct Overlay36Found Overlay36Found;

typedef struct Overlay36Entity Overlay36Entity;

typedef struct Overlay36Data {
    u8 pad00[9];
    s8 marker;
    u8 intensity;
    u8 enabled;
} Overlay36Data;

typedef struct Overlay36State {
    u8 pad00[4];
    u8 mode;
} Overlay36State;

typedef struct Overlay36Entry {
    s16 unused;
    s16 value;
} Overlay36Entry;

typedef struct Overlay36Object {
    u8 pad00[0x28];
    f32 value;
    u8 pad2C[0x1A];
    s16 type;
    u8 pad48[8];
    Overlay36State *state;
    u8 pad54[0x10];
    Overlay36Data *data;
    Overlay36Entry **entry;
} Overlay36Object;

typedef struct Overlay36QueueEntry {
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    s32 arg4;
} Overlay36QueueEntry;

typedef struct Overlay36State_UpdateInteractiveEntity {
    s32 value0;
    u16 timer4;
    u8 flags6;
    u8 usePosition7;
    u8 positionMode8;
    u8 pad9;
    u8 alphaA;
    u8 countdownB;
} Overlay36State_UpdateInteractiveEntity;

typedef struct Overlay36Record {
    s16 kind0;
    u8 callbackArg2;
    u8 flags3;
    void (*timerCallback4)(Overlay36Entity *, s32);
    void (*normalCallback8)(Overlay36Found *, Overlay36Entity *, s32, u8);
    void (*alternateCallbackC)(Overlay36Found *, Overlay36Entity *);
    u8 pad10[0xC];
    u16 normalEffect1C;
    u16 alternateEffect1E;
    u8 pad20[0xC];
    f32 queryRadius2C;
    f32 minimumY30;
    f32 maximumY34;
    u8 pad38[2];
    s16 targetAlpha3A;
} Overlay36Record;

typedef struct Overlay36Marker {
    u8 pad0[4];
    u8 active4;
} Overlay36Marker;

typedef struct Overlay36FoundState {
    u8 pad000[0x1A0];
    Overlay36Record *record1A0;
} Overlay36FoundState;

typedef struct Overlay36Work {
    Overlay36FoundState *foundState;
    Overlay36Found *found;
    u8 pad08[0x20];
} Overlay36Work;

typedef struct Overlay36Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay36Position;

typedef struct Overlay36SpawnDescriptor {
    s16 type;
    u8 value2;
    u8 value3;
    s16 x;
    s16 y;
    s16 z;
} Overlay36SpawnDescriptor;

typedef struct Overlay36SpawnState {
    u8 pad00[4];
    s16 value4;
} Overlay36SpawnState;

typedef struct Overlay36Spawned {
    u8 pad00[0x3C];
    s32 value3C;
    u8 pad40[0x24];
    Overlay36SpawnState *state;
} Overlay36Spawned;

typedef struct Overlay36World {
    u8 pad000[0x192];
    u8 timer192;
    u8 pad193[7];
    u8 inactive19A;
    u8 timer19B;
    u8 pad19C[4];
    s16 *type1A0;
} Overlay36World;

typedef struct Overlay36NestedFlags {
    s16 flags;
} Overlay36NestedFlags;

typedef struct Overlay36VectorState {
    s16 kind;
    s16 value;
    s16 mode;
    u8 pad06[0x16];
    f32 vector[2];
    f32 amount;
    u8 pad28[0x50];
    Overlay36NestedFlags *nested;
} Overlay36VectorState;

typedef struct Overlay36State_CheckNearbyHeight {
    u8 pad0[4];
    u16 active;
} Overlay36State_CheckNearbyHeight;

typedef struct Overlay36Object_CheckNearbyHeight {
    u8 pad0[6];
    s16 flags;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State_CheckNearbyHeight *state;
} Overlay36Object_CheckNearbyHeight;

typedef struct Overlay36Nearby {
    u8 pad0[0x10];
    f32 y;
} Overlay36Nearby;

typedef struct Overlay36WorldState {
    u8 pad0[0xA];
    u8 changed;
} Overlay36WorldState;

typedef struct Overlay36SelectObject {
    u8 pad000[0x19A];
    u8 state;
    u8 countdown;
    s32 timer;
    void *data;
} Overlay36SelectObject;

typedef struct Overlay36SelectSource {
    u8 pad00[0x64];
    Overlay36SelectObject *object;
} Overlay36SelectSource;

typedef struct Overlay36PositionObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay36PositionObject;

typedef struct Overlay36SpawnRequest {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36PositionObject *owner;
} Overlay36SpawnRequest;

typedef struct Overlay36SpawnedObject {
    u8 pad00[0x3C];
    s32 state;
} Overlay36SpawnedObject;

typedef struct Overlay36Inner {
    s8 selector;
    u8 pad001[0x19A];
    u8 countdown;
    u8 pad19C[0x1E9];
    u8 strength;
} Overlay36Inner;

typedef struct Overlay36Object_ChooseWeightedState {
    u8 pad000[0x64];
    Overlay36Inner *inner;
} Overlay36Object_ChooseWeightedState;

typedef struct Overlay36Choice {
    u8 pad0[3];
    u8 value;
} Overlay36Choice;

typedef struct Overlay36Node {
    u8 pad0[4];
    s16 value4;
    s16 flags6;
} Overlay36Node;

typedef struct Overlay36AltTable {
    u8 pad0[0xA8];
    u8 values[1];
} Overlay36AltTable;

typedef struct Overlay36ScaleDivisorData {
    u8 pad0[0x28];
    f32 value;
} Overlay36ScaleDivisorData;

typedef struct Overlay36ScaleMultiplierData {
    u8 pad0[0x2C];
    f32 value;
} Overlay36ScaleMultiplierData;

typedef struct Overlay36StrengthMultiplierData {
    u8 pad0[0x30];
    f32 value;
} Overlay36StrengthMultiplierData;

typedef struct Overlay36NodeDataA {
    u8 pad0[0x150];
    Overlay36Node *node;
} Overlay36NodeDataA;

typedef struct Overlay36NodeDataB {
    u8 pad0[0x14C];
    Overlay36Node *node;
} Overlay36NodeDataB;

typedef struct Overlay36PrepareObject {
    u8 pad000;
    s8 kind;
    u8 pad002[0x198];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    s32 owner;
    u8 pad1A4[4];
    u16 flags;
} Overlay36PrepareObject;

typedef struct Overlay36PrepareSource {
    u8 pad00[0x64];
    Overlay36PrepareObject *object;
} Overlay36PrepareSource;

typedef struct Overlay36Action {
    u8 pad00[0x20]; u16 kind;
} Overlay36Action;

typedef struct Overlay36State_SpawnLinked7F {
    u8 pad000[0xA8]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action *action;
} Overlay36State_SpawnLinked7F;

typedef struct Overlay36Object_SpawnLinked7F {
    s16 id0; s16 id2; s16 id4; u8 pad06[6]; f32 x, y, z;
    u8 pad18[0x4C]; Overlay36State_SpawnLinked7F *state;
} Overlay36Object_SpawnLinked7F;

typedef struct Overlay36Request0D8C {
    s16 kind; s16 pad02; s16 x; s16 y; s16 z;
    s16 id0; s16 id2; s16 id4;
    Overlay36Object_SpawnLinked7F *owner;
    void *directionalObject;
} Overlay36Request0D8C;

typedef struct Overlay36Locals0D8C {
    f32 offset[3];
    s32 gap40;
    Overlay36Request0D8C request;
} Overlay36Locals0D8C;

typedef struct Overlay36SpawnedObject_SpawnLinked7F {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject_SpawnLinked7F;

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Overlay36State_SpawnDirectional {
    u8 pad000[0xA8]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action *action;
} Overlay36State_SpawnDirectional;

typedef struct Overlay36Object_SpawnDirectional {
    s16 id0; s16 id2; s16 id4; u8 pad06[6];
    f32 x, y, z; u8 pad18[0x4C]; Overlay36State_SpawnDirectional *state;
} Overlay36Object_SpawnDirectional;

typedef struct Overlay36SpawnRequestF20 {
    s16 kind; s16 pad02; s16 x; s16 y; s16 z; s16 count;
    s32 link; f32 value; Overlay36Object_SpawnDirectional *owner;
} Overlay36SpawnRequestF20;

typedef struct Overlay36SpawnDataF20 {
    f32 offset[3];
    Overlay36SpawnRequestF20 request;
} Overlay36SpawnDataF20;

typedef struct Overlay36SpawnedObject_SpawnDirectional {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject_SpawnDirectional;

typedef struct Overlay36Action_SpawnConditional {
    u8 pad00[0x24]; u16 kind;
} Overlay36Action_SpawnConditional;

typedef struct Overlay36State_SpawnConditional {
    u8 pad000[4]; f32 direction; u8 pad008[0xA0]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action_SpawnConditional *action;
} Overlay36State_SpawnConditional;

typedef struct Overlay36Object_SpawnConditional {
    s16 id0; s16 id2; s16 id4; u8 pad06[6];
    f32 x, y, z; u8 pad18[0x4C]; Overlay36State_SpawnConditional *state;
} Overlay36Object_SpawnConditional;

typedef struct Overlay36SpawnedObject_SpawnConditional {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject_SpawnConditional;

typedef struct Overlay36State_SpawnOffsetA9 {
    u8 pad000[0xA8]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action *action;
} Overlay36State_SpawnOffsetA9;

typedef struct Overlay36Object_SpawnOffsetA9 {
    s16 id0; s16 id2; u8 pad04[8]; f32 x, y, z;
    u8 pad18[0x4C]; Overlay36State_SpawnOffsetA9 *state;
} Overlay36Object_SpawnOffsetA9;

typedef struct Overlay36Request1214 {
    s16 kind; s16 pad02; s16 x; s16 y; s16 z;
    s16 id0; s16 id2; s16 pad0E; Overlay36Object_SpawnOffsetA9 *owner;
} Overlay36Request1214;

typedef struct Overlay36Locals1214 {
    s32 actionKind;
    f32 offset[3];
    Overlay36Request1214 request;
} Overlay36Locals1214;

typedef struct Overlay36SpawnedObject_SpawnOffsetA9 {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject_SpawnOffsetA9;

typedef struct Overlay36Action_SpawnAndUpdate {
    u8 pad00[0x22];
    u16 kind;
} Overlay36Action_SpawnAndUpdate;

typedef struct Overlay36State_SpawnAndUpdate {
    u8 pad000[0xA8];
    void *resource;
    u8 pad0AC[0xEE];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    Overlay36Action_SpawnAndUpdate *action;
} Overlay36State_SpawnAndUpdate;

typedef struct Overlay36Object_SpawnAndUpdate {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State_SpawnAndUpdate *state;
} Overlay36Object_SpawnAndUpdate;

typedef struct Overlay36SpawnRequest_SpawnAndUpdate {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36Object_SpawnAndUpdate *owner;
} Overlay36SpawnRequest_SpawnAndUpdate;

typedef struct Overlay36TickObject {
    u8 pad000[0x16A];
    s16 timer;
    u8 pad16C[0x2E];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    s32 owner;
    u8 pad1A4[4];
    u16 flags;
} Overlay36TickObject;

typedef struct Overlay36TickSource {
    u8 pad00[0x64];
    Overlay36TickObject *object;
} Overlay36TickSource;

typedef struct Overlay36SpawnRequest_UpdatePeers {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36Object *owner;
} Overlay36SpawnRequest_UpdatePeers;

typedef struct Overlay36EffectState {
    u8 reserved000[0x19A];
    u8 state;
    u8 countdown;
    u8 reserved19C[4];
    void *activeAction;
} Overlay36EffectState;

typedef struct Overlay36EffectSource {
    u8 reserved00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x4C];
    Overlay36EffectState *effectState;
} Overlay36EffectSource;

typedef struct Overlay36SpawnRequest_SpawnFinalEffect {
    s16 effectType;
    s16 reserved02;
    s16 x;
    s16 y;
    s16 z;
    s16 mode;
    Overlay36EffectSource *source;
    Overlay36EffectState *effectState;
} Overlay36SpawnRequest_SpawnFinalEffect;

typedef struct Overlay36SpawnedObject_SpawnFinalEffect {
    u8 reserved00[0x3C];
    s32 owner;
} Overlay36SpawnedObject_SpawnFinalEffect;

extern u8 *func_80028F54(void);
extern s32 gOverlay36QueueCount;
extern Overlay36QueueEntry *gOverlay36QueueNext;
extern Overlay36QueueEntry gOverlay36QueueBase[];
extern void overlay1ReadSelection();
extern Overlay36Record gOverlay36Records[];
extern u8 D_8007BF0C;
extern s32 gOverlay36Elapsed;
extern Overlay36Entity *gOverlay36CurrentEntity;
extern Overlay36State_UpdateInteractiveEntity *gOverlay36CurrentState;
extern s32 D_80078F20;
extern void func_80006EA0(void);
extern void partUpdateTriggers(Overlay36Entity *entity, s32 elapsed);
extern void overlay36ReleaseEntityReloc(Overlay36Entity *entity);
extern void texAnimateSprite(void *resource, s32 *mode, s32 count, void *angles, s32 elapsed);
extern void mathOneFloatPY(Overlay36VectorState *state, f32 *vector);
extern s32 func_8005776C(f32, f32, f32, f32, s32, Overlay36Nearby **);
extern u8 *gOverlay36WorldStateReloc[];
extern u8 gOverlay36StateValues[];
extern u8 gOverlay36StateData[];
extern void overlay4AttachObject();
extern void *gOverlay36CallTarget;
extern u16 gOverlay36EnabledMask;
extern u8 gOverlay36Weights[14][10];
extern Overlay36AltTable gOverlay36AltTable;
extern Overlay36ScaleDivisorData gOverlay36ScaleDivisor;
extern Overlay36ScaleMultiplierData gOverlay36ScaleMultiplier;
extern Overlay36StrengthMultiplierData gOverlay36StrengthMultiplier;
extern Overlay36NodeDataA gOverlay36NodeA;
extern Overlay36NodeDataB gOverlay36NodeB;
extern u32 mathRnd();
extern f32 overlay1DistanceFromCurrent(Overlay36Object_ChooseWeightedState *object);
extern Overlay36Choice *func_80028F60(s8 selector, s32 argument);
extern void overlay36ActionReloc();
extern f32 gOverlay36RodataBaseReloc[];
extern void *overlay1FindDirectionalObject(Overlay36Object_SpawnLinked7F *object, s32 first, s32 second, f32 cosine, f32 maximum);
extern void amSndStopXYZ(void *resource);
extern void overlay7CommitSelection(s32 value);
extern void *func_80005820(s32 index);
extern s32 overlay27Activate(void *resource);
extern s32 gOverlay36Mode;
extern void amSndPlayXYZ(u16 kind, f32 x, f32 y, f32 z, s32 mode, void *resource);
/* The spawner is called with each caller's own request record (two or three
 * arguments, none floating-point), so it has no prototype. */
extern void *func_8000590C();
extern void mathOneFloatRPY(void *object, f32 *offset);
/* Two resident calls named by their shipped identities through the
 * surface's <name>_o036Reloc aliases; each caller's view of the callee
 * differs from the module's other uses (the initializer reads mathRnd as
 * s32, the entity update queries with its own result record). Both were
 * one placeholder, overlay36QueryReloc, before. */
extern s32 mathRnd_o036Reloc(s32 index, s16 value, Overlay36Data *data);
extern s32 func_8005776C_o036Reloc(f32 x, f32 y, f32 z, f32 radius, s32 one,
                                   Overlay36Found **found);

#endif
