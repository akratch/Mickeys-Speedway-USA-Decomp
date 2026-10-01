#include "PR/ultratypes.h"
#include "overlays/offset_records.h"

typedef struct O52State {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    u8 pad0C[4];
    s32 unk10;
} O52State;

extern u8 D_OBJECT0[];
extern s32 ext_resident_word_190;
extern s16 ext_resident_result;

/* Overlay 52's initialized data, in ROM order: this TU is its byte owner.
 * The original overlay was one translation unit, and the record copy, the
 * bounds stores and the last block are only scheduled the way the ROM has
 * them when the compiler can see every object defined here (a static reached
 * by section-relative address shares one high half; an extern does not).
 * Every object is static so no other TU's extern placeholder resolves to it;
 * the siblings keep their absolute section-relative names. */
static s16 sResourceIds[18] = {
    2, 38, 39, 41, 42, 20, 21, 22, 23, 24, 25, 26, 30, 31, 40, 80, 100, -1,
};
static s16 sPrepareIds[6] = { 0, 1, 2, 3, 4, -1 };
static OverlayOffsetRecord sListA[3] = {
    { 38, 39, 0, 0, 0 },
    { 41, 42, 0, 12, 0 },
};
static OverlayOffsetRecord sListB[2] = {
    { 38, 39, 0x00060000, 0, 0 },
};
static OverlayOffsetRecord sListC[3] = {
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
};
static OverlayOffsetRecord sListD[3] = {
    { 30, 31, 0, 0, -3 },
    { 20, 21, 0, 32, 1 },
};
static OverlayOffsetRecord sSourceRecords[10] = {
    { 20, 21, 0, -7, 0 },
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
    { 20, 21, 0x000B0000, 14, 0 },
    { 20, 21, 0, 20, 0 },
    { 20, 21, 0, 27, 0 },
    { 20, 21, 0x000A0000, 33, 0 },
    { 20, 21, 0, 40, 0 },
    { 20, 21, 0, 47, 0 },
};
static OverlayOffsetRecord sListE[10] = {
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
    { 20, 21, 0x000B0000, 14, 0 },
    { 20, 21, 0, 20, 0 },
    { 20, 21, 0, 27, 0 },
    { 20, 21, 0x000A0000, 33, 0 },
    { 20, 21, 0, 40, 0 },
    { 20, 21, 0, 47, 0 },
    { 23, 24, 0, -25, -8 },
};
static OverlayOffsetRecord sListF[2] = {
    { 25, 0, 0, -25, -6 },
};
static s8 sFlags240[12] = { 1, 1, 0, 0, 0, 1, 2, 3, 3, 3, 0, 0 };
static s16 sTable24C[84] = {
    -12, -120, 0, 160, -84, 66, 25, 251, 31, 257, -80, 26,
    768, 3072, -1056, 1248, 3200, 5504, 1920, 1920, -640, -640, 5760, 5760,
    28, 28, 28, 136, 117, 37, 117, 145, 165, 37, 165, 145,
    128, 37, 128, 145, 128, 37, 128, 145, 28, 28, 262, 28,
    88, 37, 238, 37, 110, 37, 165, 37, 92, 37, 198, 37,
    92, 37, 198, 37, 28, 16, 28, 136, 117, 25, 117, 145,
    165, 25, 165, 145, 128, 25, 128, 145, 128, 25, 128, 145,
};
static OverlayOffsetRecord sListG[2] = {
    { 20, 21, 0, -3, -4 },
};
static s32 sTail314 = 9;
static s32 sTail318 = 0;
static s8 sTail31C = 0;
static s32 sTail320[4] = { 0 };

/* Overlay 52's .bss, in address order (IDO 8-aligns arrays and 4-aligns
 * scalars, which is why the 4-byte groups below are scalars). */
static OverlayOffsetRecord sCopyA[2][3];
static OverlayOffsetRecord sCopyB[2][2];
static OverlayOffsetRecord sCopyC[2][3];
static OverlayOffsetRecord sCopyD[2][3];
static OverlayOffsetRecord sRecords[10];
static OverlayOffsetRecord sCopyE[2][10];
static OverlayOffsetRecord sCopyF[2][10];
static O52State sState;
static s32 sBssPad494;
static s32 sBssPad498;
static s32 sBssPad49C;
static s16 sBounds4A0[2];
static s32 sMode;
static s32 sFlags[2];
static f32 sHeight;
static s16 sB4B4[2];
static s16 sB4B8[2];
static s16 sB4BC[2];
static s16 sB4C0[2];
static s16 sB4C4[2];
static s16 sB4C8[2];
static s32 sCurrent;
static s16 sT4D0;
static s16 sT4D2;

extern u8 *ext_o0_28b04(void);
extern void ext_o0_39738(void *);
extern void ext_o0_39900(void *);
extern void ext_o0_c0(s32);
extern void ext_o0_31828(s32);
extern void func_overlay_052_F00004F0_189AB60(void *);
extern void func_overlay_052_F0000540_189ABB0(void *, void *, s32, s32);
extern void ext_o56_118(void);
extern s32 ext_o0_2630c(void);
extern s32 ext_o0_3a0bc(void);
extern s32 ext_o0_39e48(void);
extern void ext_o0_4ac54(s32);
extern s32 ext_o0_3a150(s32);
extern s32 ext_o45_c(s32, s32, s32, s32);
extern void ext_o45_1be0(s32, s32);

