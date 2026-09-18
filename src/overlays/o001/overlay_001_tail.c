#include "overlays/overlay_001.h"

/* ---- overlay1InitializeGaugeObjects ---- */


typedef struct O1GaugeState {
    s8 type; u8 pad001[0x1A7]; u16 flags; u8 pad1AA[0x250];
    s16 enabled; u8 pad3FC[4]; s32 value;
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeTableEntry { u8 pad00[8]; s32 value; u8 pad0C[0x1C]; } O1GaugeTableEntry;
extern O1GaugeTableEntry *overlay1GetGaugeTable(void);
extern O1GaugeObject **overlay1GetGaugeObjects(s32 *count);
extern s32 overlay1RandomRange(s32 minimum, s32 maximum);

/* Two identities close this: the object list is indexed, not walked with a
 * hand-written cursor -- IDO's own strength reduction is what produces the
 * second live base register the target keeps across the first loop -- and the
 * count is read once for the countdown and the emptiness test and again after
 * the first loop, which needs a plain and a volatile member at one address.
 * Neither half matches alone (8 and 14 words); the pair is exact. */
typedef union O1GaugeCount { s32 shared; volatile s32 reload; } O1GaugeCount;

void overlay1InitializeGaugeObjects(void) {
    O1GaugeTableEntry *table;
    O1GaugeCount count;
    O1GaugeObject **objects;
    O1GaugeObject *object;
    O1GaugeState *state;
    s32 initialIndex;
    s32 index;
    s32 loopValue;
    s32 maximum;

    table = overlay1GetGaugeTable();
    objects = overlay1GetGaugeObjects(&count.shared);
    maximum = 0;
    initialIndex = count.shared - 1;
    index = initialIndex;
    if (count.shared != 0) {
        do {
            object = objects[index];
            state = object->state;
            if ((state->enabled != 0) && (maximum < state->value)) maximum = state->value;
            loopValue = index;
            index--;
        } while (loopValue != 0);
        index = initialIndex;
    }
    if (count.reload != 0) {
        do {
            object = objects[index];
            state = object->state;
            state->flags |= 1;
            if (table[state->type].value == 0) {
                table[state->type].value = overlay1RandomRange(100, 1000) + maximum;
            }
            loopValue = index;
            index--;
        } while (loopValue != 0);
    }
}

/* ---- overlay1AssignRecordIndex ---- */


typedef struct O1VariableRecord { s16 type; u8 size; u8 pad03[9]; u16 index; } O1VariableRecord;
typedef struct O1RecordOwner { u8 pad00[0xC]; u16 index; } O1RecordOwner;
extern s32 D_1D8C;
extern void overlay1GetRomlistInfoReloc(O1VariableRecord **records, s32 *length,
                                        s32 enabled);

/* newIndex is the decoded record value; next is the candidate global maximum.
 * Keeping those roles distinct leaves a redundant assignment after optimization
 * but preserves IDO's shipped allocation. See docs/cleanup-queue.md. */
void overlay1AssignRecordIndex(s32 unused, O1RecordOwner *owner) {
    O1VariableRecord *records;
    O1VariableRecord *record;
    s32 *recordIndex;
    s32 offset;
    s32 next;
    s32 length;
    u8 size;

    if (owner->index == 0xFFFF) {
        s32 newIndex;

        overlay1GetRomlistInfoReloc(&records, &length, 1);
        offset = 0;
        record = records;
        if (length > 0) {
            do {
                if (record->type == 0xCA) {
                    recordIndex = &D_1D8C;
                    newIndex = record->index + 1;
                    next = newIndex;
                    if (*recordIndex < next) D_1D8C = next;
                }
                size = record->size;
                offset += size;
                record = (O1VariableRecord *)((u8 *)record + size);
            } while (offset < length);
        }
        owner->index = (u16)D_1D8C;
    }
}

/* ---- overlay1ChoosePath ---- */


typedef struct O1ChoiceState {
    s8 playerIndex;
    s8 relationIndex;
    u8 pad002[6];
    u16 objectValue;
    u16 scoreIndex;
    u8 pad00C[0x18E];
    u8 mode;
    u8 active;
    u8 pad19C[0x0C];
    u16 flags;
    u8 pad1AA[0x1D4];
    u8 selector;
    u8 previousSelector;
    u8 transition;
    u8 pad381[0x1B];
    f32 progress;
    u8 pad3A0[8];
    u8 relationModes[8];
} O1ChoiceState;

typedef struct O1ChoiceObject {
    u8 pad000[0x64];
    O1ChoiceState *state;
} O1ChoiceObject;

typedef struct O1ControlPoint {
    f32 x;
    f32 z;
    u8 pad008[4];
    s8 enabled;
    u8 pad00D[3];
} O1ControlPoint;

typedef struct O1ControlTable {
    u8 pad000[0x10];
    u16 flags;
    u8 pad012[2];
    O1ControlPoint points[8];
} O1ControlTable;

typedef struct O1Pair {
    f32 distance;
    f32 value;
    s32 valid;
} O1Pair;

typedef struct Overlay1ValueEntry {
    f32 value;
    u8 pad04[8];
} Overlay1ValueEntry;

typedef struct Overlay1ValueRow {
    Overlay1ValueEntry entries[6];
} Overlay1ValueRow;

extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D68Read;
extern O1ControlTable *D_1D6C;
extern O1ControlPoint *D_208;
extern O1ControlPoint *D_20C;
extern O1ControlPoint *D_210;
extern O1ControlPoint *D_214;
extern s32 D_1D94;
extern f32 D_E8;
extern f32 D_EC;
extern f32 D_F0;
extern f32 D_F4;
extern Overlay1ValueRow D_1BA8[];

extern f32 overlay1RandomWave(s32 value);
extern O1ChoiceObject **overlay1GetChoiceObjects(s32 *count);
extern void overlay1SubmitChoice(O1ChoiceObject *object);
extern void overlay1InterpolatePath(f32 *outX, f32 *outZ, s32 path,
                                    f32 offset);
extern O1ChoiceObject *overlay1FindChoice(f32 progress,
                                         O1ControlTable *table,
                                         s32 minimum, s32 *scores);
extern f32 overlay1MeasureChoice(f32 first, f32 second);

#define CHOICE_WORLD ((O1ChoiceState *)D_1DA0)

/* Workbench plateau: 447/446 instructions, exact 0x90 frame, 341 masked/346 raw
 * differences, and first mismatch at +0xC.  Removing the redundant path carrier
 * changes only allocation and improves 19 positional words; the linked trial is
 * still 12 bytes long.  Target score and interpolation locals sit higher in the
 * same frame; the flag lattice and reverse score cursor remain eliminated. */
#ifdef NON_MATCHING
void func_overlay_001_F0003750_184FB30(f32 *outX, f32 *outZ) {
    O1ChoiceState *otherState;
    O1ChoiceState *chosenState;
    O1ChoiceObject **objects;
    O1ChoiceObject **cursor;
    O1ChoiceObject *object;
    O1ChoiceObject *found;
    O1ControlTable *table;
    s32 i;
    s32 scores[8];
    s32 selected;
    s32 loopValue;
    s32 step;
    s32 value;
    f32 weight;
    f32 difference;
    f32 temporaryX;
    f32 temporaryZ;

    if (CHOICE_WORLD->transition != 0) {
        weight = (overlay1RandomWave((CHOICE_WORLD->transition << 7) + 0x8000) +
                  1.0f) * 0.5f;
        overlay1InterpolatePath(outX, outZ, CHOICE_WORLD->previousSelector, 0.5f);
        overlay1InterpolatePath(&temporaryX, &temporaryZ, CHOICE_WORLD->selector,
                                0.5f);
        *outX = ((*outX - temporaryX) * weight) + temporaryX;
        *outZ = ((*outZ - temporaryZ) * weight) + temporaryZ;
        value = D_1D94 * 8;
        if (CHOICE_WORLD->transition >= value) {
            CHOICE_WORLD->transition -= value;
        } else {
            CHOICE_WORLD->transition = 0;
        }
        return;
    }

    if (((O1ControlTable *)D_1D64)->flags & 1) {
        table = D_1D68;
        if (table->points[CHOICE_WORLD->selector].enabled == 0) {
            i = 7;
            do {
                if (table->points[i].enabled != 0 &&
                    i != CHOICE_WORLD->selector &&
                    table->points[CHOICE_WORLD->selector].x == table->points[i].x &&
                    table->points[CHOICE_WORLD->selector].z == table->points[i].z) {
                    CHOICE_WORLD->previousSelector = i;
                    CHOICE_WORLD->selector = i;
                    CHOICE_WORLD->transition = 0;
                    break;
                }
                loopValue = i;
                i--;
            } while (loopValue != 0);
        }
        overlay1InterpolatePath(outX, outZ, CHOICE_WORLD->selector, 0.5f);
        return;
    }

    i = 7;
    do {
        difference = (f32)(i - CHOICE_WORLD->selector);
        if (difference < 0.0f) difference = -difference;
        scores[i] = (s32)(48.0f - difference * 6.0f);
        loopValue = i;
        i--;
    } while (loopValue != 0);

    objects = overlay1GetChoiceObjects(&i);
    if (i-- != 0) {
        cursor = objects + i;
        do {
            object = *cursor--;
            otherState = object->state;
            if (otherState != CHOICE_WORLD && !(otherState->flags & 8)) {
                difference =
                    ((O1Pair (*)[6])D_1BA8)[CHOICE_WORLD->playerIndex]
                                                [otherState->playerIndex].value;
                if (((difference > -2.0f) && (difference < 2.0f)) ||
                    ((D_E8 < difference) && (difference < 0.0f) &&
                     (CHOICE_WORLD->relationModes[otherState->relationIndex] == 1))) {
                    value = otherState->selector;
                    step = 1;
                    if (value < CHOICE_WORLD->selector) step = -1;
                    do {
                        if (difference > 0.0f) {
                            weight = difference;
                        } else {
                            weight = -difference;
                        }
                        scores[value] =
                            (s32)((f32)scores[value] -
                                  ((2.0f - weight) * 64.0f));
                        value += step;
                    } while (value >= 0 && value < 8);
                }
                if ((difference > -2.0f) && (difference < 0.0f) &&
                    CHOICE_WORLD->relationModes[otherState->relationIndex] >= 4) {
                    scores[otherState->selector] =
                        (s32)((f32)scores[otherState->selector] +
                              D_EC);
                }
            }
            loopValue = i;
            i--;
        } while (loopValue != 0);
    }

    table = D_1D68Read;
    i = 7;
    do {
        scores[i] += table->points[i].enabled;
        loopValue = i;
        i--;
    } while (loopValue != 0);

    i = 7;
    do {
        if (table->points[i].enabled == 0) scores[i] = -1000000;
        loopValue = i;
        i--;
    } while (loopValue != 0);

    value = -1000000;
    if (CHOICE_WORLD->active != 0 && CHOICE_WORLD->mode == 6) {
        found = overlay1FindChoice(CHOICE_WORLD->progress, table, value, scores);
        if (found != 0) {
            chosenState = found->state;
            weight = (f32)chosenState->objectValue * D_F0;
            difference = overlay1MeasureChoice(weight, CHOICE_WORLD->progress);
            if (difference < D_F4) {
                overlay1SubmitChoice(D_1D9C);
            } else if (difference < 3.0f) {
                scores[chosenState->scoreIndex] += 1000;
            }
        }
    }

    selected = -1;
    i = 0;
    do {
        if (value < scores[i]) {
            selected = i;
            value = scores[i];
            D_208 = &D_1D60->points[i];
            D_20C = &((O1ControlTable *)D_1D64)->points[i];
            D_210 = &D_1D68->points[i];
            D_214 = &D_1D6C->points[i];
        }
        i++;
    } while (i < 8);

    overlay1InterpolatePath(outX, outZ, CHOICE_WORLD->selector, 0.5f);
    if (selected != -1 && selected != CHOICE_WORLD->selector) {
        CHOICE_WORLD->previousSelector = CHOICE_WORLD->selector;
        CHOICE_WORLD->selector = selected;
        CHOICE_WORLD->transition = 0xFF;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0003750_184FB30.s")
#endif

#undef CHOICE_WORLD

/* ---- overlay1SubmitGlobals ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern void overlay1SubmitReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void overlay1SubmitGlobals(s32 arg0, s32 arg1) {
    overlay1SubmitReloc(arg0, arg1, gOverlay1SubmitArg2, gOverlay1SubmitArg3);
}

/* ---- overlay1SubmitAll ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern s32 gOverlay1SubmitArg2;
extern s32 gOverlay1SubmitArg3;
extern s32 gOverlay1SubmitArg4;
extern s32 gOverlay1SubmitArg5;
extern void overlay1SubmitAllReloc(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                                   s32 arg4, s32 arg5);

void overlay1SubmitAll(s32 arg0, s32 arg1) {
    overlay1SubmitAllReloc(arg0, arg1, gOverlay1SubmitArg2,
                           gOverlay1SubmitArg3, gOverlay1SubmitArg4,
                           gOverlay1SubmitArg5);
}

/* ---- overlay1AngleBetweenSamples ---- */


typedef struct Overlay1SampleState {
    u8 pad0[0x37E];
    u8 selector;
} Overlay1SampleState;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1SampleState *gOverlay1SampleState;
extern void overlay1SampleReloc(f32 *x, f32 *y, s32 selector, f32 scale);
extern s32 overlay1SampleAngleReloc(f32 x, f32 y);

s32 overlay1AngleBetweenSamples(f32 unusedX, f32 unusedY) {
    f32 firstX;
    f32 firstY;
    f32 secondX;
    f32 secondY;

    overlay1SampleReloc(&firstX, &firstY, gOverlay1SampleState->selector, 1.0f);
    overlay1SampleReloc(&secondX, &secondY, gOverlay1SampleState->selector,
                        2.5f);
    return (s16)(overlay1SampleAngleReloc(firstX - secondX,
                                          firstY - secondY) - 0x8000);
}

/* ---- overlay1RelativeAngles ---- */


typedef struct Overlay1Position {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1Position;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1Position *gOverlay1Position;
extern s32 overlay1AngleReloc(f32 x, f32 y);

s32 overlay1RelativeAngleA(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}

s32 overlay1RelativeAngleB(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}

/* ---- overlay1TransitionState ---- */


typedef struct Transform {
    s16 rotY;
    s16 rotX;
    s16 rotZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0xE];
    struct ObjectHeader *header;
} Transform;

typedef struct ObjectHeader {
    u8 pad00[6];
    u16 flags;
} ObjectHeader;

typedef struct Spawned {
    s16 angle;
    u8 pad02[2];
    f32 y;
    u8 pad08[4];
    f32 x;
    union {
        f32 y2;
        u16 flags;
    } at10;
    f32 z;
    u8 pad18[0x2C];
    f32 x2;
    f32 z2;
} Spawned;

typedef struct State {
    u8 pad000[0x16C];
    u8 done;
    u8 pad16D[3];
    u8 phase;
    u8 pad171[0x1F];
    u8 fade;
    u8 active;
    u8 pad192[0x16];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 pathId;
    u8 selectorA;
    u8 selectorB;
    u8 selectorC;
    u8 pad381[0x5F];
    Spawned *spawned;
} State;

extern s32 G_o1_83e4;
extern void ext_o7_ccc(Transform *, s32);
/* Runtime table-2 record 548 at module +0x4018 makes this the sole inbound to
 * Overlay 1 ORT 1485, +0x378 (overlay1FindType5ByKey). The local State-pointer
 * argument and Spawned-pointer result are an inconsistent caller-side typed
 * view; preserve them until a coordinated ABI reconstruction retains codegen. */
extern Spawned *local_378(State *);
extern void ext_o0_1bed0(Transform *, f32, f32, f32, s16, s16, s16);
extern void ext_o0_1c6bc(Transform *, State *);
extern void ext_o0_5a914(Transform *, s32, s32, s32);
extern Spawned *local_414(s16, Spawned **);
extern s16 local_c0(Spawned *);

/* PLATEAU (2026-09-04): configured full-TU C is 236/237 words with 160 raw
 * and relocation-masked differences from +0x20; both frames are 0x50. An
 * asymmetric selector assignment restored the narrow index mask/shift. Phase
 * carrier, register-hint, and pointer-advance forms were flat or regressed;
 * folding the existing point update into its load is also byte-flat. The
 * promotion trial's in=0/out=0 is a build-error classification caused by
 * schedule-divergent global sites, not linked byte equality. */
#ifdef NON_MATCHING
void overlay1TransitionState(Transform *obj, State *state, s32 updateRate) {
    Spawned *sp3C;
    s32 value;
    u8 phase;
    s32 phaseValue;
    u8 index;
    u8 *point;
    Spawned *spawned;

    if (G_o1_83e4 == 3) {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_378(state);
                state->phase = 2;
                return;
            }
            phaseValue = phase;
            if (phaseValue == 2) {
                value = state->fade - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                obj->x = spawned->x;
                obj->y = spawned->at10.y2 + 100.0f;
                obj->z = spawned->z;
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = spawned->angle;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                state->fade = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = state->fade + (updateRate * 4);
                if (value >= 255) {
                    state->fade = 255;
                    state->phase = 5;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned = 0;
        }
    } else {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_414(state->pathId, &sp3C);
                state->pathId = local_c0(sp3C);
                state->spawned->at10.flags |= 8;
                state->phase = 2;
                return;
            }
            phaseValue = phase;
            if (phaseValue == 2) {
                value = obj->alpha - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                index = state->selectorA = 3;
                state->selectorB = 3;
                state->selectorC = 0;
                point = (u8 *)spawned + (index << 4);
                obj->x = *(f32 *)(point + 0x14);
                obj->y = spawned->y + 100.0f;
                point += 0x14;
                obj->z = *(f32 *)(point + 4);
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = *(s16 *)((u8 *)spawned + 0xC) + 0x4000;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                obj->alpha = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = obj->alpha + (updateRate * 4);
                if (value >= 255) {
                    obj->alpha = 255;
                    state->phase = 5;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned->at10.flags &= 0xFFF7;
                state->spawned = 0;
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0003FD8_18503B8.s")
#endif

/* ---- overlay1UpdateObjectPhysics ---- */

#undef NULL
#define NULL 0

typedef struct O1PhysicsObject O1PhysicsObject;
typedef struct O1PhysicsState O1PhysicsState;

/* Partial layouts established by Mickey's field accesses. */
struct O1PhysicsObject {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    u8 pad006[0x6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 animationProgress;
    u8 pad02C[0x2];
    s16 positionTag;
    u8 pad030[0xB];
    s8 mode;
    u8 pad03C[0x28];
    O1PhysicsState * state;
    u8 pad068[0x18];
    s32 flags80;
};
struct O1PhysicsState {
    u8 pad000[0x2];
    u8 inSurface;
    u8 surfaceKind;
    f32 forwardVelocity;
    f32 sideVelocity;
    f32 speedLimit;
    u8 pad010[0x4];
    f32 field14;
    u8 pad018[0x20];
    f32 previousX;
    f32 previousY;
    f32 previousZ;
    u8 pad044[0x18];
    f32 slope;
    f32 normalX;
    f32 normalY;
    f32 surfaceHeight;
    f32 submergedHeight;
    f32 outputScale;
    f32 impulseX;
    f32 impulseY;
    f32 impulseZ;
    f32 impulseInitial;
    f32 impulseVelocity;
    f32 impulseAcceleration;
    u8 pad08C[0x8];
    f32 actualVelocityX;
    f32 actualVelocityY;
    f32 actualVelocityZ;
    u8 pad0A0[0x8];
    void * soundA8;
    void * soundAC;
    u8 pad0B0[0x24];
    O1PhysicsObject * linkedObject;
    u8 pad0D8[0x18];
    s16 heading;
    u8 pad0F2[0xE];
    s16 field100;
    s16 spinTimer;
    s16 spinAngle;
    u8 pad106[0x2];
    s16 steeringAngle;
    u8 pad10A[0x4E];
    s16 field158;
    u8 pad15A[0xC];
    s16 field166;
    u8 pad168[0x2];
    s16 field16A;
    u8 field16C;
    u8 pad16D[0x3];
    u8 reset170;
    u8 reverseTimer;
    u8 pad172[0xF];
    u8 impulseActive;
    u8 pad182[0x1];
    s8 field183;
    u8 pad184[0x1];
    u8 boostMode;
    u8 pad186[0x1];
    s8 field187;
    f32 boostScale;
    u8 pad18C[0x1];
    s8 disabled18D;
    u8 pad18E[0x4];
    u8 level192;
    u8 pad193[0x15];
    u16 flags1A8;
    u8 pad1AA[0x112];
    s32 collisionMode;
    u8 pad2C0[0x89];
    u8 field349;
    u8 pad34A[0x32];
    s16 pathIndex;
    u8 pad37E[0x3];
    u8 pathMode;
    u8 actionMode;
    u8 pad383[0x15];
    f32 progress398;
    u8 pad39C[0x4];
    f32 speedScale;
    u8 pad3A4[0x18];
    f32 targetX;
    f32 targetZ;
    u8 pad3C4[0x8];
    s16 previousPathIndex;
    s16 stuckTimer;
    f32 pathStartX;
    f32 pathStartZ;
    f32 pathEndX;
    f32 pathEndZ;
    u8 pad3E0[0x1A];
    s16 field3FA;
    u8 pad3FC[0x20];
    s32 controlKeys;
    s32 controlDkeys;
    u8 pad424[0x4];
    s32 controlXjoy;
    s32 controlYjoy;
    u8 pad430[0x8];
    s32 joypadDisabled;
};

typedef struct O1PhysicsSurface {
    f32 height;
    u32 flags;
} O1PhysicsSurface;

typedef struct O1PhysicsPathMode {
    s32 (*test)(void);
    void (*position)(f32 *x, f32 *z);
    s32 (*heading)(f32 x, f32 z);
    u16 mask;
    u16 unused;
} O1PhysicsPathMode;

typedef struct O1PhysicsActionMode {
    s32 (*test)(void);
    void (*update)(void);
    u16 mask;
    u16 unused;
} O1PhysicsActionMode;

extern O1PhysicsPathMode gO1PhysicsPaths[4];
extern O1PhysicsActionMode gO1PhysicsActions[6];

/* Call identities come from the shipped overlay relocation table. */
s32 func_overlay_001_F00004B4_184C894(void *object);
s32 func_overlay_001_F0007D6C_185414C(s16, s16, s16, s16,
                                     s16 *, s16 *, void *);
f32 *func_overlay_008_F0000008_185DD60(void *state);
void func_8001F25C(void *state, s32 disabled);
void func_overlay_008_F00049DC_1862734(s32 value);
void func_8001D910(void *object, void *state);
void func_80029F2C(s16 *angles, f32 *vector);
u32 func_8001357C(f32 x, f32 z, f32 *height, s32 flags, void *surfaces);
void func_8001F14C(void *object, void *state);
s32 func_overlay_002_F000123C_1858034(f32 x, f32 z, void *region);
s32 func_8002A910(f32 x, f32 z);
s32 func_8002AA0C(s16 first, s16 second);
void func_overlay_007_F0000EDC_185CD64(void);
f32 func_overlay_008_F0001000_185ED58(void *object, void *state, f32 limit);
s32 func_8002675C(void);
s32 func_800299E8(s32 minimum, s32 maximum);
f32 func_8002A8BC(s32 angle);
void func_800031E8(void *handle);
void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority, void **handle);
void func_800031C0(void *handle, f32 x, f32 y, f32 z);
f32 func_8002A878(f32 base, s32 exponent);
f32 func_8002A8C0(s32 angle);
s32 func_8001E5C4(void *object, void *state, f32 update);
s32 func_8001DD70(void *object, void *state, f32 update);
s32 func_80008128(void *object, f32 x, f32 y, f32 z);
void func_overlay_008_F00049A4_18626FC(void *state);
f32 func_overlay_008_F00034A0_18611F8(void *object, void *state, f32 limit, f32 update);
void func_overlay_008_F00049B4_186270C(void *unused);
void func_8001D41C(void *object, void *state, s32 update);
void func_overlay_008_F0003278_1860FD0(void *object, void *state, s32 update);
void func_8001D960(void *object, void *state, s32 mode, s32 kind, s32 update);
void func_overlay_008_F0002EC0_1860C18(void *object, void *state, s32 update);
void func_overlay_008_F0003018_1860D70(void *object, void *state, f32 scale, s32 update);
void func_8003EDEC(void *object, s32 update);
extern f32 D_4;
extern s32 gOverlay1Mode;
extern f32 G_rt_458c4;
extern s32 G_rt_43a3c;
extern u8 G_offd_31a4;
extern void *D_1BA4;
extern f32 LOCAL_DATA_4;
extern s32 LOCAL_BSS_1D78;
extern s32 gOverlay1TimerStep;
extern void *LOCAL_BSS_1BA4;
extern void *LOCAL_BSS_1D9C;

/* Typed reconstruction remains NON_MATCHING. The object/state and callback
 * layouts follow Mickey's runtime identities and access widths. Local
 * lifetime cleanup reduces the frame; remaining CFG and allocation work
 * is measured separately in the function handoff. */
#ifdef NON_MATCHING
void func_overlay_001_F000438C_185076C(O1PhysicsObject *object, s32 updateRate) {
    f32 *tuning;
    O1PhysicsSurface surfaces[8];
    f32 normal[3];
    s16 angles[3];
    f32 targetX;
    f32 targetZ;
    s32 remaining;
    f32 limit;
    f32 velocityX;
    f32 inverseUpdate;
    f32 deltaY;
    f32 impulseX;
    f32 impulseY;
    f32 impulseZ;
    f32 scale;
    s16 heading;
    s16 targetHeading;
    s16 pathHeading;
    f32 speed;
    s16 resolvedX;
    s16 resolvedZ;
    void (*callback)(void);
    O1PhysicsPathMode *path;
    O1PhysicsActionMode *action;
    f32 value;
    f32 work;
    f32 deltaX;
    f32 deltaZ;
    f32 value2;
    f32 ceiling;
    f32 velocityZ;
    O1PhysicsState *state;
    s16 clampedAngle;
    s32 (*predicate)(void);
    u32 surfaceCount;
    s32 collision;
    s32 angleOffset;
    s32 keys;
    s32 index;
    s32 applySlope;
    s32 steering;
    s32 level;
    O1PhysicsSurface *surface;

    state = object->state;
    if (func_overlay_001_F00004B4_184C894(object) != 0) {
        tuning = func_overlay_008_F0000008_185DD60(state);
        G_rt_458c4 = *tuning;
        if (gOverlay1Mode == 1) {
            if ((G_rt_43a3c == 0) && (state->joypadDisabled == 0) && (state->spinTimer == 0) && !(state->flags1A8 & 8)) {
                if (state->pathIndex != state->previousPathIndex) {
                    state->previousPathIndex = state->pathIndex;
                    state->stuckTimer = 0;
                } else {
                    state->stuckTimer = (s16) (state->stuckTimer + updateRate);
                }
                if ((f32) state->stuckTimer > 360.0f) {
                    if (state->reset170 == 0) {
                        state->reset170 = 1U;
                    }
                    goto block_13;
                }
            } else {
block_13:
                state->stuckTimer = 0;
            }
        }
        state->outputScale = 0.0f;
        if ((state->disabled18D != 0) || (state->field158 != 0) || (state->reset170 != 0) || (state->field3FA != 0)) {
            func_8001F25C(state, 1);
        }
        gOverlay1TimerStep = updateRate;
        D_4 = (f32) updateRate;
        speed = -state->forwardVelocity;
        func_overlay_008_F00049DC_1862734(NULL);
        value = -31.99f;
        object->flags80 = 0;
        state->controlXjoy = 0;
        state->controlYjoy = 0;
        state->controlKeys = 0;
        state->controlDkeys = 0;
        if (state->forwardVelocity < value) {
            state->forwardVelocity = value;
        }
        work = 31.99f;
        if (work < state->forwardVelocity) {
            state->forwardVelocity = work;
        }
        if (state->sideVelocity < value) {
            state->sideVelocity = value;
        }
        if (work < state->sideVelocity) {
            state->sideVelocity = work;
        }
        func_8001D910(object, state);
        angles[0] = -state->heading;
        angles[1] = -object->rotationY;
        normal[2] = 0.0f;
        normal[0] = 0.0f;
        angles[2] = -object->rotationZ;
        normal[1] = -1.0f;
        func_80029F2C(angles, normal);
        state->normalX = normal[0];
        state->normalY = normal[1];
        state->slope = normal[2];
        surfaceCount = func_8001357C(object->x, object->z, NULL, 0x08010000, surfaces);
        ceiling = -32768.0f;
        index = surfaceCount - 1;
        state->surfaceHeight = -32768.0f;
        if (surfaceCount != NULL) {
            surface = &surfaces[index];
            do {
                if (surface->flags & 0x10000) {
                    state->surfaceHeight = (f32) surface->height;
                }
                if (surface->flags & 0x08000000) {
                    ceiling = surface->height;
                }
                surface -= 1;
            } while (index--);
        }
        value2 = state->surfaceHeight;
        if (object->y < value2) {
            state->inSurface = 1U;
            state->submergedHeight = value2;
        } else {
            state->inSurface = 0U;
            state->submergedHeight = 0.0f;
        }
        if (object->y < ceiling) {
            if (state->reset170 == 0) {
                state->reset170 = 1U;
            }
            if ((state->inSurface != 0) && (state->surfaceKind != 1)) {
                func_8001F14C(object, state);
            }
        }
        level = state->level192;
        if ((s32) level >= 0xB) {
            level = 0xA;
        }
        path = &gO1PhysicsPaths[1];
        index = 1;
        limit = (tuning[16] + ((f32) level * tuning[2])) * state->speedScale;
        do {
            predicate = path->test;
            if ((predicate != NULL) && (path->mask & (1 << state->pathMode))) {

                if (predicate() != 0) {
                    state->pathMode = (u8) index;
                }
            }
            index += 1;
            path++;
        } while (index != 4);
        gO1PhysicsPaths[state->pathMode].position(&targetX, &targetZ);
        if (D_1BA4 != NULL) {

            if (func_overlay_002_F000123C_1858034(((O1PhysicsObject *)D_1D9C)->x, ((O1PhysicsObject *)D_1D9C)->z, D_1BA4) != NULL) {

                ((O1PhysicsState *)D_1DA0)->pathStartX = ((O1PhysicsObject *)D_1D9C)->x;
                ((O1PhysicsState *)D_1DA0)->pathStartZ = ((O1PhysicsObject *)D_1D9C)->z;
            }
            if (func_overlay_002_F000123C_1858034(targetX, targetZ, D_1BA4) != NULL) {
                ((O1PhysicsState *)D_1DA0)->pathEndX = targetX;
                ((O1PhysicsState *)D_1DA0)->pathEndZ = targetZ;
            }
            func_overlay_001_F0007D6C_185414C(
                (s16)(s32)((O1PhysicsState *)D_1DA0)->pathStartX,
                (s16)(s32)((O1PhysicsState *)D_1DA0)->pathStartZ,
                (s16)(s32) ((O1PhysicsState *)D_1DA0)->pathEndX, (s16)(s32) ((O1PhysicsState *)D_1DA0)->pathEndZ,
                &resolvedX, &resolvedZ, D_1D9C);
            targetX = (f32) resolvedX;
            targetZ = (f32) resolvedZ;
        }
        targetHeading = func_8002A910(targetX - object->x, targetZ - object->z) + 0x8000;
        pathHeading = gO1PhysicsPaths[state->pathMode].heading(targetX, targetZ);
        state->targetX = targetX;
        state->targetZ = targetZ;
        value = (f32) func_8002AA0C(object->rotationX, targetHeading) * 0.4f;
        work = value;
        state->controlXjoy = (s32) (value * -0.4f);
        if (value < 0.0f) {
            work = -value;
        }
        if (gOverlay1Mode == 1) {
            if (work > 24576.0f) {
                clampedAngle = 0x6000;
            } else {
                clampedAngle = (s16) (s32) work;
            }
            angleOffset = (s32) (work - 8192.0f);
            if (speed < (25.0f - ((f32) clampedAngle * 0.0009765625f))) {
                state->controlKeys = (s32) (state->controlKeys | 0x8000);
            }
            clampedAngle = (s16) angleOffset;
            if ((s16) angleOffset < 0) {
                clampedAngle = (s16) angleOffset * -1;
            }
            if (((f32) clampedAngle * 0.0044759116f) < speed) {
                state->controlKeys = (s32) (state->controlKeys | 0x4000);
            }
            value = (f32) func_8002AA0C(targetHeading, pathHeading);
            work = value;
            if (value < 0.0f) {
                work = -value;
            }
            if ((work * 0.00077f) < speed) {
                state->controlKeys = (s32) (state->controlKeys | 0x4000);
            }
        } else {
            if (work > 16384.0f) {
                clampedAngle = 0x4000;
            } else {
                clampedAngle = (s16) (s32) work;
            }
            angleOffset = (s32) (work - 16384.0f);
            if (speed < (25.0f - ((f32) clampedAngle * 0.0014648438f))) {
                state->controlKeys = (s32) (state->controlKeys | 0x8000);
            }
            clampedAngle = (s16) angleOffset;
            if ((s16) angleOffset < 0) {
                clampedAngle = (s16) angleOffset * -1;
            }
            if (((f32) clampedAngle * 0.006713867f) < speed) {
                state->controlKeys = (s32) (state->controlKeys | 0x4000);
            }
            if (speed < 0.01f) {
                state->controlKeys = (s32) (state->controlKeys | 0x8000);
            }
            if (state->reverseTimer != 0) {
                if (updateRate < (s32) state->reverseTimer) {
                    state->reverseTimer = (u8) (state->reverseTimer - updateRate);
                } else {
                    state->reverseTimer = 0U;
                }
                state->controlYjoy = -0x64;
                state->controlKeys = 0x4000;
            }
        }
        if (G_rt_43a3c != 0) {
            state->controlKeys = 0x4000;
            object->velocityX = 0.0f;
            object->velocityY = 0.0f;
            object->velocityZ = 0.0f;
            state->forwardVelocity = 0.0f;
        }
        if ((LOCAL_BSS_1D78 != 0) && (G_rt_43a3c == 0)) {
            state->field183 = 1;
            state->boostMode = 2U;
            state->field187 = 6;
            state->boostScale = 1.0f;
        }
        if (G_rt_43a3c != 0) {
            func_overlay_007_F0000EDC_185CD64();
        }
        if (state->joypadDisabled != 0) {
            state->controlXjoy = 0;
            state->controlYjoy = 0;
            state->controlKeys = 0;
            state->controlDkeys = 0;
        }
        if (state->linkedObject != NULL) {
            limit *= 1.0f + (-0.3f * state->linkedObject->state->field14);
        }
        if (state->boostMode == 0) {
            value = state->slope;
            if (value != 0.0f) {
                scale = 1.0f - (value * 0.5f * tuning[3]);
                if (scale < 0.1f) {
                    scale = 0.1f;
                }
                limit *= scale;
            }
        }
        if ((state->spinTimer != 0) && ((object->mode == 0x10) || (object->mode == 0xF))) {
            scale = object->animationProgress * 1.5f;
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            if (state->spinTimer > 0) {
                scale = -scale;
            }
            state->spinAngle = (s16) (s32) (65536.0f * scale);
            if (object->animationProgress == 1.0f) {
                state->spinTimer = 0;
                state->spinAngle = 0;
            } else {
                state->boostMode = 0U;
                state->boostScale = 0.0f;
            }
        }
        if (gOverlay1TimerStep != 0) {
            remaining = gOverlay1TimerStep - 1;
            do {
                value = func_overlay_008_F0001000_185ED58(object, state, limit);
                limit = value;
                keys = state->controlKeys;
                if (((keys & 0x4000) == 0) && (state->slope > 0.0f) && (state->forwardVelocity < -value)) {
                    applySlope = 1;
                } else if (((keys & 0x4000) == 0) && (state->slope < 0.0f)) {
                    applySlope = 1;
                } else {
                    applySlope = 0;
                    if (!(keys & 0xC000) && (state->slope > 0.0f)) {
                        applySlope = 1;
                    }
                }
                if ((applySlope != 0) && (state->boostMode == 0)) {
                    state->forwardVelocity = (f32) (state->forwardVelocity + (G_rt_458c4 * state->slope));
                    value2 = tuning[7];
                    if (value2 < state->forwardVelocity) {
                        state->forwardVelocity = value2;
                    }
                }
                if (state->spinTimer != 0) {
                    value2 = 0.99f;
                    state->controlXjoy = 0;
                    state->controlYjoy = 0;
                    state->controlKeys = 0;
                    state->controlDkeys = 0;
                    state->forwardVelocity = (f32) (state->forwardVelocity * value2);
                    state->sideVelocity = (f32) (state->sideVelocity * value2);
                }
                keys = state->controlKeys;
                state->field100 = 0;
                if (keys & 0x4000) {
                    if (state->forwardVelocity < -5.0f) {
                        func_overlay_008_F00049DC_1862734(1);
                    }
                    if (state->forwardVelocity < 0.0f) {
                        state->forwardVelocity = (f32) (state->forwardVelocity + tuning[(s32) -state->forwardVelocity + 50]);
                        if ((state->forwardVelocity > 0.0f) && (state->controlYjoy >= -0x1E)) {
                            state->forwardVelocity = 0.0f;
                        }
                    } else if (state->controlYjoy < -0x1E) {
                        state->forwardVelocity = (f32) (state->forwardVelocity + tuning[(s32) state->forwardVelocity + 8]);
                        if (state->forwardVelocity > 6.0f) {
                            state->forwardVelocity = 6.0f;
                        }
                    } else {
                        state->forwardVelocity = (f32) (state->forwardVelocity - tuning[50]);
                        if (state->forwardVelocity <= 0.0f) {
                            state->forwardVelocity = 0.0f;
                        }
                    }
                } else if (keys & 0x8000) {
                    if ((state->boostMode == 1) || (state->boostMode == 2)) {
                        if (G_offd_31a4 == 0) {
                            value2 = 0.3334f;
                        } else {
                            value2 = 0.5f;
                        }
                    } else {
                        value2 = state->forwardVelocity;
                        if (value2 > 0.0f) {
                            index = (s32) value2;
                            scale = value2 - (f32) index;
                        } else {
                            work = -value2;
                            index = (s32) work;
                            scale = work - (f32) index;
                        }
                        work = tuning[index + 17];
                        value2 = ((tuning[index + 18] - work) * scale) + work;
                    }
                    if (state->forwardVelocity < (-limit)) {
                        state->forwardVelocity = (f32) (state->forwardVelocity * 0.99f);
                        if ((-limit) < state->forwardVelocity) {
                            goto block_160;
                        }
                    } else {
                        state->forwardVelocity = (f32) (state->forwardVelocity - (value2 * state->speedScale));
                        if (state->forwardVelocity < (-limit)) {
block_160:
                            state->forwardVelocity = (-limit);
                        }
                    }
                    if ((gOverlay1Mode == 1) && (G_offd_31a4 == 0) && (state->flags1A8 & 1) && (func_8002675C() == (s32)0x21) && (state->pathIndex == 0x2A) && (0.4f < state->progress398)) {
                        state->forwardVelocity = (f32) (state->forwardVelocity - 2.0f);
                    }
                } else {
                    value2 = state->forwardVelocity;
                    if ((-0.01f < value2) && (value2 < 0.01f)) {
                        state->forwardVelocity = 0.0f;
                    } else {
                        state->forwardVelocity = (f32) (value2 * 0.99f);
                    }
                }
                if ((-0.2f < state->forwardVelocity) && (state->forwardVelocity < 0.2f)) {
                    func_800299E8(0, 127);
                }
                if (state->controlXjoy >= 0x42) {
                    steering = -0x1F4;
                } else if (state->controlXjoy < -0x41) {
                    steering = 0x1F4;
                } else {
                    steering = -state->controlXjoy;
                    steering = (steering * 500) / 65;
                }
                work = -2.0f - state->forwardVelocity;
                state->steeringAngle = (s16) (state->steeringAngle + ((s32) (steering - state->steeringAngle) >> 1));
                if (work > 0.0f) {
                    scale = (func_8002A8BC((s32) (work * 1310.72f)) * 0.25f) + 0.75f;
                } else {
                    if (work < 0.0f) {
                        work = -work;
                    }
                    if (work > 2.0f) {
                        work = 2.0f;
                    }
                    scale = (func_8002A8BC((s32) (work * 16384.0f)) + 1.0f) * 0.5f;
                }
                if (state->forwardVelocity > 0.0f) {
                    scale = -scale;
                }
                state->heading = (s16) (s32) ((f32) state->heading + ((f32) state->steeringAngle * scale));
                state->sideVelocity = (f32) (state->sideVelocity * 0.96f);
                value2 = state->sideVelocity;
                if ((-0.1f < value2) && (value2 < 0.1f)) {
                    state->sideVelocity = 0.0f;
                }
            } while (remaining--);
        }
        if (state->controlDkeys & 0x2000) {
            if (state->soundA8 != NULL) {
                func_800031E8(state->soundA8);
            }
            func_80002FE0(4, object->x, object->y, object->z, 4, &state->soundA8);
        }
        if ((state->controlKeys & 0x4000) && (state->forwardVelocity < 0.0f)) {
            if (state->soundAC == NULL) {
                func_80002FE0(3, object->x, object->y, object->z, 1, &state->soundAC);
            } else {
                func_800031C0(state->soundAC, object->x, object->y, object->z);
            }
        }
        if (state->soundAC != NULL) {
            func_800031E8(state->soundAC);
        }
        object->rotationX = (s16) (state->heading + state->spinAngle);
        heading = state->heading;
        if (state->joypadDisabled == 1) {
            value = func_8002A878(0.8f, gOverlay1TimerStep);
            value2 = state->forwardVelocity;
            if ((value2 < -0.5f) || (value2 > 0.5f)) {
                state->forwardVelocity = (f32) (value2 * value);
            } else {
                state->forwardVelocity = 0.0f;
            }
            value2 = state->sideVelocity;
            if ((value2 < -0.5f) || (value2 > 0.5f)) {
                state->sideVelocity = (f32) (value2 * value);
            } else {
                state->sideVelocity = 0.0f;
            }
        }
        if (state->impulseActive != 0) {
            value = (state->impulseVelocity * D_4) + (0.5f * state->impulseAcceleration * D_4 * D_4);
            impulseX = state->impulseX * value;
            impulseY = state->impulseY * value;
            impulseZ = state->impulseZ * value;
            if (value < 0.0f) {
                state->impulseVelocity = 0.0f;
                state->impulseAcceleration = 0.0f;
                state->impulseActive = 0U;
                if (!(state->controlKeys & 0x8000)) {
                    state->forwardVelocity = 0.0f;
                    state->sideVelocity = 0.0f;
                }
            }
            value = state->impulseVelocity;
            scale = 1.0f - (value / state->impulseInitial);
            state->impulseVelocity = (f32) (value + (state->impulseAcceleration * D_4));
            velocityX = func_8002A8C0(heading) * state->forwardVelocity * scale;
            velocityZ = func_8002A8BC(heading) * state->forwardVelocity * scale;
        } else {
            impulseX = 0.0f;
            impulseY = 0.0f;
            impulseZ = 0.0f;
            velocityX = func_8002A8C0(heading) * state->forwardVelocity;
            velocityZ = func_8002A8BC(heading) * state->forwardVelocity;
        }

        velocityX += state->sideVelocity * func_8002A8BC(heading);
        value = func_8002A8C0(heading);
        velocityZ -= state->sideVelocity * value;
        deltaX = (velocityX * D_4) + impulseX;
        deltaY = ((object->velocityY * D_4) - (0.5f * G_rt_458c4 * D_4 * D_4)) + impulseY;
        inverseUpdate = 1.0f / D_4;
        deltaZ = (velocityZ * D_4) + impulseZ;
        object->velocityX = (f32) (deltaX * inverseUpdate);
        object->velocityY = (f32) (object->velocityY - (G_rt_458c4 * D_4));
        object->x += deltaX;
        object->velocityZ = (f32) (deltaZ * inverseUpdate);
        object->y = (f32) (object->y + deltaY);
        object->z += deltaZ;
        if (state->collisionMode == 1) {
            collision = func_8001E5C4(object, state, D_4);
        } else {
            collision = func_8001DD70(object, state, D_4);
        }

        if ((func_80008128(object, 0.0f, 0.0f, 0.0f) != NULL) || (object->positionTag == -1)) {
            if (state->reset170 == 0) {
                state->reset170 = 1U;
            }
            object->x = state->previousX;
            object->y = (f32) state->previousY;
            object->z = state->previousZ;

            func_80008128(object, 0.0f, 0.0f, 0.0f);
        }
        if (state->field166 != 0) {
            if (gOverlay1Mode == 3) {
                state->reverseTimer = 0x78U;
                state->field166 = 0;
            } else if (state->reset170 == 0) {
                state->reset170 = 1U;
            }
        }
        state->actualVelocityX = (object->x - state->previousX) * inverseUpdate;
        state->actualVelocityY = (f32) ((object->y - state->previousY) * inverseUpdate);
        state->actualVelocityZ = (object->z - state->previousZ) * inverseUpdate;
        if ((state->field16A == 0) && (collision != NULL)) {
            value = func_8002A878(0.9f, gOverlay1TimerStep);
            work = state->speedLimit;
            state->speedLimit = work + ((3.0f - work) * (1.0f - value));
            work = state->speedLimit;
            if (state->forwardVelocity < (-work)) {
                state->forwardVelocity = (-work);
            }
            if (work < state->forwardVelocity) {
                state->forwardVelocity = work;
            }
            if (state->sideVelocity < (-work)) {
                state->sideVelocity = (-work);
            }
            if (work < state->sideVelocity) {
                state->sideVelocity = work;
            }
        } else {
            value = func_8002A878(0.825f, gOverlay1TimerStep);
            work = state->speedLimit;
            state->speedLimit = work + ((25.0f - work) * (1.0f - value));
        }
        func_overlay_008_F00049A4_18626FC(state);
        state->outputScale = func_overlay_008_F00034A0_18611F8(object, state, limit, D_4);
        func_overlay_008_F00049B4_186270C(state);
        func_8001D41C(object, state, gOverlay1TimerStep);
        action = &gO1PhysicsActions[2];
        index = 2;
        do {
            if (index != state->actionMode) {
                predicate = action->test;
                if ((predicate != NULL) && (action->mask & (1 << state->actionMode))) {

                    predicate();
                }
            }
            index += 1;
            action++;
        } while (index != 6);
        callback = gO1PhysicsActions[state->actionMode].update;
        if (callback != NULL) {
            callback();
        }
        func_overlay_008_F0003278_1860FD0(object, state, gOverlay1TimerStep);
        func_8001D960(object, state, 0, 3, gOverlay1TimerStep);
        func_overlay_008_F0002EC0_1860C18(object, state, gOverlay1TimerStep);
        func_overlay_008_F0003018_1860D70(object, state, state->outputScale, gOverlay1TimerStep);
        func_8003EDEC(object, gOverlay1TimerStep);
        if ((state->field349 != 0) && (state->field16C == 1)) {
            state->field16C = 0U;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F000438C_185076C.s")
#endif

/* ---- overlay1InitTimedState ---- */


/* DKR v77/v80 contains only generic object-state initialization patterns. */
typedef struct Overlay1TimedState {
    u8 pad0[0x193];
    u8 field193;
    u8 pad194[0x1EE];
    u8 enabled;
    u8 pad383[0xD];
    s16 timer;
    u8 pad392[0x52];
    s32 value3E4;
} Overlay1TimedState;

typedef struct Overlay1TimedObject {
    u8 pad0[0x64];
    Overlay1TimedState *state;
} Overlay1TimedObject;

void overlay1InitTimedState(Overlay1TimedObject *object, s32 timer) {
    Overlay1TimedState *state = object->state;

    state->enabled = 1;
    state->timer = timer;
    state->field193 = 0;
    state->value3E4 = 0;
}

/* ---- overlay1ConsumeTimer ---- */


typedef struct Overlay1TimerState {
    u8 pad000[0x382];
    u8 active;
    u8 pad383[0xD];
    u16 timer;
} Overlay1TimerState;

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern s32 gOverlay1TimerStep;

void overlay1ConsumeTimer(void) {
    if (((Overlay1TimerState *)gOverlay1TimerState)->timer <=
        gOverlay1TimerStep) {
        ((Overlay1TimerState *)gOverlay1TimerState)->active = 0;
    } else {
        ((Overlay1TimerState *)gOverlay1TimerState)->timer -=
            gOverlay1TimerStep;
    }
}

/* ---- overlay1StartTimerCallbacks ---- */


typedef void (*Overlay1Callback)(void);

typedef struct Overlay1CallbackState {
    u8 pad000[0x382];
    u8 mode;
} Overlay1CallbackState;

typedef struct Overlay1CallbackObject {
    u8 pad00[0x64];
    Overlay1CallbackState *state;
} Overlay1CallbackObject;

typedef struct Overlay1CallbackEntry {
    Overlay1Callback callback;
    s32 pad4;
    u16 modeMask;
    u16 padA;
} Overlay1CallbackEntry;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor.
 * Retained genuine C measures 36/56 words with all 13 runtime-backed records.
 * Lane 704a9f31 reached an apparent exact result only by adding an empty
 * pointer condition after the callback; that invented guard is rejected and
 * must not be promoted. Reproduce the natural baseline before a source-faithful
 * allocator/lifetime attempt. */
extern s32 overlay1IsObjectActive(void *object);
extern s32 gOverlay1TimerStep;
extern f32 gOverlay1CallbackStepFloat;
extern Overlay1CallbackEntry gOverlay1CallbackDescriptor[];
extern Overlay1CallbackEntry gOverlay1ModeCallbacks[];

void overlay1StartTimerCallbacks(Overlay1CallbackObject *object, s32 amount)
{
  Overlay1CallbackEntry *entry;
  Overlay1Callback callback;
  Overlay1Callback loadedCallback;
  s32 index;
  u8 mode;
  if (overlay1IsObjectActive(object) != 0)
  {
    gOverlay1TimerStep = amount;
    gOverlay1CallbackStepFloat = amount;
    entry = gOverlay1CallbackDescriptor;
    for (index = 5; index != 6; index++, entry++)
    {
      mode = ((Overlay1CallbackState *) gOverlay1TimerState)->mode;
      if (index != mode)
      {
        loadedCallback = entry->callback;
        callback = loadedCallback;
        if (callback != 0)
        {
          if ((entry->modeMask & (1 << mode)) != 0)
          {
            callback();
            if (entry->modeMask)
            {
            }
          }
        }
      }
    }

    mode = ((Overlay1CallbackState *) gOverlay1TimerState)->mode;
    callback = gOverlay1ModeCallbacks[mode].callback;
    if (callback != 0)
    {
      callback();
    }
  }
}

/* ---- overlay1FindDirectionalObject ---- */


#ifndef DOT_CONDITION
#define DOT_CONDITION (threshold < dot)
#endif

typedef struct Overlay1ObjectState { s8 tableIndex; } Overlay1ObjectState;
typedef struct Overlay1DirectionalObject {
    s16 angle; u8 pad02[0xA]; f32 x; u8 pad10[4]; f32 z;
    u8 pad18[0x4C]; Overlay1ObjectState *state;
} Overlay1DirectionalObject;
extern Overlay1DirectionalObject **overlay1GetObjectList(s32 *count);
extern f32 sqrtf(f32 value);
extern f32 overlay1TrigX(s32 angle);
extern f32 overlay1TrigY(s32 angle);
extern f32 D_160;

Overlay1DirectionalObject *overlay1FindDirectionalObject(
    Overlay1DirectionalObject *object, void *unused1, void *unused2,
    f32 threshold, f32 maxValue) {
    s32 count;
    Overlay1DirectionalObject **objects;
    Overlay1DirectionalObject *other;
    Overlay1DirectionalObject *best;
    Overlay1ObjectState *otherState;
    Overlay1ValueEntry *entry;
    s32 remaining;
    f32 dx, dz, distance, directionX, directionY, dot, value, bestValue;

    objects = overlay1GetObjectList(&count);
    best = 0;
    bestValue = D_160;
    if (overlay1IsObjectActive(object)) {
        goto active;
    }
    return 0;
active:
    while (remaining = count--) {
        other = objects[count];
        if (other == object) {
        } else {
            dx = other->x - object->x;
            otherState = other->state;
            dz = other->z - object->z;
            distance = sqrtf((dx * dx) + (dz * dz));
            if (distance > 0.0f) {
                dx /= distance;
                dz /= distance;
            }
            directionX = -overlay1TrigX(object->angle);
            directionY = -overlay1TrigY(object->angle);
            dot = (directionX * dx) + (directionY * dz);
            if (DOT_CONDITION) {
                entry = &D_1BA8[*(s8 *)D_1DA0].entries[otherState->tableIndex];
                value = entry->value;
                if ((value <= maxValue) && (value < bestValue)) {
                    bestValue = value;
                    best = other;
                }
            }
        }
    }
    return best;
}

/* ---- overlay1ReturnZero ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor; this is a generic leaf. */
s32 overlay1ReturnZero(void) {
    return 0;
}

/* ---- overlay1DispatchMode ---- */


typedef struct Overlay1ModeState {
    u8 pad00;
    s8 index;
    u8 pad02[0x198];
    u8 mode;
    u8 timer;
    u8 pad19C[4];
    s32 task;
    u8 pad1A4[0x1DA];
    u8 group;
    u8 pad37F[0x1D];
    f32 angle;
    u8 pad3A0[8];
    u8 status[1];
} Overlay1ModeState;

typedef struct Overlay1ModeObject {
    u8 pad00[0x64];
    Overlay1ModeState *state;
} Overlay1ModeObject;
struct Overlay8ActivationOwner; struct Overlay36Object; struct Overlay36TickSource; struct Overlay36EffectSource;
#ifndef WORLD_GLOBAL_DECL
#define WORLD_GLOBAL_DECL extern Overlay1ModeState *D_1DA0_array[];
#define WORLD D_1DA0_array[0]
#endif
#ifndef CASE_END
#define CASE_END return 0
#endif
WORLD_GLOBAL_DECL
extern u8 D_6C[];
extern void func_overlay_008_F0000F1C_185EC74(struct Overlay8ActivationOwner *object, s32 arg);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern Overlay1ModeObject *overlay1FindPreviousAngle(f32 angle);
extern Overlay1ModeObject *overlay1FindNextAngle(f32 angle);
extern void overlay36SpawnDirectional(struct Overlay36Object *object);
extern void overlay36SpawnLinked7F(struct Overlay36Object *object);
extern void overlay36SpawnOffsetA9(struct Overlay36Object *object);
extern void overlay36CallModeZero(void *object);
extern void overlay36TickState(struct Overlay36TickSource *object);
extern void overlay36UpdatePeers(struct Overlay36Object *object);
extern void overlay36SpawnFinalEffect(struct Overlay36EffectSource *object);
extern f32 overlay1WrapOffset(f32 first, f32 second);

/* Tier A: the ordinary IDO body and 0x28 frame are linked-ROM exact.
 * The 61 text relocation sites retain their shipped runtime identities.
 * External calls name the canonical definitions with their existing ABI;
 * local call aliases preserve the authentic pre-loader call addends.
 * No compiler instruction is edited by the metadata-only link recipe. */
/* The retained eight-entry table remains owned by initialized data.
 * Its compiler destinations agree relative to this function; the HI/LO
 * relocations bind to its stored addend without changing instructions. */

s32 overlay1DispatchMode(void) {
    Overlay1ModeState *world;
    Overlay1ModeObject *candidateObject;
    Overlay1ModeState *angleState;
    Overlay1ModeState *candidateState;
    Overlay1ModeObject *object;
    Overlay1ModeState *state;
    f32 difference;

    world = WORLD;
    switch (world->mode) {
        case 2:
            func_overlay_008_F0000F1C_185EC74(D_1D9C, 1);
            WORLD->timer--;
            if (WORLD->timer == 0) {
                WORLD->mode = 0xFF;
                WORLD->task = 0;
            }
            CASE_END;
        case 3:
            if (D_6C[WORLD->index] < mathRnd(1, 100)) {
                object = overlay1FindPreviousAngle(WORLD->angle);
                candidateObject = object;
                if (candidateObject != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay36SpawnDirectional(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 4:
            if (D_6C[WORLD->index] < mathRnd(1, 100)) {
                object = overlay1FindPreviousAngle(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        angleState = object->state;
                        difference = overlay1WrapOffset(WORLD->angle,
                                                        angleState->angle);
                        if ((0.5f <= difference) && (difference <= 4.0f)) {
                            overlay36SpawnLinked7F(D_1D9C);
                        }
                    }
                }
            }
            CASE_END;
        case 5:
            if (D_6C[WORLD->index] < mathRnd(1, 100)) {
                object = overlay1FindPreviousAngle(WORLD->angle);
                if (object != 0) {
                    state = object->state;
                    if (WORLD->status[state->index] >= 3) {
                        overlay36SpawnOffsetA9(D_1D9C);
                    }
                }
            }
            CASE_END;
        case 6:
            object = overlay1FindNextAngle(WORLD->angle);
            if (object != 0) {
                candidateState = object->state;
                state = candidateState;
                difference = overlay1WrapOffset(WORLD->angle, state->angle);
                if ((difference <= 3.0f) &&
                    (WORLD->status[state->index] >= 3) &&
                    (state->group == WORLD->group)) {
                    overlay36CallModeZero(D_1D9C);
                }
            }
            CASE_END;
        case 7:
            overlay36TickState(D_1D9C);
            CASE_END;
        case 8:
            overlay36UpdatePeers(D_1D9C);
            CASE_END;
        case 9:
            overlay36SpawnFinalEffect(D_1D9C);
            CASE_END;
    }
    return 0;
}





/* ---- overlay1HandleCachedMode ---- */

typedef struct W { u8 p0[0xD4]; void *object; u8 pD8[0xC3]; u8 enabled; s32 state; } W;
extern s32 D_83E4;
extern s32 overlay27CanUse(void *);
extern s32 overlay3RunCachedModeAction(void *, W *);
extern s32 overlay1DispatchMode(void);
/* The clear path is the fall-through of both tests, so the short-circuit `||`
 * is what puts it there: IDO branches to the "then" block when the first
 * disjunct holds and past it when the second fails, which is exactly the
 * target's `beqzl enabled -> clear` / `beqz canUse -> dispatch` pair. Written
 * with the dispatch as the "then" instead, the clear block lands after the
 * dispatch block and costs one extra branch to reach the epilogue. The
 * world pointer is re-read after the call because it is caller-saved. */
s32 overlay1HandleCachedMode(void) {
    if ((((W *)D_1DA0)->enabled == 0) ||
        (overlay27CanUse(((W *)D_1DA0)->object) != 0)) {
        ((W *)D_1DA0)->state = 0;
        return 0;
    }
    if (D_83E4 == 3) {
        return overlay3RunCachedModeAction(D_1D9C, (W *)D_1DA0);
    }
    return overlay1DispatchMode();
}

/* ---- overlay1ChooseModeObject ---- */


typedef struct O1SelectState { s8 tableIndex; } O1SelectState;
typedef struct O1SelectObject { u8 pad00[0x64]; O1SelectState *state; } O1SelectObject;
typedef struct O1SelectEntry { f32 value; u8 pad04[8]; } O1SelectEntry;
typedef struct O1SelectRow { O1SelectEntry entries[6]; } O1SelectRow;
typedef struct O1Selection { O1SelectObject *object; s16 value; } O1Selection;
typedef struct O1SelectWorld {
    s8 row;
    u8 pad01[0x381];
    u8 mode;
    u8 pad383[0xD];
    O1Selection selection;
    u8 pad398[0x4C];
    O1SelectObject *selected;
} O1SelectWorld;

extern O1SelectObject **func_80005750(s32 *count);
extern s32 mathRnd(s32 minimum, s32 maximum);

s32 overlay1ChooseModeObject(void) {
    s32 count;
    s32 remaining;
    O1SelectObject *object;
    s32 choiceCount;
    O1Selection *selection;
    O1SelectObject *choices[5];
    O1SelectObject **objects;

    objects = func_80005750(&count);
    choiceCount = 0;
    remaining = count--;
    while (remaining != 0) {
        object = objects[count];
        {
            O1SelectState *state = object->state;
            if (object != D_1D9C) {
                if (D_1BA8[((O1SelectWorld *)D_1DA0)->row]
                        .entries[state->tableIndex]
                        .value < 600.0f) {
                    choices[choiceCount++] = object;
                }
            }
        }
        remaining = count--;
    }
    if (choiceCount != 0) {
        count = mathRnd(1, choiceCount) - 1;
        object = choices[count];
        selection = &((O1SelectWorld *)D_1DA0)->selection;
        selection->object = object;
        selection->value = mathRnd(0x5A, 0x84);
        ((O1SelectWorld *)D_1DA0)->mode = 5;
        ((O1SelectWorld *)D_1DA0)->selected = object;
        return 1;
    }
    return 0;
}

/* ---- overlay1UpdateCountdown ---- */


typedef struct Overlay1Countdown {
    u8 pad0[4];
    u16 value;
} Overlay1Countdown;

typedef struct Overlay1CountdownObject {
    u8 pad0[0x390];
    Overlay1Countdown countdown;
} Overlay1CountdownObject;

extern Overlay1CountdownObject *gOverlay1CountdownObject;
extern s32 gOverlay1CountdownAmount;
extern void *gOverlay1CountdownResource;
extern void overlay1CountdownReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this countdown update. */
void overlay1UpdateCountdown(void) {
    u8 *object;
    s32 amount;
    register s32 mode;
    u16 countdown;

    object = (u8 *)gOverlay1CountdownObject;
    amount = gOverlay1CountdownAmount;
    countdown = *(u16 *)(object + 0x394); mode = 0x78; object += 0x390;
    /* Legacy inert spelling present in the retained exact C producer; whether
     * it is codegen-required has not been isolated. Tracked in
     * docs/cleanup-queue.md. */
    if ((((u32)object & mode) != 0) && (object == 0)) {
    }
    if (countdown <= amount) {
        overlay1CountdownReloc(gOverlay1CountdownResource, mode);
    } else {
        *(u16 *)(object + 4) = countdown - amount;
    }
}

/* ---- overlay1ReadSelection ---- */


typedef struct Overlay1Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay1Vector;

typedef struct Overlay1Descriptor {
    u8 pad00[0x2D];
    u8 count;
} Overlay1Descriptor;

typedef struct Overlay1Selection {
    Overlay1Descriptor *descriptor;
    u8 pad04[4];
    s16 useObjectPosition;
    u8 pad0A[0x36];
    Overlay1Vector *vectors;
} Overlay1Selection;

typedef struct Overlay1Object {
    u8 pad00[0x0C];
    Overlay1Vector position;
    u8 pad18[0x22];
    s8 selectedIndex;
    u8 pad3B[0x0D];
    u8 *fallback;
    u8 pad4C[0x1C];
    Overlay1Selection **selections;
} Overlay1Object;

void overlay1ReadSelection(Overlay1Object *object, s32 index, f32 *outX,
                           f32 *outY, f32 *outZ) {
    Overlay1Selection *selection;
    Overlay1Descriptor *descriptor;
    Overlay1Vector *vectors;
    s32 offset;

    selection = object->selections[object->selectedIndex];
    if (selection != 0) {
        descriptor = selection->descriptor;
        if (selection->useObjectPosition != 0) {
            *outX = object->position.x;
            *outY = object->position.y;
            *outZ = object->position.z;
            return;
        }
        if (descriptor->count >= index) {
            vectors = selection->vectors;
            offset = index * sizeof(Overlay1Vector);
            if (vectors == 0) {
                return;
            }
            *outX = *(f32 *)((u8 *)vectors + offset + 0);
            *outY = *(f32 *)((u8 *)selection->vectors + offset + 4);
            *outZ = *(f32 *)((u8 *)selection->vectors + offset + 8);
            return;
        }
    }

    *outX = *(f32 *)(object->fallback + 0x24);
    *outY = *(f32 *)(object->fallback + 0x28);
    *outZ = *(f32 *)(object->fallback + 0x2C);
}

/* ---- overlay1SolveAngleCandidates ---- */


extern f32 overlay1SqrtReloc(f32 value);
extern s32 overlay1AngleReloc(f32 y, f32 x);

/* The root count is defined before the discriminant region. Keep the signed
 * radical separate from the quotient, and the switch cases in this order. */
s16 overlay1SolveAngleCandidates(
    f32 x0, f32 y0, f32 x1, f32 y1,
    f32 y2, f32 x2, f32 radius, f32 slope, s32 chooseHigh) {
    f32 dx;
    f32 dy;
    f32 distance;
    f32 sum;
    f32 discriminant;
    f32 discriminantRoot;
    f32 denominator;
    f32 root;
    f32 angleX;
    s32 solutionCount;
    s16 solutions[2];
    s32 sign;

    solutionCount = 0;
    sign = 2;
    dx = x0 - y1;
    dy = x1 - x2;
    distance = overlay1SqrtReloc((dx * dx) + (dy * dy));
    dy = y2 - y0;
    sum = (dy * slope) + (radius * radius);
    discriminant = (sum * sum) -
        ((slope * slope) * ((distance * distance) + (dy * dy)));

    if (discriminant >= 0.0f) {
        discriminantRoot = overlay1SqrtReloc(discriminant);
        denominator = (((dy * dy) / (distance * distance)) + 1.0f) * 2.0f;

        while (sign--) {
            if (sign != 0) {
                dx = discriminantRoot;
            } else {
                dx = -discriminantRoot;
            }
            root = (dx + sum) / denominator;
            if (root >= 0.0f) {
                angleX = overlay1SqrtReloc(root);
                if (distance < 0.0f) {
                    angleX = -angleX;
                }
                solutions[solutionCount] = overlay1AngleReloc(
                    ((dy / distance) * angleX) -
                    ((slope * distance) / (angleX + angleX)), angleX);
                solutionCount++;
            }
        }
    }

    switch (solutionCount) {
        case 2:
            if (solutions[0] < solutions[1]) {
                return chooseHigh ? solutions[1] : solutions[0];
            }
            return chooseHigh ? solutions[0] : solutions[1];
        case 1:
            return solutions[0];
        default:
            return 0x2000;
    }
}


/* ---- overlay1UpdateModeSound ---- */


typedef struct Overlay1SoundModeObject {
    u8 pad0[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
} Overlay1SoundModeObject;

extern Overlay1SoundModeObject *gOverlay1ModeObject;
extern void *gOverlay1ModeResource;
extern void overlay1ModeSoundReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this mode-gated wrapper. */
void overlay1UpdateModeSound(void) {
    Overlay1SoundModeObject *object;

    object = gOverlay1ModeObject;
    if (object->mode == 13) {
        if (object->flags & 2) {
            overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
        }
    } else {
        overlay1ModeSoundReloc(gOverlay1ModeResource, 0x78);
    }
}

/* ---- overlay1CopyBytes ---- */


/* DKR v77/v80 contains only generic byte-copy initialization patterns. */
typedef struct Overlay1ByteState {
    u8 values[6];
} Overlay1ByteState;

typedef struct Overlay1ByteObject {
    u8 pad0[0x64];
    Overlay1ByteState *state;
} Overlay1ByteObject;

typedef struct Overlay1ByteInit {
    u8 pad0[0xA];
    u8 values[6];
} Overlay1ByteInit;

void overlay1CopyBytes(Overlay1ByteObject *object, Overlay1ByteInit *init) {
    Overlay1ByteState *state = object->state;

    state->values[0] = init->values[0];
    state->values[1] = init->values[1];
    state->values[2] = init->values[2];
    state->values[3] = init->values[3];
    state->values[4] = init->values[4];
    state->values[5] = init->values[5];
}

/* ---- overlay1UpdateRangeFlags ---- */


typedef struct Overlay1RangeConfig {
    u8 angleHigh;
    u8 horizontalScale;
    u8 verticalScale;
    u8 mode;
    u8 soundId;
} Overlay1RangeConfig;

typedef struct Overlay1RangeState {
    u8 pad000[0x1A8];
    u16 flags;
} Overlay1RangeState;

typedef struct Overlay1HeightData {
    u8 pad000[0x5C];
    f32 height;
} Overlay1HeightData;

typedef struct Overlay1RangeObject {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x30];
    Overlay1HeightData *heightData;
    u8 pad04C[0x18];
    void *state;
} Overlay1RangeObject;

extern Overlay1RangeObject **overlay1GetObjectListReloc(s32 *count);
extern s32 overlay1GetAngleValueReloc(f32 dz, f32 dx);
extern void overlay1ActivateObjectReloc(Overlay1RangeObject *object);
extern void overlay1PlaySoundReloc(u8 soundId);

/* Plateau: exact 120 instructions, the 0x70 frame, and every allocator lane --
 * general pool 37/37, general temp 8/8, FP pool 7/7, FP temp 9/9 -- with two
 * words left. Three identities were proved here. The horizontal range squared
 * must be a named `f32`, and its two `config->horizontalScale * 10U` reads must
 * be spelled twice so IDO CSEs them: a `u32 horizontalRange` carrier spends the
 * declaration budget the `f32` needs and leaves the whole FP allocation wrong
 * (12 FP words). The angle base must be a named `u8` carrier, which is what
 * puts `config->angleHigh` on the target's pool colour instead of a ring temp.
 * The `case 1` test must be a named `u16`, which orders its `andi` web before
 * the store's. `clearMask` is not needed: IDO hoists a literal `~8` into the
 * same saved register, and dropping the declaration is what buys the budget for
 * the other two.
 *
 * The residual is two words, and `cc -K` names the mechanism exactly. ugen
 * numbers `case 0`'s two temps in emission order -- `and $9` for the test,
 * `or $10` for the store -- but numbers `case 1`'s backwards: without the test
 * carrier it emits `and $12` for the test and `and $11` for the store, so the
 * store is allocated first and the pair comes out swapped (four words). The
 * `u16` carrier fixes the order, because its truncation `and $x, $y, 65535`
 * takes the outer number $11 and as1 then folds the instruction away -- but it
 * spends $12 on the inner `and`, so the store slides to $13 and lands on t5
 * where the target has t4. One temp too many, in the right order; the
 * no-carrier form has the right count in the wrong order.
 *
 * The calls in `case 1`'s body are not the cause: removing one or both leaves
 * the $12/$11 inversion unchanged. Measured and flat, do not repeat: 40 case-1
 * body spellings (compound assignment, the `^ 0` use-site break, re-reads of
 * the field, a hoisted `cleared` local, five carrier types, `if/else if` in
 * place of the switch, a `default:` arm, and reversed case order), and all 96
 * physical line groupings of the case-1 statement list.
 *
 * 2026-09-09: the residual is not in `case 1` at all. ugen's temporary ring is
 * fresh-first over $8..$15,$24,$25 and then FIFO by free time, and both arms
 * draw from that one list in emission order, so the numbers each arm gets are
 * fixed by when the *earlier* temps were freed. Reading `cc -K` for the whole
 * loop body: the angle block emits `sll $9; sll $10; sra $11` for the left
 * operand, `sll $12; sra $13` for the s16 read of `angle`, then
 * `addu $4,$11,$13`, then `sll $14/sra $15` for the truncation. $12 is freed by
 * `sra $13,$12,16` and $11 only at the `addu`, so the queue reaching the switch
 * is $9,$10,$12,$11,$13. `case 0` takes $9,$10 and `case 1` therefore takes
 * $12,$11 -- the inversion, entirely inherited. The target's queue must be
 * $9,$10,$11,$12.
 *
 * That is reachable, and was reached: with `angle` read through a one-
 * instruction conversion the free order becomes ascending and BOTH arms are
 * exact -- `andi t3` and `and t4` in `case 1`, `andi t1` and `ori t2` in
 * `case 0`, and the shared lane matches 7/7. What then remains is two different
 * words: the sum's `addu` writes a ring temp where the target writes the pool
 * colour (`addu t5,t3,v0` against `addu a0,t3,v0`).
 *
 * The constraint that blocks it, and it is structural, not a search gap. The
 * object pins three things: the left chain must end at $11, the sign-extension
 * pair must be $14 and $15, and the `addu` must write the pool. Five ring temps
 * therefore have to be spent between them, the left chain owns three, and the
 * remaining two have to be freed after $11 -- which is freed at the `addu`. Any
 * second operand needing a two-instruction conversion frees its first temp
 * before the `addu` (the current inversion); a one-instruction conversion frees
 * it at the `addu` but leaves the count one short, and the extra instruction
 * that would make up the count sits between the `addu` and the sign extension,
 * where as1 removes it by coalescing backwards onto the `addu` and renaming its
 * destination. 1080 spellings of `angle`'s type, the left operand, the read,
 * the assignment cast and the comparison were scored against the full-TU object
 * and the floor is exactly 2 in every one of them.
 *
 * Next lever: this needs an instruction between the `addu` and the sign
 * extension that as1 deletes without back-coalescing -- i.e. one whose
 * destination is consumed by the next instruction rather than written back into
 * `angle`'s home. Every cast spelling reachable from C emits ugen's
 * write-back form (`op $13,$4,..; move $4,$13`). Look for a source shape where
 * the intermediate is not the variable itself, or accept that the owner is as1
 * and reach for a ugen/as1 trace. Do not re-search `case 1`.
 *
 * 2026-09-10 (second reader): the residual reproduces at exactly 2 words, both
 * sites one web, and the diagnosis above holds. One thing worth writing down
 * because it reads as a third difference and is not: the comparison reports a
 * hunk at the first call where the two sides name different symbols. That is a
 * relocation-naming artifact -- the target side carries the generic overlay
 * entry symbol at every R_MIPS_26 site while the candidate carries the real
 * callee -- and those words are masked, which is why the raw and the masked
 * counts both read 2. Do not spend a cycle on it.
 *
 * 2026-09-12, lane p10-tight. The 2026-09-11 reading that the free list is
 * ascending here and the residual lives inside the second switch arm is wrong;
 * see the handoff shard. ugen draws a ring register immediately before each
 * instruction it emits, so the listing order IS the draw order, and the angle
 * block hands the switch its fourth and third ring members transposed. A
 * two-word corner exists in which BOTH switch arms are byte-exact and the
 * residual is the sum's destination instead: declare the angle thirty-two-bit,
 * spell the right summand as an explicit sixteen-bit mask of it so its widening
 * costs one ring draw rather than two, and write the second arm with no carrier
 * at all. The block then owes one more zero-footprint ring draw, strictly
 * between the sum and the truncation, and it cannot be paid: as1 deletes a
 * no-op by renaming ITS PRODUCER'S destination, so a phantom placed on the sum
 * renames the sum off the pool colour, and a phantom appended to the left
 * operand's chain is folded into it and moves the survivor one slot on. That is
 * the same mechanism the note above calls back-coalescing, measured from the
 * other side. Only a zero-footprint draw on some other live narrow value would
 * pay, and nothing narrow is live there.
 *
 * A second corner confirms the reading independently: a redundant byte mask on
 * the angle-high read, inside the left operand, buys the fifth draw and makes
 * the switch byte-exact in both arms, with everything from the truncation
 * onwards exact too. It scores 3 because the draw was spent at the HEAD of the
 * chain -- as1 deletes the no-op by renaming the byte load's destination, so
 * the load loses its pool colour, and the chain's survivor moves one slot on,
 * taking the shift and the sum's first operand with it. The fifth draw has to
 * be the fifth.
 *
 * 2026-09-17, lane w2-o001. L145-L154 reopen re-measured the same 2 naming
 * words at +0x190, 33 GP/FP draws, 174 ugen emissions. Deleting rangeSquared
 * or otherState regresses (14 and a size-minus-one 109). s16 plus a one-draw
 * (u16) right operand drops one comparison draw and shifts the tail to 14;
 * every post-sum probe meant to buy that draw back is DCE'd, adds two draws
 * (19), or hoists mode and goes structural (41). s32 corners are 20-21, not
 * the recorded two-word switch-exact shape. Floor remains 2. */
#ifdef NON_MATCHING
void overlay1UpdateRangeFlags(Overlay1RangeObject *object, void *unused) {
    Overlay1RangeConfig *config;
    s32 count;
    Overlay1RangeObject **objects;

    config = object->state;
    objects = overlay1GetObjectListReloc(&count);
    if (count--) {
        do {
            Overlay1RangeObject *other;
            Overlay1RangeState *otherState;
            f32 dx;
            f32 dz;
            f32 rangeSquared;
            s16 angle;
            u8 angleHigh;

            other = objects[count];
            otherState = other->state;
            dx = other->x - object->x;
            dz = other->z - object->z;
            rangeSquared = (f32)(s32)(((u32)config->horizontalScale * 10U) *
                                      ((u32)config->horizontalScale * 10U));
            if ((dx * dx + dz * dz) < rangeSquared) {
                angle = overlay1GetAngleValueReloc(dz, dx);
                angleHigh = config->angleHigh;
                angle = (s16)((u32)angleHigh << 8) + angle;
                if ((angle < -0x4000) || (angle >= 0x4001)) {
                    if ((object->y <= other->y + other->heightData->height) &&
                        (other->y <= object->y +
                         (f32)(s32)((u32)config->verticalScale * 10U))) {
                        switch (config->mode) {
                            case 0: {
                                u16 flags;
                                flags = otherState->flags;
                                if (!(flags & 8)) {
                                    otherState->flags = flags | 8;
                                }
                                break;
                            }
                            case 1: {
                                u16 flags;
                                u16 masked;
                                flags = otherState->flags;
                                masked = flags & 8;
                                if (masked) {
                                    otherState->flags = flags & ~8;
                                    overlay1ActivateObjectReloc(other);
                                    overlay1PlaySoundReloc(config->soundId);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        } while (count--);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00067C0_1852BA0.s")
#endif

/* ---- overlay1InitMotion ---- */


/* No corresponding DKR/JFG source or object match was found. */
typedef struct Overlay1MotionState {
    f32 magnitude;
    s32 mode;
    u16 first;
    u16 second;
} Overlay1MotionState;

typedef struct Overlay1MotionObject {
    u8 pad0[8];
    f32 scaledMagnitude;
    u8 padC[0x58];
    Overlay1MotionState *state;
} Overlay1MotionObject;

typedef struct Overlay1MotionInit {
    u8 pad0[0xA];
    u8 magnitude;
    u8 mode;
    u16 first;
    u16 second;
} Overlay1MotionInit;

extern f32 gOverlay1MotionScale;

void overlay1InitMotion(Overlay1MotionObject *object, Overlay1MotionInit *init) {
    Overlay1MotionState *state;

    state = object->state;
    object->scaledMagnitude = (f32)init->magnitude * gOverlay1MotionScale;
    state->magnitude = (f32)init->magnitude;
    state->mode = init->mode;
    state->first = init->first;
    state->second = init->second;
}

/* ---- overlay1ConsumeNearbyPending ---- */


typedef struct Overlay1NearbyState {
    f32 radius;
    s32 mode;
    u16 kind;
} Overlay1NearbyState;

typedef struct Overlay1OtherState {
    s8 kind;
    u8 pad01[0x191];
    u8 pending;
    u8 pad193[0x221];
    s16 count;
} Overlay1OtherState;

typedef struct Overlay1NearbyObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    void *state;
} Overlay1NearbyObject;

/* Plateau: the exact 69-instruction extent, the 0x48 frame and now the exact
 * opcode schedule -- the verdict crossed structure-mismatch into
 * allocation-mismatch. Three things buy that and all three are needed: the
 * counter is `volatile`, so every read is its own load from sp+60 and every
 * write its own store, which is what the target does at all six sites; the
 * loop is `if (count--) { do ... while (count--); }`, which reads the counter
 * once for the test and the decrement (the previous record's finding that this
 * form costs an instruction holds only for a plain `s32`, where it needs an
 * extra copy); and the inner `object = objectArg` copy is gone, because
 * `volatile` costs eight bytes of frame and dropping that declaration is what
 * pays for it.
 *
 * The residual is 31 register-only words with one cause. uopt gives our
 * counter read a pool web, so it lands on v1 and `other`/`otherState` swap
 * colours behind it; the target spends a ugen ring temp (t6) at every counter
 * read and has six fewer pool webs -- pool lanes 16 against the target's 10,
 * ring lanes 4 against 10. Measured and flat, do not repeat: plain and
 * volatile counters crossed with five loop shapes; the counter reached through
 * `*(s32 *)&count`, `*(volatile s32 *)&count`, a plain `s32 *` local and a
 * `volatile s32 *` local; inlining `otherState`, inlining `other`, reversing
 * the kind comparison, reversing the two declarations, dropping the `state`
 * local, caching the list base, and an extra `mode` web ahead of the counter
 * read. Next lever is whatever stops uopt webbing that read.
 *
 * 2026-09-10: the single cause above is confirmed, the flag lattice is now
 * closed, and the web is narrowed from per-variable to per-load.
 *
 * The flag sweep had never been run on this function. It has been: 119
 * combinations, every one nonexact, and the project's own preset is the best
 * row. The residual is not a flag.
 *
 * The web is per-load, not per-variable. Splitting the counter across two
 * distinct union members -- one read by the head test, the other by the latch
 * -- is byte-flat at 31. So uopt is not unifying the head and latch reads into
 * a single web; it webs each volatile load of this stack local separately, and
 * they share a colour only because they do not interfere. Work aimed at
 * breaking that unification is wasted, because there is none.
 *
 * The residual restated as an allocation fact, which is the useful form: the
 * target spends its two lowest pool colours on `other` and `otherState`, which
 * leaves ugen's ring as the only home for the counter reads and starts that
 * ring at its first slot for the head read. The candidate spends the lower
 * colour on the head counter read instead, so `other` and `otherState` take
 * the same two colours in the opposite order and every ring value slides one
 * position. One extra pool web at the head explains all 31 words.
 *
 * Also measured and flat, do not repeat: `count` as int, long and unsigned;
 * casts and coercions around the decrement in the head, the latch, or both;
 * casts on the index read; the getter's argument cast; `while (count--)` and
 * `for (; count--; )`, which cfe rotates into exactly the same if/do-while, so
 * loop shape is not a lever here at all; reversing the kind comparison and the
 * mode comparison. Naming the loaded value in an explicit read-modify-write
 * pair costs two instructions in every read/write volatility combination.
 *
 * The next lever is unchanged but sharper: find what makes uopt reserve those
 * two pool colours for `other` and `otherState` across the whole function. A
 * matched precedent with the same counter idiom, overlay3ResetObjects, does
 * the opposite -- its head read takes a pool colour and only its latch read
 * takes a ring temp -- so IDO reaches both outcomes from the same source shape
 * and the difference lives in this function's loop-body variables, not in how
 * the counter is spelled. */
#ifdef NON_MATCHING
void overlay1ConsumeNearbyPending(void *objectArg, void *listArg) {
    Overlay1NearbyState *state;
    f32 radiusSquared;
    volatile s32 count;
    Overlay1NearbyObject *other;
    Overlay1OtherState *otherState;
    state = ((Overlay1NearbyObject *)objectArg)->state;
    radiusSquared = state->radius * 4.0f;
    radiusSquared *= state->radius * 4.0f;
    listArg = overlay1GetObjectListReloc((s32 *)&count);
    if (count--) {
        do {
            other = ((Overlay1NearbyObject **)listArg)[count];
            otherState = other->state;
            if (state->kind == otherState->kind) {
                f32 dx = other->x - ((Overlay1NearbyObject *)objectArg)->x;
                f32 dy = other->y - ((Overlay1NearbyObject *)objectArg)->y;
                f32 dz = other->z - ((Overlay1NearbyObject *)objectArg)->z;
                if (((dx * dx) + (dy * dy) + (dz * dz) < radiusSquared) &&
                    (state->mode == 2)) {
                    u8 pending = otherState->pending;
                    if (pending) {
                        otherState->pending = 0;
                        otherState->count += pending;
                    }
                }
            }
        } while (count--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0006A14_1852DF4.s")
#endif

/* ---- overlay1InitRange ---- */


/* Compact range initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay1InitRangeState { s16 a, b; s32 value; u8 c, d; } Overlay1InitRangeState;
typedef struct Overlay1InitRangeObject { u8 pad0[0x64]; Overlay1InitRangeState *state; } Overlay1InitRangeObject;
typedef struct Overlay1RangeInit { u8 pad0[0xA]; u8 a, b, value, c, d; } Overlay1RangeInit;
void overlay1InitRange(Overlay1InitRangeObject *object, Overlay1RangeInit *init) {
    Overlay1InitRangeState *state = object->state;
    state->a = init->a * 10; state->b = init->b * 10;
    state->value = init->value; state->c = init->c; state->d = init->d;
}

/* ---- overlay1SearchNearby ---- */


typedef struct Overlay1SearchState {
    u16 xRange;
    u16 zRange;
    u8 pad04[2];
    u8 flags;
    u8 pad07;
    u8 lookupKey;
    u8 pad09[2];
    u8 active;
    u8 pad0C[0x3A8];
    s16 counter;
} Overlay1SearchState;

typedef struct Overlay1SearchObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    Overlay1SearchState *state;
} Overlay1SearchObject;

extern Overlay1SearchObject **func_8000572C(s32 *first, s32 *limit);
extern Overlay1SearchObject *func_80005820(u8 key);
extern void overlay4RemoveObject(Overlay1SearchObject *object);

void overlay1SearchNearby(Overlay1SearchObject *object, void *unused) {
    Overlay1SearchState *range;
    s32 first;
    s32 limit;
    s32 index;
    Overlay1SearchObject **objects;
    Overlay1SearchObject *candidate;
    Overlay1SearchState *state;
    Overlay1SearchObject *linked;
    f32 delta;
    f32 threshold;

    (void)unused;
    range = object->state;
    objects = func_8000572C(&first, &limit);
    index = first;
    if (index < limit) {
        do {
            candidate = objects[index];
            if (candidate->type == 0x21) {
                delta = candidate->x - object->x;
                state = candidate->state;
                threshold = range->xRange;
                if (delta < 0.0f) {
                    delta = -delta;
                }
                if (delta <= threshold) {
                    delta = candidate->z - object->z;
                    threshold = range->zRange;
                    if (delta < 0.0f) {
                        delta = -delta;
                    }
                    if (delta <= threshold) {
                        linked = func_80005820(range->lookupKey);
                        if (linked != 0) {
                            linked->state->counter++;
                        }
                        state->flags |= 4;
                        state->active = 1;
                        overlay4RemoveObject(candidate);
                        return;
                    }
                }
            }
            index++;
        } while (index != limit);
    }
}

/* ---- overlay1SelectMaskedMode ---- */


typedef struct Overlay1MaskedState {
    u8 pad0[0x382];
    u8 bitIndex;
    u8 pad383[0xD];
    u8 timer;
} Overlay1MaskedState;

typedef struct Overlay1MaskedObject {
    u8 pad0[0x64];
    Overlay1MaskedState *state;
} Overlay1MaskedObject;

extern u8 gOverlay1ModeMasks[];
extern void overlay1SelectModeReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this table-mask selector. */
s32 overlay1SelectMaskedMode(Overlay1MaskedObject *object, s32 index) {
    Overlay1MaskedState *state;

    state = object->state;
    if (*(u16 *)(gOverlay1ModeMasks + index * 12 + 0xC4) &
        (1 << state->bitIndex)) {
        state->bitIndex = index;
        overlay1SelectModeReloc(&state->timer, 8);
        return 1;
    }
    return 0;
}

/* ---- overlay1UpdateAimedTransient ---- */


typedef struct Overlay1TransientState {
    void *owner;
    s16 mode;
    u8 type;
    u8 active;
    u8 selector;
    s8 linkedIndex;
    u8 pad0A;
} Overlay1TransientState;

typedef struct Overlay1MotionSourceExtra {
    u8 pad00[0x5C];
    f32 height;
} Overlay1MotionSourceExtra;

typedef struct Overlay1MotionSourceState {
    u8 pad00;
    s8 index;
} Overlay1MotionSourceState;

typedef struct Overlay1MotionSource {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x20];
    Overlay1MotionSourceExtra *extra;
    u8 pad4C[0x18];
    Overlay1MotionSourceState *state;
} Overlay1MotionSource;

typedef struct Overlay1VelocityExtra {
    u8 pad00[0xE0];
    f32 *value;
} Overlay1VelocityExtra;

typedef struct Overlay1TransientObject {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x18];
    Overlay1VelocityExtra *extra;
    u8 pad44[0x20];
    Overlay1TransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1TransientObject;

typedef struct Overlay1TransientOwner {
    s16 angle;
    u8 pad02[0x26];
    f32 distance;
} Overlay1TransientOwner;

typedef struct Overlay1TransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1E6];
    Overlay1MotionSource *source;
    Overlay1TransientObject *object;
    u8 pad398[0x84];
    u32 status;
} Overlay1TransientWorld;

extern f32 D_4;
extern f32 overlay1AimedScaleReloc;
extern f32 overlay1AimedThresholdReloc;
extern f32 overlay1AimedTrigReloc;
extern f32 overlay1AimedVelocityYReloc;

/* Every call below that the shipped module records as a SYMBOL relocation
 * (stored word `jal 0`, patched at load) goes through a placeholder that
 * `gmake overlay-syms` values at 0xF0000000 -- the three same-module callees
 * included, since a same-module call can still be a SYMBOL record. */
extern Overlay1TransientObject *overlay36SpawnTransientReloc(
    Overlay1TransientOwner *owner, Overlay1TransientWorld *world);
extern s16 overlay1SolveAngleCandidatesReloc(
    f32, f32, f32, f32, f32, f32, f32, f32, s32);
extern void overlay1ReadSelectionReloc(Overlay1Object *object, s32 index, f32 *outX, f32 *outY, f32 *outZ);
extern void overlay1InitTimedStateReloc(Overlay1TimedObject *object, s32 timer);
extern s32 func_8002A910(f32 y, f32 x);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);

/* Matched 2026-09-16 (lane lm-b). The last residual, fourteen positional
 * words in the prologue, was one assembler directive: the target loads the
 * shared-world pointer through a uopt-created address web, and ugen stamps
 * such a load with `.noalias <reg>,$sp`, so as1 schedules it above the
 * register saves. A declared `u32` address carrier keeps the web but launders
 * the provenance, ugen emits no directive, and as1 gives the load an ordering
 * edge from every save (measured on `cc -S` output: removing the directive from
 * a byte-exact sibling prologue in overlay 14 drops its load below ten saves;
 * adding it to this function's listing lifts the load into the target's slot).
 *
 * uopt builds that web only for a global read twice in a procedure that also
 * contains a loop, and it then serves every read of that symbol; the target
 * serves two reads from the web and four from plain symbol loads, so the two
 * groups must be spelled through different symbols at the same address. The
 * head read and the store site use `D_1DA0`; the four reloads use the
 * `D_1DA0_array` alias this TU already declares. The ROM cannot distinguish
 * the two symbols, so which group carries which name is a reconstruction
 * choice; both assignments produce identical text. The unused `owner`
 * declaration supplies the frame's last eight bytes, which the old carrier had
 * been occupying, and it must precede `savedState` for that home to land. */
void overlay1UpdateAimedTransient(void) {
    Overlay1TransientOwner *owner;
    Overlay1TransientWorld *world;
    Overlay1TransientState *savedState;
    Overlay1TransientObject *object;
    Overlay1TransientState *state;
    Overlay1MotionSource *source;
    f32 factor;
    f32 predictedX;
    f32 predictedY;
    f32 predictedZ;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;
    f32 trig;
    s32 iteration;
    s16 sourceAngle;
    s16 objectAngle;

    world = D_1DA0;
    object = world->object;
    source = world->source;
    if (object == 0) {
        object = overlay36SpawnTransientReloc(D_1D9C, world);
        if (object != 0) {
            state = object->state;
            state->owner = D_1D9C;
            state->type = 3;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            object->scale = overlay1AimedScaleReloc;
            overlay1ReadSelectionReloc(D_1D9C, 9, &object->x, &object->y,
                                       &object->z);
            ((Overlay1TransientWorld *)D_1DA0)->object = object;
            savedState = state;
        }
        world = (Overlay1TransientWorld *)D_1DA0_array[0];
        state = savedState;
    } else {
        state = object->state;
    }

    if ((world->mode == 0xD) && state->active &&
        (overlay1AimedThresholdReloc <=
         ((Overlay1TransientOwner *)D_1D9C)->distance)) {
        state->active = 0;
        state->mode = 0xC;
        if (source != 0) {
            state->linkedIndex = source->state->index;
            factor = 0.0f;
            iteration = 3;
            do {
                predictedX = source->x + (factor * source->velocityX);
                predictedY = source->y + (factor * source->velocityY) +
                    (source->extra->height * 0.5f);
                predictedZ = source->z + (factor * source->velocityZ);
                deltaX = predictedX - object->x;
                deltaY = predictedY - object->y;
                deltaZ = predictedZ - object->z;
                distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                                 (deltaZ * deltaZ));
                factor = 30.0f / distance;
                if (factor > 0.0f) {
                    deltaX *= factor;
                    deltaZ *= factor;
                }
                factor = distance / (30.0f * D_4);
            } while (iteration--);

            sourceAngle = func_8002A910(deltaX, deltaZ);
            objectAngle = overlay1SolveAngleCandidatesReloc(
                object->x, object->y, object->z,
                predictedX, predictedY, predictedZ,
                30.0f, -*object->extra->value, 0);
            trig = func_8002A8BC(objectAngle);
            object->velocityX = func_8002A8C0(sourceAngle) * trig * 30.0f;
            object->velocityY = func_8002A8C0(objectAngle) * 30.0f;
            trig = func_8002A8BC(objectAngle);
            object->velocityZ = func_8002A8BC(sourceAngle) * trig * 30.0f;
        } else {
            state->linkedIndex = -1;
            factor = func_8002A8C0(((Overlay1TransientOwner *)D_1D9C)->angle);
            trig = overlay1AimedTrigReloc;
            if (1) {
                object->velocityX = factor * trig * -30.0f;
                object->velocityY = overlay1AimedVelocityYReloc;
                object->velocityZ =
                    func_8002A8BC(((Overlay1TransientOwner *)D_1D9C)->angle) * trig *
                    -30.0f;
            }
        }
        *object->flags &= ~2;
        world = (Overlay1TransientWorld *)D_1DA0_array[0];
    }

    if (world->flags & 2) {
        if (world->mode == 0xD) {
            overlay1InitTimedStateReloc(D_1D9C, 0x78);
            world = (Overlay1TransientWorld *)D_1DA0_array[0];
        }
        if (world->mode == 0xD) {
            world->mode = 0xD;
            world = (Overlay1TransientWorld *)D_1DA0_array[0];
        }
    }
    if (!(world->status & 0x2000) && world->mode == 0xD) {
        world->mode = 0xD;
    }
}


/* ---- overlay1UpdateTransient ---- */


typedef struct Overlay1SimpleTransientState {
    void *owner;
    s16 mode;
    u8 type;
    u8 active;
    u8 selector;
    s8 linkedIndex;
    u8 pad0A;
} Overlay1SimpleTransientState;

typedef struct Overlay1SimpleTransientObject {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x3C];
    Overlay1SimpleTransientState *state;
    u8 pad68[0x10];
    s16 *flags;
} Overlay1SimpleTransientObject;

typedef struct Overlay1SimpleTransientOwner {
    u8 pad00[0x28];
    f32 distance;
} Overlay1SimpleTransientOwner;

typedef struct Overlay1SimpleTransientWorld {
    u8 pad00[0x193];
    u8 mode;
    u8 pad194[0x14];
    u16 flags;
    u8 pad1AA[0x1EA];
    Overlay1SimpleTransientObject *object;
} Overlay1SimpleTransientWorld;

extern f32 gOverlay1TransientScale;
extern f32 gOverlay1TransientThreshold;
extern f32 gOverlay1TransientVelocityY;
extern Overlay1SimpleTransientObject *overlay1TransientCallReloc();

void overlay1UpdateTransient(void) {
    Overlay1SimpleTransientObject *object;
    Overlay1SimpleTransientState *state;

    object = ((Overlay1SimpleTransientWorld *)D_1DA0)->object;
    if (object == 0) {
        object = overlay1TransientCallReloc(
            (Overlay1SimpleTransientOwner *)D_1D9C,
            (Overlay1SimpleTransientWorld *)D_1DA0);
        if (object != 0) {
            state = object->state;
            state->owner = D_1D9C;
            state->type = 2;
            state->active = 1;
            state->selector = 9;
            state->pad0A = 0;
            object->scale = gOverlay1TransientScale;
            overlay1TransientCallReloc(D_1D9C, 9, &object->x, &object->y,
                                       &object->z);
            ((Overlay1SimpleTransientWorld *)D_1DA0)->object = object;
        }
    } else {
        state = object->state;
    }

    if (((Overlay1SimpleTransientWorld *)D_1DA0)->flags & 2) {
        overlay1TransientCallReloc(D_1D9C, 0x78);
    }
    if ((((Overlay1SimpleTransientWorld *)D_1DA0)->mode == 0xD) &&
        (((Overlay1SimpleTransientOwner *)D_1D9C)->distance >= gOverlay1TransientThreshold) &&
        state->active) {
        state->active = 0;
        state->linkedIndex = -1;
        state->mode = 0xC;
        object->velocityX = 0.0f;
        object->velocityY = gOverlay1TransientVelocityY;
        object->velocityZ = 0.0f;
        *object->flags &= ~2;
    }
}

/* ---- overlay1AllocateRecord ---- */


typedef struct Overlay1PoolBits {
    u8 high;
    u8 group : 6;
    u8 unused : 1;
    u8 active : 1;
} Overlay1PoolBits;

typedef union Overlay1PoolFlags {
    u16 value;
    Overlay1PoolBits bits;
} Overlay1PoolFlags;

typedef struct Overlay1PoolRecord {
    u8 pad00[0xC0];
    Overlay1PoolFlags flags;
    u8 padC2[6];
    u32 value;
} Overlay1PoolRecord;

extern Overlay1PoolRecord *D_218;
extern Overlay1PoolRecord D_220[];
extern Overlay1PoolRecord D_1BA0[];
extern s32 D_1D88;
extern s32 gOverlay1PoolExhausted;
extern s32 D_1D84;

/* The redundant width mask is semantically inert for the u16 value; it
 * preserves IDO's shipped temporary-FIFO phase. See docs/cleanup-queue.md. */
Overlay1PoolRecord *overlay1AllocateRecord(void) {
    Overlay1PoolRecord *cursor;
    Overlay1PoolRecord *result;

    cursor = D_218;
    result = cursor;
    do {
        cursor = (D_218 = cursor + 1);
        if (cursor >= D_1BA0) {
            D_218 = D_220;
            cursor = D_220;
        }
        if (result == cursor) {
            D_1D84 = 1;
            return 0;
        }
    } while ((((u32)*((u8 *)cursor + 0xC1) >> 2) == D_1D88) &&
             (((cursor->flags.value & 0xFFFF) & 1) == 0));

    result->flags.bits.group = D_1D88;
    return result;
}

/* ---- overlay1CloneRecord ---- */


extern void *overlay1AllocateRecordReloc(u32 *source);

/* DKR v77/v80 and JFG have generic copy loops, but no exact donor. */
/* Exact C: all 23 instruction words, the -24 frame, relocation identity, and
 * linked overlay range match after bounded permutation. */
void *overlay1CloneRecord(u32 *source)
{
  u32 *destination;
  register u32 remaining;
  void *result;
  result = overlay1AllocateRecordReloc(source);
  if (result == 0)
  {
    remaining = 0;
    return remaining;
  }
  remaining = 50;
  {
    u32 *input;
    input = source;
    destination = result;
    remaining = 50;
    do
    {
      *(destination++) = *(input++);
    }
    while (remaining--);
  }
  return result;
}

/* ---- overlay1UpdateValueCache ---- */


typedef struct Overlay1CacheValueEntry {
    s16 keyA;
    s16 keyB;
    u32 value;
} Overlay1CacheValueEntry;

extern Overlay1CacheValueEntry gOverlay1ValueCache[64];

/* Matched 2026-08-29 after a bounded permuter found the zero-valued counter
 * expression that preserves IDO's target caller-saved web order. The stock
 * mixed-TU object is exact for all 120 words and both local relocations. */
s32 overlay1UpdateValueCache(s16 keyA, s16 keyB, f32 value) {
    register s32 searchKeyA = keyA;
    register s32 searchKeyB = keyB;
    Overlay1CacheValueEntry *entry;
    s32 remaining;

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if ((entry->value != 0) && (searchKeyA == entry->keyA) &&
            (searchKeyB == entry->keyB)) {
            if (value < (f32)entry->value) {
                entry->value = (u32)value;
                return 1;
            }
            return remaining * 0;
        }
        entry++;
    } while (remaining--);

    entry = gOverlay1ValueCache;
    remaining = 0x3F;
    do {
        if (entry->value == 0) {
            entry->keyA = searchKeyA;
            entry->keyB = searchKeyB;
            entry->value = (u32)value;
            return 1;
        }
        entry++;
    } while (remaining--);

    return 0;
}

/* ---- overlay1AppendPathPoint ---- */


typedef struct Overlay1PathState {
    s16 x[32];
    s16 y[32];
    u8 primary[32];
    u8 secondary[32];
    u8 count;
    u8 flags;
    u8 padC2[2];
    f32 length;
    u32 anchorDistanceSquared;
} Overlay1PathState;

extern f32 sqrtf(f32 value);
extern s32 overlay1UpdateValueCache(s16 x, s16 y, f32 value);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;

/* Exact C: 106 words, the 0x28 frame, and all eight relocation records. The
 * anchor block reuses `dx` and `dy` and reads the two anchor globals directly
 * at every site. Both halves are load-bearing and neither works alone: a named
 * `anchorX` cache adds a pool web that pushes the anchor colour off v1, and a
 * fresh delta local adds another that costs the ring phase, while reusing the
 * two already-declared deltas puts each value back in the web IDO gave it in
 * the length computation above. */
void overlay1AppendPathPoint(Overlay1PathState *state, s16 x, s16 y,
                             u8 primary, u8 secondary) {
    s32 pointX = x;
    s32 pointY = y;
    s16 dx = pointX - state->x[state->count];
    s16 dy = pointY - state->y[state->count];

    state->count = state->count + 1;
    state->x[state->count] = pointX;
    state->y[state->count] = pointY;
    state->primary[state->count] = primary;
    state->secondary[state->count] = secondary;
    state->length += sqrtf((f32)((dx * dx) + (dy * dy)));
    state->flags = (state->flags & ~3) | 1;

    if ((state->count >= 2) &&
        (overlay1UpdateValueCache(pointX, pointY, state->length) == 0)) {
        state->flags &= ~3;
        return;
    }

    if ((pointX == overlay1AnchorX) && (pointY == overlay1AnchorY)) {
        state->anchorDistanceSquared = 0;
    } else {
        dx = pointX - overlay1AnchorX;
        dy = pointY - overlay1AnchorY;
        state->anchorDistanceSquared = (dx * dx) + (dy * dy);
    }
}

/* ---- overlay1BendPathPoint ---- */


typedef struct Overlay1PathPoint {
    s16 x;
    s16 y;
} Overlay1PathPoint;

typedef struct Overlay1Path {
    Overlay1PathPoint *points;
    u32 count;
} Overlay1Path;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor. */
extern Overlay1Path *overlay1GetPathReloc(u8 selector);
extern s32 overlay1AngleReloc(f32 y, f32 x);
extern s32 overlay1AngleDifferenceReloc(s16 first, s16 second);
extern f32 overlay1TrigXReloc(s32 angle);
extern f32 overlay1TrigYReloc(s32 angle);

/* Plateau: exact 107 instructions and 0x30 frame, and now both spilled stack
 * homes. The declaration order is an identity, not a guess: the target spills
 * `current` to sp+40 and `next` to sp+32, which are the second and fourth
 * four-byte slots, so `previous, current, path, next` is the only order of the
 * four pointers that lands them (25 words to 21, all 24 orders measured). The
 * three `s32` index locals take registers, not homes, and permuting them is
 * inert -- all 6 orders crossed with 5 branch spellings and 2 assignment
 * orders, 60 forms, all flat.
 *
 * Twenty-one words remain in two clusters. Six are the prologue: the target
 * stores the third argument home out of order, puts the byte spill in the
 * call's delay slot, and reloads it into the argument register. Taking the
 * parameter's address (`*(u8 *)&index = index;`) is the only form that reaches
 * that byte -- the retained `volatile u8` local can never, because a local
 * lives in the local area -- and it does fix the first four prologue words,
 * but the byte store then lands before the call instead of in its delay slot
 * and the reload takes a ring temp instead of the argument register, costing
 * 34 words downstream (55 against 21). Measured and flat, do not repeat: the
 * address form combined with the volatile local in both orders and on both
 * sides of the call, a comma-operator store inside the call's argument list,
 * a `u8 *`/`volatile u8 *` carrier for the address, `((u8 *)&index)[0]`, a
 * read-back into a fresh `u8`/`s32` local or into `currentIndex`, and casting
 * the call's own argument.
 *
 * The other fifteen are the index block's pool colours: the target has
 * currentIndex on v1, previousIndex on a0 and the point count on a1, where we
 * get a1, v1 and a2. The emitted schedule is identical instruction for
 * instruction; only the web numbering differs. `currentIndex = index` in the
 * else branch reproduces the target's `move v1,a2` at both sites but IDO then
 * hoists the assignment out of the if/else and drops an instruction (101). */
#ifdef NON_MATCHING
void overlay1BendPathPoint(s16 *x, s16 *y, u8 index, u8 selector) {
    Overlay1PathPoint *previous;
    Overlay1PathPoint *current;
    Overlay1Path *path;
    Overlay1PathPoint *next;
    s16 firstAngle, secondAngle, midpointAngle;
    volatile u8 localIndex;
    s32 nextIndex, previousIndex, currentIndex;

    localIndex = index;
    path = overlay1GetPathReloc(selector);
    index = localIndex;
    current = &path->points[index];
    if (index != 0) {
        currentIndex = index;
        previousIndex = index - 1;
    } else {
        previousIndex = path->count;
        previousIndex--;
        currentIndex = 0;
    }
    previous = &path->points[previousIndex];
    if (currentIndex >= path->count) {
        nextIndex = 0;
    } else {
        nextIndex = currentIndex + 1;
    }
    next = &path->points[nextIndex];
    firstAngle = (s16)(overlay1AngleReloc((f32)(current->y - previous->y),
                                         (f32)(current->x - previous->x)) -
                       0x8000);
    secondAngle = (s16)(overlay1AngleReloc((f32)(current->y - next->y),
                                          (f32)(current->x - next->x)) -
                        0x8000);
    midpointAngle = firstAngle +
        (overlay1AngleDifferenceReloc(firstAngle, secondAngle) >> 1);
    *x = (s16)((f32)*x - overlay1TrigXReloc(midpointAngle) * 50.0f);
    *y = (s16)((f32)*y - overlay1TrigYReloc(midpointAngle) * 50.0f);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F0007730_1853B10.s")
#endif

/* ---- overlay1AdvancePath ---- */


typedef struct Overlay1Point {
    s16 x;
    s16 y;
} Overlay1Point;

typedef struct Overlay1PathEntry {
    Overlay1Point *points;
} Overlay1PathEntry;

typedef struct Overlay1TraceResult {
    s16 x;
    s16 y;
    u16 base;
    u16 first;
    u16 second;
    u16 secondary;
    s32 changed;
} Overlay1TraceResult;

extern s32 overlay2TracePath(f32 x, f32 y, f32 anchorX, f32 anchorY,
                             void *arg5, Overlay1TraceResult *result,
                             u8 primary, u8 secondary);
extern Overlay1PathEntry *overlay1GetEntry(u16 index);
extern void *overlay1CloneRecord(u32 *source);
extern void overlay1AppendPathPoint(Overlay1PathState *state, s16 x, s16 y,
                                    u8 primary, u8 secondary);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;
extern s32 gOverlay1PoolExhausted;

/* DKR v77/v80, JFG, and the five-reference skeleton scan found no credible
 * donor for this bounded path advance. */
/* NON_MATCHING: array views remove the inherited redundant endpoint load
 * without changing the callee ABI. Size, frame and instruction sequence now
 * agree; register draws and comparison carriers remain nonexact. */
#ifdef NON_MATCHING
s32 overlay1AdvancePath(Overlay1PathState *state) {
    s16 currentX;
    s16 currentY;
    Overlay1PathEntry *entry;
    union {
        Overlay1TraceResult fields;
        s16 coordinates[8];
        u16 values[8];
        s32 words[4];
    } result;
    Overlay1PathState *child;
    u8 count;

    count = state->count;
    currentX = state->x[count];
    currentY = state->y[count];

    state->flags = (state->flags & ~3) |
                   (*(u16 *)&state->count & 1);
    if (count >= 31) {
        return 1;
    }

    if (!overlay2TracePath((f32)currentX, (f32)currentY,
                           (f32)overlay1AnchorX, (f32)overlay1AnchorY,
                           (void *)gOverlay1SubmitArg5, &result.fields,
                           state->primary[count], state->secondary[count]) ||
        ((result.coordinates[0] == overlay1AnchorX) && (result.coordinates[1] == overlay1AnchorY))) {
        overlay1AppendPathPoint(state, overlay1AnchorX, overlay1AnchorY, 0xFF, 0);
        return 1;
    }

    entry = overlay1GetEntry(result.values[5]);
    if ((currentX != result.coordinates[0]) || (currentY != result.coordinates[1])) {
        overlay1AppendPathPoint(state, result.coordinates[0], result.coordinates[1],
                                *((u8 *)&result + 5), result.values[5]);
        if (result.words[3] != 0) {
            state->flags = (state->flags & ~3) |
                           ((*(u16 *)&state->count | 2) & 3);
        }
    }

    if ((result.values[3] != result.values[2]) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord((u32 *)state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.values[3]].x,
                                    entry->points[result.values[3]].y,
                                    *((u8 *)&result + 7), result.values[5]);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }

    if ((result.values[4] != result.values[2]) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord((u32 *)state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.values[4]].x,
                                    entry->points[result.values[4]].y,
                                    *((u8 *)&result + 9), result.values[5]);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }
    return 1;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_tail/func_overlay_001_F00078DC_1853CBC.s")
#endif

/* ---- overlay1FindBestRecord ---- */


/* Mickey-only reconstruction; pinned DKR v77/v80 and JFG scans have no exact
 * byte donor. Exact C: all 30 instruction words, frameless, and both D_1D88
 * relocation sites. The countdown and the group cache share one physical
 * source line: uopt numbers the pool webs by first surviving definition in
 * source statement order, so `remaining` must be written first to take a2 and
 * leave a3 for `group`, while ugen schedules the pair by line, so joining the
 * two statements emits the group load ahead of the countdown li. Splitting
 * them costs the two-instruction swap; reversing them costs the two colours.
 * The dead `value = 0` is load-bearing: it reserves the a1 pool colour that a
 * genuinely absent store would not, and removing it costs fifteen words. */
Overlay1PoolRecord *overlay1FindBestRecord(void) {
    Overlay1PoolRecord *record;
    Overlay1PoolRecord *result;
    u32 bestValue;
    u32 value;
    s32 remaining;
    s32 group;
    record = D_220;
    bestValue = (u32)-1;
    result = NULL;
    value = 0;
    remaining = 31; group = D_1D88;
    do {
        if (record->flags.bits.group == group) {
            value = record->value;
            if ((value == 0) ||
                (((record->flags.value & 3) == 3) &&
                 (value < bestValue))) {
                bestValue = value;
                result = record;
            }
        }
        record++;
    } while (remaining--);
    return result;
}



/* PLATEAU-HANDOFF:overlay1ConsumeNearbyPending:start
 * symbol: overlay1ConsumeNearbyPending
 * score: 31/69 words
 * frame: 0x18
 * relocations: 9
 * first-mismatch: +0x40
 * summary: 23-draw census retains counter read-modify-write web blocker; no admissible route below 31.
 * PLATEAU-HANDOFF:overlay1ConsumeNearbyPending:end
 */

/* PLATEAU-HANDOFF:overlay1UpdateRangeFlags:start
 * symbol: overlay1UpdateRangeFlags
 * score: 2/120 words
 * frame: 0x70
 * relocations: 4
 * first-mismatch: +0x190
 * summary: L145 on dx/dz keeps the t5/t4 pair. Inlining one square is 4 (same 33 draws). Floor remains 2.
 * PLATEAU-HANDOFF:overlay1UpdateRangeFlags:end
 */

/* PLATEAU-HANDOFF:overlay1AdvancePath:start
 * symbol: overlay1AdvancePath
 * score: 81/162 words
 * frame: 0x58
 * relocations: 22
 * first-mismatch: +0x10
 * summary: Proc-36 58-draw census confirms initial draw deficit and mixed later allocation residual; baseline retained.
 * PLATEAU-HANDOFF:overlay1AdvancePath:end
 */

/* PLATEAU-HANDOFF:overlay1TransitionState:start
 * symbol: overlay1TransitionState
 * score: 160 differing words
 * frame: 0x50
 * relocations: 13
 * first-mismatch: +0x20
 * summary: The size-near pointer-update probe is byte-flat; promotion trial in=0/out=0 is a schedule-divergence build error, not equality. Retain the 160-word structural plateau.
 * PLATEAU-HANDOFF:overlay1TransitionState:end
 */

/* PLATEAU-HANDOFF:func_overlay_001_F000438C_185076C:start
 * symbol: func_overlay_001_F000438C_185076C
 * score: 1196/1542 words
 * frame: 0x138
 * relocations: 184
 * first-mismatch: +0x24
 * summary: Size and frame are exact; web955 c7 reaches 1143 diagnostically only. Remaining blocker is FP/local-home shape plus unauthenticated global bindings.
 * PLATEAU-HANDOFF:func_overlay_001_F000438C_185076C:end
 */


/* PLATEAU-HANDOFF:func_overlay_001_F0003750_184FB30:start
 * symbol: func_overlay_001_F0003750_184FB30
 * score: 105/446 words
 * frame: 0x90
 * relocations: 31
 * first-mismatch: +0xC
 * summary: Direct selector loads remove the redundant path web and improve 360 to 341 masked differences without changing the 447/446 size. Linked trial has zero measured diffs but remains 12 bytes long.
 * PLATEAU-HANDOFF:func_overlay_001_F0003750_184FB30:end
 */

/* PLATEAU-HANDOFF:overlay1BendPathPoint:start
 * symbol: overlay1BendPathPoint
 * score: 21/107 words
 * frame: 0x30
 * relocations: 6
 * first-mismatch: +0xC
 * summary: 46-draw census confirms parameter-home/current-index live-range blocker; exhausted source and colour routes remain at 21.
 * PLATEAU-HANDOFF:overlay1BendPathPoint:end
 */
