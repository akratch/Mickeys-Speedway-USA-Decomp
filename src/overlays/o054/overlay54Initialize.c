#include "PR/ultratypes.h"
#include "overlays/offset_records.h"

typedef struct O54State {
    s32 field00;
    s32 field04;
    s32 field08;
    u8 pad0C[4];
    s32 field10;
} O54State;

typedef struct O54Locals {
    s16 *sentinel;
    u8 reserved04[0x28];
    union {
        u8 *context;
        u8 storage[8];
    } tail;
} O54Locals;

extern u8 gOverlay54ExternalResource[];
extern u8 gOverlay54ExternalObject[];
extern s32 gOverlay54ExternalWord;

/* Overlay 54's initialized data, in ROM order: this TU is its byte owner.
 * The original overlay was one translation unit -- its runtime relocation
 * table addresses .data, a 0x10-byte .rodata and .bss through three separate
 * section bases -- and this function's record copy is only scheduled the way
 * the ROM has it when the compiler can see both record arrays defined here.
 * Every object is static so that no other TU's extern placeholder resolves to
 * it; the siblings keep their absolute section-relative names. */
static s16 sOverlay54ResourceIds[18] = {
    2, 38, 39, 25, 26, 20, 21, 30, 31, 22, 40, 23, 24, 53, 80, 100, -1, 0,
};
static s16 sOverlay54PrepareIds[4] = { 4, 2, 3, -1 };
static OverlayOffsetRecord sOverlay54ListA[2] = {
    { 38, 39, 0, 0, 0 },
};
static OverlayOffsetRecord sOverlay54ListB[2] = {
    { 38, 39, 0x00060000, 0, 0 },
};
static OverlayOffsetRecord sOverlay54ListC[3] = {
    { 20, 21, 0, 21, 0 },
    { 20, 21, 0, 28, 0 },
};
static OverlayOffsetRecord sOverlay54ListD[3] = {
    { 30, 31, 0, 0, -3 },
    { 20, 21, 0, 32, 1 },
};
static OverlayOffsetRecord sOverlay54ListE[10] = {
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
    { 20, 21, 0x000B0000, 14, 0 },
    { 20, 21, 0, 20, 0 },
    { 20, 21, 0, 27, 0 },
    { 20, 21, 0x000A0000, 33, 0 },
    { 20, 21, 0, 40, 0 },
    { 20, 21, 0, 47, 0 },
    { 23, 24, 0, -25, -8 },
};
static OverlayOffsetRecord sOverlay54ListF[2] = {
    { 25, 0, 0, -25, -6 },
};
static s16 sOverlay54Limits[6] = { 0x300, 0xC00, -0x420, 0x4E0, 0xC80, 0x1580 };
static s16 sOverlay54Offsets[32] = {
    23, 24, 281, 132, 48, 33, 235, 141,
    91, 32, 185, 140, 76, 33, 215, 141,
    23, 12, 281, 132, 48, 25, 235, 145,
    91, 24, 185, 144, 76, 25, 215, 145,
};
static OverlayOffsetRecord sOverlay54SourceRecords[10] = {
    { 20, 21, 0, -7, 0 },
    { 20, 21, 0, 0, 0 },
    { 20, 21, 0, 7, 0 },
    { 20, 21, 0x000B0000, 14, 0 },
    { 20, 21, 0, 20, 0 },
    { 20, 21, 0, 27, 0 },
    { 20, 21, 0x000A0000, 33, 0 },
    { 20, 21, 0, 40, 0 },
    { 20, 21, 0, 47, 0 },
};
static OverlayOffsetRecord sOverlay54ListG[2] = {
    { 20, 21, 0, -3, -4 },
};
static s32 sOverlay54Tail298[4] = { 0 };
static s32 sOverlay54Tail2A8 = 9;
static s32 sOverlay54Tail2AC = 0;
static s8 sOverlay54Tail2B0[4] = { 0 };
static s32 sOverlay54Tail2B4[7] = { 0 };

