#include "overlays/overlay_056.h"

/*
 * Overlay 56, ADR 0006 consolidation: one translation unit in ROM order.
 * Exact DKR v77/v80 and JFG scans are negative for the matched C functions;
 * the unresolved middle function remains GLOBAL_ASM.
 */

void overlay56OffsetCoordinates(u32 *x, u32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x += width >> 1;
    *y = (height >> 1) - *y;
}

void overlay56CenterCoordinates(s32 *x, s32 *y) {
    u32 width;
    u32 height;

    overlay56GetDimensionsReloc(&width, &height);
    *x -= width >> 1;
    *y = (height >> 1) - *y;
}

void overlay56SplitTime(s32 value, s32 *minutes, s32 *seconds,
                        s32 *centiseconds) {
    s32 wholeMinutes;
    wholeMinutes = value / 18000;
    *minutes = wholeMinutes;
    *seconds = (value / 300) - (wholeMinutes * 60);
    *centiseconds = (value / 3) % 100;
}

void overlay56SetMode(s32 mode) {
    gOverlay56Mode = mode;
}

void overlay56LoadResource(void) {
    Overlay56Context *context;

    context = overlay56GetContextReloc();
    if (context->resourceId != -1) {
        gOverlay56Resource = overlay56LoadResourceReloc(context->resourceId);
    } else {
        gOverlay56Resource = 0;
    }
    gOverlay56ResourceState = 0;
}

void overlay56ReleaseResource(void) {
    if (gOverlay56Resource != 0) {
        overlay56ReleaseResourceReloc(gOverlay56Resource);
        gOverlay56Resource = 0;
    }
}

/* Natural-source rewrite (2026-10-07): 581/581 instructions at size delta 0, frame 0x1F8.
 * Shape edits that moved it: GBI packet macros taking dl++, one shared loop index per
 * loop family (while (i--)), three distinct resident mode bytes (game mode, player count,
 * mirror) instead of one alias name, the colour word read from the table at each unpack
 * with no carrier local, mapY rebased in place (mapY += ...), an else-if ghost selector,
 * and x/z products written inline so mapY is not forwarded past the calls.
 * Lane d-mid3: -Wab,-r4300_mul (the target's mul.s hazard nops), func_8002F618's colour
 * parameters u8 (o052's matched prototype; the ghost's one andi), a ghost-loop racer local
 * of its own (ghostAlpha then takes s1 behind racer), the packet cursor at function scope
 * and the declaration order that lands every home on the target's frame ladder.
 * Lane e-big: the matrixTranslate y sum is assigned to the ghost loop's x (one merged web,
 * offered no argument colour, decided ahead of mapX), which leaves mapY no caller colour so
 * it takes f20 as shipped; the f24 save then needs the unused colour local gone and two
 * cells below ghostAlpha (frame ladder).
 * Open: sum/mapX colours swapped (f16/f18), cos/sin f22/f24 swapped, scale/x-product
 * f0/f2 swapped, the D_84 pointer temp home (0x74 against 0x7C). */
#ifdef NON_MATCHING
typedef struct O56Gfx {
    u32 w0;
    u32 w1;
} O56Gfx;

typedef struct O56Mtx {
    f32 m[4][4];
} O56Mtx;

typedef struct O56Sprite {
    u8 pad0[6];
    u16 width;
    u16 height;
} O56Sprite;

typedef struct O56Marker {
    O56Sprite *sprite;
    s32 unk4;
    s32 unk8;
    s16 x;
    s16 y;
    s32 unk10;
    u8 pad14[0xC];
} O56Marker;

typedef struct O56Racer {
    s8 unk0;
    s8 colour;
    u8 pad2[0xD];
    u8 ghostAlpha;
    u8 pad10[0x180];
    u8 alpha;
} O56Racer;

