#include "overlays/overlay_017.h"

/*
 * Overlay 17: a textured ribbon trailing its owner. One translation unit;
 * overlay17CalculateEndpoints is unexported and static, the other four are
 * the module's exports.
 */

#define S(v,s,w) (((u32)(v)&((1U<<(w))-1U))<<(s))
#define PRIM(p) { Overlay17Gfx *m=(Overlay17Gfx *)(p); m->w0=0xFA000000; m->w1=0xFFFFFFFF; }
#define VTX(p,a,n) { Overlay17Gfx *m=(Overlay17Gfx *)(p); m->w0=S(4,24,8)|S(((n)<<3)|((u32)(a)&6),16,8)|S(((n)<<3)+((n)<<1)+8,0,16); m->w1=(u32)(a); }
#define STRIP(p,a,n,t) { Overlay17Gfx *m=(Overlay17Gfx *)(p); m->w0=S(5,24,8)|S((((n)-1)<<4)|(t),16,8)|S((n)<<4,0,16); m=(Overlay17Gfx *)((u32)m | 0); m->w1=(u32)(a); }

/* Matched 2026-10-02 (lane x-ovlb). Four edits closed it from 121 words:
 * - the TU takes -Wab,-r4300_mul: the target's `mul.s; nop; mul.s` pair and
 *   the rotated branch-likely transform loop are that flag's output;
 * - `scale` is the one length variable, assigned the squared length and then
 *   overwritten; it is live into the sqrtf call, so IDO stores it once and
 *   never reloads it (the old volatile spelling reloaded it three times);
 * - the tail is written in source order (scale both deltas, store the old
 *   position, clear dirty, then derive the far endpoint), which is the order
 *   the float ring needs;
 * - both loops share one index. The transform loop compares element
 *   addresses so IDO keeps the target's unsigned pointer test instead of
 *   rewriting it to an equality, and with no separate cursor variable the
 *   function needs one fewer register cell, which puts `scale`'s home at the
 *   target's sp+0x24.
 * The `transform != 0` test in the no-owner arm is dead, and the target
 * keeps it. */
