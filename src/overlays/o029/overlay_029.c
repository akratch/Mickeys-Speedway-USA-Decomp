#include "overlays/overlay_029.h"

/*
 * Overlay 29: one translation unit in ROM order (ADR 0006). The ORT exports
 * the initializer (+0x42C), the update (+0x5C4), the effect handler
 * (+0x10C4) and the group renderer (+0x14C8). The table selector, both
 * rotations, the chain builder, the ratio update and the sampler are static;
 * the shipped JUMP calls to them follow from that. The plane projection
 * (+0xEE0) is reached only through its stored address, so it stays extern:
 * IDO drops a static function nothing in the TU names.
 */

static void overlay29Select(s32 index);
static void overlay29RotateForward(s32 count);
static void overlay29RotateBackward(s32 count);
static void overlay29BuildChain(Overlay29ChainOwner *owner);
static void overlay29UpdateRatio(Overlay29RatioObject *object,
                                 Overlay29Ratio *ratio);
static void overlay29Sample(Overlay29SampleObject *object, f32 *out0,
                            f32 *out1, f32 *out2, f32 advance);

/* The four parallel Mickey tables have no DKR v77/v80 or JFG donor. */
static void overlay29Select(s32 index) {
    s32 offset;

    if (index != 666) {
        gOverlay29Current = index;
    }
    offset = gOverlay29Current << 4; gOverlay29Selected0 = (Overlay29PathPoint *)((u8 *)gOverlay29Base0 + offset + 0x14);
    gOverlay29Selected1 = (Overlay29PathPoint *)((u8 *)gOverlay29Base1 + offset + 0x14);
    gOverlay29Selected2 = (Overlay29PathPoint *)((u8 *)gOverlay29Base2 + offset + 0x14);
    gOverlay29Selected3 = (Overlay29PathPoint *)((u8 *)gOverlay29Base3 + offset + 0x14);
}

/* This title-specific four-value rotation has no DKR v77/v80 or JFG donor. */
static void overlay29RotateForward(s32 count) {
    while (count--) {
        gOverlay29Value0 = gOverlay29Value1;
        gOverlay29Value1 = gOverlay29Value2;
        gOverlay29Value2 = gOverlay29Value3;
        gOverlay29Value3 = overlay29AdvanceReloc(gOverlay29Value3);
    }
    overlay29AdvanceReloc(666);
}

/* This title-specific four-value rotation has no DKR v77/v80 or JFG donor. */
static void overlay29RotateBackward(s32 count) {
    while (count--) {
        gOverlay29Value3 = gOverlay29Value2;
        gOverlay29Value2 = gOverlay29Value1;
        gOverlay29Value1 = gOverlay29Value0;
        gOverlay29Value0 = overlay29AdvanceReloc(gOverlay29Value0);
    }
    overlay29AdvanceReloc(666);
}

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this call chain. */

static void overlay29BuildChain(Overlay29ChainOwner *owner) {
    Overlay29ChainResource *resource;

    resource = owner->resource;
    gOverlay29Node8 = overlay29CreateReloc(resource->first);
    gOverlay29Node4 = overlay29CreateReloc(gOverlay29Node8);
    gOverlay29Node0 = overlay29CreateReloc(gOverlay29Node4);
    gOverlay29NodeC = overlay29CreateReloc(gOverlay29Node8);
    overlay29CreateReloc(resource->last);
}

/* Exact at +0x23C; DKR v77/v80 and JFG have no exact donor for this update. */
static void overlay29UpdateRatio(Overlay29RatioObject *object, Overlay29Ratio *ratio) {
    f32 dx;
    f32 dy;
    f32 referenceDistance;
    f32 objectDistance;

    dx = gOverlay29PointA->x - gOverlay29PointB->x;
    dy = gOverlay29PointA->y - gOverlay29PointB->y;
    referenceDistance = sqrtf((dx * dx) + (dy * dy));

    dx = gOverlay29PointA->x - object->x;
    dy = gOverlay29PointA->y - object->y;
    objectDistance = sqrtf((dx * dx) + (dy * dy));

    ratio->ratio = objectDistance / referenceDistance;
    ratio->value = (f32)overlay29AngleReloc(ratio->angle) + ratio->ratio;
}

