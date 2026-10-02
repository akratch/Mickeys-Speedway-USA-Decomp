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

/* Overlay-local storage, named by the relocation records' section and
 * addend. The pad index (bss +0x1C4) and the option action (data +0x1C4)
 * are different objects. */
extern s16 gOverlay11GridPhases[13][17];
extern u8 gOverlay11GridTriangles[];
extern f32 gOverlay11Fade;
extern s16 gOverlay11OptionHighlight;
extern s16 gOverlay11HighlightStep;
extern s32 gOverlay11Mode;
extern s32 gOverlay11OptionAction;
extern s32 gOverlay11PadIndex;
extern s32 gOverlay11InputDelay;
extern s32 gOverlay11OptionDone;

/* Resident objects, reached through runtime relocation records. */
extern O11Gfx *gO11DisplayListReloc;
extern void *gO11MatrixReloc;
extern O11Vertex *gO11VertexReloc;
extern s32 gOverlay11GameStateReloc;

/* Tier B: callee identities decoded from overlay 11's relocation records.
 * The *_o011Reloc names are the generated relocation surface; the reset is
 * a same-module callee whose call site is a SYMBOL record. */
extern s32 func_800290A0_o011Reloc(void);
extern s32 mathRnd_o011Reloc(s32 minimum, s32 maximum);
extern void viGetCurrentSize_o011Reloc(u32 *width, u32 *height);
extern void camStandardOrtho_o011Reloc(O11Gfx **displayList, void **matrix);
extern void func_800349A4_o011Reloc(O11Gfx **displayList, void *texture,
                                    s32 flags, s16 parameter);
extern f32 func_8002A8C0_o011Reloc(s32 angle);
extern u32 joyGetPressed_o011Reloc(s32 controller);
extern void overlay11StopOverlay66Reloc(void *arg0);
extern void func_800290AC_o011Reloc(s32 mode);
extern void func_800291D8_o011Reloc(s32 value);
extern void amTuneSetFadeScaled_o011Reloc(f32 value, s32 volume);
extern void overlay11ResetMenuReloc(void);
extern O11Status *func_80028F54_o011Reloc(void);

extern void func_overlay_011_F00011D0_1869A18(s32 updateRate);
extern void func_overlay_011_F0001398_1869BE0(s32 updateRate);
extern void func_overlay_011_F000184C_186A094(s32 updateRate);
extern void func_overlay_011_F0001A7C_186A2C4(s32 updateRate);
extern void func_overlay_011_F0001E4C_186A694(s32 updateRate);
extern void func_overlay_011_F00022E8_186AB30(s32 updateRate);
extern void func_overlay_011_F0002714_186AF5C(s32 updateRate);

/* A bare statement list, not a `do { } while (0)` macro: the region that
 * wrapper opens is what stopped IDO unrolling the vertex-row loop. */
#define O11_WRITE_VERTEX(vertexX, vertexY, vertexAlpha) \
        gO11VertexReloc->x = (vertexX); \
        gO11VertexReloc->y = (vertexY); \
        gO11VertexReloc->z = 0; \
        gO11VertexReloc->r = 0; \
        gO11VertexReloc->g = 0; \
        gO11VertexReloc->b = 0; \
        gO11VertexReloc->a = (vertexAlpha); \
        gO11VertexReloc++

/* Matched 2026-10-02 from 534 masked words. What it took, in the order it
 * was found:
 * - the vertex writer without its `do { } while (0)` wrapper (above), and
 *   the seven rows of a column as ONE loop from the top row: IDO unrolls it
 *   by four after a three-row remainder, which is the shipped shape (the
 *   main pass indexes from a constant 3);
 * - every packet as `pointer = list++` then two stores; the two header
 *   packets and the column packets each have their own pointer local (one
 *   shared local is one web and loses v0 to the row y);
 * - x, the top y and the row y are plain s32 locals, and the row y is
 *   reset at the TOP of the column body, before the packets: that
 *   assignment is the dead copy the shipped column loop keeps;
 * - the row is indexed `alpha[row + block * 6]` (row first), the address
 *   byte is masked with 0xFF before the shift, and the switch lists mode 2
 *   last;
 * - ten scalar locals in this order put alpha, width, height and the two
 *   spill cells at the shipped homes (`unused` is the tenth);
 * - L59: the second header packet's two stores share a line, and so do the
 *   column packet's pointer step and first word.
 * The two 0.05f literals are spelled differently because the shipped pool
 * holds two entries (L103). */
