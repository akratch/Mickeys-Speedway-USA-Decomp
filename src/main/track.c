/*
 * Resident track renderer, collision, and fog code.
 * ROM 0xC950-0x16140 (VRAM 0x8000BD50-0x80015540).
 *
 * PROVENANCE -- TU attribution and reference names come from Jet Force
 * Gemini's public decomp, `src/track.c` and its built `src/track.c.o`. The
 * 66-function Mickey block follows that TU's distinctive order from the
 * update/draw/sky routines through texture scrolling, track lights, collision
 * queries, and fog, ending with the same display-list helper. Mickey's own
 * strings, calls, function boundaries, and bytes decide every disagreement.
 * Adapted bodies keep a PROVENANCE note at their point of use.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul.
 */

#include "game/track.h"
#include "game/charControl.h"
#include "game/math.h"
#include "n_audio/mbi.h"
#include "PR/os_internal.h"

typedef struct TrackRotation {
    s16 x;
    s16 y;
    s16 z;
} TrackRotation;

typedef struct TrackLocalTransform {
    s16 xRotation;
    s16 yRotation;
    s16 zRotation;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} TrackLocalTransform;

typedef struct TrackCachedPoint {
    s32 x;
    s32 y;
    s32 z;
} TrackCachedPoint;

typedef struct TrackFloatRecord {
    f32 x;
    f32 y;
    f32 z;
    f32 unkC;
} TrackFloatRecord;

typedef struct TrackKeyRecord {
    s16 key;
    s16 sortValue;
    u8 pad04[4];
} TrackKeyRecord;

typedef struct TrackPlanePoints {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
} TrackPlanePoints;

typedef struct TrackPlane {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
} TrackPlane;

typedef struct TrackLightColourEntry {
    s8 red;
    s8 green;
    s8 blue;
} TrackLightColourEntry;

typedef struct TrackLight {
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 secondaryRadius;
    f32 radiusSquared;
    f32 secondaryRadiusSquared;
    f32 falloff;
    TrackLightColourEntry colours[32];
} TrackLight;

typedef struct TrackLightAllocation {
    void *source;
    void *data;
} TrackLightAllocation;

typedef struct TrackTextureHeader {
    u8 pad00[4];
    u16 flags;
    u16 width;
    u16 height;
    u8 pad0A[6];
    u16 numOfTextures;
    u16 frameAdvanceDelay;
    Gfx *displayList;
    u8 pad18[3];
    u8 unk1B;
} TrackTextureHeader;

typedef struct TrackTextureEntry {
    TrackTextureHeader *texture;
    u32 pad04;
} TrackTextureEntry;

typedef struct TrackTextureLoadLocals {
    s32 pad20;
    void *activeTextureAddress;
    void *textureAddress;
    s32 useOriginalTexture;
    s32 pad30[4];
    s32 activeMaskS;
    s32 pad44;
    s32 maskT;
    s32 maskS;
} TrackTextureLoadLocals;

typedef struct TrackBatch {
    u8 textureIndex;
    u8 unk1;
    u8 pad02[4];
    s16 u0;
    s16 v0;
    u16 frame;
    u32 flags;
} TrackBatch;

typedef struct TrackSegment {
    void *lightData;
    void *vertexData;
    u8 pad08[0xC - 0x08];
    TrackBatch *batches;
    u32 *visibilityMasks;
    u8 pad14[0x18 - 0x14];
    u16 *surfaceIndices;
    TrackPlane *surfaces;
    s16 lightBatchCount;
    u8 pad22[0x24 - 0x22];
    s16 batchCount;
    u8 pad26[0x2C - 0x26];
    s16 unk2C;
    s8 lightingMode;
    u8 pad2F[0x30 - 0x2F];
    void *unk30;
    u8 pad34[0x38 - 0x34];
    void *unk38;
    u8 pad3C[0x40 - 0x3C];
} TrackSegment;

typedef struct TrackLightSource {
    u8 *source;
    u16 *dirtyMasks;
} TrackLightSource;

/*
 * PROVENANCE: field order comes from Diddy Kong Racing's public
 * `include/structs.h`, type `LevelModelSegmentBoundingBox`. Mickey's
 * 12-byte accessor stride independently confirms the layout size.
 */
typedef struct TrackBoundingBox {
    s16 x1;
    s16 y1;
    s16 z1;
    s16 x2;
    s16 y2;
    s16 z2;
} TrackBoundingBox;

typedef union TrackSegmentIndex {
    s32 value;
    TrackBoundingBox *bounds;
} TrackSegmentIndex;

typedef struct TrackBspNode {
    s16 left;
    s16 right;
    u8 axis;
    u8 segmentIndex;
    s16 splitValue;
} TrackBspNode;

typedef struct TrackData {
    TrackTextureEntry *textures;
    TrackSegment *segments;
    TrackBoundingBox *segmentBounds;
    u8 pad0C[0x10 - 0x0C];
    s32 *visibility;
    void *bspTree;
    s16 textureCount;
    s16 segmentCount;
} TrackData;

typedef struct TrackLevelData {
    u8 pad00[0x22];
    s8 unk22;
    u8 pad23[0x52 - 0x23];
    s8 skyMode;
    u8 skyRotationSpeed;
    u8 pad54[0xB2 - 0x54];
    u8 skyScaleS;
    u8 skyScaleT;
    u8 padB4[4];
    TrackTextureHeader *skyTexture;
    s16 skyOffsetS;
    s16 skyOffsetT;
    u8 padC0[0xD2 - 0xC0];
    u8 bottomR;
    u8 bottomG;
    u8 bottomB;
    u8 topR;
    u8 topG;
    u8 topB;
} TrackLevelData;

typedef struct TrackVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} TrackVertex;

typedef struct TrackTriangle {
    u8 flags;
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    s16 u0;
    s16 v0;
    s16 u1;
    s16 v1;
    s16 u2;
    s16 v2;
} TrackTriangle;

typedef struct TrackIntersection {
    f32 height;
    s32 flags;
} TrackIntersection;

typedef struct TrackFogChangerData {
    u8 pad00[0x0B];
    u8 red;
    u8 green;
    u8 blue;
    s16 near;
    s16 far;
    s16 duration;
} TrackFogChangerData;

typedef struct TrackFogChanger {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x3C - 0x18];
    TrackFogChangerData *data;
    u8 pad40[0x84 - 0x40];
    f32 radiusSquared;
} TrackFogChanger;

typedef struct TrackFogPlayerState {
    s8 fogIndex;
} TrackFogPlayerState;

typedef struct TrackFogPlayer {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x64 - 0x18];
    TrackFogPlayerState *state;
} TrackFogPlayer;

typedef struct TrackFallbackPlayer {
    u8 pad00[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x54 - 0x18];
} TrackFallbackPlayer;

typedef struct TrackCamera {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    u8 pad06[0xC - 6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x30 - 0x18];
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    u8 pad3C[0x3E - 0x3C];
    s16 segmentIndex;
} TrackCamera;

typedef struct TrackSkyMaterial {
    u8 pad00[0xA2];
    u8 textureIndex;
} TrackSkyMaterial;

typedef struct TrackSkyObject {
    s16 rotationY;
    u8 pad02[0xC - 2];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x40 - 0x18];
    TrackSkyMaterial *material;
} TrackSkyObject;

#define TRACK_SP_VERTEX(packet, vertex, count, first)                      \
    gDma1p(packet, G_VTX, vertex,                                         \
           (((count) << 3) + ((count) << 1)) + 8,                         \
           ((count) << 3) | (((u32) (vertex)) & 6) | (first))

#define TRACK_SP_POLYGON(packet, address, count, textured)                 \
    {                                                                      \
        Gfx *_g = (Gfx *) (packet);                                        \
        _g->words.w0 = _SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
                       _SHIFTL(5, 24, 8) | _SHIFTL((count) << 4, 0, 16);  \
        _g->words.w1 = (u32) (address);                                    \
    }

extern TrackCamera *D_800C9530;
extern TrackCachedPoint D_800C9B40;
extern Gfx *D_800C9520;
extern s32 D_8007C854;
extern s32 D_8007C858;
extern s32 D_800C9544;
extern s32 D_80079314;
extern u32 D_800C9B50[16];
extern s32 D_800792FC;
extern u8 D_8007BEF4;
extern s16 D_800C9570;
extern TrackData *D_800792E8;
extern TrackLevelData *D_800792EC;
extern s32 D_80078F84;
extern f32 D_800C99B0;
extern f32 D_800C99B4;
extern f32 D_800C99B8;
extern Mtx *D_800C9524;
extern TrackVertex *D_800C9528;
extern TrackTriangle *D_800C952C;
extern u8 D_79330[];
extern TrackTextureHeader *D_800792F0;
extern s32 D_800792F4;
extern Gfx D_80079358[];
extern Gfx D_80079380[];
extern Gfx D_800793A8[];
extern Gfx D_800793D8[];
extern u8 D_80079318[];
extern s32 D_800C9560;
extern s32 D_800C954C;
extern s32 D_800C9554;
extern s32 D_800C955C;
extern s32 D_800C9564;
extern s32 D_800C956C;
extern void *D_800C9574;
extern TrackLight *D_80079300;
extern s32 D_80079304;
extern TrackLightAllocation *D_80079308;
extern s32 D_800792F8;
extern s32 D_80079350;
extern s32 D_80079354;
extern f32 D_80081690;
extern f32 D_80081770;
extern f32 D_80081774;
extern f32 D_80081790;
extern f32 D_80081778;
extern f32 D_8008177C;
extern f32 D_80081780;
extern f32 D_80081784;
extern f32 D_80081788;
extern f32 D_8008178C;
extern s8 D_80079260;
extern s8 D_80079264;
extern s8 D_80079268;
extern s32 D_8007A124;
extern s32 D_800C9544;
extern s32 D_800C95B0[];
extern s32 D_800C95B4[];
extern s16 D_800D6C4C;
extern s16 D_800D6C54;
extern u8 D_80079274;
extern s32 D_80079278;
extern s32 D_8007930C;
extern void *D_80079310;
extern void *D_800C9548;
extern void *D_800C95A8;
extern void *D_800C9D20;
extern s32 *D_800C9D2C;
extern s32 D_800C9D3C;
extern s16 *D_800C9D30;
extern s16 *D_800C9D34;
extern s32 D_800C9D24;
extern s32 D_800C9D28;
extern void *D_8007926C;
extern s32 D_800C953C;
extern TrackPlanePoints D_8007927C[3];
extern TrackPlane D_800C9578[3];
extern u8 D_800C9B90[];
extern void *D_800C9CD0[];
extern s32 D_800C9D24;

void func_8002AB78(TrackLocalTransform *transform, MtxF matrix);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
TrackFogPlayer **func_80005750(s32 *count);
void func_800367E8(TrackTextureHeader *texture, u32 *flags, s32 *frame,
                   s32 updateRate);
void func_80014ECC(TrackTextureHeader *texture, s32 frame, s32 flags);
s32 runlinkIsModuleLoaded(s32 module);
s32 TrapDanglingJump();
void func_8000A62C(f32 x, f32 y, f32 z);
void func_8000E5EC(s32 arg0, s32 arg1);
void func_8000E920(s32 arg0, s32 arg1);
void func_80014DE4(void);
void camStandardOrtho(Gfx **displayList, Mtx **matrix);
void func_80034920(Gfx **displayList);
void func_800349A4(Gfx **displayList, void *texture, s32 mode, s32 flags);
void func_800221E8(Gfx **displayList, Mtx **matrix);
s32 camGetMode(void);
s32 camGetNo(void);
void func_80021FB0(s32 mode, s32 camera, s32 *left, s32 *bottom,
                   u32 *right, u32 *top);
void viGetCurrentSize(s32 *width, s32 *height);
void *func_800348D4(TrackTextureHeader *texture, s32 frame);
TrackCamera *camGetPtr(void);
TrackLight *trackLightAsm(TrackData *track, TrackLight *light, void *state);
s32 mainGetNumberOfCameras(void);
f32 func_8002A8BC(s32 angle);
s32 func_80013324(f32 coefficient, f32 numerator,
                  f32 *minimum, f32 *maximum);
f32 func_8002A8C0(s32 angle);
void func_8000F82C(s32 start, s32 count, s32 end);
s32 func_80010178(u32 segmentIndex);
s32 func_800103D4(void *object);
u8 *func_80028F54(void);
f32 camDistance(f32 x, f32 y, f32 z);
u8 *levelGetLevel(void);
void partDraw(Gfx **displayList, s32 arg1, s32 mode);
void func_8000DFBC(s32 segment, s32 arg1, s32 arg2, s32 arg3);
s32 func_8000DDE4(s32 key, s32 recordCount, TrackKeyRecord *records, TrackKeyRecord **matches);
void func_8000F57C(s32 *resultCount, u8 *resultSegments);
void func_8000FA2C(s32 *result, s32 arg1);
void shadowGetBuffers(s32 mode, void **a, void **b, void **c);
void func_800343F0();
void texEnableModes(s32 mode);
s32 getXZCompareMask(TrackBoundingBox *bounds, s32 x0, s32 z0, s32 x1,
                     s32 z1);
void func_800133FC(TrackVertex *v0, TrackVertex *v1, TrackVertex *v2,
                   f32 *a, f32 *b, f32 *c, f32 *d);
s32 mathXZInTri(s32 x, s32 z, TrackVertex *v0, TrackVertex *v1,
                TrackVertex *v2);
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity);
void *func_8002B280(s32 size, s32 tag);
void func_8000D570(void);
void func_8000D820(void);
void func_8000439C(void);
void func_80006EA0(void *handle);
void func_80006FA0(void);
void func_8001F364(void);
void func_800347A0(void *texture);
void mmFree(void *data);
void shadowFreeBuffers(void);
void animseqFreeLevelData(void);
f32 (*camGetInvProjMtx(void))[4];
f32 sqrtf(f32 value);
void func_80007E40(TrackSkyObject *object, s32 updateRate,
                   TrackLevelData **levelData);
void func_80009E78(Gfx **displayList, Mtx **matrix, TrackVertex **vertices,
                   TrackSkyObject *object);

/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c`, function
 * `trackUpdateFX`, supplies the three-module update structure. Mickey proves
 * its own module indices and unresolved call sites, so the name is not adopted.
 */
void func_8000BD50(s32 updateRate) {
    if (runlinkIsModuleLoaded(13) != 0) {
        TrapDanglingJump(updateRate);
    }
    if (runlinkIsModuleLoaded(12) != 0) {
        TrapDanglingJump(updateRate);
    }
    if (runlinkIsModuleLoaded(34) != 0) {
        TrapDanglingJump(updateRate);
    }
}
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident display-list,
 * camera, level, and weather declarations establish this update routine's
 * game-specific behavior. The donor adaptation is disclosed below.
 */
typedef struct TrackFrameTexture {
    u8 pad00[0x10];
    u16 unk10;
    u16 unk12;
} TrackFrameTexture;

typedef struct TrackFrameLevel {
    u8 pad00[0x52];
    s8 unk52;
    u8 pad53[0x30];
    s8 unk83;
    u8 pad84[0x1E];
    s16 unkA2;
    u8 padA4[0x10];
    s8 unkB4;
    s8 unkB5;
    u8 padB6[2];
    TrackTextureHeader *unkB8;
    s16 unkBC;
    s16 unkBE;
    u8 padC0[0x11];
    s8 unkD1;
} TrackFrameLevel;

extern u8 D_80081540[];
extern u8 D_80081550[];
extern s32 D_800C9534;
extern s32 D_800C9538;
extern s32 D_800C9568;
extern u8 D_8007A128;
extern s32 D_8007D6B0;
extern s16 D_800D6C3E;
extern void func_8000C400(s32);
extern void func_8000C5F4(void);
extern void func_8000CC78(void);
extern void func_8000CED0(s32);
extern void func_8000D018(s32, s32);
extern void func_8000FF2C(void);
extern void func_80014614(s32, s32);
extern void func_800147A4(s32);
extern s32 func_800290A0(void);
extern s32 levelInitRegionFlags(void);
extern void camDisableUserView();
extern void camEnableUserView();
extern void camSetNo();
extern void doWeather();
extern void func_800219D0(void);
extern void func_80022D20();
extern void func_80036CAC();
extern void func_80044BC8();
extern void func_800534EC();
extern void levelUpdateColourCycling();
extern void rainSetFog();
extern void shadowChangeBuffer();
extern void shadowGenerate();
extern void weather_clip_planes();

/*
 * PROVENANCE: adapted from Jet Force Gemini's public src/track.c,
 * trackDraw at efd5abb1c79636e297b831f7c2d5bf47eac39c0c. The donor supplies
 * the texture-update loop, display-list macro and camera-loop spelling.
 * Mickey's ROM proves the ABI, revised fields, branches and call order;
 * game-specific JFG paths are not imported.
 */
void func_8000BDB4(Gfx **arg0, Mtx **arg1, TrackVertex **arg2,
                   TrackTriangle **arg3, s32 arg4) {
    s32 temp_a0;
    s32 temp_s2;
    s32 targetUpdateRate;
    s32 var_v0;

    temp_s2 = mainGetNumberOfCameras();
    camSetNo(0);
    if (TrapDanglingJump() != 0) {
        TrapDanglingJump(arg0);
        return;
    }
    D_800C9520 = *arg0;
    D_800C9524 = *arg1;
    D_800C9528 = *arg2;
    D_800C952C = *arg3;
    func_80044BC8(D_800C9520, (char *) D_80081540, 0x1CC);
    D_800C9558 = 1;
    D_800C9538 = 0;
    if (func_800290A0() != 0) {
        targetUpdateRate = 0;
    } else {
        targetUpdateRate = arg4;
    }
    if (D_800792F0 != NULL) {
        var_v0 = D_800792F4;
        var_v0 += ((TrackFrameTexture *) D_800792F0)->unk12 * targetUpdateRate;
        while (var_v0 >= ((TrackFrameTexture *) D_800792F0)->unk10) {
            var_v0 -= ((TrackFrameTexture *) D_800792F0)->unk10;
        }
        D_800792F4 = var_v0;
    }
    shadowGenerate(1, arg4);
    levelUpdateColourCycling(targetUpdateRate);
    temp_a0 = *(s32 *) ((u8 *) D_800792EC + 0xC0);
    if (temp_a0 != -1) {
        func_80036CAC(temp_a0, targetUpdateRate);
    }
    if (((TrackFrameLevel *) D_800792EC)->unk83 == 2) {
        D_80079260 = 0;
    } else {
        D_80079260 = 1;
    }
    if ((((TrackFrameLevel *) D_800792EC)->unk83 == 1) ||
        (((TrackFrameLevel *) D_800792EC)->unk83 == 2) ||
        (((TrackFrameLevel *) D_800792EC)->unkD1 != 0)) {
        D_800C9544 = 1;
    }
    if (((TrackFrameLevel *) D_800792EC)->unk52 == 3) {
        var_v0 = (((TrackFrameLevel *) D_800792EC)->unkB8->width << 9) - 1;
        ((TrackFrameLevel *) D_800792EC)->unkBC =
            (((TrackFrameLevel *) D_800792EC)->unkBC +
             (((TrackFrameLevel *) D_800792EC)->unkB4 * targetUpdateRate)) & var_v0;
        var_v0 = (((TrackFrameLevel *) D_800792EC)->unkB8->height << 9) - 1;
        ((TrackFrameLevel *) D_800792EC)->unkBE =
            (((TrackFrameLevel *) D_800792EC)->unkBE +
             (((TrackFrameLevel *) D_800792EC)->unkB5 * targetUpdateRate)) & var_v0;
        func_800367E8(((TrackFrameLevel *) D_800792EC)->unkB8,
                      (u32 *) &D_800C9568,
                      &D_800C9560, targetUpdateRate);
    }
    func_80034920(&D_800C9520);
    gMoveWd(D_800C9520++, 2, 0, 0);
    if (levelInitRegionFlags() != 0) {
        gSPClearGeometryMode(D_800C9520++, G_CULL_BACK);
        gSPSetGeometryMode(D_800C9520++, G_CULL_FRONT);
    } else {
        gSPClearGeometryMode(D_800C9520++, G_CULL_FRONT);
        gSPSetGeometryMode(D_800C9520++, G_CULL_BACK);
    }
    gDPSetBlendColor(D_800C9520++, 0, 0, 0, 0x64);
    gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(D_800C9520++, 255, 255, 255, 0);
    rainSetFog();
    func_80014614(temp_s2, targetUpdateRate);
    if (*(s16 *) ((u8 *) D_800792E8 + 0x1E) > 0) {
        func_8000C400(targetUpdateRate);
    }
    if (D_80079274 != 0) {
        TrapDanglingJump((void **) (s32) targetUpdateRate);
    }
    if ((D_8007A128 != 0) && (temp_s2 == 1)) {
        camEnableUserView(0, 1);
        func_800219D0();
    }
    for (D_800C9534 = 0; D_800C9534 < temp_s2; D_800C9534++) {
        func_800147A4(D_800C9534);
        gDPPipeSync(D_800C9520++);
        camSetNo(D_800C9534);
        func_800221E8(&D_800C9520, &D_800C9524);
        func_8000FF2C();
        if (temp_s2 < 3) {
            if (((TrackFrameLevel *) D_800792EC)->unk52 == 3) {
                func_8000C5F4();
            } else if (D_800C9550 != 0) {
                func_8000CED0(arg4);
            }
            if (D_80079278 > 0) {
                if (D_800C9534 == 0) {
                    TrapDanglingJump((void **) (s32) targetUpdateRate);
                }
                TrapDanglingJump(&D_800C9520);
            }
        } else {
            if ((((TrackFrameLevel *) D_800792EC)->unk52 != 4) &&
                (((TrackFrameLevel *) D_800792EC)->unk52 != 5)) {
                func_8000CC78();
            }
        }
        func_80044BC8(D_800C9520, (char *) D_80081550, 0x26A);
        gDPPipeSync(D_800C9520++);
        func_8000D018(temp_s2, arg4);
        weather_clip_planes(-1, -0x200);
        if ((((TrackFrameLevel *) D_800792EC)->unkA2 > 0) &&
            (temp_s2 < 2)) {
            doWeather(&D_800C9520, &D_800C9524,
                      (void *) &D_800C9528, (void *) &D_800C952C,
                      targetUpdateRate);
        }
    }
    if (D_8007D6B0 > 0) {
        TrapDanglingJump(&D_800C9520);
    }
    func_800534EC((s32) &D_800C9520);
    if (D_800D6C3E != 0) {
        TrapDanglingJump(&D_800C9520);
    }
    if (levelInitRegionFlags() != 0) {
        gSPClearGeometryMode(D_800C9520++, G_CULL_FRONT);
        gSPSetGeometryMode(D_800C9520++, G_CULL_BACK);
    }
    func_80022D20(&D_800C9520);
    camDisableUserView(0, 1);
    gDPPipeSync(D_800C9520++);
    gMoveWd(D_800C9520++, 2, 0, 0);
    shadowChangeBuffer();
    *arg0 = D_800C9520;
    *arg1 = D_800C9524;
    *arg2 = D_800C9528;
    *arg3 = D_800C952C;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_800129AC_135AC`. Mickey proves the revised segment and texture layouts
 * and flag bit; the donor's placeholder name is not imported.
 */
