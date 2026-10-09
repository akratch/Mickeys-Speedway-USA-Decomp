/*
 * Display-list trace buffer -- ROM 0x45760-0x459C0 (VRAM 0x80044B60).
 *
 * PROVENANCE: the translation-unit identity and function identities are adapted
 * from Jet Force Gemini's public decompilation, src/diRcpTrace.c. Mickey's
 * own call graph, placement immediately before diRcp.c, and the four-function
 * order establish the correspondence; JFG is a permitted published decomp
 * under docs/CLEANROOM.md. Externally referenced functions retain their
 * Mickey address labels until their callers are decompiled. The bodies remain
 * Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"

typedef struct Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
} Gfx;

typedef struct DiRcpTraceEntry {
    Gfx *value0;
    char *value4;
    s32 value8;
} DiRcpTraceEntry;

extern s32 D_8007CFC8;
extern s32 D_800D4A90[];
extern s32 D_8007CFC0;
extern s32 D_8007CFC4;
extern s32 mmAlloc(s32 size, s32 tag);

#define DI_RCP_TRACE_BUFFERS ((DiRcpTraceEntry **)&D_8007CFC0)

void diRcpTraceInit(void) {
    D_8007CFC0 = mmAlloc(0x4B0, 0x8F);
    D_8007CFC4 = mmAlloc(0x4B0, 0x8F);
}
void func_80044B9C(void) {
    D_8007CFC8 = 1 - D_8007CFC8;
    D_800D4A90[D_8007CFC8] = 0;
}
/* Mickey-derived body; JFG's diRcpTrace implementation is assembly-only. */
void func_80044BC8(Gfx *value0, char *value4, s32 value8) {
    if (DI_RCP_TRACE_BUFFERS[D_8007CFC8] != 0) {
        if (D_800D4A90[D_8007CFC8] < 100) {
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value0 =
                value0;
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value4 =
                value4;
            DI_RCP_TRACE_BUFFERS[D_8007CFC8][D_800D4A90[D_8007CFC8]].value8 =
                value8;
            D_800D4A90[D_8007CFC8]++;
        }
    }
}
/*
 * PROVENANCE: loop and entry-selection spelling adapted from JFG's public
 * src/diRcpTrace.c:diRcpTraceGetInfo after that source was matched upstream
 * in commit 2f49ab3. Mickey's buffer sizes, symbols, ABI, and target bytes
 * remain authoritative. The resulting 0x12C-byte function and its six
 * relocation pairs are byte-identical to Mickey's ROM.
 */
void func_80044C94(Gfx *value, char **lowerValue4, s32 *lowerValue8,
                   Gfx **lowerValue0, char **upperValue4, s32 *upperValue8,
                   Gfx **upperValue0) {
    s32 i;
    s32 buffer;
    Gfx *entryValue;
    Gfx *lower;
    Gfx *upper;
    DiRcpTraceEntry *lowerEntry;
    DiRcpTraceEntry *upperEntry;

    buffer = 1 - D_8007CFC8;
    lower = 0;
    upper = (Gfx *)-1;
    lowerEntry = NULL;
    upperEntry = NULL;

    for (i = 0; i < D_800D4A90[buffer]; i++) {
        entryValue = DI_RCP_TRACE_BUFFERS[buffer][i].value0;
        if (value >= entryValue && lower < entryValue) {
            lower = entryValue;
            lowerEntry = DI_RCP_TRACE_BUFFERS[buffer] + i;
        }
        if (value < entryValue && entryValue < upper) {
            upper = entryValue;
            upperEntry = DI_RCP_TRACE_BUFFERS[buffer] + i;
        }
    }
    if (lowerEntry != NULL) {
        *lowerValue4 = lowerEntry->value4;
        *lowerValue8 = lowerEntry->value8;
        *lowerValue0 = lowerEntry->value0;
    } else {
        *lowerValue4 = 0;
    }
    if (upperEntry != NULL) {
        *upperValue4 = upperEntry->value4;
        *upperValue8 = upperEntry->value8;
        *upperValue0 = upperEntry->value0;
    } else {
        *upperValue4 = 0;
    }
}
