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

extern u8 gOverlay54Data[];
extern u8 gOverlay54Bss[];
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

/* Overlay 54's .bss begins with the copy's destination list. */
static OverlayOffsetRecord sOverlay54Records[10];
extern f32 gOverlay54Height;
extern s16 gOverlay54Data00;
extern O54State gOverlay54State;
extern s16 gOverlay54Bounds[];
extern void *gOverlay54Current;

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

#define DATA_PTR(off) ((void *)(gOverlay54Data + (off)))
#define BSS_PTR(off) ((void *)(gOverlay54Bss + (off)))

/* Independently reconstructed from Mickey-local evidence; no DKR/JFG donor. */
/* Workbench p7 plus L90 reopen: the extra word and the loop-exit slti are
 * unchanged (see the plateau handoff). 2026-09-23, lane B2-ov2: the record
 * copy is a nine-iteration subscript loop. IDO unrolls it by four with the
 * remainder first, and that reproduces the target's unrolled body word for
 * word (ring draws t4..t3). The remainder is still interleaved: see the
 * handoff on static data. */
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
    o54LoadResource(DATA_PTR(0));
    o54LoadResource(gOverlay54ExternalResource);
    o54PrepareResource(DATA_PTR(0x24));
    o54SetMode(4);
    *(s32 *)(gOverlay54Bss + 0x650) = 0x104;
    o54CommitMode(0xB);
    overlay54PatchIndices(DATA_PTR(0x2C));
    overlay54PatchIndices(DATA_PTR(0x4C));
    overlay54PatchIndices(DATA_PTR(0x6C));
    overlay54PatchIndices(DATA_PTR(0x9C));
    overlay54PatchIndices(DATA_PTR(0xCC));
    overlay54PatchIndices(DATA_PTR(0x16C));
    overlay54PatchIndices(DATA_PTR(0x278));

    locals.sentinel = (s16 *)BSS_PTR(0x648);
    src0 = gOverlay54Data + 0xC0;
    src1 = gOverlay54Data + 0x140;
    src2 = gOverlay54Data + 0x1C0;
    src3 = gOverlay54Data + 0x280;
    src4 = gOverlay54Data + 0x340;
    src5 = gOverlay54Data + 0x5C0;
    flag = gOverlay54Bss + 0x654;
    value = (s16 *)BSS_PTR(0x640);
    i = 0;
    do {
        overlay54CopyOffsetRecords(DATA_PTR(0x2C), src0, i, 0);
        overlay54CopyOffsetRecords(DATA_PTR(0x4C), src1, i, 0);
        overlay54CopyOffsetRecords(DATA_PTR(0x6C), src2, i, 1);
        overlay54CopyOffsetRecords(DATA_PTR(0x9C), src3, i, 2);
        overlay54CopyOffsetRecords(DATA_PTR(0xCC), src4, i, 3);
        overlay54CopyOffsetRecords(DATA_PTR(0x16C), src5, i, 3);
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
    } while (i++ < 3);

    for (j = 0; j < 9; j++) {
        sOverlay54Records[j].x = sOverlay54SourceRecords[j].x;
        sOverlay54Records[j].y = sOverlay54SourceRecords[j].y;
        sOverlay54Records[j].metadata = sOverlay54SourceRecords[j].metadata;
    }

    gOverlay54Height = -80.0f;
    o54LoadResource();
    gOverlay54Data00 = o54QueryValue();
    *(s16 *)(gOverlay54ExternalObject + 0x26) = 0x28;
    *(f32 *)(gOverlay54ExternalObject + 0x28) = 1.0f;
    state = &gOverlay54State;
    state->field08 = 0;
    state->field04 = 0;
    state->field10 = 0;
    gOverlay54Bounds[0] = -0x420;
    gOverlay54Bounds[1] = 0x4E0;
    gOverlay54Bounds[2] = -0x420;
    gOverlay54Bounds[3] = 0x4E0;
    state->field00 = gOverlay54ExternalWord;
    if (*locals.tail.context == 3) {
        o54SetupBounds(3);
        object = o54Allocate(o54GetObjectId(o54CreateObject()), 0xA0, 0x78, 0xC);
        gOverlay54Current = object;
        o54Configure(object, 0);
    } else {
        gOverlay54Current = 0;
    }
    *(s16 *)(gOverlay54Bss + 0x66E) = 0;
    *(s16 *)(gOverlay54Bss + 0x66C) = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o054/overlay54Initialize/func_overlay_054_F0000000_189ECA0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_054_F0000000_189ECA0:start
 * symbol: func_overlay_054_F0000000_189ECA0
 * score: 125 differing words
 * frame: 0x78
 * relocations: 114
 * first-mismatch: +0xF0
 * summary: Record copy is an unrolled 9-loop, body exact; remainder needs as1 to see static data (probe: 89 masked, -4); loop-A slti is L90.
 * PLATEAU-HANDOFF:func_overlay_054_F0000000_189ECA0:end
 */
