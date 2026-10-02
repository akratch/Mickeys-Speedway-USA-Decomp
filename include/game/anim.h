#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include "PR/ultratypes.h"

typedef struct AnimStreamEntry {
    u16 duration;
    u16 command;
} AnimStreamEntry;

extern s32 *D_800D6D54;
extern u8 *D_800D6D58;
extern u32 D_800D6D5C;
extern AnimStreamEntry *D_8007D698;
extern AnimStreamEntry *D_8007D69C;
extern s32 D_8007D6A0;
extern s8 D_8007D6C0[];
extern s32 *D_8007D68C;
extern s32 D_8007D6A4;
extern u32 D_8007D6A8;
extern f32 D_8007D6AC;
extern u32 osRomBase;
extern u8 *D_8007D680;
extern s32 D_8007D684;
extern s32 D_8007D688;
extern s32 D_8007D690;
extern void *D_8007D694;
extern s32 D_8007D6B0;
extern f32 D_8007D6B4;
extern f32 D_8007D6B8;
extern s32 D_8007D6BC;
extern f32 D_80083FAC;
extern f32 D_80083FB0;
extern f32 D_80084210;
extern f32 D_80084214;
extern f32 D_80084218;
extern u8 D_8007BF04;
extern u8 D_8007BF20;
extern u8 D_8007BF24;
extern u8 D_8007BF28;
extern s16 *D_8007D780[];
extern s16 D_800D6C3E;
extern s16 D_800D6C44;
extern s32 D_800D6C48;
extern s16 D_800D6C4C;
extern s16 D_800D6C52;
extern s16 D_800D6C54;

typedef struct AnimGroupDirectoryEntry {
    u32 packed;
} AnimGroupDirectoryEntry;

typedef struct AnimLevelHeader {
    u8 pad0[0xAE];
    s8 sequenceRate;
} AnimLevelHeader;

typedef struct AnimLevelRomEntry {
    s32 start;
    s32 end;
} AnimLevelRomEntry;

extern AnimLevelRomEntry *D_800D6B04;

/*
 * PROVENANCE: these resident overlay shapes are named and cross-checked
 * against Jet Force Gemini's public animation declarations. The Mickey ROM
 * is authoritative for each boundary, width, and stride below.
 */
typedef struct AnimCameraSource {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad6[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x16];
    s16 unk2E;
} AnimCameraSource;

typedef struct AnimCameraTarget {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad6[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x26];
    s16 unk3E;
} AnimCameraTarget;

typedef struct AnimLightReset {
    s32 unk0;
    u8 pad4[0xC];
} AnimLightReset;

typedef struct AnimScrollReset {
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    u8 pad8[4];
    s32 unkC;
    u8 pad10[4];
} AnimScrollReset;

typedef struct AnimLockonReset {
    s8 unk0;
    u8 pad1[7];
} AnimLockonReset;

typedef struct AnimPathObjectTarget {
    u8 pad0[0x132];
    s16 unk132;
} AnimPathObjectTarget;

typedef struct AnimPathObject {
    u8 pad0[6];
    s16 unk6;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x14];
    s32 unk3C;
    u8 pad40[4];
    s16 unk44;
    u8 pad46[0x12];
    AnimPathObjectTarget *unk58;
    u8 pad5C[0x28];
    void *soundHandle;
} AnimPathObject;

typedef struct AnimPathNode {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 unk6;
    u8 unk7;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    s16 unk18;
    s16 unk1A;
    s16 unk1C;
    u8 unk1E;
    u8 pad1F;
    f32 unk20;
    struct AnimPathNode *previous;
    struct AnimPathNode *next;
} AnimPathNode;

typedef struct AnimPath {
    u8 unk0;
    u8 unk1;
    s16 unk2;
    s16 unk4;
    u8 unk6;
    u8 unk7;
    AnimPathObject *unk8;
    f32 unkC;
    f32 unk10;
    u8 unk14;
    u8 unk15;
    u8 flags;
    u8 nodeCount;
    AnimPathNode *currentNode;
    f32 unk1C;
    AnimPathNode *nodes;
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 unk29;
    s16 unk2A;
    f32 unk2C;
    f32 unk30;
} AnimPath;

extern AnimPath **D_800D6B00;
extern AnimCameraSource *D_800D6B08[4];
extern void *D_800D6B18[16];
extern AnimScrollReset D_800D6B58[8];
extern AnimLockonReset D_800D6BF8[8];
extern AnimLockonReset D_800D6C38[];
extern AnimLightReset D_800D6C58[12];

typedef struct AnimPauseSlot {
    s16 unk0;
    u8 pad2[9];
    u8 unkB;
} AnimPauseSlot;

extern AnimPauseSlot D_800D6D18[5];

typedef struct FmvPlayer {
    s8 unk0;
    u8 pad1[0x10];
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
} FmvPlayer;

typedef struct AnimVec3f {
    f32 x;
    f32 y;
    f32 z;
} AnimVec3f;

typedef struct HitCopySource {
    u8 pad0[0x18];
    AnimVec3f current;
    AnimVec3f previous;
    u8 pad30[0x32];
    u8 unk62;
    u8 unk63;
    f32 unk64;
    u8 pad68[4];
    f32 unk6C;
} HitCopySource;

typedef struct HitCopyTarget {
    s8 unk0;
    u8 pad1[3];
    f32 unk4;
    f32 unk8;
    u8 padC[8];
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    u8 pad28[0x4C];
    AnimVec3f direction;
    f32 magnitude80;
    f32 magnitude84;
    f32 unk88;
    f32 unk8C;
    f32 unk90;
    AnimVec3f velocity;
    u8 padA0[0xE1];
    u8 unk181;
} HitCopyTarget;

