#include "overlays/overlay_007.h"

typedef struct Overlay7RuntimeState {
    u8 pad000[1];
    s8 kind;
    u8 pad002[0x1A6];
    u16 flags;
    u8 pad1AA[0x1DB];
    u8 level;
    u8 previousLevel;
    u8 pad387[0x2C];
    u8 cooldown;
    u8 pad3B4[0x10];
    f32 difference;
    f32 scale;
} Overlay7RuntimeState;

typedef struct Overlay7RuntimeObject {
    u8 pad00C[0xC];
    s32 x;
    s32 y;
    s32 z;
    u8 pad018[0x4C];
    Overlay7RuntimeState *state;
} Overlay7RuntimeObject;

extern s32 D_4;
extern Overlay7Entry *D_290;
extern u16 D_2AA;
extern Overlay7Pair D_844[];
extern void *overlay7RuntimeGateReloc;
extern void *overlay7RuntimeHandleReloc;
extern s32 overlay7RuntimeTimerReloc;
extern Overlay7RuntimeObject *overlay7RuntimeLastObjectReloc;
extern s32 overlay7RuntimeModeReloc;
extern u8 overlay7RuntimeLevelReloc;
extern u8 overlay7RuntimePreviousLevelReloc;
extern f32 overlay7RuntimeScaleReloc;
extern u8 overlay7RuntimeValuesReloc[];

extern Overlay7RuntimeObject **overlay7CollectRuntimeObjectsReloc(s32 *count);
extern void overlay7ContinueRuntimeObjectReloc(void *handle, s32 x, s32 y,
                                               s32 z);
extern f32 overlay7MeasureRuntimeObjectAReloc(Overlay7RuntimeObject *object);
extern f32 overlay7MeasureRuntimeObjectBReloc(Overlay7RuntimeObject *object);
extern void overlay7RecordRuntimeEventReloc(s32 event);
extern s32 overlay7GetRuntimeIndexReloc(s32 arg0, s32 arg1);
extern void overlay7StartRuntimeValueReloc(u8 value);
extern void overlay7ReleaseRuntimeOwnerReloc(void *owner);
extern s32 overlay7AdjustRuntimeValueReloc(s32 arg0, s32 value);
extern void *overlay7CreateRuntimeOwnerReloc(u16 value, s32 x, s32 y, s32 z,
                                             s32 kind, void **argument);
extern void overlay7ReleaseRuntimeHandleReloc(void *handle);
extern void overlay7ReleaseRuntimeEntryReloc(Overlay7Entry *entry);
extern s32 overlay7RuntimeChanceReloc(s32 minimum, s32 maximum);
extern void overlay7SetRuntimeModeReloc(Overlay7RuntimeObject *object,
                                        s32 mode);

/* Frame closed 2026-09-11. The target's frame is 0x78 and its declaration
 * block is 44 bytes, which is eleven four-byte locals; this candidate carried
 * fourteen. Three came out: `handleObject` (the target reads the global's
 * fields directly into one temp), `cursor` (strength reduction makes the
 * walking pointer out of objects[remaining]), and the nested `scale`, whose
 * hoisted loop invariant reuses the `difference` carrier -- their live ranges
 * are disjoint. Declaration order then places `objects` 6th, `object` 7th and
 * `difference` 11th, which is the target's home ladder exactly; the other
 * eight positions are byte-inert (four permutations measured identical).
 *
 * Size closed 2026-09-18: the D_844 scan is `do { D_844[0x2B - index] }
 * while (index--)`, which keeps the countdown live across the adjust call,
 * and the create-owner sixth argument is `&overlay7RuntimeHandleReloc`
 * (LOCAL data+0), not NULL. Nested scale/pair/block-scoped difference all
 * reopen the 0x80 frame. */
