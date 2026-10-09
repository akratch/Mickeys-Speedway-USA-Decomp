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

/* YAML-owned initialized data and stock sizeof prove sixteen 16-byte records. */
extern Overlay17Template gOverlay17TemplateReloc[16];
extern void *mmAlloc(s32 size, u32 colourTag);
struct TextureHeader;
extern struct TextureHeader *texLoadTexture(s32 textureId);
struct Overlay17ChainHead;
extern void overlay17CalculateEndpoints(struct Overlay17ChainHead *chain, f32 *x0, f32 *y0, f32 *z0,
                                        f32 *x1, f32 *y1, f32 *z1);


/* Matched 2026-10-07 (lane i-near). The half-buffer size is a local that
 * the allocation reads, kept alive past the call by a no-op or-with-zero
 * (deleted by the compiler) so that uopt does not substitute `count * 20`
 * into it; the two buffer arms read the expression itself. That makes the
 * size an expression web whose definition is evaluated into a ring temp and
 * copied (as1 forwards the temp into the allocation argument and the spill
 * store and deletes the copy), with count loaded into the web's own register
 * as scratch and the spill in the compiler's temp slot, as shipped. Earlier
 * steps, in order: one shared loop counter (x-ovlb, 83 -> 65); real callee
 * declarations and arities (65 -> 45); chain->count stored before dirty and
 * selectedBuffer, source/destination/template assigned inside the material
 * arm, the else arm storing the global directly, the inherited `red & 0xFF`
 * mask dropped (f-o069, 36 -> 3); the or-zero kill (3 -> 0). */
Overlay17Chain *overlay17CreateChain(
    void *owner, s32 count, Overlay17Material *materialToken, s32 materialScale,
    f32 x, f32 y, f32 z, f32 radius,
    u8 red, u8 green, u8 blue, u8 alpha) {
    s32 padFrame;
    Overlay17Template *source;
    s32 buffer;
    Overlay17Template *destination;
    s32 halfBufferBytes;
    s32 index;
    f32 x0, y0, z0, x1, y1, z1;
    Overlay17Chain *chain;

    chain = (Overlay17Chain *)0x40;
    if (materialToken != (Overlay17Material *)-1) {
        chain = (Overlay17Chain *)0x140;
    }
    halfBufferBytes = count * 20;
    chain = mmAlloc(
        (s32)chain + (halfBufferBytes * 2), 0x87);
    halfBufferBytes |= 0;
    if (chain != 0) {

    if (materialToken != (Overlay17Material *)-1) {
        chain->material = (Overlay17Material *)texLoadTexture((s32)materialToken);
        chain->buffers[0] = (Overlay17Pair *)((u8 *)chain + 0x140);
        chain->buffers[1] = (Overlay17Pair *)((u8 *)chain->buffers[0] + count * 20);
    } else {
        chain->material = 0;
        chain->buffers[0] = (Overlay17Pair *)((u8 *)chain + 0x40);
        chain->buffers[1] = (Overlay17Pair *)((u8 *)chain->buffers[0] + count * 20);
    }

    if (chain->material != 0) {
        s32 widthScale;
        s32 heightScale;
        source = gOverlay17TemplateReloc;
        destination = (Overlay17Template *)((u8 *)chain + 0x40);
        chain->template = destination;
        widthScale = chain->material->width - 1;
        heightScale = chain->material->height * materialScale;
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
        chain->template = gOverlay17TemplateReloc;
    }

    chain->count = count;
    chain->dirty = 1;
    chain->selectedBuffer = 0;
    chain->x = x;
    chain->y = y;
    chain->z = z;
    chain->radius = radius;
    chain->red = red;
    chain->green = green;
    chain->blue = blue;
    chain->alpha = alpha;
    chain->owner = owner;
    overlay17CalculateEndpoints(
        (struct Overlay17ChainHead *)chain, &x0, &y0, &z0, &x1, &y1, &z1);

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
        index = count * 2 - 1;
        if (count * 2 != 0) {
            do {
                point->alpha = 0;
                point++;
            } while (index--);
        }
    } while (buffer--);
    }

    return chain;
}
