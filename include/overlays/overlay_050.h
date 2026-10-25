#ifndef OVERLAY_050_H
#define OVERLAY_050_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "overlays/patch_indices.h"

typedef struct O50Glyph {
    void *texture;
    void *alternate;
    s32 glyph;
    s16 x;
    s16 y;
} O50Glyph;

/* Tier D: the layout func_overlay_050_F0000334_1896CA4 gives this array. */

typedef struct O50MenuObject {
    s16 value00;
    s16 value02;
    s16 angle;
    s16 index;
    f32 value08;
    f32 x;
    f32 y;
    f32 value14;
    f32 frame;
    u8 pad1C[4];
} O50MenuObject;


/* Tier B: runtime export identities; the *_o050Reloc names are the generated
 * relocation surface. */

extern O50MenuObject D_800D3550_o050Reloc[];

extern s16 D_800D304E_o050Reloc;

u8 *func_80028F54_o050Reloc(void); /* 0:+0x28B04 */

void loadFrontEndList_o050Reloc(void *list); /* 0:+0x39738 */

void setupFrontEndList_o050Reloc(void *list); /* 0:+0x39900 */

void amTunePlay_o050Reloc(s32); /* 0:+0xC0 */

void runlinkDownloadCode_o050Reloc(s32); /* 0:+0x31828 */

void overlay56LoadResource_o050Reloc(void); /* 56:+0x118 */

s32 levelGetNumber_o050Reloc(void); /* 0:+0x2630C */

void fontUseFont_o050Reloc(s32); /* 0:+0x4AC54 */

void *func_8003A5A0_o050Reloc(s32); /* 0:+0x3A150 */

void *overlay45CreateDescriptor_o050Reloc(void *, s32, s32, s32); /* 45:+0xC */

void overlay45SetMode_o050Reloc(void *, s32); /* 45:+0x1BE0 */


extern void *D_800D31C8[];

/* Tier D: field widths and offsets from this caller's loads and stores. */

typedef struct O50Racer {
    u8 pad000[4];
    f32 speed;
    u8 pad008[0x18A];
    u8 value192;
    u8 pad193[7];
    u8 item;
    u8 itemCount;
    s32 itemState;
    u8 pad1A0[8];
    u16 flags;
    u8 pad1AA[0x1D9];
    s8 laps;
    u8 pad384;
    u8 value385;
    u8 pad386[2];
    u8 value388;
    u8 pad389[0x31];
    s16 value3BA;
    u8 pad3BC[0x44];
    s32 raceTime;
    s32 lapTimes[20];
    s16 timeDifference;
    s16 differenceTimer;
    u8 pad458[4];
    u8 finished;
} O50Racer;

typedef struct O50Object {
    u8 pad00[0x64];
    O50Racer *racer;
} O50Object;

typedef struct O50Level {
    u8 pad00[0x86];
    s8 laps;
} O50Level;

/* Tier B: callee identities decoded from overlay 50 runtime exports.
 * Signatures follow Mickey callees and the O32 argument surface; the
 * *_o050Reloc names are the generated relocation surface. */

void camStandardOrtho_o050Reloc(Gfx **, Mtx **); /* 0:+0x22600 */

struct Overlay45ResourceDescriptor;
void overlay45ReleaseDescriptor_o050Reloc(
    struct Overlay45ResourceDescriptor *descriptor); /* 45:+0x270 */

void overlay56SplitTime_o050Reloc(s32, s32 *, s32 *, s32 *); /* 56:+0xB8 */

O50Level *levelGetLevel_o050Reloc(void); /* 0:+0x2634C */

s32 func_800290A0_o050Reloc(void); /* 0:+0x28C50 */

s32 func_8003A7D0_o050Reloc(void *); /* 0:+0x3A380 */

/* Tier B: the cursor and colour widths match the resident renderer. The
 * node argument still uses the legacy glyph/resource-patching view. */
void func_8002F618_o050Reloc(Gfx **, void *, s32, s32, u8, u8, u8, u8); /* 0:+0x2F1C8 */

