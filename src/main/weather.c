/*
 * Snow and rain weather system -- ROM 0x3B480-0x3D030.
 *
 * PROVENANCE -- the TU and descriptive function names are borrowed from Jet
 * Force Gemini's and Diddy Kong Racing's public retail-derived src/weather.c
 * files and JFG's nonmatching assembly names.  Evidence is tier B/D except
 * for the existing tier-A weather_clip_planes identity.  Adapted bodies carry
 * point-of-use provenance notes; Mickey's ROM remains authoritative.
 *
 * snow_update and snow_vertices are extractor-marked hand-written routines;
 * snow_vertices also uses odd single-precision FP registers.  Both stay asm.
 */

#include "PR/ultratypes.h"

typedef struct Gfx Gfx;
typedef struct Mtx Mtx;
typedef struct Camera Camera;
typedef struct Matrix Matrix;

struct Gfx {
    u32 w0;
    u32 w1;
};

struct Camera {
    s16 rotationX;
    u8 pad2[10];
    f32 x;
    f32 y;
    f32 z;
};

typedef struct WeatherClipPlanes {
    s16 near;
    s16 far;
    s32 current;
} WeatherClipPlanes;

typedef struct WeatherData {
    s32 intensity;
    s32 intensityStep;
    s32 intensityTarget;
    s32 velX;
    s32 velXStep;
    s32 velXTarget;
    s32 velY;
    s32 velYStep;
    s32 velYTarget;
    s32 velZ;
    s32 velZStep;
    s32 velZTarget;
    s32 opacity;
    s32 opacityStep;
    s32 opacityTarget;
    s32 shiftTime;
} WeatherData;

typedef struct WeatherTexture {
    u8 pad0[6];
    s16 width;
    s16 height;
} WeatherTexture;

typedef struct WeatherPosition {
    s32 x;
    s32 y;
    s32 z;
} WeatherPosition;

typedef struct WeatherGfxData {
    void *positions;
    s32 size;
    union {
        s32 type;
        WeatherTexture *texture;
    } source;
    s32 offsetX;
    s32 offsetY;
    s32 offsetZ;
    s32 radiusX;
    s32 radiusY;
    s32 radiusZ;
    s16 vertOffsetW;
    s16 vertOffsetH;
    s16 vertWidth;
    s16 vertHeight;
} WeatherGfxData;

typedef struct WeatherParticle {
    s32 x;
    s32 y;
    s32 z;
    u8 xScale;
    u8 yScale;
    u8 zScale;
    u8 index;
} WeatherParticle;

typedef struct RainSplash {
    u8 pad0[6];
    s16 state;
    u8 pad8[4];
    f32 x;
    f32 height;
    f32 z;
    s16 alpha;
    u8 pad1A[0xE];
    f32 age;
} RainSplash;

typedef struct RainHeight {
    f32 height;
    u8 pad4[8];
    s8 type;
} RainHeight;

typedef struct RainPlayer {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} RainPlayer;

typedef struct WeatherVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} WeatherVertex;

typedef struct WeatherTexCoord {
    s16 u;
    s16 v;
} WeatherTexCoord;

typedef struct WeatherTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    WeatherTexCoord uv0;
    WeatherTexCoord uv1;
    WeatherTexCoord uv2;
} WeatherTriangle;

typedef struct WeatherLevel {
    u8 pad0[0xA4];
    u8 flags;
} WeatherLevel;

extern WeatherClipPlanes D_800D40B8;
extern s32 D_8007C6EC;
extern s32 D_8007C6F8;
extern WeatherGfxData D_8007C310[];
extern WeatherParticle *D_8007C394;
extern WeatherGfxData D_8007C398;
extern WeatherTriangle *D_8007C3CC;
extern s16 *D_8007C3D0;
extern WeatherVertex *D_8007C3D4[2];
extern s32 *D_8007C3DC;
extern s8 D_8007C3E0;
extern s32 D_800D4070;
extern s32 D_800D4074;
extern WeatherData D_800D4078;
extern s32 D_800D40C0;
extern s32 D_800D40C4;
extern s8 D_800D40C8;
extern Gfx *D_800D40CC;
extern Mtx *D_800D40D0;
extern WeatherVertex *D_800D40D4;
extern WeatherTriangle *D_800D40D8;
extern Camera *D_800D40DC;
extern Matrix *D_800D40E0;
extern WeatherVertex *D_8007C3C4;
extern s32 D_8007C3C8;
extern RainSplash D_8007C3E4[16];
extern u8 D_7C6A8;
extern s32 D_8007C6E8;
extern f32 D_8007C6C8;
extern f32 D_8007C6CC;
extern f32 D_8007C6D0;
extern f32 D_8007C6D4;
extern f32 D_8007C6D8;
extern f32 D_8007C6DC;
extern f32 D_8007C6E0;
extern f32 D_8007C6E4;
extern s32 D_8007C6EC;
extern s32 D_8007C6F0;
extern s32 D_8007C6F4;
extern s32 D_8007C6F8;
extern s32 D_8007C6FC;
extern s32 D_8007C700;
extern s32 D_8007C704;
extern s32 D_8007C708;
extern s32 D_8007C70C;
extern s32 D_8007C710;
extern void *D_8007C714;
extern WeatherTexture *D_8007C718;
extern s32 D_8007C71C;
extern void *D_8007C720;
extern void *D_800D40E4;
extern f32 D_80082830;
extern s32 osTvType;

