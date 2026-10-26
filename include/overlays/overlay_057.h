#ifndef OVERLAY_057_H
#define OVERLAY_057_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "overlays/overlay_045.h"

/* Overlay 57: the title and mode-select menus. */

typedef struct O57ResourceTable {
    void *entries[0x70];
} O57ResourceTable;

typedef struct O57SeedData {
    u8 pad00[0xC];
    s32 value0C;
    s32 value10;
    s32 value14;
    u8 pad18[4];
    s32 value1C;
    u8 pad20[0x1CC];
    s32 value1EC;
    s32 value1F0;
    s32 value1F4;
} O57SeedData;

typedef struct O57AnimObject {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} O57AnimObject;

typedef struct O57SpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
    s32 state;
    f32 scale;
} O57SpawnPacket;

typedef struct O57FinalSpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    u8 byte0A;
    u8 byte0B;
    s8 state;
} O57FinalSpawnPacket;

typedef struct O57SpawnState {
    u8 pad00[8];
    s16 mode;
} O57SpawnState;

typedef struct O57Thing {
    u8 pad00[8];
    O57AnimObject *object;
    u8 pad0C[0xA];
    u8 flags;
    u8 pad17[0x25];
    s32 field3C;
    u8 pad40[0x28];
    O57SpawnState **state;
} O57Thing;

typedef struct O57ModeObject {
    s32 value;
} O57ModeObject;

typedef struct O57Choice {
    u8 pad00[0x2A];
    s8 enabled;
    u8 pad2B[9];
} O57Choice;

typedef struct O57Pair {
    s16 first;
    s16 second;
} O57Pair;

/* Tier B: actual texture renderer node fields; Tier D: this recovered
 * ten-node writable view does not establish an eleventh sentinel. */
#include "game/texture_header.h"
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct O57Config0954 {
    u8 pad00[8];
    u16 field08;
    u16 field0A;
    u16 field0C;
} O57Config0954;

typedef struct O57RenderChannel0954 {
    void *object00;
    s32 pad04;
    s32 fixed08;
} O57RenderChannel0954;

typedef struct O57Metadata0954 {
    u8 pad00[0x2C];
    u8 count2C;
} O57Metadata0954;

typedef struct O57Entry0954 {
    u16 value00;
    u8 pad02[2];
    u32 flags04;
} O57Entry0954;

typedef struct O57Owner0954 {
    O57Metadata0954 *metadata00;
    u8 pad04[0x48];
    O57Entry0954 *entries4C;
} O57Owner0954;

typedef struct O57Level2_0954 {
    O57Owner0954 *owner00;
} O57Level2_0954;

typedef struct O57Level1_0954 {
    u8 pad00[0x68];
    O57Level2_0954 *level68;
} O57Level1_0954;

typedef struct O57Lookup0954 {
    u8 pad00[8];
    O57Level1_0954 *level08;
} O57Lookup0954;

typedef struct O57GridVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O57GridVertex;

typedef struct O57GridPoint {
    f32 x;
    f32 y;
    f32 z;
} O57GridPoint;

typedef struct O57GridModel {
    u8 pad00[0x40];
    O57GridPoint *points;
} O57GridModel;

typedef struct O57GridOwner {
    u8 pad00[0x68];
    O57GridModel **model;
} O57GridOwner;

typedef struct O57GridObject {
    u8 pad00[8];
    O57GridOwner *owner;
} O57GridObject;

typedef struct O57Ease28B4Point {
    u8 pad00[0x0C];
    s16 x;
    s16 y;
} O57Ease28B4Point;

typedef struct O57Ease28B4Transform {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} O57Ease28B4Transform;

typedef struct O57Ease28B4Primary {
    u8 pad00[8];
    O57Ease28B4Transform *source;
} O57Ease28B4Primary;

typedef struct O57Ease28B4Secondary {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[9];
    O57Ease28B4Transform *target;
} O57Ease28B4Secondary;

