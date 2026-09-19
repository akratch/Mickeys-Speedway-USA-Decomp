#include "PR/ultratypes.h"

typedef struct Overlay58InputRecord {
    u8 state;
    u8 phase;
    u8 player;
    u8 reserved03[0x23];
    u16 rank;
} Overlay58InputRecord;

/* Every Reloc name is provisional; normalized assembly does not bind it. */
extern u8 gOverlay58FinalizerGateReloc;
extern s32 gOverlay58PrimaryOrderIndex; /* overlay-local BSS +0x74 */
extern u8 gOverlay58PackedModeReloc;
extern u8 gOverlay58ExtendedPackedModeReloc;
extern u16 gOverlay58PackedStatusReloc[];

extern Overlay58InputRecord *overlay58Call555CReloc(void);
extern void overlay58Call56E8Reloc(s32 code);
extern void overlay58Call56F8Reloc(s32 code);
extern void overlay58Call5720Reloc(s32 code);
extern void overlay58Call5738Reloc(void);
extern void overlay58Call5740Reloc(s32 code);
extern void overlay58Call57B4Reloc(s32 code);
extern void overlay58Call57C4Reloc(s32 code);
extern void overlay58Call57ECReloc(s32 code);
extern void overlay58Call5804Reloc(void);
extern void overlay58Call580CReloc(s32 code);
extern void overlay58Call5894Reloc(s32 code);
extern void overlay58Call58BCReloc(s32 count);
extern void overlay58Call58DCReloc(s32 code);
extern void overlay58Call5900Reloc(void);
extern void overlay58Call5908Reloc(s32 code);
extern void overlay58Call593CReloc(void);
extern void overlay58Call5944Reloc(s32 code);
extern void overlay58Call5994Reloc(void);
extern void overlay58Call59ACReloc(void);
extern void overlay58Call59C4Reloc(void);
extern void overlay58Call59DCReloc(void);
extern void overlay58Call59F4Reloc(void);
extern void overlay58Call59FCReloc(s32 code);

/* Exact semantic body for executable range +0x5554..+0x5A14. */
/*
 * Plateau (2026-09-10): 104 of 304 relocation-masked words differ, down from
 * 177, at unchanged 304-word geometry and an unchanged 0x48 frame.  The three
 * `if (... != 0);` statements below are discarded-expression probes (ido-5.3
 * L37): they cost zero instructions, and each adds one web occurrence that
 * re-orders p1's colouring.  Their positions came from a frame-constrained
 * hill climb over insert/delete/move moves; each of the three is a strict
 * gain on the base the two before it produce, and none is a gain alone in a
 * different order.  The same construct is what moved this overlay's two
 * point-quad draw routines from 70 to 26.
 *
 * 2026-09-16 (lane s1-a), 88 to 78 at delta 0: `shift` is defined before
 * `mask` in every arm (first-definition order gives shift v1 and mask a0,
 * L106), the extended test is `== 0` with its else arm hoisted above the
 * branch as the ROM lays it out, and every mode section defines `i`, then
 * `count`, then its selected player.  A clean form with no probes and the
 * selected player re-read from `records[0].player` reproduces the ROM's
 * mode-0 loop shape exactly (the copy `or a3,t0,zero`, the compare before
 * the address) but colours the packed-status base t0 where the ROM has t3
 * (44 rows), which the probes are pinning; see the shard.
 *
 * 2026-09-18 (lane w14-o058), 78 to 77 at delta 0: `mask` and `shift` are
 * `u16`.  That pair occupies the target's +0x1C home (candidate +0x18 is
 * gone).  s32/u32/unsigned are 78; s16 shift grows the function; other
 * 16-bit locals regress.  See the shard.
 *
 * 2026-09-19 (lane w28-o058f), 77 to 75 at delta 0: mode-0 `i = 0` moves
 * into the `for` initializer so that node is not initially ready, and
 * each mask arm comma-assigns `shift` in the same statement (overlay40
 * delay).  Empty `if (i) {}`, leftover OR-zero, loop-local packed
 * carriers, generated subscripts, and the other initially-ready delays
 * are inert or worse on this shape.  See the shard.
 */
