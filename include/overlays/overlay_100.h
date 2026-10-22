#ifndef OVERLAY_100_H
#define OVERLAY_100_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay100Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Overlay100Vec3;

typedef struct Overlay100Motion {
    s16 bank;
    u8 nextBank;
    u8 phase;
    s16 count;
    s16 duration;
    s16 remaining;
    u8 colorB0;
    u8 colorB1;
    u8 colorB2;
    u8 colorA0;
    u8 colorA1;
    u8 colorA2;
    Overlay100Vec3 *velocity;
    Overlay100Vec3 *frames[3];
} Overlay100Motion;

typedef struct O100View {
    f32 scaleX;
    u8 pad04[0x10];
    f32 scaleY;
    u8 pad18[0x14];
    f32 depthScale;
} O100View;

extern s32 D_80079278;
extern void *gOverlay100Entries[];
/* The placeholder names below are the identities the shipped table records
 * for the module's blob-owned registry and constants (phase 3 gives them C
 * owners); gOverlay100EntriesReloc is the same registry as gOverlay100Entries. */
extern Overlay100Motion *gOverlay100EntriesReloc[];
extern f32 gOverlay100VelocityScaleReloc[];
extern f32 gOverlay100GravityReloc[];
extern Gfx gOverlay100SegmentReloc[];

extern void *mmAlloc(s32 size, s32 tag);
extern void mmFree(void *entry);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void mathOneFloatPY(s16 *angles, Overlay100Vec3 *velocity);
extern O100View *func_8002468C(void);
extern s16 *camGetPtr(void);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void texDPInit(Gfx **commands);

Overlay100Motion *overlay100InitializeMotion(
    f32 x, f32 y, f32 z, s32 count, s32 colorB0, s32 colorB1, s32 colorB2,
    s32 colorA0, s32 colorA1, s32 colorA2, f32 durationSeconds);
void overlay100ReleaseAll(void);
void overlay100RemoveEntry(void *entry);
void overlay100ApplyValue(s32 value);
Overlay100Motion *overlay100UpdateMotion(Overlay100Motion *motion, s32 step);
void overlay100ApplyToValue(void *displayList);
void overlay100DrawMotion(Gfx **dList, Overlay100Motion *motion);

#endif
