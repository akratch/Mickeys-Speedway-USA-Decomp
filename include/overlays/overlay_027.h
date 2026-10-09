#ifndef OVERLAY_027_H
#define OVERLAY_027_H

#include "PR/ultratypes.h"

/* Overlay 27: shared declarations for the original module translation unit. */

typedef struct O27Command {
    u32 w0;
    u32 w1;
} O27Command;

typedef struct O27Transform {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
    f32 scale;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
} O27Transform;

typedef struct O27Work {
    u8 scratchBefore[0xC];
    O27Transform transform;
    u8 scratchAfter[0xC];
} O27Work;

typedef struct O27Child {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x38];
    f32 *factor;
} O27Child;

typedef struct O27Object O27Object;

typedef struct O27State {
    u8 primaryState;
    u8 pulseState;
    s16 timer;
    u8 colorR;
    u8 colorG;
    u8 colorB;
    u8 pulseR;
    u8 pulseG;
    u8 pulseB;
    s16 intensity;
    s16 fade;
    s16 pulseTimer;
    f32 scaleTarget;
    f32 fadeFloat;
    void *primaryHandle;
    void *secondaryHandle;
    O27Object *source;
    u8 reserved24[0x184];
    u16 flags1A8;
} O27State;

typedef struct O27Resource {
    u8 pad00[0xAC];
    void **displayList;
} O27Resource;

struct O27Object {
    u8 reserved00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x16];
    s16 positionTag;
    u8 reserved30[9];
    s8 alpha;
    u8 reserved3A[6];
    O27Resource *renderResource;
    u8 reserved44[0x20];
    O27State *state;
    void **updateResource;
    u8 reserved6C[0x25];
    u8 blocked;
};

typedef struct Overlay27InitData {
    u8 pad0[0xC];
    void *target;
} Overlay27InitData;

typedef struct Overlay27CoordinateRecord {
    u8 pad0;
    u8 firstIndex;
    u8 secondIndex;
    u8 thirdIndex;
    s16 firstX;
    s16 firstY;
    s16 secondX;
    s16 secondY;
    s16 thirdX;
    s16 thirdY;
} Overlay27CoordinateRecord;

typedef struct Overlay27UseResource {
    u8 state;
    u8 pad01[0x13];
    f32 value14;
} Overlay27UseResource;

typedef struct Overlay27UseObject {
    u8 pad00[0x64];
    Overlay27UseResource *resource;
} Overlay27UseObject;

extern s32 gO27Active;
extern f32 gO27Scale0;
extern f32 gO27EaseInput;
extern f32 gO27Scale8;
extern f32 gO27ScaleC;
extern u8 D_80000000[];
extern u8 D_80000050[];
extern u8 D_80000118[];
extern u8 D_80000160[];
extern Overlay27CoordinateRecord gOverlay27CoordinateRecords[];
extern s16 gOverlay27XCoordinates[];
extern s16 gOverlay27YCoordinates[];
extern s32 gOverlay27XOffset;
extern s32 gOverlay27YOffset;

void texAnimateSprite(void *, s32 *, s32, void *, s32);
f32 func_8002A878(f32, s32);
s32 func_800299E8(s32, s32);
void amSndStopXYZ(void *);
void amSndPlayXYZ(s32, f32, f32, f32, s32, void **);
void func_8002BD58(s32, s32, f32);
void amSndSetXYZ(void *, f32, f32, f32);
void amSndSetVolXYZ(void *, u8);
void func_80006EA0(O27Object *);
s16 *overlay27GetValue(void);
f32 overlay27GetChildScale(O27Child *child);
void overlay27Prepare(O27Command **commands, void *arg1,
                      O27Transform *transform, f32 arg3, f32 arg4);
void overlay27DrawPart(O27Command **commands, void *displayList, s32 arg2,
                       s32 arg3);
void overlay27SetMode(O27Command **commands, s32 arg1, s32 arg2, s32 arg3);
void overlay27Finish(O27Command **commands, void *arg1);
void overlay27Finalize(O27Command **commands, void *arg1, s16 *arg2,
                       O27Object *object);

void overlay27Init(O27Object *object, Overlay27InitData *init);
void func_overlay_027_F0000064_187BA3C(O27Object *object, s32 updateRate);
void func_overlay_027_F0000624_187BFFC(O27Command **commands, void *arg1,
                                      s16 *arg2, O27Object *object);
void overlay27UpdateCoordinates(s32 amount);
s32 overlay27CanUse(Overlay27UseObject *object);
s32 overlay27Activate(O27Object *object);

#endif
