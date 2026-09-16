/*
 * Save-device and rumble support -- ROM 0x2C8C0-0x2ECA0.
 *
 * PROVENANCE -- the translation-unit identity and the descriptive names used
 * below are adapted from Jet Force Gemini's public decompilation, src/saves.c.
 * Mickey's function order, sizes and call graph establish the correspondence;
 * adapted C bodies are identified in docs/modules.md; all remaining functions
 * stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"
#include "PR/os_pfs.h"
#include "game/saves.h"

s32 packCalculateGlobalFlagsChecksum(u8 *buffer);

extern u8 D_8007A2F8;
extern u8 D_8007A2F0;
extern u8 D_8007A2F4;
extern u8 D_8007A2E4;
extern u8 D_8007A2C8;
extern u8 D_8007A300;
extern s32 D_8007A2E8;
extern s32 D_8007A2EC;
extern s32 D_8007A2FC;
extern s32 D_8007A31C;
extern f32 D_80082088;
extern s32 osTvType;
extern u8 D_8007A284[];
extern u8 D_8007A304[];
extern void *D_8007A280;
extern OSMesgQueue *D_800D21C0;
extern OSPfs D_800D21C8[];
extern u8 D_800CF3B8[];
extern f32 D_8008208C;
s32 osMotorStart(OSPfs *pfs);
s32 osMotorStop(OSPfs *pfs);

typedef struct SavesRecord {
    u8 pad00[0xC];
    s32 unkC;
    s32 unk10;
} SavesRecord;

typedef struct SavesBitWriter {
    s32 size;
    s32 unk04;
    u8 mask;
    u8 pad09[3];
    u8 *cursor;
    u8 *start;
} SavesBitWriter;

typedef struct SavesSlot {
    s32 unk00;
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    s32 unk08;
    u8 unk0C;
    u8 unk0D;
    u8 unk0E;
    u8 unk0F;
    s32 unk10;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    s32 unk18;
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;
} SavesSlot;

typedef struct SavesPackedEntry {
    s32 unk00;
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
} SavesPackedEntry;

typedef struct SavesEepromWriteState {
    s32 messageQueue;
    s32 unused;
} SavesEepromWriteState;

typedef struct SavesEepromReadState {
    s32 messageQueue;
    u8 unused[0x14];
} SavesEepromReadState;

typedef struct SavesGameWriteState {
    u32 footer[2];
    SavesBitWriter *writer;
    s32 messageQueue;
} SavesGameWriteState;

typedef struct RumbleState {
    u8 state;
    u8 pad01;
    u8 status;
    u8 flag;
    s16 strength;
    s16 unk6;
    s16 rumbleTime;
    s16 timer;
    s16 unkC;
} RumbleState;

void mmFree(void *address);
void *func_8002B280();
SavesSlot *func_800291C4(void);
u8 joyGetController(s32 controllerIndex);
extern RumbleState D_800D2368[];
s32 osContStartReadData(OSMesgQueue *messageQueue);
s32 osMotorInit(OSMesgQueue *messageQueue, OSPfs *pfs, s32 channel);
extern s32 packReadFile(s32 controllerIndex, s32 fileNum, u8 *data,
                        s32 dataLength);
void rumbleStop(s32 controllerIndex, s32 arg1);
s32 func_800290A0(void);
s32 packOpen(s32 controllerIndex);
s32 joyMessageQ(void);
s32 func_80070170(s32 messageQueue);
s32 mainResetPressed(void);
s32 frontGetLanguage(void);
s32 func_8006FEF0(s32 arg0, u8 type, void *data, s32 size);
void func_80070030(s32 arg0, u8 arg1, void *arg2, s32 arg3);
s32 func_8002C7EC(s32 arg0, s32 arg1, void *arg2, s32 arg3);
void func_8002CD6C(void);
void func_80058010(void);
void func_800581BC(void);
void mainPreNMI(void);
char *font_codes_to_string(u8 *inString, char *outString, s32 stringLength);
char *string_to_font_codes(char *inString, char *outString, s32 stringLength);
s32 func_8002E020(s32 controllerIndex, s32 fileNum);

/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/saves.c:func_8004B070_4BC70. */
s32 func_8002BCC0(void) {
    s32 result;

    result = D_8007A2F8 != 0;
    if (result != 0) {
        return func_800290A0() == 0;
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleRumbles. */
void rumbleRumbles(s32 value) {
    D_8007A2F8 = value;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:rumbleProcessing, with Mickey's outer enable guard. */
void rumbleProcessing(s32 enabled) {
    if (D_8007A2E8 != 0) {
        if (enabled != 0 && D_8007A2F4 == 0) {
            D_8007A2F0 = 1;
            D_8007A2F4 = 1;
            return;
        }
        D_8007A2F4 = 0;
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:rumbleMax, under Mickey's established rumbleStart name. */
void rumbleStart(s32 controllerIndex, s32 arg1, f32 arg2) {
    RumbleState *rumble;
    s32 temp_f16;
    s32 controllerNum;

    if (func_8002BCC0() != 0) {
        if (controllerIndex >= 0 && controllerIndex < 4) {
            controllerNum = joyGetController(controllerIndex);
            if ((1 << controllerNum) & D_8007A2E8) {
                rumble = &D_800D2368[controllerNum];
                if (rumble->rumbleTime <= 0) {
                    rumble->strength = 0;
                }
                if (arg1 != 0) {
                    arg1 = (s32) ((f32) (arg1 * arg1) * D_80082088);
                    if (rumble->strength < arg1) {
                        rumble->strength = arg1;
                    }
                }
                if (rumble->state != 2) {
                    rumble->state = 1;
                    temp_f16 = (s32) (arg2 * 60.0f);
                    if (rumble->rumbleTime < temp_f16) {
                        rumble->rumbleTime = temp_f16;
                    }
                }
            }
        }
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:rumbleStop, with Mickey's force argument. */
void rumbleStop(s32 controllerIndex, s32 force) {
    s32 controllerNum;
    RumbleState *rumble;

    if (controllerIndex >= 0 && controllerIndex < 4) {
        controllerNum = joyGetController(controllerIndex);
        if (force != 0 ||
            (D_800D2368[controllerNum].state != 0 &&
             D_800D2368[controllerNum].state != 3)) {
            rumble = &D_800D2368[controllerNum];
            rumble->state = 3;
            rumble->flag = 1;
        }
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleKill. */
void rumbleKill(s32 arg0) {
    s32 i = 4;

    while (i--) {
        rumbleStop(i, arg0);
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleUpdate. */
void rumbleUpdate(void) {
    D_8007A2F0 = 1;
}
void func_8002BF54(s32 clearMask, s32 initMask) {
    RumbleState *rumble;
    s32 i;
    s32 controllerMask;

    i = 0;
    controllerMask = 1;
    rumble = D_800D2368;
    do {
        if (clearMask & controllerMask) {
            rumble->state = 0;
            rumble->status = 0;
            rumble->strength = 0;
            rumble->unk6 = 0;
            rumble->rumbleTime = 0;
            rumble->timer = 0;
            rumble->unkC = 30;
            D_8007A2E4 &= ~controllerMask;
        }
        if ((initMask & controllerMask) && (D_8007A300 & controllerMask)) {
            rumble->status = 1;
            if (osMotorInit(D_800D21C0, &D_800D21C8[i], i) == 0) {
                rumble->status |= 2;
                D_8007A2E4 |= controllerMask;
            }
        }
        controllerMask *= 2;
        i++;
        rumble++;
    } while (i != 4);
}
/* PROVENANCE -- the state-machine organization follows Jet Force Gemini's
 * public src/saves.c:rumbleTick; Mickey's fields, helper calls, and retry
 * protocol are taken from its own target assembly and globals. */
void rumbleTick(s32 updateRate) {
    RumbleState *rumble;
    s32 pfsStatus;
    s32 i;
    s32 retryMask;
    s32 previousState;
    s32 controllerMask;

    if (D_8007A2FC != 0) {
        osPfsIsPlug(D_800D21C0, &D_8007A300);
        func_8002BF54(0xF, 0xF);
        if (D_8007A2E4 != 0) {
            D_8007A2E8 = D_8007A2E4;
            D_8007A2F4 = 1;
            D_8007A2F0 = 0;
            D_8007A2EC = 0;
        }
        D_8007A2FC = 0;
    }
    if (D_8007A2F4 != 0) {
        if (D_8007A2F0 != 0) {
            D_8007A2EC = 0;
            D_8007A2F0 = 0;
            osPfsIsPlug(D_800D21C0, &D_8007A300);
            func_8002BF54(0xF, 0xF);
        }
        i = 0;
        retryMask = 0;
        controllerMask = 1;
        rumble = D_800D2368;
        do {
            if (rumble->status & 2) {
                pfsStatus = 0;
                previousState = rumble->state;
                switch (rumble->state) {
                    case 1:
                        rumble->rumbleTime -= updateRate;
                        rumble->timer += updateRate;
                        if (func_8002BCC0() == 0) {
                            rumble->rumbleTime = 0;
                        }
                        if (rumble->rumbleTime <= 0) {
                            rumble->state = 3;
                            rumble->flag = 1;
                        } else if (rumble->timer >= 0x78) {
                            rumble->state = 2;
                            rumble->timer = 0x78;
                            rumble->flag = 1;
                        } else if ((f32)rumble->strength > 490.0f) {
                            if (rumble->pad01 == 0) {
                                pfsStatus = osMotorStart(&D_800D21C8[i]);
                                rumble->pad01 = 1;
                            }
                        } else if ((f32)rumble->strength < D_8008208C) {
                            if (rumble->pad01 != 0) {
                                pfsStatus = osMotorStop(&D_800D21C8[i]);
                                rumble->pad01 = 0;
                            }
                        } else if (rumble->unk6 >= 0x100) {
                            if (rumble->pad01 == 0) {
                                pfsStatus = osMotorStart(&D_800D21C8[i]);
                                rumble->pad01 = 1;
                            }
                            rumble->unk6 -= 0x100;
                        } else {
                            if (rumble->pad01 != 0) {
                                pfsStatus = osMotorStop(&D_800D21C8[i]);
                                rumble->pad01 = 0;
                            }
                            rumble->unk6 += rumble->strength + 4;
                        }
                        break;
                    case 2:
                        if (rumble->flag != 0) {
                            osMotorInit(D_800D21C0, &D_800D21C8[i], i);
                            osMotorStop(&D_800D21C8[i]);
                            osMotorStop(&D_800D21C8[i]);
                            osMotorStop(&D_800D21C8[i]);
                            rumble->rumbleTime = 0;
                            rumble->pad01 = 0;
                            rumble->flag--;
                        }
                        rumble->timer -= updateRate;
                        if (rumble->timer <= 0) {
                            rumble->state = 0;
                            rumble->timer = 0;
                        }
                        break;
                    case 3:
                        osMotorInit(D_800D21C0, &D_800D21C8[i], i);
                        osMotorStop(&D_800D21C8[i]);
                        osMotorStop(&D_800D21C8[i]);
                        osMotorStop(&D_800D21C8[i]);
                        rumble->rumbleTime = 0;
                        rumble->timer = 0;
                        rumble->pad01 = 0;
                        rumble->flag--;
                        if (rumble->flag == 0) {
                            rumble->state = 0;
                        }
                        break;
                    default:
                        if (rumble->unkC != 0) {
                            rumble->unkC -= updateRate;
                            if (rumble->unkC <= 0) {
                                osMotorInit(D_800D21C0, &D_800D21C8[i], i);
                                osMotorStop(&D_800D21C8[i]);
                                osMotorStop(&D_800D21C8[i]);
                                osMotorStop(&D_800D21C8[i]);
                                rumble->unkC = 0;
                                rumble->pad01 = 0;
                            }
                        }
                        break;
                }
                if ((rumble->state == 0) && (previousState != 0)) {
                    rumble->unkC = 0x1E;
                }
                if (pfsStatus != 0) {
                    rumble->status &= ~2;
                    D_8007A2E4 &= ~controllerMask;
                }
            } else if (D_800CF3B8[i] != 0) {
                retryMask |= controllerMask;
            }
            controllerMask *= 2;
            i++;
            rumble++;
        } while (i != 4);
        retryMask &= D_8007A2E8;
        if (retryMask != 0) {
            controllerMask = 1 << (D_8007A2EC + 0x1F);
            if (D_8007A2EC == 0) {
                osPfsIsPlug(D_800D21C0, &D_8007A300);
            } else {
                if (retryMask & controllerMask) {
                    func_8002BF54(controllerMask, controllerMask);
                }
            }
            D_8007A2EC = D_8007A2EC + 1;
        }
        if ((retryMask == 0) || (D_8007A2EC >= 5)) {
            D_8007A2EC = 0;
        }
    }
}
void func_8002C5F4(void) {
    D_8007A2E8 = 0;
    D_8007A2FC = 1;
}
SavesBitWriter *func_8002C60C(s32 size, s32 clear) {
    SavesBitWriter *writer;
    u8 *data;

    writer = func_8002B280(size + 0x14, 0x85, size);
    if (writer == NULL) {
        return NULL;
    }
    data = (u8 *) writer + 0x14;
    writer->size = size;
    writer->mask = 0x80;
    writer->cursor = data;
    writer->start = data;
    if (clear != 0) {
        while (size--) {
            writer->cursor[size] = 0;
        }
    }
    return writer;
}
#ifdef NON_MATCHING
/* Bounded plateau: configured full-TU C is exact-sized and frameless at 8/28
 * words, first +0x18, with eight register-only differences and no relocations.
 *
 * 11 -> 8, lane p7-res: this is a leaf, so globalcolor runs phase two only and
 * the axis is ascending web number with each web taking the lowest free colour.
 * Two independent edits move it, both measured against the full TU:
 *   1. the bit advance belongs at the *end* of the loop body. Eight of the 72
 *      legal orderings of the six statement groups score 9 and every one of
 *      them puts the advance last; the previous body had it in the loop head.
 *   2. the `nextBit` carrier must go. With `bit >>= 1` and `while (bit != 0)`,
 *      `nextBit` stops being a p2 web, the reset constant becomes the first
 *      web coloured after the parameters, and it lands where the target keeps
 *      it. Dropping the carrier costs no instruction here (delta 0), which is
 *      the opposite of what the earlier note recorded for the old body.
 * The `valueBit`/`isSet` copy pair and the `cursorField` hoist are still
 * load-bearing: removing the hoist costs eight bytes.
 *
 * What is left is one ugen ring phase, not an allocator decision. The target
 * draws one more integer temp than this body does before the set test, so the
 * value test, the two store temps and the shifted mask each sit one position
 * further along the ring; the advance itself agrees. The claim recorded here
 * that a full p2 force sweep (every web against every colour and the split
 * path, then a second greedy round) never beats 8, and that the residual
 * therefore sits below globalcolor, is FALSIFIED -- see the p8-close note.
 * Measured flat against it on this body: twelve redundant-mask and cast
 * spellings aimed at a phantom pop, ten reset/store-arm spellings, all six
 * placements of the advance, and 23 L97 region placements.
 *
 * 2026-09-12, lane p8-close: the ring phase is read out of the instrumented
 * ugen and the requirement is now exact rather than descriptive.
 *
 * ugen builds this procedure's GP scratch free list in the fixed order
 * t6 t7 t8 t9 t0 t1 t2 t3 t4 t5, hands them out in that order without reuse,
 * and REMOVES from the list every register globalcolor has coloured -- two
 * other procedures in this file remove t0, and t0 and t1, for exactly that
 * reason. This body draws nine scratch values and so takes the first nine
 * entries; the target draws nine and takes the same list WITH t0 missing.
 *
 * Two edits together are the whole eight words, and neither is worth anything
 * alone. First, the bit advance belongs BEFORE the mask store, not after it:
 * scratch is handed out in source order, so that swap is what puts the
 * advance one entry ahead of the shifted mask, which is the only place the
 * target's list is not a plain shift of ours. Alone it is a regression to 11.
 * Second, the procedure needs one more globalcolor-coloured web at colour 7.
 * With the advance-before-store body, forcing either of this procedure's two
 * invisible webs to colour 7 -- the bitCount parameter's web, or nextCursor's,
 * neither of which shows its colour in the object -- returns an object at ZERO
 * masked words and size delta 0. A full p2 force sweep of eight webs against
 * fourteen colours and the split path puts only those two cells at zero; the
 * cursor web and the reset-constant web at colour 7 reach 3.
 *
 * Why colour 7 is unreachable from source. Phase two colours in ascending web
 * number and each web takes the lowest colour not forbidden, where forbidden
 * means an interfering web that is already assigned or whose register the ABI
 * pins. The 0x80 reset constant is a type-2 constant web; constant webs are
 * numbered above every type-3 symbol web in every source shape measured here,
 * so the constant is always the LAST decision and always takes the highest
 * colour in use. The target keeps it on a3, which is colour 6, so no web
 * decided before it can have colour 6 forbidden, and colour 7 is out of reach.
 * Reopening this needs an invisible web numbered above the constant table, or
 * a second mechanism that removes a register from ugen's scratch list.
 *
 * The prior pass's p2 force sweep is FALSIFIED, and the likely cause is the
 * CDX_PROC trap: CDX_FORCE is silently ignored unless CDX_PROC names the
 * procedure ordinal, and a dropped force returns a byte-identical object that
 * reads exactly like L101's already-forbidden decline. Every force quoted here
 * was run with CDX_PROC set and checked on the record's `forced` field, and a
 * deliberate no-CDX_PROC control reproduces the trap on this very cell. Re-run
 * with the force actually applied, colour 7 on the bitCount web beats 8 on the
 * INCUMBENT body as well, reaching 6 at delta 0. So the residual is not below
 * globalcolor: it is a globalcolor decision this procedure cannot be asked to
 * make.
 *
 * Also measured this pass, none below eight: 6,480 built candidates covering
 * all 180 topological orders of the eight loop-body statement groups crossed
 * with four positions of the reset store inside the reset arm, three set-arm
 * spellings and three local type sets, with no compile failure; eight
 * declaration permutations, which leave the web numbers and colours
 * bit-identical; and five alias families (a hoisted cursor field, a second
 * cursor alias, a mask-field alias, a writer alias, read-only and
 * write-through forms) -- the only ones that do produce a colour-7 web
 * materialise the alias and cost four or eight bytes.
 *
 * The unsigned initial shift is defined for the writer's 1..32-bit count
 * domain (observed direct counts: 4, 5, 18); zero remains a no-op. ORT 727 has
 * five direct callers in func_8002C94C; fallback linkage remains exact. The
 * authorized JFG efd5abb audit found no matching writer body. */
void func_8002C69C(SavesBitWriter *writer, s32 value, s32 bitCount) {
    s32 isSet;
    u32 bit;
    u8 *cursor;
    s32 valueBit;
    u8 *nextCursor;
    u8 **cursorField;
    u32 mask;

    if (bitCount != 0) {
        bit = 1u << (bitCount - 1);
        do {
            mask = writer->mask;
            valueBit = value & bit;
            isSet = valueBit;
            cursorField = &writer->cursor;
            if (mask == 0) {
                writer->cursor = writer->cursor + 1;
                nextCursor = writer->cursor;
                *nextCursor = 0;
                mask = writer->mask = 0x80;
            }
            if (isSet != 0) {
                cursor = *cursorField;
                *cursor |= mask;
                mask = writer->mask;
            }
            writer->mask = (u8) (mask >> 1);
            bit >>= 1;
        } while (bit != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C69C.s")
#endif
void func_8002C70C(reader, value, bitCount)
SavesBitWriter *reader;
s32 *value;
s32 bitCount;
{
    u32 bit;
    u32 shiftedMask;
    u32 mask;

    if (bitCount != 0) {
        *value = 0;
        bit = 1 << (bitCount + 0x1F);
        do {
            mask = reader->mask;
            if (mask == 0) {
                mask = reader->mask = 0x80;
                reader->cursor++;
            }
            shiftedMask = mask;
            if (*reader->cursor & mask) {
                *value |= bit;
                shiftedMask = reader->mask;
            }
            bit >>= 1;
            reader->mask = (u8) (shiftedMask >> 1);
        } while (bit != 0);
    }
}
s32 func_8002C788(SavesRecord *record) {
    return record->unk10;
}
s32 func_8002C790(SavesRecord *record) {
    return record->unkC = record->unk10;
}
void func_8002C79C(void *address) {
    mmFree(address);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packCalculateGameChecksum. */
s32 packCalculateGameChecksum(u8 *buffer, s32 count) {
    s32 checksum = 15;

    while (count--) {
        checksum += *buffer++;
    }
    return checksum;
}
/* Mickey-derived chunked save-device transfer. */
s32 func_8002C7EC(s32 arg0, s32 arg1, void *arg2, s32 arg3) {
    s32 result;
    u8 type;

    D_8007A31C = arg1 & 1;
    result = 0;
    if (arg3 > 0) {
        do {
            type = arg1;
            mainPreNMI();
            if (arg3 >= 0x21) {
                result = func_8006FEF0(arg0, type, arg2, 0x20);
                arg2 = (u8 *) arg2 + 0x20;
                arg3 -= 0x20;
                arg1 += 4;
            } else {
                result = func_8006FEF0(arg0, type, arg2, arg3);
                arg3 = 0;
            }
        } while (arg3 > 0 && result == 0);
    }
    return result;
}
void func_8002C8B4(s32 arg0, s32 arg1, void *arg2, s32 arg3) {
    u8 data[16];

    if (D_8007A31C != 0) {
        if (arg1 & 1) {
            func_8006FEF0(arg0, 0x3E, data, 8);
            D_8007A31C = 0;
        }
    } else if (!(arg1 & 1)) {
        func_8006FEF0(arg0, 0x3F, data, 8);
        D_8007A31C = 1;
    }
    func_80070030(arg0, (u8) arg1, arg2, arg3);
}
/* Mickey-derived serialization of one 0x94-byte save window. */
/* The `if (1)` block around the entry init is a register-scheduling nudge
 * needed to match the callee-saved slot/counter tie-break (permuter); tracked
 * in docs/cleanup-queue.md. */
void func_8002C94C(s32 saveIndex) {
    SavesBitWriter *writer;
    s32 inner;
    SavesSlot *slot;
    s32 outer;
    SavesPackedEntry *entry;
    u8 *buffer;
    s32 messageQueue;
    s32 byteOffset;
    s32 firstBlock;
    s32 slotCount;
    u32 footer[2];

    messageQueue = joyMessageQ();
    if (func_80070170(messageQueue) != 0) {
        slot = func_800291C4();
        writer = func_8002C60C(0x1C0, 1);
        outer = 0;
        slotCount = 0x18;
        do {
            inner = 0;
            if (1) {
                entry = (SavesPackedEntry *) slot;
            }
            do {
                func_8002C69C(writer, entry->unk04, 5);
                func_8002C69C(writer, entry->unk05, 5);
                func_8002C69C(writer, entry->unk06, 5);
                func_8002C69C(writer, entry->unk00 / 3, 0x12);
                func_8002C69C(writer, entry->unk07, 4);
                inner += sizeof(SavesPackedEntry);
                entry++;
            } while (inner != sizeof(SavesSlot));
            outer++;
            slot++;
        } while (outer != slotCount);

        buffer = (u8 *) func_8002C788((SavesRecord *) writer);
        footer[0] = packCalculateGameChecksum(buffer, 0x1C0);
        footer[1] = 0x12345678;
        if (mainResetPressed() == 0) {
            byteOffset = saveIndex * 0x94;
            firstBlock = byteOffset >> 6;
            func_8002C8B4(messageQueue, firstBlock,
                          buffer + (firstBlock * 8),
                          (((((byteOffset + 0x93) >> 6) - firstBlock) * 8) +
                           8));
            func_8002C8B4(messageQueue, 0x38, footer, sizeof(footer));
        }
        func_8002C79C(writer);
    }
}
/* Mickey-derived reconstruction of the serialized save-window loader. */
void func_8002CB18(void) {
    s32 checksum;
    s32 inner;
    s32 outer;
    s32 value;
    s32 messageQueue;
    SavesBitWriter *reader;
    SavesSlot *slot;
    SavesPackedEntry *entry;

    messageQueue = joyMessageQ();
    if (func_80070170(messageQueue) == 0) {
        func_8002CD6C();
        return;
    }

    slot = func_800291C4();
    reader = func_8002C60C(0x1C0, 0);
    func_8002C7EC(messageQueue, 0,
                  (void *) func_8002C788((SavesRecord *) reader), 0x1C0);
    outer = 0;
    do {
        inner = 0;
        entry = (SavesPackedEntry *) slot;
        do {
            func_8002C70C(reader, &value, 5);
            entry->unk04 = value;
            func_8002C70C(reader, &value, 5);
            entry->unk05 = value;
            func_8002C70C(reader, &value, 5);
            entry->unk06 = value;
            func_8002C70C(reader, &value, 0x12);
            entry->unk00 = value * 3;
            func_8002C70C(reader, &value, 4U);
            entry->unk07 = value;
            inner++;
            entry++;
        } while (inner != 4);
        outer++;
        slot++;
    } while (outer != 0x18);

    checksum = packCalculateGameChecksum(
        (u8 *) func_8002C788((SavesRecord *) reader), 0x1C0);
    {
        u32 footer[2];

        func_8002C7EC(messageQueue, 0x38, footer, sizeof(footer));
        if (checksum != footer[0] || footer[1] != 0x12345678) {
            func_8002CD6C();
        }
    }
    func_8002C79C(reader);
}
void func_8002CCE4(void) {
    s32 i;
    s32 limit;
    SavesSlot *slot;

    slot = func_800291C4();
    i = 0;
    limit = 0x18;
    do {
        i++;
        slot->unk0C = 0;
        slot->unk0D = 0;
        slot->unk0E = 0;
        slot->unk08 = 0;
        slot->unk0F = 0;
        slot->unk14 = 0;
        slot->unk15 = 0;
        slot->unk16 = 0;
        slot->unk10 = 0;
        slot->unk17 = 0;
        slot->unk1C = 0;
        slot->unk1D = 0;
        slot->unk1E = 0;
        slot->unk18 = 0;
        slot->unk1F = 0;
        slot->unk04 = 0;
        slot->unk05 = 0;
        slot->unk06 = 0;
        slot->unk00 = 0;
        slot->unk07 = 0;
        slot++;
    } while (i != limit);
}
/* Mickey-derived reconstruction; JFG supplies only the neighboring saves TU
 * order, while Mickey's writer/checksum/I/O calls establish this body. */
void func_8002CD6C(void) {
    SavesGameWriteState state;

    state.writer = func_8002C60C(0x1C0, 1);
    func_8002CCE4();
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) == 0) {
        func_8002C79C(state.writer);
        return;
    }
    state.footer[0] = packCalculateGameChecksum(
        (u8 *) func_8002C788((SavesRecord *) state.writer), 0x1C0);
    state.footer[1] = 0x12345678;
    if (mainResetPressed() == 0) {
        func_8002C8B4(state.messageQueue, 0,
                      (void *) func_8002C788((SavesRecord *) state.writer),
                      0x1C0);
        func_8002C8B4(state.messageQueue, 0x38, state.footer,
                      sizeof(state.footer));
    }
    func_8002C79C(state.writer);
}
/* Mickey-derived checksum body; the surrounding save path establishes its role. */
s32 packCalculateGlobalFlagsChecksum(buffer)
u8 *buffer;
{
    s32 bytesToChecksum = 22;
    s32 checksum = 5;

    while (bytesToChecksum--) {
        checksum += *buffer++;
    }
    return checksum;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packLoadGlobalFlagsEprom, using Mickey's record and I/O path. */
void func_8002CE54(void *globalFlags) {
    s32 failed;
    SavesEepromReadState state;
    u8 *src;
    u8 *dst;
    s32 bytesToCopy;

    failed = 0;
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) == 0) {
        failed = 1;
    }
    if (failed == 0 &&
        func_8002C7EC(state.messageQueue, 0x39, globalFlags, 0x18) != 0) {
        failed = 1;
    }
    if (packCalculateGlobalFlagsChecksum(globalFlags) !=
        *(u16 *) ((u8 *) globalFlags + 0x16)) {
        failed = 1;
    }
    src = D_8007A304;
    dst = globalFlags;
    bytesToCopy = 0x17;
    if (failed != 0) {
        do {
            *dst++ = *src++;
        } while (bytesToCopy--);
        *(u16 *) ((u8 *) globalFlags + 0x16) =
            packCalculateGlobalFlagsChecksum(globalFlags);
    }
}
/* Mickey-derived reconstruction; JFG supplies only the neighboring TU order. */
void func_8002CF0C(void *globalFlags) {
    SavesEepromWriteState state;

    *(s16 *) ((u8 *) globalFlags + 0x16) =
        packCalculateGlobalFlagsChecksum(globalFlags);
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) != 0 && mainResetPressed() == 0) {
        func_8002C8B4(state.messageQueue, 0x39, globalFlags, 0x18);
    }
}
#ifdef NON_MATCHING
/* Policy-clean configured V0 is 85/88 instructions, 10/88 positional words,
 * frame 0x30, first +0x0. All 11 relocation identities are present, but their
 * offsets drift with the shorter body. The complete 119-configuration lattice
 * is nonexact; -O2 -g3 reaches 86 instructions but not the target structure.
 * One allocator trace maps the function to procedure 26: globalFlags/stateBuffer
 * occupy s0/s1 while the saved-byte/flag webs occupy a2/a3. Moving those two
 * saved scalars into their natural lexical scope restores frame 0x48 and gives
 * the retained 85-instruction, 11/88-word result. A saved-header lifetime
 * regresses to 83 instructions, so no combination or generic batch is allowed.
 * ORT 505 and sole caller joyRead+0x130 remain authenticated. Assembly fallback
 * is canonical; no padded state, false checksum arguments, dead carrier, or
 * volatile allocation scaffold is retained. */
void func_8002CF6C(u8 *globalFlags) {
    s32 stateMessageQueue;
    u8 *stateBuffer;
    s32 messageQueue;
    u8 *allocatedBuffer;
    u8 *footerBuffer;
    u8 *src;
    u8 *dst;
    s32 count;

    messageQueue = joyMessageQ();
    stateMessageQueue = messageQueue;
    if (func_80070170(messageQueue) != 0) {
        allocatedBuffer = func_8002B280(0x200, 0x85);
        stateBuffer = allocatedBuffer;
        if (allocatedBuffer != NULL) {
            s32 savedByte;
            u32 savedFlag;

            dst = allocatedBuffer;
            count = 0x1FF;
            do {
                *dst++ = 0;
            } while (count--);
            func_8002CCE4();
            count = packCalculateGameChecksum(stateBuffer, 0x1C0);
            footerBuffer = stateBuffer;
            *(u32 *) (footerBuffer + 0x1C0) = count;
            *(u32 *) (footerBuffer + 0x1C4) = 0x12345678;
            savedByte = (s8) globalFlags[3];
            savedFlag = (u32) (*(u16 *) globalFlags << 17) >> 31;
            src = D_8007A304;
            dst = globalFlags;
            count = 0x17;
            do {
                *dst++ = *src++;
            } while (count--);
            *(u16 *) (globalFlags + 0x16) =
                packCalculateGlobalFlagsChecksum(globalFlags);
            globalFlags[0] =
                ((savedFlag << 6) & 0x40) |
                (globalFlags[0] & ~0x40);
            globalFlags[3] = savedByte;

            src = globalFlags;
            dst = stateBuffer + 0x1C8;
            count = 0x17;
            do {
                *dst++ = *src++;
            } while (count--);
            count = mainResetPressed();
            allocatedBuffer = stateBuffer;
            if (count == 0) {
                func_8002C8B4(stateMessageQueue, 0, allocatedBuffer, 0x200);
            }
            mmFree(allocatedBuffer);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF6C.s")
#endif
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packOpen, with Mickey's globals and status values. */
s32 packOpen(s32 controllerIndex) {
    OSMesg unusedMessage;
    s32 ret;
    s32 bytesNotUsed;
    s32 i;

    if (D_800D21C0->validCount == 0) {
        if (osMotorInit(D_800D21C0, &D_800D21C8[controllerIndex],
                        controllerIndex) == 0) {
            return 8;
        }
    }

    i = 0;
    while (D_800D21C0->validCount != 0 && i < 10) {
        osRecvMesg(D_800D21C0, &unusedMessage, OS_MESG_NOBLOCK);
        i++;
    }

    for (i = 0; i <= 4; i++) {
        ret = osPfsFreeBlocks(&D_800D21C8[controllerIndex], &bytesNotUsed);
        if (ret == PFS_ERR_INVALID) {
            ret = osPfsInit(D_800D21C0, &D_800D21C8[controllerIndex],
                            controllerIndex);
        }
        if (ret == PFS_ERR_ID_FATAL) {
            if (osMotorInit(D_800D21C0, &D_800D21C8[controllerIndex],
                            controllerIndex) == 0) {
                return 8;
            }
        }
        if (ret == PFS_ERR_NEW_PACK) {
            if (osPfsInit(D_800D21C0, &D_800D21C8[controllerIndex],
                          controllerIndex) == PFS_ERR_ID_FATAL &&
                osMotorInit(D_800D21C0, &D_800D21C8[controllerIndex],
                            controllerIndex) == 0) {
                return 8;
            }
            return 5;
        }
        if (ret == PFS_ERR_NOPACK || ret == PFS_ERR_DEVICE) {
            return 1;
        }
        if (ret == PFS_ERR_BAD_DATA) {
            return 6;
        }
        if (ret == PFS_ERR_ID_FATAL) {
            return 3;
        }
        if (ret == PFS_ERR_INCONSISTENT) {
            return 2;
        }
        if (ret == 0) {
            return 0;
        }
    }

    return 1;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packClose. */
s32 packClose(s32 controllerIndex) {
    osContStartReadData(D_800D21C0);
    return 0;
}
/* Matched 2026-09-16 (lane nx-b), 29 -> 0 masked words at delta 0 in one
 * measured batch of six cells, no colour force. The whole residual was one
 * ten-register ring rotation from the pak-pattern test onward: the target
 * spends one ring draw more before that load (L149). `pakPattern` is a
 * memory `u8`, so masking it again before the bit test draws a temp for an
 * `andi` that as1 folds into the `lbu` with the draw still spent; every
 * later temp then sits one ring position later, which is the target. A cast
 * to `u8` or `u32` is a no-op and inert; masking the bit instead adds a
 * word; the unread `osPfsIsPlug` result and DKR's nested `ret` form with its
 * empty `if` are inert or worse. The folded preheader line, the bit
 * recurrence before the index increment and the indexed motor-success
 * status address are lane g1's and still load-bearing. */
/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/save_data.c:init_controller_paks, with Mickey's globals and helpers. */
void packInit(void) {
    s32 controllerIndex;
    s32 ret;
    u8 controllerBit;
    u8 pakPattern;
    s8 maxControllers;
    RumbleState *rumble;

    D_800D21C0 = (OSMesgQueue *) joyMessageQ();
    rumble = D_800D2368;
    controllerIndex = 3;
    do {
        rumble->state = 0;
        rumble->status = 0;
        rumble->pad01 = 0;
        rumble->flag = 0;
        rumble->strength = 0;
        rumble->rumbleTime = 0;
        rumble->timer = 0;
        rumble->unkC = 0;
        rumble++;
    } while (controllerIndex--);
    D_8007A2E4 = 0;
    D_8007A2C8 = 0;
    osPfsIsPlug(D_800D21C0, &pakPattern);

    controllerIndex = 0; controllerBit = 1; maxControllers = 4; do { if ((pakPattern & 0xFF) & controllerBit) {
    /* Keep the preheader grouped for the PFS-base scheduling tie. */



            rumble = &D_800D2368[controllerIndex];
            ret = osPfsInit(D_800D21C0, &D_800D21C8[controllerIndex],
                            controllerIndex);
            if (ret == PFS_ERR_NEW_PACK) {
                ret = osPfsInit(D_800D21C0, &D_800D21C8[controllerIndex],
                                controllerIndex);
            }
            rumble->status |= 1;
            if (ret == 0) {
                D_8007A2C8 |= controllerBit;
            } else if (ret == PFS_ERR_ID_FATAL &&
                       osMotorInit(D_800D21C0,
                                   &D_800D21C8[controllerIndex],
                                   controllerIndex) == 0) {
                D_8007A2E4 |= controllerBit;
                D_800D2368[controllerIndex].status |= 2;
            }
        }
        controllerBit *= 2;
        controllerIndex++;
    } while ((0, controllerIndex) != maxControllers);
    func_80058010();
    func_800581BC();
    osContStartReadData(D_800D21C0);
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packIsPresent. */
s32 packIsPresent(s32 controllerIndex) {
    s32 ret;

    ret = packOpen(controllerIndex);
    packClose(controllerIndex);
    if (ret == 8) {
        D_8007A2E4 |= 1 << controllerIndex;
    }
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packDirectory, with Mickey's globals, status values, and file
 * classifier. */
s32 packDirectory(s32 controllerIndex, s32 maxNumOfFilesToGet,
                  char **fileNames, char **fileExtensions, u32 *fileSizes,
                  u8 *fileTypes) {
    OSPfsState state;
    s32 ret;
    s32 maxNumOfFilesOnCpak;
    s32 filesUsed;
    s8 *directory;
    s32 i;
    u32 gameCode;

    ret = packOpen(controllerIndex);
    if (ret != 0) {
        packClose(controllerIndex);
        return ret;
    }

    if (osPfsNumFiles(&D_800D21C8[controllerIndex],
                      &maxNumOfFilesOnCpak, &filesUsed) != 0) {
        packClose(controllerIndex);
        return 6;
    }

    if (frontGetLanguage() == 5) {
        gameCode = 0x4E44594A;
    } else if (osTvType == 0) {
        gameCode = 0x4E445950;
    } else {
        gameCode = 0x4E445945;
    }

    if (maxNumOfFilesToGet < maxNumOfFilesOnCpak) {
        maxNumOfFilesOnCpak = maxNumOfFilesToGet;
    }

    if (D_8007A280 != NULL) {
        mmFree(D_8007A280);
    }

    filesUsed = maxNumOfFilesOnCpak * 24;
    D_8007A280 = func_8002B280(filesUsed, 0xFF);
    _bzero(D_8007A280, filesUsed);
    directory = D_8007A280;

    for (i = 0; i < maxNumOfFilesOnCpak; i++) {
        fileNames[i] = (char *) directory;
        directory += 0x12;
        fileExtensions[i] = (char *) directory;
        fileSizes[i] = 0;
        fileTypes[i] = 0xFF;
        directory += 6;
    }

    while (i < maxNumOfFilesToGet) {
        fileExtensions[i] = NULL;
        fileNames[i] = NULL;
        fileSizes[i] = 0;
        fileTypes[i] = 0xFF;
        i++;
    }

    for (i = 0; i < maxNumOfFilesOnCpak; i++) {
        ret = osPfsFileState(&D_800D21C8[controllerIndex], i, &state);
        if (ret == PFS_ERR_INVALID) {
            fileNames[i] = NULL;
            continue;
        }
        if (ret != 0) {
            packClose(controllerIndex);
            return 6;
        }

        font_codes_to_string((u8 *) &state.game_name, fileNames[i],
                             PFS_FILE_NAME_LEN);
        font_codes_to_string((u8 *) &state.ext_name, fileExtensions[i],
                             PFS_FILE_EXT_LEN);
        fileSizes[i] = state.file_size;
        fileTypes[i] = 1;
        if (state.game_code == gameCode && state.company_code == 0x3459) {
            fileTypes[i] = func_8002E020(controllerIndex, i);
        }
    }

    packClose(controllerIndex);
    return 0;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packDirectoryFree. */
void packDirectoryFree(void) {
    if (D_8007A280 != NULL) {
        mmFree(D_8007A280);
    }
    D_8007A280 = NULL;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packFreeSpace, with Mickey's status constants. */
s32 packFreeSpace(s32 controllerIndex, u32 *bytesFree, s32 *notesFree) {
    s32 ret;
    s32 bytesNotUsed;
    s32 maxNotes;
    s32 notesUsed;

    ret = packOpen(controllerIndex);
    if (ret == 0) {
        if (bytesFree != NULL) {
            ret = osPfsFreeBlocks(&D_800D21C8[controllerIndex], &bytesNotUsed);
            if (ret != 0) {
                packClose(controllerIndex);
                return 6;
            }
            *bytesFree = bytesNotUsed;
        }
        if (notesFree != NULL) {
            ret = osPfsNumFiles(&D_800D21C8[controllerIndex], &maxNotes,
                                &notesUsed);
            if (ret != 0) {
                packClose(controllerIndex);
                return 6;
            }
            if (notesUsed >= 16) {
                *notesFree = 0;
            } else {
                *notesFree = 16 - notesUsed;
            }
        }
    }

    packClose(controllerIndex);
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packDeleteFile. */
s32 packDeleteFile(s32 controllerIndex, s32 fileNum) {
    OSPfsState state;
    s32 ret;

    ret = packOpen(controllerIndex);
    if (ret != 0) {
        packClose(controllerIndex);
        return ret;
    }
    ret = 6;
    if (osPfsFileState(&D_800D21C8[controllerIndex], fileNum, &state) == 0) {
        if (osPfsDeleteFile(&D_800D21C8[controllerIndex], state.company_code,
                            state.game_code, (u8 *) state.game_name,
                            (u8 *) state.ext_name) == 0) {
            ret = 0;
        }
    }
    packClose(controllerIndex);
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packOpenFile, with Mickey's language helper and game codes. */
s32 packOpenFile(s32 controllerIndex, char *fileName, char *fileExt,
                 s32 *fileNumber) {
    u32 gameCode;
    char fileNameAsFontCodes[PFS_FILE_NAME_LEN];
    s32 pad;
    char fileExtAsFontCodes[PFS_FILE_EXT_LEN];
    s32 pad2;
    s32 ret;

    string_to_font_codes(fileName, fileNameAsFontCodes, PFS_FILE_NAME_LEN);
    string_to_font_codes(fileExt, fileExtAsFontCodes, PFS_FILE_EXT_LEN);

    if (frontGetLanguage() == 5) {
        gameCode = 0x4E44594A;
    } else if (osTvType == 0) {
        gameCode = 0x4E445950;
    } else {
        gameCode = 0x4E445945;
    }

    ret = osPfsFindFile(&D_800D21C8[controllerIndex], 0x3459, gameCode,
                        (u8 *) fileNameAsFontCodes,
                        (u8 *) fileExtAsFontCodes, fileNumber);
    if (ret == 0) {
        return 0;
    }
    if (ret == PFS_ERR_NOPACK || ret == PFS_ERR_DEVICE) {
        return 1;
    }
    if (ret == PFS_ERR_INCONSISTENT) {
        return 2;
    }
    if (ret == PFS_ERR_ID_FATAL) {
        return 3;
    }
    if (ret == PFS_ERR_INVALID) {
        return 5;
    }
    return 6;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packReadFile. */
s32 packReadFile(s32 controllerIndex, s32 fileNum, u8 *data, s32 dataLength) {
    s32 readResult;

    readResult = osPfsReadWriteFile(&D_800D21C8[controllerIndex], fileNum,
                                    PFS_READ, 0, dataLength, data);
    if (readResult == 0) {
        return 0;
    }
    if (readResult == PFS_ERR_NOPACK || readResult == PFS_ERR_DEVICE) {
        return 1;
    }
    if (readResult == PFS_ERR_INCONSISTENT) {
        return 2;
    }
    if (readResult == PFS_ERR_ID_FATAL) {
        return 3;
    }
    if (readResult == PFS_ERR_INVALID) {
        return 5;
    }
    return 6;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packWriteFile, with Mickey's status values and game codes. */
s32 packWriteFile(s32 controllerIndex, s32 fileNumber, char *fileName,
                  char *fileExt, u8 *dataToWrite, s32 fileSize) {
    s32 temp;
    u8 fileNameAsFontCodes[PFS_FILE_NAME_LEN];
    s32 pad;
    u8 fileExtAsFontCodes[PFS_FILE_EXT_LEN];
    s32 ret;
    s32 file_number;
    s32 bytesToSave;
    u32 gameCode;

    ret = packOpen(controllerIndex);
    if (ret != 0) {
        packClose(controllerIndex);
        return ret;
    }

    bytesToSave = fileSize;
    temp = fileSize & 0xFF;
    if (temp != 0) {
        bytesToSave = fileSize - temp + 0x100;
    }

    string_to_font_codes(fileName, (char *) fileNameAsFontCodes,
                         PFS_FILE_NAME_LEN);
    string_to_font_codes(fileExt, (char *) fileExtAsFontCodes,
                         PFS_FILE_EXT_LEN);

    if (frontGetLanguage() == 5) {
        gameCode = 0x4E44594A;
    } else if (osTvType == 0) {
        gameCode = 0x4E445950;
    } else {
        gameCode = 0x4E445945;
    }

    ret = packOpenFile(controllerIndex, fileName, fileExt, &file_number);
    if (ret == 0) {
        if (fileNumber != -1 && fileNumber != file_number) {
            ret = 6;
        }
    } else if (ret == 5) {
        if (fileNumber != -1) {
            ret = 6;
        } else {
            temp = osPfsAllocateFile(&D_800D21C8[controllerIndex], 0x3459,
                                     gameCode, fileNameAsFontCodes,
                                     fileExtAsFontCodes, bytesToSave,
                                     &file_number);
            if (temp == 0) {
                ret = 0;
            } else if (temp == 7 || temp == 8) {
                ret = 4;
            } else {
                ret = 6;
            }
        }
    }

    if (ret == 0) {
        temp = osPfsReadWriteFile(&D_800D21C8[controllerIndex], file_number,
                                  PFS_WRITE, 0, bytesToSave, dataToWrite);
        if (temp == 0) {
            ret = 0;
        } else if (temp == PFS_ERR_NOPACK || temp == PFS_ERR_DEVICE) {
            ret = 1;
        } else if (temp == PFS_ERR_INCONSISTENT) {
            ret = 2;
        } else if (temp == PFS_ERR_ID_FATAL) {
            ret = 3;
        } else {
            ret = 6;
        }
    }

    packClose(controllerIndex);
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packFileSize. */
s32 packFileSize(s32 controllerIndex, s32 fileNum, s32 *fileSize) {
    OSPfsState state;

    *fileSize = 0;
    if (osPfsFileState(&D_800D21C8[controllerIndex], fileNum, &state) == 0) {
        *fileSize = state.file_size;
        return 0;
    }
    return 6;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:font_codes_to_string. */
char *font_codes_to_string(u8 *inString, char *outString, s32 stringLength) {
    s32 remainder;
    s32 index = *inString;
    s32 roundedLength;
    s32 peel;
    char *ret = outString;

    roundedLength = 66;
    while (index != 0 && stringLength != 0) {
        if (index < roundedLength) {
            *outString = D_8007A284[index];
            outString++;
        } else {
            *outString = '-';
            outString++;
        }
        inString++;
        stringLength--;
        index = *inString;
    }
    remainder = stringLength & 3;
    if (stringLength != 0) {
        if ((peel = -remainder) != 0) {
            roundedLength = peel + stringLength;
            do {
                stringLength--;
                *outString++ = 0;
            } while (roundedLength != stringLength);
            if (stringLength == 0) {
                goto done;
            }
        }
        do {
            stringLength -= 4;
            outString[0] = 0;
            outString[1] = 0;
            outString[2] = 0;
            outString[3] = 0;
            outString += 4;
        } while (stringLength != 0);
    }
done:
    *outString = 0;
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:string_to_font_codes. */
char *string_to_font_codes(char *inString, char *outString, s32 stringLength) {
    s32 remainder;
    s32 i;
    char currentChar;
    s32 paddingLength;
    char *ret;
    s32 peel;
    s32 roundedLength;

    ret = outString;
    while (*inString != 0 && stringLength != 0) {
        *outString = 0;
        for (i = 0; i < 65; i++) {
            currentChar = *inString;
            if (currentChar == D_8007A284[i]) {
                *outString = i;
                outString++;
                break;
            }
        }
        inString++;
        stringLength--;
    }
    remainder = stringLength & 3;
    if (stringLength != 0) {
        peel = -remainder;
        roundedLength = peel + stringLength;
        if (peel == 0) {
            goto bulk_font_codes;
        }
        do {
            stringLength--;
            *outString++ = 0;
        } while (roundedLength != stringLength);
        if (stringLength != 0) {
bulk_font_codes:
            do {
                stringLength -= 4;
                outString[0] = 0;
                outString[1] = 0;
                outString[2] = 0;
                outString[3] = 0;
                outString += 4;
            } while (stringLength != 0);
        }
    }
    *outString = 0;
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packGetFileType, while retaining Mickey's placeholder name. */
s32 func_8002E020(s32 controllerIndex, s32 fileNum) {
    s32 *data;
    s32 pad;
    s32 result = 1;

    data = func_8002B280(0x100, 0xFF);
    if (packReadFile(controllerIndex, fileNum, (u8 *) data, 0x100) == 0) {
        if (*data == 0x43484152) {
            result = 0;
        } else {
            result = 1;
        }
    } else {
        result = 1;
    }
    mmFree(data);
    return result;
}

/* PLATEAU-HANDOFF:func_8002CF6C:start
 * symbol: func_8002CF6C
 * score: 77/88 words
 * frame: 0x48
 * relocations: 11
 * first-mismatch: +0x8
 * summary: Proc-26 census confirms 10 draws; the 3-word extent deficit and buffer structure remain unresolved without a natural stack-home mechanism.
 * PLATEAU-HANDOFF:func_8002CF6C:end
 */

/* PLATEAU-HANDOFF:func_8002C69C:start
 * symbol: func_8002C69C
 * score: 8/28 words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x18
 * summary: Exhaustive 12-colour footprint is flat; test an invisible later integer constant.
 * PLATEAU-HANDOFF:func_8002C69C:end
 */