typedef struct O56Object {
    s16 angle;
    u8 pad2[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    O56Racer *racer;
} O56Object;

typedef struct O56Ghost {
    O56Object *object;
} O56Ghost;

typedef struct O56Level {
    u8 pad0[0x122];
    s16 angle;
    s16 offsetX;
    s16 offsetY;
    f32 scale;
} O56Level;

typedef struct Overlay56Data {
    u8 pad[0x50];
    u32 colors[1];
} Overlay56Data;

extern Overlay56Data gOverlay56Data;
extern u8 *func_80028F54(void);
extern void viGetCurrentSize(u32 *width, u32 *height);
extern void camStandardOrtho(O56Gfx **displayList, O56Mtx **matrix);
extern O56Object **func_80005750(s32 *count);
extern O56Level *levelGetLevel(void);
extern s32 frontGet2PlayerSplit(void);
extern void func_8002FB34(O56Gfx **displayList, O56Marker *marker, f32 x, f32 y,
                         f32 scaleX, f32 scaleY, s32 colour, s32 flags);
extern void func_8002F618(O56Gfx **displayList, O56Marker *marker, s32 x, s32 y,
                         u8 red, u8 green, u8 blue, s32 alpha);
extern void func_800349A4(O56Gfx **displayList, void *texture, s32 mode,
                         s32 flags);
extern void func_8002A82C(O56Mtx *matrix);
extern void matrixTranslate(f32 x, f32 y, f32 z, O56Mtx *matrix);
extern void func_8002A604(s16 angle, O56Mtx *matrix);
extern void func_80024978(O56Mtx *matrix);
extern void mtxf_mul(O56Mtx *lhs, O56Mtx *rhs, O56Mtx *dest);
extern void mtxf_to_mtx(O56Mtx *src, O56Mtx *dest);
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern s32 D_800C3A3C;
extern s32 D_800D3450;
extern u8 D_800D3198;
extern u8 D_800D3194;
extern u8 D_800D31A8;
extern O56Ghost *D_800D1494;
extern O56Ghost *D_800D1498;
extern O56Sprite *D_800CD788[];
extern s16 D_78[];
extern s16 D_84[];
extern u8 D_80000004[];
extern u8 D_80000030[];

#define O56_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O56_PIPESYNC(pkt) { _g = (O56Gfx *)(pkt); _g->w0 = O56_SHIFTL(0xE7, 24, 8); _g->w1 = 0; }
#define O56_SCISSOR(pkt, mode, ulx, uly, lrx, lry) { _g = (O56Gfx *)(pkt); \
    _g->w0 = O56_SHIFTL(0xED, 24, 8) | O56_SHIFTL((int)((float)(ulx) * 4.0F), 12, 12) | \
             O56_SHIFTL((int)((float)(uly) * 4.0F), 0, 12); \
    _g->w1 = O56_SHIFTL(mode, 24, 2) | O56_SHIFTL((int)((float)(lrx) * 4.0F), 12, 12) | \
             O56_SHIFTL((int)((float)(lry) * 4.0F), 0, 12); }
#define O56_DMA1P(pkt, c, s, l, p) { _g = (O56Gfx *)(pkt); \
    _g->w0 = O56_SHIFTL((c), 24, 8) | O56_SHIFTL((p), 16, 8) | O56_SHIFTL((l), 0, 16); \
    _g->w1 = (unsigned int)(s); }
#define O56_MATRIX(pkt, m) O56_DMA1P(pkt, 1, (u32)(m) + 0x80000000, 0x40, 0)
#define O56_VERTEX(pkt, v, n, v0) O56_DMA1P(pkt, 4, v, (((n) << 3) + ((n) << 1)) + 8, ((n) << 3) | ((u32)(v) & 6) | (v0))
#define O56_POLYGON(pkt, ptr, numTris, tex) { _g = (O56Gfx *)(pkt); \
    _g->w0 = O56_SHIFTL((((numTris) - 1) << 4) | (tex), 16, 8) | O56_SHIFTL(5, 24, 8) | \
             O56_SHIFTL((numTris) * 16, 0, 16); \
    _g->w1 = (unsigned int)(ptr); }
#define O56_PRIMCOLOR(pkt, rgba) { _g = (O56Gfx *)(pkt); _g->w0 = O56_SHIFTL(0xFA, 24, 8); _g->w1 = (rgba); }

