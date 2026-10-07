#include "overlays/overlay_001.h"

typedef struct Overlay1MotionState {
    s8 index;
    u8 pad001[0x1A7];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 path;
    u8 selector;
    u8 selectorCopy;
    u8 pad380[3];
    s8 modeIndex;
    s8 maxIndex;
    u8 eventMode;
    u8 pad386;
    u8 timer;
    u8 timerState;
    u8 pad389[0x0F];
    f32 weight;
    f32 offset;
    u8 pad3A0[0x12];
    s8 mode3b2;
    u8 pad3B3[7];
    s16 lastMode;
    u8 pad3BC[0x44];
    s32 total;
    s32 split[22];
    u8 changed;
    u8 nextMode;
} Overlay1MotionState;

typedef struct Overlay1Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay1Level;

typedef struct Overlay1Owner {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay1Owner;

typedef struct Overlay1PathRecord {
    u8 pad00[0xE];
    u16 flags;
    u8 pad10[0x84];
} Overlay1PathRecord;

typedef struct Overlay1Result {
    u8 mode;
    u8 pad01[7];
    s32 total;
    s32 split[7];
} Overlay1Result;

typedef struct Overlay1BestTime {
    s32 time;
    s32 pad04;
} Overlay1BestTime;

typedef struct Overlay1LevelRecord {
    Overlay1BestTime best[3];
    s32 lap;
    s32 pad1C;
} Overlay1LevelRecord;

typedef struct Overlay1ObjectRef {
    u8 pad00[0x64];
    Overlay1MotionState *state;
} Overlay1ObjectRef;

extern Overlay1Result *func_80028F54(void);
extern Overlay1ObjectRef **func_80005750(s32 *count);
extern void amSndPlay(u16 value, void **argument);
extern Overlay1LevelRecord *func_800291C4(void);
extern s32 levelGetNumber(void);
extern s32 levelGetBlurEffect(s32 level);
extern void func_8003A55C(s32 value);
extern void overlay7UpdateOwnerMode(void *owner, s32 previous);
extern void overlay68StartTimer(void);
extern void overlay7CommitSelection(s32 selection);
extern s32 overlay1FindClosestSample(f32 x, f32 y, void *source, f32 weight);
extern s32 overlay1TestDirection(void *direction, f32 x, f32 z);
extern u8 *overlay1PreviousPointer(u8 *pointer);
extern u8 *overlay1NextPointer(u8 *pointer);
extern void overlay1InitMotionScale(void);

extern s32 G_o1_83e0;
extern s32 D_1D8C;
extern s32 D_1D94;
extern s8 D_1DC0[];
extern u8 D_1DC8[];
extern u8 D_1DD0[];
extern Overlay1PathRecord *D_1D60;
extern Overlay1PathRecord *D_1D68;
extern Overlay1PathRecord *D_1D6C;
extern void *D_0208;
extern void *D_020C;
extern void *D_0210;
extern void *D_0214;
extern s32 gO1Finishers;
extern s32 gO1RaceOver;
extern u8 gO1FinishEnabled;
extern u8 gO1PlayerCount;
extern u8 gO1RankOrder[];
extern f32 gO1RankWeights[];

#define STATE ((Overlay1MotionState *)D_1DA0)
/* The same pointer read as a word: a separate IR name, so its webs are
 * coloured apart from the lap-logic reads (a1 there, as shipped). */
#define STATE_W ((Overlay1MotionState *)*(s32 *)&D_1DA0)
#define OWNER ((Overlay1Owner *)D_1D9C)
#define PREV ((Overlay1PathRecord *)D_1D64)

/* 2026-10-02 p-ovl8: rewritten from the target listing in the overlay's
 * idiom after decoding the module's runtime relocation records per call
 * site: the callees are func_80028F54 (results), func_80005750 (object
 * list), overlay1FindClosestSample, overlay1TestDirection, the ring helpers
 * overlay1NextPointer/overlay1PreviousPointer, amSndPlay, func_800291C4,
 * levelGetNumber, levelGetBlurEffect, func_8003A55C, overlay 7's owner-mode
 * update (+0xAA0) and commit (+0xDBC), overlay68StartTimer and
 * overlay1InitMotionScale; the current state is read through D_1DA0 at each
 * use; the D_D8 "global" is the literal -666.0f. 584 differing words at
 * -112 to 268 masked at size 0, frame 0x70 exact (two leading pads and the
 * +0x5C/+0x50 pads place the homes). Declaring entry and j before table and
 * valid moves those two homes down eight bytes and scores 263 masked words
 * at the same size. 2026-10-07 a-ovl4: reading the state as a word
 * (STATE_W) before the first direction test and from the second owner-mode
 * update on gives those reads their own webs, which take a1 as shipped
 * (263 -> 219 at size 0); the lap-logic stretch between still shares one
 * web that colours a2. 2026-10-07 c-ovla: the list loop holds each
 * object in its own local `ref` (a declared local is never a ring temp, so
 * it takes a0 and the counter and cursor move to a1/a2 as shipped), the
 * player-count compare is written count first, the lap compares state
 * first, and the position sum offset first: 219 -> 131 at size 0. */
#ifdef NON_MATCHING
void func_overlay_001_F0001D78_184E158(s32 index, Overlay1Level *level, s32 count) {
    Overlay1ObjectRef *ref;
    s32 padB;
    s32 switched;
    s32 objectCount;
    s32 pad5C;
    Overlay1Result *results;
    f32 position;
    s32 pad50;
    Overlay1ObjectRef **list;
    s32 entry;
    s32 j;
    Overlay1LevelRecord *table;
    s32 valid;
    s32 i;
    Overlay1PathRecord *record;

    results = func_80028F54();
    list = func_80005750(&objectCount);
    if (!(STATE_W->flags & 1)) {
        STATE_W->selector = overlay1FindClosestSample(OWNER->x, OWNER->z, D_1D64, STATE_W->weight);
        STATE_W->selectorCopy = STATE_W->selector;
    }
    if (!(STATE_W->flags & 8)) {
        position = STATE_W->offset + (f32)(STATE_W->modeIndex * D_1D8C);
    }
    switched = 0;
    if (!(STATE_W->flags & 8)) {
        if (overlay1TestDirection(D_1D68, OWNER->x, OWNER->z) != 0) {
            switched = 1;
            record = (Overlay1PathRecord *)overlay1NextPointer((u8 *)D_1D6C);
            D_1D60 = PREV;
            D_1D64 = D_1D68;
            D_1D68 = D_1D6C;
            D_1D6C = record;
            STATE->path = D_1D68 - (Overlay1PathRecord *)gOverlay1Start.records;
            if (PREV->flags == 0) {
                if (STATE->changed) {
                    STATE->changed = 0;
                    STATE->modeIndex++;
                } else if (STATE->modeIndex < level->laps) {
                    STATE->modeIndex++;
                    D_1DC8[STATE->modeIndex]++;
                    STATE->nextMode = D_1DD0[STATE->modeIndex];
                    if (STATE->modeIndex == level->laps) {
                        STATE->split[2] = (STATE->total / 3 - STATE->split[1] / 3 - STATE->split[0] / 3) * 3;
                    }
                    if (STATE->modeIndex < 0) {
                        STATE->modeIndex = 0;
                    }
                    if (!(STATE->flags & 1) && STATE->modeIndex > 0 && STATE->modeIndex < 3) {
                        amSndPlay(0x1FC, NULL);
                    }
                    if (STATE->modeIndex > 0) {
                        D_1DC0[STATE->index] = 1;
                    }
                    if (STATE->maxIndex < STATE->modeIndex) {
                        STATE->maxIndex = STATE->modeIndex;
                    }
                    if (STATE->modeIndex == level->laps && G_o1_83e0 == 1 && !(STATE->flags & 1)) {
                        if (results->mode == 1) {
                            valid = 0;
                            table = func_800291C4();
                            entry = levelGetBlurEffect(levelGetNumber());
                            for (j = 0; j < STATE->modeIndex; j++) {
                                if (STATE->split[j] < table[entry].lap || table[entry].lap == 0) {
                                    valid = 1;
                                }
                            }
                            for (j = 0; j < 3; j++) {
                                if (STATE->total < table[entry].best[j].time || table[entry].best[j].time == 0) {
                                    valid = 1;
                                }
                            }
                            if (valid) {
                                func_8003A55C(5);
                            } else {
                                func_8003A55C(6);
                            }
                        } else if (STATE->eventMode == 0) {
                            func_8003A55C(5);
                        } else if (STATE->eventMode < 4) {
                            func_8003A55C(0x1B);
                        } else {
                            func_8003A55C(6);
                        }
                    }
                    if (STATE->modeIndex > 0) {
                        if (STATE->modeIndex + 1 == level->laps) {
                            overlay7UpdateOwnerMode(D_1D9C, level->laps);
                        } else if (STATE->modeIndex == level->laps) {
                            overlay7UpdateOwnerMode(D_1D9C, level->laps);
                            if (!(STATE_W->flags & 1)) {
                                G_o1_83e0--;
                                STATE_W->flags |= 0x11;
                            }
                            gO1Finishers++;
                            if (gO1FinishEnabled != 0 && G_o1_83e0 == 1 && gO1PlayerCount == gO1Finishers + 1) {
                                i = objectCount;
                                while (i--) {
                                    ref = list[i];
                                    ref->state->flags |= 1;
                                }
                                G_o1_83e0 = 0;
                                func_8003A55C(0x1B);
                            }
                            STATE_W->lastMode = STATE_W->nextMode;
                            results[STATE_W->index].total = STATE_W->total;
                            for (j = 0; j < STATE_W->modeIndex; j++) {
                                results[STATE_W->index].split[j] = STATE_W->split[j];
                            }
                            if (results->mode == 1) {
                                overlay68StartTimer();
                            }
                        } else {
                            overlay7UpdateOwnerMode(D_1D9C, level->laps);
                        }
                        if (gO1Finishers >= count - 1) {
                            gO1RaceOver = 1;
                        }
                    }
                }
            }
            STATE_W->mode3b2 = PREV->flags;
        } else if (overlay1TestDirection(D_1D64, OWNER->x, OWNER->z) == 0) {
            switched = 1;
            record = (Overlay1PathRecord *)overlay1PreviousPointer((u8 *)D_1D60);
            D_1D6C = D_1D68;
            D_1D68 = PREV;
            D_1D64 = D_1D60;
            D_1D60 = record;
            STATE_W->path = D_1D68 - (Overlay1PathRecord *)gOverlay1Start.records;
            if (D_1D68->flags == 0 && STATE_W->lastMode == 0xFF && STATE_W->changed == 0) {
                STATE_W->changed = 1;
                if (STATE_W->modeIndex != -1) {
                    STATE_W->modeIndex--;
                }
            }
            STATE_W->mode3b2 = D_1D68->flags;
        }
        if (switched) {
            D_0208 = (u8 *)D_1D60 + STATE_W->selector * 0x10 + 0x14;
            D_020C = (u8 *)D_1D64 + STATE_W->selector * 0x10 + 0x14;
            D_0210 = (u8 *)D_1D68 + STATE_W->selector * 0x10 + 0x14;
            D_0214 = (u8 *)D_1D6C + STATE_W->selector * 0x10 + 0x14;
        }
        overlay1InitMotionScale();
    }
    if (!(STATE_W->flags & 8)) {
        gO1RankWeights[index] = (f32)(STATE_W->modeIndex * D_1D8C) + STATE_W->offset;
    } else {
        gO1RankWeights[index] = -666.0f;
    }
    gO1RankOrder[index] = STATE_W->index;
    if (!(STATE_W->flags & 8) && gO1RankWeights[index] < position) {
        if (STATE_W->timer >= D_1D94) {
            STATE_W->timer -= D_1D94;
            return;
        }
        if (!(STATE_W->flags & 1)) {
            overlay7CommitSelection(8);
        }
        STATE_W->timerState = 1;
        STATE_W->timer = 0xF0;
        return;
    }
    STATE_W->timer = 0xF0;
    STATE_W->timerState = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/func_overlay_001_F0001D78_184E158/func_overlay_001_F0001D78_184E158.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_001_F0001D78_184E158:start
 * symbol: func_overlay_001_F0001D78_184E158
 * score: 131/627 words
 * frame: 0x70
 * relocations: 185
 * first-mismatch: +0x174
 * summary: List-loop element local, count-first and state-first compares, offset-first sum: 219 to 131 at delta 0; open: lap-logic state web a2 (target a1).
 * PLATEAU-HANDOFF:func_overlay_001_F0001D78_184E158:end
 */
