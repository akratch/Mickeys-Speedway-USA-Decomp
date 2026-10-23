#include "overlays/overlay_068.h"

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static f32 overlay68Interpolate( f32 weight, s32 previous, s32 current, s32 next, s32 following, s32 wrap, f32 *derivative, s32 linear);

/*
 * Overlay 68 +0x000. Fresh DKR v77/v80 source searches for 0x2EF0 and
 * payload-limit/copy-clamp shapes were negative.
 */
s32 overlay68PayloadLimit(void) {
    return 0x2EF0;
}

void overlay68CreatePrimary(void) {
    Overlay68EntryHeader *entry;

    if (D_8007A1F4 == 0) {
        entry = mmAlloc(overlay68PayloadLimit_o068Reloc(), 0x8F);
        if (entry != 0) {
            entry->active = 0;
            entry->field4 = 0;
            entry->timer = 0;
            entry->index = -1;
            entry->width = 0;
            entry->generation = 0;
            entry->payload = entry + 1;
            D_8007A1F4 = entry;
        }
    }
}

void overlay68CreateSecondary(void) {
    Overlay68EntryHeader *entry;

    if (gOverlay68SecondaryEntry == 0) {
        entry = mmAlloc(overlay68PayloadLimit_o068Reloc(), 0x85);
        if (entry != 0) {
            entry->active = 0;
            entry->field4 = 0;
            entry->timer = 0;
            entry->index = -1;
            entry->width = 0;
            entry->generation = 0;
            entry->payload = entry + 1;
            gOverlay68SecondaryEntry = entry;
        }
    }
}

/* DKR v77/v80 has generic release wrappers only, with no exact donor. */  /* (was on extern D_8007A1F4) */
void overlay68ReleasePrimary(void) {
    if (D_8007A1F4 != 0) {
        mmFree(D_8007A1F4);
    }
    D_8007A1F4 = 0;
}

/* DKR v77/v80 has generic release wrappers only, with no exact donor. */  /* (was on extern gOverlay68Secondary) */
void overlay68ReleaseSecondary(void) {
    if (gOverlay68Secondary != 0) {
        mmFree(gOverlay68Secondary);
    }
    gOverlay68Secondary = 0;
}

/* Pinned DKR v77/v80 and JFG object/source checks found payload-construction
 * relatives only; none is an exact donor for this descriptor layout. */
void overlay68CreatePayload(Overlay68Entry *entry) {
    s32 kind;
    Overlay68CreateInfo info;
    Overlay68Payload *payload;

    if (entry != 0) {
        if (entry->width != 0) {
            if (entry->height != 0) {
                if (levelGetNumber() == entry->generation) {
                    kind = entry->kind;
                    payload = entry->payload;
                    if ((kind < 0) || (kind >= 10)) {
                        kind = 0;
                    }
                    info.type = gOverlay68Types[kind];
                    info.x = payload->x;
                    info.y = payload->y;
                    info.z = payload->z;
                    info.red = payload->red;
                    info.green = payload->green;
                    info.blue = payload->blue;
                    info.entry = entry;
                    entry->object = func_8000590C(&info, 1);
                    return;
                }
            }
        }
        entry->object = 0;
    }
}

/* DKR v77/v80 and JFG have related object attachment code, but no exact donor. */  /* (was on typedef Overlay68SourceMeta) */
void overlay68AttachObject(Overlay68SourceObject *object) {
    Overlay68Attachment *attachment;
    Overlay68Attachment * volatile savedAttachment;
    Overlay68AttachmentPayload *payload;
    s16 generation;

    attachment = (Overlay68Attachment *)gOverlay68Entry;
    if (attachment != 0) {
        attachment->object = object;
        attachment->kind = object->meta->kind;
        attachment->field5 = 0;
        savedAttachment = attachment;
        generation = levelGetNumber(object, attachment);
        attachment = savedAttachment;
        attachment->generation = generation;
        attachment->timer = 0;
        attachment->phase = 0;
        payload = attachment->payload;
        payload->red = object->x >> 8;
        payload->green = object->y >> 8;
        payload->blue = object->z >> 8;
        payload->x = object->worldX;
        payload->y = object->worldY;
        payload->z = object->worldZ;
        payload->active = 0;
    }
}

