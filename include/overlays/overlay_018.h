#ifndef OVERLAY_018_H
#define OVERLAY_018_H

#include "PR/os_internal.h"
#include "n_audio/mbi.h"

/* SDK command view; arena capacity and completion lifetime are separate. */
typedef Gfx Overlay18Gfx;

/* Resident boot steps overlay18Load runs in order. */
extern void mainPreNMI(void);
extern void overlay5InitializeAudio(void *value);
extern void runlinkFreeCode(s32 phase);
extern void func_80002500(void);
extern void *joyInit(void);
extern void texInitTextures(void);
extern void modInitModels(void);
extern void func_8005A700(void);
extern void overlay30Initialize(void);
extern void arithmeticFunction(void *localData, void *externalData);
extern void diPrintfInit(void);
extern void func_overlay_031_F00002E8_187F808(void);
extern void initWeather(void);
extern void func_8004A0F0(void);
extern void camlightInit(void);
extern void overlay10Initialize(void);
extern void packInit(void);
extern void initFront(void);
extern void animseqInit(void);
extern void fxInit(void);
extern void diRcpTraceInit(void);
extern void fmvInit(void);
/* Runtime relocation metadata resolves this role to osSetTime(OSTime). */
extern void osSetTime(OSTime);
extern void func_80029970(u32);
extern void func_80029104(void);
/* Runtime ORT 518 resolves this placeholder call to resident levelGetCounts. */
extern void levelGetCounts(void);
extern void func_80028EA0(void *);
extern void func_80028F3C(void);

extern void *mmAlloc(s32 size, s32 tag);
extern void *mmGetDelay(s32 mode);
extern void mmSetDelay(s32 selector);
extern void mmFree(void *buffer);
extern void *mmAllocAtAddr(s32 size, void *oldBuffer, s32 tag);
extern void overlay18FinishReconfigureReloc(void *token);

extern void *D_800CF5B8;
extern void *D_8007A1D4;
extern u8 gOverlay18LocalData80[];
extern u8 gOverlay18ExternalData;
extern s32 D_8007A1B8;
extern void *D_800CF5B0;
extern void *D_800D18E0;
extern u32 D_8007A1CC;

/* Display-list heap: per-mode region sizes and the two frame buffers. */
extern const s32 gOverlay18ModeScale8[];
extern const s32 gOverlay18ModeScale10[];
extern const s32 gOverlay18ModeScale64[];
extern const s32 gOverlay18ModeScale16[];
extern s32 gOverlay18Mode;
extern u8 *gOverlay18Buffers[2];
extern u8 *gOverlay18Region8[2];
extern u8 *gOverlay18Region64[2];
extern u8 *gOverlay18Region8Slot0;
extern u8 *gOverlay18Region64Slot0;
extern u8 *gOverlay18Region16Slot0;
extern u8 *D_800CF510[2];
extern Overlay18Gfx *D_800CF518;
extern u8 *D_800CF528[2];
extern u8 *D_800CF580[2];
extern u8 *D_800CF598[2];
extern u8 *D_800CF530;
extern u8 *D_800CF5A0;
extern u8 *D_800CF588;
extern s32 D_800CF51C;
extern s32 D_800CF534;
extern s32 D_800CF58C;
extern s32 D_800CF5A4;

#endif
