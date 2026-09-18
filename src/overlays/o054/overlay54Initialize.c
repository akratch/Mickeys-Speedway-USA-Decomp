#include "PR/ultratypes.h"

typedef struct O54CopyRecord {
    u8 pad00[8];
    s32 field08;
    s16 pad0C;
    s16 field0E;
    u8 pad10[8];
    s32 field18;
    s16 field1C;
    s16 field1E;
    u8 pad20[8];
    s32 field28;
    s16 field2C;
    s16 field2E;
    u8 pad30[8];
    s32 field38;
    s16 field3C;
    s16 field3E;
} O54CopyRecord;

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
extern s32 gOverlay54Data08, gOverlay54Data1E0;
extern s16 gOverlay54Data0C, gOverlay54Data0E, gOverlay54Data1E4, gOverlay54Data1E6;
extern O54CopyRecord gOverlay54CopySource[], gOverlay54CopyDest[], gOverlay54CopyEnd[];
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
/* Workbench p7 plus L90 reopen: 244/243 instructions, 0x78 frame, 127 masked/144 raw, first +0xF0.
 * Extra word is the nop delay of the context-equals-3 branch. L90 rewrites i++ then i<4 to !=4;
 * target keeps slti 4 after increment. Delay fill is reachable only on inverted polarity. */
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
    O54CopyRecord *src;
    O54CopyRecord *dst;
    O54State *state;
    void *object;
    s16 copy0C, copy0E, copy1C, copy3E, copy1E, copy2C, copy2E, copy3C;
    s32 copy38, copy18, copy28, copy08;
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

    gOverlay54Data08 = gOverlay54Data1E0;
    *(s16 *)(gOverlay54Data + 0x0E) = gOverlay54Data1E6;
    *(s16 *)(gOverlay54Data + 0x0C) = gOverlay54Data1E4;
    src = gOverlay54CopySource;
    dst = gOverlay54CopyDest;
    do {
        copy0C = src->pad0C;
        copy0E = src->field0E;
        copy1C = src->field1C;
        dst->pad0C = copy0C;
        dst->field0E = copy0E;
        copy38 = src->field38;
        copy3E = src->field3E;
        copy1E = src->field1E;
        copy18 = src->field18;
        copy2C = src->field2C;
        copy2E = src->field2E;
        copy28 = src->field28;
        copy3C = src->field3C;
        copy08 = src->field08;
        src++;
        dst++;
        dst[-1].field1C = copy1C;
        dst[-1].field38 = copy38;
        dst[-1].field3E = copy3E;
        dst[-1].field1E = copy1E;
        dst[-1].field18 = copy18;
        dst[-1].field2C = copy2C;
        dst[-1].field2E = copy2E;
        dst[-1].field28 = copy28;
        dst[-1].field3C = copy3C;
        dst[-1].field08 = copy08;
    } while (src != gOverlay54CopyEnd);

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
 * score: 127 differing words
 * frame: 0x78
 * relocations: 112
 * first-mismatch: +0xF0
 * summary: One word long. Extra instruction is the unfilled delay of the context-equals-3 branch. L90 rewrites increment-then-i-lt-4 to not-equal-4; the target keeps slti 4 after increment. Delay fill is reachable only on inverted polarity.
 * PLATEAU-HANDOFF:func_overlay_054_F0000000_189ECA0:end
 */
