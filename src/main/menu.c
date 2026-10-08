/*
 * Resident front-end menu -- ROM 0x39350-0x3B1A0 (VRAM 0x80038750).
 *
 * The translation-unit identity and function crosswalk come from Jet Force
 * Gemini's public decompilation of the same Rare engine. The boundary evidence
 * and provenance are recorded in docs/modules.md section 3.4. Functions stay
 * under GLOBAL_ASM until their C compiles to Mickey's bytes exactly.
 *
 * Flags: -O2 -mips2 -32 (the game-code preset, selected by Makefile). The
 * -Wo,-loopunroll,0 override this TU once carried was measured byte-inert for
 * every function except func_80038878, whose target is unrolled; it is gone.
 */

#include "PR/ultratypes.h"
#include "game/font.h"
#include "game/gameVi.h"
#include "game/menu.h"
#include "game/pi.h"
#include "n_audio/mbi.h"

extern s8 D_800D312B;
extern s8 D_800D3050;
extern MenuScreenModeState D_800D3128;
extern u8 D_8007C08C;
extern u8 D_8007C090;
extern u8 D_8007C088;
extern s32 D_8007C098;
extern s32 *D_8007C094;
extern s16 D_8007BF70;
extern s16 D_8007BF7C[];
extern u8 D_8007BEF4;
extern u8 D_8007BEF8;
extern u8 D_8007BF30;
extern u8 D_8007BF34;
extern u8 D_8007C0A0;
extern s32 D_8007C09C;
extern s32 D_8007C1A4;
extern s32 D_8007C1AC;
extern u8 D_8007C308[];
extern s32 D_800C947C;
extern s32 D_800D314C;
extern s32 D_800D3150[];
extern s32 D_800D3168[];
extern u8 D_800826C0[];
extern u8 D_800826D0[];
extern u16 D_800D312A;
extern u16 D_800D312C;
extern u16 D_800D312E;
extern void amTuneStop(void);
extern void amTuneSetGlobalVolume(s32 volume);
extern void alSurround_OutputType(u8 mode);
extern void func_80038750();
extern void func_800389CC(void);
extern void func_80038BC4(void);
extern void func_8003968C(void);
extern s32 levelGetRegionNo(void);
extern s8 viGetWideAdjust(void);
extern void gsSndpSetGlobalVolume(s32 volume);
extern void viSetWideAdjust(s32 offset);
extern s32 TrapDanglingJump();
extern void amSndPlay(s32 soundId, s32 *handle);
extern void amSndSetVol(s32 soundId, s32 handle, s32 volume, s32 *handleOut);
extern void amSndStop(s32 handle);
extern void func_80000510(u8 sequence);
extern void func_80006EA0(void *object);
extern void func_80006FA0(void);
extern s32 func_80005820(s32 arg0);
extern u8 *func_80028F54(void);
extern void func_800347A0(void *texture);
extern void func_800359D4(void *sprite);
extern void func_80034920(MenuCommand **displayList);
extern void freeFrontEndItem(s32 assetId);
extern void loadFrontEndItem(s32 assetId);
extern void func_80039720(s32 updateRate);
extern void func_8004BF64(s32 windowId);
extern void func_80044BC8(s32 arg0, u8 *source, s32 line);
extern u32 joyGetButtons(s32 controller);
extern u32 joyGetPressed(s32 controller);
extern s8 joyGetStickX(s32 controller);
extern s8 joyGetStickY(s32 controller);
extern void mainTitlePageInit(s32 mode);
extern void modFreeModel(void *model);
extern void *func_8002B280(s32 size, s32 tag);
extern u32 *piRomLoad(u32 assetIndex);

extern u32 D_800D3170[4];
extern u32 D_800D3180[4];
extern s8 D_800D3190[4];
extern s8 D_800D3194[4];
/* The repeat-state carve is owned here in target address order. */
s8 D_800D3198;
s8 D_800D3199;
s8 D_800D319A;
s8 D_800D319B;
s8 D_800D319C;
s8 D_800D319D;
s8 D_800D319E;
s8 D_800D319F;
u32 D_800D31A0;
u32 D_800D31A4;
u32 D_800D31A8;
u32 D_800D31AC;
u8 D_800D31B0;
s32 D_800D31B4;
s32 D_800D31B8;
s16 D_800D31BC;
s16 D_800D31BE;
s16 D_800D31C0;
s16 D_800D31C2;
extern s8 menuRepeatX[4];
extern s8 menuRepeatY[4];
extern u32 menuPreviousButtons[4];
#pragma weak menuRepeatX = D_800D3198
#pragma weak menuRepeatY = D_800D319C
#pragma weak menuPreviousButtons = D_800D31A0
#define MENU_REPEAT_X menuRepeatX
#define MENU_REPEAT_Y menuRepeatY
#define MENU_PREVIOUS_BUTTONS menuPreviousButtons
#define MENU_STICK_Y_TOTAL D_800D31C2
/* The per-player play-choice bytes are DEFINED here, not extern: the target
 * stores all four through one `lui $at`, which IDO emits only for an object
 * it owns (an extern u8[4], or an extern 4-aligned union, gets four separate
 * `lui`s -- both measured). JFG's src/menu.c reached the same conclusion for
 * its `playChoice`. IDO places the 4 bytes at .bss+0x2C, inside the 0x30-byte
 * carve this TU already owns, so the section does not grow; the retail link
 * put the object at 0x800D3044, which the bss gap object still labels, so
 * the Makefile weakens this definition and the gap's label wins. */
