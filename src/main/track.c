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
extern f32 D_80081770;
extern f32 D_80081774;
extern f32 D_80081790;
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
    func_80044BC8(D_800C9520, "track/track.c", 0x1CC);
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
        func_80044BC8(D_800C9520, "track/track.c", 0x26A);
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
                    31.99f / (radius - secondaryRadius);
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
/* PROVENANCE: JFG's public track.c supplies the resident track draw-loop
 * organization; Mickey's segment and display-list accesses are authoritative. */
/* Matched 2026-10-02 (lane n-track) by rewriting from the listing: a plain
 * while loop (its inverted entry test is the bgtz/blezl pair), batch counts
 * read from the segment at each use, D_800C9520++ packet macros, the
 * texture default as an else arm, the env value masked once into a local,
 * the polygon word opcode-first, the shadow instance re-read through the
 * object, and eight locals between itemIndex and segment for the homes. */
struct TrackShadowObject;
struct TrackShadowInstance;
extern void func_800140CC(struct TrackShadowObject *,
                          struct TrackShadowInstance *);

/* PROVENANCE: the two packet macros follow Diddy Kong Racing's public
 * include/f3ddkr.h (gSPVertexDKR, gSPPolygon), adapted to Mickey's 10-byte
 * track vertices: the DMA length is n * 10 + 8 written as two shifts, and the
 * low byte is n << 3 with the vertex address's 6 bits. */
#define TRACK_VTX(pkt, v, n)                                                 \
    gDma1p(pkt, 0x04, v, ((((n) << 3) + ((n) << 1)) + 8),                    \
           ((n) << 3) | ((u32) (v) & 6))
#define TRACK_TRI(pkt, t, n, tex) {                                          \
    Gfx *_g = (Gfx *) (pkt);                                                 \
    _g->words.w0 = _SHIFTL(0x05, 24, 8) |                                    \
                   _SHIFTL((((n) - 1) << 4) | (tex), 16, 8) |                \
                   _SHIFTL((n) * 16, 0, 16);                                 \
    _g->words.w1 = (unsigned int) (t);                                       \
}

void func_8000DFBC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 batchIndex;
    s32 groupIndex;
    s32 itemIndex;
    u8 *child;
    u8 *vertex;
    u8 *triangle;
    s32 alpha;
    u32 mode;
    s32 textureS;
    s32 objectMode;
    s32 value;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTextureHeader *texture;
    u8 *object;

    segment = &D_800792E8->segments[arg0];
    batch = segment->batches;
    groupIndex = 0;
    itemIndex = 0;
    batchIndex = 0;
    while (batchIndex < segment->batchCount || itemIndex < arg2) {
        if (batchIndex < segment->batchCount &&
            (itemIndex >= arg2 ||
             batchIndex < *(s16 *) (((u8 **) arg3)[itemIndex] + 2))) {
            if ((arg1 & (1 << groupIndex)) && groupIndex == batch->unk1) {
                if (D_8007C854 != 0) {
                    gDPSetPrimColor(D_800C9520++, 0, 0, D_8007C858, D_8007C858, D_8007C858, 255);
                }
                while (batchIndex < segment->batchCount && batch->unk1 == groupIndex) {
                    mode = batch->flags;
                    if (!(mode & 0x800)) {
                        if (batch->textureIndex != 0xFF) {
                            alpha = 1;
                            texture = D_800792E8->textures[batch->textureIndex].texture;
                        } else {
                            texture = NULL;
                            alpha = 0;
                        }
                        vertex = (u8 *) segment->lightData + (batch->u0 * 0xA);
                        triangle = (u8 *) segment->vertexData + (batch->v0 * 0x10);
                        textureS = batch->frame << 8;
                        if (texture != NULL && ((s16) texture->flags & 0x40) &&
                            (mode & 0x30) != 0x20) {
                            value = (textureS >> 8) & 0xFF;
                            gDPSetEnvColor(D_800C9520++, value, value, value, value);
                        } else {
                            gDPSetEnvColor(D_800C9520++, 255, 255, 255, 0);
                        }
                        if (!(mode & 0x180)) {
                            mode |= D_800C9544;
                        }
                        objectMode = mode & 0x4000;
                        if (objectMode) {
                            func_800343F0(2);
                        }
                        func_800349A4(&D_800C9520, texture, mode | 2, textureS);
                        if (objectMode) {
                            texEnableModes(2);
                        }
                        TRACK_VTX(D_800C9520++, vertex + 0x80000000, batch[1].u0 - batch->u0);
                        TRACK_TRI(D_800C9520++, triangle + 0x80000000, batch[1].v0 - batch->v0, alpha);
                    }
                    batchIndex++;
                    batch++;
                }
                if (D_8007C854 != 0) {
                    gDPPipeSync(D_800C9520++);
                    gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, 255);
                }
            } else {
                while (batchIndex < segment->batchCount && batch->unk1 == groupIndex) {
                    batchIndex++;
                    batch++;
                }
            }
            groupIndex++;
        } else {
            object = *(u8 **) (((u8 **) arg3)[itemIndex++] + 4);
            child = *(u8 **) (object + 0x4C);
            if (child != NULL && *(u8 *) (object + 0x8E) == 0) {
                if (*(u8 *) (child + 0x10) & 8) {
                    if (*(u8 **) (*(u8 **) (object + 0x4C) + 0x1C) != NULL) {
                        func_800140CC((struct TrackShadowObject *) object,
                                      *(struct TrackShadowInstance **)
                                          (*(u8 **) (object + 0x4C) + 0x1C));
                    }
                }
                func_800140CC((struct TrackShadowObject *) object,
                              *(struct TrackShadowInstance **) (object + 0x4C));
            }
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
            if (*(s32 *) (object + 0x54) != 0) {
                func_80049518(*(s32 *) (object + 0x54), &D_800C9520);
            }
            if (*(s16 *) (object + 6) & 0x200) {
                switch (*(s16 *) (object + 0x44)) {
                case 1:
                    func_80009414(&D_800C9520, &D_800C9524, &D_800C9528, object);
                    break;
                case 0x1D:
                    TrapDanglingJump(&D_800C9520, &D_800C9524,
                                     &D_800C9528, object);
                    break;
                case 0x49:
                    TrapDanglingJump(&D_800C9520, &D_800C9524,
                                     &D_800C9528, object);
                    break;
                case 0x3F:
                    TrapDanglingJump(&D_800C9520, &D_800C9524,
                                     &D_800C9528, object);
                    break;
                case 0x39:
                    TrapDanglingJump(&D_800C9520, &D_800C9524, object);
                    break;
                case 0x3A:
                    TrapDanglingJump(&D_800C9520, &D_800C9524, object);
                    break;
                }
            }
        }
    }
}
/*
 * PROVENANCE: Mickey's listing and resident track/particle call surfaces
 * reconstruct this draw/update coordinator. Diddy Kong Racing's public
 * `render_level_geometry_and_objects` (src/tracks.c) is the counterpart
 * routine (segment list, per-segment visibility flags, opaque pass); no
 * body is adapted from it.
 */
void func_8000E5EC(s32 updateRate, s32 arg1) {
    s32 i;
    s32 cameraSegment;
    s32 visibleCount;
    s32 resultCount;
    s32 j;
    u8 segmentList[128];
    TrackKeyRecord *records;
    TrackKeyRecord **matches;

    visibleCount = 1;
    if (D_800792E8->segmentCount >= 2) {
        if (levelGetLevel()[0x106] == 0) {
            func_8000FA2C(&visibleCount, (s32) segmentList);
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
    for (i = 1; i <= D_800792E8->segmentCount; i++) {
        D_800C95B0[i] = 0;
    }
    if (D_80079260 != 0 || D_80079264 != 0) {
        i = camGetPtr()->segmentIndex;
        if (i >= 0 && i < D_800792E8->segmentCount && D_8007926C == NULL) {
            /* Inert: resultCount is assigned 0 before any read. The store
             * is the retail register colour. See docs/cleanup-queue.md. */
            i *= (resultCount = D_800792E8->segmentCount);
            j = visibleCount;
            while (j--) {
                D_800C95B0[segmentList[j] + 1] =
                    D_800792E8->visibility[i + segmentList[j]];
            }
        } else {
            j = visibleCount;
            while (j--) {
                D_800C95B0[segmentList[j] + 1] = -1;
            }
        }
        if (D_800792E8->segmentCount < 2) {
            D_800C95B0[1] = -1;
        }
    }
    resultCount = 0;
    records = D_800C9548;
    if (D_80079268 != 0) {
        resultCount = func_8000DB34(visibleCount, segmentList,
                                    (TrackRouteResult *) records);
    }
    func_8000D978(0, arg1);
    func_80034920(&D_800C9520);
    if (D_8007A124 == 0 && camGetMode() == 0) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 1);
    }
    func_80034920(&D_800C9520);
    i = visibleCount;
    matches = (TrackKeyRecord **) &records[resultCount];
    while (i--) {
        func_8000DFBC(segmentList[i], D_800C95B0[segmentList[i] + 1],
                      func_8000DDE4(segmentList[i], resultCount, records,
                                    matches),
                      (s32) matches);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump(&D_800C9520, &D_800C9528);
    }
    if (D_8007A124 == 0 && camGetMode() == 0) {
        partDraw(&D_800C9520, (s32) &D_800C9524, 0);
    }
    D_800C9544 = 0;
}
/*
 * PROVENANCE: Mickey's m2c control-flow draft and the resident track
 * declarations reconstruct this display-list pipeline; no external function
 * body is adapted. The raw offsets retain fields absent from the local types.
 */
