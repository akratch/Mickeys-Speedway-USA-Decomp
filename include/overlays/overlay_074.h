#ifndef OVERLAY_074_H
#define OVERLAY_074_H

#include "PR/ultratypes.h"

/* Overlay 74: a channel trigger. The exact DKR and JFG scans are negative. */

typedef struct Overlay74Transform {
    f32 scale;
} Overlay74Transform;

typedef struct Overlay74State {
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74State;

typedef struct Overlay74Object {
    s16 angle;
    u8 pad02[4];
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x28];
    Overlay74Transform *transform;
    u8 pad44[0x20];
    Overlay74State *state;
} Overlay74Object;

typedef struct Overlay74Init {
    u8 pad0[0xA];
    s16 scale;
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74Init;

/* The object a proximity query returns; only its height and its state's
 * signed strength are read. */
typedef struct Overlay74HitState {
    s8 strength;
} Overlay74HitState;

typedef struct Overlay74HitObject {
    u8 pad00[0x10];
    f32 y;
    u8 pad14[0x50];
    Overlay74HitState *state;
} Overlay74HitObject;

/* Bits 26..23 of the word (bits 10..7 of its leading halfword) are a 4-bit
 * channel mask. The update reads it through the 32-bit word and writes it
 * through a halfword, which is what IDO emits for a u16 bitfield. */
typedef struct Overlay74Flags {
    u16 pad : 5;
    u16 channelMask : 4;
    u16 rest : 7;
} Overlay74Flags;

extern Overlay74Flags D_800D3128;
extern f32 gOverlay74Scale;

/* Runtime identities: func_8005776C, func_800291B4, amSndPlay, func_8003A680.
 * Resident ROM and matched callers authenticate the floating-point z ABI. */
u8 *func_80028F54(void);
s32 func_8005776C(f32 x, f32 y, f32 z, f32 strength, s32 enabled,
                  Overlay74HitObject **results);
void func_800291B4(void);
void amSndPlay(u16 soundId, void **handle);
void func_8003A680(s32 count);

void overlay74Init(Overlay74Object *object, Overlay74Init *init);
void overlay74Update(Overlay74Object *object, s32 amount);

#endif
