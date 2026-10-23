#include "overlays/overlay_045.h"

#include "game/font.h"
#include "n_audio/mbi.h"

/* The overlay's initialized owner is a display-list setup followed by the
 * allocator diagnostic and the three motion constants used by this range. */

typedef struct Overlay45InitializedData {
    Gfx setup[10];
    char allocationFailure[0x20];
    char format[4];
    f32 bounceReset;
    f32 bounceStep;
    f32 positionBlend;
} Overlay45InitializedData;

Overlay45InitializedData gOverlay45InitializedData = {
    {
        gsDPPipeSync(),
        gsDPSetTextureLOD(G_TL_TILE),
        gsDPSetTextureLUT(G_TT_NONE),
        gsDPSetAlphaCompare(G_AC_NONE),
        gsSPClearGeometryMode(G_ZBUFFER | G_FOG),
        gsDPPipeSync(),
        gsSPEndDisplayList(),
        gsDPSetCombineLERP(K4, K5, PRIMITIVE, ENVIRONMENT,
                           0, 0, 0, ENVIRONMENT,
                           K4, K5, PRIMITIVE, ENVIRONMENT,
                           0, 0, 0, ENVIRONMENT),
        {{ 0xEF082C0F, 0x00504240 }},
        {{ 0, 0 }},
    },
    "polyFontCreate: mmAlloc failed\n",
    "%s",
    0.05F,
    0.003F,
    0.075F,
};

extern DialogueBoxBackground D_800D64E8[];

extern void func_80034920(Gfx **displayList);

extern void func_8004B0A4(s32 font);

extern void func_800221E8(Gfx **displayList, void *matrix);

extern void func_80022A50(Gfx **displayList, void *matrix);

extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 opacity);

extern s32 func_8002997C(s32 minimum, s32 maximum);

extern f32 func_8002A8C0(s32 angle);

struct Overlay45Vertex;

/* The glyph renderer has no ORT row: static, so its call is a JUMP record. */
static void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, struct Overlay45Vertex **arg1, void *arg2,
    Overlay45ResourceDescriptor *descriptor);

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

extern Gfx D_8007D490[];

extern u8 D_800D60E0;

extern Overlay45FontData *D_800D60E4;

extern u32 D_800D6638;

extern u8 *D_800D6648;

extern u8 D_800D664D;

extern void func_8004D39C(char *input, char *output);

extern Overlay45GlyphData *func_8004C690(u8 character);

extern f32 func_8002A8BC(s32 angle);

/*
 * PROVENANCE -- the control-stream and font-command organization was
 * cross-checked through Mickey's resident func_8004B1DC, whose corresponding
 * source shape was adapted from Diddy Kong Racing, src/font.c
 * (render_text_string). Overlay 45's rotation, private vertex/triangle
 * records, descriptor fields, and final control flow come from Mickey's own
 * instructions and callers.
 */

/* Independently reviewed inert allocation spelling (ADR 0017): the empty
 * comparisons retain operand uses before stock IDO deletes them. Real color
 * and index carriers also preserve local homes; no unused padding is retained.
 * See docs/cleanup-queue.md. Keep the three packed triangle stores together:
 * their shared source line controls the assembler's scheduling tie.
 */

#define PKT(pkt, a, b) { Gfx *_g = (Gfx *)(pkt)++; _g->words.w0 = (a); _g->words.w1 = (b); }


/*
 * Overlay 45, ADR 0006 consolidation: the leading matched-C range is one
 * translation unit in ROM order. The matched setters after the hand-written
 * middle remain in overlay_045_tail.c because a single object cannot span
 * that separately owned asm range.
 */

/* Pinned DKR v77/v80 and JFG scans contain no exact donor. */
void overlay45ResetState(void) {
    gOverlay45ResourceHead = NULL;
}

/*
 * Overlay 45 +0x00C. Fresh DKR v77/v80 searches for the exact
 * strlen/count*0x45+0x35 allocation shape, allocation tag 0x87, and paired
 * 0x140/0xA0 random ranges were negative.
 */
