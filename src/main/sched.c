/*
 * Game scheduler -- ROM 0x30CD0-0x323A0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/sched.c. Mickey has the same
 * ordered 21-function scheduler call graph, including the three pre-existing
 * accessor/string anchors. Adapted C bodies are identified in docs/modules.md;
 * all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "PR/os_internal.h"
#include "PR/os_vi.h"
#include "game/sched_internal.h"
#include "n_audio/mbi.h"

typedef struct SchedGfx {
    u32 w0;
    u32 w1;
} SchedGfx;

#define OS_IM_NONE 1
#define VIDEO_MSG 666
#define RSP_DONE_MSG 667
#define RDP_DONE_MSG 668
#define PRE_NMI_MSG 669
#define UNK_MSG 99

#define OS_SC_DP 0x0001
#define OS_SC_SP 0x0002
#define OS_SC_YIELDED 0x0020
#define OS_SC_DRAM_DLIST 0x0004
#define OS_SC_PARALLEL_TASK 0x0010
#define OS_SC_TYPE_MASK 0x0007
#define OS_SC_XBUS (OS_SC_SP | OS_SC_DP)
#define OS_SC_DRAM (OS_SC_SP | OS_SC_DP | OS_SC_DRAM_DLIST)
#define OS_SC_DP_XBUS OS_SC_SP
#define OS_SC_DP_DRAM (OS_SC_SP | OS_SC_DRAM_DLIST)
#define OS_SC_SP_XBUS OS_SC_DP
#define OS_SC_SP_DRAM (OS_SC_DP | OS_SC_DRAM_DLIST)

extern s32 D_8007A640;
extern s32 D_8007A648;
extern s32 D_8007A650;
extern s32 D_8007A654;
extern s8 D_8007A658;
extern s32 D_8007A65C;
u64 D_8007A660 = 0;
extern char D_80082350[];
extern char D_80082354[];
extern char D_80082368[];
extern char D_8008237C[];
extern char D_80082390[];
extern char D_800823A4[];
extern char D_800823B8[];
extern char D_800823CC[];
extern char D_80082090[];
extern char D_800820A0[];
extern char D_800820AC[];
extern char D_800820B8[];
extern char D_800820D0[];
extern char D_800820E0[];
extern char D_800820F0[];
extern char D_80082100[];
extern u8 D_800CF520;
extern u8 D_800CF578;
extern u8 D_800CF590;
extern u8 D_800CF5A8;
extern u8 D_80000000[];
s32 D_800D2D40;
s32 D_800D2D44;
u64 D_800D2D48;
extern u8 *D_800D2FA0;
extern u8 *D_800D2FA8;
extern u8 *D_800D2FAC;
extern OSViMode D_80080490;
extern OSViMode D_800804E0;
extern OSViMode D_80080530;

void osCreateViManager(OSPri priority);
void osViSetMode(OSViMode *mode);
void osViBlack(u8 active);
void osViSetEvent(OSMesgQueue *mq, OSMesg msg, u32 retraceCount);
void osWritebackDCacheAll(void);
void osSpTaskLoad(OSTask *task);
void osSpTaskStartGo(OSTask *task);
void osSpTaskYield(void);
s32 osSpTaskYielded(OSTask *task);
s32 osDpSetNextBuffer(void *buffer, u64 size);
u64 osGetTime(void);
OSIntMask osSetIntMask(OSIntMask mask);
void *osViGetCurrentFramebuffer(void);
void *osViGetNextFramebuffer(void);
void rsp_segment(SchedGfx **dlist, s32 segment, void *base);
s32 diPrintf(const char *format, ...);
void diPrintfAll(SchedGfx **dlist);
void diPrintfSetXY(u16 x, u16 y);
char *osScGetTaskType(s32 taskID);
void func_800304E0(OSSched *sc);
void func_80030608(OSScTask *task);
SchedGfx *func_80030610(OSSched *sc, s32 commandIndex,
                        SchedGfx *displayList, OSMesgQueue *queue,
                        u64 *dataStart);
void func_80044C94(SchedGfx *displayList, s32 *file, s32 *unkC,
                   s32 *unk10, s32 *file2, s32 *unkC2, s32 *unk102);
void diRcpPrintDL(SchedGfx *start, SchedGfx *end, s32 count);
SchedGfx *func_80030910(OSSched *sc, s32 *arg1, s32 *arg2, s32 *arg3,
                        s32 *arg4, s32 *arg5, s32 *arg6);
void __scAppendList(OSSched *sc, OSScTask *task);
void __scYield(OSSched *sc);
void __scHandleRetrace(OSSched *sc);
void __scHandleRSP(OSSched *sc);
void __scHandleRDP(OSSched *sc);
s32 __scTaskComplete(OSSched *sc, OSScTask *task);
s32 __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp, s32 state);
void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp);
s8 func_80001BE8(void);
void __scMain(void *arg);

/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osCreateScheduler, with Mickey's VI mode symbols. */
void osCreateScheduler(OSSched *sc, void *stack, OSPri priority, u8 mode,
                       u8 numFields) {
    sc->curRSPTask = NULL;
    sc->curRDPTask = NULL;
    sc->clientList = NULL;
    sc->audioListHead = NULL;
    sc->gfxListHead = NULL;
    sc->audioListTail = NULL;
    sc->gfxListTail = NULL;
    sc->frameCount = 0;
    sc->unkTask = NULL;
    *(u16 *) sc->retraceMsg = 1;
    *(u16 *) sc->prenmiMsg = 4;

    osCreateViManager(254);
    switch (mode) {
        case 14:
            osViSetMode(&D_80080490);
            break;
        case 28:
            osViSetMode(&D_800804E0);
            break;
        default:
            osViSetMode(&D_80080530);
            break;
    }
    osViBlack(1);
    osCreateMesgQueue(&sc->interruptQ, sc->intBuf, 8);
    osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, 8);
    osSetEventMesg(4, &sc->interruptQ, (OSMesg) RSP_DONE_MSG);
    osSetEventMesg(9, &sc->interruptQ, (OSMesg) RDP_DONE_MSG);
    osSetEventMesg(14, &sc->interruptQ, (OSMesg) PRE_NMI_MSG);
    osViSetEvent(&sc->interruptQ, (OSMesg) VIDEO_MSG, numFields);

    osCreateThread((OSThread *) sc->threadAndPad, 5, __scMain, sc, stack,
                   priority);
    osStartThread((OSThread *) sc->threadAndPad);
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osScAddClient. */
void osScAddClient(OSSched *sc, OSScClient *client, OSMesgQueue *msgQ, u8 id) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->msgQ = msgQ;
    client->next = sc->clientList;
    client->id = id;
    sc->clientList = client;
    osSetIntMask(mask);
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osScRemoveClient. */
void osScRemoveClient(OSSched *sc, OSScClient *clientToRemove) {
    OSScClient *client = sc->clientList;
    OSScClient *previous = NULL;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    while (client != NULL) {
        if (client == clientToRemove) {
            if (previous != NULL) {
                previous->next = clientToRemove->next;
            } else {
                sc->clientList = clientToRemove->next;
            }
            break;
        }
        previous = client;
        client = client->next;
    }
    osSetIntMask(mask);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetCmdQ. */
OSMesgQueue *osScGetCmdQ(OSSched *scheduler) {
    return &scheduler->cmdQ;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetInterruptQ. */
OSMesgQueue *osScGetInterruptQ(OSSched *scheduler) {
    return &scheduler->interruptQ;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/sched.c:osScGetAudioSPStats. */
void osScGetAudioSPStats(f32 *first, f32 *second, f32 *third) {
    *first = 0.0f;
    *second = 0.0f;
    *third = 0.0f;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scMain, with Mickey's retrace-counter symbol. */
void __scMain(void *arg) {
    OSMesg msg = NULL;
    OSSched *sc = (OSSched *) arg;
    OSScClient *client;
    s32 state = 0;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;

    while (1) {
        osRecvMesg(&sc->interruptQ, &msg, OS_MESG_BLOCK);

        switch ((s32) msg) {
            case VIDEO_MSG:
                __scHandleRetrace(sc);
                D_8007A65C++;
                break;
            case RSP_DONE_MSG:
                __scHandleRSP(sc);
                break;
            case RDP_DONE_MSG:
                __scHandleRDP(sc);
                break;
            case UNK_MSG:
                func_800304E0(sc);
                break;
            case PRE_NMI_MSG:
                for (client = sc->clientList; client != NULL;
                     client = client->next) {
                    osSendMesg(client->msgQ, (OSMesg) sc->prenmiMsg,
                               OS_MESG_NOBLOCK);
                }
                break;
            default:
                __scAppendList(sc, (OSScTask *) msg);
                state = ((sc->curRSPTask == NULL) << 1) |
                        (sc->curRDPTask == NULL);
                if (__scSchedule(sc, &sp, &dp, state) != state) {
                    __scExec(sc, sp, dp);
                }
                break;
        }
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:func_8004FB30_50730. */
void func_800304E0(OSSched *sc) {
    s32 state;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;

    if (sc->audioListHead != NULL) {
        sc->doAudio = 1;
    }
    if (sc->doAudio != 0 && sc->curRSPTask != NULL) {
        __scYield(sc);
    } else {
        state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
        if (__scSchedule(sc, &sp, &dp, state) != state) {
            __scExec(sc, sp, dp);
        }
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:osScGetTaskType, with Mickey's own string symbols. */
char *osScGetTaskType(s32 taskID) {
    switch (taskID) {
        case 1:
            return D_80082090;
        case 2:
            return D_800820A0;
        case 3:
            return D_800820AC;
        case 4:
            return D_800820B8;
        case 5:
            return D_800820D0;
        case 6:
            return D_800820E0;
        case 7:
            return D_800820F0;
        default:
            return D_80082100;
    }
}
void func_80030608(OSScTask *arg0) {
}
#ifdef NON_MATCHING
/* 2026-09-18: frame 0x98 and the stack-home set now match. L145 reuse of
 * commandIndex as the 0xB8 walk counter drops S 48 to 40; the four unused
 * s8* pads realize declaration order [8][savedCommands][done][8][message]
 * so L is 40. Deleting the address carrier puts the one used temp at 0x4C.
 * Size is 193/192: one extra addiu of D_80000000 at +0x180. A literal first
 * addend CSEs with the 0x80000000U compare and reopens size +8. See the EOF
 * handoff. */
SchedGfx *func_80030610(OSSched *sc, s32 commandIndex,
                        SchedGfx *displayList, OSMesgQueue *queue,
                        u64 *dataStart) {
    s8 *padAbove0;
    s8 *padAbove1;
    s64 savedCommands[2];
    s32 done = 0;
    s8 *padMid0;
    s8 *padMid1;
    OSMesg message = NULL;
    SchedGfx *nextCommand;
    s32 numRetraces;
    s32 gotRdpDone;
    s8 *commandStart;
    s8 *printStart;
    u32 nestedStart;

    u32 nestedCommand;

    do {
        nextCommand = displayList + 1;
        __osSpSetStatus(0xAAAA82);
        gotRdpDone = 0;
        numRetraces = 0;
        osDpSetStatus(0x1D6);

        savedCommands[1] = *(s64 *) displayList;
        savedCommands[0] = *(s64 *) (displayList + 1);
        displayList->w1 = 0;
        displayList->w0 = 0xE9000000;
        (displayList + 1)->w1 = 0;
        (displayList + 1)->w0 = 0xB8000000;

        osWritebackDCacheAll();
        osSpTaskLoad(&sc->curRSPTask->list);
        osSpTaskStartGo(&sc->curRSPTask->list);

        do {
            osRecvMesg(queue, &message, OS_MESG_BLOCK);
            switch ((s32) message) {
                case VIDEO_MSG:
                    numRetraces++;
                    break;
                case RDP_DONE_MSG:
                    gotRdpDone = 1;
                    break;
                case RSP_DONE_MSG:
                    break;
            }
        } while (numRetraces < 10 && gotRdpDone == 0);

        *(s64 *) displayList = savedCommands[1];
        *(s64 *) nextCommand = savedCommands[0];

        if (commandIndex < 2) {
            if (*(u8 *) displayList == 6) {
                commandStart = (s8 *) displayList - 0x140;
                printStart = commandStart;
                if ((u32) commandStart < 0x80000000U) {
                    printStart = commandStart + (u32) D_80000000;
                }
                if ((s32) printStart < (s32) dataStart) {
                    printStart = (s8 *) dataStart;
                }
                diRcpPrintDL((SchedGfx *) printStart, displayList, 0x50);

                nestedCommand = displayList->w1;
                if (nestedCommand < 0x80000000U) {
                    nestedCommand += (u32) D_80000000;
                }
                nestedStart = nestedCommand;
                commandIndex = 0;
                while (*(s8 *) nestedCommand != (s8) 0xB8) {
                    nestedCommand += 8;
                    commandIndex++;
                }
                if (commandIndex & 1) {
                    commandIndex = commandIndex / 2 + 1;
                } else {
                    commandIndex = commandIndex / 2;
                }
                displayList = (SchedGfx *) (nestedStart + commandIndex * 8);
                if ((u32) displayList < 0x80000000U) {
                    displayList = (SchedGfx *) ((u32) displayList + (u32) D_80000000);
                }
                diRcpPrintDL((SchedGfx *) nestedStart, displayList, 0xA0);
                return func_80030610(sc, commandIndex, displayList, queue,
                                     (u64 *) nestedStart);
            }
            done = 1;
        }

        if (done == 0) {
            if (commandIndex & 1) {
                commandIndex = commandIndex / 2 + 1;
            } else {
                commandIndex = commandIndex / 2;
            }
            if (gotRdpDone != 0) {
                displayList += commandIndex;
            } else {
                displayList -= commandIndex;
            }
        }
    } while (done == 0);

    return displayList;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/sched/func_80030610.s")
#endif
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:func_8004FF64_50B64, with Mickey's extracted trace helper. */
SchedGfx *func_80030910(OSSched *sc, s32 *arg1, s32 *arg2, s32 *arg3,
                        s32 *arg4, s32 *arg5, s32 *arg6) {
    u32 commandIndex;
    OSMesg queueBuffer[8];
    OSMesgQueue queue;
    SchedGfx *displayList;
    OSTask *task;
    s32 file2;
    s32 file;
    s32 unkC2;
    s32 unkC;
    s32 unk102;
    s32 unk10;
    u32 startAddress;

    task = &sc->curRSPTask->list;
    commandIndex = task->t.data_size >> 1;
    displayList = (SchedGfx *) &task->t.data_ptr[commandIndex];

    osCreateMesgQueue(&queue, queueBuffer, 8);
    osSetEventMesg(4, &queue, (OSMesg) RSP_DONE_MSG);
    osSetEventMesg(9, &queue, (OSMesg) RDP_DONE_MSG);
    osViSetEvent(&queue, (OSMesg) VIDEO_MSG, 1);
    displayList = func_80030610(sc, commandIndex, displayList, &queue,
                               task->t.data_ptr);

    osSetEventMesg(4, &sc->interruptQ, (OSMesg) RSP_DONE_MSG);
    osSetEventMesg(9, &sc->interruptQ, (OSMesg) RDP_DONE_MSG);
    osViSetEvent(&sc->interruptQ, (OSMesg) VIDEO_MSG, 1);

    *arg4 = 0;
    *arg1 = 0;
    func_80044C94(displayList, &file, &unkC, &unk10, &file2, &unkC2,
                  &unk102);
    if (file2 != 0 || file != 0) {
        if (file != 0) {
            *arg1 = file;
            *arg2 = unkC;
            *arg3 = unk10;
        }
        if (file2 != 0) {
            *arg4 = file2;
            *arg5 = unkC2;
            *arg6 = unk102;
        }
    }

    startAddress = (u32) displayList - 0x140;
    if ((void *) startAddress < (void *) 0x80000000) {
        startAddress += 0x80000000U;
    }
    if ((s32) startAddress < (s32) task->t.data_ptr) {
        startAddress = (u32) task->t.data_ptr;
    }
    diRcpPrintDL((SchedGfx *) startAddress, displayList, 0x50);
    return displayList;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scHandleRetrace at efd5abb. Mickey's counter occupies the
 * existing initialized data slot. The donor's separate read name is a weak
 * alias of that same storage, preserving IDO's load/store materialization. */
extern u64 schedRetraceCounterRead;
#pragma weak schedRetraceCounterRead = D_8007A660
void __scHandleRetrace(OSSched *sc) {
    OSScTask *rspTask = NULL;
    OSScClient *client;
    s32 state;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;
    u8 clearRSPTask = FALSE;
    u8 clearRDPTask = FALSE;
    SchedGfx *spGfx;
    SchedGfx *dpGfx;
    s32 spC4;
    s32 spC0;
    s32 spBC;
    OSScTask *spB8;
    s32 spB4;
    s32 spB0;
    s32 spAC;
    s32 spA8;
    s32 spA4;
    s32 spA0;
    s32 sp9C;
    s32 sp98;
    OSScTask *unkTask;
    OSMesg intBuf[8];
    OSMesgQueue interruptQ;
    OSScTask *curTask;
    SchedGfx *dlist;
    s32 yPos;
    s32 pad;

    if (sc->curRSPTask != NULL) {
        D_8007A650++;
    }
    if (sc->curRDPTask != NULL) {
        D_8007A654++;
    }

    spGfx = NULL;
    dpGfx = NULL;
    if ((D_8007A650 > 10) && (sc->curRSPTask != NULL)) {
        if (D_800D2D40 != 0) {
            osScGetTaskType(sc->curRSPTask->taskID);
            func_80030608(sc->curRSPTask);
            if (sc->curRSPTask->list.t.type == 1) {
                spGfx = func_80030910(sc, &spB4, &spA4, &spC4, &spB0, &spA0, &spC0);
            }
            D_800D2D40 = 0;
        }
        D_8007A650 = 0;
        clearRSPTask = TRUE;
        __osSpSetStatus(0xAAAA82);
    } else if (sc->curRSPTask != NULL) {
        D_800D2D40 = 1;
    }

    if ((D_8007A654 > 10) && (sc->curRDPTask != NULL)) {
        if (sc->curRDPTask->unk68 == 0) {
            osSendMesg(sc->curRDPTask->msgQ, &D_8007A648, OS_MESG_BLOCK);
        }
        if (D_800D2D44 != 0) {
            osScGetTaskType(sc->curRDPTask->taskID);
            func_80030608(sc->curRDPTask);
            if (sc->curRDPTask->list.t.type == 1) {
                dpGfx = func_80030910(sc, &spAC, &sp9C, &spBC, &spA8, &sp98, (s32 *) &spB8);
            }
            D_800D2D44 = 0;
        }
        clearRDPTask = TRUE;
        sc->frameCount = 0;
        D_8007A654 = 0;
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
    } else if (sc->curRDPTask != NULL) {
        D_800D2D44 = 1;
    }

    if ((spGfx != NULL) || (dpGfx != NULL)) {
        osCreateMesgQueue(&interruptQ, intBuf, 8);
        osSetEventMesg(4, &interruptQ, (OSMesg) 667);
        osSetEventMesg(9, &interruptQ, (OSMesg) 668);
        osViSetEvent(&interruptQ, (OSMesg) 666, 1);
        if (sc->curRSPTask != NULL) {
            curTask = sc->curRSPTask;
        } else {
            curTask = sc->curRDPTask;
        }
        dlist = (SchedGfx *) curTask->list.t.data_ptr;
        rsp_segment(&dlist, 0, 0);
        rsp_segment(&dlist, 1, D_800D2FA8);
        rsp_segment(&dlist, 2, D_800D2FAC);
        rsp_segment(&dlist, 4, D_800D2FA0 - 0x500);
        if (spGfx != NULL) {
            diPrintfSetXY(30, 30);
            diPrintf(D_80082354, spGfx);
        }
        if (dpGfx != NULL) {
            diPrintfSetXY(30, 70);
            diPrintf(D_80082368, dpGfx);
        }
        yPos = 110;
        if (D_800CF520 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_8008237C);
            yPos += 10;
        }
        if (D_800CF578 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_80082390);
            yPos += 10;
        }
        if (D_800CF590 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_800823A4);
            yPos += 10;
        }
        if (D_800CF5A8 != 0) {
            diPrintfSetXY(30, yPos);
            diPrintf(D_800823B8);
            yPos += 10;
            clearRDPTask = FALSE;
        }
        spGfx = NULL;
        dpGfx = NULL;
        diPrintfSetXY(30, yPos + 10);
        diPrintf(D_800823CC, D_80082350);
        diPrintfAll(&dlist);
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
        {
            SchedGfx *cmd = dlist++;
            cmd->w1 = 0;
            cmd->w0 = 0xE9000000;
        }
        {
            SchedGfx *cmd = dlist++;
            cmd->w1 = 0;
            cmd->w0 = 0xB8000000;
        }
        osWritebackDCacheAll();
        osSpTaskLoad(&curTask->list);
        osSpTaskStartGo(&curTask->list);
        while (TRUE) {
        }
    }

    if (clearRSPTask) {
        sc->curRSPTask = NULL;
    }
    if (clearRDPTask) {
        sc->curRDPTask = NULL;
    }

    while (osRecvMesg(&sc->cmdQ, (OSMesg *) &rspTask, OS_MESG_NOBLOCK) != -1) {
        __scAppendList(sc, rspTask);
    }

    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }

    D_8007A660 = schedRetraceCounterRead + 1;
    sc->frameCount++;
    if ((sc->unkTask != NULL) && (sc->frameCount >= 2)) {
        unkTask = sc->unkTask;
        if (unkTask->msgQ != NULL) {
            if ((unkTask->unk68 != 0) || (unkTask->msg != NULL)) {
                osSendMesg(unkTask->msgQ, unkTask->msg, OS_MESG_BLOCK);
            } else {
                osSendMesg(unkTask->msgQ, &D_8007A640, OS_MESG_BLOCK);
            }
        }
        sc->frameCount = 0;
        sc->unkTask = NULL;
    }

    for (client = sc->clientList; client != NULL; client = client->next) {
        if (client->id == 1) {
            D_8007A658--;
            if (D_8007A658 <= 0) {
                osSendMesg(client->msgQ, sc, OS_MESG_NOBLOCK);
                if (sc->audioListHead != NULL) {
                    func_800304E0(sc);
                }
                D_8007A658 = func_80001BE8();
            }
        } else if (client->id == 2) {
            osSendMesg(client->msgQ, sc, OS_MESG_NOBLOCK);
        }
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scHandleRSP. */
void __scHandleRSP(OSSched *sc) {
    OSScTask *task;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;
    s32 state;

    task = sc->curRSPTask;
    sc->curRSPTask = NULL;
    if (task->state & 0x10) {
        if (osSpTaskYielded(&task->list) != 0) {
            task->state |= 0x20;
            if ((task->flags & 7) == 3) {
                task->next = sc->gfxListHead;
                sc->gfxListHead = task;
                if (sc->gfxListTail == NULL) {
                    sc->gfxListTail = task;
                }
            }
        } else {
            task->state &= ~2;
            do {
            } while (0);
        }
        /* Inert allocation aid retained by exact C; tracked in
         * docs/cleanup-queue.md. */
        if ((task->flags & 7) != 3) {
        }
    } else {
        task->state &= ~2;
        __scTaskComplete(sc, task);
    }

    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scHandleRDP. */
void __scHandleRDP(OSSched *sc) {
    OSScTask *task;
    OSScTask *sp = NULL;
    OSScTask *dp = NULL;
    s32 state;

    task = sc->curRDPTask;
    sc->curRDPTask = NULL;
    task->state &= ~1;
    __scTaskComplete(sc, task);
    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scTaskReady. */
OSScTask *__scTaskReady(OSScTask *task) {
    if (task != NULL) {
        if (osViGetCurrentFramebuffer() != osViGetNextFramebuffer()) {
            return NULL;
        }
        return task;
    }
    return NULL;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scTaskComplete. */
s32 __scTaskComplete(OSSched *sc, OSScTask *task) {
    if ((task->state & 3) == 0) {
        if (task->msgQ != NULL) {
            if (task->flags & 0x20) {
                if (sc->frameCount <= 1) {
                    sc->unkTask = task;
                    return 1;
                }
                if (task->unk68 != 0 || task->msg != NULL) {
                    osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
                } else {
                    osSendMesg(task->msgQ, &D_8007A640, OS_MESG_BLOCK);
                }
                sc->frameCount = 0;
                return 1;
            }
            if (task->unk68 != 0 || task->msg != NULL) {
                osSendMesg(task->msgQ, task->msg, OS_MESG_BLOCK);
                return 1;
            }
            osSendMesg(task->msgQ, &D_8007A640, OS_MESG_BLOCK);
        }
        return 1;
    }
    return 0;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scAppendList. */
void __scAppendList(OSSched *sc, OSScTask *task) {
    s32 type = task->list.t.type;

    if (type == 2) {
        if (sc->audioListTail != NULL) {
            sc->audioListTail->next = task;
        } else {
            sc->audioListHead = task;
        }
        sc->audioListTail = task;
    } else {
        if (sc->gfxListTail != NULL) {
            sc->gfxListTail->next = task;
        } else {
            sc->gfxListHead = task;
        }
        sc->gfxListTail = task;
    }
    task->next = NULL;
    task->state = task->flags & 3;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scExec, with Mickey's task counters. */
void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp) {
    if (sp != NULL) {
        if (sp->list.t.type == 2) {
            osWritebackDCacheAll();
        }
        sp->state &= ~0x30;
        osSpTaskLoad(&sp->list);
        osSpTaskStartGo(&sp->list);
        D_8007A650 = 0;
        D_8007A654 = 0;
        sc->curRSPTask = sp;
        if (sp == dp) {
            sc->curRDPTask = dp;
        }
    }
    if (dp != NULL && dp != sp) {
        osDpSetNextBuffer(dp->list.t.output_buff,
                          *dp->list.t.output_buff_size);
        sc->curRDPTask = dp;
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/sched.c:__scYield. */
void __scYield(OSSched *sc) {
    if (sc->curRSPTask->list.t.type == 1) {
        sc->curRSPTask->state |= 0x10;
        D_800D2D48 = osGetTime();
        osSpTaskYield();
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/sched.c:__scSchedule. */
s32 __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp, s32 availRCP) {
    s32 avail = availRCP;
    OSScTask *gfx = sc->gfxListHead;
    OSScTask *audio = sc->audioListHead;

    if (sc->doAudio && (avail & OS_SC_SP)) {
        if (gfx && (gfx->flags & OS_SC_PARALLEL_TASK)) {
            *sp = gfx;
            avail &= ~OS_SC_SP;
        } else {
            *sp = audio;
            avail &= ~OS_SC_SP;
            sc->doAudio = 0;
            sc->audioListHead = sc->audioListHead->next;
            if (sc->audioListHead == NULL) {
                sc->audioListTail = NULL;
            }
        }
    } else if (__scTaskReady(gfx)) {
        switch (gfx->flags & OS_SC_TYPE_MASK) {
            case OS_SC_XBUS:
                if (gfx->state & OS_SC_YIELDED) {
                    if (avail & OS_SC_SP) {
                        *sp = gfx;
                        avail &= ~OS_SC_SP;
                        if (gfx->state & OS_SC_DP) {
                            *dp = gfx;
                            avail &= ~OS_SC_DP;
                        }
                        sc->gfxListHead = sc->gfxListHead->next;
                        if (sc->gfxListHead == NULL) {
                            sc->gfxListTail = NULL;
                        }
                    }
                } else if (avail == (OS_SC_SP | OS_SC_DP)) {
                    *sp = *dp = gfx;
                    avail &= ~(OS_SC_SP | OS_SC_DP);
                    sc->gfxListHead = sc->gfxListHead->next;
                    if (sc->gfxListHead == NULL) {
                        sc->gfxListTail = NULL;
                    }
                }
                break;

            case OS_SC_DRAM:
            case OS_SC_DP_DRAM:
            case OS_SC_DP_XBUS:
                if (gfx->state & OS_SC_SP) {
                    if (avail & OS_SC_SP) {
                        *sp = gfx;
                        avail &= ~OS_SC_SP;
                    }
                }
                if (gfx->state & OS_SC_DP) {
                    if (avail & OS_SC_DP) {
                        *dp = gfx;
                        avail &= ~OS_SC_DP;
                        sc->gfxListHead = sc->gfxListHead->next;
                        if (sc->gfxListHead == NULL) {
                            sc->gfxListTail = NULL;
                        }
                    }
                }
                break;

            case OS_SC_SP_DRAM:
            case OS_SC_SP_XBUS:
            default:
                break;
        }
    }

    if (avail != availRCP) {
        avail = __scSchedule(sc, sp, dp, avail);
    }
    return avail;
}

/* PLATEAU-HANDOFF:func_80030610:start
 * symbol: func_80030610
 * score: 104 differing words
 * frame: 0x98
 * relocations: 13
 * first-mismatch: +0x8C
 * summary: Extra lo addiu of the symbol addend at +0x180; target's addend is a lui-only constant web apart from the compare constant.
 * PLATEAU-HANDOFF:func_80030610:end
 */
