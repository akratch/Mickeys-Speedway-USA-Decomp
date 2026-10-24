#include "overlays/overlay_007.h"

/*
 * Overlay 7: the sound-entry pool and the mode dispatcher. One translation
 * unit; ReleaseEntry, CreateEntry and AppendEntry are unexported and static.
 */

/* Tier B: these private objects supply the complete 0x2B0-byte physical
 * BSS allocation. The existing external current/timer/countdown/last-object
 * views remain separate until their identity and pointer-view contracts are
 * reconciled; no additional relocation filtering is used here. */
static Overlay7Entry gOverlay7EntryStorage[32];
static Overlay7Entry *sOverlay7CurrentEntryStorage;
static Overlay7Entry *gOverlay7ActiveHead;
static Overlay7Entry *gOverlay7FreeHead;
static Overlay7Entry *gOverlay7ActiveTail;
static Overlay7Entry *gOverlay7Selected;
static s32 sOverlay7RuntimeTimerStorage;
static u16 sOverlay7CountdownStorage[10];
static Overlay7RuntimeObject *sOverlay7LastObjectStorage;

static void overlay7ReleaseEntry(Overlay7Entry *entry) {
    Overlay7Entry *previous;
    Overlay7Entry *current;

    if (gOverlay7ActiveHead != 0 && entry != 0) {
        entry->active = 0;
        if (entry == gOverlay7Selected) {
            gOverlay7Selected = 0;
        }

        previous = 0;
        current = gOverlay7ActiveHead;
        if (current != entry) {
            do {
                previous = current;
                current = current->next;
            } while (current != entry);
        }

        if (previous != 0) {
            previous->next = current->next;
        }
        if (current == gOverlay7ActiveTail) {
            gOverlay7ActiveTail = previous;
        }
        if (current == gOverlay7ActiveHead) {
            gOverlay7ActiveHead = current->next;
        }
        current->next = gOverlay7FreeHead;
        gOverlay7FreeHead = current;
    }
}

/* Matched 2026-09-12, lane p9-tight. Sixteen words to zero, two edits in the
 * tail, both named below at the point of use.
 *
 * 1. The `result = entry;` self-copy in the else arm. It is the whole of the
 *    ten register words: uopt propagates `result` into every use of `entry`
 *    after the copy, so the ActiveHead store, the `entry->next = 0` store and
 *    the final ActiveTail store all read $v0 where the ROM reads $s0.
 * 2. `return entry;` inside the guard with a bare `return 0;` after it,
 *    instead of `else { entry = 0; } return entry;`. That is what puts the
 *    `move $v0, $zero` in the branch-likely's delay slot and leaves the dead
 *    copy the ROM carries at the join; the shared-exit form sets $s0 there
 *    instead and needs two more instructions to get back to $v0.
 *
 * Neither edit works alone: the reshape on its own is 22 words (recorded by the
 * 2026-09-04 probe as 74/96) and the self-copy on its own is 6.
 */
Overlay7Entry *overlay7AcquireEntry(Overlay7Owner *owner, u16 value, u8 type) {
    Overlay7Entry *entry;
    Overlay7Entry *result;
    s8 *ownerPriority;

    entry = gOverlay7ActiveHead;
    if (entry != 0) {
        do {
            if (entry->active != 0 && entry->owner == owner &&
                entry->value == value) {
                return 0;
            }
            entry = entry->next;
        } while (entry != 0);
    }

    entry = gOverlay7ActiveHead;
    ownerPriority = owner->priority;
    if (entry != 0) {
        do {
            if (entry->active != 0 &&
                ((entry->type < type && entry->owner == owner) ||
                 (*ownerPriority < D_8007C1A0 &&
                  type >= entry->type))) {
                if (entry->nested != 0) {
                    entry->nested->active = 1;
                }
                overlay7ReleaseEntry(entry);
            }
            entry = entry->next;
        } while (entry != 0);
    }

    result = gOverlay7FreeHead;
    if (result != 0) {
        entry = result;
        gOverlay7FreeHead = result->next;
        if (gOverlay7ActiveTail != 0) {
            gOverlay7ActiveTail->next = result;
            gOverlay7ActiveTail = result;
        } else {
            gOverlay7ActiveHead = entry;
            /* Load-bearing, and it compiles to nothing. uopt copy-propagates
             * `result` into every use of `entry` after `entry = result`, which
             * is why the three tail stores read $v0 here and $s0 in the ROM.
             * Redefining `result` on ONE arm makes the two disagree at the
             * merge, so the propagation stops there and the stores below keep
             * `entry`; the assignment itself is a self-copy at this point and
             * uopt deletes it. */
            result = entry;
        }
        entry->next = 0;
        gOverlay7ActiveTail = entry;
        return entry;
    }
    return 0;
}

