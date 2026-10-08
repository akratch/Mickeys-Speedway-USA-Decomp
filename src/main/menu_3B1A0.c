#include "PR/ultratypes.h"

extern u8 D_8007BF3C[];
extern s32 D_8007BF44;
extern void *func_80028F54(void);

typedef struct Menu3B1A0Record {
    u8 byte00;
    u8 byte01;
    u8 byte02;
    u8 byte03;
    u8 bytes04[3];
    u8 byte07;
    u8 bytes08[0x17];
    u8 byte1F;
    u8 bytes20[6];
    u16 half26;
} Menu3B1A0Record;

typedef struct Menu3B1A0ByteRecord {
    u8 bytes00[0x1F];
    u8 byte1F;
} Menu3B1A0ByteRecord;

typedef struct Menu3B1A0State {
    u8 bytes000[0x383];
    s8 count;
    u8 bytes384[0x80];
    /* 0x404: `count` (+1 when flag45C is set) s32 entries, bounded above by
     * flag45C at 0x45C -- 0x16 of them.  Tier D, structural: the extent is
     * fixed by the two named neighbours, the element type and base by
     * func_8003A7D0's own accumulation loop. */
    s32 entries[0x16];
    u8 flag45C;
} Menu3B1A0State;

typedef struct Menu3B1A0Object {
    u8 bytes00[0x64];
    Menu3B1A0State *state;
} Menu3B1A0Object;

typedef struct Menu3B1A0KeyGroup {
    s16 key[4];
} Menu3B1A0KeyGroup;

extern s32 *D_8007C0B8;
extern Menu3B1A0KeyGroup D_8007C0E8[];
extern Menu3B1A0KeyGroup D_8007C11C[];
extern u8 D_80082714[];

/* Matched 2026-09-12 (lane p12-tight).  The target walks the tables by INDEX,
 * not with an explicit cursor/end pointer pair, and that one difference is the
 * whole of the nine-word residual the note below described.
 *
 * With `i` as the only induction variable uopt strength-reduces `D_8007C0E8[i]`
 * into the walking pointer, strength-reduces `i * 8` into the byte offset the
 * value reads use, and then linear-function-test replacement rewrites the
 * `i < 6` exit test as a comparison of the walking pointer against
 * `&D_8007C0E8[6]`.  Both the pointer and its bound are therefore compiler
 * temporaries, created in that order, and globalcolor -- which is phase two
 * here, this being a leaf -- colours them in ascending web number: the bound
 * takes v0 and the walker a1, which is the target's assignment.
 *
 * Spelled with a `u8 *end = &D_8007C118;` local instead, the bound is a
 * propagated address constant whose web is created at its first use in the
 * LOOP TAIL.  Address-constant webs are numbered above every symbol web, so it
 * is coloured last and takes a1 while the cursor takes v0, and it is hoisted
 * last into the preheader, which also puts the two `%lo` materializations in
 * the wrong order.  All six declaration orders, all six initialization orders,
 * all four physical-line groupings and both comparison operand orders were
 * measured against that shape and every one is 9 or worse -- the web numbering
 * is not reachable from any of those axes, only from removing the pointer
 * pair.  Forcing the two colours confirmed the price: 9 -> 4, and the last
 * four words were the `%lo` order and the compare's operands.
 *
 * The `for` loop is not unrolled despite this TU using the default unroller:
 * the four interior `return`s make the body multi-exit.
 *
 * No donor counterpart: JFG's src/menu.c has no function of this shape. */
void *func_8003A5A0(s32 key) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (key == D_8007C0E8[i].key[0]) { return (void *) D_8007C0B8[D_8007C11C[i].key[0]]; }
        if (key == D_8007C0E8[i].key[1]) { return (void *) D_8007C0B8[D_8007C11C[i].key[1]]; }
        if (key == D_8007C0E8[i].key[2]) { return (void *) D_8007C0B8[D_8007C11C[i].key[2]]; }
        if (key == D_8007C0E8[i].key[3]) { return (void *) D_8007C0B8[D_8007C11C[i].key[3]]; }
    }
    return D_80082714;
}

