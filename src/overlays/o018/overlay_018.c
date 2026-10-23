#include "overlays/overlay_018.h"

/*
 * Overlay 18: boot-time initialization and the display-list heap. One
 * translation unit; overlay18Load and overlay18Reconfigure are the module's
 * exports, the other two are static.
 */

static void overlay18Initialize(void);
static void overlay18InitializeBuffers(void);

/*
 * Pinned DKR v77/v80 and JFG searches found no exact donor. The module's own
 * relocation table identifies osSetTime as resident osSetTime;
 * its OSTime-width argument accounts for the retail a0/a1 zero pair.
 */
void overlay18Load(void) {
    Overlay18Gfx *newDisplay;
    Overlay18Gfx *display;

    mainPreNMI();
    overlay5InitializeAudio(&D_800CF5B8);
    runlinkFreeCode(5);
    mainPreNMI();
    func_80002500();
    D_8007A1D4 = joyInit();
    texInitTextures();
    mainPreNMI();
    modInitModels();
    mainPreNMI();
    func_8005A700();
    mainPreNMI();
    overlay30Initialize();
    runlinkFreeCode(30);
    arithmeticFunction(gOverlay18LocalData80, &gOverlay18ExternalData);
    runlinkFreeCode(39);
    diPrintfInit();
    mainPreNMI();
    func_overlay_031_F00002E8_187F808();
    runlinkFreeCode(31);
    mainPreNMI();
    initWeather();
    mainPreNMI();
    overlay18Initialize();
    mainPreNMI();
    func_8004A0F0();
    mainPreNMI();
    camlightInit();
    mainPreNMI();
    overlay10Initialize();
    runlinkFreeCode(10);
    mainPreNMI();
    packInit();
    mainPreNMI();
    initFront();
    mainPreNMI();
    animseqInit();
    fxInit();
    mainPreNMI();
    diRcpTraceInit();
    fmvInit();
    overlay18InitializeBuffers();
    mainPreNMI();

    D_8007A1B8 = 0;
    D_800CF518 =
        (Overlay18Gfx *)D_800CF510[D_8007A1B8];
    newDisplay = D_800CF518++;
    display = newDisplay;
    display->w0 = 0xE9000000;
    display->w1 = 0;
    display = D_800CF518;
    D_800CF518 = display + 1;
    display->w1 = 0;
    display->w0 = 0xB8000000;

    osSetTime(0);
    func_80029970(0xD43E);
    func_80029104();
}

/* DKR v77/v80 and JFG have no exact donor for this initialization sequence. */
static void overlay18Initialize(void) {
    void *handle;
    register void **slot;

    levelGetCounts();
    D_800CF5B0 = NULL;
    handle = mmAlloc(0xF4, 0x8F);
    slot = &D_800D18E0;
    *slot = handle;
    func_80028EA0(*slot);
    func_80028F3C();
    D_8007A1CC = 0x10000000;
}

/*
 * PROVENANCE: Diddy Kong Racing, src/thread3_main.c
 * (alloc_displaylist_heap); adapted allocation and cursor-update structure.
 */
