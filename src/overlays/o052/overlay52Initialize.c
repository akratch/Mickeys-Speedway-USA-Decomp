#include "PR/ultratypes.h"

typedef struct O52Record {
    u8 pad00[8];
    s32 unk08;
    s16 unk0C;
    s16 unk0E;
    u8 pad10[8];
    s32 unk18;
    s16 unk1C;
    s16 unk1E;
    u8 pad20[8];
    s32 unk28;
    s16 unk2C;
    s16 unk2E;
    u8 pad30[8];
    s32 unk38;
    s16 unk3C;
    s16 unk3E;
} O52Record;

typedef struct O52State {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    u8 pad0C[4];
    s32 unk10;
} O52State;

extern u8 D_0[];
extern u8 D_OBJECT0[];
extern u8 D_BSS0[];
extern u8 D_24[];
extern u8 D_30[];
extern u8 D_60[];
extern u8 D_80[];
extern u8 D_A0[];
extern u8 D_B0[];
extern O52Record D_F0[];
extern u8 D_100[];
extern O52Record D_170[];
extern u8 D_180[];
extern u8 D_200[];
extern u8 D_220[];
extern u8 D_2F4[];
extern u8 D_340[];
extern O52State D_480;
extern s32 D_4A4;
extern s32 D_4A8[2];
extern s16 D_4B4[2];
extern s16 D_4B8[2];
extern s16 D_4BC[2];
extern s16 D_4C0[2];
extern s16 D_4C4[2];
extern s16 D_4C8[2];
extern volatile s32 D_4CC;
extern volatile s32 D_4CC_TRUE;
extern s16 D_4D0;
extern s16 D_4D2;
extern f32 D_4B0;
extern s16 D_4A0[2];

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
extern s16 ext_resident_result;
extern s32 ext_resident_word_190;

/* Independently reconstructed from Mickey-local evidence; no DKR/JFG donor. */
/* Workbench p7 batch 12: structure-mismatch; exact 316 instructions/-0x40 frame, 115 masked/119 raw words, first non-relocation +0x17C.
 * Levers: constant-audit plus earliest-loop and record-copy staging; inherited pointer/resident-order probes remain negative.
 * Remains: record-copy temp/pool allocation and resident-field scheduling; GLOBAL_ASM stays canonical. */
