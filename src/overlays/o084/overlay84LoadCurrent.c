#include "PR/ultratypes.h"

/*
 * Overlay 84 text +0xC9C..+0xDBC. The source naturally reproduces the retail
 * frame, control flow, call order, memory effects, and complete FP schedule.
 */

typedef struct Overlay84Transform {
    s16 angle0;
    s16 angle2;
    s16 angle4;
    u8 pad6[6];
    f32 x;
    f32 y;
    f32 z;
} Overlay84Transform;

typedef struct Overlay84Resource {
    u8 pad0[0x16];
    u8 flags;
    u8 pad17[9];
    Overlay84Transform *transform;
} Overlay84Resource;

typedef struct Overlay84Choice {
    u8 pad0[0x15];
    u8 resource0;
    u8 resource1;
    u8 pad17;
    u8 resource2;
    u8 resource3;
} Overlay84Choice;

typedef struct Overlay84Node {
    u8 pad0[0x3C];
    Overlay84Choice *choice;
} Overlay84Node;

typedef struct Overlay84State {
    u8 pad0;
    s8 current;
    u8 pad2[7];
    u8 mode;
    u8 resource;
    u8 padB[7];
    s16 tilt;
    s16 saved;
    u8 pad16[0xA];
    s32 angle;
    u8 pad24[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[8];
    Overlay84Node *nodes[32];
    u8 padC4;
    u8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;
extern Overlay84Resource *overlay84GetResource(
    u8 resource, Overlay84Choice *choice, Overlay84Object *object);
extern void func_8005055C(u8 resource);
extern void overlay84ReleaseResource(u8 resource);

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
  Overlay84Transform *transform;
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
        state->resource = choice->resource0;
        break;

      case 1:
        state->resource = choice->resource1;
        break;

      case 2:
        state->resource = choice->resource2;
        break;

      case 3:
        state->resource = choice->resource3;
        break;

    }

    resource = overlay84GetResource(state->resource, choice, object);
    if (resource != 0)
    {
      transform = resource->transform;
      transform->x = state->x;
      transform->y = state->y;
      transform->z = state->z;
      transform->angle0 = 0x8000 - state->angle;
      transform->angle2 = -state->tilt;
      transform->angle4 = 0;
      func_8005055C(state->resource);
      overlay84ReleaseResource(state->resource);
      resource->flags |= 2;
    }
  }
}
