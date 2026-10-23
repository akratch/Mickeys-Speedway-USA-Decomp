#include "overlays/overlay_040.h"

/*
 * Claims the first free slot (state -1) of the eight entries.
 *
 * The 2026-08-28 match (CREW-O40-ADD-PIPE-09) reached the target allocation
 * with permuter output: a `remaining = 7; do { } while (remaining--)` walk, a
 * zero carrier `new_var` used as the state and a subscript, and a chain of
 * all-ones masks of that carrier added to `green`. All three stood in for the
 * loop shape. Written as a counted `while (remaining--)` from 8, the counter
 * keeps a0 and the 30 takes v1 as shipped, with no carrier (lane c-5,
 * 2026-10-09). The `do`/`while` without the stand-ins swaps a0/v1 (4 words).
 *
 * The real-TU object is 33 instructions (0x84 bytes) with four relocations:
 * D_8007D6B0 HI16/LO16 at 0x00/0x04 and D_800D6BF8 HI16/LO16 at
 * 0x0C/0x1C.
 *
 * The one resident caller (func_800517E0, stream command 0x4A) stores a fifth
 * argument, the byte it has just tested non-zero, and dropping it there
 * changes that function. Nothing here reads it, and an unused stack argument
 * leaves no trace in the callee, so the fifth formal is declared and unused.
 */
void overlay40AddEntry(volatile s32 id, s32 red, s32 green, s32 blue, s32 unused) {
    Overlay40Entry *entry;
    s32 remaining;

    entry = D_800D6BF8;
    if (D_8007D6B0 < 8) {
        remaining = 8;
        while (remaining--) {
            if (entry->state == -1) {
                entry->state = 0;
                entry->scales[0] = 30;
                entry->scales[1] = 30;
                entry->scales[2] = 30;
                entry->red = red;
                entry->green = green;
                entry->blue = blue;
                entry->id = id;
                D_8007D6B0++;
                return;
            }
            entry++;
        }
    }
}

/* The counted walk is overlay40AddEntry's: a `while (remaining--)` from 8
 * gives the counter a0 and moves id to a1 as shipped. The function reads only
 * a0; an earlier spelling reached the same allocation through a second formal
 * overwritten before use, which the single resident caller never passes. */
void overlay40RemoveEntry(s32 id) {
    Overlay40Entry *entry;
    s32 remaining;

    entry = D_800D6BF8;
    if (D_8007D6B0 > 0) {
        remaining = 8;
        while (remaining--) {
            if (entry->state != -1 && entry->id == id) {
                entry->state = -1;
                D_8007D6B0--;
            }
            entry++;
        }
    }
}

/* The counted walk is overlay40AddEntry's `while (remaining--)` from 8. The
 * function reads only a0. An earlier spelling made the counter a second
 * formal, `remaining = 7; do { ... } while (remaining--)`, and needed the
 * count and the hoisted `D_800D6B00` address on one source line to get
 * as1's order; as a local with this loop shape it needs neither, and the
 * resident caller passes the one argument (2026-10-22). */
void overlay40UpdateEntries(s32 amount) {
    Overlay40Entry *entry;
    s8 previous;
    Overlay40Object *object;
    s32 remaining;

    entry = D_800D6BF8;
    remaining = 8;
    while (remaining--) {
        if (entry->state != -1) {
            if (entry->state < 8) {
                entry->state++;
            }
            previous = entry->scales[0];
            entry->scales[2] = entry->scales[1];
            entry->scales[0] = previous - amount;
            entry->scales[1] = previous;
            if (entry->scales[0] < 0) {
                entry->scales[0] = 0;
            }
            object = D_800D6B00[entry->id];
            if (object != 0 && object->node != 0) {
                if (entry->state & 1) {
                    object->node->effect64 = &entry->red;
                } else {
                    object->node->effect64 = 0;
                }
            }
        }
        entry++;
    }
}