void func_overlay_011_F0000150_1868998(O11Gfx **displayList, void **matrix,
                                        O11Vertex **vertices,
                                        s32 updateRate) {
    s32 row;
    s32 column;
    s32 block;
    O11Gfx *command;
    s32 x;
    u8 alpha[13][17];
    s32 top;
    u32 width;
    u32 height;
    s32 y;
    O11Gfx *scissor;
    O11Gfx *combine;
    s32 unused;

    if (func_800290A0_o011Reloc() != 0) {
        if (gOverlay11Fade == 0.0f) {
            for (row = 0; row < 13; row++) {
                for (column = 0; column < 17; column++) {
                    gOverlay11GridPhases[row][column] =
                        mathRnd_o011Reloc(-0x8000, 0x7FFF);
                }
            }
        }
        gOverlay11Fade += 0.05f * (f32)updateRate;
        if (gOverlay11Fade > 1.0f) {
            gOverlay11Fade = 1.0f;
        }
    } else {
        gOverlay11Fade -= .05f * (f32)updateRate;
        if (gOverlay11Fade <= 0.0f) {
            gOverlay11Fade = 0.0f;
            return;
        }
    }

    gO11DisplayListReloc = *displayList;
    gO11MatrixReloc = *matrix;
    gO11VertexReloc = *vertices;

    viGetCurrentSize_o011Reloc(&width, &height);
    scissor = gO11DisplayListReloc++;
    scissor->w0 = 0xED000000;
    scissor->w1 =
        ((((s32)((width - 1) * 4.0f)) & 0xFFF) << 12) |
        (((s32)((height - 1) * 4.0f)) & 0xFFF);
    camStandardOrtho_o011Reloc(&gO11DisplayListReloc, &gO11MatrixReloc);
    func_800349A4_o011Reloc(&gO11DisplayListReloc, 0, 4, 0);
    combine = gO11DisplayListReloc++;
    combine->w0 = 0xFCFFFFFF; combine->w1 = 0xFFFE793C;

    for (row = 0; row < 13; row++) {
        for (column = 0; column < 17; column++) {
            gOverlay11GridPhases[row][column] += updateRate << 8;
            alpha[row][column] =
                (u8)((func_8002A8C0_o011Reloc(gOverlay11GridPhases[row][column]) * 32.0f +
                      96.0f) *
                     gOverlay11Fade);
        }
    }

    for (block = 0; block < 2; block++) {
        x = -160;
        top = 120 - block * 120;
        for (column = 0; column < 17; column++) {
            y = top;
            command = gO11DisplayListReloc++; command->w0 = 0x04000000 | ((((((u32)gO11VertexReloc + 0x80000000) & 6) | 0x38) & 0xFF) << 16) | ((((column % 2 * 7) << 9) | 0x4E) & 0xFFFF);
            command->w1 = (u32)gO11VertexReloc + 0x80000000;
            if (column != 0) {
                command = gO11DisplayListReloc++;
                command->w0 = 0x05B100C0;
                command->w1 =
                    (u32)(gOverlay11GridTriangles + (column % 2 * 0xC0)) + 0x80000000;
            }
            for (row = 0; row < 7; row++) {
                O11_WRITE_VERTEX(x, y, alpha[row + block * 6][column]);
                y -= 20;
            }
            x += 20;
        }
    }

    if ((gOverlay11InputDelay == 0) && (gOverlay11OptionDone == 0)) {
        if ((gOverlay11Mode == 0) && (gOverlay11OptionAction == 0) &&
            (joyGetPressed_o011Reloc(gOverlay11PadIndex) & 0x5000)) {
            overlay11StopOverlay66Reloc(0);
            func_800290AC_o011Reloc(0);
            func_800291D8_o011Reloc(0x1E);
            amTuneSetFadeScaled_o011Reloc(0.5f, 0x7F);
            overlay11ResetMenuReloc();
            gOverlay11OptionDone = 1;
        } else {
            gOverlay11OptionHighlight += gOverlay11HighlightStep * updateRate;
            if (gOverlay11OptionHighlight >= 0x100) {
                gOverlay11OptionHighlight = 0x1FF - gOverlay11OptionHighlight;
                gOverlay11HighlightStep = -gOverlay11HighlightStep;
            } else if (gOverlay11OptionHighlight < 0) {
                gOverlay11OptionHighlight = -gOverlay11OptionHighlight;
                gOverlay11HighlightStep = -gOverlay11HighlightStep;
            }
            if (gOverlay11Mode != 0) {
                func_overlay_011_F00011D0_1869A18(updateRate);
            } else {
                switch (func_80028F54_o011Reloc()->mode) {
                case 0:
                    func_overlay_011_F0001398_1869BE0(updateRate);
                    break;
                case 1:
                    func_overlay_011_F0001A7C_186A2C4(updateRate);
                    break;
                case 3:
                    func_overlay_011_F000184C_186A094(updateRate);
                    break;
                case 4:
                case 5:
                    if (gOverlay11GameStateReloc == 1) {
                        func_overlay_011_F00022E8_186AB30(updateRate);
                    } else {
                        func_overlay_011_F0001E4C_186A694(updateRate);
                    }
                    break;
                case 2:
                    func_overlay_011_F0002714_186AF5C(updateRate);
                    break;
                }
            }
        }
    }

    if (gOverlay11InputDelay != 0) {
        gOverlay11InputDelay -= updateRate;
        if (gOverlay11InputDelay < 0) {
            gOverlay11InputDelay = 0;
        }
    }

    *displayList = gO11DisplayListReloc;
    *matrix = gO11MatrixReloc;
    *vertices = gO11VertexReloc;
}

#undef O11_WRITE_VERTEX
