#ifndef OVERLAY_048_H
#define OVERLAY_048_H

#include "ultra64.h"
#include "overlays/overlay_045.h"

/*
 * Overlay 48: one translation unit (ADR 0006), a scripted title sequence:
 * five caption entries fading in and out under a short control script.
 * DKR v77/v80 and JFG have no exact donor for any function here; the release
 * loop has generic fixed-handle cleanup relatives in DKR.
 */

typedef struct Overlay48Entry {
    Overlay45ResourceDescriptor *handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    s8 active;
    u8 pad0B;
    s16 opacity;
    u8 pad0E[2];
} Overlay48Entry;

/* Five live descriptor records precede the timer, script pointer and two
 * halfword controls. Compiler end alignment completes the BSS extent. */
extern Overlay48Entry gOverlay48Entries[5];

extern u8 D_8007BF60;
extern s16 o48InitialEntrySeeds[5];
extern s16 gOverlay48CreditCommands[128];
extern s16 gOverlay48Timer;
extern s16 *gOverlay48Script;
extern s16 gOverlay48ResourceIndex;
extern s16 gOverlay48Finished;
extern const char *gOverlay48ResourceTable[93];

extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_80028DE4(s32, s32, s32, s32, s32, s32);
extern void mainChangeCameras(s32 mode);
extern void amTunePlay(u8 sequenceId);
extern void func_800015F8(void);
extern void fontUseFont(s32 mode);
extern void mainTitlePageInit(s32 mode);
extern void func_80001608(void);
extern void amTuneSetFade(f32 value, u8 volume);

void overlay48Initialize(void);
void overlay48InitializeState(void);
void overlay48UpdateState(s32 updateRate);
void overlay48ReleaseAll(void);

#endif
