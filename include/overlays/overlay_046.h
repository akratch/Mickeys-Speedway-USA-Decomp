#ifndef OVERLAY_046_H
#define OVERLAY_046_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "game/math.h"

typedef struct Overlay46Resource {
    u8 pad00[0x3C];
    s32 value3C;
} Overlay46Resource;

typedef struct Overlay46Descriptor {
    s16 type;
    s16 pad02;
    s16 value4;
    s16 value6;
    s16 value8;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay46Descriptor;

extern s16 gOverlay46Ids[];

extern s32 gOverlay46Value0;

extern s32 gOverlay46Value4;

extern s32 gOverlay46Value8;

extern void *gOverlay46Value2C;

extern void *gOverlay46Resource4C;

extern void *gOverlay46Resource50;

extern void *gOverlay46Group54;

extern s32 gOverlay46Mode58;

extern s32 gOverlay46Timer5C;

extern Overlay46Resource *gOverlay46Resource184;

extern void *gOverlay46Value1E4;

extern void overlay46ReleaseIdsReloc(s16 *ids);

extern void *func_80036DD0(s32 kind);

extern void *overlay44CreateAnimationState(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                                       s32 arg4);

extern void func_80037414(s32 arg0, f32 arg1, f32 arg2, s32 arg3,
                                    s32 arg4, s32 arg5, s32 arg6);

extern Overlay46Resource *func_8000590C(
    Overlay46Descriptor *descriptor, s32 arg1);

extern void overlay46SetModeReloc(s32 mode);

extern void func_800015F8(void);

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

typedef struct Overlay46Progress {
    u8 pad00[3];
    u8 alpha;
} Overlay46Progress;

typedef struct Overlay46Counter {
    u8 pad00[4];
    s32 value;
} Overlay46Counter;

extern u8 D_60[];

extern u8 D_C0[];

extern u8 D_120[];

extern Overlay46Progress *gOverlay46Group54Render;

extern Overlay46Counter *gOverlay46Group54Update;

extern u8 *gOverlay46ExpectedName180;

typedef struct Overlay46StatusFlags {
    u32 unused0 : 4;
    u32 nameMatched : 1;
} Overlay46StatusFlags;

extern Overlay46StatusFlags D_800D3128;

extern Overlay46DisplayCommand *gDisplayListHead;

extern Overlay46DisplayCommand *gDisplayListHeadB;

extern u8 gOverlay46MotionTarget[];

extern s32 D_8007BF44;

extern s32 D_8007BF48;

extern s32 func_80037664(void);

extern void func_80037658(void);

/* Cross-overlay and resident callees are reached through runtime relocation
 * records; the three overlay 44 and 99 entry points are named as placeholders. */

extern void overlay99InitializeEntries(s32 mode, void *resource,
                                               f32 scale, f32 value);

extern s32 mathRnd(s32 minimum, s32 maximum);

extern void func_80000F94(s32 soundId, void *handle);

extern void overlay44UpdateFrameCache(void *state, s32 updateRate);

extern void *func_80058240(void);

extern void func_800291B4(void);

extern void func_8003A680(s32 value);

extern void func_80001608(void);

extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                          s32 arg5);

extern void func_80028528(s32 value);

extern void func_80028D30(s32 value);

extern void func_80036F08(Overlay46DisplayCommand **commands, void *resource,
                          s32 arg2);

extern void func_overlay_044_F0000580_188BDE0(void *state, void *target,
                                              f32 scale);

extern void fontUseFont(s32 mode);

extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);

extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);

extern s32 func_overlay_046_F000069C_188EA94(void);


#define O46_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define O46_PRIM(packet, red, green, blue, alpha) { \
    Overlay46DisplayCommand *macroCommand = (Overlay46DisplayCommand *)(packet); \
    macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8); \
    macroCommand->w1 = O46_SHIFTL(red, 24, 8) | \
        O46_SHIFTL(green, 16, 8) | O46_SHIFTL(blue, 8, 8) | \
        O46_SHIFTL(alpha, 0, 8); \
}

typedef struct Overlay46Resource_ReleaseState Overlay46Resource_ReleaseState;

typedef struct Overlay46Group Overlay46Group;

typedef struct Overlay46Data {
    s16 resourceIds[0x26];
    Overlay46Resource_ReleaseState *resource4C;
    Overlay46Resource_ReleaseState *resource50;
    Overlay46Group *group54;
    u8 pad58[0x12C];
    Overlay46Resource_ReleaseState *resource184;
} Overlay46Data;

extern Overlay46Data gOverlay46Data;

extern void overlay44ReleaseHandles(Overlay46Group *group);

extern void mmFree(Overlay46Resource_ReleaseState *resource);