/* Matched 2026-10-07 (lane c-track2; 314 at +8 -> 0): the byte walks are
 * `index = visibleCount; while (index--)` over `segmentIds[index]` (uopt makes
 * the cursor and keeps the post-decrement copy), the forward walks are
 * `for (index = 0; index < selectedCount;) object = selectedObjects[index++]`,
 * the 0x8E shadow walk sits inside the D_80079260 test, the TrapDanglingJump
 * display-list argument is uncast, and `selectedCount - 1` is written at each
 * reverse walk (uopt's spilled common subexpression, not a declared local).
 * No `child` or `segment` carrier; one unused s32 above segmentIds sizes the
 * frame. */
extern void func_8000F198(s32 segment, s32 record, s32 mode);

#define E920_U8(base, offset) (*(u8 *) ((u8 *) (base) + (offset)))
#define E920_S8(base, offset) (*(s8 *) ((u8 *) (base) + (offset)))
#define E920_S16(base, offset) (*(s16 *) ((u8 *) (base) + (offset)))
#define E920_S32(base, offset) (*(s32 *) ((u8 *) (base) + (offset)))
#define E920_PTR(base, offset) (*(void **) ((u8 *) (base) + (offset)))
#define E920_RECORD(segment) \
    (D_800C95B0[(segment) + 1])

void func_8000E920(s32 arg0, s32 arg1) {
    s32 segmentEnd;
    s32 visibleCount;
    s32 segmentCount;
    s32 selectedCount;
    s32 index;
    s32 pad;
    u8 segmentIds[128];
    void **selectedObjects;
    u8 *object;

    segmentCount = func_8000A244(&segmentEnd);
    selectedObjects = (void **) D_800C9548;
    if (D_800792E8->segmentCount >= 2) {
        if (E920_U8(levelGetLevel(), 0x106) == 0) {
            func_8000FA2C(&visibleCount, (s32) segmentIds);
        } else {
            func_8000F57C(&visibleCount, segmentIds);
        }
    } else {
        visibleCount = 1;
        segmentIds[0] = 0;
    }
    func_8000A39C(segmentCount, segmentEnd - 1);
    func_80034920(&D_800C9520);
    func_80044BC8(D_800C9520, "track/track.c", 0x58D);
    D_800C95B0[0] = -1;
    for (index = 1; index <= D_800792E8->segmentCount; index++) {
        D_800C95B0[index] = 0;
    }
    if ((D_80079260 != 0) || (D_80079264 != 0)) {
        for (index = visibleCount - 1; index >= 0; index--) {
            E920_RECORD(segmentIds[index]) = -1;
            func_8000F198(segmentIds[index], -1, 0x4000);
        }
    }
    if (D_800792E8->segmentCount < 2) {
        E920_RECORD(0) = -1;
    }
    func_8000D978(0, arg1);
    func_80044BC8(D_800C9520, "track/track.c", 0x5A1);
    if (D_80079260 != 0) {
        for (index = 0; index < visibleCount; index++) {
            func_8000F198(segmentIds[index], E920_RECORD(segmentIds[index]), 0);
        }
    }
    index = segmentCount;
    if (D_80079268 == 0) {
        index = segmentEnd;
    }
    selectedCount = 0;
    while (index < segmentEnd) {
        object = (u8 *) func_800056F0(index++);
        if ((object != NULL) &&
            (E920_RECORD(((TrackRouteObject *) object)->segmentIndex) != 0) &&
            (func_800103D4(object) != 0)) {
            selectedObjects[selectedCount++] = object;
        }
    }
    if (E920_S8(D_800792EC, 0xF6) != 0) {
        TrapDanglingJump(selectedCount, selectedObjects);
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x5D7);
    for (index = 0; index < selectedCount;) {
        object = selectedObjects[index++];
        if ((E920_S32(object, 0x58) != 0) &&
            ((E920_S16(object, 6) & 0xC) == 0) &&
            (E920_U8(object, 0x39) == 0xFF)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x5E3);
    for (index = selectedCount - 1; index >= 0;) {
        object = selectedObjects[index--];
        if ((E920_PTR(object, 0x4C) != NULL) && (E920_U8(object, 0x8E) == 0)) {
            if (E920_U8(E920_PTR(object, 0x4C), 0x10) & 8) {
                if (E920_PTR(E920_PTR(object, 0x4C), 0x1C) != NULL) {
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *) E920_PTR(E920_PTR(object, 0x4C), 0x1C));
                }
            }
            func_800140CC((struct TrackShadowObject *) object,
                          (struct TrackShadowInstance *) E920_PTR(object, 0x4C));
        }
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x5F7);
    for (index = 0; index < selectedCount;) {
        object = selectedObjects[index++];
        if (((E920_S16(object, 6) & 0xC) == 0) &&
            (E920_U8(object, 0x39) == 0xFF) &&
            (E920_S32(object, 0x58) == 0)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x603);
    for (index = selectedCount - 1; index >= 0;) {
        object = selectedObjects[index--];
        if (E920_S16(object, 6) & 8) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
    }
    if (runlinkIsModuleLoaded(0xC) != 0) {
        TrapDanglingJump(&D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (E920_S8(D_800792EC, 0xF6) != 0) {
        func_80044BC8(D_800C9520, "track/track.c", 0x61A);
        TrapDanglingJump(&D_800C9520, &D_800C9524, &D_800C9528);
        if (D_80079260 != 0) {
            index = visibleCount;
            while (index--) {
                func_8000F198(segmentIds[index], E920_RECORD(segmentIds[index]), 0x8000);
            }
            for (index = selectedCount - 1; index >= 0;) {
                object = selectedObjects[index--];
                if ((E920_PTR(object, 0x4C) != NULL) && (E920_U8(object, 0x8E) != 0)) {
                    if (E920_U8(E920_PTR(object, 0x4C), 0x10) & 8) {
                        if (E920_PTR(E920_PTR(object, 0x4C), 0x1C) != NULL) {
                            func_800140CC((struct TrackShadowObject *) object,
                                          (struct TrackShadowInstance *) E920_PTR(E920_PTR(object, 0x4C), 0x1C));
                        }
                    }
                    func_800140CC((struct TrackShadowObject *) object,
                                  (struct TrackShadowInstance *) E920_PTR(object, 0x4C));
                }
            }
        }
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x634);
    if (D_80079260 != 0) {
        index = visibleCount;
        while (index--) {
            func_8000F198(segmentIds[index], E920_RECORD(segmentIds[index]), 4);
        }
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x63B);
    for (index = selectedCount - 1; index >= 0;) {
        object = selectedObjects[index--];
        if (E920_S32(object, 0x54) != 0) {
            func_80049518(E920_S32(object, 0x54), &D_800C9520);
        }
    }
    if (runlinkIsModuleLoaded(0xD) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9524, &D_800C9528);
    }
    if (runlinkIsModuleLoaded(0x22) != 0) {
        TrapDanglingJump((s32) &D_800C9520, &D_800C9528);
    }
    func_80044BC8(D_800C9520, "track/track.c", 0x64E);
    for (index = selectedCount - 1; index >= 0;) {
        object = selectedObjects[index--];
        if ((E920_S16(object, 6) & 4) || (E920_U8(object, 0x39) < 0xFF)) {
            func_80009E78(&D_800C9520, &D_800C9524, &D_800C9528,
                          (TrackSkyObject *) object);
        }
        if (E920_S16(object, 6) & 0x200) {
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
    func_80044BC8(D_800C9520, "track/track.c", 0x678);
    if ((D_8007A124 == 0) && (camGetMode() == 0)) {
        partDraw(&D_800C9520, (s32) &D_800C9524, -1);
    }
    D_800C9544 = 0;
    func_80044BC8(D_800C9520, "track/track.c", 0x680);
}
#undef index
#undef E920_U8
#undef E920_S8
#undef E920_S16
#undef E920_S32
#undef E920_PTR
#undef E920_RECORD
/* Three warning strings no shipped instruction references. They follow the
 * fourteen file-name strings in the TU's .rodata, ahead of its literal pool;
 * the last two name the JFG-era trackPolyHeight and trackGetHeights. splat
 * once migrated them into func_8000E920's listing with that function's own
 * twelve strings; the function is C now, so C defines them. */
const char D_80081620[] = "WARNING: visible blocks exceeded 100\n";
const char D_80081648[] = "trackPolyHeight: Overflow!!!\n";
const char D_80081668[] = "trackGetHeights: Height list overflow\n";
/* PROVENANCE -- JFG's public track.c supplies the surrounding display-list
 * routine and texture vocabulary, while this Mickey body follows its own
 * fields, call sites, and assembly-only command schedule. */
/* Matched 2026-10-02 (lane x-track) by rewriting from the listing in the
 * shape of the matched sibling func_8000DFBC: a while (batchCount--) loop
 * over the segment's batches, the texture default as an else arm, the env
 * value masked once into a local, D_800C9520++ packet macros, the 0x4000
 * case first in the switch (case bodies are laid out in source order), a
 * one-argument func_800343F0 call, and s32 flags and masks (a u32 flags word
 * swaps the AND operands). */
void func_8000F198(s32 arg0, s32 arg1, s32 arg2) {
    TrackBatch *batch;
    TrackTextureHeader *texture;
    u8 *vertex;
    u8 *triangle;
    s32 renderMask;
    s32 skipMask;
    s32 flags;
    s32 alpha;
    s32 textureS;
    s32 special;
    s32 value;
    TrackSegment *segment;
    s32 batchCount;

    if (D_8007C854 != 0) {
        gDPSetPrimColor(D_800C9520++, 0, 0, D_8007C858, D_8007C858,
                        D_8007C858, 255);
    }
    segment = &D_800792E8->segments[arg0];
    switch (arg2) {
    case 0x4000:
        func_800343F0(2);
        renderMask = 0x4800;
        skipMask = 0x800;
        break;
    case 4:
        renderMask = 0xC904;
        skipMask = 0xC800;
        break;
    case 0x8000:
        renderMask = 0xC800;
        skipMask = 0x4800;
        break;
    default:
        renderMask = -1;
        skipMask = 0xC904;
        break;
    }
    batchCount = segment->batchCount;
    batch = segment->batches;
    while (batchCount--) {
        if ((1 << batch->unk1) & arg1) {
            flags = batch->flags;
            if ((flags & renderMask) && !(flags & skipMask)) {
                if (batch->textureIndex != 0xFF) {
                    alpha = 1;
                    texture = D_800792E8->textures[batch->textureIndex].texture;
                } else {
                    texture = NULL;
                    alpha = 0;
                }
                vertex = (u8 *) segment->lightData + (batch->u0 * 0xA);
                triangle = (u8 *) segment->vertexData + (batch->v0 * 0x10);
                textureS = batch->frame << 8;
                if (texture != NULL && ((s16) texture->flags & 0x40) &&
                    (flags & 0x30) != 0x20) {
                    value = (textureS >> 8) & 0xFF;
                    gDPSetEnvColor(D_800C9520++, value, value, value, value);
                } else {
                    gDPSetEnvColor(D_800C9520++, 255, 255, 255, 0);
                }
                if (!(flags & 0x180)) {
                    flags |= D_800C9544;
                }
                special = flags & 0x20000;
                if (special && texture != NULL) {
                    func_80014ECC(texture, textureS, flags);
                } else {
                    func_800349A4(&D_800C9520, texture, flags | 2,
                                  textureS);
                }
                TRACK_VTX(D_800C9520++, vertex + 0x80000000,
                          batch[1].u0 - batch->u0);
                TRACK_TRI(D_800C9520++, triangle + 0x80000000,
                          batch[1].v0 - batch->v0, alpha);
                if (special) {
                    func_80034920(&D_800C9520);
                }
            }
        }
        batch++;
    }
    if (arg2 == 0x4000) {
        texEnableModes(2);
    }
    if (D_8007C854 != 0) {
        gDPPipeSync(D_800C9520++);
        gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, 255);
    }
}
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
/* matched */
/*
 * PROVENANCE: Diddy Kong Racing's public `src/tracks.c`,
 * `get_level_segment_index_from_position`, supplies the segment scan and
 * nearest-height selection structure. Mickey's bounds are inclusive and its
 * TrackData layout, function boundary, and bytes remain authoritative.
 */
/* The segmentCount load stays live across minVal, bounds and i, and limit
 * takes that value. Caller-saved webs then put the bound temporaries in a0
 * and the count in t0, and the direct loop compare emits slt into at. */
/* Count is copied into limit so those two webs stay distinct. */
s32 func_8000FAE0(f32 x, f32 y, f32 z) {
    s32 segmentCount;
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
    s32 limit;
    TrackBoundingBox *bounds;

    result = -1;
    if (D_800792E8 != NULL) {
        segmentCount = D_800792E8->segmentCount;
        minVal = 0x7FFF;
        bounds = D_800792E8->segmentBounds;
        i = 0;
        limit = segmentCount;
        if (limit > 0) {
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
            } while (i < limit);
        }
    }
done:
    return result;
}
/* matched: fallback removed */
/* line kept so later functions stay on their measured lines */
/* line kept so later functions stay on their measured lines */
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
/* PROVENANCE: JFG's assembly-only object-alpha routine supplies the role and switch family;
 * Mickey's jump tables, fields, globals, and arithmetic are authoritative here. */
typedef struct TrackAlphaBounds {
    u8 pad00[0x16];
    s16 distanceLimit;
} TrackAlphaBounds;

typedef struct TrackAlphaObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x34 - 0x18];
    f32 radius;
    u8 pad38;
    u8 alpha;
    u8 pad3A[0x40 - 0x3A];
    TrackAlphaBounds *bounds;
    s16 kind;
    u8 pad46[0x64 - 0x46];
    u8 *state;
} TrackAlphaObject;

