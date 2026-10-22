#include "PR/ultratypes.h"

typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

extern s32 D_8007D6B0;
extern Overlay40Entry D_800D6BF8[8];

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