/* Exact at +0x304; DKR v77/v80 and JFG have no exact donor for this sampler. */
static void overlay29Sample(Overlay29SampleObject *object, f32 *out0, f32 *out1,
                     f32 *out2, f32 advance) {
    f32 frame;
    f32 fraction;
    s32 whole;

    frame = object->frame + advance;
    whole = (s32)frame;
    overlay29RotateForward(whole);
    fraction = frame - (f32)whole;

    *out0 = splinePos(
        gOverlay29Selected0->x, gOverlay29Selected1->x,
        gOverlay29Selected2->x, gOverlay29Selected3->x, fraction);
    *out1 = splinePos(
        gOverlay29Base0->y, gOverlay29Base1->y,
        gOverlay29Base2->y, gOverlay29Base3->y, fraction);
    *out2 = splinePos(
        gOverlay29Selected0->y, gOverlay29Selected1->y,
        gOverlay29Selected2->y, gOverlay29Selected3->y, fraction);
}

/* Matched 2026-09-12, lane p9-tight. Fifteen words to zero, two edits.
 *
 * 1. The `object` field was `volatile`, which gives the reloads AND as1's
 *    scheduling edges [L144]; the target needs only the reloads. Taking the
 *    address at each of the four reads keeps all four loads -- the count is
 *    observable, the ROM has four -- and drops the edges, and the entry block
 *    then collapses from a mixed structure/schedule/register residual (15) to
 *    a pure schedule one (9, category schedule-only, the instruction multiset
 *    already exact). Every prior probe on this function held the `volatile`
 *    fixed and asked which statement order would reach the target's stream;
 *    with the edges in place no order can, which is why eight positions of the
 *    source assignment measured 15 to 22 and never below.
 * 2. With the edges gone the order is the lever, and it is the natural one:
 *    the two pointer producers first, then position x/y/z, then offset x/y/z.
 *    Measured over all 1,120 interleavings that keep position and offset each
 *    in x/y/z order, with the contact and source assignments at every
 *    position: exactly two score 0, this one and the mirror with `source`
 *    ahead of `contact`; the next best is 2 and the median is 12.
 *
 * The prior closure's named decision variable -- uopt's web-creation order in
 * the entry block -- was the wrong level. The block is one basic block, so
 * every one of those forms produced the same webs; what differed was as1's
 * scheduling, and the volatile edges were holding it fixed.
 */
void func_overlay_029_F000042C_187D6DC(Overlay29Object *object,
                                       Overlay29Init *init) {
    Overlay29Contact *contact;
    Overlay29SourceState *source;
    Vec3f position;
    Vec3f offset;
    f32 distance;

    contact = object->contact;
    source = (Overlay29SourceState *)(*(Overlay29Object **)&init->object)->contact;
    position.x = object->position.x;
    position.y = object->position.y;
    position.z = object->position.z;
    offset.x = 0.0f;
    offset.y = 0.0f;
    offset.z = -60.0f;

    mathOneFloatRPY((*(Overlay29Object **)&init->object), &offset);
    object->position.x += offset.x;
    object->position.y += offset.y;
    object->position.z += offset.z;

    distance = 8.0f;
    trackMakePolylist(1, &position, &object->position, &distance, NULL, 0);
    if (func_80010900(&position, &object->position, distance, object, func_overlay_029_F0000EE0_o029Reloc) != 0) {
        func_overlay_029_F00010C4_187E374Reloc(object, 5);
        return;
    }

    object->entity->flags |= 2;
    object->entity->owner = (*(Overlay29Object **)&init->object);
    object->angle0 = init->angle0;
    object->angle1 = init->angle1;
    object->angle2 = 0;
    object->active = 1;
    contact->owner = (*(Overlay29Object **)&init->object);
    contact->recordIndex = source->recordIndex;
    contact->selector0 = source->selector0;
    contact->selector1 = source->selector1;
    contact->selector2 = source->selector2;
    contact->flags = source->flags & 8;
    overlay29BuildChain((Overlay29ChainOwner *)object);
    overlay29UpdateRatio((Overlay29RatioObject *)object, (Overlay29Ratio *)contact);
}

