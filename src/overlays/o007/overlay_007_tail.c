#define OVERLAY_007_DEFINE_BSS
#include "overlays/overlay_007.h"

/* Overlay 7's local BSS through +0x2a0; the final 0x10 remains atlas-owned. */
static Overlay7Entry gOverlay7EntryStorage[32];
static s32 gOverlay7Reserved280;
static Overlay7Entry *gOverlay7ActiveHead;
static Overlay7Entry *gOverlay7FreeHead;
static Overlay7Entry *gOverlay7ActiveTail;
static Overlay7Entry *gOverlay7Selected;

typedef struct Overlay7SelectionRow {
    u16 values[3];
} Overlay7SelectionRow;

/* Overlay 7, ADR 0006 consolidation: C after the middle assembly island. */

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

    if ((gOverlay7DispatchFlagsReloc << 22) >> 31) {
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
    if (*overlay7GetModeReloc() == 1) {
        failed = 0;
        table = overlay7GetCheckTableReloc();
        index = overlay7GetCheckIndexReloc(overlay7GetCurrentReloc());
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
    if ((gOverlay7DispatchFlagsReloc << 22) >> 31) {
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
        if (camGetModeReloc() == 0) {
            mapped = gOverlay7DispatchMap[selection];
            if (mapped != -1) {
                overlay59AppendValueReloc(0, state->index, mapped,
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
        func_800031E8(gOverlay7DispatchObject);
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

