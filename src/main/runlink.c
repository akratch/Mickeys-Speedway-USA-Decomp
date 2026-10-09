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
 * What makes that adaptation *sound* for every function below is
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
 * ProcessRelocationEntry was the last function here without that backstop.
 * It matched on 2026-10-02. Its provenance note is attached to it directly.
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
extern void *mmAlloc(s32 size, s32 tag);
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
extern void amSetMuteMode(void); /* start of .text */

/*
 * The four section anchors runlinkInit differences, under the names the
 * original link gave them. The original placed _codeSegmentEnd and
 * _dataSegmentStart at one address and _dataSegmentEnd and _bssSegmentStart
 * at another, and the target proves it: uopt shares one address
 * materialisation per *symbol* -- it shares amSetMuteMode between the
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
                    addressOffset = (s32) D_80078D60 - (s32) amSetMuteMode;
                    break;
                case RELOC_SECTION_DATA2:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80078D60 - (s32) amSetMuteMode;
                    break;
                case RELOC_SECTION_BSS:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80085A40 - (s32) amSetMuteMode;
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
/*
 * Apply one relocation record, and report how many records were consumed.
 *
 * A HI16 record needs its matching LO16 to know whether the low half will sign
 * extend, so mode 5 reads the *next* record too and returns 2. Everything else
 * returns 1. The caller's loop advances by the return value.
 *
 * PROVENANCE: adapted from Jet Force Gemini's published src/runLink.c
 * ProcessRelocationEntry, a permitted source under docs/CLEANROOM.md. That
 * includes its local declaration order and its unused `pad`, which together
 * place patchLocation, mode, resolvedAddr and nextPatchLocation at the target's
 * stack homes 0x3C..0x30 and op at 0x24. Mickey's ROM is decisive, and it
 * differs from JFG in three places. There is no HI16-without-LO16 check. The
 * 0xFFC clamp is `>=`. The section bases are Mickey's text and data base
 * globals.
 *
 * Matched 2026-10-02 (lane w2-front) by the following changes, from 126
 * masked words at size +4:
 *  - The record is read through the plain word bitfield (RelocationEntry
 *    u.f, the format tools/overlay_tables.py decodes). `op` is saved on
 *    entry, cleared for a data-section record, and written back on each
 *    exit. The inherited body's `flags & 0xFFF0` expressions were IDO's
 *    bitfield insert. They were never source.
 *  - The locals are declared in JFG's order, with the unused pad. That alone
 *    removed s1 and reached 6 words at size delta 0.
 *  - The section base is cast to u32 before the offset is added. The cast is
 *    a node (L52), so the base is drawn before the offset, as in the target.
 */
