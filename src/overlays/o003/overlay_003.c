#include "overlays/overlay_003.h"

/*
 * Overlay 3: AI target selection. One translation unit; FindClosestObject and
 * SelectScoredObject are unexported and static, the rest are the module's
 * exports.
 */

/* Tier B: the updater visits ten groups of 32 pointer/timer entries;
 * the per-group selected-object cache precedes that single entry pool.
 * These are real module-lifetime BSS definitions. Typed operand imports
 * retain the current loader representation; original declarations and every
 * caller's group preconditions are not established by this storage view. */
Overlay3Object *gOverlay3Objects[10];
Overlay3Entry gOverlay3Entries[10][32];

s32 overlay3ContainsValue(Overlay3Object *owner, Overlay3Object *object) {
    Overlay3Entry *entry;
    s32 group;
    s32 previous;
    s32 remaining;

    group = owner->state->group;
    remaining = 31;
    entry = &gOverlay3Entries_o003Reloc[group][31];
    do {
        if (object == entry->object) {
            return 1;
        }
        previous = remaining--;
        entry--;
    } while (previous);
    return 0;
}

/* DKR v77/v80 only provide generic active-object reset relatives. */
void overlay3ResetObjects(void) {
    s32 count;
    Overlay3Object **objects;
    Overlay3State *state;

    objects = func_80005750(&count);
    if (count--) {
        do {
            state = objects[count]->state;
            state->cachedIndex = 0x7F;
            state->timer = 0;
            state->scoreHigh = 0;
            state->scoreLow = 0;
        } while (count--);
    }
}

/* Matched by two shape changes: the validity flag is one natural `&&` chain
 * (cfe's own temporary supplies the copy at the join), and the encode helper
 * takes one argument -- the second argument register only held the leftover
 * intermediate of the 16-bit conversion. */
s32 overlay3RunCachedModeAction(Overlay3Object *anchor, Overlay3State *control) {
    s32 count;
    Overlay3Object **objects;
    Overlay3Object *target;
    s32 valid;
    s32 angle;
    s32 random;

    objects = func_80005750(&count);
    if (control->cachedIndex == 0x7F) {
        return 0;
    }
    target = objects[control->cachedIndex];
    if (target == anchor) {
        return 0;
    }
    angle = mathDiffAngle(anchor->heading,
        Arctanf(anchor->x - target->x, anchor->z - target->z));
    valid = (s16)angle >= -1999 && (s16)angle < 2000 &&
            func_overlay_002_F0001A94_185888C(anchor->x, anchor->z, target->x, target->z,
                                   overlay1GetEntry(1), 0, -1, 0xFFFF) == 0;
    switch (control->mode) {
        case 3:
            if (valid) {
                random = mathRnd(1, 100);
                if (gOverlay3ModeChance_o003Reloc[control->group] < random) {
                    overlay36SpawnDirectional(anchor);
                }
            }
            break;
        case 4:
            if (valid) {
                random = mathRnd(1, 100);
                if (gOverlay3ModeChance_o003Reloc[control->group] < random) {
                    overlay36SpawnLinked7F(anchor);
                }
            }
            break;
        case 6:
            overlay36CallModeZero(anchor);
            break;
        case 7:
            overlay36TickState(anchor);
            break;
    }
    return 0;
}

/* Exact C: all 77 instruction words, the 0x78 frame, relocations, and linked
 * overlay range match after bounded permutation. Two inert allocation/block
 * aids remain below and are tracked in docs/cleanup-queue.md: the empty
 * `!cursor` test and the `if (1)` around the loop body each swap the index and
 * cursor registers (s3/s4) when deleted. The `&pad` probe was inert and is
 * gone; the pad itself holds the 0x78 frame. */
static Overlay3Object *overlay3FindClosestObject(Overlay3Object *anchor, Overlay3State *unused)
{
  s32 pad;
  s32 start;
  s32 end;
  f32 distance;
  f32 bestDistance;
  Overlay3Object **objects;
  Overlay3Object **cursor;
  Overlay3Object *object;
  Overlay3Object *best;
  Overlay3State *state;
  s32 index;
  objects = func_8000572C(&start, &end);
  bestDistance = gOverlay3SearchMaxDistance_o003Reloc;
  best = 0;
  index = start;
  if (start < end)
  {
    cursor = objects;
    cursor += start;
    if (!cursor)
    {
    }
    do
    {
      object = *cursor;
      if (1)
      {
        if ((object->type == 0x86) || (object->type == 0xEB))
        {
          state = object->state;
          if ((overlay3ContainsValue(anchor, object) == 0) && (state->flags == 0))
          {
            distance = func_8000BD0C(object->x, object->y, object->z, anchor->x, anchor->y, anchor->z);
            if (distance < bestDistance)
            {
              bestDistance = distance;
              best = object;
            }
          }
        }
        index++;
        cursor++;
      }
    }
    while (index < end);
  }
  return best;
}

