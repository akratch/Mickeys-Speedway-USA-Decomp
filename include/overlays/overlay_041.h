#ifndef OVERLAY_041_H
#define OVERLAY_041_H

#include "PR/ultratypes.h"

/*
 * Overlay 41: one translation unit (ADR 0006). Scripted-object helpers: step
 * and colour records, a spline-driven object path, a transition queue, a
 * spawn burst and a captioned item. The per-function files' private views
 * collapse into the structures below. The path updater hands its object to
 * the progress and entry helpers, so Overlay41Object is also their context;
 * the block at object +0x40 is one header whether it is read for its scales,
 * its flags or its selector-indexed status bytes. Pinned exact-overlay scans
 * found no exact DKR/JFG donor; the spline sampler's provenance is noted at
 * its definition.
 */

typedef struct Overlay41StepRecord {
    u8 id;
    u8 residual;
    s16 remaining;
    s32 x;
    s32 dx;
    s32 y;
    s32 dy;
} Overlay41StepRecord;

/* Twelve colour fades, each easing a light from its current to its target
 * colour over `duration` ticks. */
typedef struct Overlay41ColorRecord {
    u32 handle;
    u8 targetRed;
    u8 red;
    u8 targetGreen;
    u8 green;
    u8 targetBlue;
    u8 blue;
    u8 targetAlpha;
    u8 alpha;
    s16 remaining;
    s16 duration;
} Overlay41ColorRecord;

typedef struct Overlay41CurveNode {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 reserved06;
    u8 linear;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    s16 tangentX;
    s16 tangentY;
    s16 tangentZ;
    u8 reserved1E[2];
    f32 speed;
    struct Overlay41CurveNode *previous;
    struct Overlay41CurveNode *next;
} Overlay41CurveNode;

/* The object's header block (object +0x40). `status` is read at the
 * selector's byte offset when the scaled selector is active. */
typedef struct Overlay41Header {
    f32 scale;
    u8 reserved04[0x10];
    u16 flags14;
    u8 reserved16[8];
    s8 status[0x36];
    f32 scaleX;
    f32 scaleY;
    u8 reserved5C[0x78];
    f32 scaleD4;
} Overlay41Header;

typedef struct Overlay41Child {
    u8 pad00[0x11];
    u8 switchEnabled;
    u8 pad12[0x3C];
    s8 refreshEnabled;
} Overlay41Child;

typedef struct Overlay41Entry {
    Overlay41Child *child;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *values[1];
} Overlay41Entry;

typedef struct Overlay41Object {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    s16 flags06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 work28[6];
    s16 positionTag;
    u8 reserved30[9];
    u8 value39;
    s8 entryIndex;
    u8 reserved3B[5];
    Overlay41Header *header;
    u8 reserved44[8];
    f32 *scalePair;
    void *argument;
    u8 reserved54[0x14];
    Overlay41Entry **entries;
    u8 reserved6C[0x27];
    u8 selector93;
} Overlay41Object;

typedef struct Overlay41Input {
    u8 reserved00;
    u8 targetIndex;
    u8 reserved02[6];
    Overlay41Object *object;
    f32 amount0C;
    f32 smoothing;
    u8 mode14;
    u8 orientationMode;
    u8 flags;
    u8 reserved17;
    Overlay41CurveNode *node;
    f32 t;
    u8 reserved20[4];
    u8 start24;
    u8 end25;
    u8 limit26;
    u8 current27;
    u8 reserved28[2];
    s16 scaleTimer;
    f32 scaleAmount;
    f32 scaleRate;
} Overlay41Input;

/* An entry of the resident object table D_800D6B00. */
typedef struct Overlay41Entity {
    u8 reserved00[8];
    void *object;
    u8 reserved0C[0x10];
    f32 scale;
} Overlay41Entity;

typedef struct Overlay41Item {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    s32 kind;
    u8 pad18[0x38];
    f32 *opacity;
} Overlay41Item;

typedef struct Overlay41ShortPair {
    s16 x;
    s16 y;
    s16 z;
} Overlay41ShortPair;

typedef struct Overlay41FloatVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay41FloatVector;

/* Five queued transitions: a countdown, a value, a screen position and a
 * colour whose intensity fades in while the countdown runs. */
