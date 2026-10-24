#include "overlays/overlay_059.h"

/* Tier B/C: the six readonly font labels referenced by the style pointers.
 * Natural NUL bounds; original declaration/qualification remains inferred. */
const u8 o59StyleText0[] = "1a";
const u8 o59StyleText1[] = "2b";
const u8 o59StyleText2[] = "3c";
const u8 o59StyleText3[] = "4d";
const u8 o59StyleText4[] = "5d";
const u8 o59StyleText5[] = "6d";


/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void overlay59Release(Overlay59State *state);
static s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex, s32 itemIndex);

/* DKR v77/v80 and JFG contain only generic fixed-handle cleanup relatives. */
static void overlay59Release(Overlay59State *state) {
    s32 offset;
    Overlay59State *cursor;

    cursor = state;
    offset = 0;
    if (state->owner != NULL) {
        do {
            if (cursor->handle != NULL) {
                overlay59ReleaseReloc(cursor->handle);
                cursor->handle = NULL;
            }
            offset += 4;
            cursor = (Overlay59State *) ((u8 *) cursor + 4);
        } while (offset != 0x20);
        state->owner = NULL;
    }
}

/*
 * PROVENANCE: Mickey-derived from this overlay's owned assembly and runtime
 * relocation tables; pinned DKR/JFG scans found no donor.
 *
 * Bounded reproof (2026-08-29): the policy-clean split-carrier C compiles to
 * the exact 0xF8/62-word extent with frame 0x28. It matches 52/62 raw words
 * and 53/62 relocation-normalized words, with first raw +0x10 and first
 * substantive +0x54. Ten raw sites remain; normalizing the table LO16 leaves
 * nine sites in one descriptor/call-argument carrier web. All 119 compiler
 * configurations were nonexact, with canonical -O2 -mips2 tied for best. One
 * instrumented UOPT trace found seven colored webs and an otherwise exact
 * temporary-register lane. The trace-supported natural form that coalesced
 * descriptor input with the acquisition result compiled byte-identically to
 * this split-carrier body, so there was no strict gain to retain.
 * The owned range is overlay
 * +0x70..+0x168 (ROM 0x18B87C0..0x18B88B8), with no target padding and the next
 * function beginning at +0x168; the separate +0xA1C..+0xA20 padding is not
 * owned. The LOCAL table pair resolves through
 * base +0xA20 plus +0x5A4 to module +0xFC4. The other four records are local
 * JUMPs to overlay59Release at +0x48/+0xD0 and resident ORT 135 calls to
 * texLoadTexture at +0x64/+0x94. The synthetic target
 * object collapses all four zero-field calls to one placeholder and cannot
 * prove their identities. No linked candidate-C proof survives.
 * The s32 three-argument ABI is called only from four sites in
 * overlay59Advance at module +0x418/+0x590/+0x614/+0x66C; it is not exported
 * and has no resident or cross-overlay inbound. The retained structural oracle
 * is negative (best similarity 0.106, with no credible donor). A 2026-09-04
 * pass merged value and handle in both directions, retaining either the u32 or
 * pointer carrier; both compile byte-identically and do not create the target
 * v0 load/call-delay web. The fallback needs a different natural partition.
 */  /* (was on typedef Overlay59Descriptor) */
static s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex, s32 itemIndex)
{
  void **descriptor;
  void *value;
  u32 handle;
  s32 count;
  s32 result;
  descriptor = ((Overlay59DescriptorGroup *) (((u8 *) gOverlay59DescriptorTables) + 0x5A4))[tableIndex].descriptors[itemIndex];
  result = 1;
  if (descriptor != entry->owner)
  {
    overlay59Release((Overlay59State *)entry);
    entry->owner = descriptor;
    count = 0;
    value = *descriptor;
    if (value != 0)
    {
      do
      {
        handle = ((u32) value) & 0xFFFFFFFF;
        handle = (u32) texLoadTexture((s32) handle);
        if (handle == 0)
        {
          result = 0;
        }
        else
        {
          entry->handles[count] = (void *)handle;
          count++;
        }
        value = descriptor[1];
        if (value != 0)
        {
          handle = ((u32) value) & 0xFFFFFFFFu;
          handle = (u32) texLoadTexture((s32) handle);
          if (handle == 0)
          {
            result = 0;
          }
          else
          {
            entry->handles[count] = (void *)handle;
            count++;
          }
        }
        descriptor += 4;
        value = *descriptor;
      }
      while (value != 0);
    }
    if (result == 0)
    {
      overlay59Release((Overlay59State *)entry);
    }
  }
  return result;
}