/*
 * Matched 2026-10-07 (natural rewrite): a typed object, the per-kind alpha switch with
 * the state pointer read once only in case 1, s32 truncating casts, the fade
 * scale as the literal 0.3f, and one counted loop over D_800C9578[i] with
 * named plane components. Register-exact through variable reuse: one s32
 * holds the kind and then the distance limit (v1), the fade scale overwrites
 * range (f12), and the fade remainder lives in the plane loop's dist (f16).
 */
#define object ((TrackAlphaObject *) objectArg)
s32 func_800103D4(void *objectArg) {
    s32 kind;
    s32 visible;
    u8 *gameMode;
    u8 *state;
    f32 distance;
    f32 range;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeD;
    f32 dist;
    s32 i;

    visible = 1;
    gameMode = func_80028F54();
    kind = object->kind;
    switch (kind) {
    case 65:
        object->alpha = (s32) *(f32 *) (object->state + 0x18);
        break;
    case 63:
        object->alpha = object->state[0xF];
        break;
    case 1:
        state = object->state;
        if (*gameMode == 5) {
            object->alpha = state[0x190];
        } else if (!(*(u16 *) (state + 0x1A8) & 1) || (state[0x170] == 0)) {
            object->alpha = 0xFF;
        }
        break;
    case 80:
        object->alpha = object->state[2];
        break;
    case 88:
        object->alpha = *(u32 *) (object->state + 4);
        break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 29:
    case 79:
        break;
    default:
        object->alpha = 0xFF;
        break;
    }
    if (object->alpha == 0) {
        return 0;
    }
    kind = object->bounds->distanceLimit;
    if (kind != 0) {
        distance = camDistance(object->x, object->y, object->z);
        range = kind;
        if (range < distance) {
            visible = 0;
        } else {
            dist = range - distance;
            range = range * 0.3f;
            if (dist < range) {
                object->alpha = (s32) (object->alpha * (dist / range));
            }
        }
    }
    i = 0;
    while (i < 3 && visible) {
        planeX = D_800C9578[i].x;
        planeY = D_800C9578[i].y;
        planeZ = D_800C9578[i].z;
        planeD = D_800C9578[i].distance;
        dist = (object->x * planeX) + (planeY * object->y) + (planeZ * object->z) + planeD +
               object->radius;
        if (dist < 0.0f) {
            visible = 0;
        }
        i++;
    }
    return visible;
}
#undef object
typedef struct TrackRayPoint {
    f32 x;
    f32 y;
    f32 z;
} TrackRayPoint;

typedef struct TrackTextureFlags {
    void *texture;
    u8 pad04[3];
    u8 flag;
} TrackTextureFlags;
/*
 * PROVENANCE: Mickey's m2c draft and the resident collision-node and plane
 * offsets reconstruct this intersection query; no external body is adapted.
 */
typedef struct TrackRayNode {
    u8 pad00[0x1C];
    TrackPlane *planes;
} TrackRayNode;

/*
 * Natural rewrite, 2026-10-07: one counted node loop, a plane-component local
 * per edge read, the edge index incremented at the loop tail, and the node
 * word reused as the edge index (that reuse keeps the entry copy and the
 * unfolded index * 2 start); the first plane index read through the edge
 * variable numbers its web ahead of the plane pointer; i = 0 as its own
 * statement and i++ at the loop tail schedule the prologue and loop top.
 * Matched 2026-10-07: the redundant `& 0xFFFF` on the u16 edge at the first
 * plane index spends the one ring draw the target spends there (as1 folds
 * the mask away), which puts the edge-loop index temps on t6/t7 as shipped.
 */
s32 func_80010654(TrackRayPoint *start, TrackRayPoint *end,
                  TrackPlane *result, f32 *maximum) {
    s32 pad94;
    TrackRayNode *node;
    s32 pad8C;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeD;
    f32 endDistance;
    f32 startDistance;
    f32 t;
    f32 hitX;
    f32 hitY;
    f32 hitZ;
    f32 value;
    f32 dist;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 nd;
    TrackPlane *plane;
    u16 *entry;
    s32 encoded;
    s32 flip;
    s32 inside;
    s32 i;
    s32 hit;
    u16 edge;

    hit = 0;
    dx = end->x - start->x;
    dy = end->y - start->y;
    dz = end->z - start->z;
    i = 0;
    for (; i < D_800C9D3C;) {
        encoded = D_800C9D2C[i];
        if (encoded > 0) {
            node = (TrackRayNode *) (encoded | 0x80000000);
        } else {
            entry = (u16 *) encoded;
            edge = *entry;
            plane = &node->planes[edge & 0xFFFF];
            encoded = 0;
            planeY = plane->y;
            if (planeY >= 0.707f) {
                planeX = plane->x;
                planeZ = plane->z;
                planeD = plane->distance;
                endDistance = (end->z * planeZ) + ((planeX * end->x) + (planeY * end->y)) + planeD;
                if (endDistance < 0.0f) {
                    startDistance = (start->z * planeZ) + ((planeX * start->x) + (planeY * start->y)) + planeD;
                    if (startDistance >= 0.0f) {
                        t = startDistance / (startDistance - endDistance);
                        if (t <= *maximum) {
                            hitX = start->x + (dx * t);
                            hitY = start->y + (dy * t);
                            hitZ = start->z + (dz * t);
                            inside = TRUE;
                            do {
                                edge = entry[encoded + 1];
                                flip = edge & 0x8000;
                                plane = &node->planes[edge ^ flip];
                                nx = plane->x;
                                ny = plane->y;
                                nz = plane->z;
                                nd = plane->distance;
                                value = (nx * hitX) + (ny * hitY) + (nz * hitZ) + nd;
                                if (flip) {
                                    value = -value;
                                }
                                if (value > 0.0f) {
                                    inside = FALSE;
                                }
                                encoded++;
                            } while (encoded < 3 && inside);
                            if (inside) {
                                *maximum = t;
                                result->x = planeX;
                                result->y = planeY;
                                result->z = planeZ;
                                hit = 1;
                            }
                        }
                    }
                }
            }
        }
        i++;
    }
    return hit;
}
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
/*
 * PROVENANCE: rewritten from the listing in the shape of the matched
 * single-ray sibling func_80010900 (same author); no external function body
 * is adapted. The record layout is the assembly offsets.
 */
