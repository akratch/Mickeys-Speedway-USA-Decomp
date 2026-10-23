#ifndef OVERLAY_091_H
#define OVERLAY_091_H

#include "PR/ultratypes.h"

typedef struct Overlay91InitState {
    s32 timer;
    s32 mode;
} Overlay91InitState;

typedef struct Overlay91InitObject {
    s16 value0;
    u8 pad2[4];
    s16 flags6;
    u8 pad8[4];
    f32 minValue;
    f32 currentValue;
    f32 maxValue;
    u8 pad18[0x16];
    s16 index2E;
    u8 pad30[0x34];
    Overlay91InitState *state;
} Overlay91InitObject;

typedef struct Overlay91TimerState {
    s32 mode;
    s32 timer;
} Overlay91TimerState;

typedef struct Overlay91TimelineObject {
    s16 value0;
    u8 pad02[4];
    s16 flags6;
    u8 pad08[4];
    f32 minValue;
    f32 currentValue;
    f32 maxValue;
    u8 pad18[0x4C];
    Overlay91TimerState *state;
    void *graphOwner;
} Overlay91TimelineObject;

typedef struct Overlay91GraphHeader {
    u8 pad00[0x2C];
    u8 count;
} Overlay91GraphHeader;

typedef struct Overlay91GraphOwner {
    Overlay91GraphHeader *header;
    u8 pad04[0x48];
    void *records;
} Overlay91GraphOwner;

typedef struct Overlay91GraphRoot {
    Overlay91GraphOwner *owner;
} Overlay91GraphRoot;

typedef struct Overlay91GraphRecord {
    s16 value;
    u8 pad02[2];
    u32 flags;
} Overlay91GraphRecord;

typedef struct Overlay91Gfx {
    u32 w0;
    u32 w1;
} Overlay91Gfx;

typedef struct Overlay91RenderObject {
    u8 pad00[6];
    s16 flags;
    u8 pad08[0x31];
    u8 alpha;
} Overlay91RenderObject;

/* Tier B: the declared ORT identities and resident/cross-overlay owners. */
extern volatile s32 D_800C9480;
extern s32 D_800C947C;

void amSndPlay(u16 effectId, void **handle);
void func_80006EA0(void *object);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
void overlay7CommitSelection(s32 selection);

s32 func_800290A0(void);
void viGetCurrentSize(void **bufferOut, u32 *widthOut);
void rcpClearZBuffer(Overlay91Gfx **displayList, void *buffer,
                            u32 width, void *optional, s32 left,
                            void *bufferAgain, s32 right);
void camStandardPersp(Overlay91Gfx **displayList,
                               void *renderContext);
void camResetView(Overlay91Gfx **displayList);
void func_80009E78(Overlay91Gfx **displayList,
                                void *renderContext, void *vertices,
                                Overlay91RenderObject *object);

void overlay91Init(Overlay91InitObject *object, void *entry);
void overlay91UpdateTimeline(Overlay91TimelineObject *object, s32 elapsed);
void overlay91Render(Overlay91Gfx **displayList, void *renderContext,
                     void *vertices, Overlay91RenderObject *object);

#endif
