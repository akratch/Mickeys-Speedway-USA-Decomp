#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101ClipNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
    u8 pad12[4];
    u8 intensity;
    u8 alpha;
} Overlay101ClipNode;

typedef struct Overlay101Texture {
    s16 width;
    s16 height;
    u8 pad04[0xC];
    u16 pixels[1];
} Overlay101Texture;

typedef struct Overlay101TextureElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    u8 pad0C[4];
    Overlay101Texture *texture;
} Overlay101TextureElement;

extern u32 D_230[];

/* Tier B: this function's four runtime R_MIPS_26 records are all SYMBOL
 * operations. Two name overlay 101 +0x1F80 (overlay101SetScissor), one names
 * overlay 101 +0x2118 (overlay101GetBounds) and one names the resident
 * func_80034920 at +0x344D0 past the resident base. The extracted assembly
 * shows all four as a jump to overlay offset 0 because a SYMBOL record ships
 * the 0xF0000000 addend rather than offset >> 2, so they must be routed
 * through the generated surface. The two overlay callees are ROM-exact, so
 * their prototypes are the matched ones. */
void overlay101GetBoundsReloc(Overlay101ClipNode *node, s32 *leftOut,
                              s32 *topOut, s32 *rightOut, s32 *bottomOut);
void overlay101SetScissorReloc(Overlay101Gfx **displayList, s32 left, s32 top,
                               s32 right, s32 bottom);
void func_80034920(Overlay101Gfx **displayList);

/*
 * NON_MATCHING reconstruction, rebuilt 2026-09-12 by lane p11-o101.
 *
 * The retained body named one symbol for all four calls and gave it four
 * different argument lists, because the extracted assembly shows all four as
 * a jump to overlay offset 0. That reading is wrong: every one of the four is
 * a SYMBOL relocation record, so the shipped word carries the 0xF0000000
 * addend rather than offset >> 2 and the real callee lives in the module's
 * runtime table. Decoded there, they are overlay101GetBounds (overlay 101
 * +0x2118), overlay101SetScissor twice (overlay 101 +0x1F80) and the resident
 * func_80034920 (+0x344D0 past the resident base, which other overlays already
 * call with a display-list pointer). Both overlay callees are ROM-exact, so
 * their prototypes are the matched ones, and the third call takes ONE argument
 * where the retained body passed four -- the two extra stack homes it needed
 * for that are gone.
 *
 * The command idiom is this overlay's own, taken from ROM-exact
 * overlay101SetScissor: a bare block per display-list word pair, with a fresh
 * pointer snapshot of the cursor. The target's `move aN,v0` before every
 * `addiu v0,v0,8` is that shape; writing the stores through the cursor itself
 * folds the increments and emits negative displacements the target never has.
 *
 * Against the retained body this moves the structural residual from 142 words
 * to 125 and the size delta from -52 to +8. The remaining +8 is exactly one
 * extra callee-saved register's save and restore: this candidate holds ten
 * where the target holds nine, so uopt has a spare register and hoists five
 * loop-invariant opcode constants the target rematerialises each iteration.
 * The positional masked count is 291 against the retained body's 276; that
 * number is worse on a shape that is measurably closer, and the handoff says
 * which axes have been measured.
 */
