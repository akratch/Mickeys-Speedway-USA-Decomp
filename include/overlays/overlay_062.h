#ifndef OVERLAY_062_H
#define OVERLAY_062_H

#include "PR/ultratypes.h"
#include "game/menu_resources.h"
#include "n_audio/mbi.h"
#include "game/texture_header.h"

/* Overlay 62. */

typedef struct Overlay62Root {
    u8 pad00[0x25C];
    void *primary;
    void *items[6];
} Overlay62Root;

typedef Gfx Overlay62Gfx;

/* Same file-scope tag and fields as the actual renderer's node contract. */
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

struct Sprite;
typedef struct Overlay45ResourceDescriptor Overlay45ResourceDescriptor;

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

/* The complete module directly references its actual DATA/BSS definitions. */

/* Resident data and functions. */
extern Overlay62Root *D_8007C0B8;
/* The shared polymorphic cache supplies entry 6 through the asset path.
 * Resource bounds and lifetime remain separate from cache storage. */
extern void loadFrontEndList(s16 *assetGroup);
extern void freeFrontEndList(s16 *assetGroup);
extern void fontUseFont(s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern Overlay45ResourceDescriptor *overlay45CreateDescriptor(
    const char *text, s16 width, s16 height, s32 flags);
extern struct Sprite *texLoadSprite(s32 spriteId, s32 flags);
extern void texFreeSprite(struct Sprite *sprite);
extern void overlay45SetMode(Overlay45ResourceDescriptor *descriptor, s32 mode);
extern void overlay45ReleaseDescriptor(Overlay45ResourceDescriptor *descriptor);
extern s32 D_800D3180;
extern Overlay62Gfx *D_800D3140;
extern Mtx *D_800D3144;
extern u8 D_800D3148[];
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
                                   char *item, s32 mode);
extern void texAnimateTexSprite(u8 *textureData, s32 *state, s32 mode,
                                      f32 *timer, s32 updateRate);
extern void func_8002F618(Overlay62Gfx **commands, RcpTextureNode *nodes,
                                    s32 x, s32 y, u8 red, u8 green,
                                    u8 blue, u8 alpha);
extern void func_8002C5F4(void);

/* This module's exports (ORT rows). */
void overlay62Initialize(void);
void overlay62Update(s32 updateRate);
void overlay62ReleaseAll(void);

#endif