/* Prior binding reproof (2026-09-06): the resident acquisition callee consumes an
 * integer texture ID and returns a pointer handle. Explicit casts preserve
 * the existing N64 32-bit carriers. Raw and configured full-TU text remains
 * unchanged; all six relocation identities now agree with the ROM records.
 * The configured resident alias stays distinct from the local release call.
 * This repairs binding evidence only: nine instruction words still differ,
 * and the ordinary ROM proof continues to use the assembly fallback.
 */

/* Matching reproof (2026-09-06): the body above supersedes that plateau.
 * Untouched configured compiler output owns all 248 bytes, with frame 40
 * and all six relocation identities exact. The linked owned range and full
 * ROM are byte-identical. The permuter's zero-valued local on the second
 * failure path was inert and is gone (lane c-5, 2026-10-09). The two
 * `& 0xFFFFFFFF` masks are not: the masked copy is a definition of `handle`
 * from `value`, so the loaded word, the argument and the call result are one
 * web and the load lands in v0 with a `move a0,v0` before each call, as
 * shipped. Without them `value` is propagated into the argument and loads
 * straight into a0 (9 words). `(u32) value`, `(s32) value`, a
 * `(s32) (u32)` round trip, and a `while ((value = descriptor[0]) != 0)` loop
 * reading the call result into `handle` all give that same object.
 */

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */  /* (was on extern gOverlay59Entries) */
void overlay59ResetEntries(void) {
    Overlay59Entry *entry;
    Overlay59Entry *cursor;
    Overlay59Entry *end;
    s32 i;
    s32 limit;

    i = 0;
    cursor = entry = gOverlay59Entries; limit = 8; end = gOverlay59EntriesEnd;
    do {
        entry->x = 0.0f;
        entry->y = 1.0f;
        entry->count = 0;
        entry->state = 0;
        entry->owner = 0;
        i = 0;
        cursor = entry;
        do {
            cursor->handles[0] = 0;
            cursor->handles[1] = 0;
            cursor->handles[2] = 0;
            cursor->handles[3] = 0;
            i += 4;
            cursor = (Overlay59Entry *)((u8 *)cursor + 0x10);
        } while (i != limit);
        entry++;
        cursor = entry;
        i = 0;
    } while (entry != end);
}

/* DKR v77/v80 and JFG contain only generic fixed-array cleanup relatives. */
void overlay59ReleaseAll(void) {
    Overlay59Entry *entry;
    Overlay59Entry *end;

    entry = gOverlay59Entries; end = gOverlay59EntriesEnd;
    do {
        overlay59Release((Overlay59State *)entry);
        entry++;
    } while (entry != end);
}

/* DKR v77/v80 and JFG contain no exact donor for this value append helper. */
void overlay59AppendValue(s32 index, s32 tableIndex, s32 offset, s32 mode) {
    Overlay59Entry *entry;
    s32 count;

    if (index < 0 || index >= 4) {
        return;
    }
    if (tableIndex < 0 || tableIndex >= 11) {
        tableIndex = 0;
    }
    if (mode < 0 || mode >= 6) {
        mode = 5;
    }
    entry = &gOverlay59Entries[index];
    offset = gOverlay59ValueTables[tableIndex][offset];
    count = entry->count;
    if (count != 0 && offset == entry->values[count - 1].first) {
        return;
    }
    if (count < 8) {
        entry->values[count].first = offset;
        entry->values[entry->count].second = mode;
        entry->count++;
    }
}

