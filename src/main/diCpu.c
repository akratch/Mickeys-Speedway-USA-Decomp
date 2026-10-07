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

/* The saved context at OSThread + 0x20: 64-bit integer registers, then the
 * 32-bit status words. */
typedef struct {
    u64 at;
    u64 v0;
    u64 v1;
    u64 a0;
    u64 a1;
    u64 a2;
    u64 a3;
    u64 t0;
    u64 t1;
    u64 t2;
    u64 t3;
    u64 t4;
    u64 t5;
    u64 t6;
    u64 t7;
    u64 s0;
    u64 s1;
    u64 s2;
    u64 s3;
    u64 s4;
    u64 s5;
    u64 s6;
    u64 s7;
    u64 t8;
    u64 t9;
    u64 gp;
    u64 sp;
    u64 s8;
    u64 ra;
    u64 lo;
    u64 hi;
    u32 sr;
    u32 pc;
    u32 cause;
    u32 badvaddr;
} MickeyThreadContext;

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
extern u16 joyGetPressed(s32 controller);
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
extern char D_80083A8C[];
extern char D_80083A98[];
extern char D_80083AAC[];
extern char D_80083AB0[];
extern char D_80083AB8[];
extern char D_80083AC0[];
extern char D_80083AC8[];
extern char D_80083AD4[];
extern char D_80083AE0[];
extern char D_80083AE4[];
extern char D_80083AEC[];
extern char D_80083AF0[];
extern char D_80083AF8[];
extern char D_80083B0C[];
extern char D_80083B10[];
extern char D_80083B20[];
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
/* PROVENANCE: JFG src/diCpu.c::func_80066EB0_67AB0 (efd5abb) supplies
 * counterpart context only; that body remains incomplete NON_EQUIVALENT.
 * This candidate is reconstructed from Mickey's own controller target.
 * The page-count branch and post-decrement row test preserve the observed
 * control flow. The redraw flag is dead while it holds the pool base; the
 * row/column storage is reused for the later digit display and page label.
 * Their shared storage recovers the frame without inventing stack padding.
 * Matched 2026-10-07 (lane i-4): u16 joyGetPressed, the editor stepping
 * the printed-value home, redraw set after the decrement clamp, the slot
 * step index first, and the counter reached through an opaque zero
 * subscript. */
