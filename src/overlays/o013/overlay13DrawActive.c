#include "PR/ultratypes.h"

typedef struct O13Record {
    u8 pad00[6];
    u8 active;
    u8 pad07[5];
    f32 x;
    f32 z;
    f32 y;
    u8 pad18[0x68];
} O13Record;

typedef struct O13View {
    u8 pad00[0x0C];
    f32 x;
    f32 z;
    f32 y;
} O13View;

extern s32 gOverlay13Active;
extern s32 gOverlay13Enabled;
extern O13Record gOverlay13Records[];
extern O13View *overlay13Initialize(void);
extern void overlay13DrawRecord(O13Record *, s32, s32, s32);

/* Frame 0x168 is the two pointer pads plus indices[26] and distances[35].
 * temporaryDistance = 0 before the branch takes the first FP colour. The
 * collection walk reuses i, so that web is coloured before record. if (1)
 * is the region boundary that rematerializes the record base. The matched
 * overlay13Initialize is void and leaves the view pointer in v0. */
void overlay13DrawActive(s32 commands, s32 mtx, s32 vertices) {
    s32 i;
    void *pad0;
    void *pad1;
    s32 count;
    s32 done;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 temporaryDistance;
    s32 temporaryIndex;
    s32 indices[26];
    f32 distances[35];
    O13View *view;
    O13Record *record;

    temporaryDistance = 0.0f;
    if (gOverlay13Active != 0 && gOverlay13Enabled != 0) {
        view = overlay13Initialize();
        if (1) {
            record = gOverlay13Records;
            i = 0;
            count = 0;
            do {
                if (record->active != 0) {
                    dx = record->x - view->x;
                    dz = record->z - view->z;
                    dy = record->y - view->y;
                    indices[count] = i;
                    distances[count] = dx * dx + dz * dz + dy * dy;
                    count++;
                }
                record++;
                if (record->active != 0) {
                    dx = record->x - view->x;
                    dz = record->z - view->z;
                    dy = record->y - view->y;
                    indices[count] = i + 1;
                    distances[count] = dx * dx + dz * dz + dy * dy;
                    count++;
                }
                record++;
                i += 2;
            } while (i != 32);
        }
        if (count >= 2) {
            do {
                done = 1;
                for (i = 0; i < count - 1; i++) {
                    if (distances[i + 1] < distances[i]) {
                        temporaryDistance = distances[i];
                        distances[i] = distances[i + 1];
                        distances[i + 1] = temporaryDistance;
                        temporaryIndex = indices[i];
                        indices[i] = indices[i + 1];
                        indices[i + 1] = temporaryIndex;
                        done = 0;
                    }
                }
            } while (done == 0);
        }
        for (i = 0; i < count; i++) {
            overlay13DrawRecord(&gOverlay13Records[indices[i]], commands, mtx, vertices);
        }
    }
}
