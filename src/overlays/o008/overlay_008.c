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

/* Matched 2026-10-02.  The four selector reads are a plain for loop that the
 * default -O2 unroller expands as shipped (the TU no longer carries
 * -Wo,-loopunroll,0); the clamp bounds and the bounce thresholds are
 * literals; the surface walk is `while (index--)`; the present flag is read
 * from its global in the test, not through a carrier; the mode dispatch is a
 * switch with the call arm second; the mode setter's last argument is a
 * float; and the locals are declared in the shipped frame order, with one
 * unused word. */
void func_overlay_008_F0000058_185DDB0(O8P0058Owner *owner,
                                       s32 updateRate) {
    O8P0058State *state;
    f32 floorHeight;
    f32 value;
    s32 pad;
    f32 vector[3];
    s16 angles[3];
    O8P0058Surface *surface;
    s32 count;
    s32 index;
    s32 present;
    O8P0058Surface surfaces[6];
    f32 update;
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
    gO8P0058ResultReloc = query->initial0;
    if (gO8P0058PresentReloc != 0) {
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
                    o8P0058SpawnReloc(owner, 0x18, -1, 0.0f);
                }
            } else if (state->active183 != 0x80) {
                state->alternate184 = 1;
            }
            state->active183 = 0;
        }
    }

    if (state->lower4 < -31.99f) {
        state->lower4 = -31.99f;
    }
    if (31.99f < state->lower4) {
        state->lower4 = 31.99f;
    }
    if (state->upper8 < -31.99f) {
        state->upper8 = -31.99f;
    }
    if (31.99f < state->upper8) {
        state->upper8 = 31.99f;
    }

    o8P0058OrientReloc(owner, state);
    angles[0] = -state->angleF0;
    angles[1] = -owner->angle2;
    angles[2] = -owner->angle4;
    vector[2] = 0.0f;
    vector[1] = -1.0f;
    vector[0] = 0.0f;
    o8P0058RotateReloc(angles, vector);
    state->direction60 = vector[0];
    state->direction64 = vector[1];
    state->direction5C = vector[2];

    count = o8P0058SurfaceReloc(owner->xC, owner->z14, 0, 0x08010000,
                                surfaces);
    floorHeight = -32768.0f;
    state->surface68 = -32768.0f;
    index = count;
    while (index--) {
        surface = &surfaces[index];
        if ((surface->flags4 & 0x10000) != 0) {
            state->surface68 = surface->height0;
        }
        if ((surface->flags4 & 0x08000000) != 0) {
            floorHeight = surface->height0;
        }
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
        for (index = 0; index < 4; index++) {
            D_8 += D_310[state->selectors320[index] & 0xF];
            D_C += query->heights148[D_350[state->selectors320[index] & 0xF]];
        }
        D_8 *= 0.25f;
        D_C *= 0.25f;

        if (state->override172 != 0) {
            D_C = state->override17C;
        }
        if (state->surfaceActive2 != 0) {
            D_C = 0.98f;
        }
        if ((state->peerD4 != 0) && (0.7f < D_8)) {
            D_8 += (0.7f - D_8) * state->peerD4->state64->blend14;
        }
    }

    switch (state->mode16C) {
        case 0:
        case 1:
            update = (f32)updateRate;
            func_overlay_008_F0001294_185EFEC(owner, state, update);
            break;
        default:
            update = (f32)updateRate;
            break;
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
            value = -0.1f;
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
            (state->position50 == state->bounce54)) {
            state->bounceVelocity3FC *= -0.5f;
            if ((-0.01f < state->bounceVelocity3FC) &&
                (state->bounceVelocity3FC < 0.01f)) {
                state->bounceActive18C = 0;
                state->bounceVelocity3FC = 0.0f;
                state->bounce54 = 1.0f;
                return;
            }
            state->bounce54 = state->bounceVelocity3FC + 1.0f;
        }
    } else {
        state->bounceActive18C = 0;
    }
}

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

/* The per-character tuning block overlay8GetIndexed returns for the motion
 * update.  Field reads and array element reads give different operand orders
 * in the shipped code, so the layout is the one those orders imply. */
typedef struct O8P1294Tuning {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20[9];
    f32 unk44[32];
    f32 unkC4;
    f32 unkC8[16];
} O8P1294Tuning;

