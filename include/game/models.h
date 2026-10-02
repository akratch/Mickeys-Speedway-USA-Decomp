#ifndef _GAME_MODELS_H_
#define _GAME_MODELS_H_

#include "PR/ultratypes.h"

typedef struct ModelTexture {
    /* 0x00 */ void *texture;
    /* 0x04 */ u16 pad4;
    /* 0x06 */ s16 textureId;
} ModelTexture;

typedef struct ObjectModel {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 pad12[4];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ ModelTexture *textures;
    /* 0x1C */ void *vertices;
    /* 0x20 */ void *triangles;
    /* 0x24 */ struct ModelGfxPart *batches;
    /* 0x28 */ void *unk28;
    /* 0x2C */ u8 textureAnimationCount;
    /* 0x2D */ u8 pad2D[3];
    /* 0x30 */ void *unk30;
    /* 0x34 */ void *unk34;
    /* 0x38 */ void *unk38;
    /* 0x3C */ u8 pad3C[0x10];
    /* 0x4C */ s16 references;
    /* 0x4E */ s8 animationCount;
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ void **animations;
    /* 0x54 */ void *unk54;
    /* 0x58 */ void *unk58;
    /* 0x5C */ void *unk5C;
    /* 0x60 */ void *unk60;
    /* 0x64 */ void *unk64;
    /* 0x68 */ void *unk68;
    /* 0x6C */ void *unk6C;
    /* 0x70 */ s32 nestedCount;
    /* 0x74 */ u8 *nestedGroups;
    /* 0x78 */ void **nestedAllocations;
    /* 0x7C */ u8 pad7C[4];
} ObjectModel;

typedef struct ModelInstance {
    /* 0x00 */ ObjectModel *objModel;
} ModelInstance;

typedef struct SuspendedModelTexture {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 value;
} SuspendedModelTexture;

void modInitModels(void);
void modFreeModel(ModelInstance *modInst);
void func_80020278(ObjectModel *model);
void func_800203E0(ObjectModel *model);
void modResumeModelTextures(void);
void modelSetModelFlags(s32 flags);
s32 modelGetModelFlags(void);

#endif
