#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct O57GridVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O57GridVertex;

typedef struct O57GridPoint {
    f32 x;
    f32 y;
    f32 z;
} O57GridPoint;

typedef struct O57GridModel {
    u8 pad00[0x40];
    O57GridPoint *points;
} O57GridModel;

typedef struct O57GridOwner {
    u8 pad00[0x68];
    O57GridModel **model;
} O57GridOwner;

typedef struct O57GridObject {
    u8 pad00[8];
    O57GridOwner *owner;
} O57GridObject;

/* Resident display-list cursor, camera context, vertex cursor and resource
 * table (renamed onto their surface entries by POSTPROCESS). */
extern Gfx *D_800D3140;
extern s32 D_800D3144;
extern O57GridVertex *D_800D3148;
extern void *D_800D31C8[];
extern u8 gOverlay57TableIndex;
extern s32 gO57GridMode184;
extern s32 gO57GridRecord160;
extern s32 gO57GridAlpha13C;
extern f32 gO57GridBlendA8;
extern f32 gO57GridBlendAC;
extern u8 D_4D4[];
extern f32 D_4E4;
extern u8 gO57GridTriangles[];

extern void camSetView(Gfx **displayList, s32 *context);
extern void texAnimateTexSprite(void *resource, void *arg1, s32 arg2, f32 *arg3, s32 updateRate);
extern O57GridObject *func_800508B4(u8 id);
extern void func_8002109C(O57GridOwner *owner);
extern void texDPTextureX(Gfx **displayList, void *resource, s32 mode, s32 flags);

#define O57_SP_VERTEX(packet, vertex, count, first)                       \
    gDma1p(packet, G_VTX, vertex,                                         \
           (((count) << 3) + ((count) << 1)) + 8,                         \
           ((count) << 3) | (((u32) (vertex)) & 6) | (first))

#define O57_SP_POLYGON(packet, address, count, textured)                 \
    {                                                                      \
        Gfx *_g = (Gfx *) (packet);                                        \
        _g->words.w0 = _SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
                       _SHIFTL(5, 24, 8) | _SHIFTL((count) << 4, 0, 16);  \
        _g->words.w1 = (u32) (address);                                    \
    }

#define O57_PHYSICAL(p) ((u32)((u8 *)(p) + 0x80000000))

#define O57_GRID_VERTEX(px, py, pz)               \
    D_800D3148->x = (px);          \
    D_800D3148->y = (py);          \
    D_800D3148->z = (pz);          \
    D_800D3148++

/* Overlay 57 text +0x1020..+0x1978, the grid quad draw. Matched 2026-10-02
 * (lane o-ovl7) from 585 masked words (size -32): the record is read as
 * points of the model at each use with the edge differences as declared
 * locals (dz is the one the target spills to its home), the midpoints and
 * the three coordinate arrays declared in the target's frame order, the
 * colour fill walks a vertex pointer beside its count, the display-list
 * commands are one packet macro each (the track.c vertex and polygon forms)
 * with the triangle addresses as constant physical offsets, and the camera
 * context is passed by address. */
