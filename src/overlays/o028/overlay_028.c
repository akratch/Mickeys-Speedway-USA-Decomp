#include "overlays/overlay_028.h"

/* Overlay 28, ADR 0006 consolidation: one translation unit in ROM order. */

/* DKR v77/v80 have only generic double-buffered particle reset relatives. */
void overlay28ResetBuffer(state, count)
Overlay28ResetState *state;
s32 count;
{
    s32 color;
    Overlay28Entry *entry;

    count = 0x10;
    state->bufferIndex ^= 1;
    entry = state->entries[state->bufferIndex];
    color = 0xFF;
    do {
        entry->x = 0;
        entry->y = 0;
        entry->z = 0;
        entry->r = color;
        entry->g = color;
        entry->b = color;
        entry->a = color;
        entry++;
    } while (count--);
}

void overlay28UpdateVertices(Overlay28VertexState *state) {
    Overlay28Entry *point;
    s32 angle;
    s32 xAngle;
    s32 yAngle;
    s32 remaining;
    f32 xScale;
    f32 yScale;
    f32 xWave;
    f32 scale;

    state->bufferIndex ^= 1;
    point = state->points[state->bufferIndex];
    angle = 0;
    xAngle = state->xAngle;
    xScale = state->xScale;
    yAngle = state->yAngle;
    yScale = state->yScale;
    remaining = 15;
    do {
        xWave = ext_o0_2a470(xAngle);
        scale = (ext_o0_2a470(yAngle) * yScale) +
                ((8.0f * state->baseScale) + (xScale * xWave));
        xAngle += state->xStep;
        yAngle += state->yStep;
        point->x = (s32)(ext_o0_2a470(angle) * scale);
        point->y = (s32)(ext_o0_2a46c(angle) * scale);
        angle += 0x1000;
        point++;
    } while (remaining--);
}

/* The shipped relocation for this field names the start of overlay 28's
   initialized data (module +0x7F0), not overlay28ResetBuffer at +0: the
   pointer is set to the data base, whatever the field is. */
extern u8 gOverlay28DataBase[];

void overlay28InitializeWork(Overlay28Owner *owner, Overlay28Source *source) {
    Overlay28Work *work;

    work = owner->work;
    work->related = source->related;
    work->x = source->x;
    work->y = source->y;
    work->z = source->z;
    work->valueA = source->valueA;
    work->valueB = source->valueB;
    work->angleA = 0;
    work->angleB = 0;
    work->stepA = 0x2000;
    work->stepB = 0x4000;
    work->stepC = 0x1000;
    work->stepD = -0x2000;
    work->reset = (void (*)())gOverlay28DataBase;
    work->scaleA = 4.0f;
    work->scaleB = 2.0f;
    ext_o0_36630(work->object);
    work->intensity = 0;
    work->bufferIndex = 0;
    overlay28ResetBuffer(work);
    overlay28ResetBuffer(work);
    if (source->related != 0) {
        owner->x = ((Overlay28Owner *)source->related)->x;
        owner->y = ((Overlay28Owner *)source->related)->y;
        owner->z = ((Overlay28Owner *)source->related)->z;
        owner->angle = ((Overlay28Owner *)source->related)->angle;
    }
}

void overlay28UpdateWork(Overlay28Owner *owner, s32 updateRate) {
    Overlay28Work *work;
    Overlay28Context *context;
    Overlay28Owner *related;
    s16 sourceIntensity;

    work = owner->work;
    related = work->related;
    if (related != 0) {
        context = (Overlay28Context *)related->work;
        work->angleA += work->stepC * updateRate;
        work->angleB += work->stepD * updateRate;
        ext_o0_29e00(work->object);
        sourceIntensity = context->intensity;
        if (sourceIntensity < 0) {
            work->intensity = 0;
        } else if (sourceIntensity < 0x80) {
            work->intensity = sourceIntensity * 2;
        } else {
            work->intensity += updateRate * 0x10;
            if (work->intensity >= 0x101) {
                work->intensity = 0x100;
            }
        }
        work->intensity = (work->intensity * related->intensityScale) >> 8;
        owner->x = related->x;
        owner->y = related->y;
        owner->z = related->z;
        owner->angle = related->angle;
        overlay28UpdateVertices((Overlay28VertexState *)work);
        if (work->handle == 0) {
            ext_o0_2b90(0x1C2, owner->x, owner->y, owner->z, 1,
                        &work->handle);
        } else {
            s16 alpha;
            ext_o0_2d70(work->handle, owner->x, owner->y, owner->z);
            alpha = work->intensity >> 1;
            if (alpha >= 0x80) {
                alpha = 0x7F;
            }
            ext_o0_2c4c(work->handle, alpha & 0xFF);
        }
    }
    if ((owner->suppressRelease == 0) &&
        ((work->intensity == 0) || (work->related == 0))) {
        ext_o0_6a50(owner);
    }
}

