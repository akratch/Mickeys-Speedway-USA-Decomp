#include "overlays/overlay_049.h"

/* Donor scan: DKR v77/v80 and JFG contain no exact initializer donor. */
void overlay49Initialize(void) {
    s32 rank;
    s32 unused1;
    s32 shift;
    s32 mask;
    s32 desired;
    s32 current;
    Overlay49LookupResult *lookup;
    Overlay49Status *status;
    s32 i;
    s8 *modes;
    u16 *packed;

    status = func_80028F54();
    overlay65Initialize();
    packed = D_800D3128;
    if (D_8007BF08 != 0) {
        mask = 0xE00;
        shift = 9;
    } else {
        mask = gOverlay49Masks[D_8007BF04];
        shift = gOverlay49Shifts[D_8007BF04];
    }
    current = (packed[status->player + 4] & mask) >> shift;
    gOverlay49Result = NULL;
    if (status[0].value == 0x24) {
        lookup = func_800508B4(0x1E);
        if (lookup != NULL) {
            gOverlay49Result = lookup->value;
        }
        desired = 4;
    } else {
        rank = 0;
        modes = gOverlay49Modes;
        for (i = 1; i < 6; i++) {
            if (status[0].value < status[i].value) {
                rank++;
            }
        }
        desired = modes[rank];
    }
    if (current < desired) {
        D_800D3128[status->player + 4] &= ~mask;
        D_800D3128[status->player + 4] |= desired << shift;
        func_8002917C(desired);
    }
    if (current < 3 && current < desired) {
        goto slow;
    }
    gOverlay49InputEnabled = 1;
    goto finish;
slow:
    gOverlay49InputEnabled = 0;
finish:
    gOverlay49Timer = 0xA50;
    gOverlay49Finished = 0;
}

/* No exact updater donor exists in DKR v77/v80 or JFG. */
void overlay49Update(s32 updateRate) {
    s32 index;
    u32 inputA;
    u32 inputB;

    inputA = 0;
    inputB = 0;
    index = 3;
    do {
        inputA |= func_800254FC(index);
        inputB |= func_8002554C(index);
    } while (index--);
    if (gOverlay49Result != NULL) {
        gOverlay49Result->mode = 3;
    }
    if (gOverlay49Finished == 0) {
        gOverlay49Timer -= updateRate;
        if (gOverlay49Timer < 2520) {
            if (gOverlay49InputEnabled != 0 && (inputB & 0x9000) != 0) {
                gOverlay49Timer = 0;
            }
            if ((inputA & 0x820) == 0x820) {
                gOverlay49Timer = 0;
            }
        }
        if (gOverlay49Timer <= 0) {
            func_800016EC(9);
            if (D_8007BF0C == 0) {
                func_8003A754();
            }
            if (D_8007BF60 != 0) {
                overlay48Initialize();
            } else {
                func_80028374(12, 0, 0, 12, 1, 0);
            }
            gOverlay49Finished = 1;
        }
    }
    overlay65UpdateParticles(D_800D3140, D_800D3144, updateRate);
}

/*
 * PROVENANCE: JFG overlay 2 supplies this exact wrapper body and name.
 * Its three trailing alignment nops remain generated assembly and receive
 * no C credit. Mickey's call relocation targets overlay 65 +0xBC0.
 */
void refractOutput(void) {
    overlay65Release();
}
