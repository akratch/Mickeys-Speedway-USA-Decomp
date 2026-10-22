#ifndef OVERLAY_095_H
#define OVERLAY_095_H

#include "PR/ultratypes.h"

/* Overlay 95: pinned DKR v77/v80 object scans are negative for both
 * functions. DKR's HUD audio updater is a semantic relative for the timed
 * handle/volume fade in overlay95Update, not an exact donor. */

typedef struct Overlay95AudioState {
    void *handle;
    s32 volume;
} Overlay95AudioState;

typedef struct Overlay95Object {
    u8 pad00[0x64];
    Overlay95AudioState *audio;
    u8 pad68[0x18];
    s32 flags;
    s32 active;
    s32 timer;
} Overlay95Object;

typedef struct Overlay95Racer {
    s8 type;
    u8 pad001[0x190];
    s8 blocked;
    u8 pad192[0x16];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 mode;
} Overlay95Racer;

typedef struct Overlay95RacerObject {
    u8 pad00[0x64];
    Overlay95Racer *racer;
} Overlay95RacerObject;

typedef struct Overlay95QueryResult {
    Overlay95RacerObject *object;
} Overlay95QueryResult;

extern u8 D_8007BF0C;
extern f32 gOverlay95FadeRate;
extern Overlay95QueryResult *func_80005750(void **found);
extern void partUpdateTriggers(Overlay95Object *object, s32 updateRate);
extern void rumbleStart(s32 type, s32 kind, f32 scale, Overlay95Racer *racer);
extern void *camGetNo(void);
extern void camStartShake(void *resource, f32 x, f32 y, f32 z, s32 count);
extern void amSndStop(void *handle);
extern void amSndPlay(s32 soundId, Overlay95AudioState *audio);
extern void amSndSetVol(s32 soundId, void *handle, s32 volume);

void overlay95NoOp(void *object, void *entry);
void overlay95Update(Overlay95Object *object, s32 updateRate);

#endif
