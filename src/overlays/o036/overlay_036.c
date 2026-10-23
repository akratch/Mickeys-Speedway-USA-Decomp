#include "overlays/overlay_036.h"
#define O36_DIRECTION_MAX 3.4028234e38f

static void func_overlay_036_F0000818_1883CD0(void *, s32);
static void overlay36CallGlobal(void *, void *, s32, s32);
static void overlay36SpawnAtPosition(void *, void *, s32, s32);
static void func_overlay_036_F0000A60_1883F18(void *, void *, s32, s32);
static void overlay36SpawnConditional(void *);

void overlay36PrepareAndTick(void *);
void overlay36SpawnLinked7F(void *);
void overlay36SpawnDirectional(void *);
void overlay36SpawnOffsetA9(void *);
void overlay36TickState(void *);
void overlay36UpdatePeers(void *);
void overlay36SpawnFinalEffect(void *);
void overlay36CallModeZero(void *);
void overlay36CallModeOne(void *);

#define O36_SCALE_DIVISOR 2.8f
#define O36_SCALE_MULTIPLIER 0.6f
#define O36_STRENGTH_MULTIPLIER 0.4f

Overlay36QueueEntry gOverlay36QueueBase[16];
Overlay36QueueEntry *gOverlay36QueueNext;
s32 gOverlay36QueueCount;
s32 gOverlay36Elapsed;
Overlay36Entity *gOverlay36CurrentEntity;
Overlay36State_UpdateInteractiveEntity *gOverlay36CurrentState;

u8 gOverlay36Weights[14][10] = {
    {0},
    {0},
    {10, 20, 40, 40, 40, 20, 20, 20, 10, 10},
    {40, 50, 20, 20, 10, 0, 0, 0, 0, 0},
    {20, 50, 40, 30, 30, 0, 0, 0, 0, 0},
    {0, 10, 15, 20, 20, 0, 0, 0, 0, 0},
    {50, 40, 20, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 10, 5, 20, 60, 60, 60, 40, 20},
    {0, 0, 0, 0, 0, 0, 20, 40, 60, 60},
    {15, 10, 10, 10, 10, 0, 0, 0, 0, 0},
    {0, 10, 30, 30, 30, 10, 10, 10, 0, 0},
    {40, 50, 20, 20, 10, 0, 0, 0, 0, 0},
    {20, 50, 40, 30, 30, 0, 0, 0, 0, 0},
    {50, 40, 20, 0, 0, 0, 0, 0, 0, 0},
};

u8 gOverlay36StateValues[14][2] = {
    {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1},
    {7, 1}, {8, 1}, {9, 1}, {2, 3}, {3, 3}, {4, 3}, {6, 3},
};

u8 gOverlay36AlternateStates[6] = {3, 3, 3, 4, 6, 7};

#define O36_RECORD(kind, arg, flags, timer, normal, positive, fallback, negative, sound, linked, update, conditional, alpha) \
    {kind, arg, flags, timer, normal, NULL, positive, fallback, negative, sound, 0, \
     linked, update, conditional, {0}, 64.0f, -45.0f, 45.0f, {0}, alpha}
#define O36_WEIGHTED(positive, fallback, negative, linked, update, conditional) \
    O36_RECORD(0x86, 1, 1, func_overlay_036_F0000818_1883CD0, \
               func_overlay_036_F0000A60_1883F18, positive, fallback, negative, 29, linked, update, conditional, 32)

Overlay36Record gOverlay36Records[14] = {
    O36_RECORD(0x1B, 20, 1, NULL, overlay36SpawnAtPosition, NULL, NULL, NULL, 0, 0, 0, 0, 32),
    O36_RECORD(0x95, 1, 0, NULL, overlay36CallGlobal, NULL, NULL, NULL, 9, 0, 0, 0, 64),
    O36_WEIGHTED(overlay36PrepareAndTick, overlay36PrepareAndTick, overlay36PrepareAndTick, 0, 0, 0),
    O36_WEIGHTED(overlay36SpawnDirectional, overlay36SpawnDirectional, overlay36SpawnConditional, 0x1C0, 0x1C0, 0x20),
    O36_WEIGHTED(overlay36SpawnLinked7F, overlay36SpawnLinked7F, NULL, 0x1C4, 0x1C4, 0x1C4),
    O36_WEIGHTED(overlay36SpawnOffsetA9, overlay36SpawnOffsetA9, NULL, 0x1C3, 0x1C3, 0),
    O36_WEIGHTED(overlay36CallModeOne, overlay36CallModeZero, overlay36CallModeZero, 0x1C1, 0x1C1, 0x1C1),
    O36_WEIGHTED(NULL, overlay36TickState, overlay36TickState, 0, 0, 0),
    O36_WEIGHTED(NULL, overlay36UpdatePeers, overlay36UpdatePeers, 0x1FB, 0x1FB, 0x1FB),
    O36_WEIGHTED(overlay36SpawnFinalEffect, overlay36SpawnFinalEffect, overlay36SpawnFinalEffect, 0, 0, 0),
    O36_WEIGHTED(overlay36PrepareAndTick, overlay36PrepareAndTick, overlay36PrepareAndTick, 0, 0, 0),
    O36_WEIGHTED(overlay36SpawnDirectional, overlay36SpawnDirectional, overlay36SpawnConditional, 0x1C0, 0x1C0, 0x20),
    O36_WEIGHTED(overlay36SpawnLinked7F, overlay36SpawnLinked7F, NULL, 0x1C4, 0x1C4, 0x1C4),
    O36_WEIGHTED(overlay36CallModeOne, overlay36CallModeZero, overlay36CallModeZero, 0x1C1, 0x1C1, 0x1C1),
};