s32 ProcessRelocationEntry(RelocationEntry *relocEntry, s32 otIndex) {
    MipsInstruction *patchLocation;
    s32 mode;
    u32 resolvedAddr;
    MipsInstruction *nextPatchLocation;
    s32 pad;
    s32 overlayNumber;
    s32 op;
    u32 nextLo;
    u32 currLo;

    mode = relocEntry->u.f.mode;
    op = relocEntry->u.f.op;
    if (relocEntry->u.f.op == RELOC_OP_DATA) {
        patchLocation = (MipsInstruction *) ((u32) D_800D2DB0 + relocEntry->u.f.targetOffset);
        relocEntry->u.f.op = RELOC_OP_SYMBOL;
    } else {
        patchLocation = (MipsInstruction *) ((u32) D_800D2DAC + relocEntry->u.f.targetOffset);
    }

    resolvedAddr = (u32) ResolveRelocAddress(relocEntry->symbolIndex, otIndex, relocEntry, patchLocation);

    if (mode == RELOC_TYPE_HI16) {
        overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
        if (overlayNumber >= 0xFFC) {
            overlayNumber = 0;
        }
        if (relocEntry->u.f.op == RELOC_OP_SYMBOL && overlayTable[overlayNumber].vramBase == 0) {
            resolvedAddr = (u32) &D_800D2DC4;
        }
        nextPatchLocation =
            (MipsInstruction *) ((u32) D_800D2DAC + relocEntry[1].u.f.targetOffset);
        currLo = patchLocation->i.immediate;
        nextLo = nextPatchLocation->i.immediate;
        if (nextLo & 0x8000) {
            nextLo |= 0xFFFF0000;
        }
        currLo = (currLo << 16) + nextLo;
        if (currLo != (u32) &D_800D2DC4) {
            resolvedAddr += currLo;
        }
        PatchInstruction(patchLocation, resolvedAddr, RELOC_TYPE_HI16);
        PatchInstruction(nextPatchLocation, resolvedAddr, RELOC_TYPE_LO16);
        relocEntry->u.f.op = op;
        return 2;
    } else if (mode == RELOC_TYPE_LO16) {
        overlayNumber = overlayRomTable[relocEntry->symbolIndex].overlayNumber;
        if (overlayNumber >= 0xFFC) {
            overlayNumber = 0;
        }
        if (relocEntry->u.f.op == RELOC_OP_SYMBOL && overlayTable[overlayNumber].vramBase == 0) {
            resolvedAddr = (u32) &D_800D2DC4;
        }
        resolvedAddr += patchLocation->i.immediate;
        PatchInstruction(patchLocation, resolvedAddr, RELOC_TYPE_LO16);
        relocEntry->u.f.op = op;
        return 1;
    } else {
        PatchInstruction(patchLocation, resolvedAddr, mode);
        relocEntry->u.f.op = op;
        return 1;
    }
}

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
    overlay->vramBase = (s32) mmAlloc(
        overlay->textSize + overlay->dataSize + overlay->bssSize +
            (u16) overlay->relocTableSize,
        0x83);
    D_8007A27C = -1;

    if (overlay->vramBase == 0) {
        return 0;
    }

    if (overlay->relocTableSize2) {
        relocTable = mmAlloc(overlay->relocTableSize2, 0x83);
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
            if (ProcessRelocationEntry(relocEntry, overlayIndex) == 2) {
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
        if (ProcessRelocationEntry(relocEntry, overlayIndex) == 2) {
            relocCount--;
            relocEntry++;
        }
        relocEntry++;
    }

    overlay = overlayTable;
    for (otherIndex = 0; otherIndex < overlayCount; otherIndex++) {
        if (overlay->vramBase != 0 && otherIndex != overlayIndex) {
            if (otherIndex == 0) {
                D_800D2DA8.textBase = (u8 *) amSetMuteMode;
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
                    if (ProcessRelocationEntry(relocEntry, otherIndex) == 2) {
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
        D_800D2DA8.textBase = (u8 *) amSetMuteMode;
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
                D_800D2DA8.textBase = (u8 *) &amSetMuteMode;
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
                    /* The patch operation is read at each use, as JFG does; held in
             * a local, IDO swaps the comparison's operands. */
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
                    ((u8 *) amSetMuteMode + (relocEntry->u.info >> 8));
            }

            /* The patch operation is read at each use, as JFG does; held in
             * a local, IDO swaps the comparison's operands. */
            if (((u32) relocEntry->u.b.flags >> 4) == RELOC_TYPE_26) {
                address = (u32) TrapDanglingJump;
            } else {
                address = 0;
            }
            PatchInstruction(patchLocation, address, (u32) relocEntry->u.b.flags >> 4);
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

    overlayTable = mmAlloc((u32) (D_184C3E0 - D_184B680) + sizeof(OverlayHeader), 0x83);
    romCopy((u32) D_184B680, (u32) (overlayTable + 1), (u32) (D_184C3E0 - D_184B680));

    overlayRomTable = mmAlloc((u32) (D_184B680 - D_1849730), 0x83);
    romCopy((u32) D_1849730, (u32) overlayRomTable, (u32) (D_184B680 - D_1849730));

    mainRelocTable = mmAlloc((u32) (D_1849730 - D_1848B70), 0x83);
    romCopy((u32) D_1848B70, (u32) mainRelocTable, (u32) (D_1849730 - D_1848B70));
    mainRelocTableCount = *(u32 *) mainRelocTable;
    mainRelocTable = (RelocationEntry *) ((u8 *) mainRelocTable + 4);
    overlayCount = ((u32) (D_184C3E0 - D_184B680) / sizeof(OverlayHeader)) + 1;

    i = PENDING_OVERLAY_LOADS;
    while (i--) {
        D_800D2DC8[i].overlayIndex = 0xFFB;
    }

    linkSlotTable = mmAlloc(overlayCount * sizeof(LinkSlot), 0x83);
    _bzero(linkSlotTable, overlayCount * sizeof(LinkSlot));

    overlayTable->vramBase = (s32) amSetMuteMode;
    overlayTable->romAddress = 0;
    overlayTable->textSize = (u32) runlinkCodeEnd - (u32) amSetMuteMode;
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
                mmAllocAtAddr(overlay->dataSize + overlay->bssSize +
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

        overlay->vramBase = (s32) mmAllocAtAddr(
            overlay->textSize + overlay->dataSize + overlay->bssSize +
                (u16) overlay->relocTableSize,
            (void *) pendingLoad.value->unk0, 0x83);
        if (overlay->vramBase == 0) {
            return;
        }

        if (overlay->relocTableSize2) {
            relocTable = mmAlloc(overlay->relocTableSize2, 0x83);
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
                    ProcessRelocationEntry(relocEntry, overlayIndex) == 2) {
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
                ProcessRelocationEntry(relocEntry, overlayIndex) == 2) {
                relocCount--;
                relocEntry++;
            }
            relocEntry++;
        }

        overlay = overlayTable;
        for (otherIndex = 0; otherIndex < overlayCount; otherIndex++) {
            if (overlay->vramBase != 0 && otherIndex != overlayIndex) {
                if (otherIndex == 0) {
                    D_800D2DA8.textBase = (u8 *) amSetMuteMode;
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
                        if (ProcessRelocationEntry(relocEntry, otherIndex) == 2) {
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
