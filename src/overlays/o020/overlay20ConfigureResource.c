#include "PR/ultratypes.h"

typedef struct Overlay20Resource {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s16 stepX;
    s16 columns;
    s16 rows;
    s16 value0E;
    s16 value10;
    u8 pad12;
    s8 current;
    f32 start;
    f32 end;
    f32 scale;
    void *buffers[3];
    void *owner;
} Overlay20Resource;

extern void *mmAlloc(s32 size, u32 colourTag);
extern void mmFree(void *data);
extern void *func_overlay_020_F000038C_1876964(void *grid);

Overlay20Resource *overlay20ConfigureResource(
    Overlay20Resource *resource, s32 x, s32 y, s32 width, s32 height,
    s32 stepX, s32 columns, s32 rows, void *owner, s32 value0E,
    s32 value10, s32 start, s32 current, s32 end, s32 scaleDivisor) {
    if (resource == 0) {
        resource = mmAlloc(0x30, 0x87);
        if (resource != 0) {
            resource->buffers[0] = 0;
            resource->buffers[1] = 0;
            resource->buffers[2] = 0;
        }
    } else if ((columns != resource->columns) || (rows != resource->rows)) {
        mmFree(resource->buffers[0]);
        mmFree(resource->buffers[1]);
        mmFree(resource->buffers[2]);
        resource->buffers[0] = 0;
        resource->buffers[1] = 0;
        resource->buffers[2] = 0;
    }

    if (resource != 0) {
        resource->x = x;
        resource->y = y;
        resource->width = width;
        resource->height = height;
        resource->stepX = stepX;
        resource->columns = columns;
        resource->rows = rows;
        resource->value0E = value0E;
        resource->value10 = value10;
        resource->current = current;
        resource->start = (f32)(current - start);
        resource->end = (f32)(end - current);
        resource->scale = 1.0f;
        if (scaleDivisor > 0) {
            resource->scale /= (f32)scaleDivisor;
        }
        resource->owner = owner;
        resource = func_overlay_020_F000038C_1876964(resource);
    }
    return resource;
}