const char gOverlay36QueueWarning[] = "WARNING: MAX_POSTPONED_READS too small\n";


/*
 * Overlay 36: one translation unit in ROM order (ADR 0006). The action
 * queue, the state selector and the spawn-and-update helper are static:
 * the entity update's, chooser's and mode wrappers' calls to them are the
 * shipped JUMP records, and they call them by the definitions' names (with
 * casts to the callees' own views). The other unexported functions (the
 * height check, the global call, the positioned and conditional spawns and
 * the weighted chooser) are reached only through the module's data tables,
 * so they stay extern: IDO drops a static function nothing in the TU names.
 */

static void overlay36QueueAction(Overlay1Object *object, s32 index, f32 *x,
                                 f32 *y, f32 *z);
static void overlay36SelectState(Overlay36SelectSource *source, void *unused,
                                 s32 selection, s32 limit);
static void overlay36SpawnAndUpdate(Overlay36Object_SpawnAndUpdate *object,
                                    s32 count);

/* DKR v77/v80 and JFG contain no exact donor for this object initializer. */
void overlay36InitObject(Overlay36Object *object, void *entry) {
    Overlay36Data *data;
    u8 *resource;

    data = object->data;
    resource = func_80028F54();
    _bzero(data, 12);
    data->intensity = 0x80;
    data->marker = -1;

    if (object->type == 0x86 || object->type == 0xEB) {
        if (*resource == 1 || D_8007BF18 == 0) {
            data->enabled = 1;
        } else {
            Overlay36Entry *entry;
            entry = *object->entry;
            object->value = mathRnd(0, entry->value);
        }
    }
    if (object->state != 0) {
        object->state->mode = 2;
    }
}

/* Fixed queue insertion is title-specific; no DKR/JFG donor exists. */
static void overlay36QueueAction(Overlay1Object *object, s32 index, f32 *x, f32 *y, f32 *z) {
    if (gOverlay36QueueCount < 16) {
        gOverlay36QueueNext->object = object;
        gOverlay36QueueNext->index = index;
        gOverlay36QueueNext->x = x;
        gOverlay36QueueNext->y = y;
        gOverlay36QueueNext->z = z;
        gOverlay36QueueNext++;
        gOverlay36QueueCount++;
    }
}

/* Queue flushing is title-specific; DKR v77/v80 and JFG have no donor. */
void overlay36FlushQueue(void) {
    s32 count;

    count = gOverlay36QueueCount;
    while (count--) {
        gOverlay36QueueNext--;
        overlay1ReadSelection(gOverlay36QueueNext->object,
                                  gOverlay36QueueNext->index,
                                  gOverlay36QueueNext->x,
                                  gOverlay36QueueNext->y,
                                  gOverlay36QueueNext->z);
    }
    gOverlay36QueueCount = 0;
    gOverlay36QueueNext = gOverlay36QueueBase;
}

struct Overlay36Found {
    u8 pad00[0x10];
    f32 y10;
    u8 pad14[0x50];
    Overlay36FoundState *state64;
};

struct Overlay36Entity {
    s16 angle0;
    u8 pad02[4];
    s16 flags6;
    f32 scale8;
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0x28];
    u8 *model40;
    u8 pad44[2];
    s16 kind46;
    u8 pad48[8];
    Overlay36Marker *marker50;
    u8 pad54[0x10];
    Overlay36State_UpdateInteractiveEntity *state64;
    void **resource68;
    u8 pad6C[0x14];
    void *update80;
    u8 pad84[0xF];
    u8 modelIndex93;
};

