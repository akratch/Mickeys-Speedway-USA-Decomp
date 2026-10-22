#ifndef OVERLAY_085_H
#define OVERLAY_085_H

#include "ultra64.h"

/*
 * Overlay 85: shared module header. The Epoch 5 DKR v77/v80 donor search
 * was negative for both functions; JFG has no exact donor either.
 *
 * overlay85Configure.c and overlay85Update.c each carried their own private
 * "Overlay85Trigger" typedef with the same field layout under different
 * field names (active/strength here vs enabled/scale in Update's copy);
 * that duplication collapses into the one definition below. The two files
 * also both spelled a type "Overlay85State" for two genuinely different
 * real structures -- Configure's large per-effect state block (this file's
 * Overlay85State, unchanged) and a small per-object status byte Update
 * reaches through Object::state. The latter is renamed Overlay85ObjectState
 * here to resolve the name collision; its field layout and offsets are
 * unchanged from Update's original private typedef.
 */

typedef struct Overlay85Resource {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad04[0x1E];
    /* 0x22 */ s8 frameCount;
    /* 0x23 */ u8 pad23[0x31];
    /* 0x54 */ f32 outputScaleX;
    /* 0x58 */ f32 outputScaleY;
} Overlay85Resource;

typedef struct Overlay85Output {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ u8 state;
} Overlay85Output;

typedef struct Overlay85Trigger {
    /* 0x00 */ u8 pad00[0x63];
    /* 0x63 */ u8 active;
    /* 0x64 */ f32 strength;
} Overlay85Trigger;

typedef struct Overlay85State {
    /* 0x00 */ s16 angle;
    /* 0x02 */ s16 value2;
    /* 0x04 */ s16 value4;
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x22];
    /* 0x3A */ s8 frame;
    /* 0x3B */ u8 pad3B[5];
    /* 0x40 */ Overlay85Resource *resource;
    /* 0x44 */ u8 pad44[2];
    /* 0x46 */ s16 type;
    /* 0x48 */ Overlay85Trigger *trigger;
    /* 0x4C */ f32 *outputScale;
    /* 0x50 */ Overlay85Output *output;
    /* 0x54 */ u8 pad54[0x2C];
    /* 0x80 */ s32 eventState;
    /* 0x84 */ s16 timer;
    /* 0x86 */ s16 velocity;
    /* 0x88 */ s32 unk88;
} Overlay85State;

typedef struct Overlay85Config {
    /* 0x00 */ u8 pad00[0xA];
    /* 0x0A */ u8 frame;
    /* 0x0B */ u8 scale;
    /* 0x0C */ u8 angle;
} Overlay85Config;

/* Small per-object status flag reached through Overlay85Object::state.
 * Distinct real struct from Overlay85State above; see the file comment. */
typedef struct Overlay85ObjectState {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ u8 active;
} Overlay85ObjectState;

typedef struct Overlay85Timer {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 value;
} Overlay85Timer;

typedef struct Overlay85Object {
    /* 0x00 */ u8 pad00[2];
    /* 0x02 */ s16 value2;
    /* 0x04 */ s16 value4;
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ f32 effectValue;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x2E];
    /* 0x46 */ s16 type;
    /* 0x48 */ Overlay85Trigger *trigger;
    /* 0x4C */ u8 pad4C[4];
    /* 0x50 */ Overlay85ObjectState *state;
    /* 0x54 */ u8 pad54[0x2C];
    /* 0x80 */ s32 mode;
    /* 0x84 */ Overlay85Timer timer;
} Overlay85Object;

extern void amSndPlayXYZ(s32 kind, f32 x, f32 y, f32 z, s32 flags,
                                 s32 arg5);
extern void func_8003EDD4(f32 value);
extern void partAdjustScaling(f32 value);
extern void partUpdateTriggers(Overlay85Object *object, s32 step);

void overlay85Configure(Overlay85State *state, Overlay85Config *config);
void overlay85Update(Overlay85Object *object, s32 step);

#endif
