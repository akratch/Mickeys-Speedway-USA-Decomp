#ifndef OVERLAY_062_H
#define OVERLAY_062_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/* Overlay 62. */

typedef struct Overlay62Root {
    u8 pad00[0x25C];
    void *primary;
    void *items[6];
} Overlay62Root;

typedef struct Overlay62Gfx {
    u32 word0;
    u32 word1;
} Overlay62Gfx;

typedef struct Overlay62Transform {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[2];
    f32 scale;
    f32 screenX;
    f32 screenY;
    f32 zero0;
    u8 pad18[0x10];
    f32 zero1;
} Overlay62Transform;

/* Real module-lifetime BSS cells; source definitions are in the main TU.
 * Existing initialized-data/resident aliases below remain separate debt. */
extern f32 o62AnimationFrame_o062Reloc;
extern s32 o62AnimationFlags_o062Reloc;
extern s32 o62FadeInTicks_o062Reloc;
extern s32 o62FadeOutTicks_o062Reloc;
extern s32 o62TransitionState_o062Reloc;
extern void *o62Descriptor_o062Reloc;
extern void *o62Sprite_o062Reloc;

/* Remaining legacy initialized-data/resident views. */
extern f32 gOverlay62Float0;
extern s32 gOverlay62Value4Initial;
extern void *gOverlay62Handle18;

/* Resident data and functions. */
extern Overlay62Root *D_8007C0B8;
extern void loadFrontEndList(void *);
extern void fontUseFont(s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern void *overlay45CreateDescriptor(void *, s32, s32, s32);
extern void *texLoadSprite(s32, s32);
extern void overlay45SetMode(void *, s32);
extern s32 D_800D3180;
extern u8 gOverlay62State[];
extern s32 gOverlay62Values[];
extern Overlay62Gfx *D_800D3140;
extern Mtx *D_800D3144;
extern u8 D_800D3148[];
extern u8 gOverlay62FirstItemReloc[];
extern void rcpSetScreenColour(s32 red, s32 green, s32 blue, s32 alpha);
extern void mainChangeLevel(s32 type, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5);
extern void camStandardOrtho(Overlay62Gfx **commands,
                                         Mtx **matrices);
extern void sprSetTextureFilter(s32 layer);
extern void camDo2DSprite(Overlay62Gfx **commands, Mtx **matrices,
                                     void *resourceB,
                                     Overlay62Transform *transform,
                                     void *handle, s32 arg5, s32 alpha);
extern void fontBackground(s32 a, s32 b, s32 c, s32 d);
extern void fontPrintXY(Overlay62Gfx **commands, s32 x, s32 y,
                                   void *item, s32 mode);
extern void texAnimateTexSprite(s32 handle, s32 *state, s32 mode,
                                      void *context, s32 updateRate);
extern void func_8002F618(Overlay62Gfx **commands, s32 *state,
                                    s32 x, s32 y, s32 red, s32 green,
                                    s32 blue, s32 alpha);
extern u8 gOverlay62InlineResource[];
extern void overlay62ReleaseReloc(void *resource);
extern void func_8002C5F4(void);

/* This module's exports (ORT rows). */
void overlay62Initialize(void);
void overlay62Update(s32 updateRate);
void overlay62ReleaseAll(void);

#endif