#ifdef NON_MATCHING
void overlay58FinalizePackedStatus(void) {
    Overlay58InputRecord *records;
    s32 desired;
    s32 current;
    s32 mode;
    s32 player;
    s32 selectedPlayer0;
    s32 selectedPlayer1;
    s32 selectedPlayer2;
    u16 mask;
    u16 shift;
    s32 i;
    s32 count;
    s32 equalFourCount;
    s32 decoded;
    u16 flags;


    records = overlay58Call555CReloc();

    if (gOverlay58FinalizerGateReloc != 0) {
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
        for (i = 1; (desired > 0) && (i != 6); i++) {
            if (selectedPlayer0 != 0);
            if (records[0].rank < records[i].rank) {
                desired--;
            }
        }
    }

    mode = gOverlay58PackedModeReloc;
    if (mode == 0) {
        shift = 0, mask = 0x7;
    } else if (mode == 1) {
        shift = 3, mask = 0x38;
    } else if (gOverlay58ExtendedPackedModeReloc == 0) {
        shift = 6, mask = 0x1C0;
    } else {
        shift = 9, mask = 0xE00;
    }

    player = records[0].player;

    if (desired < 3) {
        return;
    }
    current = (gOverlay58PackedStatusReloc[player + 4] & mask) >> shift;
    if (current >= desired) {
        return;
    }

    if (mode == 0) {
        if (current >= 3) {
            return;
        }

        count = 0;
        selectedPlayer0 = player;
        for (i = 0; i < 3; i++) {
            if (current != 0);
            if ((i != *(volatile u8 *)&records[0].player) &&
                ((gOverlay58PackedStatusReloc[i + 4] & 0x7) >= 3)) {
                count++;
            }
        }

        if (count == 0) {
            if (desired == 4) {
                overlay58Call56E8Reloc(0x17);
            } else {
                overlay58Call56F8Reloc(0x16);
            }
            return;
        }
        if (count != 2) {
            return;
        }
        if ((selectedPlayer0 < 0) || (selectedPlayer0 >= 3)) {
            return;
        }

        overlay58Call5720Reloc(0x0B);
        gOverlay58PackedStatusReloc[10] |= 0x10;
        overlay58Call5738Reloc();
        overlay58Call5740Reloc(0x15);
        return;
    }

    if (mode == 1) {
        if (current >= 3) {
            return;
        }

        i = 0;
        count = 0;
        selectedPlayer1 = *(volatile u8 *)&records[0].player;
        for (; i < 3; i++) {
            if (player != 0);
            if ((i != selectedPlayer1) &&
                (((gOverlay58PackedStatusReloc[i + 4] & 0x38) >> 3) >=
                 3)) {
                count++;
            }
        }

        if (count == 0) {
            if (desired == 4) {
                if (records != 0);
                overlay58Call57B4Reloc(0x17);
            } else {
                overlay58Call57C4Reloc(0x16);
            }
            return;
        }
        if (count != 2) {
            return;
        }
        if ((selectedPlayer1 < 0) || (selectedPlayer1 >= 3)) {
            return;
        }

        overlay58Call57ECReloc(0x0C);
        gOverlay58PackedStatusReloc[10] |= 0x04;
        overlay58Call5804Reloc();
        overlay58Call580CReloc(0x15);
        return;
    }

    if (gOverlay58ExtendedPackedModeReloc == 0) {
        i = 0;
        count = 0;
        selectedPlayer2 = *(volatile u8 *)&records[0].player;
        equalFourCount = 0;
        do {
            if (i != selectedPlayer2) {
                decoded =
                    (gOverlay58PackedStatusReloc[i + 4] & 0x1C0) >> 6;
                if (decoded >= 3) {
                    count++;
                if (player != 0);
                }
                if (decoded == 4) {
                    equalFourCount++;
                }
            }
            i++;
        } while (i != 5);

        if ((equalFourCount == 4) && (desired == 4)) {
            overlay58Call5894Reloc(0x0D);
        }

        if (current < 3) {
            count++;
            overlay58Call58BCReloc(count);
            if (count == 5) {
                overlay58Call58DCReloc(0x18);
                flags = gOverlay58PackedStatusReloc[10];
                if ((flags & 0x40) == 0) {
                    if (records != 0);
                    gOverlay58PackedStatusReloc[10] = flags | 0x40;
                    overlay58Call5900Reloc();
                    overlay58Call5908Reloc(0x15);
                }
            }
        }

        if (records[0].player != 3) {
            return;
        }
        flags = gOverlay58PackedStatusReloc[10];
        if ((flags & 0x100) != 0) {
            return;
        }
        gOverlay58PackedStatusReloc[10] = flags | 0x100;
        overlay58Call593CReloc();
        overlay58Call5944Reloc(0x15);
        return;
    }

    if ((desired != 4) || (current == 4)) {
        return;
    }

    switch (player) {
        case 0:
            gOverlay58PackedStatusReloc[10] |= 0x01;
            overlay58Call5994Reloc();
            break;
        case 1:
            gOverlay58PackedStatusReloc[10] |= 0x02;
            overlay58Call59ACReloc();
            break;
        case 2:
            gOverlay58PackedStatusReloc[10] |= 0x08;
            overlay58Call59C4Reloc();
            break;
        case 3:
            gOverlay58PackedStatusReloc[10] |= 0x20;
            overlay58Call59DCReloc();
            break;
        default:
            gOverlay58PackedStatusReloc[10] |= 0x80;
            overlay58Call59F4Reloc();
            break;
    }
    overlay58Call59FCReloc(0x15);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/overlay58FinalizePackedStatus/func_overlay_058_F0005554_18B473C.s")
#endif

/* PLATEAU-HANDOFF:overlay58FinalizePackedStatus:start
 * symbol: overlay58FinalizePackedStatus
 * score: 75 differing words
 * frame: 0x48
 * relocations: 48
 * first-mismatch: +0x18
 * summary: comma-delay of mode-0 i and shift, 77 to 75 at delta 0; L100/L145/L109 on i and mask/shift inert or worse; fifth home remains
 * PLATEAU-HANDOFF:overlay58FinalizePackedStatus:end
 */
