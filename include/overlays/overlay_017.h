#ifndef OVERLAY_017_H
#define OVERLAY_017_H

#include "PR/ultratypes.h"

/* One display-list command: two words. */
typedef struct Overlay17Gfx {
    u32 w0;
    u32 w1;
} Overlay17Gfx;

/* One strip vertex; a strip segment is a pair of them. */
typedef struct Overlay17Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay17Vertex;

typedef struct Overlay17Pair {
    Overlay17Vertex first;
    Overlay17Vertex second;
} Overlay17Pair;

/* Texture-coordinate template record, sixteen per chain. */
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

/* The texture header fields CreateChain reads. */
typedef struct Overlay17Material {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay17Material;

/* The owner's transform, as CalculateEndpoints reads it. */
typedef struct Overlay17Transform {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay17Transform;

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
    Overlay17Transform *owner;
    Overlay17Vertex *buffers[2];
    Overlay17Template *template;
} Overlay17Chain;

typedef struct Overlay17Resource {
    u8 pad0[4];
    void *nested;
} Overlay17Resource;

/* The constructor copies sixteen complete texture-coordinate records. */
extern Overlay17Template gOverlay17Templates[16];

extern void pointListRPY(
    s32 mode, Overlay17Transform *transform, f32 *source, f32 *destination);
extern f32 sqrtf(f32 value);
extern void *mmAlloc(s32 size, u32 colourTag);
struct TextureHeader;
extern struct TextureHeader *texLoadTexture(s32 textureId);
/* Tier B: the nested texture and its heap owner have distinct actual
 * release providers. Partial Resource/Material views remain separate debt. */
extern void texFreeTexture(struct TextureHeader *texture);
extern void mmFree(void *allocation);
extern void texDPTextureX(Overlay17Gfx **, void *, s32, s32);

#endif
