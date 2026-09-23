#include "overlays/overlay_008.h"

/* Overlay 008, ADR 0006 consolidation: functions remain in ROM order. */

void overlay8Ignore(volatile s32 unused) {
}

/* DKR v77/v80 and JFG checks found no exact donor for this indexed selector. */
void *overlay8GetIndexed(Overlay8IndexedObject *object) {
    s32 index = object->index;
    void *result;

    if (index < 0 || index >= 10) {
        index = 0;
    }
    if (gOverlay8IndexMode == 0) {
        result = gOverlay8Primary[index];
    } else {
        result = gOverlay8Secondary[index];
    }
    return result;
}

/* Workbench p6: structure-mismatch; 511/527 candidate/target instructions, 438 words from +0x2C.
 * A six-entry surface buffer and direct bound globals restore the exact 0xA0 frame and improve
 * relocation offset/type alignment from 23 to 32; pointer/index surface forms compile equivalently.
 * Remains: 16 missing instructions and relocation records plus unresolved global identities. */
#ifdef NON_MATCHING
void func_overlay_008_F0000058_185DDB0(O8P0058Owner *owner,
                                       s32 updateRate) {
    O8P0058State *state;
    f32 vector[3];
    s16 angles[3];
    O8P0058Surface surfaces[6];
    O8P0058Surface *surface;
    f32 floorHeight;
    f32 update;
    f32 value;
    s32 count;
    s32 index;
    s32 present;
    O8P0058Query *query;

    state = owner->state64;
    gOverlay8Buffer = (s16 *)((u8 *)state + 0x1B8);
    gOverlay8Value = 0;
    D_14 = 0;
    D_18 = 0;
    owner->flags80 = 0;
    D_10 = 25.0f;
    state->value70 = 0.0f;

    if ((state->disabled18D != 0) || (state->gate158 != 0) ||
        (state->reset170 != 0) || (owner->mode3B == 0x18) ||
        (state->gate3FA != 0)) {
        o8P0058ResetReloc(state, 1);
    }
    o8P0058ModeReloc(state, state->mode0);

    if (gO8P0058MirrorGateReloc != 0) {
        state->signed428 = -state->signed428;
        state->signed430 = -state->signed430;
    }

    query = o8P0058AcquireReloc(state);
    present = gO8P0058PresentReloc;
    gO8P0058ResultReloc = query->initial0;
    if (present != 0) {
        if (((state->modeFlags420 & 0x8000) != 0) &&
            (state->active183 == 0x80)) {
            state->active183 = (u8)gO8P0058ActiveReloc;
        }
        state->value42C = 0;
        state->value434 = 0;
        state->flags41C = (state->flags41C & 0x8008) | 0x4000;
    } else {
        if ((state->active183 & 0x80) != 0) {
            if (state->active183 == 0x84) {
                if (gO8P0058SpawnGateReloc != 0) {
                    o8P0058SpawnReloc(owner, 0x18, -1, 0);
                }
            } else if (state->active183 != 0x80) {
                state->alternate184 = 1;
            }
            state->active183 = 0;
        }
    }

    if (state->lower4 < D_B0) {
        state->lower4 = D_B0;
    }
    if (D_B4 < state->lower4) {
        state->lower4 = D_B4;
    }
    if (state->upper8 < D_B0) {
        state->upper8 = D_B0;
    }
    if (D_B4 < state->upper8) {
        state->upper8 = D_B4;
    }

    o8P0058OrientReloc(owner, state);
    angles[0] = -state->angleF0;
    angles[1] = -owner->angle2;
    angles[2] = -owner->angle4;
    vector[0] = 0.0f;
    vector[1] = -1.0f;
    vector[2] = 0.0f;
    o8P0058RotateReloc(angles, vector);
    state->direction60 = vector[0];
    state->direction64 = vector[1];
    state->direction5C = vector[2];

    count = o8P0058SurfaceReloc(owner->xC, owner->z14, 0, 0x08010000,
                                surfaces);
    floorHeight = -32768.0f;
    state->surface68 = -32768.0f;
    if (count != 0) {
        index = count - 1;
        do {
            surface = &surfaces[index];
            if ((surface->flags4 & 0x10000) != 0) {
                state->surface68 = surface->height0;
            }
            if ((surface->flags4 & 0x08000000) != 0) {
                floorHeight = surface->height0;
            }
        } while (index-- != 0);
    }

    if (owner->y10 < state->surface68) {
        state->surfaceActive2 = 1;
        state->surface6C = state->surface68;
        if ((state->surface68 - owner->y10) >= 25.0f) {
            gO8P0058ResultReloc *= 0.125f;
        }
    } else {
        state->surfaceActive2 = 0;
        state->surface6C = 0.0f;
    }

    if (owner->y10 < floorHeight) {
        state->reset170 = 1;
        if ((state->surfaceActive2 != 0) && (state->surfaceMode3 != 1)) {
            o8P0058CollisionReloc(owner, state);
        }
    }

    if (state->active16A != 0) {
        D_8 = 1.0f;
        D_C = 1.0f;
        o8P0058EffectReloc(state, 0x28, 0.15f, &D_C);
    } else {
        D_8 = 0.0f;
        D_C = 0.0f;
        index = state->selectors320[0] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[1] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[2] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        index = state->selectors320[3] & 0xF;
        D_8 += D_310[index];
        D_C += query->heights148[D_350[index]];
        D_8 *= 0.25f;
        D_C *= 0.25f;

        if (state->override172 != 0) {
            D_C = state->override17C;
        }
        if (state->surfaceActive2 != 0) {
            D_C = D_B8;
        }
        if ((state->peerD4 != 0) && (D_BC < D_8)) {
            D_8 += (D_BC - D_8) * state->peerD4->state64->blend14;
        }
    }

    if ((state->mode16C == 0) || (state->mode16C == 1)) {
        update = (f32)updateRate;
        func_overlay_008_F0001294_185EFEC(owner, state, update);
    } else {
        update = (f32)updateRate;
    }
    func_overlay_008_F000291C_1860674((O8P291CMotion *)owner,
                                      (O8P291CState *)state, update);
    state->value70 =
        o8P0058SampleReloc(owner, state, D_10, update);

    *gOverlay8Buffer = 0x2000;
    gOverlay8Buffer++;
    o8P0058UpdateReloc(owner, state, updateRate);

    if (state->lowering349 != 0) {
        if (state->mode16C == 1) {
            state->mode16C = 0;
            state->counter16E = 8;
            if (state->resourceB8 != 0) {
                o8P0058ReleaseReloc(state->resourceB8);
            }
            o8P0058CreateReloc(6, owner->xC, owner->y10, owner->z14, 4,
                               &state->resourceB8);
            value = D_C0;
            state->bounceActive18C = 1;
            state->bounce54 = value + 1.0f;
            state->bounceVelocity3FC = value;
            o8P0058BounceReloc(state, 0x32, 0.4f);
        } else if (state->counter16E > 0) {
            state->counter16E -= updateRate;
        }
    } else {
        state->counter16E = 0;
    }

    if (state->disabled18D == 0) {
        if ((state->bounceActive18C != 0) &&
            (state->bounce54 == state->position50)) {
            state->bounceVelocity3FC *= -0.5f;
            value = state->bounceVelocity3FC;
            if ((D_C4 < value) && (value < D_C8)) {
                state->bounceActive18C = 0;
                state->bounceVelocity3FC = 0.0f;
                state->bounce54 = 1.0f;
                return;
            }
            state->bounce54 = value + 1.0f;
        }
    } else {
        state->bounceActive18C = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0000058_185DDB0.s")
#endif

void func_overlay_008_F0000894_185E5EC(O8Owner *owner, O8State *state,
                                       s32 updateRate) {
    void *savedResource;
    O8Node *node;
    f32 update = (f32)updateRate;
    f32 sine;
    f32 cosine;
    f32 y;
    f32 x;
    O8Node *savedNode;
    f32 sideA;
    f32 sideB;
    f32 z;
    s32 effect;
    s32 angleA;
    s32 angleB;

    node = *owner->node68;
    savedResource = node->resource;
    if (((s8 *)owner->children40)[owner->childIndex93 + 0x1e] == 0 &&
        (node != 0) && (node->active != 0)) {
        savedNode = node;
        o8Call0894Reloc(node, savedResource, owner);
        ext_o0_19668(owner, savedNode, owner->value50,
                     savedNode->items[savedNode->index]);
        ext_o8_3368(owner, state, savedResource, node, node->active);
        savedNode->active = 0;
    }

    effect = -1;
    if ((state->active181 != 0) && (owner->position28 <= gO8FloatCC)) {
        if (owner->mode3B == 0x14) effect = 0;
        else if (owner->mode3B == 0x13) effect = 1;
        else if (owner->mode3B == 0x15) effect = 2;
        if (effect != -1) {
            f32 scale = state->value84 * update;
            sideA = state->vector74 * scale;
            sideB = state->vector78 * scale;
            z = state->vector7C * scale;
            angleA = 0x1200;
            angleB = 0x1c00;
        }
        ext_o0_1eed0_target(state, 0x4b, 0x3e19999a);
    }

    if ((state->lateral4 < -3.25f) && (effect == -1)) {
        if ((state->mode34A & 3) == 3) {
            effect = 1;
            y = 0.0f;
            x = 4.0f - state->lateral4 * gO8FloatD0;
        } else if (state->mode34A & 5) {
            effect = 0;
            y = 4.0f - state->lateral4 * gO8FloatD4;
            x = 0.0f;
        } else if (state->mode34A & 0xa) {
            effect = 2;
            y = state->lateral4 * gO8FloatD8 + -4.0f;
            x = 0.0f;
        }
        if (effect != -1) {
            sine = ext_o0_2a46c(state->angle43C);
            cosine = ext_o0_2a470(state->angle43C);
            angleA = 0x2a00;
            angleB = 0x3600;
            sideA = y * sine + x * cosine;
            sideB = 0.0f;
            z = x * sine - y * cosine;
            ext_o0_1eed0_target(state, 0x28, 0x3e19999a);
        }
    }
    if (effect != -1) {
        o8Call0894EmitReloc(owner, state, effect, angleA, angleB,
                                          sideA, sideB, z, 2);
        state->timer3B3 = 0x64;
    }

    ext_o8_3278(owner, state, updateRate);
    ext_o8_2ec0(owner, state, 0, 0, updateRate);
    ext_o0_1d510(owner, state, updateRate);
    if ((state->active181 != 0) &&
        (((state->mode34A != 0) && (state->value84 == state->value80)) ||
         (owner->peer48->gate63 != 0))) {
        if (state->resourceB8 != 0) ext_o0_2d98(state->resourceB8);
        ext_o0_2b90(7, owner->valueC, owner->value10, owner->value14, 4,
                    &state->resourceB8);
        ext_o7_ccc(owner, 0x12);
    }
    ext_o8_3018(owner, state, state->value70, updateRate);

    if ((state->condition172 != 0) && (state->lateral4 < -2.0f)) {
        owner->flags80 &= ~0x33;
        owner->flags80 |= gO8Value370;
        owner->flags80 |= gO8Value3B0;
    } else if (((state->condition2 != 0) || (state->conditionD4 != 0)) &&
               ((state->lateral4 < gO8FloatDC) ||
                (state->lateral4 > gO8FloatE0))) {
        owner->flags80 &= ~0x33;
        owner->flags80 |= gO8Value364;
        owner->flags80 |= gO8Value3A4;
    } else if (state->lateral4 < -5.0f) {
        owner->flags80 |= gO8Table360[state->selector322 & 0xf];
        owner->flags80 |= gO8Table3A0[state->selector323 & 0xf];
    }

    if (owner->flags80 & 0x10) owner->flags80 &= ~1;
    if (owner->flags80 & 0x20) owner->flags80 &= ~2;
    if (state->angle106 >= 0x1b)
        ext_o0_3e990((f32)((0x5a - (s32)state->angle106) << 2));
    ext_o0_3e99c(owner, updateRate);
    if (state->resource134 != 0) ext_o17_668(state->resource134, gO8Pointer14);
    if (state->resource138 != 0) ext_o17_668(state->resource138, gO8Pointer18);
    if (state->resourceC4 != 0)
        ext_o0_2d70(state->resourceC4, owner->valueC, owner->value10,
                    owner->value14);
}

/* Mickey-local reconstruction; the donor scans found no exact donor. */
s32 func_overlay_008_F0000E88_185EBE0(void *peer,
                                      Overlay8MotionRecord *record) {
    s16 direction;

    if ((record->gate16A == 0) &&
        (record->activeDirection == 0) &&
        (record->gate168 == 0)) {
        direction = record->direction;
        if (direction != 0) {
            record->activeDirection = -direction;
        } else if (record->fallbackSign < 0) {
            record->activeDirection = -1;
        } else {
            record->activeDirection = 1;
        }

        if (record->resource != NULL) {
            o8StartMotionResourceReloc(record->resource);
        }
        return 1;
    }
    return 0;
}

void func_overlay_008_F0000F1C_185EC74(Overlay8ActivationOwner *owner,
                                       s32 force) {
    Overlay8ActivationState *state;

    state = owner->state;
    if ((state->activeDirection != 0) || (state->gate158 != 0)) {
        return;
    }
    if ((force == 0) && (state->active185 == 1)) {
        return;
    }

    state->active183 = 1;
    state->active185 = 1;
    state->timer187 = 0x1E;

    if (((state->flags1A8 & 1) == 0) || (state->type == 0) ||
        (gOverlay8ActivationGateTimerReloc == 0)) {
        if (state->resource != NULL) {
            overlay8ReleaseResourceReloc(state->resource);
        }
        overlay8CreateResourceReloc(8, owner->valueC, owner->value10,
                                    owner->value14, 4, &state->resource);
        gOverlay8ActivationGateTimerReloc = 0x3C;
    }

    overlay8FinalizeActivationReloc(owner, 0x17);
}

/* Matched 2026-09-23 (lane A2-ov), 165/165 words at frame 0x18, unforced.
 * The empty `if (unused)` closes the unused-a0 home (lane w32-o008) and the
 * flags-before-countdown order in case 2 closes the t6/t7/t8 ring phase
 * (lane B-ovsmall). The rollover default 120 is the else arm of the control
 * test, not an initialiser: the shipped b is the control arm's jump over that
 * else, which ugen emits and as1 then empties by hoisting its 120 above the
 * test, and the clamp itself has no else. The sample is assigned straight to
 * the count, so the only copy is the mfc1 result's, which as1 folds into v1.
 * An inner else arm copying a separate sample kept the two in interfering
 * webs instead (count v0, sample v1, and a surviving move). */
f32 func_overlay_008_F0001000_185ED58(void *unused, O8PhaseState *state, f32 input) {
    s32 nextCountdown;

    if (unused) {
    }
    if (state->timer > 0) {
        if ((state->phase != 1) && (state->phase != 2)) {
            state->phase = 2;
            state->countdown = 0;
        }
    }

    switch (state->phase) {
    case 1:
        state->weight += (1.0f - state->weight) * 0.875f;
        state->flags |= 0x8000;
        state->effect = 3;
        state->countdown--;

        if (state->countdown == 0) {
            if (gO8RolloverControlReloc != 0) {
                nextCountdown = (s32)(o8RolloverSampleReloc() * 6.0f + 60.0f);
                if (nextCountdown >= 181) {
                    nextCountdown = 180;
                }
            } else {
                nextCountdown = 120;
            }
            state->phase = 2;
            state->countdown = (u8)nextCountdown;
            state->weight = 1.0f;
        }

        o8Phase1EmitReloc(state, 75, 0.5f);
        break;

    case 2:
        state->effect = 3;
        if (state->countdown != 0) {
            state->flags |= 0x8000;
            state->countdown--;
        } else if (state->timer > 0) {
            state->weight +=
                (gO8Phase2TargetReloc - state->weight) * 0.875f;
        } else {
            state->phase = 3;
        }

        if (gO8Phase2ScaleControlReloc == 0) {
            input += gO8Phase2ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    case 3:
        state->effect = 0;
        state->weight *= gO8Phase3DecayReloc;
        if (state->weight < gO8RetireThresholdReloc) {
            state->phase = 0;
            state->weight = 0.0f;
        }

        if (gO8Phase3ScaleControlReloc == 0) {
            input += gO8Phase3ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    default:
        state->effect = 0;
        break;
    }

    if (state->forceEffect != 0) {
        state->effect |= 3;
    }
    return input;
}

struct O8P1294Owner {
    u8 pad00[0x0C];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[8];
    f32 unk20;
    u8 pad24[4];
    f32 unk28;
    u8 pad2C[0x0F];
    s8 unk3B;
    u8 pad3C[0x44];
    s32 unk80;
};

struct O8P1294State {
    u8 pad000[2];
    u8 unk2;
    u8 pad003;
    f32 unk4;
    f32 unk8;
    u8 pad00C[0x50];
    f32 unk5C;
    u8 pad060[0x42];
    s16 unkA2;
    u8 pad0A4[4];
    void *unkA8;
    void *unkAC;
    u8 pad0B0[8];
    void *unkB8;
    u8 pad0BC[0x18];
    O8P0058Peer *unkD4;
    u8 pad0D8[8];
    f32 unkE0;
    f32 unkE4;
    u8 pad0E8[8];
    s16 unkF0;
    u8 pad0F2[0x0A];
    s16 unkFC;
    s16 unkFE;
    s16 unk100;
    s16 unk102;
    s16 unk104;
    s16 unk106;
    s16 unk108;
    u8 pad10A[0x2A];
    void *unk134;
    void *unk138;
    u8 pad13C[8];
    s16 unk144;
    s16 unk146;
    f32 unk148;
    u8 pad14C[0x0C];
    s16 unk158;
    u8 pad15A[0x14];
    s8 unk16E;
    u8 pad16F[0x13];
    u8 unk182;
    u8 pad183;
    u8 unk184;
    u8 unk185;
    u8 pad186[2];
    f32 unk188;
    u8 pad18C;
    s8 unk18D;
    u8 pad18E[4];
    u8 unk192;
    u8 pad193[5];
    u8 unk198;
    u8 pad199[0x189];
    u8 unk322;
    u8 unk323;
    u8 pad324[0x25];
    u8 unk349;
    u8 pad34A[0xD2];
    u32 unk41C;
    u32 unk420;
    u8 pad424[4];
    s32 unk428;
    s32 unk42C;
};

typedef struct O8P1294ColorTarget {
    u8 pad00[0x24];
    u8 color24[4];
} O8P1294ColorTarget;

extern s16 D_4;
extern f32 D_F8, D_FC, D_100, D_104, D_108, D_10C, D_110, D_114;
extern f32 D_118, D_11C, D_120, D_124, D_128, D_12C, D_130, D_134;
extern f32 D_138, D_13C, D_140, D_144, D_148, D_14C, D_150, D_154;
extern f32 D_158, D_15C, D_160, D_164, D_168, D_16C, D_170, D_174;
extern f32 D_178, D_17C, D_180, D_184, D_188, D_18C, D_190, D_194;
extern s32 D_3E0[], D_420[], D_460[], D_4A0[];
extern u8 D_4E0[];
extern f32 gO8P1294MotionScalarReloc;
extern u8 gO8P1294ImpactGateReloc;
extern u8 gO8P1294ColorGateReloc;
extern void controlSetRumble(void *player, s32 strength, f32 duration);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void func_800031E8(void *handle);
extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern void func_800031C0(void *handle, f32 x, f32 y, f32 z);
extern s32 func_8002A204(s16 angle);
extern s32 mathDiffAngle(s32 current, s32 target);

/* NON_MATCHING reconstruction: the configured full TU has the exact 1259-word
 * size, 636 masked differences and a 0xC8 frame versus the target's 0xB0.
 * The update loop tests the old counter; the braking global is a halfword;
 * mathDiffAngle accepts the full requested angle. Paired color-enable homes
 * and distinct raw/clamped steering values retain the observed dataflow. */
#ifdef NON_MATCHING
void func_overlay_008_F0001294_185EFEC(O8P1294Owner *owner,
                                       O8P1294State *state, f32 update) {
    s32 updatesRemaining;
    s32 updateCount;
    s32 impactBoost;
    s32 colorEnabled[2];
    f32 value;
    f32 speed;
    f32 speedLimit;
    f32 scale;
    s16 driftDirection;
    s16 cooldown;
    s32 steeringAngle;
    s32 steeringInput;
    s32 clampedSteering;
    s32 inputFlags;
    s32 angleStep;
    s32 targetAngle;
    s32 braking;
    s32 leftSelector;
    s32 rightSelector;
    s32 turnAmount;
    s32 effectMask;
    s32 applyReverseMotion;
    s32 turnDirection;
    s32 driftStep;
    s8 animation;
    u8 modeFlags;
    s32 index;
    O8P0058Peer *peer;
    f32 *tuning;
    O8P1294ColorTarget *colorTarget;
    u8 *color;
    f32 *curve;

    tuning = overlay8GetIndexed((Overlay8IndexedObject *)state);
    impactBoost = 0;
    index = state->unk192;
    effectMask = 0;
    if ((s32) index >= 0xB) {
        index = 0xA;
    }
    D_10 = tuning[0x40 / sizeof(f32)] + ((f32) index * tuning[0x8 / sizeof(f32)]);
    peer = state->unkD4;
    if (peer != NULL) {
        D_10 *= 1.0f + (D_F8 * peer->state64->blend14);
    }
    if (state->unk185 == 0) {
        value = state->unk5C;
        if (value != 0.0f) {
            scale = 1.0f -
                (value * 0.5f * tuning[0xC / sizeof(f32)]);
            if (scale < D_FC) {
                scale = D_100;
            }
            D_10 *= scale;
        }
    }
    driftDirection = state->unk102;
    if (driftDirection != 0) {
        animation = owner->unk3B;
        if ((animation == 0x10) || (animation == 0xF)) {
            scale = owner->unk28 * 1.5f;
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            if (driftDirection > 0) {
                scale = -scale;
            }
            state->unk104 = (s16) (s32) (65536.0f * scale);
            if (owner->unk28 == 1.0f) {
                state->unk102 = 0;
                state->unk104 = 0;
            } else {
                state->unk185 = 0U;
                state->unk188 = 0.0f;
            }
        }
        controlSetRumble(state, 0x32, 0.15f);
    }
    modeFlags = state->unk184;
    if ((modeFlags != 0) && (((s32) modeFlags >= 2) ||
        (inputFlags = state->unk41C, ((inputFlags & 0x4000) != 0)) ||
        !(inputFlags & 0x8000))) {
        state->unk184 = 0U;
    }
    updateCount = (s32) update;
    updatesRemaining = updateCount - 1;
    if (updateCount != 0) {
        do {
            value = func_overlay_008_F0001000_185ED58(
                owner, (O8PhaseState *)state, D_10);
            speedLimit = value;
            if (D_104 < value) {
                speedLimit = D_108;
            }
            inputFlags = state->unk41C;
            braking = inputFlags & 0x4000;
            if ((braking == 0) && (state->unk5C > 0.0f) && (state->unk4 < -D_10)) {
                applyReverseMotion = 1;
            } else if ((braking == 0) && (state->unk5C < 0.0f)) {
                applyReverseMotion = 1;
            } else if (!(inputFlags & 0xC000) && (state->unk5C > 0.0f)) {
                applyReverseMotion = 1;
            } else {
                applyReverseMotion = 0;
            }
            if ((applyReverseMotion != 0) && (state->unk185 == 0)) {
                state->unk4 = (f32) (state->unk4 +
                    (gO8P1294MotionScalarReloc * state->unk5C));
                value = tuning[0x1C / sizeof(f32)];
                if (value < state->unk4) {
                    state->unk4 = value;
                }
            }
            if (state->unk102 != 0) {
                value = D_10C;
                state->unk428 = 0;
                state->unk42C = 0;
                state->unk41C = 0;
                state->unk420 = 0;
                state->unk4 = (f32) (state->unk4 * value);
                state->unk8 = (f32) (state->unk8 * value);
            }
            value = state->unk148 - state->unk4;
            scale = value * D_110;
            if (scale < D_114) {
                scale = D_118;
            }
            if (D_11C < scale) {
                scale = D_120;
            }
            steeringAngle = state->unk144;
            state->unk144 = (s16) (steeringAngle + ((s32) ((s32) scale - steeringAngle) >> 3));
            scale = value * D_124;
            if (scale < D_128) {
                scale = D_12C;
            }
            if (D_130 < scale) {
                scale = D_134;
            }
            steeringAngle = state->unk146;
            inputFlags = state->unk41C;
            state->unk148 = state->unk4;
            braking = inputFlags & 0x4000;
            state->unk146 = (s16) (steeringAngle + ((s32) ((s32) scale - steeringAngle) >> 3));
            if ((braking != 0) && (inputFlags & 0x10) && ((steeringInput = state->unk428, ((steeringInput < -0x1E) != 0)) || (steeringInput >= 0x1F))) {
                if (state->unk4 < -5.0f) {
                    state->unk4 = (f32) (state->unk4 + D_138);
                    if (state->unk4 > -5.0f) {
                        state->unk4 = -5.0f;
                    }
                } else {
                    state->unk4 = (f32) (state->unk4 - D_13C);
                    if (state->unk4 < -5.0f) {
                        state->unk4 = -5.0f;
                    }
                }
                D_4 = 1;
                value = state->unkE4;
                effectMask |= 0x3C;
                if (value < 1.0f) {
                    state->unkE4 = (f32) (value + D_140);
                }
            } else if (braking != 0) {
                if (state->unk4 < -5.0f) {
                    D_4 = 1;
                    effectMask |= 0x3C;
                }
                if (state->unk4 < 0.0f) {
                    state->unk4 = (f32) (state->unk4 + tuning[0xC8 / sizeof(f32) + (s32)-state->unk4]);
                    if ((state->unk4 > 0.0f) && (state->unk42C >= -0x1E)) {
                        goto block_74;
                    }
                } else if (state->unk42C < -0x1E) {
                    state->unk4 = (f32) (state->unk4 + tuning[0x20 / sizeof(f32) + (s32)state->unk4]);
                    value = tuning[0x1C / sizeof(f32)];
                    if (value < state->unk4) {
                        state->unk4 = value;
                    }
                } else {
                    state->unk4 = (f32) (state->unk4 - tuning[0xC8 / sizeof(f32)]);
                    if (state->unk4 <= 0.0f) {
block_74:
                        state->unk4 = 0.0f;
                    }
                }
                value = state->unkE4;
                if (value < 1.0f) {
                    state->unkE4 = (f32) (value + D_144);
                }
            } else if (inputFlags & 0x8000) {
                if ((state->unk184 != 0) ||
                    (modeFlags = state->unk185, (modeFlags == 1)) ||
                    (modeFlags == 2)) {
                    impactBoost = 1;
                    if (gO8P1294ImpactGateReloc == 0) {
                        scale = D_148;
                    } else {
                        scale = 0.5f;
                    }
                    controlSetRumble(state, 0x32, 0.15f);

                } else {
                    if (state->unk4 > 0.0f) {
                        curve = &tuning[0xC8 / sizeof(f32)];
                        index = (s32) state->unk4;
                        scale = state->unk4 - (f32) index;
                    } else {
                        value = -state->unk4;
                        curve = &tuning[0x44 / sizeof(f32)];
                        index = (s32) value;
                        scale = value - (f32) index;
                    }
                    curve = &curve[index];
                    value = curve[0];
                    scale = ((curve[1] - value) * scale) + value;
                }
                value = -speedLimit;
                if (state->unk4 < value) {
                    state->unk4 = (f32) (state->unk4 * D_14C);
                    if (value < state->unk4) {
                        state->unk4 = value;
                    }
                    if (state->unk184 != 0) {
                        state->unk184 = 2U;
                    }
                } else {
                    state->unk4 = (f32) (state->unk4 - scale);
                    if (state->unk4 < value) {
                        state->unk4 = value;
                        if (state->unk184 != 0) {
                            state->unk184 = 2U;
                        }
                    } else if (D_150 <= scale) {
                        if ((impactBoost == 0) || (state->unk184 != 0)) {
                            effectMask |= 0xC;
                        }
                        effectMask |= 0x30;
                    }
                }
                value = state->unkE4;
                if (D_154 <= value) {
                    state->unkE4 = (f32) (value - D_158);
                }
            } else {
                value = tuning[0xC4 / sizeof(f32)];
                if ((-value < state->unk4) &&
                    (state->unk4 < value)) {
                    state->unk4 = 0.0f;
                } else {
                    state->unk4 = (f32) (state->unk4 * D_15C);
                }
            }
            if ((state->unk18D == 0) && (state->unk158 == 0) && (state->unk349 != 0)) {
                speed = state->unk4;
                if ((D_160 < speed) && (speed < D_164) &&
                    (func_800299E8(0, 0x7F) >= 0x73)) {
                    owner->unk80 = (s32) (owner->unk80 | 0xC);
                }
            }
            cooldown = state->unkA2;
            if (cooldown != 0) {
                state->unkA2 = (s16) (cooldown - 1);
            }
            if ((state->unk41C & 0x10) && (state->unk100 == 0) && (state->unk4 < -3.0f)) {
                if (state->unk428 < -0xF) {
                    state->unk100 = -1;
                }
                if (state->unk428 >= 0x10) {
                    state->unk100 = 1;
                }
                if (state->unk100 != 0) {
                    if (state->unkA2 == 0) {
                        owner->unk20 = (f32) (owner->unk20 + 3.0f);
                        state->unkA2 = 0xF;
                    }
                    if (state->unkA8 != NULL) {
                        func_800031E8(state->unkA8);
                    }
                    func_80002FE0(5, owner->unkC, owner->unk10,
                                  owner->unk14, 4, &state->unkA8);
                }
            }
            if (state->unk4 > -2.0f) {
                state->unk100 = 0;
            }
            scale = state->unk4;
            steeringInput = state->unk428;
            if (state->unk4 < 0.0f) {
                scale = -scale;
            }
            if (scale <= D_168) {
                scale = 0.0f;
            } else {
                scale = scale - D_16C;
                if (D_170 < scale) {
                    scale = D_174;
                }
                if (state->unk100 != 0) {
                    scale = (scale * 68.0f * 60.0f) / tuning[0x10 / sizeof(f32)];
                } else {
                    scale = (scale * 58.0f * 60.0f) / tuning[0x10 / sizeof(f32)];
                }
                if (state->unk4 > 0.0f) {
                    scale = -scale;
                }
            }

            clampedSteering = steeringInput;
            if ((state->unk41C & 0x10) && (driftDirection = state->unk100, turnAmount = 0x2710, (driftDirection != 0))) {
                if (steeringInput >= 0x33) {
                    clampedSteering = 0x32;
                } else if (clampedSteering < -0x32) {
                    clampedSteering = -0x32;
                }
                if (((clampedSteering > 0) && (driftDirection > 0)) || ((clampedSteering < 0) && (driftDirection < 0))) {
                    clampedSteering = clampedSteering >> 2;
                }
                clampedSteering += driftDirection * 0x3C;
            } else {
                state->unk100 = 0;
                if (clampedSteering >= 0x3D) {
                    clampedSteering = 0x3C;
                } else if (clampedSteering < -0x3C) {
                    clampedSteering = -0x3C;
                }
                value = tuning[0x14 / sizeof(f32)];
                turnAmount = (s32) value;
            }
            steeringAngle = state->unk108;
            targetAngle = (s32) (((f32) -clampedSteering * tuning[0x10 / sizeof(f32)]) / 60.0f);
            angleStep = (s32) ((f32) (targetAngle - steeringAngle) * D_178);
            if (angleStep != 0) {
                state->unk108 = (s16) (steeringAngle + angleStep);
            } else {
                state->unk108 = (s16) targetAngle;
            }
            speed = state->unk4;
            if ((speed > -10.0f) && (speed < 10.0f)) {
                if (speed < 0.0f) {
                    value = speed * D_17C;
                } else {
                    value = speed * D_180;
                }
                value = ((D_C - 1.0f) * value) + 1.0f;
            } else {
                value = D_C;
            }
            steeringAngle = state->unk108;
            state->unk4 = (f32) (speed * value);
            if ((steeringAngle < -turnAmount) || (turnAmount < steeringAngle)) {
                state->unk4 = (f32) (state->unk4 * tuning[0x18 / sizeof(f32)]);
            }
            state->unk182 = (u8) ((state->unk182 + 1) & 0xF);
            turnAmount = (s32) ((f32) state->unk108 * scale);
            if (turnAmount != 0) {
                driftDirection = state->unk100;
                turnDirection = 0;
                if ((driftDirection < 0) || (turnAmount < -0x1400)) {
                    turnDirection = -1;
                } else if ((driftDirection > 0) || (turnAmount >= 0x1401)) {
                    turnDirection = 1;
                }
                if (turnDirection != 0) {
                    steeringInput = (func_8002A204((s16)(state->unk182 << 12)) *
                               ((state->unk106 * 8) + 0x200)) >> 16;

                    if (turnDirection < 0) {
                        steeringInput = -steeringInput;
                    }
                    turnAmount += steeringInput;
                }
            }
            angleStep = mathDiffAngle(state->unkFC, turnAmount) >> 2;

            turnAmount = angleStep;
            if (angleStep < -0x2EE) {
                turnAmount = -0x2EE;
            } else if (angleStep >= 0x2EF) {
                turnAmount = 0x2EE;
            }
            driftDirection = state->unk100;
            steeringAngle = state->unkFE;
            targetAngle = (driftDirection << 0xD) - steeringAngle;
            driftStep = targetAngle >> 4;
            state->unkFC = (s16) (state->unkFC + turnAmount);
            if (driftStep == 0) {
                driftStep = targetAngle;
            }
            state->unkFE = (s16) (steeringAngle + driftStep);
            if (driftDirection != 0) {
                steeringInput = state->unk428;
                if (((steeringInput >= 0x1A) && (driftDirection < 0)) || ((steeringInput < -0x19) && (driftDirection > 0))) {
                    state->unk106 = (s16) (state->unk106 + 1);
                } else {
                    state->unk106 = 0;
                }
                if (state->unk106 >= 0x3D) {
                    controlSetRumble(state, 0x28, 0.15f);
                }
                if (state->unk106 >= 0x5B) {
                    driftDirection = state->unk100;
                    state->unk100 = 0;
                    state->unk102 = (s16) -driftDirection;
                }
            } else {
                state->unk106 = 0;
            }
            speed = state->unk4;
            scale = speed;
            if (speed < 0.0f) {
                scale = -scale;
            }
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            if (speed > 0.0f) {
                scale = -scale;
            }
            if (state->unk41C & 0x4000) {
                if (state->unk100 != 0) {
                    scale *= 3.0f;
                } else {
                    scale *= 1.0f + state->unkE4;
                }
            }
            value = state->unkE0;
            state->unkF0 = (s16) (state->unkF0 + (s32) ((f32) state->unk108 * (scale * D_8)));
            state->unkE0 = (f32) (value + (((D_184 * speed) - value) * D_188));
            state->unk8 = (f32) (state->unk8 * D_18C);
            speed = state->unk8;
            if ((D_190 < speed) && (speed < D_194)) {
                state->unk8 = 0.0f;
            }
            updateCount = updatesRemaining;
            updatesRemaining = updateCount - 1;
        } while (updateCount != 0);
    }
    if ((state->unk41C & 0x4000) && (state->unk420 & 0x8000) && (state->unk4 == 0.0f)) {
        if (state->unkB8 != NULL) {
            func_800031E8(state->unkB8);
        }
        func_80002FE0(2, owner->unkC, owner->unk10, owner->unk14, 4,
                      &state->unkB8);
    }
    if ((state->unk349 != 0) && ((state->unk16E > 0) || (state->unk100 != 0) || (state->unk102 != 0) || (effectMask & 0x30) || (owner->unk3B == 0x18) || ((state->unk41C & 0x4000) && (state->unk4 < 0.0f)))) {
        if (state->unkAC == NULL) {
            func_80002FE0(3, owner->unkC, owner->unk10, owner->unk14, 1,
                          &state->unkAC);
        } else {
            func_800031C0(state->unkAC, owner->unkC, owner->unk10,
                          owner->unk14);
        }
    } else {
        if (state->unkAC != NULL) {
            func_800031E8(state->unkAC);
        }
    }
    leftSelector = 1;
    if ((state->unk2 != 0) || (state->unkD4 != NULL)) {
        rightSelector = 1;
    } else {
        leftSelector = state->unk322 & 0xF;
        rightSelector = state->unk323 & 0xF;
    }
    if (state->unk102 != 0) {
        effectMask |= 0x30;
    } else if ((state->unk100 != 0) && (state->unk41C & 0x8000)) {
        modeFlags = state->unk349;
        if (modeFlags & 4) {
            effectMask |= 0x11;
        }
        if (modeFlags & 8) {
            effectMask |= 0x22;
        }
    } else if (state->unk16E > 0) {
        effectMask |= 0x3C;
    } else if ((state->unk198 != 0) && (state->unk41C & 0x8000)) {
        effectMask |= 0x33;
    } else if (owner->unk3B == 0x18) {
        effectMask |= 3;
    }
    if (effectMask & 1) {
        owner->unk80 = (s32) (owner->unk80 | D_460[leftSelector]);
    }
    if (effectMask & 2) {
        owner->unk80 = (s32) (owner->unk80 | D_4A0[rightSelector]);
    }
    if (effectMask & 4) {
        owner->unk80 = (s32) (owner->unk80 | D_3E0[leftSelector]);
    }
    if (effectMask & 8) {
        owner->unk80 = (s32) (owner->unk80 | D_420[rightSelector]);
    }
    colorEnabled[0] = 0;
    colorEnabled[1] = 0;
    if ((state->unk2 == 0) && (gO8P1294ColorGateReloc == 0)) {
        if (effectMask & 0x10) {
            colorEnabled[0] = 1;
        }
        if (effectMask & 0x20) {
            colorEnabled[1] = 1;
        }
    }
    colorTarget = state->unk134;
    if ((colorTarget != NULL) && (colorEnabled[0] != 0) && (state->unk349 & 4)) {
        color = (leftSelector * 4) + D_4E0;
        if (color[3] != 0) {
            colorTarget->color24[0] = (u8) color[0];
            colorTarget->color24[1] = (u8) color[1];
            colorTarget->color24[2] = (u8) color[2];
            colorTarget->color24[3] = (u8) color[3];
            D_14 = 1;
        }
    }
    colorTarget = state->unk138;
    if ((colorTarget != NULL) && (colorEnabled[1] != 0) && (state->unk349 & 8)) {
        color = (rightSelector * 4) + D_4E0;
        if (color[3] != 0) {
            colorTarget->color24[0] = (u8) color[0];
            colorTarget->color24[1] = (u8) color[1];
            colorTarget->color24[2] = (u8) color[2];
            colorTarget->color24[3] = (u8) color[3];
            D_18 = 1;
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0001294_185EFEC.s")
#endif

/* 37/183 differing words, exact 183 instructions, 0xD0 frame, size delta 0 (lane p9-mid, 2026-09-12;
 * was 62).  The previous closure asked to "resume with the missing pool value first" and recorded
 * the record/base homes at +0x78 and +0xCC as an open question.  That question is now closed: it
 * was declaration order, and the frame census -- which nobody had run on this function -- named it
 * in one command.  Both sides carry 36 slots in a 0xD0 frame, and before these edits the target
 * homed the record aggregate at +0x78..+0xAB with a four-byte local above it at +0xCC while this
 * candidate homed the record at the frame top and that local at +0x80.  Declaring baseValue first
 * and the record tenth puts nine four-byte autos above the aggregate, and **the two ladders are now
 * slot-for-slot identical**.  62 -> 53.
 *
 * Two statement moves follow from the same reading of the object:
 *   spread before record.magnitude4 in the emission loop, 53 -> 45.  The shipped code interleaves
 *   the two integer-to-float conversions and issues the spread product first; all 24 permutations
 *   of the four independent loop-body statements were measured and this is the unique optimum.
 *   magnitudeScale hoisted above the emission-count guard, 45 -> 37.  Its fourteen other positions
 *   through the preamble are flat, as are eight positions of record.phase14.
 *
 * What is left is one word's worth of cause: the shipped code emits an extra `move` of the emission
 * count into a scratch register at +0x104, before the loop, which shifts +0x104..+0x150 by one word
 * and accounts for essentially the whole residual.  That is L113 read backwards -- the shipped
 * loop's index does not die at strength reduction and this candidate's does.  Ten loop-counter
 * spellings (post-decrement in the guard, decrement-then-test-negative, four tail conditions, two
 * guard comparisons, an or-with-zero probe, and a separate counter local) are all 37 or worse. */
#ifdef NON_MATCHING
void func_overlay_008_F0002640_1860398(
    O8P2640Anchor *anchor, O8P2640Config *config, s32 orientation,
    s32 randomLow, s32 randomHigh, f32 distanceX, f32 unusedStackFloat,
    f32 distanceZ, s32 emissionCount) {
    s32 baseValue;
    const O8P2640Tuning *tuning;
    f32 axisA;
    f32 axisB;
    f32 clampedDistance;
    f32 spread;
    f32 magnitudeScale;
    s32 randomOffset;
    s32 randomValue;
    O8P2640Record record;
    s32 magnitudeValue;
    s32 tuningIndex;

    (void)unusedStackFloat;
    tuningIndex = config->tuningIndex1;
    if (tuningIndex >= 10) {
        tuningIndex = 0;
    }

    clampedDistance = O8P2640_call_26AC(
        distanceX * distanceX + distanceZ * distanceZ);
    if (clampedDistance < 2.0f) {
        return;
    }
    if (clampedDistance > 8.0f) {
        clampedDistance = 8.0f;
    }

    baseValue = O8P2640_call_26F0(-distanceX, -distanceZ);
    axisA = O8P2640_call_26FC(anchor->helperInput0);
    tuning = &D_2110[tuningIndex];
    axisB = O8P2640_call_2708(anchor->helperInput0);

    record.coordC = anchor->coord10 + tuning->offset4;
    record.phase14 = O8P2640_data_198;
    record.size18 = 0x80;
    record.kind1A = 5;
    record.packed1C = 0xFFFFFFFF;
    record.packed28 = 0xFFFF80FF;
    record.packed20 = 0xFFFF80E0;
    record.packed2C = 0xFF8000E0;
    record.packed24 = 0xFF800000;
    record.packed30 = 0xFF000000;

    magnitudeScale = O8P2640_data_19C;
    if (emissionCount == 0) {
        return;
    }
    emissionCount--;
    do {
        randomOffset = O8P2640_call_27BC(-0xC80, 0xC80);
        randomValue = O8P2640_call_27CC(randomLow, randomHigh);
        magnitudeValue = O8P2640_call_27DC(0x50, 0x78);
        record.value0 = (s16)(baseValue + randomOffset);
        record.value2 = (s16)randomValue;
        spread = (f32)randomOffset * tuning->spreadScale8;
        record.magnitude4 = (f32)magnitudeValue *
                            clampedDistance * magnitudeScale;
        if (orientation == 0) {
            record.coord8 = anchor->coordC - tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB +
                             tuning->extent0 * axisA;
        } else if (orientation == 2) {
            record.coord8 = anchor->coordC + tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB -
                             tuning->extent0 * axisA;
        } else {
            record.coord8 = anchor->coordC + spread * axisB;
            record.coord10 = anchor->coord14 - spread * axisA;
        }

        O8P2640_call_28C0(&record);
        record.phase14 = 1.0f;
    } while (emissionCount--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0002640_1860398.s")
#endif

void func_overlay_008_F000291C_1860674(O8P291CMotion *motion,
                                       O8P291CState *state,
                                       f32 update) {
    f32 horizontal;
    f32 vertical;
    f32 invUpdate;
    f32 nextY;
    f32 nextX;
    f32 nextZ;
    f32 displacementX;
    f32 displacementY;
    f32 displacementZ;
    f32 delta;
    f32 scale;
    f32 blendLimit;
    s16 angle;
    s32 contact;

    motion->heading0 = state->angleF0 + state->angleFC + state->angle104;
    angle = state->angleF0 + state->angleFE;

    if (state->mode438 == 1) {
        delta = o8Approach291CReloc(O8P291C_data_1A0, (s32)update);
        if ((state->control4 < -0.5f) || (state->control4 > 0.5f)) {
            state->control4 *= delta;
        } else {
            state->control4 = 0.0f;
        }
        if ((state->control8 < -0.5f) || (state->control8 > 0.5f)) {
            state->control8 *= delta;
        } else {
            state->control8 = 0.0f;
        }
    }

    if (state->active181 != 0) {
        delta = state->speed84 * update +
                       (0.5f * state->accel88 * update * update);
        displacementX = state->axis74 * delta;
        displacementY = state->axis78 * delta;
        displacementZ = state->axis7C * delta;
        if (delta < 0.0f) {
            state->speed84 = 0.0f;
            state->accel88 = 0.0f;
            state->active181 = 0;
            if (((state->flags41C & 0x8000) == 0) &&
                (state->suppress185 == 0)) {
                state->control4 = 0.0f;
                state->control8 = 0.0f;
            }
        }
        scale = 1.0f - state->speed84 / state->speed80;
        state->speed84 += state->accel88 * update;
        horizontal = O8P291C_call_sin(angle) * state->control4 * scale;
        vertical = O8P291C_call_cos(angle) * state->control4 * scale;
    } else {
        displacementX = 0.0f;
        displacementY = 0.0f;
        displacementZ = 0.0f;
        horizontal = O8P291C_call_sin(angle) * state->control4;
        vertical = O8P291C_call_cos(angle) * state->control4;
    }

    horizontal += state->control8 * O8P291C_call_cos(angle);
    vertical -= state->control8 * O8P291C_call_sin(angle);
    o8Surface291CReloc(motion, state, (s32)update);

    nextX = horizontal * update + displacementX;
    nextY = (motion->velocity20 * update) -
        (0.5f * O8P291C_gravity * update * update) + displacementY;
    nextZ = vertical * update + displacementZ;
    invUpdate = 1.0f / update;
    motion->velocity1C = nextX * invUpdate;
    motion->velocity20 -= O8P291C_gravity * update;
    motion->velocity24 = nextZ * invUpdate;
    motion->positionC += nextX;
    motion->position10 += nextY;
    motion->position14 += nextZ;

    contact = O8P291C_call_037C(motion, state, update);
    if ((O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f) != 0) ||
        (motion->state2E == -1)) {
        state->reset170 = 1;
        motion->positionC = state->origin38;
        motion->position10 = state->origin3C;
        motion->position14 = state->origin40;
        O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f);
    }

    state->delta94 = (motion->positionC - state->origin38) * invUpdate;
    state->delta98 = (motion->position10 - state->origin3C) * invUpdate;
    state->delta9C = (motion->position14 - state->origin40) * invUpdate;

    if ((state->mode16A == 0) &&
        ((contact != 0) || (motion->link48->gate62 != 0))) {
        invUpdate = (state->flags41C & 0x8000) ? 3.0f : 0.0f;
        state->blendC +=
            (invUpdate - state->blendC) *
            (1.0f - o8Approach291CReloc(O8P291C_data_1A4, (s32)update));
        if (state->control4 < -state->blendC) state->control4 = -state->blendC;
        if (state->blendC < state->control4) state->control4 = state->blendC;
        if (state->control8 < -state->blendC) state->control8 = -state->blendC;
        if (state->blendC < state->control8) state->control8 = state->blendC;
    } else {
        if (D_10 < state->blendC + O8P291C_data_1A8) {
            state->blendC = D_10;
            return;
        }
        state->blendC +=
            (D_10 - state->blendC) *
            (1.0f - o8Approach291CReloc(O8P291C_data_1AC, (s32)update));
    }
}

void func_overlay_008_F0002EC0_1860C18(register Overlay8UpdateOwner *owner,
                                       Overlay8UpdateInput *input,
                                       s32 updateRate) {
    Overlay8UpdateFlag *flag;
    f32 delta;
    f32 decay;
    s32 remaining;

    if (owner->child == NULL) {
        return;
    }

    flag = owner->child->flag;
    owner->child->near = 0;
    if (flag != NULL) {
        flag->flags &= ~2;
    }

    if (input->active != 0) {
        input->delta6C = input->position68 - owner->position10;
        delta = input->delta6C;

        if (delta < 25.0f) {
            if ((input->lateral < gOverlay8UpdateLowerReloc) ||
                (input->lateral > gOverlay8UpdateUpperReloc)) {
                if (flag != NULL) {
                    flag->flags |= 2;
                }
            } else if (delta < 12.0f) {
                owner->child->near = 1;
            }
            owner->child->target80 = owner->position10 + 4.0f;
        } else if ((owner->velocity20 < 0.0f) && (updateRate != 0)) {
            remaining = updateRate - 1;
            decay = gOverlay8UpdateDecayReloc;
            do {
                owner->velocity20 -= owner->velocity20 * decay;
            } while (remaining--);
        }
    }

    overlay8FinishUpdateReloc(owner, updateRate);
}

/* Mickey-local reconstruction. The compiler-generated constants are linked
 * against the overlay's retained literal pool through an ABS section symbol;
 * no instruction word is changed after compilation. */
void overlay8UpdateChannels(void *unused, Overlay8ChannelState *state,
                            f32 gate, void *sampleState) {
    f32 factor;
    f32 maximumFactor;
    f32 sample;
    f32 position;
    register const f32 upper = 0.1f;
    register const f32 lower = -0.1f;
    register const f32 multiplier = -0.67f;
    s32 i;
    s32 selectorIndex;
    const f32 *selectorScales;

    selectorScales = gOverlay8SelectorScales;
    maximumFactor = 0.0f;
    for (i = 0; i < 4; i++) {
        if (gate < 8.0f) {
            if (state->modes[i] != 0) {
                state->modes[i] = 2;
                state->values[i] = state->values[i] * multiplier;
                if ((lower < state->values[i]) &&
                    (state->values[i] < upper)) {
                    state->modes[i] = 0;
                }
            } else {
                if (state->selectorMode == 1) {
                    selectorIndex = 0;
                } else {
                    selectorIndex = i;
                }
                factor = selectorScales[
                    state->selectors[selectorIndex] & 0xF];
                *(volatile f32 *) &state->values[i] =
                    gOverlay8PhaseScales[state->phases[i]] * state->position;
                state->values[i] *= factor;
                if (maximumFactor < factor) {
                    maximumFactor = factor;
                }
            }
        } else {
            sample = overlay8SampleChannel(0.95f, sampleState);
            state->values[i] +=
                (-4.0f - state->values[i]) * (1.0f - sample);
            state->modes[i] = 1;
        }

        state->phases[i] = (state->phases[i] + 1) & 0x1F;
    }

    if (0.05f <= maximumFactor) {
        position = state->position;
        if ((position < -4.5f) || (position > 4.5f)) {
            overlay8EmitChannel(state, 0x28, 0.15f);
        }
    }
}

void func_overlay_008_F0003278_1860FD0(void *unused0,
                                       Overlay8ColorState *state,
                                       void *unused2) {
    s32 red;
    s32 green;
    s32 blue;

    if ((state->flags186 & 3) == 0) {
        return;
    }

    if ((state->timer16A > 0) && (state->owner != NULL)) {
        red = state->owner->colors->red;
        green = state->owner->colors->green;
        blue = state->owner->colors->blue;
    } else {
        red = 0;
        green = 0x40;
        if (state->alternate184 != 0) {
            green = 0;
            red = 0xE0;
            blue = 0x40;
        } else {
            blue = 0xE0;
        }
    }

    if (state->target354 != NULL) {
        o8ApplyColorsReloc(state->target354, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
    if (state->target360 != NULL) {
        o8ApplyColorsReloc(state->target360, 0xFF, 0xFF, 0xFF,
                                          red, green, blue);
    }
}

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans found no
 * donor. Loading the lower threshold through upperThreshold before
 * overwriting it preserves IDO's copy-coalescing order ($f2, then $f0).
 * Retained post-correction C is 78/78 after four runtime LOCAL tuples; a fresh
 * compile/link/full-bin chain remains queued. */
void overlay8ScaleOutputs(void *unused, Overlay8ScaleState *state,
                          Overlay8ScaleContext *context,
                          Overlay8ScaleOutput *output) {
    Overlay8ScalePair *pair;
    Overlay8ScaleRecord *record;
    s16 *cursor;
    s32 i;
    u32 index;
    f32 upperThreshold;
    f32 lowerThreshold;

    pair = output->pairs;
    cursor = output->outputs[output->outputIndex];
    i = 0;

    if (context->count > 0) {
        upperThreshold = gOverlay8ScaleLowerReloc;
        lowerThreshold = upperThreshold;
        upperThreshold = gOverlay8ScaleUpperReloc;
        do {
            output = (Overlay8ScaleOutput *)pair->selector;
            index = (u32)output & 0xFF;
            /* Inert allocation aid retained by exact C; tracked in
             * docs/cleanup-queue.md. */
            if (!index) {}
            record = context->slots[index].record;

            if (((u32)output & 0x00100000) != 0) {
                if ((state->flags41C & 0x4000) != 0) {
                    if ((state->position >= upperThreshold) &&
                        (state->value42C < -30)) {
                        pair->first = 0x100;
                    } else {
                        pair->first = 0x200;
                    }
                } else if (state->position <= lowerThreshold) {
                    pair->first = 0;
                } else {
                    pair->first = 0x100;
                }
            }

            *cursor++ = (pair->first >> 8) * record->scale;
            if (pair->second >= 0) {
                *cursor++ = (pair->second >> 8) * record->scale;
            }
            i++;
            pair++;
        } while (i < context->count);
    }
}

/* NON_MATCHING: 897 vs 898 instructions, 466 masked/490 raw words different,
 * frame -0x80 exact, 107 of the shipped 109 relocations with 59 offset/type
 * pairs agreeing and 50 identities resolved, first mismatch +0x1C.  Eighteen
 * of the twenty-one call-delimited regions are exact; the three that are not
 * are +1, +1 and -3 words.  Eight of the fifteen stack homes -- the scratch
 * quad, the mode/target/counter trio and every incoming-argument home -- sit
 * at the shipped offsets.
 *
 * Two things block the rest.  The six remaining homes are four bytes high
 * because the shipped frame wants one escaping four-byte pointer at +0x40 and
 * a separate escaping four-word block at +0x58, with +0x44/+0x48/+0x4C
 * serving as ordinary float homes between them; measured with this compiler,
 * a single aggregate never lends its padding to a home, four plain scalars
 * lose the -1 stores to dead-store elimination, and volatile scalars keep the
 * stores at a cost of roughly 270 extra differing words.  Separately, the
 * shipped selector burst re-reads gOverlay8Buffer even straight after storing
 * it, which is the whole of the -3 region and the two missing relocations;
 * declaring the pointer volatile reproduces that shape but changes code this
 * unit already matches, so it is not available.  GLOBAL_ASM stays canonical. */
#ifdef NON_MATCHING
f32 func_overlay_008_F00034A0_18611F8(O8P34A0Owner *owner,
                                      O8P34A0State *state, f32 limit,
                                      f32 update) {
    s32 index;
    s32 sampleCount;
    s32 target;
    s32 selectedMode;
    f32 trigB;
    f32 delta;
    f32 **sample;
    O8P34A0Query query;
    f32 blend;
    f32 selectedValue;
    f32 result;
    f32 trigA;
    f32 factor;
    f32 strength;
    s16 outputAngle;
    s8 ownerMode;
    s32 steps;

    selectedMode = owner->mode3B;
    query.scratch04 = -1;
    query.scratch08 = -1;
    query.scratch0C = -1;
    query.scratch10 = -1;
    result = 0.0f;
    selectedValue = 0.0f;
    blend = 0.0f;

    if (state->motion4 < -0.5f) {
        if (state->direction100 < 0) {
            selectedMode = 6;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x16;
                selectedValue = D_1D8;
            } else {
                selectedValue = D_1DC;
            }
        } else if (state->direction100 > 0) {
            selectedMode = 7;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x17;
                selectedValue = D_1E0;
            } else {
                selectedValue = D_1E4;
            }
        } else {
            selectedValue = D_1E8;
            if ((state->flags1A8 & 1) != 0) {
                state->smoothed38A -= state->smoothed38A / 8;
                state->smoothed38A += state->steering108 / 8;
                if (state->smoothed38A >= 0xDD) {
                    selectedMode = 4;
                } else if (state->smoothed38A < -0xDC) {
                    selectedMode = 5;
                } else if ((state->smoothed38A >= 0x42) &&
                           (state->smoothed38A < 0x96)) {
                    selectedMode = 2;
                } else if ((state->smoothed38A < -0x41) &&
                           (state->smoothed38A >= -0x95)) {
                    selectedMode = 3;
                } else if ((state->smoothed38A >= -0x18) &&
                           (state->smoothed38A < 0x19)) {
                    selectedMode = 1;
                } else {
                    selectedMode = owner->mode3B;
                }
            } else if (state->steering108 >= 0x75) {
                selectedMode = 4;
            } else if (state->steering108 < -0x74) {
                selectedMode = 5;
            } else if (state->steering108 >= 0x11) {
                selectedMode = 2;
            } else if (state->steering108 < -0x10) {
                selectedMode = 3;
            } else {
                selectedMode = 1;
            }
        }
    } else if (state->motion4 > 0.5f) {
        if (state->steering108 >= 0x11) {
            if ((owner->mode3B == 8) && (owner->scale28 == 1.0f)) {
                blend = 1.0f;
                selectedMode = 0xA;
                selectedValue = D_1EC;
            } else if (owner->mode3B != 0xA) {
                selectedMode = 8;
                selectedValue = D_1F0;
            }
        } else if ((owner->mode3B == 9) && (owner->scale28 == 1.0f)) {
            blend = 1.0f;
            selectedMode = 0xB;
            selectedValue = D_1F4;
        } else if (owner->mode3B != 0xB) {
            selectedMode = 9;
            selectedValue = D_1F8;
        }
    } else {
        selectedMode = 0;
        selectedValue = D_1FC;
        if ((owner->mode3B == 0x11) || (owner->mode3B == 0x12)) {
            if (owner->scale28 != 1.0f) {
                selectedMode = owner->mode3B;
                selectedValue = D_200;
            }
        } else if ((owner->mode3B == 0) &&
                   (o8P34A0RandomReloc(0, 0x3FF) >= 0x3FB)) {
            blend = 0.0f;
            selectedMode = o8P34A0RandomReloc(0x11, 0x12);
            selectedValue = D_204;
        }
    }

    if ((state->lowering349 == 0) && (state->override172 == 0)) {
        if (owner->mode3B == 0xC) {
            selectedMode = 0xC;
            blend = 0.0f;
            gO8P34A0ScaleReloc *= D_208;
            selectedValue = D_20C;
            result = 40.0f;
        } else {
            sampleCount = o8P34A0TerrainReloc(owner->xC, owner->z14,
                                              0x1800, &query.samples0);
            if (sampleCount != 0) {
                index = 0;
                if (sampleCount > 0) {
                    sample = query.samples0;
                    do {
                        result = **sample;
                        index++;
                        sample++;
                    } while ((result >= owner->y10) &&
                             (index != sampleCount));
                }
                result = owner->y10 - result;
                if (result > 40.0f) {
                    gO8P34A0ScaleReloc *= D_210;
                    state->mode16C = 1;
                    selectedMode = 0xC;
                    blend = 0.0f;
                    selectedValue = D_214;
                    o8P34A0EffectReloc(owner, 0x15, 0xC);
                }
            }
        }
    }

    ownerMode = owner->mode3B;
    if ((state->force185 == 1) ||
        ((ownerMode == 0xE) && (owner->scale28 != 1.0f))) {
        selectedMode = 0xE;
        blend = 0.0f;
        selectedValue = (gO8P34A0ModeReloc == 0) ? D_218 : D_21C;
    } else if (((ownerMode == 0x13) || (ownerMode == 0x14) ||
                (ownerMode == 0x15)) &&
               (owner->scale28 != 1.0f)) {
        selectedMode = ownerMode;
        blend = 0.0f;
        selectedValue = D_220;
    } else if (((state->flags41C & 0x2000) != 0) &&
               ((state->gate19B == 0) || (state->gate19C != 0))) {
        selectedMode = 0xD;
        blend = 0.0f;
        selectedValue = D_224;
    }

    if (state->overrideMode193 != 0) {
        selectedMode = state->overrideMode193;
        selectedValue = *(f32 *)((u8 *)state + 0x194);
        blend = 0.0f;
    }

    if (((owner->peer48->gate63 != 0) || (state->motion34A != 0)) &&
        (state->active181 != 0) && (ownerMode != 0x13) &&
        (ownerMode != 0x14) && (ownerMode != 0x15)) {
        state->activity3C8 += 2.0f;
        blend = 0.0f;
        selectedValue = D_228;
        if ((state->value8C <= D_22C) || (D_230 <= state->value8C)) {
            selectedMode = 0x13;
            ownerMode = owner->mode3B;
        } else if (state->value90 >= 0.0f) {
            selectedMode = 0x14;
            ownerMode = owner->mode3B;
        } else {
            selectedMode = 0x15;
            ownerMode = owner->mode3B;
        }
    }

    if ((ownerMode == 0x18) && (owner->scale28 != 1.0f)) {
        selectedMode = 0x18;
        blend = 0.0f;
        selectedValue = D_234;
    } else if (state->secondary102 > 0) {
        selectedMode = 0xF;
        blend = 0.0f;
        selectedValue = D_238;
    } else if (state->secondary102 < 0) {
        selectedMode = 0x10;
        blend = 0.0f;
        selectedValue = D_23C;
    }

    if (selectedMode != ownerMode) {
        o8P34A0SetModeReloc(owner, selectedMode, -1, blend);
    }
    if ((o8P34A0AnimateReloc(owner, selectedValue, update) != 0) &&
        (selectedValue != 0.0f)) {
        state->flags1A8 |= 2;
    } else {
        state->flags1A8 &= 0xFFFD;
    }
    if ((state->overrideMode193 != 0) &&
        (selectedMode != state->overrideMode193)) {
        o8P34A0EventReloc(owner, 0x3C, selectedMode);
    }

    index = 0;
    if (gOverlay8Value != 0) {
        index = 2;
        o8P34A0StateEffectReloc(state, 0x28, 0.15f);
    }
    for (; index < 4; index++) {
        state->angles114[index] +=
            (s32)(state->motion4 * update * D_240);
    }

    target = state->steering428;
    if (target < -0x3C) {
        target = -0x1770;
    } else if (target >= 0x3D) {
        target = 0x1770;
    } else {
        target *= 0x64;
    }
    steps = (s32)update;
    if (steps != 0) {
        index = steps - 1;
        do {
            state->angle110 +=
                o8P34A0ApproachReloc(state->angle110, target) >> 2;
        } while (index-- != 0);
    }
    state->angle112 = state->angle110;

    *gOverlay8Buffer = 3;
    gOverlay8Buffer++;
    *gOverlay8Buffer = state->angle144;
    gOverlay8Buffer++;
    *gOverlay8Buffer = 9;
    gOverlay8Buffer++;
    *gOverlay8Buffer = state->angle146;
    gOverlay8Buffer++;
    func_overlay_008_F00049E8_1862740(owner, state, update);

    if (state->kind1 == 4) {
        /* Address-form reads reload limit from its incoming slot (L144). */
        if ((state->motion4 < 0.0f) && (*(f32 *)&limit != 0.0f)) {
            if (state->motion4 < -*(f32 *)&limit) {
                outputAngle = -0x3000;
            } else {
                outputAngle = (s16)(s32)((12288.0f / *(f32 *)&limit) *
                                         state->motion4);
            }
            trigA = D_244;
            strength = -state->motion4 / *(f32 *)&limit;
            delta = (update / 60.0f) * strength * 25.0f;
            state->phase3EC += delta;
            state->phase3F0 += delta;
            if (trigA <= state->phase3EC) {
                state->phase3EC -= trigA;
            }
            if (trigA <= state->phase3F0) {
                state->phase3F0 -= trigA;
            }
            trigA = o8P34A0TrigAReloc(
                (s32)((state->phase3EC / trigA) * 65536.0f));
            trigB = o8P34A0TrigBReloc(
                (s32)((state->phase3F0 / D_248) * 65536.0f));
            factor = strength * 4096.0f;
            state->output3F8 = -outputAngle;
            state->output3F4 = outputAngle + (s32)(factor * trigA);
            state->output3F6 = outputAngle + (s32)(factor * trigB);
        } else {
            factor = 1.0f - o8P34A0DecayReloc(D_24C, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(D_250, steps);
            state->output3F6 =
                o8P34A0BlendReloc(state->output3F6, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(D_254, steps);
            state->output3F8 =
                o8P34A0BlendReloc(state->output3F8, 0, factor);
        }

        *gOverlay8Buffer = 0x1E;
        gOverlay8Buffer++;
        *gOverlay8Buffer =
            state->output3F8 + state->angle144 + state->angle146;
        gOverlay8Buffer++;
        *gOverlay8Buffer = 0x21;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F4;
        gOverlay8Buffer++;
        *gOverlay8Buffer = 0x24;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F6;
        gOverlay8Buffer++;
    } else if (state->kind1 == 2) {
        if ((state->motion4 < 0.0f) && (limit != 0.0f)) {
            factor = D_258;
            strength = -state->motion4 / limit;
            state->phase3EC += (update / 60.0f) * strength * 30.0f;
            if (factor <= state->phase3EC) {
                state->phase3EC -= factor;
            }
            trigA = o8P34A0TrigAReloc(
                (s32)((state->phase3EC / factor) * 65536.0f));
            state->output3F4 = (s32)(8192.0f * strength * trigA);
        } else {
            factor = 1.0f - o8P34A0DecayReloc(D_25C, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
        }
        *gOverlay8Buffer = 0x1E;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->output3F4;
        gOverlay8Buffer++;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00034A0_18611F8.s")
#endif

/* Workbench p4: structure-mismatch; 384 positional/384 raw words differ,
 * 430/447 instructions, first +0x0, frame exact -160. Levers: mode home and
 * targetMotion volatility; remains FP/register web. */
#ifdef NON_MATCHING
void func_overlay_008_F00042A8_1862000(O8P42A8Actor *actor,
                                       O8P42A8Owner *owner, f32 update) {
    O8P42A8State *state;
    s32 mode;
    f32 targetMotion;
    f32 targetHeight;
    f32 phase;
    f32 smoothing;
    f32 firstTrig;
    f32 secondTrig;
    f32 relativeFirst;
    f32 relativeSecond;
    f32 lateral;
    f32 forward;
    f32 vertical;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 targetTilt;
    f32 absoluteVelocity;
    f32 acceleration;
    f32 steering;
    s32 randomMode;
    s32 tableIndex;
    s32 targetAngle;
    s32 steps;
    s32 remaining;
    s16 savedAngle;

    state = owner->state64;
    mode = state->mode0 & 3;
    o8P42A8SampleReloc(60.0f, 0);

    if (state->reset170 != 0) {
        steps = (s32)update;
        savedAngle = actor->angle0;
        if (steps != 0) {
            remaining = steps - 1;
            do {
                actor->angle0 +=
                    o8P42A8ApproachReloc(actor->angle0, savedAngle) >> 3;
                actor->angle2 +=
                    o8P42A8ApproachReloc(actor->angle2, 0x800) >> 3;
                actor->angle4 -= actor->angle4 >> 3;
            } while (remaining--);
        }
        return;
    }

    if ((state->cycleFlags420 & 8) != 0) {
        D_0[mode]++;
    }
    D_0[mode] &= 3;
    tableIndex = D_0[mode];
    randomMode = o8P42A8RandomReloc() & 3;
    if ((state->reset170 != 0) || (state->lock191 != 0)) {
        tableIndex = 1;
    }

    targetAngle = ((s16 *)0x2208)[randomMode];
    targetMotion = O8_F32(0x2188 + (((randomMode * 4) + tableIndex) * 4));
    phase = D_2210[mode];
    if ((state->lowering349 == 0) && (state->lock191 == 0)) {
        phase += O8_F32(0x260) * update;
        if (phase > 1.0f) {
            phase = 1.0f;
        }
    } else {
        phase -= 0.125f * update;
        if (phase < 0.0f) {
            phase = 0.0f;
        }
    }
    D_2210[mode] = phase;

    state->directionDC = 0x8000 - state->angleF0;
    acceleration = state->accelerationE4;
    steering = state->steeringE0;
    targetAngle -= (owner->angle2 * 3) >> 2;
    if (targetAngle >= 0x2001) {
        targetAngle = 0x2000;
    }
    if (targetAngle < -0x2000) {
        targetAngle = -0x2000;
    }
    targetHeight = O8_F32(0x21C8 + (((randomMode * 4) + tableIndex) * 4));
    targetHeight += phase * O8_F32(0x2220 + (randomMode * 4));

    steps = (s32)update;
    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->angle2 +=
                o8P42A8ApproachReloc(actor->angle2, targetAngle) >> 4;
        } while (remaining--);
    }

    if (state->velocity4 < 0.0f) {
        f32 reduction = -6.0f * acceleration * state->velocity4;

        if (state->modifier100 != 0) {
            reduction *= 0.5f;
        }
        if (reduction > 65.0f) {
            reduction = 65.0f;
        }
        targetMotion -= reduction;
    }

    smoothing = steering * 60.0f;
    if (state->double184 != 0) {
        smoothing += smoothing;
    }
    if (state->modifier100 != 0) {
        smoothing *= 0.5f;
    }
    targetMotion += smoothing;
    if ((state->force185 == 1) && ((state->flags41C & 0x4000) == 0)) {
        targetMotion = 32.0f;
    }
    if (state->special18D != 0) {
        targetMotion = 200.0f;
        targetHeight = 10.0f;
    }

    if (O8_S32(0) == 0) {
        smoothing = O8_F32(0x264);
    } else {
        smoothing = O8_F32(0x268);
    }
    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->motion24 += (targetMotion - actor->motion24) * smoothing;
            actor->motion28 += (targetHeight - actor->motion28) * smoothing;
        } while (remaining--);
    }

    firstTrig = o8P42A8TrigAReloc(0x8000 - state->directionDC);
    secondTrig = o8P42A8TrigBReloc(0x8000 - state->directionDC);
    relativeFirst = o8P42A8TrigAReloc(actor->angle2 - targetAngle);
    relativeSecond = o8P42A8TrigBReloc(actor->angle2 - targetAngle);
    lateral = (actor->motion24 * relativeSecond) -
              (actor->motion28 * relativeFirst);
    baseX = lateral * firstTrig;
    baseY = (actor->motion24 * relativeFirst) +
            (actor->motion28 * relativeSecond);
    baseZ = lateral * secondTrig;

    if (state->sign102 == 0) {
        targetTilt = -10.0f;
    } else {
        targetTilt = 10.0f;
    }
    remaining = steps - 1;
    if (steps != 0) {
        do {
            state->heightE8 +=
                (targetTilt - state->heightE8) * O8_F32(0x27C);
        } while (remaining--);
    }

    baseX += owner->xC + (state->offset14 * state->heightE8);
    baseY += owner->y10 + (state->offset18 * state->heightE8);
    baseZ += owner->z14 + (state->offset1C * state->heightE8);
    firstTrig = o8P42A8TrigAReloc(state->directionDC + 0x4000);
    secondTrig = o8P42A8TrigBReloc(state->directionDC + 0x4000);

    if (state->modifier100 != 0) {
        targetTilt = (f32)(state->modifier100 * -12);
    } else if ((state->flags41C & 0x8000) != 0) {
        absoluteVelocity = state->velocity4;
        if (absoluteVelocity < 0.0f) {
            absoluteVelocity = -absoluteVelocity;
        }
        if (absoluteVelocity > 1.0f) {
            absoluteVelocity = 1.0f;
        }
        targetTilt = absoluteVelocity * (f32)state->magnitude108 *
                     O8_F32(0x288);
    } else {
        targetTilt = 0.0f;
    }

    remaining = steps - 1;
    if (steps != 0) {
        do {
            state->tiltEC +=
                (targetTilt - state->tiltEC) * O8_F32(0x284);
        } while (remaining--);
    }

    vertical = state->tiltEC;
    actor->xC = baseX + (vertical * firstTrig);
    actor->y10 = baseY;
    actor->z14 = (baseZ - (vertical * secondTrig));
    actor->angle0 = state->directionDC;

    remaining = steps - 1;
    if (steps != 0) {
        do {
            actor->angle4 +=
                o8P42A8ApproachReloc(actor->angle4, owner->angle4 >> 1) >> 5;
        } while (remaining--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00042A8_1862000.s")
#endif

void overlay8SetBuffer(void *base) {
    gOverlay8Buffer = (s16 *)((u8 *)base + 0x1B8);
}

void overlay8WriteCommand(volatile s32 unused) {
    *gOverlay8Buffer = 0x2000;
    gOverlay8Buffer++;
}

void overlay8SetValue(s32 value) {
    gOverlay8Value = value;
}

void overlay8UpdateMotionOutput(Overlay8MotionAnchor *anchor,
                                Overlay8MotionState *state,
                                f32 inputScale) {
    const Overlay8MotionRow *row;
    s32 span;
    Overlay8MotionAnchor *target;
    f32 delta;
    struct {
        f32 first;
        f32 second;
    } scales;

    row = &D_2230[state->rowIndex];
    target = state->target;
    span = row->firstScale + row->secondScale;

    if (target == NULL) {
        delta = 0.0f;
    } else {
        delta = (f32) overlay8ConvertDirectionReloc(
            anchor->helperInput,
            overlay8MeasureDirectionReloc(anchor->x - target->x,
                                          anchor->y - target->y));
    }

    if (span == 0) {
        return;
    }

    if ((f32) span < delta) {
        delta = (f32) span;
    } else if (delta < (f32) -span) {
        delta = (f32) -span;
    }

    if ((8000.0f < delta) || (delta < -8000.0f)) {
        if (state->outsideLatch == 0) {
            state->outsideLatch = 1;
            state->outsideValue = gOverlay8MotionOutsideValueReloc;
        }
    } else if (state->outsideLatch == 1) {
        state->outsideLatch = 0;
    }

    if (delta != (f32) state->primary) {
        register f32 step;

        step = overlay8ApproachMotionReloc(
            (s32) (delta - (f32) state->primary),
            (f32) state->secondary * 2.0f, 800.0f);
        state->secondary = (s16) (s32) ((f32) state->secondary + step);
        state->primary = (s16) (s32) (
            (f32) state->primary +
            (f32) state->secondary * inputScale * 0.5f);
    }

    scales.first = (f32) row->firstScale * (1.0f / (f32) span);
    scales.second = (f32) row->secondScale * (1.0f / (f32) span);

    *gOverlay8Buffer = (s16) ((u32) row->firstSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.first);
    gOverlay8Buffer++;
    *gOverlay8Buffer = (s16) ((u32) row->secondSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.second);
    gOverlay8Buffer++;
}

/* Plateau (2026-09-09): allocation-mismatch, 39 masked words, exact 270/270
 * instructions and exact -0x90 frame, and every stack home now at the target
 * displacement -- the surface-normal aggregate included, which was the four-byte
 * gap the previous handoff left open.
 *
 * The whole residual is one extra FP pool web.  At the normal-vector product the
 * target keeps `normal.x` in a ugen ring temp and gives axisA's reload the first
 * pool colour; the candidate colours `normal.x` instead, so axisA takes the third
 * colour and the ring pops one slot out of phase from there.  That single
 * displacement carries all of it: an f4/f6 ring exchange over 32 of the 39 sites,
 * FP pool slot 2 at row 113 and FP temp slot 14 at row 115.  Every integer lane
 * is identical.
 *
 * Exhausted for this residual, each measured on the exact-home candidate:
 * the `horizontalB = normal.x` carrier cannot be removed (reading the member
 * twice under `volatile` emits two loads, 57 words; without `volatile` uopt folds
 * the copy and the body is one instruction short at 269, 159 words); moving the
 * carrier to any other local -- `factor`, `blendFactor`, a fresh inner-block
 * local, or `horizontalA` with the two products swapped -- also drops to 269;
 * the 16-form operand-order lattice over the two products is flat; the 15-form
 * volatile-placement lattice over the aggregate's four members has `volatile x`
 * alone as its unique optimum; 37 physical line joins across the function are
 * byte-inert, so the line-grouping lever does not apply in this TU; and
 * permuting the point-initialisation, point-accumulate and activation statement
 * groups is flat at 39.
 *
 * Declaration order is fixed and not a free variable: homes follow declaration
 * order top-down, so horizontalB must stay eighth (home 0x70) and axisA ninth
 * (home 0x6C), which is exactly the order that numbers the normal.x web first.
 * Carrier identity is now tested too and is not the lever (2026-09-10): axisA
 * is uniquely correct in both blocks, axisB's carrier is inert because it never
 * becomes a coloured web, and 34 carrier, inner-block-scope, negation-splitting
 * and read-back forms are flat at 39 or lose an instruction.  An all-volatile
 * aggregate with direct reads scores 37 and is NOT closer -- it emits five
 * loads where the target emits three and fills both r4300 multiply-hazard nop
 * slots the target keeps.
 * Corrected 2026-09-11 (lane p7-ovl2): this is NOT a globalcolor decision.
 * Thirty single-web forces over the five float webs of this region, crossed
 * with the split verdict and the five lowest float colours, leave it at 39 or
 * worse in every cell.  The corrected float-bank census reads all 39 sites as
 * float with a closed two-cycle over the two lowest scratch registers on 32 of
 * them, and those two are ugen's rotation, never a p1 colour -- so the residual
 * is a ugen float free-list phase.  The carrier-free form is also no longer an
 * instruction short: it is delta 0 at 270 words and 57 masked.  Resume with a
 * ugen float free-list trace, not a colouring receipt. */
/* Ownership trial (2026-08-28): fixed the TU's +0x27C..+0x2AC .rodata range;
 * linked promotion is text-differs after removing the TU growth; codegen remains.
 * The candidate's literal pool is retained as the remaining structural gap. */
#ifdef NON_MATCHING
void func_overlay_008_F0004CF0_1862A48(O8P4CF0Actor *actor,
                                       O8P4CF0State *state,
                                       s32 updateRate) {
    s32 start;
    s32 end;
    O8P4CF0SceneItem **items;
    f32 motionTarget;
    f32 blendFactor;
    s32 targetB;
    f32 horizontalA;
    f32 horizontalB;
    f32 axisA;
    f32 axisB;
    f32 surfaceHeight;
    O8P4CF0Vec3f point;
    O8P4CF0Normal normal;

    state->activated173 = 0;

    if (O8P4CF0_call_4D14(20) != 0) {
        items = O8P4CF0_call_4D24(&start, &end);
        point.x = 0.0f;
        point.y = 0.0f;
        point.z = 7.0f;
        O8P4CF0_call_4D54(1, actor, &point, &point);
        point.x += actor->x00C;
        point.y += actor->y010;
        point.z += actor->z014;
        surfaceHeight = point.y;

        if (start < end) {
            do {
                O8P4CF0SceneItem *item = items[start++];

                if (item->category044 == 0x3D) {
                    O8P4CF0Bounds *bounds = item->bounds084;

                    if (bounds != 0) {
                        if ((bounds->minX000 <= (s32)point.x) &&
                            ((s32)point.x <=
                             bounds->minX000 + bounds->extentX006)) {
                            if ((bounds->minZ004 <= (s32)point.z) &&
                                ((s32)point.z <=
                                 bounds->minZ004 + bounds->extentZ008)) {
                                if (item->callbackGate088 != 0) {
                                    O8P4CF0_call_4E50(item);
                                }
                                surfaceHeight = O8P4CF0_call_4E64(
                                    bounds, point.x, point.z, &normal);
                                break;
                            }
                        }
                    }
                }
            } while (start < end);
        }

        if (surfaceHeight > point.y) {
            s32 targetA;
            f32 factor;

            axisA = O8P4CF0_call_4E9C(-actor->angle000);
            axisB = O8P4CF0_call_4EAC(-actor->angle000);
            horizontalB = normal.x;
            horizontalA = -(normal.z * axisA + horizontalB * axisB);
            horizontalB = normal.z * axisB - horizontalB * axisA;
            targetA = O8P4CF0_call_4EE8(horizontalA, normal.y);
            targetB = O8P4CF0_call_4EF8(horizontalB, normal.y);

            factor = O8P4CF0_call_4F0C(0.9f, updateRate);
            actor->angle004 = (s16)O8P4CF0_call_4F34(
                actor->angle004, (s16)targetA, 1.0f - factor);
            factor = O8P4CF0_call_4F48(0.9f, updateRate);
            actor->angle002 = (s16)O8P4CF0_call_4F68(
                actor->angle002, (s16)targetB, 1.0f - factor);

            start = updateRate - 1;
            if (updateRate != 0) {
                do {
                    actor->vertical020 +=
                        (((surfaceHeight - state->height178 - point.y) *
                          0.5f) - actor->vertical020) * 0.25f;
                } while (start--);
            }

            O8P4CF0_data_4FD4 *= 0.4f;
            state->timer172 = 12;
            state->activated173 = 1;
        }
    }

    if (state->timer172 != 0) {
        state->timer172 =
            (s8)((u32)(s32)state->timer172 - (u32)updateRate);
        if (state->timer172 <= 0) {
            state->timer172 = 0;
            state->blend174 = (f32)1;
            state->height178 = 2.0f;
            state->derived17C = 1.0f;
            return;
        } else {
            axisA = state->motion004;
            if (axisA < 0.0f) {
                axisA = -axisA;
            }
            axisA *= 0.2f;
            if (1.0f < axisA) {
                axisA = 1.0f;
            }

            start = updateRate - 1;
            if (updateRate != 0) {
                blendFactor = 0.05f;

                do {
                    state->blend174 +=
                        (axisA - state->blend174) * blendFactor;
                } while (start--);
            }
            state->height178 = state->blend174 * -4.0f + 6.0f;
            state->derived17C =
                state->blend174 * 0.024999976f + 0.975f;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F0004CF0_1862A48.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_008_F0004CF0_1862A48:start
 * symbol: func_overlay_008_F0004CF0_1862A48
 * score: 39 differing words
 * frame: -0x90
 * relocations: 15
 * first-mismatch: +0x1C4
 * summary: source question is the ugen float free-list phase at the +0x1F4 window
 * PLATEAU-HANDOFF:func_overlay_008_F0004CF0_1862A48:end
 */

/* PLATEAU-HANDOFF:func_overlay_008_F0002640_1860398:start
 * symbol: func_overlay_008_F0002640_1860398
 * score: 37/183 words
 * frame: 0xD0
 * relocations: 14
 * first-mismatch: +0xF0
 * summary: source question is the preheader emission-count move and loop strength reduction
 * PLATEAU-HANDOFF:func_overlay_008_F0002640_1860398:end
 */



/* PLATEAU-HANDOFF:func_overlay_008_F00042A8_1862000:start
 * symbol: func_overlay_008_F00042A8_1862000
 * score: 384 differing words
 * frame: 0xA0
 * relocations: 40
 * first-mismatch: +0x44
 * summary: Configured flags remain best; reconstruct FP lifetimes and named data identities to supply 17 words and 22 relocation records without changing the exact frame.
 * PLATEAU-HANDOFF:func_overlay_008_F00042A8_1862000:end
 */

/* PLATEAU-HANDOFF:func_overlay_008_F0000058_185DDB0:start
 * symbol: func_overlay_008_F0000058_185DDB0
 * score: 438 differing words
 * frame: 0xA0
 * relocations: 86
 * first-mismatch: +0x2C
 * summary: Six surface entries and direct bounds restore the exact frame; recover the remaining eight global relocation pairs and 16 instructions before allocator work.
 * PLATEAU-HANDOFF:func_overlay_008_F0000058_185DDB0:end
 */

/* PLATEAU-HANDOFF:func_overlay_008_F00034A0_18611F8:start
 * symbol: func_overlay_008_F00034A0_18611F8
 * score: 308/336 words
 * frame: 0x80
 * relocations: 107
 * first-mismatch: +0x1C
 * summary: L144 address-form of limit in kind 4 spills the f18 fragment at delta 0; 308 masked. Pointer home, a3 vs a2, and six insertion sites remain.
 * PLATEAU-HANDOFF:func_overlay_008_F00034A0_18611F8:end
 */

/* PLATEAU-HANDOFF:func_overlay_008_F0001294_185EFEC:start
 * symbol: func_overlay_008_F0001294_185EFEC
 * score: 636 differing words
 * frame: 0xC8
 * relocations: 137
 * first-mismatch: +0x0
 * summary: Procedure-7 census prices 355 draws; remaining mixed integer and FP ring phases have no target-backed source lever.
 * PLATEAU-HANDOFF:func_overlay_008_F0001294_185EFEC:end
 */