/* Independently reconstructed from Mickey-local evidence; no DKR/JFG donor. */
/* Plateau: 31 masked words at delta 0 with the data owned above (see the
 * shard's 2026-10-01 d-ovl1 section for what each edit was worth). */
#ifdef NON_MATCHING
void func_overlay_052_F0000000_189A670(void) {
    s32 i;
    u8 *state;
    state = ext_o0_28b04();
    ext_o0_39738(sResourceIds);
    ext_o0_39900(sPrepareIds);
    ext_o0_c0(4);
    sMode = 0x104;
    ext_o0_31828(11);
    func_overlay_052_F00004F0_189AB60(sListA);
    func_overlay_052_F00004F0_189AB60(sListB);
    func_overlay_052_F00004F0_189AB60(sListE);
    func_overlay_052_F00004F0_189AB60(sListF);
    func_overlay_052_F00004F0_189AB60(sListC);
    func_overlay_052_F00004F0_189AB60(sListD);
    func_overlay_052_F00004F0_189AB60(sListG);
    for (i = 0; i < 2; i++) {
        func_overlay_052_F0000540_189ABB0(sListA, sCopyA[i], i, 0);
        func_overlay_052_F0000540_189ABB0(sListB, sCopyB[i], i, 0);
        func_overlay_052_F0000540_189ABB0(sListE, sCopyE[i], i, 3);
        func_overlay_052_F0000540_189ABB0(sListF, sCopyF[i], i, 4);
        func_overlay_052_F0000540_189ABB0(sListC, sCopyC[i], i, 1);
        func_overlay_052_F0000540_189ABB0(sListD, sCopyD[i], i, 2);
    }
    sHeight = -80.0f;
    *(s16 *)(D_OBJECT0 + 0x26) = 40;
    *(f32 *)(D_OBJECT0 + 0x28) = 1.0f;
    ext_o56_118();
    sFlags[0] = -1;
    sFlags[1] = -1;
    ext_resident_result = ext_o0_2630c();
    for (i = 0; i < 9; i++) {
        sRecords[i].x = sSourceRecords[i].x;
        sRecords[i].y = sSourceRecords[i].y;
        sRecords[i].metadata = sSourceRecords[i].metadata;
    }
    if (ext_o0_3a0bc() != 0) {
        sB4BC[0] = -0x500; sB4C0[0] = -0x140;
        sB4BC[1] = 0x400; sB4C0[1] = -0x140;
        sB4C4[0] = 0x3F0; sB4C8[0] = 0x3C0;
        sB4C4[1] = 0xCF0; sB4C8[1] = 0x3C0;
        sBounds4A0[0] = -0x420; sBounds4A0[1] = 0x4E0;
    } else if (ext_o0_39e48() & 1) {
        sB4BC[0] = -0x500; sB4C0[0] = -0x140;
        sB4BC[1] = -0x500; sB4C0[1] = -0x140;
        sB4C4[0] = 0x830; sB4C8[0] = 0x210;
        sB4C4[1] = 0x830; sB4C8[1] = 0x990;
        sBounds4A0[0] = -0x280; sBounds4A0[1] = -0x280;
    } else {
        sB4BC[0] = -0x500; sB4C0[0] = -0x140;
        sB4BC[1] = -0x500; sB4C0[1] = -0x140;
        sB4C4[0] = 0x830; sB4C8[0] = 0x2D0;
        sB4C4[1] = 0x830; sB4C8[1] = 0x990;
        sBounds4A0[0] = -0x280; sBounds4A0[1] = -0x280;
    }
    sState.unk00 = ext_resident_word_190;
    sState.unk04 = 0;
    sState.unk08 = 0;
    sState.unk10 = 0;
    sB4B4[0] = sB4BC[0]; sB4B4[1] = sB4BC[1];
    sB4B8[0] = sB4C0[0]; sB4B8[1] = sB4C0[1];
    if (*state == 3) {
        s32 handle;
        ext_o0_4ac54(3);
        handle = ext_o0_2630c();
        handle = ext_o0_3a150(handle);
        sCurrent = ext_o45_c(handle, 0xA0, 0x78, 0xC);
        ext_o45_1be0(sCurrent, 0);
    } else {
        sCurrent = 0;
    }
    sT4D2 = 0;
    sT4D0 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o052/overlay52Initialize/func_overlay_052_F0000000_189A670.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_052_F0000000_189A670:start
 * symbol: func_overlay_052_F0000000_189A670
 * score: 31/316 words
 * frame: 0x40
 * relocations: 141
 * first-mismatch: +0x30C
 * summary: Data owned in C: 115 to 31. Open: bounds s16 pair store order (as1), D_480 base lui hoisted above loads.
 * PLATEAU-HANDOFF:func_overlay_052_F0000000_189A670:end
 */