typedef struct O57Tail2C28Smooth {
    u8 pad00[0xC];
    s16 x;
    s16 y;
} O57Tail2C28Smooth;

typedef struct O57Tail2C28Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} O57Tail2C28Position;

typedef struct Overlay57ModeValue {
    s32 value;
} Overlay57ModeValue;

typedef struct Overlay57Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay57Position;

typedef struct Overlay57Transition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} Overlay57Transition;

typedef struct Overlay57Child {
    u8 pad0[0x3A];
    s8 index;
    u8 pad3B[0x2D];
    void **items;
} Overlay57Child;

typedef struct Overlay57Entry {
    u8 pad0[8];
    Overlay57Child *child;
} Overlay57Entry;

typedef struct Overlay57Draw32A0Record {
    s32 word0;
    s32 word4;
    u32 packed8;
} Overlay57Draw32A0Record;

typedef struct Overlay57SelectionChild {
    u8 reserved00[0x28];
    f32 value28;
    u8 reserved2C[0x0E];
    s8 selector3A;
    s8 selector3B;
    u8 reserved3C[0x2C];
    struct Overlay57Selection **selections68;
} Overlay57SelectionChild;

typedef struct Overlay57SelectionResult {
    u8 reserved00[8];
    Overlay57SelectionChild *child;
} Overlay57SelectionResult;

typedef struct Overlay57MarkedEntry {
    s16 value00;
    u8 reserved02[2];
    u32 flags04;
} Overlay57MarkedEntry;

typedef struct Overlay57CountOwner {
    u8 reserved00[0x2C];
    u8 count2C;
} Overlay57CountOwner;

typedef struct Overlay57Selection {
    Overlay57CountOwner *countOwner00;
    u8 reserved04[0x48];
    Overlay57MarkedEntry *entries4C;
} Overlay57Selection;

typedef struct Overlay57LookupResult {
    u8 reserved00[8];
    Overlay57SelectionChild *child08;
} Overlay57LookupResult;

typedef struct Overlay57Object {
    s32 value;
} Overlay57Object;

/* An animation path record (func_800508B4's result). */
typedef struct Overlay57Result {
    u8 pad00[8];
    struct Overlay57Node *node;
    f32 valueC;
    u8 pad10[4];
    u8 byte14;
    u8 pad15;
    u8 flags;
} Overlay57Result;

typedef struct Overlay57ModeChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 enabled;
    u8 pad2B[9];
} Overlay57ModeChoice;

typedef struct Overlay57ModeOutput {
    u8 mode;
    u8 pad01[3];
    s8 value;
    u8 pad05[0x23];
} Overlay57ModeOutput;

typedef struct O57MenuTransition {
    u8 pad00[0xC];
    s16 horizontal;
    s16 vertical;
} O57MenuTransition;

typedef struct Overlay57MenuSource {
    u8 pad00[0x28];
    s16 index;
    s8 active;
    u8 pad2B[9];
} Overlay57MenuSource;

typedef struct Overlay57MenuEntry {
    u8 type;
    u8 variant;
    u8 mode;
    u8 subtype;
    s8 controller;
    u8 pad05[3];
    s32 value08;
    u8 pad0C[0x1C];
} Overlay57MenuEntry;

/* The resident option word block at D_800D3128 (bitfields, MSB first). */
typedef struct O57MenuFlags {
    u32 pad0 : 13;
    u32 allUnlocked : 1;
    u32 unlockPending : 1;
    u32 pending : 1;
    u32 low : 16;
    u8 pad04[0xF];
    u8 unlockMask;
} O57MenuFlags;

typedef struct Overlay57ModeObject {
    s32 value;
} Overlay57ModeObject;

typedef struct Overlay57ModeResult {
    u8 pad00[0x16];
    u8 flags16;
} Overlay57ModeResult;

typedef struct O57MiddleRenderItem {
    s32 type;
    u8 value04;
    u8 value05;
    u8 value06;
    u8 value07;
} O57MiddleRenderItem;

typedef struct O57MiddleInfo {
    u8 pad00[8];
    u8 value08;
    u8 value09;
    u8 value0A;
} O57MiddleInfo;

