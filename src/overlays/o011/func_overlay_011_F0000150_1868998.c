#include "PR/ultratypes.h"

typedef struct O11Gfx {
    u32 w0;
    u32 w1;
} O11Gfx;

typedef struct O11Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O11Vertex;

typedef struct O11Status {
    u8 mode;
    u8 value1;
    u8 value2;
    u8 value3;
} O11Status;

extern s16 gO11GridPhases[13][17];
extern u8 gO11GridTriangles[];
extern O11Gfx *gO11DisplayListReloc;
extern void *gO11MatrixReloc;
extern O11Vertex *gO11VertexReloc;

extern f32 D_1B0;
extern s16 D_1B8;
extern s16 D_1BC;
extern s32 D_1C0;
extern s32 D_1C4;
/* Relocation identities (checklist item 20): the pad index is bss +0x1C4,
 * a different object from the option action at data +0x1C4, and options 4
 * and 5 test the resident game-state word, not the action. */
extern s32 gO11PadIndex;
extern s32 gO11GameStateReloc;
extern s32 D_200;
extern s32 D_204;

extern s32 func_800290A0(void);
extern s32 func_8002997C(s32 minimum, s32 maximum);
extern void func_80033CBC(u32 *width, u32 *height);
extern void func_80022A50(O11Gfx **displayList, void **matrix);
extern void func_800349A4(O11Gfx **displayList, void *texture, s32 flags,
                          s16 parameter);
extern f32 func_8002A8C0(s32 angle);
extern u32 func_8002554C(s32 controller);
extern void overlay66Select(s32 selection);
extern void func_800290AC(s32 mode);
extern void func_800291D8(s32 value);
extern void func_800006BC(f32 value, s32 volume);
extern void func_8003A754(void);
extern O11Status *func_80028F54(void);

extern void func_overlay_011_F00011D0_1869A18(s32 updateRate);
extern void func_overlay_011_F0001398_1869BE0(s32 updateRate);
extern void func_overlay_011_F000184C_186A094(s32 updateRate);
extern void func_overlay_011_F0001A7C_186A2C4(s32 updateRate);
extern void func_overlay_011_F0001E4C_186A694(s32 updateRate);
extern void func_overlay_011_F00022E8_186AB30(s32 updateRate);
extern void func_overlay_011_F0002714_186AF5C(s32 updateRate);

#define O11_WRITE_VERTEX(vertexX, vertexY, vertexAlpha) \
        gO11VertexReloc->x = (vertexX); \
        gO11VertexReloc->y = (vertexY); \
        gO11VertexReloc->z = 0; \
        gO11VertexReloc->r = 0; \
        gO11VertexReloc->g = 0; \
        gO11VertexReloc->b = 0; \
        gO11VertexReloc->a = (vertexAlpha); \
        gO11VertexReloc++

/* 2026-10-02 n-ovl6: the scissor word is a float expression
 * (`(s32)((width - 1) * 4.0f) & 0xFFF`, 535 to 522 and -120 to -32 bytes),
 * the column parity is a signed `% 2`, and its stride a `(parity * 7) << 9`
 * (529 masked, -8 bytes).
 * 2026-10-02 x-o051 (534 at -4 to 298 at -8, frame now 0x178):
 * - the vertex writer is a bare statement list, not a `do { } while (0)`
 *   macro: the region the wrapper opens is what stopped IDO unrolling the
 *   six-row loop (in a non-leaf; a leaf unrolls either way). The six rows
 *   are one `for (row = 1; row < 7; row++)` loop, unrolled by four after a
 *   two-row remainder exactly as shipped;
 * - every packet is `command = gO11DisplayListReloc++` then two stores;
 * - x, top, y and the row y are plain s32 locals (no s16 re-extension);
 * - the vertex-address byte is masked (`& 0xFF`) before the shift;
 * - the pad index is bss +0x1C4 (not the action at data +0x1C4) and
 *   options 4/5 test the resident game-state word. */
