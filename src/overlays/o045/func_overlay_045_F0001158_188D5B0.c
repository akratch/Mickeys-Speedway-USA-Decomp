#include "overlays/overlay_045.h"
#include "game/font.h"
#include "n_audio/mbi.h"

typedef struct Overlay45FontData {
    u8 width;
    u8 verticalExtent;
    u8 characterWidth;
    u8 height;
    u32 romOffset;
    s16 textureSize;
    u16 format;
    u32 unused0C;
    Gfx *displayList;
} Overlay45FontData;

typedef struct Overlay45GlyphData {
    u8 font;
    u8 character;
    u16 allocationOffset;
    u8 state;
    u8 chainLength;
    u16 textureOffset;
    u16 textureOffset2;
    u8 left;
    u8 top;
    u8 right;
    u8 bottom;
    u8 advance;
} Overlay45GlyphData;

typedef struct Overlay45Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay45Vertex;

typedef struct Overlay45Triangle {
    u8 flags;
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    s16 s0;
    s16 t0;
    s16 s1;
    s16 t1;
    s16 s2;
    s16 t2;
} Overlay45Triangle;

extern DialogueBoxBackground D_800D64E8[];
extern Gfx D_8007D490[];
extern u8 D_800D60E0;
extern Overlay45FontData *D_800D60E4;
extern u32 D_800D6638;
extern u8 *D_800D6648;
extern u8 D_800D664D;

extern void func_80034920(Gfx **displayList);
extern void func_8004D39C(char *input, char *output);
extern Overlay45GlyphData *func_8004C690(u8 character);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);

/*
 * PROVENANCE -- the control-stream and font-command organization was
 * cross-checked through Mickey's resident func_8004B1DC, whose corresponding
 * source shape was adapted from Diddy Kong Racing, src/font.c
 * (render_text_string). Overlay 45's rotation, private vertex/triangle
 * records, descriptor fields, and final control flow come from Mickey's own
 * instructions and callers.
 */
/* 2026-10-07 (lane b-o045), 395 -> 153 at size delta 0, frame 0x190, by
 * porting the matched sibling func_8004B1DC's shape: the scissor clamp
 * reuses left/top/right/bottom (the target colours them t4/t0/ra/t5 in both
 * regions); the loop tests *current and reads first = *current++, and the
 * escape byte is read back into first; the screen offsets are unsigned
 * subtractions (0xA0U), which stops uopt folding (r - 0xA0) - (l - 0xA0)
 * into a pre-call r - l; the expand amounts reuse x and y instead of block
 * locals (frame 0x198 -> 0x190); the corner offsets go through
 * negativeHalfWidth/negativeHalfHeight, which gives the target's shared
 * products and its float spill. 153 -> 135: each vertex writes its colour
 * bytes before z, and the x sums convert the screen offset inline
 * ((f32)screenLeft + ...), which puts the conversion on the target's left
 * operand; leftFloat/rightFloat stay declared as frame slots. The
 * triangles are written through the walking pointer (triangles->field,
 * triangles++ after each), which retires the inherited all-ones mask the
 * indexed form needed to keep the allocation pointer in s1. 135 -> 119:
 * func_8004C690 takes a u8, as its definition in src/main/font.c does;
 * the escape byte is then loaded into a0 and copied into first's v0 as
 * shipped, and the colour packets take v1/a0. 119 -> 117: each triangle
 * writes vertex2 before vertex1. 117 -> 116 (lane e-ovl1): no dead
 * `leftFloat = screenLeft;` store; the left conversion is then numbered at
 * its first use and takes f26 as shipped. The dead `rightFloat` store stays:
 * it numbers the right conversion ahead of the half-height webs (removing it
 * is 117). Earlier passes: see the shard. */
