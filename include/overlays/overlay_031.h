#ifndef OVERLAY_031_H
#define OVERLAY_031_H

#include "PR/ultratypes.h"

/* Tier D: two 16-byte triangles per record (flags, three vertex indices,
 * three s16 texture coordinate pairs), from this function's stores. */
typedef struct Overlay31IndexRecord {
    u8 a0;
    u8 a1;
    u8 a2;
    u8 a3;
    s16 b4;
    s16 b6;
    s16 b8;
    s16 bA;
    s16 bC;
    s16 bE;
    u8 c10;
    u8 c11;
    u8 c12;
    u8 c13;
    s16 d14;
    s16 d16;
    s16 d18;
    s16 d1A;
    s16 d1C;
    s16 d1E;
} Overlay31IndexRecord;

/* Tier B: resident .data identities (selector 0xFFD +0x3D00 and +0x3D38). */
extern Overlay31IndexRecord *D_8007CA60[7][2];
extern f32 *D_8007CA98[7];

typedef struct Overlay31Behaviour {
    u8 pad00[0x9C];
    void *colourLoop;
} Overlay31Behaviour;

typedef struct Overlay31PaletteState {
    s32 unused;
    s32 count;
} Overlay31PaletteState;

typedef struct Overlay31PaletteCursor {
    u8 reserved00[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    Overlay31PaletteState *state;
} Overlay31PaletteCursor;

typedef struct Overlay31Item {
    u32 flags;
    u8 reserved04[0x1B];
    u8 paletteCount;
    u32 *palette;
    u16 paletteId;
} Overlay31Item;

typedef struct Overlay31EffectRecord {
    u8 pad00[0x24];
    u8 active;
    u8 pad25[3];
} Overlay31EffectRecord;

typedef struct Overlay31Vertex {
    u8 bytes[10];
} Overlay31Vertex;

typedef struct Overlay31ConfigRecord {
    u8 pad00[0x2C];
    s16 level;
    u8 pad2E[0x1E];
    s32 state;
    u8 pad50[0x28];
} Overlay31ConfigRecord;

typedef struct Overlay31Config {
    u8 pad00[4];
    s32 arg2;
    s32 arg3;
    s32 arg0;
    u8 pad10[4];
    Overlay31ConfigRecord *records;
    s32 count;
    s32 reserved1C;
    u32 *masks;
    s32 lastMask;
    s32 reserved28;
    void *source;
    void *source10;
    void *source20;
} Overlay31Config;

typedef struct Overlay31Record {
    void *owner;
    u8 *blocks[9];
    u8 data[9][0x1C];
    u8 active;
    u8 pad125[0x23];
} Overlay31Record;

typedef struct Overlay31PoolRecord {
    s32 state[16];
    u8 reserved40[0x64];
    u8 active;
    u8 reservedA5[0x1B];
} Overlay31PoolRecord;

extern void *D_800D4120[5];
extern u8 *D_8007C8A8;
extern s32 D_8007C8B0;
extern u8 *D_8007C8B4;
extern Overlay31Behaviour **D_8007C8B8;
extern s32 D_8007C8A4;
extern Overlay31Item **D_8007C8AC;
extern s32 D_8007C88C;
extern s32 D_8007C890;
extern void *D_8007C89C[2];
extern u8 D_8007CAB4[];
extern u8 D_8007CAC0[];
extern u8 D_8007CC40[];
extern u8 D_8007CC50[];
extern void *D_8007C898;
extern void *D_8007C894;
extern void **D_8007C884;
extern s32 D_8007C888;
extern Overlay31EffectRecord *D_8007C8BC;
extern s32 D_8007C8C0;

extern void overlay31BuildPalettes(void);

extern void *mmAlloc(s32 size, s32 tag); /* 0:+0x2AE30 */
extern f32 func_8002A8BC(s32 angle); /* 0:+0x2A46C */
extern f32 func_8002A8C0(s32 angle); /* 0:+0x2A470 */
extern void mmFree(void *asset);
extern void *piRomLoad(s32 assetId);
extern void *func_800056A4(s32 assetId);
extern void initColourCycle(Overlay31PaletteCursor *cursor,
                                      s32 paletteId);
extern void updateColourCycle(Overlay31PaletteCursor *cursor,
                                         s32 step);
extern void reset_particles(void);
extern void *texLoadSprite(s32 spriteId, s32 flags);
extern void *texLoadTexture(s32 textureId);
extern void *modLoadModel(s32 modelId, s32 flags);

#endif