/*
 * PROVENANCE: Overlay28Gfx.words and the _SHIFTL/display-list macro spelling
 * were adapted from Diddy Kong Racing and Jet Force Gemini include/f3ddkr.h.
 * Mickey's own bytes decide every type, constant, command and operation order.
 */
#define _SHIFTL(v, s, w) \
    ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define VERTEX_ADDRESS(work) \
    ((u32)(work) + ((u32)(work)->bufferIndex * 0xAAU) + 0x80000044U)
#define WRITE_PRIM(pkt, rgba) { \
    Overlay28Gfx *_g = (Overlay28Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFA, 24, 8); \
    _g->words.w1 = rgba; \
}
#define WRITE_ENV(pkt, rgba) { \
    Overlay28Gfx *_g = (Overlay28Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFB, 24, 8); \
    _g->words.w1 = rgba; \
}
#define WRITE_VERTEX(pkt, address) { \
    Overlay28Gfx *_g = (Overlay28Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(4, 24, 8) | \
                   _SHIFTL((((address) & 6) | 0x88), 16, 8) | \
                   _SHIFTL(0xB2, 0, 16); \
    _g->words.w1 = (u32)(address); \
}
#define WRITE_DL(pkt, address) { \
    Overlay28Gfx *_g = (Overlay28Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(5, 24, 8) | _SHIFTL(0xF1, 16, 8) | \
                   _SHIFTL(0x100, 0, 16); \
    _g->words.w1 = (u32)(address); \
}
#define WRITE_SYNC(pkt) { \
    Overlay28Gfx *_g = (Overlay28Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xE7, 24, 8); \
    _g->words.w1 = 0; \
}

void func_overlay_028_F00004D8_187CDA8(Overlay28Gfx **dl, void *matrixArg,
                                        Overlay28Owner *owner) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquared;
    f32 ratio;
    Overlay28Transform transformHome;
    void *matrix;
    Overlay28Work *work;
    Overlay28Owner *related;
    Overlay28View *view;
    s32 mask;

    view = ext_o0_241dc();
    work = owner->work;
    related = work->related;
    if (related != 0) {
        ext_o0_29e00(1, related, &work->x, &transformHome.x);
        transformHome.x += related->x;
        transformHome.y += related->y;
        transformHome.z += related->z;
        dx = view->x - transformHome.x;
        dy = view->y - transformHome.y;
        dz = view->z - transformHome.z;
        distanceSquared = (dy * dy) + ((dx * dx) + (dz * dz));
        ratio = distanceSquared;
        if (distanceSquared > 0.0f) {
            ratio = work->valueA / ext_o0_6ec00(distanceSquared);
        }
        transformHome.pitch = -view->pitch;
        transformHome.yaw = view->yaw;
        transformHome.roll = 0;
        transformHome.scale = ext_o0_9ab8(related) * 0.125f;
        transformHome.x += dx * ratio;
        transformHome.y += dy * ratio;
        transformHome.z += dz * ratio;
        matrix = *owner->matrix;
        ext_o0_2409c(dl, matrixArg, &transformHome, 1.0f, 0.0f);
        ext_o0_34554(dl, matrix, 0x214, 0);

        mask = -0x100;
        WRITE_PRIM((*dl)++, (work->intensity & 0xFF) | mask);
        WRITE_ENV((*dl)++,
                  _SHIFTL(work->red, 24, 8) |
                  _SHIFTL(work->green, 16, 8) |
                  _SHIFTL(work->blue, 8, 8) | 0xFF);
        WRITE_VERTEX((*dl)++, VERTEX_ADDRESS(work));
        WRITE_DL((*dl)++, gOverlay28DisplayList28);
        WRITE_SYNC((*dl)++);
        if (work->intensity != 0xFF) {
            WRITE_PRIM((*dl)++, -1);
        }
        WRITE_ENV((*dl)++, mask);
        ext_o0_241bc(dl);
    }
}