/* DKR us.v77/us.v80 and JFG scans found no exact or semantic donor. */
void overlay59Update(s32 index) {
    Overlay59Record *record;
    s32 i;

    if (index < 0 || index >= 4) {
        return;
    }
    record = (Overlay59Record *)&gOverlay59Entries[index];
    if (record->state == 0 || record->state == 5) {
        return;
    }
    if (record->count != 0) {
        record->count--;
        record->field1C = 0;
        record->state = 5;
        if (record->count != 0) {
            for (i = 0; i < record->count; i++) {
                record->values[i].first = record->values[i + 1].first;
                record->values[i].second = record->values[i + 1].second;
            }
        }
    }
}

/* The `0.15f` approach factor is a compiler literal in this unit's own
 * .rodata, not an imported global: IDO parks it at .rodata+0 ahead of the
 * six-entry switch table and hoists it into the loop preheader beside the
 * 1.0f and 0.0f constants.  Spelling it as an `extern f32` instead makes the
 * load a pre-loop statement, which ugen emits before the preheader; that
 * ordering alone was the whole 19-word residual this candidate carried. */
void overlay59Advance(s32 steps) {
    Overlay59Entry *entry;
    Overlay59Entry *current;
    Overlay59Value *cursor;
    f32 x;
    f32 y;
    s32 remaining;
    s32 repeat;
    s32 i;
    s32 count;
    s32 target;
    s32 stateOne;
    s32 stateTwo;
    s32 stateFour;
    s32 stateFive;

    stateOne = 1;
    stateTwo = 2;
    stateFour = 4;
    stateFive = 5;
    /* IDO emits this pre-loop copy at its own source line, ahead of the
     * preheader that carries the hoisted 1.0f/0.0f/0.15f constants.  On the
     * loop-header line it joins that preheader, where the four values are
     * emitted in decreasing register order and the entries base lands ahead of
     * the 0.15f literal.  Splitting the line costs two words. */
    entry = gOverlay59Entries; do {
        current = entry;
        remaining = steps;
        do {
            repeat = 0;
            switch (entry->state) {
                case 0:
                    if (entry->count != 0) {
                        if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                            entry->position = 0;
                            entry->timer = 0;
                            entry->state = stateOne;
                            repeat = stateOne;
                        }
                    }
                    break;

                case 1:
                    if (remaining != 0 && entry->timer < 30) {
                        do {
                            x = entry->x;
                            y = entry->y;
                            entry->x += (1.0f - x) * 0.15f;
                            entry->y += (0.0f - y) * 0.15f;
                            remaining--;
                            entry->timer++;
                        } while (remaining != 0 && current->timer < 30);
                    }
                    if (entry->timer >= 30) {
                        entry->x = 1.0f;
                        entry->y = 0.0f;
                        entry->timer = 0;
                        entry->state = stateTwo;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 2:
                    entry->timer += remaining;
                    if (entry->timer >= 10) {
                        remaining = entry->timer - 10;
                        entry->timer = 0;
                        entry->state = 3;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 3:
                    target = (gOverlay59TableInfo[entry->values[0].first].count - 1) << 8;
                    if (remaining != 0 && entry->position < target) {
                        do {
                            remaining--;
                            entry->position += 0x40;
                        } while (remaining != 0 && current->position < target);
                    }
                    if (entry->position >= target) {
                        entry->position = target;
                        entry->timer = 0;
                        entry->state = stateFour;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    overlay59PrepareEntry(entry, entry->values[0].first,
                                          entry->position >> 8);
                    break;

                case 4:
                    entry->timer += steps;
                    if (entry->timer >= 60) {
                        remaining = entry->timer - 60;
                        entry->count--;
                        count = entry->count;
                        if (count != 0) {
                            i = 0;
                            if (count > 0) {
                                cursor = (Overlay59Value *) entry;
                                do {
                                    cursor[4].first = cursor[5].first;
                                    cursor[4].second = cursor[5].second;
                                    i++;
                                    cursor++;
                                } while (i < entry->count);
                            }
                            if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                                entry->position = 0;
                                entry->timer = 0;
                                entry->state = stateTwo;
                            }
                        }
                        if ((s16) entry->state == stateFour) {
                            entry->count = 0;
                            entry->timer = 0;
                            entry->state = stateFive;
                        }
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 5:
                    if (entry->count != 0) {
                        if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                            entry->position = 0;
                            entry->timer = 0;
                            entry->state = stateOne;
                            repeat = stateOne;
                            break;
                        }
                        entry->count = 0;
                        entry->x = 0.0f;
                        entry->y = 1.0f;
                        entry->state = 0;
                        break;
                    }
                    if (remaining != 0 && entry->timer < 30) {
                        do {
                            x = entry->x;
                            y = entry->y;
                            entry->x += (0.0f - x) * 0.15f;
                            entry->y += (1.0f - y) * 0.15f;
                            remaining--;
                            entry->timer++;
                        } while (remaining != 0 && current->timer < 30);
                    }
                    if (entry->timer >= 30) {
                        overlay59Release((Overlay59State *)entry);
                        entry->x = 0.0f;
                        entry->y = 1.0f;
                        entry->state = 0;
                    }
                    break;
            }
        } while (repeat != 0);
        entry++;
    } while (entry != gOverlay59EntriesOneEnd);
}

/* DKR v77/v80 and JFG contain no exact donor for this interpolation helper. */
s32 overlay59Interpolate(s32 index, s32 x, s32 y, s32 targetX, s32 targetY,
                         s32 *outX, s32 *outY, s32 useFirstScale) {
    Overlay59Entry *entry;
    f32 deltaX;
    f32 deltaY;
    s32 result = 0;

    *outX = targetX;
    *outY = targetY;
    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        if (entry->owner != NULL) {
            deltaX = targetX - x;
            deltaY = targetY - y;
            if (useFirstScale) {
                deltaX *= entry->x;
                deltaY *= entry->x;
            } else {
                deltaX *= entry->y;
                deltaY *= entry->y;
            }
            *outX = (s32) deltaX + x;
            *outY = (s32) deltaY + y;
            result = 1;
        }
    }
    return result;
}