/* Matched (lane c-ovla, 2026-10-07) from the a-ovl3 natural rewrite by:
 * the record base taken at entry; the countdown and timer each copied
 * after their nonzero test, the compare reading the field and the
 * subtraction the copy (the timer's copy is the loop counter `count`, the
 * countdown's a separate `temp` that the marker test reuses, which gives
 * it the save to take v0 ahead of the countdown load); a `hit` local for
 * the found object and a separately named reload of the slot for the
 * state pointer; and found[9], an unobservable length that pays for the
 * extra cell so the frame stays 0x80. */
void overlay36UpdateInteractiveEntity(Overlay36Entity *entity,
                                       s32 elapsed) {
    Overlay36State_UpdateInteractiveEntity *state;
    Overlay36Record *record;
    s32 count;
    s32 kind;
    f32 deltaY;
    s32 animation;
    s32 temp;
    HitCopyState *found[9];
    Overlay36FoundState *foundState;
    Overlay36Found *hit;

    state = entity->state64;
    record = gOverlay36Records;
    gOverlay36Elapsed = elapsed;
    gOverlay36CurrentEntity = entity;
    gOverlay36CurrentState = entity->state64;

    if (state->countdownB != 0) {
        temp = state->countdownB;
        if (elapsed >= state->countdownB) {
            state->countdownB = 0;
            func_80006EA0(entity);
            return;
        }
        if (temp != 0xFF) {
            state->countdownB = temp - elapsed;
        }
    }

    kind = entity->kind46;
    if (D_8007BF0C != 0) {
        if (kind == 0xEA) {
            kind = 0x1B;
        }
        if (kind == 0xEB) {
            kind = 0x86;
        }
    }

    count = 13;
    do {
        if (kind == record->kind0) {
            break;
        }
        record++;
    } while (count--);

    if (state->timer4 != 0) {
        count = state->timer4;
        if (elapsed >= state->timer4) {
            state->timer4 = 0;
        } else {
            state->timer4 = count - elapsed;
        }
        if (record->timerCallback4 != 0) {
            record->timerCallback4(entity, elapsed);
        }
    }

    if (state->value0 != 0) {
        if (state->alphaA < record->targetAlpha3A) {
            state->alphaA += elapsed * 4;
            if (state->alphaA > record->targetAlpha3A) {
                state->alphaA = record->targetAlpha3A;
            }
        } else if (state->alphaA > record->targetAlpha3A) {
            state->alphaA -= elapsed * 4;
            if (state->alphaA < record->targetAlpha3A) {
                state->alphaA = record->targetAlpha3A;
            }
        }
    } else if (state->alphaA < 0x80) {
        state->alphaA += elapsed * 4;
        if (state->alphaA > 0x80) {
            state->alphaA = 0x80;
        }
    }

    entity->scale8 = (*(f32 *)entity->model40 * (f32)state->alphaA) * 0.0078125f;
    if ((s8)entity->model40[entity->modelIndex93 + 0x1E] == 1) {
        animation = 9;
        texAnimateSprite(*entity->resource68, &animation, 0x14,
                              (u8 *)entity + 0x28, elapsed);
    } else {
        entity->angle0 += elapsed << 8;
    }

    if (entity->update80 != 0) {
        partUpdateTriggers(entity, elapsed);
    }

    if (state->usePosition7 != 0) {
        overlay36QueueAction(state->value0, state->positionMode8,
                                    &entity->xC, &entity->y10,
                                    &entity->z14);
        return;
    }

    temp = (s32)entity->marker50;
    if (temp != 0) {
        ((Overlay36Marker *)temp)->active4 = 1;
    }

    if (state->countdownB != 0) {
        return;
    }
    if (entity->flags6 & 0x400) {
        return;
    }
    if (state->timer4 != 0) {
        D_80078F20 = state->value0;
    } else {
        D_80078F20 = 0;
    }

    if (func_8005776C(entity->xC, entity->y10, entity->z14,
                            record->queryRadius2C, 1, found) != 0) {
        hit = (Overlay36Found *)found[0];
        deltaY = hit->y10 - entity->y10;
        if ((record->minimumY30 < deltaY) && (deltaY < record->maximumY34)) {
            /* The target reloads found[0] here instead of reusing hit's
             * register; an s32 read of the slot is a separate IR name, so
             * uopt does not CSE it with hit's load. */
            foundState = ((Overlay36Found *)*(s32 *)&found[0])->state64;
            if (state->flags6 & 2) {
                if (record->alternateCallbackC != 0) {
                    record->alternateCallbackC(hit, entity);
                }
                if (record->alternateEffect1E != 0) {
                    amSndPlayXYZ(record->alternateEffect1E,
                                               entity->xC, entity->y10,
                                               entity->z14, 4, 0);
                }
            } else {
                if (record->normalCallback8 != 0) {
                    record->normalCallback8(hit, entity,
                                            record - gOverlay36Records,
                                            record->callbackArg2);
                }
                if (record->normalEffect1C != 0) {
                    amSndPlayXYZ(record->normalEffect1C,
                                               entity->xC, entity->y10,
                                               entity->z14, 4, 0);
                }
                if (!(record->flags3 & 1)) {
                    foundState->record1A0 = record;
                }
            }
            if ((state->countdownB == 0) && (state->timer4 == 0)) {
                func_80006EA0(entity);
            }
        }
    }
}

