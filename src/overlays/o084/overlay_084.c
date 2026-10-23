#include "overlays/overlay_084.h"

static void func_overlay_084_F0000314_18D07F4(Overlay84Object *object,
                                              Overlay84State *state,
                                              s32 updateRate);
static void overlay84UpdateResource(void *unused, Overlay84State *state, s32 value);
static void overlay84RefreshCurrent(void *unused, Overlay84State *state, s32 unused2);

/* State bootstrap; pinned DKR objects provide no exact donor. */
void overlay84InitState(Overlay84Object *object, void *entry) {
    Overlay84State *state = object->state;

    state->initialized = 0;
    state->current = 0;
    state->last = -1;
    state->inputLatch = 0;
    state->inputTimer = 20;
    gOverlay84Object = object;
    state->first = 255;
    state->second = 255;
    state->active = 1;
    state->marked = 0;
}

/* `start` and `end` are declared after `node` so their automatic homes land
 * at sp+0x44/sp+0x40: IDO 5.3 hands out homes descending from the top of the
 * frame in declaration order, and with them declared first they sat two words
 * too high. `scratch` is the function's eight-byte aggregate automatic, kept
 * at the end of the list so it stays below the temp region and holds the
 * frame at 0x58. */
void overlay84InitializeAndUpdate(Overlay84Object *object, s32 arg) {
    s32 i;
    Overlay84Node *initialNode;
    Overlay84Node **nodes;
    Overlay84Node *node;
    s32 start;
    s32 end;
    s16 tilt;
    Overlay84State *state;
    s32 scratch[2];

    (void)&scratch;

    state = object->state;
    if (state->initialized == 0) {
        nodes = func_8000572C(&start, &end);
        for (i = start; i < end; i++) {
            node = nodes[i];
            if (node->kind == 0x33) {
                s32 slot;

                slot = node->choice->slot;
                if (state->last < slot) {
                    state->last = slot;
                }
                state->nodes[slot] = node;
            }
        }

        initialNode = state->nodes[0];
        if (initialNode != 0) {
            state->targetAngle = Arctanf(initialNode->x - object->x,
                                          initialNode->z - object->z);
            tilt = -initialNode->tilt;
            state->tilt = tilt;
            state->targetTilt = tilt;
            state->outputAngle.word = initialNode->angle,
            state->targetHeight = state->height = initialNode->y;
            state->blendTimer = 0;
            state->action = 0;
            state->blend = 1.0f;
            state->x = initialNode->x;
            state->y = initialNode->y;
            state->z = initialNode->z;
        }
        state->initialized = 1;
        state->mode = 0;
    }

    switch (state->mode) {
    case 1:
        overlay84RefreshCurrent(object, state, arg);
        break;
    case 2:
        func_overlay_084_F0000314_18D07F4(object, state, arg);
        break;
    case 3:
        overlay84UpdateResource(object, state, arg);
        break;
    }
    state->active = 1;
}

/* The blendTimer decrement carries the author's `& 0xFF`: IDO folds the mask
 * into the byte store, but the spent temp draw is what puts the subtract and
 * everything after it on the shipped $t registers (LANE_BRIEF L149, checklist
 * 16). -Wab,-r4300_mul is applied for this object like its siblings. */