/* Matched 2026-10-02 (lane x-near) and promoted: 1259 words, the 0xB0 frame,
 * 137 relocations.  The last three words were the update count's first read
 * (v0 for v1): the count is `index` (the one symbol web coloured v1 here),
 * and the dead store `update = 0.0f` after the conversion is what keeps it
 * one -- it redefines the conversion's operand, so uopt cannot forward
 * `(s32) update` into the two uses as a one-block temporary that takes v0
 * first, and the trunc is written straight into index's register; the store
 * itself is removed as redundant (delta 0).  Every spelling that kept the
 * operand intact (five positions, ten carriers, kills of the result,
 * cast round-trips, literal types, regions) left the temporary in place.
 * What moved it from 636 (2026-10-01): the pool floats are literals at each
 * use, one pool entry per use as shipped; state fields are read directly
 * rather than through a shared value carrier; scale is multiplied by D_8 in
 * place; the pre-loop factor is its own symbol, so the loop's scale outranks
 * the cached state->unk4 web; and integer carriers are shared the way the
 * shipped registers share them (noted on the declarations).
 * 275 to 78 (lane d-o008): the stick test is a plain compare; the turn
 * limit, the yaw step and the drift step take the carriers the shipped
 * registers imply; the curve sum adds the base first.
 * 78 to 16 (lane e-o008, 2026-10-02): the tuning block is a struct read by
 * field (0x40 is the last element of the 0x20 array), so the pre-loop factor
 * is value * 0.5f * tuning->unkC with the product on the left; the
 * owner->unk80 updates are |=; the selector default is assigned in both arms;
 * effectMask is or-ed before D_4 is set; the frame counter is a plain
 * (x + 1) & 0xF after the turn amount (the u8 cast was standing in for the
 * draw the turn conversion spends); the spin-out negation reads unk100
 * directly; unkFC is updated before the drift step; the brake result
 * borrows angle's symbol and its test reads the input bit directly.
 * 16 to 7 (lane h-r8, 2026-10-02): no driftDirection local, state->unk100 is
 * read at each use; the stick is not copied into a carrier before the clamp
 * (steeringTarget = state->unk428, first test on the field), so uopt keeps
 * the copy and the later clamp tests read it; the spin-arming tests read the
 * stick directly; the wobble term rides angleStep; modeFlags is s16; the
 * 1.6 clamp is one source line.
 * 7 to 3 (lane x-near, 2026-10-02), both from the decision records: the
 * pre-loop factor test reads the field (that is the shipped c.eq.s operand
 * order) and the field's load web outranks the factor's only when the
 * factor web spans six blocks, so its clamp and its use each sit in a
 * do-while(0) region (nocs 3, save 4/3 under the load's 3/2) and region 2
 * borrows scale; the drift step's unkFE value rides cooldown's symbol web,
 * whose save of 30 is coloured v0 before the difference web (20), and the
 * shift sits in a region so it is written straight into index rather than
 * through a one-block temporary that would otherwise take v0 first.
 * The update loop tests the old counter; the braking global is a halfword;
 * mathDiffAngle accepts the full requested angle. */
