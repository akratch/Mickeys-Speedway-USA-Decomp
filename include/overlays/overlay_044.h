#ifndef OVERLAY_044_H
#define OVERLAY_044_H

#include "PR/ultratypes.h"
#include "game/pi.h"

/*
 * Overlay 44: one translation unit (ADR 0006). Three cached animation
 * states streamed from ROM into four frame handles each, and the renderer
 * that draws two 16-bit frames as texture rectangles. The release helper's
 * private view of the state is the same block: its first byte is the
 * source index, -1 when the slot is free.
 */

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    s32 dataOffset;
    s32 frameSize;
} Overlay44FrameSource;

typedef struct Overlay44AnimationState {
    s8 sourceIndex;
    u8 storageMode;
    u8 flags;
    u8 subtype;
    s32 phase;
    s16 value8;
    s16 valueA;
    u8 pad0C[2];
    s8 protectedSlot0;
    s8 protectedSlot1;
    s8 cachedFrame[4];
    u8 *handles[4];
} Overlay44AnimationState;

extern Overlay44AnimationState D_800D76D8[3];

extern Overlay44FrameSource *D_800D76D0;

extern void *mmAlloc(s32 size, s32 tag);

extern void overlay44ReleaseHandles(Overlay44AnimationState *state);

extern void viFrameRateReset(void);


void mmFree(void *handle);


/* PROVENANCE: shared graphics types/macros come from the public JFG-derived
 * n_audio/mbi.h already adopted by this project; Mickey proves these packets. */
#include "n_audio/mbi.h"

extern const Gfx gOverlay44UnscaledState[5];
extern const Gfx gOverlay44ScaledState[5];
extern void texDPInit(Gfx **dl);

#endif