/*
 * Matched 2026-09-16 (lane lm-a), 118/118 words at frame 0x80, unforced.
 * Two facts, both measured (docs/lastmile-region-boundary.md):
 *   - the loop subscripts `objects[index]` and lets strength reduction own
 *     the walking cursor (L160). With a declared cursor the helper result
 *     was one web coloured v0 unopposed; generated, uopt keeps the call
 *     result for the cached path, copies it into a1 before the guard, and
 *     both reloads of `count` take v0, which is the ROM's shape;
 *   - the frame is 0x80 only with one more pointer-typed declaration than
 *     the body uses: `cursor` is kept, unused, for that reason. An unused
 *     f32 in its place is eliminated and the frame falls to 0x78. Listed in
 *     docs/cleanup-queue.md.
 */
static Overlay3Object *overlay3SelectScoredObject(Overlay3Object *anchor, Overlay3State *search, s32 elapsed) {
    s32 count; Overlay3Object **objects; Overlay3Object **cursor; Overlay3Object *result;
    Overlay3Object *object; Overlay3State *state; s32 index; s32 bestScore;
    s32 bestIndex; s32 score; f32 dx; f32 dz; u16 timer; u8 cachedIndex;
    objects = func_80005750(&count);
    result = 0;
    if ((elapsed < search->timer) && ((cachedIndex = search->cachedIndex) != 0x7F)) {
        search->timer = search->timer - elapsed;
        result = objects[cachedIndex];
    } else {
        bestScore = -1000000;
        index = count - 1;
        if (count != 0) {
            do {
                object = objects[index];
                if ((object != anchor) && (object->state->blocked == 0) &&
                    (overlay3ContainsValue(anchor, object) == 0)) {
                    score = mathRnd(0, 2000);
                    state = object->state;
                    score += (state->scoreHigh - state->scoreLow) * 1000;
                    dx = anchor->x - object->x;
                    dz = anchor->z - object->z;
                    score -= ((s32)sqrtf((dx * dx) + (dz * dz))) / 8;
                    score += search->weights[state->group] << 8;
                    if (bestScore < score) {
                        bestScore = score;
                        bestIndex = index;
                        result = object;
                    }
                }
            } while (index--);
        }
        if (result != 0) {
            search->cachedIndex = bestIndex;
            search->timer = mathRnd(3, 6) * 60;
        }
    }
    return result;
}

void overlay3SelectTarget(
    f32 *outX, f32 *outZ, Overlay3State *state,
    Overlay3Object *anchor, s32 updateRate) {
    Overlay3Object *selected;

    if (state->useWeightedSearch == 0) {
        selected = overlay3FindClosestObject(anchor, state);
    } else {
        selected = overlay3SelectScoredObject(anchor, state, updateRate);
        if (selected == 0) {
            selected = overlay3FindClosestObject(anchor, state);
        }
    }

    if (selected == gOverlay3Objects_o003Reloc[state->group]) {
        state->sameObjectTimer += updateRate;
        if (state->sameObjectTimer > 300.0f) {
            overlay3TouchObject_o003Reloc(state->group);
            selected = overlay3FindClosestObject(anchor, state);
            state->sameObjectTimer = 0;
        }
    } else {
        state->sameObjectTimer = 0;
    }

    gOverlay3Objects_o003Reloc[state->group] = selected;
    if (selected != 0 && selected->kind != 1) {
        state->cachedIndex = 0x7F;
        state->timer = 0;
    }

    if (selected != 0) {
        *outX = selected->x;
        *outZ = selected->z;
    } else {
        *outX = anchor->x;
        *outZ = anchor->z;
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this fixed object registry. */
void overlay3TouchObject(s32 group, s32 remaining) {
    Overlay3Entry *base;
    Overlay3Entry *entry;

    remaining = 31;
    if (gOverlay3Objects_o003Reloc[group] == 0) {
        return;
    }
    base = gOverlay3Entries_o003Reloc[group];
    entry = &base[remaining];
    do {
        if (entry->object == gOverlay3Objects_o003Reloc[group]) {
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);

    remaining = 31;
    base += 31;
    entry = base;
    do {
        if (entry->object == 0) {
            entry->object = gOverlay3Objects_o003Reloc[group];
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);
}

void overlay3UpdateTimedEntries(s32 amount) {
    Overlay3Entry *group;
    Overlay3Entry *entry;
    s32 groupRemaining;
    s32 remaining;
    register s32 timer;

    timer = 0;
    groupRemaining = 9; group = gOverlay3Entries_o003Reloc[9];
    do {
        remaining = 31;
        entry = &group[31];
        do {
            timer = entry->timer;
            if (timer != 0) {
                if (amount < timer) {
                    entry->timer = timer - amount;
                } else {
                    entry->object = NULL;
                    entry->timer = 0;
                }
            }
            entry--;
        } while (remaining--);
        group -= 32;
    } while (groupRemaining--);
}