typedef struct O57MiddleChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 active;
    u8 pad2B[9];
} O57MiddleChoice;

typedef struct O57MiddleOutput {
    u8 value0;
    u8 pad01[3];
    u8 controller;
    u8 pad05[0x23];
} O57MiddleOutput;

typedef struct O57MiddleTextureNode {
    void *texture;
    void *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} O57MiddleTextureNode;

typedef struct O57MiddleRenderParameters {
    u8 pad00[0x10C];
    f32 position;
    f32 scale;
} O57MiddleRenderParameters;

typedef struct O57MiddleRecord {
    O57MiddleRenderItem items[4];
} O57MiddleRecord;

typedef struct {
    u8 pad00[0x28];
    f32 parameter;
} Overlay57Target;

/* A path's target object; the node setter reads +0x28 as a float
 * (Overlay57Target), the node update as an integer. */
typedef struct Overlay57Node {
    u8 pad00[0x28];
    s32 value28;
    u8 pad2C[0xE];
    u8 byte3A;
    u8 pad3B[0x4D];
    s32 value88;
} Overlay57Node;

extern void *gO57Current100Reloc;

extern void *gO57Previous108Reloc;
extern void *gO57Runtime1B8Reloc;
extern void *gO57ResidentCurrentReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor00Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor00PrepareReloc;
extern Overlay45ResourceDescriptor *gO57DescriptorF8Reloc;
extern Overlay45ResourceDescriptor *gO57DescriptorF8PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor6CReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor6CPrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor70Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor70PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor74Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor74PrepareReloc;
extern Overlay45ResourceDescriptor *gO57Descriptor78Reloc;
extern Overlay45ResourceDescriptor *gO57Descriptor78PrepareReloc;
extern Overlay45ResourceDescriptor *gO57DescriptorFCReloc;
extern Overlay45ResourceDescriptor *gO57Descriptors08Reloc[9];
extern Overlay45ResourceDescriptor *gO57Descriptors30Reloc[9];
extern Overlay45ResourceDescriptor *gO57Descriptors80Reloc[24];
extern Overlay45ResourceDescriptor *gO57DescriptorsE0Reloc[6];
extern Overlay45ResourceDescriptor *gO57Descriptors58Reloc[5];
extern s16 gO57DescriptorList80Reloc[];
extern s16 gO57DescriptorIndicesD8Reloc[9];
extern s16 gO57DescriptorIndicesECReloc[9];
extern s16 gO57DescriptorIndices130Reloc[6];
extern s16 gO57DescriptorList130Reloc[];
extern s32 gO57Value1FCReloc;
extern s32 gO57Value21CReloc;
extern s32 gO57Value23CReloc;
extern s32 gO57Value25CReloc;
extern s32 gO57Value32CReloc;
extern s32 gO57Value34CReloc;
extern RcpTextureNode gO57Rows27CReloc[10];

extern s32 gO57Ids134Reloc[];

extern s32 gO57Value148Reloc;
extern s32 gO57Value14CReloc;


extern s32 gO57NodeValues17CReloc[];
extern s16 gO57ColourCycle1A8Reloc;

extern O57ModeObject gO57ModeObject180Reloc;
extern s32 gO57SpecialModeReloc;
extern u8 D_800D31B0;
extern O57Choice gO57ChoicesReloc[4];
extern u8 gO57ChoiceMaskReloc;
extern u8 gO57ChoiceMaskReferenceReloc;
extern u8 gO57ChoiceSourceReloc;
extern s32 gO57ChoiceDirtyReloc;
extern u8 gO57ChoicePublishedReloc;

extern u16 gO57ResidentFlags10Reloc;