static void func_overlay_084_F0000314_18D07F4(Overlay84Object *object,
                                              Overlay84State *state,
                                              s32 updateRate) {
    Overlay84Node *node;
    s32 i;
    Overlay84Choice *choice;
    Overlay84Output *output;
    s32 angleStep;
    s32 pad;
    s32 angleFlag;
    s16 currentAngle;
    s32 targetAngle;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    Overlay84Scale *scale;

    node = state->nodes[state->current];
    scale = object->scale;
    if (node != 0) {
        if (state->inputTimer > 0) {
            state->inputTimer -= updateRate;
        } else if (state->inputLatch == 0 &&
                   (D_800D31B8 & 0x9000) != 0 &&
                   state->marked == 0) {
            amSndPlay(0xC, 0);
            state->inputLatch = 1;
        }

        if (state->inputLatch == 0) {
            if (state->actionTimer > 0) {
                state->actionTimer -= updateRate;
            } else if (D_800D31C0 < -0x10 && state->marked == 0) {
                overlay84AdvanceCurrent_o084Reloc(1);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                amSndPlay(0xF, 0);
            } else if (D_800D31C0 >= 0x11 && state->marked == 0) {
                overlay84AdvanceCurrent_o084Reloc(0);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                amSndPlay(0xF, 0);
            }
        }

        currentAngle = state->angle;
        targetAngle = state->targetAngle;
        angleFlag = state->flags & 1;
        if (angleFlag != 0) {
            if (angleFlag == 1) {
                if (currentAngle < targetAngle) {
                    targetAngle += 0xFFFF0000;
                }
            }
        } else {
            if (targetAngle < currentAngle) {
                targetAngle += 0x10000;
            }
        }

        for (i = 0; i < updateRate; i++) {
            state->tilt += (state->targetTilt - state->tilt) * 0.125f;
            state->height += (state->targetHeight - state->height) * 0.125f;
            angleStep =
                mathDiffAngle(currentAngle, targetAngle) >> 5;
            if (angleStep == 0) {
                currentAngle = targetAngle;
            } else {
                currentAngle += angleStep;
            }
        }

        if (angleStep >= -0x1F && angleStep < 0x20) {
            choice = node->choice;
            state->first = choice->first;
            state->second = choice->second;
        } else {
            state->first = 0xFF;
            state->second = 0xFF;
        }
        state->angle = currentAngle;

        if (state->blendTimer > 0) {
            state->blendTimer = (state->blendTimer - updateRate) & 0xFF;
        } else {
            state->blend += gOverlay84BlendStep * updateRate;
            if (state->blend > 1.0f) {
                state->blend = 1.0f;
            }
        }

        baseX = func_8002A8C0(state->angle) * scale->radius + object->x;
        baseY = state->height + object->y;
        baseZ = func_8002A8BC(state->angle) * scale->radius + object->z;
        targetX = baseX + state->blend * (node->x - baseX);
        targetY = baseY + state->blend * (node->y - baseY);
        targetZ = baseZ + state->blend * (node->z - baseZ);

        for (i = 0; i < updateRate; i++) {
            state->x += (targetX - state->x) * 0.125f;
            state->y += (targetY - state->y) * 0.125f;
            state->z += (targetZ - state->z) * 0.125f;
        }

        camSetNo(0);
        output = camGetPtr();
        output->x = state->x;
        output->y = state->y;
        output->z = state->z;
        output->verticalAngle =
            func_8000FAE0(output->x, output->y, output->z);
        angleStep = -Arctanf(object->x - output->x,
                             object->z - output->z);
        angleStep = dAngle(angleStep, 0x8000 - node->angle, state->blend);
        state->outputAngle.word = dAngle(
            state->outputAngle.half.base, angleStep,
            1.0f - (f32)state->actionTimer / 20.0f);
        output->angle = state->outputAngle.word;
        output->tilt = state->tilt + state->angleOffset;
        output->roll = 0;
    }
}

/* DKR v77/v80 have no matching input-gated resource update. */
static void overlay84UpdateResource(void *unused, Overlay84State *state, s32 value) {
    Overlay84Resource *resource;

    if ((D_800D31B8 & 0xD000) != 0 && state->active != 0 &&
        state->marked == 0) {
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            resource->scale = 1.0f;
            func_overlay_041_F0000854_1887B8C(resource, (f32)value, value, value);
        }
    }
    resource = overlay84GetResource(state->resource);
    if (resource != 0) {
        state->mode = 0;
        overlay84ReleaseResource(state->resource);
    }
}