typedef struct TrackContactRecord {
    s32 unk0;
    f32 unk4[12];
    f32 distance;
    s32 unk38;
    u8 unk3C;
    u8 flags;
    u8 pad3E[2];
} TrackContactRecord;

struct TrackCollisionSurface;
struct TrackCollisionRecord;
extern void func_800115E4(
    s32 mode, TrackRayPoint *position, TrackRayPoint *offset, f32 scale,
    struct TrackCollisionSurface *surface,
    struct TrackCollisionRecord *record);

s32 func_80010B4C(s32 count, f32 *start, f32 *end, f32 *radius,
                  TrackContactRecord *records, f32 *origin, s32 arg6) {
    TrackRayHit intersection;
    s32 collision;
    s32 queryResult;
    s32 auxiliaryResult;
    f32 *relativeCursor;
    s32 i;
    f32 relative[12];
    union {
        TrackRayPoint point;
        f32 f[3];
    } direction;
    f32 *rel;
    f32 *point;
    f32 lengthSquared;
    f32 minimumLength;
    f32 scale;
    s32 minimumIndex;
    u32 collisionMask;
    s32 tries;
    u32 resultMask;
    s32 index;
    s32 attempt;
    u32 bit;
    u32 failureMask;
    TrackContactRecord *record;

    if (origin != NULL) {
        relativeCursor = relative;
        point = end;
        for (i = 0; i < count; i++) {
            relativeCursor[0] = point[0] - origin[0];
            relativeCursor[1] = point[1] - origin[1];
            relativeCursor[2] = point[2] - origin[2];
            relativeCursor += 3;
            point += 3;
        }
    }
    for (i = 0; i < count; i++) {
        record = &records[i];
        record->unk0 = 0;
        record->flags = 0;
        record->unk4[0] = 0.0f;
        record->unk4[1] = 0.0f;
        record->unk4[2] = 0.0f;
        record->unk4[3] = 0.0f;
        record->unk4[4] = 0.0f;
        record->unk4[5] = 0.0f;
        record->unk4[6] = 0.0f;
        record->unk4[7] = 0.0f;
        record->unk4[8] = 0.0f;
        record->unk4[9] = 0.0f;
        record->unk4[10] = 0.0f;
        record->unk4[11] = 0.0f;
        record->distance = 32000.0f;
        record->unk3C = 0;
        record->unk38 = 0;
    }
    resultMask = 0;
    attempt = 0;
    failureMask = 0;
    do {
        index = 0;
        collisionMask = 0;
        bit = 1;
        do {
            rel = &start[index + index + index];
            scale = radius[index];
            point = &end[index + index + index];
            tries = 0;
            do {
                /* Defined, inert allocation aid: bit is initialized above.
                 * Retained for exact IDO output; see docs/cleanup-queue.md. */
                bit = bit | 0;
                collision = 0;
                auxiliaryResult = 0;
                direction.point.x = point[0] - rel[0];
                direction.point.y = point[1] - rel[1];
                direction.point.z = point[2] - rel[2];
                lengthSquared = (direction.f[2] * direction.f[2]) +
                                ((direction.point.x * direction.point.x) +
                                 (direction.point.y * direction.point.y));
                if (lengthSquared > 0.0f) {
                    lengthSquared = sqrtf(lengthSquared);
                    intersection.ratio = lengthSquared;
                    direction.point.x /= lengthSquared;
                    direction.point.y /= lengthSquared;
                    direction.point.z /= lengthSquared;
                    if (D_800C9D28 != 0) {
                        queryResult = func_80011980((TrackRayPoint *) rel, (TrackRayPoint *) point,
                                                    &direction.point, lengthSquared,
                                                    scale, 0.0f, &intersection);
                    } else {
                        queryResult = func_80011980((TrackRayPoint *) rel, (TrackRayPoint *) point,
                                                    &direction.point, lengthSquared,
                                                    scale, scale, &intersection);
                    }
                    if (D_800C9D28 != 0) {
                        auxiliaryResult = func_80011CDC(
                            (TrackVec3f *) rel, (TrackVec3f *) &direction.point,
                            scale, &intersection);
                    }
                    if ((queryResult | auxiliaryResult) != 0) {
                        record = &records[index];
                        func_800115E4((s32) rel, (TrackRayPoint *) point,
                                      &direction.point, lengthSquared,
                                      (struct TrackCollisionSurface *) &intersection,
                                      (struct TrackCollisionRecord *) record);
                        record->distance = intersection.ratio;
                        collision = 1;
                        collisionMask |= bit;
                    }
                    if (collision != 0) {
                        tries++;
                        if (tries >= 11) {
                            collisionMask = 0;
                            collision = 0;
                            failureMask |= 0x40000000;
                        }
                    }
                }
            } while (collision != 0);
            index++;
            bit <<= 1;
        } while ((index < count) && (failureMask == 0));
        if (((collisionMask != 0) && (attempt >= 11)) || (failureMask != 0)) {
            point = end;
            rel = start;
            i = 0;
            resultMask = 0;
            for (; i < count; i++) {
                point[0] = rel[0];
                point[1] = rel[1];
                point[2] = rel[2];
                point += 3;
                rel += 3;
            }
            origin[0] = start[0] - relative[0];
            origin[1] = start[1] - relative[1];
            origin[2] = start[2] - relative[2];
            if (attempt >= 11) {
                failureMask |= 0x80000000;
            }
        } else if ((collisionMask != 0) && (origin != NULL)) {
            minimumIndex = 0;
            minimumLength = 32000.0f;
            bit = 1;
            i = 0;
            relativeCursor = relative;
            rel = end;
            for (; i < count; i++) {
                if (collisionMask & bit) {
                    record = &records[i];
                    if (record->distance < minimumLength) {
                        minimumIndex = i;
                        minimumLength = record->distance;
                    }
                }
                bit <<= 1;
            }
            record = &records[minimumIndex];
            record->flags |= 1;
            origin[0] = end[(minimumIndex + minimumIndex + minimumIndex)] - relative[(minimumIndex + minimumIndex + minimumIndex)];
            origin[1] = end[(minimumIndex + minimumIndex + minimumIndex) + 1] - relative[(minimumIndex + minimumIndex + minimumIndex) + 1];
            origin[2] = end[(minimumIndex + minimumIndex + minimumIndex) + 2] - relative[(minimumIndex + minimumIndex + minimumIndex) + 2];
            for (i = 0; i < count; i++) {
                *rel++ = origin[0] + relativeCursor[0];
                *rel++ = origin[1] + relativeCursor[1];
                *rel++ = origin[2] + relativeCursor[2];
                relativeCursor += 3;
            }
            resultMask |= collisionMask;
        }
        attempt++;
    } while ((collisionMask != 0) && (failureMask == 0));
    return resultMask | failureMask;
}

/*
 * PROVENANCE: Mickey's collision-response fields, calls and bytes are authority.
 * The newly matched Mickey func_8001EC44 in charControl.c supplies the shared
 * cross-product and projected-point carrier organization; no external body is
 * adapted. TrackRayPoint is the caller's existing three-float scalar view.
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

s32 Arctanf(f32 x, f32 y);

/* Named vector members preserve the compiler's separate scalar identities.
 * The three first-cross carriers become the projected point in the last branch;
 * len is reused while delta preserves the plane offset across calls. Arctanf
 * returns an integer angle, explicitly narrowed for the signed consumer. */
