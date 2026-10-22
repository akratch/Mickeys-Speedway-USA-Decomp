#include "PR/ultratypes.h"

/* DKR obj_init_scenery shares only the scale-prefix semantics, not this body. */
typedef struct Overlay97State {
    f32 direction[3];
    f32 offset;
    s32 scaleIndex;
    s16 value18;
    s16 value15;
} Overlay97State;

typedef struct Overlay97Object {
    s16 angleA;
    s16 angleB;
    u8 pad4[4];
    f32 scale;
    f32 position[3];
    u8 pad18[0x4C];
    Overlay97State *state;
} Overlay97Object;

typedef struct Overlay97Config {
    u8 pad0[0x12];
    u8 scaleIndex;
    u8 angleA;
    u8 pad14;
    s8 value15;
    u8 pad16[2];
    s8 value18;
    u8 pad19[2];
    u8 angleB;
} Overlay97Config;

extern f32 gOverlay97MinimumScale;
extern f32 gOverlay97ScaleStep;
extern f32 overlay97TrigAReloc(s16 angle);
extern f32 func_8002A8BC(s16 angle);

void overlay97InitDirection(Overlay97Object *object, Overlay97Config *config) {
    Overlay97State *state;
    f32 saved;

    if (config->scaleIndex < 5) {
        object->scale = gOverlay97MinimumScale;
    } else {
        object->scale = config->scaleIndex * gOverlay97ScaleStep;
    }

    state = object->state;
    object->angleA = (config->angleA & 0xFF) << 10;
    object->angleB = (config->angleB & 0xFF) << 10;

    saved = overlay97TrigAReloc(object->angleA);
    state->direction[0] = func_8002A8BC(object->angleB) * saved;
    state->direction[1] = -overlay97TrigAReloc(object->angleB);
    saved = overlay97TrigAReloc(object->angleA);
    state->direction[2] = func_8002A8BC(object->angleB) * saved;
    state->offset = -((state->direction[2] * object->position[2]) +
                      ((state->direction[1] * object->position[1]) +
                       (object->position[0] * state->direction[0])));
    state->scaleIndex = config->scaleIndex;
    state->value18 = config->value18;
    state->value15 = config->value15;
}
