#include "overlays/overlay_004.h"

void overlay4InitializeObjectMotion(Overlay4InitObject *object,
                                    Overlay4InitConfig *config) {
    Overlay4InitState *state;

    state = object->state;
    object->heading = config->heading << 8;
    object->outputHeading = config->outputHeading << 8;
    state->speed = (f32)(config->speed * 10);
    state->timer = (u8)((f32)config->timer * 6.0f);
    state->trigger = 0;
    state->phase = 0;
    func_8005AD64(object, 0, -1, 0.0f);
    gOverlay4InitStatus = 2;
}

/*
 * Exact configured C from the bounded standing sweep and full-TU promotion.
 * The discovered context restores func_8005ABA8's integer status return.
 * Its target implementation explicitly returns that status in the ABI result
 * register; the overlay 79 caller also consumes it. This caller ignores it.
 * Keeping the correct declaration resolves the selector register allocation.
 * The spawn packet precedes the final three pointer locals as discovered.
 * Function geometry, all runtime relocation identities, the complete overlay,
 * and the rebuilt ROM are checked by the canonical promotion proof.
 * PROVENANCE: Mickey-only source, target metadata, and bounded permuter output.
 */
void overlay4UpdateObjectMotion(Overlay4MotionObject *object, s32 updateRate)
{
  Overlay4MotionState *motion;
  Overlay4Config *config;
  Overlay4SpawnPacket packet;
  Overlay4PositionOwner *positionOwner;
  Overlay4Spawned *spawned;
  Overlay4SpawnState *spawnState;
  s16 delta;
  s32 timer;
  motion = object->motion;
  config = object->config;
  func_8005ABA8(object, 0.1f, (f32) updateRate);
  switch (config->mode)
  {
    case 0:
      timer = motion->timer;
      if (updateRate >= timer)
    {
      motion->trigger = 1;
      motion->timer = (u8) (((f32) func_8002997C(config->timerMinimum, config->timerMaximum)) * 6.0f);
    }
    else
    {
      motion->timer = timer - updateRate;
    }
      break;

    case 1:
      delta = func_8002AA0C(object->angle, motion->targetAngle);
      if ((delta <= config->threshold) && (delta >= (-config->threshold)))
    {
      motion->targetAngle = func_8002997C(-0x8000, 0x7FFF);
      motion->trigger = 1;
    }
    else
    {
      motion->increment += func_80029274(func_8002AA0C(object->angle, motion->targetAngle), motion->increment, (f32) config->threshold);
    }
      break;

  }

  object->angle = (s16) (((f32) object->angle) + motion->increment);
  motion->phase += (updateRate * config->phaseSpeed) * 10;
  object->outputAngle = (s16) (((func_8002A8C0(motion->phase) * ((f32) config->amplitude)) + ((f32) config->baseAngle)) * 256.0f);
  if (motion->trigger != 0)
  {
    motion->trigger = 0;
    if (func_80004590(0x21) < config->spawnChance)
    {
      positionOwner = object->positionOwners[object->positionIndex];
      packet.kind = 0x95;
      packet.mode = 10;
      packet.flags = 0;
      packet.x = (s16) positionOwner->position->x;
      packet.y = (s16) (positionOwner->position->y - 30.0f);
      packet.z = (s16) positionOwner->position->z;
      spawned = func_8000590C(&packet, 1);
      if (spawned != 0)
      {
        spawned->config = 0;
        func_overlay_036_F00007B0(spawned, object->angle, (s16) (object->outputAngle + 0x1DDD), (f32) func_8002997C(config->spawnMinimum, config->spawnMaximum));
        spawnState = spawned->state;
        spawnState->angle = 0;
        spawnState->value = 0;
        spawnState->active = 1;
        spawnState->field07 = 0;
        spawnState->field08 = 0;
        spawnState->field0A = 0x80;
      }
    }
  }
}