/* DKR v77/v80 and JFG searches found no matching fixed-point trail updater. */  /* (was on typedef Overlay68TrailSource) */
void overlay68UpdateTrail(s32 updateRate) {
    Overlay68Trail *trail;
    Overlay68TrailSource *source;
    Overlay68TrailPoint *point;
    Overlay68Trail * volatile savedTrail;
    s8 delay;
    s16 index;
    s32 threshold;

    trail = gOverlay68Entry;
    if (trail != 0) {
        source = trail->source;
        if (source != 0) {
            index = trail->index;
            if (index < 0x4AF) {
                point = &trail->points[index];
                if (D_800C947C != 0) {
                    point->red = source->x >> 8;
                    point->green = source->y >> 8;
                    point->blue = source->z >> 8;
                    point->x = source->worldX;
                    point->y = source->worldY;
                    point->z = source->worldZ;
                    return;
                }

                delay = trail->delay;
                if (delay == 0) {
                    trail->elapsed += updateRate;
                } else {
                    trail->delay = delay - updateRate;
                    if (trail->delay <= 0) {
                        savedTrail = trail;
                        overlay68FinishEntry_o068Reloc(delay, trail, updateRate);
                        trail = savedTrail;
                        trail->delay = 0;
                        return;
                    }
                }

                point->timer += updateRate;
                index = trail->index;
                threshold = index < 5 ? 6 : 12;
                if (point->timer >= threshold) {
                    trail->index = index + 1;
                    point++;
                    point->red = source->x >> 8;
                    point->green = source->y >> 8;
                    point->blue = source->z >> 8;
                    point->x = source->worldX;
                    point->y = source->worldY;
                    point->z = source->worldZ;
                    point->timer = 0;
                }
            }
        }
    }
}

void overlay68ClearNestedFlag(Overlay68NestedObject **objectPtr) {
    Overlay68NestedObject *object;
    Overlay68NestedState *state;

    if (objectPtr != 0) {
        object = *objectPtr;
        if (object != 0) {
            state = object->state;
            if (state != 0) {
                state->flag = 0;
            }
        }
    }
}

void overlay68FinishEntry(void) {
    Overlay68EntryHeader *entry = (Overlay68EntryHeader *)gOverlay68Entry;

    if (entry != 0 && entry->active != 0) {
        entry->generation++;
        entry->active = 0;
    }
}

void overlay68StartTimer(void) {
    Overlay68TimerEntry *entry = gOverlay68TimerEntry;

    if (entry != 0 && entry->active != 0 && entry->timer == 0) {
        entry->timer = 120;
    }
}

/* Matched 2026-08-29 by a bounded decomp-permuter pass after the prior hand,
 * flag, and allocator-trace probes plateaued on the frame and stack homes.
 * Untouched IDO output preserves the 77-word body, 0x30 frame, and nine
 * relocation sites; the complete linked US ROM is byte-identical.
 *
 * `primaryCopy` (the permuter's `new_var`) is never read, but the chained
 * assignment is load-bearing (lane c-5, 2026-10-09): a plain
 * `primary = D_8007A1F4;` moves the stack homes (10 words), with
 * or without a pad in its place, and testing and passing `primary` instead of
 * re-reading the global differs in 14. */
void overlay68PromoteSecondary(void)
{
  Overlay68RecordEntry *primary;
  Overlay68Record *source;
  Overlay68Record *destination;
  Overlay68RecordEntry *secondary;
  Overlay68RecordEntry *primaryCopy;
  s32 remaining;
  secondary = (Overlay68RecordEntry *)gOverlay68SecondaryEntry;
  if (secondary != ((void *) 0))
  {
    primary = (primaryCopy = (Overlay68RecordEntry *)D_8007A1F4);
    if (D_8007A1F4 != ((void *) 0))
    {
      overlay68ClearNestedFlag_o068Reloc(D_8007A1F4);
      overlay68ClearNestedFlag_o068Reloc(D_8007A1F8);
      if (secondary->field5 != 0)
      {
        overlay68FinishEntry_o068Reloc();
      }
      if (((secondary->timer != 0) && (secondary->object == ((void *) 0))) && ((secondary->timer < primary->timer) || (secondary->generation != primary->generation)))
      {
        primary->kind = secondary->kind;
        primary->generation = secondary->generation;
        primary->timer = secondary->timer;
        primary->recordCount = secondary->recordCount;
        remaining = secondary->recordCount;
        source = secondary->records;
        destination = primary->records;
        while (remaining--)
        {
          destination->red = source->red;
          destination->green = source->green;
          destination->blue = source->blue;
          destination->x = source->x;
          destination->y = source->y;
          destination->z = source->z;
          destination->alpha = source->alpha;
          source++;
          destination++;
        }

        secondary->timer = 0;
      }
    }
  }
}

