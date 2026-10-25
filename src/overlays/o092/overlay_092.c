#include "overlays/overlay_092.h"

void overlay92Init(Overlay92Object *object, Overlay92InitData *init) {
    Overlay92State *state = object->state;
    state->x = init->x << 8; state->y = init->y << 8; state->z = init->z << 8;
    state->w = init->w << 8; state->a = init->a; state->b = init->b; state->mode = init->mode;
    object->x = state->z; object->y = state->y; object->z = state->x;
}

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
    objects = func_8000572C(&start, &end);
    nearestDistance = 3.402823466e+38f;
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
        nearestDistance = 0.87f;
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

/* Reconstructed from Mickey-owned code and relocation evidence.
 * The union self-copy is an independently reviewed IDO-specific allocation
 * aid, not a claim about original source spelling. */
void func_overlay_092_F0000308_18D6228(Overlay92Owner *owner,
                                       Overlay92UpdateConfig *config,
                                       Overlay92CameraState *state,
                                       s32 update) {
    union { f32 value; u32 bits; } x;
    f32 y;
    f32 z;
    f32 pathX;
    f32 pathY;
    f32 pathZ;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 blend;
    f32 wave;
    s16 remaining;
    Overlay92CameraRecord *record;
    s32 pathIndex;
    s32 level;

    record = &D_0[config->recordIndex];
    if (update >= state->timer) {
        state->timer = 0;
    } else {
        state->timer -= update;
    }

    if (config->smoothAngles != 0) {
        remaining = update;
        while (remaining--) {
            state->pitch += mathDiffAngle(state->pitch, 0x800) >> 3;
            state->roll -= state->roll >> 3;
        }
        return;
    }

    switch (state->mode) {
        case 1:
            blend = (f32)state->timer - 150.0f;
            if (blend < 0.0f) {
                blend = 0.0f;
            }
            blend = 1.0f - (blend / 150.0f);
            wave = (func_8002A8C0((s32)((blend * 32767.0f) + 16384.0f)) *
                    -0.5f) +
                   0.5f;
            if (state->timer == 0) {
                state->mode = 2;
                state->timer = mathRnd(0x1C2, 0x546);
            }

            func_overlay_008_F00042A8_1862000(record, owner, (f32)update);
            x.value = record->x;
            y = record->y;
            z = record->z;
            overlay1ResolveMotionPoint(owner, 0, &pathX, &pathY,
                                              &pathZ);
            /* Initialized representation self-copy: inert on IDO/o32.
             * Retains the stack-backed float lifetime; see cleanup queue. */
            x.bits = x.bits;
            x.value = ((pathX - x.value) * wave) + x.value;
            y = ((((func_8002A8BC((s32)(wave * 65535.0f)) * -0.5f) +
                    0.5f) *
                   30.0f) +
                 (y + ((pathY - y) * wave)));
            z = z + ((pathZ - z) * wave);

            state->targetX = x.value;
            state->x = state->targetX;
            state->targetY = y;
            state->y = state->targetY;
            state->targetZ = z;
            state->z = state->targetZ;
            state->positionTag = func_8000FAE0(x.value, y, z);

            dx = x.value - owner->x;
            dy = (y - owner->y) - 20.0f;
            dz = z - owner->z;
            wave = blend * 3.0f;
            if (wave > 1.0f) {
                wave = 1.0f;
            }
            state->yaw =
                (s16)((mathDiffAngle(record->yaw,
                                     Arctanf(dz, dx) + 0x4000) *
                      wave) +
                      record->yaw);
            level = (s16)Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->pitch =
                (s16)((record->pitch * (1.0f - wave)) +
                      (level *
                       wave));
            state->roll = (s16)(record->roll * (1.0f - wave));
            break;

        case 2:
            if (func_overlay_092_F0000068_18D5F88(
                    (O92Racer *)owner, &x.value, &y, &z, &pathIndex) != 0) {
                state->targetX = x.value;
                state->x = state->targetX;
                state->targetY = y;
                state->y = state->targetY;
                state->targetZ = z;
                state->z = state->targetZ;
                state->positionTag = func_8000FAE0(x.value, y, z);
                dx = x.value - owner->x;
                dy = (y - owner->y) - 20.0f;
                dz = z - owner->z;
            } else {
                dx = state->x - owner->x;
                dy = (state->y - owner->y) - 20.0f;
                dz = state->z - owner->z;
            }
            state->yaw = Arctanf(dz, dx) + 0x4000;
            state->pitch = Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->roll = 0;
            break;

        case 0:
            level = levelGetNumber();
            _bcopy(state, record, 0x54);
            if ((level == 0x11) || (level == 0x10)) {
                state->timer = 0;
            } else {
                state->mode = 3;
                state->timer = 0x12C;
            }
            /* fallthrough */

        case 3:
            if ((state->timer == 0) ||
                (D_800C947C != 0)) {
                state->mode = 2;
                state->timer = mathRnd(0x1C2, 0x546);
            }
            if (state->pathIndex == -1) {
                if ((func_overlay_092_F0000068_18D5F88(
                         (O92Racer *)owner, &x.value, &y, &z, &pathIndex) == 0) ||
                    (pathIndex == -1)) {
                    pathIndex = 3;
                }
                state->pathIndex = pathIndex;
            }
            state->pathIndex = 3;
            overlay1ResolveMotionPoint(owner, state->pathIndex, &x.value,
                                              &y, &z);
            state->targetX = x.value;
            state->x = state->targetX;
            state->targetY = y;
            state->y = state->targetY;
            state->targetZ = z;
            state->z = state->targetZ;
            state->positionTag = func_8000FAE0(x.value, y, z);
            dx = x.value - owner->x;
            dy = (y - owner->y) - 20.0f;
            dz = z - owner->z;
            state->yaw = Arctanf(dz, dx) + 0x4000;
            state->pitch = Arctanf(dy, sqrtf((dx * dx) + (dz * dz)));
            state->roll = 0;
            break;
    }
}
