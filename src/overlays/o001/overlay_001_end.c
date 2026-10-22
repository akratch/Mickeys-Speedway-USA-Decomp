#include "overlays/overlay_001.h"

/* ---- overlay1ResolvePathPoint ---- */


typedef struct Overlay1PoolRecord {
    s16 x[32];
    s16 y[32];
    u8 selector[32];
    u8 mode[32];
    u8 count;
    u8 type;
    u8 padC2[6];
    u32 value;
} Overlay1PoolRecord;

typedef struct Overlay1ErrorOwner {
    u8 pad0;
    s8 sound;
} Overlay1ErrorOwner;

extern Overlay1PoolRecord D_220[32];
extern u8 D_8[0x200];
extern Overlay1PoolRecord *D_218;
extern s32 D_1D88;
extern s32 D_1D84;
extern void *D_1BA4;

extern void _bzero(void *address, s32 size);
extern Overlay1PoolRecord *func_overlay_001_F0007BDC_1853FBC(
    s16 x0, s16 y0, s16 x1, s16 y1);
extern void overlay3TouchObject(s32 sound);
extern s32 func_overlay_002_F0001A94_185888C(f32 x0, f32 y0, f32 x1, f32 y1,
                                void *world, s16 *result, s32 sentinel,
                                u32 mask);
extern void func_overlay_001_F0007730_1853B10(s16 *x, s16 *y,
                                               u8 selector, u8 mode);

/* PROVENANCE: Mickey-derived; pinned DKR v77/v80 and JFG scans found no donor.
 * Matched 2026-09-10.  Three edits carried this body from 8 differing words
 * to zero, and each one is a placement, not a rewrite:
 *   - the two globals are spelled directly (`D_1D88 = 0x3F; D_1D88--;`)
 *     rather than through an address carrier, which lets the twice-spelled
 *     global interleave its own `%hi` between the `lui $at` and the `sw`;
 *   - the six locals are declared in the order below because that is the one
 *     that leaves `result` fifth and so at frame offset 0x60;
 *   - the induction pointer is re-formed inside the scan loop, so the
 *     loop-entry pointer is drawn from the saved copy of `record`.
 * The last word, and the reason `point` is assigned once after the if/else
 * rather than in each arm, is explained where it is written. */
s32 overlay1ResolvePathPoint(s16 x0, s16 y0, s16 x1, s16 y1,
                             s16 *outX, s16 *outY, void *unused) {
    s32 index;
    Overlay1PoolRecord *record;
    s16 *point;
    s32 scanIndex;
    s16 result[4];
    s32 product;

    _bzero(D_220, sizeof(D_220));
    _bzero(D_8, sizeof(D_8));
    D_218 = D_220;
    D_1D88 = 0x3F;
    D_1D84 = 0;

    record = func_overlay_001_F0007BDC_1853FBC(x0, y0, x1, y1);
    if (record == 0) {
        overlay3TouchObject(((Overlay1ErrorOwner *)D_1DA0)->sound);
        *outX = x0;
        *outY = y0;
        return -1;
    }

    D_1D88--;
    index = 2;
    if (record->count >= 2) {
        scanIndex = 2;
        if (record->count >= 3) {
            do {
                point = &record->x[scanIndex];
                if (func_overlay_002_F0001A94_185888C((f32)x0, (f32)y0,
                                         (f32)point[0], (f32)point[32],
                                         D_1BA4, result, -1, 0xFFFF) != 0) {
                    /* Spelling the product twice lets uopt fold the pair into
                     * one multiply while ugen still computes the product into
                     * its first statement register and the sum into its
                     * second, which is the `addu $v1, $v0, $v0` the target
                     * shows.  `product = product + product` writes back in
                     * place on $v0; `product * 2`, `product << 1`, an inline
                     * `(product + product)` and a seventh local all demote the
                     * doubling to a ugen ring temp. */
                    product = ((result[0] - point[0]) *
                               (result[0] - point[32])) +
                              ((result[0] - point[0]) *
                               (result[0] - point[32]));
                    if ((u32)product >= 0x11U) {
                        break;
                    }
                }
                scanIndex++;
            } while (scanIndex < record->count);
        }
    } else {
        scanIndex = index;
    }
    /* One assignment after the if/else, not one per arm.  An assignment
     * before the `if` shares the block with the call, so uopt forwards the
     * call's own return register into it and the base reads $v0; written
     * here it belongs to the join block, where the raw return web has ended
     * and the base is the saved copy $s4 the target uses. */
    point = &record->x[scanIndex];

    if (record->selector[scanIndex - 1] != 0xFF) {
        func_overlay_001_F0007730_1853B10(
            point - 1, point + 31, record->selector[scanIndex - 1],
            record->mode[scanIndex - 1]);
    }
    *outX = point[-1];
    *outY = point[31];
    return record->count;
}

/* ---- overlay1ModeChecks ---- */


/* Generic scalar predicates only; pinned DKR objects provide no donor. */
extern s32 gOverlay1Mode;

s32 overlay1ModeIsOne(void) { return gOverlay1Mode == 1; }
s32 overlay1ModeIsTwo(void) { return gOverlay1Mode == 2; }
s32 overlay1ModeIsThree(void) { return gOverlay1Mode == 3; }

/* ---- overlay1DistanceFromCurrent ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern f32 overlay1SignedOffset(void *first, void *second);

f32 overlay1DistanceFromCurrent(void *other) {
    void *current;

    current = gOverlay1DistanceObject;
    if (current != NULL) {
        return overlay1SignedOffset(current, other);
    }
    return 0.0f;
}

/* ---- overlay1DistanceFromSelected ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern u8 gOverlay1SelectedIndex;
extern void **func_80005750(s32 *count);

f32 overlay1DistanceFromSelected(void *object) {
    s32 count;
    void **objects;
    volatile s32 reservation[2];

    if (object == gOverlay1DistanceObject) {
        objects = func_80005750(&count);
        if (count >= 2) {
            return overlay1SignedOffset(object, objects[gOverlay1SelectedIndex]);
        }
    }
    return 0.0f;
}