extern O57Pair gO57SpawnPairs3E8Reloc[4];
extern O57Thing *gO57Spawned150Reloc[4];
extern void *func_80028F54(void);
extern void loadFrontEndList(s16 *assetGroup);
extern void setupFrontEndList(s16 *objectGroup);
extern O57Thing *func_8000590C(void *packet, s32 mode);
extern void initColourCycle(void *cycle, s32 count);
extern void joyResetMap(void);
extern s32 frontGetMode(void);
extern void func_8003A754(void);
extern void func_80001708(void);
extern Overlay45ResourceDescriptor *overlay45CreateDescriptor_o057Reloc(
    const char *text, s16 width, s16 height, s32 flags);
extern void overlay45SetMode_o057Reloc(Overlay45ResourceDescriptor *descriptor,
                                       s32 mode);
extern u8 O57_D_1A8;

extern s32 O57_D_224;
extern s32 O57_D_244;
extern s32 O57_D_264;
extern s32 O57_D_4CC;
extern O57Config0954 O57_config0954;
extern u32 O57_nibbleFlags0954;
extern u32 O57_node30Flags0954;
extern s8 O57_labels390[12];
extern s8 O57_labels39CEnd;
extern s8 O57_labels39CStart[4];
extern s8 O57_labels3A0End;
extern s8 O57_labels3A0Start[4];
extern s8 O57_labels3A4;
extern void *O57_queryObject0954;
extern u8 O57_renderData21C[];
extern u8 O57_renderData23C[];
extern O57RenderChannel0954 O57_renderChannel1FC;
extern u8 O57_channelData4D0[];
extern f32 O57_channelValue4DC;
extern u8 O57_channelData4D8[];
extern f32 O57_channelValue4E0;
extern void updateColourCycle(void *state, s32 updateRate);
extern void overlay84SelectCurrent(void *object);
extern void overlay84InitializeCurrent(void);
extern void overlay84ClearActive(void);
extern void overlay84SetBit(s32 notification);
extern O57Lookup0954 *O57_call_0DA4(s32 id);
extern O57GridVertex *D_800D3148;
extern u8 gOverlay57TableIndex;

extern u8 D_4D4[];
extern f32 D_4E4;
extern u8 gO57GridTriangles[];
extern void camSetView(Gfx **displayList, Mtx **context);
extern void func_8002109C(O57GridOwner *owner);
extern void texDPTextureX(Gfx **displayList, void *resource, s32 mode, s32 flags);
extern void *gOverlay57Range150Start[];
extern void *gOverlay57Range150End[];
extern s16 gOverlay57Object100[];
extern void func_80006EA0(void *resource);
extern void freeFrontEndList(s16 *assetGroup);
extern s32 gOverlay57Selection100;

extern void *D_148;
extern s32 gOverlay57UpdateSelection;
extern s32 D_500;
extern s32 D_504;
extern void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                                 u8 red, u8 green, u8 blue, s8 state);
extern void amSndStop(void *handle);
extern O57Ease28B4Point gO57Ease28B4Record21C;
extern O57Ease28B4Point gO57Ease28B4Record23C;

extern s32 gO57Ease28B4State180;
extern u8 gO57Ease28B4Id183;

extern u32 D_800D31B8;
extern O57Ease28B4Secondary *o57Ease28B4LookupSecondaryReloc(s32 id);
extern void mainTitlePageInit(s32 argument);
extern O57Tail2C28Smooth gO57Tail2C28Record21C;
extern O57Tail2C28Smooth gO57Tail2C28Record23C;
extern O57Tail2C28Position *gO57Tail2C28Reference14C;
extern f32 gO57Tail2C28DistanceLimit104;
extern s32 gO57Tail2C28QueryWord180;


extern void camSetNo(s32 value);
extern void *overlay41IsUnitScale(s32 value);
extern void overlay84Mark(void);
extern f32 gOverlay57DistanceLimit;
extern Overlay57Position *gOverlay57Reference;

extern s32 gOverlay57Delay;
extern Overlay57Transition gOverlay57Transition;
extern void modSetTextureFrame(void *item, s32 value, s32 scaled);
extern s32 gO57Draw32A0Available4CC;