/* Matched 2026-10-02 (lane w2-ovlc), rewritten from the listing (360 before).
 * The four record updates go through x/y/z locals in the order y, vel.y, x,
 * z, the three position adds, then the angles, with the gravity literals
 * inline: the three locals tie on save and take f0/f2/f12 in first-use
 * order, and updateRateF, the acceleration, the step and zero land in
 * f20/f16/f18/f22. Compares against int 0 keep the float zero out of a
 * callee-saved web (no f26). The heading deltas are s16; the selection is an
 * if/else; the final distance reads linkedEntity, linkedState and entity
 * locals, and reusing linkedEntity there is what spills its first range
 * through 0x88. The unused locals are frame cells in the target's order. */
void func_overlay_029_F00005C4_187D874(Overlay29TailObject *object,
                                        s32 updateRate) {
    f32 updateRateF;
    Overlay29TailState *state;
    Overlay29TailRecord *record;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 radius;
    Overlay29TailVec3f oldPosition;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    Overlay29TailLinkedObject *linked;
    Overlay29TailEntity *linkedEntity;
    s32 remaining;
    f32 horizontalDistance;
    s16 targetAngle;
    s16 delta;
    s32 selection;
    s32 i;
    s32 useLinkedPosition;
    f32 x;
    f32 y;
    f32 z;
    Overlay29TailLinkedState *linkedState;
    Overlay29TailEntity *entity;
    s32 unused;

    updateRateF = updateRate;
    state = object->state;
    overlay29BuildChain((Overlay29ChainOwner *)object);

    if (object->entity->flags & 2) {
        object->entity->owner = 0;
        object->entity->flags &= ~2;
    }
    if ((state->recordsActive == 0) && (state->pendingEffect != 0)) {
        func_overlay_029_F00010C4_187E374Reloc(object, 0xE);
        state->pendingEffect = 0;
    }

    if (state->recordsActive != 0) {
        record = &state->records[0];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
            }
        }
        record = &state->records[1];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
            }
        }
        record = &state->records[2];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
            }
        }
        record = &state->records[3];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
            }
        }
        if (state->recordsFinished == 4) {
            func_80006EA0(object);
        }
        return;
    }

    partUpdateTriggers(object, updateRate);
    if (state->skipTargeting != 0) {
        linked = NULL;
        useLinkedPosition = 0;
    } else {
        if (overlay1TestDirection(gOverlay29Base2, object->position.x,
                                              object->position.z) != 0) {
            overlay29RotateForward(1);
            state->selection0 = gOverlay29Base2->selection;
            state->selection1 = gOverlay29Base1->selection;
        } else if (overlay1TestDirection(
                       gOverlay29Base1, object->position.x, object->position.z) == 0) {
            overlay29RotateBackward(1);
            state->selection0 = gOverlay29Base2->selection;
            state->selection1 = gOverlay29Base2->selection;
        }
        overlay29UpdateRatio((Overlay29RatioObject *)object, (Overlay29Ratio *)state);
        linked = overlay1FindPreviousAngle(state->referenceAngle);
        if (linked == (Overlay29TailLinkedObject *)state->owner) {
            linked = overlay1FindPreviousAngle(linked->state->angle);
        }
        if (linked != NULL) {
            linkedEntity = linked->entity;
            dx = object->position.x - linkedEntity->position.x;
            dy = object->position.y - linkedEntity->position.y;
            dz = object->position.z - linkedEntity->position.z;
            distance = sqrtf(dx * dx + dy * dy + dz * dz);
        } else {
            distance = 3.40282347e+38f;
        }
        useLinkedPosition = 0;
        if (distance < 750.0f) {
            targetX = linkedEntity->position.x;
            targetY = linkedEntity->position.y;
            targetZ = linkedEntity->position.z;
            useLinkedPosition = 1;
        } else {
            if (linked != NULL) {
                selection = linked->state->pad001[0x37D];
            } else {
                selection = 3;
            }
            overlay29Select(selection);
            overlay29Sample((Overlay29SampleObject *)state, &targetX, &targetY,
                                              &targetZ, 0.4f);
            targetY += 130.0f;
        }

        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                dx = object->position.x - targetX;
                dz = object->position.z - targetZ;
                if (dz == 0) {
                    targetAngle = object->angle0;
                } else {
                    targetAngle = Arctanf(dx, dz);
                }
                delta = mathDiffAngle(object->angle0, targetAngle) >> 3;
                if (delta > 0x384) {
                    delta = 0x384;
                }
                if (delta < -0x384) {
                    delta = -0x384;
                }
                object->angle0 += delta;
                object->angularVelocity0 = delta * 30;

                dy = object->position.y - targetY;
                horizontalDistance = sqrtf(dx * dx + dz * dz);
                if (horizontalDistance == 0) {
                    targetAngle = object->angle1;
                } else {
                    targetAngle = -Arctanf(dy, horizontalDistance);
                }
                delta = mathDiffAngle(object->angle1, targetAngle) >> 3;
                if (delta > 0x384) {
                    delta = 0x384;
                }
                if (delta < -0x384) {
                    delta = -0x384;
                }
                object->angle1 += delta;
            } while (remaining--);
        }
    }

    object->velocity.x = 0.0f;
    object->velocity.y = 0.0f;
    object->velocity.z = -30.0f;
    mathOneFloatPY(object, &object->velocity);
    oldPosition.x = object->position.x;
    oldPosition.y = object->position.y;
    oldPosition.z = object->position.z;
    func_80008128(object, object->velocity.x * updateRateF,
                  object->velocity.y * updateRateF,
                  object->velocity.z * updateRateF);

    state->collisionFlags = 0;
    radius = 8.0f;
    trackMakePolylist(1, &oldPosition, &object->position, &radius, NULL, 0);
    if ((func_80010900(&oldPosition, &object->position, radius, object,
                       func_overlay_029_F0000EE0_o029Reloc) != 0) &&
        (state->collisionFlags & 4)) {
        state->pendingEffect = 1;
    }

    if ((useLinkedPosition != 0) && (linked != NULL)) {
        entity = object->entity;
        linkedEntity = linked->entity;
        linkedState = linked->state;
        dx = entity->position.x - linkedEntity->position.x;
        dy = entity->position.y - linkedEntity->position.y;
        dz = entity->position.z - linkedEntity->position.z;
        distance = sqrtf(dx * dx + dy * dy + dz * dz);
        overlay37RecordMinimum(linkedState->index, distance);
    }
}