void func_overlay_008_F0001294_185EFEC(O8P1294Owner *owner,
                                       O8P1294State *state, f32 update) {
    s32 updateCount;
    f32 value;
    s32 angle;              /* brake result, then the state->unk108 copy */
    s32 steeringInput;      /* the drift difference, then the stick at the spin test */
    s32 updatesRemaining;
    s32 steeringTarget;     /* the stick read directly, clamped, then the target angle */
    s32 turnAmount;
    s32 angleStep;          /* angle step, the wobble term, then the left selector */
    s32 impactBoost;
    s32 colorEnabled[2];
    s32 braking;            /* brake bit, then the turn limit */
    s32 inputFlags;         /* input word, then the right selector */
    s32 turnDirection;
    s32 effectMask;
    s32 index;              /* speed level, curve index, yaw step, then the drift step */
    s16 cooldown;           /* unkA2 cooldown, then the unkFE drift value */
    f32 scale;              /* loop scale, and the spin factor before the loop */
    s8 animation;
    s16 modeFlags;          /* read as one web with the unk349 bits; u8 splits it */
    f32 speedLimit;
    O8P1294Tuning *tuning;
    O8P1294ColorTarget *colorTarget;
    u8 *color;
    f32 *curve;
    f32 factor;             /* the pre-loop speed factor only */

    tuning = o8P1294IndexedReloc((Overlay8IndexedObject *)state);
    impactBoost = 0;
    index = state->unk192;
    effectMask = 0;
    if ((s32) index >= 0xB) {
        index = 0xA;
    }
    D_10 = ((f32) index * tuning->unk8) + tuning->unk20[8];
    if (state->unkD4 != NULL) {
        D_10 *= 1.0f + (-0.3f * state->unkD4->state64->blend14);
    }
    if (state->unk185 == 0) {
        if (state->unk5C != 0.0f) {
            factor = 1.0f - state->unk5C * 0.5f * tuning->unkC;
            /* Two regions: factor then spans six blocks and the unk5C load
             * is coloured first (f0), as shipped. */
            do { if (factor < 0.1f) { factor = 0.1f; } } while (0);
            do { D_10 *= factor; } while (0);
        }
    }
    if (state->unk102 != 0) {
        animation = owner->unk3B;
        if ((animation == 0x10) || (animation == 0xF)) {
            scale = owner->unk28 * 1.5f;
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            if (state->unk102 > 0) {
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
    index = (s32) update;
    update = 0.0f; /* kills the conversion's operand: index keeps the count (v1) */
    updatesRemaining = index - 1;
    if (index != 0) {
        do {
            value = o8P1294PhaseReloc(owner, (O8PhaseState *)state, D_10);
            speedLimit = value;
            if (31.99f < value) {
                speedLimit = 31.99f;
            }
            inputFlags = state->unk41C;
            if ((!(inputFlags & 0x4000)) && (state->unk5C > 0.0f) && (state->unk4 < -D_10)) {
                angle = 1;
            } else if ((!(inputFlags & 0x4000)) && (state->unk5C < 0.0f)) {
                angle = 1;
            } else if (!(inputFlags & 0xC000) && (state->unk5C > 0.0f)) {
                angle = 1;
            } else {
                angle = 0;
            }
            if ((angle != 0) && (state->unk185 == 0)) {
                state->unk4 += gO8P1294MotionScalarReloc * state->unk5C;
                if (state->unk4 > tuning->unk1C) {
                    state->unk4 = tuning->unk1C;
                }
            }
            if (state->unk102 != 0) {
                state->unk428 = 0;
                state->unk42C = 0;
                state->unk41C = 0;
                state->unk420 = 0;
                state->unk4 *= 0.99f;
                state->unk8 *= 0.99f;
            }
            scale = (state->unk148 - state->unk4) * 10920.0f;
            if (scale < -1092.0f) {
                scale = -1092.0f;
            }
            if (1092.0f < scale) {
                scale = 1092.0f;
            }
            state->unk144 += ((s32) scale - state->unk144) >> 3;
            scale = (state->unk148 - state->unk4) * 16380.0f;
            if (scale < -2184.0f) {
                scale = -2184.0f;
            }
            if (2184.0f < scale) {
                scale = 2184.0f;
            }
            inputFlags = state->unk41C;
            state->unk148 = state->unk4;
            braking = inputFlags & 0x4000;
            state->unk146 += ((s32) scale - state->unk146) >> 3;
            if ((braking != 0) && (inputFlags & 0x10) && ((state->unk428 < -0x1E) || (state->unk428 >= 0x1F))) {
                if (state->unk4 < -5.0f) {
                    state->unk4 += 0.2f;
                    if (state->unk4 > -5.0f) {
                        state->unk4 = -5.0f;
                    }
                } else {
                    state->unk4 -= 0.1f;
                    if (state->unk4 < -5.0f) {
                        state->unk4 = -5.0f;
                    }
                }
                effectMask |= 0x3C;
                D_4 = 1;
                if (state->unkE4 < 1.0f) {
                    state->unkE4 += 0.1f;
                }
            } else if (braking != 0) {
                if (state->unk4 < -5.0f) {
                    effectMask |= 0x3C;
                    D_4 = 1;
                }
                if (state->unk4 < 0.0f) {
                    state->unk4 += tuning->unkC8[(s32)-state->unk4];
                    if ((state->unk4 > 0.0f) && (state->unk42C >= -0x1E)) {
                        goto block_74;
                    }
                } else if (state->unk42C < -0x1E) {
                    state->unk4 += tuning->unk20[(s32)state->unk4];
                    if (state->unk4 > tuning->unk1C) {
                        state->unk4 = tuning->unk1C;
                    }
                } else {
                    state->unk4 -= tuning->unkC8[0];
                    if (state->unk4 <= 0.0f) {
block_74:
                        state->unk4 = 0.0f;
                    }
                }
                if (state->unkE4 < 1.0f) {
                    state->unkE4 += 0.1f;
                }
            } else if (inputFlags & 0x8000) {
                if ((state->unk184 != 0) ||
                    (modeFlags = state->unk185, (modeFlags == 1)) ||
                    (modeFlags == 2)) {
                    impactBoost = 1;
                    if (gO8P1294ImpactGateReloc == 0) {
                        scale = 0.3334f;
                    } else {
                        scale = 0.5f;
                    }
                    controlSetRumble(state, 0x32, 0.15f);

                } else {
                    if (state->unk4 > 0.0f) {
                        curve = tuning->unkC8;
                        index = (s32) state->unk4;
                        scale = state->unk4 - (f32) index;
                    } else {
                        curve = tuning->unk44;
                        index = (s32) -state->unk4;
                        scale = -state->unk4 - (f32) index;
                    }
                    scale = curve[index] + ((curve[index + 1] - curve[index]) * scale);
                }
                if (state->unk4 < -speedLimit) {
                    state->unk4 *= 0.99f;
                    if (state->unk4 > -speedLimit) {
                        state->unk4 = -speedLimit;
                    }
                    if (state->unk184 != 0) {
                        state->unk184 = 2U;
                    }
                } else {
                    state->unk4 -= scale;
                    if (state->unk4 < -speedLimit) {
                        state->unk4 = -speedLimit;
                        if (state->unk184 != 0) {
                            state->unk184 = 2U;
                        }
                    } else if (0.2f <= scale) {
                        if ((impactBoost == 0) || (state->unk184 != 0)) {
                            effectMask |= 0xC;
                        }
                        effectMask |= 0x30;
                    }
                }
                if (0.05f <= state->unkE4) {
                    state->unkE4 -= 0.05f;
                }
            } else {
                if ((-tuning->unkC4 < state->unk4) &&
                    (state->unk4 < tuning->unkC4)) {
                    state->unk4 = 0.0f;
                } else {
                    state->unk4 *= 0.99f;
                }
            }
            if ((state->unk18D == 0) && (state->unk158 == 0) && (state->unk349 != 0)) {
                if ((-0.2f < state->unk4) && (state->unk4 < 0.2f) &&
                    (func_800299E8(0, 0x7F) >= 0x73)) {
                    owner->unk80 |= 0xC;
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
                        owner->unk20 += 3.0f;
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
            if (state->unk4 < 0.0f) {
                scale = -scale;
            }
            if (scale <= 0.2f) {
                scale = 0.0f;
            } else {
                scale = scale - 0.2f;
                if (1.6f < scale) { scale = 1.6f; }
                if (state->unk100 != 0) {
                    scale = (scale * 68.0f * 60.0f) / tuning->unk10;
                } else {
                    scale = (scale * 58.0f * 60.0f) / tuning->unk10;
                }
                if (state->unk4 > 0.0f) {
                    scale = -scale;
                }
            }

            steeringTarget = state->unk428;
            if ((state->unk41C & 0x10) && (braking = 0x2710, (state->unk100 != 0))) {
                if (state->unk428 >= 0x33) {
                    steeringTarget = 0x32;
                } else if (steeringTarget < -0x32) {
                    steeringTarget = -0x32;
                }
                if (((steeringTarget > 0) && (state->unk100 > 0)) || ((steeringTarget < 0) && (state->unk100 < 0))) {
                    steeringTarget = steeringTarget >> 2;
                }
                steeringTarget += state->unk100 * 0x3C;
            } else {
                state->unk100 = 0;
                if (steeringTarget >= 0x3D) {
                    steeringTarget = 0x3C;
                } else if (steeringTarget < -0x3C) {
                    steeringTarget = -0x3C;
                }
                braking = (s32) tuning->unk14;
            }
            steeringTarget = (s32) (((f32) -steeringTarget * tuning->unk10) / 60.0f);
            angleStep = (s32) ((f32) (steeringTarget - state->unk108) * 0.145f);
            if (angleStep != 0) {
                state->unk108 += angleStep;
            } else {
                state->unk108 = (s16) steeringTarget;
            }
            if ((state->unk4 > -10.0f) && (state->unk4 < 10.0f)) {
                if (state->unk4 < 0.0f) {
                    value = state->unk4 * -0.1f;
                } else {
                    value = state->unk4 * 0.1f;
                }
                value = ((D_C - 1.0f) * value) + 1.0f;
            } else {
                value = D_C;
            }
            angle = state->unk108;
            state->unk4 *= value;
            if ((angle < -braking) || (braking < angle)) {
                state->unk4 *= tuning->unk18;
            }
            turnAmount = (s32) ((f32) state->unk108 * scale);
            state->unk182 = (state->unk182 + 1) & 0xF;
            if (turnAmount != 0) {
                turnDirection = 0;
                if ((state->unk100 < 0) || (turnAmount < -0x1400)) {
                    turnDirection = -1;
                } else if ((state->unk100 > 0) || (turnAmount >= 0x1401)) {
                    turnDirection = 1;
                }
                if (turnDirection != 0) {
                    angleStep = (func_8002A204((s16)(state->unk182 << 12)) *
                               ((state->unk106 * 8) + 0x200)) >> 16;
                    if (turnDirection < 0) {
                        angleStep = -angleStep;
                    }
                    turnAmount += angleStep;
                }
            }
            index = mathDiffAngle(state->unkFC, turnAmount) >> 2;

            turnAmount = index;
            if (index < -0x2EE) {
                turnAmount = -0x2EE;
            } else if (index >= 0x2EF) {
                turnAmount = 0x2EE;
            }
            state->unkFC += turnAmount;
            cooldown = state->unkFE; /* v0 before the difference is coloured */
            steeringInput = (state->unk100 << 0xD) - cooldown;
            do { index = steeringInput >> 4; } while (0); /* no shift temporary */
            if (index == 0) {
                index = steeringInput;
            }
            state->unkFE = cooldown + index;
            if (state->unk100 != 0) {
                steeringInput = state->unk428;
                if (((steeringInput >= 0x1A) && (state->unk100 < 0)) || ((steeringInput < -0x19) && (state->unk100 > 0))) {
                    state->unk106++;
                } else {
                    state->unk106 = 0;
                }
                if (state->unk106 >= 0x3D) {
                    controlSetRumble(state, 0x28, 0.15f);
                }
                if (state->unk106 >= 0x5B) {
                    state->unk102 = -state->unk100;
                    state->unk100 = 0;
                }
            } else {
                state->unk106 = 0;
            }
            scale = state->unk4;
            if (state->unk4 < 0.0f) {
                scale = -scale;
            }
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            if (state->unk4 > 0.0f) {
                scale = -scale;
            }
            if (state->unk41C & 0x4000) {
                if (state->unk100 != 0) {
                    scale *= 3.0f;
                } else {
                    scale *= 1.0f + state->unkE4;
                }
            }
            scale *= D_8;
            state->unkF0 += (s32) ((f32) state->unk108 * scale);
            state->unkE0 += ((-0.05f * state->unk4) - state->unkE0) * 0.065f;
            state->unk8 *= 0.96f;
            if ((-0.1f < state->unk8) && (state->unk8 < 0.1f)) {
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
    if ((state->unk2 != 0) || (state->unkD4 != NULL)) {
        angleStep = 1;
        inputFlags = 1;
    } else {
        angleStep = state->unk322 & 0xF;
        inputFlags = state->unk323 & 0xF;
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
        owner->unk80 |= D_460[angleStep];
    }
    if (effectMask & 2) {
        owner->unk80 |= D_4A0[inputFlags];
    }
    if (effectMask & 4) {
        owner->unk80 |= D_3E0[angleStep];
    }
    if (effectMask & 8) {
        owner->unk80 |= D_420[inputFlags];
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
        color = (angleStep * 4) + D_4E0;
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
        color = (inputFlags * 4) + D_4E0;
        if (color[3] != 0) {
            colorTarget->color24[0] = (u8) color[0];
            colorTarget->color24[1] = (u8) color[1];
            colorTarget->color24[2] = (u8) color[2];
            colorTarget->color24[3] = (u8) color[3];
            D_18 = 1;
        }
    }
}

/* Matched 2026-10-01 (was 11/183).  Three edits, needed together:
 *   the two pool floats are literals at their uses, with no magnitudeScale
 *   carrier, which puts the 1.0f materialisation ahead of the 0.01f load;
 *   spread is computed before record.value2 is stored;
 *   the record emitter is declared as returning a value, so the dead copy of
 *   the old emission count is not offered v0 and takes v1 (L101).
 * The unused float keeps the ninth four-byte auto above the record, which
 * the home ladder needs (declaration order, 2026-09-12). */
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
    f32 unusedScale;
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
    record.phase14 = 1.6f;
    record.size18 = 0x80;
    record.kind1A = 5;
    record.packed1C = 0xFFFFFFFF;
    record.packed28 = 0xFFFF80FF;
    record.packed20 = 0xFFFF80E0;
    record.packed2C = 0xFF8000E0;
    record.packed24 = 0xFF800000;
    record.packed30 = 0xFF000000;

    if (emissionCount--) {
        do {
            randomOffset = O8P2640_call_27BC(-0xC80, 0xC80);
            randomValue = O8P2640_call_27CC(randomLow, randomHigh);
            magnitudeValue = O8P2640_call_27DC(0x50, 0x78);
            spread = (f32)randomOffset * tuning->spreadScale8;
            record.value2 = (s16)randomValue;
            record.value0 = (s16)(baseValue + randomOffset);
            record.magnitude4 = (f32)magnitudeValue *
                                clampedDistance * 0.01f;
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
}

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

/* NON_MATCHING: exact size and frame, 17 masked words (2026-10-02).  Every
 * stack home is at its shipped offset: the terrain query's pointer and the
 * four -1 scratch words are separate locals, and the update count is
 * declared between trigB and blend.  The float constants are this function's
 * own literal pool, one entry per use.  The effect call is
 * overlay7DispatchSelection, which takes two arguments.  `limit` is read
 * plainly, and the approach loop is `while (index--)`.
 *
 * The motion tests read the mode through `ownerMode`, assigned in each arm:
 * uopt colours a symbol as one web, so those reads take the v1 the later
 * tests take (an expression web there takes v0).  The kind-4 outputs add
 * `(s16)(strength * trig)` to the angle, with `strength *= 4096.0f` in place
 * and the negated angle stored last: the deleted narrowing spends the three
 * scratch draws the whole command burst after it was rotated by.  The zero
 * initialisers are result, blend, selectedValue; the 0xA/0xB arms assign the
 * mode before blend; the landing flag is stored after the animation rate.
 *
 * The command burst reaches the buffer through a second name from its second
 * pair on: that gives the shipped second address build.  Each pair is its own
 * block, so the two address webs do not interfere.  Left (17): the first
 * pair's one-block address web takes v0 where the shipped one takes v1 (14
 * words; forced, exact); the approach loop saves the counter after loading
 * the second argument, shipped before (2); the mode compare's operands are
 * reversed (1).  GLOBAL_ASM stays canonical. */
#ifdef NON_MATCHING
f32 func_overlay_008_F00034A0_18611F8(O8P34A0Owner *owner,
                                      O8P34A0State *state, f32 limit,
                                      f32 update) {
    s32 index;
    s32 sampleCount;
    s32 target;
    s32 selectedMode;
    f32 delta;
    f32 **sample;
    s32 scratch[4];
    f32 trigB;
    s32 steps;
    f32 blend;
    f32 selectedValue;
    f32 result;
    f32 **samples;
    f32 trigA;
    f32 factor;
    f32 strength;
    s16 outputAngle;
    s8 ownerMode;

    selectedMode = owner->mode3B;
    scratch[0] = -1;
    scratch[1] = -1;
    scratch[2] = -1;
    scratch[3] = -1;
    result = 0.0f;
    blend = 0.0f;
    selectedValue = 0.0f;

    if (state->motion4 < -0.5f) {
        if (state->direction100 < 0) {
            selectedMode = 6;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x16;
                selectedValue = 0.05f;
            } else {
                selectedValue = 0.05f;
            }
        } else if (state->direction100 > 0) {
            selectedMode = 7;
            if ((state->flags41C & 0x4000) != 0) {
                selectedMode = 0x17;
                selectedValue = 0.05f;
            } else {
                selectedValue = 0.05f;
            }
        } else {
            selectedValue = 0.04f;
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
            ownerMode = owner->mode3B;
            if ((ownerMode == 8) && (owner->scale28 == 1.0f)) {
                selectedMode = 0xA;
                blend = 1.0f;
                selectedValue = 0.04f;
            } else if (ownerMode != 0xA) {
                selectedMode = 8;
                selectedValue = 0.025f;
            }
        } else {
            ownerMode = owner->mode3B;
            if ((ownerMode == 9) && (owner->scale28 == 1.0f)) {
                selectedMode = 0xB;
                blend = 1.0f;
                selectedValue = 0.04f;
            } else if (ownerMode != 0xB) {
                selectedMode = 9;
                selectedValue = 0.025f;
            }
        }
    } else {
        selectedMode = 0;
        selectedValue = 0.04f;
        ownerMode = owner->mode3B;
        if ((ownerMode == 0x11) || (ownerMode == 0x12)) {
            if (owner->scale28 != 1.0f) {
                selectedMode = ownerMode;
                selectedValue = 0.008f;
            }
        } else if ((ownerMode == 0) &&
                   (o8P34A0RandomReloc(0, 0x3FF) >= 0x3FB)) {
            blend = 0.0f;
            selectedMode = o8P34A0RandomReloc(0x11, 0x12);
            selectedValue = 0.008f;
        }
    }

    if ((state->lowering349 == 0) && (state->override172 == 0)) {
        if (owner->mode3B == 0xC) {
            selectedMode = 0xC;
            blend = 0.0f;
            gO8P34A0ScaleReloc *= 0.9f;
            selectedValue = 0.0667f;
            result = 40.0f;
        } else {
            sampleCount = o8P34A0TerrainReloc(owner->xC, owner->z14,
                                              0x1800, &samples);
            if (sampleCount != 0) {
                index = 0;
                if (sampleCount > 0) {
                    sample = samples;
                    do {
                        result = **sample;
                        index++;
                        if (result < owner->y10) {
                            break;
                        }
                        sample++;
                    } while (index != sampleCount);
                }
                result = owner->y10 - result;
                if (result > 40.0f) {
                    gO8P34A0ScaleReloc *= 0.9f;
                    selectedMode = 0xC;
                    blend = 0.0f;
                    selectedValue = 0.0667f;
                    state->mode16C = 1;
                    o8P34A0EffectReloc(owner, 0x15);
                }
            }
        }
    }

    ownerMode = owner->mode3B;
    if ((state->force185 == 1) ||
        ((ownerMode == 0xE) && (owner->scale28 != 1.0f))) {
        selectedMode = 0xE;
        blend = 0.0f;
        selectedValue = (gO8P34A0ModeReloc == 0) ? 0.0167f : 0.0134f;
    } else if (((ownerMode == 0x13) || (ownerMode == 0x14) ||
                (ownerMode == 0x15)) &&
               (owner->scale28 != 1.0f)) {
        selectedMode = ownerMode;
        blend = 0.0f;
        selectedValue = 0.0334f;
    } else if (((state->flags41C & 0x2000) != 0) &&
               ((state->gate19B == 0) || (state->gate19C != 0))) {
        selectedMode = 0xD;
        blend = 0.0f;
        selectedValue = 0.0334f;
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
        selectedValue = 0.0334f;
        if ((state->value8C <= -0.707f) || (0.707f <= state->value8C)) {
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
        selectedValue = 0.0111f;
    } else if (state->secondary102 > 0) {
        selectedMode = 0xF;
        blend = 0.0f;
        selectedValue = 0.0111f;
    } else if (state->secondary102 < 0) {
        selectedMode = 0x10;
        blend = 0.0f;
        selectedValue = 0.0111f;
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
            (s32)(state->motion4 * update * 1638.4f);
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
    index = steps;
    while (index--) {
        state->angle110 +=
            o8P34A0ApproachReloc(state->angle110, target) >> 2;
    }
    state->angle112 = state->angle110;

    do {
        *gOverlay8Buffer = 3;
        gOverlay8Buffer++;
        *gOverlay8Buffer = state->angle144;
        gOverlay8Buffer++;
    } while (0);
    do {
        *gOverlay8CommandBuffer = 9;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = state->angle146;
        gOverlay8CommandBuffer++;
    } while (0);
    func_overlay_008_F00049E8_1862740(owner, state, update);

    if (state->kind1 == 4) {
        if ((state->motion4 < 0.0f) && (limit != 0.0f)) {
            if (state->motion4 < -limit) {
                outputAngle = -0x3000;
            } else {
                outputAngle = (s16)(s32)((12288.0f / limit) *
                                         state->motion4);
            }
            trigA = 6.283185f;
            strength = -state->motion4 / limit;
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
                (s32)((state->phase3F0 / 6.283185f) * 65536.0f));
            strength *= 4096.0f;
            state->output3F4 = outputAngle + (s16)(strength * trigA);
            state->output3F6 = outputAngle + (s16)(strength * trigB);
            state->output3F8 = -outputAngle;
        } else {
            factor = 1.0f - o8P34A0DecayReloc(0.95f, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(0.95f, steps);
            state->output3F6 =
                o8P34A0BlendReloc(state->output3F6, 0, factor);
            factor = 1.0f - o8P34A0DecayReloc(0.95f, steps);
            state->output3F8 =
                o8P34A0BlendReloc(state->output3F8, 0, factor);
        }

        *gOverlay8CommandBuffer = 0x1E;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer =
            state->output3F8 + state->angle144 + state->angle146;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = 0x21;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = state->output3F4;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = 0x24;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = state->output3F6;
        gOverlay8CommandBuffer++;
    } else if (state->kind1 == 2) {
        if ((state->motion4 < 0.0f) && (limit != 0.0f)) {
            factor = 6.283185f;
            strength = -state->motion4 / limit;
            state->phase3EC += (update / 60.0f) * strength * 30.0f;
            if (factor <= state->phase3EC) {
                state->phase3EC -= factor;
            }
            trigA = o8P34A0TrigAReloc(
                (s32)((state->phase3EC / factor) * 65536.0f));
            state->output3F4 = (s32)(8192.0f * strength * trigA);
        } else {
            factor = 1.0f - o8P34A0DecayReloc(0.95f, steps);
            state->output3F4 =
                o8P34A0BlendReloc(state->output3F4, 0, factor);
        }
        *gOverlay8CommandBuffer = 0x1E;
        gOverlay8CommandBuffer++;
        *gOverlay8CommandBuffer = state->output3F4;
        gOverlay8CommandBuffer++;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay_008/func_overlay_008_F00034A0_18611F8.s")
#endif

/* NON_MATCHING: exact size and frame, 126 masked words (2026-10-02).  The
 * tables are real data symbols (D_2188, D_21C8, D_2208, D_2220), the float
 * constants are this function's literal pool, every step loop is
 * `remaining = steps; while (remaining--)`, the random draw precedes the
 * re-read of D_0[mode], the relative trig calls take the unclamped table
 * angle, and the position sums are separate locals from the trig products.
 *
 * One float local carries the phase, the velocity reduction, the blend
 * constant, the +-10 height target and the tilt target: uopt colours a
 * symbol as one web, the union spans the trig calls, so it is denied f0 and
 * takes f14 throughout, as shipped (two pads keep its old cells).  The tail
 * is the matched overlay 9 sibling's: `posX += tilt * trig; posZ -= ...;`
 * then three plain sums, and `tilt *= (f32)magnitude * 0.035f`.  The sample
 * call returns nothing, so update is offered f0 after it.
 *
 * Left: the integer webs of the table lookup (randomMode a2, tableIndex a0,
 * their sum in v1, no shared `randomMode * 4`), the `steps` home (0x34
 * shipped, a compiler temporary at 0x2C here) and the counter reload after
 * the approach loop.  The gate word read as `O8_S32(0)` is a resident symbol
 * (records at +0x2CC/+0x3C4): declared as one, its high half is hoisted
 * after the approach loop as shipped, at +4 bytes until that reload moves.
 * GLOBAL_ASM stays canonical. */
#ifdef NON_MATCHING
void func_overlay_008_F00042A8_1862000(O8P42A8Actor *actor,
                                       O8P42A8Owner *owner, f32 update) {
    O8P42A8State *state;
    s32 mode;
    f32 pad1;
    f32 smoothing;
    s32 tableAngle;
    s32 targetAngle;
    f32 pad2;
    f32 posX;
    f32 posY;
    f32 posZ;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 firstTrig;
    f32 secondTrig;
    f32 relativeFirst;
    f32 relativeSecond;
    f32 lateral;
    f32 targetMotion;
    f32 targetHeight;
    f32 acceleration;
    f32 steering;
    s32 steps;
    s32 randomMode;
    s16 savedAngle;
    s32 tableIndex;
    s32 remaining;

    state = owner->state64;
    mode = state->mode0 & 3;
    o8P42A8SampleReloc(60.0f, 0);

    if (state->reset170 != 0) {
        steps = (s32)update;
        savedAngle = actor->angle0;
        remaining = steps;
        while (remaining--) {
            actor->angle0 +=
                o8P42A8ApproachReloc(actor->angle0, savedAngle) >> 3;
            actor->angle2 +=
                o8P42A8ApproachReloc(actor->angle2, 0x800) >> 3;
            actor->angle4 -= actor->angle4 >> 3;
        }
        return;
    }

    if ((state->cycleFlags420 & 8) != 0) {
        D_0[mode]++;
    }
    D_0[mode] &= 3;
    randomMode = o8P42A8RandomReloc() & 3;
    tableIndex = D_0[mode];
    if ((state->reset170 != 0) || (state->lock191 != 0)) {
        tableIndex = 1;
    }

    tableAngle = D_2208[randomMode];
    targetMotion = D_2188[(randomMode * 4) + tableIndex];
    targetHeight = D_21C8[(randomMode * 4) + tableIndex];
    smoothing = D_2210[mode];
    if ((state->lowering349 == 0) && (state->lock191 == 0)) {
        smoothing += 0.08f * update;
        if (smoothing > 1.0f) {
            smoothing = 1.0f;
        }
    } else {
        smoothing -= 0.125f * update;
        if (smoothing < 0.0f) {
            smoothing = 0.0f;
        }
    }
    D_2210[mode] = smoothing;

    state->directionDC = 0x8000 - state->angleF0;
    targetHeight += smoothing * D_2220[randomMode];
    acceleration = state->accelerationE4;
    steering = state->steeringE0;
    targetAngle = tableAngle - ((owner->angle2 * 3) >> 2);
    if (targetAngle >= 0x2001) {
        targetAngle = 0x2000;
    }
    if (targetAngle < -0x2000) {
        targetAngle = -0x2000;
    }

    steps = (s32)update;
    remaining = steps;
    while (remaining--) {
        actor->angle2 +=
            o8P42A8ApproachReloc(actor->angle2, targetAngle) >> 4;
    }

    if (state->velocity4 < 0.0f) {
        smoothing = -6.0f * acceleration * state->velocity4;

        if (state->modifier100 != 0) {
            smoothing *= 0.5f;
        }
        if (smoothing > 65.0f) {
            smoothing = 65.0f;
        }
        targetMotion -= smoothing;
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
        smoothing = 0.0435f;
    } else {
        smoothing = 0.01f;
    }
    remaining = steps;
    while (remaining--) {
        actor->motion24 += (targetMotion - actor->motion24) * smoothing;
        actor->motion28 += (targetHeight - actor->motion28) * smoothing;
    }

    firstTrig = o8P42A8TrigAReloc(0x8000 - state->directionDC);
    secondTrig = o8P42A8TrigBReloc(0x8000 - state->directionDC);
    relativeFirst = o8P42A8TrigAReloc(actor->angle2 - tableAngle);
    relativeSecond = o8P42A8TrigBReloc(actor->angle2 - tableAngle);
    lateral = (actor->motion24 * relativeSecond) -
              (actor->motion28 * relativeFirst);
    baseX = lateral * firstTrig;
    baseY = (actor->motion24 * relativeFirst) +
            (actor->motion28 * relativeSecond);
    baseZ = lateral * secondTrig;

    if (state->sign102 == 0) {
        smoothing = -10.0f;
    } else {
        smoothing = 10.0f;
    }
    remaining = steps;
    while (remaining--) {
        state->heightE8 +=
            (smoothing - state->heightE8) * 0.0915f;
    }

    posX = owner->xC + (state->offset14 * state->heightE8);
    posY = owner->y10 + (state->offset18 * state->heightE8);
    posZ = owner->z14 + (state->offset1C * state->heightE8);
    firstTrig = o8P42A8TrigAReloc(state->directionDC + 0x4000);
    secondTrig = o8P42A8TrigBReloc(state->directionDC + 0x4000);

    if (state->modifier100 != 0) {
        smoothing = (f32)(state->modifier100 * -12);
    } else if ((state->flags41C & 0x8000) != 0) {
        smoothing = state->velocity4;
        if (smoothing < 0.0f) {
            smoothing = -smoothing;
        }
        if (smoothing > 1.0f) {
            smoothing = 1.0f;
        }
        smoothing *= (f32)state->magnitude108 * 0.035f;
    } else {
        smoothing = 0.0f;
    }

    remaining = steps;
    while (remaining--) {
        state->tiltEC += (smoothing - state->tiltEC) * 0.0435f;
    }

    posX += state->tiltEC * firstTrig;
    posZ -= state->tiltEC * secondTrig;
    actor->xC = posX + baseX;
    actor->y10 = posY + baseY;
    actor->z14 = posZ + baseZ;
    actor->angle0 = state->directionDC;

    remaining = steps;
    while (remaining--) {
        actor->angle4 +=
            o8P42A8ApproachReloc(actor->angle4, owner->angle4 >> 1) >> 5;
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

/* Matched 2026-10-01.  The surface normal is read through a pointer taken
 * at entry and handed to the surface query: the indirect reads keep the first
 * horizontal product a register web ahead of the second, as shipped, where
 * direct member reads let uopt forward it into the call and emit the second
 * product first.  The second block carries its value in horizontalA so axisA
 * stays a short web, and the negation is written at the call. */
void func_overlay_008_F0004CF0_1862A48(O8P4CF0Actor *actor,
                                       O8P4CF0State *state,
                                       s32 updateRate) {
    s32 start;
    s32 end;
    O8P4CF0SceneItem **items;
    O8P4CF0Normal *normalPtr;
    f32 blendFactor;
    s32 targetB;
    f32 horizontalA;
    f32 horizontalB;
    f32 axisA;
    f32 axisB;
    f32 surfaceHeight;
    O8P4CF0Vec3f point;
    O8P4CF0Normal normal;

    normalPtr = &normal;
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
                                    bounds, point.x, point.z, normalPtr);
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
            horizontalA = normalPtr->z * axisA + normalPtr->x * axisB;
            horizontalB = normalPtr->z * axisB - normalPtr->x * axisA;
            targetA = O8P4CF0_call_4EE8(-horizontalA, normalPtr->y);
            targetB = O8P4CF0_call_4EF8(horizontalB, normalPtr->y);

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
            horizontalA = state->motion004;
            if (horizontalA < 0.0f) {
                horizontalA = -horizontalA;
            }
            horizontalA *= 0.2f;
            if (1.0f < horizontalA) {
                horizontalA = 1.0f;
            }

            start = updateRate - 1;
            if (updateRate != 0) {
                blendFactor = 0.05f;

                do {
                    state->blend174 +=
                        (horizontalA - state->blend174) * blendFactor;
                } while (start--);
            }
            state->height178 = state->blend174 * -4.0f + 6.0f;
            state->derived17C =
                state->blend174 * 0.024999976f + 0.975f;
        }
    }
}

/* Natural owner for the overlay's original 0x20-byte BSS range.  The live
 * fields occupy 0x1C bytes; the remaining four bytes are section reservation. */
typedef struct {
    s16 *commandBuffer;
    s16 effectFlag;
    f32 surfaceSteeringScale;
    f32 surfaceMotionScale;
    f32 speedLimit;
    s32 leftAlphaEnabled;
    s32 rightAlphaEnabled;
} Overlay8BssOwner;

Overlay8BssOwner gOverlay8BssOwner;

/* PLATEAU-HANDOFF:func_overlay_008_F00042A8_1862000:start
 * symbol: func_overlay_008_F00042A8_1862000
 * score: 126/447 words
 * frame: 0xA0
 * relocations: 40
 * first-mismatch: +0x11C
 * summary: One float carrier (phase to tilt target) on f14, sibling tail, void sample call: 240 to 126. Left: table-lookup int webs, steps home, counter reload.
 * PLATEAU-HANDOFF:func_overlay_008_F00042A8_1862000:end
 */

/* PLATEAU-HANDOFF:func_overlay_008_F00034A0_18611F8:start
 * symbol: func_overlay_008_F00034A0_18611F8
 * score: 17/898 words
 * frame: 0x80
 * relocations: 107
 * first-mismatch: +0x77C
 * summary: ownerMode symbol web, (s16) trig sums, statement orders: 103 to 17. Left: first pair web v0 not v1 (14), loop save order (2), mode compare (1).
 * PLATEAU-HANDOFF:func_overlay_008_F00034A0_18611F8:end
 */