#ifdef NON_MATCHING
void func_overlay_007_F0000324_185C1AC(s32 arg0, s32 elapsed) {
    s32 count;
    u16 *timer;
    s32 remaining;
    Overlay7Entry *entry;
    Overlay7RuntimeState *state;
    Overlay7RuntimeObject **objects;
    Overlay7RuntimeObject *object;
    s32 index;
    s32 found;
    u16 value;
    f32 difference;

    objects = overlay7CollectRuntimeObjectsReloc(&count);
    timer = &D_2AA;
    remaining = 9;
    do {
        if (*timer != 0) {
            if (*timer >= elapsed) {
                *timer -= elapsed;
            } else {
                *timer = 0;
            }
        }
        timer--;
    } while (remaining--);

    if (overlay7RuntimeGateReloc == NULL) {
        entry = gOverlay7ActiveHead;
        while (entry != NULL) {
            if (entry->active != 0) {
                entry->field04 += elapsed;
            }
            entry = entry->next;
        }

        if (overlay7RuntimeHandleReloc != NULL) {

            overlay7ContinueRuntimeObjectReloc(overlay7RuntimeHandleReloc,
                overlay7RuntimeLastObjectReloc->x, overlay7RuntimeLastObjectReloc->y,
                overlay7RuntimeLastObjectReloc->z);
        }

        if (elapsed < overlay7RuntimeTimerReloc) {
            overlay7RuntimeTimerReloc -= elapsed;
            remaining = count - 1;
        } else {
            if (overlay7RuntimeModeReloc == 1 && count != 0) {
                remaining = count - 1;

                do {
                    object = objects[remaining];
                    state = object->state;
                    if (!(state->flags & 1) && state->kind < 6 &&
                        state->cooldown == 0) {
                        difference = overlay7MeasureRuntimeObjectAReloc(object) -
                                     overlay7MeasureRuntimeObjectBReloc(object);
                        if (difference - state->difference > 4.0f) {
                            overlay7RecordRuntimeEventReloc(state->kind + 0x15);
                        }
                        state->difference = difference;
                    }

                } while (remaining--);
            }

            remaining = count - 1;
            if (overlay7RuntimeModeReloc == 1 &&
                ((s32)overlay7RuntimePreviousLevelReloc -
                     (s32)overlay7RuntimeLevelReloc ==
                 1) &&
                overlay7RuntimeLevelReloc > 0 && count != 0) {

                do {
                    object = objects[remaining];
                    state = object->state;
                    if (!(state->flags & 1) && state->level != 0 &&
                        state->scale + (f32)state->level > 9.0f) {
                        index = overlay7GetRuntimeIndexReloc(0, 4);
                        overlay7StartRuntimeValueReloc(
                            overlay7RuntimeValuesReloc[index + 0x924]);
                    }

                } while (remaining--);
                remaining = count - 1;
            }
            overlay7RuntimeTimerReloc = 0x12C;
        }

        if (count != 0) {
            difference = overlay7RuntimeScaleReloc;

            do {
                object = objects[remaining];
                state = object->state;
                if (state->cooldown != 0) {
                    state->cooldown--;
                }
                state->scale *= difference;

            } while (remaining--);
        }

        if (overlay7RuntimeModeReloc == 0 && D_4 == 0) {
            found = 0;
            if (D_290 != NULL) {
                if (D_290->nested != NULL) {
                    entry = D_290->nested;
                    entry->active = 1;
                    found = 1;
                }
                overlay7ReleaseRuntimeEntryReloc(D_290);
            }
            if (!found) {
                entry = gOverlay7ActiveHead;
                while (entry != NULL) {
                    if (entry->active == 1) {
                        found = 1;
                        break;
                    }
                    entry = entry->next;
                }
            }
            if (found) {
                value = entry->value;
                index = 0x2B;
                do {
                    if (value == D_844[0x2B - index].key) {
                        value += overlay7AdjustRuntimeValueReloc(
                            0, D_844[0x2B - index].value);
                        break;
                    }
                } while (index--);
                if ((u32)entry->field04 < 0xB4) {
                    overlay7CreateRuntimeOwnerReloc(
                        value, object->x, object->y, object->z, 4,
                        &overlay7RuntimeHandleReloc);
                    overlay7RuntimeLastObjectReloc = object;
                    D_290 = entry;
                } else {
                    if (entry->nested != NULL) {
                        overlay7ReleaseRuntimeHandleReloc(entry->nested);
                    }
                    overlay7ReleaseRuntimeEntryReloc(entry);
                }
            }
        }

        remaining = count - 1;
        if (count != 0) {

            do {
                object = objects[remaining];
                state = object->state;
                if (state->level < state->previousLevel) {
                    if (state->level == 0) {
                        overlay7SetRuntimeModeReloc(object, 0x10);
                    } else if (!(state->flags & 1) ||
                               overlay7RuntimeChanceReloc(1, 0x64) < 0x24) {
                        overlay7SetRuntimeModeReloc(object, 0xE);
                    }
                } else if (state->previousLevel < state->level &&
                           (!(state->flags & 1) ||
                            overlay7RuntimeChanceReloc(1, 0x64) < 0x24)) {
                    overlay7SetRuntimeModeReloc(object, 0xF);
                }
                state->previousLevel = state->level;

            } while (remaining--);
        }

        entry = gOverlay7ActiveHead;
        while (entry != NULL) {
            entry = entry->next;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/func_overlay_007_F0000324_185C1AC/func_overlay_007_F0000324_185C1AC.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_007_F0000324_185C1AC:start
 * symbol: func_overlay_007_F0000324_185C1AC
 * score: 10/348 words
 * frame: 0x78
 * relocations: 61
 * first-mismatch: +0x120
 * summary: Colour floor 10: web 99 denied f0 (L142); a distinct f0 name costs a twelfth home or size. as1 delays are besttime, not lineno.
 * PLATEAU-HANDOFF:func_overlay_007_F0000324_185C1AC:end
 */