static void overlay17CalculateEndpoints(
    Overlay17Chain *chain, f32 *outX0, f32 *outY0, f32 *outZ0,
    f32 *outX1, f32 *outY1, f32 *outZ1) {
    s32 index;
    f32 deltaX;
    f32 deltaZ;
    f32 scale;
    f32 points[6];
    Overlay17Transform *transform;

    if (chain != 0) {
        transform = chain->owner;
        if (transform != 0) {
            points[0] = chain->x * transform->scale;
            points[1] = chain->y * transform->scale;
            points[2] = chain->z * transform->scale;
            pointListRPY(1, transform, points, points);
            points[0] += transform->x;
            points[1] += transform->y;
            points[2] += transform->z;

            if (chain->dirty == 0) {
                deltaX = points[0] - chain->oldX;
                deltaZ = points[2] - chain->oldZ;
            } else {
                deltaX = 0.0f;
                deltaZ = 0.0f;
            }
            scale = (deltaX * deltaX) + (deltaZ * deltaZ);
            if (scale > 0.0f) {
                scale = (chain->radius * transform->scale) / sqrtf(scale);
            }
            deltaX *= scale;
            deltaZ *= scale;
            chain->oldX = points[0];
            chain->oldY = points[1];
            chain->oldZ = points[2];
            chain->dirty = 0;
            points[3] = points[0] + deltaZ;
            points[4] = points[1];
            points[5] = points[2] - deltaX;
            points[0] -= deltaZ;
            points[2] += deltaX;
        } else {
            points[0] = chain->x - chain->radius;
            points[1] = chain->y;
            points[2] = chain->z;
            points[3] = chain->x + chain->radius;
            points[4] = chain->y;
            points[5] = chain->z;
            if (transform != 0) {
                pointListRPY(2, transform, points, points);
                for (index = 0; &points[index] < &points[6]; index += 3) {
                    points[index + 0] = (points[index + 0] * transform->scale) + transform->x;
                    points[index + 1] = (points[index + 1] * transform->scale) + transform->y;
                    points[index + 2] = (points[index + 2] * transform->scale) + transform->z;
                }
            }
        }
    } else {
        index = 5;
        do {
            points[index] = 0.0f;
        } while (index--);
    }

    *outX0 = points[0];
    *outY0 = points[1];
    *outZ0 = points[2];
    *outX1 = points[3];
    *outY1 = points[4];
    *outZ1 = points[5];
}

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
    void *owner, s32 count, s32 textureId, s32 materialScale,
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
    if (textureId != -1) {
        chain = (Overlay17Chain *)0x140;
    }
    halfBufferBytes = count * 20;
    chain = mmAlloc(
        (s32)chain + (halfBufferBytes * 2), 0x87);
    halfBufferBytes |= 0;
    if (chain != 0) {

    if (textureId != -1) {
        chain->material = (Overlay17Material *)texLoadTexture(textureId);
        chain->buffers[0] = (Overlay17Vertex *)((u8 *)chain + 0x140);
        chain->buffers[1] = (Overlay17Vertex *)((u8 *)chain->buffers[0] + count * 20);
    } else {
        chain->material = 0;
        chain->buffers[0] = (Overlay17Vertex *)((u8 *)chain + 0x40);
        chain->buffers[1] = (Overlay17Vertex *)((u8 *)chain->buffers[0] + count * 20);
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
        chain, &x0, &y0, &z0, &x1, &y1, &z1);

    buffer = 1;
    do {
        Overlay17Vertex *point = chain->buffers[buffer];
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

/* DKR v77/v80 and JFG checks found only generic nested-release semantics. */
void overlay17ReleaseChain(Overlay17Resource *resource) {
    if (resource != NULL) {
        if (resource->nested != NULL) {
            overlay17ReleaseReloc(resource->nested);
        }
        overlay17ReleaseReloc(resource);
    }
}

/*
 * Matched 2026-10-02. The count is read from the chain each time, with no
 * local copy. The two buffer indices are unsigned and the copy count is a
 * signed multiply of the same unsigned (count - 1): the two index scalings
 * then share one constant register and multiply by it, while the copy
 * count's constant is a different type, has one use, and is expanded into
 * shifts; the shared unsigned subtraction keeps the multiply from being
 * distributed over it.
 */
void overlay17AdvanceChain(Overlay17Chain *chain, s32 useAlpha) {
    s32 n;
    s32 alpha;
    f32 x0, y0, z0, x1, y1, z1;
    u16 *src;
    u16 *dst;
    Overlay17Vertex *vtx;

    if (chain == 0) {
        return;
    }

    src = (u16 *)&chain->buffers[chain->selectedBuffer][(chain->count - 1U) << 1];
    chain->selectedBuffer ^= 1;
    dst = (u16 *)&chain->buffers[chain->selectedBuffer][(u32)chain->count << 1];
    n = (s32)(chain->count - 1U) * 10;
    while (n--) {
        *--dst = *--src;
    }
    vtx = chain->buffers[chain->selectedBuffer];
    if (useAlpha) {
        alpha = chain->alpha;
    } else {
        alpha = 0;
    }
    overlay17CalculateEndpoints(chain, &x0, &y0, &z0, &x1, &y1, &z1);
    vtx[0].x = x0;
    vtx[0].y = y0;
    vtx[0].z = z0;
    vtx[0].red = chain->red;
    vtx[0].green = chain->green;
    vtx[0].blue = chain->blue;
    vtx[0].alpha = alpha;
    vtx[1].x = x1;
    vtx[1].y = y1;
    vtx[1].z = z1;
    vtx[1].red = chain->red;
    vtx[1].green = chain->green;
    vtx[1].blue = chain->blue;
    vtx[1].alpha = alpha;
    vtx += 2;
    n = chain->count - 1;
    while (n--) {
        if (vtx[0].alpha != 0) {
            alpha = (chain->alpha * n) / (chain->count - 1);
            vtx[0].alpha = alpha;
            vtx[1].alpha = alpha;
        }
        vtx += 2;
    }
}

/* Tier A: 119 stock-IDO words match the linked retail range.
 * STRIP deliberately reassigns its initialized cursor through unsigned OR zero.
 * On this 32-bit ABI that preserves every pointer bit and adds no memory access.
 * It gives the cursor two definitions and five references, lowering its measured
 * allocation save from 30 to 25; IDO then removes the operation. A plain copy
 * gives the wrong allocation. See docs/cleanup-queue.md for naturalization. */
void overlay17DrawStrip(Overlay17Gfx **commands, Overlay17Chain *strip) {
    Overlay17Pair *pair;
    Overlay17Pair *previous;
    Overlay17Pair *start;
    s32 textured;
    s32 vertices;
    s32 flush;
    u32 remaining;
    u32 segment;
    if (strip == 0) return;
    segment=0x80000000U;
    textured = strip->material ? 1 : 0;
    textured |= (texDPTextureX(commands, strip->material, 0x1F, 0), 0);
    PRIM((*commands)++);
    vertices=strip->count;
    remaining=vertices;
    previous=0;
    pair=(Overlay17Pair *)strip->buffers[strip->selectedBuffer];
    start=0; vertices=0; flush=0;
    /* Post-decrement, not a separate test-then-decrement: cfe reuses ONE temp
     * symbol for this entry test and for the do-while's own `remaining--`, so
     * the two share a single web instead of two. That one merge moves the
     * prologue's whole colour assignment onto the target's and empties the
     * structural residual: 14 -> 3 masked words, zero size delta. */
    if (remaining--) {
        do {
            vertices += 2;
            if (remaining == 0) {
                flush=1;
                goto check_flush;
            }
            if (pair->first.alpha == 0) {
                flush=1;
                goto check_flush;
            }
            if (start == 0) {
                if (previous != 0) { start=previous; vertices=4; }
                else { start=pair; vertices=2; }
            }
            if (vertices >= 17) flush=1;
check_flush:
            if (flush) {
                flush=0;
                if (start != 0) {
                    VTX((*commands)++, (void *)((u32)start+segment), vertices);
                    STRIP((*commands)++, (void *)((u32)strip->template+segment), vertices-2,
                          textured);
                    start=0;
                }
                vertices=0;
            }
            previous=pair++;
        } while (remaining--);
    }
    pair = (Overlay17Pair *)(*commands)++;
    ((Overlay17Gfx *)pair)->w1 = 0;
    ((Overlay17Gfx *)pair)->w0 = 0xE7000000;
}

