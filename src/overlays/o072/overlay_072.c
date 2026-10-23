#include "overlays/overlay_072.h"

const f32 gOverlay72ComponentScale = 0.1f;

/* Overlay 72 +0x000. Fresh DKR v77/v80 and JFG scans are negative. */
void overlay72Init(Overlay72InitObject *object, Overlay72Config *config) {
    Overlay72State *state = object->state;
    Overlay72Component *component;

    object->flags |= 0x800;
    state->scale = config->scale * 1.125f;
    state->y = object->y + config->yOffset;
    state->z = object->y + config->zOffset;
    object->angle = config->angle << 8;
    component = object->component;
    if (component != 0) {
        component->scale = state->scale * gOverlay72ComponentScale;
        component = object->component;
        component->pairedScale = component->scale;
    }
}

/* Overlay 72 +0x0B4. Fresh DKR v77/v80 and JFG scans are negative. */
void overlay72Update(Overlay72QueryObject *object, s32 updateRate) {
    Overlay72Bounds *bounds = object->bounds;
    s32 count;
    Overlay72Candidate *results[8];
    register s32 index;

    count = func_8005776C(
        object->x,
        object->y,
        object->z,
        bounds->radius,
        1,
        results
    );
    if (count != 0) {
        index = count - 1;
        do {
            Overlay72Candidate *candidate = results[index];
            if (bounds->minimum < candidate->height &&
                candidate->height < bounds->maximum) {
                func_overlay_008_F0000F1C_185EC74(candidate, 0);
            }
            count = index;
            index--;
        } while (count != 0);
    }
}
