#ifndef OVERLAY_027_H
#define OVERLAY_027_H

#include "PR/ultratypes.h"

/* Overlay 27: shared declarations for the original module translation unit. */

typedef struct O27Command {
    u32 w0;
    u32 w1;
} O27Command;

typedef struct Overlay27Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay27Vertex;

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
    u8 flags;
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

extern s32 D_8007A1E4;
extern Overlay27Vertex gOverlay27FadeVertices[];
extern Overlay27Vertex gOverlay27PulseVertices[];
extern Overlay27CoordinateRecord gOverlay27PulseTriangles[];
extern Overlay27CoordinateRecord gOverlay27CoordinateRecords[];
extern s16 gOverlay27XCoordinates[];
extern s16 gOverlay27YCoordinates[];
extern s32 gOverlay27XOffset;
extern s32 gOverlay27YOffset;

void texAnimateSprite(void *, s32 *, s32, void *, s32);
f32 Powerf(f32, s32);
s32 func_800299E8(s32, s32);
void amSndStopXYZ(void *);
void amSndPlayXYZ(s32, f32, f32, f32, s32, void **);
void rumbleStart(s32, s32, f32);
void amSndSetXYZ(void *, f32, f32, f32);
void amSndSetVolXYZ(void *, u8);
void func_80006EA0(O27Object *);
extern s16 *camGetPtr(void);
extern f32 func_80009F08(void *arg);
extern void camPushModelMtx(O27Command **commands, void *mtx,
                            O27Transform *transform, f32 scale, f32 scaleY);
extern void texDPTextureX(O27Command **commands, void *texture, s32 flags,
                          s32 frame);
extern void camPopModelMtx(O27Command **commands);
extern void func_80009E78(O27Command **commands, void *mtx, s16 *vertices,
                          O27Object *object);


void overlay27Init(O27Object *object, Overlay27InitData *init);
void func_overlay_027_F0000064_187BA3C(O27Object *object, s32 updateRate);
void func_overlay_027_F0000624_187BFFC(O27Command **commands, void *arg1,
                                      s16 *arg2, O27Object *object);
void overlay27UpdateCoordinates(s32 amount);
s32 overlay27CanUse(Overlay27UseObject *object);
s32 overlay27Activate(O27Object *object);

#endif
