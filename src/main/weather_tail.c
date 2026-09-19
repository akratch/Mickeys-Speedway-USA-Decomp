/*
 * Weather tail -- ROM 0x3D370-0x3D5F0.
 *
 * This is the C-owned tail of main/weather after its hand-written snow
 * assembly island.  The final helper remains descriptively unresolved.
 */

#include "PR/ultratypes.h"

extern s32 D_8007C854;
extern s32 D_8007C858;
extern s32 D_8007C85C;
extern s32 D_8007C860;
extern s32 D_8007C864;
extern s32 D_8007C868;

void func_8003C80C();

void func_8003C770(s32 arg0, s32 arg1) {
    s32 index;

    index = arg0;
    if ((arg0 < 0) || (arg0 >= 24)) {
        index = 0;
    }
    if (arg1 < 0) {
        arg1 = 0;
    }
    if (arg1 == 0) {
        D_8007C854 = 0;
        D_8007C858 = 255;
        D_8007C85C = 255;
        return;
    }
    D_8007C854 = 1;
    D_8007C860 = index >> 2;
    D_8007C864 = ((index & 3) * arg1) >> 2;
    D_8007C868 = arg1;
    func_8003C80C(0, arg1, index);
}

extern u8 *D_8007C838[];
extern u8 D_800D40F0[];
extern void func_8002EBD4(u32 value);

/* The one-word deficit was two missing register copies, and cause and cure are
 * the same line pair.
 *
 * Census against the target was `lw +1, move -2`. The target initialises the
 * two palette cursors with `move v0,t3` and `move a3,t4` from the pair it has
 * just loaded out of the frame table, keeping four live carriers: two for the
 * loaded pointers, which it still needs after the loop for the two tail
 * interpolations, and two for the cursors the loop advances. Assigning the
 * cursors from the named `temp_t3`/`temp_t4` gives each pointer one carrier,
 * so no copy is emitted and `temp_t4` is spilled to the frame and reloaded
 * instead -- the extra `lw`.
 *
 * Spelling the two cursor initialisers as the table read again, rather than as
 * the named locals, restores both copies and removes the spill: 118 words
 * against 118, instruction delta zero, every opcode count equal, frame 0x38
 * on both sides, and the aligned structural residual down from 75 to 45. The
 * values are provably identical -- `temp_t2` is a local, the reads have no
 * intervening store, and `sp20`/`sp1C` still take the named locals, which is
 * what keeps them alive across the call.
 *
 * L99: an unused pointer first (not pad[8]) keeps frame 0x38 and moves the
 * extra home from +0x18 to +0x24, 104 to 103 masked, draws unchanged. Named
 * size levers (remat-delete, empty-if, leftover OR-zero, comma-assign, L160)
 * are flat or size -4. The first delay is still the arg copy; target extra
 * home remains +0x30. */
/* PROVENANCE: palette layouts and interpolation control flow are reconstructed from Mickey's target accesses; no external donor body was used. */
#ifdef NON_MATCHING
void func_8003C80C(s32 arg0) {
    void *pad;
    s32 sp30;
    u8 *sp20;
    u8 *sp1C;
    u8 *temp_t3;
    u8 *temp_t4;
    s32 temp_lo;
    s32 temp_t6;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_a1;
    s32 var_t0;
    u8 *var_a3;
    u8 *var_v0;
    u8 temp_a0;
    u8 temp_a1;
    u8 temp_a2;
    u8 temp_t7;
    u8 *var_v1;
    void *temp_t2;

    if (D_8007C854 != 0) {
        var_v1 = D_800D40F0;
        var_a1 = D_8007C864 + arg0;
        D_8007C864 = var_a1;
        var_t0 = 8;
        if (var_a1 >= D_8007C868) {
            do {
                D_8007C864 = var_a1 - D_8007C868;
                temp_t6 = D_8007C860 + 1;
                D_8007C860 = temp_t6;
                if (temp_t6 >= 6) {
                    D_8007C860 = 0;
                }
                var_a1 = D_8007C864;
            } while (var_a1 >= D_8007C868);
        }
        temp_t2 = (void *) ((D_8007C860 * 4) + (s32) D_8007C838);
        temp_t3 = *(u8 **) temp_t2;
        temp_t4 = *(u8 **) ((u8 *) temp_t2 + 4);
        temp_lo = (var_a1 << 0x10) / D_8007C868;
        var_v0 = *(u8 **) temp_t2;
        var_a3 = *(u8 **) ((u8 *) temp_t2 + 4);
        do {
            temp_a0 = *var_v0;
            temp_t7 = *var_a3;
            var_v0 += 4;
            var_a3 += 4;
            var_v1 += 4;
            var_v1[-4] = (s8) ((((temp_t7 - temp_a0) * temp_lo) >> 0x10) + temp_a0);
            temp_a1 = var_v0[-3];
            var_v1[-3] = (s8) ((((var_a3[-3] - temp_a1) * temp_lo) >> 0x10) + temp_a1);
            temp_a2 = var_v0[-2];
            var_v1[-2] = (s8) ((((var_a3[-2] - temp_a2) * temp_lo) >> 0x10) + temp_a2);
            var_v1[-1] = var_v0[-1];
            var_t0 -= 1;
        } while (var_t0 != 0);
        sp30 = temp_lo;
        sp20 = temp_t3;
        sp1C = temp_t4;
        func_8002EBD4((u32) D_800D40F0);
        temp_v0 = *(s32 *) (sp20 + 0x24);
        D_8007C858 = temp_v0 + ((*(s32 *) (sp1C + 0x24) - temp_v0) * sp30 >> 0x10);
        temp_v1 = *(s32 *) (sp20 + 0x28);
        D_8007C85C = temp_v1 + ((*(s32 *) (sp1C + 0x28) - temp_v1) * sp30 >> 0x10);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather_tail/func_8003C80C.s")
#endif

/* PLATEAU-HANDOFF:func_8003C80C:start
 * symbol: func_8003C80C
 * score: 103/118 words
 * frame: 0x38
 * relocations: 21
 * first-mismatch: +0x14
 * summary: L99 unused pointer: 103/118, extra home +0x24 not +0x30. remat/empty-if size -4; leftover/comma/L160 flat. First delay still the arg copy.
 * PLATEAU-HANDOFF:func_8003C80C:end
 */