void func_overlay_057_F0001020_18A4C18(s32 updateRate) {
    O57GridObject *object;
    f32 x01;
    f32 x02;
    f32 x12;
    f32 x13;
    f32 x23;
    f32 y01;
    f32 y02;
    f32 y12;
    f32 y13;
    f32 y23;
    f32 z01;
    f32 z02;
    f32 z12;
    f32 z13;
    f32 z23;
    f32 x[4];
    f32 y[4];
    f32 z[4];
    f32 dx;
    f32 dy;
    f32 dz;
    f32 blend;
    s32 i;
    O57GridVertex *vertex;
    s32 index;
    s32 renderMode;
    O57GridModel *model;

    model = NULL;
    camSetView(&D_800D3140, &D_800D3144);
    texAnimateTexSprite(D_800D31C8[62], D_4D4, 2, &D_4E4, updateRate);

    if (gO57GridMode184 == 0x2F || gO57GridMode184 == 0x4B) {
        object = func_800508B4(gO57GridMode184);
        if (object != NULL && object->owner != NULL) {
            model = *object->owner->model;
            index = gO57GridRecord160 * 4;
            blend = gO57GridBlendA8;
            renderMode = 4;
        }
        if (gO57GridMode184 == 0x4B) {
            func_8002109C(object->owner);
            gO57GridAlpha13C = 0xFF;
        }
    } else if (gO57GridMode184 == 0x4C) {
        object = func_800508B4(0x4C);
        if (object != NULL && object->owner != NULL) {
            model = *object->owner->model;
            if (gOverlay57TableIndex != 0) {
                index = 4;
            } else {
                index = 0;
            }
            renderMode = 6;
            blend = gO57GridBlendAC;
        }
        gO57GridAlpha13C = 0xFF;
    }

    if (model != NULL) {
        gDPSetPrimColor(D_800D3140++, 0, 0, 0xFF, 0xFF, 0xFF, gO57GridAlpha13C);
        O57_SP_VERTEX(D_800D3140++, (u32)D_800D3148 + 0x80000000, 9, 0);
        texDPTextureX(&D_800D3140, D_800D31C8[62], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x00]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[63], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x20]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[64], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x40]), 2, 1);
        texDPTextureX(&D_800D3140, D_800D31C8[65], renderMode, D_4E4 * 65536.0f);
        O57_SP_POLYGON(D_800D3140++, O57_PHYSICAL(&gO57GridTriangles[0x60]), 2, 1);

        dx = model->points[index].x - model->points[index + 3].x;
        dy = model->points[index].y - model->points[index + 3].y;
        dz = model->points[index].z - model->points[index + 3].z;
        x[0] = model->points[index].x + dx * blend;
        y[0] = model->points[index].y + dy * blend;
        z[0] = model->points[index].z + dz * blend;
        x[3] = model->points[index + 3].x - dx * blend;
        y[3] = model->points[index + 3].y - dy * blend;
        z[3] = model->points[index + 3].z - dz * blend;

        dx = model->points[index + 1].x - model->points[index + 2].x;
        dy = model->points[index + 1].y - model->points[index + 2].y;
        dz = model->points[index + 1].z - model->points[index + 2].z;
        x[1] = model->points[index + 1].x + dx * blend;
        y[1] = model->points[index + 1].y + dy * blend;
        z[1] = model->points[index + 1].z + dz * blend;
        x[2] = model->points[index + 2].x - dx * blend;
        y[2] = model->points[index + 2].y - dy * blend;
        z[2] = model->points[index + 2].z - dz * blend;

        x01 = x[0] + (x[1] - x[0]) * 0.5f;
        x02 = x[0] + (x[2] - x[0]) * 0.5f;
        x12 = x[1] + (x[2] - x[1]) * 0.5f;
        x13 = x[1] + (x[3] - x[1]) * 0.5f;
        x23 = x[2] + (x[3] - x[2]) * 0.5f;
        y01 = y[0] + (y[1] - y[0]) * 0.5f;
        y02 = y[0] + (y[2] - y[0]) * 0.5f;
        y12 = y[1] + (y[2] - y[1]) * 0.5f;
        y13 = y[1] + (y[3] - y[1]) * 0.5f;
        y23 = y[2] + (y[3] - y[2]) * 0.5f;
        z01 = z[0] + (z[1] - z[0]) * 0.5f;
        z02 = z[0] + (z[2] - z[0]) * 0.5f;
        z12 = z[1] + (z[2] - z[1]) * 0.5f;
        z13 = z[1] + (z[3] - z[1]) * 0.5f;
        z23 = z[2] + (z[3] - z[2]) * 0.5f;

        vertex = D_800D3148;
        for (i = 0; i < 9; i++) {
            vertex->r = 0xFF;
            vertex->g = 0xFF;
            vertex->b = 0xFF;
            vertex->a = 0xFF;
            vertex++;
        }

        O57_GRID_VERTEX(x[0], y[0], z[0]);
        O57_GRID_VERTEX(x01, y01, z01);
        O57_GRID_VERTEX(x[1], y[1], z[1]);
        O57_GRID_VERTEX(x02, y02, z02);
        O57_GRID_VERTEX(x12, y12, z12);
        O57_GRID_VERTEX(x13, y13, z13);
        O57_GRID_VERTEX(x[2], y[2], z[2]);
        O57_GRID_VERTEX(x23, y23, z23);
        O57_GRID_VERTEX(x[3], y[3], z[3]);
    }
}
