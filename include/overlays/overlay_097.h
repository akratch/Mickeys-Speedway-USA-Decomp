#ifndef OVERLAY_097_H
#define OVERLAY_097_H

#include "PR/ultratypes.h"
#include "game/math.h"
/* DKR obj_init_scenery shares only the scale-prefix semantics, not this body. */
typedef struct Overlay97State {
    f32 direction[3];
    f32 offset;
    s32 scaleIndex;
    s16 value18;
    s16 value15;
} Overlay97State;

typedef struct Overlay97Object {
    s16 angleA;
    s16 angleB;
    u8 pad4[4];
    f32 scale;
    f32 position[3];
    u8 pad18[0x4C];
    Overlay97State *state;
    u8 pad68[0x1C];
    f32 radiusSquared;
} Overlay97Object;

typedef struct Overlay97Config {
    u8 pad0[0x12];
    u8 scaleIndex;
    u8 angleA;
    u8 pad14;
    s8 value15;
    u8 pad16[2];
    s8 value18;
    u8 pad19[2];
    u8 angleB;
} Overlay97Config;



/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Angles {
    s16 x, y, z;
} Overlay97Angles;

typedef struct Overlay97AngleInit {
    u8 pad0[0xA];
    s16 z, y, x;
} Overlay97AngleInit;

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Transform {
    s16 angle;
    u8 pad2[0x82];
    s32 mode;
    s32 index;
} Overlay97Transform;

typedef struct Overlay97TransformInit {
    u8 pad0[0xB];
    u8 index;
    u8 angle;
    s8 mode;
} Overlay97TransformInit;

extern s32 gOverlay97TransformContext;

extern void SetLinkSlot(s32 context, s32, s32,
                                         Overlay97Transform *transform);

/* Radius initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay97InitData { u8 pad0[0xA]; u8 radius; } Overlay97InitData;

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

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Selection {
    s16 index;
    u8 pad2[2];
    s16 first;
    s16 second;
    s16 third;
    s16 fourth;
} Overlay97Selection;

typedef struct Overlay97SelectionObject {
    u8 pad0[0x64];
    Overlay97Selection *selection;
} Overlay97SelectionObject;

typedef struct Overlay97SelectionInit {
    u8 pad0[0xA];
    s16 index;
    s8 first;
    s8 second;
} Overlay97SelectionInit;

typedef struct Overlay97SelectionTable {
    u8 pad0[0x18];
    s16 count;
} Overlay97SelectionTable;

extern Overlay97SelectionTable *trackGetTrack(void);

/* DKR v77/v80 source checks found no matching state-allocation wrapper. */


extern void *addRomdefLight(Overlay97Object *object, void *entry);

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


typedef struct Overlay97ModelBounds {
    s16 bounds[6];
} Overlay97ModelBounds;

typedef struct Overlay97ModelValue {
    s16 value;
} Overlay97ModelValue;

typedef struct Overlay97ModelHeader {
    u8 bytes[0x3C];
    Overlay97ModelBounds bounds;
} Overlay97ModelHeader;

typedef struct Overlay97ModelInstance {
    Overlay97ModelHeader *model;
} Overlay97ModelInstance;

typedef struct Overlay97Header {
    f32 scale;
    u8 pad04[0x1E];
    s8 modelCount;
    u8 pad23[0x31];
    f32 shadowScale;
    f32 shadowOffset;
} Overlay97Header;

typedef struct Overlay97Shadow {
    f32 scale;
    f32 offset;
} Overlay97Shadow;

typedef struct Overlay97Interaction {
    u8 pad00[0x132];
    s16 enabled;
} Overlay97Interaction;

typedef struct Overlay97ScaleObject {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    u8 pad0C[0x28];
    f32 radius;
    u8 pad38[2];
    s8 modelIndex;
    u8 pad3B[5];
    Overlay97Header *header;
    u8 pad44[8];
    Overlay97Shadow *shadow;
    u8 pad50[8];
    Overlay97Interaction *interaction;
    u8 pad5C[0xC];
    Overlay97ModelInstance **models;
} Overlay97ScaleObject;

typedef struct Overlay97ScaleEntry {
    u8 pad00[0xA];
    u8 modelIndex;
    u8 radius;
    u8 angle;
} Overlay97ScaleEntry;

/* DKR source/object scans found no corresponding bounds initializer. */
typedef struct Overlay97Bounds {
    f32 first;
    f32 topA;
    f32 bottomA;
    f32 second;
    f32 topB;
    f32 bottomB;
    u32 third;
    f32 width;
    f32 height;
    f32 x;
    f32 y;
    f32 z;
} Overlay97Bounds;

typedef struct Overlay97BoundsObject {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay97Bounds *bounds;
} Overlay97BoundsObject;

typedef struct Overlay97BoundsInit {
    u8 pad0[6];
    s16 center;
    u8 pad8[2];
    u16 first;
    u16 second;
    u16 third;
    s16 width;
    s16 height;
    s16 x;
    s16 y;
    s16 z;
    s16 halfA;
    s16 halfB;
} Overlay97BoundsInit;

extern void func_8001C054(void);

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Plane {
    f32 x, y, z;
    f32 distance;
    f32 radiusSquared;
    f32 height;
    u16 first;
    u16 second;
} Overlay97Plane;

typedef struct Overlay97PlaneObject {
    u8 pad0[0xC];
    f32 x, y, z;
    u8 pad18[0x4C];
    Overlay97Plane *plane;
} Overlay97PlaneObject;

typedef struct Overlay97PlaneInit {
    u8 pad0[0xA];
    u16 angle;
    u16 height;
    u16 radius;
    u16 first;
    u16 second;
} Overlay97PlaneInit;

extern void mathOneFloatRPY(s16 *angles, Overlay97Plane *plane,
                                     Overlay97PlaneObject *object,
                                     Overlay97PlaneInit *init);

#endif
