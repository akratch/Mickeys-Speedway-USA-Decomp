#ifndef OVERLAY_054_H
#define OVERLAY_054_H

/*
 * Overlay 54: one translation unit (ADR 0006), a race HUD: the initializer,
 * the resource-index patcher, the offset-record copier and lookup, the
 * per-frame HUD and the release sweep. Fresh pinned DKR v77/v80 and JFG scans
 * found no exact donor for this cluster.
 */
#include "game/charControl.h"
#include "game/menu.h"
#include "n_audio/mbi.h"
#include "overlays/offset_records.h"
#include "overlays/patch_indices.h"
#include "overlays/overlay_045.h"

/* Tier B: overlay54PatchIndices stores resolved resource addresses in the
 * signed words copied by overlay54CopyOffsetRecords. Retain that shared
 * record type and the project's explicit pointer-to-word conversion. */

/* Tier B: overlay 54's runtime records identify the resident calls and the
 * overlay 45/56 exports below. Their source declarations establish the ABI;
 * the *_o054Reloc names are the generated relocation surface. */
extern void camStandardOrtho_o054Reloc(MenuCommand **dlist, Mtx **matrix);
extern void camSetNo_o054Reloc(s32 camera);
extern void camSetScissor_o054Reloc(MenuCommand **dlist);
extern void **func_80005750_o054Reloc(s32 *count);
extern u8 *levelGetLevel_o054Reloc(void);
extern s32 func_800290A0_o054Reloc(void);
extern s32 func_8003A7D0_o054Reloc(ControlActor *actor);
extern void viGetCurrentSize_o054Reloc(s32 *width, s32 *height);
extern s32 texAnimateSprite_o054Reloc(u8 *resource, s32 *state,
    s32 animation, f32 *frame, s32 updateRate);
extern void func_8002F618_o054Reloc(MenuCommand **dlist,
    OverlayOffsetRecord *records, s32 x, s32 y, u8 red, u8 green,
    u8 blue, u8 alpha);
/* The last parameter is unsigned: the shipped icon call materializes its
 * literal 1 in a fresh temporary instead of reusing the function's s32
 * constant-1 register, which a signed parameter does (one ring draw, worth
 * about 340 aligned rows of temp-register rotation downstream). */
extern void func_8002FB34_o054Reloc(MenuCommand **dlist,
    OverlayOffsetRecord *records, f32 x, f32 y, f32 scaleX, f32 scaleY,
    s32 colour, u8 mode);
extern u16 joyGetPressed_o054Reloc(s32 player);
extern void texDPInit_o054Reloc(MenuCommand **dlist);
extern void sprSetTextureFilter_o054Reloc(s32 mode);
extern void frontDrawObj_o054Reloc(s32 index);
extern s32 frontGetScreenMode_o054Reloc(void);
extern s32 mainGetMode_o054Reloc(void);
extern void *func_80028F54_o054Reloc(void);
extern void mainChangeCameras_o054Reloc(s32 cameras);
extern void func_800016EC_o054Reloc(u8 mode);
extern void func_8003A590_o054Reloc(void);
extern void func_80037414_o054Reloc(s32 kind, f32 duration, f32 delay,
    s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void mainChangeLevel_o054Reloc(s32 level, s32 character,
    s32 animation, s32 mode, s32 arg4, s32 arg5);
extern void amTuneSetFade_o054Reloc(f32 fade, u8 volume);
extern void amSndPlay_o054Reloc(u16 sound, void **handle);
extern void overlay45ReleaseDescriptor_o054Reloc(Overlay45ResourceDescriptor *descriptor);
extern void overlay45SetMode_o054Reloc(Overlay45ResourceDescriptor *descriptor, s32 value);
extern void overlay56SplitTime_o054Reloc(s32 time, s32 *minutes,
    s32 *seconds, s32 *centiseconds);

/* The current-object layout is shared with src/main/menu.c. */
typedef struct O54MenuObject {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    s16 index;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 frame;
    s8 flags[4];
} O54MenuObject;

extern MenuCommand *D_800D3140_o054Reloc;
extern Mtx *D_800D3144_o054Reloc;
extern void *D_800D31C8_o054Reloc[];
extern O54MenuObject D_800D3550_o054Reloc[];
extern u8 D_8007BEF4_o054Reloc;
extern s32 D_800C947C_o054Reloc;
extern s16 D_8007C180_o054Reloc[];
extern s32 D_8007C1B0_o054Reloc;
extern s32 o001_data_83E0_o054Reloc;


extern u8 *func_80028F54(void);
extern s16 gOverlay54Data00;

extern void o54LoadResource();
extern void amTunePlay(s32 mode);
extern void runlinkDownloadCode(s32 mode);
extern void fontUseFont(s32 arg0);
extern s16 levelGetNumber(void);
extern s32 o54CreateObject(void);
extern s32 func_8003A5A0(s32 object);


extern void *D_800D31C8[];
extern u8 gOverlay54ExternalResource[];
extern u8 D_800D3550[];
extern s32 gOverlay54ExternalWord;
extern s16 gOverlay54Offsets[];
extern s32 frontGetScreenMode(void);
extern void overlay54ReleaseReloc();
extern void *gOverlay54Current;
extern u8 gOverlay54ResourceA[];
extern u8 gOverlay54ResourceB[];

/* These fields fall in gaps in the partial ControlPlayer header. Their
 * signedness and widths come from this function's own loads. */
#define O54_LAP(p) ((s8) (p)->pad37C[7])
#define O54_PLACE(p) ((p)->pad37C[9])
#define O54_TIME(p) (*(s32 *) ((p)->pad3FC + 4))
#define O54_TIME_DELTA(p) (*(s16 *) (p)->pad454)

#endif
