#ifndef OVERLAY_055_H
#define OVERLAY_055_H

#include "ultra64.h"
#include "game/menu_resources.h"
#include "n_audio/mbi.h"
#include "overlays/offset_records.h"
#include "overlays/patch_indices.h"
#include "game/menu.h"

/* Overlay 55: the four-player race HUD (lap clock and item icons). */

typedef struct Overlay55Digit {
    void *resource;
    void *alternate;
    s32 value;
    s16 x;
    s16 y;
} Overlay55Digit;

typedef struct Overlay55PlayerState {
    u8 pad000[0x19A];
    u8 character;
    u8 pad19B;
    s32 effectTimer;
    u8 pad1A0[0x383 - 0x1A0];
    s8 racerIndex;
    u8 pad384[0x400 - 0x384];
    s32 time;
} Overlay55PlayerState;

typedef struct Overlay55Object {
    u8 pad00[0x64];
    Overlay55PlayerState *state;
} Overlay55Object;

typedef struct Overlay55Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay55Level;

typedef struct Overlay55Transform {
    void *resource;
    s32 unk04;
    s32 unk08;
    s16 x;
    s16 y;
    s32 unk10;
} Overlay55Transform;

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

typedef struct Overlay55DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay55DisplayCommand;

/* Module data. The initialized image is still a raw blob (phase 3 of
 * docs/OVERLAY_STRUCTURE_SPRINT.md), so these are declarations only. */
extern s16 gOverlay55SetupBase[];
extern u8 gOverlay55SourceBase[];
extern s16 gOverlay55ExternalSetupTarget[];
extern s16 gOverlay55SetupSecondary[];
extern OverlayOffsetRecord gOverlay55RecordsA[];
extern OverlayOffsetRecord gOverlay55RecordsB[];
extern u8 gOverlay55SourceB[];
extern s8 gOverlay55Status[];
extern s32 gOverlay55StateWord;
extern f32 gOverlay55StateValue;
extern s16 gOverlay55Result;
extern s16 gOverlay55Offsets[];
extern Overlay55Digit gOverlay55TimeDigits[4][10];
extern Overlay55Digit gOverlay55ClockDigits[4][2];
extern Overlay55Digit gOverlay55TimeTemplate[];
extern s32 gOverlay55TransitionDone;
extern s8 gOverlay55BlinkCounter;
extern s32 gOverlay55IconAlpha[];
extern s8 gOverlay55Items[];
extern f32 gOverlay55HudHeight;
extern s16 gOverlay55ResourceA[];
extern s16 gOverlay55ResourceB[];

/* Resident data. */
extern u8 D_8007BEF4;
extern s16 D_8007C180[];
extern s32 D_800C947C;
extern Overlay55DisplayCommand *D_800D3140;
extern Mtx *D_800D3144;
extern MenuCurrentObject D_800D3550[];
extern s32 ext_o1_83e0;

/* Resident functions. */
extern void amTunePlay(s32 mode);
extern void runlinkDownloadCode(s32 resource);
extern s32 levelGetNumber(void);
extern s32 frontGetScreenMode(void);
extern u8 *func_80028F54(void);
extern void camStandardOrtho(Overlay55DisplayCommand **, Mtx **);
extern Overlay55Object **func_80005750(s32 *);
extern void viGetCurrentSize(s32 *, s32 *);
extern void camSetNo(s32);
extern void camSetScissor(Overlay55DisplayCommand **);
extern Overlay55Level *levelGetLevel(void);
extern s32 func_800290A0(void);
extern void texDPInit(Overlay55DisplayCommand **);
extern void func_8002F618(Overlay55DisplayCommand **, Overlay55Digit *, s32, s32, u8, u8, u8, u8);
extern void frontDrawObj(s32);
extern void func_8002FB34(Overlay55DisplayCommand **, Overlay55Transform *, f32, f32, f32, f32, s32, u8);
extern s32 mainGetMode(void);
extern void mainChangeCameras(s32);
extern void func_800016EC(u8);
extern void func_8003A590(void);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void amTuneSetFade(f32, u8);

/* Overlay 56's exports. */
extern void overlay56LoadResource(void);
extern void overlay56ReleaseResource(void);
extern void overlay56SplitTime(s32, s32 *, s32 *, s32 *);

/* This module's exports (ORT rows). */
void overlay55Initialize(void);
void func_overlay_055_F000031C_18A1E34(s32 updateRate);
void overlay55ReleaseAll(void);

#endif
