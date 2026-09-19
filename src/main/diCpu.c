/*
 * CPU exception/debug monitor -- ROM 0x465B0-0x47A60 (VRAM 0x800459B0).
 *
 * PROVENANCE: the translation-unit identity and descriptive names are
 * adapted from Jet Force Gemini's public decompilation, src/diCpu.c. The
 * Tier-A diCpuTraceInit match, debug strings, direct callers and function
 * order establish the correspondence. JFG address-placeholder names are not
 * imported. The bodies remain Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "game/memory.h"
#include "libc/stdarg.h"

typedef struct {
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[4];
    u32 words[68];
} MickeyEpcInfo;

extern s32 D_8007CFD8;
extern s32 D_8007CFDC;
extern u32 D_8007CFD0;
extern OSThread diCpuOSThread;
extern u64 diCpuThreadStack[];
extern f32 D_80083DBC;
extern OSMesgQueue D_800D5CD0;
extern OSMesg D_800D5CE8[8];
extern OSMesg D_800D5D08[8];
extern OSMesgQueue D_800D5D28;
extern void func_8004D5E0(s32 priority, OSMesgQueue *queue, OSMesg *messages,
                          s32 count);
extern void osViSetSpecialFeatures(u32 features);
extern void func_80045CAC(void);
extern s32 joyGetPressed(s32 controller);
extern void joyRead(s32 updateRate, s32 controllers);
extern void osWritebackDCacheAll(void);
extern s32 viGetVideoMode(void);
extern void viGetCurrentSize(s32 *width, s32 *height);
extern s32 runlinkGetAddressInfo(u32 address, s32 *moduleId,
                                 s32 *moduleAddress, u32 **outputAddress);
extern void render_epc_lock_up_display(MickeyEpcInfo *arg0);
extern void cpuXYPrintf(s32 x, s32 y, const char *format, ...);
extern void func_80046E00(void);
extern void func_80046BCC(s32 x, s32 y, char *text);
extern s32 vsprintf(char *text, const char *format, va_list args);
extern u16 D_8007D034[];
extern s32 func_80005820(s32 arg0);
extern s32 levelGetLevel(void);
extern s32 D_80000310;
extern s32 D_8007A210;
extern s32 D_8007A218;
extern s32 D_8007A21C;
extern s32 D_8007A220[];
extern s32 D_8007A1E0;
extern s32 D_8007A200;
extern s32 D_8007CFE0;
extern s32 D_8007CFE4;
extern s32 D_8007CFE8;
extern s32 D_8007CFEC[];
extern s32 D_8007D02C;
extern s32 D_8007D030;
extern u16 D_8007D2F0[];
extern u16 D_8007D2F8[];
extern u16 D_8007D300[];
extern char D_80083A80;
extern char D_80083A88;
extern char D_80083B2C[];
extern char D_80083B48[];
extern char D_80083B5C[];
extern char D_80083B78[];
extern char D_80083B84[];
extern char D_80083B90[];
extern char D_80083BA0[];
extern char D_80083BAC[];
extern char D_80083BB8[];
extern char D_80083BC8[];
extern char D_80083BE4[];
extern char D_80083BF4[];
extern char D_80083C04[];
extern char D_80083C0C[];
extern char D_80083C14[];
extern char D_80083C1C[];
extern char D_80083C28[];
extern char D_80083C38[];
extern char D_80083C48[];
extern char D_80083C58[];
extern char D_80083C68[];
extern char D_80083C7C[];
extern char D_80083C90[];
extern char D_80083CA4[];
extern char D_80083CB8[];
extern char D_80083CCC[];
extern char D_80083CE0[];
extern char D_80083CF4[];
extern char D_80083D08[];
extern char D_80083D1C[];
extern char D_80083D30[];
extern char D_80083D44[];
extern char D_80083D58[];
extern char D_80083D6C[];
extern char D_80083D80[];
extern char D_80083D8C[];
extern char D_80083D9C[];
extern void *D_800D5D40;
extern u8 D_800D5D48[];
extern s32 D_800D5DF0[];
extern s32 D_800D5E98[];
extern s32 D_800D5F40[];
extern s32 D_800D21B0;
extern s16 *D_800D2FA8;
extern s32 packWriteFile(s32 controllerIndex, s32 fileNumber, char *fileName,
                         char *fileExt, u8 *dataToWrite, s32 fileSize);

typedef struct {
    u8 pad0[0x44];
    s16 unk44;
} EpcDebugObject;

extern EpcDebugObject *D_8007A214;
void stop_all_threads_except_main(void);
void diCpuThread(void *unused);
void func_80045BBC(OSThread *thread);
void func_80045D34();

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceInit. */
void diCpuTraceInit(void) {
    osCreateThread(&diCpuOSThread, 0, diCpuThread, 0, diCpuThreadStack, 0xFF);
    osStartThread(&diCpuOSThread);
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuThread; Mickey's own
 * draft supplies its extra VI delay loop and exact event handling. */
void diCpuThread(void *unused) {
    s32 events;
    register s32 i;
    OSMesg message;
    register f32 divisor;
    register f32 sum;

    events = 0;
    osCreateMesgQueue(&D_800D5CD0, D_800D5CE8, 8);
    osSetEventMesg(12, &D_800D5CD0, (OSMesg)8);
    osSetEventMesg(10, &D_800D5CD0, (OSMesg)2);
    func_8004D5E0(150, &D_800D5D28, D_800D5D08, 8);
    for (divisor = D_80083DBC; ; ) {
        osRecvMesg(&D_800D5CD0, &message, OS_MESG_BLOCK);
        events |= (s32)message;
        if ((events & 8) || (events & 2)) {
            osViSetSpecialFeatures(0xA2);
            sum = 0.0f;
            i = 1000000;
            while (i--) {
                sum += (f32)i / divisor;
            }
            events &= ~8;
            stop_all_threads_except_main();
            func_80045CAC();
        }
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::stop_all_threads_except_main. */
void stop_all_threads_except_main(void) {
    OSThread *thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > OS_PRIORITY_IDLE && thread->priority < 128) {
            osStopThread(thread);
        }
        thread = thread->tlnext;
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_800676F8. That JFG
 * source is a disabled, assembly-backed structural draft rather than an
 * exact-C donor; Mickey's bytes remain authoritative. Mickey's target fixes
 * the dump-size calculation to use the copied range. */
void func_80045BBC(OSThread *thread) {
    s32 copySize;
    void *source;
    register u8 *destination;
    s32 writeSize;

    *(s32 *)0x80705014 = D_8007CFE8;
    *(s32 *)0x80705018 = D_8007CFE0;
    *(s32 *)0x8070501C = D_8007CFE4;
    destination = (u8 *)0x80705094;
    _bcopy(thread, destination, 0x230);
    destination += 0x200;
    /* The saved SP is a 64-bit context field; the N64 pointer uses its low word. */
    source = (void *)(u32)*(u64 *)&thread->context[0xD0];
    copySize = 0x200;
    _bcopy(source, destination, copySize);
    D_800D5D40 = source;
    _bcopy(source, D_800D5D48, copySize);
    destination += 0x200;
    writeSize = (s32)destination + 0x7F900000;
    if (writeSize & 0x1F) {
        writeSize = (writeSize & ~0x1F) + 0x20;
    }
    packWriteFile(0, -1, &D_80083A80, &D_80083A88, (u8 *)0x80700000,
                  writeSize);
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_80066E14_67A14. */
void func_80045CAC(void) {
    OSThread *thread;

    for (thread = __osGetActiveQueue(); thread->priority != -1;
         thread = thread->tlnext) {
        if (thread->priority > OS_PRIORITY_IDLE) {
            if ((thread->flags & 2) || (thread->flags & 1)) {
                break;
            }
        }
    }
    if (thread->priority != -1) {
        func_80045BBC(thread);
    }
    func_80045D34(thread);
}
#ifdef NON_MATCHING
/* PROVENANCE: JFG src/diCpu.c::func_80066EB0_67AB0 (efd5abb) supplies
 * counterpart context only; that body remains incomplete NON_EQUIVALENT.
 * This candidate is reconstructed from Mickey's own controller target.
 * The page-count branch and post-decrement row test preserve the observed
 * control flow. The redraw flag is dead while it holds the pool base; the
 * row/column storage is reused for the later digit display and page label.
 * Their shared storage recovers the frame without inventing stack padding.
 * The configured candidate remains NON_MATCHING; see its handoff shard. */
void func_80045D34(s32 arg0) {
    s32 row;
    s32 *words;
    u32 oldPage;
    s32 buttons;
    s32 pageCount;
    s32 currentPage;
    s32 nibble;
    s32 redraw;
    s32 memoryIndex;
    u32 printedValue;
    s32 pageColumn;
    s32 moduleOffset;
    u32 address;
    s32 index;
    s32 limit;
    s32 selectedRegion;
    s32 tag;
    u32 mask;
    u32 candidate;
    MemoryPoolSlot *slot;

    oldPage = -1U;
    currentPage = 0;
    redraw = 1;
    memoryIndex = 0;
    /* The fallback count belongs to the logging-mode branch. */
    if (D_8007A200 == 0 || D_80000310 != 0x17D9) {
        while (1) {
        }
    }
    if (D_8007CFE8 == 0) {
        pageCount = D_8007CFE4 / 20;
        if ((D_8007CFE4 % 20) != 0) {
            pageCount++;
        }
    } else { pageCount = 25; }
    selectedRegion = 0;
    pageCount += 5;
    if (viGetVideoMode() != 0) { D_8007D02C = 1; } else { D_8007D02C = 0; }
    address = 0x80100000;
    nibble = 1;
    index = 0;
    do {
        func_80046E00();
        index++;
    } while (index != 100);
    if (D_8007A1E0 == 0) {
        osWritebackDCacheAll();
        while (1) {
        }
    }

    while (1) {
        joyRead(0, 2);
        buttons = joyGetPressed(0);
        if (buttons & 0x8000) {
            currentPage++;
        } else if (buttons & 0x4000) {
            currentPage--;
        }
        if (currentPage < 0) {
            currentPage = pageCount;
        }
        if (pageCount < currentPage) {
            currentPage = 0;
        }
        if (oldPage == 1) {
            if (buttons & 2) {
                redraw = 1;
                nibble++;
                if (nibble >= 6) {
                    nibble = 1;
                }
            }
            if (buttons & 1) {
                redraw = 1;
                nibble--;
                if (nibble <= 0) {
                    nibble = 5;
                }
            }
            if (buttons & 0xC) {
                mask = 0xF << (nibble * 4);
                if (buttons & 8) {
                    candidate = address + (1 << (nibble * 4));
                } else {
                    candidate = address - (1 << (nibble * 4));
                }
                candidate = (candidate & mask) | (address & ~mask);
                redraw = 1;
                if (candidate >= 0x803FFF60U) {
                    candidate = 0x803FFF60;
                }
                if (candidate < 0x80000451U) {
                    candidate = 0x80000450;
                }
                address = candidate;
                printedValue = candidate;
            }
        }
        if (oldPage == 5) {
            if (buttons & 0x800) {
                memoryIndex -= 18;
                redraw = 1;
                if (memoryIndex < 0) {
                    memoryIndex = 0;
                }
            }
            if (buttons & 0x400) {
                memoryIndex += 18;
                redraw = 1;
            }
            if ((buttons & 8) && memoryIndex > 0) {
                memoryIndex--;
                redraw = 1;
            }
            if (buttons & 4) {
                memoryIndex++;
                redraw = 1;
            }
            if (buttons & 0x2000) {
                selectedRegion ^= 1;
                redraw = 1;
            }
        }
        if (buttons & 0x10) {
            redraw = 1;
            D_8007D02C++;
            if (D_8007D02C >= 3) {
                D_8007D02C = 0;
            }
        }
        if (oldPage != (u32)currentPage || redraw != 0) {
            redraw = 0;
            switch (currentPage) {
                case 0:
                    oldPage = currentPage;
                    redraw = 0;
                    render_epc_lock_up_display((MickeyEpcInfo *)(u32)arg0);
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    func_80046E00();
                    pageColumn = 168;
                    switch (currentPage) {
                        case 1: words = (s32 *)(address + 0xA0); break;
                        case 2: words = D_800D5DF0; break;
                        case 3: words = D_800D5E98; break;
                        default: words = D_800D5F40; break;
                    }



                    do {
                        row = 20;
                        do {
                            words--;
                            cpuXYPrintf(pageColumn, (row + 3) * 8,
                                        "%08x:%08x", words, *words);
                            if (currentPage == 1 && row == 1) {
                                row = 0;
                            }
                            /* The exit test uses the old row value. */
                        } while (row--);
                        pageColumn -= 148;
                    } while (pageColumn != -128);
                    oldPage = currentPage;
                    redraw = 0;
                    break;
                case 5:
                    index = 0;
                    row = 44;
                    func_80046E00();
                    D_8007D030 = 1;
                    cpuXYPrintf(100, 24, "MEMORY REGION %d", selectedRegion);
                    D_8007D030 = 2;
                    cpuXYPrintf(32, 32, "MOD");
                    cpuXYPrintf(72, 32, "OFFSET");
                    cpuXYPrintf(152, 32, "SIZE");
                    cpuXYPrintf(224, 32, "ADDRESS");
                    D_8007D030 = 0;
                    redraw = (s32)mmGetSlotPtr(selectedRegion);
                    slot = (MemoryPoolSlot *)redraw;
                    do {
                        if (slot->flags != 0) {
                            if (index >= memoryIndex &&
                                index < memoryIndex + 18) {
                                tag = (slot->colourTag >> 24) & 0xFF;
                                printedValue = slot->colourTag & 0xFFFFFF;
                                if (tag == 0xFF) {
                                    cpuXYPrintf(32, row, "Texture %d",
                                                printedValue);
                                } else if (tag == 0xFE) {
                                    cpuXYPrintf(32, row, "Module %d",
                                                printedValue);
                                } else {
                                    cpuXYPrintf(32, row, "%d", tag);
                                    cpuXYPrintf(72, row, "%08x",
                                                printedValue);
                                }
                                cpuXYPrintf(152, row, "%d", slot->size);
                                cpuXYPrintf(224, row, "%08x", slot->data);
                                row += 8;
                            }
                            index++;
                        }
                        slot = &((MemoryPoolSlot *)redraw)[slot->nextIndex];
                    } while (slot->nextIndex != -1);
                    oldPage = currentPage;
                    redraw = 0;
                    break;
                default:
                    currentPage = 0;
                    oldPage = 0;
                    break;
            }
            if (currentPage == 1) {
                if (runlinkGetAddressInfo(address, &printedValue,
                                          &moduleOffset, 0) != 0) {
                    D_8007D030 = 1;
                    cpuXYPrintf(20, 24, "Module %d at %08x", printedValue,
                                moduleOffset);
                    D_8007D030 = 0;
                }
                pageColumn = address;
                row = 0;
                do {
                    if (row == nibble) {
                        D_8007D030 = 1;
                    }
                    cpuXYPrintf(76 - (row * 8), 32, "%1x", pageColumn & 0xF);
                    row++;
                    D_8007D030 = 0;
                    pageColumn = (s32)pageColumn >> 4;
                } while (row != 8);
            }
            if (D_8007CFE8 == 0) {
                printedValue = D_8007CFE4;
            } else {
                printedValue = 500;
            }
            row = currentPage + 1;
            /* The log count is already in the printed-value home. */
            limit = pageCount + 1;
            cpuXYPrintf(50, 200, "%d lines logged", printedValue);
            cpuXYPrintf(220, 200, "Page %d/%d", row, limit);
        }
        osWritebackDCacheAll();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045D34.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuReportWatchpoint. */
void diCpuReportWatchpoint(u32 address) {
    s32 moduleAddress;
    s32 moduleId;
    s32 i;

    for (i = 0; i < 100; i++) {
        func_80046E00();
    }
    cpuXYPrintf(30, 80, D_80083B2C, address);
    if (runlinkGetAddressInfo(address, &moduleId, &moduleAddress, 0)) {
        cpuXYPrintf(30, 100, D_80083B48, moduleId, moduleAddress);
    }
    while (1) {
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceGetFault. */
s32 func_80046504(void) {
    return 0;
}

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceTick. */
void func_8004650C(s32 ticks) {
    D_8007CFDC += ticks;
    if (D_8007CFDC > 60) {
        D_8007CFDC = 0;
        D_8007CFD8++;
    }
}

#ifdef NON_MATCHING
/* PROVENANCE: adapted from the SDK-style crash-display control flow in JFG
 * src/diCpu.c; Mickey's own m2c control flow, globals, and ABI are authoritative. */
/* 51/344 differing words, exact 344 words, frame 0x50, size delta 0 (lane p9-mid, 2026-09-12;
 * was 62).  All 127 relocation records are present and 125 identities align.
 *
 * The residual is a ugen integer ring phase, and the ring itself was measured here rather than
 * inferred.  An instrumented-ugen free-list trace over this TU reports this function as procedure
 * ordinal 9 with 32 integer allocations, and its draw order is t6, t7, t8, t9, t0, t1, t2, t3, t4,
 * t5, wrapping -- so a ring draw that emits nothing still advances every later temp by one name.
 * Each draw is stamped with the source line that consumed it, which is what makes the phase
 * settable from source.
 *
 * Three hoists of a printf argument into the already-present value local, each worth the draw it
 * removes: the second argument of the exception-address line (62 -> 56), the cause-table lookup
 * (56 -> 53), and the tick counter on the first line (53 -> 51).  A greedy subset search over
 * eighteen such hoists -- every single-line cpuXYPrintf in the function with a stack-passed
 * argument -- converges there.  Buckets went 283 byte-exact / 57 naming / 1 immediate / 4
 * structural to 293 / 49 / 0 / 2 at an unchanged size, so this is more agreement on every axis, not
 * a trade.
 *
 * One tension worth stating: the last of the three puts the first line's stack argument in value's
 * own colour where the shipped code uses a ring temp, so it disagrees at three sites it used to
 * agree on while agreeing at five more.  It wins on every bucket and the schedule does not move,
 * which is why it is retained, but a form that removes that draw *without* naming the value would
 * be strictly better.
 *
 * The remaining 49 naming words are one fact: the shipped code's first visible ring temp is t7,
 * one position past the head of the list ugen builds, so it makes one draw this candidate does not
 * before any instruction is emitted.  A phantom pop at the top of the function is the shape to look
 * for.  Earlier work, retained: ten natural source forms over pointer lifetime, direct field access,
 * context reassignment, declaration order, volatility, control boundaries and cause-word reuse; a
 * 119-row flag lattice, nonexact; a bounded permuter run with no zero; and the JFG revision once
 * cited as a donor, which changes only README and tooling and supplies no body. */
void render_epc_lock_up_display(MickeyEpcInfo *arg0) {
    u32 sp4c;
    u32 sp48;
    u32 sp44;
    char *region;
    u32 value;
    u32 *regs = (u32 *)((u8 *)arg0 + 0x20);

    func_80046E00();
    value = D_8007CFD0;
    cpuXYPrintf(0x20, 0x18, D_80083B5C, arg0->unk14, value);
    value = regs[0xFC / 4];
    if (value == 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B78);
    } else if (runlinkGetAddressInfo(value, (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B84, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x22, D_80083B90, regs[0xFC / 4]);
    }
    if (regs[0xE4 / 4] == 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BA0);
    } else if (runlinkGetAddressInfo(regs[0xE4 / 4], (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BAC, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x28, D_80083BB8, regs[0xE4 / 4]);
    }
    if (regs[0x100 / 4] == -1U) {
        value = regs[0x24 / 4];
        cpuXYPrintf(0x20, 0x2E, D_80083BC8, regs[0x1C / 4], value);
    } else {
        if ((((regs[0x100 / 4]) >> 2) & 0x1F) < 0x10) {
            value = D_8007CFEC[(regs[0x100 / 4] >> 2) & 0x1F];
            cpuXYPrintf(0x20, 0x2E, D_80083BE4, value);
        } else {
            cpuXYPrintf(0x20, 0x2E, D_80083BF4, regs[0x100 / 4]);
        }
    }
    if ((D_8007A21C != 4) && (D_8007A210 != 0)) {
        if ((D_8007A21C == 1) || (D_8007A21C == 3) || (D_8007A21C == 2)) {
            if (D_8007A21C == 1) {
                region = D_80083C04;
            } else if (D_8007A21C == 3) {
                region = D_80083C0C;
            } else {
                region = D_80083C14;
            }
            if (D_8007A218 != 0) {
                cpuXYPrintf(0x20, 0x34, D_80083C1C, region,
                            D_8007A220[D_8007A210], D_8007A218);
            } else if (D_8007A214 != NULL) {
                cpuXYPrintf(0x20, 0x34, D_80083C28, region,
                            D_8007A220[D_8007A210], D_8007A214->unk44);
            } else {
                cpuXYPrintf(0x20, 0x34, D_80083C38, region,
                            D_8007A220[D_8007A210]);
            }
        }
    } else {
        cpuXYPrintf(0x20, 0x34, D_80083C48, regs[0x104 / 4]);
    }
    cpuXYPrintf(0x20, 0x3A, D_80083C58, D_800D21B0);
    cpuXYPrintf(0x20, 0x44, D_80083C68, regs[0x4 / 4], regs[0xC / 4]);
    cpuXYPrintf(0x20, 0x4A, D_80083C7C, regs[0x14 / 4], regs[0x1C / 4]);
    cpuXYPrintf(0x20, 0x50, D_80083C90, regs[0x24 / 4], regs[0x2C / 4]);
    cpuXYPrintf(0x20, 0x56, D_80083CA4, regs[0x34 / 4], regs[0x3C / 4]);
    cpuXYPrintf(0x20, 0x5C, D_80083CB8, regs[0x44 / 4], regs[0x4C / 4]);
    cpuXYPrintf(0x20, 0x62, D_80083CCC, regs[0x54 / 4], regs[0x5C / 4]);
    cpuXYPrintf(0x20, 0x68, D_80083CE0, regs[0x64 / 4], regs[0x6C / 4]);
    cpuXYPrintf(0x20, 0x6E, D_80083CF4, regs[0x74 / 4], regs[0x7C / 4]);
    cpuXYPrintf(0x20, 0x74, D_80083D08, regs[0x84 / 4], regs[0x8C / 4]);
    cpuXYPrintf(0x20, 0x7A, D_80083D1C, regs[0x94 / 4], regs[0x9C / 4]);
    cpuXYPrintf(0x20, 0x80, D_80083D30, regs[0xA4 / 4], regs[0xAC / 4]);
    cpuXYPrintf(0x20, 0x86, D_80083D44, regs[0xB4 / 4], regs[0xBC / 4]);
    cpuXYPrintf(0x20, 0x8C, D_80083D58, regs[0xC4 / 4], regs[0xCC / 4]);
    cpuXYPrintf(0x20, 0x92, D_80083D6C, regs[0xD4 / 4], regs[0xDC / 4]);
    cpuXYPrintf(0x20, 0x98, D_80083D80, regs[0xF8 / 4]);
    value = levelGetLevel();
    if ((value != 0) && (value & 0x80000000)) {
        cpuXYPrintf(0x20, 0xA4, D_80083D8C, value);
    }
    value = func_80005820(0);
    if ((value != 0) && (value & 0x80000000)) {
        cpuXYPrintf(0x20, 0xAA, D_80083D9C, value + 0xC,
                    value + 0x10, value + 0x14);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/render_epc_lock_up_display.s")
#endif
/* Mickey-derived body; JFG's corresponding func_800680B0_68CB0 is
 * assembly-only and confirms the packed-glyph loop structure. */
void func_80046AA8(s32 x, s32 y, u16 *glyph) {
    s16 *destination;
    s16 *pixel;
    u16 *palette;
    u16 bits;
    s32 lines;
    s32 rows;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    /* The do/while(0) closes the setup as its own basic block before the row
     * loop; without that boundary IDO schedules the blit loop's registers
     * differently and the function does not match. */
    do {
        destination = D_800D2FA8 + ((y * width) + x);
        if (D_8007D030 == 2) {
            palette = D_8007D300;
        } else if (D_8007D030 != 0) {
            palette = D_8007D2F8;
        } else {
            palette = D_8007D2F0;
        }
        rows = 5;
    } while (0);
    while (rows--) {
        lines = 1;
        if (D_8007D02C != 0) {
            lines = 2;
        }
        while (lines--) {
            bits = *glyph;
            pixel = destination;
            while (bits) {
                *pixel++ = palette[bits & 3];
                bits >>= 2;
            }
            destination += width;
        }
        glyph++;
    }
}
/* Workbench: allocation-mismatch, 16 differing words, first mismatch +0x2C.
 * 106/106 words, frame 0x40, all three relocation sites exact, and every
 * instruction positionally identical -- the comparator calls it register-only.
 *
 * 2026-09-09: the ninth callee-saved web is the case-conversion working copy,
 * and it is a real source variable. Folding it away closed the +4 size
 * mismatch but freed a callee-saved register, so the candidate hoisted a
 * third loop-invariant constant (0x78) into s8 where the target materialises
 * it inline with `li at` and hoists only 0xA and 0x30. Reinstating the copy
 * -- `var_s0` carries the character through the range tests and the converted
 * value is written back to `var_s2`, which is the only value the next
 * iteration's `temp_s6` needs -- restores the target's two hoisted constants
 * in the target's registers (s7 = 0xA, s8 = 0x30) and takes 41 differing
 * words to 31.
 *
 * What is left is one live-range split. The target computes the masked
 * character straight into its callee-saved carrier (`andi s2,v0,0xff`) and
 * splits a copy into s0 for the range tests. The candidate computes it into a
 * caller-saved temporary, runs every range test out of that temporary, and
 * copies into the saved carrier, which also exchanges var_s2 with var_s3 and
 * var_v0 with its own temp. Same instruction count, 31 register names.
 * Measured flat against it: both initialiser orders; all legal orders of the
 * three loop-head statements; `u8` var_v0; the loop test written as
 * `while ((var_v0 = *var_s4) != 0)`; the range tests spelled entirely on
 * either variable; reversed equality operands at two sites; and the copy
 * written after the case block, inside each arm, or as a plain working copy
 * with a single write-back (40, 41). Resume on why the mask lands in a
 * caller-saved temporary here and directly in the saved carrier there.
 *
 * 2026-09-09 (second pass): the instrumented uopt answers that question and
 * closes the reordering space. The masked character is a compiler temporary in
 * its own right -- phase-one web 32 -- and it is coloured *first*, before every
 * declared local, taking v0; `var_v0` is web 0 and gets v1 behind it. The
 * target has no such web at all: its mask writes the callee-saved carrier
 * directly. `CDX_FORCE=p1:w0=c1` (put `var_v0` back in v0) is declined twice
 * because web 32 already holds it and the two interfere, so the register file
 * cannot be recovered by moving `var_v0`; web 32 has to stop existing.
 * Forcing web 32 into a callee-saved colour instead reaches the target's
 * registers and wrecks the schedule (73 and 74 differing words for s2 and s3),
 * and forcing its split path costs two instructions (108 words).
 * Web 32 exists because `var_s2 = var_v0 & 0xFF;` is immediately followed by
 * `var_s0 = var_s2;` in the same block: the value has two destinations, so uopt
 * commons it into a temporary and copy-propagates both names onto it, which is
 * also why every range test reads the temporary rather than a carrier.
 * Newly eliminated: nested `if`s in place of the `&&` pairs (31, byte-flat);
 * `var_s0` spelled as a second `var_v0 & 0xFF` (31, flat); the copy moved ahead
 * of the pointer increment (32); the copy inside each arm with an else copy on
 * the short path (103, and the frame moves); `var_s0 = var_s2 & 0xFF` (89);
 * both range tests on `var_s2` with the copy after the case block (41);
 * `var_v0` masked at the load (86); and `temp_s6` taken from `var_s0` (56).
 * Resume by removing the second destination of the mask, not by reordering it.
 *
 * 2026-09-11 (lane `lane/p4-xfer`): 31 to 16, and the temp is gone. Three
 * edits, none of which pays on its own:
 *
 *   1. The working copy is `var_v0` itself, not a fresh `var_s0` (L115). Five
 *      carrier identities were measured across 40 forms -- `var_s0`, `var_v0`,
 *      `x`, `y`, `temp_s6` -- and only `var_v0` reaches 16; the next best is
 *      27. Reusing the parameter's own local ends its live range where the
 *      copy begins, so the mask has one destination and web 32 stops existing.
 *   2. The copy sits inside each arm of the case-state test, not ahead of it.
 *      On its own that is 32 against 27, which is why earlier passes dropped
 *      it; with edit 1 and edit 3 it is the shape the target schedules into
 *      the two `bnez` delay slots.
 *   3. The tab case is written as two statements,
 *      `var_s1 = var_s1 - (var_s1 & 0xF); var_s1 = var_s1 + 0x10;`. Same three
 *      instructions, one more reference to `var_s1`, which lifts its
 *      totalsave 91 to 111 and wins the p1 tie against `var_s2` at 101/10 --
 *      var_s1 takes s1 and var_s2 takes s2, as in the target. Written as one
 *      expression it loses that tie and the callee-saved file rotates.
 *   4. `var_s2 = 0` before `var_s3 = 0` in the preamble, worth two words.
 *
 * The 16 left are one allocation fact with no source form yet reaching it:
 * `var_v0` doubles as the working copy, so its range crosses the call and it
 * colours callee-saved s0, where the target keeps it caller-saved in v0 with a
 * separate s0 copy. That accounts for the four `lbu`/`beqz` sites and the
 * eight sites inside the two conversion bodies, where our intermediate lands
 * in the working copy and the target's lands in the carrier. Separating them
 * again brings web 32 straight back: all four three-variable forms measure 31,
 * 33, 89 and 91.
 *
 * Measured flat this pass, exhaustively, against the axes an earlier pass left
 * open:
 *   - 42 structural forms (`if (1) { }` and `do { } while (0)` region openers
 *     of L97, copy placement, test operand, two- versus three-statement
 *     conversion) collapse to three outcomes decided by copy placement alone.
 *     Both region openers are byte-inert here.
 *   - 32 forms of the comparison operand order, every subset of the six
 *     comparison groups, are one object.
 *   - 16 forms of the range-test operands, including asymmetric ones, are one
 *     object, and the records show `var_s2` totalsave pinned at 101 across all
 *     of them.
 *   - 32 forms of declared type against explicit masking (`u8` versus `s32`
 *     for the two char variables and `var_v0`, each mask present or absent):
 *     nothing beats the baseline, and `var_v0`'s declared type is byte-inert.
 *   - L109's discarded-expression probe does not work on a local that is
 *     already read. Eighteen forms -- or-with-zero, and-with-minus-one and
 *     xor-with-zero, at nought to five copies each -- are byte-flat, and the
 *     allocator records confirm the probe never
 *     reaches totalsave: web 0 stays at 32.0 in every one.
 */
/* PROVENANCE: adapted from Jet Force Gemini's public
 * asm/nonmatchings/diCpu/func_800681D0_68DD0.s; Mickey's glyph table,
 * helper symbol, and target bytes determine the final bindings. */
#ifdef NON_MATCHING
void func_80046BCC(s32 x, s32 y, char *text) {
    s32 temp_s6;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3;
    s32 var_s5;
    u8 *var_s4;
    s32 var_v0;

    var_v0 = *(u8 *)text;
    var_s1 = x;
    var_s4 = (u8 *)text;
    var_s5 = y;
    var_s2 = 0;
    var_s3 = 0;
    if (var_v0 != 0) {
        do {
            temp_s6 = var_s2 & 0xFF;
            var_s2 = var_v0 & 0xFF;
            var_s4 += 1;
            if (var_s3 != 0) {
                var_v0 = var_s2;
                if ((var_s2 >= 0x41) && (var_v0 < 0x47)) {
                    var_v0 = (var_v0 + 0x20) & 0xFF;
                    var_s2 = var_v0;
                }
            } else {
                var_v0 = var_s2;
                if ((var_s2 >= 0x61) && (var_v0 < 0x7B)) {
                    var_v0 = (var_v0 - 0x20) & 0xFF;
                    var_s2 = var_v0;
                }
            }
            if (var_v0 == 0xA) {
                var_s5 += 6;
                var_s1 = 0x20;
            } else if (var_v0 == 9) {
                var_s1 = var_s1 - (var_s1 & 0xF);
                var_s1 = var_s1 + 0x10;
            } else if (var_v0 == 0x20) {
                var_s1 += 4;
            } else if ((var_v0 >= 0x21) && (var_v0 < 0x67)) {
                func_80046AA8(var_s1, var_s5, &D_8007D034[(var_v0 * 5) - 0xA5]);
                var_s1 += 8;
            }
            if ((var_s3 != 0) && ((var_v0 < 0x30) || (var_v0 >= 0x3A)) &&
                ((var_v0 < 0x61) || (var_v0 >= 0x67))) {
                var_s3 = 0;
            }
            if ((temp_s6 == 0x30) && ((var_v0 == 0x78) || (var_v0 == 0x58))) {
                var_s3 = 1;
            }
            var_v0 = *var_s4;
        } while (var_v0 != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046BCC.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::cpuXYPrintf. */
void cpuXYPrintf(s32 x, s32 y, const char *format, ...) {
    va_list args;
    char text[255];

    va_start(args, format);
    vsprintf(text, format, args);
    va_end(args);

    if (D_8007D02C != 0) {
        if (D_8007D02C == 1) {
            y -= 8;
        } else {
            y -= 104;
        }
        if (y >= 0 && y < 116) {
            y *= 2;
            goto draw;
        }
    } else {
draw:
        func_80046BCC(x, y, text);
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_8006837C_68F7C. */
void func_80046E00(void) {
    s32 pad;
    s32 height;
    s32 width;
    s32 screenSize;
    s16 *screen;

    viGetCurrentSize(&height, &width);
    screenSize = height * width;
    screen = D_800D2FA8;
    while (screenSize--) {
        *screen++ = 0;
    }
}

/* PLATEAU-HANDOFF:func_80046BCC:start
 * symbol: func_80046BCC
 * score: 16/106 words
 * frame: 0x40
 * relocations: 3
 * first-mismatch: +0x2C
 * summary: arg0 OR-zero spans the glyph call (72); loop-local zero folds. Three-var region still 21; forcing the mask temp to s2 is 74. Occupancy cannot occupy v0. Best 16
 * PLATEAU-HANDOFF:func_80046BCC:end
 */

/* PLATEAU-HANDOFF:render_epc_lock_up_display:start
 * symbol: render_epc_lock_up_display
 * score: 51 differing words
 * frame: 0x50
 * relocations: 127
 * first-mismatch: +0x18
 * summary: Dead tick updates are byte-inert and compensated preincrement adds an instruction; the zero-byte leading ring draw remains open.
 * PLATEAU-HANDOFF:render_epc_lock_up_display:end
 */

/* PLATEAU-HANDOFF:func_80045D34:start
 * symbol: func_80045D34
 * score: 254/459 words
 * frame: 0xA8
 * relocations: 91
 * first-mismatch: +0xC
 * summary: Control-flow and lifetime corrections recover 459 words and exact frame/home traffic; 298 aligned exact, with type and emission residuals remaining.
 * PLATEAU-HANDOFF:func_80045D34:end
 */
