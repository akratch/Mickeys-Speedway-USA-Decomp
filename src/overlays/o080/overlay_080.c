#include "overlays/overlay_080.h"

void overlay80InitializeContact(Overlay80Object *object, const Overlay80Init *init)
{
  Overlay80State *state;
  Overlay80Candidate **candidates;
  s32 count;
  s32 index;
  s32 selected;
  state = object->state;
  object->scaledValue = (*object->objectScale) * (((f32) ((0, init->scale))) * gOverlay80Scale0);
  object->key = init->key;
  count = func_8001398C(object->positionX, object->positionZ, 0x1800, &candidates);
  if (count != ((0, 0)))
  {
    selected = -1;
    for (index = 0; index < count; index++)
    {
      if (candidates[index]->threshold <= object->positionY)
      {
        selected = index;
        break;
      }
 } if (selected != (-1)) {
      func_8001DCD0(object->key, candidates[index]->value, &object->outputA, &object->outputB);
    }
  }
  state->scale = (f32) init->initialValue;
  if (object->notice != 0)
  {
    object->notice->state = 2;
  }
 ;
}

/* `pad` holds the 0x80 frame: the permuter's two never-read aliases (of
 * `contacts` and of `magnitude`) each reserved a frame word, and without them every
 * stack offset moves (4 words each). Its third alias, of `contactState`, was
 * inert and is gone (lane c-5, 2026-10-09). */
void overlay80UpdateContact(Overlay80Object *object, s32 ticks)
{
  Overlay80State *state;
  Overlay80State *contactState;
  Overlay80Object *contacts[9];
  s32 pad[2];
  f32 impact;
  f32 dx;
  f32 dy;
  f32 dz;
  f32 magnitude;
  Overlay80Object *contact;
  f32 distance;
  f32 ratio;
  s32 kind;
  state = object->state;
  if (func_8005776C(object->positionX, object->positionY, object->positionZ, state->scale, 0, contacts) != 0)
  {
    contact = contacts[0];
    dx = object->positionX - contact->positionX;
    dy = object->positionY - contact->positionY;
    dz = object->positionZ - contact->positionZ;
    contactState = contact->state;
    distance = ((contact->normalX * dx) + (dy * contact->normalY)) + (dz * contact->normalZ);
    if (((distance < 0.0f) && (state->previousDistance > 0.0f)) || ((state->previousDistance < 0.0f) && (distance > 0.0f)))
    {
      ratio = 1.0f - (sqrtf(((dx * dx) + (dy * dy)) + (dz * dz)) / state->scale);
      magnitude = contactState->scale;
      if (magnitude < 0.0f)
      {
        magnitude = -magnitude;
      }
      kind = -1;
      impact = (magnitude / gOverlay80MagnitudeScale) * ratio;
      if ((gOverlay80Band0Lower < impact) && (impact <= gOverlay80Band0Upper))
      {
        kind = 0;
      }
      else
        if ((gOverlay80Band1Lower < impact) && (impact <= gOverlay80Band1Upper))
      {
        kind = 1;
      }
      else
        if ((gOverlay80Band2Lower < impact) && (impact <= gOverlay80Band2Upper))
      {
        kind = 2;
      }
      else
        if (gOverlay80Band3Lower < impact)
      {
        kind = 3;
      }
      if ((state->active == 0) && (kind != (-1)))
      {
        state->active = 1;
        func_8005AD64(object, kind, -1, 0.0f);
      }
    }
    state->previousDistance = distance;
  }
  else
  {
    state->previousDistance = 0.0f;
  }
  if ((state->active != 0) && (func_8005ABA8(object, 0.0075f, (f32) ticks) != 0))
  {
    state->active = 0;
  }
  if (object->notice != 0)
  {
    object->notice->state = 1;
  }
}
