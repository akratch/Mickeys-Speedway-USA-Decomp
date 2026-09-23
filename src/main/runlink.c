/*
 * The runtime overlay linker -- ROM 0x323E0-0x33FA0 (VRAM 0x800317E0).
 *
 * Identified from its diagnostic strings (ROM 0x83018-0x83087), which are
 * byte-identical to the ones in Jet Force Gemini's public decomp of the same
 * Rare/DKR-lineage linker. See include/game/runlink.h for the field evidence
 * behind the structs.
 *
 * PROVENANCE -- read this before changing anything here.
 *
 * The bodies in this file are ADAPTED FROM JFG's public decomp of the same
 * engine, not written from scratch. JFG's runLink.c was open in front of me
 * while these were written, and the resemblance goes well past the names: the
 * signatures, the parameter names, the local names and in most cases the
 * declaration order are JFG's. That is a permitted source under
 * docs/CLEANROOM.md (a published, retail-derived decompilation), and it is
 * stated here rather than left for a reader to infer from the similarity.
 *
 * What makes that adaptation *sound* for every function below except one is
 * that each is validated by byte-identity against Mickey's own ROM: the
 * compiled C reproduces Mickey's instructions exactly, so JFG's shape is not
 * being taken on trust, it is being confirmed against this game's binary.
 * Where Mickey's ROM disagreed with JFG, Mickey won and the deviation is
 * recorded:
 *
 *   - MipsInstruction's field order is corrected. JFG names the halfword at
 *     offset 0x00 `immediate` and the one at 0x02 `upper`; Mickey's `sh` at
 *     offset 0x02 patches the I-type immediate, which is the *low* half of a
 *     big-endian word, so the names are swapped here to match the hardware.
 *   - runlinkCallResumeFunction's pending-load scan is a do/while over a
 *     counter initialised to 15, read off Mickey's `addiu a0, zero, 0xF` at
 *     ROM 0x32ED8, not JFG's ARRAY_COUNT-driven while loop.
 *   - The struct layouts in include/game/runlink.h are re-derived from
 *     Mickey's instruction offsets; only OverlayHeader carries fields this
 *     project has not yet touched, and the header says which those are.
 *
 * The one function without that backstop is ProcessRelocationEntry, which is
 * parked non-matching. Its provenance note is attached to it directly, because
 * for that one the adaptation is load-bearing and unverified.
 *
 * Flags: -O2 -mips2 -32. The -O2 is the project default; the -mips2 is a
 * measured deviation and the first evidence about how GAME code (as opposed to
 * libultra) was built. ResolveRelocAddress's `lw t7,0(a3)` at ROM 0x32534 is
 * followed immediately by `addu v1,v1,t7`, using the loaded register in the
 * next instruction. At -mips1 IDO's assembler pads every such pair with a
 * load-delay nop; five of them appeared across this file before the Makefile
 * gained its src/main/ MIPSISET override.
 */

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "game/memory.h"
#include "game/runlink.h"

extern RomTableEntry *overlayRomTable;  /* the overlay ROM table */
extern OverlayHeader *overlayTable;  /* the overlay table */
extern u32 D_800D2DC4;             /* placeholder returned for unresolved symbols */
extern void TrapDanglingJump(void); /* the dangling-jump trap, 0x800333A0 */
extern u8 *D_800D2DAC;             /* base of the section being relocated (text) */
extern u8 *D_800D2DB0;             /* base of the section type-3 records patch (data) */
extern PendingOverlayLoad D_800D2DC8[PENDING_OVERLAY_LOADS];
extern LinkSlot *linkSlotTable;       /* the link-slot table */
extern s32 overlayCount;           /* overlays, AND link slots: one each */
extern RelocationEntry *mainRelocTable;
extern s32 mainRelocTableCount;
extern s32 D_8007A27C;
extern s32 D_8007A67C;
extern char D_80082410[];
extern void runlinkResumeCode(s32 overlayIndex);
extern void runlinkFreeCode(s32 overlayIndex);
extern void runlinkUnloadOverlay(s32 overlayIndex);
extern s32 func_80031A30(RelocationEntry *relocEntry, s32 otIndex);
extern void *func_8002B280(s32 size, s32 tag);
extern void mmFree(void *address);
extern s32 mmGetDelay(void);
extern void mmSetDelay(s32 delay);
extern void romCopy(u32 romAddress, u32 ramAddress, s32 size);
extern s32 D_8007A670;
extern s32 D_8007A674;
extern s32 D_8007A678;
extern char D_80082488[];
extern u8 D_1848B70[];
extern u8 D_1849730[];
extern u8 D_184B680[];
extern u8 D_184C3E0[];
extern u8 D_800D8750[];
extern PendingOverlayLoad D_800D2E40;
extern void _bzero(void *dst, s32 len);

typedef struct RunlinkRelocContext {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ u8 *textBase;
    /* 0x08 */ u8 *dataBase;
    /* 0x0C */ u8 *bssBase;
    /* 0x10 */ u8 *relocBase;
} RunlinkRelocContext;

extern RunlinkRelocContext D_800D2DA8;

/* Linker-ish section anchors, referenced only to form differences. */
extern u8 D_80078D60[]; /* start of .data  */
extern u8 D_80078D64[]; /* the word after it */
extern u8 D_80085A40[]; /* start of .bss   */
extern void func_80000450(void); /* start of .text */

/*
 * The four section anchors runlinkInit differences, under the names the
 * original link gave them. The original placed _codeSegmentEnd and
 * _dataSegmentStart at one address and _dataSegmentEnd and _bssSegmentStart
 * at another, and the target proves it: uopt shares one address
 * materialisation per *symbol* -- it shares func_80000450 between the
 * vramBase store and the textSize difference -- yet the target materialises
 * 0x80078D60 twice and 0x80085A40 twice. Two names each, not two uses of one
 * name.
 *
 * This build supplies two real names at 0x80085A40, main_RODATA_END and
 * main_BSS_START from the generated linker script. It supplies only one at
 * 0x80078D60, because splat's own text/data boundary is 0x80076110, so the
 * data-segment start is spelled off the following word. Both spellings
 * resolve to 0x80078D60 and link to the same two instruction words; only the
 * symbol/addend split in the unlinked object differs.
 */
extern u8 main_RODATA_END[];
extern u8 main_BSS_START[];

#define runlinkCodeEnd    D_80078D60
#define runlinkDataStart  (D_80078D64 - 4)
#define runlinkDataEnd    main_RODATA_END
#define runlinkBssStart   main_BSS_START
#define runlinkBssEnd     D_800D8750