void func_8000C400(s32 updateRate) {
    s32 segmentNumber;
    TrackTextureHeader *texture;
    s32 batchNumber;
    TrackBatch *batch;
    TrackSegment *segments;
    s32 frame;

    segments = D_800792E8->segments;
    for (segmentNumber = 0; segmentNumber < D_800792E8->segmentCount; segmentNumber++) {
        batch = segments[segmentNumber].batches;
        for (batchNumber = 0; batchNumber < segments[segmentNumber].batchCount; batchNumber++) {
            if (batch[batchNumber].flags & 0x100000) {
                if (batch[batchNumber].textureIndex != 0xFF) {
                    texture = D_800792E8->textures[batch[batchNumber].textureIndex].texture;
                    if ((texture->numOfTextures != 0x100) && (texture->frameAdvanceDelay != 0)) {
                        frame = batch[batchNumber].frame;
                        func_800367E8(texture, &batch[batchNumber].flags, &frame, updateRate);
                        batch[batchNumber].frame = frame;
                    }
                }
            }
        }
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `initSky`. Mickey adds the player-count guard and proves its own object-field
 * offsets; the public name is not adopted from tier-D TU position alone.
 */
void func_8000C540(s32 arg0) {
    ControlSpawnPacket packet;

    if ((arg0 == -1) || (D_8007BEF4 >= 3)) {
        D_800C9550 = NULL;
        D_800C9570 = arg0;
    } else {
        packet.x = 0;
        packet.y = 0;
        packet.z = 0;
        packet.mode = 10;
        packet.kind = arg0;
        D_800C9550 = func_8000590C(&packet, 2);
        D_800C9570 = arg0;
        if (D_800C9550 != NULL) {
            ((ControlSpawned *) D_800C9550)->unk3C = 0;
            ((ControlSpawned *) D_800C9550)->unk46 = -1;
        }
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function `trackSkySet`. */
void trackSkySet(s32 skyDome) {
    D_800C9558 = skyDome;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_80012BAC_137AC`, with the load-bearing local padding documented by
 * Diddy Kong Racing's public `src/tracks.c` version. Mickey proves the revised
 * level-data offsets, command bindings, and vertex/triangle layouts; the
 * donor's placeholder name is not imported.
 */
void func_8000C5F4(void) {
    TrackTriangle *triangles;
    TrackVertex *vertices;
    s32 maskT;
    s32 maskS;
    f32 scaledXSin;
    f32 scaledXCos;
    f32 var_f16;
    s16 textureS[9];
    s16 textureT[9];
    f32 xCos;
    f32 xSin;
    f32 pad_sp108;
    TrackCamera *camera;
    f32 pad_sp100;
    f32 xPositions[9];
    f32 zPositions[9];
    TrackVec3f pos;
    s32 i;
    s32 var_v0;
    s32 var_v1;
    s32 var_a1;
    s32 var_a2;
    u8 *var_v0_3;
    f32 var_f14;
    s16 vertY;
    s16 vTempCoord;
    s16 uTempCoord;
    TrackTextureHeader *texture;
    /* These donor-shaped locals determine IDO's stack homes and FP colours. */
    s32 pad[4];

    vertices = D_800C9528;
    triangles = D_800C952C;
    camera = camGetPtr();
    texture = D_800792EC->skyTexture;
    D_800C9570 = -1;

    maskS = (texture->width << 5) - 1;
    maskT = (texture->height << 5) - 1;
    xSin = func_8002A8C0(-camera->rotationX);
    xCos = func_8002A8BC(-camera->rotationX);

    scaledXSin = xSin * 1280.0f;
    scaledXCos = xCos * 1280.0f;
    pad_sp100 = 2.0f * scaledXSin;
    xPositions[0] = -scaledXCos - (xSin * 1280.0f);
    zPositions[0] = -scaledXCos + (xSin * 1280.0f);
    xPositions[1] = scaledXCos - (xSin * 1280.0f);
    zPositions[1] = -scaledXCos - (xSin * 1280.0f);
    xPositions[2] = scaledXCos + scaledXSin;
    zPositions[2] = scaledXCos - (xSin * 1280.0f);
    xPositions[3] = -scaledXCos + (xSin * 1280.0f);
    zPositions[3] = scaledXCos + (xSin * 1280.0f);
    xPositions[4] = 0.0f;
    zPositions[4] = 0.0f;

    xPositions[5] = -(xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[5] = scaledXSin + -(2.0f * (xCos * 1280.0f));
    xPositions[6] = (xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[6] = -(2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[7] = (xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[7] = (2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[8] = -(xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[8] = (2.0f * (xCos * 1280.0f)) + scaledXSin;

    scaledXCos = 1280.0f;
    var_f14 = scaledXCos * 0.25f;
    var_a1 = texture->width * 16 * D_800792EC->skyScaleS;
    var_a2 = texture->height * 16 * D_800792EC->skyScaleT;
    var_v0 = ((s32)(camera->x * ((scaledXCos * 0.25f) / var_a1)) +
              (D_800792EC->skyOffsetS >> 4)) & maskS;
    var_v1 = ((s32)(camera->z * ((scaledXCos * 0.25f) / var_a2)) +
              (D_800792EC->skyOffsetT >> 4)) & maskT;

    var_f14 = var_a1 * xCos;
    pos.f[2] = var_a1 * xCos;
    pos.f[0] = var_a1 * xCos;
    var_f16 = var_a2 * xSin;
    xCos = var_f16;
    pad_sp108 = var_f16;

    var_a2 = texture->height * 16 * D_800792EC->skyScaleT;

    textureS[0] = (s16)(-var_f14 - pad_sp108) + var_v0;
    textureT[0] = (s16)(var_f16 - var_f14) + var_v1;
    textureS[1] = (s16)(var_f14 - pad_sp108) + var_v0;
    textureT[1] = (s16)(-var_f14 - var_f16) + var_v1;
    textureS[2] = (s16)(var_f14 + var_f16) + var_v0;
    textureT[2] = (s16)(var_f14 - var_f16) + var_v1;
    textureS[3] = (s16)(var_f16 - var_f14) + var_v0;
    textureT[3] = (s16)(var_f14 + var_f16) + var_v1;
    textureS[4] = var_v0;
    textureT[4] = var_v1;
    textureS[5] = (s16)(-var_f14 - (2.0f * xCos)) + var_v0;
    textureT[5] = (s16)(var_f16 - (2.0f * var_f14)) + var_v1;
    textureS[6] = (s16)(var_f14 - (2.0f * xCos)) + var_v0;
    textureT[6] = (s16)(-(2.0f * var_f14) - var_f16) + var_v1;
    textureS[7] = (s16)(pos.f[2] + (2.0f * xCos)) + var_v0;
    textureT[7] = (s16)((2.0f * pos.f[0]) - var_f16) + var_v1;
    textureS[8] = (s16)((2.0f * xCos) - pos.f[2]) + var_v0;
    textureT[8] = (s16)((2.0f * pos.f[0]) + var_f16) + var_v1;

    func_800349A4(&D_800C9520, texture, 0x10, D_800C9560 << 8);
    gDPSetPrimColor(D_800C9520++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor(D_800C9520++, 0xFF, 0xFF, 0xFF, 0xFF);
    TRACK_SP_VERTEX(D_800C9520++, (u32)D_800C9528 + 0x80000000, 9, 0);
    TRACK_SP_POLYGON(D_800C9520++, (u32)D_800C952C + 0x80000000, 8, 1);
    gDPPipeSync(D_800C9520++);

    vertY = camera->y + 192.0f;
    for (i = 0; i < 9; i++) {
        vertices->x = xPositions[i] + camera->x;
        vertices->y = vertY;
        vertices->z = zPositions[i] + camera->z;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = (i <= 4) ? 0xFF : 0;
        vertices++;
    }

    var_v0_3 = D_80079318;
    for (i = 0; i < 8; i++) {
        triangles->flags = 0x40;
        triangles->vertex0 = *var_v0_3;
        triangles->u0 = textureS[*var_v0_3];
        triangles->v0 = textureT[*var_v0_3];
        var_v0_3++;
        triangles->vertex1 = *var_v0_3;
        triangles->u1 = textureS[*var_v0_3];
        triangles->v1 = textureT[*var_v0_3];
        var_v0_3++;
        triangles->vertex2 = *var_v0_3;
        triangles->u2 = textureS[*var_v0_3];
        triangles->v2 = textureT[*var_v0_3];
        var_v0_3++;
        triangles++;
    }

    D_800C9528 = vertices;
    D_800C952C = triangles;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `func_80013454_14054`, including its display-list command structure. Mickey
 * proves the ten-byte vertex layout and all resident function bindings; the
 * donor's placeholder name is not imported.
 */
void func_8000CC78(void) {
    s32 width;
    s32 height;
    s32 left;
    s32 bottom;
    u32 right;
    u32 top;
    u8 topR;
    u8 topG;
    u8 topB;
    u8 bottomR;
    u8 bottomG;
    u8 bottomB;
    TrackVertex *vertices;

    vertices = D_800C9528;
    D_800C9570 = -1;
    camStandardOrtho(&D_800C9520, &D_800C9524);
    func_80034920(&D_800C9520);
    func_800349A4(&D_800C9520, NULL, 8, 0);

    TRACK_SP_VERTEX(D_800C9520++, (u32) vertices + 0x80000000, 4, 0);
    TRACK_SP_POLYGON(D_800C9520++, D_79330, 2, 0);

    func_800221E8(&D_800C9520, &D_800C9524);
    topR = D_800792EC->topR;
    topG = D_800792EC->topG;
    topB = D_800792EC->topB;
    bottomR = D_800792EC->bottomR;
    bottomG = D_800792EC->bottomG;
    bottomB = D_800792EC->bottomB;
    viGetCurrentSize(&width, &height);
    func_80021FB0(camGetMode(), camGetNo(), &left, &bottom, &right, &top);
    width = (u32) width >> 1;
    height = (u32) height >> 1;

    vertices->x = left - (u32) width;
    vertices->y = (u32) height - top;
    vertices->z = 0x10;
    vertices->r = topR;
    vertices->g = topG;
    vertices->b = topB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = right - (u32) width;
    vertices->y = (u32) height - top;
    vertices->z = 0x10;
    vertices->r = topR;
    vertices->g = topG;
    vertices->b = topB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = left - (u32) width;
    vertices->y = (u32) height - bottom;
    vertices->z = 0x10;
    vertices->r = bottomR;
    vertices->g = bottomG;
    vertices->b = bottomB;
    vertices->a = 0xFF;
    vertices++;

    vertices->x = right - (u32) width;
    vertices->y = (u32) height - bottom;
    vertices->z = 0x10;
    vertices->r = bottomR;
    vertices->g = bottomG;
    vertices->b = bottomB;
    vertices->a = 0xFF;
    vertices++;

    D_800C9528 = vertices;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c` and
 * assembly-only `func_80013478`. Mickey proves the revised mode test, field
 * offsets, calls, and final draw condition; the donor's placeholder name is
 * not imported.
 */
void func_8000CED0(s32 updateRate) {
    TrackCamera *camera;

    if (D_800C9550 != NULL) {
        camera = camGetPtr();
        if ((D_800792EC->skyMode != 2) && (D_800792EC->skyMode != 5)) {
            ((TrackSkyObject *) D_800C9550)->x = camera->x + camera->offsetX;
            ((TrackSkyObject *) D_800C9550)->y = camera->y + camera->offsetY;
            ((TrackSkyObject *) D_800C9550)->z = camera->z + camera->offsetZ;
            ((TrackSkyObject *) D_800C9550)->rotationY +=
                D_800792EC->skyRotationSpeed * updateRate;
            if (((TrackSkyObject *) D_800C9550)->material->textureIndex !=
                0xFF) {
                func_80007E40(D_800C9550, updateRate, &D_800792EC);
            }
        } else {
            ((TrackSkyObject *) D_800C9550)->x = camera->offsetX;
            ((TrackSkyObject *) D_800C9550)->y = camera->offsetY;
            ((TrackSkyObject *) D_800C9550)->z = camera->offsetZ;
        }
        if (D_800C9558 != 0) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          D_800C9550);
        }
    }
}
/*
 * JFG's corresponding TU position is `trackGetSky`, but this three-word
 * Mickey function is kept unnamed because it has no adoptable naming tier.
 */
void *func_8000D00C(void) {
    return D_800C9550;
}
/* Dangling overlay call taking the camera world position (three f32 args in
 * f12/f14/a2-raw) and returning a pointer. Typed weak alias so this call site
 * passes single-precision floats without the default-argument double promotion
 * that the unprototyped `s32 TrapDanglingJump()` forces; the build canonicalizes
 * the undefined symbol to the shared TrapDanglingJump target (0x800333A0)
 * without changing section contents. */
#pragma weak trackCamPosTrap = TrapDanglingJump
extern void *trackCamPosTrap(f32, f32, f32);
/* PROVENANCE: the camera/update structure is adapted from Jet Force Gemini's
 * public src/track.c TU position (`func_800135E0`); Mickey's fields, globals,
 * call graph, and instruction boundary remain authoritative. */
void func_8000D018(s32 arg0, s32 arg1) {
    TrackData *track;
    s16 segmentIndex;

    D_800C9530 = camGetPtr();
    func_80014DE4();
    func_8000A62C((f32) D_800C9B40.x / 65536.0f,
                  (f32) D_800C9B40.y / 65536.0f,
                  (f32) D_800C9B40.z / 65536.0f);
    segmentIndex = D_800C9530->segmentIndex;
    if ((segmentIndex >= 0) &&
        ((track = D_800792E8), segmentIndex < track->segmentCount)) {
        D_800C953C = track->segments[segmentIndex].unk2C;
    } else {
        D_800C953C = -1;
    }
    D_800C99B0 = D_800C9530->x;
    D_800C99B4 = D_800C9530->y;
    D_800C99B8 = D_800C9530->z;
    if (D_80078F84 > 0) {
        D_8007926C = trackCamPosTrap(D_800C9530->x, D_800C9530->y,
                                     D_800C9530->z);
    } else {
        D_8007926C = 0;
    }
    if (D_800792EC->unk22 != 0) {
        func_8000E5EC(arg0, arg1);
        return;
    }
    func_8000E920(arg0, arg1);
}
void func_8000D16C(s32 arg0, s32 arg1, s32 arg2) {
    if (D_80079314 < 16) {
        D_800C9B50[D_80079314] =
            (arg0 << 24) | ((arg1 & 0xFFF) << 12) | (arg2 & 0xFFF);
        D_80079314++;
    }
}
/* PROVENANCE: Mickey's target accesses reconstruct the packed-scroll and
 * nested segment/batch/vertex loops; Jet Force Gemini's assembly-only
 * trackUpdateTextureScroll supplies TU-position and role context only. */
/*
 * Matched by reusing the packed command word as the texture index once both
 * scroll deltas have been shifted out of it (so the deltas stay in the outer
 * loop instead of being propagated into the triangle loop), shifting each
 * delta left and then right in place, reading the segment count after the
 * mask selection, and forming the triangle cursor before its count.
 */
void func_8000D1B8(void) {
    u32 *command;
    s32 count;
    u32 packed;
    s32 scrollU;
    s32 scrollV;
    TrackTextureHeader *texture;
    s32 maskU;
    s32 maskV;
    s32 segmentCount;
    TrackSegment *segment;
    s32 batchCount;
    TrackBatch *batch;
    s32 triangleCount;
    TrackTriangle *triangle;
    s32 u;
    s32 v;
    s32 du1;
    s32 dv1;
    s32 du2;
    s32 dv2;

    if (D_800792E8 != NULL) {
        count = D_80079314;
        if (count != 0) {
            command = D_800C9B50;
            while (count--) {
                packed = *command++;
                scrollU = packed << 8;
                scrollV = packed << 20;
                packed = ((s32) packed >> 24) & 0xFF;
                scrollU >>= 20;
                scrollV >>= 20;
                texture = D_800792E8->textures[packed].texture;
                segment = D_800792E8->segments;
                if (texture->width < 65 && texture->height < 65) {
                    maskU = (texture->width << 8) - 1;
                    maskV = (texture->height << 8) - 1;
                } else {
                    maskU = (texture->width << 6) - 1;
                    maskV = (texture->height << 6) - 1;
                }
                segmentCount = D_800792E8->segmentCount;
                while (segmentCount--) {
                    batchCount = segment->batchCount;
                    batch = segment->batches;
                    while (batchCount--) {
                        if (packed == batch->textureIndex) {
                            triangle = (TrackTriangle *) segment->vertexData + batch->v0;
                            triangleCount = batch[1].v0 - batch[0].v0;
                            while (triangleCount--) {
                                u = triangle->u0;
                                v = triangle->v0;
                                du1 = triangle->u1 - u;
                                dv1 = triangle->v1 - v;
                                du2 = triangle->u2 - u;
                                dv2 = triangle->v2 - v;
                                u = (u + scrollU) & maskU;
                                v = (v + scrollV) & maskV;
                                triangle->u0 = u;
                                triangle->v0 = v;
                                triangle->u1 = u + du1;
                                triangle->v1 = v + dv1;
                                triangle->u2 = u + du2;
                                triangle->v2 = v + dv2;
                                triangle++;
                            }
                        }
                        batch++;
                    }
                    segment++;
                }
            }
        }
    }
    D_80079314 = 0;
}
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackLightAllocate`
 * establishes the pool/segment allocation role. Mickey's +0x20 lighting
 * count and two-pointer allocation record are reconstructed from the target
 * accesses; the donor placeholder name is not adopted.
 */
void func_8000D3B8(s32 lightCount, s32 copyData) {
    s32 index;
    s32 copyFailed;
    s32 byteCount;
    u8 *source;
    u8 *destination;
    TrackSegment *segment;
    TrackLightAllocation *allocation;

    D_800792FC = 0;
    D_800792F8 = lightCount;
    copyFailed = 1;
    D_80079300 = func_8002B280(D_800792F8 * sizeof(TrackLight), 0x91);
    if (D_80079300 != NULL) {
        index = D_800792F8;
        while (index--) {
            D_80079300[index].radius = 0.0f;
        }
        copyFailed = copyData;
        if (copyData != 0) {
            allocation = func_8002B280(
                D_800792E8->segmentCount * sizeof(TrackLightAllocation), 0x91);
            if (allocation != NULL) {
                segment = D_800792E8->segments;
                D_80079304 = 1;
                D_80079308 = allocation;
                index = 0;
                copyFailed = 0;
                if (D_800792E8->segmentCount > 0) {
                    do {
                        source = segment->lightData;
                        byteCount = segment->lightBatchCount * 10;
                        allocation->source = source;
                        allocation->data = func_8002B280(byteCount, 0x91);
                        destination = allocation->data;
                        if (destination != NULL) {
                            while (byteCount--) {
                                *destination++ = *source++;
                            }
                        } else {
                            copyFailed = 1;
                        }
                        index++;
                        allocation++;
                        segment++;
                    } while (index < D_800792E8->segmentCount);
                }
            }
        }
    }
    if (copyFailed != 0) {
        func_8000D570();
    }
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` and built
 * `trackLightFreeMem` establish this function's role and control-flow
 * skeleton. Mickey's own globals, types, and bytes determine this body.
 */
void func_8000D570(void) {
    s32 lightIndex;

    if (D_80079308 != NULL) {
        lightIndex = D_800792E8->segmentCount;
        while (lightIndex--) {
            if (D_80079308[lightIndex].data != NULL) {
                mmFree(D_80079308[lightIndex].data);
            }
        }
        mmFree(D_80079308);
        D_80079308 = NULL;
    }
    if (D_80079300 != 0) {
        mmFree(D_80079300);
        D_80079300 = NULL;
    }
    D_800792FC = 0;
    D_800792F8 = 0;
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c`, assembly-only
 * `trackLightAdd`, supplies the role and 0x80-byte pool stride. Mickey's own
 * stores establish the record fields and body; the public name is not adopted.
 */
TrackLight *func_8000D62C(f32 x, f32 y, f32 z, f32 radius,
                          f32 secondaryRadius, s32 red, s32 green, s32 blue) {
    s32 lightIndex;
    TrackLight *light;

    if (radius <= 0.0f) {
        return NULL;
    }
    light = D_80079300;
    lightIndex = D_800792F8;
    if (lightIndex--) {
        do {
            if (light->radius == 0.0f) {
                light->x = x;
                light->y = y;
                light->z = z;
                light->radius = radius;
                light->secondaryRadius = secondaryRadius;
                light->radiusSquared = radius * radius;
                light->secondaryRadiusSquared =
                    secondaryRadius * secondaryRadius;
                light->falloff =
                    D_80081690 / (radius - secondaryRadius);
                func_8000D768(light, red, green, blue, 0xFF);
                D_800792FC++;
                return light;
            }
            light++;
        } while (lightIndex--);
    }
    return NULL;
}
void func_8000D728(TrackFloatRecord *arg0) {
    if ((arg0 != NULL) && (arg0->unkC != 0.0f)) {
        arg0->unkC = 0.0f;
        D_800792FC--;
    }
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` supplies the
 * `trackLightColour` role at this established TU position. Its body remains
 * assembly-only; this reconstruction comes from Mickey's own accesses.
 */
void func_8000D768(TrackLight *light, s32 red, s32 green, s32 blue,
                   s32 intensity) {
    TrackLightColourEntry *colour;
    s32 redStep;
    s32 greenStep;
    s32 blueStep;
    s32 colourIndex;

    if (light != NULL) {
        if (intensity < 255) {
            red = (red * intensity) >> 8;
            green = (green * intensity) >> 8;
            blue = (blue * intensity) >> 8;
        }
        colour = light->colours;
        redStep = red;
        greenStep = green;
        blueStep = blue;
        colourIndex = 31;
        do {
            colour->red = red >> 5;
            colour->green = green >> 5;
            colour->blue = blue >> 5;
            red += redStep;
            green += greenStep;
            blue += blueStep;
            colour++;
        } while (colourIndex--);
    }
}
void func_8000D7F8(TrackFloatRecord *arg0, f32 arg1, f32 arg2, f32 arg3) {
    if (arg0 != NULL) {
        arg0->x = arg1;
        arg0->y = arg2;
        arg0->z = arg3;
    }
}
/*
 * PROVENANCE: Jet Force Gemini's public `src/track.c` lighting-update
 * skeleton supplies the alternating packed-colour copy. Mickey's separate
 * lighting count at `TrackSegment +0x20`, source record, and dirty-mask
 * fields are established by the resident accesses above.
 *
 * Matched by reusing lightCount as the dirty-mask group counter (one web, so
 * the reassignment keeps its copy), reading the dirty flag and the middle
 * colour byte without declared carriers, and defining the mask after the two
 * cursors so the leaf's web order follows the target's.
 */
void func_8000D820(void) {
    s32 segmentCount;
    s32 lightCount;
    s32 *segmentFlags;
    s32 copyMode;
    u8 *dirtyMasks;
    u32 mask;
    TrackLightSource *sourceRecord;
    u8 *sourceBase;
    u8 *lightData;
    u8 *source;
    u8 *destination;
    TrackSegment *segment;

    segmentFlags = D_800C95B4;
    segment = D_800792E8->segments;
    segmentCount = D_800792E8->segmentCount;
    copyMode = (D_80079308 != NULL) ? 1 : -1;
    while (segmentCount--) {
        segmentFlags++;
        if ((segmentFlags[-1] != 0) && ((segment->lightingMode & copyMode) != 0)) {
            sourceRecord = (TrackLightSource *) segment->unk30;
            if (sourceRecord != NULL) {
                sourceBase = sourceRecord->source;
                lightData = (u8 *) segment->lightData;
                lightCount = segment->lightBatchCount;
                if (copyMode > 0) {
                    while (lightCount--) {
                        lightData += 10;
                        lightData[-4] = sourceBase[0];
                        lightData[-3] = sourceBase[1];
                        sourceBase += 3;
                        lightData[-2] = sourceBase[-1];
                    }
                    segment->lightingMode ^= 1;
                } else {
                    lightCount = (lightCount + 15) >> 4;
                    dirtyMasks = (u8 *) sourceRecord->dirtyMasks;
                    while (lightCount--) {
                        destination = lightData;
                        source = sourceBase;
                        mask = *(u16 *) dirtyMasks;
                        *(u16 *) dirtyMasks = 0;
                        if (mask != 0) {
                            do {
                                if ((mask & 1) != 0) {
                                    destination[6] = source[0];
                                    destination[7] = source[1];
                                    destination[8] = source[2];
                                }
                                destination += 10;
                                source += 3;
                                mask >>= 1;
                            } while (mask != 0);
                        }
                        lightData += 0xA0;
                        sourceBase += 0x30;
                        dirtyMasks += 2;
                    }
                    segment->lightingMode = 0;
                }
            }
        }
        segment++;
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c` and
 * assembly-only `trackUpdateLighting`. Mickey's module path, segment layout,
 * globals, and bytes are authoritative; the public name is not adopted.
 */
void func_8000D978(s32 copySegmentData, s32 updateRate) {
    s32 segmentCount;
    s8 mode;
    TrackSegment *segment;
    TrackLightAllocation *allocation;
    TrackLight *light;

    if ((D_800792E8 != NULL) && (mainGetNumberOfCameras() < 2) &&
        ((copySegmentData == 0) || (D_80079308 == NULL)) &&
        ((copySegmentData != 0) || (D_80079308 != NULL))) {
        allocation = D_80079308;
        if (allocation != NULL) {
            D_80079304 ^= 1;
            segmentCount = D_800792E8->segmentCount;
            segment = D_800792E8->segments;
            while (segmentCount--) {
                mode = segment->lightingMode;
                segment->lightData =
                    ((void **) allocation)[D_80079304];
                segment->lightingMode =
                    ((mode << 1) & 2) | ((mode >> 1) & 1);
                segment++;
                allocation++;
            }
        }
        if (runlinkIsModuleLoaded(16) != 0) {
            TrapDanglingJump(&D_800C95B4, D_800792E8, updateRate);
        } else if ((D_800D6C54 != 0xFF) || (D_800D6C4C != 0)) {
            /* Runtime relocation: overlay 40 +0x690 (overlay40FadeRecords). */
            TrapDanglingJump(&D_800C95B4, D_800792E8, updateRate);
        } else {
            func_8000D820();
        }
        segmentCount = D_800792F8;
        light = D_80079300;
        while (segmentCount--) {
            if (light->radius != 0.0f) {
                trackLightAsm(D_800792E8, light, &D_800C95B4);
            }
            light++;
        }
    }
}
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident object and
 * bounding-box offsets supply this route-list reconstruction. No external
 * function body is copied here; the public JFG routine is context only.
 */
typedef struct TrackRouteObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 segmentIndex;
    u8 pad30[4];
    f32 radius;
} TrackRouteObject;

typedef struct TrackRouteResult {
    s16 segmentIndex;
    s16 flags;
    TrackRouteObject *object;
} TrackRouteResult;

extern s32 func_8000A244(s32 *resultCount);
extern void func_8000A39C(s32 first, s32 last);
extern TrackRouteObject *func_800056F0(s32 index);

/* Mickey m2c restores the inclusive reverse object range and signed count ABI.
 * Matched (Track B) by subscripting indices[] in both loops so strength
 * reduction builds the cursors, scanning with while (mapIndex--), doubling
 * objectRadius in place, pre-decrementing lastIndex in the lookup call, and
 * three unreferenced locals that give the 0x190 frame its home layout. */
s32 func_8000DB34(s32 count, u8 *indices, TrackRouteResult *results) {
    s32 heapCount;
    s32 mapIndex;
    s32 unusedA;
    s32 unusedB;
    s32 lastIndex;
    s32 resultCount;
    s32 objectRadius;
    s32 minX;
    s32 minY;
    s32 minZ;
    s32 candidateSegment;
    s32 unusedC;
    u8 inputIndex;
    u8 map[256];
    TrackRouteObject *object;
    TrackBoundingBox *bounds;

    mapIndex = 0;
    do {
        map[mapIndex++] = 0xFF;
    } while (mapIndex < 256);
    mapIndex = 0;
    if (count > 0) {
        do {
            map[indices[mapIndex]] = mapIndex;
            mapIndex++;
        } while (mapIndex != count);
    }

    heapCount = func_8000A244(&lastIndex);
    func_8000A39C(heapCount, lastIndex - 1);
    resultCount = 0;
    if (heapCount < lastIndex) {
        do {
            object = func_800056F0(--lastIndex);
            if ((object->segmentIndex != -1) &&
                (map[object->segmentIndex] != 0xFF) &&
                (func_800103D4(object) != 0)) {
                objectRadius = (s32) object->radius;
                candidateSegment = object->segmentIndex;
                minX = (s32) object->x - objectRadius;
                minY = (s32) object->y - objectRadius;
                minZ = (s32) object->z - objectRadius;
                objectRadius *= 2;
                mapIndex = map[object->segmentIndex];
                while (mapIndex--) {
                    inputIndex = indices[mapIndex];
                    bounds = &D_800792E8->segmentBounds[inputIndex];
                    if ((minX < bounds->x2) &&
                        (minY < bounds->y2) &&
                        (minZ < bounds->z2) &&
                        (bounds->x1 < (minX + objectRadius)) &&
                        (bounds->y1 < (minY + objectRadius)) &&
                        (bounds->z1 < (minZ + objectRadius))) {
                        candidateSegment = inputIndex;
                    }
                }
                results->segmentIndex = candidateSegment;
                results->flags = 0xFF;
                results->object = object;
                results++;
                resultCount++;
            }
        } while (heapCount < lastIndex);
    }
    return resultCount;
}
/* Exact 118-word C: reusing the dead recordIndex carrier for the later sort
 * passes closes the 24-site allocator bijection while preserving the 0x28
 * frame and both R_MIPS_26 call identities. The complete flag lattice was
 * nonexact and one codegen-faithful allocator trace selected this lifetime
 * merge; the reference skeleton scan found no credible donor. */
s32 func_8000DDE4(s32 key, s32 recordCount, TrackKeyRecord *records,
                  TrackKeyRecord **matches) {
    s32 recordIndex;
    s32 matchCount;
    s32 compareCount;
    s32 sorted;
    TrackKeyRecord **match;
    TrackKeyRecord *current;
    TrackKeyRecord *next;
    s32 currentValue;
    s32 nextValue;

    matchCount = 0;
    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        if (records[recordIndex].key == key) {
            matches[matchCount++] = &records[recordIndex];
        }
    }
    if ((matchCount > 0) && (runlinkIsModuleLoaded(21) != 0)) {
        TrapDanglingJump(key, matchCount, matches);
    }
    if (matchCount >= 2) {
        recordIndex = matchCount - 1;
        if (matchCount != 0) {
            do {
                current = matches[0];
                match = matches;
                sorted = TRUE;
                compareCount = recordIndex - 1;
                currentValue = current->sortValue;
                if (recordIndex != 0) {
                    do {
                        next = match[1];
                        nextValue = next->sortValue;
                        if (nextValue < currentValue) {
                            match[0] = next;
                            match++;
                            sorted = FALSE;
                        } else {
                            match[0] = current;
                            match++;
                            current = next;
                            currentValue = nextValue;
                        }
                    } while (compareCount--);
                }
                match[0] = current;
                if (sorted) {
                    recordIndex = 0;
                }
            } while (recordIndex--);
        }
    }
    return matchCount;
}
#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c supplies the resident track draw-loop
 * organization; Mickey's segment and display-list accesses are authoritative. */
/* Candidate: 398/396 words, 303 differing, first mismatch +0x48, frame 0x70 exact. */
/* Declaring batchIndex and groupIndex ahead of the pointers saves one word. */
/* The +8 tail spills and the display-list schedule are unchanged. */
struct TrackShadowObject;
struct TrackShadowInstance;
extern void func_800140CC(struct TrackShadowObject *,
                          struct TrackShadowInstance *);
extern void overlay69DrawSortedGeometry(Gfx **, Mtx **, TrackVertex **, void *);
extern void overlay88DrawSortedGeometry(Gfx **, Mtx **, TrackVertex **, void *);
extern void overlay68DrawSortedEntries(Gfx **, Mtx **, TrackVertex **, void *);
extern void overlay29DrawGroups(Gfx **, Mtx **, void *);

void func_8000DFBC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 batchIndex;
    s32 groupIndex;
    TrackSegment *segment;
    TrackBatch *batch;
    Gfx *gfx;
    TrackTextureHeader *texture;
    u8 *object;
    u8 *objectChild;
    u8 *vertex;
    u8 *triangle;
    s16 batchCount;
    s32 itemIndex;
    s32 alpha;
    u32 mode;
    s32 vertexCount;
    s32 textureS;
    u32 vertexAddress;
    s32 objectMode;
    u32 value;
    s16 objectType;

    batchIndex = 0;
    segment = &D_800792E8->segments[arg0];
    batch = segment->batches;
    groupIndex = 0;
    itemIndex = 0;
    batchCount = segment->batchCount;
    if (batchCount <= 0 && arg2 <= 0) {
        return;
    }
    do {
        if ((batchIndex < batchCount) &&
            ((itemIndex >= arg2) ||
             (batchIndex <
              *(s16 *) (((u8 **) (u32) arg3)[itemIndex] + 2)))) {
            if ((arg1 & (1U << groupIndex)) &&
                (groupIndex == batch->unk1)) {
                if (D_8007C854 != 0) {
                    gfx = D_800C9520;
                    gfx->words.w0 = 0xFA000000;
                    value = D_8007C858 & 0xFF;
                    gfx->words.w1 = (value << 24) | (value << 16) |
                                     (value << 8) | 0xFF;
                    gfx++;
                    D_800C9520 = gfx;
                    batchCount = segment->batchCount;
                }
                if ((batchIndex < batchCount) &&
                    (groupIndex == batch->unk1)) {
                    do {
                        mode = batch->flags;
                        if (!(mode & 0x800)) {
                            alpha = 0;
                            texture = NULL;
                            if (batch->textureIndex != 0xFF) {
                                alpha = 1;
                                texture =
                                    D_800792E8->textures[batch->textureIndex]
                                        .texture;
                            }
                            textureS = batch->frame << 8;
                            vertex = (u8 *) segment->lightData +
                                     (batch->u0 * 0xA);
                            triangle = (u8 *) segment->vertexData +
                                       (batch->v0 * 0x10);
                            if ((texture != NULL) &&
                                ((s16) texture->flags & 0x40) &&
                                ((mode & 0x30) != 0x20)) {
                                gfx = D_800C9520;
                                gfx->words.w0 = 0xFB000000;
                                value = (textureS >> 8) & 0xFF;
                                gfx->words.w1 = (value << 24) |
                                                 (value << 16) |
                                                 (value << 8) | value;
                                gfx++;
                                D_800C9520 = gfx;
                            } else {
                                gfx = D_800C9520;
                                gfx->words.w0 = 0xFB000000;
                                gfx->words.w1 = -0x100;
                                gfx++;
                                D_800C9520 = gfx;
                            }
                            if (!(mode & 0x180)) {
                                mode |= D_800C9544;
                            }
                            objectMode = mode & 0x4000;
                            if (objectMode != 0) {
                                func_800343F0(2);
                            }
                            func_800349A4(&D_800C9520, texture,
                                          mode | 2, textureS);
                            if (objectMode != 0) {
                                texEnableModes(2);
                            }
                            vertexAddress = (u32) vertex + 0x80000000U;
                            vertexCount = batch[1].u0 - batch->u0;
                            gfx = D_800C9520;
                            gfx->words.w1 = vertexAddress;
                            gfx->words.w0 = (((vertexCount * 0xA) + 8) & 0xFFFF) |
                                             0x04000000 |
                                             ((((vertexCount * 8) |
                                                (vertexAddress & 6)) & 0xFF) << 16);
                            gfx++;
                            D_800C9520 = gfx;
                            gfx->words.w1 = (u32) triangle + 0x80000000U;
                            vertexCount = batch[1].v0 - batch->v0;
                            gfx->words.w0 = ((vertexCount * 0x10) & 0xFFFF) |
                                             0x05000000 |
                                             (((((vertexCount - 1) * 0x10) |
                                                alpha) & 0xFF) << 16);
                            gfx++;
                            D_800C9520 = gfx;
                            batch = (TrackBatch *) ((u8 *) batch + 0x10);
                            batchIndex++;
                            batchCount = segment->batchCount;
                        } else {
                            batch = (TrackBatch *) ((u8 *) batch + 0x10);
                            batchIndex++;
                        }
                    } while ((batchIndex < batchCount) &&
                             (batch->unk1 == groupIndex));
                    batchCount = segment->batchCount;
                }
                if (D_8007C854 != 0) {
                    gfx = D_800C9520;
                    gfx->words.w1 = 0;
                    gfx->words.w0 = 0xE7000000;
                    gfx++;
                    gfx->words.w1 = -1;
                    gfx->words.w0 = 0xFA000000;
                    gfx++;
                    D_800C9520 = gfx;
                }
            } else if ((batchIndex < batchCount) &&
                       (batch->unk1 == groupIndex)) {
                do {
                    batch = (TrackBatch *) ((u8 *) batch + 0x10);
                    batchIndex++;
                } while ((batchIndex < batchCount) &&
                         (batch->unk1 == groupIndex));
            }
            groupIndex++;
        } else {
            object = *(u8 **)
                (((u8 **) (u32) arg3)[itemIndex++] + 4);
            objectChild = *(u8 **) (object + 0x4C);
            if ((objectChild != NULL) && (*(u8 *) (object + 0x8E) == 0)) {
                if (*(u8 *) (objectChild + 0x10) & 8) {
                    u8 *child = *(u8 **) (objectChild + 0x1C);
                    if (child != NULL) {
                        func_800140CC((struct TrackShadowObject *) object,
                                      (struct TrackShadowInstance *) child);
                    }
                }
                func_800140CC((struct TrackShadowObject *) object,
                              *(struct TrackShadowInstance **)
                                  (object + 0x4C));
            }
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
            value = *(s32 *) (object + 0x54);
            if (value != 0) {
                func_80049518(value, &D_800C9520);
            }
            if (*(s16 *) (object + 6) & 0x200) {
                objectType = *(s16 *) (object + 0x44);
                switch (objectType) {
                case 1:
                    func_80009414(&D_800C9520, &D_800C9524,
                                  &D_800C9528, object);
                    break;
                case 0x1D:
                    overlay69DrawSortedGeometry(&D_800C9520, &D_800C9524,
                                                &D_800C9528, object);
                    break;
                case 0x49:
                    overlay88DrawSortedGeometry(&D_800C9520, &D_800C9524,
                                                &D_800C9528, object);
                    break;
                case 0x3F:
                    overlay68DrawSortedEntries(&D_800C9520, &D_800C9524,
                                               &D_800C9528, object);
                    break;
                case 0x39:
                    TrapDanglingJump(&D_800C9520, &D_800C9524, object);
                    break;
                case 0x3A:
                    overlay29DrawGroups(&D_800C9520, &D_800C9524, object);
                    break;
                }
            }
        }
    } while ((batchIndex < (batchCount = segment->batchCount)) ||
             (itemIndex < arg2));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000DFBC.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c draft and resident track/particle call surfaces
 * reconstruct this draw/update coordinator; no external function body is adapted.
 */
/* Workbench verdict: structure-mismatch, 185 differing words, first mismatch +0x0. */
/* Candidate: 209/205 instructions with a -0xE8 frame versus target -0xD8; all 56 relocations are present, with 14 offset/type and 9 identity sites aligned. */
/* Shape status: the 128-byte segment list and nested dispatch are reconstructed; temporary-local stack layout and early loop scheduling remain unresolved. */
void func_8000E5EC(s32 updateRate, s32 arg1) {
    u8 *segment;
    s32 resultCount;
    s32 visibleCount;
    s32 segmentIndex;
    s32 lastIndex;
    s32 displayOffset;
    s32 *visibility;
    s16 cameraSegment;
    s16 segmentCount;
    u8 mode;
    s32 *segmentFlags;
    s32 displayList;
    u8 segmentList[128];

    visibleCount = 1;
    if (D_800792E8->segmentCount >= 2) {
        if (levelGetLevel()[0x106] == 0) {
            func_8000FA2C(&visibleCount, segmentList);
        } else {
            func_8000F57C(&visibleCount, segmentList);
        }
    } else {
        segmentList[0] = 0;
    }
    if (D_80079260 == 0) {
        visibleCount = 0;
    }
    D_800C95B0[0] = -1;
    segmentIndex = 1;
    if (D_800792E8->segmentCount > 0) {
        segmentFlags = D_800C95B4;
        do {
            *segmentFlags++ = 0;
            segmentIndex++;
        } while (D_800792E8->segmentCount >= segmentIndex);
    }
    if ((D_80079260 != 0) || (D_80079264 != 0)) {
        cameraSegment = camGetPtr()->segmentIndex;
        segmentCount = D_800792E8->segmentCount;
        if ((cameraSegment >= 0) && (cameraSegment < segmentCount) &&
            (D_8007926C == 0)) {
            lastIndex = visibleCount - 1;
            segment = segmentList + lastIndex;
            if (visibleCount != 0) {
                do {
                    mode = *segment--;
                    visibility = D_800792E8->visibility;
                    D_800C95B0[mode + 1] =
                        visibility[(cameraSegment * segmentCount) + mode];
                } while (lastIndex-- != 0);
            }
        } else {
            lastIndex = visibleCount - 1;
            if (visibleCount != 0) {
                segment = segmentList + lastIndex;
                do {
                    mode = *segment--;
                    D_800C95B0[mode + 1] = -1;
                } while (lastIndex-- != 0);
            }
        }
        if (D_800792E8->segmentCount < 2) {
            D_800C95B0[1] = -1;
        }
    }
    resultCount = 0;
    displayList = (s32) D_800C9548;
    if (D_80079268 != 0) {
        resultCount = func_8000DB34(visibleCount, segmentList,
                                    (TrackRouteResult *) displayList);
    }
    func_8000D978(0, arg1);
    func_80034920(&D_800C9520);
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 1);
    }
    func_80034920(&D_800C9520);
    lastIndex = visibleCount - 1;
    if (visibleCount != 0) {
        segment = segmentList + lastIndex;
        displayOffset = (resultCount * 8) + displayList;
        do {
            func_8000DFBC(*segment, D_800C95B0[*segment + 1],
                          func_8000DDE4(*segment, resultCount,
                                        (TrackKeyRecord *) displayList,
                                        (TrackKeyRecord **) displayOffset),
                          displayOffset);
            segment--;
        } while (lastIndex-- != 0);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump(&D_800C9520, &D_800C9528);
    }
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 0);
    }
    D_800C9544 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E5EC.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident track
 * declarations reconstruct this display-list pipeline; no external function
 * body is adapted. The raw offsets retain fields absent from the local types.
 */
/* Workbench verdict: structure-mismatch, 459 differing words, first mismatch +0x0. */
/* Candidate is 547/542 instructions with a -0x100 frame versus -0xF8 and 112/114 relocations. */
/* Remaining gap: reverse-pass lifetime scoping and display-list scheduling. */
extern u8 D_80081560[];
extern u8 D_80081570[];
extern u8 D_80081580[];
extern u8 D_80081590[];
extern u8 D_800815A0[];
extern u8 D_800815B0[];
extern u8 D_800815C0[];
extern u8 D_800815D0[];
extern u8 D_800815E0[];
extern u8 D_800815F0[];
extern u8 D_80081600[];
extern u8 D_80081610[];
extern void func_8000F198(s32 segment, s32 record, s32 mode);

#define E920_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define E920_S8(base, offset) (*(s8 *) ((u8 *) (base) + (offset)))
#define E920_S16(base, offset) (*(s16 *) ((u8 *) (base) + (offset)))
#define E920_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define E920_PTR(base, offset) (*(void **) ((u8 *) (base) + (offset)))
#define E920_RECORD(segment) \
    (D_800C95B0[(segment) + 1])

void func_8000E920(s32 arg0, s32 arg1) {
    s32 segmentCount;
    s32 visibleCount;
    s32 segmentEnd;
    s32 selectedCount;
    s32 index;
    s32 reverseIndex;
    s32 lastSelected;
    s16 modeCount;
    u8 segment;
    u32 segmentCursor;
    u8 segmentIds[0x70];
    s32 *segmentFlags;
    void **selectedObjects;
    void **objectCursor;
    void *object;
    void *surface;
    void *childSurface;
    s32 record;

    segmentCount = func_8000A244(&segmentEnd);
    selectedObjects = (void **) D_800C9548;
    if (D_800792E8->segmentCount >= 2) {
        if (E920_U8(levelGetLevel(), 0x106) == 0) {
            func_8000FA2C(&visibleCount, (s32) &segmentIds[0]);
        } else {
            func_8000F57C(&visibleCount, &segmentIds[0]);
        }
    } else {
        visibleCount = 1;
        segmentIds[0] = 0;
    }
    func_8000A39C(segmentCount, segmentEnd - 1);
    func_80034920(&D_800C9520);
    func_80044BC8(D_800C9520, D_80081560, 0x58D);
    D_800C95B0[0] = -1;
    modeCount = D_800792E8->segmentCount;
    index = 1;
    if (modeCount > 0) {
        segmentFlags = D_800C95B4;
        do {
            *segmentFlags = 0;
            modeCount = D_800792E8->segmentCount;
            index++;
            segmentFlags++;
        } while (modeCount >= index);
    }
    if ((D_80079260 != 0) || (D_80079264 != 0)) {
        reverseIndex = visibleCount - 1;
        segmentCursor = (u32) segmentIds + reverseIndex;
        if (reverseIndex >= 0) {
            do {
                segment = *(u8 *) segmentCursor;
                E920_RECORD(segment) = -1;
                func_8000F198(segment, -1, 0x4000);
                segmentCursor--;
            } while (segmentCursor >= (u32) segmentIds);
            modeCount = D_800792E8->segmentCount;
        }
    }
    if (modeCount < 2) {
        E920_RECORD(0) = -1;
    }
    func_8000D978(0, arg1);
    func_80044BC8(D_800C9520, D_80081570, 0x5A1);
    if (D_80079260 != 0) {
        for (index = 0; index < visibleCount; index++) {
            segment = segmentIds[index];
            func_8000F198(segment, E920_RECORD(segment), 0);
        }
    }
    index = segmentCount;
    if (D_80079268 == 0) {
        index = segmentEnd;
    }
    selectedCount = 0;
    for (; index < segmentEnd; index++) {
        object = func_800056F0(index);
        if ((object != NULL) &&
            (E920_RECORD(((TrackRouteObject *) object)->segmentIndex) != 0) &&
            (func_800103D4(object) != 0)) {
            selectedObjects[selectedCount++] = object;
        }
    }
    if (E920_S8(D_800792EC, 0xF6) != 0) {
        TrapDanglingJump(selectedCount, selectedObjects);
    }
    func_80044BC8(D_800C9520, D_80081580, 0x5D7);
    index = 0;
    if (selectedCount > 0) {
        objectCursor = selectedObjects;
        do {
            object = *objectCursor++;
            index++;
            if ((E920_S32(object, 0x58) != 0) &&
                ((E920_S16(object, 6) & 0xC) == 0) &&
                (E920_U8(object, 0x39) == 0xFF)) {
                func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                              (TrackSkyObject *) object);
            }
        } while (index != selectedCount);
    }
    func_80044BC8(D_800C9520, D_80081590, 0x5E3);
    lastSelected = selectedCount - 1;
    for (reverseIndex = lastSelected * 4; reverseIndex >= 0;) {
        object = *(void **) ((u8 *) selectedObjects + reverseIndex);
        reverseIndex -= 4;
        surface = E920_PTR(object, 0x4C);
        if ((surface != NULL) && (E920_U8(object, 0x8E) == 0)) {
            if ((E920_U8(surface, 0x10) & 8) != 0) {
                childSurface = E920_PTR(surface, 0x1C);
                if (childSurface != NULL) {
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *) childSurface);
                }
            }
            func_800140CC((struct TrackShadowObject *) object,
                          (struct TrackShadowInstance *)
                              E920_PTR(object, 0x4C));
        }
    }
    func_80044BC8(D_800C9520, D_800815A0, 0x5F7);
    index = 0;
    if (selectedCount > 0) {
        objectCursor = selectedObjects;
        do {
            object = *objectCursor++;
            index++;
            if (((E920_S16(object, 6) & 0xC) == 0) &&
                (E920_U8(object, 0x39) == 0xFF) &&
                (E920_S32(object, 0x58) == 0)) {
                func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                              (TrackSkyObject *) object);
            }
        } while (index != selectedCount);
    }
    func_80044BC8(D_800C9520, D_800815B0, 0x603);
    for (reverseIndex = lastSelected * 4; reverseIndex >= 0;) {
        object = *(void **) ((u8 *) selectedObjects + reverseIndex);
        reverseIndex -= 4;
        if ((E920_S16(object, 6) & 8) != 0) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    if (runlinkIsModuleLoaded(0xC) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (E920_S8(D_800792EC, 0xF6) != 0) {
        func_80044BC8(D_800C9520, D_800815C0, 0x61A);
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
        if (D_80079260 != 0) {
            for (reverseIndex = visibleCount - 1; reverseIndex >= 0;
                 reverseIndex--) {
                segment = segmentIds[reverseIndex];
                func_8000F198(segment, E920_RECORD(segment), 0x8000);
            }
            for (reverseIndex = lastSelected * 4; reverseIndex >= 0;) {
                object = *(void **) ((u8 *) selectedObjects + reverseIndex);
                reverseIndex -= 4;
                surface = E920_PTR(object, 0x4C);
                if ((surface != NULL) && (E920_U8(object, 0x8E) != 0)) {
                    if ((E920_U8(surface, 0x10) & 8) != 0) {
                        childSurface = E920_PTR(surface, 0x1C);
                        if (childSurface != NULL) {
                            func_800140CC(
                                (struct TrackShadowObject *) object,
                                (struct TrackShadowInstance *) childSurface);
                        }
                    }
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *)
                                      E920_PTR(object, 0x4C));
                }
            }
        }
    }
    func_80044BC8(D_800C9520, D_800815D0, 0x634);
    if (D_80079260 != 0) {
        for (reverseIndex = visibleCount - 1; reverseIndex >= 0; reverseIndex--) {
            segment = segmentIds[reverseIndex];
            func_8000F198(segment, E920_RECORD(segment), 4);
        }
    }
    func_80044BC8(D_800C9520, D_800815E0, 0x63B);
    for (reverseIndex = lastSelected * 4; reverseIndex >= 0;) {
        object = *(void **) ((u8 *) selectedObjects + reverseIndex);
        reverseIndex -= 4;
        record = E920_S32(object, 0x54);
        if (record != 0) {
            func_80049518(record, &D_800C9520);
        }
    }
    if (runlinkIsModuleLoaded(0xD) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9528);
    }
    func_80044BC8(D_800C9520, D_800815F0, 0x64E);
    for (reverseIndex = lastSelected * 4; reverseIndex >= 0;) {
        object = *(void **) ((u8 *) selectedObjects + reverseIndex);
        reverseIndex -= 4;
        if (((E920_S16(object, 6) & 4) != 0) ||
            ((s32) E920_U8(object, 0x39) < 0xFF)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
        if ((E920_S16(object, 6) & 0x200) != 0) {
            switch (E920_S16(object, 0x44)) {
            case 1:
                func_80009414(&D_800C9520, &D_800C9524, &D_800C9528,
                              (TrackSkyObject *) object);
                break;
            case 0x1D:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524,
                                 &D_800C9528, object);
                break;
            case 0x49:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524,
                                 &D_800C9528, object);
                break;
            case 0x3F:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524,
                                 &D_800C9528, object);
                break;
            case 0x39:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524, object);
                break;
            case 0x3A:
                TrapDanglingJump((s32) &D_800C9520, &D_800C9524, object);
                break;
            }
        }
    }
    func_80044BC8(D_800C9520, D_80081600, 0x678);
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, -1);
    }
    D_800C9544 = 0;
    func_80044BC8(D_800C9520, D_80081610, 0x680);
}
#undef E920_U8
#undef E920_S8
#undef E920_S16
#undef E920_S32
#undef E920_PTR
#undef E920_RECORD
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000E920.s")
#endif
/* PROVENANCE -- JFG's public track.c supplies the surrounding display-list
 * routine and texture vocabulary, while this Mickey body follows its own
 * fields, call sites, and assembly-only command schedule. */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch; 185 differing words, first mismatch +0x0. */
/* Exact 249-word size; candidate frame 0x58 versus target 0x70, 18/21 relocation sites exact. */
/* Unsigned batch flags preserve the recovered masks; the 24-byte frame deficit remains. */
void func_8000F198(s32 arg0, s32 arg1, s32 arg2) {
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTextureHeader *texture;
    s32 specialFlags;
    s32 sp5C;
    s32 sp58;
    u32 vertexAddress;
    u32 positionAddress;
    s32 textureFrame;
    u32 textureFlags;
    s32 hasTexture;
    s32 vertexCount;
    s32 positionCount;
    s32 color;
    s32 batchCount;
    s32 index;

    if (D_8007C854 != 0) {
        gDPSetPrimColor(D_800C9520++, 0, 0, D_8007C858 & 0xFF,
                        D_8007C858 & 0xFF, D_8007C858 & 0xFF, 0xFF);
    }

    segment = &D_800792E8->segments[arg0];
    switch (arg2) {
    case 4:
        sp5C = 0xC904;
        sp58 = 0xC800;
        break;
    case 0x4000:
        func_800343F0(2, arg0);
        sp5C = 0x4800;
        sp58 = 0x800;
        break;
    case 0x8000:
        sp5C = 0xC800;
        sp58 = 0x4800;
        break;
    default:
        sp5C = -1;
        sp58 = 0xC904;
        break;
    }

    batchCount = segment->batchCount;
    batch = segment->batches;
    if (batchCount != 0) {
        index = batchCount - 1;
        do {
            if ((1U << batch->unk1) & arg1) {
                textureFlags = batch->flags;
                if ((textureFlags & sp5C) && !(textureFlags & sp58)) {
                    texture = NULL;
                    hasTexture = 0;
                    if (batch->textureIndex != 0xFF) {
                        hasTexture = 1;
                        texture = D_800792E8->textures[batch->textureIndex].texture;
                    }
                    vertexAddress = (u32) segment->lightData +
                                    (batch->u0 * 0xA);
                    positionAddress = (u32) segment->vertexData +
                                      (batch->v0 * 0x10);
                    textureFrame = batch->frame << 8;
                    if ((texture != NULL) && ((s16) texture->flags & 0x40) &&
                        ((textureFlags & 0x30) != 0x20)) {
                        color = (textureFrame >> 8) & 0xFF;
                        gDPSetEnvColor(D_800C9520++, color, color, color,
                                       color);
                    } else {
                        gDPSetEnvColor(D_800C9520++, 0xFF, 0xFF, 0xFF, 0);
                    }
                    if (!(textureFlags & 0x180)) {
                        textureFlags |= D_800C9544;
                    }
                    specialFlags = textureFlags & 0x20000;
                    if ((specialFlags != 0) && (texture != NULL)) {
                        func_80014ECC(texture, textureFrame, textureFlags);
                    } else {
                        func_800349A4(&D_800C9520, texture,
                                      textureFlags | 2, textureFrame);
                    }
                    vertexAddress += 0x80000000;
                    vertexCount = batch[1].u0 - batch->u0;
                    TRACK_SP_VERTEX(D_800C9520++, vertexAddress,
                                    vertexCount, 0);
                    positionCount = batch[1].v0 - batch->v0;
                    positionAddress += 0x80000000;
                    TRACK_SP_POLYGON(D_800C9520++, positionAddress,
                                     positionCount, hasTexture);
                    if (specialFlags != 0) {
                        func_80034920(&D_800C9520);
                    }
                }
            }
            batch++;
        } while (index--);
    }
    if (arg2 == 0x4000) {
        texEnableModes(2);
    }
    if (D_8007C854 != 0) {
        gDPPipeSync(D_800C9520++);
        gDPSetPrimColor(D_800C9520++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000F198.s")
#endif
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackGetBlockList` in
 * `src/track.c` supplies tier-D TU-position and role context. The body and
 * resident layouts below are reconstructed from Mickey-only evidence; the
 * public name is not adopted.
 */
void func_8000F57C(s32 *resultCount, u8 *resultSegments) {
    s32 distanceX;
    s32 distanceY;
    s32 distanceZ;
    s32 resultIndex;
    s32 lastIndex;
    s32 cameraX;
    s32 cameraY;
    s32 cameraZ;
    s32 index;
    s32 tempDistance;
    s32 segmentIndex;
    s32 distances[100];
    TrackBoundingBox *bounds;

    cameraX = D_800C9530->x;
    resultIndex = 0;
    cameraY = D_800C9530->y;
    segmentIndex = 0;
    bounds = D_800792E8->segmentBounds;
    cameraZ = D_800C9530->z;

    if (D_800792E8->segmentCount > 0) {
        do {
            if (func_80010178(segmentIndex) != 0) {
                if (cameraX < bounds->x1) {
                    distanceX = bounds->x1 - cameraX;
                } else if (bounds->x2 < cameraX) {
                    distanceX = cameraX - bounds->x2;
                } else {
                    distanceX = 0;
                }

                if (cameraY < bounds->y1) {
                    distanceY = bounds->y1 - cameraY;
                } else if (bounds->y2 < cameraY) {
                    distanceY = cameraY - bounds->y2;
                } else {
                    distanceY = 0;
                }

                if (cameraZ < bounds->z1) {
                    distanceZ = bounds->z1 - cameraZ;
                } else if (bounds->z2 < cameraZ) {
                    distanceZ = cameraZ - bounds->z2;
                } else {
                    distanceZ = 0;
                }

                distances[resultIndex] =
                    (distanceX * distanceX) + (distanceY * distanceY) +
                    (distanceZ * distanceZ);
                resultSegments[resultIndex] = segmentIndex;
                resultIndex++;
                if (resultIndex >= 100) {
                    segmentIndex = D_800792E8->segmentCount;
                }
            }
            segmentIndex++;
            bounds++;
        } while (segmentIndex < D_800792E8->segmentCount);
    }

    lastIndex = resultIndex - 1;
    while (lastIndex > 0) {
        index = 0;
        while (index < lastIndex) {
            if (distances[index + 1] < distances[index]) {
                tempDistance = *(resultSegments + index);
                *(resultSegments + index) = *(resultSegments + index + 1);
                *(resultSegments + index + 1) = tempDistance;
                tempDistance = distances[index];
                distances[index] = distances[index + 1];
                distances[index + 1] = tempDistance;
            }
            index++;
        }
        lastIndex--;
    }
    *resultCount = resultIndex;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `traverse_segments_bsp_tree`; JFG's assembly-only `func_800150A4` confirms
 * the same TU role. Mickey's global result state and integer camera values are
 * authoritative, and the donor names are not imported.
 */
void func_8000F82C(s32 nodeIndex, s32 segmentIndex, s32 segmentIndex2) {
    TrackBspNode *node;
    s32 cameraValue;

    node = (TrackBspNode *)
        ((nodeIndex * sizeof(TrackBspNode)) + (u8 *) D_800C9574);
    if (node->axis == 0) {
        cameraValue = D_800C954C;
    } else if (node->axis == 1) {
        cameraValue = D_800C9554;
    } else {
        cameraValue = D_800C955C;
    }

    if (cameraValue < node->splitValue) {
        if (node->left != -1) {
            func_8000F82C(node->left, segmentIndex,
                          node->segmentIndex - 1);
        } else if (func_80010178(segmentIndex) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex;
        }

        if (node->right != -1) {
            func_8000F82C(node->right, node->segmentIndex,
                          segmentIndex2);
        } else if (func_80010178(segmentIndex2) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex2;
        }
    } else {
        if (node->right != -1) {
            func_8000F82C(node->right, node->segmentIndex,
                          segmentIndex2);
        } else if (func_80010178(segmentIndex2) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex2;
        }

        if (node->left != -1) {
            func_8000F82C(node->left, segmentIndex,
                          node->segmentIndex - 1);
        } else if (func_80010178(segmentIndex) != 0) {
            ((u8 *) D_800C956C)[D_800C9564++] = segmentIndex;
        }
    }
}
void func_8000FA2C(s32 *result, s32 arg1) {
    D_800C954C = D_800C9530->x;
    D_800C9554 = D_800C9530->y;
    D_800C955C = D_800C9530->z;
    D_800C9574 = D_800792E8->bspTree;
    D_800C9564 = 0;
    D_800C956C = arg1;
    func_8000F82C(0, 0, D_800792E8->segmentCount - 1);
    *result = D_800C9564;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Diddy Kong Racing's public `src/tracks.c`,
 * `get_level_segment_index_from_position`, supplies the segment scan and
 * nearest-height selection structure. Mickey's bounds are inclusive and its
 * TrackData layout, function boundary, and bytes remain authoritative.
 */
/* Configured C: 16 naming differences, first +0x1C. Defining x/z lower
 * bounds first removes four differences without changing the instruction shape. */
/* Candidate: target/candidate 62/62 instructions with matching -0x10 frames and exact opcode schedule. */
/* Shape status: the remaining pool-position/temp-FIFO residual is register-only. */
s32 func_8000FAE0(f32 x, f32 y, f32 z) {
    s16 segmentCount;
    s16 xLower;
    s16 xUpper;
    s16 zLower;
    s16 zUpper;
    s16 yLower;
    s16 yUpper;
    s32 xInt;
    s32 zInt;
    s32 yInt;
    s32 minVal;
    s32 i;
    s32 heightDiff;
    s32 result;
    s32 keepGoing;
    TrackBoundingBox *bounds;

    result = -1;
    if (D_800792E8 != NULL) {
        segmentCount = D_800792E8->segmentCount;
        minVal = 0x7FFF;
        bounds = D_800792E8->segmentBounds;
        i = 0;
        if (segmentCount > 0) {
            xInt = x;
            do {
                xLower = bounds->x1;
                xUpper = bounds->x2;
                if (xUpper < xInt) {
                    goto block_14;
                }
                if (xInt < xLower) {
                    goto block_14;
                }
                zInt = z;
                zLower = bounds->z1;
                zUpper = bounds->z2;
                if (zUpper < zInt) {
                    goto block_14;
                }
                if (zInt < zLower) {
                    goto block_14;
                }
                yInt = y;
                yLower = bounds->y1;
                yUpper = bounds->y2;
                if ((yInt >= yLower) && (yUpper >= yInt)) {
                    result = i;
                    goto done;
                }
                if (yInt < yLower) {
                    heightDiff = yLower - yInt;
                } else {
                    heightDiff = yInt - yUpper;
                }
                if (heightDiff < minVal) {
                    minVal = heightDiff;
                    result = i;
                }
block_14:
                i++;
                bounds++;
                keepGoing = i < segmentCount;
            } while (keepGoing != 0);
        }
    }
done:
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8000FAE0.s")
#endif
/*
 * PROVENANCE: Diddy Kong Racing's public `src/tracks.c`,
 * `check_if_inside_segment`, supplies the bounding-box containment structure.
 * Mickey's function takes coordinates directly and uses inclusive bounds.
 */
s32 func_8000FBD8(TrackSegmentIndex segmentIndex, f32 x, f32 y, f32 z) {
    s32 xInt;
    s32 yInt;
    s32 zInt;

    if (D_800792E8 != NULL) {
        xInt = x;
        segmentIndex.bounds =
            &D_800792E8->segmentBounds[segmentIndex.value];
        if (xInt >= segmentIndex.bounds->x1 &&
            segmentIndex.bounds->x2 >= xInt) {
            yInt = y;
            if (yInt >= segmentIndex.bounds->y1 &&
                segmentIndex.bounds->y2 >= yInt) {
                zInt = z;
                if (zInt >= segmentIndex.bounds->z1 &&
                    segmentIndex.bounds->z2 >= zInt) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `get_inside_segment_count_xz`. Mickey uses 16-bit output indices and its
 * resident track/bounding-box types and bindings.
 */
s32 func_8000FCA4(s32 x, s32 z, s16 *segments) {
    s32 segmentIndex;
    s32 count = 0;
    TrackBoundingBox *bounds;

    for (segmentIndex = 0; segmentIndex < D_800792E8->segmentCount;
         segmentIndex++) {
        bounds = D_800792E8->segmentBounds + segmentIndex;
        if (x < bounds->x2 + 4 && bounds->x1 - 4 < x &&
            z < bounds->z2 + 4 && bounds->z1 - 4 < z) {
            *segments = segmentIndex;
            count++;
            segments++;
        }
    }
    return count;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `get_inside_segment_count_xyz`. Mickey's resident types, bindings, and
 * instruction schedule are authoritative; the donor name is not adopted.
 */
s32 func_8000FD68(s32 *segments, s16 x1, s16 y1, s16 z1, s16 x2, s16 y2,
                  s16 z2) {
    s32 count;
    s32 segmentIndex;
    TrackBoundingBox *bounds;

    x1 -= 4;
    y1 -= 4;
    z1 -= 4;
    x2 += 4;
    y2 += 4;
    z2 += 4;

    segmentIndex = 0;
    count = 0;

    while (segmentIndex < D_800792E8->segmentCount) {
        bounds = &D_800792E8->segmentBounds[segmentIndex];
        if ((bounds->x2 >= x1) && (x2 >= bounds->x1) &&
            (bounds->z2 >= z1) && (z2 >= bounds->z1) &&
            (bounds->y2 >= y1) && (y2 >= bounds->y1)) {
            count++;
            *segments++ = segmentIndex;
        }
        segmentIndex++;
    }
    return count;
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * function `block_get`. Mickey's stricter upper bound, TrackData layout,
 * function boundary, and bytes are authoritative.
 */
TrackSegment *func_8000FEB4(s32 segmentIndex) {
    if ((segmentIndex < 0) ||
        (segmentIndex >= D_800792E8->segmentCount)) {
        return NULL;
    }
    return &D_800792E8->segments[segmentIndex];
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * function `block_boundbox`. Mickey's TrackData layout, function boundary,
 * and bytes are authoritative.
 */
TrackBoundingBox *func_8000FEEC(s32 segmentIndex) {
    if ((segmentIndex < 0) ||
        (D_800792E8->segmentCount < segmentIndex)) {
        return NULL;
    }
    return &D_800792E8->segmentBounds[segmentIndex];
}
/*
 * PROVENANCE: JFG's public `src/track.c` supplies a same-position,
 * assembly-only placeholder with the same three-plane skeleton. Mickey's
 * matrix, inputs, arithmetic, and output layout are authoritative.
 */
void func_8000FF2C(void) {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 pad0;
    f32 distance;
    TrackPlanePoints *points;
    TrackPlane *plane;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 inverseLength;
    f32 (*matrix)[4];
    s32 index;

    points = D_8007927C;
    plane = D_800C9578;
    matrix = camGetInvProjMtx();
    index = 0;
    do {
        mtxf_transform_point(matrix, points->x0, points->y0, points->z0,
                             &x0, &y0, &z0);
        mtxf_transform_point(matrix, points->x1, points->y1, points->z1,
                             &x1, &y1, &z1);
        mtxf_transform_point(matrix, points->x2, points->y2, points->z2,
                             &x2, &y2, &z2);

        normalX = ((z1 - z2) * y0) + (y1 * (z2 - z0)) +
                  (y2 * (z0 - z1));
        normalY = ((x1 - x2) * z0) + (z1 * (x2 - x0)) +
                  (z2 * (x0 - x1));
        normalZ = ((y1 - y2) * x0) + (x1 * (y2 - y0)) +
                  (x2 * (y0 - y1));
        inverseLength = 1.0f /
            sqrtf((normalX * normalX) + (normalY * normalY) +
                  (normalZ * normalZ));
        if (inverseLength > 0.0f) {
            normalX *= inverseLength;
            normalY *= inverseLength;
            normalZ *= inverseLength;
        }

        distance = -((x0 * normalX) + (y0 * normalY) +
                     (z0 * normalZ));
        index++;
        plane->x = normalX;
        plane->y = normalY;
        plane->z = normalZ;
        points++;
        plane++;
        plane[-1].distance = distance;
    } while (index != 3);
}
s32 func_80010178(u32 segmentIndex) {
    f32 pad0;
    f32 pad1;
    f32 pad2;
    f32 pad3;
    f32 pad4;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 pad5;
    f32 y2;
    f32 z2;
    f32 x2;
    f32 pad6;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    TrackPlane *plane;
    TrackBoundingBox *bounds;
    s32 planeCount;

    if (D_8007926C != NULL) {
        if (TrapDanglingJump(D_8007926C, segmentIndex) == 0) {
            return FALSE;
        }
    } else {
        if ((segmentIndex >= (u32) D_800792E8->segmentCount) ||
            (D_800C953C == -1) ||
            (D_800792E8->visibility[D_800C953C + segmentIndex] == 0)) {
            return FALSE;
        }
    }

    bounds = &D_800792E8->segmentBounds[segmentIndex];
    plane = D_800C9578;
    planeCount = 2;
    x2 = bounds->x2;
    y2 = bounds->y2;
    z2 = bounds->z2;
    x1 = bounds->x1;
    y1 = bounds->y1;
    z1 = bounds->z1;
    do {
        planeX = plane->x;
        planeY = plane->y;
        planeZ = plane->z;
        if ((-plane->distance <
             (((x2 * planeX) + (y2 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y2 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y1 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y1 * planeY)) + (z2 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y2 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y2 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x2 * planeX) + (y1 * planeY)) + (z1 * planeZ))) ||
            (-plane->distance <
             (((x1 * planeX) + (y1 * planeY)) + (z1 * planeZ)))) {
            goto next_plane;
        }
        return FALSE;
next_plane:
        plane++;
    } while (planeCount--);
    return TRUE;
}
/* Workbench verdict: structure-mismatch, 158 raw/masked differing words;
 * first mismatch +0x0. Corrected Mickey jump-table dispatch remains nonexact. */
/* Candidate: 156/160 instructions with a -0x60 frame versus target -0x38;
 * target/candidate static relocations are 20/12. */
/* Shape status: both jump tables and all seven no-op kinds are reconstructed;
 * the saved-FP declaration web remains unresolved. */
/* PROVENANCE: JFG's assembly-only object-alpha routine supplies the role and switch family;
 * Mickey's jump tables, fields, globals, and arithmetic are authoritative here. */
#ifdef NON_MATCHING
s32 func_800103D4(void *object) {
    u8 *gameMode;
    void *state;
    void *bounds;
    TrackPlane *plane;
    f32 objectX;
    f32 objectY;
    f32 objectZ;
    f32 radius;
    f32 fadeDistance;
    f32 fadeRange;
    f32 fadeScale;
    f32 fadeRemaining;
    s16 kind;
    s16 distanceLimit;
    s32 visible;

    visible = 1;
    gameMode = func_80028F54();
    kind = *(s16 *) ((u8 *) object + 0x44);
    switch (kind) {
    case 65:
        state = *(void **) ((u8 *) object + 0x64);
        *(u8 *) ((u8 *) object + 0x39) = (s32) *(f32 *) ((u8 *) state + 0x18);
        break;
    case 63:
        state = *(void **) ((u8 *) object + 0x64);
        *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) state + 0xF);
        break;
    case 1:
        state = *(void **) ((u8 *) object + 0x64);
        if (*gameMode == 5) {
            *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) state + 0x190);
        } else if (((*(u16 *) ((u8 *) state + 0x1A8) & 1) == 0) ||
                   (*(u8 *) ((u8 *) state + 0x170) == 0)) {
            *(u8 *) ((u8 *) object + 0x39) = 0xFF;
        }
        break;
    case 80:
        state = *(void **) ((u8 *) object + 0x64);
        *(u8 *) ((u8 *) object + 0x39) = *(u8 *) ((u8 *) state + 2);
        break;
    case 88:
        state = *(void **) ((u8 *) object + 0x64);
        *(u8 *) ((u8 *) object + 0x39) = *(u32 *) ((u8 *) state + 4);
        break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 29:
    case 79:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 27:
    case 28:
    case 64:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    default:
        *(u8 *) ((u8 *) object + 0x39) = 0xFF;
        break;
    }
    if (*(u8 *) ((u8 *) object + 0x39) == 0) {
        return 0;
    }
    bounds = *(void **) ((u8 *) object + 0x40);
    distanceLimit = *(s16 *) ((u8 *) bounds + 0x16);
    if (distanceLimit != 0) {
        fadeDistance = camDistance(*(f32 *) ((u8 *) object + 0xC),
                                   *(f32 *) ((u8 *) object + 0x10),
                                   *(f32 *) ((u8 *) object + 0x14));
        fadeRange = (f32) distanceLimit;
        if (fadeRange < fadeDistance) {
            visible = 0;
        } else {
            fadeRemaining = fadeRange - fadeDistance;
            fadeScale = fadeRange * D_80081770;
            if (fadeRemaining < fadeScale) {
                *(u8 *) ((u8 *) object + 0x39) =
                    (s32) ((f32) *(u8 *) ((u8 *) object + 0x39) *
                           (fadeRemaining / fadeScale));
            }
        }
    }
    if (visible != 0) {
        objectX = *(f32 *) ((u8 *) object + 0xC);
        objectY = *(f32 *) ((u8 *) object + 0x10);
        objectZ = *(f32 *) ((u8 *) object + 0x14);
        radius = *(f32 *) ((u8 *) object + 0x34);
        plane = D_800C9578;
        do {
            f32 planeX;
            f32 planeY;
            f32 planeZ;
            f32 planeDistance;

            planeX = plane->x;
            planeY = plane->y;
            planeZ = plane->z;
            planeDistance = plane->distance;
            if ((((objectX * planeX) + (objectY * planeY)) +
                 (objectZ * planeZ) + planeDistance + radius) < 0.0f) {
                visible = 0;
            }
            plane++;
            if (visible == 0) {
                break;
            }
        } while ((u8 *) plane < (u8 *) &D_800C95A8);
    }
    return visible;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800103D4.s")
#endif
typedef struct TrackRayPoint {
    f32 x;
    f32 y;
    f32 z;
} TrackRayPoint;

#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c draft and the resident collision-node and plane
 * offsets reconstruct this intersection query; no external body is adapted.
 */
typedef struct TrackRayNode {
    u8 pad00[0x1C];
    TrackPlane *planes;
} TrackRayNode;

/* Candidate: 171/171 words, 162 differing, first mismatch +0x0, frame 0x70 versus 0x98. */
/* Forming the hit point once before the edge loop closes the +4 size gap. */
/* Two callee-saves remain: a dead edge copy and the hoisted D_80081774 address. */
s32 func_80010654(TrackRayPoint *start, TrackRayPoint *end,
                  TrackPlane *result, f32 *maximum) {
    u8 *node;
    f32 planeX;
    f32 planeZ;
    f32 differenceX;
    f32 differenceY;
    f32 differenceZ;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f12;
    f32 temp_f14;
    f32 temp_f16;
    f32 temp_f18;
    f32 temp_f18_2;
    f32 temp_f28;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f30;
    f32 normalValue;
    s32 temp_a2;
    s32 temp_t2;
    s32 var_a3;
    s32 var_s1;
    s32 var_s2;
    s32 var_v0;
    s32 var_v1;
    u16 edge;

    differenceX = end->x - start->x;
    var_v1 = 0;
    var_v0 = 0;
    var_a3 = 0;
    differenceY = end->y - start->y;
    differenceZ = end->z - start->z;
    if (D_800C9D3C > 0) {
        do {
            var_v0++;
            temp_a2 = *(s32 *) ((u8 *) D_800C9D2C + var_a3);
            if (temp_a2 > 0) {
                node = (u8 *) (temp_a2 | (s32) 0x80000000);
            } else {
                TrackPlane *planes;
                TrackPlane *plane;
                u16 *entry;

                entry = (u16 *) temp_a2;
                planes = ((TrackRayNode *) node)->planes;
                plane = &planes[*entry];
                temp_f30 = plane->y;
                if (D_80081774 <= temp_f30) {
                    temp_f0 = plane->x;
                    temp_f2 = plane->z;
                    temp_f16 = plane->distance;
                    planeX = temp_f0;
                    planeZ = temp_f2;
                    temp_f18 = (end->z * temp_f2) +
                               ((temp_f0 * end->x) + (temp_f30 * end->y)) + temp_f16;
                    if (temp_f18 < 0.0f) {
                        temp_f2_2 = start->x;
                        temp_f12 = start->y;
                        temp_f14 = start->z;
                        temp_f0_2 = (temp_f14 * planeZ) +
                                    ((planeX * temp_f2_2) + (temp_f30 * temp_f12)) + temp_f16;
                        if (temp_f0_2 >= 0.0f) {
                            temp_f28 = temp_f0_2 / (temp_f0_2 - temp_f18);
                            if (temp_f28 <= *maximum) {
                                temp_f2_2 = temp_f2_2 + (differenceX * temp_f28);
                                temp_f12 = temp_f12 + (differenceY * temp_f28);
                                temp_f14 = temp_f14 + (differenceZ * temp_f28);
                                var_s2 = 0 * 2;
                                var_s1 = 1;
loop_80010654:
                                    var_s2 += 2;
                                    edge = *(u16 *) ((u8 *) entry + var_s2);
                                    temp_t2 = edge & 0x8000;
                                    plane = &planes[edge ^ temp_t2];
                                    temp_f18_2 = ((plane->x * temp_f2_2) +
                                                  (plane->y * temp_f12)) +
                                                 (plane->z * temp_f14) + plane->distance;
                                    normalValue = temp_f18_2;
                                    if (temp_t2 != 0) {
                                        normalValue = -temp_f18_2;
                                    }
                                    if (normalValue > 0.0f) {
                                        var_s1 = 0;
                                    }
                                    if ((var_s2 < 6) && (var_s1 != 0)) {
                                        goto loop_80010654;
                                    }
                                if (var_s1 != 0) {
                                    *maximum = temp_f28;
                                    result->y = temp_f30;
                                    result->x = planeX;
                                    var_v1 = 1;
                                    result->z = planeZ;
                                }
                            }
                        }
                    }
                }
            }
            var_a3 += 4;
        } while (var_v0 < D_800C9D3C);
    }
    return var_v1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010654.s")
#endif
/*
 * PROVENANCE: Mickey's m2c control-flow draft and resident collision
 * records reconstruct this wrapper; no external function body is adapted.
 *
 * The z square is `(&scratch.direction.x)[2]` rather than `scratch.direction.z`
 * so the sum is a first definition of lengthSquared that uopt does not copy-prop
 * into an expression temp (L145/L160). Assigning that sum through temp_f20 folds
 * the def; temp_f0 and temp_f20 stay declared as unused frame carriers. var_s4
 * is initialised after the first calls so its p1 save outranks the scratch
 * address webs.
 */
typedef struct TrackRayHit {
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
    f32 x;
    f32 y;
    f32 z;
    f32 ratio;
    s32 faceData;
    u8 material;
} TrackRayHit;

typedef struct TrackRayScratch {
    TrackRayPoint direction;
    u8 result[0x1C];
    f32 length;
} TrackRayScratch;

extern s32 func_80011980(TrackRayPoint *start, TrackRayPoint *end,
                         TrackRayPoint *offset, f32 scale, f32 planeOffset,
                         f32 threshold, TrackRayHit *hit);
extern s32 func_80011CDC(TrackVec3f *origin, TrackVec3f *direction, f32 radius,
                         TrackRayHit *hit);

/* Declaration order is load-bearing: homes descend from the frame top, so
 * var_s4/var_s7 take the two cells above `scratch` and sp6C/sp68 the two lowest. */
s32 func_80010900(TrackVec3f *arg0, TrackVec3f *arg1, f32 arg2, s32 arg3,
                  void (*arg4)(void *, void *, f32 *, f32, void *, s32)) {
    s32 var_s4;
    s32 var_s7;
    TrackRayScratch scratch;
    f32 temp_f0;
    f32 temp_f20;
    f32 lengthSquared;
    s32 var_s2;
    s32 var_v0;
    s32 sp6C;
    s32 sp68;
    sp6C = 0;
    sp68 = 0;
    var_s7 = 0;
    do {
        var_s2 = 0;
        scratch.direction.x = arg1->f[0] - arg0->f[0];
        scratch.direction.y = arg1->f[1] - arg0->f[1];
        scratch.direction.z = arg1->f[2] - arg0->f[2];
        lengthSquared = ((&scratch.direction.x)[2] * (&scratch.direction.x)[2]) +
                        ((scratch.direction.x * scratch.direction.x) +
                         (scratch.direction.y * scratch.direction.y));
        if (lengthSquared > 0.0f) {
            lengthSquared = sqrtf(lengthSquared);
            scratch.length = lengthSquared;
            scratch.direction.x /= lengthSquared;
            scratch.direction.y /= lengthSquared;
            scratch.direction.z /= lengthSquared;
            if (D_800C9D28 != 0) {
                var_v0 = func_80011980(arg0, arg1, &scratch.direction,
                                       lengthSquared, arg2, 0.0f,
                                       (TrackRayHit *) scratch.result);
            } else {
                var_v0 = func_80011980(arg0, arg1, &scratch.direction,
                                       lengthSquared, arg2, arg2,
                                       (TrackRayHit *) scratch.result);
            }
            /* `var_s4 = 0` belongs here, not at the top of the loop body.
             * At the top its live range spans four calls, so its p1 save is
             * totalsave 30 / nocs 4 = 7.5 and loses the round to the two
             * `scratch` address webs at 8.0. Initialising it here drops the
             * span and lifts the save above 8.0. */
            var_s4 = 0;
            if (D_800C9D28 != 0) {
                var_s4 = func_80011CDC((TrackVec3f *) arg0,
                                       (TrackVec3f *) &scratch.direction, arg2,
                                       (TrackRayHit *) scratch.result);
            }
            if ((var_v0 | var_s4) != 0) {
                arg4(arg0, arg1, (f32 *) &scratch.direction, lengthSquared,
                     scratch.result, arg3);
                var_s2 = 1;
                sp68 = 1;
            }
            if (var_s2 != 0) {
                var_s7 += 1;
                if (var_s7 >= 6) {
                    sp68 = 0;
                    sp6C |= 0x40000000;
                    var_s2 = 0;
                    arg1->f[0] = arg0->f[0];
                    arg1->f[1] = arg0->f[1];
                    arg1->f[2] = arg0->f[2];
                }
            }
        }
    } while (var_s2 != 0);
    return sp68 | sp6C;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-response draft and the resident ray
 * helper declarations reconstruct this player-intersection loop; no external
 * function body is adapted. The record writes retain the assembly offsets.
 */
/* Workbench verdict: structure-mismatch, 662 differing words, first mismatch +0x0. */
/* Candidate has 692/678 words and nine relocations, with frame -0x158 versus -0x148. */
/* Remaining gap: fourteen excess words, a 16-byte frame excess, and unresolved FP/pointer scheduling. */
struct TrackCollisionSurface;
struct TrackCollisionRecord;
extern void func_800115E4(
    s32 mode, TrackVec3f *position, TrackVec3f *offset, f32 scale,
    struct TrackCollisionSurface *surface,
    struct TrackCollisionRecord *record);

#define B4C_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define B4C_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define B4C_F32(base, offset) (*(f32 *) ((u8 *) (base) + (offset)))

s32 func_80010B4C(s32 arg0, void *arg1, f32 *arg2, f32 *arg3,
                  void *arg4, void *arg5, void *arg6) {
    TrackRayHit intersection;
    f32 relative[16];
    TrackRayPoint direction;
    TrackRayPoint *start;
    TrackRayPoint *end;
    u8 *record;
    f32 *scalePtr;
    f32 lengthSquared;
    f32 length;
    f32 minimumLength;
    f32 scale;
    s32 minimumIndex;
    s32 count;
    s32 index;
    s32 attempt;
    u32 bit;
    u32 collisionMask;
    u32 resultMask;
    u32 failureMask;
    s32 collision;
    s32 queryResult;
    s32 auxiliaryResult;

    if (arg5 != NULL) {
        for (index = 0; index < arg0; index++) {
            relative[index * 3] =
                ((f32 *) arg2)[index * 3] - B4C_F32(arg5, 0);
            relative[(index * 3) + 1] =
                ((f32 *) arg2)[(index * 3) + 1] - B4C_F32(arg5, 4);
            relative[(index * 3) + 2] =
                ((f32 *) arg2)[(index * 3) + 2] - B4C_F32(arg5, 8);
        }
    }
    if (arg0 > 0) {
        for (index = 0; index < arg0; index++) {
            record = (u8 *) ((u32) arg4 + (index * 0x40));
            B4C_S32(record, 0) = 0;
            B4C_U8(record, 0x3D) = 0;
            B4C_F32(record, 4) = 0.0f;
            B4C_F32(record, 8) = 0.0f;
            B4C_F32(record, 0xC) = 0.0f;
            B4C_F32(record, 0x10) = 0.0f;
            B4C_F32(record, 0x14) = 0.0f;
            B4C_F32(record, 0x18) = 0.0f;
            B4C_F32(record, 0x1C) = 0.0f;
            B4C_F32(record, 0x20) = 0.0f;
            B4C_F32(record, 0x24) = 0.0f;
            B4C_F32(record, 0x28) = 0.0f;
            B4C_F32(record, 0x2C) = 0.0f;
            B4C_F32(record, 0x30) = 0.0f;
            B4C_F32(record, 0x34) = 32000.0f;
            B4C_U8(record, 0x3C) = 0;
            B4C_S32(record, 0x38) = 0;
        }
    }
    resultMask = 0;
    attempt = 0;
    failureMask = 0;
    do {
        collisionMask = 0;
        bit = 1;
        scalePtr = arg3;
        index = 0;
        do {
            start = (TrackRayPoint *) ((u8 *) arg1 + (index * 0xC));
            end = (TrackRayPoint *) ((u8 *) arg2 + (index * 0xC));
            scale = *scalePtr;
            count = 0;
            do {
                collision = 0;
                auxiliaryResult = 0;
                direction.x = end->x - start->x;
                direction.y = end->y - start->y;
                direction.z = end->z - start->z;
                lengthSquared = (direction.z * direction.z) +
                                ((direction.x * direction.x) +
                                 (direction.y * direction.y));
                if (lengthSquared > 0.0f) {
                    length = sqrtf(lengthSquared);
                    intersection.ratio = length;
                    direction.x /= length;
                    direction.y /= length;
                    direction.z /= length;
                    if (D_800C9D28 != 0) {
                        queryResult = func_80011980(
                            start, end,
                            &direction, length, scale, 0.0f,
                            &intersection);
                    } else {
                        queryResult = func_80011980(
                            start, end,
                            &direction, length, scale, scale,
                            &intersection);
                    }
                    if (D_800C9D28 != 0) {
                        auxiliaryResult = func_80011CDC(
                            (TrackVec3f *) start, (TrackVec3f *) &direction,
                            scale, (TrackRayHit *) &intersection);
                    }
                    if ((queryResult | auxiliaryResult) != 0) {
                        record = (u8 *) ((u32) arg4 + (index * 0x40));
                        func_800115E4(
                            (s32) start, (TrackVec3f *) end, &direction, length,
                            (struct TrackCollisionSurface *) &intersection,
                            (struct TrackCollisionRecord *) record);
                        B4C_F32(record, 0x34) = intersection.ratio;
                        collision = 1;
                        collisionMask |= bit;
                    }
                    if (collision != 0) {
                        count++;
                        if (count >= 0xB) {
                            collisionMask = 0;
                            collision = 0;
                            failureMask |= 0x40000000;
                        }
                    }
                }
            } while (collision != 0);
            bit <<= 1;
            index++;
            scalePtr++;
        } while ((index < arg0) && (failureMask == 0));
        if (((collisionMask != 0) && (attempt >= 0xB)) ||
            (failureMask != 0)) {
            for (index = 0; index < arg0; index++) {
                ((f32 *) arg2)[index * 3] =
                    ((f32 *) arg1)[index * 3];
                ((f32 *) arg2)[(index * 3) + 1] =
                    ((f32 *) arg1)[(index * 3) + 1];
                ((f32 *) arg2)[(index * 3) + 2] =
                    ((f32 *) arg1)[(index * 3) + 2];
            }
            resultMask = 0;
            B4C_F32(arg5, 0) = B4C_F32(arg1, 0) - relative[0];
            B4C_F32(arg5, 4) = B4C_F32(arg1, 4) - relative[1];
            B4C_F32(arg5, 8) = B4C_F32(arg1, 8) - relative[2];
            if (attempt >= 0xB) {
                failureMask |= 0x80000000;
            }
        } else if (collisionMask != 0) {
            minimumIndex = 0;
            if (arg5 != NULL) {
                minimumLength = 32000.0f;
                bit = 1;
                for (index = 0; index < arg0; index++) {
                    if ((collisionMask & bit) != 0) {
                        record = (u8 *) ((u32) arg4 + (index * 0x40));
                        if (B4C_F32(record, 0x34) < minimumLength) {
                            minimumIndex = index;
                            minimumLength = B4C_F32(record, 0x34);
                        }
                    }
                    bit <<= 1;
                }
                record = (u8 *) ((u32) arg4 + (minimumIndex * 0x40));
                B4C_U8(record, 0x3D) |= 1;
                B4C_F32(arg5, 0) = B4C_F32(arg2, minimumIndex * 0xC) -
                          relative[minimumIndex * 3];
                B4C_F32(arg5, 4) = B4C_F32(arg2, (minimumIndex * 0xC) + 4) -
                          relative[(minimumIndex * 3) + 1];
                B4C_F32(arg5, 8) = B4C_F32(arg2, (minimumIndex * 0xC) + 8) -
                          relative[(minimumIndex * 3) + 2];
                for (index = 0; index < arg0; index++) {
                    B4C_F32(arg2, index * 0xC) =
                        relative[index * 3] + B4C_F32(arg5, 0);
                    B4C_F32(arg2, (index * 0xC) + 4) =
                        relative[(index * 3) + 1] + B4C_F32(arg5, 4);
                    B4C_F32(arg2, (index * 0xC) + 8) =
                        relative[(index * 3) + 2] + B4C_F32(arg5, 8);
                }
                resultMask |= collisionMask;
            }
        }
        attempt++;
    } while ((collisionMask != 0) && (failureMask == 0));
    return resultMask | failureMask;
}
#undef B4C_U8
#undef B4C_S32
#undef B4C_F32
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80010B4C.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-response draft and resident plane and
 * record offsets reconstruct this routine; no external function body is adapted.
 */
typedef struct TrackCollisionSurface {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    f32 positionDistance;
    s32 flags;
    u8 material;
} TrackCollisionSurface;

typedef struct TrackCollisionRecord {
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 value0C;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    f32 value24;
    f32 value28;
    f32 value2C;
    f32 value30;
    u8 pad34[4];
    s32 value38;
    u8 value3C;
    u8 value3D;
} TrackCollisionRecord;

s16 Arctanf(f32 x, f32 y);

/* Workbench verdict: structure-mismatch, 217 differing words, first mismatch +0x0. */
/* Candidate: 237/231 words, frame 0xA0 versus 0x98, 6/17 relocation sites exact. */
/* Mickey m2c restores the defined flag test and numerator negation before division. */
void func_800115E4(s32 mode, TrackVec3f *position, TrackVec3f *offset,
                   f32 scale, TrackCollisionSurface *surface,
                   TrackCollisionRecord *record) {
    f32 firstCrossX;
    f32 firstCrossY;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 surfaceDistance;
    f32 surfaceX;
    f32 surfaceY;
    f32 surfaceZ;
    f32 planeValue;
    f32 crossLengthSquared;
    f32 crossLength;
    f32 distance;
    f32 time;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 differenceX;
    f32 differenceY;
    f32 differenceZ;
    f32 angle;
    f32 horizontalLength;
    s16 angleValue;

    surfaceDistance = surface->distance;
    surfaceX = surface->x;
    surfaceY = surface->y;
    surfaceZ = surface->z;
    planeValue = (position->f[2] * surfaceZ) +
                 ((surfaceX * position->f[0]) +
                  (surfaceY * position->f[1])) + surfaceDistance;
    if ((D_80081778 <= surfaceY) || (surface->flags & 0x10000000)) {
        firstCrossX = offset->f[2] * surfaceY;
        firstCrossY = (surfaceZ * offset->f[0]) -
                      (offset->f[2] * surfaceX);
        crossZ = -(offset->f[0] * surfaceY);
        crossX = (firstCrossY * surfaceZ) - (crossZ * surfaceY);
        crossY = (crossZ * surfaceX) - (firstCrossX * surfaceZ);
        crossZ = (firstCrossX * surfaceY) - (firstCrossY * surfaceX);
        crossLengthSquared = (crossX * crossX) +
                             (crossY * crossY) +
                             (crossZ * crossZ);
        if (D_8008177C < crossLengthSquared) {
            distance = sqrtf(crossLengthSquared);
            time = scale - surface->positionDistance;
            position->f[0] = surface->positionX +
                             (time * (crossX / distance));
            position->f[1] = surface->positionY +
                             (time * (crossY / distance));
            position->f[2] = surface->positionZ +
                             (time * (crossZ / distance));
        } else {
            position->f[1] = (-((position->f[2] * surfaceZ) +
                                 (surfaceX * position->f[0]) +
                                 surfaceDistance) / surfaceY) + D_80081780;
        }
        record->pointY = surfaceX;
        record->pointZ = surfaceY;
        record->value0C = surfaceZ;
        record->value3D |= 2;
    } else if (surfaceY <= D_80081784) {
        distance = D_80081788 - planeValue;
        position->f[0] += distance * surfaceX;
        position->f[1] += distance * surfaceY;
        position->f[2] += distance * surfaceZ;
        record->value1C = surfaceX;
        record->value20 = surfaceY;
        record->value24 = surfaceZ;
        record->value3D |= 8;
    } else {
        projectedX = position->f[0];
        projectedY = position->f[1];
        projectedZ = position->f[2];
        distance = D_8008178C - planeValue;
        projectedX = projectedX + (distance * surfaceX);
        projectedY = projectedY + (distance * surfaceY);
        projectedZ = projectedZ + (distance * surfaceZ);
        differenceX = position->f[0] - projectedX;
        differenceY = position->f[1] - projectedY;
        differenceZ = position->f[2] - projectedZ;
        angle = sqrtf((differenceX * differenceX) +
                      (differenceZ * differenceZ));
        angleValue = Arctanf(differenceY, angle);
        angle = func_8002A8BC(angleValue);
        if (angle != 0.0f) {
            time = distance / angle;
            horizontalLength = sqrtf((surfaceX * surfaceX) +
                                     (surfaceZ * surfaceZ));
            position->f[0] += time * (surfaceX / horizontalLength);
            position->f[2] += time * (surfaceZ / horizontalLength);
        } else {
            position->f[0] = projectedX;
            position->f[1] = projectedY;
            position->f[2] = projectedZ;
        }
        record->value10 = surfaceX;
        record->value14 = surfaceY;
        record->value18 = surfaceZ;
        record->value3D |= 4;
    }
    record->value28 = surfaceX;
    record->value2C = surfaceY;
    record->value30 = surfaceZ;
    record->value38 = surface->flags;
    record->value3C = surface->material;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800115E4.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision-query draft and resident node/plane
 * offsets reconstruct this ray query; no external function body is adapted.
 */
typedef struct TrackRayFace {
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
} TrackRayFace;

typedef struct TrackRayMeta {
    u8 material;
    u8 pad01[0x0B];
    s32 data;
} TrackRayMeta;

typedef struct TrackRayNodeExtended {
    u8 pad00[0x0C];
    TrackRayMeta *metadata;
    u8 pad10[0x0C];
    TrackRayFace *planes;
} TrackRayNodeExtended;

/* Candidate (Track B, 2026-09-23): 216/215 words, 195 differing, frame 0xD8
 * versus 0xC8. Typed plane/metadata subscripts, sums left-associated, the edge
 * index incremented at the loop's end and initialised in the entry block (which
 * reproduces the target's shifted-zero preheader). The rest is the p1 ranking:
 * the target gives the inner loop's edge, sign and face pointer the last four
 * callee-saved registers and keeps the entry pointer in a copy, while this body
 * gives them the first caller-saved colours and runs out of registers (ra). */
s32 func_80011980(TrackRayPoint *start, TrackRayPoint *end,
                  TrackRayPoint *offset, f32 scale, f32 planeOffset,
                  f32 threshold, TrackRayHit *hit) {
    TrackRayNodeExtended *node;
    u16 *entry;
    TrackRayFace *planes;
    TrackRayFace *face;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeValue;
    f32 startValue;
    f32 endValue;
    f32 ratio;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 edgeValue;
    f32 adjustedOffset;
    s32 encoded;
    s32 segmentIndex;
    s32 edgeValid;
    s32 valid;
    s32 sign;
    s32 i;
    u16 edge;

    valid = 0;
    segmentIndex = 0;
    if (D_800C9D3C > 0) {
        do {
        encoded = D_800C9D2C[segmentIndex];
        if (encoded > 0) {
            node = (TrackRayNodeExtended *) (encoded | (s32) 0x80000000);
        } else {
            entry = (u16 *) encoded;
            i = 0;
            planes = node->planes;
            face = &planes[*entry];
            planeX = face->x;
            planeY = face->y;
            planeZ = face->z;
            planeValue = face->distance - planeOffset;
            endValue = planeX * end->x + planeY * end->y + end->z * planeZ +
                       planeValue;
            if (endValue < 0.0f) {
                startValue = planeX * start->x + planeY * start->y +
                             start->z * planeZ + planeValue;
                if (startValue >= 0.0f) {
                    ratio = (startValue / (startValue - endValue)) * scale;
                    if (ratio <= hit->ratio) {
                        edgeValid = 1;
                        pointX = ((offset->x * ratio) + start->x) -
                                 (planeOffset * planeX);
                        pointY = ((offset->y * ratio) + start->y) -
                                 (planeOffset * planeY);
                        pointZ = ((offset->z * ratio) + start->z) -
                                 (planeOffset * planeZ);
                        do {
                            edge = entry[i + 1];
                            sign = edge & 0x8000;
                            face = &planes[edge ^ sign];
                            edgeValue = face->distance +
                                        (face->x * pointX + face->y * pointY +
                                         face->z * pointZ);
                            if (sign != 0) {
                                edgeValue = -edgeValue;
                            }
                            if (threshold < edgeValue) {
                                edgeValid = 0;
                            }
                            i++;
                        } while (i < 3 && edgeValid != 0);
                        if (edgeValid != 0) {
                            hit->normalX = planeX;
                            hit->normalY = planeY;
                            hit->normalZ = planeZ;
                            hit->distance = planeValue;
                            adjustedOffset = D_80081790 + planeOffset;
                            hit->x = (adjustedOffset * planeX) + pointX;
                            hit->y = (adjustedOffset * planeY) + pointY;
                            hit->z = (adjustedOffset * planeZ) + pointZ;
                            hit->faceData =
                                node->metadata[D_800C9D30[segmentIndex]].data;
                            hit->material = ((u8 *) &D_800792E8->textures[
                                node->metadata[D_800C9D30[segmentIndex]]
                                    .material])[7];
                            hit->ratio = ratio;
                            valid = 1;
                        }
                    }
                }
            }
        }
            segmentIndex++;
        } while (segmentIndex < D_800C9D3C);
    }
    return valid;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011980.s")
#endif
typedef struct TrackClipVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[4];
} TrackClipVertex;

typedef struct TrackClipFace {
    u8 flags;
    u8 vertices[3];
    u8 pad04[0x0C];
} TrackClipFace;

typedef struct TrackClipIndex {
    u8 material;
    u8 pad01[5];
    s16 vertexBase;
    u8 pad08[4];
    s32 data;
} TrackClipIndex;

typedef struct TrackClipNode {
    TrackClipVertex *origin;
    TrackClipFace *faces;
    u8 pad08[4];
    TrackClipIndex *indices;
} TrackClipNode;

typedef struct TrackClipOutput {
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 dx;
    f32 dy;
    f32 dz;
    TrackClipNode *node;
    s16 segment;
} TrackClipOutput;

#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c supplies the ray/edge collision role;
 * this body uses Mickey's resident edge records and output layout. */
/* Candidate (Track B, 2026-09-23): 347/342 words, 329 differing, frame 0xC8
 * versus 0xC0. Typed edge records (TrackClipOutput) and hit (TrackRayHit),
 * point sums written origin-first, all three points computed before the hit
 * stores (t is address-taken, so an interleaved store forces a reload), the
 * foot point carried in the difference locals, unused locals dropped. One p1
 * decision is left: the D_800792E8 address web (save 30/7) outranks the record
 * counter (31/8), is kept in s8, and pushes the counter into a caller-saved
 * register spilled at each call. Forcing that web to split alone gives
 * 342/342 words and 150 differing. */
extern s32 func_80012234(TrackVec3f *point, TrackVec3f *direction,
                         TrackVec3f *origin, TrackVec3f *planeDirection,
                         f32 radius, f32 *minimum, f32 *maximum);

s32 func_80011CDC(TrackVec3f *origin, TrackVec3f *direction, f32 radius,
                  TrackRayHit *hit) {
    TrackClipOutput *record;
    TrackClipNode *node;
    f32 t;
    f32 tEnd;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 differenceX;
    f32 differenceY;
    f32 differenceZ;
    f32 planeDistance;
    s32 recordCount;
    s32 result;
    s32 edgeHit;

    result = 0;
    recordCount = 0;
    if (D_800C9D24 > 0) {
        do {
            edgeHit = 0;
            record = recordCount + (TrackClipOutput *) D_800C9D20;
            if ((func_80012234(origin, direction, (TrackVec3f *) &record->x0,
                               (TrackVec3f *) &record->dx, radius, &t,
                               &tEnd) != 0) &&
                (t >= 0.0f) && (t <= hit->ratio)) {
                normalX = record->dx;
                normalY = record->dy;
                normalZ = record->dz;
                pointX = origin->f[0] + direction->f[0] * t;
                pointY = origin->f[1] + direction->f[1] * t;
                pointZ = origin->f[2] + direction->f[2] * t;
                differenceX = pointX - record->x0;
                differenceY = pointY - record->y0;
                differenceZ = pointZ - record->z0;
                planeDistance = (differenceX * normalX + differenceY * normalY +
                                 differenceZ * normalZ) /
                                (normalX * normalX + normalY * normalY +
                                 normalZ * normalZ);
                if ((planeDistance >= 0.0f) && (planeDistance <= 1.0f)) {
                    hit->x = pointX;
                    hit->y = pointY;
                    hit->z = pointZ;
                    result = 1;
                    edgeHit = 1;
                    differenceX = record->dx * planeDistance + record->x0;
                    differenceY = record->dy * planeDistance + record->y0;
                    differenceZ = record->dz * planeDistance + record->z0;
                    hit->normalX = (pointX - differenceX) / radius;
                    hit->normalY = (pointY - differenceY) / radius;
                    normalX = (pointZ - differenceZ) / radius;
                    hit->normalZ = normalX;
                    hit->distance = -(pointX * hit->normalX +
                                      pointY * hit->normalY +
                                      normalX * pointZ);
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((u8 *) &D_800792E8->textures[
                        node->indices[record->segment].material])[7];
                    hit->ratio = t;
                }
            }
            if (edgeHit == 0) {
                if ((func_80012574(origin, direction, (TrackVec3f *) &record->x0,
                                   radius, &t, &tEnd) != 0) &&
                    (t >= 0.0f) && (t <= hit->ratio)) {
                    edgeHit = 1;
                    result = 1;
                    pointX = origin->f[0] + direction->f[0] * t;
                    pointY = origin->f[1] + direction->f[1] * t;
                    pointZ = origin->f[2] + direction->f[2] * t;
                    hit->x = pointX;
                    hit->y = pointY;
                    hit->z = pointZ;
                    hit->normalX = (pointX - record->x0) / radius;
                    hit->normalY = (pointY - record->y0) / radius;
                    hit->normalZ = (pointZ - record->z0) / radius;
                    hit->distance = -(pointX * hit->normalX +
                                      pointY * hit->normalY +
                                      hit->normalZ * pointZ);
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((u8 *) &D_800792E8->textures[
                        node->indices[record->segment].material])[7];
                    hit->ratio = t;
                }
            }
            if (edgeHit == 0) {
                if ((func_80012574(origin, direction, (TrackVec3f *) &record->x1,
                                   radius, &t, &tEnd) != 0) &&
                    (t >= 0.0f) && (t <= hit->ratio)) {
                    result = 1;
                    pointX = origin->f[0] + direction->f[0] * t;
                    pointY = origin->f[1] + direction->f[1] * t;
                    pointZ = origin->f[2] + direction->f[2] * t;
                    hit->x = pointX;
                    hit->y = pointY;
                    hit->z = pointZ;
                    hit->normalX = (pointX - record->x1) / radius;
                    hit->normalY = (pointY - record->y1) / radius;
                    hit->normalZ = (pointZ - record->z1) / radius;
                    hit->distance = -(pointX * hit->normalX +
                                      pointY * hit->normalY +
                                      hit->normalZ * pointZ);
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((u8 *) &D_800792E8->textures[
                        node->indices[record->segment].material])[7];
                    hit->ratio = t;
                }
            }
            recordCount++;
        } while (recordCount < D_800C9D24);
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_80011CDC.s")
#endif
/*
 * PROVENANCE: Mickey's m2c FP dataflow and the resident vector layout
 * reconstruct this plane-intersection query; no external function body is adapted.
 * Matched 2026-09-23 (Track B): the three sums written left-associated and
 * the cross products in textbook a[j]*b[k] - a[k]*b[j] order (size +12 -> 0),
 * normalLength declared first so it takes the frame's top home, the dot
 * product assigned straight into absoluteDot and negated in place (a separate
 * scalar carrier gave the stack radius a bb-local web that outranked it for
 * $f2), and the plane dot carried by directionDot, which it later reuses.
 */
s32 func_80012234(TrackVec3f *point, TrackVec3f *direction,
                  TrackVec3f *origin, TrackVec3f *planeDirection,
                  f32 radius, f32 *minimum, f32 *maximum) {
    f32 normalLength;
    f32 delta[3];
    f32 normal[3];
    f32 cross[3];
    f32 absoluteDot;
    f32 planeOffset;
    f32 scalar;
    f32 interval;
    f32 directionDot;
    s32 result;

    delta[0] = point->f[0] - origin->f[0];
    delta[1] = point->f[1] - origin->f[1];
    delta[2] = point->f[2] - origin->f[2];
    normal[0] = (direction->f[1] * planeDirection->f[2]) -
                (direction->f[2] * planeDirection->f[1]);
    normal[1] = (direction->f[2] * planeDirection->f[0]) -
                (direction->f[0] * planeDirection->f[2]);
    normal[2] = (direction->f[0] * planeDirection->f[1]) -
                (direction->f[1] * planeDirection->f[0]);
    normalLength = normal[0] * normal[0] + normal[1] * normal[1] +
                   normal[2] * normal[2];
    if (normalLength == 0.0f) {
        return 0;
    }
    normalLength = sqrtf(normalLength);
    normal[0] = normal[0] / normalLength;
    normal[1] = normal[1] / normalLength;
    normal[2] = normal[2] / normalLength;
    absoluteDot = delta[0] * normal[0] + delta[1] * normal[1] +
                  delta[2] * normal[2];
    if (absoluteDot < 0.0f) {
        absoluteDot = -absoluteDot;
    }
    result = 0;
    if (absoluteDot <= radius) {
        result = 1;
    }
    if (result != 0) {
        cross[0] = (delta[1] * planeDirection->f[2]) -
                   (delta[2] * planeDirection->f[1]);
        cross[1] = (delta[2] * planeDirection->f[0]) -
                   (delta[0] * planeDirection->f[2]);
        cross[2] = (delta[0] * planeDirection->f[1]) -
                   (delta[1] * planeDirection->f[0]);
        directionDot = cross[0] * normal[0] + cross[1] * normal[1] +
                       cross[2] * normal[2];
        planeOffset = -directionDot / normalLength;
        cross[0] = (normal[1] * planeDirection->f[2]) -
                   (normal[2] * planeDirection->f[1]);
        cross[1] = (normal[2] * planeDirection->f[0]) -
                   (normal[0] * planeDirection->f[2]);
        cross[2] = (normal[0] * planeDirection->f[1]) -
                   (normal[1] * planeDirection->f[0]);
        normalLength = sqrtf(cross[0] * cross[0] + cross[1] * cross[1] +
                             cross[2] * cross[2]);
        cross[0] = cross[0] / normalLength;
        cross[1] = cross[1] / normalLength;
        cross[2] = cross[2] / normalLength;
        directionDot = direction->f[0] * cross[0] +
                       direction->f[1] * cross[1] +
                       direction->f[2] * cross[2];
        scalar = radius * radius;
        interval = sqrtf(scalar - (absoluteDot * absoluteDot)) /
                   directionDot;
        if (interval < 0.0f) {
            interval = -interval;
        }
        *minimum = planeOffset - interval;
        *maximum = planeOffset + interval;
    }
    return result;
}
s32 func_80012574(TrackVec3f *origin, TrackVec3f *direction, TrackVec3f *center, f32 radius, f32 *minimum, f32 *maximum)
{
  f32 temp_f0;
  f32 temp_f0_2;
  f32 temp_f12;
  f32 temp_f14;
  float new_var2;
  f32 temp_f16;
  f32 temp_f18;
  f32 temp_f2;
  f32 temp_f2_2;
  f32 new_var;
  s32 var_v1;
  temp_f0 = origin->f[0] - center->f[0];
  temp_f2 = origin->f[1] - center->f[1];
  var_v1 = 0;
  temp_f12 = origin->f[2] - center->f[2];
  temp_f14 = ((temp_f0 * direction->f[0]) + (temp_f2 * direction->f[1])) + (temp_f12 * direction->f[2]);
  new_var = temp_f14;
  new_var2 = (((temp_f0 * temp_f0) + (temp_f2 * temp_f2)) + (temp_f12 * temp_f12)) - (radius * radius);
  temp_f18 = new_var * new_var;
  temp_f16 = new_var2;
  if (temp_f16 <= temp_f18)
  {
    var_v1 = 1;
  }
  if (var_v1 != 0)
  {
    temp_f0_2 = sqrtf(temp_f18 - temp_f16);
 do { temp_f2_2 = -new_var; *minimum = temp_f2_2 - temp_f0_2; *maximum = temp_f2_2 + temp_f0_2; } while (0);
  }
  return var_v1;
}
/*
 * PROVENANCE: Mickey's m2c draft, collision-node offsets, and output-record
 * writes reconstruct this routine; no external function body is adapted.
 */
/*
 * Matched with the face and vertex-base cursors formed once per node from
 * index locals read in D_800C9D30/D_800C9D34 order, the corner flag tested
 * through node->faces again (it is reloaded after the output stores), the
 * output record addressed as count + base, and two unreferenced s32 locals
 * declared ahead of the node pointer so its home lands at +0x34 of the 0x40
 * frame.
 */
void func_80012658(s32 flags) {
    s32 pad0;
    s32 pad1;
    TrackClipNode *node;
    TrackClipFace *face;
    TrackClipVertex *vertices;
    TrackClipVertex *first;
    TrackClipVertex *second;
    TrackClipOutput *output;
    s32 encoded;
    s32 i;
    s32 corner;
    s32 nextCorner;
    s16 faceIndex;
    s16 segmentIndex;

    D_800C9D24 = 0;
    if ((flags & 1) == 0) {
        D_800C9D28 = 0;
        return;
    }
    D_800C9D28 = 1;
    for (i = 0; i < D_800C9D3C; i++) {
        encoded = D_800C9D2C[i];
        if (encoded > 0) {
            node = (TrackClipNode *) (encoded | 0x80000000);
        } else {
            faceIndex = D_800C9D30[i];
            segmentIndex = D_800C9D34[i];
            face = &node->faces[segmentIndex];
            vertices = &node->origin[node->indices[faceIndex].vertexBase];
            for (corner = 0; corner < 3; corner++) {
                if (node->faces[segmentIndex].flags & (1 << corner)) {
                    nextCorner = corner + 1;
                    output = D_800C9D24 + (TrackClipOutput *) D_800C9D20;
                    if (nextCorner >= 3) {
                        nextCorner = 0;
                    }
                    first = &vertices[face->vertices[corner]];
                    second = &vertices[face->vertices[nextCorner]];
                    output->x0 = first->x;
                    output->y0 = first->y;
                    output->z0 = first->z;
                    output->x1 = second->x;
                    output->y1 = second->y;
                    output->z1 = second->z;
                    output->dx = output->x1 - output->x0;
                    output->node = node;
                    output->dy = output->y1 - output->y0;
                    output->dz = output->z1 - output->z0;
                    output->segment = D_800C9D30[i];
                    D_800C9D24++;
                    if (D_800C9D24 >= *(s16 *) ((u8 *) D_800792EC + 0xF0)) {
                        corner = 3;
                        i = D_800C9D3C;
                    }
                }
            }
        }
    }
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c collision trace and the resident vector/track
 * declarations reconstruct this query; no external function body is adapted.
 * Raw offsets retain the compact segment and polygon records.
 */
/* Workbench verdict: structure-mismatch, 499 differing words, first mismatch +0x0. */
/* Candidate is 538/548 instructions with frame -0x2B0 versus target -0x288. */
/* Remaining gap: ten missing instructions, 40 excess frame bytes, and two excess relocations. */
extern s32 func_800131AC(TrackVec3f *origin, TrackVec3f *direction,
                         TrackVec3f *minimum, TrackVec3f *maximum,
                         f32 *nearClip, f32 *farClip);
extern u8 getYCompareMask(void *bounds, s32 y0, s32 y1);

#define E129_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define E129_S16(base, offset) (*(s16 *) ((u8 *) (base) + (offset)))
#define E129_U16(base, offset) (*(u16 *) ((u8 *) (base) + (offset)))
#define E129_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define E129_F32(base, offset) (*(f32 *) ((u8 *) (base) + (offset)))
#define E129_PTR(base, offset) (*(void **) ((u8 *) (base) + (offset)))

s32 func_8001291C(f32 *arg0, f32 *arg1, f32 *arg2, s32 arg3, s32 arg4) {
    TrackSegment *segments[20];
    f32 entryTimes[20];
    s32 xzMasks[20];
    u8 yMasks[20];
    TrackVec3f direction;
    TrackVec3f minimum;
    TrackVec3f maximum;
    f32 bestX;
    f32 bestY;
    f32 bestZ;
    TrackBoundingBox *bounds;
    TrackData *track;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackPlane *surfaceBase;
    TrackPlane *plane;
    TrackPlane *bestPlane;
    u16 *polygon;
    f32 nearClip;
    f32 farClip;
    f32 bestDistance;
    f32 side0;
    f32 side1;
    f32 fraction;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 planeDistance;
    f32 edgeValue;
    s32 segmentIndex;
    s32 hitCount;
    s32 insertIndex;
    s32 batchCount;
    s32 batchIndex;
    s32 triangleIndex;
    s32 firstTriangle;
    s32 lastTriangle;
    s32 edgeIndex;
    s32 inside;
    s32 hit;
    u32 bestFlags;
    u32 batchFlags;
    u8 bestTexture;
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 edgeNumber;
    u16 edge;
    u16 edgeSign;
    u8 temporaryY;
    u8 *surfaceBytes;
    TrackSegment *temporarySegment;
    s32 temporaryXZ;

    direction.f[0] = arg1[0] - arg0[0];
    direction.f[1] = arg1[1] - arg0[1];
    direction.f[2] = arg1[2] - arg0[2];
    if ((direction.f[0] != 0.0f) || (direction.f[1] != 0.0f) ||
        (direction.f[2] != 0.0f)) {
        track = D_800792E8;
        hitCount = 0;
        for (segmentIndex = 0;
             segmentIndex < E129_S16(D_800792E8, 0x1A);
             segmentIndex++) {
            bounds = track->segmentBounds + segmentIndex;
            minimum.f[0] = (f32) bounds->x1;
            minimum.f[1] = (f32) bounds->y1;
            minimum.f[2] = (f32) bounds->z1;
            maximum.f[0] = (f32) bounds->x2;
            maximum.f[1] = (f32) bounds->y2;
            maximum.f[2] = (f32) bounds->z2;
            if ((func_800131AC((TrackVec3f *) arg0, &direction,
                               &minimum, &maximum, &nearClip, &farClip) != 0) &&
                (((nearClip <= 0.0f) && (farClip >= 0.0f)) ||
                 ((nearClip >= 0.0f) && (nearClip <= 1.0f)))) {
                if (nearClip < 0.0f) {
                    nearClip = 0.0f;
                }
                if (farClip > 1.0f) {
                    farClip = 1.0f;
                }
                x0 = (s32) ((direction.f[0] * nearClip) + arg0[0]);
                y0 = (s32) ((direction.f[1] * nearClip) + arg0[1]);
                z0 = (s32) ((direction.f[2] * nearClip) + arg0[2]);
                x1 = (s32) ((direction.f[0] * farClip) + arg0[0]);
                y1 = (s32) ((direction.f[1] * farClip) + arg0[1]);
                z1 = (s32) ((direction.f[2] * farClip) + arg0[2]);
                if (x1 < x0) {
                    s32 temporary = x1;
                    x1 = x0;
                    x0 = temporary;
                }
                if (y1 < y0) {
                    s32 temporary = y1;
                    y1 = y0;
                    y0 = temporary;
                }
                if (z1 < z0) {
                    s32 temporary = z1;
                    z1 = z0;
                    z0 = temporary;
                }
                entryTimes[hitCount] = nearClip;
                segments[hitCount] =
                    &D_800792E8->segments[segmentIndex];
                xzMasks[hitCount] = getXZCompareMask(bounds, x0, z0, x1, z1);
                yMasks[hitCount] = getYCompareMask(bounds, y0, y1);
                insertIndex = hitCount;
                while ((insertIndex > 0) &&
                       (entryTimes[insertIndex] <
                        entryTimes[insertIndex - 1])) {
                    nearClip = entryTimes[insertIndex];
                    temporarySegment = segments[insertIndex];
                    temporaryXZ = xzMasks[insertIndex];
                    temporaryY = yMasks[insertIndex];

                    entryTimes[insertIndex] = entryTimes[insertIndex - 1];
                    segments[insertIndex] = segments[insertIndex - 1];
                    xzMasks[insertIndex] = xzMasks[insertIndex - 1];
                    yMasks[insertIndex] = yMasks[insertIndex - 1];
                    entryTimes[insertIndex - 1] = nearClip;
                    segments[insertIndex - 1] = temporarySegment;
                    xzMasks[insertIndex - 1] = temporaryXZ;
                    yMasks[insertIndex - 1] = temporaryY;
                    insertIndex--;
                }
                hitCount++;
                if (hitCount >= 20) {
                    segmentIndex = E129_S16(D_800792E8, 0x1A);
                }
            }
        }
    }
    hit = 0;
    bestDistance = 1.0f;
    bestX = arg1[0];
    bestY = arg1[1];
    bestZ = arg1[2];
    arg3 |= 0x1080;
    for (segmentIndex = 0;
         (segmentIndex < hitCount) && (hit == 0);
         segmentIndex++) {
        segment = segments[segmentIndex];
        surfaceBase = segment->surfaces;
        batch = segment->batches;
        batchCount = segment->batchCount;
        for (batchIndex = 0; batchIndex < batchCount; batchIndex++) {
            firstTriangle = batch->v0;
            lastTriangle = batch[1].v0;
            batchFlags = batch->flags;
            if ((batchFlags & arg3) ||
                ((arg4 != 0) && ((batchFlags & arg4) == 0))) {
                firstTriangle = lastTriangle;
            }
            for (triangleIndex = firstTriangle;
                 triangleIndex < lastTriangle; triangleIndex++) {
                s32 visibility = E129_S32(
                    E129_PTR(segment, 0x10), triangleIndex * 4);
                visibility &= xzMasks[segmentIndex];
                if (((visibility & 0xFFFF) != 0) &&
                    ((visibility & 0xFFFF0000) != 0) &&
                    ((E129_U8(E129_PTR(segment, 0x14), triangleIndex) &
                      yMasks[segmentIndex]) != 0)) {
                    polygon = &segment->surfaceIndices[triangleIndex * 4];
                    plane = &surfaceBase[polygon[0]];
                    normalX = plane->x;
                    normalY = plane->y;
                    normalZ = plane->z;
                    planeDistance = plane->distance;
                    side1 = (arg1[2] * normalZ) +
                              ((normalX * arg1[0]) +
                               (normalY * arg1[1])) + planeDistance;
                    if (side1 < 0.0f) {
                        side0 = (arg0[2] * normalZ) +
                              ((normalX * arg0[0]) +
                               (normalY * arg0[1])) + planeDistance;
                        if (side0 >= 0.0f) {
                            fraction = side0 / (side0 - side1);
                            pointX = (direction.f[0] * fraction) + arg0[0];
                            pointY = (direction.f[1] * fraction) + arg0[1];
                            pointZ = (direction.f[2] * fraction) + arg0[2];
                            inside = 1;
                            for (edgeIndex = 0; (edgeIndex < 3) && (inside != 0); edgeIndex++) {
                                edge = E129_U16(polygon, (edgeIndex + 1) * 2);
                                edgeSign = edge & 0x8000;
                                edgeNumber = edge ^ edgeSign;
                                surfaceBytes = (u8 *) surfaceBase +
                                               (edgeNumber * 0x10);
                                edgeValue =
                                    (E129_F32(surfaceBytes, 0) * pointX) +
                                    (E129_F32(surfaceBytes, 4) * pointY) +
                                    (E129_F32(surfaceBytes, 8) * pointZ) +
                                    E129_F32(surfaceBytes, 0xC);
                                if (edgeSign != 0) {
                                    edgeValue = -edgeValue;
                                }
                                if (edgeValue > 0.0f) {
                                    inside = 0;
                                }
                            }
                            if ((inside != 0) && (fraction < bestDistance)) {
                                bestDistance = fraction;
                                bestX = pointX;
                                bestY = pointY;
                                bestZ = pointZ;
                                bestPlane = plane;
                                bestFlags = batch->flags;
                                bestTexture = E129_U8(
                                    D_800792E8->textures,
                                    (batch->textureIndex * 8) + 7);
                                hit = 1;
                            }
                        }
                    }
                }
            }
            batch++;
        }
    }
    if (hit != 0) {
        E129_S32(arg2, 0) = 0;
        E129_F32(arg2, 4) = bestX;
        E129_F32(arg2, 8) = bestY;
        E129_F32(arg2, 0xC) = bestZ;
        E129_F32(arg2, 0x10) = bestPlane->x;
        E129_F32(arg2, 0x14) = bestPlane->y;
        E129_F32(arg2, 0x18) = bestPlane->z;
        E129_F32(arg2, 0x1C) = bestPlane->distance;
        direction.f[0] *= bestDistance;
        direction.f[1] *= bestDistance;
        direction.f[2] *= bestDistance;
        E129_F32(arg2, 0x20) = sqrtf(
            (direction.f[2] * direction.f[2]) +
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1])));
        E129_S32(arg2, 0x24) = bestFlags;
        E129_S32(arg2, 0x28) = bestTexture;
    } else {
        E129_F32(arg2, 0x20) = sqrtf(
            (direction.f[2] * direction.f[2]) +
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1])));
    }
    return hit;
}
#undef E129_U8
#undef E129_S16
#undef E129_U16
#undef E129_S32
#undef E129_F32
#undef E129_PTR
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001291C.s")
#endif
/*
 * PROVENANCE: Jet Force Gemini's public assembly-only `trackClip3D` in
 * `src/track.c` supplies the six-plane clipping structure and paired helper
 * context. Mickey's shorter function boundary, fields, globals, and body are
 * reconstructed from Mickey-only evidence.
 */
s32 func_800131AC(TrackVec3f *origin, TrackVec3f *direction,
                  TrackVec3f *minimum, TrackVec3f *maximum,
                  f32 *nearClip, f32 *farClip) {
    f32 near;
    f32 far;
    s32 result;

    D_80079350 = 0;
    result = FALSE;
    near = -32000.0f;
    far = 32000.0f;
    if ((func_80013324(direction->f[0],
                       minimum->f[0] - origin->f[0], &near, &far) != 0) &&
        (func_80013324(-direction->f[0],
                       origin->f[0] - maximum->f[0], &near, &far) != 0) &&
        (func_80013324(direction->f[1],
                       minimum->f[1] - origin->f[1], &near, &far) != 0) &&
        (func_80013324(-direction->f[1],
                       origin->f[1] - maximum->f[1], &near, &far) != 0) &&
        (func_80013324(direction->f[2],
                       minimum->f[2] - origin->f[2], &near, &far) != 0) &&
        (func_80013324(-direction->f[2],
                       origin->f[2] - maximum->f[2], &near, &far) != 0)) {
        result = D_80079354;
        *nearClip = near;
        *farClip = far;
    }
    return result;
}
s32 func_80013324(f32 coefficient, f32 numerator,
                  f32 *minimum, f32 *maximum) {
    f32 ratio;

    D_80079350++;
    if (coefficient > 0.0f) {
        ratio = numerator / coefficient;
        if (*maximum < ratio) {
            return FALSE;
        }
        if (*minimum < ratio) {
            *minimum = ratio;
            D_80079354 = D_80079350;
        }
    } else if (coefficient < 0.0f) {
        ratio = numerator / coefficient;
        if (ratio < *minimum) {
            return FALSE;
        }
        if (ratio < *maximum) {
            *maximum = ratio;
        }
    } else if (numerator > 0.0f) {
        return FALSE;
    }
    return TRUE;
}
/* Three-point plane: unit normal and plane distance. Matched (Track B) by
 * replacing the m2c carriers with one local per coordinate, delta and normal
 * component; the deltas are declared before the normal with three
 * unreferenced locals between them (the 0xA0 frame's home layout) and are
 * computed grouped by axis, which gives the target's FP colouring. */
void func_800133FC(TrackVertex *arg0, TrackVertex *arg1,
                   TrackVertex *arg2, f32 *arg3, f32 *arg4,
                   f32 *arg5, f32 *arg6) {
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 x2;
    s32 y2;
    s32 z2;
    f32 dx1;
    f32 dy1;
    f32 dz1;
    f32 dx2;
    f32 dy2;
    f32 dz2;
    s32 pad0;
    s32 pad1;
    s32 pad2;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 mag;

    x0 = arg0->x;
    y0 = arg0->y;
    z0 = arg0->z;
    x1 = arg1->x;
    y1 = arg1->y;
    z1 = arg1->z;
    x2 = arg2->x;
    y2 = arg2->y;
    z2 = arg2->z;
    dx1 = x1 - x0;
    dx2 = x2 - x1;
    dy1 = y1 - y0;
    dy2 = y2 - y1;
    dz1 = z1 - z0;
    dz2 = z2 - z1;
    nx = (dy1 * dz2) - (dz1 * dy2);
    ny = (dz1 * dx2) - (dx1 * dz2);
    nz = (dx1 * dy2) - (dy1 * dx2);
    mag = sqrtf((nx * nx) + (ny * ny) + (nz * nz));
    if (mag > 0.0f) {
        nx /= mag;
        ny /= mag;
        nz /= mag;
    }
    *arg3 = nx;
    *arg4 = ny;
    *arg5 = nz;
    *arg6 = -((x0 * nx) + (y0 * ny) + (z0 * nz));
}
/*
 * PROVENANCE: Mickey-only reconstruction from the target's collision-query
 * callers, resident track layouts, and the neighboring collision helpers;
 * no published donor body is used here.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch; 189 differing words, first mismatch +0x8. */
/* Exact 260-word size and 0x138 frame, 7/8 relocation sites exact under configured flags. */
/* Mickey m2c recovers the compact post-decrement sort and post-call batch flag reload. */
u32 func_8001357C(f32 arg0, f32 arg1, f32 *arg2, s32 arg3, void *arg4) {
    s16 *segmentIndexPointer;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTriangle *triangle;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;
    TrackVertex *vertex3;
    TrackPlane *plane;
    TrackIntersection *intersection;
    s32 x;
    s32 z;
    s32 segmentCount;
    s16 segmentIndices[28];
    s32 segmentNumber;
    s32 batchNumber;
    s32 vertexIndex;
    s32 compareMask;
    u32 batchFlags;
    u32 resultCount;
    u32 visibility;
    f32 height;
    TrackPlane computedPlane;
    s32 outer;
    s32 inner;
    TrackIntersection *record;
    f32 temporaryHeight;
    s32 temporaryFlags;

    x = (s32) arg0;
    z = (s32) arg1;
    segmentCount = func_8000FCA4(x, z, &segmentIndices[0]);
    resultCount = 0;
    if (arg2 != NULL) {
        *arg2 = -32768.0f;
    }
    segmentNumber = 0;
    if (segmentCount > 0) {
        segmentIndexPointer = &segmentIndices[0];
        do {
            compareMask = getXZCompareMask(
                &D_800792E8->segmentBounds[*segmentIndexPointer], x, z, x,
                z);
            segment = &D_800792E8->segments[*segmentIndexPointer];
            segmentNumber++;
            segmentIndexPointer++;
            batch = segment->batches;
            batchNumber = segment->batchCount;
            batchNumber--;
            if (segment->batchCount != 0) {
                do {
                    batchFlags = batch->flags;
                    if (batchFlags & arg3) {
                        vertexIndex = batch->v0;
                        triangle =
                            (TrackTriangle *) segment->vertexData + batch->v0;
                        vertex0 =
                            (TrackVertex *) segment->lightData + batch->u0;
                        if (vertexIndex < batch[1].v0) {
                            do {
                                visibility = segment->visibilityMasks[vertexIndex];
                                visibility &= compareMask;
                                if ((visibility >> 16) != 0 &&
                                    (visibility & 0xFFFF) != 0) {
                                    vertex1 = vertex0 + triangle->vertex0;
                                    vertex2 = vertex0 + triangle->vertex1;
                                    vertex3 = vertex0 + triangle->vertex2;
                                    if (mathXZInTri(x, z, vertex1, vertex2,
                                                    vertex3) != 0) {
                                        height = (f32) vertex1->y;
                                        if (vertex1->y != vertex2->y ||
                                            vertex1->y != vertex3->y) {
                                            if (batch->flags & 0x1080) {
                                                func_800133FC(
                                                    vertex1, vertex2, vertex3,
                                                    &computedPlane.x, &computedPlane.y,
                                                    &computedPlane.z,
                                                    &computedPlane.distance);
                                                plane = &computedPlane;
                                            } else {
                                                plane = segment->surfaces +
                                                    (segment->surfaceIndices[
                                                         vertexIndex * 4] *
                                                     1);
                                            }
                                            if (plane->y > 0.0f) {
                                                height = -(((plane->x * arg0) +
                                                             (plane->z * arg1) +
                                                             plane->distance) /
                                                            plane->y);
                                            }
                                        }
                                        if (arg4 != NULL) {
                                            if (resultCount >= 8U) {
                                                resultCount = 7;
                                            }
                                            intersection =
                                                (TrackIntersection *) arg4 +
                                                resultCount;
                                            intersection->height = height;
                                            resultCount++;
                                            intersection->flags = batchFlags;
                                        } else {
                                            *arg2 = height;
                                            return batchFlags;
                                        }
                                    }
                                }
                                vertexIndex++;
                                triangle++;
                            } while (vertexIndex < batch[1].v0);
                        }
                    }
                    batch++;
                } while (batchNumber-- != 0);
            }
        } while (segmentNumber != segmentCount);
    }
    if (resultCount >= 2U) {
        outer = resultCount - 2;
        if (resultCount - 1 != 0) {
            do {
                record = (TrackIntersection *) arg4;
                inner = outer;
                if (outer + 1 != 0) {
                    do {
                        if (record->height < (record + 1)->height) {
                            temporaryHeight = record->height;
                            temporaryFlags = (record + 1)->flags;
                            record->height = (record + 1)->height;
                            (record + 1)->height = temporaryHeight;
                            (record + 1)->flags = record->flags;
                            record->flags = temporaryFlags;
                        }
                        record++;
                    } while (inner-- != 0);
                }
            } while (outer-- != 0);
        }
    }
    return resultCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001357C.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: JFG's public track.c retains this collision collector as
 * assembly; Mickey's segment, batch, plane and hit-list accesses are used. */
/* Workbench verdict: structure-mismatch, 163 differing words; first mismatch is at +0x60. */
/* Target and candidate are both 330 instructions with frame -320 and 21 relocations. */
/* Remaining gap is allocator scheduling; 19 relocation identities align. */
s32 func_8001398C(f32 arg0, f32 arg1, s32 arg2, void **arg3) {
    typedef struct TrackCollisionHit {
        f32 height;
        TrackPlane *surface;
        u32 flags;
        s8 textureFlag;
        u8 pad0D[3];
    } TrackCollisionHit;

    s16 segmentIndices[32];
    s32 segmentCount;
    s32 x;
    s32 z;
    s32 segmentNumber;
    s32 segmentIndex;
    s32 batchNumber;
    s32 triangleIndex;
    s32 compareMask;
    u32 batchFlags;
    s8 textureFlag;
    s32 resultCount;
    s32 orderIndex;
    s32 orderCount;
    s32 changed;
    s32 firstTriangle;
    s32 lastTriangle;
    s16 textureOffset;
    f32 planeHeight;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTriangle *triangle;
    TrackPlane *surface;
    TrackCollisionHit *hit;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;
    void *temporary;

    x = (s32) arg0;
    z = (s32) arg1;
    *arg3 = NULL;
    segmentCount = func_8000FCA4(x, z, segmentIndices);
    if ((segmentCount == 0) || (segmentCount >= 0x20)) {
        return 0;
    }
    arg2 |= 0x1080;
    resultCount = 0;
    segmentNumber = 0;
    if (segmentCount > 0) {
        do {
        segmentIndex = segmentIndices[segmentNumber];
        segment = &D_800792E8->segments[segmentIndex];
        compareMask = getXZCompareMask(
            &D_800792E8->segmentBounds[segmentIndex], x, z, x, z);
        batchNumber = 0;
        if (segment->batchCount > 0) {
            do {
                batch = &segment->batches[batchNumber];
                batchFlags = batch->flags;
                firstTriangle = batch->v0;
                textureOffset = batch->u0;
                lastTriangle = batch[1].v0;
                if (batchFlags & 0x10000) {
                    textureFlag = 1;
                } else {
                    textureFlag = ((s8 *)
                        &D_800792E8->textures[batch->textureIndex])[7];
                }
                if (batchFlags & arg2) {
                    firstTriangle = lastTriangle;
                }
                triangleIndex = firstTriangle;
                if (firstTriangle < lastTriangle) {
                    do {
                        u32 visibility = segment->visibilityMasks[triangleIndex];
                        visibility &= compareMask;
                        if ((visibility >> 16) != 0 &&
                            (visibility & 0xFFFF) != 0) {
                            surface = &segment->surfaces[
                                *(u16 *) ((u8 *) segment->surfaceIndices +
                                          (triangleIndex * 8))];
                            planeHeight = surface->y;
                            if (planeHeight > 0.0f) {
                                triangle = (TrackTriangle *)
                                    ((u8 *) segment->vertexData +
                                     (triangleIndex * 0x10));
                                vertex0 = (TrackVertex *)
                                    ((u8 *) segment->lightData +
                                     ((triangle->vertex0 + textureOffset) * 0xA));
                                vertex1 = (TrackVertex *)
                                    ((u8 *) segment->lightData +
                                     ((triangle->vertex1 + textureOffset) * 0xA));
                                vertex2 = (TrackVertex *)
                                    ((u8 *) segment->lightData +
                                     ((triangle->vertex2 + textureOffset) * 0xA));
                                if (mathXZInTri(x, z, vertex0, vertex1,
                                                vertex2) != 0) {
                                    hit = (TrackCollisionHit *) D_800C9B90 +
                                          resultCount;
                                    hit->surface = surface;
                                    resultCount++;
                                    hit->height = -(((surface->x * arg0) +
                                                     (surface->z * arg1) +
                                                     surface->distance) /
                                                    planeHeight);
                                    hit->textureFlag = textureFlag;
                                    hit->flags = segment->batches[batchNumber].flags;
                                    if (resultCount >= 0x14) {
                                        triangleIndex = lastTriangle;
                                        batchNumber = segment->batchCount;
                                        segmentNumber = segmentCount;
                                    }
                                }
                            }
                        }
                        triangleIndex++;
                    } while (triangleIndex < lastTriangle);
                }
                batchNumber++;
            } while (batchNumber < segment->batchCount);
        }
        segmentNumber++;
        } while (segmentNumber < segmentCount);
    }
    hit = (TrackCollisionHit *) D_800C9B90;
    orderIndex = 0;
    if (resultCount > 0) {
        do {
            D_800C9CD0[orderIndex] = &hit[orderIndex];
            orderIndex++;
        } while (orderIndex != resultCount);
    }
    do {
        orderCount = resultCount - 1;
        changed = 1;
        if (orderCount > 0) {
            orderIndex = 0;
            do {
                if (*(f32 *) D_800C9CD0[orderIndex] <
                    *(f32 *) D_800C9CD0[orderIndex + 1]) {
                    temporary = D_800C9CD0[orderIndex];
                    D_800C9CD0[orderIndex] = D_800C9CD0[orderIndex + 1];
                    D_800C9CD0[orderIndex + 1] = temporary;
                    changed = 0;
                }
                orderIndex++;
            } while (orderIndex < (resultCount - 1));
        }
    } while (changed == 0);
    *arg3 = D_800C9CD0;
    return resultCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_8001398C.s")
#endif
/*
 * PROVENANCE: JFG supplies the name `trackGetTrack`; this trivial body is
 * reconstructed from Mickey.
 */
void *trackGetTrack(void) {
    return D_800792E8;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackFreeAll`, whose assembly-only body supplies the teardown order and
 * loop structure. Mickey's pointers, calls, and object layouts are
 * authoritative; the donor name is not adopted.
 */
void func_80013EC0(void) {
    TrackSegment *segment;
    TrackData *track;
    TrackData **trackSlot;
    s32 index;
    s32 offset;

    trackSlot = &D_800792E8;
    if (D_80079278 > 0) {
        TrapDanglingJump();
        D_80079278 = 0;
    }
    func_8000D570();
    if (D_80079310 != NULL) {
        mmFree(D_80079310);
        D_80079310 = NULL;
        D_8007930C = 0;
    }
    func_8001F364();
    if (D_800792F0 != NULL) {
        func_800347A0(D_800792F0);
        D_800792F0 = NULL;
    }

    track = *trackSlot;
    index = 0;
    if (track->segmentCount > 0) {
        offset = 0;
        do {
            segment = (TrackSegment *) ((u8 *) track->segments + offset);
            if (segment->unk30 != NULL) {
                mmFree(segment->unk30);
                track = *trackSlot;
                segment = (TrackSegment *)
                    ((u8 *) track->segments + offset);
            }
            if (segment->unk38 != NULL) {
                TrapDanglingJump(segment->unk38);
                track = *trackSlot;
            }
            index++;
            offset += sizeof(TrackSegment);
        } while (index < track->segmentCount);
        index = 0;
    }

    if (track->textureCount > 0) {
        offset = 0;
        do {
            func_800347A0(((TrackTextureEntry *)
                ((u8 *) track->textures + offset))->texture);
            track = *trackSlot;
            index++;
            offset += sizeof(TrackTextureEntry);
        } while (index < track->textureCount);
    }

    mmFree(D_800C95A8);
    mmFree(D_800C9D2C);
    mmFree(D_800C9D30);
    mmFree(D_800C9D34);
    if (TrapDanglingJump(osRomBase) != 0) {
        mmFree(D_800C9D20);
    }
    shadowFreeBuffers();
    if (D_800C9550 != NULL) {
        func_80006EA0(D_800C9550);
        func_80006FA0();
    }
    animseqFreeLevelData();
    func_8000439C();
    D_800792E8 = NULL;
    if (D_800C9548 != NULL) {
        mmFree(D_800C9548);
        D_800C9548 = NULL;
    }
    D_80079274 = 0;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c display-list draft and resident shadow-buffer and
 * command offsets reconstruct this renderer; no external function body is adapted.
 */
typedef struct TrackShadowObject {
    u8 pad00[0x39];
    u8 alpha;
    u8 pad3A[0x0A];
    s16 kind;
    u8 pad46[0x1E];
    void *material;
} TrackShadowObject;

typedef struct TrackShadowInstance {
    u8 pad00[0x0C];
    u16 textureScale;
    u8 pad0E[2];
    u8 active;
    u8 pad11[2];
    u8 count;
    s16 shadowIndex;
    u8 pad16[2];
    s16 endIndex;
} TrackShadowInstance;

/* Adjacent eight-byte descriptors supply the next index and vertex boundaries. */
typedef struct TrackShadowBatch {
    void *texture;
    s16 firstIndex;
    s16 firstVertex;
} TrackShadowBatch;

typedef struct TrackShadowMaterial {
    u8 pad00[0x18];
    u8 red;
    u8 green;
    u8 blue;
} TrackShadowMaterial;

/* Workbench verdict: structure-mismatch, 187 differing words, first mismatch +0x0. */
/* Candidate: 217/217 instructions with a -0x90 frame versus target -0xA8; 3/4 relocation placements align. */
/* Shape status: alpha branches, 8-byte shadow stepping, geometry commands, and FA/FB cleanup writes are preserved, but it is not shape-exact. */
void func_800140CC(TrackShadowObject *object, TrackShadowInstance *instance) {
    s32 loopIndex;
    s32 closeTexture;
    s32 closeCombiner;
    void *commandBuffer;
    void *indexBuffer;
    void *vertexBuffer;
    s32 shadowCount;
    s32 alphaValue;
    s32 commandMode;
    s32 textureSpan;
    s32 indexSpan;
    u32 vertexAddress;
    u32 indexAddress;
    s16 shadowIndex;
    TrackShadowInstance *current;
    TrackShadowBatch *shadow;
    u8 active;
    TrackShadowMaterial *material;
    Gfx *command;

    active = instance->active;
    if (active != 0) {
        shadowGetBuffers(active, &vertexBuffer, &indexBuffer,
                         &commandBuffer);
        loopIndex = 0;
        current = instance;
        if (current->count > 0) {
            do {
                shadowIndex = current->shadowIndex;
                if (shadowIndex != -1) {
                    shadow = (TrackShadowBatch *) commandBuffer + shadowIndex;
                    shadowCount = (s32) object->alpha *
                                  *(u8 *) ((u8 *) vertexBuffer +
                                  (shadow->firstVertex * 0x0A) + 9);
                    shadowCount >>= 8;
                    if (shadowCount > 0) {
                        commandMode = 0x0E;
                        if (object->kind == 0x3C) {
                            command = D_800C9520;
                            material = object->material;
                            D_800C9520 = command + 1;
                            command->words.w1 = (shadowCount & 0xFF) | ~0xFF;
                            command->words.w0 = 0xFA000000;
                            command = D_800C9520;
                            commandMode = 0x20E;
                            D_800C9520 = command + 1;
                            command->words.w0 = 0xFB000000;
                            command->words.w1 = ((u32) material->red << 24) |
                                                ((u32) material->green << 16) |
                                                ((u32) material->blue << 8);
                            closeTexture = 1;
                            closeCombiner = 1;
                        } else {
                            closeCombiner = 0;
                            if ((object->kind == 0x35) ||
                                (object->kind == 0x58)) {
                                command = D_800C9520;
                                D_800C9520 = command + 1;
                                command->words.w0 = 0xFA000000;
                                command->words.w1 = (shadowCount & 0xFF) | ~0xFF;
                                closeTexture = shadowCount != 0xFF;
                            } else {
                                command = D_800C9520;
                                D_800C9520 = command + 1;
                                command->words.w1 = shadowCount & 0xFF;
                                command->words.w0 = 0xFA000000;
                                closeTexture = 1;
                            }
                        }
                        shadowIndex = current->shadowIndex;
                        while (shadowIndex < current->endIndex) {
                            func_800349A4(&D_800C9520, shadow->texture,
                                          commandMode,
                                          instance->textureScale << 8);
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            textureSpan = shadow[1].firstVertex -
                                          shadow->firstVertex;
                            vertexAddress = (u32) vertexBuffer +
                                            (shadow->firstVertex * 10) +
                                            0x80000000U;
                            command->words.w0 = (((((textureSpan * 8) |
                                                   (vertexAddress & 6)) & 0xFF) << 16) |
                                                 0x04000000 |
                                                 ((textureSpan * 10 + 8) & 0xFFFF));
                            command->words.w1 = vertexAddress;
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            indexSpan = shadow[1].firstIndex -
                                        shadow->firstIndex;
                            indexAddress = (shadow->firstIndex * 16) +
                                           (u32) indexBuffer + 0x80000000U;
                            command->words.w1 = indexAddress;
                            command->words.w0 = ((((((indexSpan - 1) * 16) |
                                                   1) & 0xFF) << 16) |
                                                 0x05000000 |
                                                 ((indexSpan * 16) & 0xFFFF));
                            shadowIndex++;
                            shadow++;
                        }
                        if (closeTexture != 0) {
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            command->words.w1 = -1;
                            command->words.w0 = 0xFA000000;
                        }
                        if (closeCombiner != 0) {
                            command = D_800C9520;
                            D_800C9520 = command + 1;
                            command->words.w1 = -0x100;
                            command->words.w0 = 0xFB000000;
                        }
                    }
                }
                current = (TrackShadowInstance *) ((u8 *) current + 2);
                loopIndex++;
            } while (loopIndex < instance->count);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/track/func_800140CC.s")
#endif
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackSetFog`. Mickey's function boundary and fog-data accesses are
 * authoritative where the revisions differ.
 */
void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                 u8 red, u8 green, u8 blue, s8 state) {
    s32 tempNear;
    TrackFog *fogData;

    fogData = &D_800C99C0[fogIndex];

    if (far < near) {
        tempNear = near;
        near = far;
        far = tempNear;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->addFog.near = 0;
    fogData->addFog.far = 0;
    fogData->addFog.r = 0;
    fogData->addFog.g = 0;
    fogData->addFog.b = 0;
    fogData->fog.r = red << 16;
    fogData->fog.g = green << 16;
    fogData->fog.b = blue << 16;
    fogData->fog.near = near << 16;
    fogData->fog.far = far << 16;
    fogData->initialNear = near << 16;
    fogData->targetNear = targetNear << 16;
    fogData->intendedFog.state = state;
    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;
    fogData->switchTimer = 0;
    fogData->fogChanger = NULL;
    fogData->intendedFog.b = blue;
}
/*
 * PROVENANCE: adapted from the direct fog-data path in Jet Force Gemini's
 * public `src/track.c`, function `trackGetFog`. Mickey omits JFG's overlay
 * special cases; Mickey's function boundary and accesses are authoritative.
 */
void trackGetFog(s32 playerID, s16 *near, s16 *far, s16 *targetNear,
                 u8 *red, u8 *green, u8 *blue, s8 *state) {
    TrackFog *fogData;

    fogData = &D_800C99C0[playerID];
    *near = fogData->fog.near >> 16;
    *far = fogData->fog.far >> 16;
    *targetNear = fogData->targetNear >> 16;
    *red = fogData->fog.r >> 16;
    *green = fogData->fog.g >> 16;
    *blue = fogData->fog.b >> 16;
    *state = fogData->intendedFog.state & 0x7F;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackSetFogOff`. Mickey's tier-A match to JFG's built function and the
 * offsets used below independently validate this body against Mickey's ROM.
 */
void trackSetFogOff(s32 fogIndex) {
    D_800C99C0[fogIndex].addFog.near = 0;
    D_800C99C0[fogIndex].addFog.far = 0;
    D_800C99C0[fogIndex].addFog.r = 0;
    D_800C99C0[fogIndex].addFog.g = 0;
    D_800C99C0[fogIndex].addFog.b = 0;
    D_800C99C0[fogIndex].fog.near = 1018 << 16;
    D_800C99C0[fogIndex].fog.far = 1023 << 16;
    D_800C99C0[fogIndex].intendedFog.r = D_800C99C0[fogIndex].fog.r >> 16;
    D_800C99C0[fogIndex].intendedFog.g = D_800C99C0[fogIndex].fog.g >> 16;
    D_800C99C0[fogIndex].intendedFog.b = D_800C99C0[fogIndex].fog.b >> 16;
    D_800C99C0[fogIndex].intendedFog.near = 1018;
    D_800C99C0[fogIndex].intendedFog.far = 1023;
    D_800C99C0[fogIndex].switchTimer = 0;
    D_800C99C0[fogIndex].fogChanger = NULL;
}
/*
 * PROVENANCE: JFG's corresponding track.c placeholder supplies strong
 * skeleton and TU-position context. The body is reconstructed from Mickey's
 * TrackFog accesses and control flow; no JFG placeholder name is imported.
 */
void func_80014614(s32 fogCount, s32 updateRate) {
    TrackFog *fogData;
    s32 fogIndex;
    s8 state;

    fogIndex = 0;
    if (fogCount > 0) {
        fogData = D_800C99C0;
        do {
            state = fogData->intendedFog.state;
            fogIndex++;
            if (state > 0) {
                fogData->fog.near +=
                    (updateRate * (state & 0x7F)) << 11;
                if (fogData->targetNear < fogData->fog.near) {
                    fogData->fog.near =
                        (fogData->targetNear - fogData->fog.near) +
                        fogData->targetNear;
                    fogData->intendedFog.state = state | 0x80;
                }
            } else if (state < 0) {
                fogData->fog.near -=
                    (updateRate * (state & 0x7F)) << 11;
                if (fogData->fog.near < fogData->initialNear) {
                    fogData->fog.near =
                        (fogData->initialNear - fogData->fog.near) +
                        fogData->initialNear;
                    fogData->intendedFog.state = state ^ 0x80;
                }
            } else {
                if (fogData->switchTimer > 0) {
                    if (updateRate < fogData->switchTimer) {
                        fogData->fog.r += fogData->addFog.r * updateRate;
                        fogData->fog.g += fogData->addFog.g * updateRate;
                        fogData->fog.b += fogData->addFog.b * updateRate;
                        fogData->fog.near += fogData->addFog.near * updateRate;
                        fogData->fog.far += fogData->addFog.far * updateRate;
                        /* The volatile cast forces IDO to re-load switchTimer
                         * from memory immediately before the subtraction,
                         * instead of reusing the value it already has in a
                         * register from the `switchTimer > 0` and
                         * `updateRate < switchTimer` comparisons above. That
                         * reload is what the target's instruction schedule
                         * requires; without it the match breaks. */
                        fogData->switchTimer =
                            *(volatile s32 *)&fogData->switchTimer - updateRate;
                    } else {
                        fogData->fog.r = fogData->intendedFog.r << 16;
                        fogData->fog.g = fogData->intendedFog.g << 16;
                        fogData->fog.b = fogData->intendedFog.b << 16;
                        fogData->fog.near = fogData->intendedFog.near << 16;
                        fogData->fog.far = fogData->intendedFog.far << 16;
                        fogData->switchTimer = 0;
                    }
                }
            }
            fogData++;
        } while (fogIndex != fogCount);
    }
}
/*
 * PROVENANCE: JFG's corresponding track.c function supplies tier-D position
 * and structural context. The body is reconstructed from Mickey's display-
 * list writes and its call to trackGetFog; JFG's placeholder is not imported.
 */
void func_800147A4(s32 playerID) {
    s16 near;
    s16 far;
    s16 targetNear;
    u8 red;
    u8 green;
    u8 blue;
    s8 state;

    trackGetFog(playerID, &near, &far, &targetNear, &red, &green, &blue,
                &state);
    gDPSetFogColor(D_800C9520++, red, green, blue, 0xFF);
    gSPFogPosition(D_800C9520++, near, far);
}
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public `src/tracks.c`,
 * `obj_loop_fogchanger`; JFG's assembly-only `trackChangeFog` independently
 * supplies the TU position. Mickey's object fields, direct player-list call,
 * fallback stride, radius offset, and fog layout are authoritative.
 */
void func_800148E0(TrackFogChanger *changer) {
    s32 nearTemp;
    s32 fogNear;
    s32 views;
    s32 playerIndex;
    s32 index;
    s32 pad;
    s32 fogFar;
    s32 i;
    s32 fogR;
    s32 fogG;
    s32 fogB;
    f32 x;
    f32 z;
    s32 switchTimer;
    TrackFogChangerData *fogChanger;
    TrackFogPlayer **racers;
    TrackFogPlayerState *racer;
    s32 pad2;
    TrackFog *fogData;
    TrackFallbackPlayer *camera;

    racers = NULL;
    fogChanger = changer->data;
    camera = NULL;
    racers = func_80005750(&views);

    i = 0;
    if (views > 0) {
        do {
            index = -1;
            if (racers != NULL) {
                racer = racers[i]->state;
                playerIndex = racer->fogIndex;
                if ((playerIndex >= 0) && (playerIndex < 4) &&
                    (changer != D_800C99C0[playerIndex].fogChanger)) {
                    index = playerIndex;
                    x = racers[i]->x;
                    z = racers[i]->z;
                }
            } else if ((i < 4) &&
                       (changer != D_800C99C0[i].fogChanger)) {
                index = i;
                x = camera[i].x;
                z = camera[i].z;
            }

            if (index != -1) {
                x -= changer->x;
                z -= changer->z;
                /* Inert allocation aid retained by exact C; tracked in
                 * docs/cleanup-queue.md. */
                if (1) {
                }
                if (((x * x) + (z * z)) <
                    changer->radiusSquared) {
                    fogNear = fogChanger->near;
                    fogFar = fogChanger->far;
                    fogR = fogChanger->red;
                    fogG = fogChanger->green;
                    fogB = fogChanger->blue;
                    switchTimer = fogChanger->duration;
                    if (fogFar < fogNear) {
                        nearTemp = fogNear;
                        fogNear = fogFar;
                        fogFar = nearTemp;
                    }
                    if (fogFar > 1023) {
                        fogFar = 1023;
                    }
                    if (fogNear >= fogFar - 5) {
                        fogNear = fogFar - 5;
                    }

                    fogData = &D_800C99C0[index];
                    fogData->intendedFog.r = fogR;
                    fogData->intendedFog.g = fogG;
                    fogData->intendedFog.b = fogB;
                    fogData->intendedFog.near = fogNear;
                    fogData->intendedFog.far = fogFar;
                    fogData->addFog.r =
                        ((fogR << 16) - fogData->fog.r) / switchTimer;
                    fogData->addFog.g =
                        ((fogG << 16) - fogData->fog.g) / switchTimer;
                    fogData->addFog.b =
                        ((fogB << 16) - fogData->fog.b) / switchTimer;
                    fogData->addFog.near =
                        ((fogNear << 16) - fogData->fog.near) / switchTimer;
                    fogData->addFog.far =
                        ((fogFar << 16) - fogData->fog.far) / switchTimer;
                    fogData->switchTimer = switchTimer;
                    fogData->fogChanger = changer;
                }
            }
            i++;
        } while (i != views);
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public `src/track.c`, function
 * `trackFadeFog`. Mickey's argument width and direct fog-data path are
 * authoritative where the revisions differ; JFG's name is not adopted.
 */
void func_80014BAC(s32 fogIndex, s32 red, s32 green, s32 blue, s32 near,
                   s32 far, f32 timer) {
    s32 temp;
    s32 switchTimer;
    TrackFog *fogData;

    fogData = &D_800C99C0[fogIndex];

    if (osTvType == 0) {
        switchTimer = timer * 50.0f;
    } else {
        switchTimer = timer * 60.0f;
    }

    if (far < near) {
        temp = near;
        near = far;
        far = temp;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.b = blue;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;

    if (switchTimer > 0) {
        fogData->switchTimer = switchTimer;
        fogData->addFog.r = ((red << 16) - fogData->fog.r) / switchTimer;
        fogData->addFog.g = ((green << 16) - fogData->fog.g) / switchTimer;
        fogData->addFog.b = ((blue << 16) - fogData->fog.b) / switchTimer;
        fogData->addFog.near = ((near << 16) - fogData->fog.near) / switchTimer;
        fogData->addFog.far = ((far << 16) - fogData->fog.far) / switchTimer;
    } else {
        fogData->switchTimer = 0;
        fogData->fog.r = red << 16;
        fogData->fog.g = green << 16;
        fogData->fog.b = blue << 16;
        fogData->fog.near = near << 16;
        fogData->fog.far = far << 16;
    }
    fogData->fogChanger = NULL;
}
/*
 * PROVENANCE: JFG's corresponding track.c position identifies the transform
 * role. This body and its local layout are reconstructed from Mickey's own
 * function bytes and call signatures.
 */
void func_80014DE4(void) {
    MtxF matrix;
    TrackLocalTransform transform;
    f32 x;
    f32 y;
    f32 z;

    x = 0.0f;
    y = 0.0f;
    z = -65536.0f;
    transform.zRotation = D_800C9530->rotationZ;
    transform.yRotation = D_800C9530->rotationY;
    transform.xRotation = D_800C9530->rotationX;
    transform.x = 0.0f;
    transform.y = 0.0f;
    transform.z = 0.0f;
    func_8002AB78(&transform, matrix);
    mtxf_transform_point(matrix, x, y, z, &x, &y, &z);
    D_800C9B40.x = (s32)x;
    D_800C9B40.y = (s32)y;
    D_800C9B40.z = (s32)z;
}
s32 func_80014EAC(u32 value) {
    s32 result;

    result = -1;
    while (value != 0) {
        result++;
        value >>= 1;
    }
    return result;
}
/*
 * PROVENANCE: Jet Force Gemini's public built `src/track.c.o` and its
 * assembly-only final source entry establish the tier-D TU position and
 * display-list-helper structure. The body is reconstructed from Mickey with
 * the SDK GBI macros; JFG's placeholder name is not imported.
 */
void func_80014ECC(TrackTextureHeader *texture, s32 frame, s32 flags) {
    TrackTextureLoadLocals locals;
    TrackTextureHeader *activeTexture;
    s32 activeFrame;
    s32 activeMaskT;
    s32 intensity;
    s32 shiftS;
    s32 shiftT;

    locals.textureAddress = func_800348D4(texture, frame);
    if (texture->unk1B >= 2) {
        D_800C9520->words.w0 = texture->displayList->words.w0;
        D_800C9520->words.w1 = (u32) locals.textureAddress;
        D_800C9520++;
        gSPDisplayList(D_800C9520++, texture->displayList + 1);
        gSPDisplayList(D_800C9520++, D_800793D8);
        return;
    }

    locals.maskS = func_80014EAC(texture->width);
    locals.maskT = func_80014EAC(texture->height);
    activeTexture = D_800792F0;
    if (activeTexture != NULL) {
        activeFrame = D_800792F4;
        locals.useOriginalTexture = FALSE;
    } else {
        activeTexture = texture;
        activeFrame = (frame >> 8) + 0x100;
        if (activeFrame >= texture->numOfTextures) {
            activeFrame -= texture->numOfTextures;
        }
        activeFrame <<= 8;
        locals.useOriginalTexture = TRUE;
    }

    locals.activeTextureAddress = func_800348D4(activeTexture, activeFrame);
    locals.activeMaskS = func_80014EAC(activeTexture->width);
    activeMaskT = func_80014EAC(activeTexture->height);
    shiftS = (locals.maskS - locals.activeMaskS) & 0xF;
    shiftT = (locals.maskT - activeMaskT) & 0xF;
    gDPLoadMultiBlockS(D_800C9520++, locals.activeTextureAddress, 0x100, 1,
                       G_IM_FMT_IA, G_IM_SIZ_8b, activeTexture->width,
                       activeTexture->height, 0,
                       G_TX_NOMIRROR | G_TX_WRAP,
                       G_TX_NOMIRROR | G_TX_WRAP, locals.activeMaskS,
                       activeMaskT,
                       shiftS, shiftT);

    if (!locals.useOriginalTexture) {
        gDPLoadMultiBlockS(D_800C9520++, locals.textureAddress, 0, 0,
                           G_IM_FMT_RGBA, G_IM_SIZ_16b, texture->width,
                           texture->height, 0,
                           G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, locals.maskS,
                           locals.maskT,
                           G_TX_NOLOD, G_TX_NOLOD);
        gSPDisplayList(D_800C9520++, D_80079358);
        return;
    }

    gDPLoadTextureBlockS(D_800C9520++, locals.textureAddress, G_IM_FMT_IA,
                         G_IM_SIZ_8b, texture->width, texture->height, 0,
                         G_TX_NOMIRROR | G_TX_WRAP,
                         G_TX_NOMIRROR | G_TX_WRAP, locals.maskS,
                         locals.maskT,
                         G_TX_NOLOD, G_TX_NOLOD);
    if ((flags & 0x70) == 0x10) {
        gSPDisplayList(D_800C9520++, D_800793A8);
    } else {
        gSPDisplayList(D_800C9520++, D_80079380);
    }
    intensity = (frame >> 8) & 0xFF;
    gDPSetEnvColor(D_800C9520++, intensity, intensity, intensity, intensity);
}

/* PLATEAU-HANDOFF:func_8000FAE0:start
 * symbol: func_8000FAE0
 * score: 16/62 words
 * frame: 0x10
 * relocations: 2
 * first-mismatch: +0x1C
 * summary: 10900 subscript did not transfer. Combined p2 force still 2 (keepGoing t9 vs at). Dummy xLower 18; delete count 33.
 * PLATEAU-HANDOFF:func_8000FAE0:end
 */

/* PLATEAU-HANDOFF:func_800140CC:start
 * symbol: func_800140CC
 * score: 187 differing words
 * frame: 0x90
 * relocations: 4
 * first-mismatch: +0x0
 * summary: Recovered shadow buffer ABI and eight-byte records; 187 differences remain. Next: source evidence for flag and pointer stack homes.
 * PLATEAU-HANDOFF:func_800140CC:end
 */

/* PLATEAU-HANDOFF:func_8000F198:start
 * symbol: func_8000F198
 * score: 185 differing words
 * frame: 0x58
 * relocations: 21
 * first-mismatch: +0x0
 * summary: Unsigned batch flag types preserve 185 differences and exact249-word size; m2c adds no missing CFG. Next: counter and flag home evidence.
 * PLATEAU-HANDOFF:func_8000F198:end
 */

/* PLATEAU-HANDOFF:func_80011980:start
 * symbol: func_80011980
 * score: 195 differing words
 * frame: 0xd8
 * relocations: 12
 * first-mismatch: +0x0
 * summary: Second metadata add is the missing-CSE word and the target has it too. Textures local and volatile threshold stay at 195, size +4; ra still holds D_80081790.
 * PLATEAU-HANDOFF:func_80011980:end
 */


/* PLATEAU-HANDOFF:func_80010654:start
 * symbol: func_80010654
 * score: 162 differing words
 * frame: 0x70
 * relocations: 8
 * first-mismatch: +0x0
 * summary: Hoisting the hit point before the edge loop closes size +4 to 0 at 162 words. Frame stays 0x70 versus 0x98: dead edge-copy and hoisted D_80081774.
 * PLATEAU-HANDOFF:func_80010654:end
 */

/* PLATEAU-HANDOFF:func_800115E4:start
 * symbol: func_800115E4
 * score: 217 differing words
 * frame: 0xa0
 * relocations: 17
 * first-mismatch: +0x0
 * summary: Mickey flag and negation order improve 231 to 217 differences, 6/17 relocation sites exact. Next: source-attributed FP home evidence.
 * PLATEAU-HANDOFF:func_800115E4:end
 */

/* PLATEAU-HANDOFF:func_800103D4:start
 * symbol: func_800103D4
 * score: 158/160 words
 * frame: 0x50
 * relocations: 12
 * first-mismatch: +0x0
 * summary: Scoped plane FP lifetimes lower the candidate frame from 0x60 to 0x50; accepted web 163 force to c32 scores 95 but source route remains unproved.
 * PLATEAU-HANDOFF:func_800103D4:end
 */

/* PLATEAU-HANDOFF:func_8001357C:start
 * symbol: func_8001357C
 * score: 189 differing words
 * frame: 0x138
 * relocations: 8
 * first-mismatch: +0x8
 * summary: Mickey m2c sort and flag reload recover exact260-word size/frame and improve 289 to189 differences. Next: early call/home lifetime evidence.
 * PLATEAU-HANDOFF:func_8001357C:end
 */

/* PLATEAU-HANDOFF:func_8000DFBC:start
 * symbol: func_8000DFBC
 * score: 303 differing words
 * frame: 0x70
 * relocations: 51
 * first-mismatch: +0x48
 * summary: Declaring batchIndex and groupIndex first improves 304 to 303. Size stays +8, frame 0x70, first +0x48. Tail call spills remain.
 * PLATEAU-HANDOFF:func_8000DFBC:end
 */

/* PLATEAU-HANDOFF:func_8000E5EC:start
 * symbol: func_8000E5EC
 * score: 185/205 words
 * frame: 0xE8
 * relocations: 56
 * first-mismatch: +0x0
 * summary: Scoped-carrier forms are inert; direct visibility access changes the 31-draw schedule but regresses to 187 words. The 0x10 frame deficit remains.
 * PLATEAU-HANDOFF:func_8000E5EC:end
 */

/* PLATEAU-HANDOFF:func_80011CDC:start
 * symbol: func_80011CDC
 * score: 329 differing words
 * frame: 0xc8
 * relocations: 11
 * first-mismatch: +0x0
 * summary: Aligned 243 to 193. One p1 decision left: D_800792E8 address web outranks the record counter; forcing it split gives 342/342 words, 150 diff.
 * PLATEAU-HANDOFF:func_80011CDC:end
 */

/* PLATEAU-HANDOFF:func_8001398C:start
 * symbol: func_8001398C
 * score: 163 differing words
 * frame: 0x140
 * relocations: 21
 * first-mismatch: +0x60
 * summary: Mickey m2c audit retains defined flag/material types; hoisted sorting bound suppresses required unrolling. Next: source-proved query lifetimes.
 * PLATEAU-HANDOFF:func_8001398C:end
 */

/* PLATEAU-HANDOFF:func_8000E920:start
 * symbol: func_8000E920
 * score: 459 differing words
 * frame: 0x100
 * relocations: 112
 * first-mismatch: +0x0
 * summary: Mickey m2c fixes visibility stride and shadow ABI, then improves 491 to 459 diffs. Next: source-proved frame and reverse-pass lifetimes.
 * PLATEAU-HANDOFF:func_8000E920:end
 */

/* PLATEAU-HANDOFF:func_8001291C:start
 * symbol: func_8001291C
 * score: 499 differing words
 * frame: 0x2b0
 * relocations: 15
 * first-mismatch: +0x0
 * summary: Mickey m2c fixes batch stride and best-hit state; retained 499 diffs, target-count alternative preserved. Next: authenticated source lifetimes.
 * PLATEAU-HANDOFF:func_8001291C:end
 */

/* PLATEAU-HANDOFF:func_80010B4C:start
 * symbol: func_80010B4C
 * score: 662 differing words
 * frame: 0x158
 * relocations: 9
 * first-mismatch: +0x0
 * summary: Mickey m2c fixes caller ABI and distance state; corrected candidate retains 662 diffs after five stalled follow-ups. Next: source-proved lifetimes.
 * PLATEAU-HANDOFF:func_80010B4C:end
 */
