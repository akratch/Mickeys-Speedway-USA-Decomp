#include "overlays/overlay_036.h"

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

static void overlay36QueueAction(s32 action, s32 param1, s32 param2,
                                 s32 param3, s32 param4);
static void overlay36SelectState(Overlay36SelectSource *source, s32 unused,
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
        if (*resource == 1 || *(u16 *)&gOverlay36Mode == 0) {
            data->enabled = 1;
        } else {
            Overlay36Entry *entry;
            entry = *object->entry;
            object->value = mathRnd_o036Reloc(0, entry->value, data);
        }
    }
    if (object->state != 0) {
        object->state->mode = 2;
    }
}

/* Fixed queue insertion is title-specific; no DKR/JFG donor exists. */
static void overlay36QueueAction(s32 action, s32 param1, s32 param2, s32 param3, s32 param4) {
    if (gOverlay36QueueCount < 16) {
        gOverlay36QueueNext->arg0 = action;
        gOverlay36QueueNext->arg1 = param1;
        gOverlay36QueueNext->arg2 = param2;
        gOverlay36QueueNext->arg3 = param3;
        gOverlay36QueueNext->arg4 = param4;
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
        overlay1ReadSelection(gOverlay36QueueNext->arg0,
                                  gOverlay36QueueNext->arg1,
                                  gOverlay36QueueNext->arg2,
                                  gOverlay36QueueNext->arg3,
                                  gOverlay36QueueNext->arg4);
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
    Overlay36Found *found[9];
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
            func_80006EA0();
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
                                    (s32)&entity->xC, (s32)&entity->y10,
                                    (s32)&entity->z14);
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

    if (func_8005776C_o036Reloc(entity->xC, entity->y10, entity->z14,
                            record->queryRadius2C, 1, found) != 0) {
        hit = found[0];
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
                overlay36ReleaseEntityReloc(entity);
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

    spawned = func_8000590C(&desc, 1, world);
    if (spawned != 0) {
        spawned->value3C = 0;
        state = spawned->state;
        if (gOverlay36Mode == 3) {
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
    mathOneFloatPY(state, state->vector);
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

void func_overlay_036_F0000818_1883CD0(Overlay36Object_CheckNearbyHeight *object,
                                       s32 remaining) {
    f32 center;
    f32 low;
    f32 high;
    s32 i;
    Overlay36Nearby *results[9];
    Overlay36State_CheckNearbyHeight *state;
    Overlay36Nearby *nearby;

    state = object->state;
    if (state->active == 0) {
        remaining = func_8005776C(object->x, object->y, object->z,
                                  64.0f, 1, results);
        if (remaining != 0) {
            high = 45.0f;
            center = object->y;
            i = remaining - 1;
            low = center - 45.0f;
            high = low;
            low = center + 45.0f;
            if (remaining != 0) {
                do {
                    nearby = results[i];
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
            object->flags &= ~0x400;
    ((Overlay36WorldState *)gOverlay36WorldStateReloc[0x54])->changed = 1;
        }
    }
}

/* Mickey state-table selection has no exact DKR v77/v80 or JFG donor. */
static void overlay36SelectState(Overlay36SelectSource *source, s32 unused,
                          s32 selection, s32 limit) {
    Overlay36SelectObject *object;
    u8 *values;

    object = source->object;
    if (selection == object->state) {
        if (object->countdown < limit) {
            object->countdown++;
        }
    } else {
        object->data = &gOverlay36StateData[selection * 60];
        values = &gOverlay36StateValues[selection * 2];
        object->state = values[0];
        object->countdown = values[1];
        object->timer = 120;
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */

void overlay36CallGlobal(s32 param, s32 unused1, s32 unused2, s32 unused3) {
    overlay4AttachObject(param, gOverlay36CallTarget);
}

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
void overlay36SpawnAtPosition(Overlay36PositionObject *object, s32 unused1,
                              s32 unused2, s32 unused3) {
    Overlay36SpawnRequest request;
    Overlay36SpawnedObject *spawned;

    if (D_8007BF0C != 0) {
        request.kind = 0xED;
    } else {
        request.kind = 0x90;
    }
    request.x = object->x;
    request.y = object->y;
    request.z = object->z;
    request.count = 1;
    request.owner = object;
    spawned = func_8000590C(&request, 1, object);
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
void func_overlay_036_F0000A60_1883F18(Overlay36Object_ChooseWeightedState *object, s32 argument,
                                       volatile s32 unused2,
                                       volatile s32 unused3) {
    Overlay36Inner *inner;
    s32 total;
    s32 state;
    s32 i;
    s32 position;
    f32 value;
    f32 blend;

    inner = object->inner;
    total = 0;
    if (inner->countdown == 0) {
        if (gOverlay36Mode == 3) {
            state = gOverlay36AltTable.values[mathRnd(0, 5)];
        } else {
            value = ((overlay1DistanceFromCurrent(object) /
                      gOverlay36ScaleDivisor.value) *
                         gOverlay36ScaleMultiplier.value) +
                    ((f32)inner->strength * gOverlay36StrengthMultiplier.value);

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
                if (gOverlay36EnabledMask & (1 << i)) {
                    state = i;
                    total += gOverlay36Weights[i][position];
                }
            }

            if (total >= 2) {
                state = mathRnd(1, total, state, position);
                i = 14;
                while (i--) {
                    if (gOverlay36EnabledMask & (1 << i)) {
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

    if (gOverlay36Mode == 3) {
        gOverlay36NodeA.node->value4 = 0xF0;
    } else {
        gOverlay36NodeA.node->value4 = 0x3C;
    }
    gOverlay36NodeB.node->flags6 |= 0x400;
}

/* Title-specific countdown/action dispatch; no DKR v77/v80 or JFG donor. */
void overlay36PrepareAndTick(Overlay36PrepareSource *source) {
    Overlay36PrepareObject *object;

    overlay36ActionReloc(source, 1);
    object = source->object;
    object->countdown--;
    if (object->countdown == 0) {
        object->state = 255;
        object->owner = 0;
    }
    if (!(object->flags & 1)) {
        if (object->kind != 6 && object->kind != 7 && object->kind != 8) {
            overlay36ActionReloc(6);
        }
    }
}

void overlay36SpawnLinked7F(Overlay36Object_SpawnLinked7F *object) {
    Overlay36State_SpawnLinked7F *state;
    Overlay36Locals0D8C locals;
    s32 actionKind;
    Overlay36SpawnedObject_SpawnLinked7F *spawned;

    state = object->state;
    locals.offset[0] = 0.0f;
    locals.offset[1] = 16.0f;
    locals.offset[2] = 14.0f;
    mathOneFloatRPY(object, locals.offset);
    locals.request.kind = 0x7F;
    locals.request.x = (s16)(locals.offset[0] + object->x);
    locals.request.y = (s16)(locals.offset[1] + object->y);
    locals.request.z = (s16)(locals.offset[2] + object->z);
    locals.request.id0 = object->id0;
    locals.request.id2 = object->id2;
    locals.request.id4 = object->id4;
    locals.request.owner = object;
    locals.request.directionalObject = overlay1FindDirectionalObject(
        object, 3, 5, 0.707f, gOverlay36RodataBaseReloc[13]);
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
        amSndPlayXYZ((u16)actionKind, object->x,
                                     object->y, object->z, 4,
                                     &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnDirectional(Overlay36Object_SpawnDirectional *object) {
    Overlay36State_SpawnDirectional *state;
    Overlay36SpawnedObject_SpawnDirectional *spawned;
    Overlay36SpawnDataF20 data;
    s32 actionKind;

    state = object->state;
    data.offset[0] = 0.0f;
    data.offset[1] = 16.0f;
    data.offset[2] = 14.0f;
    mathOneFloatRPY(object, data.offset);
    data.request.kind = 0x9F;
    data.request.x = (s16)(data.offset[0] + object->x);
    data.request.y = (s16)(data.offset[1] + object->y);
    data.request.z = (s16)(data.offset[2] + object->z);
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
        amSndPlayXYZ((u16)actionKind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnConditional(Overlay36Object_SpawnConditional *object) {
    Overlay36State_SpawnConditional *state;
    Overlay36SpawnedObject_SpawnConditional *spawned;
    Overlay36SpawnDataF20 data;
    s32 actionKind;

    state = object->state;
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
    data.request.x = (s16)(data.offset[0] + object->x);
    data.request.y = (s16)(data.offset[1] + object->y);
    data.request.z = (s16)(data.offset[2] + object->z);
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
        amSndPlayXYZ((u16)actionKind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnOffsetA9(Overlay36Object_SpawnOffsetA9 *object) {
    Overlay36SpawnedObject_SpawnOffsetA9 *spawned;
    Overlay36State_SpawnOffsetA9 *state;
    Overlay36Locals1214 locals;

    state = object->state;
    locals.offset[0] = 0.0f;
    locals.offset[1] = 25.0f;
    locals.offset[2] = 14.0f;
    mathOneFloatRPY(object, locals.offset);
    locals.request.kind = 0xA9;
    locals.request.x = (s16)(locals.offset[0] + object->x);
    locals.request.y = (s16)(locals.offset[1] + object->y);
    locals.request.z = (s16)(locals.offset[2] + object->z);
    locals.request.id0 = object->id0;
    locals.request.id2 = object->id2;
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
        amSndPlayXYZ((u16)locals.actionKind, object->x,
                                     object->y, object->z, 4,
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
void overlay36TickState(Overlay36TickSource *source) {
    Overlay36TickObject *object;

    object = source->object;
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

void overlay36UpdatePeers(Overlay36Object_UpdatePeers *object) {
    Overlay36Object_UpdatePeers *peer;
    Overlay36SpawnRequest_UpdatePeers request;
    Overlay36State_UpdatePeers *peerState;
    Overlay36State_UpdatePeers *state;
    s32 index;

    state = object->state;
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
        amSndPlayXYZ(state->action->kind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}

void overlay36SpawnFinalEffect(Overlay36EffectSource *source) {
    Overlay36SpawnRequest_SpawnFinalEffect request;
    register Overlay36EffectState *state;

    state = source->effectState;
    if (D_8007BF0C != 0) {
        request.effectType = 0xED;
    } else {
        request.effectType = 0x90;
    }
    request.x = (s16)source->x;
    request.y = (s16)source->y;
    request.z = (s16)source->z;
    request.mode = 3;
    request.source = source;
    request.effectState = state;

    source = (Overlay36EffectSource *)func_8000590C(&request, 1, source);
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
