#ifndef OVERLAY_060_H
#define OVERLAY_060_H

#include "PR/ultratypes.h"
#include "game/menu.h"
#include "game/font.h"
#include "game/charControl.h"
#include "game/anim.h"
#include "game/math.h"
#include "game/gameVi.h"
#include "n_audio/mbi.h"

/* Overlay 60: the options and save-file menus. */

typedef struct Overlay60SpawnDesc {
    s16 objectId;
    s8 size;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A;
    u8 alpha;
    u8 pad0C;
} Overlay60SpawnDesc;

typedef struct Overlay60Inner {
    u8 pad00[8];
    s16 mode;
} Overlay60Inner;

typedef struct Overlay60Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    u8 pad14[2];
    u8 flags;
    u8 pad17[0x51];
    Overlay60Inner **inner;
} Overlay60Object;

typedef struct RcpTextureInfo RcpTextureInfo;

typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct SavesPackedEntry {
    s32 time;
    u8 initials[3];
    u8 character;
} SavesPackedEntry;

typedef struct SavesSlot { SavesPackedEntry records[4]; } SavesSlot;

typedef struct Overlay60Point {
    s16 x;
    s16 y;
} Overlay60Point;

typedef struct MenuSpawnInner {
    u8 pad00[8];
    s16 mode;
} MenuSpawnInner;

typedef struct Overlay60SettingsBits {
    u32 upper : 5;
    u32 field26_23 : 4;
    u32 stereoMode : 2;
    u32 field20 : 1;
    u32 field19 : 1;
    u32 field18 : 1;
    u32 gap17_16 : 2;
    u32 language : 6;
    u32 field9 : 1;
    u32 field8 : 1;
    u32 lower : 8;
} Overlay60SettingsBits;

typedef struct Overlay60Settings {
    Overlay60SettingsBits bits;
    u16 sfxVolume;
    u16 bgmVolume;
    u16 progress[5];
    u8 pad12;
    u8 unlocked;
    u16 enabledMask;
} Overlay60Settings;

typedef struct Overlay60Command {
    u32 w0;
    u32 w1;
} Overlay60Command;

typedef struct Overlay60Color {
    u8 pad00[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad0B;
} Overlay60Color;

typedef struct Overlay60Choice {
    u8 pad0[0x28];
    s16 slot;
    u8 pad2A;
    s8 active;
    u8 pad2C[8];
} Overlay60Choice;

extern char *o60MenuText_o060Reloc[10];
extern char *o60StereoText_o060Reloc[4];
extern char *o60ToggleText_o060Reloc[2];
extern s32 o60MenuOpacity_o060Reloc;
extern s32 o60PanelOpacity_o060Reloc;
extern s32 o60MenuLabelY_o060Reloc;
extern s32 o60ActivePanel_o060Reloc;
extern s32 o60PanelSelection_o060Reloc;
extern s32 o60PanelScroll_o060Reloc;
extern s32 o60MenuPathIndex_o060Reloc;
extern u8 D_800D31C8[];
extern u8 gOverlay60Bss00[];
extern u32 gOverlay60Bss20;
extern u32 gOverlay60Bss40;
extern u32 gOverlay60Bss60;
extern u32 gOverlay60Bss80;
extern Overlay60Object *gOverlay60ObjectC8;
extern Overlay60Object *gOverlay60ObjectCC;
extern Overlay60Object *gOverlay60ObjectCCY;
extern Overlay60Object *gOverlay60ObjectD0;
extern Overlay60Object *gOverlay60ObjectD0Y;
extern Overlay60Object *gOverlay60ObjectD4;
extern Overlay60Object *gOverlay60ObjectD4Y;
extern s16 gOverlay60CoordsD8[];
extern s16 gOverlay60CoordE8;
extern s16 gOverlay60CoordEA;
extern s16 gOverlay60CoordEC;
extern s16 gOverlay60CoordEE;
extern s16 gOverlay60CoordF0;
extern s16 gOverlay60CoordF2;
extern s16 gOverlay60CoordF4;
extern s16 gOverlay60CoordF6;
extern u32 gOverlay60Bss144;
extern u32 gOverlay60Bss1E4;
extern u32 gOverlay60Bss1EC;
extern u32 gOverlay60Bss0C;
extern Overlay60Object *func_8000590C(Overlay60SpawnDesc *desc, s32 arg1);
/* Called with an Overlay60Object and with a ControlActor. */
extern void func_8005AD64(void *object, s32 arg1, s32 arg2, f32 arg3);
extern s32 frontGetScreenMode(void);
extern void animseqStartPath(u8 index);
extern int sprintf(char *buffer, const char *format, ...);
extern ControlActor **func_8000572C(s32 *start, s32 *end);
extern void amSndPlay(u16 sound, void **handle);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void amTunePlay(u8 sequence);
extern void amTuneSetFade(f32 fade, u8 volume);
extern void fontUseFont(s32 font);
extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);
extern void fontPrintXY(Gfx **commands, s32 x, s32 y, char *text, s32 flags);
extern void func_8002F618(Gfx **commands, RcpTextureNode *texture,
                        s32 x, s32 y, s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8002FB34(Gfx **commands, RcpTextureNode *texture,
                        f32 x, f32 y, f32 scaleX, f32 scaleY, s32 alpha, s32 flags);
extern void texAnimateTexSprite(void *texture, s32 *state, s32 speed, f32 *frame, s32 ticks);
extern void frontSet2PlayerSplit(s32 split);
extern s32 func_8003A700(u8 initial);
extern void viChangeMode(s32 screenMode);
extern void viNoClear(void);
extern void camSetFOV(f32 fov, s32 force);
extern MtxF *func_8002468C(void);
extern void func_80029198(void);
extern SavesSlot *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 level);
extern void texDPInit(Gfx **commands);
/* Called with Gfx and Overlay60Command list heads. */
extern void texDPTextureX(void *commands, s32 arg1, s32 arg2, s32 arg3);
extern void camStandardPersp(Gfx **commands, Mtx **matrices);
extern void camStandardOrtho(Gfx **commands, Mtx **matrices);
extern void rcpClearZBuffer(Gfx **commands, s32 width, s32 height,
                           s32 left, s32 top, s32 right, s32 bottom);