void func_800115E4(s32 mode, TrackRayPoint *pos, TrackRayPoint *vel,
                   f32 radius, TrackCollisionSurface *plane,
                   TrackCollisionRecord *record) {
    f32 dx;
    f32 v;
    f32 dz;
    f32 d;
    f32 u;
    f32 dy;
    f32 w;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 delta;
    f32 len;
    f32 value;
    f32 angle;

    nx = plane->x;
    ny = plane->y;
    nz = plane->z;
    d = plane->distance;
    value = (pos->z * nz) + ((nx * pos->x) + (ny * pos->y)) + d;
    if ((0.707f <= ny) || (plane->flags & 0x10000000)) {
        u = vel->z * ny;
        v = -(vel->z * nx) + (nz * vel->x);
        w = -(vel->x * ny);
        dx = (v * nz) - (w * ny);
        dy = (w * nx) - (u * nz);
        dz = (u * ny) - (v * nx);
        len = (dx * dx) + (dy * dy) + (dz * dz);
        if (0.1f < len) {
            len = sqrtf(len);
            dx /= len;
            dy /= len;
            dz /= len;
            value = radius - plane->positionDistance;
            pos->x = plane->positionX + (value * dx);
            pos->y = plane->positionY + (value * dy);
            pos->z = plane->positionZ + (value * dz);
        } else {
            pos->y = (-((pos->z * nz) + (nx * pos->x) + d) / ny) + 0.01f;
        }
        record->pointY = nx;
        record->pointZ = ny;
        record->value0C = nz;
        record->value3D |= 2;
    } else if (ny <= -0.866f) {
        value = 0.01f - value;
        pos->x = pos->x + (value * nx);
        pos->y = pos->y + (value * ny);
        pos->z = pos->z + (value * nz);
        record->value1C = nx;
        record->value20 = ny;
        record->value24 = nz;
        record->value3D |= 8;
    } else {
        len = 0.01f - value;
        delta = len;
        u = pos->x + (len * nx);
        v = pos->y + (len * ny);
        w = pos->z + (len * nz);
        dx = pos->x - u;
        dy = pos->y - v;
        dz = pos->z - w;
        len = sqrtf((dx * dx) + (dz * dz));
        angle = func_8002A8BC((s16) Arctanf(dy, len));
        if (angle != 0.0f) {
            value = delta / angle;
            len = sqrtf((nx * nx) + (nz * nz));
            pos->x += value * (nx / len);
            pos->z += value * (nz / len);
        } else {
            pos->x = u;
            pos->y = v;
            pos->z = w;
        }
        record->value10 = nx;
        record->value14 = ny;
        record->value18 = nz;
        record->value3D |= 4;
    }
    record->value28 = nx;
    record->value2C = ny;
    record->value30 = nz;
    record->value38 = plane->flags;
    record->value3C = plane->material;
}
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

/*
 * Natural rewrite, 2026-10-07: one counted segment loop, the 0.01f literal,
 * and the node word reused as the edge index (as in func_80010654, it keeps
 * the entry copy and the unfolded index shift). Declarations give the target's 0xC8 frame: one pad
 * slot above the node, face before the plane components, entry and planes
 * after them, start and the hit offset read inline. 107 masked at delta 0.
 * Lane d-mid2: the plane distance loaded into planeValue and then reduced
 * (the target's load lands in the variable's register and planeOffset takes
 * the first ring draw), no block in the arm head (any boundary there hoists
 * the end-point loads into coloured webs), one zero-cost block around
 * `inside = 1` (raises the plane and planeValue save divisors so the start
 * coordinates colour first and planeValue splits to its home), the loop face
 * as a byte offset (base first in the add) and `planeOffset + 0.01f`: 89 -> 24.
 * Both dot products summed z term first, `z + (x + y) + d`: the target's add
 * takes the z product first and its mul the plane component first, which
 * also fixes the ring free order after each sum: 24 -> 10.
 */
s32 func_80011980(TrackRayPoint *start, TrackRayPoint *end,
                  TrackRayPoint *offset, f32 scale, f32 planeOffset,
                  f32 threshold, TrackRayHit *hit) {
    s32 pad;
    TrackRayNodeExtended *node;
    TrackRayFace *face;
    f32 planeX;
    f32 planeY;
    f32 planeZ;
    f32 planeValue;
    u16 *entry;
    TrackRayFace *planes;
    f32 startValue;
    f32 endValue;
    f32 ratio;
    f32 pointX;
    f32 pointY;
    f32 pointZ;
    f32 edgeValue;
    s32 encoded;
    s32 segmentIndex;
    s32 valid;
    s32 sign;
    s32 inside;
    u16 edge;

    valid = 0;
    for (segmentIndex = 0; segmentIndex < D_800C9D3C; segmentIndex++) {
        encoded = D_800C9D2C[segmentIndex];
        if (encoded > 0) {
            node = (TrackRayNodeExtended *) (encoded | 0x80000000);
        } else {
            entry = (u16 *) encoded;
            planes = node->planes;
            face = &planes[*entry];
            encoded = 0;
            planeX = face->x;
            planeY = face->y;
            planeZ = face->z;
            planeValue = face->distance;
            planeValue -= planeOffset;
            endValue = end->z * planeZ + (planeX * end->x + planeY * end->y) + planeValue;
            if (endValue < 0.0f) {
                startValue = start->z * planeZ + (planeX * start->x + planeY * start->y) + planeValue;
                if (startValue >= 0.0f) {
                    ratio = (startValue / (startValue - endValue)) * scale;
                    if (ratio <= hit->ratio) {
                        pointX = ((offset->x * ratio) + start->x) - (planeOffset * planeX);
                        pointY = ((offset->y * ratio) + start->y) - (planeOffset * planeY);
                        pointZ = ((offset->z * ratio) + start->z) - (planeOffset * planeZ);
                        do {
                            inside = 1;
                        } while (0);
                        do {
                            edge = entry[encoded + 1];
                            sign = edge & 0x8000;
                            face = (TrackRayFace *) ((u8 *) planes + ((edge ^ sign) << 4));
                            edgeValue = face->distance +
                                        (face->x * pointX + face->y * pointY + face->z * pointZ);
                            if (sign != 0) {
                                edgeValue = -edgeValue;
                            }
                            if (threshold < edgeValue) {
                                inside = 0;
                            }
                            encoded++;
                        } while (encoded < 3 && inside != 0);
                        if (inside != 0) {
                            hit->normalX = planeX;
                            hit->normalY = planeY;
                            hit->normalZ = planeZ;
                            hit->distance = planeValue;
                            hit->x = ((planeOffset + 0.01f) * planeX) + pointX;
                            hit->y = ((planeOffset + 0.01f) * planeY) + pointY;
                            hit->z = ((planeOffset + 0.01f) * planeZ) + pointZ;
                            hit->faceData = node->metadata[D_800C9D30[segmentIndex]].data;
                            hit->material = ((TrackTextureFlags *) D_800792E8->textures)[
                                node->metadata[D_800C9D30[segmentIndex]].material].flag;
                            hit->ratio = ratio;
                            valid = 1;
                        }
                    }
                }
            }
        }
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

/* PROVENANCE: JFG's public track.c supplies the ray/edge collision role;
 * this body uses Mickey's resident edge records and output layout. */
/* Matched 2026-10-07 (lane c-track2; 329 at +20 -> 0): the record
 * direction's y and z read from the record rather than held in locals (two
 * frame cells), the material flag a struct field (base-first add), the sum of
 * squares and the distance with the z term first, and one zero-cost block in
 * the loop so the counter outranks the D_800792E8 address web. */
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
            /* One extra basic block (brief checklist item 18): it moves every
             * loop-spanning web's save divisor to 8, so the record counter
             * (31) ties the byte offset and wins on web number, and the
             * D_800792E8 address is rematerialised at each use as shipped. */
            do {
                record = recordCount + (TrackClipOutput *) D_800C9D20;
            } while (0);
            if ((func_80012234(origin, direction, (TrackVec3f *) &record->x0,
                               (TrackVec3f *) &record->dx, radius, &t,
                               &tEnd) != 0) &&
                (t >= 0.0f) && (t <= hit->ratio)) {
                normalX = record->dx;
                pointX = origin->f[0] + direction->f[0] * t;
                pointY = origin->f[1] + direction->f[1] * t;
                pointZ = origin->f[2] + direction->f[2] * t;
                differenceX = pointX - record->x0;
                differenceY = pointY - record->y0;
                differenceZ = pointZ - record->z0;
                planeDistance = (differenceX * normalX + differenceY * record->dy +
                                 differenceZ * record->dz) /
                                (record->dz * record->dz +
                                 (normalX * normalX + record->dy * record->dy));
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
                    hit->distance = -(normalX * pointZ + (pointX * hit->normalX + pointY * hit->normalY));
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((TrackTextureFlags *) D_800792E8->textures)[node->indices[record->segment].material].flag;
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
                    hit->distance = -(hit->normalZ * pointZ + (pointX * hit->normalX + pointY * hit->normalY));
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((TrackTextureFlags *) D_800792E8->textures)[node->indices[record->segment].material].flag;
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
                    hit->distance = -(hit->normalZ * pointZ + (pointX * hit->normalX + pointY * hit->normalY));
                    node = record->node;
                    hit->faceData = node->indices[record->segment].data;
                    hit->material = ((TrackTextureFlags *) D_800792E8->textures)[node->indices[record->segment].material].flag;
                    hit->ratio = t;
                }
            }
            recordCount++;
        } while (recordCount < D_800C9D24);
    }
    return result;
}
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
/*
 * PROVENANCE: Mickey's m2c collision trace and the resident vector/track
 * declarations reconstruct this query; no external function body is adapted.
 * Raw offsets retain the compact segment and polygon records.
 */
