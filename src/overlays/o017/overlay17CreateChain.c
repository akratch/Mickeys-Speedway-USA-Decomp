#include "PR/ultratypes.h"

typedef struct Overlay17Point {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay17Point;

typedef struct Overlay17Pair {
    Overlay17Point first;
    Overlay17Point second;
} Overlay17Pair;

typedef struct Overlay17Template {
    u8 byte0;
    u8 byte1;
    u8 byte2;
    u8 byte3;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
} Overlay17Template;

typedef struct Overlay17Material {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay17Material;

typedef struct Overlay17Chain {
    s16 count;
    u8 selectedBuffer;
    u8 dirty;
    Overlay17Material *material;
    f32 x;
    f32 y;
    f32 z;
    f32 oldX;
    f32 oldY;
    f32 oldZ;
    f32 radius;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    void *owner;
    Overlay17Pair *buffers[2];
    Overlay17Template *template;
} Overlay17Chain;

extern Overlay17Template gOverlay17TemplateReloc[];
extern void *func_overlay_017_F0000000_18739B8();

/* Plateau (2026-08-25): exact-size 0x310, 130 words differ from +0x0;
 * removing an unused local reduced the frame from 0x90 to 0x88 (target 0x80).
 * The flag lattice was neutral; the 40-minute permuter bottomed out at 2440. */
#ifdef NON_MATCHING
Overlay17Chain *overlay17CreateChain(
    void *owner, s32 count, Overlay17Material *materialToken, s32 materialScale,
    f32 x, f32 y, f32 z, f32 radius,
    u8 red, u8 green, u8 blue, u8 alpha) {
    s32 halfBufferBytes;
    Overlay17Chain *chain;
    Overlay17Template *destination;
    Overlay17Template *source;
    s32 index;
    s32 buffer;
    s32 vertex;
    f32 x0, y0, z0, x1, y1, z1;

    chain = (Overlay17Chain *)0x40;
    if (materialToken != (Overlay17Material *)-1) {
        chain = (Overlay17Chain *)0x140;
    }
    halfBufferBytes = count * 20;
    chain = func_overlay_017_F0000000_18739B8(
        (s32)chain + (halfBufferBytes * 2), 0x87, count);
    if (chain != 0) {

    if (materialToken != (Overlay17Material *)-1) {
        chain->material = func_overlay_017_F0000000_18739B8(materialToken);
        chain->buffers[0] = (Overlay17Pair *)((u8 *)chain + 0x140);
        chain->buffers[1] = (Overlay17Pair *)((u8 *)chain->buffers[0] + halfBufferBytes);
    } else {
        chain->material = 0;
        chain->buffers[0] = (Overlay17Pair *)((u8 *)chain + 0x40);
        chain->buffers[1] = (Overlay17Pair *)((u8 *)chain->buffers[0] + halfBufferBytes);
    }

    source = gOverlay17TemplateReloc;
    destination = (Overlay17Template *)((u8 *)chain + 0x40);
    if (chain->material != 0) {
        s32 widthScale = chain->material->width - 1;
        s32 heightScale = chain->material->height * materialScale;
        chain->template = destination;
        index = 15;
        do {
            destination->byte0 = source->byte0;
            destination->byte1 = source->byte1;
            destination->byte2 = source->byte2;
            destination->byte3 = source->byte3;
            destination->x0 = source->x0 * widthScale;
            destination->y0 = source->y0 * heightScale;
            destination->x1 = source->x1 * widthScale;
            destination->y1 = source->y1 * heightScale;
            destination->x2 = source->x2 * widthScale;
            destination->y2 = source->y2 * heightScale;
            destination++;
            source++;
        } while (index--);
    } else {
        chain->template = source;
    }

    chain->dirty = 1;
    chain->selectedBuffer = 0;
    chain->count = count;
    chain->x = x;
    chain->y = y;
    chain->z = z;
    chain->radius = radius;
    chain->red = red;
    chain->green = green;
    chain->blue = blue;
    chain->alpha = alpha;
    chain->owner = owner;
    func_overlay_017_F0000000_18739B8(
        chain, &x0, &y0, &z0, &x1, &y1, &z1);

    buffer = 1;
    do {
        Overlay17Point *point = (Overlay17Point *)chain->buffers[buffer];
        point[0].x = (s16)(s32)x0;
        point[0].y = (s16)(s32)y0;
        point[0].z = (s16)(s32)z0;
        point[0].red = chain->red;
        point[0].green = chain->green;
        point[0].blue = chain->blue;
        point[1].x = (s16)(s32)x1;
        point[1].y = (s16)(s32)y1;
        point[1].z = (s16)(s32)z1;
        point[1].red = chain->red;
        point[1].green = chain->green;
        point[1].blue = chain->blue;
        vertex = count * 2 - 1;
        if (count * 2 != 0) {
            do {
                point->alpha = 0;
                point++;
            } while (vertex--);
        }
    } while (buffer--);
    }

    return chain;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o017/overlay17CreateChain/func_overlay_017_F0000318_1873CD0.s")
#endif

/* PLATEAU-HANDOFF:overlay17CreateChain:start
 * symbol: overlay17CreateChain
 * score: 115 differing words
 * frame: 0x80
 * relocations: 7
 * first-mismatch: +0x34
 * summary: Template pointer is source on the no-material arm; source-before-destination; while(x--) loops: 130 to 115, frame 0x80 closed. Ring colour rotation remains.
 * PLATEAU-HANDOFF:overlay17CreateChain:end
 */