#ifdef NON_MATCHING
void func_overlay_011_F0000150_1868998(O11Gfx **displayList, void **matrix,
                                        O11Vertex **vertices,
                                        s32 updateRate) {
    u8 alpha[13][17];
    u32 width;
    u32 height;
    s32 x;
    s32 top;
    s32 y;
    s32 yy;
    s32 row;
    s32 column;
    s32 block;
    O11Gfx *command;

    if (func_800290A0() != 0) {
        if (D_1B0 == 0.0f) {
            for (row = 0; row < 13; row++) {
                for (column = 0; column < 17; column++) {
                    gO11GridPhases[row][column] =
                        func_8002997C(-0x8000, 0x7FFF);
                }
            }
        }
        /* Two rodata pool entries of 0.05f (+0x20, +0x24): the pool is keyed
         * on spelling (L103), so the two uses are spelled differently. */
        D_1B0 += 0.05f * (f32)updateRate;
        if (D_1B0 > 1.0f) {
            D_1B0 = 1.0f;
        }
    } else {
        D_1B0 -= .05f * (f32)updateRate;
        if (D_1B0 <= 0.0f) {
            D_1B0 = 0.0f;
            return;
        }
    }

    gO11DisplayListReloc = *displayList;
    gO11MatrixReloc = *matrix;
    gO11VertexReloc = *vertices;

    func_80033CBC(&width, &height);
    command = gO11DisplayListReloc++;
    command->w0 = 0xED000000;
    command->w1 =
        ((((s32)((width - 1) * 4.0f)) & 0xFFF) << 12) |
        (((s32)((height - 1) * 4.0f)) & 0xFFF);
    func_80022A50(&gO11DisplayListReloc, &gO11MatrixReloc);
    func_800349A4(&gO11DisplayListReloc, 0, 4, 0);
    command = gO11DisplayListReloc++;
    command->w0 = 0xFCFFFFFF;
    command->w1 = 0xFFFE793C;

    for (row = 0; row < 13; row++) {
        for (column = 0; column < 17; column++) {
            gO11GridPhases[row][column] += updateRate << 8;
            alpha[row][column] =
                (u8)((func_8002A8C0(gO11GridPhases[row][column]) * 32.0f +
                      96.0f) *
                     D_1B0);
        }
    }

    for (block = 0; block < 2; block++) {
        x = -160;
        top = 120 - block * 120;
        y = 100 - block * 120;
        for (column = 0; column < 17; column++) {
            command = gO11DisplayListReloc++;
            command->w0 = 0x04000000 |
                          ((((((u32)gO11VertexReloc + 0x80000000) & 6) |
                            0x38) & 0xFF) << 16) |
                          ((((column % 2 * 7) << 9) | 0x4E) & 0xFFFF);
            command->w1 = (u32)gO11VertexReloc + 0x80000000;
            if (column != 0) {
                command = gO11DisplayListReloc++;
                command->w0 = 0x05B100C0;
                command->w1 =
                    (u32)(gO11GridTriangles + (column % 2 * 0xC0)) + 0x80000000;
            }
            O11_WRITE_VERTEX(x, top, alpha[block * 6][column]);
            yy = y;
            for (row = 1; row < 7; row++) {
                O11_WRITE_VERTEX(x, yy, alpha[block * 6 + row][column]);
                yy -= 20;
            }
            x += 20;
        }
    }

    if ((D_200 == 0) && (D_204 == 0)) {
        if ((D_1C0 == 0) && (D_1C4 == 0) &&
            (func_8002554C(gO11PadIndex) & 0x5000)) {
            overlay66Select(0);
            func_800290AC(0);
            func_800291D8(0x1E);
            func_800006BC(0.5f, 0x7F);
            func_8003A754();
            D_204 = 1;
        } else {
            D_1B8 += D_1BC * updateRate;
            if (D_1B8 >= 0x100) {
                D_1B8 = 0x1FF - D_1B8;
                D_1BC = -D_1BC;
            } else if (D_1B8 < 0) {
                D_1B8 = -D_1B8;
                D_1BC = -D_1BC;
            }
            if (D_1C0 != 0) {
                func_overlay_011_F00011D0_1869A18(updateRate);
            } else {
                switch (func_80028F54()->mode) {
                case 0:
                    func_overlay_011_F0001398_1869BE0(updateRate);
                    break;
                case 1:
                    func_overlay_011_F0001A7C_186A2C4(updateRate);
                    break;
                case 2:
                    func_overlay_011_F0002714_186AF5C(updateRate);
                    break;
                case 3:
                    func_overlay_011_F000184C_186A094(updateRate);
                    break;
                case 4:
                case 5:
                    if (gO11GameStateReloc == 1) {
                        func_overlay_011_F00022E8_186AB30(updateRate);
                    } else {
                        func_overlay_011_F0001E4C_186A694(updateRate);
                    }
                    break;
                }
            }
        }
    }

    if (D_200 != 0) {
        D_200 -= updateRate;
        if (D_200 < 0) {
            D_200 = 0;
        }
    }

    *displayList = gO11DisplayListReloc;
    *matrix = gO11MatrixReloc;
    *vertices = gO11VertexReloc;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o011/func_overlay_011_F0000150_1868998/func_overlay_011_F0000150_1868998.s")
#endif

#undef O11_WRITE_VERTEX

/* PLATEAU-HANDOFF:func_overlay_011_F0000150_1868998:start
 * symbol: func_overlay_011_F0000150_1868998
 * score: 298/564 words
 * frame: 0x178
 * relocations: 81
 * first-mismatch: +0x7C
 * summary: Bare vertex writer (a do-while(0) macro stops IDO's unroller), command packets, s32 coordinates: 534 to 298 at -8, frame 0x178.
 * PLATEAU-HANDOFF:func_overlay_011_F0000150_1868998:end
 */