/* 339 masked words at size +8 (499 at -40, 2026-10-07 lane c-track2):
 * rewritten from the listing. Declarations in the target's frame order (every
 * declared local takes a cell; the homes of segments, bestPlane, entryTimes,
 * best, the three vectors, nearClip/farClip, hitCount, hit, batchIndex,
 * xzMasks, bestFlags, yMasks and the u8 yMask/bestTexture all sit at the
 * target's distance from the frame top), D_800792E8 read by name (no `track`
 * carrier), farClip reused as the best distance, the batch read as
 * segment->batches[batchIndex] at each use, the y test held in a u8 local,
 * named fields for the hit plane and separate ones for each edge plane, one
 * condition for the batch skip, the texture flag as a struct field, the edge
 * plane read in place and the coordinate swaps through temporaryXZ. Left:
 * bestTexture takes s8 where the target keeps it only in its home (three
 * words), the frame 0x20 large (seven more declared scalars than the target's
 * 35 cells between the arrays), and the register naming that follows.
 * 2026-10-08 (lane k-4): 339 -> 310 at +8 (aligned residual 315 -> 285) from
 * three spellings read off the listing: the clip and hit points written
 * `arg0[i] + direction * t` (uopt then emits the product first, as shipped),
 * both plane sides as x, y, z terms in order then the distance, and the
 * polygon as an 8-byte facet record indexed by the triangle (the target
 * scales the triangle by 8, not the shared triangle * 4).
 * 2026-10-08 (lane p-2): 310 at +8 -> 293 at 0 (aligned residual 285 -> 192):
 * one plane pointer for the first plane and each edge plane (the edge plane
 * from a second read of the polygon word, lane m-3, merges the a0 plane web),
 * batch flags and the visibility word read in place (two fewer cells), pads
 * before nearClip, after hit and after xzMasks (target cell counts), and the
 * batch skip as an else-if (the target's two `first = last` arms). The frame
 * is still 0x18 large: eight more cells between farClip and hitCount.
 * 2026-10-08 (lane q-2): 293 -> 242 at 0 (aligned residual 192 -> 119), frame
 * 0x288 exact with the allocation byte-identical. Every declared local takes
 * a cell here, so the eight are locals the target's source reuses: the first
 * plane is read into value, fraction, pointX and pointY (the target's normalZ
 * and pointX share f20, the distance and pointY f22), the batch's first
 * triangle is held in edgeIndex, and xzMask, triangleIndex and lastTriangle
 * sit where the three frame pads were. Each merge was screened alone for an
 * unchanged object outside the frame offsets; only these were neutral.
 * 242 -> 234 at 0 (aligned residual 119 -> 93), lane q-2: the target also
 * reuses first-phase locals as second-phase webs (a scan of every same-typed
 * merge, ranked aligned): the edge word in x1, the inside flag in
 * insertIndex, the triangle index in z1, the swap temporary in segment, and
 * the edge value negated in place (no edgeValue). The declarations those
 * free hold the first-plane floats, so the frame stays 0x288.
 * 234 -> 64 at 0 (aligned residual 93 -> 64), lane r-3: the insertion sort
 * is a while loop swapping one array at a time and both distances sum x
 * and y before z; then 64 -> 57: the visibility word masked by xzMask is
 * read into x0 (a phase-1 local) before yHit, which gives the target's
 * load order and its two v0/v1 tests with no yHit copy.
 * 57 -> 45 at 0 (lane s-3): arg4 is unsigned and tested `> 0`. An unsigned
 * compare with zero lets ugen keep the one arg4 load for the and block (a
 * `!= 0` test reloads it and spends one more ring draw, which rotated every
 * ring register from the else-if on); the yHit test then needs its `& 0xFF`
 * back for the target's second andi. 45 -> 22: the hit block stores
 * bestTexture masked (`& 0xFF`, the draw the target spends before its flag
 * load) and assigns bestPlane before `hit = 1`, which removed the ring
 * rotation from +0x72C to the end. 22 -> 18: the masked visibility word is
 * held in edgeSign (a phase-2 local), not in x0, whose phase-1 range kept it
 * out of v0. 18 -> 6: the phase-1 z coordinate is edgeIndex (its own web,
 * v0) and z1 is only the triangle index; the edge value's sign is a
 * conditional expression, whose extra blocks order the triangle index after
 * polygon (t3/t2 as shipped); xzMask is also the insertion sort's xzMasks
 * swap temporary, which ties it with the batch cursor and wins on web number
 * (s4/s5); the coordinate swaps go through insertIndex. pad is the unused
 * cell the old swap temporary held; deleting it moves the frame.
 * 6 -> 4 at 0 (lane t-1): both plane lookups take the DKR tracks.c form,
 * the plane index scaled by four into a local and then an f32 subscript,
 * which puts the base first in the add. The local is reused for its next
 * role straight after the lookup (edgeIndex becomes the edge counter, x1
 * the edge word); without that later definition uopt substitutes the
 * address into the plane's field loads and the plane leaves a0. The sign
 * mask is written first in the xor for the shipped operand order.
 * Matched 2026-10-09 (lane u-1): the hit block stores the texture flag with
 * no mask (the `& 0xFF` that stood in for the target's draw is gone) and the
 * hit plane takes the same f32-subscript form as the other plane lookups,
 * `[polygon[0] << 2]`: the two scales spend one more ring draw, which as1
 * folds into a single shift, and that is the draw the target spends. 4 to 0. */
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

/* One collision facet: the plane index, then the three edge-plane indices. */
typedef struct TrackFacet {
    u16 indices[4];
} TrackFacet;