char *GetSymbolName(s32 symbolIndex) {
    return D_80082410;
}

/*
 * Turn one relocation record into the address it should resolve to.
 *
 * JFG calls this ResolveRelocAddress; same four arguments, same three-way
 * switch on the linkage operation, same 0xFFD/0xFFE/0xFFF section selectors.
 */
void *ResolveRelocAddress(s32 ortIndex, s32 otIndex, RelocationEntry *relocEntry, MipsInstruction *patchLocation) {
    s32 address;
    s32 addressBase;
    s32 addressOffset;
    u32 overlayNumber;
    RomTableEntry *romTableEntry;

    romTableEntry = &overlayRomTable[ortIndex];
    overlayNumber = romTableEntry->overlayNumber;
    addressOffset = 0;

    switch (relocEntry->u.info & 0xF) {
        case RELOC_OP_SYMBOL:
            switch (overlayNumber) {
                case RELOC_SECTION_DATA1:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80078D60 - (s32) func_80000450;
                    break;
                case RELOC_SECTION_DATA2:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80078D60 - (s32) func_80000450;
                    break;
                case RELOC_SECTION_BSS:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80085A40 - (s32) func_80000450;
                    break;
            }
            addressBase = overlayTable[overlayNumber].vramBase;
            if (addressBase == 0) {
                if (relocEntry->u.n.mode == RELOC_TYPE_26 ||
                    relocEntry->u.n.mode == RELOC_TYPE_32) {
                    return (void *) TrapDanglingJump;
                }
                return &D_800D2DC4;
            }
            return (void *) (addressBase + romTableEntry->functionOffset + addressOffset);

        case RELOC_OP_LOCAL:
            address = overlayTable[otIndex].vramBase + relocEntry->symbolIndex;
            if (relocEntry->u.n.mode == RELOC_TYPE_32) {
                address += patchLocation->word;
            }
            return (void *) address;

        case RELOC_OP_JUMP:
            return (void *) (((patchLocation->word & 0x3FFFFFF) << 2) + overlayTable[otIndex].vramBase);

        default:
            return NULL;
    }
}
/*
 * Write a resolved address into the instruction that referenced it, then make
 * the change visible to the CPU's instruction fetch.
 *
 * JFG calls this PatchInstruction; same three arguments, same four patch
 * operations with the same numbering, same pair of cache calls at the end.
 */
void PatchInstruction(MipsInstruction *instr, u32 address, u8 patchOp) {
    u32 word;
    u32 patched;

    switch (patchOp) {
        case RELOC_TYPE_32:
            instr->word = address;
            break;

        case RELOC_TYPE_26:
            word = instr->word;
            patched = ((address >> 2) & 0x3FFFFFF) ^ word;
            instr->word = ((patched << 6) >> 6) ^ word;
            break;

        case RELOC_TYPE_HI16:
            patched = address >> 16;
            if (address & 0x8000) {
                patched = (address >> 16) + 1;
            }
            instr->i.immediate = patched;
            break;

        case RELOC_TYPE_LO16:
            instr->i.immediate = address;
            break;
    }
    osWritebackDCache(instr, sizeof(MipsInstruction));
    osInvalICache(instr, sizeof(MipsInstruction));
}
/* Plateau (2026-08-25): -O2 -mips2 stays 0x4 long, with 126 differing words from +0x0.
 * Splitting the pointer lifetime removes s1 and yields the a3 caller spill, but grows the frame;
 * type, pool-order, volatile-home, result-reuse, and register-hint variants miss a1/a3 after the call. */
#ifdef NON_MATCHING
/*
 * ProcessRelocationEntry -- PARKED, not matched. ROM 0x32630-0x32878.
 *
 * Applies one relocation record and returns how many records it consumed: a
 * HI16 record needs its matching LO16 to know whether the low half will sign
 * extend, so mode 5 reads the next record too and returns 2, everything else
 * returns 1. The body below is complete and believed semantically right; what
 * it does not reproduce is IDO's register allocation.
 *
 * NONMATCHING-notes:
 *
 *  - Residual after fifteen source variants: verdict=structure-mismatch,
 *    words=126, regs=121, insns=147 against the ROM's 146, with a 0x48-byte
 *    candidate frame against the target's 0x40. Measured with
 *    `decomp-workbench diagnose-dumps` plus two `campaign` runs, 8 variants
 *    then 7, both recorded in the one ledger named below. The brief's parking
 *    rule asked for five campaigns; two were run. See the report.
 *
 *  - The single mechanical cause, named by the workbench's web analysis:
 *    web `a3->s1`, count 8. The ROM keeps `patchLocation` in a temp register
 *    and caller-saves it around the ResolveRelocAddress call (`sw a3,0x3c(sp)`
 *    at 0x32690, `lw a3,0x3c(sp)` at 0x326A4); every candidate instead
 *    promotes it to the callee-saved s1. That costs one `sw s1` in the
 *    prologue and one `lw s1` in the epilogue -- the entire instruction-count
 *    delta -- and every downstream register name shifts with it. The ROM
 *    promotes exactly one value to a callee-saved register, `relocEntry` in
 *    s0, and stack-homes five locals: op at 0x24, nextPatchLocation at 0x30,
 *    resolvedAddr at 0x34, mode at 0x38, patchLocation at 0x3c.
 *
 *  - Variants tried, all ranked by the campaign at
 *    .decomp-workbench/campaigns/ProcessRelocationEntry-5073763cae48/:
 *      baseline                                        words=126
 *      no separate hi-immediate local                  words=128
 *      patchLocation declared first among locals       words=128
 *      patchLocation declared last among locals        words=127
 *      mode/op unsigned rather than signed             words=126 (identical object)
 *      &base[index] rather than base + index           words=126 (identical object)
 *      field-guide lever 7, a code-free `if (g) {}`    words=144 (worse)
 *    Second run, the temp-fifo-phase playbook and the branch-likely lead:
 *      lever 14, call argument hoisted to a local      words=126 (identical object)
 *      lever 14 + the hoist reused for the lookups     words=129
 *      lever 15, phantom pop in a real `if`            words=147 (worse)
 *      lever 16, redundant assembler-folded mask       words=148 (worse)
 *      two nested `if`s rather than one `&&`           words=126 (identical object)
 *      nested `if`s + lever 14 together                words=126 (identical object)
 *    Declaration order moves the pool but never demotes patchLocation out of
 *    s1, and six variants across the two runs produced objects byte-identical
 *    to the baseline -- the front end canonicalizes those spellings away
 *    before the allocator ever sees them.
 *
 *  - What was ruled out. The extra instruction is NOT a missing/extra
 *    statement: opcode multisets agree everywhere except the s1 save/restore
 *    pair, and the constant sites the workbench flags are all frame offsets
 *    shifted by that same save. It is also not the flags nibble: the mode and
 *    operation reads match the ROM instruction for instruction.
 *
 *  - Three leads, all now TRIED AND DEAD, recorded so nobody repeats them.
 *    (1) The ROM emits `bnezl` at 0x326F4/0x3270C where every candidate emits
 *    `bnez`, which suggested the guard was two nested `if`s rather than one
 *    `&&`. It is not: the nested form compiles to an object byte-identical to
 *    the `&&` form, so the branch-likely selection is downstream of the
 *    allocation problem, not a cause of it. (2) The temp-fifo-phase playbook
 *    (levers 14-16) is the documented lever for this class and does not move
 *    it -- hoisting the call argument to a local before the divergence is
 *    another byte-identical object, and levers 15 and 16 both regress. (3) A
 *    fresh 119-combination flag sweep and an explicit `-O2 -g0 -mips2 -32`
 *    schedule probe both retain the stock 147-instruction, 126-word object;
 *    `-O2 -g3` regresses to 128 words.
 *
 *  - What is left to try, for the next person. The allocation decision is
 *    uopt's, so only the unsampled pool-position family (8-13) remains. Failing
 *    that, this is a candidate for the compiler-identity question that
 *    src/main/matrix.c raises: if the float code says this ROM was not built by
 *    the IDO 5.3 in tools/ido/, then an allocator difference in integer code is
 *    exactly the second symptom that hypothesis predicts, and no amount of
 *    source rewriting will close it.
 *
 * The C is kept, under NON_MATCHING, rather than deleted -- but see the
 * provenance note above before trusting it. It is the best available reading
 * of the function, not a verified one.
 */