void func_8003A680(s32 value) {
    if (D_8007BF44 < 8) {
        D_8007BF3C[D_8007BF44] = value;
        D_8007BF44 += 1;
    }
}
s32 func_8003A6B0(u8 character) {
    s32 code = character;
    s32 codeCopy = code;

    if (code == 0x21) {
        goto return_1B;
    }
    if (code == 0x2A) {
        goto return_1C;
    }
    if (code != 0x3F) {
        goto return_default;
    }
    return 0x1A;

return_1B:
    return 0x1B;
return_1C:
    return 0x1C;
return_default:
    return (codeCopy - 0x41) & 0xFF;
}
s32 func_8003A700(u8 character) {
    s32 code = character;
    s32 codeCopy = code;

    switch (code) {
    case 0x1A:
        return 0x3F;
    case 0x1B:
        return 0x21;
    case 0x1C:
        return 0x2A;
    default:
        return (codeCopy + 0x41) & 0xFF;
    }
}
/* Clears ten 0x28-byte records: each record's half26, byte07 and bytes20[],
 * then the next record's byte00..byte03.  Matched 2026-09-11; three edits
 * closed the last twelve words and each is a measured reconstruction, not
 * recovered source (see docs/ido-learnings.md):
 *  - `zero`: the inner counter starts from a local copy of 0.  uopt's EQ_INEQ
 *    pass rewrites a basic induction variable's `i < 6` into `i != 6` with the
 *    6 hoisted to a register, and it runs BEFORE copy propagation; a
 *    copy-initialised counter is not constant there, so the `slti` survives,
 *    and the later propagation still emits `move v1, zero`.
 *  - the trailing empty `if`: it keeps the call result live to the exit, which
 *    holds v0 through both loops; uopt deletes the folded branch after
 *    liveness, so no instruction is emitted, and every web takes its target
 *    colour.  Without it the inner counter takes v0 and all seven webs shift.
 *  - `offset = 0` written before the pointer copies: the comparison against
 *    `record` in that `if` weighs the walked-pointer web above the offset web,
 *    so the offset is defined first (as1 emits its move first) yet coloured
 *    second.
 * Four explicit induction variables are the target's: uopt merges any two
 * pointers derived from one expression, so the three record pointers and the
 * byte offset are separate variables in the source. */
void func_8003A754(void) {
    u8 *base;
    Menu3B1A0Record *record;
    Menu3B1A0Record *header;
    Menu3B1A0Record *tail;
    u8 *cursor;
    s32 offset;
    s32 i;
    s32 zero;

    base = func_80028F54();
    zero = 0;
    offset = 0;
    record = (Menu3B1A0Record *) base;
    header = (Menu3B1A0Record *) base;
    tail = (Menu3B1A0Record *) base;
    do {
        tail->half26 = 0;
        tail->byte07 = 0;
        i = zero;
        cursor = (u8 *) record;
        do {
            i++;
            cursor++;
            cursor[0x1F] = 0;
        } while (i < 6);
        offset += 0x28;
        record++;
        header++;
        tail++;
        header->byte01 = 0;
        header->byte02 = 0;
        header->byte03 = 0;
        header->byte00 = 0;
    } while (offset < 0x190);
    if ((u8 *) record != base) {
    }
}
/* PROVENANCE: no donor counterpart -- JFG's src/menu.c has no function of
 * this shape, so this is ordinary matching against Mickey's own bytes.
 * The target's 4x-unrolled accumulation loop is the compiler's, not the
 * source's: this TU's -Wo,-loopunroll,0 was removed (it was byte-inert for
 * every function the TU still compiles) and the plain `for` then unrolls to
 * the target's exact 44 words.  The guard and the final test read
 * `state->count` again rather than the local: IDO CSEs all three reads onto
 * one carrier and copies it into the second, which is what puts the -1
 * compare on the original and the increment on the copy (lever 45). */
s32 func_8003A7D0(Menu3B1A0Object *object) {
    Menu3B1A0State *state;
    s32 total;
    s32 limit;
    s32 i;

    state = object->state;
    total = 0;
    limit = state->count;
    if ((state->flag45C != 0) && (state->count != -1)) {
        limit += 1;
    }
    for (i = 0; i < limit; i++) {
        total += state->entries[i];
    }
    if (state->count < 3) {
        total += 0x2BF20;
    }
    return total;
}