static f32 overlay68Interpolate(
    f32 weight,
    s32 previous,
    s32 current,
    s32 next,
    s32 following,
    s32 wrap,
    f32 *derivative,
    s32 linear) {
    f32 fPrevious;
    f32 fCurrent;
    f32 fNext;
    f32 fFollowing;
    s32 difference;
    u32 period;

    if (wrap != 0) {
        difference = current - previous;
        period = 0x10000;
        if (difference >= 0x8001) {
            current -= period;
            next -= period;
            following -= period;
        } else if (difference < -0x8000) {
            current += period;
            next += period;
            following += period;
        }

        difference = next - current;
        period = 0x10000;
        if (difference >= 0x8001) {
            next -= period;
            following -= period;
        } else if (difference < -0x8000) {
            next += period;
            following += period;
        }

        difference = following - next;
        if (difference >= 0x8001) {
            following -= period;
        } else if (difference < -0x8000) {
            following += period;
        }
    }

    if (linear != 0) {
        fCurrent = (f32)current;
        if (derivative != NULL) {
            *derivative = 0.0f;
        }
        return (((f32)next - fCurrent) * weight) + fCurrent;
    }

    if (derivative != NULL) {
        fPrevious = (f32)previous;
        fCurrent = (f32)current;
        fNext = (f32)next;
        fFollowing = (f32)following;
        *derivative = (((((-0.5f * fPrevious) + (1.5f * fCurrent) + (-1.5f * fNext) +
            (0.5f * fFollowing)) * 3.0f * weight) +
            (2.0f * (fPrevious + (-2.5f * fCurrent) + (2.0f * fNext) +
            (-0.5f * fFollowing)))) * weight) +
            ((-0.5f * fPrevious) + (0.5f * fNext));
    }

    fPrevious = (f32)previous;
    fCurrent = (f32)current;
    fNext = (f32)next;
    fFollowing = (f32)following;
    return (((((((-0.5f * fPrevious) + (1.5f * fCurrent) + (-1.5f * fNext) +
        (0.5f * fFollowing)) * weight) +
        (fPrevious + (-2.5f * fCurrent) + (2.0f * fNext) +
        (-0.5f * fFollowing))) * weight) +
        ((-0.5f * fPrevious) + (0.5f * fNext))) * weight) + fCurrent;
}

/*
 * DKR v77/v80 and JFG source searches found only generic colour/opacity and
 * fixed-point initialization idioms, not an exact donor for this state shape.
 */  /* (was on typedef Overlay68ObjectState) */
void overlay68InitializeObject(Overlay68Object *object, Overlay68ObjectInit *init) {
    Overlay68ObjectState *state;

    state = object->state;
    object->red = init->red << 8;
    object->green = init->green << 8;
    object->blue = init->blue << 8;
    state->animation = init->resource;
    state->elapsed = 0;
    state->angle = 0;
    state->keyframeIndex = 0;
    state->active = 1;
    state->opacity = 0x80;
    state->fraction = 0.0f;
    func_8005AD64(object, 0, -1, 0.0f);
}

/* The split assembly normalizes these five resident roles to one opaque call
 * symbol.  Distinct proxies preserve their independently proved physical
 * signatures while source shape is iterated. */  /* (was on extern func_8000FAE0) */
/* The shipped overlay relocation table proves both loads resolve through its
 * reserved-BSS entry to resident D_800C947C.  This proxy remains zero-linked
 * in the overlay object; the retained runtime relocation table is authoritative. */  /* (was on extern D_800C947C) */
#define OVERLAY68_GLOBAL_FLAG D_800C947C