extern void func_80006EA0(Overlay46Resource_ReleaseState *resource);

extern u8 frontRecallScreenMode(void);

typedef struct Overlay46Particle {
    s16 value00;
    s16 value02;
    s16 value04;
    s16 angle06;
    f32 scale08;
    f32 baseX0C;
    f32 baseY10;
    f32 value14;
    s16 angle18;
    s16 angle1A;
    f32 positionX1C;
    f32 positionY20;
    f32 value24;
    f32 value28;
    f32 targetX2C;
    f32 targetY30;
    s16 value34;
    s16 variant36;
    void *resource38;
} Overlay46Particle;

typedef struct Overlay46ParticleConfig {
    u8 resourceIndex;
    u8 variant;
    s8 x;
    s8 y;
} Overlay46ParticleConfig;

extern void *D_800D31C8[];

extern s32 gOverlay46StateC;

extern s32 gOverlay46Value10;

extern s32 gOverlay46Value14;

extern s32 gOverlay46Value18;

extern Overlay46Particle gOverlay46Particles20[];


extern Overlay46ParticleConfig gOverlay46Configs1C8[];

extern f32 overlay46RandomSignedReloc(s32 magnitude);

extern s32 func_800299E8(s32 minimum, s32 maximum);

typedef struct Overlay46Particle_F0000874_188 {
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
} Overlay46Particle_F0000874_188;

extern s32 D_10;

extern s32 D_14;

extern s32 D_18;

extern Overlay46Particle_F0000874_188 D_20[];

extern u8 D_2C[];

extern s32 D_5C;

extern s32 D_C;

/* Resident display-list, matrix and vertex cursors, reached through runtime
 * relocation records and passed by address. */

extern Overlay46DisplayCommand *D_800D3140;

extern void *D_800D3144;

extern void *D_800D3148;

extern void texDPInit(void *commands);

extern void camStandardOrtho(void *commands, void *matrices);


extern void fontPrintXY(void *commands, s32 x, s32 y, void *text,
                          s32 size);

extern void func_8002F618(void *commands, void *data, s32 x, s32 y,
                          s32 red, s32 green, s32 blue, s32 alpha);

extern void camDo2DSprite(void *commands, void *matrices, void *vertices,
                          Overlay46Particle_F0000874_188 *particle, void *resource,
                          s32 flags, s32 alpha);

extern s32 gOverlay46Enabled;

extern s32 gOverlay46Limit;

extern u8 gOverlay46StateA[];

extern u8 gOverlay46StateB[];

extern s16 gOverlay46Selection;

extern void overlay46InitResourceReloc(void *state);


extern u8 gOverlay46DisplayState[];

extern u8 gOverlay46DisplayOutput[];

extern s32 gOverlay46FadeOutput;

extern void camSetView(void *state, s32 mode);

extern void func_80022B94(void *state, void *output);

extern s32 overlay41IsUnitScale(s32 index);

extern void frontDrawObj(s32 index);


/* Generic submission wrapper only; no exact DKR donor. */

extern unsigned char gOverlay46SubmitBlock[];

extern void freeFrontEndList(void *block);

typedef struct Overlay46State {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    s16 value10;
    u8 pad12[2];
    s32 value14;
} Overlay46State;

typedef struct Overlay46ColorEntry {
    u8 pad0[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay46ColorEntry;

typedef struct Overlay46BufferSlot {
    u8 pad00[0x14];
    s16 value14;
    u8 pad16[0x16];
    s16 value2C;
    u8 pad2E[0x16];
    s16 value44;
    u8 pad46[0x16];
    s16 value5C;
    u8 pad5E[2];
} Overlay46BufferSlot;

extern Overlay46State *gOverlay46State;

extern Overlay46State gOverlay46StateStorage;

extern f32 gOverlay46InitialX;

extern f32 gOverlay46InitialY;

extern f32 gOverlay46InitialW;

extern Overlay46ColorEntry gOverlay46ColorEntries[];

extern s16 gOverlay46InitialValue;

extern s16 gOverlay46Value;

extern Overlay46BufferSlot gOverlay46BufferSlots[];

extern Overlay46BufferSlot gOverlay46BufferSlotsEnd[];

extern s16 gOverlay46BufferEnd;

typedef struct Overlay46Particle_F0001228_188 {
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
} Overlay46Particle_F0001228_188;

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

extern void amSndPlay(s32 soundId, void *handle);


extern void camDoSprite(void *commands, void *matrices, void *vertices,
                          Overlay46DrawPoint *particle, void *resource,
                          s32 flags, s32 alpha);

extern void texDPTextureX(void *commands, void *texture, s32 flags, s32 parameter);

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

#endif
