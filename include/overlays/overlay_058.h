#ifndef OVERLAY_058_H
#define OVERLAY_058_H

#include "PR/ultratypes.h"
#include "game/anim.h"
#include "game/menu.h"
#include "game/font.h"
#include "overlays/overlay_056.h"

/* Overlay 58: the race-results and ranking screens. */

#include "game/texture_header.h"

typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct Overlay58RaceEntry {
    u8 character;
    u8 variant;
    u8 variantCopy;
    u8 gap;
    s32 value;
    s32 lapTimes[3];
    u8 pad14[8];
    u8 counters[6];
    u16 rank;
    u8 flags[4];
} Overlay58RaceEntry;

typedef struct Overlay58RaceState {
    u8 mode;
    u8 active;
    u8 player;
    u8 countdown;
    Overlay58RaceEntry entries[6];
} Overlay58RaceState;

typedef struct Overlay58PlayerSlot {
    u8 pad00[0x2A];
    s8 enabled;
    u8 pad2B[9];
} Overlay58PlayerSlot;

typedef struct Overlay58LanguageText {
    char *text[182];
} Overlay58LanguageText;

typedef struct ColourCycle {
    s32 frame;
    s32 time;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    struct ColourCycle *table;
} ColourCycle;

typedef struct Overlay58Status {
    u8 mode;
    u8 active;
    u8 player;
    u8 value3;
    u8 value4;
} Overlay58Status;

typedef struct Overlay58Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay58Vec3f;

typedef struct Overlay58PathGeometry {
    u8 pad00[0x40];
    Overlay58Vec3f *vertices;
} Overlay58PathGeometry;

typedef struct Overlay58PathObject {
    u8 pad00[0x68];
    Overlay58PathGeometry **geometry;
} Overlay58PathObject;

typedef struct Overlay58AnimPath {
    u8 pad00[8];
    Overlay58PathObject *object;
    u8 pad0C[0xA];
    u8 flags;
} Overlay58AnimPath;

typedef struct Overlay58Gfx {
    u32 w0;
    u32 w1;
} Overlay58Gfx;

typedef struct Overlay58Options {
    u32 flags;
    u8 pad04[0xF];
    u8 bits13;
} Overlay58Options;

typedef struct {
    u8 pad00[0x28];
    f32 parameter;
} Overlay58Target;

typedef struct {
    u8 pad00[8];
    Overlay58Target *target;
    f32 value;
    u8 pad10[4];
    u8 active;
} Overlay58Node;

typedef struct SavesPackedEntry {
    s32 value;
    u8 name[3];
    u8 character;
} SavesPackedEntry;

typedef struct SavesSlot {
    SavesPackedEntry records[4];
} SavesSlot;

typedef union Overlay58AnimationState {
    s32 word;
    u8 bytes[4];
} Overlay58AnimationState;

typedef struct Overlay58Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58Vertex;

typedef struct Overlay58RankEntry {
    s32 value;
    u8 reserved4;
    u8 reserved5;
    u8 reserved6;
    u8 tag;
} Overlay58RankEntry;

typedef struct Overlay58RankSet {
    Overlay58RankEntry entries[3];
    s32 floorValue;
    u8 reserved1C;
    u8 reserved1D;
    u8 reserved1E;
    u8 floorTag;
} Overlay58RankSet;

typedef struct Overlay58PrioritySource {
    u8 reserved0[4];
    u8 tag;
    u8 reserved5[3];
    s32 insertValue;
    s32 floorCandidates[3];
} Overlay58PrioritySource;

typedef struct Overlay58Resource {
    u8 pad0[0x12];
    u8 count;
} Overlay58Resource;

typedef struct Overlay58InputRecord {
    u8 state;
    u8 phase;
    u8 player;
    u8 reserved03[0x23];
    u16 rank;
} Overlay58InputRecord;

