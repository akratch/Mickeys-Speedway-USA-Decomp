#include "overlays/overlay_070.h"

void func_overlay_070_F0000000_18C91C8(O70Object *object, O70Input *input) {
    O70State *state;
    u8 type;

    state = object->state;
    state->related = input->related;
    state->type = input->type;
    state->angle = mathRnd(-0x8000, 0x7FFF);
    if (mathRnd(0, 1) == 0) {
        state->angleStep = mathRnd(-0x200, -0x180);
    } else {
        state->angleStep = mathRnd(0x180, 0x200);
    }
    state->countdown = 0;
    state->active = 0;
    state->timer = 0xBE;
    state->flags = 0xD;
    object->value28 = gOverlay70FloatTableReloc.values[state->type];
    type = state->type;
    state->verticalStep = gOverlay70VerticalStepReloc.values[type];
    object->phase = gOverlay70AngleReloc.values[type];
}

void func_overlay_070_F00000D8_18C92A0(O70Object *object, s32 ticks) {
    O70State *state;
    O70Object *related;
    O70RelatedState *relatedState;
    O70Pair *pair;
    f32 sine;
    f32 cosine;
    f32 pairX;
    f32 pairZ;
    s32 angle;

    state = object->state;
    related = state->related;
    relatedState = (O70RelatedState *)related->state;

    if (state->active != 0) {
        object->y += (f32)ticks;
        if (state->threshold < object->y) {
            if ((ticks * 8) < state->timer) {
                state->timer -= ticks * 8;
            } else {
                func_80006EA0(object);
            }
        }
    } else {
        if (state->countdown != 0) {
            state->countdown -= ticks;
            if (state->countdown < 0) {
                state->countdown = 0;
            }
        } else {
            angle = state->angle;
            angle += state->angleStep * ticks;
            if (angle >= 0x8000) {
                state->angle = 0x7FFF;
                state->angleStep = mathRnd(-0x200, -0x100);
                state->countdown = mathRnd(0, 0x3C);
            } else if (angle < -0x8000) {
                state->angle = -0x8000;
                state->angleStep = mathRnd(0x100, 0x200);
                state->countdown = mathRnd(0, 0x3C);
            } else {
                state->angle = angle;
            }
        }

        sine = func_8002A8BC(-related->angle);
        cosine = func_8002A8C0(-related->angle);
        pair = (O70Pair *)&gOverlay70PairTableReloc[state->type * 2];
        pairX = (f32)pair->x;
        pairZ = (f32)pair->z;
        object->x = (related->x + (pairX * sine)) - (pairZ * cosine);
        object->y = (f32)gOverlay70HeightTableReloc[state->type + 12] +
                    (related->y + 20.0f) + relatedState->verticalOffset;
        object->z = related->z + (pairZ * sine) + (pairX * cosine);
        object->facing = related->facing;
    }

    object->phase += state->verticalStep * ticks;
    angle = object->phase;
    if (angle >= 0x301) {
        object->phase = 0x600 - angle;
        state->verticalStep = -state->verticalStep;
    } else if (angle < -0x300) {
        object->phase = -0x600 - angle;
        state->verticalStep = -state->verticalStep;
    }
    texAnimateSprite(*object->targetList, &state->flags, 2, &object->value28,
                   ticks);
}

void func_overlay_070_F0000384_18C954C(O70Object *object) {
    O70Object **objects;
    O70Object **allObjects;
    O70Object *entry;
    s32 start;
    s32 end;
    O70State *state;
    O70State *best;
    s32 count;
    O70ManagerState *entryState;
    O70Record *records;
    s32 bestType;
    s32 i;
    O70ManagerState *manager;
    s32 maximum;
    s32 inactiveCount;

    objects = func_8000572C(&start, &end);
    bestType = 0x80;
    best = 0;
    manager = (O70ManagerState *)object->state;
    for (i = start; i < end; i++) {
        entry = objects[i];
        if (entry->kind == 0x50) {
            state = entry->state;
            if (state->related == object && state->active == 0 &&
                state->type < bestType) {
                bestType = state->type;
                best = state;
            }
        }
    }

    if (best != 0) {
        best->active = 1;
        best->threshold = object->y + 200.0f;
    }

    if (manager->mode3B8 == 3) {
        records = func_80028F54();
        manager->registered3FA = 1;
        object->control->flags &= ~1;
        records[manager->index].metric = manager->metric400;
        if (!(manager->flags1A8 & 1)) {
            gOverlay70SharedCounterReloc--;
        }

        allObjects = func_80005750(&count);
        maximum = 0;
        inactiveCount = 0;
        i = count - 1;
        if (count != 0) {
            do {
                entry = allObjects[i];
                entryState = (O70ManagerState *)entry->state;
                if (entryState->registered3FA != 0) {
                    if (maximum < entryState->metric400) {
                        maximum = entryState->metric400;
                    }
                } else {
                    inactiveCount++;
                }
            } while (i--);
        }

        if (((gOverlay70SharedCounterReloc == 0) &&
             (manager->flags1A8 & 1)) || inactiveCount == 1) {
            if (gOverlay70SharedCounterReloc == 0) {
                func_8003A55C(6);
            } else {
                func_8003A55C(5);
            }

            i = count - 1;
            if (count != 0) {
                do {
                    entry = allObjects[i];
                    entryState = (O70ManagerState *)entry->state;
                    if (entryState->registered3FA == 0) {
                        entryState->flags1A8 |= 1;
                    }
                } while (i--);
            }
            gOverlay70SharedCounterReloc = 0;
        }
    } else {
        manager->timer168 = 0x78;
    }
}
