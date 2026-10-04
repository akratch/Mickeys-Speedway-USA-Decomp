#include "PR/os_internal.h"
#include "PR/ultratypes.h"

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
extern s32 overlay92TransitionFlagReloc;

extern s32 mathDiffAngle(s16 current, s16 target);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 Arctanf(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern s32 levelGetNumber(void);
extern void func_overlay_008_F00042A8_1862000(void *actor, void *owner,
                                              f32 update);
extern void func_overlay_001_F0000DF4_184D1D4(void *owner, s32 path,
                                              f32 *x, f32 *y, f32 *z);
extern s32 func_overlay_092_F0000068_18D5F88(void *owner, f32 *x, f32 *y,
                                             f32 *z, s32 *pathIndex);

/* Reconstructed from Mickey-owned code and relocation evidence.
 * The union self-copy is an independently reviewed IDO-specific allocation
 * aid, not a claim about original source spelling. */
void func_overlay_092_F0000308_18D6228(Overlay92Owner *owner,
                                       Overlay92UpdateConfig *config,
                                       Overlay92CameraState *state,
                                       s32 update) {
    union { f32 value; u32 bits; } x;
    f32 y;
    f32 z;
    f32 pathX;
    f32 pathY;
    f32 pathZ;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 blend;
    f32 wave;
    s16 remaining;
    Overlay92CameraRecord *record;
    s32 pathIndex;
    s32 level;

    record = &D_0[config->recordIndex];
    if (update >= state->timer) {
        state->timer = 0;
    } else {
        state->timer -= update;
    }

    if (config->smoothAngles != 0) {
        remaining = update;
        while (remaining--) {
            state->pitch += mathDiffAngle(state->pitch, 0x800) >> 3;
            state->roll -= state->roll >> 3;
        }
        return;
    }

    switch (state->mode) {
        case 1:
            blend = (f32)state->timer - 150.0f;
            if (blend < 0.0f) {
                blend = 0.0f;
            }
            blend = 1.0f - (blend / 150.0f);
            wave = (func_8002A8C0((s32)((blend * D_8) + 16384.0f)) *
                    -0.5f) +
                   0.5f;
            if (state->timer == 0) {
                state->mode = 2;
                state->timer = mathRnd(0x1C2, 0x546);
            }

            func_overlay_008_F00042A8_1862000(record, owner, (f32)update);
            x.value = record->x;
            y = record->y;
            z = record->z;
            func_overlay_001_F0000DF4_184D1D4(owner, 0, &pathX, &pathY,
                                              &pathZ);
            /* Initialized representation self-copy: inert on IDO/o32.
             * Retains the stack-backed float lifetime; see cleanup queue. */
            x.bits = x.bits;
            x.value = ((pathX - x.value) * wave) + x.value;
            y = ((((func_8002A8BC((s32)(wave * D_C)) * -0.5f) +
                    0.5f) *
                   30.0f) +
                 (y + ((pathY - y) * wave)));
            z = z + ((pathZ - z) * wave);

            state->targetX = x.value;
            state->x = state->targetX;
            state->targetY = y;
            state->y = state->targetY;
            state->targetZ = z;
            state->z = state->targetZ;
            state->positionTag = func_8000FAE0(x.value, y, z);

            dx = x.value - owner->x;
            dy = (y - owner->y) - 20.0f;
            dz = z - owner->z;
            wave = blend * 3.0f;
            if (wave > 1.0f) {
                wave = 1.0f;
            }
            state->yaw =
                (s16)((mathDiffAngle(record->yaw,
                                     Arctanf(dz, dx) + 0x4000) *
                      wave) +
                      record->yaw);
            level = (s16)Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->pitch =
                (s16)((record->pitch * (1.0f - wave)) +
                      (level *
                       wave));
            state->roll = (s16)(record->roll * (1.0f - wave));
            break;

        case 2:
            if (func_overlay_092_F0000068_18D5F88(
                    owner, &x.value, &y, &z, &pathIndex) != 0) {
                state->targetX = x.value;
                state->x = state->targetX;
                state->targetY = y;
                state->y = state->targetY;
                state->targetZ = z;
                state->z = state->targetZ;
                state->positionTag = func_8000FAE0(x.value, y, z);
                dx = x.value - owner->x;
                dy = (y - owner->y) - 20.0f;
                dz = z - owner->z;
            } else {
                dx = state->x - owner->x;
                dy = (state->y - owner->y) - 20.0f;
                dz = state->z - owner->z;
            }
            state->yaw = Arctanf(dz, dx) + 0x4000;
            state->pitch = Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->roll = 0;
            break;

        case 0:
            level = levelGetNumber();
            _bcopy(state, record, 0x54);
            if ((level == 0x11) || (level == 0x10)) {
                state->timer = 0;
            } else {
                state->mode = 3;
                state->timer = 0x12C;
            }
            /* fallthrough */

        case 3:
            if ((state->timer == 0) ||
                (overlay92TransitionFlagReloc != 0)) {
                state->mode = 2;
                state->timer = mathRnd(0x1C2, 0x546);
            }
            if (state->pathIndex == -1) {
                if ((func_overlay_092_F0000068_18D5F88(
                         owner, &x.value, &y, &z, &pathIndex) == 0) ||
                    (pathIndex == -1)) {
                    pathIndex = 3;
                }
                state->pathIndex = pathIndex;
            }
            state->pathIndex = 3;
            func_overlay_001_F0000DF4_184D1D4(owner, state->pathIndex, &x.value,
                                              &y, &z);
            state->targetX = x.value;
            state->x = state->targetX;
            state->targetY = y;
            state->y = state->targetY;
            state->targetZ = z;
            state->z = state->targetZ;
            state->positionTag = func_8000FAE0(x.value, y, z);
            dx = x.value - owner->x;
            dy = (y - owner->y) - 20.0f;
            dz = z - owner->z;
            state->yaw = Arctanf(dz, dx) + 0x4000;
            state->pitch = Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->roll = 0;
            break;
    }
}