/* DKR v77/v80 and JFG contain no exact donor for this list projection. */
void overlay59BuildList(s32 index, Overlay59Output *output) {
    Overlay59Entry *entry;
    Overlay59Node *node;
    s32 *values;

    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        node = (Overlay59Node *)entry->owner;
        if (node != NULL && node->active != 0) {
            values = (s32 *) entry;
            do {
                output->first = values[9];
                values++;
                if (node->includeSecond != 0) {
                    output->second = values[9];
                    values++;
                } else {
                    output->second = 0;
                }
                output->value8 = node->value8;
                output->valueC = node->valueC;
                output->valueE = node->valueE;
                output++;
                node++;
            } while (node->active != 0);
        }
    }
    output->first = 0;
}

/*
 * DKR v77/v80 and JFG contain generic white primitive-color and menu-frame
 * rendering, but no matching three-pass frame routine or exact object donor.
 */
void overlay59DrawFrame(s32 displayList, s32 index, s32 x, s32 y) {
    s32 style;
    void *owner;
    Overlay59Entry *entry;

    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        x += 4;
        owner = entry->owner;
        if (owner != 0) {
            y += 12;
            style = gOverlay59Styles[entry->values[0].second];
            fontUseFont(0);
            fontBackground(0, 0, 0, 0);
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            fontPrintXY(displayList, x - 1, y - 1, style, 0);
            fontColour(0, 0, 0, 0xFF, 0xFF);
            fontPrintXY(displayList, x + 1, y + 1, style, 0);
            fontColour(0xFF, 0, 0, 0xFF, 0xFF);
            fontPrintXY(displayList, x, y, style, 0);
        }
    }
}

/* Tier B/D: reset/release walk four real 68-byte entries. The existing
 * imported views remain separate; this defines storage, not native type closure. */
Overlay59Entry o59EntryStorage[4];
#ifdef TARGET_N64
typedef char O59EntryStorageSize[(sizeof(o59EntryStorage) == 272) ? 1 : -1];
#endif
