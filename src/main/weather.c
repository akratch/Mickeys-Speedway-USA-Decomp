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
extern f32 D_8007C6C8[4];
extern f32 D_8007C6D8[4];
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
extern s32 osTvType;

extern s32 func_800299E8(s32 min, s32 max);
extern RainPlayer *func_80005820(s32 arg0);
extern s32 func_8001398C(f32 x, f32 z, s32 arg2, RainHeight ***arg3);
extern void camDoSprite(Gfx **dList, Mtx **matrix, WeatherVertex **vertices,
                           RainSplash *splash, void *texture, s32 arg5, s32 arg6);
extern s32 mathRnd(s32 min, s32 max);
extern void *mmAlloc(s32 size, s32 tag);
extern Camera *camGetPtr(void);
extern Matrix *camGetRotationMtx(void);
extern s32 *piRomLoad(s32 assetId);
extern s32 coss_s16(s16 angle);
extern s32 func_8002A1A4(s16 angle);
extern WeatherTexture *texLoadTexture(s32 textureId);
extern s32 func_80049864(s32 mode);
extern void func_800498FC(s32 mode, f32 arg1, f32 arg2, s32 red, s32 green, s32 blue, s32 alpha);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void amSndSetXYZ(void *sound, f32 x, f32 y, f32 z);
extern void *texLoadSprite(s32 assetId, s32 arg1);
extern s32 camGetMode(void);
extern void TrapDanglingJump(f32, f32, f32, s32);
extern WeatherLevel *levelGetLevel(void);
extern void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                        u8 red, u8 green, u8 blue, s8 state);
extern void mmFree(void *ptr);
extern void texFreeTexture(WeatherTexture *texture);
extern Mtx *camGetProjOrgMtx(void);
extern void texDPTextureX(Gfx **dList, void *texture, s32 flags, s32 arg3);

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
    s32 *table;

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
        table = piRomLoad(0x1B);
        D_8007C3E0 = 0;
        D_8007C3DC = table;
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
        texFreeTexture(D_8007C398.source.texture);
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
void setupWeather(s32 type, s32 numParticles, s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity) {
    s16 maxU;
    s16 maxV;
    s32 bufferSize;
    u8 byteCount;
    s32 i;
    WeatherParticle *particle;
    WeatherTriangle *triangle;
    WeatherVertex *vertex;
    s32 j;
    s8 *dest;
    u8 *src;
    s32 pad;
    s32 numOfElements;

    freeWeather();
    D_800D4078.velX = velX;
    D_800D4078.velXStep = 0;
    D_800D4078.velXTarget = velX;
    D_800D4078.velY = velY;
    D_800D4078.velYStep = 0;
    D_800D4078.velYTarget = velY;
    D_800D4078.velZStep = 0;
    D_800D4078.intensityStep = 0;
    D_800D4078.opacityStep = 0;
    D_800D4078.shiftTime = 0;
    D_800D4078.velZ = velZ;
    D_800D4078.velZTarget = velZ;
    D_800D4078.intensity = intensity;
    D_800D4078.intensityTarget = intensity;
    D_800D4078.opacity = opacity;
    D_800D4078.opacityTarget = opacity;
    if (type > 1) {
        type = 1;
    }
    if (D_8007C310[type].source.type == 1) {
        rain_init(numParticles, intensity + 1, opacity + 1, intensity);
        return;
    }
    src = (u8 *) &D_8007C310[type];
    dest = (s8 *) &D_8007C398;
    byteCount = 0x2C;
    while (byteCount--) {
        *dest++ = *src++;
    }
    if (!particle) {
        ;
    }
    D_8007C398.positions = mmAlloc(D_8007C310[type].size * 0xC, 0x93);
    if (D_8007C310[type].source.type == 0) {
        snow_init();
    }
    numOfElements = numParticles;
    D_800D4070 = numParticles;
    D_8007C3D0 = mmAlloc(numParticles * sizeof(s16), 0x93);
    D_8007C394 = mmAlloc(numParticles * sizeof(WeatherParticle), 0x93);
    particle = D_8007C394;
    for (i = 0; i < D_800D4070; i++) {
        particle->x = func_800299E8(0, D_8007C398.radiusX);
        particle->y = func_800299E8(0, D_8007C398.radiusY);
        particle->z = func_800299E8(0, D_8007C398.radiusZ);
        particle->xScale = 1 << (func_800299E8(0, 0x1F) + 5);
        particle->yScale = 1 << (func_800299E8(0, 0x1F) + 5);
        particle->zScale = 1 << (func_800299E8(0, 0x1F) + 5);
        particle->index = mathRnd(0, D_8007C398.size - 1);
        particle++;
    }
    numOfElements *= 4;
    bufferSize = sizeof(WeatherVertex);
    bufferSize *= numOfElements;
    D_8007C3D4[0] = mmAlloc(bufferSize, 0x93);
    D_8007C3D4[1] = mmAlloc(bufferSize, 0x93);
    j = 0;
    do {
        vertex = D_8007C3D4[j];
        for (i = 0; i < numOfElements; i++, vertex++) {
            vertex->r = 0xFF;
            vertex->g = 0xFF;
            vertex->b = 0xFF;
            vertex->a = 0xFF;
        }
        j++;
    } while (&D_8007C3D4[j] < (WeatherVertex **) &D_8007C3DC);
    maxU = (D_8007C398.source.texture->width << 5) - 1;
    maxV = (D_8007C398.source.texture->height << 5) - 1;
    D_8007C3CC = mmAlloc(D_800D40C4 * sizeof(WeatherTriangle), 0x93);
    triangle = D_8007C3CC;
    for (i = 0; i < D_800D40C4; i += 2) {
        triangle[0].flags = 0;
        triangle[0].vi0 = (i << 1) + 3;
        triangle[0].uv0.u = 0;
        triangle[0].uv0.v = maxV;
        triangle[0].vi1 = (i << 1) + 1;
        triangle[0].uv1.u = maxU;
        triangle[0].uv1.v = 0;
        triangle[0].vi2 = i << 1;
        triangle[0].uv2.u = 0;
        triangle[0].uv2.v = 0;
        triangle[1].flags = 0;
        triangle[1].vi0 = (i << 1) + 3;
        triangle[1].uv0.u = 0;
        triangle[1].uv0.v = maxV;
        triangle[1].vi1 = (i << 1) + 2;
        triangle[1].uv1.u = maxU;
        triangle[1].uv1.v = maxV;
        triangle[1].vi2 = (i << 1) + 1;
        triangle[1].uv2.u = maxU;
        triangle[1].uv2.v = 0;
        triangle += 2;
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
    D_8007C398.source.texture = texLoadTexture(*D_8007C3DC);
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::changeWeather. Mickey's condition and assignment ordering
 * are authoritative here.
 */
void changeWeather(s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity, s32 duration) {
    if ((duration > 0) &&
        ((velX != D_800D4078.velXTarget) || (velY != D_800D4078.velYTarget) ||
         (velZ != D_800D4078.velZTarget) || (intensity != D_800D4078.intensity) ||
         (opacity != D_800D4078.opacity))) {
        D_800D4078.velXStep = (s32) ((velX - D_800D4078.velX) / duration);
        D_800D4078.velXTarget = velX;
        D_800D4078.velYStep = (s32) ((velY - D_800D4078.velY) / duration);
        D_800D4078.velYTarget = velY;
        D_800D4078.velZStep = (s32) ((velZ - D_800D4078.velZ) / duration);
        D_800D4078.velZTarget = velZ;
        if (D_8007C6E8 == 0) {
            D_800D4078.intensityTarget = intensity;
            D_800D4078.intensityStep = (s32) ((intensity - D_800D4078.intensity) / duration);
            D_800D4078.opacityStep = (s32) ((opacity - D_800D4078.opacity) / duration);
            D_800D4078.opacityTarget = opacity;
            D_800D4078.shiftTime = duration;
            return;
        }
        D_800D4078.intensity = intensity;
        D_800D4078.opacity = opacity;
        D_800D4078.shiftTime = 0;
        rain_set(intensity + 1, opacity + 1, (f32) duration / 60.0f);
    }
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::doWeather. Mickey's split vertex/render calls and globals
 * are authoritative here.
 */
void doWeather(Gfx **gfxList, Mtx **mtxList, WeatherVertex **vertexList, WeatherTriangle **triangleList, s32 updateRate) {
    D_800D40CC = *gfxList;
    D_800D40D0 = *mtxList;
    D_800D40D4 = *vertexList;
    D_800D40D8 = *triangleList;
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
    *gfxList = D_800D40CC;
    *mtxList = D_800D40D0;
    *vertexList = D_800D40D4;
    *triangleList = D_800D40D8;
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
    texDPTextureX(&dList, D_8007C398.source.texture, 2, 0);
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
    D_8007C714 = texLoadSprite(0x26, 0);
    D_8007C718 = texLoadTexture(0x6A);
    D_8007C6E8 = 1;
    D_800D40E4 = NULL;
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005BD30_5C930 (DKR's free_rain_memory). Mickey's
 * globals and trap binding are authoritative here.
 */
extern void texFreeSprite(void *sprite);
extern void amSndStopXYZ(void *sound);
extern void rainFreeTrap(void);
void free_rain_memory(void) {
    if (D_8007C714 != NULL) {
        texFreeSprite(D_8007C714);
        D_8007C714 = NULL;
    }
    if (D_8007C718 != NULL) {
        texFreeTexture(D_8007C718);
        D_8007C718 = NULL;
    }
    if (D_8007C720 != NULL) {
        amSndStopXYZ(D_8007C720);
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
 * PROVENANCE -- control flow, spawn while-loop, delay increment, splash
 * search, the 0.175f/4.0f literals and the plain 0xFF vertex colours adapted
 * from Jet Force Gemini's public retail-derived
 * src/weather.c::func_8005C188_5CD88 C body. Mickey's single height query,
 * random bounds, type test, and display-list words remain authoritative.
 *
 * Matched 2026-10-02 (lane x-res; earlier passes d-res1, e-res3, w6-rain).
 * What closed it, each measured as a product: the four splash vertices are
 * one counted loop over two four-float offset tables, which IDO's unroller
 * expands into the target's four copies (100 -> 8, the s1/s2/s3 ranking that
 * every earlier pass worked on is the unrolled loop's, not a colour fact); the
 * age step is the 0.175f literal (a loop-invariant constant web, where a
 * global load or an `age` carrier swaps f20/f22); each display-list packet is
 * one GBI-style macro on one line, which gives the target's w1-before-w0
 * store order for every independent packet with no per-packet reordering;
 * the block-scope packet pointers and two unused declarations place the
 * height-result home at 0x84 in the 0xB8 frame.
 */
#define RAIN_PACKET(pkt, word0, word1)                                        \
    {                                                                          \
        Gfx *_g = (Gfx *) (pkt);                                               \
        _g->w0 = (word0);                                                      \
        _g->w1 = (word1);                                                      \
    }
void rain_render_splashes(s32 updateRate) {
    s32 unused0;
    RainSplash *splash;
    RainPlayer *player;
    s32 density;
    s32 index;
    s32 found;
    s32 temp;
    f32 radius;
    f32 x;
    f32 z;
    s32 j;
    s32 unused1;
    RainHeight **heightResult;

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
    RAIN_PACKET(D_800D40CC++, 0xFB000000, (u32) -0x100);
    splash = D_8007C3E4;
    for (index = 0; index < 0x10; index++, splash++) {
        if (splash->state != 0) {
            splash->age += updateRate * 0.175f;
            if (splash->age < 4.0f) {
                if (splash->state == 1) {
                    RAIN_PACKET(D_800D40CC++, 0xFA000000, (u32) ((splash->alpha & 0xFF) | ~0xFF));
                    camDoSprite(&D_800D40CC, &D_800D40D0, &D_800D40D4,
                                  splash, D_8007C714, 0xE, 0);
                } else {
                    texDPTextureX(&D_800D40CC, D_8007C718, 0xE, 0);
                    RAIN_PACKET(D_800D40CC++, 0xFA000000, 0xC0E0FFFF);
                    RAIN_PACKET(D_800D40CC++, ((((((s32) D_800D40D4 + 0x80000000) & 6) | 0x20) & 0xFF) << 16) | 0x04000000 | 0x30, (u32) ((s32) D_800D40D4 + 0x80000000));
                    RAIN_PACKET(D_800D40CC++, 0x05110020, (u32) &D_7C6A8);
                    for (j = 0; j < 4; j++) {
                        D_800D40D4->x = (D_8007C6C8[j] * splash->age) + splash->x;
                        D_800D40D4->y = splash->height;
                        D_800D40D4->z = (D_8007C6D8[j] * splash->age) + splash->z;
                        D_800D40D4->r = 0xFF;
                        D_800D40D4->g = 0xFF;
                        D_800D40D4->b = 0xFF;
                        D_800D40D4->a = 0xFF;
                        D_800D40D4++;
                    }
                }
            } else {
                splash->state = 0;
            }
        }
    }
    RAIN_PACKET(D_800D40CC++, 0xFA000000, (u32) -1);
}
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
        amSndSetXYZ(D_8007C720, x, y, z);
    }
}