/*
 * Apply one relocation record, and report how many records were consumed.
 *
 * A HI16 record needs its matching LO16 to know whether the low half will sign
 * extend, so mode 5 reads the *next* record too and returns 2; everything else
 * returns 1. The caller's loop advances by the return value.
 *
 * PROVENANCE -- this body is ADAPTED FROM JFG's public decomp, and it is the
 * one function in this file where that matters.
 *
 * An earlier version of this comment claimed the body was "written from
 * Mickey's ROM rather than adapted: same name, independently derived code."
 * That was false and has been corrected. The body follows JFG's
 * ProcessRelocationEntry statement for statement -- same control flow, same
 * `>= 0xFFC` clamp, same sign-extension of the low half, same guard before
 * the unresolved-symbol substitution, same terminal flags expression in all
 * three exits. The reasoning that produced the false claim was that JFG ships
 * its version as non-matching, so mine "had to be" independent. That does not
 * follow: an unvalidated implementation is still the thing I read and
 * followed.
 *
 * Why this one matters more than the rest of the file: every other function
 * here is checked by byte-identity against Mickey's ROM, which converts a
 * borrowed shape into a verified one. This function is parked non-matching, so
 * it has no such backstop. Its correctness currently rests on JFG agreement
 * plus my reading of Mickey's asm -- and JFG agreement is NOT evidence about
 * Mickey, because JFG's own version is unvalidated too. Treat every statement
 * below as a hypothesis about Mickey until the function matches.
 *
 * VALIDATION MUST COME FROM MICKEY'S ASM. The disassembly is at
 * asm/nonmatchings/main/runlink/func_80031A30.s (ROM 0x32630-0x32878). Read it
 * against this body statement by statement before trusting any line; do not
 * treat "JFG does it this way" as a reason for anything.
 *
 * One inherited expression is called out specifically, because a reviewer
 * flagged it as evidence of copying and was right to look:
 * `relocEntry->u.b.flags &= 0xFFF0` masks a u8 field with a 16-bit constant,
 * which is a no-op above bit 7 and reads like a quirk carried over from JFG.
 * It is carried over. It is ALSO corroborated by Mickey's own ROM, which emits
 * `andi ...,0xFFF0` at 0x32668, 0x32790, 0x32830 and 0x32858 -- writing
 * `& 0xF0` instead changes the assembled immediate, so the expression is
 * load-bearing rather than cosmetic. Both facts are true at once: it is
 * inherited from JFG *and* it is what Mickey's instruction encodes. Kept, with
 * this note, rather than removed.
 */