extern s32 func_8005ABA8(ControlActor *object, f32 scale, f32 ticks);
extern void modSetTextureFrame(MenuSpawnInner *model, s32 index, s32 frame);
extern void func_80009E78(Gfx **commands, Mtx **matrices,
                        void **vertices, ControlActor *object);
extern void *texLoadSprite(s32 asset, s32 flags);
extern void texFreeSprite(void *sprite);
extern void camDo2DSprite(Gfx **commands, Mtx **matrices,
                        void **vertices, void *transform, void *sprite,
                        s32 flags, s32 alpha);
extern void mainChangeLevel(s32 level, s32 entrance, s32 cutscene,
                            s32 arg3, s32 arg4, s32 arg5);
extern s32 overlay41IsUnitScale(s32 index);
extern void overlay48Initialize(void);
extern void overlay56SplitTime(s32 time, s32 *minutes,
                                           s32 *seconds, s32 *hundredths);
extern s32 overlay68CheckKind(s32 index);
extern s32 overlay82GetSelection(ControlActor *object);
extern s32 overlay82IsActive(ControlActor *object);
extern void overlay82Disable(ControlActor *object);
extern void overlay82Enable(ControlActor *object);
extern u16 D_8007BF14_o060Reloc;
extern u16 D_8007BF1C_o060Reloc;
extern char **D_8007C0B8_o060Reloc;
extern s16 D_8007C0E8_o060Reloc[];
extern s16 D_8007C11C_o060Reloc[];
extern s32 D_800D2FC0_o060Reloc;
extern Overlay60Settings D_800D3128_o060Reloc;
extern Gfx *D_800D3140_o060Reloc;
extern Mtx *D_800D3144_o060Reloc;
extern void *D_800D3148_o060Reloc;
extern s32 D_800D31B4_o060Reloc;
extern s32 D_800D31B8_o060Reloc;
extern s16 D_800D31BC_o060Reloc;
extern s16 D_800D31BE_o060Reloc;
extern RcpTextureInfo *D_800D31C8_o060Reloc[];
extern RcpTextureNode gOverlay60Data020;
extern RcpTextureNode gOverlay60Data040;
extern RcpTextureNode gOverlay60Data060;
extern RcpTextureNode gOverlay60Data080;
extern s32 gOverlay60Data0A0;
extern f32 gOverlay60Data0A4;
extern ControlActor *gOverlay60Data0A8;
extern char gOverlay60Data0C0[];
extern char gOverlay60Data0C4[];
extern ControlActor *gOverlay60Data0C8[];
extern Overlay60Point gOverlay60Data0D8[];
extern Overlay60Point gOverlay60Data0E8[];
extern f32 gOverlay60Data0F8[];
extern s32 gOverlay60Data10C[];
extern s32 gOverlay60Data11C[];
extern s8 gOverlay60Data12B[];
extern s8 gOverlay60Data12C[];
struct Sprite;
extern s32 o60PreviewMode_o060Reloc;
extern void *o60PreviewSound_o060Reloc;
extern u16 o60PreviewSoundIds_o060Reloc[10];
extern s32 o60SubmenuState_o060Reloc;
extern s32 o60PanelValue_o060Reloc;
extern s32 o60PanelPhase_o060Reloc;
extern s32 o60VideoTransition_o060Reloc;
extern s32 o60PreviewTune_o060Reloc;
extern s32 o60SettingsPending_o060Reloc;
extern s32 o60ExitRequested_o060Reloc;
extern s16 o60CharacterSpriteIds_o060Reloc[6];
extern struct Sprite *o60CharacterSprite_o060Reloc;

extern u8 gOverlay60Data178[];
extern char *gOverlay60Data1A4[];
extern s16 o60RecordColumns_o060Reloc[13];
extern s16 o60RecordRows_o060Reloc[4];
extern const char o60ControlLabelFormat_o060Reloc[];
extern const char o60ControlValueFormat_o060Reloc[];
extern const char o60EmptyRecordFormat_o060Reloc[];
extern const char o60RecordFormat_o060Reloc[];
extern const char o60StereoLabelFormat_o060Reloc[];
extern const char o60StereoValueFormat_o060Reloc[];
extern Gfx gOverlay60Data0B0[];
extern void *gOverlay60Resources[];
extern void *gOverlay60ResourcesEnd[];
extern u8 gOverlay60PrimaryResource;
extern void overlay60ReleaseReloc(void *resource);
extern void func_8002917C(void);
extern u8 gOverlay60LocalDataReloc[];
extern Overlay60Command *D_800D3140;
extern s16 D_800D31C0;
extern u8 D_800000B0[];
extern s32 overlay82IsActive_o060Reloc(void *object);
extern void initColourCycle(Overlay60Color *color, s32 index);
extern void updateColourCycle(Overlay60Color *color, s32 amount);
extern Overlay60Choice D_800D3058_o060Reloc[4];
extern Overlay60Choice D_800D3058Pass2_o060Reloc[4];
extern Overlay60Choice D_800D3058Pass2End_o060Reloc[];

/* This module's exports (ORT rows). */
void func_overlay_060_F0000000_18B9DD8(void);
void func_overlay_060_F0000334_18BA10C(s32 ticks);
void overlay60ReleaseResources(void);

#endif