Overlay45ResourceDescriptor *overlay45CreateDescriptor(
    const char *text, s16 width, s16 height, s32 flags) {
    s32 count;
    s32 i;
    Overlay45Element *element;
    Overlay45ResourceDescriptor *descriptor;
    Overlay45ResourceDescriptor *allocation;

    count = overlay45StringLengthReloc(text);
    allocation = overlay45AllocReloc((count << 5) +
                                     (count * sizeof(Overlay45Element)) +
                                     count + 0x35,
                                     0x87);
    if (allocation == NULL) {
        return NULL;
    }
    descriptor = (Overlay45ResourceDescriptor *)((u8 *)allocation + (count << 5));
    descriptor->allocation = allocation;
    descriptor->flags = flags;
    descriptor->unk0C = 0;
    descriptor->width = width;
    descriptor->height = height;
    descriptor->count = count;
    descriptor->elements = (u8 *)descriptor + 0x34;
    descriptor->elementEnd = (u8 *)descriptor->elements +
                             (count * sizeof(Overlay45Element));
    descriptor->unk28 = gOverlay45Defaults;
    descriptor->mode = gOverlay45Defaults[0x1C];
    descriptor->unk10 = (f32)0.0;
    descriptor->unk14 = 0.0f;
    descriptor->unk1E = 0;
    descriptor->unk20 = 0;
    descriptor->unk21 = gOverlay45Defaults[0x1D];
    descriptor->unk22 = 0;
    if (flags & 0x200) {
        descriptor->optionalValue = overlay45RandomRangeReloc(0x78, 0x258);
    }

    element = descriptor->elements;
    i = 0;
    if (count > 0) {
        do {
            element->unk0A = 0;
            element->x = (f32)width;
            element->y = (f32)height;
            element->x = (f32)overlay45RandomRangeReloc(0, 0x140);
            element->y = (f32)overlay45RandomRangeReloc(0, 0xA0);
            element->scale = 1.0f;
            element->unk0C = overlay45RandomRangeReloc(0, 0x8000);
            element->unk0E = overlay45RandomRangeReloc(0x600, 0xA00);
            element->unk10 = overlay45RandomRangeReloc(0x100, 0x300);
            if (overlay45RandomRangeReloc(0, 1) == 0) {
                element->unk10 = -element->unk10;
            }
            i++;
            element->unk13 = 0;
            element->unk1E = 0;
            element->unk1F = 0;
            element->unk20 = 0;
            element->unk22 = 0;
            element++;
        } while (i != count);
    }

    descriptor->next = gOverlay45ResourceHead;
    gOverlay45ResourceHead = descriptor;
    overlay45FormatReloc(descriptor->elementEnd,
                         (char *)gOverlay45Defaults + 0x20, text);
    overlay45ConfigureReloc(descriptor, width, height, flags);
    return descriptor;
}

/*
 * Overlay 45 +0x270. Fresh DKR v77/v80 searches for linked-list removal via
 * offsets +0x30/+0x2C, and for analogous descriptor release loops, were
 * negative.
 */
void overlay45ReleaseDescriptor(Overlay45ResourceDescriptor *descriptor) {
    Overlay45ResourceDescriptor *current;
    Overlay45ResourceDescriptor *previous;
    Overlay45ResourceDescriptor *target;

    current = gOverlay45ResourceHead;
    target = descriptor;
    previous = NULL;
    if (current != NULL && descriptor != NULL) {
        do {
            if (current == target) {
                if (previous == NULL) {
                    gOverlay45ResourceHead = target->next;
                } else {
                    previous->next = target->next;
                }
                overlay45FreeReloc(target->allocation);
                target = NULL;
            } else {
                previous = current;
                current = current->next;
            }
        } while (current != NULL && target != NULL);
    }
}

/*
 * Overlay 45 text +0x0314..+0x0640.
 *
 * The two signed-halfword formals are widened immediately. Keeping x/y as
 * s16 work variables would introduce repeated truncation absent from the
 * target. The matched +0x00C caller already declares this physical ABI.
 */
