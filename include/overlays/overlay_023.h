#ifndef OVERLAY_023_H
#define OVERLAY_023_H

#include "PR/ultratypes.h"

typedef struct Gfx {
    u32 w0;
    u32 w1;
} Gfx;

typedef struct Overlay23OwnerPoint {
    f32 x;
    f32 y;
    f32 z;
} Overlay23OwnerPoint;

typedef struct Overlay23Attachment {
    u8 pad00[4];
    s8 pointIndex;
    u8 pad05[3];
    f32 scale;
    u8 pad0C[8];
} Overlay23Attachment;

typedef struct Overlay23Model {
    f32 scale;
    u8 pad04[0x3C];
    u8 *resourceEntries;
} Overlay23Model;

typedef struct Overlay23PointModel {
    u8 pad00[0x3F];
    u8 enabled;
    Overlay23OwnerPoint *points;
} Overlay23PointModel;

typedef struct Overlay23PointModelLink {
    Overlay23PointModel *current;
} Overlay23PointModelLink;

typedef struct Overlay23OwnerObject {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[2];
    f32 scale;
    f32 baseX;
    u8 pad10[4];
    f32 baseZ;
    u8 pad18[4];
    f32 motionX;
    u8 pad20[4];
    f32 motionZ;
    u8 pad28[0x18];
    Overlay23Model *model;
    u8 pad44[0x1C];
    Overlay23Attachment *attachments;
    u8 pad64[4];
    Overlay23PointModelLink *pointModelLink;
    u8 pad6C[0x20];
    u8 attachmentCount;
} Overlay23OwnerObject;

typedef struct Overlay23OwnerAnimation {
    u8 pad00[0x10C];
    s16 angleTable0[4];
    s16 angleTable1[4];
    u8 pad11C[0x3C];
    s16 blocked;
} Overlay23OwnerAnimation;

typedef struct Overlay23SpawnPacket {
    s16 kind;
    u8 pad02[2];
    s16 pointX;
    s16 pointY;
    s16 pointZ;
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u16 resource;
    s16 lifetime;
    s16 animation0;
    s16 animation1;
    f32 scaledSize;
    f32 ownerScale;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 motionX;
    f32 motionZ;
} Overlay23SpawnPacket;

typedef struct Overlay23State {
    f32 velocityX;
    f32 velocityY;
    f32 scale;
    s16 timer;
    s16 halfTimer;
    s16 angle0;
    s16 angle1;
    s16 randomStep;
    u8 pad16[2];
    f32 alpha;
    f32 alphaStep;
    void *asset;
} Overlay23State;

/* overlay23Init's scale and overlay23Update's velocity views of one block. */
typedef struct Overlay23Transform {
    s16 flags;
    u8 pad2[2];
    f32 scale;
    u8 pad8[0x14];
    f32 velocityX;
    f32 velocityY;
} Overlay23Transform;

/* The update stores the fade through a signed byte; the initializer never
 * touches it. */
typedef struct Overlay23Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad6[2];
    f32 scale;
    u8 padC[0x10];
    f32 x;
    f32 y;
    f32 z;
    u8 pad28[0x11];
    s8 alpha;
    u8 pad3A[0x16];
    s32 renderFlags;
    u8 pad54[0x10];
    Overlay23State *state;
    u8 pad68[0x10];
    Overlay23Transform *transform;
} Overlay23Object;

typedef struct Overlay23InitData {
    u8 pad0[0xA];
    s16 angle0;
    s16 angle1;
    s16 angle2;
    s16 assetId;
    s16 timer;
    s16 stateAngle0;
    s16 stateAngle1;
    f32 stateScale;
    f32 objectScale;
    f32 x;
    f32 y;
    f32 z;
    f32 velocityX;
    f32 velocityY;
} Overlay23InitData;

typedef struct Overlay23RenderState {
    u8 pad00[8];
    f32 scale;
    u8 pad0C[4];
    s16 value0;
    s16 value1;
    u8 pad14[0x0C];
    s32 resource;
} Overlay23RenderState;

typedef struct Overlay23RenderObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0x16];
    void *renderResource;
    u8 pad54[0x10];
    Overlay23RenderState *state;
} Overlay23RenderObject;

typedef struct Overlay23RenderPacket {
    s16 value0;
    s16 value1;
    u8 pad04[2];
    s16 mode;
    f32 scale;
    f32 unitScale;
    f32 x;
    f32 y;
    f32 z;
    s32 color;
    s32 resource;
} Overlay23RenderPacket;

extern s32 mathRnd(s32 minimum, s32 maximum);
extern void *func_8000590C(Overlay23SpawnPacket *packet, s32 mode);
void *texLoadSprite(s32 assetId, s32 mode);
extern f32 gOverlay23VelocityFactor;
f32 Powerf(f32 factor, s32 exponent);
void func_80006EA0(Overlay23Object *object, s32 updateRate);

#endif
