#include "PR/os_internal.h"

typedef struct Overlay18Gfx {
    u32 w0;
    u32 w1;
} Overlay18Gfx;

extern void mainPreNMI(void);
extern void overlay5InitializeAudio(void *value);
extern void runlinkFreeCode(s32 phase);
extern void func_80002500(void);
extern void *joyInit(void);
extern void texInitTextures(void);
extern void modInitModels(void);
extern void func_8005A700(void);
extern void overlay30Initialize(void);
extern void arithmeticFunction(void *localData, void *externalData);
extern void diPrintfInit(void);
extern void func_overlay_031_F00002E8_187F808(void);
extern void initWeather(void);
extern void overlay18Initialize(void);
extern void func_8004A0F0(void);
extern void camlightInit(void);
extern void overlay10Initialize(void);
extern void packInit(void);
extern void initFront(void);
extern void animseqInit(void);
extern void fxInit(void);
extern void diRcpTraceInit(void);
extern void fmvInit(void);
extern void overlay18InitializeBuffers(void);
/* Runtime relocation metadata resolves this role to osSetTime(OSTime). */
extern void osSetTime(OSTime);
extern void func_80029970(u32);
extern void func_80029104(void);

extern void *D_800CF5B8;
extern void *D_8007A1D4;
extern u8 gOverlay18LocalData80[];
extern u8 gOverlay18ExternalData;
extern s32 D_8007A1B8;
extern Overlay18Gfx *D_800CF510[];
extern Overlay18Gfx *D_800CF518;

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
        D_800CF510[D_8007A1B8];
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