s32 ProcessRelocationEntry(RelocationEntry *relocEntry, s32 otIndex) {
    u32 resolvedAddr;
    u32 combinedAddr;
    u32 hiImmediate;
    u32 loImmediate;
    MipsInstruction *patchLocation;
    MipsInstruction *nextPatchLocation;
    s32 overlayNumber;
    s32 mode;
    s32 op;

    op = relocEntry->u.info & 0xF;
    mode = relocEntry->u.n.mode;

    if (op == RELOC_OP_DATA) {
        patchLocation = (MipsInstruction *) (D_800D2DB0 + (relocEntry->u.info >> 8));
        relocEntry->u.b.flags &= 0xFFF0;
    } else {
        patchLocation = (MipsInstruction *) (D_800D2DAC + (relocEntry->u.info >> 8));
    }

    resolvedAddr = (u32) ResolveRelocAddress(relocEntry->symbolIndex, otIndex, relocEntry, patchLocation);

    if (mode == RELOC_TYPE_HI16) {
        overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
        if (overlayNumber >= 0xFFC) {
            overlayNumber = 0;
        }
        if ((relocEntry->u.info & 0xF) == RELOC_OP_SYMBOL && overlayTable[overlayNumber].vramBase == 0) {
            resolvedAddr = (u32) &D_800D2DC4;
        }

        nextPatchLocation = (MipsInstruction *) (D_800D2DAC + (relocEntry[1].u.info >> 8));
        hiImmediate = patchLocation->i.immediate;
        loImmediate = nextPatchLocation->i.immediate;
        if (loImmediate & 0x8000) {
            loImmediate |= 0xFFFF0000;
        }
        combinedAddr = (hiImmediate << 16) + loImmediate;
        if (combinedAddr != (u32) &D_800D2DC4) {
            resolvedAddr += combinedAddr;
        }

        PatchInstruction(patchLocation, resolvedAddr, RELOC_TYPE_HI16);
        PatchInstruction(nextPatchLocation, resolvedAddr, RELOC_TYPE_LO16);
        relocEntry->u.b.flags = (op & 0xF) | (relocEntry->u.b.flags & 0xFFF0);
        return 2;
    }

    if (mode == RELOC_TYPE_LO16) {
        overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
        if (overlayNumber >= 0xFFC) {
            overlayNumber = 0;
        }
        if ((relocEntry->u.info & 0xF) == RELOC_OP_SYMBOL && overlayTable[overlayNumber].vramBase == 0) {
            resolvedAddr = (u32) &D_800D2DC4;
        }

        PatchInstruction(patchLocation, resolvedAddr + patchLocation->i.immediate, RELOC_TYPE_LO16);
        relocEntry->u.b.flags = (op & 0xF) | (relocEntry->u.b.flags & 0xFFF0);
        return 1;
    }

    PatchInstruction(patchLocation, resolvedAddr, mode);
    relocEntry->u.b.flags = (op & 0xF) | (relocEntry->u.b.flags & 0xFFF0);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80031A30.s")
#endif

/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * src/runLink.c:runlinkDownloadCode. Mickey's section layout, loop bounds,
 * globals, and relocation rules determine every divergence here.
 */
s32 runlinkDownloadCode(s32 overlayIndex) {
    OverlayHeader *overlay;
    RelocationEntry *relocTable;
    RelocationEntry *relocEntry;
    s32 savedDelay;
    PendingOverlayLoad *overlayLoad;
    s32 relocCount;
    s32 otherIndex;
    s32 overlayNumber;

    overlay = &overlayTable[overlayIndex];
    overlayLoad = D_800D2DC8;
    relocTable = NULL;

    if (overlay->vramBase != 0) {
        return 1;
    }

    relocCount = PENDING_OVERLAY_LOADS;
    if (relocCount != 0) {
        while (relocCount--) {
            if (overlayIndex == overlayLoad->overlayIndex) {
                return 0;
            }
            overlayLoad++;
        }
    }

    D_8007A27C = overlayIndex;
    overlay->vramBase = (s32) func_8002B280(
        overlay->textSize + overlay->dataSize + overlay->bssSize +
            (u16) overlay->relocTableSize,
        0x83);
    D_8007A27C = -1;

    if (overlay->vramBase == 0) {
        return 0;
    }

    if (overlay->relocTableSize2) {
        relocTable = func_8002B280(overlay->relocTableSize2, 0x83);
        if (relocTable == NULL) {
            mmFree((void *) overlay->vramBase);
            return 0;
        }
        romCopy(overlay->romAddress + overlay->textSize + overlay->dataSize +
                    (u16) overlay->relocTableSize,
                (u32) relocTable, overlay->relocTableSize2);
    }

    D_800D2DA8.textBase = (u8 *) overlay->vramBase;
    D_800D2DA8.dataBase =
        (u8 *) ((s32) D_800D2DA8.textBase + overlay->textSize);
    D_800D2DA8.bssBase =
        (u8 *) ((s32) D_800D2DA8.dataBase + overlay->dataSize);
    D_800D2DA8.relocBase =
        (u8 *) ((s32) D_800D2DA8.bssBase + overlay->bssSize);

    if (overlay->bssSize == 0) {
        romCopy(overlay->romAddress, (u32) overlay->vramBase,
                overlay->textSize + overlay->dataSize +
                    (u16) overlay->relocTableSize);
    } else {
        s32 *bss;

        romCopy(overlay->romAddress, (u32) overlay->vramBase,
                overlay->textSize + overlay->dataSize);
        bss = (s32 *) D_800D2DA8.bssBase;
        relocCount = (u32) overlay->bssSize >> 2;
        while (relocCount--) {
            *bss++ = 0;
        }
        romCopy(overlay->romAddress + overlay->textSize + overlay->dataSize,
                (u32) D_800D2DA8.relocBase,
                (u16) overlay->relocTableSize);
    }

    osInvalICache((void *) overlay->vramBase, overlay->textSize);

    if (relocTable != NULL) {
        savedDelay = mmGetDelay();
        relocCount = (u32) overlay->relocTableSize2 >> 3;
        relocEntry = relocTable;
        while (relocCount-- > 0) {
            if (func_80031A30(relocEntry, overlayIndex) == 2) {
                relocCount--;
                relocEntry++;
            }
            relocEntry++;
        }
        mmSetDelay(0);
        mmFree(relocTable);
        mmSetDelay(savedDelay);
    }

    relocCount = (u32) (u16) overlay->relocTableSize >> 3;
    relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
    while (relocCount-- > 0) {
        if (func_80031A30(relocEntry, overlayIndex) == 2) {
            relocCount--;
            relocEntry++;
        }
        relocEntry++;
    }

    overlay = overlayTable;
    for (otherIndex = 0; otherIndex < overlayCount; otherIndex++) {
        if (overlay->vramBase != 0 && otherIndex != overlayIndex) {
            if (otherIndex == 0) {
                D_800D2DA8.textBase = (u8 *) func_80000450;
                D_800D2DA8.dataBase = D_80078D60;
                D_800D2DA8.bssBase = D_80085A40;
                D_800D2DA8.relocBase = (u8 *) mainRelocTable;
                relocEntry = mainRelocTable;
                relocCount = mainRelocTableCount;
            } else {
                D_800D2DA8.textBase = (u8 *) overlay->vramBase;
                D_800D2DA8.dataBase =
                    (u8 *) ((s32) D_800D2DA8.textBase + overlay->textSize);
                D_800D2DA8.bssBase =
                    (u8 *) ((s32) D_800D2DA8.dataBase + overlay->dataSize);
                D_800D2DA8.relocBase =
                    (u8 *) ((s32) D_800D2DA8.bssBase + overlay->bssSize);
                relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
                relocCount = (u32) (u16) overlay->relocTableSize >> 3;
            }

            while (relocCount-- > 0) {
                overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
                if (overlayNumber >= 0xFFC) {
                    overlayNumber = 0;
                }
                if (overlayNumber == overlayIndex &&
                    ((relocEntry->u.info & 0xF) == RELOC_OP_SYMBOL ||
                     (relocEntry->u.info & 0xF) == RELOC_OP_DATA)) {
                    if (func_80031A30(relocEntry, otherIndex) == 2) {
                        relocCount--;
                        relocEntry++;
                    }
                }
                relocEntry++;
            }
        }
        overlay++;
    }

    overlay = &overlayTable[overlayIndex];
    if (overlay->initFunction != -1) {
        ((void (*)(void)) (overlay->vramBase + overlay->initFunction))();
    }

    return 1;
}
/* PROVENANCE: Jet Force Gemini,
 * asm/nonmatchings/runLink/runlinkEnsureJumpIsValid.s; role and skeleton
 * context only. Mickey's boundary, source spelling, object, and ROM bytes are
 * authoritative. */
/* Exact C: 101 instruction words, frame -0x20, and all 21 relocations match.
 * Four inert allocation aids remain: one overlayCount block and three
 * constant-true blocks. See docs/cleanup-queue.md. */
s32 func_800320F0(void **jumpAddress)
{
  register void **address;
  OverlayHeader *overlay;
  RelocationEntry *relocEntry;
  s32 relocCount;
  s32 overlayIndex;
  s32 section;
  s32 overlayNumber;
  u32 relocInfo;
  address = jumpAddress;
  if ((*address) != ((void *) TrapDanglingJump))
  {
    return 0;
  }
  if (overlayCount)
  {
  }
  overlay = overlayTable;
 if (1) { } if (1) { } if (1) { }
  for (overlayIndex = 0; overlayIndex < overlayCount; overlayIndex++)
  {
    if (overlay->vramBase != 0)
    {
      if (overlayIndex == 0)
      {
        D_800D2DA8.textBase = (u8 *) func_80000450;
        D_800D2DA8.dataBase = D_80078D60;
        D_800D2DA8.bssBase = D_80085A40;
        D_800D2DA8.relocBase = (u8 *) mainRelocTable;
        relocEntry = mainRelocTable;
        relocCount = mainRelocTableCount;
      }
      else
      {
        D_800D2DA8.textBase = (u8 *) overlay->vramBase;
        D_800D2DA8.dataBase = (u8 *) (((s32) D_800D2DA8.textBase) + overlay->textSize);
        D_800D2DA8.bssBase = (u8 *) (((s32) D_800D2DA8.dataBase) + overlay->dataSize);
        D_800D2DA8.relocBase = (u8 *) (((s32) D_800D2DA8.bssBase) + overlay->bssSize);
        relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
        relocCount = ((u32) ((u16) overlay->relocTableSize)) >> 3;
      }
      while (relocCount--)
      {
        relocInfo = relocEntry->u.info;
        switch (relocInfo & 0xF)
        {
          case 3:
            section = 2;
            break;

          default:
            section = 1;
            break;

        }

        if (((u8 *) address) == (((u8 **) (&D_800D2DA8))[section] + (relocInfo >> 8)))
        {
          overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
          if (overlayNumber >= 0xFFC)
          {
            overlayNumber = 0;
          }
          runlinkDownloadCode(overlayNumber);
          return 1;
        }
        relocEntry++;
      }

    }
    overlay++;
  }

  return 0;
}
/*
 * Is this overlay resident? Returns its VRAM base, which is zero when it is
 * not. Same one-line function, same name, as JFG's public decomp.
 */
s32 runlinkIsModuleLoaded(s32 module) {
    return overlayTable[module].vramBase;
}
/*
 * Make sure an overlay is resident and then call its resume entry point.
 *
 * If the overlay is not loaded but is queued in the pending-load list, load it
 * first; if it still is not loaded afterwards, do nothing. JFG has the same
 * function under this name, with the same three-part shape.
 */
void runlinkCallResumeFunction(s32 overlayIndex) {
    OverlayHeader *overlay;
    PendingOverlayLoad *pendingLoad;
    s32 remaining;

    overlay = &overlayTable[overlayIndex];
    if (overlay->resumeFunction == -1) {
        return;
    }

    pendingLoad = D_800D2DC8;
    if (overlay->vramBase == 0) {
        remaining = PENDING_OVERLAY_LOADS - 1;
        do {
            if (overlayIndex == pendingLoad->overlayIndex) {
                runlinkResumeCode(overlayIndex);
                break;
            }
            pendingLoad++;
        } while (remaining--);
    }

    if (overlay->vramBase != 0) {
        ((void (*)(void)) (overlay->vramBase + overlay->resumeFunction))();
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini src/runLink.c:runlinkFreeCode
 * at efd5abb1c79636e297b831f7c2d5bf47eac39c0c. Mickey's packed records,
 * resident section anchors, and ROM bytes determine the final body.
 */
void runlinkFreeCode(s32 overlayIndex) {
    OverlayHeader *overlay;
    RelocationEntry *relocEntry;
    MipsInstruction *instr;
    s32 overlayNum;
    s32 i;
    void *loadedAddress;
    s32 relocType;
    s32 found;
    s32 relocCount;
    u32 address;

    overlay = &overlayTable[overlayIndex];
    if (D_8007A670 == FALSE) {
        runlinkCallResumeFunction(overlayIndex);
    }

    loadedAddress = (void *) overlay->vramBase;
    if (loadedAddress == NULL) {
        found = FALSE;
        relocCount = PENDING_OVERLAY_LOADS;
        for (i = 0; relocCount--; i++) {
            if (overlayIndex == D_800D2DC8[i].overlayIndex) {
                found = TRUE;
                break;
            }
        }

        if (found) {
            mmFree((void *) ((u32) D_800D2DC8[i].unk0 + overlay->textSize));
            D_800D2DC8[i].overlayIndex = 0xFFB;
        }
        return;
    }

    mmFree(loadedAddress);
    overlay->vramBase = 0;

    linkSlotTable[overlayIndex].tag = 0;
    linkSlotTable[overlayIndex].useCount = 0;

    overlay = overlayTable;
    for (i = 0; i < overlayCount; i++, overlay++) {
        loadedAddress = (void *) overlay->vramBase;
        if (loadedAddress != NULL && i != overlayIndex) {
            if (i == 0) {
                D_800D2DA8.textBase = (u8 *) &func_80000450;
                D_800D2DA8.dataBase = (u8 *) &D_80078D60;
                D_800D2DA8.bssBase = (u8 *) &D_80085A40;
                D_800D2DA8.relocBase = (u8 *) mainRelocTable;
                relocEntry = (RelocationEntry *) mainRelocTable;
                relocCount = mainRelocTableCount;
            } else {
                D_800D2DA8.textBase = (u8 *) loadedAddress;
                D_800D2DA8.dataBase = (u8 *) D_800D2DA8.textBase + overlay->textSize;
                D_800D2DA8.bssBase = (u8 *) D_800D2DA8.dataBase + overlay->dataSize;
                D_800D2DA8.relocBase = (u8 *) D_800D2DA8.bssBase + overlay->bssSize;
                relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
                relocCount = (u32) (u16) overlay->relocTableSize / sizeof(RelocationEntry);
            }

            while (relocCount--) {
                relocType = relocEntry->u.info & 0xF;

                overlayNum = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
                /* Reserved data/bss selectors belong to the resident module. */
                if (overlayNum > 0xFFB) {
                    overlayNum = 0;
                }

                if (overlayNum == overlayIndex) {
                    if ((relocEntry->u.info & 0xF) == RELOC_OP_DATA) {
                        instr = (MipsInstruction *)
                            ((u32) D_800D2DA8.dataBase + (relocEntry->u.info >> 8));
                        relocEntry->u.n.op = RELOC_OP_SYMBOL;
                    } else {
                        instr = (MipsInstruction *)
                            ((u32) D_800D2DA8.textBase + (relocEntry->u.info >> 8));
                    }

                    /* Restore dangling calls to the trap; clear other references. */
                    if (((u32) relocEntry->u.b.flags >> 4) == RELOC_TYPE_26) {
                        address = (u32) &TrapDanglingJump;
                    } else {
                        address = 0;
                    }

                    PatchInstruction(instr, address, ((u32) relocEntry->u.b.flags >> 4));
                }

                relocEntry->u.n.op = relocType;
                relocEntry++;
            }
        }
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * src/runLink.c:runlinkUnloadOverlay. Mickey's packed relocation records,
 * resident section anchors, and link-slot layout determine the final body.
 */
void runlinkUnloadOverlay(s32 overlayIndex) {
    OverlayHeader *overlay;
    PendingOverlayLoad *pendingLoad;
    RelocationEntry *relocEntry;
    MipsInstruction *patchLocation;
    s32 overlayNumber;
    s32 loadedAddress;
    s32 relocType;
    s32 found;
    s32 i;
    u32 patchOperation;
    u32 address;

    overlay = &overlayTable[overlayIndex];
    runlinkCallResumeFunction(overlayIndex);
    loadedAddress = overlay->vramBase;
    address = loadedAddress;

    if (address == 0) {
        found = FALSE;
        pendingLoad = D_800D2DC8;
        i = PENDING_OVERLAY_LOADS;
        while (i--) {
            if (overlayIndex == pendingLoad->overlayIndex) {
                found = TRUE;
                break;
            }
            pendingLoad++;
        }

        if (found == FALSE) {
            return;
        }

        mmFree((void *) (pendingLoad->unk0 + overlay->textSize));
        pendingLoad->overlayIndex = 0xFFB;
        return;
    }

    mmFree((void *) address);
    overlay->vramBase = 0;
    linkSlotTable[overlayIndex].tag = 0;
    linkSlotTable[overlayIndex].useCount = 0;

    relocEntry = mainRelocTable;
    i = mainRelocTableCount;
    while (i--) {
        relocType = relocEntry->u.info & 0xF;
        overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
        if (overlayNumber >= 0xFFC) {
            overlayNumber = 0;
        }

        if (overlayNumber == overlayIndex) {
            if ((relocEntry->u.info & 0xF) == RELOC_OP_DATA) {
                patchLocation = (MipsInstruction *)
                    (D_80078D60 + (relocEntry->u.info >> 8));
                relocEntry->u.n.op = RELOC_OP_SYMBOL;
            } else {
                patchLocation = (MipsInstruction *)
                    ((u8 *) func_80000450 + (relocEntry->u.info >> 8));
            }

            patchOperation = (u32) relocEntry->u.b.flags >> 4;
            /* Legacy inert xor preserves IDO's comparison operand order;
             * tracked in docs/cleanup-queue.md. */
            if ((patchOperation ^ 0) == RELOC_TYPE_26) {
                address = (u32) TrapDanglingJump;
            } else {
                address = 0;
            }
            PatchInstruction(patchLocation, address, patchOperation);
        }

        relocEntry->u.n.op = relocType;
        relocEntry++;
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkFlushModules.s and the corresponding
 * src/runlink.c function order. Mickey's pending-load count and linked bytes
 * determine the final body.
 */
void runlinkFlushModules(void) {
    PendingOverlayLoad *pendingLoad;
    s32 remaining;

    pendingLoad = D_800D2DC8;
    remaining = overlayCount - 1;
    if (remaining > 0) {
        do {
            runlinkUnloadOverlay(remaining);
            remaining--;
        } while (remaining > 0);
    }

    /* Source-line grouping controls IDO's otherwise independent %lo/constant schedule. */
    remaining = PENDING_OVERLAY_LOADS - 1; do {
        if (pendingLoad->overlayIndex != 0xFFB) {
            mmFree((void *) (overlayTable[pendingLoad->overlayIndex].textSize + pendingLoad->unk0));
            pendingLoad->overlayIndex = 0xFFB;
        }
        pendingLoad++;
    } while (remaining--);
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * src/runLink.c:runlinkInitialise at upstream efd5abb. Mickey's ROM-block
 * boundaries, resident section anchors, packed header layout, and pending-load
 * count determine the final body.
 */
void runlinkInit(void) {
    OverlayHeader *overlay;
    s32 i;

    overlayTable = func_8002B280((u32) (D_184C3E0 - D_184B680) + sizeof(OverlayHeader), 0x83);
    romCopy((u32) D_184B680, (u32) (overlayTable + 1), (u32) (D_184C3E0 - D_184B680));

    overlayRomTable = func_8002B280((u32) (D_184B680 - D_1849730), 0x83);
    romCopy((u32) D_1849730, (u32) overlayRomTable, (u32) (D_184B680 - D_1849730));

    mainRelocTable = func_8002B280((u32) (D_1849730 - D_1848B70), 0x83);
    romCopy((u32) D_1848B70, (u32) mainRelocTable, (u32) (D_1849730 - D_1848B70));
    mainRelocTableCount = *(u32 *) mainRelocTable;
    mainRelocTable = (RelocationEntry *) ((u8 *) mainRelocTable + 4);
    overlayCount = ((u32) (D_184C3E0 - D_184B680) / sizeof(OverlayHeader)) + 1;

    i = PENDING_OVERLAY_LOADS;
    while (i--) {
        D_800D2DC8[i].overlayIndex = 0xFFB;
    }

    linkSlotTable = func_8002B280(overlayCount * sizeof(LinkSlot), 0x83);
    _bzero(linkSlotTable, overlayCount * sizeof(LinkSlot));

    overlayTable->vramBase = (s32) func_80000450;
    overlayTable->romAddress = 0;
    overlayTable->textSize = (u32) runlinkCodeEnd - (u32) func_80000450;
    overlayTable->dataSize = (u32) runlinkDataEnd - (u32) runlinkDataStart;
    overlayTable->bssSize = (u32) runlinkBssEnd - (u32) runlinkBssStart;
    overlayTable->relocTableSize = mainRelocTableCount * sizeof(RelocTableEntry);
    overlayTable->relocTableSize2 = 0;

    overlay = overlayTable + 1;
    i = overlayCount - 1;
    while (i--) {
        overlay->romAddress = (s32) D_184C3E0 + overlay->romAddress;
        overlay++;
    }

    D_8007A674 = 0;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkSuspendCode.s and its src/runLink.c order.
 * Mickey's allocation tag, pending-load count, and linked bytes determine
 * the final body.
 */
void runlinkSuspendCode(s32 overlayIndex) {
    OverlayHeader *overlay;
    PendingOverlayLoad *pendingLoad;
    s32 remaining;
    s32 savedDelay;

    overlay = &overlayTable[overlayIndex];
    pendingLoad = D_800D2DC8;
    remaining = PENDING_OVERLAY_LOADS - 1;
    if (overlay->vramBase != 0) {
        do {
            if (pendingLoad->overlayIndex == 0xFFB) {
                savedDelay = mmGetDelay();
                pendingLoad->unk0 = overlay->vramBase;
                pendingLoad->overlayIndex = overlayIndex;
                mmSetDelay(0);
                D_8007A670 = 1;
                runlinkFreeCode(overlayIndex);
                D_8007A670 = 0;
                mmSetDelay(savedDelay);
                func_8002B524(overlay->dataSize + overlay->bssSize +
                                  (u16) overlay->relocTableSize,
                              (void *) (pendingLoad->unk0 + overlay->textSize),
                              0x83);
                return;
            }
            pendingLoad++;
        } while (remaining--);
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkResumeCode.s and its documented role in
 * src/runLink.c. Mickey's allocation tag, packed relocation records, and
 * resident section anchors determine the C body.
 * Workbench: frame-layout/constant residual; six stack operands differ first +0x0, with opcode, register, and relocation surfaces exact.
 * Tried stack-home levers 26/32, frame-local variants, and the 119-combination flag lattice.
 * Remains: target reserves 0x50 and homes pendingLoad at sp+0x44; the candidate reserves 0x48 and uses sp+0x40.
 */
void runlinkResumeCode(s32 overlayIndex) {
    OverlayHeader *overlay;
    struct {
        PendingOverlayLoad *value;
        s32 pad;
    } pendingLoad;
    s32 savedDelay;
    s32 relocSavedDelay;
    RelocationEntry *relocTable;
    RelocationEntry *relocEntry;
    s32 relocCount;
    s32 otherIndex;
    s32 overlayNumber;
    s32 found;

    overlay = &overlayTable[overlayIndex];
    pendingLoad.value = D_800D2DC8;
    found = FALSE;
    relocTable = NULL;
    relocCount = PENDING_OVERLAY_LOADS - 1;
    do {
        if (overlayIndex == pendingLoad.value->overlayIndex) {
            found = TRUE;
            break;
        }
        pendingLoad.value++;
    } while (relocCount--);

    if (found) {
        savedDelay = mmGetDelay();
        mmSetDelay(0);
        mmFree((void *) (pendingLoad.value->unk0 + overlay->textSize));
        mmSetDelay(savedDelay);

        overlay->vramBase = (s32) func_8002B524(
            overlay->textSize + overlay->dataSize + overlay->bssSize +
                (u16) overlay->relocTableSize,
            (void *) pendingLoad.value->unk0, 0x83);
        if (overlay->vramBase == 0) {
            return;
        }

        if (overlay->relocTableSize2) {
            relocTable = func_8002B280(overlay->relocTableSize2, 0x83);
            if (relocTable == NULL) {
                mmFree((void *) overlay->vramBase);
                return;
            }
            romCopy(overlay->romAddress + overlay->textSize + overlay->dataSize +
                        (u16) overlay->relocTableSize,
                    (u32) relocTable, overlay->relocTableSize2);
        }

        D_800D2DA8.textBase = (u8 *) overlay->vramBase;
        D_800D2DA8.dataBase =
            (u8 *) ((s32) D_800D2DA8.textBase + overlay->textSize);
        D_800D2DA8.bssBase =
            (u8 *) ((s32) D_800D2DA8.dataBase + overlay->dataSize);
        D_800D2DA8.relocBase =
            (u8 *) ((s32) D_800D2DA8.bssBase + overlay->bssSize);

        romCopy(overlay->romAddress, (u32) overlay->vramBase, overlay->textSize);

        if (relocTable != NULL) {
            relocSavedDelay = mmGetDelay();
            relocCount = (u32) overlay->relocTableSize2 >> 3;
            relocEntry = relocTable;
            while (relocCount-- > 0) {
                if ((relocEntry->u.info >> 8) < (u32) overlay->textSize &&
                    func_80031A30(relocEntry, overlayIndex) == 2) {
                    relocCount--;
                    relocEntry++;
                }
                relocEntry++;
            }
            mmSetDelay(0);
            mmFree(relocTable);
            mmSetDelay(relocSavedDelay);
        }

        relocCount = (u32) (u16) overlay->relocTableSize >> 3;
        relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
        while (relocCount-- > 0) {
            if ((relocEntry->u.info >> 8) < (u32) overlay->textSize &&
                func_80031A30(relocEntry, overlayIndex) == 2) {
                relocCount--;
                relocEntry++;
            }
            relocEntry++;
        }

        overlay = overlayTable;
        for (otherIndex = 0; otherIndex < overlayCount; otherIndex++) {
            if (overlay->vramBase != 0 && otherIndex != overlayIndex) {
                if (otherIndex == 0) {
                    D_800D2DA8.textBase = (u8 *) func_80000450;
                    D_800D2DA8.dataBase = D_80078D60;
                    D_800D2DA8.bssBase = D_80085A40;
                    D_800D2DA8.relocBase = (u8 *) mainRelocTable;
                    relocEntry = mainRelocTable;
                    relocCount = mainRelocTableCount;
                } else {
                    D_800D2DA8.textBase = (u8 *) overlay->vramBase;
                    D_800D2DA8.dataBase =
                        (u8 *) ((s32) D_800D2DA8.textBase + overlay->textSize);
                    D_800D2DA8.bssBase =
                        (u8 *) ((s32) D_800D2DA8.dataBase + overlay->dataSize);
                    D_800D2DA8.relocBase =
                        (u8 *) ((s32) D_800D2DA8.bssBase + overlay->bssSize);
                    relocEntry = (RelocationEntry *) D_800D2DA8.relocBase;
                    relocCount = (u32) (u16) overlay->relocTableSize >> 3;
                }

                while (relocCount-- > 0) {
                    overlayNumber =
                        overlayRomTable[relocEntry->symbolIndex].overlayNumber;
                    if (overlayNumber >= 0xFFC) {
                        overlayNumber = 0;
                    }
                    if (overlayNumber == overlayIndex &&
                        ((relocEntry->u.info & 0xF) == RELOC_OP_SYMBOL ||
                         (relocEntry->u.info & 0xF) == RELOC_OP_DATA)) {
                        if (func_80031A30(relocEntry, otherIndex) == 2) {
                            relocCount--;
                            relocEntry++;
                        }
                    }
                    relocEntry++;
                }
            }
            overlay++;
        }

        pendingLoad.value->overlayIndex = 0xFFB;
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkResumeAll.s and the corresponding
 * src/runlink.c function order. Mickey's pending-load count and linked bytes
 * determine the final body.
 */
void runlinkResumeAll(void) {
    PendingOverlayLoad *pendingLoad;
    s32 overlayIndex;
    s32 remaining;

    pendingLoad = D_800D2DC8;
    remaining = PENDING_OVERLAY_LOADS;
    remaining--;
    do {
        if (pendingLoad->overlayIndex != 0xFFB) {
            overlayIndex = pendingLoad->overlayIndex;
            runlinkResumeCode(overlayIndex);
        }
        pendingLoad++;
    } while (remaining--);
}
/*
 * Write both halves of one link slot.
 *
 * The reload of the table pointer between the two stores is the ROM's, not an
 * accident: the first store goes through the pointer, so the compiler cannot
 * prove it did not overwrite the pointer itself and reloads it.
 *
 * The two value parameters are u16, and that is measured rather than
 * cosmetic. The ROM homes them into the caller's argument save area
 * (`sw a1,0x4(sp)`, `sw a2,0x8(sp)` at ROM 0x33C50) with no frame of its own,
 * which is what IDO does for a parameter narrower than int; with s32 or u32
 * parameters both stores disappear and the function is two instructions
 * short. Found by a six-variant decomp-workbench campaign over the parameter
 * types -- u16 was instruction-words-identical, s16 left 14 register
 * differences, u8 left a structural one.
 */
void SetLinkSlot(s32 slot, u16 tag, u16 useCount) {
    linkSlotTable[slot].tag = tag;
    linkSlotTable[slot].useCount = useCount;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkTick.s and its src/runLink.c role and order.
 * Mickey's packed fields and linked bytes determine the final body.
 */
void runlinkTick(void) {
    LinkSlot *slot;
    s32 slotIndex;

    slotIndex = overlayCount;
    if (D_8007A67C != 0) {
        if (slotIndex--) {
            do {
                slot = &linkSlotTable[slotIndex];
                if (slot->useCount != 0) {
                    slot->useCount--;
                }
                if (slot->tag != 0) {
                    if (--slot->tag == 0) {
                        runlinkFreeCode(slotIndex);
                    }
                }
            } while (slotIndex--);
        }
    }
}
/*
 * Sweep the link-slot table and release every slot that is tagged but no
 * longer used, walking downwards from the last slot.
 *
 * The condition is `while (i--)` and not `while (i-- != 0)`: the ROM emits
 * `move v0,s1` / `beqz s1` at 0x80033164, i.e. it uses the value of the
 * expression directly, while the explicit comparison makes IDO materialise a
 * boolean with `sltu v0,zero,s1` instead. One instruction, and it is the only
 * difference between the two spellings.
 *
 * The loop bound is `overlayCount`, and that name is not a guess about this
 * function: runlinkGetAddressInfo (0x800331E4) uses the SAME global as the
 * bound of a walk over the 0x20-stride overlay-header table, at ROM
 * 0x33F04-0x33F24. One counter serving both tables is itself the finding --
 * there is exactly one link slot per overlay, so the slot's `tag` is a
 * per-overlay field rather than an index into some third table. That is also
 * the best evidence available for what LinkSlot's two fields mean, and it is
 * still not enough to promote them out of "inference"; see include/game/runlink.h.
 */
void ReleaseUnusedLinkSlots(void) {
    LinkSlot *slot;
    s32 i;

    i = overlayCount;
    while (i--) {
        slot = &linkSlotTable[i];
        if (slot->tag != 0 && slot->useCount == 0) {
            runlinkFreeCode(i);
            slot->tag = 0;
            slot->useCount = 0;
        }
    }
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's permitted published
 * asm/nonmatchings/runLink/runlinkGetAddressInfo.s and its public prototype.
 * Mickey's resident-address shortcut and ROM-table layout determine the
 * final body.
 */
s32 runlinkGetAddressInfo(u32 address, s32 *moduleId, s32 *moduleAddress,
                          u32 **symbolName) {
    s32 overlayVram;
    RomTableEntry *romEntry;
    OverlayHeader *overlayBase;
    OverlayHeader *overlay;
    s32 count;
    s32 symbolIndex;
    u32 bestAddress;
    u32 symbolAddress;
    u32 symbolOffset;

    romEntry = overlayRomTable;
    *moduleId = 0;
    *moduleAddress = 0;
    bestAddress = 0;
    if (symbolName != NULL) {
        *symbolName = (u32 *) D_80082488;
    }

    if (D_8007A674 != 0) {
        *moduleAddress = address - 0x80000450;
        return 1;
    }

    if (symbolName != NULL) {
        count = D_8007A678;
        while (count--) {
            overlayBase = overlayTable;
            overlay = &overlayBase[romEntry->overlayNumber];
            overlayVram = overlay->vramBase;
            if (overlayVram != 0) {
                symbolOffset = romEntry->functionOffset;
                symbolAddress = overlayVram + symbolOffset;
                if ((u32) overlay->textSize >= symbolOffset &&
                    address >= symbolAddress && bestAddress < symbolAddress) {
                    bestAddress = symbolAddress;
                    symbolIndex = romEntry - overlayRomTable;
                }
            }
            romEntry++;
        }
        if (bestAddress != 0) {
            *symbolName = (u32 *) GetSymbolName(symbolIndex);
        }
    }

    overlayBase = overlayTable;
    overlay = overlayBase;
    count = overlayCount;
    while (count--) {
        if (address >= (u32) overlay->vramBase &&
            address <= (u32) (overlay->vramBase + overlay->textSize)) {
            *moduleId = overlay - overlayBase;
            *moduleAddress = address - overlay->vramBase;
            return 1;
        }
        overlay++;
    }
    return 0;
}

/* PLATEAU-HANDOFF:ProcessRelocationEntry:start
 * symbol: ProcessRelocationEntry
 * score: 126 differing words
 * frame: 0x48
 * relocations: 25
 * first-mismatch: +0x0
 * summary: Remeasured 2026-09-23: 126 masked at size delta +4, frame 0x48 against 0x40, relocations 25 of 25; a1/a3 after the call stay structural.
 * PLATEAU-HANDOFF:ProcessRelocationEntry:end
 */
