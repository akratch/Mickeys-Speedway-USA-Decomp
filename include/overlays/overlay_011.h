#ifndef OVERLAY_011_H
#define OVERLAY_011_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay11Gfx {
    u32 w0;
    u32 w1;
} Overlay11Gfx;

typedef struct Overlay11Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay11Vertex;

/* The resident game-status slot func_80028F54 returns. */
typedef struct Overlay11Status {
    u8 mode;
    u8 value1;
    u8 value2;
    u8 value3;
    u8 pad04[4];
    s32 field8;
    u8 pad0C[0x1C];
} Overlay11Status;

typedef struct Overlay11ObjectSub {
    u8 pad0;
    s8 value1;
} Overlay11ObjectSub;

typedef struct Overlay11Object {
    u8 pad00[0x64];
    Overlay11ObjectSub *sub64;
} Overlay11Object;

/* The resident menu resource table at D_8007C0B8. */
typedef struct Overlay11Resources {
    u8 pad000[0x50];
    void *resource50;
    u8 pad054[0x4];
    void *resource58;
    void *resource5C;
    u8 pad060[0x34];
    void *resource94;
    void *resource98;
    u8 pad09C[0xF4];
    s32 value190;
    char *format194;
    void *resource198;
    void *resource19C;
    void *resource1A0;
    void *resource1A4;
    void *resource1A8;
    void *resource1AC;
    u8 pad1B0[0x14];
    void *resource1C4;
    u8 pad1C8[0xBC];
    void *resource284;
    u8 pad288[0x48];
    void *resource2D0;
} Overlay11Resources;

/* Overlay-local storage, named by the relocation records' section and
 * addend. The pad index (bss +0x1C4) and the option action (data +0x1C4)
 * are different objects. Other unambiguous readers retain their existing names
 * for these words. */
extern s32 gOverlay11Timer, gOverlay11Counter;
extern s16 gOverlay11Selection;
extern s32 gOverlay11Initialized;
extern s16 gOverlay11GridPhases[13][17];
extern u8 gOverlay11GridTriangles[];
extern f32 gOverlay11Fade;
extern s16 gOverlay11OptionHighlight;
extern s16 gOverlay11HighlightStep;
/* Distinct real DATA/BSS owners keep separate consumer identities. Their
 * per-input aliases preserve the module-local displacement representation. */
extern s32 gOverlay11StorageDataMode_o011Reloc;
extern s32 gOverlay11StorageAction_o011Reloc;
extern s32 gOverlay11StorageHandleMode_o011Reloc;
extern s32 gOverlay11StoragePadIndex_o011Reloc;
extern s32 gOverlay11OptionAction;
extern s32 gOverlay11PadIndex;
extern s32 gOverlay11InputDelay;
extern s32 gOverlay11OptionDone;
extern s32 gOverlay11OptionIndex;
extern void *gOverlay11OptionHandles[6];
extern u8 gOverlay11DirectResource[];
extern void *gOverlay11Handles[4];
extern void *gOverlay11Created[6];
extern void *gOverlay11Created0;
extern void *gOverlay11Created1;
extern void *gOverlay11Created2;
extern s32 gOverlay11CreatedActive;
extern char gOverlay11Format[];
extern s32 gOverlay11ModeStart;
extern s32 gOverlay11ModeFinish;
extern s32 gOverlay11Flag;
extern void *gOverlay11Handle2;
extern void *gOverlay11Handle3;
extern s32 gOverlay11GroupsActive;
extern void *gOverlay11GroupHandles[];
extern void *gOverlay11GroupEnd3[];
extern void *gOverlay11GroupEnd4[];
extern void *gOverlay11GroupEnd6[];
extern u8 gOverlay11GameStateFlagReloc;
extern s32 gOverlay11NextModeReloc;
extern s8 D_0[];
extern s8 D_0_reload_success[];
extern s8 D_0_reload_failure[];
extern s8 D_INPUT[];
extern s8 D_menuBase[];
extern s8 D_menuCounterBase[];
extern s8 D_menuInputBase[];
extern s16 D_1B8;
extern s32 D_1BC;
extern s32 D_1C4;
extern void *D_1CC[];
extern s32 D_204;
extern s16 D_table[][4];
extern s32 D_paramA;
extern s32 D_paramB;
extern s32 D_paramC;
extern s32 D_paramD;
extern s32 D_lastMode;

#define OVERLAY11_SELECTION (*(s16 *)(D_menuBase + 0x1B8))
#define OVERLAY11_ARGUMENT (*(s32 *)(D_INPUT + 0x1C4))
#define OVERLAY11_COUNTER (*(s32 *)(D_menuCounterBase + 0x204))
/* The menu input word in initialized data, distinct from the argument word
   OVERLAY11_ARGUMENT reads at the same offset past the BSS base. */
