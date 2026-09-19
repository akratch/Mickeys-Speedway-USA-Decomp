#include "overlay31BuildLookupTables.h"

#ifndef OVERLAY31_MAX_LEVEL
#define OVERLAY31_MAX_LEVEL 9
#endif

#define INIT_SHORT_PREFIX(record) \
    (record)->b4 = 0;         \
    (record)->b6 = 0;         \
    (record)->b8 = 0x200;     \
    (record)->bA = 0;         \
    (record)->bC = 0x200;     \
    (record)->bE = 0x200;     \
    (record)->c10 = 0x40

#define INIT_SHORT_SUFFIX(record) \
    (record)->d14 = 0;        \
    (record)->d16 = 0;        \
    (record)->d18 = 0x200;    \
    (record)->d1A = 0x200;    \
    (record)->d1C = 0;        \
    (record)->d1E = 0x200

#define INIT_SHORT_PREFIX_REVERSED(record) \
    (record)->c10 = 0x40;     \
    (record)->bE = 0x200;     \
    (record)->bC = 0x200;     \
    (record)->bA = 0;         \
    (record)->b8 = 0x200;     \
    (record)->b6 = 0;         \
    (record)->b4 = 0

#define INIT_SHORT_SUFFIX_REVERSED(record) \
    (record)->d1E = 0x200;    \
    (record)->d1C = 0;        \
    (record)->d1A = 0x200;    \
    (record)->d18 = 0x200;    \
    (record)->d16 = 0;        \
    (record)->d14 = 0

/* Workbench structure-mismatch: 186/186 instructions, 94 normalized words; first +0x2C.
 * 71 aligned residuals (19 structural, 4 schedule, 47 register); signed counter/field probes worsened.
 * Target cursor/global layout versus meaningful array source leaves pool/temp divergence. */
#ifdef NON_MATCHING
void func_overlay_031_F0000000_187F520(void) {
    Overlay31IndexRecord *first;
    Overlay31IndexRecord *second;
    Overlay31FloatPair *pairs;
    s32 level;
    s32 index;
    s32 sum;

    first = overlay31AllocateReloc(0x880, 0x8C);
    gOverlay31IndexRows[0][0] = first;
    second = first + 34;
    level = 2;
    do {
        s32 limit;

        limit = level - 1;
        gOverlay31IndexRows[level - 2][0] = first;
        gOverlay31IndexRows[level - 2][1] = second;
        index = 0;
        if (limit > 0) {
            s32 next;
            s32 firstNeighbor;
            s32 secondNeighbor;

            firstNeighbor = level;
            secondNeighbor = level + 1;
            do {
                first->a1 = index;
                next = index + 1;
                first->a3 = secondNeighbor;
                first->c11 = index;
                first->c12 = secondNeighbor;
                first->c13 = firstNeighbor;
                first->a0 = 0x40;
                first->a2 = next;
                INIT_SHORT_PREFIX(first);
                INIT_SHORT_SUFFIX(first);

                second->a1 = firstNeighbor;
                second->a2 = secondNeighbor;
                second->c11 = firstNeighbor;
                second->c13 = index;
                index = next;
                firstNeighbor++;
                secondNeighbor++;
                first++;
                second->a0 = 0x40;
                second->a3 = next;
                INIT_SHORT_PREFIX_REVERSED(second);
                second->c12 = next;
                INIT_SHORT_SUFFIX_REVERSED(second);
                second++;
            } while (next < limit);
        }
        sum = index + level;
        if (level >= 3) {
            first->a0 = 0x40;
            first->a1 = index;
            first->a2 = 0;
            first->a3 = level;
            INIT_SHORT_PREFIX(first);
            first->c11 = index;
            first->c12 = level;
            first->c13 = sum;
            INIT_SHORT_SUFFIX(first);
            first++;

            second->a0 = 0x40;
            second->a1 = sum;
            second->a2 = level;
            second->a3 = 0;
            INIT_SHORT_PREFIX_REVERSED(second);
            second->c11 = sum;
            second->c12 = 0;
            second->c13 = index;
            INIT_SHORT_SUFFIX_REVERSED(second);
            second++;
        }
        level++;
    } while (level < OVERLAY31_MAX_LEVEL);

    pairs = overlay31AllocateReloc(0x118, 0x8C);
    {
        level = 2;
        do {
            s16 angle;
            s16 step;

            gOverlay31FloatRows[level - 2] = pairs;
            angle = 0;
            index = 0;
            if (level > 0) {
                step = (s16)(0xFFFF / level);
                do {
                f32 firstValue;
                f32 secondValue;
                f32 doubledFirst;
                f32 doubledSecond;

                firstValue = func_8002A8BC(angle);
                secondValue = func_8002A8C0(angle);
                doubledFirst = firstValue + firstValue;
                doubledSecond = secondValue + secondValue;
                angle += step;
                index++;
                pairs->first = doubledFirst + doubledSecond;
                pairs->second = doubledFirst - doubledSecond;
                    pairs++;
                    pairs = (Overlay31FloatPair *) ((u32) pairs | 0);
                } while (index != level);
            }
            level++;
        } while (level != OVERLAY31_MAX_LEVEL);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o031/overlay31BuildLookupTables/func_overlay_031_F0000000_187F520.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_031_F0000000_187F520:start
 * symbol: func_overlay_031_F0000000_187F520
 * score: 86/186 words
 * frame: 0x40
 * relocations: 10
 * first-mismatch: +0x34
 * summary: L160 indexed walks missize 20-68. L109 zero stays 0x40 delta 0 at 92 (phantom t3). lui-addiu first store is +4. Neighbour colour forces inert at 86.
 * PLATEAU-HANDOFF:func_overlay_031_F0000000_187F520:end
 */
