#include "ultra64.h"
#include "game/font.h"
#include "game/gameVi.h"
#include "game/level.h"
#include "game/memory.h"
#include "game/menu.h"
#include "game/sched_internal.h"
#include "n_audio/mbi.h"

/*
 * Resident main state and frame control, ROM 0x27760-0x2A250.
 *
 * PROVENANCE: the TU attribution and adopted function names are adapted from
 * Jet Force Gemini's published src/main.c and built src/main.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's call graph
 * and six `main/main.c` string references establish the correspondence
 * independently (tiers B and C). JFG address-placeholder names are not used.
 */

extern s32 D_8007A164;
extern s32 D_8007A160;
extern s32 D_8007A170;
extern s32 D_8007A174;
extern s32 D_8007A178;
extern s32 D_8007A17C;
extern s32 D_8007A180;
extern s32 D_8007A184;
extern s32 D_8007A188;
extern s32 D_8007A18C;
extern s32 D_8007A154;
extern s32 D_8007A14C;
extern s32 D_8007A15C;
extern s32 D_8007A16C;
extern s32 D_8007A190;
extern s32 D_8007A148;
extern s32 D_8007A150;
extern s32 D_8007A158;
extern s32 D_8007A168;
extern s32 D_8007A1B0;
extern s32 D_8007A1BC;
extern s32 D_8007A1AC;
extern s32 D_8007A1C0;
extern s32 D_8007A13C;
extern s32 D_8007A140;
extern s32 D_8007A144;
extern s32 D_8007A128;
extern s32 D_8007A12C;
extern s32 D_8007A130;
extern s32 D_8007A134;
extern s32 D_8007A138;
extern s32 D_8007A120;
extern s32 D_8007A124;
extern s32 D_8007A1A4;
extern s32 D_8007A1A8;
extern s32 D_8007A194;
extern s32 D_8007A198;
extern s32 D_8007A67C;
extern s32 D_80078F7C;
extern s32 D_8007C1A0;
extern u8 D_8007A1A0;
extern u32 D_8007A1CC;
extern s32 D_8007A1D4;
extern s32 D_8007A1EC;
extern s32 D_8007A1B4;
extern s32 D_8007A1D8;
extern s32 D_8007A1B8;
extern s32 D_8007A19C;
extern s32 D_8007A1D0;
extern s32 D_8007A1E0;
extern s32 D_8007A1E4;
extern s32 D_8007A20C;
extern s32 D_8007A248;
extern u8 D_8007A24B;
extern s32 D_8007A200;
extern void *D_8007A204;
extern s32 D_8007A24C;
extern s32 D_8007A320;
extern s32 D_8007A6A8;
extern u8 *D_8007A244;
extern s16 D_8007A250[];
extern s32 D_8007A258;
extern u8 D_8007BEF4;
extern u8 D_8007BEF8;
extern u8 D_8007BEFC;
extern u8 D_8007BF04;
extern u8 D_8007BF0C;
extern s8 D_8007BEF0;
extern s8 D_800CF53F[];
extern s32 D_80078DF0;
extern char D_80081B98[];
extern char D_80081BA0[];
extern char D_80081BA8[];
extern char D_80081BB0[];
extern char D_80081BBC[];
extern char D_80081BC0[];
extern char D_80081BC4[];
extern char D_80081BC8[];
extern char D_80081B24[];
extern char D_80081B30[];
extern char D_80081B3C[];
extern char D_80081B48[];
extern char D_80081B0C[];
extern char D_80081B18[];
extern f32 D_80081BD0;
extern Gfx *D_800CF510[];
extern Gfx *D_800CF518;
extern s32 D_800CF51C;
extern s32 D_800CF520;
extern s32 D_800CF524;
extern Mtx *D_800CF528[];
extern Mtx *D_800CF530;
extern s32 D_800CF534;
extern s32 D_800CF578;
extern s32 D_800CF57C;
extern s32 D_800CF58C;
extern s32 D_800CF590;
extern s32 D_800CF594;
extern s32 D_800CF5A4;
extern s32 D_800CF5A8;
extern s32 D_800CF5AC;
extern s32 D_800CF5B0;

typedef struct MainGameEntry {
    u8 pad0[4];
    u8 character;
    u8 pad5[35];
} MainGameEntry;

typedef struct MainCharacterState {
    u8 character;
    u8 variant;
    u8 variantCopy;
    u8 pad03[0x19];
    u8 counters[6];
    u16 value22;
    u8 flags[4];
} MainCharacterState;

typedef struct MainGameState {
    u8 flag0;
    u8 flag1;
    u8 pad2[2];
    MainCharacterState characters[6];
} MainGameState;

typedef struct MainZBCheck {
    s16 x;
    s16 y;
    u8 pad4;
    s8 width;
    s8 height;
    s8 enabled;
} MainZBCheck;

/*
 * PROVENANCE: bitfield names and layout adapted from JFG include/mips.h;
 * Mickey's own patch sequence and byte identity validate their use here.
 */
typedef union MainMipsInstruction {
    u32 word;
    struct {
        u32 opcode : 6;
        u32 sourceRegister : 5;
        u32 targetRegister : 5;
        u32 destinationRegister : 5;
        u32 shiftAmount : 5;
        u32 function : 6;
    } shiftEncoding;
    struct {
        u32 opcode : 6;
        u32 sourceRegister : 5;
        u32 targetRegister : 5;
        u32 immediate : 16;
    } addiu;
    u8 bytes[4];
} MainMipsInstruction;

typedef struct MainDebugMemory {
    s16 count;
    u8 pad2[10];
    f32 valueC;
    f32 value10;
    f32 value14;
} MainDebugMemory;

typedef struct MainVertex {
    u8 bytes[10];
} MainVertex;

typedef struct MainTriangle {
    u8 bytes[16];
} MainTriangle;

extern MainVertex *D_800CF580[];
extern MainVertex *D_800CF588;
extern MainTriangle *D_800CF598[];
extern MainTriangle *D_800CF5A0;

