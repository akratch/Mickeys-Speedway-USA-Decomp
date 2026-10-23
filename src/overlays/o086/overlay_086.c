#include "overlays/overlay_086.h"
#include "game/math.h"

void overlay86ProcessCurrent(Overlay86Object *owner) {
    Overlay86Current *current;

    current = *owner->currentSlot;
    if (current != NULL && current->resource != NULL && current->active != 0) {
        func_8005AF14(current, current->resource, owner);
        lightObject(owner, current, owner->context,
                             current->values[current->index]);
        current->active = 0;
    }
}

void overlay86ScaledVectorPosition(Overlay86Object *object,
                                   Overlay86RacerState *state, f32 *outX,
                                   f32 *outY, f32 *outZ) {
    s32 index;
    f32 vector[3];

    index = state->vectorIndex;

    if ((index < 0) || (index >= 10)) {
        index = 0;
    }

    vector[0] = gOverlay86Vectors[index].x * object->scale;
    vector[1] = gOverlay86Vectors[index].y * object->scale;
    vector[2] = gOverlay86Vectors[index].z * object->scale;
    pointListRPY(1, object, vector, vector);
    *outX = vector[0] + object->position.x;
    *outY = vector[1] + object->position.y;
    *outZ = vector[2] + object->position.z;
}

void overlay86BuildTransform(Overlay86Object *object,
                             Overlay86Object *transform) {
    s32 index;
    volatile Overlay86Vec3f target; /* load-bearing: without it the body is 4 words short */
    Overlay86Vec3f vector;
    Overlay86Current *current;
    s16 angle;

    index = ((Overlay86RacerState *)transform->state)->vectorIndex;
    if ((index < 0) || (index >= 10)) {
        index = 0;
    }
    current = *object->currentSlot;
    if ((current != 0) && (current->resource != 0) &&
        (current->resource->active != 0)) {
        overlay86ProcessCurrent_o086Reloc(object);
        target.x = current->position->x;
        target.y = current->position->y;
        target.z = current->position->z;
    } else {
        target.x = object->position.x;
        target.y = object->position.y;
        target.z = object->position.z;
    }
    transform->angle = object->angle + 0x8000;
    transform->tilt = 0;
    transform->roll = 0;
    vector.x = gOverlay86Vectors[index].x * transform->scale;
    vector.y = gOverlay86Vectors[index].y * transform->scale;
    vector.z = gOverlay86Vectors[index].z * transform->scale;
    pointListRPY(1, transform, &vector, &vector);
    transform->position.x = target.x - vector.x;
    transform->position.y = target.y - vector.y;
    transform->position.z = target.z - vector.z;
    angle = func_8000FAE0(transform->position.x,
                                          transform->position.y,
                                          transform->position.z);
    if (angle != -1) {
        transform->resultAngle = angle;
    }
}

/*
 * Near-miss exactness note: IDO assigns the five homed locals in reverse
 * declaration order here; keeping the owner comparison in node-first order
 * also preserves the target's two-load schedule.
 */
s16 overlay86SelectPosition(Overlay86Object *object, Overlay86State *output) {
    s32 start;
    s32 end;
    s32 selected;
    Overlay86Object **nodes;
    Overlay86Object *node;
    Overlay86RacerState *state;
    Overlay86RingRecord *record;

    state = object->state;
    record = overlay1FindPreviousUsable(state->selectedIndex, &selected);
    state->selectedIndex = overlay1NextIndex(selected);
    output->spawn.x = 0.0f;
    output->spawn.y = 0.0f;
    output->spawn.z = 0.0f;

    if (record != 0) {
        output->spawn.x = record->x;
        output->spawn.y = record->y + 96.0f;
        output->spawn.z = record->z;
        state->record = record;
        record->flags |= 8;
        return record->angle + 0x4000;
    }

    nodes = func_8000572C(&start, &end);
    while (start < end) {
        node = nodes[start++];
        if ((node->kind == 5) && (node->ownerIndex == state->ownerIndex)) {
            output->spawn.x = node->position.x;
            output->spawn.y = node->position.y + 96.0f;
            output->spawn.z = node->position.z;
            state->record = 0;
            return node->angle;
        }
    }
    return 0;
}

void overlay86Init(Overlay86Object *object, Overlay86Entry *entry) {
    Overlay86State *state = object->state;
    state->mode = 0;
    state->targetIndex = entry->type;
    state->loopSound = 0;
    state->effectSound = 0;
    object->resultAngle = -1;
    object->flags |= 0x400;
}