u8 D_800D3044[4];
extern u8 D_800D3498[];

struct MenuCommand {
    u32 w0;
    u32 w1;
};

extern MenuCommand D_8007C2E8[];

/* Historical display-list macro topology is significant to IDO's register
 * allocation in the rectangle batcher. */
#define MENU_COMMAND(pkt, a, b) { \
    MenuCommand *_g = (MenuCommand *)(pkt); \
    _g->w0 = (u32)(a); \
    _g->w1 = (u32)(b); \
}

/* The same packet written through the function's one `command` carrier. */
#define MENU_COMMAND_CARRIER(a, b) { \
    command = D_800D3140; \
    D_800D3140 = command + 1; \
    command->w0 = (a); \
    command->w1 = (b); \
}

typedef struct MenuRectangle {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 colour;
} MenuRectangle;

typedef struct MenuCurrentObject {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 index;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    s8 pad1C[4];
} MenuCurrentObject;

typedef struct MenuObjectResource {
    u8 pad0[0x4E];
    s8 unk4E;
    u8 pad4F[0x19];
    void *unk68;
} MenuObjectResource;

typedef struct MenuFrontObject {
    MenuObjectResource *resource;
    s32 unk4;
    u8 pad8[2];
    s16 indexA;
    s32 unkC[1];
} MenuFrontObject;

typedef struct MenuSpawnPacket {
    /* 0x00 */ s16 kind;
    /* 0x02 */ u8 mode;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 flags;
    /* 0x0C */ u8 unkC;
} MenuSpawnPacket;

typedef struct MenuSpawnHeader {
    /* 0x00 */ u8 pad0[0x22];
    /* 0x22 */ s8 count;
} MenuSpawnHeader;

typedef struct MenuSpawnInner {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s16 mode;
} MenuSpawnInner;

typedef struct MenuSpawnedObject {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ MenuSpawnHeader *header;
    /* 0x44 */ u8 pad44[0x24];
    /* 0x68 */ MenuSpawnInner **inner;
} MenuSpawnedObject;

extern void *func_80034448(s32 assetId);
extern void *func_800355A0(s32 assetId, s32 arg1);
extern MenuSpawnedObject *func_8000590C(MenuSpawnPacket *packet, s32 mode);
extern void *func_8001F520(s32 assetId, s32 arg1);