#ifdef NON_MATCHING
void func_overlay_101_F0002510_18DDD30(Overlay101Gfx **displayList,
                                       Overlay101ClipNode *node,
                                       Overlay101TextureElement *element) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 x;
    s32 y;
    s32 edgeX;
    s32 edgeY;
    s32 shift;
    s32 rows;
    s32 stride;
    s32 sourceX;
    s32 sourceY;
    s32 drawX;
    s32 drawY;
    s32 drawWidth;
    s32 drawHeight;
    s32 nextY;
    s32 rowOffset;
    s32 mask;
    s32 loadCount;
    s32 loadLimit;
    s32 chunkRows;
    u8 *source;
    Overlay101Texture *texture;
    Overlay101Gfx *gfx;

    if ((node->type == 2) || (node->type == 4)) {
        texture = element->texture;
        if (texture != 0) {
            overlay101GetBoundsReloc(node, &left, &top, &right, &bottom);
            y = node->y + element->y;
            x = node->x + element->x;
            edgeX = x + texture->width;
            edgeY = y + texture->height;
            if ((right >= x) && (bottom >= y) && (edgeX >= left) &&
                (edgeY >= top)) {
                overlay101SetScissorReloc(displayList, left, top, right,
                                          bottom);

                loadLimit = 0x800 / texture->width;
                if (loadLimit >= 8) {
                    shift = 3;
                } else if (loadLimit >= 4) {
                    shift = 2;
                } else {
                    shift = 1;
                }
                rows = 1 << shift;
                stride = texture->width * rows;
                gfx = *displayList;
                sourceX = 0;
                mask = rows - 1;
                drawX = x;
                if (x < left) {
                    drawX = left;
                    sourceX = left - x;
                }
                drawWidth = texture->width - sourceX;
                if ((right - drawX) < drawWidth) {
                    drawWidth = right - drawX;
                }
                if (y < top) {
                    drawY = top;
                    sourceY = top - y;
                } else {
                    drawY = y;
                    sourceY = 0;
                }
                drawHeight = texture->height - sourceY;
                if ((bottom - drawY) < drawHeight) {
                    drawHeight = bottom - drawY;
                }

                source = (u8 *)texture + (stride * (sourceY >> shift) * 2) +
                         0x10;
                rowOffset = (sourceY & mask) << 5;
                drawY *= 4;
                drawX *= 4;
                sourceX <<= 5;
                {
                    Overlay101Gfx *command;
                    command = gfx++;
                    command->w0 = 0x06000000;
                    command->w1 = (u32)D_230;
                }
                {
                    Overlay101Gfx *command;
                    command = gfx++;
                    command->w0 = 0xFA000000;
                    command->w1 = (node->intensity << 24) |
                                  (node->intensity << 16) |
                                  (node->intensity << 8) | node->alpha;
                }

                if (drawHeight > 0) {
                    stride *= 2;
                    do {
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xFD100000;
                            command->w1 = (u32)source;
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xF5100000;
                            command->w1 = 0x07080200;
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xE6000000;
                            command->w1 = 0;
                        }
                        loadCount = (texture->width * rows) - 1;
                        if (loadCount >= 0x7FF) {
                            loadCount = 0x7FF;
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xF3000000;
                            command->w1 = ((loadCount & 0xFFF) << 12) |
                                          0x07000000;
                        }
                        source += stride;
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xE7000000;
                            command->w1 = 0;
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 =
                                (((((texture->width * 2) + 7) >> 3) & 0x1FF)
                                 << 9) |
                                0xF5100000;
                            command->w1 = 0x00080200;
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xF2000000;
                            command->w1 =
                                ((((texture->width - 1) * 4) & 0xFFF) << 12) |
                                ((mask * 4) & 0xFFF);
                        }
                        chunkRows = rows - (rowOffset >> 5);
                        if (drawHeight < chunkRows) {
                            chunkRows = drawHeight;
                        }
                        nextY = drawY + chunkRows * 4;
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xE4000000 |
                                          (((drawX + drawWidth * 4) & 0xFFF)
                                           << 12) |
                                          (nextY & 0xFFF);
                            command->w1 = ((drawX & 0xFFF) << 12) |
                                          (drawY & 0xFFF);
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xB3000000;
                            command->w1 = (sourceX << 16) |
                                          (rowOffset & 0xFFFF);
                        }
                        {
                            Overlay101Gfx *command;
                            command = gfx++;
                            command->w0 = 0xB2000000;
                            command->w1 = 0x04000400;
                        }
                        drawHeight -= chunkRows;
                        rowOffset = 0;
                        drawY = nextY;
                    } while (drawHeight > 0);
                }
                *displayList = gfx;
                func_80034920(displayList);
                overlay101SetScissorReloc(displayList, 0, 0, 1000, 1000);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F0002510_18DDD30/func_overlay_101_F0002510_18DDD30.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_101_F0002510_18DDD30:start
 * symbol: func_overlay_101_F0002510_18DDD30
 * score: 291 differing words
 * frame: 0xF8
 * relocations: 6
 * first-mismatch: +0x0
 * summary: 291 words, size +8, frame 0xF8 vs 0xE8. Bounds-array and width-local spellings did not beat it.
 * PLATEAU-HANDOFF:func_overlay_101_F0002510_18DDD30:end
 */
