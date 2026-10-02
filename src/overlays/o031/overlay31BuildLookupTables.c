#include "overlay31BuildLookupTables.h"

/* Matched 2026-10-02 from an 86-word plateau. What it took: plain counted
 * loops with every vertex index written as an expression of the loop index
 * (uopt builds the level + i cursors itself); the triangle row bound held in
 * a local, which keeps the signed compare the target tests; the record
 * pointers read back from the row table rather than from the allocation's
 * return; the float table written through an f32 cursor with two
 * post-increments, whose extra references rank it above the loop index for
 * s1; and angle cleared before the row store. */
void func_overlay_031_F0000000_187F520(void) {
    Overlay31IndexRecord *first;
    Overlay31IndexRecord *second;
    f32 *out;
    s32 level;
    s32 i;
    s32 limit;
    s16 angle;
    f32 s;
    f32 c;

    gOverlay31IndexRows[0][0] = overlay31AllocateReloc(0x880, 0x8C);
    first = gOverlay31IndexRows[0][0];
    second = gOverlay31IndexRows[0][0] + 34;
    for (level = 2; level < 9; level++) {
        gOverlay31IndexRows[level - 2][0] = first;
        gOverlay31IndexRows[level - 2][1] = second;
        limit = level - 1;
        for (i = 0; i < limit; i++) {
            first->a0 = 0x40;
            first->a1 = i;
            first->a2 = i + 1;
            first->a3 = level + i + 1;
            first->b4 = 0;
            first->b6 = 0;
            first->b8 = 0x200;
            first->bA = 0;
            first->bC = 0x200;
            first->bE = 0x200;
            first->c10 = 0x40;
            first->c11 = i;
            first->c12 = level + i + 1;
            first->c13 = level + i;
            first->d14 = 0;
            first->d16 = 0;
            first->d18 = 0x200;
            first->d1A = 0x200;
            first->d1C = 0;
            first->d1E = 0x200;
            first++;
            second->a0 = 0x40;
            second->a1 = level + i;
            second->a2 = level + i + 1;
            second->a3 = i + 1;
            second->b4 = 0;
            second->b6 = 0;
            second->b8 = 0x200;
            second->bA = 0;
            second->bC = 0x200;
            second->bE = 0x200;
            second->c10 = 0x40;
            second->c11 = level + i;
            second->c12 = i + 1;
            second->c13 = i;
            second->d14 = 0;
            second->d16 = 0;
            second->d18 = 0x200;
            second->d1A = 0x200;
            second->d1C = 0;
            second->d1E = 0x200;
            second++;
        }
        if (level >= 3) {
            first->a0 = 0x40;
            first->a1 = i;
            first->a2 = 0;
            first->a3 = level;
            first->b4 = 0;
            first->b6 = 0;
            first->b8 = 0x200;
            first->bA = 0;
            first->bC = 0x200;
            first->bE = 0x200;
            first->c10 = 0x40;
            first->c11 = i;
            first->c12 = level;
            first->c13 = i + level;
            first->d14 = 0;
            first->d16 = 0;
            first->d18 = 0x200;
            first->d1A = 0x200;
            first->d1C = 0;
            first->d1E = 0x200;
            first++;
            second->a0 = 0x40;
            second->a1 = i + level;
            second->a2 = level;
            second->a3 = 0;
            second->b4 = 0;
            second->b6 = 0;
            second->b8 = 0x200;
            second->bA = 0;
            second->bC = 0x200;
            second->bE = 0x200;
            second->c10 = 0x40;
            second->c11 = i + level;
            second->c12 = 0;
            second->c13 = i;
            second->d14 = 0;
            second->d16 = 0;
            second->d18 = 0x200;
            second->d1A = 0x200;
            second->d1C = 0;
            second->d1E = 0x200;
            second++;
        }
    }

    out = overlay31AllocateReloc(0x118, 0x8C);
    for (level = 2; level < 9; level++) {
        angle = 0;
        gOverlay31FloatRows[level - 2] = out;
        for (i = 0; i < level; i++) {
            s = func_8002A8BC_o031Reloc(angle);
            c = func_8002A8C0_o031Reloc(angle);
            *out++ = s * 2.0f + c * 2.0f;
            *out++ = s * 2.0f - c * 2.0f;
            angle += (s16)(0xFFFF / level);
        }
    }
}