Overlay36Spawned *overlay36SpawnTransient(
    Overlay36Position *position, Overlay36World *world) {
    Overlay36SpawnDescriptor desc;
    Overlay36Spawned *spawned;
    Overlay36SpawnState *state;
    volatile s32 framePad[1];

    desc.type = *world->type1A0;
    desc.value2 = 10;
    desc.value3 = 0;
    desc.x = (s16)(s32)position->x;
    desc.y = (s16)(s32)(position->y + 30.0f);
    desc.z = (s16)(s32)position->z;

    spawned = func_8000590C(&desc, 1);
    if (spawned != 0) {
        spawned->value3C = 0;
        state = spawned->state;
        if (B_overlay_001_F00083E4 == 3) {
            state->value4 = 0xF0;
        } else {
            state->value4 = 0x5A;
        }
        if (*world->type1A0 == 0x1B) {
            world->timer192--;
            if (world->timer192 == 0) {
                world->inactive19A = 0xFF;
                world->type1A0 = 0;
            }
        } else {
            world->timer19B--;
            if (world->timer19B == 0) {
                world->inactive19A = 0xFF;
                world->type1A0 = 0;
            }
        }
    }
    return spawned;
}

/* Vector-state initialization is Mickey-specific; no DKR/JFG donor exists. */
void overlay36InitVectorState(Overlay36VectorState *state, s16 kind,
                              s16 value, f32 amount) {
    state->value = value;
    state->amount = -amount;
    state->mode = 0;
    state->kind = kind;
    state->vector[0] = 0.0f;
    state->vector[1] = 0.0f;
    mathOneFloatPY((s16 *)state, state->vector);
    state->nested->flags &= ~2;
}

/* Count nearby objects whose height is outside a 45-unit band around this
 * object's, and latch the world "changed" flag when none are left.
 * PROVENANCE: Mickey-local reconstruction; pinned DKR v77/v80 are negative and
 * JFG's Overlay 36 hits occur only at the unrelated +0x1470/+0x1490 wrappers.
 * No external C is adapted here.
 *
 * 2026-09-10, lane o7-ovl: seven words to three, and the residual is now the
 * frame SIZE and nothing else. Every displacement inside the frame is exact --
 * the results array at sp+60 and the state spill at sp+56 both land on the
 * target's slot -- so the only three differing words are the two `addiu sp`
 * and the incoming-argument home, which is frame+4 and follows from them.
 *
 * The lever was declaration ORDER, an axis no earlier pass swept. cfe lays
 * declared locals from the frame top in declaration order, so a local's home
 * is `frameTop - (bytes declared at or before it)` and the array's base is
 * `frame - 52 - (bytes declared before it)`. All 5,040 orders were compiled:
 * 120 of them score 3, 1,080 score 5, and 3,840 score 7. The 3-word set is
 * exactly the orders with four scalars ahead of the array and `state` sixth,
 * which puts the array at 60 and `state` at 56 while the frame is still 128.
 *
 * That also closes the frame arithmetically. Only six sp displacements are
 * ever touched (16/20 outgoing arguments, 32/36 saves, 56 state, frame+4 the
 * argument home), so every other home is reserved and never read: the frame is
 * pure bookkeeping. It is `round8(40 + block + temps)` with 40 = argument area
 * plus saves, block = 52 + 4 * (scalars declared after the array), and temps
 * the compiler's spill area. The target's 112 needs block + temps = 72, and a
 * seven-name block is 76 on its own. **The target therefore declares at most
 * five scalars beside the array**, and a six-scalar source cannot reach 112 in
 * any order -- which retires the whole ordering axis as a route to the frame
 * and names the next one.
 *
 * Six-name sources bottom out at 120. Dropping any single name frees eight
 * bytes of block but the compiler takes some back as temps, and each drop also
 * moves registers: nearby 20 words, center 11, low 23, nearby+center 22. A
 * five-name source needs temps to be exactly zero. Earlier passes also
 * falsified, all byte-flat or regressing: `register` on all five scratch
 * locals, declaring them in an inner block, an entirely unused extra local
 * (eliminated outright), three top-tested rewrites of the scan loop (frame
 * 128), and every form that drops low/high, which sinks the sub.s/add.s into
 * the loop instead of hoisting them. An explicit `!=` exit test reaches 0x78
 * by dropping the counter but unrolls the loop, adding 224-236 bytes against a
 * zero-delta target. Moving three scratch values out as trailing parameters
 * reaches the exact 112 frame and the exact homes, and fails by exactly two
 * instructions: IDO unconditionally homes a2 and a3 once a function declares
 * three or more parameters, and the target stores only a1 -- which is what
 * proves the target has exactly two parameters.
 *
 * 2026-09-10, lane w8-tu: MATCHED, by shrinking the results array from 13
 * elements to 9. The closure above held every free parameter fixed except the
 * one that is pure reconstruction -- the array's element count. Nothing reads
 * past `results[remaining - 1]`, so its length was never evidence, and the
 * frame identity solves for it directly: with frame = round8(40 + block +
 * temps), block = 4 * scalars + sizeof(results), and the seven-name temp
 * budget of 12 that this handoff itself measured, the target's 0x70 needs
 * block = 60, i.e. a 36-byte array. Nine elements is that array, and it keeps
 * both homes the declaration order already fixed: four scalars ahead of it put
 * its base at 112 - 52 = 60 and the `state` spill at 56. Lengths 8..13 score
 * 7, 0, 4, 3, 7, 3 -- only 9 is flat, which is the arithmetic's own prediction
 * rather than a search. */