typedef struct HitCopyState {
    u8 pad0[0xC];
    AnimVec3f position;
    u8 pad18[4];
    AnimVec3f velocity;
    u8 pad28[0x20];
    HitCopySource *source;
    u8 pad4C[0x18];
    HitCopyTarget *target;
} HitCopyState;

/*
 * PROVENANCE: the following collision overlays use the public Jet Force
 * Gemini hit.c family only for names and shape comparisons. Every Mickey
 * field width, offset, and boundary is taken from this resident block's
 * target assembly.
 */
typedef struct HitCollisionVehicle {
    s8 playerIndex;
    u8 pad1[0xC3];
    s32 soundHandle;
    u8 padC8[0x88];
    f32 unk150;
    u8 pad154[4];
    s16 unk158;
    s16 unk15A;
    s16 unk15C;
    u8 pad15E[0xA];
    s16 unk168;
    s16 unk16A;
    u8 pad16C[0x19];
    u8 unk185;
    u8 pad186[2];
    f32 unk188;
    u8 pad18C[0x1C];
    u16 flags1A8;
    u8 pad1AA[0x20C];
    s16 unk3B6;
    s16 unk3B8;
} HitCollisionVehicle;

typedef struct HitCollisionLink {
    u8 pad0[0x38];
    HitCopyState *state;
} HitCollisionLink;

typedef struct HitCollisionNormalLink {
    HitCopyState *state;
    u8 pad4[0x10];
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
} HitCollisionNormalLink;

typedef struct HitInitEntry {
    u8 pad0[0x18];
    AnimVec3f position;
    f32 scaleX;
    f32 scaleY;
    u8 pad2C[8];
} HitInitEntry;

typedef struct HitInitRecord {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    u16 flags;
    u8 kind;
    u8 mode;
    s16 entryCount;
    AnimVec3f localOffset;
    AnimVec3f position;
    AnimVec3f basePosition;
    u8 pad30[0xC];
    f32 minX;
    f32 minY;
    f32 minZ;
    f32 maxX;
    f32 maxY;
    f32 maxZ;
    u8 pad54[4];
    f32 radius;
    f32 height;
    s8 collisionType;
    u8 pad61[7];
    f32 unk68;
    f32 unk6C;
    u8 pad70[4];
    HitInitEntry *entries;
} HitInitRecord;

typedef struct HitInitDescriptor {
    u16 vertexIndex;
    u8 pad2[6];
    f32 scale;
} HitInitDescriptor;

typedef struct HitInitHeader {
    u8 pad0[0x38];
    HitInitDescriptor *descriptors;
    u8 pad3C[0x12];
    s8 useFloatPositions;
} HitInitHeader;

typedef struct HitInitModel {
    HitInitHeader *header;
    s16 *vertices;
    u8 pad8[0x40];
    f32 *floatPositions;
} HitInitModel;

typedef struct HitInitSource {
    u8 pad0[8];
    f32 scale;
    AnimVec3f position;
    u8 pad18[0x30];
    HitInitRecord *hit;
    u8 pad4C[0x1C];
    HitInitModel **model;
} HitInitSource;

typedef struct HitOverlapVolume {
    u8 pad0[8];
    u8 shape;
    u8 pad9[0x13];
    f32 unk1C;
    u8 pad20[4];
    AnimVec3f position;
    u8 pad30[0x28];
    f32 radius;
    f32 height;
    u8 pad60;
    u8 active;
} HitOverlapVolume;

typedef struct HitOverlapVehicle {
    u8 pad0[0x54];
    f32 overlap54;
    u8 pad58[0x112];
    s16 unk16A;
    u8 pad16C[0x21];
    u8 timer18D;
    u8 pad18E[0x1AA];
    void *target338;
} HitOverlapVehicle;

typedef struct HitOverlapState {
    u8 pad0[0xC];
    AnimVec3f position;
    u8 pad18[0x2C];
    s16 kind44;
    u8 pad46[0x1E];
    HitOverlapVehicle *vehicle;
} HitOverlapState;

extern void *D_800D76D0[2];
extern FmvPlayer D_800D76D8[2];

void func_80050000(s32 *stream);
s32 func_80050024(u32 bitCount);
s32 func_800500A4(u32 bitCount);
void func_8005013C(void);
void func_8005017C(void);
s32 func_800501AC(AnimStreamEntry *entry);
s32 func_800501C8();
void func_8005027C(void);
void func_800502CC(u8 pathIndex);
void func_80050AD4(u8 pathIndex);
void func_80006EA0(void *ptr);
void func_80050348();
void func_8005055C();
void animseqStartPath(u8 pathIndex);
void animseqStopPath(u8 pathIndex);
void animseqInitGroup(void);
void animseqPlay(void);
AnimPath *func_800508B4(u8 pathIndex);
void animseqLockPath(u8 pathIndex);
void animseqUnLockPath(u8 pathIndex);
u32 func_8005077C(u8 pathIndex);
void animseqHoldPath(u8 pathIndex);
void amSndStop(void *ptr);
void func_80050D50(void);
void mmFree(void *ptr);
void animseqFreeLevelData(void);
void func_80050E9C(void);
void func_800534C0();
s32 TrapDanglingJump();
void func_800534EC(s32 arg0);
void animseqResetGroup(void);
void *piRomLoad(s32 resourceId);
void fmvInit(void);
void func_800557F8(HitCopyState *first, HitCopyState *second, f32 unused);
void func_80055D08(HitCopyState *first, HitCopyState *second, f32 unused);
void func_80055E50(HitCopyState *first, HitCopyState *second, f32 unused);
void func_80056274(HitCopyState *first, HitCopyState *second, f32 unused);
void func_800572AC(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat);
void func_80057350(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat);

#endif
