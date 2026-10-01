#include "PR/ultratypes.h"
#include "overlays/offset_records.h"

typedef struct O54State {
    s32 field00;
    s32 field04;
    s32 field08;
    u8 pad0C[4];
    s32 field10;
} O54State;

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
 * 4-aligns scalars, which is why the 4-byte groups below are scalars. The
 * 4-byte flag array is the measured exception: it stays 4-aligned. */
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
static s8 sOverlay54Flags[4];
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
/* Matched 2026-10-01 by writing both loops as plain subscript loops over one
 * counter.  The 17-word plateau walked eight declared pointers, kept the
 * ninth and the context in a volatile struct, and stored through [-1] after
 * hand-placed increments; as1 then hoisted the flag and value stores because
 * a declared pointer carries no noalias fact.  With `array[i]` at every site
 * uopt creates all nine induction pointers itself, emits their noalias facts,
 * and spills the ninth to its own temporary, which is the target's schedule.
 * What else it took, each measured:
 *   - the flag bytes are a signed char array (the stored constant is -1);
 *   - `i` is the only counter: reusing it for the record copy keeps the first
 *     loop's exit test a set-less-than against 4 (L90 otherwise rewrites it),
 *     which retires the xor-with-zero;
 *   - two declared locals, `i` then `context`, land the frame: no state
 *     pointer and no object local, the stores name the statics directly. */
void func_overlay_054_F0000000_189ECA0(void) {
    s32 i;
    u8 *context;

    context = o54GetContext();
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

    for (i = 0; i < 4; i++) {
        overlay54CopyOffsetRecords(sOverlay54ListA, sOverlay54ListCopyA[i], i, 0);
        overlay54CopyOffsetRecords(sOverlay54ListB, sOverlay54ListCopyB[i], i, 0);
        overlay54CopyOffsetRecords(sOverlay54ListC, sOverlay54ListCopyC[i], i, 1);
        overlay54CopyOffsetRecords(sOverlay54ListD, sOverlay54ListCopyD[i], i, 2);
        overlay54CopyOffsetRecords(sOverlay54ListE, sOverlay54ListCopyE[i], i, 3);
        overlay54CopyOffsetRecords(sOverlay54ListF, sOverlay54ListCopyF[i], i, 3);
        sOverlay54Flags[i] = -1;
        sOverlay54Values[i] = -0x500;
        sOverlay54Sentinels[i] = -0x140;
    }

    for (i = 0; i < 9; i++) {
        sOverlay54Records[i].x = sOverlay54SourceRecords[i].x;
        sOverlay54Records[i].y = sOverlay54SourceRecords[i].y;
        sOverlay54Records[i].metadata = sOverlay54SourceRecords[i].metadata;
    }

    sOverlay54Height = -80.0f;
    o54LoadResource();
    gOverlay54Data00 = o54QueryValue();
    *(s16 *)(gOverlay54ExternalObject + 0x26) = 0x28;
    *(f32 *)(gOverlay54ExternalObject + 0x28) = 1.0f;
    sOverlay54State.field08 = 0;
    sOverlay54State.field00 = gOverlay54ExternalWord;
    sOverlay54State.field04 = 0;
    sOverlay54State.field10 = 0;

    sOverlay54Bounds[0] = -0x420;
    sOverlay54Bounds[1] = 0x4E0;
    sOverlay54Bounds[2] = -0x420;
    sOverlay54Bounds[3] = 0x4E0;
    if (*context == 3) {
        o54SetupBounds(3);
        sOverlay54Current = o54Allocate(o54GetObjectId(o54CreateObject()), 0xA0, 0x78, 0xC);
        o54Configure(sOverlay54Current, 0);
    } else {
        sOverlay54Current = 0;
    }
    sOverlay54Tail66E = 0;
    sOverlay54Tail66C = 0;
}
