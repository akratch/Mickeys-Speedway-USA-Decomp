#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding resource initializer. */
typedef struct Overlay97Resource {
    u16 first;
    u16 second;
    u8 colorA;
    u8 colorB;
    u8 mode;
    u8 pad7;
    s32 handle;
    u8 flags;
    u8 extra;
} Overlay97Resource;

typedef struct Overlay97ResourceObject {
    u8 pad0[0x64];
    Overlay97Resource *resource;
} Overlay97ResourceObject;

typedef struct Overlay97ResourceInit {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    u16 first;
    u16 second;
    u8 colorA;
    u8 colorB;
    u8 mode;
    u8 flags;
    u8 extra;
} Overlay97ResourceInit;

extern s32 amSoundIsLooped(u16 first);
extern void amCreateAudioPoint(u16 first, f32 x, f32 y, f32 z,
                                         s32 kind, s32 colorB, s32 colorA,
                                         s32 second, s32 flags, s32 mode,
                                         s32 extra, s32 zero, s32 *handle);
extern void func_80006EA0(Overlay97ResourceObject *object);

void overlay97InitResource(Overlay97ResourceObject *object,
                           Overlay97ResourceInit *init) {
    Overlay97Resource *resource;

    resource = object->resource;
    resource->first = init->first;
    resource->second = init->second;
    resource->flags = init->flags;
    resource->mode = init->mode;
    resource->colorA = init->colorA;
    resource->colorB = init->colorB;
    resource->extra = init->extra;
    resource->handle = 0;
    if (amSoundIsLooped(resource->first)) {
        amCreateAudioPoint(resource->first, (f32)init->x,
                                     (f32)init->y, (f32)init->z, 9,
                                     resource->colorB, resource->colorA,
                                     resource->second, resource->flags,
                                     resource->mode, resource->extra, 0,
                                     &resource->handle);
    } else {
        amCreateAudioPoint(resource->first, (f32)init->x,
                                     (f32)init->y, (f32)init->z, 10,
                                     resource->colorB, resource->colorA,
                                     resource->second, resource->flags,
                                     resource->mode, resource->extra, 0,
                                     &resource->handle);
    }
    func_80006EA0(object);
}
