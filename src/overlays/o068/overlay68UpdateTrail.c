#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no matching fixed-point trail updater. */

typedef struct Overlay68TrailSource {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[6];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay68TrailSource;

typedef struct Overlay68TrailPoint {
    s16 x;
    s16 y;
    s16 z;
    s8 red;
    s8 green;
    s8 blue;
    u8 timer;
} Overlay68TrailPoint;

typedef struct Overlay68Trail {
    Overlay68TrailSource *source;
    s8 kind;
    s8 delay;
    s16 generation;
    s16 elapsed;
    s16 index;
    Overlay68TrailPoint *points;
} Overlay68Trail;

extern Overlay68Trail *gOverlay68Entry;
extern s32 D_800C947C;
extern void overlay68FinishEntry(s8 delay, Overlay68Trail *trail, s32 updateRate);

void overlay68UpdateTrail(s32 updateRate) {
    Overlay68Trail *trail;
    Overlay68TrailSource *source;
    Overlay68TrailPoint *point;
    Overlay68Trail * volatile savedTrail;
    s8 delay;
    s16 index;
    s32 threshold;

    trail = gOverlay68Entry;
    if (trail != 0) {
        source = trail->source;
        if (source != 0) {
            index = trail->index;
            if (index < 0x4AF) {
                point = &trail->points[index];
                if (D_800C947C != 0) {
                    point->red = source->x >> 8;
                    point->green = source->y >> 8;
                    point->blue = source->z >> 8;
                    point->x = source->worldX;
                    point->y = source->worldY;
                    point->z = source->worldZ;
                    return;
                }

                delay = trail->delay;
                if (delay == 0) {
                    trail->elapsed += updateRate;
                } else {
                    trail->delay = delay - updateRate;
                    if (trail->delay <= 0) {
                        savedTrail = trail;
                        overlay68FinishEntry(delay, trail, updateRate);
                        trail = savedTrail;
                        trail->delay = 0;
                        return;
                    }
                }

                point->timer += updateRate;
                index = trail->index;
                threshold = index < 5 ? 6 : 12;
                if (point->timer >= threshold) {
                    trail->index = index + 1;
                    point++;
                    point->red = source->x >> 8;
                    point->green = source->y >> 8;
                    point->blue = source->z >> 8;
                    point->x = source->worldX;
                    point->y = source->worldY;
                    point->z = source->worldZ;
                    point->timer = 0;
                }
            }
        }
    }
}