/*
 * Matched 2026-10-07 (lane g-near), 2 -> 0 at size 0. The last two words
 * were the order of atStart's home store against its argument store before
 * the first interpolation call: the target saves the v0 web with the call's
 * spill group (ascending register order, after the argument stores), ours
 * stored it at the definition. uopt forward-substituted `index < 1` into
 * the argument on every statement form, which left the symbol's call-block
 * piece with no reference but its store, so it was never coloured. The
 * no-op redefinition `index = (s16)index;` between the atStart statement and
 * the call kills that substitution (the compiler deletes the cast itself:
 * `|= 0`, `&= -1` and `^= 0` measure the same; `+= 0` and `= index` do
 * not), so the argument reads the symbol, the web is coloured v0 and saved
 * at the call, while index stays live into the call block and keeps t0.
 * Earlier steps, in order: the duration loop carries the keyframe index in
 * `index` (stored as index + 1, re-read in the exit test) so one web spans
 * the loop and the neighbour selection (lane x-sort, 180 -> 108); the
 * neighbour selection is three plain ifs after copies made in the order
 * afterAfter, after, before, the red sample is read in the argument list,
 * and the opacity store is the plain byte store ahead of the elapsed add
 * (lane a-ovl2, 106 -> 6); the declarations follow the target's home map
 * (atStart 0x6C, tangents 0x60/0x5C, current 0x58, before 0x54, after 0x4C,
 * afterAfter 0x48, state 0x40) with two unused cells for the 0x78 frame.
 */
void overlay68UpdateAnimation(Overlay68Object *object, s32 updateRate) {
    s32 direction;
    s32 animationOpacity;
    s32 atStart;
    s16 angle;
    s32 opacity;
    f32 tangentX;
    f32 tangentZ;
    Overlay68Keyframe *current;
    Overlay68Keyframe *before;
    s32 index;
    Overlay68Keyframe *after;
    Overlay68Keyframe *afterAfter;
    Overlay68Animation *animation;
    Overlay68ObjectState *state;
    s32 heading;
    s32 unused2;

    state = object->state;
    if (OVERLAY68_GLOBAL_FLAG != 0) {
        updateRate = 0;
    }

    if (state->active != 0) {
        animation = state->animation;
        if (animation != 0) {
            state->angle += updateRate;
            if (state->angle >= 0x3841) {
                state->angle = 0x3840;
            }

            if (state->angle >= 0x37C1) {
                opacity = 0x80 - ((state->angle - 0x37C0) << 1);
            } else {
                opacity = 0x80;
            }
            if (animation->endAngle < state->angle) {
                animationOpacity = 0x80 -
                    ((state->angle - animation->endAngle) << 1);
            } else {
                animationOpacity = 0x80;
            }
            if (opacity < animationOpacity) {
                animationOpacity = opacity;
            }
            if (animationOpacity < 0) {
                animationOpacity = 0;
            }
            state->opacity = animationOpacity;
            state->elapsed += updateRate;
            index = state->keyframeIndex;
            current = &animation->keyframes[index];
            while (state->elapsed >= current->duration) {
                state->elapsed -= current->duration;
                state->keyframeIndex = index + 1;
                current++;
                if ((index = state->keyframeIndex) >= animation->keyframeCount) {
                    state->keyframeIndex = animation->keyframeCount - 1;
                    state->elapsed = 0;
                    state->active = 0;
                    current = &animation->keyframes[state->keyframeIndex];
                    break;
                }
            }

            if (current->duration) {
                state->fraction = (f32)state->elapsed / (f32)current->duration;
            } else {
                state->fraction = 0.0f;
            }

            index = state->keyframeIndex;
            afterAfter = current;
            after = current;
            before = current;
            if (index > 0) {
                before = current - 1;
            }
            if (index < animation->keyframeCount - 1) {
                after = current + 1;
            }
            if (index < animation->keyframeCount - 2) {
                afterAfter = current + 2;
            }

            atStart = index < 1;
            index = (s16)index;
            object->red = (s16)(s32)overlay68Interpolate(
                state->fraction, before->red << 8, current->red << 8,
                after->red << 8, afterAfter->red << 8, 1, 0, atStart);
            object->green = (s16)(s32)overlay68Interpolate(
                state->fraction, before->green << 8, current->green << 8,
                after->green << 8, afterAfter->green << 8, 1, 0, atStart);
            object->blue = (s16)(s32)overlay68Interpolate(
                state->fraction, before->blue << 8, current->blue << 8,
                after->blue << 8, afterAfter->blue << 8, 1, 0, atStart);
            object->x = overlay68Interpolate(
                state->fraction, before->x, current->x, after->x,
                afterAfter->x, 0, &tangentX, atStart);
            object->y = overlay68Interpolate(
                state->fraction, before->y, current->y, after->y,
                afterAfter->y, 0, 0, atStart);
            object->z = overlay68Interpolate(
                state->fraction, before->z, current->z, after->z,
                afterAfter->z, 0, &tangentZ, atStart);

            object->facingAngle = func_8000FAE0(object->x, object->y,
                object->z);
            if ((tangentX == 0.0f) && (tangentZ == 0.0f)) {
                angle = 0;
            } else {
                heading = Arctanf(tangentX, tangentZ);
                angle = mathDiffAngle(object->red,
                    heading - 0x8000);
            }

            if (OVERLAY68_GLOBAL_FLAG != 0) {
                direction = 0;
            } else if (angle < -0x2000) {
                direction = 7;
            } else if (angle < -0x800) {
                direction = 5;
            } else if (angle < -0x100) {
                direction = 3;
            } else if (angle >= 0x2001) {
                direction = 6;
            } else if (angle >= 0x801) {
                direction = 4;
            } else if (angle >= 0x101) {
                direction = 2;
            } else {
                direction = 1;
            }

            if (direction != object->direction) {
                func_8005AD64(object, direction, -1, 0.0f);
            }
            func_8005ABA8(object, 0.04f, (f32)updateRate);
        }
    }
}

