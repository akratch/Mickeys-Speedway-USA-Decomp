#ifndef OVERLAY_068_H
#define OVERLAY_068_H

#include "PR/ultratypes.h"
#include "game/pi.h"

/* Overlay 68: the trail and payload effect entries. */

typedef struct Overlay68EntryHeader {
    s32 active;
    u8 field4;
    u8 timer;
    s16 index;
    s16 width;
    s16 generation;
    void *payload;
} Overlay68EntryHeader;

typedef struct Overlay68Payload {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
} Overlay68Payload;

typedef struct Overlay68Entry {
    void *object;
    s8 kind;
    u8 field5;
    s16 generation;
    s16 width;
    s16 height;
    Overlay68Payload *payload;
} Overlay68Entry;

typedef struct Overlay68Record {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay68Record;

/* The promotion step's view of the same 0x10-byte trail entry: it reads the
 * byte at +5 signed, where the payload builder reads it unsigned. */
typedef struct Overlay68RecordEntry {
    void *object;
    s8 kind;
    s8 field5;
    s16 generation;
    s16 timer;
    s16 recordCount;
    Overlay68Record *records;
} Overlay68RecordEntry;

typedef struct Overlay68CreateInfo {
    s16 type;
    s16 pad2;
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 padD[3];
    Overlay68Entry *entry;
} Overlay68CreateInfo;

typedef struct Overlay68SourceMeta {
    u8 pad0;
    s8 kind;
} Overlay68SourceMeta;

typedef struct Overlay68SourceObject {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[6];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x4C];
    Overlay68SourceMeta *meta;
} Overlay68SourceObject;

typedef struct Overlay68AttachmentPayload {
    s16 x;
    s16 y;
    s16 z;
    s8 red;
    s8 green;
    s8 blue;
    u8 active;
} Overlay68AttachmentPayload;

typedef struct Overlay68Attachment {
    Overlay68SourceObject *object;
    s8 kind;
    u8 field5;
    s16 generation;
    s16 timer;
    s16 phase;
    Overlay68AttachmentPayload *payload;
} Overlay68Attachment;

typedef struct Overlay68TrailSource {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[6];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay68TrailSource;

typedef struct Overlay68TrailPoint {
    s16 x;
    s16 y;
    s16 z;
    s8 red;
    s8 green;
    s8 blue;
    u8 timer;
} Overlay68TrailPoint;

typedef struct Overlay68Trail {
    Overlay68TrailSource *source;
    s8 kind;
    s8 delay;
    s16 generation;
    s16 elapsed;
    s16 index;
    Overlay68TrailPoint *points;
} Overlay68Trail;

typedef struct Overlay68NestedState { u8 pad0[0xE]; u8 flag; } Overlay68NestedState;

typedef struct Overlay68NestedObject { u8 pad0[0x64]; Overlay68NestedState *state; } Overlay68NestedObject;

typedef struct Overlay68TimerEntry {
    s32 active;
    u8 pad4;
    s8 timer;
} Overlay68TimerEntry;

typedef struct Overlay68Keyframe {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 duration;
} Overlay68Keyframe;

typedef struct Overlay68Animation {
    u8 pad00[8];
    s16 endAngle;
    s16 keyframeCount;
    Overlay68Keyframe *keyframes;
} Overlay68Animation;

typedef struct Overlay68ObjectState {
    Overlay68Animation *animation;
    f32 fraction;
    s16 keyframeIndex;
    s16 elapsed;
    s16 angle;
    u8 active;
    u8 opacity;
} Overlay68ObjectState;

typedef struct Overlay68Object {
    s16 red;
    s16 green;
    s16 blue;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 facingAngle;
    u8 pad30[0x0B];
    s8 direction;
    u8 pad3C[0x28];
    Overlay68ObjectState *state;
} Overlay68Object;

typedef struct Overlay68ObjectInit {
    u8 pad0[0xA];
    u8 red;
    u8 green;
    u8 blue;
    u8 padD[3];
    void *resource;
} Overlay68ObjectInit;

typedef struct Overlay68Command {
    u32 w0;
    u32 w1;
} Overlay68Command;

typedef struct Overlay68Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay68Vector;

typedef struct Overlay68VectorOwner {
    u8 pad00[0x40];
    Overlay68Vector *vectors;
} Overlay68VectorOwner;

typedef struct Overlay68DrawEntry {
    s32 word0;
    s8 vectorIndex;
    u8 pad05[3];
    f32 weight;
    u8 pad0C[8];
} Overlay68DrawEntry;

typedef struct Overlay68DrawObject {
    u8 pad00[0x39];
    u8 mode39;
    u8 pad3A[6];
    f32 *scale;
    u8 pad44[0x0C];
    s32 renderState;
    u8 pad54[0x0C];
    Overlay68DrawEntry *entries;
    u8 pad64[4];
    Overlay68VectorOwner **vectorOwner;
    u8 pad6C[0x20];
    u8 entryCount;
} Overlay68DrawObject;

typedef struct Overlay68DrawDescriptor {
    s16 zero0;
    s16 zero2;
    s16 pad4;
    s16 mode6;
    f32 weight;
    f32 one;
    f32 x;
    f32 y;
    f32 z;
    s32 angle;
    s32 sourceWord;
} Overlay68DrawDescriptor;

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    s32 assetOffsets[4];
    s32 assetSizes[4];
    s16 values[4];
} Overlay68Probe;

