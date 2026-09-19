#include "overlays/overlay_001.h"

/* ---- overlay1FindNextAngle ---- */


typedef struct Overlay1AngleData {
    u8 pad00[0x39C];
    f32 angle;
} Overlay1AngleData;

typedef struct Overlay1AngleObject {
    u8 pad00[0x64];
    Overlay1AngleData *data;
} Overlay1AngleObject;

extern Overlay1AngleObject **overlay1GetAngleObjectsReloc(s32 *count);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1NextAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
/* Matched 2026-09-09. Two mechanisms, each a regression on its own:
 * (1) lever 51 -- the hand-written `cursor` walk is what blocked the match.
 *     Writing the scan as `objects[remaining]` lets uopt build its own
 *     strength-reduced induction variable, and that also stops ugen spending a
 *     ring temp on `count - 1`: with the cursor spelling it emitted
 *     `addu $3,$4,-1; move $17,$3`, with the indexed spelling `addu $17,$4,-1`.
 * (2) with the copy gone, hoisting `remaining = count - 1` above the guard puts
 *     that single instruction immediately before the `beq`, which is what as1
 *     needs to schedule it into the branch delay slot. Hoisting alone (with the
 *     copy still there) costs 34 words, because the copy goes into the slot
 *     instead and cannot then be coalesced across the block boundary.
 * The named `data` carrier is lever 45: `object->data->angle` spends a ugen
 * ring temp (t7) where the target holds the pointer in a pool colour (v0). */
Overlay1AngleObject *overlay1FindNextAngle(f32 angle) {
    s32 count;
    Overlay1AngleObject **objects;
    Overlay1AngleData *data;
    Overlay1AngleObject *object;
    Overlay1AngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;
    s32 loopValue;

    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1NextAngleLimit;
    best = (Overlay1AngleObject *)(count - count);
    remaining = count - 1;
    if (count != 0) {
        do {
            object = objects[remaining];
            data = object->data;
            difference = overlay1WrapOffset(angle, data->angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            loopValue = remaining--;
        } while (loopValue);
    }
    return best;
}

/* ---- overlay1FindPreviousAngle ---- */


typedef Overlay1AngleData Overlay1PreviousAngleData;
typedef Overlay1AngleObject Overlay1PreviousAngleObject;
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1PreviousAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
/* Matched 2026-09-09 by the same pair of edits as overlay1FindNextAngle above:
 * indexed access in place of the hand-written cursor, `remaining = count - 1`
 * hoisted above the guard so as1 can fill the branch delay slot with it, and a
 * named `data` carrier for the pool colour. Only the argument order of the
 * wrap-offset call and the limit global differ between the two. */
Overlay1PreviousAngleObject *overlay1FindPreviousAngle(f32 angle) {
    s32 count;
    Overlay1PreviousAngleObject **objects;
    Overlay1PreviousAngleData *data;
    Overlay1PreviousAngleObject *object;
    Overlay1PreviousAngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;
    s32 loopValue;

    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1PreviousAngleLimit;
    best = (Overlay1PreviousAngleObject *)(count - count);
    remaining = count - 1;
    if (count != 0) {
        do {
            object = objects[remaining];
            data = object->data;
            difference = overlay1WrapOffset(data->angle, angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            loopValue = remaining--;
        } while (loopValue);
    }
    return best;
}

/* ---- overlay1RefreshMode ---- */


typedef struct Overlay1ModeObject {
    u8 pad00[0x38C];
    u8 mode;
} Overlay1ModeObject;

extern void *gOverlay1ModeSource;
extern Overlay1ModeObject *gOverlay1ModeObject;
extern s32 overlay1ReadModeReloc(void *source);

/* DKR v77/v80 and JFG have no exact donor for this mode refresh wrapper. */
void overlay1RefreshMode(s32 arg0, s32 arg1, s32 arg2) {
    if (overlay1ReadModeReloc(gOverlay1ModeSource) >= 3) {
        gOverlay1ModeObject->mode = 2;
    } else {
        gOverlay1ModeObject->mode = 1;
    }
    overlay1ReadModeReloc(gOverlay1ModeSource);
}

/* ---- overlay1CallGlobal ---- */


/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern void overlay1GlobalCallReloc();
extern void *gOverlay1SubmitArg4;

void overlay1CallGlobal(s32 unused0, s32 unused1, s32 unused2) {
    overlay1GlobalCallReloc(gOverlay1SubmitArg4);
}

/* ---- overlay1AdvanceObjectGauges ---- */


typedef struct O1GaugeState {
    u8 pad000[0x384]; s8 level; u8 pad385[0x7B]; s32 value; s32 levelValues[1];
} O1GaugeState;
typedef struct O1GaugeObject { u8 pad00[0x64]; O1GaugeState *state; } O1GaugeObject;
typedef struct O1GaugeOwner { u8 pad00[0x86]; s8 levelLimit; } O1GaugeOwner;
extern s32 D_0;
extern s32 overlay1GetGaugeObjectsRaw(s32 *count);
extern s32 overlay1GetGaugeLimit(O1GaugeObject *object);

void overlay1AdvanceObjectGauges(O1GaugeOwner *owner, s32 amount) {
    O1GaugeObject **objects;
    O1GaugeObject *object;
    O1GaugeState *state;
    s32 count;
    s32 index;
    s32 delta;
    s32 limit;
    s32 loopValue;

    objects = (O1GaugeObject **)overlay1GetGaugeObjectsRaw(&count);
    if (count != 0) {
        index = count - 1; objects += index; do { object = *objects;
            state = object->state;
            if ((D_0 == 0) && (state->level < owner->levelLimit)) {
                delta = amount * 5;
                state->value += delta;
                limit = overlay1GetGaugeLimit(object);
                if (limit < state->value) state->value = limit;
                state->levelValues[state->level] += delta;
                if (state->levelValues[state->level] >= 180001) {
                    state->levelValues[state->level] = 180000;
                }
            }
            loopValue = index;
            objects--;
            index--;
        } while (loopValue != 0);
    }
}

/* ---- overlay1AdvanceGauge ---- */


typedef struct O1AdvanceGaugeState { u8 pad000[0x3FA]; s16 disabled; u8 pad3FC[4]; s32 value; } O1AdvanceGaugeState;
typedef struct O1AdvanceGaugeObject { u8 pad00[0x64]; O1AdvanceGaugeState *state; } O1AdvanceGaugeObject;
extern s32 D_0;
extern O1AdvanceGaugeObject **overlay1GetGaugeObjects(s32 *count);

/* Matched by the same indexed scan as overlay1FindNextAngle above: a
 * hand-written walking cursor spent one extra pool colour, and every ring
 * value slid one position. `objects[index]` lets uopt build the cursor, and
 * hoisting `index = count - 1` above the guard fills the delay slot. The
 * unused volatile pad the walking shape needed for the stack home is gone. */
void overlay1AdvanceGauge(s32 amount) {
    O1AdvanceGaugeObject **objects;
    O1AdvanceGaugeObject *object;
    O1AdvanceGaugeState *state;
    s32 count;
    s32 index;
    s32 loopValue;

    objects = overlay1GetGaugeObjects(&count);
    index = count - 1;
    if (count != 0) {
        do {
            object = objects[index];
            state = object->state;
            if ((D_0 == 0) && (state->disabled == 0)) {
                state->value += amount * 5;
                if (state->value >= 540001) state->value = 540000;
            }
            loopValue = index--;
        } while (loopValue);
    }
}