#define OVERLAY68_SET_ENV_COLOR(pkt) { Overlay68Command *_g = (Overlay68Command *)(pkt); _g->w0 = 0xFB000000; _g->w1 = 0xFFFFFF00; }

/*
 * Matched 2026-10-02 (lane x-sort) by rewriting the inherited body from the
 * listing in the shape of overlay 69's sorted renderer: the environment
 * colour as one packet macro, a plain counted collect loop ending in
 * i != 4, entry++ before count++, the bubble sort reusing i as its outer
 * index with the swap through one temporary, and the submit loop reading
 * order[i] into its own local before indexing entries[].  The 0x108 frame
 * comes from declaring five scalars above order[] and three between the
 * descriptor and entries[].
 */
void overlay68DrawSortedEntries(Overlay68Command **displayList, void *mtx, void *vertices,
                                Overlay68DrawObject *object) {
    Overlay68VectorOwner *owner;
    Overlay68DrawEntry *entry;
    Overlay68Vector *vector;
    s32 count;
    s32 i;
    s16 order[8];
    f32 inverseScale;
    f32 distances[8];
    Overlay68DrawDescriptor descriptor;
    s32 slot;
    s32 j;
    s32 temp;
    Overlay68DrawEntry *entries[8];

    owner = *object->vectorOwner;
    if (owner == 0) {
        return;
    }

    OVERLAY68_SET_ENV_COLOR((*displayList)++);

    entry = object->entries;
    count = 0;
    if (entry != 0) {
        for (i = 0; (i < object->entryCount) && (i != 4); i++) {
            vector = &owner->vectors[entry->vectorIndex];
            distances[count] = camGetProjZ(vector->x, vector->y,
                                                           vector->z);
            entries[count] = entry;
            order[count] = count;
            entry++;
            count++;
        }
    }

    if (count > 0) {
        for (i = count - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                if (distances[order[j + 1]] < distances[order[j]]) {
                    temp = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = temp;
                }
            }
        }

        func_80022E80(object);
        inverseScale = 1.0f / *object->scale;
        descriptor.mode6 = 3;
        descriptor.angle = 0x3333;
        for (i = 0; i < count; i++) {
            slot = order[i];
            entry = entries[slot];
            vector = &owner->vectors[entry->vectorIndex];
            descriptor.zero0 = 0;
            descriptor.zero2 = 0;
            descriptor.weight = entry->weight * inverseScale;
            descriptor.one = 1.0f;
            descriptor.x = vector->x;
            descriptor.y = vector->y;
            descriptor.z = vector->z;
            descriptor.sourceWord = entry->word0;
            func_80022FD4(displayList, mtx, vertices, object,
                                      object->renderState, &descriptor, 0xE,
                                      object->mode39);
        }
    }
}