/* DKR v77/v80 and JFG contain no exact donor for this group attachment. */
void overlay4AttachObject(Overlay4GroupObject *owner,
                          Overlay4GroupObject *object) {
    Overlay4GroupState *ownerState;
    Overlay4AttachState *state;
    Overlay4Group *group;

    ownerState = owner->state;
    state = object->state;
    group = &gOverlay4Groups[ownerState->group];
    /* Keeping these on one source line reproduces IDO's original schedule. */
    group->objects[group->count] = object; group->count++;
    state->owner = owner;
    state->flags |= 0xC;
    state->marker = 0xFF;
}

/* DKR v77/v80 and JFG contain no exact donor for this group-list removal. */
void overlay4RemoveObject(Overlay4RemoveObject *object) {
    Overlay4Group *group;
    Overlay4RemoveLink *link;
    Overlay4RemoveState *state;
    Overlay4GroupState *header;
    s32 index;
    s32 shiftIndex;

    link = object->link;
    state = link->state;
    header = state->header;
    group = &gOverlay4Groups[header->group];
    index = group->count;
    if (index--) {
        do {
            if (group->objects[index] == object) {
                break;
            }
        } while (index--);
    }
    group->count--;
    shiftIndex = index;
    while (shiftIndex < group->count) {
        group->objects[shiftIndex] = group->objects[shiftIndex + 1];
        shiftIndex++;
    }
}

void overlay4UpdateGroupSpacing(Overlay4ChainObject *object) {
    Overlay4Group *group;
    Overlay4GroupState *state;
    Overlay4ChainObject **objects;
    Overlay4ChainObject *previous;
    s32 count;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 scale;

    state = object->state;
    group = &gOverlay4Groups[state->group];
    count = group->count;
    objects = (Overlay4ChainObject **)group->objects;
    while (count--) {
        previous = object;
        object = *objects++;
        dx = object->x - previous->x;
        dy = object->y - previous->y;
        dz = object->z - previous->z;
        distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        scale = distance;
        if (distance > 0.0f) {
            scale = 38.0f / distance;
        }
        dx *= scale;
        dy *= scale;
        dz *= scale;
        object->x = previous->x + dx;
        object->y = previous->y + dy;
        object->z = previous->z + dz;
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
s32 overlay4GroupCount(Overlay4GroupObject *object) {
    Overlay4GroupState *state;

    state = object->state;
    return gOverlay4Groups[state->group].count;
}

Overlay4SearchObject *overlay4FindCategory2Object(Overlay4SearchKey *key) {
    s32 start;
    s32 end;
    Overlay4SearchObject **objects;
    Overlay4SearchObject *object;
    Overlay4SearchPayload *payload;
    s32 i;

    objects = func_8000572C(&start, &end);
    i = start;
    while (i < end) {
        object = objects[i++];
        if (object->type == 0x30) {
            payload = object->data.payload;
            if (payload->category == 2) {
                if (payload->identifier == key->identifier) {
                    return object;
                }
            }
        }
    }
    return NULL;
}

void overlay4FindSearchPosition(f32 *outX, f32 *outZ,
                                Overlay4SearchKey *key,
                                Overlay4SearchObject *anchor) {
    s32 start;
    s32 end;
    s32 i;
    Overlay4SearchObject **objects;
    Overlay4SearchObject *object;
    Overlay4SearchObject *best;
    volatile s32 *state;
    f32 dx;
    f32 dz;
    f32 distance;
    f32 bestDistance;

    objects = func_8000572C(&start, &end);
    bestDistance = gOverlay4SearchMaxDistance;
    if (key->mode == 1) {
        best = NULL;
        i = start;
        while (i < end) {
            object = objects[i++];
            if (object->type == 0x21) {
                state = object->data.state;
                if (*state == 0) {
                    dx = anchor->x - object->x;
                    dz = anchor->z - object->z;
                    distance = (dx * dx) + (dz * dz);
                    if (*state != 0) {
                        distance *= 4.0f;
                    }
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        best = object;
                    }
                }
            }
        }
        if (best != NULL) {
            *outX = best->x;
            *outZ = best->z;
            return;
        }
    } else {
        best = func_overlay_004_F0000734_185ADAC(key);
        if (best != NULL) {
            *outX = best->x;
            *outZ = best->z;
            return;
        }
    }
    *outX = 0.0f;
    *outZ = 0.0f;
}
