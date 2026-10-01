#include "PR/ultratypes.h"

typedef struct Overlay58InputRecord {
    u8 state;
    u8 phase;
    u8 player;
    u8 reserved03[0x23];
    u16 rank;
} Overlay58InputRecord;

/* The gate, the two mode bytes and the packed status words are resident
 * objects reached through runtime relocation records. */
extern u8 gOverlay58FinalizerGate;
extern s32 gOverlay58PrimaryOrderIndex; /* overlay-local BSS +0x74 */
extern u8 gOverlay58PackedMode;
extern u8 gOverlay58ExtendedPackedMode;
extern u16 gOverlay58PackedStatus[];

extern Overlay58InputRecord *func_80028F54(void);
extern void func_8003A680(s32 code);
extern void func_800291B4(void);

/* Exact body for executable range +0x5554..+0x5A14.
 *
 * Matched 2026-10-01 by discarding the probes, volatile reads and comma
 * assignments of the plateau shape and writing what the relocation records
 * and the frame name:
 * - all twenty-four call sites are three resident functions;
 * - nine locals, with the records pointer declared last. The packed mode
 *   and the selected player are not locals: each is read from its object at
 *   every use, so the compiler keeps one shared load;
 * - `self` is assigned from the record inside each counting loop. The
 *   assignment is hoisted to the preheader as a copy of the shared load,
 *   and the in-loop reference is what ranks that load ahead of the other
 *   caller-saved values;
 * - the decoded field of the third loop reuses `mask`, which keeps it in the
 *   mask's register;
 * - the posted count is the expression `count + 1` at both uses, so it is a
 *   compiler temporary with its own home rather than the count's. */
void overlay58FinalizePackedStatus(void) {
    s32 i;
    s32 count;
    s32 fourCount;
    s32 desired;
    s32 self;
    s32 current;
    s32 mask;
    s32 shift;
    Overlay58InputRecord *records;

    records = func_80028F54();

    if (gOverlay58FinalizerGate != 0) {
        return;
    }
    if (records[0].state != 0) {
        return;
    }
    if (records[0].phase != 3) {
        return;
    }
    if (gOverlay58PrimaryOrderIndex >= 4) {
        return;
    }

    desired = 3;
    if (records[0].rank == 0x24) {
        desired = 4;
    } else {
        for (i = 1; (desired > 0) && (i != 6);) {
            if (records[0].rank < records[i++].rank) {
                desired--;
            }
        }
    }

    if (gOverlay58PackedMode == 0) {
        shift = 0;
        mask = 0x7;
    } else if (gOverlay58PackedMode == 1) {
        shift = 3;
        mask = 0x38;
    } else if (gOverlay58ExtendedPackedMode == 0) {
        shift = 6;
        mask = 0x1C0;
    } else {
        shift = 9;
        mask = 0xE00;
    }

    current = (gOverlay58PackedStatus[records[0].player + 4] & mask) >> shift;
    if (desired < 3) {
        return;
    }
    if (current >= desired) {
        return;
    }

    if (gOverlay58PackedMode == 0) {
        if (current >= 3) {
            return;
        }
        i = 0;
        count = 0;
        for (; i < 3; i++) {
            self = records[0].player;
            if ((i != self) && ((gOverlay58PackedStatus[i + 4] & 0x7) >= 3)) {
                count++;
            }
        }
        if (count == 0) {
            if (desired == 4) {
                func_8003A680(0x17);
            } else {
                func_8003A680(0x16);
            }
        } else if (count == 2) {
            if ((self >= 0) && (self < 3)) {
                func_8003A680(0x0B);
                gOverlay58PackedStatus[10] |= 0x10;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if (gOverlay58PackedMode == 1) {
        if (current >= 3) {
            return;
        }
        i = 0;
        count = 0;
        for (; i < 3; i++) {
            self = records[0].player;
            if ((i != self) &&
                (((gOverlay58PackedStatus[i + 4] & 0x38) >> 3) >= 3)) {
                count++;
            }
        }
        if (count == 0) {
            if (desired == 4) {
                func_8003A680(0x17);
            } else {
                func_8003A680(0x16);
            }
        } else if (count == 2) {
            if ((self >= 0) && (self < 3)) {
                func_8003A680(0x0C);
                gOverlay58PackedStatus[10] |= 0x04;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if (gOverlay58ExtendedPackedMode == 0) {
        i = 0;
        count = 0;
        fourCount = 0;
        for (; i < 5; i++) {
            self = records[0].player;
            if (i != self) {
                mask = (gOverlay58PackedStatus[i + 4] & 0x1C0) >> 6;
                if (mask >= 3) {
                    count++;
                }
                if (mask == 4) {
                    fourCount++;
                }
            }
        }

        if ((fourCount == 4) && (desired == 4)) {
            func_8003A680(0x0D);
        }

        if (current < 3) {
            func_8003A680(count + 1);
            if ((count + 1) == 5) {
                func_8003A680(0x18);
                if ((gOverlay58PackedStatus[10] & 0x40) == 0) {
                    gOverlay58PackedStatus[10] |= 0x40;
                    func_800291B4();
                    func_8003A680(0x15);
                }
            }
        }

        if (records[0].player == 3) {
            if ((gOverlay58PackedStatus[10] & 0x100) == 0) {
                gOverlay58PackedStatus[10] |= 0x100;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if ((desired != 4) || (current == 4)) {
        return;
    }

    switch (records[0].player) {
        case 0:
            gOverlay58PackedStatus[10] |= 0x01;
            func_800291B4();
            break;
        case 1:
            gOverlay58PackedStatus[10] |= 0x02;
            func_800291B4();
            break;
        case 2:
            gOverlay58PackedStatus[10] |= 0x08;
            func_800291B4();
            break;
        case 3:
            gOverlay58PackedStatus[10] |= 0x20;
            func_800291B4();
            break;
        default:
            gOverlay58PackedStatus[10] |= 0x80;
            func_800291B4();
            break;
    }
    func_8003A680(0x15);
}