/* Overlay 54's .bss, in address order. IDO 8-aligns .bss arrays and
 * 4-aligns scalars, which is why the 4-byte groups below are scalars. */
static OverlayOffsetRecord sOverlay54Records[10];
static O54State sOverlay54State;
static s32 sOverlay54BssPadB4;
static s32 sOverlay54BssPadB8;
static s32 sOverlay54BssPadBC;
static OverlayOffsetRecord sOverlay54ListCopyA[4][2];
static OverlayOffsetRecord sOverlay54ListCopyB[4][2];
static OverlayOffsetRecord sOverlay54ListCopyC[4][3];
static OverlayOffsetRecord sOverlay54ListCopyD[4][3];
static OverlayOffsetRecord sOverlay54ListCopyE[4][10];
static OverlayOffsetRecord sOverlay54ListCopyF[4][2];
static s16 sOverlay54Values[4];
static s16 sOverlay54Sentinels[4];
static s32 sOverlay54Mode;
static s32 sOverlay54Flags;
static f32 sOverlay54Height;
static s32 sOverlay54BssPad65C;
static s16 sOverlay54Bounds[4];
static void *sOverlay54Current;
static s16 sOverlay54Tail66C;
static s16 sOverlay54Tail66E;

extern s16 gOverlay54Data00;

extern u8 *o54GetContext(void);
extern void o54LoadResource();
extern void o54PrepareResource(void *resource);
extern void o54SetMode(s32 mode);
extern void o54CommitMode(s32 mode);
extern void o54SetupBounds(s32 arg0);
extern s16 o54QueryValue(void);
extern s32 o54CreateObject(void);
extern s32 o54GetObjectId(s32 object);
extern void *o54Allocate(s32 id, s32 width, s32 height, s32 format);
extern void o54Configure(void *object, s32 mode);

extern void overlay54PatchIndices(void *entry);
extern void overlay54CopyOffsetRecords(void *src, void *dst, s32 mode, s32 index);


/* Independently reconstructed from Mickey-local evidence; no DKR/JFG donor. */
/* 2026-09-23, lane B2-ov2: the record copy is a nine-iteration subscript
 * loop; IDO unrolls it by four with the remainder first.
 * 2026-09-23, lane B3-o054, 125 (+4) -> 17 (delta 0, frame exact, naming 0):
 * - the TU owns overlay 54's data and .bss (above), so as1 can prove the two
 *   record arrays disjoint and the remainder loads precede its stores;
 * - the loop-A pointer setup is one physical line (L59: the target sets the
 *   pointers in reverse);
 * - the tail stores follow the target's order;
 * - o54Configure reads sOverlay54Current back, which keeps its address a
 *   register web across both arms: the missing branch-delay word;
 * - the exit test's xor-with-zero keeps slti after the increment (L90).
 * The rest is loop A's tail: as1 hoists the flag and value stores above
 * their pointer increments, which the target does not. See the handoff. */
