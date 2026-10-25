#ifndef OVERLAY_052_H
#define OVERLAY_052_H

/*
 * Overlay 52: one translation unit (ADR 0006), a race HUD of the overlay
 * 50..55 family: the initializer, the resource-index patcher, the
 * offset-entry copier, the per-frame HUD and the release sweep. Pinned DKR
 * v77/v80 and JFG scans found no exact donor; DKR's resource cleanup shares
 * only the generic release-and-clear pattern.
 */

#include "overlays/patch_indices.h"
#include "PR/ultratypes.h"
#include "game/menu.h"
#include "game/gameVi.h"
#include "overlays/overlay_045.h"
#include "overlays/overlay_056.h"

#include "game/texture_header.h"

/* Tier B: the renderer/copier consume 16-byte nodes terminated by a null
 * first word. Initial cache indices become CPU texture pointers; the legacy
 * pointer initializers/integer patcher view are not native type closure. */
typedef struct Overlay52Entry {
    RcpTextureInfo *resource;
    RcpTextureInfo *alternate;
    u32 value8;
    s16 x;
    s16 y;
} Overlay52Entry;

/* Partial racer layout, from this overlay's accesses and Mickey's vehicle code. */
typedef struct Overlay52Racer {
    s8 playerIndex;
    u8 pad001[0x192 - 1];
    u8 value192;
    u8 pad193[7];
    u8 item;
    u8 itemCount;
    s32 value19C;
    u8 pad1A0[8];
    u16 flags;
    u8 pad1AA[0x383 - 0x1AA];
    s8 lap;
    u8 pad384;
    u8 position;
    u8 pad386[2];
    u8 value388;
    u8 pad389[0x3BA - 0x389];
    s16 value3BA;
    u8 pad3BC[0x400 - 0x3BC];
    s32 time;
    u8 pad404[0x454 - 0x404];
    s16 timeDifference;
    s16 timeDifferenceTimer;
    u8 pad458[4];
    u8 value45C;
} Overlay52Racer;

typedef struct Overlay52Object {
    u8 pad00[0x64];
    Overlay52Racer *racer;
} Overlay52Object;

typedef struct Overlay52Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay52Level;

/* Same 0x20-byte record used by menu.c's MenuCurrentObject. */
typedef struct MenuCurrentObject {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 index;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    s8 pad1C[4];
} MenuCurrentObject;

extern MenuCommand *D_800D3140;
extern void *D_800D3144;
struct MenuFrontObject;
extern struct MenuFrontObject *D_800D31C8[];
extern MenuCurrentObject D_800D3550[];
extern s16 D_8007C180[];
extern s32 D_8007C1B0;
extern s32 D_800C947C;
extern s32 B_overlay_001_F00083E0;

extern void camStandardOrtho(MenuCommand **, void **);
extern Overlay52Object **func_80005750(s32 *);
extern Overlay52Level *levelGetLevel(void);
extern s32 texAnimateSprite(u8 *, s32 *, s32, f32 *, s32);
extern u16 joyGetPressed(s32);
extern void camSetNo(s32);
extern void camSetScissor(MenuCommand **);
extern void func_8002F618(MenuCommand **, Overlay52Entry *, s32, s32, u8, u8, u8, u8);
extern void texDPInit(MenuCommand **);
extern void sprSetTextureFilter(s32);
extern void frontDrawObj(s32);
extern s32 func_800290A0(void);
extern s32 func_8003A7D0(Overlay52Object *);
extern void amSndPlay(u16, void **);
extern s32 mainGetMode(void);
extern u8 *func_80028F54(void);
extern void mainChangeCameras(s32);
extern void func_800016EC(u8);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void amTuneSetFade(f32, u8);


extern u8 *func_80028F54(void);
extern s16 D_800D304E;
extern void amTunePlay(s32);
extern void runlinkDownloadCode(s32);
extern void overlay56LoadResource(void);
extern s32 levelGetNumber(void);
extern void fontUseFont(s32);
extern s32 func_8003A5A0(s32);


extern void overlay56ReleaseResource(void);
extern void freeFrontEndItem(s32 slot);


#endif