/* Matched 2026-10-02 (lane x-ovlb), 60 -> 0 masked words. Earlier lanes had
 * the eight records as constant subscripts, with #line directives, a volatile
 * round trip and a padding aggregate. What closed it was writing the records
 * the plain way:
 * - one cursor walks the array, so IDO keeps the last record's base in a
 *   register (the target's v0 = sp+0x94) and spills one shared sum for
 *   register pressure, as the target does. Nothing in the source asks for
 *   either;
 * - the cursor is declared before the array, with the three scalars, so the
 *   array lands at sp+0x40;
 * - each field store is on its own line. A one-line macro gives all five
 *   stores one line number, and as1 then breaks the tie in the wrong order. */
static void overlay40BuildFrame(void *displayList, s32 x, s32 y, s32 width,
                         s32 height, s32 red, s32 green, s32 blue, s32 alpha) {
    s32 right;
    s32 bottom;
    u32 color;
    Overlay40FrameRecord *rec;
    Overlay40FrameRecord records[8];

    right = x + width;
    bottom = y + height;
    color = (red << 24) | (green << 16) | (blue << 8) | (alpha & 0xFF);
    rec = records;
    rec->left = x - 2;
    rec->top = y - 2;
    rec->right = right + 2;
    rec->bottom = y + 3;
    rec->color = 0;
    rec++;
    rec->left = x;
    rec->top = y;
    rec->right = right + 1;
    rec->bottom = y + 1;
    rec->color = color;
    rec++;
    rec->left = x - 2;
    rec->top = y - 2;
    rec->right = x + 3;
    rec->bottom = bottom + 2;
    rec->color = 0;
    rec++;
    rec->left = x;
    rec->top = y + 1;
    rec->right = x + 1;
    rec->bottom = bottom;
    rec->color = color;
    rec++;
    rec->left = right - 2;
    rec->top = y - 2;
    rec->right = right + 3;
    rec->bottom = bottom + 2;
    rec->color = 0;
    rec++;
    rec->left = right;
    rec->top = y + 1;
    rec->right = right + 1;
    rec->bottom = bottom;
    rec->color = color;
    rec++;
    rec->left = x - 2;
    rec->top = bottom - 2;
    rec->right = right + 2;
    rec->bottom = bottom + 3;
    rec->color = 0;
    rec++;
    rec->left = x;
    rec->top = bottom;
    rec->right = right + 1;
    rec->bottom = bottom + 1;
    rec->color = color;
    frontDrawRectangles(displayList, 8, records, 0);
}

void overlay40DrawEntries(void *displayList) {
    Overlay40Entry *entry;
    Overlay40Object *object;
    Overlay40Node *position;
    s8 *scaleValue;
    s32 remaining;
    f32 screenX;
    f32 screenY;
    s32 scaleRemaining;
    s32 alpha;
    s32 scale;
    s32 lastScale;

    entry = D_800D6BF8;
    /* IDO schedules both relocated table bases before the counter on this line. */
    remaining = 7; do {
        if (entry->state != -1) {
            object = D_800D6B00[entry->id];
            if (object != NULL) {
                position = object->node;
                if (position != NULL &&
                    camProjectPoint(position->x, position->y, position->z,
                                          &screenX, &screenY, 1) != 0) {
                    scaleValue = entry->scales;
                    alpha = 0xA0;
                    lastScale = -1;
                    scaleRemaining = 2;
                    do {
                        scale = (*scaleValue * 8) + 0x10;
                        if (scale != lastScale) {
                            overlay40BuildFrame(displayList,
                                               (s32) screenX - scale,
                                               (s32) screenY - scale,
                                               scale * 2, scale * 2,
                                               entry->red, entry->green,
                                               entry->blue, alpha);
                            lastScale = scale;
                        }
                        alpha -= 0x34;
                        scaleValue++;
                    } while (scaleRemaining--);
                }
            }
        }
        entry++;
    } while (remaining--);
}

void overlay40SetValues(s32 value0, s32 value1, s32 value2, s32 value3,
                        s32 value4) {
    D_800D6C38 = value0;
    gOverlay40Value1 = value1;
    gOverlay40Value4 = value2;
    gOverlay40Value5 = D_800D6C3E;
    gOverlay40Value6 = value3;
    gOverlay40Value7 = value4;
    D_800D6C44 = value4;
}