static void func_overlay_036_F0000818_1883CD0(void *object, s32 remaining) {
    f32 center;
    f32 low;
    f32 high;
    s32 i;
    HitCopyState *results[9];
    Overlay36State_CheckNearbyHeight *state;
    Overlay36Nearby *nearby;

    state = ((Overlay36Object_CheckNearbyHeight *)object)->state;
    if (state->active == 0) {
        remaining = func_8005776C(((Overlay36Object_CheckNearbyHeight *)object)->x, ((Overlay36Object_CheckNearbyHeight *)object)->y, ((Overlay36Object_CheckNearbyHeight *)object)->z,
                                  64.0f, 1, results);
        if (remaining != 0) {
            high = 45.0f;
            center = ((Overlay36Object_CheckNearbyHeight *)object)->y;
            i = remaining - 1;
            low = center - 45.0f;
            high = low;
            low = center + 45.0f;
            if (remaining != 0) {
                do {
                    nearby = (Overlay36Nearby *)results[i];
                    center = nearby->y;
                    if ((center < high) || (low < center)) {
                        remaining--;
                    }
                } while (i--);
            }
        }
        if (remaining != 0) {
            state->active = 1;
        } else {
            ((Overlay36Object_CheckNearbyHeight *)object)->flags &= ~0x400;
    gOverlay36CurrentState->alphaA = 1;
        }
    }
}

