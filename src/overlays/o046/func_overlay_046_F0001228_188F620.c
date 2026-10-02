#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay46Particle {
    s16 value00;
    s16 value02;
    s16 value04;
    s16 angle06;
    f32 scale08;
    f32 baseX0C;
    f32 baseY10;
    f32 value14;
    u16 angle18;
    s16 angle1A;
    f32 positionX1C;
    f32 positionY20;
    f32 progress24;
    f32 value28;
    f32 targetX2C;
    f32 targetY30;
    s16 angle34;
    s16 variant36;
    void *resource38;
} Overlay46Particle;

typedef struct Overlay46DrawPoint {
    s16 value00;
    s16 value02;
    s16 value04;
    s16 angle06;
    f32 scale08;
    f32 baseX0C;
    f32 baseY10;
    f32 value14;
    u8 pad18[0x10];
    f32 value28;
    u8 pad2C[8];
} Overlay46DrawPoint;

typedef struct Overlay46Emitter {
    f32 x;
    f32 y;
    f32 z;
    f32 speed;
    s16 angle;
    s32 vertexCount;
} Overlay46Emitter;

typedef struct Overlay46Spark {
    f32 x;
    f32 y;
    f32 z;
    f32 velocityX;
    f32 velocityY;
    s16 age;
    s16 alpha;
} Overlay46Spark;

typedef struct Overlay46TrailVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay46TrailVertex;

extern s16 D_194;
extern s16 D_19C;
extern s16 D_494;
extern Overlay46TrailVertex D_498[];
extern Overlay46Emitter *D_1450;
extern Overlay46Spark D_1458[200];
extern s16 D_2718;
extern u8 D_230[];

extern Gfx *gDisplayListHead;
extern void *gOverlay46MatrixHead;
extern void *gOverlay46VertexHead;
extern void *D_800D31C8[];

extern void amSndPlay(s32 soundId, void *handle);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void camStandardOrtho(void *commands, void *matrices);
extern void func_80023A08(void *commands, void *matrices, void *vertices,
                          Overlay46DrawPoint *particle, void *resource,
                          s32 flags, s32 alpha);
extern void func_800349A4(void *commands, void *texture, s32 flags, s32 parameter);

#define O46_SP_VERTEX(packet, vertex, count, first)                       \
    gDma1p(packet, G_VTX, vertex,                                         \
           (((count) << 3) + ((count) << 1)) + 8,                         \
           ((count) << 3) | (((u32) (vertex)) & 6) | (first))

#define O46_SP_POLYGON(packet, address, count, textured)                 \
    {                                                                      \
        Gfx *_g = (Gfx *) (packet);                                        \
        _g->words.w0 = _SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
                       _SHIFTL(5, 24, 8) | _SHIFTL((count) << 4, 0, 16);  \
        _g->words.w1 = (u32) (address);                                    \
    }

#define O46_PHYSICAL(p) ((u32)((u8 *)(p) + 0x80000000))

/* Overlay 46 text +0x1228..+0x195C, the trail and spark draw. Matched
 * 2026-10-02 (lanes o-ovl7, p-ovl8) from 415 masked words at -96, rewritten
 * from the target listing in the matched sibling
 * func_overlay_046_F0000874_188EC6C's shape: the draw point is a 0x34-byte
 * stack struct, the cursors are passed by address, each command is one
 * packet macro, the emitter is read through D_1450 at every use, and the
 * spawn and draw loops are indexed. The last 8 words were the batch vertex
 * count n+2: it is a variable k assigned on its own line (a0 in the loop),
 * and the remainder block spells `count % 16` at each use with no variable,
 * so its k web is numbered ahead of the expression webs (v1). */