void func_80045D34(s32 arg0) {
    s32 row;
    s32 *words;
    s32 oldPage;
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
    MemoryPoolSlot *slot;

    oldPage = -1;
    currentPage = 0;
    redraw = 1;
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
    /* The memory-page index is initialised with the region selector, after
     * the anti-piracy test, not with the page state. */
    memoryIndex = 0;
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
                nibble--;
                if (nibble <= 0) {
                    nibble = 5;
                }
                redraw = 1;
            }
            if (buttons & 0xC) {
                mask = 0xF << (nibble * 4);
                /* Stepped in place in the printed-value home. */
                printedValue = address;
                if (buttons & 8) {
                    printedValue += 1 << (nibble * 4);
                } else {
                    printedValue -= 1 << (nibble * 4);
                }
                printedValue &= mask;
                printedValue |= address & ~mask;
                if (printedValue >= 0x803FFF60U) {
                    printedValue = 0x803FFF60;
                }
                if (printedValue < 0x80000451U) {
                    printedValue = 0x80000450;
                }
                redraw = 1;
                address = printedValue;
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
            /* An opaque zero subscript (uopt does not fold `x * 0`, ugen
             * does): the counter's address becomes its own piece, formed
             * once into v0 for the increment and the clear, as shipped. */
            (&D_8007D02C)[nibble * 0]++;
            if ((&D_8007D02C)[nibble * 0] >= 3) {
                (&D_8007D02C)[nibble * 0] = 0;
            }
        }
        /* Each page clears redraw itself (the default page first), so the
         * per-page stores survive; one shared clear before the switch lets
         * uopt delete them and hands redraw a lower save than the memory
         * index. The address editor sets it only after both clamps. */
        if (oldPage != currentPage || redraw != 0) {
            switch (currentPage) {
                case 0:
                    render_epc_lock_up_display((MickeyEpcInfo *)(u32)arg0);
                    oldPage = currentPage;
                    redraw = 0;
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
                            cpuXYPrintf(pageColumn, (row + 3) << 3,
                                        D_80083A8C, words, *words);
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
                    cpuXYPrintf(100, 24, D_80083A98, selectedRegion);
                    D_8007D030 = 2;
                    cpuXYPrintf(32, 32, D_80083AAC);
                    cpuXYPrintf(72, 32, D_80083AB0);
                    cpuXYPrintf(152, 32, D_80083AB8);
                    cpuXYPrintf(224, 32, D_80083AC0);
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
                                    cpuXYPrintf(32, row, D_80083AC8,
                                                printedValue);
                                } else if (tag == 0xFE) {
                                    cpuXYPrintf(32, row, D_80083AD4,
                                                printedValue);
                                } else {
                                    cpuXYPrintf(32, row, D_80083AE0, tag);
                                    cpuXYPrintf(72, row, D_80083AE4,
                                                printedValue);
                                }
                                cpuXYPrintf(152, row, D_80083AEC, slot->size);
                                cpuXYPrintf(224, row, D_80083AF0, slot->data);
                                row += 8;
                            }
                            index++;
                        }
                        /* Index first: the target adds the scaled index
                         * before the pool base. */
                        slot = slot->nextIndex + (MemoryPoolSlot *)redraw;
                    } while (slot->nextIndex != -1);
                    oldPage = currentPage;
                    redraw = 0;
                    break;
                default:
                    redraw = 0;
                    currentPage = 0;
                    oldPage = 0;
                    break;
            }
            if (currentPage == 1) {
                if (runlinkGetAddressInfo(address, &printedValue,
                                          &moduleOffset, 0) != 0) {
                    D_8007D030 = 1;
                    cpuXYPrintf(20, 24, D_80083AF8, printedValue,
                                moduleOffset);
                    D_8007D030 = 0;
                }
                pageColumn = address;
                row = 0;
                do {
                    if (row == nibble) {
                        D_8007D030 = 1;
                    }
                    cpuXYPrintf(76 - (row * 8), 32, D_80083B0C, pageColumn & 0xF);
                    D_8007D030 = 0;
                    pageColumn >>= 4;
                    row++;
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
            cpuXYPrintf(50, 200, D_80083B10, printedValue);
            cpuXYPrintf(220, 200, D_80083B20, row, limit);
        }
        osWritebackDCacheAll();
    }
}
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

/* PROVENANCE: the crash-display control flow and the 64-bit saved-register
 * reads narrowed at each use are adapted from the public Diddy Kong Racing
 * decompilation, src/thread0_epc.c::render_epc_lock_up_display (its GET_REG
 * macro), and JFG include/structs.h::epcInfo for the field layout; Mickey's
 * own target supplies the control flow, globals and ABI.
 *
 * Matched (was 51 masked words, all one ring phase) by typing the saved
 * integer registers as the 64-bit fields they are and narrowing each one at
 * its use: every narrowed read draws a register pair and emits one load, which
 * is the draw the u32-array shape never spent.  With that in place the three
 * argument hoists into `value` came out again (tick counter, cause-table
 * lookup, mmAlloc colour tag are passed directly), and the level result has
 * its own local. */
