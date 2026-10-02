#include "PR/ultratypes.h"

typedef s32 O53Unknown;

typedef struct O53DrawPacket {
    u32 unk00;
    s32 unk04;
    s32 unk08;
    s16 unk0C;
    s16 unk0E;
    s32 unk10;
} O53DrawPacket;

typedef struct O53Entry {
    void *resource;
    s32 value4;
    s32 value8;
    s16 x;
    s16 y;
} O53Entry;

typedef struct O53Inner {
    u8 pad0[0x19A];
    u8 b19A;
    u8 pad19B;
    s32 w19C;
    u8 pad1A0[0x383 - 0x1A0];
    s8 b383;
    u8 pad384[0x400 - 0x384];
    s32 w400;
} O53Inner;

typedef struct O53Obj {
    u8 pad0[0x64];
    O53Inner *inner;
} O53Obj;

typedef struct O53Cam {
    u8 pad0[0x86];
    s8 b86;
} O53Cam;

extern s32 overlay53ExternalReloc();
extern O53Obj **overlay53ListReloc();
extern u8 *overlay53ByteReloc();
extern O53Cam *overlay53CamReloc();
extern void overlay53FloatReloc(f32 a, s32 b);
extern void overlay53Float3Reloc(s32 a, f32 b, f32 c, s32 d, s32 e, s32 f, s32 g);
extern void overlay53CopyOffsetEntries();

extern s32 D_0;
extern s16 D_84;
extern struct { u8 pad[0x8C]; f32 x; f32 y; } D_HUD;
extern O53Entry D_ENT[][10];
extern s32 D_110;
extern s8 D_114;
extern s16 D_T16[];
extern u32 D_T32[];
extern s16 D_EC[];
extern s16 D_E4[];
extern s16 D_E8[];
extern s16 D_DC[];
extern s32 D_1C;
extern s32 D_118[2];
extern s32 D_140[];
extern s32 D_288[2];
extern f32 D_290;
extern s32 D_P0, D_P1, D_Q0, D_Q1, D_Q2, D_Q3, D_R, D_S, D_T, D_U;

/*
 * Mickey-only reconstruction, no donor. The HUD easing loops are plain for
 * loops under -Wab,-r4300_mul (see mk/overlays.mk); one loop variable serves
 * the whole function; the six time digits are computed as quotient/remainder
 * pairs so each dividend divides once. Callee and data identities are not
 * recoverable from the extracted object, so the placeholder names assert
 * nothing but distinctness.
 */

