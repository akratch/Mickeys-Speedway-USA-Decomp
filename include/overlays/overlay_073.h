#ifndef OVERLAY_073_H
#define OVERLAY_073_H

#include "PR/ultratypes.h"
#include "game/math.h"

/* Overlay 73: a twelve-vertex quad effect that tracks a nearby target.
 * DKR v77/v80 and JFG contain no exact donor for its functions. */

typedef struct Overlay73Command {
    u32 w0;
    u32 w1;
} Overlay73Command;

typedef struct Overlay73Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay73Vertex;

typedef struct Overlay73State {
    Overlay73Vertex vertices[12];
    void *resource;
    u8 vertexBank;
    u8 mode;
    u16 flags;
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 timer;
    s16 angle;
    u16 countdown;
    void *target;
} Overlay73State;

typedef struct Overlay73Config {
    u8 pad00[0x22];
    s8 resourceCount;
} Overlay73Config;

typedef struct Overlay73Object {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 timer;
    u8 pad2C[0x0D];
    u8 alpha;
    u8 pad3A[6];
    Overlay73Config *config;
    u8 pad44[0x08];
    f32 *output;
    u8 pad50[0x14];
    Overlay73State *state;
    void **resources;
} Overlay73Object;

typedef struct Overlay73Header {
    u8 pad00[4];
    s16 x;
    s16 y;
    s16 z;
    u16 scale;
    u8 resourceIndex;
    u8 outputScale;
} Overlay73Header;

typedef struct Overlay73TargetData {
    u8 pad00[0x18];
    f32 x;
    f32 y;
    f32 z;
} Overlay73TargetData;

typedef struct Overlay73Target {
    u8 pad00[0x48];
    Overlay73TargetData *data;
} Overlay73Target;

struct HitCopyState;

extern s16 D_80[12][3];
extern u8 D_80000000[];

extern s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode,
                         struct HitCopyState **hits);
extern f32 sqrtf(f32 value);
extern void func_80008118(void);
extern void func_80008128(void *object, f32 x, f32 y, f32 z);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void camPushModelMtx(Overlay73Command **commands, void *context,
                            Overlay73Object *object, f32 scale, f32 extra);
extern void texDPTextureX(Overlay73Command **commands, void *resource,
                          s32 mode, s32 flags);
extern void camPopModelMtx(Overlay73Command **commands);

void func_overlay_073_F0000000_18CAAC0(Overlay73Object *object,
                                       Overlay73Header *header,
                                       s32 preserveState);
void func_overlay_073_F0000190_18CAC50(Overlay73Object *object,
                                       s32 updateRate);
void func_overlay_073_F0000D70_18CB830(Overlay73Command **commands,
                                       void *context,
                                       Overlay73Object *object);

#endif