#define PKT(pkt, a, b) { Gfx *_g = (Gfx *)(pkt)++; _g->words.w0 = (a); _g->words.w1 = (b); }
#ifdef NON_MATCHING
void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, Overlay45Vertex **vertexPtr, void *unused,
    Overlay45ResourceDescriptor *descriptor) {
    s32 savedFont;
    s32 screenLeft;
    s32 loopY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 padGlyphW;
    s32 padGlyphH;
    s32 textureLeft;
    s32 textureTop;
    s32 textureRight;
    s32 upper;
    s32 lower;
    s32 textureBottom;
    s32 activeColour;
    f32 sine;
    f32 x;
    f32 y;
    f32 cosine;
    f32 halfWidth;
    f32 halfHeight;
    f32 negativeHalfWidth;
    f32 negativeHalfHeight;
    f32 leftFloat;
    f32 rightFloat;
    f32 upperFloat;
    f32 lowerFloat;
    s32 screenRight;
    s32 padA;
    s32 padB;
    s32 padC;
    s32 padD;
    u8 *current;
    Overlay45FontData *font;
    Overlay45GlyphData *glyph;
    Gfx *dList;
    Gfx *fontCommands;
    Overlay45Vertex *vertices;
    Overlay45Triangle *triangles;
    DialogueBoxBackground *window;
    Overlay45Element *element;
    u8 first;

    if (descriptor == NULL) {
        return;
    }

    window = descriptor->unk28;
    dList = *displayList;
    vertices = *vertexPtr;
    triangles = descriptor->allocation;
    func_8004D39C(descriptor->elementEnd, D_800D6648);
    savedFont = D_800D60E0;
    D_800D60E0 = descriptor->unk21;
    font = &D_800D60E4[D_800D60E0];
    current = D_800D6648;

    gSPDisplayList(dList++, D_8007D490);
    if (window != D_800D64E8) {
        left = window->x1;
        top = window->y1;
        right = window->x2;
        bottom = window->y2;
        if ((D_800D64E8[0].x2 < left) || (D_800D64E8[0].y2 < top) ||
            (right < 0) || (bottom < 0)) {
            return;
        }
        if (left < 0) {
            left = 0;
        }
        if (top < 0) {
            top = 0;
        }
        if (D_800D64E8[0].x2 < right) {
            right = D_800D64E8[0].x2;
        }
        if (D_800D64E8[0].y2 < bottom) {
            bottom = D_800D64E8[0].y2;
        }
        gDPSetScissor(dList++, G_SC_NON_INTERLACE, left, top, right, bottom);
    }
    gDPSetPrimColor(dList++, 0, 0, 0xFF, 0xFF, 0xFF, descriptor->mode);
    gDPSetEnvColor(dList++, window->textColourR, window->textColourG,
                   window->textColourB, descriptor->unk22);

    activeColour = 0;
    element = descriptor->elements;
    while ((*current != 0) && (window->y2 >= loopY)) {
        first = *current++;
        if (first & 0x80) {
            first = *current++;
            if ((first != 0) && (first != 0xF)) {
                if (D_800D664D != 0) {
                    if (first == 2) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0, 0xFF, 0xFF);
                        activeColour = 1;
                    } else if (first == 0xE) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0xFF, 0, 0xFF);
                        activeColour = 1;
                    } else if ((first >= 0x41) && (first < 0x45)) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0xFF, 0xFF, 0, 0xFF);
                        activeColour = 1;
                    } else if (activeColour != 0) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, window->textColourR,
                                       window->textColourG,
                                       window->textColourB,
                                       window->textColourA);
                        activeColour = 0;
                    }
                }

                glyph = func_8004C690(first);
                if (glyph != NULL) {
                    left = (s32)element->x;
                    top = (s32)element->y;
                    right = (glyph->right + left) - glyph->left;
                    bottom = (glyph->bottom + top) - glyph->top;
                    textureLeft = glyph->left << 5;
                    textureTop = glyph->top << 5;
                    textureRight = (((right - left) << 5) + textureLeft) - 0x20;
                    textureBottom = (((bottom - top) << 5) + textureTop) - 0x20;

                    fontCommands = font->displayList;
                    dList->words.w0 = fontCommands->words.w0;
                    dList->words.w1 = (u32)((u8 *)D_800D6638 + glyph->textureOffset);
                    dList++;
                    fontCommands++;
                    if (font->format == 4) {
                        PKT(dList, 0x07060030, (u32)fontCommands + 0x80000000);
                        dList->words.w0 = fontCommands[6].words.w0;
                        dList->words.w1 = (u32)((u8 *)D_800D6638 + glyph->textureOffset2);
                        dList++;
                        fontCommands += 7;
                    }
                    PKT(dList, 0x07080040, (u32)fontCommands + 0x80000000);
                    PKT(dList, 0xEF082C0F, 0x00504240);
                    gDma1p(dList++, G_VTX, (u32)vertices + 0x80000000, 0x30, (4 << 3) | (((u32)vertices + 0x80000000) & 6));
                    PKT(dList, 0x05110020, (u32)triangles + 0x80000000);

                    if (descriptor->unk10 != 0.0f) {
                        x = descriptor->unk10 * (f32)(right - left) * 0.5f;
                        y = descriptor->unk10 * (f32)(bottom - top) * 0.5f;
                        left = (s32)((f32)left - x);
                        right = (s32)((f32)right + x);
                        top = (s32)((f32)top - y);
                        bottom = (s32)((f32)bottom + y);
                    }

                    upper = (0x78 - top) + element->unk12 + element->unk1E;
                    lower = (0x78 - bottom) + element->unk12 + element->unk1E;
                    sine = func_8002A8BC(
                        element->unk0A + ((s16)element->unk20 << 8));
                    cosine = func_8002A8C0(
                        element->unk0A + ((s16)element->unk20 << 8));
                    screenLeft = left - 0xA0U;
                    screenRight = right - 0xA0U;
                    halfWidth = (f32)(s32)(screenRight - screenLeft) * 0.5f;
                    rightFloat = screenRight;
                    halfHeight = (f32)(upper - lower) * 0.5f;
                    negativeHalfWidth = -halfWidth;
                    negativeHalfHeight = -halfHeight;
                    x = negativeHalfWidth;
                    y = halfHeight;
                    vertices[0].x = (f32)screenLeft + ((x * sine - y * cosine) - x);
                    vertices[0].y = (f32)upper + ((y * sine + x * cosine) - y);
                    vertices[0].red = 0xFF;
                    vertices[0].green = 0xFF;
                    vertices[0].blue = 0xFF;
                    vertices[0].alpha = 0xFF;
                    vertices[0].z = 0;
                    x = halfWidth;
                    vertices[1].x = (f32)screenRight + ((x * sine - y * cosine) - x);
                    vertices[1].y = (f32)upper + ((y * sine + x * cosine) - y);
                    vertices[1].red = 0xFF;
                    vertices[1].green = 0xFF;
                    vertices[1].blue = 0xFF;
                    vertices[1].alpha = 0xFF;
                    vertices[1].z = 0;
                    x = negativeHalfWidth;
                    y = negativeHalfHeight;
                    vertices[2].x = (f32)screenLeft + ((x * sine - y * cosine) - x);
                    vertices[2].y = (f32)lower + ((y * sine + x * cosine) - y);
                    vertices[2].red = 0xFF;
                    vertices[2].green = 0xFF;
                    vertices[2].blue = 0xFF;
                    vertices[2].alpha = 0xFF;
                    vertices[2].z = 0;
                    x = -negativeHalfWidth;
                    vertices[3].x = (f32)screenRight + ((x * sine - y * cosine) - x);
                    vertices[3].y = (f32)lower + ((y * sine + x * cosine) - y);
                    vertices[3].red = 0xFF;
                    vertices[3].green = 0xFF;
                    vertices[3].blue = 0xFF;
                    vertices[3].alpha = 0xFF;
                    vertices[3].z = 0;

                    triangles->flags = 0x40;
                    triangles->vertex0 = 0;
                    triangles->vertex2 = 2;
                    triangles->vertex1 = 1;
                    triangles->s0 = textureLeft;
                    triangles->t0 = textureTop;
                    triangles->s1 = textureRight;
                    triangles->t1 = textureTop;
                    triangles->s2 = textureLeft;
                    triangles->t2 = textureBottom;
                    triangles++;

                    triangles->flags = 0x40;
                    triangles->vertex0 = 1;
                    triangles->vertex2 = 2;
                    triangles->vertex1 = 3;
                    triangles->s0 = textureRight;
                    triangles->t0 = textureTop;
                    triangles->s1 = textureRight;
                    triangles->t1 = textureBottom;
                    triangles->s2 = textureLeft;
                    triangles->t2 = textureBottom;
                    triangles++;

                    vertices += 4;
                }
            }
        }
        element++;
    }

    D_800D60E0 = savedFont;
    func_80034920(&dList);
    if (window != D_800D64E8) {
        gDPSetScissor(dList++, G_SC_NON_INTERLACE,
                      D_800D64E8[0].x1, D_800D64E8[0].y1,
                      D_800D64E8[0].x2, D_800D64E8[0].y2);
    }
    *displayList = dList;
    *vertexPtr = vertices;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o045/func_overlay_045_F0001158_188D5B0/func_overlay_045_F0001158_188D5B0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_045_F0001158_188D5B0:start
 * symbol: func_overlay_045_F0001158_188D5B0
 * score: 116/674 words
 * frame: 0x190
 * relocations: 25
 * first-mismatch: +0x748
 * summary: 116 at size delta 0; the target's float decision order (CDX_BIAS) leaves 1 naming row; the source of that order depends on block cuts not yet reproduced.
 * PLATEAU-HANDOFF:func_overlay_045_F0001158_188D5B0:end
 */