typedef struct Overlay41Transition {
    s16 remaining;
    s16 value;
    s16 x;
    s16 y;
    s8 red;
    s8 green;
    s8 blue;
    u8 intensity;
} Overlay41Transition;

typedef struct Overlay41DrawItem {
    u16 unused0;
    u16 resourceIndex;
    u16 x;
    u16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
} Overlay41DrawItem;

extern Overlay41StepRecord D_800D6B58[8];
extern Overlay41ColorRecord D_800D6C58[12];
extern Overlay41Transition D_800D6D18[5];
extern s32 D_8007D6A4;
extern Overlay41Entity **D_800D6B00;
extern Overlay41Object *D_800D6B08;
extern void **D_8007C0B8;

#define gOverlay41StepRecords D_800D6B58
#define overlay41EmitStep trackAddTextureScroll

extern void trackAddTextureScroll(s32 id, s32 x, s32 y);
extern void changeLightColour(s32 handle, s32 red, s32 green, s32 blue);
extern void changeLightIntensity(s32 handle, s32 alpha);
extern void func_8003EDEC(Overlay41Object *, s32);
extern s32 func_8000FAE0(f32, f32, f32);
extern f32 sqrtf(f32);
extern s32 func_8002A910(f32, f32);
extern f32 func_8002A878(f32, s32);
extern void func_8005ABA8(Overlay41Object *state, f32 amount, f32 step);
extern void texAnimateSprite(Overlay41Entry *, s32 *, s32, void *, s32);
extern void func_8005AF14(Overlay41Entry *, Overlay41Child *,
                          Overlay41Object *, Overlay41Input *);
extern void lightObject(Overlay41Object *, Overlay41Entry *, void *, void *);
extern void func_80007C68(Overlay41Object *, Overlay41Child *,
                          Overlay41Entry *, s32);
extern s32 mathRnd(s32 lower, s32 upper);
extern void mathOneFloatPY(Overlay41ShortPair *input,
                           Overlay41FloatVector *output);
extern void func_overlay_012_F00001B4_186D434(
    f32 x, f32 y, s32 kind, f32 outX, f32 outY, f32 z, s32 opacity, s32 mode,
    f32 scale);
extern void fontUseFont(s32 mode);
extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);
extern s32 fontStringWidth(char *text, s32 font, s32 convertString);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern void func_overlay_067_F0000000_18C6F40(void *displayList, s32 arg1,
                                              s32 x, s32 y, s32 width,
                                              s32 height, s32 intensity);
extern void fontPrintXY(void *displayList, s32 x, s32 y, void *resource,
                        s32 arg4);

void func_overlay_041_F0000000_1887338(s32 amount);
void func_overlay_041_F0000124_188745C(s32 amount);
void func_overlay_041_F00002AC_18875E4(Overlay41CurveNode *node, f32 t,
                                       f32 *valueOut, f32 *tangentOut,
                                       s32 component);
void func_overlay_041_F0000854_1887B8C(Overlay41Input *input, f32 amount,
                                       s32 updateRate, s32 argument);
s32 overlay41IsUnitScale(s32 index);
void func_overlay_041_F0001650_1888988(void *object, volatile s32 value1,
                                       s32 value3, s32 value5, s32 value7,
                                       f32 amount, s32 alternateColors);
void overlay41Ignore(void *object, s32 value1, s32 value2, s32 value3,
                     s32 value4, s32 value5, s32 value6);
void overlay41SpawnItems(s32 rootIndex, s32 count, s32 mode, s32 centerX,
                         s32 centerY, s32 radiusX, s32 radiusY, s32 centerZ,
                         s32 radiusZ);
void func_overlay_041_F000195C_1888C94(s32 value2, s32 timer, s32 value4,
                                       s32 value6, s32 value8, s32 value9,
                                       s32 valueA);
void overlay41TickTransitions(s32 step);
void overlay41DrawItem(void *displayList, Overlay41DrawItem *item);

/* The sampler is exported through the module's runtime table, so the path
 * updater's calls to it are SYMBOL records that store zero: they name the
 * module-local placeholder, not the definition, until the runlink packer
 * writes the tables (docs/OVERLAY_STRUCTURE_SPRINT.md, phase 4). */
extern void func_overlay_041_F00002AC_18875E4_o041Reloc(
    Overlay41CurveNode *node, f32 t, f32 *valueOut, f32 *tangentOut,
    s32 component);

#endif
