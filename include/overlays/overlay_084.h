#ifndef OVERLAY_084_H
#define OVERLAY_084_H

#include "PR/ultratypes.h"

/* Overlay 84: a node-cycling camera selector. Pinned DKR v77/v80 and JFG
 * object scans found no exact donor for any of its functions. */

/* Per-node selection record; two callers index different byte quartets. */
typedef struct Overlay84Choice {
    u8 pad00[0x10];
    u8 slot;
    u8 activateResource0;
    u8 first;
    u8 second;
    u8 activateResource1;
    u8 loadResource0;
    u8 loadResource1;
    u8 pad17;
    u8 loadResource2;
    u8 loadResource3;
    u8 activateResource2;
    u8 activateResource3;
} Overlay84Choice;

typedef struct Overlay84Node {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84Choice *choice;
    u8 pad40[4];
    s16 kind;
} Overlay84Node;

/* One 0x2C-byte record of an animation path. */
typedef struct Overlay84ResourceRecord {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x14];
} Overlay84ResourceRecord;

typedef struct Overlay84Resource {
    u8 pad00[0x16];
    u8 flags;
    u8 recordIndex;
    u8 pad18[4];
    f32 scale;
    Overlay84ResourceRecord *records;
} Overlay84Resource;

/* The camera yaw is written as a word and read back by its low half. */
typedef union Overlay84Angle {
    s32 word;
    struct {
        s16 upper;
        s16 base;
    } half;
} Overlay84Angle;

typedef struct Overlay84State {
    u8 initialized;
    s8 current;
    s8 last;
    s8 actionTimer;
    s8 blendTimer;
    s8 action;
    s8 inputLatch;
    s8 inputTimer;
    u8 pad08;
    u8 mode;
    u8 resource;
    u8 pad0B[3];
    s16 angleOffset;
    s16 targetTilt;
    s16 tilt;
    s16 angle;
    s16 targetAngle;
    u32 flags;
    u8 pad1C[4];
    Overlay84Angle outputAngle;
    f32 targetHeight;
    f32 height;
    f32 blend;
    f32 x;
    f32 y;
    f32 z;
    s32 first;
    s32 second;
    Overlay84Node *nodes[32];
    u8 selectedResource;
    u8 active;
    u8 marked;
    u8 padC7;
    u32 disabled;
} Overlay84State;

typedef struct Overlay84Scale {
    u8 pad00[0x0A];
    s16 radius;
} Overlay84Scale;

typedef struct Overlay84Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84Scale *scale;
    u8 pad40[0x24];
    Overlay84State *state;
} Overlay84Object;

/* Camera record returned by camGetPtr. */
typedef struct Overlay84Output {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x26];
    s16 verticalAngle;
} Overlay84Output;

typedef struct Overlay84Pair {
    s16 first;
    s16 second;
} Overlay84Pair;

typedef struct Overlay84Entry {
    u8 pad0[0xC];
    s16 second;
    s16 first;
} Overlay84Entry;

extern Overlay84Object *gOverlay84Object;
/* Same address as gOverlay84Object; overlay84AdvanceCurrent needs a second
 * name for its reload (see there). */
extern Overlay84Object *gOverlay84ObjectWrite;
extern u32 D_800D31B8;
extern s16 D_800D31C0;
extern f32 gOverlay84BlendStep;

/* Resident callees. */
extern Overlay84Node **func_8000572C(s32 *start, s32 *end);
extern s32 Arctanf(f32 x, f32 z);
extern void amSndPlay(u16 soundId, void **handle);
extern s32 mathDiffAngle(s32 current, s32 target);
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern void camSetNo(s32 camera);
extern Overlay84Output *camGetPtr(void);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern void func_8005055C(u8 resource);
extern Overlay84Resource *func_800508B4(u8 resource);
extern void animseqStartPath(u8 resource);
extern void func_overlay_041_F0000854_1887B8C(Overlay84Resource *resource,
                                              f32 scale, s32 arg2, s32 arg3);
/* Runtime relocation identities: overlay84GetResource -> func_800508B4 and
 * overlay84ReleaseResource -> animseqStartPath. The getter is declared
 * without a prototype because overlay84LoadCurrent widens the call to
 * (resource, choice, object) to keep choice/object in a1/a2, while the other
 * callers pass the path index alone. */
extern Overlay84Resource *overlay84GetResource();
extern void overlay84ReleaseResource(u8 resource);
/* The in-module calls to overlay84AdvanceCurrent are SYMBOL relocation
 * records whose shipped word holds the 0xF0000000 addend, so they go through
 * this placeholder rather than the definition. */
extern void overlay84AdvanceCurrent_o084Reloc(s32 direction);

/* Exported. */
void overlay84InitState(Overlay84Object *object, void *entry);
void overlay84InitializeAndUpdate(Overlay84Object *object, s32 arg);
void overlay84ResetCurrent(void);
s32 overlay84GetActive(void);
void overlay84LoadCurrent(s32 kind);
void overlay84CopyPair(Overlay84Pair *dst, Overlay84Entry *src);
void overlay84AdvanceCurrent(s32 direction);
void overlay84SelectCurrent(s32 index);
s32 overlay84GetCurrent(void);
void overlay84SetBit(s32 index);
void overlay84GetValues(s32 *first, s32 *second);
void overlay84ActivateCurrent(s32 kind);
void overlay84InitializeCurrent(void);
void overlay84ClearActive(void);
s32 overlay84IsUnitScale(void);
s32 overlay84GetEnabledCurrent(void);
void overlay84ClearMode(void);
void overlay84SetAngle(s16 angle);
void overlay84Mark(void);

#endif