void func_overlay_056_F00001A0_18A2F18(O56Gfx **displayList, O56Mtx **matrixCursor,
                                      s32 updateRate) {
    s32 pad0;
    s32 pad1;
    s32 pad2;
    s32 pad3;
    s32 pad4;
    s32 pad5;
    O56Racer *ghostRacer;
    O56Gfx *_g;
    s32 count;
    O56Object **racers;
    O56Level *level;
    O56Sprite *dot;
    O56Object *obj;
    O56Racer *racer;
    s32 shift;
    s32 slot;
    f32 mapX;
    s32 alpha;
    s32 i;
    f32 cosA;
    O56Mtx mtxA;
    O56Mtx mtxB;
    O56Mtx mtxC;
    u32 screenWidth;
    u32 screenHeight;
    O56Marker marker;
    u8 *gameState;
    f32 sinA;
    O56Gfx *dl;
    O56Mtx *mtx;
    u32 width;
    u32 height;
    f32 x;
    f32 z;
    f32 mapY;
    f32 rotX;
    s32 blue;
    s32 posX;
    s32 ghostAlpha;
    s32 red;
    s32 green;

    gameState = func_80028F54();
    if (D_800C3A3C == 0) {
        if (D_800D3198 == 3) {
            shift = 2;
        } else {
            shift = 1;
        }
        gOverlay56ResourceState += updateRate << shift;
        if (D_800D3198 != 3 && gOverlay56ResourceState > 160) {
            gOverlay56ResourceState = 160;
        } else if (gOverlay56ResourceState > 255) {
            gOverlay56ResourceState = 255;
        }
    }
    if (gOverlay56Resource == NULL) {
        return;
    }
    if (D_800D3450 != 0 && (*gameState == 0 || *gameState == 2 || *gameState == 1 ||
                            *gameState == 3 || *gameState == 4)) {
        return;
    }
    dl = *displayList;
    mtx = *matrixCursor;
    viGetCurrentSize(&width, &height);
    O56_PIPESYNC(dl++);
    O56_SCISSOR(dl++, 0, 0, 0, width, height);
    camStandardOrtho(&dl, &mtx);
    racers = func_80005750(&count);
    level = levelGetLevel();
    marker.unk4 = 0;
    marker.sprite = gOverlay56Resource;
    if (D_800D3194 == 2 && frontGet2PlayerSplit() != 0) {
        slot = 4;
    } else {
        slot = D_800D3194 - 1;
    }
    marker.y = D_84[slot] - (((O56Sprite *)gOverlay56Resource)->height >> 1) + 180;
    marker.unk10 = 0;
    if (D_800D31A8 != 0) {
        marker.x = -D_78[slot] - (((O56Sprite *)gOverlay56Resource)->width >> 1) + 60;
        func_8002FB34(&dl, &marker, 320.0f, 0.0f, 1.0f, 1.0f,
                      gOverlay56ResourceState | ~0xFF, 0x1002);
    } else {
        marker.x = D_78[slot] - (((O56Sprite *)gOverlay56Resource)->width >> 1) + 260;
        func_8002F618(&dl, &marker, 0, 0, 255, 255, 255, gOverlay56ResourceState);
    }
    viGetCurrentSize(&screenWidth, &screenHeight);
    dot = D_800CD788[22];
    cosA = func_8002A8BC(level->angle);
    sinA = func_8002A8C0(level->angle);
    alpha = gOverlay56ResourceState * 2;
    alpha = (alpha > 255) ? 255 : alpha;
    marker.unk4 = 0;
    marker.unk10 = 0;
    i = count;
    while (i--) {
        obj = racers[i];
        racer = obj->racer;
        rotX = obj->x * level->scale * cosA - obj->z * level->scale * sinA;
        mapY = obj->z * level->scale * cosA + obj->x * level->scale * sinA;
        mapX = rotX;
        if (D_800D31A8 != 0) {
            mapX = -rotX;
        }
        if (i < D_800D3194) {
            func_800349A4(&dl, NULL, 5, 0);
            func_8002A82C(&mtxA);
            if (D_800D31A8 != 0) {
                mapX += (f32)(D_78[slot] - level->offsetX + 520);
            } else {
                mapX += (f32)(level->offsetX + D_78[slot]);
            }
            x = mapY + (f32)(level->offsetY + D_84[slot]);
            matrixTranslate(mapX - (f32)(screenWidth >> 1),
                            (f32)(screenHeight >> 1) - x,
                            0.0f, &mtxA);
            if (D_800D31A8 != 0) {
                func_8002A604(-(obj->angle - level->angle), &mtxA);
            } else {
                func_8002A604(obj->angle - level->angle, &mtxA);
            }
            func_80024978(&mtxB);
            mtxf_mul(&mtxA, &mtxB, &mtxC);
            mtxf_to_mtx(&mtxC, mtx);
            O56_MATRIX(dl++, mtx);
            mtx++;
            if (((racer->alpha * alpha) >> 8) > 0) {
                O56_PRIMCOLOR(dl++, gOverlay56Data.colors[racer->colour] | ((alpha * racer->alpha) >> 8));
                O56_VERTEX(dl++, D_80000004, 4, 0);
                O56_POLYGON(dl++, D_80000030, 2, 1);
            }
        } else {
            marker.sprite = D_800CD788[22];
            marker.x = mapX - (f32)(dot->width >> 1);
            marker.y = mapY - (f32)(dot->height >> 1);
            red = gOverlay56Data.colors[racer->colour] >> 24;
            green = (gOverlay56Data.colors[racer->colour] >> 16) & 0xFF;
            blue = (gOverlay56Data.colors[racer->colour] >> 8) & 0xFF;
            if (D_800D31A8 != 0) {
                posX = D_78[slot] - level->offsetX + 520;
            } else {
                posX = D_78[slot] + level->offsetX;
            }
            func_8002F618(&dl, &marker, posX, D_84[slot] + level->offsetY, red, green,
                          blue, (racer->alpha * alpha) >> 8);
        }
    }
    if (*gameState == 1) {
        i = 2;
        while (i--) {
            if (i != 0 && D_800D1494 != NULL) {
                obj = D_800D1494->object;
                ghostAlpha = 255;
            } else if (i == 0 && D_800D1498 != NULL) {
                obj = D_800D1498->object;
                ghostAlpha = 85;
            } else {
                obj = NULL;
            }
            if (obj != NULL) {
                x = obj->x * level->scale;
                z = obj->z * level->scale;
                marker.sprite = D_800CD788[22];
                ghostRacer = obj->racer;
                red = green = blue = ghostAlpha;
                marker.x = (x * cosA - z * sinA) - (f32)(dot->width >> 1);
                marker.y = (z * cosA + x * sinA) - (f32)(dot->height >> 1);
                func_8002F618(&dl, &marker, D_78[slot] + level->offsetX,
                              D_84[slot] + level->offsetY, red, green, blue,
                              (ghostRacer->ghostAlpha * alpha) >> 7);
            }
        }
    }
    *displayList = dl;
    *matrixCursor = mtx;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o056/overlay_056/func_overlay_056_F00001A0_18A2F18.s")
#endif

void overlay56UnpackColor(s32 index, u32 *red, s32 *green, s32 *blue) {
    u32 *color = &gOverlay56Colors[index];
    *red = *color >> 24;
    *green = (*color >> 16) & 0xFF;
    *blue = (*color >> 8) & 0xFF;
}

/* PLATEAU-HANDOFF:func_overlay_056_F00001A0_18A2F18:start
 * symbol: func_overlay_056_F00001A0_18A2F18
 * score: 163 differing words
 * frame: 0x1F8
 * relocations: 75
 * first-mismatch: +0x60
 * summary: mapY in f20 via the y sum merged into the ghost x web, frame ladder for the f24 save: 542 to 163 at 0; open: sum/mapX, cos/sin, scale/x colour pairs.
 * PLATEAU-HANDOFF:func_overlay_056_F00001A0_18A2F18:end
 */