typedef struct Overlay68ResidentEntry {
    s32 thresholdNumerator;
    u8 pad04[0x1C];
} Overlay68ResidentEntry;

/* Same-module exports reached through SYMBOL records: the shipped words store
 * the module base, so these calls name per-module placeholders, not the
 * definitions (CLAUDE.md, "A same-module call can still be a SYMBOL
 * relocation record"). */
extern s32 overlay68PayloadLimit_o068Reloc(void);
extern void overlay68FinishEntry_o068Reloc();
extern void overlay68ClearNestedFlag_o068Reloc(void *entryOrHandle);

/* Module and resident data. */
extern Overlay68EntryHeader *D_8007A1F4;
extern Overlay68EntryHeader *gOverlay68SecondaryEntry;
extern void *mmAlloc(s32 size, s32 tag);
extern void mmFree(void *resource);
extern void *gOverlay68Secondary;
extern s16 gOverlay68Types[];
/* Called both bare and with two arguments, as the shipped call sites set up
 * the argument registers. */
extern s32 levelGetNumber();
extern void *func_8000590C(Overlay68CreateInfo *info, s32 count);
/* The live trail entry, also read as an attachment and as an entry header. */
extern Overlay68Trail *gOverlay68Entry;
extern s32 D_800C947C;
extern Overlay68TimerEntry *gOverlay68TimerEntry;
extern void *D_8007A1F8;
extern void func_8005AD64(Overlay68Object *object, s32 arg1, s32 arg2, f32 arg3);
extern s16 func_8000FAE0(f32 x, f32 y, f32 z);
extern s16 Arctanf(f32 x, f32 z);
extern s16 mathDiffAngle(s32 from, s32 to);
extern void func_8005ABA8(Overlay68Object *object, f32 scale,
    f32 updateRate);
extern f32 camGetProjZ(f32 x, f32 y, f32 z);
extern void func_80022E80(Overlay68DrawObject *object);
extern void func_80022FD4(Overlay68Command **displayList, void *mtx, void *vertices,
                                      Overlay68DrawObject *object,
                                      s32 renderState,
                                      Overlay68DrawDescriptor *descriptor,
                                      s32 mode, s32 objectMode);
extern const Overlay68KindPair gOverlay68KindMap[];
extern Overlay68ResidentEntry *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 kind);
extern const Overlay68KindPair gOverlay68KindMapInitial[];
extern volatile const s8 gOverlay68KindMapLoop;

/* This module's exports (ORT rows). */
s32 overlay68PayloadLimit(void);
void overlay68CreatePrimary(void);
void overlay68CreateSecondary(void);
void overlay68ReleasePrimary(void);
void overlay68ReleaseSecondary(void);
void overlay68CreatePayload(Overlay68Entry *entry);
void overlay68AttachObject(Overlay68SourceObject *object);
void overlay68UpdateTrail(s32 updateRate);
void overlay68ClearNestedFlag(Overlay68NestedObject **objectPtr);
void overlay68FinishEntry(void);
void overlay68StartTimer(void);
void overlay68PromoteSecondary(void);
void overlay68InitializeObject(Overlay68Object *object, Overlay68ObjectInit *init);
void overlay68UpdateAnimation(Overlay68Object *object, s32 updateRate);
void overlay68RebuildSecondaryEntry(s32 kind);
void overlay68ReleaseTertiary(void);
s32 overlay68CheckKind(s32 kind);

#endif