/* DKR v77/v80 and JFG exact-object scans are negative for this allocator. */
static void overlay7CreateEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;

    entry = overlay7Acquire(owner, value, type);
    if (entry == NULL) {
        gOverlay7Current = NULL;
    } else {
        gOverlay7Current = entry;
        entry->owner = owner;
        entry->field04 = 0;
        entry->value = value;
        entry->type = type;
        entry->nested = 0;
        entry->active = 1;
    }
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
static void overlay7AppendEntry(void *owner, u16 value, u8 type) {
    Overlay7Entry *entry;
    Overlay7Entry *current;

    entry = overlay7Acquire(owner, value, type);
    current = gOverlay7Current;
    if (current == 0) {
        overlay7CreateEntry(owner, value, type);
    } else {
        current->nested = entry;
        if (entry != 0) {
            entry->owner = owner;
            entry->field04 = 0;
            entry->value = value;
            entry->type = type;
            entry->nested = 0;
            entry->active = 2;
        }
    }
}

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
void func_overlay_007_F0000324_185C1AC(void *displayList, s32 elapsed) {
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

    objects = func_80005750(&count);
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

    if (D_800C947C == NULL) {
        entry = gOverlay7ActiveHead;
        while (entry != NULL) {
            if (entry->active != 0) {
                entry->field04 += elapsed;
            }
            entry = entry->next;
        }

        if (overlay7RuntimeHandleReloc != NULL) {

            amSndSetXYZ(overlay7RuntimeHandleReloc,
                overlay7RuntimeLastObjectReloc->x, overlay7RuntimeLastObjectReloc->y,
                overlay7RuntimeLastObjectReloc->z);
        }

        if (elapsed < overlay7RuntimeTimerReloc) {
            overlay7RuntimeTimerReloc -= elapsed;
            remaining = count - 1;
        } else {
            if (B_overlay_001_F00083E4 == 1) {
                remaining = count;

                while (remaining--) {
                    object = objects[remaining];
                    state = object->state;
                    if (!(state->flags & 1) && state->kind < 6 &&
                        state->cooldown == 0) {
                        difference = overlay1DistanceFromSelected(object) -
                                     overlay1DistanceFromCurrent(object);
                        if (difference - state->difference > 4.0f) {
                            overlay7CommitSelection_o007Reloc(state->kind + 0x15);
                        }
                        state->difference = difference;
                    }

                }
            }

            remaining = count - 1;
            if (B_overlay_001_F00083E4 == 1 &&
                ((s32)D_8007BEF8 -
                     (s32)D_8007BEFC ==
                 1) &&
                D_8007BEFC > 0 && count != 0) {

                do {
                    object = objects[remaining];
                    state = object->state;
                    if (!(state->flags & 1) && state->level != 0 &&
                        state->scale + (f32)state->level > 9.0f) {
                        index = mathRnd(0, 4);
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

        if (overlay7RuntimeHandleReloc == NULL && gOverlay7CommitArgument == NULL) {
            found = 0;
            if (gOverlay7Selected != NULL) {
                if (gOverlay7Selected->nested != NULL) {
                    entry = gOverlay7Selected->nested;
                    entry->active = 1;
                    found = 1;
                }
                overlay7ReleaseEntry(gOverlay7Selected);
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
                        value += mathRnd(
                            0, cursor[1]);
                        break;
                    }
                    cursor += 2;
                } while (index--);
                if ((u32)entry->field04 < 0xB4) {
                    amSndPlayXYZ(
                        value, object->x, object->y, object->z, 4,
                        &overlay7RuntimeHandleReloc);
                    overlay7RuntimeLastObjectReloc = object;
                    gOverlay7Selected = entry;
                } else {
                    if (entry->nested != NULL) {
                        overlay7ReleaseRuntimeHandleReloc(entry->nested);
                    }
                    overlay7ReleaseEntry(entry);
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
                               mathRnd(1, 0x64) < 0x24) {
                        overlay7SetRuntimeModeReloc(object, 0xE);
                    }
                } else if (state->previousLevel < state->level &&
                           (!(state->flags & 1) ||
                            mathRnd(1, 0x64) < 0x24)) {
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

/* Bounded reproof 2026-08-29: the identity-correct masked spelling is 128/131
 * raw and 129/131 after runtime relocation normalization, with exact 0x20C
 * size, 0x20 frame, and first substantive mismatch +0x44. The two residual
 * sites at +0x44/+0x64 are one t4-versus-target-t3 flags carrier; the raw
 * +0xA4 switch-table LO16 addend normalizes away. Runtime metadata proves all
 * 23 text plus seven table offsets, types, and identities, including mathRnd
 * at +0x124/+0x1BC. Clean unmasked V0 regressed to 121/131. All 119 flag rows
 * were nonexact; a proc-0 trace found every uopt pool assignment exact and the
 * temp FIFO diverging only at slot 4. Two natural scalar/scope forms regressed
 * to 112/131 and shifted a relocation. ORT 1471 and all six callers are
 * authenticated. The TU's +0x934..+0x950 rodata ownership clears module
 * growth; the function owns +0x894..+0xAA0 with no padding. The fallback
 * remains canonical; retry only after a new natural temp-FIFO phase/reuse
 * spelling, not more flags, explicit carriers, or a generic batch. */
/* Matching reproof 2026-09-08: extracting the unsigned flags sign bit through
 * the right shift below restores the target temp-FIFO phase. Untouched IDO
 * output owns all 0x20C bytes with the exact 0x20 frame and all 23 text plus
 * seven switch-table runtime relocations. The linked owned range and full ROM
 * are byte-identical. */
void overlay7DispatchModes(Overlay7ModeOwner *first, Overlay7ModeOwner *second) {
    Overlay7ModeState *firstState;
    Overlay7ModeState *secondState;
    Overlay7ModeRecord *record;
    Overlay7ModeRecord (*modes)[10];

    secondState = second->state;
    firstState = first->state;
    modes = secondState->alternate == 0 ? gOverlay7AlternateModes
                                        : gOverlay7PrimaryModes;
    record = &modes[firstState->index][secondState->index];

    if ((D_800D3128 << 22) >> 31) {
        secondState->timer = 100;
        secondState->height += 5.0f;
        switch (record->mode) {
        case 1:
            overlay7CreateEntry(first, record->first, 3);
            break;
        case 2:
            overlay7CreateEntry(first, record->first, 3);
            overlay7AppendEntry(second, record->second, 3);
            break;
        case 3:
            overlay7CreateEntry(second, record->second, 3);
            overlay7AppendEntry(first, record->first, 3);
            break;
        case 4:
            if (mathRnd(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                overlay7AppendEntry(second, record->second, 3);
            } else {
                overlay7CreateEntry(second, record->second, 3);
                overlay7AppendEntry(first, record->first, 3);
            }
            break;
        case 0:
        case 5:
            overlay7CreateEntry(first, record->first, 3);
            overlay7CreateEntry(second, record->second, 3);
            break;
        case 6:
            if (mathRnd(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                break;
            }
            overlay7CreateEntry(second, record->second, 3);
            break;
        }
    }
}

/* Matched 2026-10-02 (lane x-ovlb), 81 -> 0 masked words. All 81 were
 * register naming: every web in the check loop sat one colour too high
 * because its top web, the loaded entry value, was denied v0 (the
 * allocator's records show v0 in its forbidden mask). What decides it is
 * where the row pointer is formed. Inside any `if` that guards the loop,
 * the denial happens; one statement after the index call, it does not.
 * The row form `table[index]` also emits the table reload before the
 * shift, which is the target's ring order. `base + index * 4` in the same
 * place was off by one temp and stopped at 27. */
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous) {
    Overlay7CheckState *state;
    s32 failed;
    Overlay7CheckEntry *entries;
    s32 reference;
    s32 index;
    Overlay7CheckEntry (*table)[4];
    s32 i;

    state = owner->state;
    if (*func_80028F54() == 1) {
        failed = 0;
        table = func_800291C4();
        index = levelGetBlurEffect(levelGetNumber());
        entries = table[index];

        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            if (D_8 == 2) {
                reference = entries[3].value;
                for (i = 0; i < 3; i++) {
                    if (state->limits404[i] < reference || reference == 0 ||
                        state->field400 < entries[i].value ||
                        entries[i].value == 0) {
                        failed = 1;
                    }
                }
                if (failed) {
                    overlay7RecordCheckReloc(5);
                } else {
                    overlay7RecordCheckReloc(D_8 + 2);
                }
            } else {
                overlay7RecordCheckReloc(D_8 + 2);
            }
            D_8++;
        }

        if (previous == state->field383) {
            if (failed) {
                overlay7SetOwnerModeReloc(owner, 6);
            } else {
                overlay7SetOwnerModeReloc(owner, 12);
            }
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    } else {
        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            overlay7RecordCheckReloc(D_8 + 2);
            D_8++;
        }
        if (previous == state->field383) {
            overlay7SetOwnerModeReloc(owner, state->field385 + 7);
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    }
}

/* Matched 2026-09-09. The flags test is the same idiom overlay7DispatchModes
 * uses above: `(flags << 22) >> 31` on the unsigned global, not a mask and a
 * signed compare. Both spellings give ugen `lw $14,sym; and $15,$14,1023;
 * sll $24,...`, and as1 folds the redundant `and` either way -- but with the
 * masked spelling it folds it forward, renaming the load's destination to $15
 * (`lui t6; lw t7,0(t6)`), and with the shift-pair spelling it folds it
 * backward onto the shift, leaving the load on its own register
 * (`lui t6; lw t6,0(t6)`). That one temp was the whole two-word residual; the
 * previous plateau's "temp-FIFO phase" reading was right about the symptom and
 * wrong about the owner -- uopt's numbering never moved. */
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection) {
    Overlay7DispatchState *state;
    u16 *override;
    s8 mapped;

    state = owner->state;
    if ((D_800D3128 << 22) >> 31) {
        if (selection >= 14 && selection < 17) {
            override = &gOverlay7DispatchOverride[state->index];
            if (*override == 0) {
                *override = 0x10E;
                goto create;
            }
        } else {
create:
            overlay7CreateEntry(owner,
                                gOverlay7DispatchValues[state->index][selection],
                                gOverlay7DispatchTypes[selection]);
            goto query;
        }
    } else {
query:
        if (camGetMode() == 0) {
            mapped = gOverlay7DispatchMap[selection];
            if (mapped != -1) {
                overlay59AppendValue(0, state->index, mapped,
                                          state->field45D);
            }
        }
    }
}

/* Exact ordinary C: the configured mixed-TU build and linked ROM prove all
 * 72 owned words (288 bytes), with a 48-byte frame and no instruction edits.
 * Fresh raw compiler output has all 17 runtime relocation offsets, types,
 * and identities. The configured object retains 15 static records after
 * two explicitly declared, runtime-owned BSS metadata filters.
 * The removed HI/LO pair is at function offsets +0xF0/+0xF8, corresponding
 * to TU offsets +0x618/+0x620. Both independently resolve to overlay 7's
 * runtime identity +0x1BA0: original BSS base +0x1910 plus addend +0x290.
 * The configured recipe changes relocation metadata, not instruction bytes.
 * Absent-object rebuilding, owned linked bytes, and the full ROM are exact;
 * neighboring functions and data remain unchanged by this function's C.
 */
void overlay7CommitSelection(s32 selection)
{
  u16 value;
  Overlay7Pair *pair;
  s32 remaining;
  if (gOverlay7DispatchModeReloc & 1)
  {
    switch (selection)
    {
      case 29:
        value = 0xCF;
        break;

      case 30:
        value = 0xF5;
        break;

      case 31:
        value = 0x116;
        break;

      default:
        value = ((Overlay7SelectionRow *) (&gOverlay7DispatchData[0x754]))[selection].values[mathRnd(0, 2)];
        break;

    }

    pair = (Overlay7Pair *) (&gOverlay7DispatchData[0x8F4]);
    remaining = 11;
    do
    {
      if (pair->key == value)
      {
        value += mathRnd(0, pair->value);
        break;
      }
      pair++;
    }
    while (remaining--);
    if (value)
    {
      if (gOverlay7DispatchObject != 0)
      {
        amSndStopXYZ(gOverlay7DispatchObject);
        overlay7ReleaseEntry(gOverlay7Selected);
      }
      amSndPlay(value, &gOverlay7CommitArgument);
    }
  }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. The empty
 * condition is inert and keeps the retail register assignment. The store is
 * an ordinary write. See docs/cleanup-queue.md. */
s32 overlay7FillValues(s16 *value) {
    s32 remaining;

    if (((!value) & 0xFFFFU) && (!value)) {
    }
    value = &gOverlay7ValuesEnd;
    remaining = 9;
    do {
        *value = 0xF0;
        value--;
    } while (remaining--);
}

/* Naturally exact under the overlay's ordinary IDO -O2 -mips2 flags. */
void overlay7InitPool(void) {
    Overlay7Entry *entry;
    s32 i;

    entry = gOverlay7FreeHead = gOverlay7EntryStorage;
    for (i = 0; i < 31; i++) {
        entry->next = entry + 1;
        entry->active = 0;
        entry++;
    }
    entry->next = 0;
    gOverlay7ActiveHead = 0;
    gOverlay7ActiveTail = 0;
    gOverlay7Selected = 0;
}