void overlay45ConfigureLayout(
    Overlay45ResourceDescriptor *descriptor, s32 x, s32 y, s32 flags) {
    Overlay45Element *element;
    Overlay45LayoutDefaults *defaults;
    Overlay45Metrics *metrics;
    Overlay45Glyph *glyph;
    u8 *stream;
    u8 code;
    s32 advance;
    s32 drawX;
    s32 drawY;
    s32 tabWidth;
    s32 randomize;
    s32 unused;

    overlay45BuildControlStream(descriptor->elementEnd,
                                gOverlay45ControlStream);
    stream = gOverlay45ControlStream;
    defaults = descriptor->unk28;
    gOverlay45MetricsIndex = descriptor->unk21;
    metrics = &gOverlay45Metrics[gOverlay45MetricsIndex];

    if (flags & 5) {
        s32 width;

        width = overlay45MeasureStream(stream, descriptor->unk21, 0);
        if (flags & 1) {
            x = x - width + 1;
        } else {
            x -= width >> 1;
        }
    }
    if (flags & 2) {
        y = y - metrics->lineHeight + 1;
    }
    if (flags & 8) {
        y -= metrics->lineHeight >> 1;
    }

    randomize = flags & 0x10;
    element = descriptor->elements;
    while (*stream != 0) {
        code = *stream++;
        if (code & 0x80) {
            code = *stream++;
            if (code == 0 || code == 0xF) {
                advance = metrics->advance;
            } else {
                drawX = defaults->x + x;
                drawY = defaults->y + y;
                advance = gOverlay45AdvanceTables[gOverlay45MetricsIndex][code];
                glyph = overlay45GetGlyph(code);
                if (glyph != NULL) {
                    advance = glyph->advance;
                    element->drawX = drawX;
                    element->drawY = drawY + glyph->yOffset;
                    descriptor->flags |= 0x40;
                    descriptor->flags &= ~0x80;
                }
            }
            if (gOverlay45TightAdvance != 0 && advance != 0) {
                advance--;
            }
            x += advance;
        } else {
            switch (code) {
            case 10:
                x = defaults->lineStartX;
                y += metrics->verticalAdvance;
                break;
            case 9:
                tabWidth = metrics->advance << 2;
                x = x + tabWidth -
                    ((x - defaults->lineStartX) % tabWidth);
                break;
            case 11:
                y += metrics->verticalAdvance;
                break;
            case 13:
                x = defaults->lineStartX;
                break;
            default:
                x += metrics->advance;
                break;
            }
        }

        if (randomize) {
            element->x = (f32)overlay45RandomRangeReloc(-0x20, 0x160);
            element->y = (f32)overlay45RandomRangeReloc(-0x20, 0xC0);
        } else if (!(flags & 0x100)) {
            element->x = (f32)element->drawX;
            element->y = (f32)element->drawY;
        }
        element++;
    }
}

/* DKR v77/v80 and JFG checks found no exact donor for this coordinate accessor. */
void overlay45ReadPair(Overlay45PairOwner *owner, s32 *x, s32 *y, s32 index) {
    Overlay45PairRecord *record = &owner->records[index];

    *x = record->x;
    *y = record->y;
}

void func_overlay_045_F000067C_188CAD4(
    Overlay45ResourceDescriptor *descriptor, s32 flags) {
    Overlay45Element *element;
    s32 i;

    element = descriptor->elements;
    descriptor->flags |= flags;
    if (flags & 0x20) {
        descriptor->flags &= ~0x40;
        descriptor->flags |= 0x80;
        i = 0;
        if (descriptor->count > 0) {
            do {
                element->offsetX = overlay45RandomRangeStoredReloc(-5, -1);
                if (overlay45RandomRangeStoredReloc(0, 1) == 1) {
                    element->offsetX = -element->offsetX;
                }
                element->offsetY = overlay45RandomRangeStoredReloc(-5, -1);
                if (overlay45RandomRangeStoredReloc(0, 1) == 1) {
                    element->offsetY = -element->offsetY;
                }
                i++;
                element++;
            } while (i < descriptor->count);
        }
    }
}

