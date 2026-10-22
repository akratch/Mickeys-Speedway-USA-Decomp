#include "PR/ultratypes.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay29PathState {
    u8 pad00[0x14];
    u32 flags;
    u8 pad18[4];
    Vec3f direction;
} Overlay29PathState;

typedef struct Overlay29Transform {
    Vec3f direction;
    u8 pad0C[4];
    Vec3f position;
    f32 height;
    u32 flags;
} Overlay29Transform;

typedef struct Overlay29Owner {
    u8 pad00[0x64];
    Overlay29PathState *state;
} Overlay29Owner;

extern u8 gOverlay29MinimumYReloc[];
extern f32 sqrtf(f32 value);

/* Exact configured IDO output: 121 words, a 0x68 frame, and all three
 * static relocation identities. The linked owned range and full US ROM
 * are byte-identical.
 * Keep the apparently redundant single-precision multiply by 1.0f:
 * in the proved context it changes one product's emitted operand order
 * without adding an instruction. Its optimization-pass mechanism is
 * untraced; this is not a general floating-point simplification rule.
 * Retain this spelling until a naturalized form repeats the full proof. */
void func_overlay_029_F0000EE0_187E190(s32 unused, Vec3f *output, Vec3f *axis, f32 height, Overlay29Transform *transform, Overlay29Owner *owner)
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
