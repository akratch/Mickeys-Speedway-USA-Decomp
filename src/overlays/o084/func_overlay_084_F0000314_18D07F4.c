#include "PR/ultratypes.h"

typedef struct Overlay84UpdateChoice {
    u8 pad00[0x12];
    u8 first;
    u8 second;
} Overlay84UpdateChoice;

typedef struct Overlay84UpdateNode {
    s16 angle;
    u8 pad02[0x0A];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84UpdateChoice *choice;
} Overlay84UpdateNode;

typedef struct Overlay84UpdateState {
    u8 initialized;
    s8 current;
    s8 status;
    s8 actionTimer;
    s8 blendTimer;
    s8 action;
    s8 inputLatch;
    s8 inputTimer;
    u8 pad08;
    u8 mode;
    u8 pad0A[4];
    s16 angleOffset;
    s16 targetTilt;
    s16 tilt;
    s16 angle;
    s16 targetAngle;
    u32 flags;
    u8 pad1C[4];
    union {
        s32 word;
        struct {
            s16 upper;
            s16 base;
        } half;
    } outputAngle;
    f32 targetHeight;
    f32 height;
    f32 blend;
    f32 x;
    f32 y;
    f32 z;
    s32 first;
    s32 second;
    Overlay84UpdateNode *nodes[32];
    u8 padC4[2];
    u8 marked;
} Overlay84UpdateState;

typedef struct Overlay84UpdateScale {
    u8 pad00[0x0A];
    s16 radius;
} Overlay84UpdateScale;

typedef struct Overlay84UpdateObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84UpdateScale *scale;
} Overlay84UpdateObject;

typedef struct Overlay84Output {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x26];
    s16 verticalAngle;
} Overlay84Output;

extern u32 gOverlay84InputFlags;
extern s16 gOverlay84InputAxis;
extern f32 gOverlay84BlendStep;

extern void amSndPlay(u16 soundId, void **handle);
extern void overlay84AdvanceCurrent(s32 direction);
extern s32 mathDiffAngle(s32 current, s32 target);
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern void camSetNo(s32 camera);
extern Overlay84Output *camGetPtr(void);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern s32 Arctanf(f32 y, f32 x);
extern s16 dAngle(s16 current, s16 target, f32 fraction);

/* 2026-10-02: the goto/register-s16/volatile-scale spelling is retired.
 * The compare/adjust pair is plain if/else, the tilt easing is the float
 * expression the shipped code evaluates (tilt promoted, trunc back), and
 * angleStep is read after the loop uninitialised exactly as the shipped
 * code does. -Wab,-r4300_mul is applied for this object like its siblings.
 * Remaining residual: one extra temp-ring draw ahead of blendTimer, which
 * shifts every later $t register by one. */
#ifdef NON_MATCHING
void func_overlay_084_F0000314_18D07F4(Overlay84UpdateObject *object,
                                       Overlay84UpdateState *state,
                                       s32 updateRate) {
    Overlay84UpdateNode *node;
    s32 i;
    Overlay84UpdateChoice *choice;
    Overlay84Output *output;
    s32 angleStep;
    s32 pad;
    s32 angleFlag;
    s16 currentAngle;
    s32 targetAngle;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    Overlay84UpdateScale *scale;

    node = state->nodes[state->current];
    scale = object->scale;
    if (node != 0) {
        if (state->inputTimer > 0) {
            state->inputTimer -= updateRate;
        } else if (state->inputLatch == 0 &&
                   (gOverlay84InputFlags & 0x9000) != 0 &&
                   state->marked == 0) {
            amSndPlay(0xC, 0);
            state->inputLatch = 1;
        }

        if (state->inputLatch == 0) {
            if (state->actionTimer > 0) {
                state->actionTimer -= updateRate;
            } else if (gOverlay84InputAxis < -0x10 && state->marked == 0) {
                overlay84AdvanceCurrent(1);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                amSndPlay(0xF, 0);
            } else if (gOverlay84InputAxis >= 0x11 && state->marked == 0) {
                overlay84AdvanceCurrent(0);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                amSndPlay(0xF, 0);
            }
        }

        currentAngle = state->angle;
        targetAngle = state->targetAngle;
        angleFlag = state->flags & 1;
        if (angleFlag != 0) {
            if (angleFlag == 1) {
                if (currentAngle < targetAngle) {
                    targetAngle += 0xFFFF0000;
                }
            }
        } else {
            if (targetAngle < currentAngle) {
                targetAngle += 0x10000;
            }
        }

        for (i = 0; i < updateRate; i++) {
            state->tilt += (state->targetTilt - state->tilt) * 0.125f;
            state->height += (state->targetHeight - state->height) * 0.125f;
            angleStep =
                mathDiffAngle(currentAngle, targetAngle) >> 5;
            if (angleStep == 0) {
                currentAngle = targetAngle;
            } else {
                currentAngle += angleStep;
            }
        }

        if (angleStep >= -0x1F && angleStep < 0x20) {
            choice = node->choice;
            state->first = choice->first;
            state->second = choice->second;
        } else {
            state->first = 0xFF;
            state->second = 0xFF;
        }
        state->angle = currentAngle;

        if (state->blendTimer > 0) {
            state->blendTimer -= updateRate;
        } else {
            state->blend += gOverlay84BlendStep * updateRate;
            if (state->blend > 1.0f) {
                state->blend = 1.0f;
            }
        }

        baseX = func_8002A8C0(state->angle) * scale->radius + object->x;
        baseY = state->height + object->y;
        baseZ = func_8002A8BC(state->angle) * scale->radius + object->z;
        targetX = baseX + state->blend * (node->x - baseX);
        targetY = baseY + state->blend * (node->y - baseY);
        targetZ = baseZ + state->blend * (node->z - baseZ);

        for (i = 0; i < updateRate; i++) {
            state->x += (targetX - state->x) * 0.125f;
            state->y += (targetY - state->y) * 0.125f;
            state->z += (targetZ - state->z) * 0.125f;
        }

        camSetNo(0);
        output = camGetPtr();
        output->x = state->x;
        output->y = state->y;
        output->z = state->z;
        output->verticalAngle =
            func_8000FAE0(output->x, output->y, output->z);
        angleStep = -Arctanf(object->x - output->x,
                             object->z - output->z);
        angleStep = dAngle(angleStep, 0x8000 - node->angle, state->blend);
        state->outputAngle.word = dAngle(
            state->outputAngle.half.base, angleStep,
            1.0f - (f32)state->actionTimer / 20.0f);
        output->angle = state->outputAngle.word;
        output->roll = 0;
        output->tilt = state->tilt + state->angleOffset;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o084/func_overlay_084_F0000314_18D07F4/func_overlay_084_F0000314_18D07F4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_084_F0000314_18D07F4:start
 * symbol: func_overlay_084_F0000314_18D07F4
 * score: 37 differing words
 * frame: 0x70
 * relocations: 20
 * first-mismatch: +0x2A0
 * summary: 449 to 37 at delta 0: no goto/volatile, float tilt expr, one missing t-ring draw before blendTimer
 * PLATEAU-HANDOFF:func_overlay_084_F0000314_18D07F4:end
 */