s32 func_8001291C(f32 *arg0, f32 *arg1, f32 *arg2, s32 arg3, u32 arg4) {
    TrackSegment *segments[20];
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    TrackPlane *bestPlane;
    f32 entryTimes[20];
    TrackVec3f best;
    TrackVec3f minimum;
    TrackVec3f maximum;
    TrackVec3f direction;
    s32 y1;
    s32 z1;
    s32 insertIndex;
    f32 planeDistance;
    s32 pad;
    TrackBoundingBox *bounds;
    s32 segmentIndex;
    s32 xzMask;
    f32 nearClip;
    f32 farClip;
    TrackSegment *segment;
    TrackPlane *surfaceBase;
    u16 *polygon;
    TrackPlane *plane;
    f32 pointX;
    f32 pointY;
    f32 edgeX;
    f32 edgeY;
    f32 edgeZ;
    f32 edgeD;
    u8 yHit;
    f32 side0;
    f32 side1;
    f32 fraction;
    f32 pointZ;
    s32 hitCount;
    s32 edgeIndex;
    s32 firstTriangle;
    s32 hit;
    f32 normalZ;
    f32 normalX;
    s32 edgeSign;
    s32 batchIndex;
    f32 value;
    s32 xzMasks[20];
    s32 lastTriangle;
    f32 normalY;
    u32 bestFlags;
    u8 yMasks[20];
    u8 yMask;
    u8 temporaryY;
    u8 bestTexture;

    direction.f[0] = arg1[0] - arg0[0];
    direction.f[1] = arg1[1] - arg0[1];
    direction.f[2] = arg1[2] - arg0[2];
    if ((direction.f[0] != 0.0f) || (direction.f[1] != 0.0f) ||
        (direction.f[2] != 0.0f)) {
        hitCount = 0;
        for (segmentIndex = 0; segmentIndex < D_800792E8->segmentCount;
             segmentIndex++) {
            bounds = &D_800792E8->segmentBounds[segmentIndex];
            minimum.f[0] = bounds->x1;
            minimum.f[1] = bounds->y1;
            minimum.f[2] = bounds->z1;
            maximum.f[0] = bounds->x2;
            maximum.f[1] = bounds->y2;
            maximum.f[2] = bounds->z2;
            if ((func_800131AC((TrackVec3f *) arg0, &direction, &minimum,
                               &maximum, &nearClip, &farClip) != 0) &&
                (((nearClip <= 0.0f) && (farClip >= 0.0f)) ||
                 ((nearClip >= 0.0f) && (nearClip <= 1.0f)))) {
                if (nearClip < 0.0f) {
                    nearClip = 0.0f;
                }
                if (farClip > 1.0f) {
                    farClip = 1.0f;
                }
                x0 = arg0[0] + direction.f[0] * nearClip;
                y0 = arg0[1] + direction.f[1] * nearClip;
                z0 = arg0[2] + direction.f[2] * nearClip;
                x1 = arg0[0] + direction.f[0] * farClip;
                y1 = arg0[1] + direction.f[1] * farClip;
                edgeIndex = arg0[2] + direction.f[2] * farClip;
                if (x1 < x0) {
                    insertIndex = x1;
                    x1 = x0;
                    x0 = insertIndex;
                }
                if (y1 < y0) {
                    insertIndex = y1;
                    y1 = y0;
                    y0 = insertIndex;
                }
                if (edgeIndex < z0) {
                    insertIndex = edgeIndex;
                    edgeIndex = z0;
                    z0 = insertIndex;
                }
                xzMasks[hitCount] = getXZCompareMask(bounds, x0, z0, x1, edgeIndex);
                yMasks[hitCount] = getYCompareMask(bounds, y0, y1);
                entryTimes[hitCount] = nearClip;
                segments[hitCount] = &D_800792E8->segments[segmentIndex];
                insertIndex = hitCount;
                while ((insertIndex > 0) && (entryTimes[insertIndex] < entryTimes[insertIndex - 1])) {
                    nearClip = entryTimes[insertIndex];
                    entryTimes[insertIndex] = entryTimes[insertIndex - 1];
                    entryTimes[insertIndex - 1] = nearClip;
                    segment = segments[insertIndex];
                    segments[insertIndex] = segments[insertIndex - 1];
                    segments[insertIndex - 1] = segment;
                    xzMask = xzMasks[insertIndex];
                    xzMasks[insertIndex] = xzMasks[insertIndex - 1];
                    xzMasks[insertIndex - 1] = xzMask;
                    temporaryY = yMasks[insertIndex];
                    yMasks[insertIndex] = yMasks[insertIndex - 1];
                    yMasks[insertIndex - 1] = temporaryY;
                    insertIndex--;
                }
                hitCount++;
                if (hitCount >= 20) {
                    segmentIndex = D_800792E8->segmentCount;
                }
            }
        }
    }
    hit = 0;
    farClip = 1.0f;
    best.f[0] = arg1[0];
    best.f[1] = arg1[1];
    best.f[2] = arg1[2];
    arg3 |= 0x1080;
    for (segmentIndex = 0; (segmentIndex < hitCount) && (hit == 0);
         segmentIndex++) {
        segment = segments[segmentIndex];
        xzMask = xzMasks[segmentIndex];
        yMask = yMasks[segmentIndex];
        surfaceBase = segment->surfaces;
        for (batchIndex = 0; batchIndex < segment->batchCount; batchIndex++) {
            firstTriangle = segment->batches[batchIndex].v0;
            lastTriangle = segment->batches[batchIndex + 1].v0;
            if (segment->batches[batchIndex].flags & arg3) {
                firstTriangle = lastTriangle;
            } else if ((arg4 > 0) &&
                       ((segment->batches[batchIndex].flags & arg4) == 0)) {
                firstTriangle = lastTriangle;
            }
            for (z1 = firstTriangle; z1 < lastTriangle;
                 z1++) {
                edgeSign = segment->visibilityMasks[z1] & xzMask;
                yHit = E129_U8(E129_PTR(segment, 0x14), z1) & yMask;
                if (((edgeSign & 0xFFFF) != 0) && ((edgeSign & 0xFFFF0000) != 0) &&
                    ((yHit & 0xFF) != 0)) {
                    polygon = ((TrackFacet *) segment->surfaceIndices)[z1].indices;
                    edgeIndex = polygon[0] << 2;
                    plane = (TrackPlane *) &((f32 *) surfaceBase)[edgeIndex];
                    edgeIndex = 0;
                    normalX = plane->x;
                    normalY = plane->y;
                    normalZ = plane->z;
                    planeDistance = plane->distance;
                    side1 = (normalX * arg1[0]) + (normalY * arg1[1]) +
                            (normalZ * arg1[2]) + planeDistance;
                    if (side1 < 0.0f) {
                        side0 = (normalX * arg0[0]) + (normalY * arg0[1]) +
                                (normalZ * arg0[2]) + planeDistance;
                        if (side0 >= 0.0f) {
                            fraction = side0 / (side0 - side1);
                            pointX = arg0[0] + (direction.f[0] * fraction);
                            pointY = arg0[1] + (direction.f[1] * fraction);
                            pointZ = arg0[2] + (direction.f[2] * fraction);
                            insertIndex = 1;
                            for (; (edgeIndex < 3) && (insertIndex != 0); edgeIndex++) {
                                x1 = ((polygon[edgeIndex + 1] & 0x8000) ^
                                      polygon[edgeIndex + 1]) << 2;
                                plane = (TrackPlane *) &((f32 *) surfaceBase)[x1];
                                x1 = polygon[edgeIndex + 1];
                                edgeSign = x1 & 0x8000;
                                edgeX = plane->x;
                                edgeY = plane->y;
                                edgeZ = plane->z;
                                edgeD = plane->distance;
                                value = (edgeX * pointX) + (edgeY * pointY) +
                                        (edgeZ * pointZ) + edgeD;
                                value = (edgeSign != 0) ? -value : value;
                                if (value > 0.0f) {
                                    insertIndex = 0;
                                }
                            }
                            if ((insertIndex != 0) && (fraction < farClip)) {
                                farClip = fraction;
                                best.f[0] = pointX;
                                best.f[1] = pointY;
                                best.f[2] = pointZ;
                                bestFlags = segment->batches[batchIndex].flags;
                                bestTexture = ((TrackTextureFlags *) D_800792E8->textures)[
                                    segment->batches[batchIndex].textureIndex].flag;
                                bestPlane = (TrackPlane *) &((f32 *) surfaceBase)[polygon[0] << 2];
                                hit = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    if (hit != 0) {
        E129_S32(arg2, 0) = 0;
        E129_F32(arg2, 4) = best.f[0];
        E129_F32(arg2, 8) = best.f[1];
        E129_F32(arg2, 0xC) = best.f[2];
        E129_F32(arg2, 0x10) = bestPlane->x;
        E129_F32(arg2, 0x14) = bestPlane->y;
        E129_F32(arg2, 0x18) = bestPlane->z;
        E129_F32(arg2, 0x1C) = bestPlane->distance;
        direction.f[0] *= farClip;
        direction.f[1] *= farClip;
        direction.f[2] *= farClip;
        E129_F32(arg2, 0x20) = sqrtf(
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1])) +
            (direction.f[2] * direction.f[2]));
        E129_S32(arg2, 0x24) = bestFlags;
        E129_S32(arg2, 0x28) = bestTexture;
    } else {
        E129_F32(arg2, 0x20) = sqrtf(
            ((direction.f[0] * direction.f[0]) +
             (direction.f[1] * direction.f[1])) +
            (direction.f[2] * direction.f[2]));
    }
    return hit;
}
#undef E129_U8
#undef E129_S16
#undef E129_U16
#undef E129_S32
#undef E129_F32
#undef E129_PTR
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
 * PROVENANCE: Mickey reconstruction from the target's collision-query
 * callers, resident track layouts, and the neighboring collision helpers.
 * The plane lookup's statement pair (index scaled by four into a local, then
 * an f32 subscript) is adapted from Diddy Kong Racing's src/tracks.c, which
 * reads `collisionFacets[j].basePlaneIndex << 2` the same way.
 */
/* Matched 2026-10-02 (lane z-track). Listing rewrite in func_8001398C's
 * indexed shape (lane x-track, 189 -> 2), closed by the plane lookup: the
 * facet's plane index is scaled by four into the block's scratch local and
 * then indexes the planes as an f32 array, the DKR tracks.c form
 * (`var = facets[j].basePlaneIndex << 2; planes[var]`). uopt folds the two
 * shifts into one and forwards the index as an unnamed value, so the base is
 * loaded between the index load and its scale and is the add's first
 * operand. Reusing the visibility local keeps the frame; a fresh local adds
 * a cell and moves three spill homes. */
u32 func_8001357C(f32 arg0, f32 arg1, f32 *arg2, s32 arg3, void *arg4) {
    s32 compareMask;
    s32 x;
    u32 resultCount;
    s32 segmentCount;
    s32 z;
    TrackSegment *segment;
    TrackTriangle *triangle;
    TrackVertex *vertices;
    s32 batchFlags;
    s32 segmentNumber;
    s32 batchNumber;
    s16 segmentIndices[32];
    TrackPlane *surface;
    s32 triangleIndex;
    TrackPlane plane;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;
    TrackIntersection *hit;
    f32 height;
    TrackBatch *batch;
    f32 swapHeight;
    s32 swapFlags;
    u32 outer;

    x = (s32) arg0;
    z = (s32) arg1;
    segmentCount = func_8000FCA4(x, z, segmentIndices);
    if (arg2 != NULL) {
        *arg2 = -32768.0f;
    }
    resultCount = 0;
    segmentNumber = 0;
    if (segmentCount > 0) {
        do {
            compareMask = getXZCompareMask(
                &D_800792E8->segmentBounds[segmentIndices[segmentNumber]], x,
                z, x, z);
            segment = &D_800792E8->segments[segmentIndices[segmentNumber]];
            segmentNumber++;
            batch = segment->batches;
            batchNumber = segment->batchCount;
            while (batchNumber--) {
                batchFlags = batch->flags;
                if (batchFlags & arg3) {
                    triangle =
                        &((TrackTriangle *) segment->vertexData)[batch->v0];
                    vertices = &((TrackVertex *) segment->lightData)[batch->u0];
                    triangleIndex = batch->v0;
                    if (batch->v0 < batch[1].v0) {
                        do {
                            u32 temp =
                                segment->visibilityMasks[triangleIndex] &
                                compareMask;
                            if ((temp >> 16) != 0 &&
                                (temp & 0xFFFF) != 0) {
                                vertex0 = &vertices[triangle->vertex0];
                                vertex1 = &vertices[triangle->vertex1];
                                vertex2 = &vertices[triangle->vertex2];
                                if (mathXZInTri(x, z, vertex0, vertex1,
                                                vertex2) != 0) {
                                    height = vertex0->y;
                                    if (vertex1->y != vertex0->y ||
                                        vertex2->y != vertex0->y) {
                                        if (batch->flags & 0x1080) {
                                            func_800133FC(vertex0, vertex1,
                                                          vertex2, &plane.x,
                                                          &plane.y, &plane.z,
                                                          &plane.distance);
                                            surface = &plane;
                                        } else {
                                            temp = *(u16 *) ((u8 *) segment->surfaceIndices +
                                                             triangleIndex * 8) << 2;
                                            surface = (TrackPlane *) &((f32 *) segment->surfaces)[temp];
                                        }
                                        if (surface->y > 0.0f) {
                                            height = -(((surface->x * arg0) +
                                                        (surface->z * arg1) +
                                                        surface->distance) /
                                                       surface->y);
                                        }
                                    }
                                    if (arg4 != NULL) {
                                        if (resultCount >= 8) {
                                            resultCount = 7;
                                        }
                                        hit = &((TrackIntersection *) arg4)[resultCount];
                                        hit->height = height;
                                        hit->flags = batchFlags;
                                        resultCount++;
                                    } else {
                                        *arg2 = height;
                                        return batchFlags;
                                    }
                                }
                            }
                            triangleIndex++;
                            triangle++;
                        } while (triangleIndex < batch[1].v0);
                    }
                }
                batch++;
            }
        } while (segmentNumber != segmentCount);
    }
    if (resultCount >= 2) {
        outer = resultCount - 1;
        while (outer--) {
            hit = arg4;
            batchNumber = outer + 1;
            while (batchNumber--) {
                if (hit[0].height < hit[1].height) {
                    swapHeight = hit[0].height;
                    hit[0].height = hit[1].height;
                    hit[1].height = swapHeight;
                    swapFlags = hit[0].flags;
                    hit[0].flags = hit[1].flags;
                    hit[1].flags = swapFlags;
                }
                hit++;
            }
        }
    }
    return resultCount;
}
/* PROVENANCE: JFG's public track.c retains this collision collector as
 * assembly; Mickey's segment, batch, plane and hit-list accesses are used.
 * The plane lookup (index scaled by four into a local, then an f32
 * subscript) and the tail (a local list pointer filled and then bubble
 * sorted through) are adapted from Diddy Kong Racing's src/tracks.c
 * water-height collector, which has both. */
/* Matched 2026-10-02 (lane z-track), 8 -> 0 on two edits to the shape the
 * earlier lanes built (163 -> 8; their steps are in the shard). The plane
 * index is shifted by two into the block's scratch local and the planes
 * are subscripted as f32, so the base loads between the index load and its
 * scale (+0x1A8, as func_8001357C). The hit list is reached through a local
 * pointer in both tail loops, which makes uopt create the unrolled sort's
 * end pointer before its cursor (+0x400) and retires the dead right-element
 * read the direct-symbol form needed. The swap goes through `batch`, the
 * one existing pointer whose web leaves a0/a1 to the compared pair. */
s32 func_8001398C(f32 arg0, f32 arg1, s32 arg2, void **arg3) {
    typedef struct TrackCollisionHit {
        f32 height;
        TrackPlane *surface;
        u32 flags;
        s8 textureFlag;
        u8 pad0D[3];
    } TrackCollisionHit;

    f32 px;
    f32 pz;
    f32 pd;
    s32 x;
    s32 z;
    s32 batchNumber;
    s32 triangleIndex;
    s32 batchFlags;
    s8 textureFlag;
    s32 resultCount;
    s32 orderIndex;
    s32 changed;
    s32 firstTriangle;
    s32 lastTriangle;
    s32 segmentCount;
    s32 segmentNumber;
    TrackPlane *surface;
    s16 textureOffset;
    TrackSegment *segment;
    TrackBatch *batch;
    TrackTriangle *triangle;
    s16 segmentIndices[32];
    TrackCollisionHit *hit;
    TrackVertex *vertex0;
    TrackVertex *vertex1;
    TrackVertex *vertex2;
    f32 planeHeight;
    TrackCollisionHit **list;

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
        s32 compareMask;
        segment = &D_800792E8->segments[segmentIndices[segmentNumber]];
        compareMask = getXZCompareMask(
            &D_800792E8->segmentBounds[segmentIndices[segmentNumber]], x, z, x, z);
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
                        u32 temp = segment->visibilityMasks[triangleIndex] & compareMask;
                        if ((temp >> 16) != 0 &&
                            (temp & 0xFFFF) != 0) {
                            temp = *(u16 *) ((u8 *) segment->surfaceIndices +
                                                   (triangleIndex * 8)) << 2;
                            surface = (TrackPlane *) &((f32 *) segment->surfaces)[temp];
                            planeHeight = surface->y;
                            if (planeHeight > 0.0f) {
                                triangle = (TrackTriangle *)
                                    ((u8 *) segment->vertexData +
                                     (triangleIndex * 0x10));
                                vertex0 = (TrackVertex *)
                                    (((triangle->vertex0 + textureOffset) * 0xA) + (u8 *) segment->lightData);
                                vertex1 = (TrackVertex *)
                                    (((triangle->vertex1 + textureOffset) * 0xA) + (u8 *) segment->lightData);
                                vertex2 = (TrackVertex *)
                                    (((triangle->vertex2 + textureOffset) * 0xA) + (u8 *) segment->lightData);
                                if (mathXZInTri(x, z, vertex0, vertex1,
                                                vertex2) != 0) {
                                    px = surface->x;
                                    pz = surface->z;
                                    pd = surface->distance;
                                    hit = (TrackCollisionHit *) D_800C9B90 + resultCount;
                                    hit->height = -(((px * arg0) + (pz * arg1) + pd) / planeHeight);
                                    hit->surface = surface;
                                    resultCount++;
                                    hit->flags = segment->batches[batchNumber].flags;
                                    hit->textureFlag = textureFlag;
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
    list = (TrackCollisionHit **) D_800C9CD0;
    orderIndex = 0;
    if (resultCount > 0) {
        do {
            list[orderIndex] = &hit[orderIndex];
        } while (++orderIndex != resultCount);
    }
    do {
        orderIndex = 0;
        changed = 1;
        for (; orderIndex < resultCount - 1; orderIndex++) {
            if (list[orderIndex]->height < list[orderIndex + 1]->height) {
                batch = (void *) list[orderIndex];
                list[orderIndex] = list[orderIndex + 1];
                list[orderIndex + 1] = (void *) batch;
                changed = 0;
            }
        }
    } while (changed == 0);
    *arg3 = D_800C9CD0;
    return resultCount;
}
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
/*
 * PROVENANCE: the shape follows Diddy Kong Racing's public `src/tracks.c`,
 * `shadow_render`: the mesh's first vertex alpha read into `alpha` and then
 * scaled by the object's opacity, a heap-record cursor walked from the mesh
 * start, the vertex and triangle counts read and then reused for the spans,
 * and the `gSPVertexDKR`/`gSPPolygon` packet pair. Mickey's shadow-buffer
 * ABI, per-kind colour setup and two-mesh instance layout are authoritative.
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
    s16 meshStart[2];
    s16 meshEnd[2];
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

/* Matched 2026-10-02 (lane x-track) by rewriting from the listing in the
 * DKR shadow_render shape: typed vertex/triangle buffers, the vertex alpha
 * read into alpha before the opacity scale (the target loads it straight
 * into alpha's register), one count variable each for the first index and
 * the span, a heap cursor, the 0xE flags default as an else arm (it fills
 * the kind branch's delay slot), and one unreferenced local between the
 * triangle and vertex buffers for the 0xA8 frame's homes. */
void func_800140CC(TrackShadowObject *object, TrackShadowInstance *shadow) {
    s32 k;
    s32 i;
    s32 alpha;
    s32 flags;
    TrackShadowMaterial *material;
    s32 closePrim;
    s32 closeEnv;
    s32 numVerts;
    s32 numTris;
    TrackShadowBatch *heapData;
    TrackTriangle *triangles;
    s32 unused;
    TrackVertex *vertices;
    TrackVertex *vtx;
    TrackTriangle *tri;
    TrackShadowBatch *heap;

    if (shadow->active) {
        shadowGetBuffers(shadow->active, (void **) &vertices,
                         (void **) &triangles, (void **) &heapData);
        for (k = 0; k < shadow->count; k++) {
            if (shadow->meshStart[k] != -1) {
                i = shadow->meshStart[k];
                heap = &heapData[i];
                alpha = vertices[heap->firstVertex].a;
                alpha = (object->alpha * alpha) >> 8;
                if (alpha > 0) {
                    if (object->kind == 0x3C) {
                        material = object->material;
                        gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, alpha);
                        flags = 0x20E;
                        gDPSetEnvColor(D_800C9520++, material->red,
                                       material->green, material->blue, 0);
                        closePrim = TRUE;
                        closeEnv = TRUE;
                    } else {
                        flags = 0xE;
                        closeEnv = FALSE;
                        if (object->kind == 0x35 || object->kind == 0x58) {
                            gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, alpha);
                            closePrim = alpha != 255;
                        } else {
                            gDPSetPrimColor(D_800C9520++, 0, 0, 0, 0, 0, alpha);
                            closePrim = TRUE;
                        }
                    }
                    while (i < shadow->meshEnd[k]) {
                        func_800349A4(&D_800C9520, heap->texture, flags,
                                      shadow->textureScale << 8);
                        numTris = heap->firstIndex;
                        numVerts = heap->firstVertex;
                        tri = &triangles[numTris];
                        vtx = &vertices[numVerts];
                        numTris = heap[1].firstIndex - numTris;
                        numVerts = heap[1].firstVertex - numVerts;
                        TRACK_VTX(D_800C9520++, (u8 *) vtx + 0x80000000, numVerts);
                        TRACK_TRI(D_800C9520++, (u8 *) tri + 0x80000000, numTris, 1);
                        i++;
                        heap++;
                    }
                    if (closePrim) {
                        gDPSetPrimColor(D_800C9520++, 0, 0, 255, 255, 255, 255);
                    }
                    if (closeEnv) {
                        gDPSetEnvColor(D_800C9520++, 255, 255, 255, 0);
                    }
                }
            }
        }
    }
}
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

/* PLATEAU-HANDOFF:func_80011980:start
 * symbol: func_80011980
 * score: 10/215 words
 * frame: 0xC8
 * relocations: 12
 * first-mismatch: +0xC0
 * summary: Unchanged at 10, all naming; double-read lever sweep and the sibling 10654 shape product flat.
 * PLATEAU-HANDOFF:func_80011980:end
 */