extern s32 func_800299E8(s32 min, s32 max);
extern RainPlayer *func_80005820(s32 arg0);
extern s32 func_8001398C(f32 x, f32 z, s32 arg2, RainHeight ***arg3);
extern void func_80023A08(Gfx **dList, Mtx **matrix, WeatherVertex **vertices,
                           RainSplash *splash, void *texture, s32 arg5, s32 arg6);
extern s32 mathRnd(s32 min, s32 max);
extern void *func_8002B280(s32 size, s32 tag);
extern Camera *camGetPtr(void);
extern Matrix *camGetRotationMtx(void);
extern s32 *piRomLoad(s32 assetId);
extern s32 coss_s16(s16 angle);
extern s32 func_8002A1A4(s16 angle);
extern WeatherTexture *func_80034448(s32 textureId);
extern s32 func_80049864(s32 mode);
extern void func_800498FC(s32 mode, f32 arg1, f32 arg2, s32 red, s32 green, s32 blue, s32 alpha);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void func_800031C0(void *sound, f32 x, f32 y, f32 z);
extern void *func_800355A0(s32 assetId, s32 arg1);
extern s32 camGetMode(void);
extern void TrapDanglingJump(f32, f32, f32, s32);
extern WeatherLevel *levelGetLevel(void);
extern void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                        u8 red, u8 green, u8 blue, s8 state);
extern void mmFree(void *ptr);
extern void func_800347A0(WeatherTexture *texture);
extern Mtx *camGetProjOrgMtx(void);
extern void func_800349A4(Gfx **dList, void *texture, s32 flags, s32 arg3);

void freeWeather(void);
void snow_init(void);
void rain_init();
void free_rain_memory(void);
void rain_update(s32 updateRate);
void rain_set(s32 intensity, s32 opacity, f32 seconds);
void rain_render_splashes(s32 updateRate);
void rain_lightning(s32 updateRate);
void rain_sound(s32 updateRate);
void snow_update(WeatherData *weather, WeatherGfxData *gfx, s32 particleCount, WeatherParticle *particles,
                 s32 updateRate);
s32 snow_vertices(Camera *camera, WeatherGfxData *gfx, s32 particleCount, WeatherParticle *particles,
                  Matrix *cameraMatrix, WeatherVertex *vertices);
void snow_render(void);

/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::initWeather. Mickey's globals and asset ID are authoritative.
 */
void initWeather(void) {
    s32 *temp_v0;

    D_8007C398.positions = NULL;
    D_8007C398.size = 0;
    D_8007C394 = NULL;
    D_800D4070 = 0;
    D_800D40C0 = 6;
    D_800D40C0 <<= 2;
    D_800D40C4 = D_800D40C0 >> 1;
    D_8007C3D4[0] = NULL;
    D_8007C3D4[1] = NULL;
    D_8007C3CC = NULL;
    D_800D40B8.near = -1;
    D_800D40B8.far = -0x200;
    if (D_8007C3DC == NULL) {
        temp_v0 = piRomLoad(0x1B);
        D_8007C3E0 = 0;
        D_8007C3DC = temp_v0;
        while (D_8007C3DC[D_8007C3E0] != -1) {
            D_8007C3E0++;
        }
    }
    D_800D40C8 = 0;
}
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's public retail-derived
 * src/weather.c::weather_clip_planes.  Mickey's bytes and global layout are
 * authoritative here.
 */