#define OVERLAY11_MENU_INPUT (*(s32 *)(D_menuInputBase + 0x1C4))

/* Resident objects, reached through runtime relocation records. */
extern Overlay11Resources *D_8007C0B8;
extern Overlay11Gfx *D_800D3140;
extern Mtx *D_800D3144;
extern Overlay11Vertex *D_800D3148;
extern s32 D_8007C1A0;
extern s8 D_800D3190[];
extern s8 D_800D3194[];
/*
 * Two of the menu updates store these resident bytes as s8 and two as u8,
 * and the store's width type is load-bearing: a u8 store of a constant the
 * function already holds rematerializes it, an s8 store reuses the register.
 * One object, two views.
 */
typedef union Overlay11Byte {
    s8 s;
    u8 u;
} Overlay11Byte;
extern Overlay11Byte D_8007BEF8;
extern Overlay11Byte D_8007BEFC;
extern s8 D_8007BF0C;
extern u16 D_8007BF1C;
extern s32 D_8007BF44;

extern void func_800290AC(s32 mode);
extern void func_800291D8(s32 value);
extern void amTuneSetFade(f32 value, s32 arg1);
extern void amTuneSetFadeScaled(f32 value, s32 volume);
extern void overlay66Select(s32 selection);
extern Overlay11Status *func_80028F54(void);
extern void fontUseFont(s32 mode);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern void *overlay45CreateDescriptor(void *resource, s32 x, s32 y, s32 mode);
extern void overlay45ReleaseDescriptor(void *handle);
/* Overlay 45 owns this tagged descriptor; setters require its real pointer
 * contract without importing a private layout into this overlay. */
struct Overlay45ResourceDescriptor;
extern void overlay45SetMode(struct Overlay45ResourceDescriptor *descriptor,
                             s32 mode);
extern void overlay45SetField22(struct Overlay45ResourceDescriptor *descriptor,
                                s32 value);
extern s32 sprintf(char *buffer, const char *format, ...);
extern void func_80000F94(s32 soundId, void *handle);
extern u32 func_8002554C(s32 controller);
/* Overlay 45's +0x1BF4 is overlay45SetField22, with the same real contract.
 * This already-qualified import keeps its existing surface name. */
extern void func_overlay_045_F0001BF4_188E04C_o011Reloc(
    struct Overlay45ResourceDescriptor *descriptor, s32 value);
extern void func_overlay_066_F0000000(void *arg0);
extern Overlay11Object *func_80005820(s32 controller);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                          s32 arg5);
extern void func_80028528(s32 arg0);
extern void func_8003A754(void);
extern void func_800006BC(f32 arg0, s32 arg1);
extern s32 func_8002675C(void);
extern void mainChangeCameras(s32 cameraCount);
extern void mainChangeLevel(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                            s32 arg5);
extern void amSndPlay(s32 soundId, void *handle);
extern u32 joyGetPressed(s32 controller);
extern s32 levelGetNumber(void);

/* The menu background draw names its resident callees by their generated
 * relocation-surface placeholders. */
extern s32 func_800290A0_o011Reloc(void);
extern s32 mathRnd_o011Reloc(s32 minimum, s32 maximum);
extern void viGetCurrentSize_o011Reloc(u32 *width, u32 *height);
extern void camStandardOrtho_o011Reloc(Overlay11Gfx **displayList, Mtx **matrix);
extern void texDPTextureX_o011Reloc(Overlay11Gfx **displayList, void *texture,
                                    s32 flags, s16 parameter);
extern f32 func_8002A8C0_o011Reloc(s32 angle);
extern u32 joyGetPressed_o011Reloc(s32 controller);
extern void func_800290AC_o011Reloc(s32 mode);
extern void func_800291D8_o011Reloc(s32 value);
extern void amTuneSetFadeScaled_o011Reloc(f32 value, s32 volume);
extern Overlay11Status *func_80028F54_o011Reloc(void);

/*
 * Every call to the exported overlay11ReleaseCurrentGroup is a SYMBOL
 * relocation record: the shipped word stores the 0xF0000000 addend, not the
 * callee's offset, so the calls name this undefined placeholder until the
 * overlay tables are linker output.
 */
extern void overlay11ReleaseCurrentGroup_o011Reloc(void);

/* A bare statement list, not a `do { } while (0)` macro: the region that
 * wrapper opens is what stopped IDO unrolling the vertex-row loop. */
#define O11_WRITE_VERTEX(vertexX, vertexY, vertexAlpha) \
        D_800D3148->x = (vertexX); \
        D_800D3148->y = (vertexY); \
        D_800D3148->z = 0; \
        D_800D3148->r = 0; \
        D_800D3148->g = 0; \
        D_800D3148->b = 0; \
        D_800D3148->a = (vertexAlpha); \
        D_800D3148++

#endif
