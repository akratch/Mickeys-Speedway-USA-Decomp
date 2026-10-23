#include "overlays/overlay_021.h"

/*
 * Overlay 21: one translation unit in ROM order (ADR 0006); both functions
 * are ORT exports. Plane registration and plane-side priority selection are
 * reconstructed from Mickey's call and field evidence; exact DKR v77/v80 and
 * JFG scans are negative. The module compiles under the overlay flag set
 * (-Wab,-r4300_mul), which reproduces the shipped FP schedule.
 */

void overlay21RegisterPlane(Overlay21Object *object, Overlay21Init *init) {
    Overlay21Plane *plane;
    f32 value;
    s32 count;

    object->angle0 = init->angle0 << 8;
    object->angle1 = init->angle1 << 8;
    object->unused4 = 0;
    plane = object->plane;
    plane->id = object->id;
    value = func_8002A8C0(object->angle0);
    plane->normalX = overlay21CosReloc(object->angle1) * value;
    plane->normalY = -overlay21CosReloc(object->angle1);
    value = overlay21CosReloc(object->angle0);
    plane->normalZ = overlay21CosReloc(object->angle1) * value;
    plane->distance = -((plane->normalX * object->x) +
                        (plane->normalY * object->y) +
                        (plane->normalZ * object->z));
    plane->lowerPriority = init->lowerPriority;
    plane->upperPriority = init->upperPriority;
    count = gOverlay21ObjectCount;
    gOverlay21Objects[count] = object;
    gOverlay21ObjectCount = count + 1;
}

/*
 * Exact 114-word match under the measured -Wab,-r4300_mul override. The
 * `selected` flag (the permuter's `new_var`) holds the plane test in its own
 * variable, which preserves IDO's shipped caller-saved allocation. Measured
 * (lane c-5, 2026-10-09): testing `plane->id == planeId` directly differs in
 * 10 words, and `if (plane->id != planeId) continue;` grows the function.
 */
void overlay21ApplyPriorities(s32 planeId, s32 entryCount, Overlay21PriorityEntry **entries)
{
  Overlay21Position *reference;
  Overlay21Plane *plane;
  Overlay21PriorityEntry *entry;
  Overlay21PriorityEntry **cursor;
  int selected;
  Overlay21Position *position;
  f32 distance;
  f32 normalX;
  f32 normalY;
  f32 normalZ;
  f32 planeDistance;
  s32 planeIndex;
  s32 entryIndex;
  s32 continueLoop;
  reference = camGetPtr();
  for (planeIndex = 0; planeIndex < gOverlay21ObjectCount; planeIndex++)
  {
    plane = gOverlay21Objects[planeIndex]->plane;
    selected = plane->id == planeId;
    if (selected)
    {
      normalX = plane->normalX;
      normalY = plane->normalY;
      normalZ = plane->normalZ;
      planeDistance = plane->distance;
      cursor = entries;
      distance = (((reference->x * normalX) + (reference->y * normalY)) + (reference->z * normalZ)) + planeDistance;
      if (distance > 0.0f)
      {
        if (entryCount != 0)
        {
          entryIndex = entryCount - 1;
          do
          {
            entry = *(cursor++);
            position = entry->position;
            distance = (((position->x * normalX) + (position->y * normalY)) + (position->z * normalZ)) + planeDistance;
            if ((distance < 0.0f) && (plane->lowerPriority < entry->priority))
            {
              entry->priority = plane->lowerPriority;
            }
            continueLoop = entryIndex;
            entryIndex--;
          }
          while (continueLoop != 0);
        }
      }
      else
        if ((plane->upperPriority >= 0) && (entryCount != 0))
      {
        entryIndex = entryCount - 1;
        do
        {
          entry = *(cursor++);
          position = entry->position;
          distance = (((position->x * normalX) + (position->y * normalY)) + (position->z * normalZ)) + planeDistance;
          if ((distance > 0.0f) && (plane->upperPriority < entry->priority))
          {
            entry->priority = plane->upperPriority;
          }
          continueLoop = entryIndex;
          entryIndex--;
        }
        while (continueLoop != 0);
      }
    }
  }

}