/* DKR v77/v80 have no matching state/resource reset sequence. */
void overlay84ResetCurrent(void) {
    Overlay84State *state;
    Overlay84Resource *resource;

    if (gOverlay84Object != 0) {
        state = gOverlay84Object->state;
        state->mode = 3;
        state->active = 0;
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            resource->scale = 1.0f;
            func_overlay_041_F0000854_1887B8C(resource, 2.0f, 2, 2);
            state->mode = 0;
            overlay84ReleaseResource(state->resource);
        }
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this resource refresh. */
static void overlay84RefreshCurrent(void *unused, Overlay84State *state, s32 unused2) {
    struct {
        s32 changed;
        s32 reserved[2];
    } locals;
    Overlay84Resource *resource;
    Overlay84ResourceRecord *node;

    locals.changed = 0;
    if ((D_800D31B8 & 0xD000) != 0 && state->active != 0 &&
        state->marked == 0) {
        locals.changed = 1;
        resource = overlay84GetResource(state->selectedResource);
        if (resource != 0) {
            node = &resource->records[resource->recordIndex - 1];
            state->x = node->x;
            state->y = node->y;
            state->z = node->z;
            state->outputAngle.word = node->angle;
        }
    }
    resource = overlay84GetResource(state->selectedResource);
    if (resource != 0 || locals.changed != 0) {
        state->blend = 1.0f;
        state->angle = state->targetAngle;
        state->mode = 2;
        state->actionTimer = 0;
        overlay84ReleaseResource(state->selectedResource);
        state->selectedResource = 0xFF;
    }
}

s32 overlay84GetActive(void) {
    Overlay84State *state;

    if (gOverlay84Object == 0) {
        return -1;
    }
    state = gOverlay84Object->state;
    return state->mode;
}

/*
 * Overlay 84 text +0xC9C..+0xDBC. The source naturally reproduces the retail
 * frame, control flow, call order, memory effects, and complete FP schedule.
 */
/* Runtime relocation identities are animation-path operations:
 * overlay84GetResource -> func_800508B4, func_8005055C ->
 * func_8005055C, and overlay84ReleaseResource -> animseqStartPath. The getter
 * consumes only the path index; this widened prototype deliberately preserves
 * choice/object in a1/a2 for the exact allocator topology. */
/* Matched 2026-08-28 by a bounded decomp-permuter pass after the prior hand
 * and flag probes plateaued on the state-pointer spill home. The untouched
 * IDO output preserves the 72-word body, 0x28 frame, and five relocation
 * sites. Evidence re-reviewed 2026-08-29: the owned range, complete Overlay 84
 * image, and preserved full ROM are byte-identical to retail. */
void overlay84LoadCurrent(s32 kind)
{
  Overlay84State *state;
  Overlay84Choice *choice;
  Overlay84Object *object;
  Overlay84Resource *resource;
  Overlay84Node *node;
  Overlay84ResourceRecord *transform;
  object = gOverlay84Object;
  if (object != 0)
  {
    state = object->state;
    state->mode = 3;
    state->active = 0;
    node = state->nodes[state->current];
    choice = node->choice;
    switch (kind)
    {
      case 0:
        state->resource = choice->loadResource0;
        break;

      case 1:
        state->resource = choice->loadResource1;
        break;

      case 2:
        state->resource = choice->loadResource2;
        break;

      case 3:
        state->resource = choice->loadResource3;
        break;

    }

    resource = overlay84GetResource(state->resource, choice, object);
    if (resource != 0)
    {
      transform = resource->records;
      transform->x = state->x;
      transform->y = state->y;
      transform->z = state->z;
      transform->angle = 0x8000 - state->outputAngle.word;
      transform->tilt = -state->tilt;
      transform->roll = 0;
      func_8005055C(state->resource);
      overlay84ReleaseResource(state->resource);
      resource->flags |= 2;
    }
  }
}

void overlay84CopyPair(Overlay84Pair *dst, Overlay84Entry *src) {
    dst->first = src->first;
    dst->second = src->second;
}

/* Exact C: all 82 instruction words, the -48 frame, relocations, and linked
 * overlay range match after bounded permutation. */
void overlay84AdvanceCurrent(s32 direction)
{
  Overlay84Object *object;
  Overlay84State *state;
  s32 found;
  s32 pad;
  Overlay84Node *node;
  object = gOverlay84Object;
  found = 0;
  if (&pad)
  {
    ;
  }
  if (object != 0)
  {
    state = object->state;
    if (state->last != (-1))
    {
      do
      {
        if (direction == 1)
        {
          state->current--;
          if (state->current < 0)
          {
            state->current = state->last;
          }
        }
        else
        {
          state->current++;
          if (state->last < state->current)
          {
            state->current = 0;
          }
        }
        if ((state->nodes[state->current] != 0) && ((state->disabled & (1 << state->current)) == 0))
        {
          found = 1;
        }
      }
      while (found == 0);
      /* A second name for gOverlay84Object: with one name IDO reuses the
       * first load and the body no longer matches. */
      object = gOverlay84ObjectWrite;
      /* The volatile read forces the reload of `current` the target has. */
      node = state->nodes[*((volatile s8 *) (&state->current))];
      state->targetAngle = Arctanf(node->x - object->x, node->z - object->z);
      state->targetTilt = -node->tilt;
      state->targetHeight = node->y;
      state->flags &= ~1;
      state->flags |= direction;
    }
  }
}

/*
 * DKR v77/v80 contain many generic arctan2 object-heading calculations, but
 * neither DKR nor JFG has this indexed-node selection/update sequence or an
 * exact object donor.
 */
void overlay84SelectCurrent(s32 index) {
    register Overlay84State *state;
    register Overlay84Object *object;
    u16 scratch[2];

    (void)&scratch;
    object = gOverlay84Object;
    if (object != 0) {
        Overlay84Node *node;
        state = object->state;
        if (state->last != -1) {
            state->current = index;
            object = gOverlay84Object;
            node = state->nodes[state->current];
            state->targetAngle =
                Arctanf(node->x - object->x, node->z - object->z);
            state->targetTilt = -node->tilt;
            state->targetHeight = node->y;
            state->flags &= ~1;
        }
    }
}

s32 overlay84GetCurrent(void) {
    Overlay84State *state;

    if (gOverlay84Object == 0) {
        return 0;
    }
    state = gOverlay84Object->state;
    if (state->last == -1) {
        return 0;
    }
    return state->current;
}

void overlay84SetBit(s32 index) {
    if (gOverlay84Object != NULL) {
        gOverlay84Object->state->disabled |= 1 << index;
    }
}

void overlay84GetValues(s32 *first, s32 *second) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        *first = state->first;
        *second = state->second;
    }
}

