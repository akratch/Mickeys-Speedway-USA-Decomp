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
extern Overlay45GlyphData *func_8004C690(s32 character);
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
/* Plateau 2026-10-02 (lane q-ovl10): 573 masked at -4 (was 593 at +28).
 * Priced edits: element x/y are floats truncated at use, upper/lower are
 * ints converted at use, one packet macro per command (gDma1p for the
 * vertex load), the -0xA0 screen offset applied after the sine/cosine
 * calls, and the corner pair (x, y) negated in place for the lower row.
 * 573 -> 566 (lane w2-ovle, 2026-10-02): locals declared in the target's
 * frame-ladder order put the frame at 0x190 and savedFont (+0x18C), the
 * loop bound (+0x184), activeColour (+0x150) and textureBottom's spill
 * (+0x154) on their shipped homes; aligned byte-exact 179 -> 193. Remains:
 * allocator -- the target spills left/right/textureBottom to their homes
 * across the angle calls and keeps textureLeft/Top/Right in s5/s6/fp, and
 * keeps current in memory at +0x108. */
#define PKT(pkt, a, b) { Gfx *_g = (Gfx *)(pkt)++; _g->words.w0 = (a); _g->words.w1 = (b); }
#ifdef NON_MATCHING
void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, Overlay45Vertex **vertexPtr, void *unused,
    Overlay45ResourceDescriptor *descriptor) {
    s32 savedFont;
    s32 pad188;
    s32 loopY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 glyphWidth;
    s32 glyphHeight;
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
    s32 pad12C;
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
    u8 second;
    u32 vertexAddress;

    if (descriptor == NULL) {
        return;
    }

    dList = *displayList;
    window = descriptor->unk28;
    vertices = *vertexPtr;
    triangles = descriptor->allocation;
    func_8004D39C(descriptor->elementEnd, D_800D6648);
    savedFont = D_800D60E0;
    D_800D60E0 = descriptor->unk21;
    font = &D_800D60E4[D_800D60E0];
    current = D_800D6648;

    gSPDisplayList(dList++, D_8007D490);
    if (window != D_800D64E8) {
        s32 x1;
        s32 y1;
        s32 x2;
        s32 y2;

        x1 = window->x1;
        y1 = window->y1;
        x2 = window->x2;
        y2 = window->y2;
        if ((D_800D64E8[0].x2 < x1) || (D_800D64E8[0].y2 < y1) ||
            (x2 < 0) || (y2 < 0)) {
            return;
        }
        if (x1 < 0) {
            x1 = 0;
        }
        if (y1 < 0) {
            y1 = 0;
        }
        if (D_800D64E8[0].x2 < x2) {
            x2 = D_800D64E8[0].x2;
        }
        if (D_800D64E8[0].y2 < y2) {
            y2 = D_800D64E8[0].y2;
        }
        gDPSetScissor(dList++, G_SC_NON_INTERLACE, x1, y1, x2, y2);
    }

    gDPSetPrimColor(dList++, 0, 0, 0xFF, 0xFF, 0xFF, descriptor->mode);
    gDPSetEnvColor(dList++, window->textColourR, window->textColourG,
                   window->textColourB, descriptor->unk22);

    activeColour = 0;
    first = *current;
    element = descriptor->elements;
    while ((first != 0) && (window->y2 >= loopY)) {
        current++;
        if (first & 0x80) {
            second = *current++;
            if ((second != 0) && (second != 0xF)) {
                if (D_800D664D != 0) {
                    if (second == 2) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0, 0xFF, 0xFF);
                        activeColour = 1;
                    } else if (second == 0xE) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0xFF, 0, 0xFF);
                        activeColour = 1;
                    } else if ((second >= 0x41) && (second < 0x45)) {
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

                glyph = func_8004C690(second);
                if (glyph != NULL) {
                    left = (s32)element->x;
                    top = (s32)element->y;
                    right = (glyph->right + left) - glyph->left;
                    bottom = (glyph->bottom + top) - glyph->top;
                    glyphWidth = right - left;
                    glyphHeight = bottom - top;
                    textureLeft = glyph->left << 5;
                    textureTop = glyph->top << 5;
                    textureRight = ((glyphWidth << 5) + textureLeft) - 0x20;
                    textureBottom = ((glyphHeight << 5) + textureTop) - 0x20;

                    fontCommands = font->displayList;
                    dList->words.w0 = fontCommands->words.w0;
                    dList->words.w1 = D_800D6638 + glyph->textureOffset;
                    dList++;
                    fontCommands++;
                    if (font->format == 4) {
                        PKT(dList, 0x07060030, (u32)fontCommands + 0x80000000);
                        dList->words.w0 = fontCommands[6].words.w0;
                        dList->words.w1 = D_800D6638 + glyph->textureOffset2;
                        dList++;
                        fontCommands += 7;
                    }
                    PKT(dList, 0x07080040, (u32)fontCommands + 0x80000000);
                    PKT(dList, 0xEF082C0F, 0x00504240);
                    gDma1p(dList++, G_VTX, (u32)vertices + 0x80000000, 0x30, (4 << 3) | (((u32)vertices + 0x80000000) & 6));
                    PKT(dList, 0x05110020, (u32)triangles + 0x80000000);

                    if (descriptor->unk10 != 0.0f) {
                        f32 expandX;
                        f32 expandY;

                        expandX = descriptor->unk10 * (f32)glyphWidth * 0.5f;
                        expandY = descriptor->unk10 * (f32)glyphHeight * 0.5f;
                        left = (s32)((f32)left - expandX);
                        right = (s32)((f32)right + expandX);
                        top = (s32)((f32)top - expandY);
                        bottom = (s32)((f32)bottom + expandY);
                    }

                    upper = (0x78 - top) + element->unk12 + element->unk1E;
                    lower = (0x78 - bottom) + element->unk12 + element->unk1E;
                    sine = func_8002A8BC(
                        element->unk0A + ((s16)element->unk20 << 8));
                    cosine = func_8002A8C0(
                        element->unk0A + ((s16)element->unk20 << 8));
                    left -= 0xA0;
                    right -= 0xA0;
                    halfWidth = (f32)(right - left) * 0.5f;
                    halfHeight = (f32)(upper - lower) * 0.5f;
                    x = -halfWidth;
                    y = halfHeight;
                    vertices[0].x = (f32)left + ((x * sine - y * cosine) - x);
                    vertices[0].y = (f32)upper + ((y * sine + x * cosine) - y);
                    vertices[0].z = 0;
                    vertices[0].red = 0xFF;
                    vertices[0].green = 0xFF;
                    vertices[0].blue = 0xFF;
                    vertices[0].alpha = 0xFF;
                    x = halfWidth;
                    vertices[1].x = (f32)right + ((x * sine - y * cosine) - x);
                    vertices[1].y = (f32)upper + ((y * sine + x * cosine) - y);
                    vertices[1].z = 0;
                    vertices[1].red = 0xFF;
                    vertices[1].green = 0xFF;
                    vertices[1].blue = 0xFF;
                    vertices[1].alpha = 0xFF;
                    x = -x;
                    y = -y;
                    vertices[2].x = (f32)left + ((x * sine - y * cosine) - x);
                    vertices[2].y = (f32)lower + ((y * sine + x * cosine) - y);
                    vertices[2].z = 0;
                    vertices[2].red = 0xFF;
                    vertices[2].green = 0xFF;
                    vertices[2].blue = 0xFF;
                    vertices[2].alpha = 0xFF;
                    x = -x;
                    vertices[3].x = (f32)right + ((x * sine - y * cosine) - x);
                    vertices[3].y = (f32)lower + ((y * sine + x * cosine) - y);
                    vertices[3].z = 0;
                    vertices[3].red = 0xFF;
                    vertices[3].green = 0xFF;
                    vertices[3].blue = 0xFF;
                    vertices[3].alpha = 0xFF;

                    triangles[0].flags = 0x40;
                    triangles[0].vertex0 = 0;
                    triangles[0].vertex1 = 1;
                    triangles[0].vertex2 = 2;
                    triangles[0].s0 = textureLeft;
                    triangles[0].t0 = textureTop;
                    triangles[0].s1 = textureRight;
                    triangles[0].t1 = textureTop;
                    triangles[0].s2 = textureLeft;
                    triangles[0].t2 = textureBottom;

                    triangles[1].flags = 0x40;
                    triangles[1].vertex0 = 1;
                    triangles[1].vertex1 = 3;
                    triangles[1].vertex2 = 2;
                    triangles[1].s0 = textureRight;
                    triangles[1].t0 = textureTop;
                    triangles[1].s1 = textureRight;
                    triangles[1].t1 = textureBottom;
                    triangles[1].s2 = textureLeft;
                    triangles[1].t2 = textureBottom;

                    vertices += 4;
                    triangles += 2;
                }
            }
        }
        element++;
        first = *current;
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
 * score: 566/674 words
 * frame: 0x190
 * relocations: 25
 * first-mismatch: +0x4
 * summary: 566 at -4: locals in frame-ladder order, frame 0x190. Open: left/right take s-regs; target saves them around the angle calls.
 * PLATEAU-HANDOFF:func_overlay_045_F0001158_188D5B0:end
 */
