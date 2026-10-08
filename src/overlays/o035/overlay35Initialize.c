#include "ultra64.h"

typedef struct O35System {
    u8 pad0[0x82];
    u8 flag82;
    s8 mode83;
    u8 pad84[0xE];
    s16 handle92;
    u8 pad94[0x62];
    s8 flagF6;
} O35System;

extern void *D_o35_alloc;
extern s32 D_4;
extern O35System *D_o35_system;
extern s32 D_o35_reset1;
extern s32 D_o35_reset2;
extern void *D_o35_paramA;
extern void *D_o35_paramB;
extern s32 D_o35_reset3;
extern s32 D_o35_mask;
extern void *D_o35_handle;
extern s32 D_o35_zero;
extern u8 D_o35_byte;

extern void *call_o0_0_2AE30(s32, s32);
extern O35System *call_o0_0_2634C(void);
extern s32 call_o0_0_28DA0(void);
extern void call_o0_0_214D4(s32);
extern void call_o0_0_47D8(s32, s32);
extern void call_o0_0_509A0(s32);
extern void call_o0_0_4B90(s32);
extern void call_o0_0_15A40(void *, void *, s32, s32, s32);
extern void call_o0_0_15C48(s32, s32);
extern void call_o0_0_15BE4(void);
extern void call_o0_0_20114(s32);
extern void call_o0_0_C0F0(s32);
extern void *call_o0_0_33FF8(s16);
extern void call_o98_0_0(void *);
extern void func_overlay_035_F00001E0_1881EC0(s32);

void func_overlay_035_F0000000_1881CE0(s32 modelId, s32 value1, s32 value2,
                                        s32 value3, s32 value4, s32 value5) {
    O35System *system;

    D_o35_alloc = call_o0_0_2AE30(0x800, 0x8F);
    D_o35_system = call_o0_0_2634C();
    D_4 = 0;
    if ((D_o35_system->flagF6 == 0) &&
        ((D_o35_system->mode83 == 1) || (D_o35_system->mode83 == 2))) {
        D_4 = 1;
    }

    func_overlay_035_F00001E0_1881EC0(modelId);
    D_o35_reset1 = 0;
    call_o0_0_214D4(call_o0_0_28DA0() - 1);
    call_o0_0_47D8(value4, 0);
    call_o0_0_47D8(value3, 1);
    call_o0_0_509A0(value5);
    call_o0_0_4B90(value2);

    D_o35_reset2 = 0;
    call_o0_0_15A40(D_o35_paramA, D_o35_paramB, 0x1F4, 0xC8, 0x64);
    call_o0_0_15C48(2, 0);
    call_o0_0_15C48(1, 0);
    call_o0_0_15BE4();
    call_o0_0_15C48(2, 0);
    call_o0_0_15C48(1, 0);
    call_o0_0_15BE4();

    if (D_o35_system->flag82 == 0) {
        call_o0_0_20114(0);
    }
    call_o0_0_C0F0(value1);
    if (D_o35_system->flag82 == 0) {
        call_o0_0_20114(8);
    }

    D_o35_reset3 = 0;
    D_o35_mask = 0x100000;
    if (D_o35_system->handle92 != -1) {
        D_o35_handle = call_o0_0_33FF8(D_o35_system->handle92);
        D_o35_zero = 0; }
    D_o35_byte = 0;
    if (D_o35_system->flagF6 != 0) {
        call_o98_0_0(D_o35_paramA);
    }
}
