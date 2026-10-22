#include "ultra64.h"

typedef struct O35System {
    u8 pad0[0x82];
    u8 flag82;
    s8 mode83;
    u8 pad84[0xE];
    s16 handle92;
    u8 pad94[0x62];
    s8 flagF6;
} O35System;

extern void *D_800C9548;
extern s32 D_4;
extern O35System *D_800792EC;
extern s32 D_o35_reset1;
extern s32 D_800C9544;
extern void *D_800792E8;
extern void *D_o35_paramB;
extern s32 D_800C9560;
extern s32 D_800C9568;
extern void *D_800792F0;
extern s32 D_800792F4;
extern u8 D_80079274;

extern void *mmAlloc(s32, s32);
extern O35System *levelGetLevel(void);
extern s32 mainGetNumberOfCameras(void);
extern void camSetMode(s32);
extern void func_80004C28(s32, s32);
extern void animseqLoadLevelData(s32);
extern void func_80004FE0(s32);
extern void shadowInitBuffers(void *, void *, s32, s32, s32);
extern void shadowGenerate(s32, s32);
extern void shadowChangeBuffer(void);
extern void modelSetModelFlags(s32);
extern void func_8000C540(s32);
extern void *texLoadTexture(s16);
extern void overlay98CollectUniqueY(void *);
extern void func_overlay_035_F00001E0_1881EC0(s32);

void func_overlay_035_F0000000_1881CE0(s32 modelId, s32 value1, s32 value2,
                                        s32 value3, s32 value4, s32 value5) {
    O35System *system;

    D_800C9548 = mmAlloc(0x800, 0x8F);
    D_800792EC = levelGetLevel();
    D_4 = 0;
    if ((D_800792EC->flagF6 == 0) &&
        ((D_800792EC->mode83 == 1) || (D_800792EC->mode83 == 2))) {
        D_4 = 1;
    }

    func_overlay_035_F00001E0_1881EC0(modelId);
    D_o35_reset1 = 0;
    camSetMode(mainGetNumberOfCameras() - 1);
    func_80004C28(value4, 0);
    func_80004C28(value3, 1);
    animseqLoadLevelData(value5);
    func_80004FE0(value2);

    D_800C9544 = 0;
    shadowInitBuffers(D_800792E8, D_o35_paramB, 0x1F4, 0xC8, 0x64);
    shadowGenerate(2, 0);
    shadowGenerate(1, 0);
    shadowChangeBuffer();
    shadowGenerate(2, 0);
    shadowGenerate(1, 0);
    shadowChangeBuffer();

    if (D_800792EC->flag82 == 0) {
        modelSetModelFlags(0);
    }
    func_8000C540(value1);
    if (D_800792EC->flag82 == 0) {
        modelSetModelFlags(8);
    }

    D_800C9560 = 0;
    D_800C9568 = 0x100000;
    if (D_800792EC->handle92 != -1) {
        D_800792F0 = texLoadTexture(D_800792EC->handle92);
        D_800792F4 = 0; }
    D_80079274 = 0;
    if (D_800792EC->flagF6 != 0) {
        overlay98CollectUniqueY(D_800792E8);
    }
}
