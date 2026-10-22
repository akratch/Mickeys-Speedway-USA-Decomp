#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Descriptor {
    u8 flags;
    s8 kind;
    s16 y;
    s16 x;
    s16 zero;
    f32 vectorX;
    f32 vectorY;
    f32 vectorZ;
    u8 red, green, blue, alpha;
    s16 target;
    s16 width;
    s16 height;
    s8 parent;
    s8 enabled;
} Overlay97Descriptor;

typedef struct Overlay97DescriptorObject {
    u8 pad0[0x84];
    s32 handle;
} Overlay97DescriptorObject;

typedef struct Overlay97DescriptorInit {
    u8 pad0[0xA];
    s16 x;
    s16 y;
    u8 alternate;
    u8 additive;
    u8 hasTarget;
    u8 pad11;
    u8 red, green, blue, alpha;
} Overlay97DescriptorInit;

extern s32 camlightAdd(Overlay97DescriptorObject *object,
                                          Overlay97Descriptor *descriptor,
                                          Overlay97DescriptorInit *init);

void overlay97CreateDescriptor(Overlay97DescriptorObject *object,
                               Overlay97DescriptorInit *init) {
    Overlay97Descriptor descriptor;

    descriptor.flags = 0x21;
    descriptor.target = -1;
    if (init->alternate != 0) {
        descriptor.flags = 0x23;
    }
    if (init->additive != 0) {
        descriptor.flags |= 4;
    }
    if (init->hasTarget != 0) {
        descriptor.target = 0x2B;
    }
    descriptor.kind = 0x50;
    descriptor.y = init->y;
    descriptor.x = init->x;
    descriptor.zero = 0;
    descriptor.vectorX = 0.0f;
    descriptor.vectorY = 0.0f;
    descriptor.vectorZ = 0.0f;
    descriptor.red = init->red;
    descriptor.green = init->green;
    descriptor.blue = init->blue;
    descriptor.alpha = init->alpha;
    descriptor.width = 100;
    descriptor.height = 100;
    descriptor.parent = -1;
    descriptor.enabled = 1;
    object->handle = camlightAdd(object, &descriptor, init);
}
