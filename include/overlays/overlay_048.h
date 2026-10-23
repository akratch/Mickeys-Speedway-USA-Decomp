#ifndef OVERLAY_048_H
#define OVERLAY_048_H

#include "ultra64.h"

/*
 * Overlay 48: one translation unit (ADR 0006), a scripted title sequence:
 * five caption entries fading in and out under a short control script.
 * DKR v77/v80 and JFG have no exact donor for any function here; the release
 * loop has generic fixed-handle cleanup relatives in DKR.
 */

typedef struct Overlay48Entry {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    s8 active;
    u8 pad0B;
    s16 opacity;
    u8 pad0E[2];
} Overlay48Entry;

typedef struct Overlay48InitialSeeds {
    s16 values[5];
    u8 pad0A[0x26A];
    s16 initial;
} Overlay48InitialSeeds;

/* Overlay 48's BSS is one six-record owner: a header at +0x0 followed by
 * five runtime entries at +0x10..+0x5F. Historical D_* aliases and the
 * indexed timer/script tail resolve within this block at load time. */
extern Overlay48Entry gOverlay48Entries[6];

extern u8 gOverlay48State;
extern Overlay48InitialSeeds D_274;
extern s16 D_174[];
extern s16 gOverlay48Timer;
extern s16 *gOverlay48Script;
extern s16 gOverlay48ScriptIndex;
extern s16 gOverlay48ResourceIndex;
extern s16 gOverlay48Finished;
extern s32 gOverlay48ResourceTable[];
extern f32 gOverlay48TransitionValue;

extern void overlay48ConfigureReloc();
extern void mainChangeCameras(s32 mode);
extern void amTunePlay(u8 sequenceId);
extern void func_800015F8(void);
extern void fontUseFont(s32 mode);
extern void overlay45ReleaseDescriptor(s32 handle);
extern void overlay45SetMode(s32 handle, s16 value);
extern s32 overlay45CreateDescriptor(s32 resource, s32 value, s16 seed, s32 kind);
extern void mainTitlePageInit(s32 mode);
extern void func_80001608(void);
extern void amTuneSetFade(f32 value, s32 mode);

void overlay48Initialize(void);
void overlay48InitializeState(void);
void overlay48UpdateState(s32 updateRate);
void overlay48ReleaseAll(void);

#endif
