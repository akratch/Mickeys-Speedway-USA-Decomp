#include "PR/ultratypes.h"

typedef struct Overlay40FrameRecord {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 color;
} Overlay40FrameRecord;

/* frontDrawRectangles (resident 0x80039380), reached through the overlay
 * loader's SYMBOL record; the placeholder carries the shipped jal addend. */
extern void overlay40DrawRectanglesReloc(void *displayList, s32 count,
                                         Overlay40FrameRecord *records,
                                         s32 translucent);

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
void overlay40BuildFrame(void *displayList, s32 x, s32 y, s32 width,
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
    overlay40DrawRectanglesReloc(displayList, 8, records, 0);
}
