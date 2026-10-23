#ifndef OVERLAY_070_H
#define OVERLAY_070_H

#include "PR/ultratypes.h"

/* Overlay 70. */

typedef struct O70Object O70Object;

/* An effect object's state block, as all three functions see it. */
typedef struct O70State {
    u8 active;
    u8 type;
    u8 timer;
    s8 countdown;
    s16 angle;
    s16 angleStep;
    s16 verticalStep;
    u8 pad0A[2];
    s32 flags;
    f32 threshold;
    O70Object *related;
    u8 pad18[0x150];
    u16 timer168;
    u8 pad16A[0x3E];
    u16 flags1A8;
    u8 pad1AA[0x20E];
    s16 mode3B8;
    u8 pad3BA[0x40];
    s16 registered3FA;
    u8 pad3FC[4];
    s32 metric400;
} O70State;

/* The same block read through its first byte as a signed manager index. */
typedef struct O70ManagerState {
    s8 index;
    u8 type;
    u8 pad02[0xE];
    f32 threshold;
    O70Object *related;
    u8 pad18[0x150];
    u16 timer168;
    u8 pad16A[0x3E];
    u16 flags1A8;
    u8 pad1AA[0x20E];
    s16 mode3B8;
    u8 pad3BA[0x40];
    s16 registered3FA;
    u8 pad3FC[4];
    s32 metric400;
} O70ManagerState;

typedef struct O70Control {
    u8 pad00[6];
    u16 flags;
} O70Control;

struct O70Object {
    s16 angle;
    u8 pad02[2];
    s16 phase;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 value28;
    u8 pad2C[2];
    s16 facing;
    u8 pad30[0x14];
    s16 kind;
    u8 pad46[2];
    O70Control *control;
    u8 pad4C[0x18];
    O70State *state;
    void **targetList;
};

typedef struct O70Input {
    u8 pad00[0xA];
    u8 type;
    u8 pad0B;
    void *related;
} O70Input;

typedef struct O70FloatTable {
    u8 pad00[0x20];
    f32 values[1];
} O70FloatTable;

typedef struct O70VerticalStepTable {
    u8 pad00[0xC];
    s8 values[1];
} O70VerticalStepTable;

typedef struct O70AngleTable {
    u8 pad00[0x10];
    s16 values[1];
} O70AngleTable;

typedef struct O70RelatedState {
    u8 pad00[0x154];
    f32 verticalOffset;
} O70RelatedState;

typedef struct O70Pair {
    s16 x;
    s16 z;
} O70Pair;

typedef struct O70Record {
    u8 pad00[8];
    s32 metric;
    u8 pad0C[0x1C];
} O70Record;

/* Module data. The image is still a raw blob (phase 3), and the shipped
 * tables carry no relocation for most of these sites (mk/overlay_070.mk). */
extern O70FloatTable gOverlay70FloatTableReloc;
extern O70VerticalStepTable gOverlay70VerticalStepReloc;
extern O70AngleTable gOverlay70AngleReloc;
extern s16 gOverlay70PairTableReloc[];
extern s16 gOverlay70HeightTableReloc[];
extern s32 gOverlay70SharedCounterReloc;

/* Resident functions. */
extern s32 mathRnd(s32 lower, s32 upper);
extern void func_80006EA0(O70Object *object);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void texAnimateSprite(void *target, s32 *flags, s32 mode,
                             f32 *coordinates, s32 ticks);
extern O70Object **func_8000572C(s32 *start, s32 *end);
extern void *func_80028F54(void);
extern O70Object **func_80005750(s32 *count);
extern void func_8003A55C(s32 value);

/* This module's exports (ORT rows). */
void func_overlay_070_F0000000_18C91C8(O70Object *object, O70Input *input);
void func_overlay_070_F00000D8_18C92A0(O70Object *object, s32 ticks);
void func_overlay_070_F0000384_18C954C(O70Object *object);

#endif
