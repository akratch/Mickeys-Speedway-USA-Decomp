#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have related object attachment code, but no exact donor. */

typedef struct Overlay68SourceMeta {
    u8 pad0;
    s8 kind;
} Overlay68SourceMeta;

typedef struct Overlay68SourceObject {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[6];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x4C];
    Overlay68SourceMeta *meta;
} Overlay68SourceObject;

typedef struct Overlay68AttachmentPayload {
    s16 x;
    s16 y;
    s16 z;
    s8 red;
    s8 green;
    s8 blue;
    u8 active;
} Overlay68AttachmentPayload;

typedef struct Overlay68Attachment {
    Overlay68SourceObject *object;
    s8 kind;
    u8 field5;
    s16 generation;
    s16 timer;
    s16 phase;
    Overlay68AttachmentPayload *payload;
} Overlay68Attachment;

extern Overlay68Attachment *gOverlay68Entry;
extern s16 levelGetNumber(Overlay68SourceObject *object, Overlay68Attachment *attachment);

void overlay68AttachObject(Overlay68SourceObject *object) {
    Overlay68Attachment *attachment;
    Overlay68Attachment * volatile savedAttachment;
    Overlay68AttachmentPayload *payload;
    s16 generation;

    attachment = gOverlay68Entry;
    if (attachment != 0) {
        attachment->object = object;
        attachment->kind = object->meta->kind;
        attachment->field5 = 0;
        savedAttachment = attachment;
        generation = levelGetNumber(object, attachment);
        attachment = savedAttachment;
        attachment->generation = generation;
        attachment->timer = 0;
        attachment->phase = 0;
        payload = attachment->payload;
        payload->red = object->x >> 8;
        payload->green = object->y >> 8;
        payload->blue = object->z >> 8;
        payload->x = object->worldX;
        payload->y = object->worldY;
        payload->z = object->worldZ;
        payload->active = 0;
    }
}
