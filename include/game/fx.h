#ifndef GAME_FX_H
#define GAME_FX_H

#include "PR/ultratypes.h"

/*
 * PROVENANCE: the cone and wake role names follow Jet Force Gemini's public
 * src/fx.c. JFG publishes no corresponding struct layouts; the field widths,
 * offsets, and aggregate boundaries below are reconstructed from Mickey's
 * target loads, stores, and relocations.
 */
typedef struct FxConeTextureInfo {
    u8 pad0[6];
    u16 width;
    u16 height;
} FxConeTextureInfo;

typedef struct FxConeVertex {
    u8 pad0;
    u8 index0;
    u8 index1;
    u8 index2;
    s16 s0;
    s16 t0;
    s16 s1;
    s16 t1;
    s16 s2;
    s16 t2;
} FxConeVertex;

typedef union FxTextureRef {
    FxConeTextureInfo *pointer;
    s32 value;
} FxTextureRef;

typedef struct FxCone {
    FxTextureRef texture;
    FxTextureRef alternateTexture;
    u8 *addresses[2];
    u8 *vertices;
    u8 mode;
    u8 segmentCount;
    u8 addressIndex;
    u8 flags;
    f32 value18;
    f32 value1C;
    s16 value20;
    s16 value22;
    s16 value24;
    s16 value26;
    s16 value28;
    s16 value2A;
    u8 primRed;
    u8 primGreen;
    u8 primBlue;
    u8 pad2F;
    u8 envRed;
    u8 envGreen;
    u8 envBlue;
    u8 pad33[5];
} FxCone;

typedef FxCone FxDrawCone;

typedef struct Wake {
    u8 flags;
    u8 segmentCount;
    u8 state;
    u8 textureIndex;
    f32 value4;
    s16 value8;
    u8 padA[2];
    f32 valueC;
    void *vertices;
    void *samples;
    u8 pad18[0x18];
    void *linked;
    s16 value34;
    s16 value36;
    u8 value38;
    u8 value39;
    u8 value3A;
    u8 value3B;
    s32 value3C;
} Wake;

typedef struct WakeRipple {
    u8 pad0[0x70];
    void *linked;
    u8 value74;
    u8 value75;
    s16 value76;
    s16 value78;
    s16 value7A;
    f32 value7C;
    f32 value80;
    Wake *wake;
} WakeRipple;

typedef struct FxFlags {
    u16 value;
    u8 pad2[0x1E];
} FxFlags;

typedef struct FxStatus {
    u8 value;
    u8 pad1[0x1F];
} FxStatus;

typedef struct FxScreenEffect {
    s32 type;
    s16 value4;
    s16 value6;
    s16 value8;
    s16 valueA;
    s16 valueC;
    s16 valueE;
    s32 value10;
} FxScreenEffect;

typedef struct FxSpdRecord {
    s16 value0;
    s16 value2;
    s16 value4;
    u8 value6;
    u8 value7;
} FxSpdRecord;

typedef struct FxRecord {
    u8 state;
    u8 status;
    u16 flags;
    s32 value4;
    s32 value8;
    s32 valueC;
    s32 value10;
    s16 value14;
    s16 value16;
    s16 value18;
    u8 red;
    u8 green;
    u8 blue;
    u8 value1D;
    u8 value1E;
    u8 value1F;
} FxRecord;

typedef struct FxGfx {
    u32 w0;
    u32 w1;
} FxGfx;

typedef void (*FxTextureCallback)(s32 index, s32 value, s32 arg2);

extern void func_800347A0(void *texture);
extern void func_800320F0(s32 callback);
extern void func_8004ACC4(void);
extern void mmFree(void *ptr);
extern FxFlags D_800D5F5A[];
extern FxStatus D_800D5F59[];
extern FxRecord D_800D5F58[5];
extern s32 D_800D5F50;
extern s32 D_800D6038[2];
extern s32 D_800D6040;
extern FxSpdRecord D_800D5FF8[2][4];
extern FxTextureCallback D_8007D47C[4];
extern FxTextureCallback D_8007D488;
extern s32 D_800D6098[4];
extern s32 D_800D60A8;
extern void *D_800D60B0[4];
extern void *D_800D60BC;
extern void *D_800D60C0[4];
extern void *D_800D60CC;
extern u8 D_800D60D0[4];
extern s32 D_8007D478;
extern FxScreenEffect D_800D6048[4];
extern FxGfx D_800D5FD8[4];
extern void TrapDanglingJump(void);
extern void fxScreenEffect(FxGfx **dList, s32 type, s32 value4, s32 value6,
                           s32 value8, s32 valueA, s32 valueC, s32 valueE,
                           s32 value10);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern f32 D_80083DE8;
extern void func_800349A4(FxGfx **dList, s32 texture, s32 flags, s32 arg3);
extern void func_80047CD8(FxGfx **dList, FxCone *cone, s32 flags, u8 alpha);
extern void func_8004A10C(s32 *screen, u8 glyph, s32 x, s32 y, s32 arg4);
extern s32 sprintf(char *buffer, const char *format, ...);
extern u32 D_8007D320[16];
extern u8 D_8007D364[12];
extern s16 D_8007D370[2];
extern s16 D_8007D374[2];
extern s16 D_8007D378[4];
extern FxGfx D_8007D380[10];
extern FxGfx D_8007D3D0[7];
extern FxGfx D_8007D408[14];
extern s32 *D_800D2FA0;

#endif /* GAME_FX_H */