void overlay40Interpolate(s32 amount) {
    if (gOverlay40BlendTimer != 0) {
        gOverlay40BlendOutput = gOverlay40BlendCurrent;
        gOverlay40BlendTimer -= amount;
        if (gOverlay40BlendTimer > 0) {
            gOverlay40BlendOutput +=
                ((gOverlay40BlendTarget - gOverlay40BlendCurrent) *
                 gOverlay40BlendTimer) /
                gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
    }
}

/* Overlay 40 text +0x534..+0x690. */
void overlay40DrawTintRectangle(Gfx **displayList) {
    s32 inverse, red, green, blue;
    s32 x;
    s32 y;

    if (D_800D6C3E != 0) {
        o40TintCallReloc(&x, &y, displayList);
        inverse = 0xFF - D_800D6C3E;
        red = (D_800D6C38 * inverse) >> 8;
        green = (gO40TintGreenReloc * inverse) >> 8;
        blue = (gO40TintBlueReloc * inverse) >> 8;
        gSPDisplayList((*displayList)++, gO40TintDisplayListReloc);
        gDPSetPrimColor((*displayList)++, 0, 0, red, green, blue,
                        D_800D6C3E);
        gDPFillRectangle((*displayList)++, 0, 0, x, y);
        o40TintCallReloc(displayList);
        gDPSetPrimColor((*displayList)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}

/* The blend base is read twice, not copied into a second local. The target
 * wants the load itself in v1 and the running output in v0; with two named
 * locals both take pool colours in first-definition order, so whichever name
 * the load defines takes v0 and the pair can only be swapped, never separated
 * (98/101 either way round). Spelling the second use as the global read again
 * makes the loaded value a CSE temporary, which uopt numbers after every
 * named local, so it takes v1 and `output` -- now defined by the copy -- takes
 * v0. Keeping `current` declared *and* assigned from the global costs a second
 * %hi materialisation (102 words); dropping the local is what collapses it.
 * The object emits all ten runtime HI16/LO16 roles: D_800D6C4C(timer),
 * D_800D6C52(current), D_800D6C50(target), D_800D6C4E(duration), and
 * D_800D6C54(output).
 * Owned Overlay 40 +0x690..+0x824 / ROM 0x1886F40..0x18870D4 excludes separate
 * +0x824..+0x830 padding. ORT 1314 and resident runtime record 156 at
 * trackUpdateLighting+0x130 authenticate the sole inbound trap site. Mickey-only;
 * exact pinned DKR v77/v80/JFG scans are negative. */
void overlay40FadeRecords(register s32 *enabled, Overlay40FadeContext *context,
                          s32 amount) {
    Overlay40FadeRecord *record;
    Overlay40ColorSource *source;
    Overlay40Color *color;
    Overlay40Vertex *vertex;
    s16 *group;
    s32 output;
    s32 timer;
    s32 remaining;
    s32 groupRemaining;
    s32 vertexRemaining;

    output = gOverlay40BlendCurrent;
    timer = gOverlay40BlendTimer;
    if (timer != 0) {
        if (amount < timer) {
            gOverlay40BlendTimer = timer - amount;
            output += ((gOverlay40BlendTarget - gOverlay40BlendCurrent) *
                      gOverlay40BlendTimer) / gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
        gOverlay40BlendOutput = output;
    }

    record = context->records;
    remaining = context->count;
    while (remaining--) {
        if (*enabled != 0) {
            source = record->source;
            if (source != 0) {
                group = source->groups;
                groupRemaining = (record->count + 0xF) >> 4;
                while (groupRemaining--) {
                    *group++ = 0;
                }

                color = source->colors;
                vertexRemaining = record->count;
                vertex = record->vertices;
                while (vertexRemaining--) {
                    vertex->red = (color->red * output) >> 8;
                    vertex->green = (color->green * output) >> 8;
                    vertex->blue = (color->blue * output) >> 8;
                    vertex++;
                    color++;
                }
                record->dirty = 0;
            }
        }
        enabled++;
        record++;
    }
}