void func_overlay_046_F0001228_188F620(s32 updateRate) {
    Overlay46TrailVertex *vertex;
    s32 i;
    s32 j;
    s32 batches;
    s32 remaining;
    f32 halfX;
    f32 halfY;
    Overlay46DrawPoint point;
    s32 count;
    s32 k;

    if (D_19C < D_494) {
        D_494 -= updateRate;
        if (D_494 + updateRate > 0 && D_494 <= 0) {
            amSndPlay(0x11, NULL);
        }
    }
    if (D_494 > 0) {
        return;
    }

    if (D_19C < D_494) {
        D_2718 = 0xFF;
        for (i = 0; i < updateRate; i++) {
            if (D_1450->vertexCount < 400) {
                vertex = &D_498[D_1450->vertexCount];
                D_1450->x += D_1450->speed * func_8002A8C0(D_1450->angle);
                D_1450->y += D_1450->speed * func_8002A8BC(D_1450->angle);
                halfX = func_8002A8C0(D_1450->angle + 0x4000) * 5.0f;
                halfY = func_8002A8BC(D_1450->angle + 0x4000) * 5.0f;
                vertex->x = D_1450->x - halfX;
                vertex->y = D_1450->y - halfY;
                vertex->z = D_1450->z;
                vertex++;
                vertex->x = D_1450->x + halfX;
                vertex->y = D_1450->y + halfY;
                vertex->z = D_1450->z;
                D_1450->angle -= D_194;
                D_1450->vertexCount += 2;
            }
            for (j = 0; j < 200; j++) {
                if (D_1458[j].age < 0) {
                    D_1458[j].age = 0;
                    D_1458[j].x = D_1450->x;
                    D_1458[j].y = D_1450->y;
                    D_1458[j].z = D_1450->z;
                    D_1458[j].alpha = 0xFF;
                    D_1458[j].velocityX = (f32)mathRnd(-500, 500) / 500.0f;
                    D_1458[j].velocityY = (f32)mathRnd(-500, 500) / 500.0f;
                    j = 200;
                }
            }
        }
    } else {
        D_2718 -= updateRate * 8;
        if (D_2718 < 0) {
            D_2718 = 0;
        }
    }

    camStandardOrtho(&gDisplayListHead, &gOverlay46MatrixHead);
    point.value04 = 0;
    point.value00 = 0;
    point.value02 = 0;
    point.scale08 = 6.0f;
    point.value28 = 0.0f;
    for (i = 0; i < 200; i++) {
        if (D_1458[i].age != -1) {
            point.baseX0C = D_1458[i].x;
            point.baseY10 = D_1458[i].y;
            point.value14 = D_1458[i].z;
            gDPSetPrimColor(gDisplayListHead++, 0, 0, 0xA0, 0xA0, 0xA0, D_1458[i].alpha);
            func_80023A08(&gDisplayListHead, &gOverlay46MatrixHead, &gOverlay46VertexHead,
                          &point, D_800D31C8[9], 7, D_1458[i].alpha);
        }
    }
    func_800349A4(&gDisplayListHead, NULL, 3, 0);
    gDPSetPrimColor(gDisplayListHead++, 0, 0, 0xC0, 0xC0, 0xC0, 0xFF);

    vertex = D_498;
    count = D_1450->vertexCount - 2;
    batches = count / 16;
    remaining = batches * 16;
    for (i = 0; i < batches; i++) {
        if (remaining >= 16) {
            j = 16;
        } else {
            j = remaining;
        }
        k = j + 2;
        O46_SP_VERTEX(gDisplayListHead++, O46_PHYSICAL(vertex), k, 0);
        O46_SP_POLYGON(gDisplayListHead++, O46_PHYSICAL(D_230), j, 1);
        remaining -= j;
        vertex += j;
    }
    if (count % 16 != 0) {
        k = count % 16 + 2;
        O46_SP_VERTEX(gDisplayListHead++, O46_PHYSICAL(vertex), k, 0);
        O46_SP_POLYGON(gDisplayListHead++, O46_PHYSICAL(D_230), count % 16, 1);
    }

    if (D_2718 > 0) {
        point.baseX0C = D_1450->x;
        point.baseY10 = D_1450->y;
        point.value14 = D_1450->z;
        point.scale08 = 6.0f;
        gDPSetPrimColor(gDisplayListHead++, 0, 0, 0xA0, 0xA0, 0xA0, D_2718);
        func_80023A08(&gDisplayListHead, &gOverlay46MatrixHead, &gOverlay46VertexHead,
                      &point, D_800D31C8[10], 7, D_2718);
    }

    for (i = 0; i < 200; i++) {
        if (D_1458[i].age != -1) {

            D_1458[i].age += updateRate;
            D_1458[i].x += D_1458[i].velocityX * (f32)updateRate;
            D_1458[i].y += D_1458[i].velocityY * (f32)updateRate;
            if (D_1458[i].age >= 61) {
                D_1458[i].alpha -= updateRate * 8;
                if (D_1458[i].alpha < 0) {
                    D_1458[i].age = -1;
                }
            }
        }
    }
}
