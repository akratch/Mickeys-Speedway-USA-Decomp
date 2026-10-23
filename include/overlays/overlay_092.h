#ifndef OVERLAY_092_H
#define OVERLAY_092_H

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "game/math.h"
/* Exact DKR and JFG scans are negative. */
typedef struct Overlay92State { s16 x, y, z, w; u16 a, b; s8 mode; } Overlay92State;

typedef struct Overlay92Object { s16 x, y, z; u8 pad6[0x5E]; Overlay92State *state; } Overlay92Object;

typedef struct Overlay92InitData {
    u8 pad0[0xA]; s8 x, y, z, mode; u16 a, b; u8 w;
} Overlay92InitData;

typedef struct O92CourseEntry {
    u8 pad00[8];
    u16 start;
    u16 end;
    s8 value;
} O92CourseEntry;

typedef struct O92Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    O92CourseEntry *course;
} O92Object;

typedef struct O92VehicleState {
    u8 pad00[0x3B2];
    u8 coursePosition;
} O92VehicleState;

typedef struct O92Racer {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    O92VehicleState *vehicle;
} O92Racer;
extern f32 gOverlay92DistanceParameters[2];
extern O92Object **func_8000572C(s32 *start, s32 *end);
extern f32 sqrtf(f32 value);

typedef struct Overlay92Owner {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay92Owner;

typedef struct Overlay92CameraRecord {
    s16 yaw;
    s16 pitch;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x3C];
} Overlay92CameraRecord;

typedef struct Overlay92CameraState {
    s16 yaw;
    s16 pitch;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    u8 pad24[0x1A];
    s16 positionTag;
    u8 pad40[0x0E];
    u8 mode;
    s8 pathIndex;
    u16 timer;
} Overlay92CameraState;

typedef struct Overlay92UpdateConfig {
    s8 recordIndex;
    u8 pad001[0x16F];
    u8 smoothAngles;
} Overlay92UpdateConfig;
extern Overlay92CameraRecord D_0[];

/* Typed views of the retained module-local literal pool. */
extern f32 D_8;
extern f32 D_C;
extern s32 D_800C947C;
extern s32 mathDiffAngle(s16 current, s16 target);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern s32 levelGetNumber(void);
extern void func_overlay_008_F00042A8_1862000(void *actor, void *owner,
                                              f32 update);
extern void overlay1ResolveMotionPoint(void *owner, s32 path,
                                              f32 *x, f32 *y, f32 *z);

void overlay92Init(Overlay92Object *object, Overlay92InitData *init);
/* No ORT row (a JUMP target), but kept external: IDO omits static functions
 * from the object's symbol table, which the promotion proofs read. */
s32 func_overlay_092_F0000068_18D5F88(O92Racer *racer, f32 *outX,
                                      f32 *outY, f32 *outZ, s32 *outValue);
void func_overlay_092_F0000308_18D6228(Overlay92Owner *owner,
                                       Overlay92UpdateConfig *config,
                                       Overlay92CameraState *state,
                                       s32 update);

#endif
