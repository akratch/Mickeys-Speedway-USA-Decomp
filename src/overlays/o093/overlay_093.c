#include "overlays/overlay_093.h"

/* Overlay 93 +0x000; exact donor scans are negative. */

void overlay93Init(Overlay93Object *object, Overlay93Config *config) {
    f32 radius = (f32)config->x;
    radius *= radius;
    object->radiusSquared = radius;
}

void overlay93Update(Overlay93Object *object, s32 updateRate) {
    Overlay93Object *target = func_80005820(0, object);

    if (target != 0) {
        f32 dx = object->x - target->x;
        f32 radiusSquared = object->radiusSquared;
        Overlay93UpdateConfig *config = object->config;
        f32 dz = object->z - target->z;

        if ((dx * dx) + (dz * dz) <= radiusSquared) {
            changeWeather(
                config->x << 8,
                config->y << 8,
                config->z << 8,
                config->red * 0x101,
                config->green * 0x101,
                config->arg5
            );
        }
    }
}