#ifdef NON_MATCHING
void func_overlay_054_F0000000_189ECA0(void) {
    volatile O54Locals locals;
    s16 *value;
    u8 *flag;
    u8 *src0;
    u8 *src1;
    u8 *src2;
    u8 *src3;
    u8 *src4;
    u8 *src5;
    s32 i;
    s32 j;
    O54State *state;
    void *object;
    s16 *nextSentinel;
    s32 loopFlag, storeFlag, storeValue, storeSentinel;

    locals.tail.context = o54GetContext();
    o54LoadResource(sOverlay54ResourceIds);
    o54LoadResource(gOverlay54ExternalResource);
    o54PrepareResource(sOverlay54PrepareIds);
    o54SetMode(4);
    sOverlay54Mode = 0x104;
    o54CommitMode(0xB);
    overlay54PatchIndices(sOverlay54ListA);
    overlay54PatchIndices(sOverlay54ListB);
    overlay54PatchIndices(sOverlay54ListC);
    overlay54PatchIndices(sOverlay54ListD);
    overlay54PatchIndices(sOverlay54ListE);
    overlay54PatchIndices(sOverlay54ListF);
    overlay54PatchIndices(sOverlay54ListG);

    locals.sentinel = sOverlay54Sentinels; src0 = (u8 *)sOverlay54ListCopyA; src1 = (u8 *)sOverlay54ListCopyB; src2 = (u8 *)sOverlay54ListCopyC; src3 = (u8 *)sOverlay54ListCopyD; src4 = (u8 *)sOverlay54ListCopyE; src5 = (u8 *)sOverlay54ListCopyF; flag = (u8 *)&sOverlay54Flags; value = sOverlay54Values;
    i = 0;
    do {
        overlay54CopyOffsetRecords(sOverlay54ListA, src0, i, 0);
        overlay54CopyOffsetRecords(sOverlay54ListB, src1, i, 0);
        overlay54CopyOffsetRecords(sOverlay54ListC, src2, i, 1);
        overlay54CopyOffsetRecords(sOverlay54ListD, src3, i, 2);
        overlay54CopyOffsetRecords(sOverlay54ListE, src4, i, 3);
        overlay54CopyOffsetRecords(sOverlay54ListF, src5, i, 3);
        storeFlag = -1;
        storeValue = -0x500;
        storeSentinel = -0x140;
        nextSentinel = locals.sentinel + 1;
        locals.sentinel = nextSentinel;
        src0 += 0x20;
        src1 += 0x20;
        src2 += 0x30;
        src3 += 0x30;
        src4 += 0xA0;
        src5 += 0x20;
        flag++;
        value++;
        flag[-1] = storeFlag;
        value[-1] = storeValue;
        nextSentinel[-1] = storeSentinel;
    } while ((++i ^ 0) < 4);

    for (j = 0; j < 9; j++) {
        sOverlay54Records[j].x = sOverlay54SourceRecords[j].x;
        sOverlay54Records[j].y = sOverlay54SourceRecords[j].y;
        sOverlay54Records[j].metadata = sOverlay54SourceRecords[j].metadata;
    }

    sOverlay54Height = -80.0f;
    o54LoadResource();
    gOverlay54Data00 = o54QueryValue();
    state = &sOverlay54State;
    *(s16 *)(gOverlay54ExternalObject + 0x26) = 0x28;
    *(f32 *)(gOverlay54ExternalObject + 0x28) = 1.0f;
    state->field08 = 0;
    state->field00 = gOverlay54ExternalWord;
    state->field04 = 0;
    state->field10 = 0;

    sOverlay54Bounds[0] = -0x420;
    sOverlay54Bounds[1] = 0x4E0;
    sOverlay54Bounds[2] = -0x420;
    sOverlay54Bounds[3] = 0x4E0;
    if (*locals.tail.context == 3) {
        o54SetupBounds(3);
        object = o54Allocate(o54GetObjectId(o54CreateObject()), 0xA0, 0x78, 0xC);
        sOverlay54Current = object;
        o54Configure(sOverlay54Current, 0);
    } else {
        sOverlay54Current = 0;
    }
    sOverlay54Tail66E = 0;
    sOverlay54Tail66C = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o054/overlay54Initialize/func_overlay_054_F0000000_189ECA0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_054_F0000000_189ECA0:start
 * symbol: func_overlay_054_F0000000_189ECA0
 * score: 17 differing words
 * frame: 0x78
 * relocations: 114
 * first-mismatch: +0x1AC
 * summary: TU owns overlay 54 data; delta 0, frame exact; left: as1 hoists loop-A flag/value stores above their increments (no noalias for declared pointers).
 * PLATEAU-HANDOFF:func_overlay_054_F0000000_189ECA0:end
 */