extern MainGameEntry *D_800D18E0;
extern void *D_800D18E4;
extern u8 D_800D1928[];
extern OSMesgQueue D_800D18F8;
extern OSMesg D_800D18F4;
extern OSScClient D_800D18E8;
extern OSSched D_800CF5B8;
extern s32 D_800D1910;
extern s32 D_800D18C0;
extern s32 D_800D18C4;
extern s32 D_800D18C8;
extern s32 D_800D18CC;
extern s32 D_800D18D0;
extern s32 D_800D18D4;
extern s32 D_800D18D8;
extern s32 D_800D18DC;
extern s32 D_800C947C;
extern s32 D_8007C854;
extern s16 D_800D6C44;
extern MainZBCheck D_800CF538[];
extern u16 *D_800D2FAC;
extern s32 *D_800D2FA0;
extern s32 *D_800D2FA8;
extern s32 osTvType;
extern s32 levelNGetType(s32 level);
extern void func_80028EFC(MainCharacterState *, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_80005548(s32);
extern void func_80028DE4(s32, s32, s32, s32, s32, s32);
extern void func_8003A544(s32);
extern void joyResetMap(void);
extern void func_8004978C(s32, s32, s32);
extern s32 func_80049864(s32);
extern s32 func_8004989C(s32, s32 *);
extern void func_800498FC(s32, f32, f32, s32, s32, s32, s32);
extern s32 TrapDanglingJump();
extern s32 joyRead(s32, s32);
extern void mainInitGame(void);
extern void mainPreNMI(void);
extern void func_80026FB4(void);
extern void bootCheckStack(void);
extern void func_80001BC4(void);
extern void viReset(void);
extern void rumbleKill(s32);
extern void rumbleTick(s32);
extern void osSetTime(OSTime);
extern OSTime osGetTime(void);
extern u16 joyGetButtons(s32);
extern u16 joyGetPressed(s32);
extern void mainFrontInit(s32, s32, s32);
extern void RevealReturnAddresses(void);
extern void mmInit(void);
extern void func_8004D750(void);
extern void *mmAlloc(s32, s32);
extern void osCreateScheduler(OSSched *, void *, s32, u8, u8);
extern void osScAddClient(OSSched *, OSScClient *, OSMesgQueue *, u8);
extern void piInit(void);
extern void rcpInit(OSSched *);
extern void runlinkInit(void);
extern void runlinkFreeCode(s32);
extern s32 camIsUserView(s32);
extern void camGetUserView(s32, s32 *, s32 *, s32 *, s32 *);
extern f32 rainDensity(void);
extern s32 runlinkIsModuleLoaded(s32);
extern void func_8004A0F0(void);
extern void func_8004A51C(void);
extern void fxGenerateTextures(void);
extern s32 D_800D40E4;
extern f32 D_80081BCC;
extern u8 amTuneGetSeqNo(void);
extern void amTuneSetFade(f32, s32);
extern s32 func_80001614(void);
extern s32 func_800290A0(void);
/* Unsigned here on evidence: func_80028564 compares this result and a byte
 * field against 1 in neighbouring tests, and the target keeps the two
 * constants apart, which IDO does only when their types differ. The
 * definition's own code does not depend on the signedness. */
extern u32 func_80037664(void);
extern s32 levelGetTune(s32);
extern s32 levelGetScreenMode(s32);
extern u32 levelGetGfxIndex(s32);
extern void levelFreeAll(void);
extern void rumbleRumbles(s32);
extern void rumbleUpdate(void);
extern void reset_particles(void);
extern void func_8004B0A4(s32);
extern void func_8004B0DC(s32, s32, s32, s32);
extern void func_8004B0F8(Gfx **, s32, s32, char *, s32);
extern MainDebugMemory *func_80005820(s32);
extern s32 sprintf(char *, const char *, ...);
extern u8 *levelGetLevel(void);
extern void diRcpTrace(Gfx *, char *, s32);
/* The checkpoint hook always receives a string and a number that read as a
 * file name and a source line, which is what a macro built on __FILE__ and
 * __LINE__ leaves behind. The do-while wrapper is the usual form for such a
 * macro and it is load-bearing: its region boundary is what the main loop's
 * register allocation depends on. */
#define MAIN_DL_CHECKPOINT(dl, file, line) \
    do { \
        diRcpTrace((dl), (file), (line)); \
    } while (0)
extern void func_80008028(s32);
extern void func_80051364(s32);
extern void func_8000784C(s32);
extern void func_80007844(void);
extern void partUpdateParticles(s32);
extern s32 camGetPtr(void);
extern void func_80053420(s32, s32);
extern void func_8000BD50(s32);
extern void func_80006FA0(void);
extern void func_8000BDB4(Gfx **, Mtx **, MainVertex **, MainTriangle **, s32);
extern void func_8004EDA8(s32);
extern void screenDraw(Gfx **);
extern void rsp_segment(Gfx **, s32, void *);
extern void rcpFast3d(Gfx *, Gfx *, s32, void *);
extern void rcpInitSp(Gfx **);
extern void rcpInitDp(Gfx **);
extern void rcpClearScreen(Gfx **, Mtx **, s32);
extern s32 rcpWaitDP(void);
extern void bgdraw_fillcolour(s32, s32, s32);
extern void camSetUserView(s32, s32, s32, s32, s32);
extern void camEnableUserView(s32, s32);
extern void camDisableUserView(s32, s32);
extern void diRcpTraceReset(void);
extern void diCpuTraceGetFault(void);
extern void diCpuTraceTick(s32);
extern void fxUnQueueScreenEffect(Gfx **);
extern s32 func_80049B14(s32);
extern void frontUpdate(Gfx **, Mtx **, MainVertex **, MainTriangle **, s32);
extern void func_80049E4C(Gfx **, s32);
extern void amAudioTick(u8);
extern void diPrintfAll(Gfx **);
extern void fontWindowsDraw(Gfx **, Mtx **, MainVertex **);
extern void func_8004BFD8(s32);
extern void func_8004BF64(s32);
extern s32 func_800291E4(void);
extern void func_800376CC(s32);
extern void func_80038190(Gfx **, Mtx **, MainVertex **);
extern void func_8005A770(void);
extern void camTick(s32);
extern void camUserViewTick(void);
extern void func_80027D14(s32);
extern void func_8003C80C(s32);
extern void func_8004D32C(void);
extern void trackUpdateTextureScroll(void);
extern void trackUpdateLighting(s32, s32);
extern void runlinkTick(void);
extern void func_80027628(s32);
extern void func_80027EC0(s32);
extern void func_80027FB8(s32);
extern void func_80028564(s32);
extern void func_800293D0(void);
extern void mainUpdateZBCheck(void);
extern void mainCPUeffects(u16 *, s32);
extern void mmFlushFreeStack(void);
extern void mmSetDelay(s32);
extern void frontFreeMode(void);
extern void func_80037150(void);
extern void func_8004E99C(void);
extern void amTuneStop(void);
extern void viChangeMode(s32);
extern void camInit(void);
extern void func_8005A764(void);
extern void func_8004E8E0(void);
extern void func_80049A8C(s32);
extern void func_80037414(s32, f32, f32, s32, s32, s32, s32);
extern void amWaitForMidiSync(void);

#pragma weak mainCPUeffectsRainDraw = TrapDanglingJump
extern void mainCPUeffectsRainDraw(void *, s32, s32, f32, f32);

/*
 * PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is
 * decisive. Keep the nested statement line-spliced: IDO uses its source-line
 * group when scheduling the three hoisted loop constants.
 */
void RevealReturnAddresses(void) {
    s32 outer;
    MainMipsInstruction **returnAddress;
    MainMipsInstruction *scan;
    s32 inner;
    s32 opcode;
    s32 canary;

    do { \
        opcode = 9, canary = 0x666, outer = 4; \
        do { \
            returnAddress = (MainMipsInstruction **) &D_8007A244, outer = 4; \
            do { \
                scan = *returnAddress; \
                inner = 0x3F; \
                do { \
                    if ((scan->addiu.opcode == opcode) && \
                        (scan->addiu.immediate == canary)) { \
                        scan->shiftEncoding.opcode = 0; \
                        scan->shiftEncoding.sourceRegister = 31; \
                        scan->shiftEncoding.destinationRegister = \
                            scan->addiu.targetRegister; \
                        scan->shiftEncoding.targetRegister = 0; \
                        scan->shiftEncoding.shiftAmount = 0; \
                        scan->shiftEncoding.function = 0x25; \
                        osWritebackDCache(scan, 4); \
                        osInvalICache(scan, 4); \
                        break; \
                    } \
                    scan++; \
                } while (inner--); \
                returnAddress--; \
            } while (outer--); \
        } while (0); \
    } while (0);
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
/* Workbench p6: relocation-layout mismatch; 50/50 words and -0x20 frame exact, first +0x18.
 * Rechecked symbolic/indexed RAM-end forms; each adds eight words, while the literal preserves the target schedule.
 * Remains: D_803FFFFC HI16/LO16 identity in untouched compiler output. */
void mainThread(void *unused) {
    s32 i;

    mainInitGame();
    if (osTvType == 0) {
        i = 0;
        while (1) {
            ((volatile u32 *) 0x80400000)[--i] = 0;
        }
    }
    D_8007A200 = 1;
    TrapDanglingJump();
    D_8007A1CC = joyRead(D_8007A1CC, 0);
    D_8007A1B4 = 1;
    D_8007A1D8 = 0;
    D_8007A1BC = 6;
    mainChangeLevel(0, D_8007A154, 0, 0, 1, 0);
    while (1) {
        mainPreNMI();
        func_80026FB4();
        bootCheckStack();
    }
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainResetPressed(void) {
    if (D_800D1910 == 0) {
        D_800D1910 = (osRecvMesg(&D_800D18F8, NULL, OS_MESG_NOBLOCK) + 1) != 0;
    }
    return D_800D1910;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainPreNMI(void) {
    OSTime time;

    if (mainResetPressed()) {
        if (D_8007A1B4 != 0) {
            func_80001BC4();
            osSetTime(0);
            time = osGetTime();
            while (time < 2300000) {
                time = osGetTime();
            }
            __osSpSetStatus(0xAAAA82);
            osDpSetStatus(0x1D6);
            viReset();
            rumbleKill(1);
            rumbleTick(2);
            rumbleTick(2);
            rumbleTick(2);
        }
        while (1) {
        }
    }
}

/*
 * PROVENANCE: body adapted from Jet Force Gemini src/main.c::mainInitGame;
 * Mickey's subsystem calls, storage and byte identity are decisive.
 */
void mainInitGame(void) {
    s32 viMode;

    if (osTvType == 0) {
        viMode = 0xE;
    } else if (osTvType == 1) {
        viMode = 0;
    } else if (osTvType == 2) {
        viMode = 0x1C;
    }
    osCreateScheduler(&D_800CF5B8, &D_800D18C0, 13, viMode, 1);
    RevealReturnAddresses();
    mmInit();
    func_8004D750();
    D_8007A204 = mmAlloc(0x40, 0x7F7F7FFF);
    D_8007A1AC = 0;
    osCreateMesgQueue(&D_800D18F8, &D_800D18F4, 1);
    osScAddClient(&D_800CF5B8, &D_800D18E8, &D_800D18F8, 3);
    D_800D1910 = 0;
    D_8007A320 = 1;
    D_8007A1B4 = 0;
    viInit(&D_800CF5B8);
    mainPreNMI();
    piInit();
    mainPreNMI();
    rcpInit(&D_800CF5B8);
    mainPreNMI();
    runlinkInit();
    mainPreNMI();
    TrapDanglingJump();
    mainPreNMI();
    runlinkFreeCode(0x12);
    D_8007A320 = 0;
}

/*
 * PROVENANCE: the main-loop role and frame-pipeline organization are adapted
 * from Diddy Kong Racing's published src/thread3_main.c::main_game_loop and
 * cross-checked against JFG's published src/main.c TU ordering. Mickey's own
 * call graph, resident storage and instructions determine this body.
 *
 * What matched it, under the plain preset (uopt runs on this procedure): the
 * display-list checkpoint is a do-while macro, whose region boundary keeps
 * the cursor address off a callee-saved register; the two closing display
 * commands are the standard macros; the buffer index is toggled in place; and
 * the frame-sync result is held in a local, as in the DKR loop, so the clamp
 * tests the call result and the final call rereads the global.
 */
void func_80026FB4(void) {
    s32 drawTransition;
    s32 updateRate;

    if (D_8007A20C != 0) {
        TrapDanglingJump(NULL);
    }
    osSetTime(0);
    if (D_8007A1D0 == 8) {
        D_800CF518 = D_800CF510[D_8007A1B8];
        rsp_segment(&D_800CF518, 0, NULL);
        rsp_segment(&D_800CF518, 1, D_800D2FA0);
        rsp_segment(&D_800CF518, 2, D_800D2FAC);
        rsp_segment(&D_800CF518, 4, (u8 *) D_800D2FA0 - 0x500);
    }
    rcpFast3d(D_800CF510[D_8007A1B8], D_800CF518, 0, D_800D2FA0);
    if (D_8007A20C != 0) {
        TrapDanglingJump();
    }

    D_8007A1B8 ^= 1;
    diRcpTraceReset();
    D_800CF518 = D_800CF510[D_8007A1B8];
    D_800CF530 = D_800CF528[D_8007A1B8];
    D_800CF588 = D_800CF580[D_8007A1B8];
    D_800CF5A0 = D_800CF598[D_8007A1B8];
    MAIN_DL_CHECKPOINT(D_800CF518, D_80081B0C, 0x2E6);
    rsp_segment(&D_800CF518, 0, NULL);
    rsp_segment(&D_800CF518, 1, D_800D2FA8);
    rsp_segment(&D_800CF518, 2, D_800D2FAC);
    rsp_segment(&D_800CF518, 4, (u8 *) D_800D2FA8 - 0x500);
    rcpInitSp(&D_800CF518);

    rcpInitDp(&D_800CF518);
    if (D_8007A128 != 0) {
        camSetUserView(0, D_8007A12C, D_8007A130, D_8007A134, D_8007A138);
        camEnableUserView(0, 1);
        bgdraw_fillcolour(0, 0, 0);
    }
    rcpClearScreen(&D_800CF518, &D_800CF530, 1);
    camDisableUserView(0, 1);

    D_8007A1CC = joyRead(D_8007A1CC, D_8007A248);
    diCpuTraceGetFault();
    if (D_8007A1BC == 5) {
        diCpuTraceTick(D_8007A248);
    } else {
        func_80027FB8(D_8007A248);
        if (D_8007A1BC == 6) {
            func_80027EC0(D_8007A248);
        }
    }

    fxUnQueueScreenEffect(&D_800CF518);
    drawTransition = func_80049B14(D_8007A248);
    frontUpdate(&D_800CF518, &D_800CF530, &D_800CF588, &D_800CF5A0,
                  D_8007A248);
    if (drawTransition != 0) {
        func_80049E4C(&D_800CF518, 0);
    }
    TrapDanglingJump(&D_800CF518, &D_800CF530, D_8007A248);
    if (runlinkIsModuleLoaded(0xB) != 0) {
        TrapDanglingJump(&D_800CF518, &D_800CF530, &D_800CF588,
                         D_8007A248);
    }
    TrapDanglingJump(&D_800CF518, &D_800CF530, &D_800CF588, &D_800CF5A0, 0,
                     D_8007A248);

    amAudioTick(D_8007A24B);

    diPrintfAll(&D_800CF518);
    if (D_8007A1E0 != 0) {
        func_800293D0();
    }
    fontWindowsDraw(&D_800CF518, &D_800CF530, &D_800CF588);
    func_8004BFD8(4);

    func_8004BF64(4);
    if (drawTransition != 0) {
        func_80049E4C(&D_800CF518, 1);
    }
    if ((D_8007A1D8 >= 8) && (func_800291E4() != 0)) {
        frontDemoMessage(&D_800CF518, D_8007A248);
    }

    MAIN_DL_CHECKPOINT(D_800CF518, D_80081B18, 0x355);
    func_800376CC(D_8007A248);
    func_80038190(&D_800CF518, &D_800CF530, &D_800CF588);
    gDPFullSync(D_800CF518++);
    gSPEndDisplayList(D_800CF518++);
    if ((runlinkIsModuleLoaded(0x21) != 0) && (func_80049864(4) == 0)) {
        TrapDanglingJump();
    }
    func_8005A770();

    camTick(D_8007A248);

    camUserViewTick();

    func_80027D14(D_8007A248);
    if (D_800D6C44 != 0) {
        TrapDanglingJump(D_8007A248);
    }
    if ((D_8007C854 != 0) && (func_800290A0() == 0)) {
        func_8003C80C(D_8007A248);
    }
    func_8004D32C();
    D_8007A1D0 = rcpWaitDP();
    mainUpdateZBCheck();
    mainCPUeffects((u16 *) D_800D2FA0, D_8007A248);
    trackUpdateTextureScroll();
    trackUpdateLighting(1, D_8007A248);
    if (D_8007A1E4 != 0) {
        TrapDanglingJump(D_8007A248);
        D_8007A1E4 = 0;
    }
    runlinkTick();

    mmFreeTick();

    func_80027628(D_8007A248);

    updateRate = viFrameSync(D_8007A1D0);
    D_8007A248 = updateRate;
    if (D_8007A19C != 0) {
        viFrameRateReset();
        D_8007A248 = 2;
    } else if (updateRate >= 7) {
        D_8007A248 = 6;
    }
    func_80028564(D_8007A248);
}

/*
 * PROVENANCE: the display-list, matrix, vertex and triangle roles are adapted
 * from Diddy Kong Racing's published src/thread3_main.c::main_game_loop and
 * JFG's main-TU ordering. Mickey's own instructions determine this body.
 */
void func_80027628(s32 updateRate) {
    s32 displayListCount;
    s32 matrixCount;
    s32 vertexCount;
    s32 triangleCount;
    s32 copyCount;
    s32 *source;
    s32 *destination;
    s32 width;
    s32 height;

    displayListCount = D_800CF518 - D_800CF510[D_8007A1B8];
    matrixCount = D_800CF530 - D_800CF528[D_8007A1B8];
    vertexCount = D_800CF588 - D_800CF580[D_8007A1B8];
    triangleCount = D_800CF5A0 - D_800CF598[D_8007A1B8];

    if (D_800CF524 < displayListCount) {
        D_800CF524 = displayListCount;
    }
    if (D_800CF57C < matrixCount) {
        D_800CF57C = matrixCount;
    }
    if (D_800CF594 < vertexCount) {
        D_800CF594 = vertexCount;
    }
    if (D_800CF5AC < triangleCount) {
        D_800CF5AC = triangleCount;
    }

    if (D_800CF51C < displayListCount) {
        D_800CF520 = 1;
    }
    if (D_800CF534 < matrixCount) {
        D_800CF578 = 1;
    }
    if (D_800CF58C < vertexCount) {
        D_800CF590 = 1;
    }
    if (D_800CF5A4 < triangleCount) {
        D_800CF5A8 = 1;
    }

    if (D_8007A120 != 0) {
        viGetCurrentSize(&width, &height);
        source = D_800D2FA0;
        destination = D_800D2FA8;
        copyCount = ((u32) (width * height) >> 1) - 1;
        if (((u32) (width * height) >> 1) != 0) {
            do {
                *destination++ = *source++;
            } while (copyCount--);
        }
    }

    D_8007A124 -= updateRate;
    if (D_8007A124 < 0) {
        D_8007A124 = 0;
    }
    D_8007A120 = 0;
    if ((D_800CF520 != 0) || (D_800CF578 != 0) ||
        (D_800CF590 != 0) || (D_800CF5A8 != 0)) {
        D_800CF518 = D_800CF510[D_8007A1B8];
        gDPFullSync(D_800CF518++);
        gSPEndDisplayList(D_800CF518++);
        D_8007A124 = 60;
        D_8007A120 = 1;
    }
    D_800CF520 = 0;
    D_800CF578 = 0;
    D_800CF590 = 0;
    D_800CF5A8 = 0;
}

s32 mainAddZBCheck(s32 x, s32 y, s32 radius) {
    s32 result;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 screenWidth;
    s32 screenHeight;

    viGetCurrentSize(&screenWidth, &screenHeight);
    result = -1;
    if (D_8007A24C < 8) {
        x2 = x + radius;
        y2 = y + radius;
        if ((x2 >= 0) && (y2 >= 0)) {
            x1 = x - radius;
            y1 = y - radius;
            if ((x1 < screenWidth) && (y1 < screenHeight)) {
                if (x1 < 0) {
                    x1 = 0;
                }
                if (y1 < 0) {
                    y1 = 0;
                }
                if (x2 >= screenWidth) {
                    x2 = screenWidth - 1;
                }
                if (y2 >= screenHeight) {
                    y2 = screenHeight - 1;
                }
                result = D_8007A24C++;
                D_800CF538[result].x = x1;
                D_800CF538[result].y = y1;
                D_800CF538[result].width = (x2 - x1) + 1;
                D_800CF538[result].height = (y2 - y1) + 1;
            }
        }
    }
    return result;
}

void mainUpdateZBCheck(void) {
    MainZBCheck *check;
    s32 i;
    s32 enabled;
    s32 rows;
    s32 screenWidth;
    s32 screenHeight;
    s32 columns;
    u8 *row;

    viGetCurrentSize(&screenWidth, &screenHeight);
    check = D_800CF538;
    i = 7; do {
        enabled = 1;
        if (D_8007A24C > 0) {
            D_8007A24C--;
            rows = check->height;
            row = (((check->y * screenWidth) + check->x) * 2) +
                  (u8 *) D_800D2FAC;
            if (rows--) {
                do {
                    columns = check->width;
                    if (columns--) {
                        do {
                            if ((*((u16 *) row + columns) & 0xFFFC) == 0xFFFC) {
                                enabled = 0;
                                goto nextCheck;
                            }
                        } while (columns--);
                    }
                    row += screenWidth * 2;
                } while (rows--);
            }
        }
nextCheck:
        check->enabled = enabled;
        check++;
    } while (i--);
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s8 mainGetZBCheck(s32 index) {
    if ((index < 0) || (index >= 8)) {
        return 1;
    }
    return D_800CF53F[index * 8];
}

/*
 * PROVENANCE: function role adapted from JFG src/main.c::mainCPUeffects. JFG
 * retains assembly, so the body is reconstructed from Mickey-only control-flow
 * and call evidence; Mickey byte identity is decisive.
 */
void mainCPUeffects(u16 *framebuffer, s32 unused) {
    s32 screenWidth;
    u32 screenHeight;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    viGetCurrentSize(&screenWidth, (s32 *) &screenHeight);
    if (camIsUserView(0) != 0) {
        camGetUserView(0, &x1, &y1, &x2, &y2);
        framebuffer = (y1 * screenWidth) + framebuffer;
        screenHeight = (y2 - y1) + 1;
    }
    if ((func_80049864(4) == 0) &&
        (runlinkIsModuleLoaded(0xF) != 0) &&
        (TrapDanglingJump() != 0)) {
        if ((D_800D40E4 > 0) && (screenHeight >= D_800D40E4)) {
            mainCPUeffectsRainDraw(framebuffer, screenWidth, D_800D40E4,
                                   256.0f, rainDensity());
        }
        D_800D40E4 = screenHeight;
    }
    fxGenerateTextures();
    if ((runlinkIsModuleLoaded(0x21) != 0) &&
        (func_80049864(4) == 0)) {
        TrapDanglingJump();
    }
    if (func_80049864(4) == 0) {
        func_8004A51C();
    } else {
        func_8004A0F0();
    }
}

void mainSetGameWindow(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                       s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    D_800D18C0 = arg0;
    D_800D18C4 = arg1;
    D_800D18C8 = arg2;
    D_800D18CC = arg3;
    D_800D18D0 = arg4;
    D_800D18D4 = arg5;
    D_800D18D8 = arg6;
    D_800D18DC = arg7;
    if (arg8 < 0) {
        D_8007A140 = -arg8;
        D_8007A144 = 1;
    } else {
        D_8007A140 = arg8;
        D_8007A144 = 0;
    }
    D_8007A13C = D_8007A140;
}

/*
 * PROVENANCE: function role and interpolation structure compared with Jet
 * Force Gemini src/main.c::func_80045C38_46838; JFG retains assembly, and the
 * body below is reconstructed from Mickey-only control-flow and data evidence.
 */
void func_80027D14(s32 updateRate) {
    s32 fraction;
    s32 countdown;
    s32 changing;

    countdown = D_8007A13C;
    if (countdown == 0) {
        goto updateChanging;
    }
    D_8007A12C = D_800D18D0;
    D_8007A130 = D_800D18D4;
    D_8007A134 = D_800D18D8;
    D_8007A138 = D_800D18DC;
    countdown = D_8007A13C = countdown - updateRate;
    if (countdown > 0) {
        fraction = (countdown << 16) / D_8007A140;
        D_8007A12C += ((D_800D18C0 - D_800D18D0) * fraction) >> 16;
        D_8007A130 += ((D_800D18C4 - D_800D18D4) * fraction) >> 16;
        D_8007A134 += ((D_800D18C8 - D_800D18D8) * fraction) >> 16;
        D_8007A138 += ((D_800D18CC - D_800D18DC) * fraction) >> 16;
    } else {
        D_8007A13C = 0;
        countdown = 0;
    }
updateChanging:
    changing = countdown != 0;
    if (changing == 0) {
        changing = D_8007A144 == 0;
    }
    D_8007A128 = changing;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGameWindowChanging(void) {
    return D_8007A13C;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainGameWindowSize(s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    *x1 = D_8007A12C;
    *y1 = D_8007A130;
    *x2 = D_8007A134;
    *y2 = D_8007A138;
}

void func_80027EC0(s32 updateRate) {
    s32 i;
    s32 buttons;

    i = 0;
    buttons = 0;
    do {
        buttons |= joyGetButtons(i) & 0x1000;
        i++;
    } while (i != 4);
    if (buttons != 0) {
        D_8007BEF0 = 1;
    }
    D_8007A1D8++;
    if (D_8007A1D8 >= 10) {
        frontSetLanguage(frontGetLanguage());
        frontSetScreenMode(frontGetScreenMode());
        frontSetWideAdjust(frontGetWideAdjust());
        frontSetStereoMode(frontGetStereoMode());
        frontSetSfxVolume(frontGetSfxVolume());
        frontSetBgmVolume(frontGetBgmVolume());
        frontStoreScreenMode();
        mainFrontInit(2, 0, 0);
    }
}

/*
 * PROVENANCE: function placement and main-loop role were compared with JFG
 * src/main.c::func_80045F00_46B00, which remains assembly. This body is
 * reconstructed from Mickey's own control flow and call graph.
 */
void func_80027FB8(s32 updateRate) {
    s32 unused[4];
    u8 *level;
    s32 controller;
    s32 heldButtons;
    s32 pressedButtons;
    s32 pauseController;

    level = levelGetLevel();
    pauseController = -1;
    heldButtons = 0;
    pressedButtons = 0;
    controller = D_8007C1A0;
    if (controller--) {
        do {
            heldButtons |= joyGetButtons(controller);
            pressedButtons |= joyGetPressed(controller);
            if ((pressedButtons & 0x1000) && (pauseController == -1)) {
                pauseController = controller;
            }
        } while (controller--);
    }

    MAIN_DL_CHECKPOINT(D_800CF518, D_80081B24, 0x526);
    func_80008028(updateRate);
    D_8007A67C = 1;
    if (D_8007A1A8 == 0) {
        D_800CF5B0 += updateRate;
        func_80051364(updateRate);
        func_8000784C(updateRate);
        if (runlinkIsModuleLoaded(1) != 0) {
            TrapDanglingJump(updateRate);
        }
        partUpdateParticles(updateRate);
        func_80053420(0, camGetPtr());
        func_8000BD50(updateRate);
        if (((s8 *) level)[0x83] != 1 && ((s8 *) level)[0x83] != 2 &&
            (pressedButtons & 0x1000) && D_8007A1A4 == 0 &&
            D_8007A194 == 0 && D_8007A1BC == 0 && D_800C947C == 0) {
            rumbleKill(1);
            TrapDanglingJump(pauseController);
            D_8007A67C = 0;
        }
    } else {
        func_80007844();
        D_8007A67C = 0;
    }

    if (runlinkIsModuleLoaded(0x24) != 0) {
        TrapDanglingJump();
    }
    if (D_8007A1A4 > 0) {
        D_8007A1A4 -= updateRate;
        if (D_8007A1A4 < 0) {
            D_8007A1A4 = 0;
        }
    }
    func_80006FA0();
    diRcpTrace(D_800CF518, D_80081B30, 0x563);
    if (D_8007A198 != 0) {
        func_8000BDB4(&D_800CF518, &D_800CF530, &D_800CF588,
                      &D_800CF5A0, updateRate);
    }
    diRcpTrace(D_800CF518, D_80081B3C, 0x589);
    func_8004EDA8(updateRate);
    if (runlinkIsModuleLoaded(0xE) != 0) {
        TrapDanglingJump(&D_800CF518, updateRate);
    }
    if (runlinkIsModuleLoaded(7) != 0) {
        TrapDanglingJump(&D_800CF518, updateRate);
    }
    if ((D_8007A1A8 == 0) ||
        ((D_8007A1A8 != 0) && (D_8007A194 != 0))) {
        screenDraw(&D_800CF518);
    }
    if (D_80078F7C != 0) {
        TrapDanglingJump(&D_800CF518, &D_800CF530, &D_800CF588,
                         D_80078F7C);
    }
    diRcpTrace(D_800CF518, D_80081B48, 0x5A0);
}

void func_800282C8(void) {
    if (D_8007A194 == 0) {
        mainChangeLevel(D_8007A148, D_8007A150, D_8007A158, D_8007A168, 0, 0);
        if (func_80049864(4) == 0) {
            D_8007A1A0 = 0;
            func_800498FC(4, 0.34f, -1.0f, 0xFF, 0xFF, 0xFF, 0);
            func_8004978C(4, 1, 1);
        }
        D_8007A194 = 0x1E;
    }
}

/*
 * PROVENANCE: function role and control structure compared with Jet Force
 * Gemini src/main.c::mainChangeLevel; JFG retains assembly. This body is
 * reconstructed from Mickey-only control-flow and data evidence.
 */
void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup,
                     s32 nextCamera, s32 fadeOut, s32 flags) {
    if ((D_8007A194 == 0) || (nextLevel != D_8007A14C) ||
        (nextCharacter != D_8007A154) || (nextCamera != D_8007A16C)) {
        D_8007A14C = nextLevel;
        D_8007A164 = 0;
        D_8007A15C = nextAnimGroup;
        D_8007A154 = nextCharacter;
        D_8007A16C = nextCamera;
        if (D_8007A194 <= 0) {
            D_8007A194 = 10;
            if ((func_80049864(4) == 0) && (func_80037664() == 0)) {
                if (fadeOut != 0) {
                    D_8007A1A0 = 1;
                    func_800498FC(4, 0.34f, -1.0f, 0, 0, 0, 0);
                } else {
                    D_8007A1A0 = 0;
                    func_800498FC(4, 0.34f, -1.0f,
                                  0xFF, 0xFF, 0xFF, 0);
                }
                func_8004978C(4, 1, 1);
                D_8007A194 = 30;
            }
            if ((amTuneGetSeqNo() != levelGetTune(D_8007A14C)) &&
                (func_800290A0() == 0) && (func_80001614() == 0)) {
                amTuneSetFade(D_80081BCC, 0);
            }
            rumbleKill(1);
            rumbleRumbles(0);
        }
        D_8007A190 = 0;
        if (flags & 1) {
            D_8007A190 = 1;
        }
    }
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSetAnimGroup(s32 animGroup) {
    D_8007A164 = animGroup;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetAnimGroup(void) {
    return D_8007A160;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainChangeCameras(s32 cameraCount) {
    D_8007A170 = cameraCount;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNextCharacter(void) {
    return D_8007A154;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNextLevel(void) {
    return D_8007A14C;
}

/*
 * PROVENANCE: control flow adapted from Jet Force Gemini's published
 * src/main.c::func_800468EC_474EC (kiosk listing name func_80046070). JFG
 * still ships that body as assembly; the C here follows its shared-engine
 * unload/reload sequence (countdown, module teardown, tune handoff, video
 * mode, gfx/cam/level reinit) with Mickey names and Mickey-only calls.
 * Mickey's ROM decides disagreements: no JFG game-flag/world/subtitle path,
 * and the clear uses func_8004989C rather than JFG's setupClearScreen.
 *
 * What matched it: this procedure is optimised (the TU carried an Olimit cap
 * that kept uopt off it); the two character tables are six-pass loops, which
 * uopt unrolls into two folded passes and one runtime group of four; the
 * clear is a post-decrement while loop, which it does not unroll; the tune
 * test compares the two call results directly, so the first is a byte
 * temporary below the locals; the clear's locals belong to its own block,
 * after the display-list macros' temporaries; and the two fade arguments are
 * float literals.
 */
void func_80028564(s32 updateRate) {
    s32 screenMode;
    s32 i;

    mainPreNMI();
    D_8007A19C = 0;
    if (D_8007A194 != 0) {
        D_8007A194 -= updateRate;
        if (D_8007A194 <= 0) {
            if ((viDisplayingScreen0() == 0) || (func_80037664() == 1)) {
                D_8007A194 = 1;
                return;
            }
            if (D_800D18E0->pad0[0] == 1) {
                TrapDanglingJump();
                TrapDanglingJump();
                TrapDanglingJump();
            }
            D_8007A320 = 1;
            mmFlushFreeStack();
            mmSetDelay(0);
            mainPreNMI();
            frontFreeMode();
            mainPreNMI();
            if (runlinkIsModuleLoaded(0x22)) {
                TrapDanglingJump();
            }
            if (runlinkIsModuleLoaded(0xC)) {
                TrapDanglingJump();
            }
            if (runlinkIsModuleLoaded(0xE)) {
                TrapDanglingJump();
            }
            if (runlinkIsModuleLoaded(0xB)) {
                TrapDanglingJump();
            }
            mainPreNMI();
            func_80037150();
            if (D_8007A198) {
                mainPreNMI();
                if (D_8007A190) {
                    if ((D_800D18E0->pad0[0] == 5) ||
                        (D_800D18E0->pad0[0] == 6)) {
                        TrapDanglingJump();
                    } else {
                        TrapDanglingJump();
                    }
                }
                levelFreeAll();
                mainPreNMI();
                reset_particles();
                mainPreNMI();
                func_8004E99C();
                D_800CF518 = D_800CF510[D_8007A1B8];
                gDPFullSync(D_800CF518++);
                gSPEndDisplayList(D_800CF518++);
                D_8007A198 = 0;
            }
            if (D_8007A1EC) {
                D_8007A1CC |= 0x08000000;
                D_8007A1EC = 0;
            }
            if (amTuneGetSeqNo() != levelGetTune(D_8007A14C)) {
                amTuneStop();
            }
            D_8007A148 = D_8007A14C;
            D_8007A160 = D_8007A164;
            D_8007A158 = D_8007A15C;
            D_8007A150 = D_8007A154;
            D_8007A168 = D_8007A16C;
            if (D_8007A18C) {
                D_8007BEF8 = D_8007A174;
                D_8007BEFC = D_8007A178;
                D_8007BF0C = D_8007A17C;
                D_8007BF04 = D_8007A180;
                D_800D18E0->pad0[0] = D_8007A184;
                if (D_8007A188 != 0) {
                    for (i = 0; i < 6; i++) {
                        D_800D18E0[i].character = ((u8 *) D_8007A188)[i];
                    }
                } else {
                    for (i = 0; i < 6; i++) {
                        D_800D18E0[i].character = i;
                    }
                }
                D_8007A18C = 0;
            }
            if (D_8007A170) {
                D_8007BEF4 = D_8007A170;
                D_8007A170 = 0;
            }
            mainPreNMI();
            screenMode = frontGetLevelScreenMode(D_8007A148);
            viSetTrippleBuffer(levelGetScreenMode(D_8007A148));
            if ((viGetVideoMode() != screenMode) || viChangeBuffers()) {
                viChangeMode(screenMode);
            } else {
                u32 pixelCount;
                s32 fill;
                s32 *framebuffer;
                s32 width;
                s32 height;

                viGetCurrentSize(&width, &height);
                framebuffer = D_800D2FA0;
                pixelCount = (u32) (width * height) >> 1;
                fill = func_8004989C(4, framebuffer);
                while (pixelCount--) {
                    *framebuffer++ = fill;
                }
            }
            D_8007A6A8 = 0;
            mainPreNMI();
            mmFlushFreeStack();
            mmSetDelay(0);
            mainPreNMI();
            TrapDanglingJump(levelGetGfxIndex(D_8007A148));
            runlinkFreeCode(0x12);
            mainPreNMI();
            mmFlushFreeStack();
            mmSetDelay(0);
            camInit();
            func_8005A764();
            mainPreNMI();
            func_8004E8E0();
            mainPreNMI();
            TrapDanglingJump(0x10, 0x10, 0x64, 0xA, 0xA, 0xC, 0xA);
            mainPreNMI();
            runlinkFreeCode(0x1F);
            mainPreNMI();
            levelInit(D_8007A148, D_8007A158, D_8007A150, D_8007A160);
            mainPreNMI();
            func_80049A8C(-1);
            if ((s8) levelGetLevel()[0x83] == 0) {
                if (D_8007A168 == 3) {
                    func_80037414(1, 1.5f, 0.0f, 0, 0, 0, 0);
                } else {
                    func_80037414(1, 3.0f, 0.0f, 0, 0, 0, 0);
                }
            } else {
                if (D_8007A1A0) {
                    func_800498FC(4, 0.34f, 0.0f, 0, 0, 0, 0x80);
                } else {
                    func_800498FC(4, 0.34f, 0.0f, 0xFF, 0xFF, 0xFF, 0x80);
                }
                func_8004978C(4, 4, 1);
            }
            osSetTime(0);
            mainPreNMI();
            frontSetMode(D_8007A168);
            mmSetDelay(2);
            rumbleUpdate();
            D_8007A1CC = joyRead(D_8007A1CC, 2);
            D_8007A1A8 = 0;
            D_8007A194 = 0;
            D_8007A198 = 1;
            D_8007A19C = 1;
            if (D_8007A1B0) {
                amWaitForMidiSync();
                D_8007A1B0 = 0;
            }
            rumbleRumbles(1);
            D_8007A320 = 0;
        }
    }
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSyncNextLevel(void) {
    D_8007A1B0 = 1;
}

s32 mainGetMode(void) {
    return D_8007A1BC;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSetMode(s32 modeToSet) {
    D_8007A1BC = modeToSet;
}

/* PROVENANCE: function role adapted from JFG src/main.c order and body. */
void mainTitlePageInit(s32 unused) {
    D_8007A1BC = 1;
    D_8007A1C0 = 1;
    func_8003A544(0);
    mainChangeLevel(D_8007A250[D_8007A258], 0, 0, 3, 1, 0);
    mainChangeCameras(1);
    func_80028DE4(6, 6, 0, 2, 0, 0);
    func_80005548(6);
    joyResetMap();
    D_8007A258 = (D_8007A258 + 1) & 3;
}

void func_80028DE4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    D_8007A174 = arg0;
    D_8007A178 = arg1;
    D_8007A17C = arg2;
    D_8007A180 = arg3;
    D_8007A184 = arg4;
    D_8007A188 = arg5;
    D_8007A18C = 1;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainFrontInit(s32 mode, s32 level, s32 unused) {
    D_8007A1BC = 1;
    D_8007A1C0 = 1;
    if (D_8007A1AC == 0) {
        D_8007A1AC = 1;
    }
    mainChangeLevel(level, D_8007A154, 0, mode, 1, 0);
}

/* PROVENANCE: stripped function role follows JFG src/main.c order. */
void mainStartGame(void) {
}

void func_80028EA0(MainGameState *state) {
    s32 i;

    state->flag0 = 0;
    state->flag1 = 0;
    for (i = 0; i < 6; i++) {
        func_80028EFC(&state->characters[i], i, i);
    }
}

void func_80028EFC(MainCharacterState *state, s32 character, s32 variant) {
    s32 i;
    u8 *cursor;

    state->character = character;
    state->variant = variant;
    state->variantCopy = variant;
    state->value22 = 0;
    i = 0;
    cursor = state;
    while (TRUE) {
        i++;
        cursor++;
        cursor[0x1B] = 0;
        if (i < 6) {
            continue;
        }
        break;
    }
    state->flags[1] = 0;
    state->flags[2] = 0;
    state->flags[3] = 0;
    state->flags[0] = 0;
}

void func_80028F3C(void) {
}

void func_80028F44(void) {
}

void func_80028F4C(void) {
}

void *func_80028F54(void) {
    return D_800D18E0;
}

u8 *func_80028F60(s32 index, s32 unused) {
    if ((index < 0) || (index >= 6)) {
        index = 0;
    }
    return (u8 *) D_800D18E0 + (index * 40) + 4;
}

void func_80028F98(s32 unused0, s32 unused1, s32 unused2) {
}

void func_80028FA8(s32 unused0, s32 unused1, s32 unused2) {
}

s32 func_80028FB8(s32 unused0, s32 unused1, s32 unused2) {
    return 0;
}

/*
 * PROVENANCE: Jet Force Gemini src/main.c::mainAnyoneHas at upstream commit
 * efd5abb remains GLOBAL_ASM and supplies only an assembly/object structural
 * analogue and TU-role comparison; no C body was adapted. Mickey establishes
 * this boundary, ABI, calls, Boolean normalization, and candidate body.
 *
 * Matched 2026-10-07 as a plain short-circuit or with a plain `int` return
 * type. With the s32 (`long`) return type the int-valued `||` is converted
 * on return, and uopt colours the carrier a0 with one move at the join;
 * returned as int, each arm normalises into a ring temp and copies it to v0
 * in the branch delay slot, as shipped.
 */
int func_80028FCC(s32 value) {
    return func_80028FB8(0, 0, value) || func_80028FB8(0, 0, value) ||
           func_80028FB8(0, 0, value);
}

s32 func_80029038(s32 unused0, s32 unused1, s32 unused2) {
    return 0;
}

s32 func_8002904C(s32 unused0, s32 unused1) {
    return 0;
}

s32 func_8002905C(s32 value) {
    return func_8002904C((s32) D_800D18E0, value);
}

void func_80029084(s32 unused0, s32 unused1) {
}

void func_80029090(s32 unused0, s32 unused1, s32 unused2) {
}

s32 func_800290A0(void) {
    return D_8007A1A8;
}

void func_800290AC(s32 newValue) {
    register s32 *value = &D_8007A1A8;

    *value = newValue;
    if (*value == 0) {
        amSetMuteMode(0);
        return;
    }
    amSetMuteMode(1);
}

s32 func_800290EC(void) {
    return D_8007A148;
}

void *func_800290F8(void) {
    return D_800D18E4;
}

void func_80029104(void) {
    D_8007A1CC |= 0x40000000;
}

void func_80029120(u32 value) {
    D_8007A1CC |= 0x20000000 | (value & 0x1F);
}

void func_80029144(void) {
    D_8007A1CC |= 0x80000000;
}

void func_80029160(void) {
    D_8007A1CC |= 0x10000000;
}

void func_8002917C(void) {
    D_8007A1CC |= 0x08000000;
}

void func_80029198(void) {
    D_8007A1CC |= 0x00800000;
}

void func_800291B4(void) {
    D_8007A1EC = 1;
}

u8 *func_800291C4(void) {
    return D_800D1928;
}

void func_800291D0(void) {
}

void func_800291D8(s32 value) {
    D_8007A1A4 = value;
}

s32 func_800291E4(void) {
    return D_8007A1D4;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNumberOfCameras(void) {
    return D_8007BEF4;
}

s32 func_800291FC(void) {
    s32 type = levelNGetType(D_8007A148);

    if ((type == 1) || (type == 2)) {
        return 0;
    }
    return D_8007BEF8;
}

u8 func_80029240(s32 index) {
    if ((index < 0) || (index >= 6)) {
        index = 0;
    }
    return D_800D18E0[index].character;
}

/* PROVENANCE: structural comparison uses Jet Force Gemini's public decomp,
 * src/overlays/o3/overlay_3.c::GetSmoothAcceleration. Rechecked at upstream
 * efd5abb1c79636e297b831f7c2d5bf47eac39c0c, JFG retains assembly, so no body
 * was adapted from that revision. This body is reconstructed from Mickey-only
 * control-flow evidence, and Mickey byte identity remains decisive.
 *
 * What the allocator needed, read from its decision records: the float
 * parameters are used and negated in place (no carrier copies); `result` is
 * initialised with the two accumulators, which numbers its web ahead of the
 * parameters so it takes the register the conversion later needs; the early
 * exit is one conditional expression; and the loop bound is the conversion
 * itself, named only after the loop, so no separate copy of it survives. */
f32 func_80029274(s32 distance, f32 speed, f32 acceleration) {
    f32 sum;
    f32 step;
    f32 result;
    f32 dist;
    f32 lower;
    f32 next;
    s32 neg;

    sum = 0.0f;
    step = 0.0f;
    result = 0.0f;
    neg = distance < 0;
    if (neg) {
        distance = -distance;
        speed = -speed;
    }
    if (speed < 0.0f) {
        return neg ? -acceleration : acceleration;
    }
    do {
        step += acceleration;
        sum += step;
    } while ((sum + step) < (f32) distance);
    dist = (f32) distance;
    if ((dist <= acceleration) && (speed <= acceleration) &&
        (((distance >= 0) && (speed >= 0.0f)) || ((distance <= 0) && (speed <= 0.0f)))) {
        result = 0.0f;
    } else {
        lower = step - acceleration;
        next = speed + acceleration;
        if (next <= lower) {
            result = next;
        } else {
            next = speed - acceleration;
            if (next < step) {
                result = lower;
                if (step == acceleration) {
                    result = acceleration;
                }
            } else {
                result = next;
                if (next == 0.0f) {
                    result = acceleration;
                }
            }
        }
    }
    if (neg) {
        result = -result;
        speed = -speed;
    }
    return result - speed;
}

/*
 * PROVENANCE: the debug-memory role was compared with JFG src/main.c, where
 * debug_print_memory remains assembly. This body is reconstructed from
 * Mickey's own call graph, data accesses and ABI evidence.
 */
void func_800293D0(void) {
    f32 ratio;
    char text[64];
    MainDebugMemory *memory;

    if (D_8007A168 != 13) {
        func_8004B0A4(2);
        fontColour(255, 255, 255, 255, 255);
        func_8004B0DC(0, 0, 0, 0);
        memory = func_80005820(0);
        if ((memory != NULL) || (D_80078DF0 != 0)) {
            frontDrawRectangle(&D_800CF518, 0x18, 0xAC, 0x6C, 0xD8, 0xC0);
        }
        if (memory != NULL) {
            sprintf(text, D_80081B98, (s32) memory->valueC);
            func_8004B0F8(&D_800CF518, 0x1C, 0xAF, text, 0);
            sprintf(text, D_80081BA0, (s32) memory->value10);
            func_8004B0F8(&D_800CF518, 0x1C, 0xB9, text, 0);
            sprintf(text, D_80081BA8, (s32) memory->value14);
            func_8004B0F8(&D_800CF518, 0x1C, 0xC3, text, 0);
            ratio = (f32) memory->count / D_80081BD0;
            sprintf(text, D_80081BB0, &ratio);
            func_8004B0F8(&D_800CF518, 0x1C, 0xCD, text, 0);
        }
        if (D_80078DF0 & 1) {
            func_8004B0F8(&D_800CF518, 0x62, 0xAF, D_80081BBC, 0);
        }
        if (D_80078DF0 & 2) {
            func_8004B0F8(&D_800CF518, 0x62, 0xB9, D_80081BC0, 0);
        }
        if (D_80078DF0 & 4) {
            func_8004B0F8(&D_800CF518, 0x62, 0xC3, D_80081BC4, 0);
        }
        if (D_80078DF0 & 8) {
            func_8004B0F8(&D_800CF518, 0x62, 0xCD, D_80081BC8, 0);
        }
    }
}