/* Exact configured IDO output: 121 words, a 0x68 frame, and all three
 * static relocation identities. The linked owned range and full US ROM
 * are byte-identical.
 * Keep the apparently redundant single-precision multiply by 1.0f:
 * in the proved context it changes one product's emitted operand order
 * without adding an instruction. Its optimization-pass mechanism is
 * untraced; this is not a general floating-point simplification rule.
 * Retain this spelling until a naturalized form repeats the full proof. */
void func_overlay_029_F0000EE0_187E190(s32 unused, Vec3f *output, Vec3f *axis, f32 height, Overlay29Transform *transform, Overlay29ProjectionOwner *owner)
{
  Overlay29PathState *state;
  f32 dirX;
  f32 dirY;
  f32 dirZ;
  f32 projectedX;
  f32 projectedY;
  f32 projectedZ;
  f32 lengthSquared;
  f32 crossX;
  f32 crossY;
  f32 crossZ;
  dirY = transform->direction.y;
  dirX = transform->direction.x;
  dirZ = transform->direction.z;
  state = owner->state;
  if (((*((f32 *) (gOverlay29MinimumYReloc + 0x14))) <= dirY) || ((transform->flags & 0x10000000) != 0))
  {
    crossX = (dirY * axis->z) - ((lengthSquared = axis->y) * dirZ);
    crossY = ((dirZ * 1.0f) * axis->x) - (axis->z * dirX);
    crossZ = (dirX * lengthSquared) - (axis->x * dirY);
    projectedX = (crossY * dirZ) - (crossZ * dirY);
    projectedY = (crossZ * dirX) - (crossX * dirZ);
    projectedZ = (crossX * dirY) - (crossY * dirX);
    lengthSquared = ((projectedX * projectedX) + (projectedY * projectedY)) + (projectedZ * projectedZ);
    if (lengthSquared > 0.0f)
    {
      lengthSquared = sqrtf(lengthSquared);
      projectedY /= lengthSquared;
      projectedX /= lengthSquared;
      projectedZ /= lengthSquared;
      lengthSquared = height - transform->height;
      output->x = transform->position.x + (lengthSquared * projectedX);
      output->y = transform->position.y + (lengthSquared * projectedY);
      output->z = transform->position.z + (lengthSquared * projectedZ);
    }
    else
    {
      output->x = transform->position.x;
      output->y = transform->position.y;
      output->z = transform->position.z;
    }
    state->flags |= 2;
  }
  else
  {
    output->x = transform->position.x;
    output->y = transform->position.y;
    output->z = transform->position.z;
    state->direction.x = dirX;
    state->direction.y = dirY;
    state->direction.z = dirZ;
    state->flags |= 4;
  }
}

