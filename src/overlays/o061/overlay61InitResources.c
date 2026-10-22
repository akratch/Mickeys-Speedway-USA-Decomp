#include "ultra64.h"
#include "overlays/o045/resource_descriptor.h"

extern u8 gOverlay61SetupData[];
extern u8 gOverlay61RecordD0[];
extern u8 gOverlay61RecordDC[];
extern u8 gOverlay61RecordE8[];
extern u8 gOverlay61RecordF4[];
extern u8 gOverlay61Record100[];
extern u8 gOverlay61Record10C[];
extern u8 gOverlay61Record114[];
extern u8 gOverlay61Record120[];
extern u8 gOverlay61Record12C[];
extern u8 gOverlay61Record130[];
extern u8 gOverlay61Record134[];
extern u8 gOverlay61Record13C[];
extern u8 gOverlay61Record144[];
extern s32 gOverlay61Handle58;
extern s32 gOverlay61Handle5C;
extern s32 gOverlay61Handle60;
extern s32 gOverlay61Handle64;
extern s32 gOverlay61Handle68;
extern s32 gOverlay61Handle6C;
extern s32 gOverlay61Handle70;
extern s32 gOverlay61Handle74;
extern s32 gOverlay61Handle78;
extern s32 gOverlay61Handle7C;
extern s32 gOverlay61Handle80;
extern s32 gOverlay61Handle84;
extern s32 gOverlay61Handle88;
extern s32 gOverlay61Config08;
extern s32 gOverlay61Config30;
extern s32 gOverlay61StateA4;
extern s32 gOverlay61StateA8;
extern s32 gOverlay61StateAC;
extern s32 gOverlay61StateB0;
extern s32 D_800D31C8[];

void loadFrontEndList(void *data);
void fontUseFont(s32 mode);
Overlay45ResourceDescriptor *overlay61CreateDescriptorReloc(
    void *data, s32 width, s32 height, s32 flags);
void rumbleProcessing(s32 arg0, s32 arg1);

/*
 * Overlay 61 +0x968. Fresh DKR v77/v80 source searches for the full
 * (0xA0/0xB0/0xCC, 0x34/0x10C) thirteen-record call sequence and final
 * 2,0,2,7 state tuple were negative. The nearby DKR ghost/controller-pak
 * source remains a semantic navigation lead only, not a donor or naming
 * basis.
 */
void overlay61InitResources(void) {
    loadFrontEndList(gOverlay61SetupData);
    fontUseFont(3);
    gOverlay61Handle58 = (s32)overlay61CreateDescriptorReloc(gOverlay61RecordD0, 0xA0, 0xE, 4);
    gOverlay61Handle5C = (s32)overlay61CreateDescriptorReloc(gOverlay61RecordDC, 0xA0, 0xB0, 4);
    gOverlay61Handle60 = (s32)overlay61CreateDescriptorReloc(gOverlay61RecordE8, 0xA0, 0xB0, 4);
    gOverlay61Handle64 = (s32)overlay61CreateDescriptorReloc(gOverlay61RecordF4, 0xA0, 0xB0, 4);
    gOverlay61Handle68 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record100, 0xA0, 0xCC, 4);
    gOverlay61Handle6C = (s32)overlay61CreateDescriptorReloc(gOverlay61Record10C, 0xA0, 0xB0, 4);
    gOverlay61Handle70 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record114, 0xA0, 0xB0, 4);
    gOverlay61Handle74 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record120, 0xA0, 0xB0, 4);
    gOverlay61Handle78 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record12C, 0x34, 0xCC, 0);
    gOverlay61Handle7C = (s32)overlay61CreateDescriptorReloc(gOverlay61Record130, 0x10C, 0xCC, 1);
    gOverlay61Handle80 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record134, 0x34, 0xCC, 0);
    gOverlay61Handle84 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record13C, 0x10C, 0xCC, 1);
    gOverlay61Handle88 = (s32)overlay61CreateDescriptorReloc(gOverlay61Record144, 0xA0, 0xB0, 4);
    gOverlay61Config08 = D_800D31C8[4];
    gOverlay61Config30 = D_800D31C8[5];
    gOverlay61StateA4 = 2;
    gOverlay61StateA8 = 0;
    gOverlay61StateAC = 2;
    gOverlay61StateB0 = 7;
    rumbleProcessing(0, 2);
}