/* Reusing the word-sized selector parameter preserves the retail register
 * lifetime and stack frame while the selected byte is stored in state. */
void overlay84ActivateCurrent(s32 kind) {
    Overlay84State *state;
    Overlay84Node *node;
    Overlay84Choice *choice;
    Overlay84Resource *resource;
    Overlay84ResourceRecord *record;
    s16 tilt;
    f32 height;

    if (gOverlay84Object != NULL) {
        state = gOverlay84Object->state;
        node = state->nodes[state->current];
        state->active = 0;
        choice = node->choice;

        switch (kind) {
        case 0:
            kind = choice->activateResource0;
            break;
        case 1:
            kind = choice->activateResource1;
            break;
        case 2:
            kind = choice->activateResource2;
            break;
        case 3:
            kind = choice->activateResource3;
            break;
        }

        resource = func_800508B4(kind);
        if (resource != NULL) {
            record = &resource->records[resource->recordIndex - 1];
            record->x = node->x;
            record->y = node->y;
            record->z = node->z;
            record->angle = node->angle;
            record->tilt = node->tilt;
            record->roll = node->roll;

            state->selectedResource = kind;
            state->blendTimer = 0;
            state->actionTimer = 20;
            state->blend = 1.0f;
            tilt = -node->tilt;
            state->targetTilt = tilt;
            state->tilt = tilt;
            state->angle = state->targetAngle;
            state->outputAngle.word = 0x8000 - node->angle;
            height = node->y;
            state->targetHeight = height;
            state->height = height;
            state->x = node->x;
            state->y = node->y;
            state->z = node->z;

            func_8005055C(state->selectedResource);
            animseqStartPath(state->selectedResource);
            resource->flags |= 2;
        }
        state->mode = 1;
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay84InitializeCurrent(void) {
    Overlay84Object *object;
    Overlay84State *state;
    Overlay84Node *template;
    s16 tilt;
    f32 position;

    object = gOverlay84Object;
    if (object != NULL) {
        state = object->state;
        template = state->nodes[state->current];
        state->blend = 1.0f;
        state->mode = 2;
        state->selectedResource = 0xFF;
        state->blendTimer = 0;
        state->actionTimer = 0x14;
        tilt = -template->tilt; state->angle = state->targetAngle;
        state->targetTilt = tilt;
        state->tilt = tilt;
        state->outputAngle.word = 0x8000 - template->angle;
        position = template->y;
        state->targetHeight = position;
        state->height = position;
        state->x = template->x;
        state->y = template->y;
        state->z = template->z;
    }
}

void overlay84ClearActive(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->mode = 0;
    }
}

s32 overlay84IsUnitScale(void) {
    Overlay84State *state;

    if (gOverlay84Object == 0) {
        return 0;
    }
    state = gOverlay84Object->state;
    return state->blend == 1.0f;
}

s32 overlay84GetEnabledCurrent(void) {
    Overlay84State *state;

    if (gOverlay84Object == 0) {
        return -1;
    }
    state = gOverlay84Object->state;
    if (state->last == -1) {
        return -1;
    }
    if (state->inputLatch != 0) {
        return state->current;
    }
    return -1;
}

void overlay84ClearMode(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->inputLatch = 0;
    }
}

void overlay84SetAngle(s16 angle) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->angleOffset = angle;
    }
}

void overlay84Mark(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->marked = 1;
    }
}
