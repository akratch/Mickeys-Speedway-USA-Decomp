#ifndef OVERLAY_004_H
#define OVERLAY_004_H

#include "PR/ultratypes.h"
#include "game/math.h"

typedef struct Overlay4InitState {
    f32 speed;
    u8 pad04[2];
    s16 phase;
    u8 timer;
    u8 trigger;
} Overlay4InitState;

typedef struct Overlay4InitConfig {
    u8 pad00[0xA];
    s8 speed;
    u8 heading;
    u8 pad0C[2];
    u8 timer;
    u8 pad0F[3];
    u8 outputHeading;
} Overlay4InitConfig;

typedef struct Overlay4InitObject {
    s16 heading;
    s16 outputHeading;
    u8 pad04[0x60];
    Overlay4InitState *state;
} Overlay4InitObject;

typedef struct Overlay4MotionState {
    f32 increment;
    s16 targetAngle;
    s16 phase;
    u8 timer;
    u8 trigger;
} Overlay4MotionState;

typedef struct Overlay4Config {
    u8 pad00[0xA];
    s8 threshold;
    u8 pad0B;
    u8 mode;
    u8 timerMinimum;
    u8 timerMaximum;
    u8 spawnMinimum;
    u8 spawnMaximum;
    u8 spawnChance;
    u8 baseAngle;
    u8 phaseSpeed;
    s8 amplitude;
} Overlay4Config;

typedef struct Overlay4Position {
    f32 x;
    f32 y;
    f32 z;
} Overlay4Position;

typedef struct Overlay4PositionOwner {
    u8 pad00[0x40];
    Overlay4Position *position;
} Overlay4PositionOwner;

typedef struct Overlay4SpawnState {
    s32 value;
    s16 angle;
    u8 active;
    u8 field07;
    u8 field08;
    u8 pad09;
    u8 field0A;
} Overlay4SpawnState;

typedef struct Overlay4Spawned {
    u8 pad00[0x3C];
    s32 config;
    u8 pad40[0x24];
    Overlay4SpawnState *state;
} Overlay4Spawned;

typedef struct Overlay4MotionObject {
    s16 angle;
    s16 outputAngle;
    u8 pad04[0x36];
    s8 positionIndex;
    u8 pad3B;
    Overlay4Config *config;
    u8 pad40[0x24];
    Overlay4MotionState *motion;
    Overlay4PositionOwner **positionOwners;
} Overlay4MotionObject;

typedef struct Overlay4SpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
} Overlay4SpawnPacket;

typedef struct Overlay4GroupState {
    s8 group;
} Overlay4GroupState;

typedef struct Overlay4GroupObject {
    u8 pad00[0x64];
    void *state;
} Overlay4GroupObject;

typedef struct Overlay4AttachState {
    Overlay4GroupObject *owner;
    u8 pad04[2];
    u8 flags;
    u8 pad07[4];
    u8 marker;
} Overlay4AttachState;

typedef struct Overlay4Group {
    void *objects[0x20];
    s32 count;
} Overlay4Group;

typedef struct Overlay4RemoveState {
    u8 pad00[0x64];
    Overlay4GroupState *header;
} Overlay4RemoveState;

typedef struct Overlay4RemoveLink {
    Overlay4RemoveState *state;
} Overlay4RemoveLink;

typedef struct Overlay4RemoveObject {
    u8 pad00[0x64];
    Overlay4RemoveLink *link;
} Overlay4RemoveObject;

typedef struct Overlay4ChainObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay4GroupState *state;
} Overlay4ChainObject;

typedef struct Overlay4SearchPayload {
    u8 reserved00[4];
    s32 category;
    u8 identifier;
} Overlay4SearchPayload;

typedef union Overlay4SearchData {
    Overlay4SearchPayload *payload;
    volatile s32 *state;
} Overlay4SearchData;

typedef struct Overlay4SearchObject {
    u8 reserved00[0xC];
    f32 x;
    u8 reserved10[4];
    f32 z;
    u8 reserved18[0x2C];
    s16 type;
    u8 reserved46[0x1E];
    Overlay4SearchData data;
} Overlay4SearchObject;

typedef struct Overlay4SearchKey {
    s8 identifier;
    u8 reserved001[0x38B];
    u8 mode;
} Overlay4SearchKey;

extern s32 gOverlay4InitStatus;
extern Overlay4Group gOverlay4Groups[];
extern f32 gOverlay4SearchMaxDistance;

extern Overlay4SearchObject **func_8000572C(s32 *start, s32 *end);
extern void func_8005AD64(Overlay4InitObject *object, s32 arg1,
                                     s32 arg2, f32 arg3);
extern s32 func_8005ABA8(Overlay4MotionObject *object, f32 scale,
                          f32 updateRate);
extern s32 func_8002997C(s32 minimum, s32 maximum);
extern s32 func_8002AA0C(s16 current, s16 target);
extern f32 func_80029274(s32 delta, f32 current, f32 amount);
extern s32 func_80004590(s32 maximum);
extern Overlay4Spawned *func_8000590C(Overlay4SpawnPacket *packet, s32 mode);
extern void func_overlay_036_F00007B0(Overlay4Spawned *spawned, s16 angle,
                                      s16 outputAngle, f32 value);
extern f32 sqrtf(f32 value);

#endif
