#include "PR/ultratypes.h"

typedef struct Overlay11Status {
    u8 mode;
    u8 value1;
    u8 value2;
    u8 value3;
} Overlay11Status;

extern s32 gOverlay11Argument, gOverlay11Timer, gOverlay11Counter;
extern s16 gOverlay11Selection;
extern s32 gOverlay11Initialized, gOverlay11Variant;
extern void func_800290AC(s32 mode);
extern void amTuneSetFade(f32 value, s32 arg1);
extern void overlay66Select(s32 selection);
extern Overlay11Status *func_80028F54(void);
extern void func_8004B0A4(s32 mode);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern void func_overlay_011_F0000AF4_186933C(void);
extern void func_overlay_011_F0000C88_18694D0(void);
extern void func_overlay_011_F0000D70_18695B8(void);
extern void func_overlay_011_F0000DFC_1869644(void);
extern void func_overlay_011_F0000EE4_186972C(void);
extern void func_overlay_011_F0000FCC_1869814(void);
extern void func_overlay_011_F0000A18_1869260(void);

/* DKR v77/v80 and JFG contain no matching Overlay 11 initializer donor. */
void overlay11Initialize(s32 argument) {
    Overlay11Status *status;

    func_800290AC(1);
    amTuneSetFade(0.5f, 0);
    gOverlay11Argument = argument;
    gOverlay11Timer = 30;
    gOverlay11Counter = 0;
    gOverlay11Selection = 0;
    overlay66Select(1);
    status = func_80028F54();
    if (gOverlay11Initialized == 0) {
        func_8004B0A4(3);
        func_8004B0B8(255, 255, 255, 0, 255);
        switch (status->mode) {
        case 0:
            func_overlay_011_F0000AF4_186933C();
            break;
        case 1:
            func_overlay_011_F0000C88_18694D0();
            break;
        case 3:
            func_overlay_011_F0000D70_18695B8();
            break;
        case 4:
        case 5:
            if (gOverlay11Variant == 1) {
                func_overlay_011_F0000EE4_186972C();
            } else {
                func_overlay_011_F0000DFC_1869644();
            }
            break;
        case 2:
            func_overlay_011_F0000FCC_1869814();
            break;
        }
        gOverlay11Argument = 0;
        func_overlay_011_F0000A18_1869260();
        gOverlay11Initialized = 1;
    }
}
