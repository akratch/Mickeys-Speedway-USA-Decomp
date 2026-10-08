#include "PR/ultratypes.h"
typedef struct State { u8 pad0[4]; u16 flags; } State;
typedef struct Object {
    u8 pad00[0xC]; f32 x; f32 y; f32 z; u8 pad18[0x2E]; s16 type;
    u8 pad48[0x1C]; State *state;
} Object;
extern Object **overlay3GetObjectRangeReloc(s32 *, s32 *);
extern s32 overlay3ContainsValueReloc(Object *, Object *);
extern f32 overlay3DistanceSquaredReloc(f32, f32, f32, f32, f32, f32);
extern f32 gOverlay3SearchMaxDistance[];
/* Exact C: all 77 instruction words, the 0x78 frame, relocations, and linked
 * overlay range match after bounded permutation. Two inert allocation/block
 * aids remain below and are tracked in docs/cleanup-queue.md: the empty
 * `!cursor` test and the `if (1)` around the loop body each swap the index and
 * cursor registers (s3/s4) when deleted. The `&pad` probe was inert and is
 * gone; the pad itself holds the 0x78 frame. */
Object *overlay3FindClosestObject(Object *anchor, void *unused)
{
  s32 pad;
  s32 start;
  s32 end;
  f32 distance;
  f32 bestDistance;
  Object **objects;
  Object **cursor;
  Object *object;
  Object *best;
  State *state;
  s32 index;
  objects = overlay3GetObjectRangeReloc(&start, &end);
  bestDistance = gOverlay3SearchMaxDistance[3];
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
          if ((overlay3ContainsValueReloc(anchor, object) == 0) && (state->flags == 0))
          {
            distance = overlay3DistanceSquaredReloc(object->x, object->y, object->z, anchor->x, anchor->y, anchor->z);
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
