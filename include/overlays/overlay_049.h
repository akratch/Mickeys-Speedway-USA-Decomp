#ifndef OVERLAY_049_H
#define OVERLAY_049_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay49Status {
    u8 pad00[2];
    u8 player;
    u8 pad03[0x23];
    u16 value;
} Overlay49Status;

typedef struct Overlay49LookupResult {
    u8 pad00[8];
    void *value;
} Overlay49LookupResult;

typedef struct Overlay49Object {
    u8 pad00[0x88];
    s32 mode;
} Overlay49Object;

extern s32 gOverlay49Timer;
extern s32 gOverlay49Finished;
extern s8 gOverlay49Modes[];
extern s32 gOverlay49Masks[];
extern s32 gOverlay49Shifts[];
extern Overlay49Object *gOverlay49Result;
extern s32 gOverlay49InputEnabled;
/* Overlay-runtime proxies for resident menu state used by the initializer. */
extern u8 D_8007BF04;
extern u8 D_8007BF08;
extern u16 D_800D3128[];
extern u8 D_8007BF60;
extern u8 D_8007BF0C;
extern Gfx *D_800D3140;
extern Mtx *D_800D3144;

extern Overlay49Status *func_80028F54(void);
extern void overlay65Initialize(void);
extern Overlay49LookupResult *func_800508B4(s32 id);
extern void func_8002917C(s32 mode);
extern u32 func_800254FC(s32 index);
extern u32 func_8002554C(s32 index);
extern void func_800016EC(s32 mode);
extern void func_8003A754(void);
extern void overlay48Initialize(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                         s32 arg5);
extern void overlay65UpdateParticles(Gfx **arg0, Mtx **arg1, s32 updateRate);
extern void overlay65Release(void);

#endif
