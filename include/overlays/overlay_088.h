#ifndef OVERLAY_088_H
#define OVERLAY_088_H

#include "PR/ultratypes.h"

/* Overlay 88: an anchored geometry object. Exact DKR and JFG scans are
 * negative for its initializer and anchor update; its renderer is overlay
 * 69's, byte for byte (see overlay_088.c). */

/* The object's state block: the initializer clears its counters and flags,
 * the anchor update writes the anchor position and heading. */
typedef struct Overlay88State {
    s16 field0, field2, field4, field6;
    s16 field8, fieldA, fieldC, fieldE;
    s32 field10, field14, field18, field1C;
    u8 pad20[0x14];
    u8 byte34, byte35, byte36, byte37;
    u8 pad38[0x0C];
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
} Overlay88State;

typedef struct Overlay88Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay88Vector;

typedef struct Overlay88Transform {
    u8 pad00[0x30];
    f32 x;
    u8 pad34[4];
    f32 z;
} Overlay88Transform;

typedef struct Overlay88Node {
    void *descriptor;
    u8 pad04[4];
    s16 selector;
    s16 transformIndex;
    Overlay88Transform *transforms[1];
} Overlay88Node;

typedef struct Overlay88Descriptor {
    u8 pad00[0x11];
    u8 needsRefresh;
} Overlay88Descriptor;

typedef struct Overlay88Object {
    u8 pad00[0x10];
    f32 y;
    u8 pad14[0x3C];
    void *context;
    u8 pad54[0x10];
    Overlay88State *state;
    Overlay88Node **nodes;
    u8 pad6C[0x27];
    u8 nodeIndex;
} Overlay88Object;

extern void func_8005AF14(Overlay88Node *node, void *descriptor,
                          Overlay88Object *object);
extern void lightObject(Overlay88Object *object, Overlay88Node *node,
                        void *context, Overlay88Transform *transform);
extern void func_80007C68(Overlay88Object *object,
                          Overlay88Descriptor *descriptor,
                          Overlay88Node *sameNode, s16 selector);
extern void func_8002B040(Overlay88Transform *transform, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
extern s16 Arctanf(f32 x, f32 z);
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s32 angle);

void overlay88Init(Overlay88Object *object, void *entry);
void overlay88UpdateAnchor(Overlay88Object *object, s32 updateContext);

#endif