extern void *func_80028F54(void);
extern void loadFrontEndList(s16 *assetGroup);
extern void joyResetMap(void);
extern void initColourCycle(ColourCycle *cycle, s32 tableIndex);
extern u8 D_8007BEF8_o058Reloc;
extern u8 D_8007BF0C_o058Reloc;
extern u16 D_8007BF1C_o058Reloc;
extern s32 D_8007BF6C_o058Reloc;
extern Overlay58LanguageText *D_8007C0B8_o058Reloc;
extern s32 D_8007C1B4_o058Reloc;
extern Overlay58PlayerSlot D_800D3058_o058Reloc[];
extern RcpTextureInfo *D_800D31C8_o058Reloc[];
extern s16 D_o058_5AE8[];
extern s32 D_o058_5B28[6];
extern RcpTextureNode D_o058_5B40[];
extern RcpTextureNode D_o058_5B60[];
extern RcpTextureNode D_o058_5B80[];
extern RcpTextureNode D_o058_5BA0[];
/* Tier B: the actual result asset list and looping-sound handle have
 * distinct existing C owners and distinct release providers. */
extern s16 o58ResultAssetIds_o058Reloc[32];
extern void *o58TransitionSound_o058Reloc;
extern s16 D_B8[];
extern s16 D_1A0[];
extern u8 D_2A8[];
extern s32 D_2B0;
extern s32 D_2B4;
/* The state update's looping sound handle. */
extern void *D_2BC;
extern f32 D_2C0;
extern f32 D_o058_5CE0;
extern u8 D_8007BF0C;
extern s16 gOverlay58SelectionTableReloc[][4];
extern s16 gOverlay58UnlockTableReloc[][4];
/* Resident option and status words: read as an array by the packed-status
 * pass and through Overlay58Options/Overlay58Resource elsewhere. */
extern u16 D_800D3128[];
extern s32 D_8007BF44;
extern u8 D_8007BF20;
extern u8 D_8007BF24;
extern u8 D_8007BF28;
extern u8 D_8007BF2C;
extern s32 gOverlay58LevelReloc;
extern s32 gOverlay58TrackReloc;
extern s32 gOverlay58VehicleReloc;
extern s32 gOverlay58TransitionModeReloc;
extern u8 D_8007BEF8;
extern u8 D_8007BEFC;
extern s32 D_8007C1A0;
extern u8 gOverlay58CameraGateReloc;
extern s32 D_800D31B8;
extern Overlay58Gfx *D_800D3140;
extern void *D_800D3144;
/* Called with two and with three arguments. */
extern void amSndPlay();
extern s32 overlay41IsUnitScale(s32 index);
extern void func_800291B4(void);
extern void func_8003A680(s32 value);
extern void joyCreateMap(s8 *activePlayers);
extern void mainChangeLevel(s32 level, s32 track, s32 vehicle, s32 mode,
                            s32 arg4, s32 arg5);
extern void mainChangeCameras(s32 mode);
extern void camSetView(Overlay58Gfx **displayList, void **matrix);
extern void func_8005AD64(Overlay58Target *target, s32 argument,
                                        s32 mode, f32 parameter);
