#ifndef OVERLAY_013_H
#define OVERLAY_013_H

#include "PR/ultratypes.h"

typedef struct Overlay13QuadTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    s16 uv0U;
    s16 uv0V;
    s16 uv1U;
    s16 uv1V;
    s16 uv2U;
    s16 uv2V;
} Overlay13QuadTriangle;

/* Tier B: the active walks establish 32 records with a 128-byte stride.
 * pad00 and vertices retain incomplete byte-layout views; this does not
 * establish the original field declarations or complete graphics format. */
typedef struct Overlay13Record {
    u8 pad00[4];
    s16 field04;
    u8 state;
    u8 timer;
    f32 scale;
    f32 x;
    f32 z;
    f32 y;
    f32 targetZ;
    f32 velocityX;
    f32 velocityZ;
    f32 velocityY;
    f32 phase;
    s32 vertexIndex;
    u8 vertices[0x50];
} Overlay13Record;

typedef struct Overlay13Command {
    u32 w0;
    u32 w1;
} Overlay13Command;

typedef struct Overlay13RenderState {
    u8 pad00[0xF7];
    u8 r;
    u8 g;
    u8 b;
} Overlay13RenderState;

typedef struct Overlay13View {
    u8 pad00[0x0C];
    f32 x;
    f32 z;
    f32 y;
} Overlay13View;

struct Sprite;
struct TextureHeader;

extern struct Sprite *gOverlay13Resource0;
extern struct Sprite *gOverlay13Resource1;
extern struct TextureHeader *gOverlay13Resource2;
extern s32 gOverlay13ActiveCount;
/* Actual handle cells and signed DATA count, distinct from the BSS active flag.
 * Private imports retain loader operands. Release does not clear the handles
 * or count; cache balance, reentry and graphics lifetime remain caller duties. */
extern struct Sprite *gOverlay13Resource0_o013Reloc;
extern struct Sprite *gOverlay13Resource1_o013Reloc;
extern struct TextureHeader *gOverlay13Resource2_o013Reloc;
extern s32 gOverlay13ActiveCount_o013Reloc;
extern s32 gOverlay13Active_o013Reloc;
extern f32 gOverlay13Gravity;
extern f32 gOverlay13SecondaryScale;
extern f32 gOverlay13Gravity_o013Reloc;
extern f32 gOverlay13SecondaryScale_o013Reloc;
/* Actual backing pool; this import retains the loader operand. */
extern Overlay13Record gOverlay13Records_o013Reloc[32];
extern Overlay13QuadTriangle gOverlay13QuadTriangles[2];
extern Overlay13QuadTriangle gOverlay13QuadTriangles_o013Reloc[2];

extern u8 D_80000000[];

extern struct Sprite *texLoadSprite(s32 assetId, s32 arg1);
extern struct TextureHeader *texLoadTexture(s32 resourceId);
extern void texFreeSprite(struct Sprite *resource);
extern void texFreeTexture(struct TextureHeader *resource);
extern void SetLinkSlot(s32, u16, u16);
extern Overlay13RenderState *levelGetLevel(void);
extern void sprSetIA2ColOverride(u8, u8, u8, u8, u8, u8);
extern void camDoSprite(Overlay13Command **, void *, void *, Overlay13Record *, u8 *, s32, s32);
extern void sprClearIA2ColOverride(void);
extern void texDPTextureX(Overlay13Command **, struct TextureHeader *, s32, s32);
extern Overlay13View *camGetPtr(void);

/*
 * Calls to the module's own exports are SYMBOL relocation records: the
 * shipped word stores the 0xF0000000 addend, not the callee's offset, so
 * they name undefined placeholders until the overlay tables are linker
 * output.
 */
extern void overlay13UpdateRecord_o013Reloc(Overlay13Record *record, s32 ticks);
extern void overlay13Release_o013Reloc(void);

#endif