#define INITIALIZE_RECORD(rec_) \
    do { \
        record->position.x = object->position.x; \
        record->position.y = object->position.y; \
        record->position.z = object->position.z; \
        record->angle0 = object->angle0; \
        record->angle1 = object->angle1; \
        record->angle2 = object->angle2; \
        record->scalar = object->scalar; \
        record->random0 = mathRnd(-0x500, 0x500);       \
        record->random1 = mathRnd(-0x500, 0x500);       \
        *(volatile s16 *) &record->random2 =                              \
            mathRnd(-0x500, 0x500);                     \
        *(volatile s16 *) &record->value = 0xFF;                           \
    } while (0)

/* The second block's angle setup belongs to the first block's initialiser
 * line, not to its own.  Emitting it there keeps `record` -- the first
 * block's pool web, colour v1 -- live across the shared
 * `verticalAngle + 0x2000` CSE, and the CSE moves off v0 onto a2, which is
 * what the target does.  The macro must spell the body out rather than
 * wrap INITIALIZE_RECORD: a nested expansion resynchronises acpp's line
 * counter and puts the two statements back on their own line (17 words). */
#define INITIALIZE_RECORD_THEN(rec_, next_)                                \
    do {                                                                   \
        record->position.x = object->position.x;                           \
        record->position.y = object->position.y;                           \
        record->position.z = object->position.z;                           \
        record->angle0 = object->angle0;                                   \
        record->angle1 = object->angle1;                                   \
        record->angle2 = object->angle2;                                   \
        record->scalar = object->scalar;                                   \
        record->random0 = mathRnd(-0x500, 0x500);        \
        record->random1 = mathRnd(-0x500, 0x500);        \
        *(volatile s16 *) &record->random2 =                               \
            mathRnd(-0x500, 0x500);                      \
        *(volatile s16 *) &record->value = 0xFF;                           \
        next_;                                                             \
    } while (0)

/* Exact size (1028 bytes / 257 words), exact 0x48 frame, all 22 relocation
 * offsets and types.  253 of 257 words agree and the four that do not are one
 * fact: the CSE web for `verticalAngle + 0x2000` -- defined in the second
 * block, spilled to the compiler temp at sp+0x2C and reloaded by the third and
 * fourth -- takes `v0` here and `a2` in the target.  The four words are
 * +0x13C, +0x148, +0x150 and +0x164, which is exactly that web's load, add,
 * store and spill; its temp slot, its schedule, its spill point and every
 * other register in the function agree.  ugen emits the register itself
 * (`addu $2, $2, 8192` in its listing), so this is ugen's allocation, not a
 * scheduling artefact of as1.
 *
 * The declaration list is closed on both sides.  A sixth local costs 22 words
 * at best in any of six positions and any of seven types -- the frame law is
 * frame = align8(below + S) with below = 0x38 and S = 0x10, so 0x48 admits
 * exactly these five.  The obvious composition -- drop `state` to make room,
 * then name the shared value -- does not work either: the named carrier is
 * free, but spelling `object->state` at its six uses is nine words SHORTER
 * (248/257), so `state` is load-bearing as a local and the list cannot be
 * traded.  That closes the L88 "name the value to add a pool colour" route.
 *
 * The double mask in the first block is load-bearing: a single `& 0xFFFFU`, a
 * `(u16)` cast or no mask at all is 72 words, first difference +0x30.  It
 * costs no instruction (ugen emits a bare `addu $t3, $2, 12288`), so it is a
 * pure web-structure edit.  Both `volatile` casts in INITIALIZE_RECORD are
 * load-bearing at eight words each.
 *
 * What is exhausted, all flat at four words.  Earlier lanes: statement order
 * and record placement across all four blocks (3,888); physical line grouping
 * within and across blocks (1,203); mask spelling per block (1,110); macro
 * body order (576); the record-pointer spelling lattice (256); local and
 * prototype types (163); inner-block scope placement and declaration
 * permutation (136); `register` hints and a sixth local (50); naming the
 * shared value (48); signature return type and extra parameters (12); the
 * driver flag lattice (13 -- `-Wab,-r4300_mul` confirmed).  Added here: all
 * 4,096 identity-operation forms of the three `verticalAngle + 0x2000` sites
 * (`^ 0`, `| 0`, `+ 0`, `- 0`, `* 1`, `-(-x)`, `(s16)`, `(u16)`, single and
 * double mask, reversed operands), of which 1,868 keep the size and every one
 * scores four; all 721 statement orders of block 2; the `angles` array
 * respelled as a two-field struct with either field order and three argument
 * spellings; a 96-cell prototype lattice over the four relocated callees'
 * return and parameter types; and per-phase optimisation levels through
 * `ido-phases.py` (uopt/ugen/as1 at -O3, uopt at -O1).
 *
 * The one genuinely new fact is a reachability bound.  A 4,000-point
 * randomised cross of macro body order, both `volatile` casts, and the mask
 * spelling of all five angle expressions moves this carrier to `v0` (2,745
 * cells), `v1` (603), `t6` (250), `t7` (177), `t5` (107), `t8` (82) and `t9`
 * (29) -- and never once to `a0`-`a3`, at any score.  The 96-cell prototype
 * lattice and the 721 statement orders are also uniformly `v0`.  So the
 * target's `a2` is not in this candidate's reachable allocation set: the next
 * lever has to change the allocation regime, not the spelling or the
 * schedule.  Note the permuter is not usable here -- its scratch scores this
 * body 135 against a measured 4. */