void render_epc_lock_up_display(MickeyEpcInfo *arg0) {
    u32 sp4c;
    u32 sp48;
    u32 sp44;
    char *region;
    u32 value;
    u32 level;
    MickeyThreadContext *context = (MickeyThreadContext *)((u8 *)arg0 + 0x20);

    func_80046E00();
    cpuXYPrintf(0x20, 0x18, D_80083B5C, arg0->unk14, D_8007CFD0);
    value = context->pc;
    if (value == 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B78);
    } else if (runlinkGetAddressInfo(value, (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B84, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x22, D_80083B90, context->pc);
    }
    if ((u32)context->ra == 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BA0);
    } else if (runlinkGetAddressInfo((u32)context->ra, (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BAC, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x28, D_80083BB8, (u32)context->ra);
    }
    if (context->cause == -1U) {
        cpuXYPrintf(0x20, 0x2E, D_80083BC8, (u32)context->a0, (u32)context->a1);
    } else {
        if ((((context->cause) >> 2) & 0x1F) < 0x10) {
            cpuXYPrintf(0x20, 0x2E, D_80083BE4, D_8007CFEC[(context->cause >> 2) & 0x1F]);
        } else {
            cpuXYPrintf(0x20, 0x2E, D_80083BF4, context->cause);
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
        cpuXYPrintf(0x20, 0x34, D_80083C48, context->badvaddr);
    }
    cpuXYPrintf(0x20, 0x3A, D_80083C58, D_800D21B0);
    cpuXYPrintf(0x20, 0x44, D_80083C68, (u32)context->at, (u32)context->v0);
    cpuXYPrintf(0x20, 0x4A, D_80083C7C, (u32)context->v1, (u32)context->a0);
    cpuXYPrintf(0x20, 0x50, D_80083C90, (u32)context->a1, (u32)context->a2);
    cpuXYPrintf(0x20, 0x56, D_80083CA4, (u32)context->a3, (u32)context->t0);
    cpuXYPrintf(0x20, 0x5C, D_80083CB8, (u32)context->t1, (u32)context->t2);
    cpuXYPrintf(0x20, 0x62, D_80083CCC, (u32)context->t3, (u32)context->t4);
    cpuXYPrintf(0x20, 0x68, D_80083CE0, (u32)context->t5, (u32)context->t6);
    cpuXYPrintf(0x20, 0x6E, D_80083CF4, (u32)context->t7, (u32)context->s0);
    cpuXYPrintf(0x20, 0x74, D_80083D08, (u32)context->s1, (u32)context->s2);
    cpuXYPrintf(0x20, 0x7A, D_80083D1C, (u32)context->s3, (u32)context->s4);
    cpuXYPrintf(0x20, 0x80, D_80083D30, (u32)context->s5, (u32)context->s6);
    cpuXYPrintf(0x20, 0x86, D_80083D44, (u32)context->s7, (u32)context->t8);
    cpuXYPrintf(0x20, 0x8C, D_80083D58, (u32)context->t9, (u32)context->gp);
    cpuXYPrintf(0x20, 0x92, D_80083D6C, (u32)context->sp, (u32)context->s8);
    cpuXYPrintf(0x20, 0x98, D_80083D80, context->sr);
    level = levelGetLevel();
    if ((level != 0) && (level & 0x80000000)) {
        cpuXYPrintf(0x20, 0xA4, D_80083D8C, level);
    }
    value = func_80005820(0);
    if ((value != 0) && (value & 0x80000000)) {
        cpuXYPrintf(0x20, 0xAA, D_80083D9C, value + 0xC,
                    value + 0x10, value + 0x14);
    }
}
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
/* Matched on the natural shape: one u8 character and a u8 previous
 * character, the parameters used in place, and a while loop whose test and
 * body both read *text, so the loaded byte is a compiler temporary rather
 * than a declared local that doubles as the working copy. */
/* PROVENANCE: adapted from Jet Force Gemini's public
 * asm/nonmatchings/diCpu/func_800681D0_68DD0.s; Mickey's glyph table,
 * helper symbol, and target bytes determine the final bindings. */
void func_80046BCC(s32 x, s32 y, char *text) {
    u8 prev;
    u8 c;
    s32 hex;

    c = 0;
    hex = 0;
    while (*text != 0) {
        prev = c;
        c = *text++;
        if (hex != 0) {
            if ((c >= 'A') && (c < 'G')) {
                c += 0x20;
            }
        } else {
            if ((c >= 'a') && (c < '{')) {
                c -= 0x20;
            }
        }
        if (c == '\n') {
            y += 6;
            x = 0x20;
        } else if (c == '\t') {
            x = x - (x & 0xF);
            x = x + 0x10;
        } else if (c == ' ') {
            x += 4;
        } else if ((c >= 0x21) && (c < 0x67)) {
            func_80046AA8(x, y, &D_8007D034[(c * 5) - 0xA5]);
            x += 8;
        }
        if ((hex != 0) && ((c < '0') || (c >= 0x3A)) &&
            ((c < 'a') || (c >= 0x67))) {
            hex = 0;
        }
        if ((prev == '0') && ((c == 'x') || (c == 'X'))) {
            hex = 1;
        }
    }
}
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