/* Mickey state-table selection has no exact DKR v77/v80 or JFG donor. */
static void overlay36SelectState(Overlay36SelectSource *source, void *unused,
                          s32 selection, s32 limit) {
    Overlay36SelectObject *object;
    u8 *values;

    object = source->object;
    if (selection == object->state) {
        if (object->countdown < limit) {
            object->countdown++;
        }
    } else {
        object->data = &gOverlay36Records[selection];
        values = gOverlay36StateValues[selection];
        object->state = values[0];
        object->countdown = values[1];
        object->timer = 120;
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */

static void overlay36CallGlobal(void *param, void *unused1, s32 unused2, s32 unused3) {
    overlay4AttachObject(param, (Overlay4GroupObject *)gOverlay36CurrentEntity);
}

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
static void overlay36SpawnAtPosition(void *object, void *unused1, s32 unused2, s32 unused3) {
    Overlay36SpawnRequest request;
    Overlay36SpawnedObject *spawned;

    if (D_8007BF0C != 0) {
        request.kind = 0xED;
    } else {
        request.kind = 0x90;
    }
    request.x = ((Overlay36PositionObject *)object)->x;
    request.y = ((Overlay36PositionObject *)object)->y;
    request.z = ((Overlay36PositionObject *)object)->z;
    request.count = 1;
    request.owner = object;
    spawned = func_8000590C(&request, 1);
    if (spawned != 0) {
        spawned->state = 0;
    }
}

/* Matched 2026-10-02. Three edits closed it from 71 words: the overlay 1
 * distance callee takes the object alone (the 5 the old candidate passed it
 * was the random call's own argument, which as1 hoists above the mode
 * branch); the blend is written weight-term first; and both walks are
 * `i = 14; while (i--)`, whose guard block orders the preheader as shipped.
 * `i` is declared ahead of `position` for the position home. */
static void func_overlay_036_F0000A60_1883F18(void *object, void *argument, s32 unused2, s32 unused3) {
    Overlay36Inner *inner;
    s32 total;
    s32 state;
    s32 i;
    s32 position;
    f32 value;
    f32 blend;

    inner = ((Overlay36Object_ChooseWeightedState *)object)->inner;
    total = 0;
    if (inner->countdown == 0) {
        if (B_overlay_001_F00083E4 == 3) {
            state = gOverlay36AlternateStates[mathRnd(0, 5)];
        } else {
            value = ((overlay1DistanceFromCurrent(object) /
                      O36_SCALE_DIVISOR) *
                         O36_SCALE_MULTIPLIER) +
                    ((f32)inner->strength * O36_STRENGTH_MULTIPLIER);

            if (D_8007BF0C != 0) {
                Overlay36Choice *choice;

                choice = func_80028F60(inner->selector, 0);
                if (choice->value >= 0x21) {
                    blend = 32.0f;
                } else {
                    blend = (f32)choice->value;
                }
                blend *= 0.015625f;
                value = (blend * 10.0f) + ((1.0f - blend) * value);
            }

            state = -1;
            position = (s32)value;
            if (position >= 10) {
                position = 9;
            }

            i = 14;
            while (i--) {
                if (D_8007BF18 & (1 << i)) {
                    state = i;
                    total += gOverlay36Weights[i][position];
                }
            }

            if (total >= 2) {
                state = mathRnd(1, total);
                i = 14;
                while (i--) {
                    if (D_8007BF18 & (1 << i)) {
                        state -= gOverlay36Weights[i][position];
                        if (state <= 0) {
                            state = i;
                            break;
                        }
                    }
                }
            }

            if (state == -1) {
                state = 9;
            }
        }
        overlay36SelectState((Overlay36SelectSource *)object, argument, state, 1);
    }

    if (B_overlay_001_F00083E4 == 3) {
        gOverlay36CurrentState->timer4 = 0xF0;
    } else {
        gOverlay36CurrentState->timer4 = 0x3C;
    }
    gOverlay36CurrentEntity->flags6 |= 0x400;
}

/* Title-specific countdown/action dispatch; no DKR v77/v80 or JFG donor. */
void overlay36PrepareAndTick(void *source) {
    Overlay36PrepareObject *object;

    func_overlay_008_F0000F1C_185EC74((Overlay8ActivationOwner *)source, 1);
    object = ((Overlay36PrepareSource *)source)->object;
    object->countdown--;
    if (object->countdown == 0) {
        object->state = 255;
        object->owner = 0;
    }
    if (!(object->flags & 1)) {
        if (object->kind != 6 && object->kind != 7 && object->kind != 8) {
            overlay7CommitSelection(6);
        }
    }
}

void overlay36SpawnLinked7F(void *object) {
    Overlay36State_SpawnLinked7F *state;
    Overlay36Locals0D8C locals;
    s32 actionKind;
    Overlay36SpawnedObject_SpawnLinked7F *spawned;

    state = ((Overlay36Object_SpawnLinked7F *)object)->state;
    locals.offset[0] = 0.0f;
    locals.offset[1] = 16.0f;
    locals.offset[2] = 14.0f;
    mathOneFloatRPY(object, locals.offset);
    locals.request.kind = 0x7F;
    locals.request.x = (s16)(locals.offset[0] + ((Overlay36Object_SpawnLinked7F *)object)->x);
    locals.request.y = (s16)(locals.offset[1] + ((Overlay36Object_SpawnLinked7F *)object)->y);
    locals.request.z = (s16)(locals.offset[2] + ((Overlay36Object_SpawnLinked7F *)object)->z);
    locals.request.id0 = ((Overlay36Object_SpawnLinked7F *)object)->id0;
    locals.request.id2 = ((Overlay36Object_SpawnLinked7F *)object)->id2;
    locals.request.id4 = ((Overlay36Object_SpawnLinked7F *)object)->id4;
    locals.request.owner = object;
    locals.request.directionalObject = overlay1FindDirectionalObject(
        object, 3, 5, 0.707f, O36_DIRECTION_MAX);
    actionKind = state->action->kind;
    spawned = func_8000590C(&locals.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            actionKind = 0xD;
        }
    }
    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)actionKind, ((Overlay36Object_SpawnLinked7F *)object)->x,
                                     ((Overlay36Object_SpawnLinked7F *)object)->y, ((Overlay36Object_SpawnLinked7F *)object)->z, 4,
                                     &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnDirectional(void *object) {
    Overlay36State_SpawnDirectional *state;
    Overlay36SpawnedObject_SpawnDirectional *spawned;
    Overlay36SpawnDataF20 data;
    s32 actionKind;

    state = ((Overlay36Object_SpawnDirectional *)object)->state;
    data.offset[0] = 0.0f;
    data.offset[1] = 16.0f;
    data.offset[2] = 14.0f;
    mathOneFloatRPY(object, data.offset);
    data.request.kind = 0x9F;
    data.request.x = (s16)(data.offset[0] + ((Overlay36Object_SpawnDirectional *)object)->x);
    data.request.y = (s16)(data.offset[1] + ((Overlay36Object_SpawnDirectional *)object)->y);
    data.request.z = (s16)(data.offset[2] + ((Overlay36Object_SpawnDirectional *)object)->z);
    data.request.count = 0;
    data.request.link = 0x21;
    data.request.value = -45.0f;
    data.request.owner = object;
    actionKind = state->action->kind;
    spawned = func_8000590C(&data.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            actionKind = 0xD;
        }
    }
    if (actionKind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)actionKind, ((Overlay36Object_SpawnDirectional *)object)->x, ((Overlay36Object_SpawnDirectional *)object)->y,
                                     ((Overlay36Object_SpawnDirectional *)object)->z, 4, &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

static void overlay36SpawnConditional(void *object) {
    Overlay36State_SpawnConditional *state;
    Overlay36SpawnedObject_SpawnConditional *spawned;
    Overlay36SpawnDataF20 data;
    s32 actionKind;

    state = ((Overlay36Object_SpawnConditional *)object)->state;
    data.offset[0] = 0.0f;
    data.offset[1] = 16.0f;
    if (state->direction <= 0.0f) {
        data.offset[2] = 0.0f;
        data.request.value = 45.0f;
    } else {
        data.offset[2] = 14.0f;
        data.request.value = -45.0f;
    }
    mathOneFloatRPY(object, data.offset);
    data.request.kind = 0x9F;
    data.request.x = (s16)(data.offset[0] + ((Overlay36Object_SpawnConditional *)object)->x);
    data.request.y = (s16)(data.offset[1] + ((Overlay36Object_SpawnConditional *)object)->y);
    data.request.z = (s16)(data.offset[2] + ((Overlay36Object_SpawnConditional *)object)->z);
    data.request.count = 2;
    data.request.link = 0;
    data.request.owner = (void *)object;
    actionKind = state->action->kind;
    spawned = func_8000590C(&data.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            actionKind = 0xD;
        }
    }
    if (actionKind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)actionKind, ((Overlay36Object_SpawnConditional *)object)->x, ((Overlay36Object_SpawnConditional *)object)->y,
                                     ((Overlay36Object_SpawnConditional *)object)->z, 4, &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnOffsetA9(void *object) {
    Overlay36SpawnedObject_SpawnOffsetA9 *spawned;
    Overlay36State_SpawnOffsetA9 *state;
    Overlay36Locals1214 locals;

    state = ((Overlay36Object_SpawnOffsetA9 *)object)->state;
    locals.offset[0] = 0.0f;
    locals.offset[1] = 25.0f;
    locals.offset[2] = 14.0f;
    mathOneFloatRPY(object, locals.offset);
    locals.request.kind = 0xA9;
    locals.request.x = (s16)(locals.offset[0] + ((Overlay36Object_SpawnOffsetA9 *)object)->x);
    locals.request.y = (s16)(locals.offset[1] + ((Overlay36Object_SpawnOffsetA9 *)object)->y);
    locals.request.z = (s16)(locals.offset[2] + ((Overlay36Object_SpawnOffsetA9 *)object)->z);
    locals.request.id0 = ((Overlay36Object_SpawnOffsetA9 *)object)->id0;
    locals.request.id2 = ((Overlay36Object_SpawnOffsetA9 *)object)->id2;
    locals.request.owner = object;
    locals.actionKind = state->action->kind;
    spawned = func_8000590C(&locals.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            locals.actionKind = 0xD;
        }
    }
    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)locals.actionKind, ((Overlay36Object_SpawnOffsetA9 *)object)->x,
                                     ((Overlay36Object_SpawnOffsetA9 *)object)->y, ((Overlay36Object_SpawnOffsetA9 *)object)->z, 4,
                                     &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

