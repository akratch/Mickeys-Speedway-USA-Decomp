#ifndef OVERLAY_077_H
#define OVERLAY_077_H

#include "PR/ultratypes.h"

/*
 * Overlay 77: shared module header. DKR v77/v80 object initializers
 * supplied the radius/clamp/divide source idiom for overlay77Init; DKR's
 * projectile physics are a useful semantic reference for overlay77Update
 * only, no donor name or source was adopted. The pinned DKR v77/v80 and
 * JFG object ledgers are exact negative for overlay77Update and for the
 * state tail (overlay77EnsureSelection/overlay77RunCallback).
 *
 * overlay77Init.c and overlay77Update.c each carried their own private
 * "Overlay77Object" typedef, padding over whichever fields that file
 * didn't touch. The two views agreed on every field they shared (offset,
 * size, and, for x/y/z, that overlay77Update reinterprets the bits through
 * a union while overlay77Init only ever reads/writes the float); the
 * canonical struct below is their union, so overlay77Init's body now goes
 * through the same Overlay77Coord accessor for x/y/z.
 */

typedef struct Overlay77State {
    s16 kind;
    s16 sequence;
    f32 acceleration;
    f32 scale;
    f32 targetY;
    f32 targetX;
    f32 targetYCopy;
    f32 targetZ;
} Overlay77State;

typedef struct Overlay77Header {
    f32 scale;
} Overlay77Header;

typedef union Overlay77Coord {
    f32 value;
    s32 bits;
} Overlay77Coord;

typedef struct Overlay77Object {
    s16 angle;
    u8 pad2[4];
    s16 flags;
    f32 scale;
    Overlay77Coord x;
    Overlay77Coord y;
    Overlay77Coord z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 field28;
    u8 pad2C[0x14];
    Overlay77Header *header;
    u8 pad44[0x20];
    Overlay77State *state;
    void **path;
} Overlay77Object;

typedef struct Overlay77Init {
    u8 pad0[0xA];
    u8 kind;
    u8 radius;
    s16 fieldC;
    s16 angle;
} Overlay77Init;

/* gOverlay77Handle is defined as plain s32 in overlay_077.c, matching
 * overlay77Init/overlay77Update's original per-file declaration, and
 * redeclared volatile in overlay_077_tail.c, matching the tail's original
 * declaration. The qualifier changes which bytes IDO emits, so it can't be
 * unified in this shared header. See overlay_077.c's file comment. */
extern s32 gOverlay77Selection;
extern void *gOverlay77CallbackArgument;
extern s32 gOverlay77Sequence;

s32 mathRnd(s32 minimum, s32 maximum);
void amSndStopXYZ(void *argument);
f32 func_8002A8C0(s16 angle);
f32 func_8002A8BC(s16 angle);
void texAnimateSprite(void *path, s32 *mode, s16 kind, f32 *field,
                        s32 updateRate);
void amSndPlayXYZ(s32 objectId, s32 x, s32 y, s32 z, s32 mode,
                         void *argument);
void amSndSetXYZ(void *handle, s32 x, s32 y, s32 z);
s32 overlay77MoveReloc(Overlay77Object *object, f32 x, f32 y, f32 z);
void overlay77OrientReloc(Overlay77Object *object, f32 x, f32 y, f32 z);
void amSndStopXYZ(void *handle);

void overlay77Init(Overlay77Object *object, Overlay77Init *init, s32 preserveSequence);
void overlay77Update(Overlay77Object *object, volatile s32 updateRate);
void overlay77EnsureSelection(void);
void overlay77RunCallback(void);

#endif