typedef struct MenuDrawStack {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    u8 pad82[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad94[0x10];
    f32 frame;
    u8 padA8[4];
    MenuFrontObject *savedObject;
    u8 padB0[8];
} MenuDrawStack;

extern u8 D_8007C0A4;
extern u8 D_8007C0A8;
extern u8 D_8007C0AC;
extern s32 D_8007C0B4;
extern s32 D_8007C0BC;
extern u8 D_8007C07C;
extern s16 *D_8007C1B8;
extern s16 D_8007C1BC;
extern s16 D_8007C1C0;
extern MenuCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D3148;
extern MenuFrontObject *D_800D31C8[];
extern MenuCurrentObject D_8007C1C4[];
extern MenuCurrentObject D_800D3550[];
typedef struct MenuLanguageText {
    u8 pad0[0x30];
    char *demoMessage;
} MenuLanguageText;
extern s32 *D_8007C0B8;
extern void func_8004B0A4(s32 font);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8004B0F8(MenuCommand **displayList, s32 x, s32 y,
                          char *text, s32 alignmentFlags);
extern void func_80009E78(MenuCommand **commands, void **matrices,
                          void **vertices, void *object);
extern void func_80023F84(MenuCommand **commands, void **matrices,
                          void **vertices, void *transform, void *object,
                          s32 arg5, s32 arg6);
extern void camPushModelMtx(MenuCommand **commands, void **matrices,
                          void *transform, f32 scale, f32 extra);
extern void camPopModelMtx(MenuCommand **commands);

/* PROVENANCE: body adapted from JFG src/menu.c; Mickey byte identity is decisive. */
/* The relocation loop re-caches the table base into `destination` each
 * iteration: the CDX trace showed `destination` carries the piRomLoadSection
 * a1-argument affinity, so the base web rides a1 while the element stays a
 * junior expression temp on a0, and the `-1 ==` spelling keeps the hoisted
 * constant first in the compare (allocator-trace-guided). The jump table is
 * the TU's own first rodata entry, carved at ROM 0x83334. */
void func_80038750(s32 language) {
    s32 *header;
    s32 *offsets;
    s32 assetIndex;
    s32 destination;
    s32 end;
    s32 index;

    header = D_8007C094;
    if (header == NULL) {
        D_8007C094 = (s32 *) piRomLoad(7);
        header = D_8007C094;
    }
    assetIndex = 1;
    switch (language) {
        case 5:
            assetIndex = 6;
            break;
        case 4:
            assetIndex = 5;
            break;
        case 3:
            assetIndex = 4;
            break;
        case 2:
            assetIndex = 3;
            break;
        case 1:
            assetIndex = 2;
            break;
        default:
            assetIndex = 1;
            break;
    }
    offsets = (s32 *) ((u8 *) header + (assetIndex * 4));
    end = offsets[1];
    assetIndex = offsets[0];
    destination = (s32) D_8007C0B8;
    end -= assetIndex;
    if (destination != 0) {
        piRomLoadSection(6, destination, assetIndex, end);
        index = 0;
        while (index < D_8007C094[0]) {
            destination = (s32) D_8007C0B8;
            if (-1 == ((s32 *) destination)[index]) {
                ((s32 *) destination)[index] = 0;
            } else {
                ((s32 *) destination)[index] =
                    ((s32 *) destination)[index] + destination;
            }
            index++;
        }
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's permitted published decomp,
 * src/menu.c::initFront, as matched in pull request #37 at head
 * d45123d1c528955d5e12ddad805076267a690d76 (the PR is unmerged upstream, so
 * the SHA is the citation). Correspondence: same call sequence (allocate the
 * per-player table, allocate and load the language text, clear the 180-entry
 * front-end object pointer table, load asset 0x1A and count it to -1, zero
 * the per-asset loaded flags, set every play choice to 2); Mickey's constants
 * are its own (6 x 0xF4 records against JFG's 3 x 0x538) and Mickey adds the
 * trailing joystick-mask reset. The [i - 1] initial read at i == 0 is the
 * donor's spelling and is what the bytes do.
 *
 * Both `for`/`while` loops over the pointer table and the play choices are
 * unrolled in the target (4 stores per iteration; fully), so this TU compiles
 * WITHOUT -Wo,-loopunroll,0: measured byte-inert for every other function in
 * menu.c and required by this one. */
void func_80038878(void) {
    s32 i;

    D_800D3150[0] = (s32) func_8002B280(0x5B8, 0x8F);
    for (i = 0; i < 6; i++) {
        D_800D3150[i] = D_800D3150[i - 1] + 0xF4;
    }
    D_8007C0B8 = func_8002B280(0x1000, 0x8F);
    func_80038750(0);
    i = 0;
    while (i < 180) {
        D_800D31C8[i] = NULL;
        i++;
    }
    D_8007C088 = 0;
    D_8007C1B8 = (s16 *) piRomLoad(0x1A);
    D_8007C1BC = 0;
    while (D_8007C1B8[D_8007C1BC] != -1) {
        D_8007C1BC++;
    }
    D_8007C1C0 = 0;
    for (i = 0; i < D_8007C1BC; i++) {
        D_800D3498[i] = 0;
    }
    i = 0;
    while (i < 4) {
        D_800D3044[i++] = 2;
    }
    D_800D31B0 = 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontFreeMode;
 * Mickey supplies the smaller 19-mode switch and exact resident state. */
void func_800389CC(void) {
    u8 *selection;
    u8 value;

    if (D_8007C088 != 0) {
        selection = func_80028F54();
        switch (D_8007C0A0) {
        case 0:
            func_8004BF64(1);
            break;
        case 1:
            break;
        case 2:
            TrapDanglingJump();
            break;
        case 6:
            TrapDanglingJump();
            break;
        case 3:
            TrapDanglingJump();
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump();
            break;
        case 9:
        case 11:
            TrapDanglingJump();
            break;
        case 7:
            TrapDanglingJump();
            break;
        case 8:
            TrapDanglingJump();
            break;
        case 5:
            value = *selection;
            if ((value == 5) || (value == 6)) {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump();
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump();
                } else {
                    TrapDanglingJump();
                }
            } else {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump();
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump();
                } else {
                    TrapDanglingJump();
                }
            }
            TrapDanglingJump();
            break;
        case 13:
            TrapDanglingJump();
            break;
        case 14:
            TrapDanglingJump();
            break;
        case 15:
            TrapDanglingJump();
            break;
        case 16:
            TrapDanglingJump();
            break;
        }
        if (D_8007C1A4 != 0) {
            amSndStop(D_8007C1A4);
        }
        D_8007C1AC = 0;
        D_8007C088 = 0;
    }
}
/* PROVENANCE: role and switch ordering compared with JFG's public
 * src/menu.c::frontInitMode; JFG retains assembly, and this body is derived
 * from Mickey's state and call surface. */
void func_80038BC4(void) {
    u8 *selection;
    u8 value;

    if (D_8007C088 == 0) {
        selection = func_80028F54();
        switch (D_8007C0A0) {
        case 0:
        case 1:
            break;
        case 2:
            TrapDanglingJump();
            break;
        case 6:
            TrapDanglingJump();
            break;
        case 3:
            D_800D31B0 = 1;
            TrapDanglingJump();
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump();
            break;
        case 9:
        case 11:
            TrapDanglingJump();
            break;
        case 7:
            TrapDanglingJump();
            break;
        case 8:
            TrapDanglingJump();
            break;
        case 5:
            value = *selection;
            if ((value == 5) || (value == 6)) {
                switch (D_8007BEF4) {
                case 1:
                    TrapDanglingJump();
                    break;
                case 2:
                    TrapDanglingJump();
                    break;
                default:
                    TrapDanglingJump();
                    break;
                }
            } else {
                switch (D_8007BEF4) {
                case 1:
                    TrapDanglingJump();
                    break;
                case 2:
                    TrapDanglingJump();
                    break;
                default:
                    TrapDanglingJump();
                    break;
                }
            }
            TrapDanglingJump();
            break;
        case 13:
            TrapDanglingJump();
            break;
        case 14:
            TrapDanglingJump();
            break;
        case 15:
            TrapDanglingJump();
            break;
        case 16:
            TrapDanglingJump();
            break;
        }
        D_8007C088 = 1;
        D_8007BF70 = -1;
    }
}
/* PROVENANCE: name, role, call order, and state resets compared with JFG's
 * public src/menu.c::frontSetMode; Mickey supplies the exact state surface. */
void frontSetMode(s32 mode) {
    func_800389CC();
    D_8007C0A0 = mode;
    func_80038BC4();
    func_8003968C();
    D_8007BF30 = 0;
    D_8007BF34 = 1;
    if (mode == 0) {
        D_8007BEF8 = 1;
        D_8007BEF4 = 1;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetMode. */
u8 frontGetMode(void) {
    return D_8007C0A0;
}
/* PROVENANCE: adapted from Jet Force Gemini's permitted published decomp,
 * src/menu.c::frontUpdate, as matched in pull request #37 at head
 * d45123d1c528955d5e12ddad805076267a690d76 (unmerged upstream; the SHA is the
 * citation). Correspondence: the same frame -- poll the joysticks
 * (func_80039720 is JFG's func_80059A98), latch the four display-list
 * cursors into the front-end globals, diRcpTrace in, dispatch the current
 * front-end mode through one switch whose `case 0: break;` is the donor's
 * (it widens the jump table to 19 entries), trace out, write the cursors
 * back and clear `disable`. Mickey's additions are its own: the title-music
 * fade, the demo/attract timer with its any-pad-START check, the early
 * `return 0` when the overlay trap reports the frame consumed, and the fade
 * countdown that plays a tune through func_80000510 (JFG's amTunePlay, one
 * argument -- the other Mickey callers agree). The locals keep their target
 * homes in declaration order: the four joyGetPressed halves and the two
 * call results are the whole 0x28 frame. */
s32 func_80038E1C(s32 *gfxList, s32 *mtxList, s32 *vertexList, s32 *triangleList, s32 updateRate) {
    s32 timerState;
    u8 *selection;
    u16 pressed0;
    u16 pressed1;
    u16 pressed2;
    s32 playerObject;
    u8 mode;

    selection = func_80028F54();
    if (D_800C947C != 0) {
        if (D_8007C1A4 == 0) {
            amSndPlay(0xA, &D_8007C1A4);
        }
        D_8007C1AC = 0x7F;
    } else if (D_8007C1AC > 0) {
        D_8007C1AC -= updateRate * 3;
        if (D_8007C1AC <= 0) {
            if (D_8007C1A4 != 0) {
                amSndStop(D_8007C1A4);
            }
        } else {
            amSndSetVol(0xA, D_8007C1A4, D_8007C1AC & 0xFF, &D_8007C1A4);
        }
    }
    func_80039720(updateRate);
    if (TrapDanglingJump() != 0) {
        return 0;
    }
        if (func_8003A550() != 0) {
            func_8003A544(0);
            D_8007C09C -= updateRate;
            if ((D_8007C09C < 0) ||
                (pressed2 = joyGetPressed(2), pressed1 = joyGetPressed(1),
                 pressed0 = joyGetPressed(0),
                 ((joyGetPressed(3) | pressed0 | pressed1 | pressed2) & 0x9000) != 0)) {
                mainTitlePageInit(1);
                D_8007C09C = 0x4B0;
            } else {
                func_8003A544(1);
            }
        }
        playerObject = func_80005820(0);
        D_800D3140 = *gfxList;
        D_800D3144 = *mtxList;
        D_800D3148 = *vertexList;
        timerState = playerObject;
        D_800D314C = *triangleList;
        func_80044BC8(D_800D3140, D_800826C0, 0x297);
        switch (D_8007C0A0) {
        case 0:
            break;
        case 2:
            TrapDanglingJump(updateRate);
            break;
        case 6:
            TrapDanglingJump(updateRate);
            break;
        case 3:
            TrapDanglingJump(updateRate);
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump(updateRate);
            break;
        case 9:
        case 11:
            TrapDanglingJump(updateRate);
            break;
        case 7:
            TrapDanglingJump(updateRate);
            break;
        case 8:
            TrapDanglingJump(updateRate);
            break;
        case 5:
            mode = *selection;
            if ((mode == 5) || (mode == 6)) {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump(timerState, updateRate);
                    TrapDanglingJump(updateRate);
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump(updateRate);
                } else {
                    TrapDanglingJump(updateRate);
                }
            } else if (D_8007BEF4 == 1) {
                TrapDanglingJump(timerState, updateRate);
                TrapDanglingJump(updateRate);
            } else if (D_8007BEF4 < 3) {
                TrapDanglingJump(updateRate);
            } else {
                TrapDanglingJump(updateRate);
            }
            break;
        case 13:
            TrapDanglingJump(updateRate);
            break;
        case 14:
            TrapDanglingJump(updateRate);
            break;
        case 15:
            TrapDanglingJump((s32)&D_800D3140, (s32)&D_800D3144, &D_800D3148, updateRate);
            break;
        case 16:
            TrapDanglingJump(updateRate);
            break;
        }
        func_80044BC8(D_800D3140, D_800826D0, 0x2C5);
        *gfxList = D_800D3140;
        *mtxList = D_800D3144;
        *vertexList = D_800D3148;
        *triangleList = D_800D314C;
        D_8007BF34 = 0;
        if (D_8007BF70 != -1) {
            D_8007BF70 -= updateRate;
            if (D_8007BF70 <= 0) {
                D_8007BF70 = -1;
                func_80000510(D_800D3050);
            }
        }
    return 0;
}
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::frontDemoMessage; the body is derived from Mickey. */
void frontDemoMessage(MenuCommand **displayList, s32 updateRate) {
    s32 x;
    s32 y;

    D_8007C07C += updateRate;
    if (D_8007C07C & 0x10) {
        x = 0xA0;
        y = 0xD0;
        viConvertXY(&x, &y);
        func_80038750(frontGetLanguage());
        func_8004B0A4(2);
        func_8004B0DC(0, 0, 0, 0);
        fontColour(0, 0, 0, 0xFF, 0xFF);
        func_8004B0F8(displayList, x + 1, y + 1,
                      ((MenuLanguageText *) D_8007C0B8)->demoMessage, 0xC);
        fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
        func_8004B0F8(displayList, x, y,
                      ((MenuLanguageText *) D_8007C0B8)->demoMessage, 0xC);
    }
}
/* PROVENANCE: adapted from JFG's public frontDrawRectangles assembly in
 * asm/nonmatchings/menu/frontDrawRectangles.s; Mickey supplies the exact
 * render-state commands, clipping order, and smaller loop schedule. */
void frontDrawRectangles(MenuCommand **displayList, s32 count, MenuRectangle *rectangles, s32 translucent) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    u32 currentColour;
    u32 width;
    u32 height;
    u32 colour;
    u32 primitiveCommand;

    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    gSPDisplayList((*displayList)++, D_8007C2E8);
    primitiveCommand = 0xFA000000;
    if (translucent != 0) {
        MENU_COMMAND((*displayList)++, 0xEF082C0F, 0x00504340);
    } else {
        MENU_COMMAND((*displayList)++, 0xEF082C0F, 0x00504240);
    }
    currentColour = rectangles->colour;
    MENU_COMMAND((*displayList)++, primitiveCommand, currentColour);
    while (count--) {
        right = rectangles->right;
        bottom = rectangles->bottom;
        if ((right >= 0) && (bottom >= 0)) {
            left = rectangles->left;
            top = rectangles->top;
            if ((left < (s32) width) && (top < (s32) height)) {
                if (left < 0) {
                    left = 0;
                }
                if (top < 0) {
                    top = 0;
                }
                if ((s32) width < right) {
                    right = (s32) width;
                }
                if ((s32) height < bottom) {
                    bottom = (s32) height;
                }
                colour = rectangles->colour;
                if (currentColour != colour) {
                    currentColour = colour;
                    gDPPipeSync((*displayList)++);
                    MENU_COMMAND((*displayList)++, primitiveCommand, colour);
                }
                gDPFillRectangle((*displayList)++, left, top, right, bottom);
            }
        }
        rectangles++;
    }
    func_80034920(displayList);
    MENU_COMMAND((*displayList)++, primitiveCommand, -1);
}
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontDrawRectangle; body and rectangle layout derived from Mickey. */
void frontDrawRectangle(MenuCommand **displayList, s32 left, s32 top, s32 right, s32 bottom, u32 colour) {
    MenuRectangle rectangle;

    rectangle.left = left;
    rectangle.top = top;
    rectangle.right = right;
    rectangle.bottom = bottom;
    rectangle.colour = colour;
    frontDrawRectangles(displayList, 1, &rectangle, 1);
}
/* PROVENANCE: name and screen-limit role compared with JFG's public decomp,
 * src/menu.c::frontPlayerScreenLimits; body and table indexing derived from Mickey. */
void frontPlayerScreenLimits(s32 player, s32 *left, s32 *top, s32 *right, s32 *bottom) {
    s32 offset;
    s16 *limits;

    offset = (((D_8007BEF4 - 1) << 2) + player) << 2;
    if (D_8007BEF4 == 2 && frontGet2PlayerSplit()) {
        offset += 0x40;
    }
    limits = &D_8007BF7C[offset];
    *left = limits[0];
    *top = limits[1];
    viConvertXY(left, top);
    *right = limits[2];
    *bottom = limits[3];
    viConvertXY(right, bottom);
}
void func_8003968C(void) {
    s32 controller;

    for (controller = 0; controller < 4; controller++) {
        MENU_PREVIOUS_BUTTONS[controller] = -1;
        MENU_REPEAT_X[controller] = 20;
        MENU_REPEAT_Y[controller] = 15;
    }
}
void func_80039720(s32 updateRate) {
    s32 controller;
    s8 repeatXNegative;
    s8 repeatXPositive;
    s8 repeatYNegative;
    s8 repeatYPositive;
    s8 axisX;
    s8 axisY;

    D_800D31B8 = 0;
    D_800D31B4 = 0;
    D_800D31C0 = 0;
    MENU_STICK_Y_TOTAL = 0;
    D_800D31BC = 0;
    D_800D31BE = 0;
    for (controller = 0; controller != 4; controller++) {
        D_800D3170[controller] = joyGetButtons(controller);
        D_800D3180[controller] =
            D_800D3170[controller] & ~MENU_PREVIOUS_BUTTONS[controller];
        MENU_PREVIOUS_BUTTONS[controller] = D_800D3170[controller];
        D_800D3190[controller] = joyGetStickX(controller);
        axisX = D_800D3190[controller];
        if (axisX < -0x23) {
            repeatXNegative = MENU_REPEAT_X[controller];
            if (repeatXNegative < 0) {
                MENU_REPEAT_X[controller] = repeatXNegative + updateRate;
                if (MENU_REPEAT_X[controller] >= 0) {
                    MENU_REPEAT_X[controller] = -0xF;
                } else {
                    D_800D3190[controller] = 0;
                }
            } else {
                MENU_REPEAT_X[controller] = -0x14;
            }
        } else if (axisX >= 0x24) {
            repeatXPositive = MENU_REPEAT_X[controller];
            if (repeatXPositive > 0) {
                MENU_REPEAT_X[controller] = repeatXPositive - updateRate;
                if (MENU_REPEAT_X[controller] <= 0) {
                    MENU_REPEAT_X[controller] = 0xF;
                } else {
                    D_800D3190[controller] = 0;
                }
            } else {
                MENU_REPEAT_X[controller] = 0x14;
            }
        } else {
            D_800D3190[controller] = 0;
            MENU_REPEAT_X[controller] = 0;
        }
        D_800D3194[controller] = joyGetStickY(controller);
        axisY = D_800D3194[controller];
        if (axisY < -0x23) {
            repeatYNegative = MENU_REPEAT_Y[controller];
            if (repeatYNegative < 0) {
                MENU_REPEAT_Y[controller] = repeatYNegative + updateRate;
                if (MENU_REPEAT_Y[controller] >= 0) {
                    MENU_REPEAT_Y[controller] = -0xF;
                } else {
                    D_800D3194[controller] = 0;
                }
            } else {
                MENU_REPEAT_Y[controller] = -0x14;
            }
        } else if (axisY >= 0x24) {
            repeatYPositive = MENU_REPEAT_Y[controller];
            if (repeatYPositive > 0) {
                MENU_REPEAT_Y[controller] = repeatYPositive - updateRate;
                if (MENU_REPEAT_Y[controller] <= 0) {
                    MENU_REPEAT_Y[controller] = 0xF;
                } else {
                    D_800D3194[controller] = 0;
                }
            } else {
                MENU_REPEAT_Y[controller] = 0x14;
            }
        } else {
            D_800D3194[controller] = 0;
            MENU_REPEAT_Y[controller] = 0;
        }
        if (D_800D31B0 & (1 << controller)) {
            D_800D31B8 |= joyGetPressed(controller);
            D_800D31B4 |= joyGetButtons(controller);
            D_800D31C0 += joyGetStickX(controller);
            MENU_STICK_Y_TOTAL += joyGetStickY(controller);
            D_800D31BC += D_800D3190[controller];
            D_800D31BE += D_800D3194[controller];
        }
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_assetgroup_free;
 * JFG's public src/menu.c supplies the freeFrontEndList role and order. */
#pragma weak func_80039A40 = freeFrontEndList
void freeFrontEndList(s16 *assetGroup) {
    s32 index = 0;

    while (assetGroup[index] != -1) {
        freeFrontEndItem(assetGroup[index++]);
    }
}
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::freeFrontEndItem; the body is derived from Mickey. */
#pragma weak func_80039A9C = freeFrontEndItem
void freeFrontEndItem(s32 assetId) {
    if (D_800D3498[assetId] != 0) {
        if (((s32 *) D_800D31C8)[assetId] != 0) {
            if (((D_8007C1B8[assetId] & 0xC000) == 0xC000) &&
                (((s32 *) D_800D31C8)[assetId] != 0)) {
                func_800347A0((void *) ((s32 *) D_800D31C8)[assetId]);
            } else if (D_8007C1B8[assetId] & 0x8000) {
                func_800359D4((void *) ((s32 *) D_800D31C8)[assetId]);
            } else if (D_8007C1B8[assetId] & 0x4000) {
                func_80006EA0((void *) ((s32 *) D_800D31C8)[assetId]);
            } else {
                modFreeModel((void *) ((s32 *) D_800D31C8)[assetId]);
            }
        }
        ((s32 *) D_800D31C8)[assetId] = 0;
        D_800D3498[assetId] = 0;
        D_8007C1C0--;
        func_80006FA0();
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_assetgroup_load;
 * JFG's public src/menu.c supplies the loadFrontEndList role and order. */
void loadFrontEndList(s16 *assetGroup) {
    s32 index = 0;

    while (assetGroup[index] != -1) {
        loadFrontEndItem(assetGroup[index++]);
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_asset_load;
 * JFG's public src/menu.c::loadFrontEndItem and exact-size assembly skeleton
 * supply the role and order. Mickey supplies the packet and object offsets. */
#pragma weak func_80039BE4 = loadFrontEndItem
void loadFrontEndItem(s32 assetId) {
    s16 resourceId;
    MenuSpawnPacket packet;
    MenuSpawnedObject *object;

    if (D_800D3498[assetId] == 0) {
        resourceId = D_8007C1B8[assetId];
        if ((resourceId & 0xC000) == 0xC000) {
            D_800D31C8[assetId] = func_80034448(resourceId & 0x3FFF);
        } else if (resourceId & 0x8000) {
            D_800D31C8[assetId] = func_800355A0(resourceId & 0x3FFF, 0);
        } else if (resourceId & 0x4000) {
            packet.kind = resourceId & 0x3FFF;
            packet.mode = 0xA;
            packet.x = 0;
            packet.y = 0;
            packet.z = 0;
            packet.unkC = 0;
            packet.flags = 0x40;
            packet.unkA = 0;
            object = func_8000590C(&packet, 0);
            if (object->header->count > 0) {
                MenuSpawnInner *inner;

                inner = *object->inner;
                inner->mode = 2;
            }
            D_800D31C8[assetId] = (MenuFrontObject *) object;
        } else {
            D_800D31C8[assetId] = func_8001F520(resourceId & 0x3FFF, 0);
        }
        D_800D3498[assetId] = 1;
        D_8007C1C0++;
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_imagegroup_load;
 * JFG's public src/menu.c supplies the setupFrontEndList role and order. */
void setupFrontEndList(s16 *objectGroup) {
    s32 index = 0;

    while (objectGroup[index] != -1) {
        setupFrontEndObject(objectGroup[index++]);
    }
}
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::setupFrontEndObject; record layout and body derived from Mickey. */
void setupFrontEndObject(s32 objectId) {
    MenuCurrentObject *destination;
    MenuCurrentObject *source;

    destination = &D_800D3550[objectId];
    source = &D_8007C1C4[objectId];
    destination->unk0 = source->unk0;
    destination->unk2 = source->unk2;
    destination->unk4 = source->unk4;
    destination->index = source->index;
    destination->unkC = source->unkC;
    destination->unk10 = source->unk10;
    destination->unk14 = source->unk14;
    destination->unk8 = source->unk8;
    destination->unk18 = source->unk18;
    destination->pad1C[0] = source->pad1C[0];
    destination->pad1C[1] = source->pad1C[1];
    destination->pad1C[2] = source->pad1C[2];
    destination->pad1C[3] = source->pad1C[3];
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/menu.c
 * ::frontDrawObj, at pull request #37 (head d45123d1c528955d5e12ddad805076267a690d76),
 * which takes that file to 48 bodies and 0 GLOBAL_ASM pragmas. The
 * correspondence is positional and total: Mickey's menu.c is JFG's menu.c in
 * the same order with functions dropped, and this is the slot after
 * setupFrontEndObject in both. The donor's separate `tex` carrier for the
 * 0x8000 sprite branch is adopted here; Mickey's own bytes decide the rest,
 * so the volatile copy accesses, the compound guard and the single `command`
 * carrier are Mickey's, not the donor's.
 * Matched by writing the display-list words as packets: the opcode word
 * first through MENU_COMMAND(D_800D3140++, ...) at nine sites, the single
 * `command` carrier at the five sites around the end of the 0x8000 branch and
 * the head of the plain path, and each colour as `(g & 0xFF) << n` terms (the
 * _SHIFTL form). The block-scope packet pointers are what fill the frame the
 * volatile stack struct's old pad stood in for. */
void func_80039E34(s32 index) {
    volatile MenuDrawStack stack;
    s16 flags;
    MenuFrontObject *renderObject;
    MenuCurrentObject *current;
    MenuCommand *command;
    void *tex;

    current = &D_800D3550[index];
    if ((D_800D31C8[current->index] != NULL) &&
        ((D_8007C1B8[current->index] & 0xC000) != 0xC000)) {
        stack.rotationY = *(volatile s16 *)&current->unk0;
        stack.rotationX = *(volatile s16 *)&current->unk2;
        stack.rotationZ = *(volatile s16 *)&current->unk4;
        stack.x = *(volatile f32 *)&current->unkC;
        stack.y = *(volatile f32 *)&current->unk10;
        stack.z = *(volatile f32 *)&current->unk14;
        stack.scale = *(volatile f32 *)&current->unk8;
        flags = D_8007C1B8[*(volatile s16 *)&current->index];
        if (flags & 0x4000) {
            MenuCurrentObject *drawObject =
                (MenuCurrentObject *)D_800D31C8[current->index];

            drawObject->unk0 = current->unk0;
            drawObject->unk2 = current->unk2;
            drawObject->unk4 = current->unk4;
            drawObject->unkC = current->unkC;
            drawObject->unk10 = current->unk10;
            drawObject->unk14 = current->unk14;
            drawObject->unk8 = current->unk8;
            *((s8 *)drawObject + 0x39) = D_8007C0BC;
            func_80009E78(&D_800D3140, &D_800D3144, &D_800D3148,
                          drawObject);
            return;
        }
        if (flags & 0x8000) {
            stack.frame = current->unk18;
            tex = D_800D31C8[current->index];
            MENU_COMMAND(D_800D3140++, 0xE7000000, 0);
            MENU_COMMAND(D_800D3140++, 0xFA000000,
                         ((D_8007C0A4 & 0xFF) << 24) |
                             ((D_8007C0A8 & 0xFF) << 16) |
                             ((D_8007C0AC & 0xFF) << 8) |
                             (D_8007C0BC & 0xFF));
            MENU_COMMAND(D_800D3140++, 0xFB000000, -0x100);
            func_80023F84(&D_800D3140, &D_800D3144, &D_800D3148,
                          &stack.rotationY,
                          tex, D_8007C0B4,
                          D_8007C0BC);
            MENU_COMMAND_CARRIER(0xE7000000, 0);
            MENU_COMMAND_CARRIER(0xFA000000, -1);
            return;
        }
        MENU_COMMAND_CARRIER(0xE7000000, 0);
        if (D_8007C0BC < 0xFF) {
            MENU_COMMAND_CARRIER(0xFA000000, (D_8007C0BC & 0xFF) | ~0xFF);
        } else {
            MENU_COMMAND_CARRIER(0xFA000000, -1);
        }
        MENU_COMMAND(D_800D3140++, 0xFB000000, -0x100);
        stack.frame = current->unk18 * 0.0625f;
        renderObject = D_800D31C8[current->index];
        if (renderObject->resource->unk4E == 0) {
            stack.savedObject = renderObject;
            camPushModelMtx(&D_800D3140, &D_800D3144, &stack.rotationY, 1.0f,
                          0.0f);
            renderObject = stack.savedObject;
            MENU_COMMAND(D_800D3140++,
                         (((renderObject->unkC[renderObject->indexA] +
                            0x80000000) & 0xFFFFFF) | 0xBF000000),
                         renderObject->unk4 + 0x80000000);
            MENU_COMMAND(D_800D3140++, 0x06000000,
                         (s32)renderObject->resource->unk68 + 0x80000000);
            MENU_COMMAND(D_800D3140++, 0xBF000000, 0);
            camPopModelMtx(&D_800D3140);
        }
        if (D_8007C0BC < 0xFF) {
            MENU_COMMAND(D_800D3140++, 0xE7000000, 0);
            MENU_COMMAND(D_800D3140++, 0xFA000000, -1);
        }
    }
}
#pragma weak func_8003A24C = frontGetLanguage
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontGetLanguage; body derived from Mickey. */
s32 frontGetLanguage(void) {
    return (u32)D_800D312A >> 10;
}
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontSetLanguage; body and bitfield derived from Mickey. */
void frontSetLanguage(s32 language) {
    D_800D3128.bits.language = language;
    func_80038750(language);
}
s32 frontGetScreenMode(void) {
    s32 mode;

    mode = 0;
    if (D_800D3128.bits.modeBit0) {
        mode = 1;
    }
    if (D_800D3128.bits.modeBit1) {
        mode |= 2;
    }
    return mode;
}
/* The guard keeps two carriers of the masked mode alive: the int, which the
 * equality test and the store read, and the byte copy the two bit tests read.
 * Naming the byte copy as the comparison's operand is what the plain spelling
 * does, and it reads the byte where the target reads the int; the comma's
 * second operand restores the int at the use without moving the schedule.
 * The store then needs the int too, and spelling it as a third bare
 * `screenMode & 3` gives the global's address a later pool colour -- `^ 0`
 * breaks the lock at that use instead, which costs no colour. Either edit
 * alone is a regression; the pair is exact. */
/* PROVENANCE: ordered accessor-family role rechecked against JFG efd5abb's
 * assembly-backed src/menu.c::frontSetScreenMode; mask, guard, and packed fields
 * are Mickey-derived. */
void func_8003A2C8(s32 screenMode) {
    u8 *modeBitPtr;
    u8 *modeState;
    u8 modeBits;

    modeState = &D_8007C090;
    modeBitPtr = &modeBits;
    if (*modeState != (modeBits = screenMode & 3, screenMode & 3)) {
        D_8007C090 = (screenMode & 3) ^ 0;
        if (*modeBitPtr & 1) {
            D_800D3128.bits.modeBit0 = 1;
        } else {
            D_800D3128.bits.modeBit0 = 0;
        }
        modeBits = screenMode & 3;
        if (modeBits & 2) {
            D_800D3128.bits.modeBit1 = 1;
        } else {
            D_800D3128.bits.modeBit1 = 0;
        }
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontStoreScreenMode. */
void frontStoreScreenMode(void) {
    D_8007C08C = D_8007C090;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontRecallScreenMode. */
u8 frontRecallScreenMode(void) {
    return D_8007C08C;
}
s32 frontGetLevelScreenMode(void) {
    /* Mickey-derived control flow; JFG's body remains GLOBAL_ASM. */
    if (D_8007C090 == 1) {
        goto mode_one;
    }
    if (D_8007C090 == 2) {
        goto mode_two;
    }
    if (D_8007C090 != 3) {
        goto current_mode;
    }

    return 3;
mode_two:
    return levelGetRegionNo() | 2;
mode_one:
    return 1;
current_mode:
    return levelGetRegionNo();
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetWideAdjust. */
s8 frontGetWideAdjust(void) {
    return D_800D312B;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetWideAdjust. */
void frontSetWideAdjust(s32 offset) {
    viSetWideAdjust(offset);
    D_800D312B = viGetWideAdjust();
}
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontGetStereoMode; body and bitfield derived from Mickey. */
u32 frontGetStereoMode(void) {
    return D_800D3128.bits.stereoMode;
}
/* PROVENANCE: name, clamp, table lookup, and order compared with JFG's public
 * src/menu.c::frontSetStereoMode; packed storage derived from Mickey. */
void frontSetStereoMode(s32 mode) {
    if (mode < 0) {
        mode = 0;
    }
    if (mode >= 4) {
        mode = 3;
    }
    D_800D3128.bits.stereoMode = mode;
    alSurround_OutputType(D_8007C308[mode]);
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A47C = frontGetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetSfxVolume. */
u16 frontGetSfxVolume(void) {
    return D_800D312C;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A488 = frontSetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetSfxVolume. */
void frontSetSfxVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312C = volume;
    gsSndpSetGlobalVolume(volume);
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4C4 = frontGetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetBgmVolume. */
u16 frontGetBgmVolume(void) {
    return D_800D312E;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4D0 = frontSetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetBgmVolume. */
void frontSetBgmVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312E = volume;
    amTuneSetGlobalVolume(volume);
}
s32 frontGet2PlayerSplit(void) {
    s32 split;

    split = D_800D3128.bits.twoPlayerSplit;
    return split;
}
/* PROVENANCE: role and order compared with JFG src/menu.c::frontSet2PlayerSplit.
 * The `& 1` is load-bearing: it is folded into the 1-bit field insert but still
 * consumes one ugen temp-ring slot, which rotates the FIFO into the ROM's
 * t0/t8/t9/t1/t2 assignment (allocator-trace-guided, field-guide lever 16). */
void func_8003A520(s32 split) {
    D_800D3128.bits.twoPlayerSplit = split & 1;
}
void func_8003A544(s32 value) {
    D_8007C098 = value;
}
s32 func_8003A550(void) {
    return D_8007C098;
}
void func_8003A55C(s32 value) {
    amTuneStop();
    D_800D3050 = value;
    D_8007BF70 = 0x78;
}
void func_8003A590(void) {
    D_8007BF70 = -1;
}
