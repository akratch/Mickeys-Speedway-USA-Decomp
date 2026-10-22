#ifndef OVERLAY_024_H
#define OVERLAY_024_H

#include "PR/ultratypes.h"

typedef struct Overlay24TargetState {
    s8 status;
    u8 pad001[0x191];
    u8 adjustment;
    u8 pad193[0x15];
    u16 flags;
} Overlay24TargetState;

typedef struct Overlay24Target {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x16];
    s16 copiedValue;
    u8 pad030[0x34];
    Overlay24TargetState *state;
} Overlay24Target;

typedef struct Overlay24State {
    u8 mode;
    s8 remaining;
    s16 phaseTicks;
    f32 height;
    f32 velocity;
    s32 progress;
    Overlay24Target *target;
} Overlay24State;

typedef struct Overlay24Object {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x10];
    f32 relationValue;
    u8 pad02C[2];
    s16 copiedValue;
    u8 pad030[0x34];
    Overlay24State *state;
    void **relationResource;
} Overlay24Object;

typedef struct Overlay24InitData {
    u8 pad0[0xA];
    s16 remaining;
    Overlay24Target *target;
} Overlay24InitData;

typedef struct Overlay24InputState {
    u8 mode;
} Overlay24InputState;

typedef struct Overlay24Command {
    u32 w0;
    u32 w1;
} Overlay24Command;

typedef struct Overlay24Source {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x38];
    f32 *opacity;
} Overlay24Source;

typedef struct Overlay24RenderState {
    u8 pad00[4];
    f32 yOffset;
    u8 pad08[4];
    s32 enabled;
    Overlay24Source *source;
} Overlay24RenderState;

typedef struct Overlay24RenderObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 opacity;
    u8 pad3A[0x2A];
    Overlay24RenderState *state;
    void **resource;
} Overlay24RenderObject;

extern s16 D_8007BF1C;
extern s32 D_8007C854;
extern s32 D_8007C85C;

void func_80006EA0(Overlay24Object *object);
s32 texAnimateSprite(void *resource, s32 *eventId, s32 limit,
                                 f32 *relationValue, s32 updateRate);
void amSndPlay(s32 eventId, void *handle);
Overlay24InputState *func_80028F54(void);
void camDoSprite(Overlay24Command **commands, void *arg1,
                                void *arg2, Overlay24RenderObject *object,
                                void *resource, s32 mode, s32 intensity);

void overlay24Init(Overlay24Object *object, Overlay24InitData *init);
void overlay24Update(Overlay24Object *object, s32 updateRate);
void overlay24RenderState(Overlay24Command **commands, void *arg1, void *arg2,
                          Overlay24RenderObject *object);

#endif