void texDPInit_o050Reloc(Gfx **); /* 0:+0x344D0 */

void frontDrawObj_o050Reloc(s32); /* 0:+0x399E4 */

void freeFrontEndItem_o050Reloc(s32); /* 0:+0x3964C */

void loadFrontEndItem_o050Reloc(s32); /* 0:+0x39794 */

s32 overlay59Interpolate_o050Reloc(s32, s32, s32, s32, s32, s32 *, s32 *, s32); /* 59:+0x784 */

void overlay59BuildList_o050Reloc(s32, void *); /* 59:+0x84C */

void overlay59DrawFrame_o050Reloc(Gfx **, s32, s32, s32); /* 59:+0x8EC */

s32 texAnimateSprite_o050Reloc(void *, s32 *, s32, f32 *, s32); /* 0:+0x360F4 */

void amSndPlay_o050Reloc(u16, void **); /* 0:+0xB44 */

u32 joyGetPressed_o050Reloc(s32); /* 0:+0x250FC */

void func_8004A4B0_o050Reloc(s32, s32, s32, s32, s32); /* 0:+0x4A060 */

void fontBackground_o050Reloc(s32, s32, s32, s32); /* 0:+0x4AC8C */

void fontColour_o050Reloc(s32, s32, s32, s32, s32); /* 0:+0x4AC68 */

void fontPrintXY_o050Reloc(Gfx **, s32, s32, char *, s32); /* 0:+0x4ACA8 */

s32 mainGetMode_o050Reloc(void); /* 0:+0x288C8 */

void func_800016EC_o050Reloc(s32); /* 0:+0x129C */

void func_8003A590_o050Reloc(void); /* 0:+0x3A140 */

void func_80037414_o050Reloc(s32, f32, f32, s32, s32, s32, s32); /* 0:+0x36FC4 */

void mainChangeLevel_o050Reloc(s32, s32, s32, s32, s32, s32); /* 0:+0x27F24 */

void amTuneSetFade_o050Reloc(f32, u8); /* 0:+0x17C */


/* The three read-only objects stay extern; the relocation surface values
 * them at their .rodata offsets. */

extern char o50SpeedText[];

extern f32 o50ReverseScale;

extern f32 o50ForwardScale;

/* Tier B: runtime export identities, kept distinct from stored addends. */

extern Gfx *D_800D3140_o050Reloc;

extern Mtx *D_800D3144_o050Reloc;

extern s32 D_800C947C_o050Reloc;

extern s16 D_8007C180_o050Reloc[];

extern void *D_800D31C8_o050Reloc[];

extern s32 D_8007C1B0_o050Reloc;

extern u16 D_8007BF1C_o050Reloc;

extern s32 D_8007C0BC_o050Reloc;

extern void *gOverlay50ResourceA;

extern void *gOverlay50ResourceB;

extern u8 gOverlay50Storage;

extern s8 gOverlay50Slot;

/* Tier B: the cleanup's first call is the resident sound-handle stop,
 * while its second releases an overlay45 descriptor. */
void amSndStop_o050Reloc(void *sound);

extern void freeFrontEndList(void *storage);

extern void overlay56ReleaseResource(void);

extern void freeFrontEndItem(s32 slot);

typedef struct Overlay50GlyphRecord {
    s32 sharedWord0;
    s32 sharedWord1;
    s32 packedGlyph;
    s16 x;
    s16 y;
} Overlay50GlyphRecord;

extern s32 gOverlay50GlyphOffsets[9];

extern s32 gOverlay50GlyphWord0;

extern s32 gOverlay50GlyphWord1;

extern u8 D_800D3140[];

extern void overlay56SplitTime(s32 value, s32 *minutes, s32 *seconds,
                                    s32 *centiseconds);

extern void func_8002F618(void *context,
                                      Overlay50GlyphRecord *records,
                                      s32 arg2, s32 arg3, s32 red, s32 green,
                                      s32 blue, s32 alpha);

#endif
