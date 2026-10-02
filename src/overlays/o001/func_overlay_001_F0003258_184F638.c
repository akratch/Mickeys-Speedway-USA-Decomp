#include "overlays/overlay_001.h"

/* Matched 2026-10-02 (lane x-o058), 182 masked words at size -48 -> 0.
 *
 * Written from the listing in the shape of its matched sibling
 * func_overlay_001_F0002B4C_184EF2C (the overlay's `while (i--)` loops, the
 * resident rank order and weight tables, the per-object state).  What the
 * inherited candidate lacked, in order of what each was worth:
 *  - the order search is one `for` whose test carries the match
 *    (`j < count && state->index != gO1RankOrder[j]`); a do/while with the
 *    test split across an if left the loop strength-reduced and 16 bytes short
 *    (-16 -> 0, 177 -> 85);
 *  - the placing loop reuses `j`, so its index is a web of its own and not
 *    the sort's `i` (85 -> 36);
 *  - the time ratio is its own local (`ratio`, f2), and the dispatch call and
 *    the `initialized = 1` store are one comma statement (36 -> 25);
 *  - the score and state arrays have six entries (one per racer, like the race
 *    state's), with the locals declared around them in the target's home
 *    order; that lands every home and the 0xD8 frame (25 -> 0).
 * The -666.0f sentinel is a literal in this TU's pool; the retained overlay
 * data already holds it at rodata-relative +0xE4.  Neither inherited flag
 * override (-Wab,-r4300_mul, -O2 -Wo,-loopunroll,0) was load-bearing: the
 * object is byte-identical without them. */

typedef struct O1RankState {
    s8 index;
    s8 id;
    u8 pad002[0x1A6];
    u16 flags;
    u8 pad1AA[0x1D9];
    s8 lap;
    u8 pad384;
    u8 rank;
    u8 pad386[0x16];
    f32 offset;
    f32 spacing;
    u8 pad3A4[0x16];
    s16 position;
    u8 pad3BC[0x44];
    s32 total;
} O1RankState;

typedef struct O1RankObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x4C];
    O1RankState *state;
} O1RankObject;

/* Tier D: the one field read here. */
typedef struct O1Level {
    u8 pad00[0x86];
    s8 laps;
} O1Level;

typedef struct O1RaceEntry {
    u8 character;
    u8 variant;
    u8 position;
    u8 gap;
    s32 value;
    u8 pad08[0x20];
} O1RaceEntry;

typedef struct O1RaceState {
    u8 mode;
    u8 active;
    u8 player;
    u8 countdown;
    O1RaceEntry entries[6];
} O1RaceState;

extern s32 D_1D58;
extern s32 D_1D8C;
extern u8 gO1RankOrder[];
extern f32 gO1RankWeights[];

extern O1RankObject **func_80005750(s32 *count);
extern O1Level *levelGetLevel(void);
extern O1RaceState *func_80028F54(void);
extern void overlay7DispatchSelection(void *owner, s32 selection);

void func_overlay_001_F0003258_184F638(void) {
    O1RankObject **objects;
    O1RankObject *object;
    s32 count;
    s32 j;
    s32 initialized;
    s32 i;
    s32 changed;
    O1RankState *states[6];
    O1RankState *state;
    O1Level *level;
    O1RaceState *race;
    f32 distance;
    f32 ratio;
    s32 scores[6];
    s32 temp;

    initialized = 0;
    objects = func_80005750(&count);
    level = levelGetLevel();
    race = func_80028F54();
    if (count == 0) {
        return;
    }
    if (D_1D58 == 0) {
        return;
    }
    i = count;
    while (i--) {
        object = objects[i];
        state = object->state;
        if (state->lap < level->laps) {
            for (j = 0; j < count && state->index != gO1RankOrder[j]; j++) {
            }
            if (!initialized) {
                overlay7DispatchSelection(object, 0x11), initialized = 1;
            }
            if (state->index == gO1RankOrder[j]) {
                distance = gO1RankWeights[j];
                if (distance == -666.0f) {
                    distance = (f32)(state->lap * D_1D8C) + state->offset;
                }
                ratio = distance / (f32)(level->laps * D_1D8C);
                state->total += (s32)((f32)state->total * (1.0f - ratio));
            }
            race->entries[state->index].value = state->total;
        }
        scores[state->index] = race->entries[state->index].value;
        states[state->index] = state;
    }
    do {
        changed = 0;
        i = count - 1;
        while (i--) {
            if (scores[i] > scores[i + 1]) {
                temp = scores[i];
                scores[i] = scores[i + 1];
                scores[i + 1] = temp;
                changed = 1;
                state = states[i];
                states[i] = states[i + 1];
                states[i + 1] = state;
            }
        }
    } while (changed);
    for (j = 0; j < count; j++) {
        state = states[j];
        race->entries[state->index].position = j;
        state->position = j;
    }
}
