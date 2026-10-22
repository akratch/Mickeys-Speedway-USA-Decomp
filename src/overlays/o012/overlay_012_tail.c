#include "overlays/overlay_012.h"

/* Overlay 12, ADR 0006 consolidation: default-O2 tail. */

void func_overlay_012_F00001B4_186D434(f32 x, f32 y, f32 z, f32 x2, f32 y2,
                                       f32 z2, s32 scale, s32 type, f32 value) {
    Overlay12Effect *effect;
    s32 i;

    if (gOverlay12Ready == 0) {
        overlay12Initialize();
    }
    if (gOverlay12EffectCount < 64) {
        effect = gOverlay12Effects;
        for (i = 0; i < 64; i++, effect++) {
            if (effect->active == 0) {
                break;
            }
        }
        if (i < 64) {
            effect->x2 = x2;
            effect->y2 = y2;
            effect->z2 = z2;
            effect->value = value;
            effect->active = 1;
            effect->lifetime = 300;
            effect->x1 = x;
            effect->y1 = y;
            effect->zero = 0.0f;
            effect->z1 = z;
            effect->x0 = x;
            effect->y0 = y;
            effect->z0 = z;
            effect->kind1 = mathRnd(0, 1);
            effect->scaleX = scale << 5;
            effect->scaleY = scale << 5;
            effect->type = type;
            effect->kind2 = mathRnd(0, 2);
            gOverlay12EffectCount++;
        }
    }
}