extern void updateColourCycle(void *cycle, s32 updateRate);
extern void fontUseFont(s32 font);
extern void fontPrintXY(MenuCommand **displayList, s32 x, s32 y, char *text, s32 flags);
extern void func_8002F618(MenuCommand **displayList, RcpTextureNode *node, s32 x, s32 y, u8 red, u8 green, u8 blue, u8 alpha);
extern s32 sprintf(char *buffer, const char *format, ...);
extern void mainSetAnimGroup(s32 group);
extern void func_8003A754(void);
extern char *func_8003A5A0(s32 value);
extern void *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 level);
extern s32 func_8003A6B0(u8 index);
extern void func_80029120(u32 flags);
extern void func_8002917C(void);
extern s32 func_8003A700(u8 index);
extern u8 D_8007BEFC_o058Reloc;
extern s32 D_8007BF44_o058Reloc;
extern s32 D_8007C1A0_o058Reloc;
extern MenuCommand *D_800D3140_o058Reloc;
extern s32 D_800D31B8_o058Reloc;
extern s16 D_800D31BC_o058Reloc;
extern s16 D_800D31BE_o058Reloc;
extern u8 D_800D31C4_o058Reloc[4];
extern s16 D_8007C0C0_o058Reloc[4][4];
extern char *D_o058_5C5C[3];
extern s16 D_o058_5C68[6];
extern s16 D_o058_5C74[6];
extern s16 D_o058_5C80[6];
extern s16 D_o058_5C8C[6];
extern char *D_o058_5C98[6];
extern s16 D_o058_5CB0[12];
extern s32 D_8007BF48_o058Reloc;
extern s32 D_8007BF4C_o058Reloc;
extern s32 D_8007BF50_o058Reloc;
extern s32 D_8007BF54_o058Reloc;
extern u8 D_8007BF74_o058Reloc;
extern s16 D_800D304E_o058Reloc;
extern s32 D_o058_5CD8;
extern char D_o058_5D2C[];
extern char D_o058_5D30[];
extern char D_o058_5D34[];
extern char D_o058_5D38[];
extern char D_o058_5D3C[];
extern char D_o058_5D44[];
extern char D_o058_5D48[];
extern char D_o058_5D50[];
extern char D_o058_5D54[];
extern char D_o058_5D5C[];
extern char D_o058_5D60[];
extern char D_o058_5D64[];
extern char D_o058_5D68[];
extern char D_o058_5D6C[];
extern char D_o058_5D70[];
extern char D_o058_5D74[];
extern char D_o058_5D78[];
extern char D_o058_5D7C[];
extern char D_o058_5D84[];
extern char D_o058_5D88[];
extern char D_o058_5D90[];
extern char D_o058_5D94[];
extern char D_o058_5D98[];
extern char D_o058_5DA0[];
extern char D_o058_5DA4[];
extern char D_o058_5DAC[];
extern char D_o058_5DB0[];
extern char D_o058_5DB4[];
extern char D_o058_5DB8[];
extern char D_o058_5DC4[];
extern Overlay58Vertex *D_800D3148;
extern u8 D_80000058[];
extern f32 sqrtf(f32 value);
extern void texDPTextureX(void *displayList, void *texture, s32 flags, s32 arg3);
extern void *D_800D31C8[];
extern u8 D_80000078[];
extern u8 D_80000098[];
extern s16 gOverlay58RankSetIdReloc;
extern void *D_8007A1FC;
extern s16 gOverlay58ResourceId;
extern s32 gOverlay58ResourceState;
extern void *overlay68CheckKind(s16 id);
extern u8 D_8007BF04;
extern u8 D_8007BF08;

/* Actual BSS backing definitions use typed private operand imports. */
extern s32 o58ScoreDeltas_o058Reloc[6];
extern char *o58HeadingText_o058Reloc[4];
extern s32 o58SelectedAlphabetRow_o058Reloc;
extern s32 o58SelectedAlphabetColumn_o058Reloc;
extern s32 o58PathState_o058Reloc;
extern Overlay58AnimationState o58Animation_o058Reloc;
extern s32 o58PathDelay_o058Reloc;
extern s32 gOverlay58FloorChoice_o058Reloc;
extern s32 gOverlay58InsertChoice_o058Reloc;
extern s32 o58ScreenState_o058Reloc;
extern s32 o58PrimaryX_o058Reloc;
extern s32 o58RankX_o058Reloc;
extern s32 o58ScoreX_o058Reloc;
extern s32 o58HeadingX_o058Reloc;
extern s32 o58SlideX_o058Reloc;
extern s32 o58RowsY_o058Reloc;
extern s32 o58NextScreen_o058Reloc;
extern s32 o58ScoreTickDelay_o058Reloc;
extern s32 o58PathAlpha_o058Reloc;
extern s32 o58ActiveState_o058Reloc;
extern s32 o58PathFinished_o058Reloc;
extern s32 gOverlay58PrimaryOrderIndex_o058Reloc;
extern Overlay58RaceEntry *o58TimeOrder_o058Reloc[6];
extern Overlay58RaceEntry *o58RankOrder_o058Reloc[6];
extern s32 o58TimeTies_o058Reloc[6];
extern s32 o58RankTies_o058Reloc[6];
extern s32 o58MenuChoice_o058Reloc;
extern s32 o58PathPending_o058Reloc;
extern s32 gOverlay58ResourceReady_o058Reloc;
extern ColourCycle o58Highlight_o058Reloc;
extern s8 o58ActivePlayers_o058Reloc[4];

/* This module's exports (ORT rows). */
void func_overlay_058_F0000000_18AF1E8(void);
void overlay58ReleaseResources(void);
void func_overlay_058_F00005FC_18AF7E4(s32 updateRate);

#endif