static void overlay36SpawnAndUpdate(Overlay36Object_SpawnAndUpdate *object, s32 count) {
    Overlay36SpawnRequest_SpawnAndUpdate request;
    Overlay36SpawnedObject *spawned;
    Overlay36State_SpawnAndUpdate *state;

    state = object->state;
    request.kind = 0x82;
    request.x = (s16)object->x;
    request.y = (s16)object->y;
    request.z = (s16)object->z;
    request.count = count;
    request.owner = object;
    spawned = func_8000590C(&request, 1);
    if (spawned != 0) {
        spawned->state = 0;
    }

    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ(state->action->kind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

/* Generic mode wrappers only; no exact DKR donor. */

void overlay36CallModeZero(void *object) { overlay36SpawnAndUpdate(object, 0); }
void overlay36CallModeOne(void *object) { overlay36SpawnAndUpdate(object, 1); }

/* State countdown semantics are title-specific; no DKR/JFG donor exists. */
void overlay36TickState(void *source) {
    Overlay36TickObject *object;

    object = ((Overlay36TickSource *)source)->object;
    object->timer = 600;
    object->countdown--;
    if (object->countdown == 0) {
        object->state = 255;
        object->owner = 0;
    }
    if (!(object->flags & 1)) {
        overlay7CommitSelection(7);
    }
}

#ifndef OVERLAY36_PEER_EFFECT_KIND
#define OVERLAY36_PEER_EFFECT_KIND 0xD7
#endif

typedef struct Overlay36State_UpdatePeers Overlay36State_UpdatePeers;
typedef struct Overlay36Object_UpdatePeers Overlay36Object_UpdatePeers;
typedef struct Overlay36SpawnedObject_UpdatePeers Overlay36SpawnedObject_UpdatePeers;

struct Overlay36State_UpdatePeers {
    u8 pad000[0xA8];
    void *resource;
    u8 pad0AC[0x28];
    Overlay36SpawnedObject_UpdatePeers *peerResource;
    u8 pad0D8[0x92];
    s16 timer;
    u8 pad16C[0x2E];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    Overlay36Action *action;
    u8 pad1A4[0x1E1];
    u8 rank;
};

struct Overlay36Object_UpdatePeers {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State_UpdatePeers *state;
};

struct Overlay36SpawnedObject_UpdatePeers {
    u8 pad00[0x3C];
    s32 state;
};

void overlay36UpdatePeers(void *object) {
    Overlay36Object_UpdatePeers *peer;
    Overlay36SpawnRequest_UpdatePeers request;
    Overlay36State_UpdatePeers *peerState;
    Overlay36State_UpdatePeers *state;
    s32 index;

    state = ((Overlay36Object_UpdatePeers *)object)->state;
    index = 0;
    while ((peer = func_80005820(index++)) != 0) {
        if (peer != object) {
            peerState = peer->state;
            if (peerState->rank < state->rank && peerState->timer < 0x40 &&
                overlay27Activate(peerState->peerResource) == 0) {
                request.kind = OVERLAY36_PEER_EFFECT_KIND;
                request.x = (s16)peer->x;
                request.y = (s16)peer->y;
                request.z = (s16)peer->z;
                request.owner = (void *)peer;
                peerState->peerResource = func_8000590C(&request, 0);
                if (peerState->peerResource != 0) {
                    peerState->peerResource->state = 0;
                }
            }
        }
    }

    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ(state->action->kind, ((Overlay36Object_UpdatePeers *)object)->x, ((Overlay36Object_UpdatePeers *)object)->y,
                                     ((Overlay36Object_UpdatePeers *)object)->z, 4, &state->resource);
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnFinalEffect(void *source) {
    Overlay36SpawnRequest_SpawnFinalEffect request;
    register Overlay36EffectState *state;

    state = ((Overlay36EffectSource *)source)->effectState;
    if (D_8007BF0C != 0) {
        request.effectType = 0xED;
    } else {
        request.effectType = 0x90;
    }
    request.x = (s16)((Overlay36EffectSource *)source)->x;
    request.y = (s16)((Overlay36EffectSource *)source)->y;
    request.z = (s16)((Overlay36EffectSource *)source)->z;
    request.mode = 3;
    request.source = source;
    request.effectState = state;

    source = (Overlay36EffectSource *)func_8000590C(&request, 1);
    state = request.effectState;
    if (source != NULL) {
        ((Overlay36SpawnedObject_SpawnFinalEffect *)source)->owner = 0;
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->activeAction = NULL;
    }
}