/* DKR v77/v80 and JFG searches found no exact donor for this resource path. */  /* (was on typedef Overlay68KindPair) */
void overlay68RebuildSecondaryEntry(s32 kind) {
    s32 amount;
    const Overlay68KindPair *mapping;
    s32 entrySize;
    Overlay68ResidentEntry *selectedEntry;
    void *payload;

    gOverlay68SecondaryEntry = 0;
    amount = -1;
    mapping = gOverlay68KindMap;

    while (mapping->kind != -1) {
        if (kind == mapping->kind) {
            amount = mapping->amount;
            break;
        }
        mapping++;
    }

    if (amount != -1) {
        Overlay68EntryHeader *entry;

        entrySize = overlay68PayloadLimit_o068Reloc();
        entry = mmAlloc(entrySize, 0x85);
        if (entry != 0) {
            Overlay68Probe *probe;

            probe = mmAlloc(sizeof(*probe), 0x85);
            if (probe != 0) {
                Overlay68ResidentEntry *entries;
                s32 threshold;
                s32 index;

                piRomLoadSection(0x3F, probe,
                                             amount * (s32)sizeof(*probe),
                                             sizeof(*probe));
                entries = func_800291C4();
                index = levelGetBlurEffect(kind);
                selectedEntry = entries;
                selectedEntry += index;
                threshold = selectedEntry->thresholdNumerator / 5;
                if (threshold == 0) {
                    threshold = 0x7080;
                }

                index = 0;
                while ((index < 4) && (probe->assetSizes[index] != 0) &&
                       (threshold < probe->values[index])) {
                    index++;
                }

                if (index >= 4) {
                    index = 3;
                } else if (probe->assetSizes[index] == 0) {
                    index--;
                }

                if (index >= 0) {
                    piRomLoadSection(0x40, entry,
                                                 probe->assetOffsets[index],
                                                 probe->assetSizes[index]);
                    payload = entry + 1;
                    entry->payload = payload;
                    gOverlay68SecondaryEntry = entry;
                }
                mmFree(probe);
            }
            if (gOverlay68SecondaryEntry == 0) {
                mmFree(entry);
            }
        }
    }
}

/* DKR v77/v80 has generic release wrappers only, with no exact donor. */  /* (was on extern D_8007A1F8) */
void overlay68ReleaseTertiary(void) {
    if (D_8007A1F8 != 0) {
        mmFree(D_8007A1F8);
    }
    D_8007A1F8 = 0;
}

/* DKR v77/v80 and JFG searches found no exact donor for this probe check. */  /* (was on typedef Overlay68KindPair) */
/* Both aliases bind to the same overlay-local kind/amount table. */  /* (was on extern gOverlay68KindMapInitial) */
/* The runtime relocation stream preserves these five distinct call roles. */  /* (was on extern mmAlloc) */
/* Matched 2026-09-17 (lane w5-o068), 10 -> 0 masked words at delta 0,
 * frame 0x48, nine relocations, unforced. L145/L160: delete the walking
 * valueCursor local and subscript probe->values[index] so IDO generates
 * the scaled cursor. A while, not a guarded do, keeps that index as a
 * preheader variable so as1 emits the scaled-zero init. L67: the map-loop
 * back-edge keeps the copy-propagated -1 first only when currentKind is
 * materialised at the test; the inert xor is tracked in
 * docs/cleanup-queue.md. */
s32 overlay68CheckKind(s32 kind) {
    s32 amount;
    const Overlay68KindPair *mapping;
    volatile const s8 *loopMapping;
    s32 currentKind;
    s32 result;
    s32 threshold;
    Overlay68Probe *probe;
    s32 value;
    Overlay68ResidentEntry *entries;
    s32 index;

    result = 0;
    amount = -1;
    mapping = gOverlay68KindMapInitial;

    if (mapping->kind != -1) {
        loopMapping = &gOverlay68KindMapLoop;
        currentKind = *loopMapping;
        do {
            if (kind == currentKind) {
                amount = mapping->amount;
                break;
            }
            mapping++;
            currentKind = mapping->kind;
            /* Legacy inert xor preserves IDO's comparison operand order;
             * tracked in docs/cleanup-queue.md. */
        } while ((currentKind ^ 0) != -1);
    }

    if (amount != -1) {
        probe = mmAlloc(sizeof(*probe), 0x85);
        if (probe != NULL) {
            piRomLoadSection(0x3F, probe,
                                    amount * (s32)sizeof(*probe),
                                    sizeof(*probe));
            entries = (Overlay68ResidentEntry *)func_800291C4();
            index = levelGetBlurEffect(kind);
            threshold = entries[index].thresholdNumerator / 5;
            if (threshold == 0) {
                threshold = 0x7080;
            }

            value = 0x8CA0;
            index = 0;
            while ((index < 4) && (probe->assetSizes[index] != 0)) {
                value = probe->values[index];
                index++;
            }

            if (threshold < value) {
                result = 1;
            }
        }
        mmFree(probe);
    }

    return result;
}
