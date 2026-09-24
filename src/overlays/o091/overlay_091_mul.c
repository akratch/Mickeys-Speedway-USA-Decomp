#include "overlays/overlay_091.h"

/* This TU retains the module's required -Wab,-r4300_mul compiler boundary. */

/* Each call names its retail callee; the relocations stay filtered, so no
 * name here is linked, and each spelling is the identity the shipped table
 * records at its site. All keep the old proxy's unprototyped f32 shape. */
extern f32 func_80000F94();
extern f32 func_80006EA0();
extern f32 func_8002A8BC();
extern f32 func_8002A8C0();
extern f32 func_overlay_007_F0000DBC_185CC44();

void overlay91UpdateTimeline(Overlay91TimelineObject *object, s32 elapsed) {
    Overlay91TimerState *state;
    s32 animation;
    s32 value;
    s32 fraction;
    u32 count;
    f32 ratio;
    Overlay91GraphOwner *owner;
    Overlay91GraphRecord *record;
    f32 maximum = 144.0f;
    register f32 minimum = -225.0f;

    state = object->state;
    if (elapsed != 0) {
        do {
            switch (state->mode) {
            case 0:
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 180) {
                    elapsed = value - 180;
                    state->timer = 0;
                    state->mode = 1;
                } else {
                    elapsed = 0;
                }
                break;
            case 1: {
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    object->minValue = 0.0f;
                    object->currentValue = 0.0f;
                    object->maxValue = maximum;
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 2;
                } else {
                    fraction = (value << 14) / 60;
                    elapsed = 0;
                    ratio = func_8002A8C0(fraction);
                    object->minValue = ((-(-225.0f)) * ratio) + minimum;
                    object->currentValue =
                        ((0.0f - 0.0f) * ratio) + 0.0f;
                    object->maxValue = object->currentValue + maximum;
                }
                break;
            }
            case 2:
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 120) {
                    object->value0 = -0x8000;
                    elapsed = value - 120;
                    state->timer = 0;
                    state->mode = 3;
                    func_80000F94(0x12, 0);
                    func_overlay_007_F0000DBC_185CC44(0x1D);
                } else {
                    fraction = (value << 14) / 120;
                    elapsed = 0;
                    ratio = func_8002A8C0(fraction);
                    object->value0 = (s16)(32768.0f * ratio);
                }
                break;
            case 3:
                animation = 1;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 4;
                    func_80000F94(0x13, 0);
                } else {
                    elapsed = 0;
                }
                break;
            case 4:
                animation = 2;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 5;
                    func_80000F94(0x14, 0);
                    func_overlay_007_F0000DBC_185CC44(0x1E);
                    overlay91GlobalA = 0;
                } else {
                    elapsed = 0;
                }
                break;
            case 5:
                animation = 3;
                state->timer += elapsed;
                if (state->timer >= 30) {
                    overlay91GlobalA = 0x83;
                } else if (state->timer >= 15) {
                    overlay91GlobalA = 0x84;
                }
                if (state->timer >= 60) {
                    elapsed = state->timer - 60;
                    state->timer = 0;
                    state->mode = 6;
                    func_80000F94(0x15, 0);
                    func_overlay_007_F0000DBC_185CC44(0x1F);
                    overlay91GlobalB = 0;
                } else {
                    elapsed = 0;
                }
                break;
            case 6:
                animation = 4;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 7;
                }
                break;
            case 7: {
                animation = 4;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    func_80006EA0(object);
                    return;
                } else {
                    fraction = (value << 14) / 60;
                    elapsed = 0;
                    ratio = func_8002A8BC(fraction);
                    object->minValue = 225.0f + (minimum * ratio);
                    object->currentValue =
                        ((0.0f - 0.0f) * ratio) + 0.0f;
                    object->maxValue = object->currentValue + maximum;
                }
                break;
            }
            }
        } while (elapsed != 0);
    }

    owner = ((Overlay91GraphRoot *)object->graphOwner)->owner;
    if (owner != 0) {
        record = owner->records;
        if (record != 0) {
            count = owner->header->count;
            if (count--) {
                do {
                    if (record->flags & 0x00100000) {
                        record->value = (s16)(animation << 8);
                    }
                    record++;
                } while (count--);
            }
        }
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay91Render(Overlay91Gfx **displayList, void *renderContext,
                     u32 renderArg, Overlay91RenderObject *object) {
    void *buffer;
    u32 width;
    u32 center;

    if (overlay91CanRenderReloc() == 0) {
        object->alpha = 0xFF;
        overlay91GetDimensionsReloc(&buffer, &width);
        center = width >> 1;
        overlay91DrawBandReloc(displayList, buffer, width, 0, center - 20,
                               buffer, center + 20);
        overlay91BeginRenderReloc(displayList, renderContext);
        overlay91EndRenderReloc(displayList);
        object->flags &= ~0x0400;
        overlay91RenderObjectReloc(displayList, renderContext, renderArg,
                                   object);
        object->flags |= 0x0400;
    }
}