void overlay18Reconfigure(s32 mode) {
    s32 index;
    s32 size;
    void *token;

    if (mode != gOverlay18Mode) {
        index = mode;
        gOverlay18Mode = mode;
        token = mmGetDelay(mode);
        mmSetDelay(0);

        size = (gOverlay18ModeScale8[index] * 8) +
               (gOverlay18ModeScale64[index] * 64) +
               (gOverlay18ModeScale10[index] * 10) +
               (gOverlay18ModeScale16[index] * 16);

        mmFree(gOverlay18Buffers[0]);
        mmFree(gOverlay18Buffers[1]);
        gOverlay18Buffers[0] =
            mmAllocAtAddr(size, gOverlay18Buffers[0], 0x87);
        gOverlay18Buffers[1] =
            mmAllocAtAddr(size, gOverlay18Buffers[1], 0x87);

        if ((gOverlay18Buffers[0] == 0) ||
            (gOverlay18Buffers[1] == 0)) {
            if (gOverlay18Buffers[0] != 0) {
                mmFree(gOverlay18Buffers[0]);
                gOverlay18Buffers[0] = 0;
            }
            if (gOverlay18Buffers[1] != 0) {
                mmFree(gOverlay18Buffers[1]);
                gOverlay18Buffers[1] = 0;
            }
            overlay18InitializeBuffers();
        }

        gOverlay18Region8Slot0 = gOverlay18Buffers[0] +
                                (gOverlay18ModeScale8[index] * 8);
        gOverlay18Region64Slot0 = gOverlay18Region8[0] +
                                 (gOverlay18ModeScale64[index] * 64);
        gOverlay18Region16Slot0 = gOverlay18Region64[0] +
                                 (gOverlay18ModeScale16[index] * 16);
        D_800CF528[1] = gOverlay18Buffers[1] +
                                    (gOverlay18ModeScale8[index] * 8);
        D_800CF598[1] = gOverlay18Region8[1] +
                                     (gOverlay18ModeScale64[index] * 64);
        D_800CF580[1] = gOverlay18Region64[1] +
                                     (gOverlay18ModeScale16[index] * 16);

        D_800CF51C = gOverlay18ModeScale8[index];
        D_800CF534 = gOverlay18ModeScale64[index];
        D_800CF5A4 = gOverlay18ModeScale16[index];
        D_800CF58C = gOverlay18ModeScale10[index];
        overlay18FinishReconfigureReloc(token);
    }

    D_800CF518 =
        (Overlay18Gfx *)gOverlay18Buffers[D_8007A1B8];
    D_800CF530 = gOverlay18Region8[D_8007A1B8];
    D_800CF5A0 = gOverlay18Region64[D_8007A1B8];
    D_800CF588 = D_800CF580[D_8007A1B8];

    {
        Overlay18Gfx *command;

        command = D_800CF518++;
        command->w1 = 0;
        command->w0 = 0xE9000000;
    }
    {
        Overlay18Gfx *command;

        command = D_800CF518++;
        command->w1 = 0;
        command->w0 = 0xB8000000;
    }
}

/* PROVENANCE: adapted from Diddy Kong Racing src/thread3_main.c
 * (default_alloc_displaylist_heap); Mickey's globals and bytes are authoritative. */
static void overlay18InitializeBuffers(void) {
    s32 mode;
    s32 size;

    mode = 3;
    gOverlay18Mode = mode;
    size = (gOverlay18ModeScale8[mode] * 8) +
           (gOverlay18ModeScale64[mode] * 64) +
           (gOverlay18ModeScale10[mode] * 10) +
           (gOverlay18ModeScale16[mode] * 16);
    D_800CF510[0] = mmAlloc(size, 0x87);
    D_800CF528[0] = D_800CF510[0] +
                           (gOverlay18ModeScale8[mode] * 8);
    D_800CF580[0] = D_800CF528[0] +
                           (gOverlay18ModeScale64[mode] * 64);
    D_800CF598[0] = D_800CF580[0] +
                           (gOverlay18ModeScale10[mode] * 10);

    D_800CF510[1] = mmAlloc(size, 0x87);
    D_800CF528[1] = D_800CF510[1] +
                           (gOverlay18ModeScale8[mode] * 8);
    D_800CF580[1] = D_800CF528[1] +
                           (gOverlay18ModeScale64[mode] * 64);
    D_800CF598[1] = D_800CF580[1] +
                           (gOverlay18ModeScale10[mode] * 10);

    D_800CF51C = gOverlay18ModeScale8[mode];
    D_800CF534 = gOverlay18ModeScale64[mode];
    D_800CF5A4 = gOverlay18ModeScale16[mode];
    D_800CF58C = gOverlay18ModeScale10[mode];
}
