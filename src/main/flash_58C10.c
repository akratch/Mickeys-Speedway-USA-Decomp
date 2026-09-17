#include "PR/ultratypes.h"
#include "PR/os_gbpak.h"

extern s32 func_8006A5A0(OSMesgQueue *queue, OSPfs *pfs, s32 channel);
extern s32 func_8006AC60(OSPfs *pfs, s32 flag);
extern s32 func_8006B020(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size);
extern OSMesgQueue *joyMessageQ(void);
extern OSPfs D_800D7830;
extern OSGbpakId D_800D77E0;
extern OSGbpakId *D_8007F7A0;
extern s8 D_8007F7A4;
extern s32 D_8007A1CC;
extern s32 D_800D789C;
extern s32 D_800D78A4;
extern s32 D_800D78AC;
extern s32 D_800D7898;
extern u8 D_800D789A[];
extern u8 *D_800D78A0[];
extern s32 D_800D78A8[];

void func_80058010(void) {
    u8 status[5];

    if (func_8006A5A0(joyMessageQ(), &D_800D7830, 0) == 0) {
        if (osGbpakCheckConnector(&D_800D7830, &status[3]) == 0) {
            D_8007F7A4 = 1;
            return;
        }
        func_8006AC60(&D_800D7830, 0);
    }
}
/* This body was byte-exact for years; what blocked the match was the carve.
 * Nothing in the ROM references 0x800580E8, so splat folded the `jr $ra`/`nop`
 * that lives there into this function's span, and every measurement read the
 * function as two instructions short. Seven different return spellings
 * (early return, trailing return, goto, while/break, do-while(0)) all fold to
 * the same 27 instructions, which is what ruled a dead epilogue out. The 8
 * bytes are func_800580E8 below, an empty stub; inter-TU padding in this
 * region is zero-filled, not a return sequence.
 */
void func_8005807C(void) {
    u8 status;

    if (D_8007F7A4 != 0) {
        if (osGbpakReadId(&D_800D7830, &D_800D77E0, &status) == 0) {
            D_8007F7A0 = &D_800D77E0;
        } else {
            D_8007F7A0 = NULL;
        }
        func_8006AC60(&D_800D7830, 0);
    }
}

void func_800580E8(void) {
}
/* Each inner call sits in its own do-while(0) region so &D_800D7830 stays an
 * address constant and rematerializes (L97/L110). One unused OSPfs * keeps the
 * 0x30 frame after that spill disappears (L99). The first status call still
 * takes the signed address so its identity stays distinct from the plains.
 */
void func_800580F0(s32 arg0) {
    u8 status;
    OSPfs *pfs;

    if (D_8007F7A4 != 0) {
        osGbpakGetStatus((OSPfs *)(s32)&D_800D7830, &status);
        if (func_8006AC60(&D_800D7830, 1) == 0) {
            do {
            osGbpakCheckConnector(&D_800D7830, &status);
            } while (0);
            do {
            func_8006B020(&D_800D7830, (u16)arg0,
                          *(u16 *)(&D_800D789A[arg0 * 4]),
                          D_800D78A0[arg0], D_800D78A8[arg0]);
            } while (0);
            do {
            func_8006AC60(&D_800D7830, 0);
            } while (0);
            if (osGbpakGetStatus(&D_800D7830, &status) != 0) {
                    func_80058010();
                }
            }
    }
}
void func_800581BC(void) {
    D_8007A1CC |= 0x04000000;
}
void func_800581D8(s32 arg0, s32 arg1, s32 arg2) {
    D_8007A1CC |= 0x02000000;
    D_800D789C = arg0;
    D_800D78A4 = arg1;
    D_800D78AC = arg2;
}
void func_8005820C(s32 arg0, s32 arg1, s32 arg2) {
    D_8007A1CC |= 0x01000000;
    D_800D7898 = arg0;
    D_800D78A0[0] = (u8 *)arg1;
    D_800D78A8[0] = arg2;
}

OSGbpakId *func_80058240(void) {
    return D_8007F7A0;
}