void weather_clip_planes(s16 near, s16 far) {
    if (D_800D40B8.far < D_800D40B8.near) {
        D_800D40B8.near = near;
        D_800D40B8.far = far;
    } else {
        D_800D40B8.near = far;
        D_800D40B8.far = near;
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::freeWeather. Mickey's globals and release order are
 * authoritative here.
 */
void freeWeather(void) {
    if (D_8007C3CC != NULL) {
        mmFree(D_8007C3CC);
        D_8007C3CC = NULL;
    }
    if (D_8007C3D4[0] != NULL) {
        mmFree(D_8007C3D4[0]);
        D_8007C3D4[0] = NULL;
    }
    if (D_8007C3D4[1] != NULL) {
        mmFree(D_8007C3D4[1]);
        D_8007C3D4[1] = NULL;
    }
    if (D_8007C394 != NULL) {
        mmFree(D_8007C394);
        D_8007C394 = NULL;
    }
    if (D_8007C398.positions != NULL) {
        mmFree(D_8007C398.positions);
        D_8007C398.positions = NULL;
    }
    if (D_8007C398.source.texture != NULL) {
        func_800347A0(D_8007C398.source.texture);
        D_8007C398.source.texture = NULL;
    }
    if (D_8007C3D0 != NULL) {
        mmFree(D_8007C3D0);
        D_8007C3D0 = NULL;
    }
    if (D_8007C6E8 != 0) {
        free_rain_memory();
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::setupWeather.  Mickey's bytes, extra rain-init argument,
 * random bounds, and globals are authoritative here.
 */
void setupWeather(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s16 temp_s1_2;
    s16 temp_s2;
    s32 temp_s1;
    u8 var_a1;
    s32 i;
    WeatherParticle *var_s1_2;
    WeatherTriangle *var_v1_2;
    WeatherVertex *var_a3;
    s32 j;
    s8 *var_a0;
    u8 *var_v1;
    s32 pad;
    s32 numOfElements;

    freeWeather();
    D_800D4078.velX = arg2;
    D_800D4078.velXStep = 0;
    D_800D4078.velXTarget = arg2;
    D_800D4078.velY = arg3;
    D_800D4078.velYStep = 0;
    D_800D4078.velYTarget = arg3;
    D_800D4078.velZStep = 0;
    D_800D4078.intensityStep = 0;
    D_800D4078.opacityStep = 0;
    D_800D4078.shiftTime = 0;
    D_800D4078.velZ = arg4;
    D_800D4078.velZTarget = arg4;
    D_800D4078.intensity = arg5;
    D_800D4078.intensityTarget = arg5;
    D_800D4078.opacity = arg6;
    D_800D4078.opacityTarget = arg6;
    if (arg0 > 1) {
        arg0 = 1;
    }
    if (D_8007C310[arg0].source.type == 1) {
        rain_init(arg1, arg5 + 1, arg6 + 1, arg5);
        return;
    }
    var_v1 = (u8 *) &D_8007C310[arg0];
    var_a0 = (s8 *) &D_8007C398;
    var_a1 = 0x2C;
    while (var_a1--) {
        *var_a0++ = *var_v1++;
    }
    if (!var_s1_2) {
        ;
    }
    D_8007C398.positions = func_8002B280(D_8007C310[arg0].size * 0xC, 0x93);
    if (D_8007C310[arg0].source.type == 0) {
        snow_init();
    }
    numOfElements = arg1;
    D_800D4070 = arg1;
    D_8007C3D0 = func_8002B280(arg1 * sizeof(s16), 0x93);
    D_8007C394 = func_8002B280(arg1 * sizeof(WeatherParticle), 0x93);
    var_s1_2 = D_8007C394;
    for (i = 0; i < D_800D4070; i++) {
        var_s1_2->x = func_800299E8(0, D_8007C398.radiusX);
        var_s1_2->y = func_800299E8(0, D_8007C398.radiusY);
        var_s1_2->z = func_800299E8(0, D_8007C398.radiusZ);
        var_s1_2->xScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->yScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->zScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->index = mathRnd(0, D_8007C398.size - 1);
        var_s1_2++;
    }
    numOfElements *= 4;
    temp_s1 = sizeof(WeatherVertex);
    temp_s1 *= numOfElements;
    D_8007C3D4[0] = func_8002B280(temp_s1, 0x93);
    D_8007C3D4[1] = func_8002B280(temp_s1, 0x93);
    j = 0;
    do {
        var_a3 = D_8007C3D4[j];
        for (i = 0; i < numOfElements; i++, var_a3++) {
            var_a3->r = 0xFF;
            var_a3->g = 0xFF;
            var_a3->b = 0xFF;
            var_a3->a = 0xFF;
        }
        j++;
    } while (&D_8007C3D4[j] < (WeatherVertex **) &D_8007C3DC);
    temp_s1_2 = (D_8007C398.source.texture->width << 5) - 1;
    temp_s2 = (D_8007C398.source.texture->height << 5) - 1;
    D_8007C3CC = func_8002B280(D_800D40C4 * sizeof(WeatherTriangle), 0x93);
    var_v1_2 = D_8007C3CC;
    for (i = 0; i < D_800D40C4; i += 2) {
        var_v1_2[0].flags = 0;
        var_v1_2[0].vi0 = (i << 1) + 3;
        var_v1_2[0].uv0.u = 0;
        var_v1_2[0].uv0.v = temp_s2;
        var_v1_2[0].vi1 = (i << 1) + 1;
        var_v1_2[0].uv1.u = temp_s1_2;
        var_v1_2[0].uv1.v = 0;
        var_v1_2[0].vi2 = i << 1;
        var_v1_2[0].uv2.u = 0;
        var_v1_2[0].uv2.v = 0;
        var_v1_2[1].flags = 0;
        var_v1_2[1].vi0 = (i << 1) + 3;
        var_v1_2[1].uv0.u = 0;
        var_v1_2[1].uv0.v = temp_s2;
        var_v1_2[1].vi1 = (i << 1) + 2;
        var_v1_2[1].uv1.u = temp_s1_2;
        var_v1_2[1].uv1.v = temp_s2;
        var_v1_2[1].vi2 = (i << 1) + 1;
        var_v1_2[1].uv2.u = temp_s1_2;
        var_v1_2[1].uv2.v = 0;
        var_v1_2 += 2;
    }
    D_800D40C8 = 0;
}
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's public retail-derived
 * src/weather.c::snow_init. Mickey's scale constants and texture loader are
 * authoritative here.
 */
void snow_init(void) {
    s32 step;
    s32 offset;
    s32 i;

    step = 0x10000 / D_8007C398.size;
    offset = 0;
    for (i = 0; i < D_8007C398.size; i++) {
        ((WeatherPosition *) D_8007C398.positions)[i].x = coss_s16(offset & 0xFFFF) * 4;
        ((WeatherPosition *) D_8007C398.positions)[i].y = 0xFFFE0000;
        ((WeatherPosition *) D_8007C398.positions)[i].z = func_8002A1A4(offset & 0xFFFF);
        offset += step;
    }
    D_8007C398.source.texture = func_80034448(*D_8007C3DC);
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::changeWeather. Mickey's condition and assignment ordering
 * are authoritative here.
 */
void changeWeather(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    if ((arg5 > 0) &&
        ((arg0 != D_800D4078.velXTarget) || (arg1 != D_800D4078.velYTarget) ||
         (arg2 != D_800D4078.velZTarget) || (arg3 != D_800D4078.intensity) ||
         (arg4 != D_800D4078.opacity))) {
        D_800D4078.velXStep = (s32) ((arg0 - D_800D4078.velX) / arg5);
        D_800D4078.velXTarget = arg0;
        D_800D4078.velYStep = (s32) ((arg1 - D_800D4078.velY) / arg5);
        D_800D4078.velYTarget = arg1;
        D_800D4078.velZStep = (s32) ((arg2 - D_800D4078.velZ) / arg5);
        D_800D4078.velZTarget = arg2;
        if (D_8007C6E8 == 0) {
            D_800D4078.intensityTarget = arg3;
            D_800D4078.intensityStep = (s32) ((arg3 - D_800D4078.intensity) / arg5);
            D_800D4078.opacityStep = (s32) ((arg4 - D_800D4078.opacity) / arg5);
            D_800D4078.opacityTarget = arg4;
            D_800D4078.shiftTime = arg5;
            return;
        }
        D_800D4078.intensity = arg3;
        D_800D4078.opacity = arg4;
        D_800D4078.shiftTime = 0;
        rain_set(arg3 + 1, arg4 + 1, (f32) arg5 / 60.0f);
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::doWeather. Mickey's split vertex/render calls and globals
 * are authoritative here.
 */
void doWeather(Gfx **arg0, Mtx **arg1, WeatherVertex **arg2, WeatherTriangle **arg3, s32 updateRate) {
    D_800D40CC = *arg0;
    D_800D40D0 = *arg1;
    D_800D40D4 = *arg2;
    D_800D40D8 = *arg3;
    D_800D40DC = camGetPtr();
    D_800D40E0 = camGetRotationMtx();
    if (D_8007C6E8 != 0) {
        rain_update(updateRate);
    } else {
        if (D_800D4078.shiftTime > 0) {
            if (updateRate < D_800D4078.shiftTime) {
                D_800D4078.intensity += D_800D4078.intensityStep * updateRate;
                D_800D4078.velX += D_800D4078.velXStep * updateRate;
                D_800D4078.velY += D_800D4078.velYStep * updateRate;
                D_800D4078.velZ += D_800D4078.velZStep * updateRate;
                D_800D4078.opacity += D_800D4078.opacityStep * updateRate;
                D_800D4078.shiftTime -= updateRate;
            } else {
                D_800D4078.shiftTime = 0;
                D_800D4078.intensity = D_800D4078.intensityTarget;
                D_800D4078.velX = D_800D4078.velXTarget;
                D_800D4078.velY = D_800D4078.velYTarget;
                D_800D4078.velZ = D_800D4078.velZTarget;
                D_800D4078.opacity = D_800D4078.opacityTarget;
            }
        }
        D_800D4074 = (D_800D4070 * D_800D4078.intensity) >> 16;
        D_800D40B8.current =
            (D_800D40B8.near + ((D_800D40B8.far - D_800D40B8.near) * D_800D4078.opacity)) >> 16;
        snow_update(&D_800D4078, &D_8007C398, D_800D4070, D_8007C394, updateRate);
        if (D_800D4074 > 0 && D_800D40B8.current < D_800D40B8.near) {
            D_8007C3C4 = D_8007C3D4[D_800D40C8];
            D_8007C3C8 = snow_vertices(D_800D40DC, &D_8007C398, D_800D4074, D_8007C394,
                                       D_800D40E0, D_8007C3C4);
            snow_render();
            D_800D40C8 = 1 - D_800D40C8;
        }
    }
    *arg0 = D_800D40CC;
    *arg1 = D_800D40D0;
    *arg2 = D_800D40D4;
    *arg3 = D_800D40D8;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005B928_5C528 (revision efd5abb), with command expansion
 * from include/f3ddkr.h and include/PR/gbi.h. Mickey caches inputs before calls,
 * consumes the remaining vertex count, and omits JFG's batch resets/colors.
 * Mickey's own ROM defines the command formats and storage used here.
 */
void snow_render(void) {
    s32 count;
    Gfx *dList;
    WeatherVertex *vertices;
    WeatherTriangle *triangles;
    Mtx *mtx;

    if (D_8007C398.source.texture == NULL) {
        return;
    }
    if (D_8007C3C8 < 4) {
        return;
    }

    count = D_8007C3C8;
    dList = D_800D40CC;
    vertices = D_8007C3C4;
    triangles = D_8007C3CC;
    mtx = camGetProjOrgMtx();
    {
        Gfx *command = dList++;
        /* Keep the donor macro's shared statement line for IDO scheduling. */
        command->w0 = 0x01000040; command->w1 = (u32)mtx + 0x80000000;
    }
    {
        Gfx *command = dList++;
        command->w1 = 0;
        command->w0 = 0xBC00000A;
    }
    func_800349A4(&dList, D_8007C398.source.texture, 2, 0);
    while (D_800D40C0 < count) {
        {
            Gfx *command = dList++;
            command->w0 = (((((D_800D40C0 << 3) | (((u32)vertices + 0x80000000) & 6)) & 0xFF) << 16) |
                           0x04000000 | ((((D_800D40C0 << 3) + (D_800D40C0 << 1)) + 8) & 0xFFFF));
            command->w1 = (u32)vertices + 0x80000000;
        }
        {
            Gfx *command = dList++;
            command->w0 = ((((((D_800D40C4 - 1) << 4) | 1) & 0xFF) << 16) |
                           0x05000000 | ((D_800D40C4 * 16) & 0xFFFF));
            command->w1 = (u32)triangles + 0x80000000;
        }
        count -= D_800D40C0;
        vertices += D_800D40C0;
    }
    {
        Gfx *command = dList++;
        command->w0 = (((((count << 3) | (((u32)vertices + 0x80000000) & 6)) & 0xFF) << 16) |
                       0x04000000 | ((((count << 3) + (count << 1)) + 8) & 0xFFFF));
        command->w1 = (u32)vertices + 0x80000000;
    }
    {
        Gfx *command = dList++;
        command->w0 = (((((((count >> 1) - 1) << 4) | 1) & 0xFF) << 16) |
                       0x05000000 | (((count >> 1) * 16) & 0xFFFF));
        command->w1 = (u32)triangles + 0x80000000;
    }
    D_800D40CC = dList;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005BC44_5C844 (DKR's rain_init). Mickey's rain setup
 * constants and asset IDs are authoritative. The typed alias preserves this
 * call's integer ABI; the build canonicalizes its undefined symbol name to the
 * shared TrapDanglingJump target without changing section contents.
 */
#pragma weak rainInitTrap = TrapDanglingJump
extern void rainInitTrap(s32, s32, s32, s32, s32, s32, s32);
void rain_init(s32 count, s32 intensity, s32 opacity) {
    D_8007C6EC = intensity;
    D_8007C6F0 = 0;
    D_8007C6F4 = D_8007C6EC;
    D_8007C6F8 = opacity;
    D_8007C6FC = 0;
    D_8007C700 = D_8007C6F8;
    D_8007C704 = 0;
    D_8007C708 = 0;
    D_8007C70C = 0;
    D_8007C710 = 0;
    D_8007C71C = 0;

    rainInitTrap(count, 700, 700, 700, 0x2080E002, 0xA0E0FF04, 550);
    D_8007C714 = func_800355A0(0x26, 0);
    D_8007C718 = func_80034448(0x6A);
    D_8007C6E8 = 1;
    D_800D40E4 = NULL;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005BD30_5C930 (DKR's free_rain_memory). Mickey's
 * globals and trap binding are authoritative here.
 */
extern void func_800359D4(void *sprite);
extern void func_800031E8(void *sound);
extern void rainFreeTrap(void);
void free_rain_memory(void) {
    if (D_8007C714 != NULL) {
        func_800359D4(D_8007C714);
        D_8007C714 = NULL;
    }
    if (D_8007C718 != NULL) {
        func_800347A0(D_8007C718);
        D_8007C718 = NULL;
    }
    if (D_8007C720 != NULL) {
        func_800031E8(D_8007C720);
        D_8007C720 = NULL;
    }
    rainFreeTrap();
    D_8007C6E8 = 0;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005BDB8_5C9B8 (DKR's rain_set). Mickey's global
 * bindings are authoritative here.
 */
void rain_set(s32 intensity, s32 opacity, f32 seconds) {
    if (osTvType == 0) {
        D_8007C704 = (s32) (50.0f * seconds);
    } else {
        D_8007C704 = (s32) (60.0f * seconds);
    }

    D_8007C6F4 = intensity;
    D_8007C6F0 = (D_8007C6F4 - D_8007C6EC) / D_8007C704;
    D_8007C700 = opacity;
    D_8007C6FC = (D_8007C700 - D_8007C6F8) / D_8007C704;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::rainSetFog. Mickey's level flag and fog call are
 * authoritative here.
 */
void rainSetFog(void) {
    s32 near;
    s32 far;

    if ((D_8007C6E8 != 0) && (camGetMode() == 0)) {
        if (!(levelGetLevel()->flags & 1)) {
            near = ((D_8007C6EC * -38) >> 16) + 1018;
            far = ((D_8007C6EC * -20) >> 16) + 1023;
            trackSetFog(0, near, far, near, 28, 15, 36, 0);
        }
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::rainDensity.  Mickey's bytes and globals are authoritative.
 */
f32 rainDensity(void) {
    f32 density;

    density = (f32)(((D_8007C6F8 >> 2) * D_8007C6EC) >> 14) / 0x10000;
    if (density < 0.0f) {
        density = 0.0f;
    }
    if (density > 1.0f) {
        density = 1.0f;
    }
    return density;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005C040_5CC40 (DKR's rain_update). Mickey's globals
 * and unresolved rain-movement call are authoritative here.
 */
void rain_update(s32 updateRate) {
    if ((camGetMode() != 0) || (D_8007C6E8 == 0)) {
        return;
    }

    if (D_8007C704 > 0) {
        if (updateRate < D_8007C704) {
            D_8007C704 -= updateRate;
            D_8007C6EC += D_8007C6F0 * updateRate;
            D_8007C6F8 += D_8007C6FC * updateRate;
        } else {
            D_8007C704 = 0;
            D_8007C6EC = D_8007C6F4;
            D_8007C6F8 = D_8007C700;
        }
    }

    TrapDanglingJump((f32) D_800D4078.velX / 65536.0f,
                     ((f32) D_800D4078.velY / 65536.0f) - 5.0f,
                     (f32) D_800D4078.velZ / 65536.0f, updateRate);
    rain_sound(updateRate);
    rain_render_splashes(updateRate);
    rain_lightning(updateRate);
}
/*
 * PROVENANCE -- control flow, spawn while-loop, delay increment, and splash
 * search adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005C188_5CD88 C body. Mickey's single height query,
 * random bounds, type test, age global, and display-list words remain
 * authoritative. Four unused pointer declarations hold the 0xB8 frame (L99).
 */
#ifdef NON_MATCHING
void rain_render_splashes(s32 updateRate) {
    void *unused0;
    void *unused1;
    void *unused2;
    void *unused3;
    RainSplash *splash;
    RainPlayer *player;
    RainHeight **heightResult;
    s32 density;
    s32 delay;
    s32 countdown;
    s32 index;
    s32 found;
    s32 temp;
    f32 radius;
    f32 x;
    f32 z;
    f32 age;
    Gfx *cmd;
    s16 *vertex;

    if (D_8007C714 == NULL || D_8007C718 == NULL) {
        return;
    }
    density = ((D_8007C6F8 >> 2) * D_8007C6EC) >> 14;
    if (density > 0x4000) {
        player = func_80005820(0);
        if (player != NULL) {
            D_8007C710 -= updateRate;
            while (D_8007C710 <= 0) {
                index = 0x10;
                found = 0;
                splash = D_8007C3E4;
                while (index > 0 && found == 0) {
                    if (splash->state == 0) {
                        found = 1;
                    } else {
                        splash += 1;
                    }
                    index--;
                }
                if (found != 0) {
                    temp = func_800299E8(0, 0xFFFF);
                    radius = (f32) func_800299E8(0x26, 0xFF);
                    x = (func_8002A8C0(temp) * radius) + player->x;
                    z = (func_8002A8BC(temp) * radius) + player->z;
                    if (func_8001398C(x, z, 0x800, &heightResult) != 0) {
                        splash->x = x;
                        splash->z = z;
                        splash->state = 1;
                        splash->age = 0.0f;
                        splash->alpha = (s16) mathRnd(0x40, (density >> 10) + 0x60);
                        splash->height = (*heightResult)->height;
                        if ((*heightResult)->type == (s8) 1) {
                            splash->state += 1;
                        }
                    }
                }
                D_8007C710 += 2;
                if (D_8007C710 >= 0) {
                    D_8007C710 = (D_8007C710 - (density >> 10)) + 0x40;
                    if (D_8007C710 < 0) {
                        D_8007C710 = 0;
                    }
                }
            }
        }
    }
    cmd = D_800D40CC;
    D_800D40CC = cmd + 1;
    cmd->w0 = 0xFB000000;
    cmd->w1 = (u32) -0x100;
    age = D_80082830;
    splash = D_8007C3E4;
    index = 0;
    do {
        if (splash->state != 0) {
            splash->age += (f32) updateRate * age;
            if (splash->age < 4.0f) {
                if (splash->state == 1) {
                    cmd = D_800D40CC;
                    D_800D40CC = cmd + 1;
                    cmd->w0 = 0xFA000000;
                    cmd->w1 = (u32) ((splash->alpha & 0xFF) | ~0xFF);
                    func_80023A08(&D_800D40CC, &D_800D40D0, &D_800D40D4,
                                  splash, D_8007C714, 0xE, 0);
                } else {
                    func_800349A4(&D_800D40CC, D_8007C718, 0xE, 0);
                    cmd = D_800D40CC;
                    D_800D40CC = cmd + 1;
                    cmd->w0 = 0xFA000000;
                    cmd->w1 = 0xC0E0FFFF;
                    cmd = D_800D40CC;
                    D_800D40CC = cmd + 1;
                    cmd->w0 = ((((((s32) D_800D40D4 + 0x80000000) & 6) | 0x20) & 0xFF) << 16) |
                               0x04000000 | 0x30;
                    cmd->w1 = (u32) ((s32) D_800D40D4 + 0x80000000);
                    cmd = D_800D40CC;
                    D_800D40CC = cmd + 1;
                    cmd->w0 = 0x05110020;
                    cmd->w1 = (u32) &D_7C6A8;

                    D_800D40D4->x = (s16) (D_8007C6C8 * splash->age + splash->x);
                    D_800D40D4->y = (s16) splash->height;
                    D_800D40D4->z = (s16) (D_8007C6D8 * splash->age + splash->z);
                    D_800D40D4->r = 0xFF;
                    D_800D40D4->g = 0xFF;
                    D_800D40D4->b = 0xFF;
                    D_800D40D4->a = 0xFF;
                    D_800D40D4++;
                    D_800D40D4->x = (s16) (D_8007C6CC * splash->age + splash->x);
                    D_800D40D4->y = (s16) splash->height;
                    D_800D40D4->z = (s16) (D_8007C6DC * splash->age + splash->z);
                    D_800D40D4->r = 0xFF;
                    D_800D40D4->g = 0xFF;
                    D_800D40D4->b = 0xFF;
                    D_800D40D4->a = 0xFF;
                    D_800D40D4++;
                    D_800D40D4->x = (s16) (D_8007C6D0 * splash->age + splash->x);
                    D_800D40D4->y = (s16) splash->height;
                    D_800D40D4->z = (s16) (D_8007C6E0 * splash->age + splash->z);
                    D_800D40D4->r = 0xFF;
                    D_800D40D4->g = 0xFF;
                    D_800D40D4->b = 0xFF;
                    D_800D40D4->a = 0xFF;
                    D_800D40D4++;
                    D_800D40D4->x = (s16) (D_8007C6D4 * splash->age + splash->x);
                    D_800D40D4->y = (s16) splash->height;
                    D_800D40D4->z = (s16) (D_8007C6E4 * splash->age + splash->z);
                    D_800D40D4->r = 0xFF;
                    D_800D40D4->g = 0xFF;
                    D_800D40D4->b = 0xFF;
                    D_800D40D4->a = 0xFF;
                    D_800D40D4++;
                }
            } else {
                splash->state = 0;
            }
        }
        index++;
        splash++;
    } while (index != 0x10);
    cmd = D_800D40CC;
    D_800D40CC = cmd + 1;
    cmd->w0 = 0xFA000000;
    cmd->w1 = (u32) -1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_render_splashes.s")
#endif
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's and Jet Force Gemini's
 * public retail-derived src/weather.c::rain_lightning. Mickey's thresholds,
 * transition call, and timer arithmetic are authoritative here.
 */
void rain_lightning(s32 updateRate) {
    s32 delay;

    if (D_8007C70C > 0) {
        D_8007C70C -= updateRate;
        if (D_8007C70C <= 0) {
            if (D_8007C6F8 > 0x8000) {
                if (func_80049864(4) == 0) {
                    func_800498FC(4, 0.0834f, 0.0334f, 0xFF, 0xFF, 0xFF, 0x40);
                }
            }
            D_8007C70C = 0;
        }
    } else if (D_8007C6EC > 0xC000) {
        if (D_8007C708 > 0) {
            D_8007C708 -= updateRate;
        } else {
            delay = (s32) ((D_8007C6EC * 0x258) + 0xFE3E0000) >> 14;
            D_8007C70C = delay + 0x3C;
            D_8007C708 = mathRnd(0x4B0, 0x5DC) - delay;
        }
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::rain_sound. Mickey's sound-handle behavior is authoritative.
 */
void rain_sound(s32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 length;
    f32 cosOffset;
    f32 sinOffset;

    length = 1152.0f - (f32) (D_8007C6EC >> 6);
    cosOffset = func_8002A8C0(-0x8000 - D_800D40DC->rotationX);
    sinOffset = func_8002A8BC(-0x8000 - D_800D40DC->rotationX);
    x = D_800D40DC->x - (length * cosOffset);
    y = D_800D40DC->y;
    z = D_800D40DC->z - (length * sinOffset);
    if (D_8007C720 != NULL) {
        func_800031C0(D_8007C720, x, y, z);
    }
}

/* PLATEAU-HANDOFF:rain_render_splashes:start
 * symbol: rain_render_splashes
 * score: 113 differing words
 * frame: 0xB8
 * relocations: 53
 * first-mismatch: +0x94
 * summary: Size and frame match after JFG while-loop. Remaining 113 is s1-s2-s3 colour (forced floor 44) plus height-result home 0x9C vs 0x84.
 * PLATEAU-HANDOFF:rain_render_splashes:end
 */
