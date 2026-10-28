#ifndef OVERLAY_053_H
#define OVERLAY_053_H

#include "PR/ultratypes.h"
#include "game/menu_resources.h"
#include "overlays/patch_indices.h"
#include "game/menu.h"
#include "game/gameVi.h"
#include "overlays/overlay_056.h"

typedef struct Overlay53Config {
    u8 bytes[0xA0];
} Overlay53Config;

extern s16 gOverlay53Resource0[];

extern s16 gOverlay53Resource14[];

extern u8 gOverlay53PatchTableA[];

extern u8 gOverlay53PatchTableB[];

extern Overlay53Config gOverlay53ConfigA[];

extern Overlay53Config gOverlay53ConfigB[];

extern s32 gOverlay53Value280;

extern s32 gOverlay53Handles288[2];

extern f32 gOverlay53Height290;

extern u16 gOverlay53GlobalHandleReloc;



extern void amTunePlay(s32 mode);

extern void runlinkDownloadCode(s32 type);

extern void overlay56LoadResource(void);

extern s32 levelGetNumber(void);




/* The 0x10-byte front-end display entry the offset copier and the HUD share. */
typedef struct Overlay53Entry {
    void *resource;
    void *alternate;
    u32 value8;
    s16 x;
    s16 y;
} Overlay53Entry;

extern s16 gOverlay53Offsets[];

extern s32 frontGetScreenMode(void);



typedef struct Overlay53Racer {
    u8 pad000[0x19A];
    u8 item;
    u8 pad19B;
    s32 value19C;
    u8 pad1A0[0x383 - 0x1A0];
    s8 lap;
    u8 pad384[0x400 - 0x384];
    s32 time;
} Overlay53Racer;

typedef struct Overlay53Object {
    u8 pad00[0x64];
    Overlay53Racer *racer;
} Overlay53Object;

typedef struct Overlay53Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay53Level;

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

extern Mtx *D_800D3144;

extern MenuCurrentObject D_800D3550[];

extern s16 D_8007C180[];

extern s32 D_800C947C;

extern s32 ext_o1_83e0;

extern void camStandardOrtho(MenuCommand **, Mtx **);

extern Overlay53Object **func_80005750(s32 *);

extern Overlay53Level *levelGetLevel(void);

extern void camSetNo(s32);

extern void camSetScissor(MenuCommand **);

extern void func_8002F618(MenuCommand **, Overlay53Entry *, s32, s32, u8, u8, u8, u8);

extern void texDPInit(MenuCommand **);

extern void frontDrawObj(s32);

extern s32 func_800290A0(void);

extern s32 mainGetMode(void);

extern u8 *func_80028F54(void);

extern void mainChangeCameras(s32);

extern void func_800016EC(u8);

extern void func_8003A590(void);

extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);

extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);

extern void amTuneSetFade(f32, u8);

extern Overlay53Entry gOverlay53TimeDigits[2][10];

extern Overlay53Entry gOverlay53ClockDigits[2][10];

extern Overlay53Entry gOverlay53TimeTemplate[];

extern s32 gOverlay53TransitionDone;

extern s8 gOverlay53BlinkCounter;

extern s32 gOverlay53IconAlpha[2];

extern s16 gOverlay53HudRowY[];

extern s16 gOverlay53IconSplitX[];

extern s16 gOverlay53ItemSplitX[];

extern s16 gOverlay53ClockSplitX[];

extern s32 gOverlay53Items[2];

extern f32 gOverlay53HudHeight;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this cluster. */

extern void overlay53ReleaseReloc();

extern u8 gOverlay53ResourceA[];

extern s32 gOverlay53Handle0;

extern s32 gOverlay53HandleEnd;

#endif