#ifdef NON_MATCHING
void func_overlay_053_F0000240_189DBE8(s32 arg0) {
    s32 spC8[2];
    O53Cam *spC4;
    s32 spB4[4];
    s32 spB0;
    s32 spAC;
    s32 spA8;
    s32 spA4;
    O53DrawPacket pk;
    s32 sp80;
    u32 sp7C;
    u8 *sp78;
    O53Obj **list;
    O53Obj *obj;
    O53Inner *inner;
    O53Entry *ent;
    s32 i;
    s32 q;
    s32 j;
    s32 d0, d1, d2, d3, d4, d5;

    sp78 = overlay53ByteReloc();
    overlay53ExternalReloc(&sp80, &sp7C);
    sp7C >>= 1;
    overlay53ExternalReloc(&D_P0, &D_P1);
    list = overlay53ListReloc(spB4);
    if (D_0 == 0) {
        for (i = 0; i < arg0; i++) {
            D_290 += (-11.0f - D_290) * 0.125f;
        }
    }
    spA4 = (s32)D_290;
    spC4 = overlay53CamReloc();
    D_114 = D_114 + 1;
    D_114 = D_114 % 10;
    spC8[0] = -1;
    spC8[1] = -1;
    for (i = 0; i < 2; i++) {
        obj = list[i];
        if (obj == 0) {
            return;
        }
        inner = obj->inner;
        if (inner->b19A != 0xFF) {
            D_118[i] += arg0 * 16;
            if (D_118[i] >= 0x100) {
                D_118[i] = 0xFF;
            }
        } else {
            D_118[i] -= arg0 * 8;
            if (D_118[i] < 0) {
                D_118[i] = 0;
            }
        }
        if (D_118[i] > 0) {
            if (inner->w19C != 0) {
                spC8[i] = 0x35;
            } else if (inner->b19A != 0xFF) {
                spC8[i] = D_T16[inner->b19A];
            } else {
                spC8[i] = D_288[i];
            }
            if (spC8[i] != D_288[i]) {
                if (D_288[i] != -1) {
                    overlay53ExternalReloc(D_288[i]);
                }
                D_288[i] = spC8[i];
                if (D_288[i] != -1) {
                    overlay53ExternalReloc(D_288[i]);
                }
            }
        }
    }
    for (q = 0; q < 2; q++) {
        if (D_288[q] != -1 && D_288[q] != spC8[0] && D_288[q] != spC8[1]) {
            overlay53ExternalReloc(D_288[q]);
            D_288[q] = -1;
        }
    }
    for (i = 0; i < 2; i++) {
        if (spC8[i] != -1) {
            D_288[i] = spC8[i];
            if (D_T32[spC8[i]] == 0) {
                overlay53ExternalReloc(spC8[i]);
            }
        }
    }
    for (i = 0; i < 2; i++) {
        obj = list[i];
        if (obj == 0) {
            return;
        }
        inner = obj->inner;
        overlay53ExternalReloc(i);
        overlay53ExternalReloc(&D_Q0);
        if (*sp78 == 6) {
            ent = D_ENT[i];
            overlay53ExternalReloc(inner->w400, &spB0, &spAC, &spA8);
            if (D_0 == 0 && spC4->b86 != inner->b383 &&
                overlay53ExternalReloc() == 0 && inner->w400 != 0x83D60) {
                spA8 = (spA8 - (spA8 % 10)) + D_114;
            }
            overlay53CopyOffsetEntries(&D_1C, ent, i, 0);
            d0 = spB0 / 10;
            d1 = spB0 % 10;
            ent[0].value8 = d0 << 16;
            ent[1].value8 = d1 << 16;
            d2 = spAC / 10;
            d3 = spAC % 10;
            ent[3].value8 = d2 << 16;
            ent[4].value8 = d3 << 16;
            d4 = spA8 / 10;
            d5 = spA8 % 10;
            ent[6].value8 = d4 << 16;
            ent[7].value8 = d5 << 16;
            for (j = 0; j < 8; j++) {
                if ((ent[j].value8 >> 16) == 1) {
                    if (j == 0 || j == 3 || j == 6) {
                        ent[j].x++;
                    } else {
                        ent[j].x--;
                    }
                }
            }
            overlay53ExternalReloc(&D_Q1, ent, 0, spA4, 0xFF, 0xFF, 0xFF, 0xFF);
            overlay53ExternalReloc(&D_Q2);
            if (overlay53ExternalReloc() == 1) {
                D_HUD.y = (f32)(0x50 - spA4);
                D_HUD.x = (f32)D_EC[i];
            } else {
                D_HUD.x = -44.0f;
                D_HUD.y = (f32)(D_DC[i] - spA4 + 0x5C);
            }
            D_84 = (-inner->w400 << 16) / 300;
            overlay53ExternalReloc(4);
            overlay53ExternalReloc(&D_R, D_140 + i * 40, 0, spA4, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (D_118[i] > 0) {
            if (D_288[i] != -1) {
                if (overlay53ExternalReloc() != 0) {
                    if (D_288[i] == 0x35) {
                        pk.unk0E = 0xB4;
                        pk.unk0C = D_E4[i];
                    } else {
                        pk.unk0E = 0xBA;
                        pk.unk0C = D_E8[i];
                    }
                } else {
                    if (D_288[i] == 0x35) {
                        pk.unk0C = 0x19;
                        pk.unk0E = 0x3E - D_DC[i];
                    } else {
                        pk.unk0C = 0x1F;
                        pk.unk0E = 0x44 - D_DC[i];
                    }
                }
                pk.unk08 = 0;
                pk.unk04 = 0;
                pk.unk10 = 0;
                pk.unk00 = D_T32[D_288[i]];
                overlay53ExternalReloc(&D_S, &pk, 0, 0, 0xFF, 0xFF, 0xFF, D_118[i]);
            }
        }
        if (overlay53ExternalReloc() == 0 && *overlay53ByteReloc() == 5 &&
            D_0 == 0 && D_110 == 0) {
            overlay53ExternalReloc(1);
            overlay53ExternalReloc(1);
            overlay53ExternalReloc();
            overlay53Float3Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
            overlay53ExternalReloc(0x12, 0, 0, 7, 1, 1);
            overlay53FloatReloc(3.0f, 0);
            D_110 = 1;
        }
    }
    overlay53ExternalReloc(&D_T, &D_U);
    overlay53ExternalReloc(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o053/func_overlay_053_F0000240_189DBE8/func_overlay_053_F0000240_189DBE8.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_053_F0000240_189DBE8:start
 * symbol: func_overlay_053_F0000240_189DBE8
 * score: 403 differing words
 * frame: 0xF8
 * relocations: 117
 * first-mismatch: +0x0
 * summary: 636 to 403 at delta 0: -Wab,-r4300_mul, single loop var, paired div/mod; frame 0xF8 vs 0xD8 open
 * PLATEAU-HANDOFF:func_overlay_053_F0000240_189DBE8:end
 */
