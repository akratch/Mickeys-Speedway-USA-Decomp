#include "PR/ultratypes.h"

typedef struct Overlay60Choice {
    u8 pad0[0x28];
    s16 slot;
    u8 pad2A;
    s8 active;
    u8 pad2C[8];
} Overlay60Choice;

/* The four-entry resident player block. Two pass names keep the two
 * address-materialization lifetimes; the pass-2 end sentinel is a distinct
 * symbol so the back-edge comparison keeps End first (overlay 57's form). */
extern Overlay60Choice D_800D3058_o060Reloc[4];
extern Overlay60Choice D_800D3058Pass2_o060Reloc[4];
extern Overlay60Choice D_800D3058Pass2End_o060Reloc[];

/* Matched 2026-09-17 (lane w8-o060), 35 -> 0 masked words at delta 0,
 * frame 0x20, eight relocations, unforced. L160: delete the walking choice
 * pointer and subscript the player block from a while-index so IDO generates
 * the cursor. available[14] holds the index home at the 0x20 frame. */
void overlay60ReassignChoiceSlots(void) {
    s32 i;
    u8 available[14];

    i = 0;
    while (i < 10) {
        available[i] = 1;
        i++;
    }

    i = 0;
    do {
        if (D_800D3058_o060Reloc[i].active != 0) {
            available[D_800D3058_o060Reloc[i].slot & 0xF] = 0;
        }
        i++;
    } while (&D_800D3058_o060Reloc[i] < &D_800D3058_o060Reloc[4]);

    i = 0;
    do {
        if ((D_800D3058Pass2_o060Reloc[i].active != 0) && (D_800D3058Pass2_o060Reloc[i].slot >= 6)) {
            D_800D3058Pass2_o060Reloc[i].slot = 0;
            while (available[D_800D3058Pass2_o060Reloc[i].slot] == 0) {
                D_800D3058Pass2_o060Reloc[i].slot++;
            }
            available[D_800D3058Pass2_o060Reloc[i].slot] = 0;
        }
        i++;
    } while (D_800D3058Pass2End_o060Reloc != &D_800D3058Pass2_o060Reloc[i]);
}