void func_overlay_029_F00010C4_187E374(Overlay29EffectObject *object, s32 mode) {
    Overlay29Record *record;
    Overlay29State *state;
    s16 angles[2];
    s16 baseAngle;
    s16 verticalAngle;

    state = object->state;
    if ((mode & 1) != 0) {
        func_80006EA0();
    } else if ((mode & 2) != 0) {
        baseAngle = Arctanf(state->direction.x, state->direction.z);
        verticalAngle = Arctanf(
            sqrtf((state->direction.z * state->direction.z) +
                               (state->direction.x * state->direction.x)),
            state->direction.y);

        record = &state->records[0];
        angles[0] = baseAngle;
        angles[1] = ((verticalAngle + 0x3000) & 0xFFFFU) & 0xFFFFU;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD_THEN(record, angles[1] = verticalAngle + 0x2000;
                                       angles[0] = baseAngle);

        record = &state->records[1];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);

        record = &state->records[2];
        angles[0] = baseAngle - 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);

        record = &state->records[3];
        angles[0] = baseAngle + 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        mathOneFloatPY(angles, &record->vector);
        INITIALIZE_RECORD(record);
        state->active = 1;
    }

    if ((mode & 4) != 0) {
        object->flags80 |= 2;
        partUpdateTriggers(object, 1);
    }
    if ((mode & 8) != 0) {
        amSndPlayXYZ(0x27A, object->position.x, object->position.y,
                           object->position.z, 4, 0);
    }
    object->resource->flags &= ~1;
    object->flags06 |= 0x400;
}

/* PROVENANCE: command expansions use the existing SDK macros in
 * n_audio/mbi.h. Node layout, indexing, call order and command words are
 * reconstructed from Mickey's own overlay. */
void overlay29DrawGroups(Gfx **dl, void *drawContext,
                         Overlay29Context *context) {
    Overlay29RenderState *render;
    Overlay29Group *group;
    Overlay29Node *node;
    Overlay29ResourceChoice *choice;
    Gfx *gfx;
    u32 resourceSegment;
    s32 groupIndex;

    render = context->render;
    if (render->enabled != 0) {
        groupIndex = 4;
        gDPPipeSync((*dl)++);
        gDPSetEnvColor((*dl)++, 0xFF, 0xFF, 0xFF, 0);

        while (groupIndex--) {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else if (groupIndex == 1) {
                group = &render->group1;
            } else {
                group = &render->group0;
            }

            node = context->nodeTable[4 - groupIndex];
            choice = (Overlay29ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, group->selector);
            camPushModelMtx(dl, drawContext, group, 1.0f, 0.0f);

            gfx = (*dl)++;
            gfx->words.w0 = 0xBF000000;
            gfx->words.w1 = node->segment + 0x80000000;
            gSPDisplayList((*dl)++, resourceSegment + 0x80000000);
            gSP1Triangle((*dl)++, 0, 0, 0, 0);
            camRestoreModelMtx(dl);

            node->consumed = 0;
        }

        texDPInit(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