extern u32 gO57Draw32A0WordTable4B8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records3F8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records458[];
extern Overlay57Draw32A0Record gO57Draw32A0Work34C[];
extern u32 gO57Draw32A0Masks3C8[];
extern u32 gO57Draw32A0Shifts3D8[];
extern u8 gO57Draw32A0Local08[];
extern u8 gO57Draw32A0Local18[];
extern u8 gO57Draw32A0Local28[];
extern u8 gO57Draw32A0StripeReloc[];
extern s32 overlay84IsUnitScale(void);
extern void frontDrawRectangles(void *anchor, s32 count,
                                  Overlay57Draw32A0Record *records, s32 mode);
extern s32 gO57SelectionList8Reloc[];
extern s32 gO57SelectionList30Reloc[];
extern s32 gO57SelectionCurrent100Reloc[];

extern s32 gO57SelectionIds134Reloc[];

extern s32 gO57SelectionValues158Reloc[];
extern s32 gO57SelectionValues17CReloc[];
extern s32 gO57SelectionPrimary4E8Reloc;
extern s32 gO57SelectionSecondary4ECReloc;
extern s32 gO57SelectionChanging4F0Reloc;

extern s16 gOverlay57Threshold38D8Reloc;
extern s16 gOverlay57Threshold3970Reloc;
extern s32 gOverlay57PublishedIndex3A28Reloc;
extern void overlay84GetValues(s32 *primary, s32 *secondary);
extern void *overlay57Call36F4Reloc(u8 id);
extern s32 overlay84GetEnabledCurrent(void);
extern void overlay84LoadCurrent(s32 argument);
extern u8 gOverlay57Byte3D1CReloc;
extern u8 gOverlay57Byte3EF4Reloc;

extern s32 gO57ModeState144BaseReloc[];
extern s32 gO57ModeState11CBaseReloc[];
extern s32 gO57ModeState118BaseReloc[];
extern s32 gO57ModeState160BaseReloc[];
extern s32 gO57ModeState164BaseReloc[];
extern s32 gO57ModeState184BaseReloc[];
extern s32 gO57ModeChoice4F8;

extern s32 gO57ModePrimaryIds134[];
extern s32 gO57ModeSecondaryIds1A0[];
extern void overlay84ResetCurrent(void);
extern u8 gOverlay57ObjectId;
extern void overlay94SetValue(s32 value);
extern s16 o57CharacterOrder_o057Reloc[10];
extern Overlay57ModeChoice gO57ModeChoices[4];
extern Overlay57ModeChoice gO57ModeChoicesEnd[];
extern Overlay57ModeOutput *gO57ModeOutputs;
extern u8 D_8007BEF8;
extern u8 D_8007BEFC;
extern u8 D_8007BF04;
extern s16 gO57ModeShortValue;
extern s32 gO57ModePrimaryValues[];
extern u8 gO57ModeExternalByte;
extern u8 gO57ModeFinalArg1[];
extern void overlay84ClearMode(void);

extern Overlay57MenuEntry *gOverlay57MenuEntries;
extern u16 gOverlay57MenuHalf;
extern u16 gO57MiddleData31B4;
extern u8 gOverlay57MirrorFlag;
extern Overlay57MenuSource gOverlay57MenuSources[];
extern s32 D_8007BF44;
extern s32 gO57MiddleData31E8;
extern s32 gO57MiddleData31F0;
extern s16 gO57UnlockTable[][4];
extern s16 gO57CourseTable[][4];
extern void mainChangeCameras(s32 camera);
extern void mainSetAnimGroup(s32 group);
extern void func_800291B4(void);
extern void *overlay57TailQueryModeReloc(void);
extern s32 overlay57TailQueryChoiceReloc(s32 mode);
extern void overlay57TailPrepareObjectReloc(u8 id);
extern void overlay57TailStartObjectReloc(u8 id);
extern Overlay57ModeResult *overlay57TailFindObjectReloc(u8 id);
extern void overlay57AdvanceReloc(s32 updateRate);
extern O57MiddleChoice gO57MiddleChoices[];
extern u16 gO57MiddleData31B4, gO57MiddleData31B8;
extern u8 gO57MiddleData3208, gO57MiddleData3214;
extern void *gO57MiddleItems[];