/* Mickey-local reconstruction. IDO emits the retail instruction fields; the
 * compiler-private literal-pool relocations are linked to the identical
 * constants already retained in gOverlay45InitializedData. */
void func_overlay_045_F0000764_188CBBC(
    Gfx **displayList, void *matrix, void *arg2, void *arg3,
    s32 mode, s32 updateRate) {
    Overlay45ResourceDescriptor *descriptor;
    Overlay45Element *element;
    s32 remaining;
    s32 i;
    s32 anyVisible;

    anyVisible = 0;
    func_80034920(displayList);
    func_8004B0A4(3);
    descriptor = gOverlay45ResourceHead;
    func_800221E8(displayList, NULL);
    func_80022A50(displayList, matrix);

    gDPSetScissor((*displayList)++, G_SC_NON_INTERLACE,
                  D_800D64E8[0].x1, D_800D64E8[0].y1,
                  D_800D64E8[0].x2, D_800D64E8[0].y2);

    while (descriptor != NULL) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, descriptor->unk22, 0xFF);
        if ((descriptor->mode > 0) && (mode == descriptor->unk20)) {
            remaining = descriptor->count;
            element = descriptor->elements;
            while (remaining != 0) {
                if (element->unk13 != 0) {
                    element->unk1E += element->unk1F * updateRate;
                    if (element->unk20 != 0xFF00) {
                        element->unk20 += updateRate * 8;
                        if (element->unk20 >= 0x100) {
                            element->unk20 = 0xFF00;
                        }
                    }
                    if (element->unk1E < 0) {
                        element->unk1E = 0;
                        element->unk13 = 0;
                        descriptor->unk1E--;
                        element->unk20 = 0;
                    }
                    element->unk1F -= updateRate;
                }

                if (descriptor->flags & 0x40) {
                    for (i = 0; i < updateRate; i++) {
                        element->x +=
                            ((f32)element->drawX - element->x) * 0.075f;
                        element->y +=
                            ((f32)element->drawY - element->y) * 0.075f;
                    }
                } else if (descriptor->flags & 0x80) {
                    element->x += (f32)(element->offsetX * updateRate);
                    element->y += (f32)(element->offsetY * updateRate);
                    if (element->x < -32.0f) {
                        element->x = -32.0f;
                    }
                    if (element->x > 352.0f) {
                        element->x = 352.0f;
                    }
                    if (element->y < -32.0f) {
                        element->y = -32.0f;
                    }
                    if (element->y > 272.0f) {
                        element->y = 272.0f;
                    }
                }

                element->unk0C += element->unk0E * updateRate;
                if (element->unk0C >= 0x8001) {
                    element->unk0C -= 0x8000;
                    element->unk0E = func_8002997C(0x600, 0xA00);
                }

                element->unk0A += element->unk10 * updateRate;
                if (element->unk0A < -0x1000) {
                    element->unk0A = -0x1000;
                    element->unk10 = func_8002997C(0x100, 0x200);
                } else if (element->unk0A >= 0x1001) {
                    element->unk0A = 0x1000;
                    element->unk10 = -func_8002997C(0x100, 0x200);
                }

                if ((element->x > -16.0f) && (element->x < 336.0f) &&
                    (element->y > -16.0f) && (element->y < 256.0f)) {
                    anyVisible = 1;
                    if (descriptor->flags & 0x200) {
                        s32 expired;

                        expired = 0;
                        if (element->unk22 > 0) {
                            element->unk22 -= updateRate;
                            if (element->unk22 <= 0) {
                                expired = 1;
                            }
                        }
                        if ((element->unk13 == 0) &&
                            ((func_8002997C(0, 0x3E8) >= 0x3E7) ||
                             expired)) {
                            element->unk13 = 1;
                            descriptor->unk1E++;
                            element->unk1F = 0xA;
                        }
                    }
                }

                element->unk12 = (s32)(func_8002A8C0(element->unk0C) * 5.0f);
                remaining--;
                element++;
            }

            ((s16 *)descriptor->unk28)[0] = descriptor->width;
            ((s16 *)descriptor->unk28)[1] = descriptor->height;

            if (anyVisible != 0) {
                if (descriptor->flags & 0x200) {
                    if (descriptor->optionalValue > 0) {
                        descriptor->optionalValue -= updateRate;
                    } else if (descriptor->unk1E == 0) {
                        s32 direction;
                        s32 delay;
                        s32 forward;

                        element = descriptor->elements;
                        if (descriptor->unk1F > 0) {
                            forward = 1;
                        } else if (descriptor->unk1F < 0) {
                            forward = 0;
                        } else {
                            forward = func_8002997C(0, 1);
                        }
                        descriptor->unk1F = 0;
                        if (forward != 0) {
                            delay = 4;
                            direction = 4;
                        } else {
                            direction = -4;
                            delay = descriptor->count * 4;
                        }
                        for (i = 0; i < descriptor->count; i++) {
                            element->unk22 = delay;
                            delay += direction;
                            element++;
                        }
                        if (func_8002997C(0, 3) == 0) {
                            descriptor->optionalValue = 8;
                            descriptor->unk1F = -direction;
                        } else {
                            descriptor->optionalValue =
                                func_8002997C(0x78, 0x258);
                        }
                    }
                }

                if (descriptor->flags & 0x400) {
                    for (i = 0; i < updateRate; i++) {
                        descriptor->unk10 += descriptor->unk14;
                        descriptor->unk14 -= 0.003f;
                        if (descriptor->unk10 < 0.0f) {
                            descriptor->unk10 = 0.0f;
                            descriptor->unk14 = 0.05f;
                        }
                    }
                } else {
                    descriptor->unk10 *= 0.5f;
                }

                func_overlay_045_F0001158_188D5B0(
                    displayList, (struct Overlay45Vertex **)arg2, (void *)arg3, descriptor);
            }
        }
        descriptor = descriptor->next;
    }

    if (mode == 2) {
        gDPFullSync((*displayList)++);
        gSPEndDisplayList((*displayList)++);
    }
}

