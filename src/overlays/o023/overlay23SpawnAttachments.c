#include "PR/ultratypes.h"

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

extern s32 mathRnd(s32 minimum, s32 maximum);
extern void *func_8000590C(Overlay23SpawnPacket *packet, s32 mode);

void overlay23SpawnAttachments(Overlay23OwnerObject *object,
                               Overlay23OwnerAnimation *animation) {
    Overlay23Attachment *attachment;
    Overlay23OwnerPoint *point;
    u8 *resourceEntry;
    Overlay23PointModel *pointModel;
    void *spawned;
    Overlay23SpawnPacket packet;
    s32 index;

    pointModel = object->pointModelLink->current;
    if (animation->blocked != 0 || pointModel->enabled == 0) {
        return;
    }

    attachment = object->attachments;
    resourceEntry = object->model->resourceEntries;
    packet.kind = 0xAA;
    packet.lifetime = 0x78;

    for (index = 0; index < object->attachmentCount && index < 4; index++) {
        point = &pointModel->points[attachment->pointIndex];
        packet.pointX = point->x;
        packet.pointY = point->y;
        packet.pointZ = point->z;
        packet.angle0 = object->angle0;
        packet.angle1 = object->angle1;
        packet.angle2 = object->angle2;
        packet.ownerScale = object->scale;
        packet.scaledSize = attachment->scale * (1.0f / object->model->scale);
        packet.animation0 = animation->angleTable0[index];
        packet.animation1 = animation->angleTable1[index];
        packet.velocityX = (point->x - object->baseX) / 20.0f;
        packet.velocityY = (f32)mathRnd(4, 8);
        packet.velocityZ = (point->z - object->baseZ) / 20.0f;
        packet.motionX = object->motionX;
        packet.motionZ = object->motionZ;
        packet.resource = *(u16 *)resourceEntry;

        spawned = func_8000590C(&packet, 1);
        if (spawned != NULL) {
            *(s32 *)((u8 *)spawned + 0x3C) = 0;
        }

        attachment++;
        resourceEntry += 0xC;
    }
}