#define M2C_UNK s32
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))

/* Resident callees of func_overlay_086_F0000474_18D22AC. */
M2C_UNK amSndPlayXYZ(M2C_UNK, f32, f32, f32, s32, void *); /* extern */
M2C_UNK amSndSetVolXYZ(s32, s32);                     /* extern */
M2C_UNK amSndSetPitchXYZ(s32, s32);                     /* extern */
M2C_UNK controlPlayerReInit(void *, s32, s32, s32, s32, s32, s32); /* extern */
M2C_UNK amSndSetXYZ(s32, f32, f32, f32);           /* extern */
M2C_UNK amSndStopXYZ();                             /* extern */
M2C_UNK func_800291D8(M2C_UNK);                     /* extern */
s32 mathRnd(M2C_UNK, M2C_UNK);                /* extern */
M2C_UNK mathOneFloatPY(void *, void *);              /* extern */
void *func_80005820(u8);                             /* extern */
M2C_UNK func_800498FC(u8, M2C_UNK, f32, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005ABA8(void *, M2C_UNK, f32);        /* extern */
void func_8005AD64(void *, M2C_UNK, s32, f32);   /* extern */
M2C_UNK overlay7CommitSelection(M2C_UNK);                       /* extern */

/* Matched 2026-09-16 (lane lm-a), 662/662 words at frame 0xA8, unforced.
 * The fact the score does not show, measured on the allocator's own records
 * (docs/lastmile-region-boundary.md; the shard has the seven passes before):
 * a call's unused result is a v0 web that occupies the call's whole basic
 * block, so a pointer loaded after the call in that block is denied v0 (L101
 * is block-granular).  `func_8005AD64` is the last call before both the
 * case-0 `+0x48` access and the `+0x3E0` access, its result is never used,
 * and declaring it `void` removes the web: both loads then take v0 as the ROM
 * does.  An `if (1) { }` region boundary between the call and the load is the
 * byte-identical alternative.  Both accesses are written as the expression
 * itself rather than through the declared `result` carrier (L145), which
 * keeps `result` a head-only web on a0 by argument affinity.
 * Also load-bearing: the one-argument `amSndStopXYZ` calls (the ROM cannot
 * distinguish them from two-argument calls whose a1 already holds the
 * pointer, and the arity frees a1 for the command pointer), the split advance
 * `*cursor++ = 0xC; cursor += 5;`, the payload stores in memory order, and the
 * cursor definition folded onto the `0x2C` store line for as1's tie (L59).
 * Eleven declarations give the 0xA8 frame; a twelfth is 0xB0. */
void func_overlay_086_F0000474_18D22AC(void *object, s32 steps) {
    f32 elapsed;
    s32 shake;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 scale;
    s32 result;
    s16 *cursor;
    void *state;
    void *racer;
    void *target;

    elapsed = (f32) steps;
    state = M2C_FIELD(object, void **, 0x64);
    M2C_FIELD(state, s16 *, 0x28) = 0x2000;
    target = func_80005820(M2C_FIELD(state, u8 *, 1));
    if (target != NULL) {
        racer = M2C_FIELD(target, void **, 0x64);
        shake = 0;
        result = M2C_FIELD(state, u8 *, 0);
        if (result != 0) {
            func_8005ABA8(object, 0x3C88CE70, elapsed);
            M2C_FIELD(state, s16 *, 0x28) = 0x22;
            M2C_FIELD(state, s16 *, 0x2C) = 0x24; cursor = (s16 *)((u8 *)state + 0x30);
            M2C_FIELD(state, s16 *, 0x2A) = M2C_FIELD(state, s16 *, 0x1C);
            M2C_FIELD(state, s16 *, 0x2E) = M2C_FIELD(state, s16 *, 0x1C);
            /* Keep the cursor definition on the 0x2C store line for the as1 tie. */
            if (M2C_FIELD(state, s16 *, 0x24) != 0) {
                M2C_FIELD(state, s16 *, 0x26) = (s16) (M2C_FIELD(state, s16 *, 0x26) + (steps << 9));
                if (M2C_FIELD(state, u8 *, 0) != 2) {
                    M2C_FIELD(state, s16 *, 0x24) = (s16) (M2C_FIELD(state, s16 *, 0x24) - (steps * 0x11));
                    if (M2C_FIELD(state, s16 *, 0x24) < 0) {
                        M2C_FIELD(state, s16 *, 0x24) = 0;
                    }
                    result = M2C_FIELD(state, s32 *, 0x40);
                    if (result == 0) {
                        amSndPlayXYZ(0x1BE, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 1, (u8 *)state + 0x40);
                    } else if (result != 0) {
                        amSndSetXYZ(result, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    }
                } else {
                    result = M2C_FIELD(state, s32 *, 0x40);
                    if (result != 0) {
                        amSndStopXYZ(result);
                    }
                }
                shake = (s32) (func_8002A8C0(M2C_FIELD(state, s16 *, 0x26)) * 1024.0f);
                *cursor++ = 0xC;
                cursor += 5;
                M2C_FIELD(cursor, s16 *, -0xA) = (s16) M2C_FIELD(state, s16 *, 0x24);
                M2C_FIELD(cursor, s16 *, -8) = 3;
                M2C_FIELD(cursor, s16 *, -6) = (s16) (M2C_FIELD(state, s16 *, 0x24) * 2);
                M2C_FIELD(cursor, s16 *, -4) = 0xB;
                M2C_FIELD(cursor, s16 *, -2) = (s16) shake;
            } else {
                result = M2C_FIELD(state, s32 *, 0x40);
                if (result != 0) {
                    amSndStopXYZ(result);
                }
            }
            *cursor = 0x2000;
        }
        if (steps != 0) {
            scale = 0.05f;
            do {
                switch (M2C_FIELD(state, u8 *, 0)) {
                case 0:
                    if ((M2C_FIELD(racer, u8 *, 0x170) != 0) && !(M2C_FIELD(racer, u16 *, 0x1A8) & 1)) {
                        overlay86ScaledVectorPosition_o086Reloc(target, racer, &targetX, &targetY, &targetZ);
                        M2C_FIELD(object, s16 *, 0) = (s16) (M2C_FIELD(target, s16 *, 0) + 0x8000);
                        M2C_FIELD(object, s16 *, 2) = -0x1000;
                        M2C_FIELD(object, s16 *, 4) = 0;
                        M2C_FIELD(object, f32 *, 0xC) = targetX;
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (targetY + 320.0f);
                        M2C_FIELD(object, f32 *, 0x14) = targetZ;
                        M2C_FIELD(object, s16 *, 0x2E) = func_8000FAE0(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                        M2C_FIELD(state, s16 *, 2) = 0x3C;
                        M2C_FIELD(state, u8 *, 0) = 1U;
                        M2C_FIELD(state, f32 *, 0x10) = 0.0f;
                        M2C_FIELD(state, s16 *, 0x24) = 0;
                        M2C_FIELD(state, f32 *, 0x14) = 320.0f;
                        func_800498FC(M2C_FIELD(state, u8 *, 1), 0x3F4CCCCD, -1.0f, 0, 0, 0, 0);
                        func_800291D8(0x5A);
                        M2C_FIELD(object, s16 *, 6) = (s16) (M2C_FIELD(object, s16 *, 6) & 0xFBFF);
                        func_8005AD64(object, 1, -1, 0.0f);
                        M2C_FIELD(M2C_FIELD(target, void **, 0x48), u16 *, 6) &= 0xFFFE;
                    } else {
                        steps = 0;
                    }
                    break;
                case 1:
                    overlay86ScaledVectorPosition_o086Reloc(target, racer, &targetX, &targetY, &targetZ);
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_26:
                        M2C_FIELD(object, f32 *, 0xC) += (targetX - M2C_FIELD(object, f32 *, 0xC)) * 0.1f;
                        M2C_FIELD(object, f32 *, 0x14) += (targetZ - M2C_FIELD(object, f32 *, 0x14)) * 0.1f;
                        M2C_FIELD(state, f32 *, 0x10) = M2C_FIELD(state, f32 *, 0x14) * scale;
                        if (M2C_FIELD(state, f32 *, 0x10) > 10.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = 10.0f;
                        }
                        steps -= 1;
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        M2C_FIELD(state, f32 *, 0x14) = (f32) (M2C_FIELD(state, f32 *, 0x14) - M2C_FIELD(state, f32 *, 0x10));
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_26;
                        }
                    }
                    M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x14) + targetY);
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        M2C_FIELD(object, s16 *, 0) = (s16) (overlay86SelectPosition_o086Reloc(target, state) + 0x8000);
                        M2C_FIELD(object, s16 *, 2) = -0x1000;
                        M2C_FIELD(object, f32 *, 0xC) = (f32) M2C_FIELD(state, f32 *, 4);
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 8) + 400.0f);
                        M2C_FIELD(object, f32 *, 0x14) = (f32) M2C_FIELD(state, f32 *, 0xC);
                        overlay86BuildTransform_o086Reloc(object, target);
                        controlPlayerReInit(target, M2C_FIELD(target, s32 *, 0xC), M2C_FIELD(target, s32 *, 0x10), M2C_FIELD(target, s32 *, 0x14), (s32) M2C_FIELD(target, s16 *, 0), (s32) M2C_FIELD(target, s16 *, 2), (s32) M2C_FIELD(target, s16 *, 4));
                        M2C_FIELD(racer, s8 *, 0x191) = 1;
                        M2C_FIELD(racer, u8 *, 0x170) = 0U;
                        M2C_FIELD(racer, u16 *, 0x1A8) = (u16) (M2C_FIELD(racer, u16 *, 0x1A8) & 0xFFF7);
                        M2C_FIELD(state, u8 *, 0) = 2U;
                        M2C_FIELD(state, s16 *, 0x24) = 0x400;
                        M2C_FIELD(state, s16 *, 2) = 0x5A;
                        func_800498FC(M2C_FIELD(state, u8 *, 1), 0x3EB33333, 0.0f, 0, 0, 0, 0x80);
                        overlay7CommitSelection(9);
                    }
                    result = func_8000FAE0(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    if (result != -1) {
                        M2C_FIELD(object, s16 *, 0x2E) = result;
                    }
                    break;
                case 2:
                case 3:
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_37:
                        M2C_FIELD(state, f32 *, 0x10) = (f32) ((M2C_FIELD(state, f32 *, 8) - M2C_FIELD(object, f32 *, 0x10)) * scale);
                        if (M2C_FIELD(state, f32 *, 0x10) < -6.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = -6.0f;
                        }
                        steps -= 1;
                        M2C_FIELD(object, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x10) + M2C_FIELD(object, f32 *, 0x10));
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_37;
                        }
                    }
                    overlay86BuildTransform_o086Reloc(object, target);
                    M2C_FIELD(target, s16 *, 4) = (s16) shake;
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        if (M2C_FIELD(state, u8 *, 0) == 2) {
                            M2C_FIELD(state, s16 *, 2) = 0x1E;
                            M2C_FIELD(state, u8 *, 0) = 3U;
                        } else {
                            amSndPlayXYZ(0x1BF, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 4, NULL);
                            M2C_FIELD(racer, s8 *, 0x191) = 0;
                            M2C_FIELD(racer, s8 *, 0x16C) = 1;
                            func_8005AD64(target, 0xC, -1, 0.0f);
                            M2C_FIELD(state, s16 *, 0x1E) = 0;
                            M2C_FIELD(state, s16 *, 0x20) = 0;
                            M2C_FIELD(state, s16 *, 0x22) = 0;
                            M2C_FIELD(state, s16 *, 2) = 0xB4;
                            M2C_FIELD(state, u8 *, 0) = 4U;
                            M2C_FIELD(state, f32 *, 0x10) = 0;
                            if (M2C_FIELD(racer, void **, 0x3E0) != NULL) {
                                M2C_FIELD(M2C_FIELD(racer, void **, 0x3E0), u16 *, 0x10) &= 0xFFF7;
                                M2C_FIELD(racer, void **, 0x3E0) = NULL;
                            }
                            M2C_FIELD(M2C_FIELD(target, void **, 0x48), u16 *, 6) |= 1;
                        }
                    }
                    result = func_8000FAE0(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                    if (result != -1) {
                        M2C_FIELD(object, s16 *, 0x2E) = result;
                    }
                    break;
                case 4:
                    if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
loop_52:
                        M2C_FIELD(state, f32 *, 0x10) = (f32) (M2C_FIELD(state, f32 *, 0x10) + scale);
                        M2C_FIELD(object, s16 *, 0) = (s16) (M2C_FIELD(state, s16 *, 0x1E) + M2C_FIELD(object, s16 *, 0));
                        M2C_FIELD(object, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 0x20) + M2C_FIELD(object, s16 *, 2));
                        M2C_FIELD(object, s16 *, 4) = (s16) (M2C_FIELD(state, s16 *, 0x22) + M2C_FIELD(object, s16 *, 4));
                        if (M2C_FIELD(state, s16 *, 0x1E) >= -0x3F) {
                            M2C_FIELD(state, s16 *, 0x1E) -= 2;
                        }
                        if (M2C_FIELD(state, s16 *, 0x20) < 0x80) {
                            M2C_FIELD(state, s16 *, 0x20) += 8;
                        }
                        if (M2C_FIELD(state, s16 *, 0x22) >= -0x7F) {
                            M2C_FIELD(state, s16 *, 0x22) -= 8;
                        }
                        steps -= 1;
                        M2C_FIELD(state, s16 *, 2) = (s16) (M2C_FIELD(state, s16 *, 2) - 1);
                        if ((M2C_FIELD(state, s16 *, 2) != 0) && (steps != 0)) {
                            goto loop_52;
                        }
                    }
                    if (M2C_FIELD(state, s16 *, 2) == 0) {
                        M2C_FIELD(object, s16 *, 0x2E) = -1;
                        M2C_FIELD(object, s16 *, 6) = (s16) (M2C_FIELD(object, s16 *, 6) | 0x400);
                        M2C_FIELD(state, u8 *, 0) = 0U;
                    } else {
                        if (M2C_FIELD(state, f32 *, 0x10) > 5.0f) {
                            M2C_FIELD(state, f32 *, 0x10) = 5.0f;
                        }
                        if (M2C_FIELD(object, s16 *, 2) >= 0x4001) {
                            M2C_FIELD(object, s16 *, 2) = 0x4000;
                        }
                        if (M2C_FIELD(object, s16 *, 4) < -0x2000) {
                            M2C_FIELD(object, s16 *, 4) = -0x2000;
                        }
                        M2C_FIELD(object, f32 *, 0x24) = (f32) (-M2C_FIELD(state, f32 *, 0x10) * elapsed);
                        mathOneFloatPY(object, (u8 *)object + 0x1C);
                        M2C_FIELD(object, f32 *, 0xC) += M2C_FIELD(object, f32 *, 0x1C) * elapsed;
                        M2C_FIELD(object, f32 *, 0x10) += M2C_FIELD(object, f32 *, 0x20) * elapsed;
                        M2C_FIELD(object, f32 *, 0x14) += M2C_FIELD(object, f32 *, 0x24) * elapsed;
                        result = func_8000FAE0(M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                        if (result != -1) {
                            M2C_FIELD(object, s16 *, 0x2E) = result;
                        }
                    }
                    break;
                }
            } while (steps != 0);
        }
        if (M2C_FIELD(state, u8 *, 0) != 0) {
            steps = M2C_FIELD(state, s32 *, 0x18);
            if (steps == 0) {
                amSndPlayXYZ(0x16, M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14), 1, (u8 *)state + 0x18);
                steps = M2C_FIELD(state, s32 *, 0x18);
                if (steps != 0) {
                    amSndSetVolXYZ(steps, 0x7F);
                }
            }
            steps = M2C_FIELD(state, s32 *, 0x18);
            if (steps != 0) {
                scale = M2C_FIELD(state, f32 *, 0x10) * 8.0f;
                if (scale < 0.0f) {
                    scale = -scale;
                }
                scale += 100.0f;
                if (scale > 150.0f) {
                    scale = 150.0f;
                }
                scale += (f32) mathRnd(-5, 5);
                amSndSetXYZ(M2C_FIELD(state, s32 *, 0x18), M2C_FIELD(object, f32 *, 0xC), M2C_FIELD(object, f32 *, 0x10), M2C_FIELD(object, f32 *, 0x14));
                amSndSetPitchXYZ(M2C_FIELD(state, s32 *, 0x18), (u32) scale & 0xFF);
                if (M2C_FIELD(state, u8 *, 0) == 4) {
                    amSndSetVolXYZ(M2C_FIELD(state, s32 *, 0x18), ((s32) (M2C_FIELD(state, s16 *, 2) * 0x7F) / 180) & 0xFF);
                }
            }
            shake = (s32) (M2C_FIELD(state, f32 *, 0x10) * 256.0f);
            if (shake < 0) {
                shake = -shake;
            }
            shake += 0x800;
            if (shake >= 0x1001) {
                shake = 0x1000;
            }
            M2C_FIELD(state, s16 *, 0x1C) = (s16) (M2C_FIELD(state, s16 *, 0x1C) + (shake * (s32) elapsed));
            return;
        }
        steps = M2C_FIELD(state, s32 *, 0x18);
        if (steps != 0) {
            amSndStopXYZ(steps);
        }
    }
}