static void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, Overlay45Vertex **vertexPtr, void *unused,
    Overlay45ResourceDescriptor *descriptor) {
    s32 savedFont;
    s32 screenLeft;
    /* Retail defect: intentionally uninitialized; authenticated stack reads.
     * Owner-approved preservation, docs/adr/0019-preserve-retail-loop-defect.md.
     * Initialize only in a separate future port, not the matching build.
     */
    s32 loopY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 secondVertexIndex;
    s32 thirdVertexIndex;
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
    Overlay45Vertex *vertex1;
    s32 opaqueWhite;
    s32 triangleFlags;
    s32 firstVertexIndex;
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

    gSPDisplayList(dList++, gOverlay45InitializedData.setup);
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
                    negativeHalfWidth = -halfWidth;
                    halfHeight = (f32)(upper - lower) * 0.5f;
                    negativeHalfHeight = -halfHeight;
                    lowerFloat = (f32)screenLeft;
                    rightFloat = (f32)screenRight;
                    opaqueWhite = 0xFF;
                    vertices[0].red = opaqueWhite;
                    vertices[0].green = 0xFF;
                    vertices[0].blue = 0xFF;
                    vertices[0].alpha = 0xFF;
                    vertices[0].x = (f32)screenLeft + ((negativeHalfWidth * sine - halfHeight * cosine) - negativeHalfWidth);
                    upperFloat = (f32)upper;
                    leftFloat = halfHeight * sine;
                    y = negativeHalfWidth * cosine;
                    y = (leftFloat + y) - halfHeight;
                    vertices[0].y = upperFloat + y;
                    vertices[0].z = 0;
                    vertex1 = &vertices[1];
                    vertex1->red = 0xFF;
                    vertex1->green = 0xFF;
                    vertex1->blue = 0xFF;
                    vertex1->alpha = 0xFF;
                    y = (halfWidth * sine - halfHeight * cosine) - halfWidth;
                    vertices[1].x = rightFloat + y;
                    y = halfWidth * cosine;
                    y = (leftFloat + y) - halfHeight;
                    vertices[1].y = upperFloat + y;
                    if (((halfHeight * cosine < halfHeight) | (halfHeight * cosine > halfHeight)) | (upperFloat < leftFloat) | (sine < cosine) | (sine > cosine)) {}
                    vertices[1].z = 0;
                    rightFloat = screenRight;
                    vertices[2].red = 0xFF;
                    vertices[2].green = 0xFF;
                    vertices[2].blue = 0xFF;
                    vertices[2].alpha = 0xFF;
                    y = (negativeHalfWidth * sine - negativeHalfHeight * cosine) - negativeHalfWidth;
                    vertices[2].x = lowerFloat + y;
                    vertices[2].y = (f32)lower + ((negativeHalfHeight * sine + negativeHalfWidth * cosine) - negativeHalfHeight);
                    if ((negativeHalfHeight < negativeHalfHeight * cosine) | (negativeHalfHeight > negativeHalfHeight * cosine) | (negativeHalfHeight < (f32)lower) | (negativeHalfHeight > (f32)lower) | (negativeHalfHeight < negativeHalfHeight * sine) | (negativeHalfHeight * cosine < negativeHalfHeight * sine) | (textureLeft < textureTop) | (textureRight < textureBottom)) {}
                    vertices[2].z = 0;
                    x = -negativeHalfWidth;
                    vertices[3].red = 0xFF;
                    vertices[3].green = 0xFF;
                    vertices[3].blue = 0xFF;
                    vertices[3].alpha = 0xFF;
                    y = (x * sine - negativeHalfHeight * cosine) - x;
                    vertices[3].x = rightFloat + y;
                    vertices[3].y = (f32)lower + ((negativeHalfHeight * sine + x * cosine) - negativeHalfHeight);
                    vertices[3].z = 0;

                    triangles->vertex2 = (thirdVertexIndex = 2, thirdVertexIndex);
                    triangleFlags = 0x40;
                    firstVertexIndex = 0;
                    triangles->flags = triangleFlags; triangles->vertex1 = (secondVertexIndex = 1, secondVertexIndex); triangles->vertex0 = firstVertexIndex;
                    triangles->s0 = textureLeft;
                    triangles->t0 = textureTop;
                    triangles->s1 = textureRight;
                    triangles->t1 = textureTop;
                    triangles->s2 = textureLeft;
                    triangles->t2 = textureBottom;
                    triangles++;

                    triangles->vertex0 = 1;
                    triangles->vertex1 = 3;
                    triangles->vertex2 = 2;
                    triangles->flags = 0x40;
                    triangles->s0 = textureRight;
                    triangles->t0 = textureTop;
                    triangles->s1 = textureRight;
                    triangles->t1 = textureBottom;
                    triangles->s2 = textureLeft;
                    triangles->t2 = textureBottom;
                    vertices += 4;
                    triangles++;
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

/*
 * Overlay 45 matched-C tail after the hand-written middle. Exact DKR v77/v80
 * and JFG scans are negative beyond generic null-guarded field setters.
 */

void overlay45SetMode(Overlay45ResourceDescriptor *descriptor, s32 mode) {
    if (descriptor != NULL) {
        descriptor->mode = mode;
    }
}

void overlay45SetField22(Overlay45ResourceDescriptor *descriptor, s32 value) {
    if (descriptor != NULL) {
        descriptor->unk22 = value;
    }
}

void overlay45SetField20(Overlay45ResourceDescriptor *descriptor, s32 value) {
    if (descriptor != NULL) {
        descriptor->unk20 = value;
    }
}
