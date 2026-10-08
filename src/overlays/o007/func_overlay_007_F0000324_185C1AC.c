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

/* Matched 2026-10-01 by rewriting the shape rather than the allocation:
 * - the per-object decay factor is the float literal 0.995f from this TU's
 *   own .rodata (a LOCAL relocation, not a resident global), so uopt hoists
 *   the load itself and no local carries it;
 * - one `u16 *cursor` walks both the timer table and the D_844 pairs, which
 *   keeps the declaration block at eleven cells (frame 0x78) and emits the
 *   table address before the countdown initialiser;
 * - the first object loop is the plain `remaining = count; while
 *   (remaining--)`.
 * The create-owner sixth argument is `&overlay7RuntimeHandleReloc`. */
void func_overlay_007_F0000324_185C1AC(s32 unused0, s32 elapsed) {
    s32 count;
    u16 *cursor;
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
    cursor = &D_2AA;
    remaining = 9;
    do {
        if (*cursor != 0) {
            if (*cursor >= elapsed) {
                *cursor -= elapsed;
            } else {
                *cursor = 0;
            }
        }
        cursor--;
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
            if (overlay7RuntimeModeReloc == 1) {
                remaining = count;

                while (remaining--) {
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

                }
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

            do {
                object = objects[remaining];
                state = object->state;
                if (state->cooldown != 0) {
                    state->cooldown--;
                }
                state->scale *= 0.995f;

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
                cursor = (u16 *)D_844;
                index = 0x2B;
                do {
                    if (value == cursor[0]) {
                        value += overlay7AdjustRuntimeValueReloc(
                            0, cursor[1]);
                        break;
                    }
                    cursor += 2;
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
