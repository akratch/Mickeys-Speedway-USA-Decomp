#ifndef OVERLAY_047_H
#define OVERLAY_047_H

#include "PR/ultratypes.h"
#include "game/menu_resources.h"
#include "n_audio/mbi.h"
#include "game/math.h"
#include "game/models.h"
#include "overlays/overlay_045.h"

/* Tier D: member widths and offsets follow this module's access sites. */
typedef struct Overlay47TextureScroll {
    u8 pad00[8];
    s16 offsets[4];
} Overlay47TextureScroll;

typedef struct Overlay47Actor {
    s16 rotation;
    u8 pad02[0xA];
    f32 x, y, z;
    u8 pad18[0xC];
    s32 field24;
    f32 frameValue;
    u8 pad2C[0xF];
    u8 frame;
    s32 field3C;
    u8 pad40[6];
    s16 kind;
    u8 pad48[0x1C];
    Overlay47TextureScroll *scroll;
    void *animationStates;
    u8 pad6C[0x14];
    s32 trigger;
} Overlay47Actor;

typedef struct Overlay47Player {
    f32 x, y, z;
    f32 targetX, targetY, targetZ;
    f32 screenX, screenY;
    s16 rotation, targetRotation;
    Overlay47Actor *actor;
    s16 selector;
    s8 ready, active, leaving;
    u8 pad2D;
    s16 idleTimer;
    void *sound;
} Overlay47Player;

typedef struct Overlay47Position {
    f32 x, y, z;
    s16 rotation;
    u8 pad0E[2];
} Overlay47Position;

typedef struct Overlay47Icon {
    s16 rotationX, rotationY, rotationZ;
    u16 flags;
    f32 scale, x, y, z;
    u8 pad18[0x10];
    f32 selector;
} Overlay47Icon;

typedef struct Overlay47TextureNode {
    void *texture;
    void *alternate;
    u32 packedOffset;
    s16 x, y;
} Overlay47TextureNode;

/* Command cursor type shared with the resident Gfx APIs. */
typedef Gfx Overlay47Command;

typedef struct Overlay47Vertex {
    s16 x, y, z;
    u8 red, green, blue, alpha;
} Overlay47Vertex;

typedef struct Overlay47Triangle {
    u8 flags, vertex0, vertex1, vertex2;
    s16 s0, t0, s1, t1, s2, t2;
} Overlay47Triangle;

typedef struct Overlay47VertexSet6 {
    Overlay47Vertex vertices[6];
    u8 reserved[4];
} Overlay47VertexSet6;

typedef struct Overlay47SpawnPacket {
    s16 selector;
    u8 type;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 byte0A;
    u8 byte0B;
    s8 state;
} Overlay47SpawnPacket;

typedef struct Overlay47PathNode {
    s16 selector;
    u8 pad02[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    struct Overlay47PathNode *next;
} Overlay47PathNode;

typedef struct Overlay47PathObject {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[9];
    Overlay47PathNode *path;
} Overlay47PathObject;

typedef struct Overlay47Save {
    u32 pad0 : 4;
    u32 unlocked : 1;
    u32 pad5 : 27;
    u8 pad04[4];
    u16 configuration[4];
    u16 field10;
} Overlay47Save;

extern Overlay47Player D_800D3058[4];
extern s8 D_800D3190[4];
extern s32 D_8007C1A0;
extern u8 D_8007BF74;
extern char **D_8007C0B8;
extern Overlay47Command *D_800D3140;
extern Mtx *D_800D3144;
extern void *D_800D3148;
extern u16 joyGetPressed(s32 player);
extern void amSndPlay(u16 soundId, void **handle);
extern void amSndStop(void *handle);
extern void func_80006EA0(void *object);
extern void func_8005AD64(void *object, s32 frame, s32 index, f32 value);
extern s32 func_8005ABA8(void *object, f32 speed, f32 updateRate);
extern s32 camProjectPoint(f32 x, f32 y, f32 z, f32 *screenX, f32 *screenY, u8 mode);
extern s32 func_8001398C(f32 x, f32 z, s32 flags, f32 ***height);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern f32 camGetFOV(void);
extern void camSetFOV(f32 fov, s32 force);
extern void camSetView(Overlay47Command **commands, Mtx **matrices);
extern void camStandardOrtho(Overlay47Command **commands, Mtx **matrices);
extern void texDPTextureX(Overlay47Command **commands, void *texture, s32 flags, s32 offset);
extern void texDPInit(Overlay47Command **commands);
extern void func_8002FB34(Overlay47Command **commands, Overlay47TextureNode *textures,
                        f32 x, f32 y, f32 scaleX, f32 scaleY, s32 mode, s32 flags);
extern void func_8002A82C(MtxF matrix);
extern void matrixTranslate(f32 x, f32 y, f32 z, MtxF matrix);
extern void matrixScale(f32 x, f32 y, f32 z, MtxF matrix);
extern void func_8002A604(s16 rotation, MtxF matrix);
extern void camCopyOrthoMatrix(MtxF matrix);
extern void mtxf_mul(MtxF lhs, MtxF rhs, MtxF dest);
extern void mtxf_to_mtx(MtxF source, Mtx *dest);
extern void camDo2DSprite(Overlay47Command **commands, Mtx **matrices, void **vertices,
                        void *transform, void *sprite, s32 flags, u8 alpha);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 blend);
extern void fontUseFont(s32 font);
extern void fontPrintXY(Overlay47Command **commands, s32 x, s32 y, char *text, s32 flags);
extern void texAnimateTexSprite(void *texture, s32 *state, s32 speed, f32 *frame, s32 updateRate);
extern void mainChangeLevel(s32 level, s32 character, s32 animGroup, s32 mode, s32 arg4, s32 arg5);
extern Overlay47Save D_800D3128;
extern u8 D_8007BF68;
extern u8 D_800D31B0;
void loadFrontEndList(s16 *list);
void setupFrontEndList(s16 *list);
void joyResetMap(void);
Overlay47Actor *func_8000590C(Overlay47SpawnPacket *packet, s32 mode);
Overlay47PathObject *func_800508B4(s32 selector);
s32 frontGetMode(void);
void func_8003A754(void);
void animseqStartPath(s32 selector);
void func_80005768(Overlay47Actor *spawned);
void freeFrontEndList(s16 *assetGroup);

void func_overlay_047_F0000000_1890E18(void);
void func_overlay_047_F00009D0_18917E8(void);
void func_overlay_047_F0000B30_1891948(s32 updateRate);

#endif
