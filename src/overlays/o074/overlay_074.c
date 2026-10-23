#include "overlays/overlay_074.h"

void overlay74Init(Overlay74Object *object, Overlay74Init *init) {
    Overlay74State *state;
    u8 *status;

    status = func_80028F54();
    state = object->state;
    if ((*status != 0) ||
        ((D_800D3128.channelMask & (1 << init->channel)) != 0)) {
        object->flags |= 0x400;
    }

    state->strength = init->strength;
    state->minimum = init->minimum;
    state->maximum = init->maximum;
    state->channel = init->channel;
    object->scale = (f32) init->scale * 0.05f * object->transform->scale;
}

void overlay74Update(Overlay74Object *object, s32 amount) {
    Overlay74HitObject *results[13];
    f32 delta;
    Overlay74State *state;
    Overlay74HitObject *hitObject;

    if (!(object->flags & 0x400)) {
        object->angle += amount << 8;
        state = object->state;
        if (func_8005776C(object->x, object->y, object->z,
                                (f32)state->strength, 1, results) != 0) {
            hitObject = results[0];
            amount = (s32)hitObject->state;
            delta = hitObject->y - object->y;
            if ((((Overlay74HitState *)amount)->strength == 0) &&
                ((f32)state->minimum < delta) &&
                (delta < (f32)state->maximum)) {
                object->flags |= 0x400;
                D_800D3128.channelMask |= 1 << state->channel;
                func_800291B4();
                amSndPlay(0x27C, 0);

                object = (Overlay74Object *)5;
                state = (Overlay74State *)D_800D3128.channelMask;
                hitObject = (Overlay74HitObject *)8;
                do {
                    if ((s32)state & (s32)hitObject) {
                        object = (Overlay74Object *)((s32)object + 1);
                    }
                    hitObject = (Overlay74HitObject *)((s32)hitObject >> 1);
                } while (hitObject != NULL);
                if ((s32)object >= 6) {
                    func_8003A680((s32)object);
                }
            }
        }
    }
}
