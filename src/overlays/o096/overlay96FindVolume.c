#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */

typedef struct Overlay96Plane {
    f32 x;
    f32 y;
    f32 z;
    f32 offset;
} Overlay96Plane;

typedef struct Overlay96Volume {
    u8 pad00[0x50];
    Overlay96Plane planes[6];
} Overlay96Volume;

extern s32 D_80078F84;
extern Overlay96Volume *gO96EntriesReloc[];

Overlay96Volume *overlay96FindVolume(f32 x, f32 y, f32 z) {
    s32 count = D_80078F84;
    s32 planeIndex;
    s32 inside;
    Overlay96Volume *volume;
    Overlay96Plane *plane;
    f32 distance;

    while (count--) {
        volume = gO96EntriesReloc[count];
        plane = volume->planes;
        inside = 1;
        planeIndex = 5;
        do {
            distance = plane->x * x + y * plane->y +
                       z * plane->z + plane->offset;
            plane++;
            if (distance < 0.0f) {
                inside = 0;
                break;
            }
        } while (planeIndex--);
        if (inside != 0) {
            return volume;
        }
    }
    return NULL;
}