extern O57MiddleInfo gO57MiddleInfo;
extern s32 gO57MiddleStopList[], gO57MiddlePathIndices[], gO57MiddlePathList[];
extern O57MiddleOutput *gO57MiddleOutput;
/* Tier B: all seven former transition views name this existing DATA owner. */
extern s32 o57TransitionComplete_o057Reloc;
extern s32 gO57MiddlePanelPosition, gO57MiddleCanLeave;
extern const char *o57ResultLabelPointers_o057Reloc[6];
extern s16 o57ResultColumns_o057Reloc[11];
extern char o57EmptyTimeFormat_o057Reloc[], o57SplitTimeFormat_o057Reloc[];
extern O57MiddleTextureNode gO57MiddleCaption[], gO57MiddleBadge[];
extern u8 gO57MiddlePathId;
extern O57MiddleRenderParameters D_800D3550;
extern void func_80000F94(u16 soundId, void **handle);
extern void func_80022A50(void **displayList, Mtx **matrices);
extern void func_80025444(s8 *players);
extern s32 func_80025D60(s32 course);
extern void func_80028374(s32 level, s32 character, s32 animation, s32 mode, s32 arg4, s32 arg5);
extern void func_80028528(s32 group);
extern void func_80028540(s32 cameras);
extern void func_80028D24(s32 mode);
extern O57MiddleRecord *func_800291C4(void);
extern void func_8002F618(void **displayList, O57MiddleTextureNode *nodes, s32 x, s32 y, u8 r, u8 g, u8 b, u8 a);
extern void frontDrawObj(s32 spacing);
extern s32 func_8003A700(u8 character);
extern s32 func_800429A4(char *buffer, const char *format, ...);
extern void func_8004B0B8(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_80050688(u8 path);
extern void func_80050704(u8 path);
extern void overlay56SplitTime(s32 time, s32 *first, s32 *second, s32 *third);
extern s32 overlay68CheckKind(s32 course);

extern s32 gOverlay57ObjectReady;
extern Overlay45ResourceDescriptor *gOverlay57Layouts[];
extern s32 gOverlay57StartLinks[];
extern s32 gOverlay57StopLinks[];
extern s32 gOverlay57NodeArguments[];
extern s16 gOverlay57LevelBySelection[];
extern s16 D_800D31BC;

extern s8 gOverlay57TableLabels[5];
extern s8 gOverlay57TableA[][5];
extern s8 gOverlay57TableB[][5];
extern u8 gOverlay57UpdateIndex;
extern u8 D_8007BF2C;
extern u16 gOverlay57UpdateFlags[];
extern u32 gOverlay57UpdateMasks[];
extern u32 gOverlay57UpdateShifts[];
extern s8 gOverlay57UpdateValues[];
extern f32 gOverlay57UpdateScale;
extern void *func_8005077C(s32 id);

extern char **D_8007C0B8;
extern s16 D_8007C0E8[];
extern s16 D_8007C11C[24];
extern void *D_800D31C8[];
extern s32 D_8007C1A0;
extern void fontUseFont(s32 font);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void animseqStartPath(u8 pathId);
extern void *func_800508B4(u8 pathId);
extern void func_8005AD64(void *target, s32 argument, s32 mode, f32 parameter);
/* The node update passes an integer fourth argument: a second spelling of
 * the same resident callee (both rebind to func_8005AD64_o057Reloc). */
extern void func_8005AD64_o057Reloc(void *target, s32 argument, s32 mode, s32 value);
extern Gfx *D_800D3140;
extern void overlay84ActivateCurrent(s32 value);
extern void func_8002FB34(void *object, void *data, f32 x, f32 y, f32 scaleX, f32 scaleY, s32 color, s32 flags);
extern void texAnimateTexSprite(void *object, void *data, s32 count, f32 *value, s32 updateRate);
extern Mtx *D_800D3144;
extern void amSndPlay(u16 sound, void **handle);
extern void animseqStopPath(u8 pathId);
extern void func_8005055C(u8 id);
extern void *camGetPtr(void);
extern void func_800498FC(s32 type, u32 scaleBits, u32 valueBits, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle, s32 cutscene, s32 multiplayer, s32 arg5);
extern s32 overlay84GetActive();
extern O57MenuFlags D_800D3128;
extern s32 overlay84GetCurrent();
extern void fontPrintXY(void *displayList, s32 x, s32 y, void *text, s32 align);
extern O57MenuTransition gO57ModeSetup21C;
extern u8 D_8007BF0C;
extern u8 D_8007BF08;
extern s32 gO57ModePrimaryIds[];
extern s32 gO57ModeSecondaryIds[];
extern void joyCreateMap(const s8 *enabled);
extern void mainSetMode(s32 mode);
extern s16 D_800D31BE;
extern s32 gO57MiddleData31EC;
extern s32 gO57MiddleData31F4;
extern void func_80005548(s32 value);
extern void func_8003A680(s32 value);
extern Overlay45ResourceDescriptor *gOverlay57Resource06C;
extern Overlay45ResourceDescriptor *gOverlay57Resource074;
extern Overlay45ResourceDescriptor *gOverlay57Resource070;
extern Overlay45ResourceDescriptor *gOverlay57Resource078;
extern Overlay45ResourceDescriptor *gOverlay57Resource000;
extern Overlay45ResourceDescriptor *gOverlay57Resource0F8;
extern Overlay45ResourceDescriptor *gOverlay57Resource0FC;
extern Overlay45ResourceDescriptor *gOverlay57Range058Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range058End[];
extern Overlay45ResourceDescriptor *gOverlay57Range008Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range008End[];
extern Overlay45ResourceDescriptor *gOverlay57Range030Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range030End[];
extern Overlay45ResourceDescriptor *gOverlay57Range080Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range080End[];
extern Overlay45ResourceDescriptor *gOverlay57Range0E0Start[];
extern Overlay45ResourceDescriptor *gOverlay57Range0E0End[];

/* Qualified references to the actual private storage owners. */
extern s32 gOverlay57Countdown_o057Reloc;
extern s32 gOverlay57ModeFlag_o057Reloc;
extern s32 gOverlay57ObjectStatus_o057Reloc;
extern s32 gOverlay57SetupDelay_o057Reloc;
extern s32 gOverlay57SetupStatus_o057Reloc;
extern s32 gOverlay57Timer_o057Reloc;
extern s32 gOverlay57TriggerLatched_o057Reloc;
extern s32 o57DetailEnvelope_o057Reloc;
extern s32 o57DetailLayoutMode_o057Reloc;
extern s32 o57DetailSelection_o057Reloc;
extern s32 o57FirstUnlocked_o057Reloc;
extern s32 o57FogFar_o057Reloc;
extern s32 o57FogNear_o057Reloc;
extern s32 o57GridObjectId_o057Reloc;
extern f32 o57InitialPosition_o057Reloc;
extern s32 o57LayoutMoving_o057Reloc;
extern s32 o57LayoutValue_o057Reloc;
extern s32 o57PreviousEntry_o057Reloc;
extern s32 o57PreviousLayoutValue_o057Reloc;
extern s32 o57PreviousSelection_o057Reloc;
extern s32 o57SecondUnlocked_o057Reloc;
extern s32 o57SelectedEntry_o057Reloc;
extern s32 o57SelectionCount_o057Reloc;
extern s32 o57SelectionCountdown_o057Reloc;
extern f32 o57SelectionThreshold_o057Reloc;
extern s32 o57State_o057Reloc;
extern s32 o57VerticalOffset_o057Reloc;
extern s32 o57ZeroedState_o057Reloc;

/* This module's exports (ORT rows). */
void func_overlay_057_F0000000_18A3BF8(void);
void func_overlay_057_F0000954_18A454C(s32 updateRate);
void overlay57ReleaseAll(void);

#endif
