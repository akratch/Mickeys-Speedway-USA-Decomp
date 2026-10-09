#include "PR/ultratypes.h"

typedef struct Overlay80State {
    f32 previousDistance;
    f32 scale;
    s32 active;
} Overlay80State;

typedef struct Overlay80Notice {
    u8 pad00[4];
    u8 state;
} Overlay80Notice;

typedef struct Overlay80Object {
    s16 key;
    s16 outputB;
    s16 outputA;
    u8 pad06[2];
    f32 scaledValue;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    u8 pad18[4];
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    u8 pad28[0x18];
    f32 *objectScale;
    u8 pad44[0x0C];
    Overlay80Notice *notice;
    u8 pad54[0x10];
    Overlay80State *state;
} Overlay80Object;

extern f32 gOverlay80MagnitudeScale;
extern f32 gOverlay80Band0Lower;
extern f32 gOverlay80Band0Upper;
extern f32 gOverlay80Band1Lower;
extern f32 gOverlay80Band1Upper;
extern f32 gOverlay80Band2Lower;
extern f32 gOverlay80Band2Upper;
extern f32 gOverlay80Band3Lower;

extern s32 overlay80FindContactReloc(f32 positionX, f32 positionY,
                                     f32 positionZ, f32 radius,
                                     s32 ignoreY,
                                     Overlay80Object **contactOut);
extern f32 overlay80SqrtReloc(f32 value);
extern void overlay80EmitContactReloc(Overlay80Object *object, s32 kind,
                                      s32 value, f32 amount);
extern s32 overlay80AdvanceContactReloc(Overlay80Object *object, f32 rate,
                                        f32 ticks);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
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
  if (overlay80FindContactReloc(object->positionX, object->positionY, object->positionZ, state->scale, 0, contacts) != 0)
  {
    contact = contacts[0];
    dx = object->positionX - contact->positionX;
    dy = object->positionY - contact->positionY;
    dz = object->positionZ - contact->positionZ;
    contactState = contact->state;
    distance = ((contact->normalX * dx) + (dy * contact->normalY)) + (dz * contact->normalZ);
    if (((distance < 0.0f) && (state->previousDistance > 0.0f)) || ((state->previousDistance < 0.0f) && (distance > 0.0f)))
    {
      ratio = 1.0f - (overlay80SqrtReloc(((dx * dx) + (dy * dy)) + (dz * dz)) / state->scale);
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
        overlay80EmitContactReloc(object, kind, -1, 0.0f);
      }
    }
    state->previousDistance = distance;
  }
  else
  {
    state->previousDistance = 0.0f;
  }
  if ((state->active != 0) && (overlay80AdvanceContactReloc(object, 0.0075f, (f32) ticks) != 0))
  {
    state->active = 0;
  }
  if (object->notice != 0)
  {
    object->notice->state = 1;
  }
}
