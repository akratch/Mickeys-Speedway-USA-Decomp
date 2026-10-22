#include "PR/ultratypes.h"

typedef struct Overlay36EffectState {
    u8 reserved000[0x19A];
    u8 state;
    u8 countdown;
    u8 reserved19C[4];
    void *activeAction;
} Overlay36EffectState;

typedef struct Overlay36EffectSource {
    u8 reserved00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x4C];
    Overlay36EffectState *effectState;
} Overlay36EffectSource;

typedef struct Overlay36SpawnRequest {
    s16 effectType;
    s16 reserved02;
    s16 x;
    s16 y;
    s16 z;
    s16 mode;
    Overlay36EffectSource *source;
    Overlay36EffectState *effectState;
} Overlay36SpawnRequest;

typedef struct Overlay36SpawnedObject {
    u8 reserved00[0x3C];
    s32 owner;
} Overlay36SpawnedObject;

extern u8 D_8007BF0C;
extern Overlay36SpawnedObject *func_8000590C(
    Overlay36SpawnRequest *request, s32 count, Overlay36EffectSource *source);

void overlay36SpawnFinalEffect(Overlay36EffectSource *source) {
    Overlay36SpawnRequest request;
    register Overlay36EffectState *state;

    state = source->effectState;
    if (D_8007BF0C != 0) {
        request.effectType = 0xED;
    } else {
        request.effectType = 0x90;
    }
    request.x = (s16)source->x;
    request.y = (s16)source->y;
    request.z = (s16)source->z;
    request.mode = 3;
    request.source = source;
    request.effectState = state;

    source = (Overlay36EffectSource *)func_8000590C(&request, 1, source);
    state = request.effectState;
    if (source != NULL) {
        ((Overlay36SpawnedObject *)source)->owner = 0;
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->activeAction = NULL;
    }
}
