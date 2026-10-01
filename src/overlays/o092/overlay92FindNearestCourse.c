#include "PR/ultratypes.h"

typedef struct O92CourseEntry {
    u8 pad00[8];
    u16 start;
    u16 end;
    s8 value;
} O92CourseEntry;

typedef struct O92Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    O92CourseEntry *course;
} O92Object;

typedef struct O92VehicleState {
    u8 pad00[0x3B2];
    u8 coursePosition;
} O92VehicleState;

typedef struct O92Racer {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    O92VehicleState *vehicle;
} O92Racer;

extern f32 gOverlay92DistanceParameters[2];
extern O92Object **overlay92GetObjectRange(s32 *start, s32 *end);
extern f32 sqrtf(f32 value);

/* Matched by writing the scan as a plain `for` over the object range with an
 * early `continue`: the loop test is then strength-reduced by the compiler,
 * which is where the scaled index, the cursor and the per-arm bound come from.
 * No cursor, scaled index or limit is declared. The racer's course position
 * and the entry's start are named locals read before the valid flag is
 * cleared (both are frame cells as well as the schedule), and the scan's
 * distance is one expression, so its deltas are compiler temporaries distinct
 * from the named deltas of the projection below.
 */
s32 func_overlay_092_F0000068_18D5F88(O92Racer *racer, f32 *outX,
                                      f32 *outY, f32 *outZ, s32 *outValue) {
    O92Object **objects;
    s32 start;
    s32 end;
    O92Object *object;
    O92Object *nearest;
    O92VehicleState *vehicle;
    s32 i;
    s32 valid;
    s32 position;
    s32 first;
    O92CourseEntry *course;
    f32 nearestDistance;
    f32 distance;
    f32 scale;
    f32 dx;
    f32 dy;
    f32 dz;

    vehicle = racer->vehicle;
    objects = overlay92GetObjectRange(&start, &end);
    nearestDistance = gOverlay92DistanceParameters[0];
    nearest = 0;

    for (i = start; i < end; i++) {
        object = objects[i];
        if (object->type != 12) {
            continue;
        }
        course = object->course;
        position = vehicle->coursePosition;
        first = course->start;
        valid = 0;
        if (first < course->end) {
            if ((position >= first) && (position < course->end)) {
                valid = 1;
            }
        } else if ((position >= first) || (position < course->end)) {
            valid = 1;
        }
        if (valid != 0) {
            distance = sqrtf(((object->x - racer->x) * (object->x - racer->x)) +
                             ((object->y - racer->y) * (object->y - racer->y)) +
                             ((object->z - racer->z) * (object->z - racer->z)));
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearest = object;
            }
        }
    }

    if (nearest != 0) {
        dx = nearest->x - racer->x;
        dy = nearest->y - racer->y;
        dz = nearest->z - racer->z;
        course = nearest->course;
        distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        nearestDistance = gOverlay92DistanceParameters[1];
        scale = 60.0f / distance;
        dx *= scale;
        dy *= scale;
        dz *= scale;
        *outX = racer->x + dx;
        *outY = racer->y + dy;
        *outZ = racer->z + dz;
        dx = *outX - nearest->x;
        dy = *outY - nearest->y;
        dz = *outZ - nearest->z;
        *outX = nearest->x + (dx * nearestDistance);
        *outY = nearest->y + (dy * nearestDistance);
        *outZ = nearest->z + (dz * nearestDistance);
        *outValue = course->value;
        return 1;
    }
    return 0;
}