#ifdef NON_MATCHING
void func_overlay_052_F0000000_189A670(void) {
    u8 *state;
    s32 i;
    O52Record *src;
    O52Record *dst;

    state = ext_o0_28b04();
    ext_o0_39738(D_0);
    ext_o0_39900(D_24);
    ext_o0_c0(4);
    D_4A4 = 0x104;
    ext_o0_31828(11);
    func_overlay_052_F00004F0_189AB60(D_30);
    func_overlay_052_F00004F0_189AB60(D_60);
    func_overlay_052_F00004F0_189AB60(D_180);
    func_overlay_052_F00004F0_189AB60(D_220);
    func_overlay_052_F00004F0_189AB60(D_80);
    func_overlay_052_F00004F0_189AB60(D_B0);
    func_overlay_052_F00004F0_189AB60(D_2F4);

    {
    u8 *p0 = D_0, *p1 = D_60, *p2 = D_200, *p3 = D_340, *p4 = D_A0, *p5 = D_100;
    i = 0;
    do {
        func_overlay_052_F0000540_189ABB0(D_30, p0, i, 0);
        func_overlay_052_F0000540_189ABB0(D_60, p1, i, 0);
        func_overlay_052_F0000540_189ABB0(D_180, p2, i, 3);
        func_overlay_052_F0000540_189ABB0(D_220, p3, i, 4);
        func_overlay_052_F0000540_189ABB0(D_80, p4, i, 1);
        func_overlay_052_F0000540_189ABB0(D_B0, p5, i, 2);
        p0 += 0x30;
        p1 += 0x20;
        p2 += 0xA0;
        p3 += 0xA0;
        p4 += 0x30;
        p5 += 0x30;
        i++;
    } while (i < 2);
    }

    D_4B0 = -80.0f;
    *(s16 *)(D_OBJECT0 + 0x26) = 40;
    *(f32 *)(D_OBJECT0 + 0x28) = 1.0f;
    ext_o56_118();
    D_4A8[0] = -1;
    D_4A8[1] = -1;
    ext_resident_result = ext_o0_2630c();
    {
        s32 residentWord = *(s32 *)(D_0 + 0xE8);
        s16 residentEE = *(s16 *)(D_0 + 0xEE);
        s16 residentEC = *(s16 *)(D_0 + 0xEC);
        *(s32 *)(D_BSS0 + 0x168) = residentWord;
        *(s16 *)(D_BSS0 + 0x16E) = residentEE;
        *(s16 *)(D_BSS0 + 0x16C) = residentEC;
    }

    src = D_F0;
    dst = D_170;
    do {
        {
        s16 v1C = src->unk1C;
        s16 v1E = src->unk1E;
        s16 v0C = src->unk0C;
        s16 v0E = src->unk0E;
        s32 v08 = src->unk08;
        dst->unk1C = v1C;
        dst->unk1E = v1E;
        dst->unk0C = v0C;
        dst->unk0E = v0E;
        dst->unk08 = v08;
        }
        {
        s16 v3C = src->unk3C;
        s32 v28 = src->unk28;
        s16 v2E = src->unk2E;
        s32 v38 = src->unk38;
        s16 v3E = src->unk3E;
        s32 v18 = src->unk18;
        s16 v2C = src->unk2C;
        src++;
        dst++;
        dst[-1].unk3C = v3C;
        dst[-1].unk28 = v28;
        dst[-1].unk2E = v2E;
        dst[-1].unk38 = v38;
        dst[-1].unk3E = v3E;
        dst[-1].unk18 = v18;
        dst[-1].unk2C = v2C;
        }
    } while (src != D_170);

    if (ext_o0_3a0bc() != 0) {
        D_4BC[0] = -0x500; D_4C0[0] = -0x140;
        D_4BC[1] = 0x400;  D_4C0[1] = -0x140;
        D_4C4[0] = 0x3F0;  D_4C8[0] = 0x3C0;
        D_4C4[1] = 0xCF0;  D_4C8[1] = 0x3C0;
        D_4A0[0] = -0x420; D_4A0[1] = 0x4E0;
    } else if (ext_o0_39e48() & 1) {
        D_4BC[0] = -0x500; D_4C0[0] = -0x140;
        D_4BC[1] = -0x500; D_4C0[1] = -0x140;
        D_4C4[0] = 0x830;  D_4C8[0] = 0x210;
        D_4C4[1] = 0x830;  D_4C8[1] = 0x990;
        D_4A0[0] = -0x280; D_4A0[1] = -0x280;
    } else {
        D_4BC[0] = -0x500; D_4C0[0] = -0x140;
        D_4BC[1] = -0x500; D_4C0[1] = -0x140;
        D_4C4[0] = 0x830;  D_4C8[0] = 0x2D0;
        D_4C4[1] = 0x830;  D_4C8[1] = 0x990;
        D_4A0[0] = -0x280; D_4A0[1] = -0x280;
    }

    D_480.unk08 = 0;
    D_4B4[0] = D_4BC[0]; D_4B4[1] = D_4BC[1];
    D_4B8[0] = D_4C0[0]; D_4B8[1] = D_4C0[1];
    D_480.unk04 = 0;
    D_480.unk10 = 0;
    D_480.unk00 = ext_resident_word_190;
    if (*state == 3) {
        s32 handle;
        ext_o0_4ac54(3);
        handle = ext_o0_2630c();
        handle = ext_o0_3a150(handle);
        handle = ext_o45_c(handle, 0xA0, 0x78, 0xC);
        D_4CC_TRUE = handle;
        ext_o45_1be0(handle, 0);
    } else {
        D_4CC = 0;
    }
    D_4D2 = 0;
    D_4D0 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o052/overlay52Initialize/func_overlay_052_F0000000_189A670.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_052_F0000000_189A670:start
 * symbol: func_overlay_052_F0000000_189A670
 * score: 115/316 words
 * frame: 0x40
 * relocations: 141
 * first-mismatch: +0x2C
 * summary: Proc-0 census confirms 23 draws; incoherent p1 allocation excludes ring transfer while record-copy CFG and relocation identity remain unresolved.
 * PLATEAU-HANDOFF:func_overlay_052_F0000000_189A670:end
 */
