#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os.h"
#include "PR/os_pi.h"
#include "n_audio/libaudio.h"
#include "n_libaudio.h"
#include "game/sched_internal.h"

typedef struct AudioManagerDMABuffer {
    ALLink node;
    u32 startAddr;
    u32 lastFrame;
    char *ptr;
} AudioManagerDMABuffer;

typedef struct AudioManagerDMAState {
    u8 initialized;
    u8 pad1[3];
    AudioManagerDMABuffer *firstUsed;
    AudioManagerDMABuffer *firstFree;
} AudioManagerDMAState;

typedef struct Overlay5SoundConfig {
    void *field00;
    s32 field04;
    s32 field08;
    s32 field0C;
    s32 field10;
    void *field14;
    s32 field18;
    s8 field1C;
} Overlay5SoundConfig;

typedef struct AudioManagerConfig {
    void *field00;
    s32 field04;
    s32 field08;
    s32 maxFXbusses;
    ALDMANew dmaproc;
    ALHeap *heap;
    s32 outputRate;
    u8 fxType[4];
    void *params[2];
} AudioManagerConfig;

typedef struct AudioManagerState {
    u8 pad000[0x280];
    u8 *bufferStart;
    u8 *bufferEnd;
    u8 *altBufferStart;
    u8 *altBufferEnd;
    void *largeBufferStart;
    void *largeBufferEnd;
    OSScTask task;
    s16 frameSamples[3];
    u16 pad30E;
    u8 *cmdLists[3];
    u8 *cmdListsAlt[3];
    u8 *largeData[2];
} AudioManagerState;

extern AudioManagerDMAState D_800C7DF8;
extern AudioManagerDMABuffer D_800C7E08[];
extern s32 func_80002188(s32 addr, s32 len, void *state);
extern OSThread D_800C7A50;
extern s16 D_800C7A56[];
extern ALHeap *D_800BFA34;
extern u8 D_80078DF4[];
extern u32 D_80078DD0;
extern u32 D_80078DD4;
extern AudioManagerDMABuffer *D_80078DC0;
extern s32 D_80078DC4;
extern s32 D_80078DC8;
extern s32 D_80078DCC;
extern s32 D_80078DD8;
extern s32 D_80078DDC;
extern s32 D_80078DE0;
extern s32 D_80078DE4;
extern s32 D_80078DE8;
extern s32 D_80078DEC;
extern volatile u32 D_80078DF0;
extern OSMesgQueue D_800C9020;
extern OSIoMesg D_800C8648[];
extern OSSched *D_800BFA30;
extern OSScClient D_800BFA38;
extern OSMesgQueue D_800C7D84;
extern OSMesgQueue D_800C7D9C;
extern OSMesg D_800C7DB4[];
extern OSMesg D_800C7DD4[];
extern N_ALGlobals D_800C7C80;
extern u64 D_800C7A48[];
extern OSMesg D_800C9038[];
extern s32 D_800D2FB0;
extern s32 D_800C863C;
extern s32 D_800C8640;
extern s32 D_800C8644;
extern s32 D_800C91DC;
extern OSIoMesg D_800C8648[];
extern OSScTask D_800C7CE8;
extern u32 D_A4500004;
extern u64 D_80076110[];
extern u64 D_80077950[];
extern u64 D_80077AD0[];
extern u64 D_80084B00[];
extern void *func_8002B280(s32 size, s32 tag);
extern void mmFree(void *address);
extern s32 osAiSetFrequency(s32 frequency);
extern s32 osAiSetNextBuffer(void *bufPtr, u32 size);
extern void osScAddClient(OSSched *sc, OSScClient *client, OSMesgQueue *msgQ, u8 id);
extern OSMesgQueue *osScGetCmdQ(OSSched *scheduler);
extern ALDMAproc audioManager_DMAInitProc(void *state);
extern void func_80001A84(void *arg);
extern void func_8000238C(void);
extern void func_80001BF4(void);
extern void func_80002134(void);

/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/audiomgr.c::amCreateAudioMgr, cross-checked against Jet Force Gemini
 * efd5abb src/audiomgr.c::amCreateAudioMgr (still unmatched there; same SGI
 * initializer call order). Mickey's config, heap sizes, DMA stride, and
 * queue depth remain authoritative. */
/* Verdict: size-mismatch; 153 masked words, 203 candidate vs 209 target instructions. */
/* First mismatch: function offset +0x00; candidate frame 0x158 vs target 0x150. */
/* Gap: six missing words (sltu, two lui, addiu, or, nop) and 8 extra frame bytes. */
#ifdef NON_MATCHING
typedef struct AudioManagerEffectParams {
    u32 words[0x108 / sizeof(u32)];
} AudioManagerEffectParams;

void func_80001740(Overlay5SoundConfig *configArg, s32 priority, void *context) {
    s32 i;
    f32 fsize;
    u8 *dmaMemory;
    AudioManagerDMABuffer *dmaPtr;
    AudioManagerDMABuffer *prevDmaPtr;

    D_800BFA30 = (OSSched *)context;
    D_800BFA34 = (ALHeap *)configArg->field14;
    configArg->field18 = osAiSetFrequency(0x5604);
    ((AudioManagerConfig *)configArg)->dmaproc = audioManager_DMAInitProc;

    for (i = 0; i < configArg->field0C; i++) {
        if (((AudioManagerConfig *)configArg)->fxType[i] == AL_FX_CUSTOM) {
            AudioManagerEffectParams customParams;

            customParams = *(AudioManagerEffectParams *)D_80078DF4;
            ((AudioManagerConfig *)configArg)->params[i] = &customParams;
        }
    }

    n_alInit(&D_800C7C80, (ALSynConfig *)configArg);
    fsize = ((f32)configArg->field18 * 2) / (f32)D_800D2FB0;
    i = (s32)fsize;
    D_800C863C = i;
    if ((f32)i < fsize) {
        i++;
        D_800C863C = i;
    }
    i = ((i / 0xB8) * 0xB8) + 0xB8;
    D_800C863C = i;
    D_800C8640 = i - 0xB8;
    D_800C8644 = 0x1000;

    ((AudioManagerState *)&D_800C7A50)->bufferStart =
        alHeapDBAlloc(0, 0, (ALHeap *)configArg->field14, 1, 0x7580);
    ((AudioManagerState *)&D_800C7A50)->bufferEnd =
        ((AudioManagerState *)&D_800C7A50)->bufferStart + 0x3AC0;
    ((AudioManagerState *)&D_800C7A50)->altBufferStart =
        ((AudioManagerState *)&D_800C7A50)->bufferStart;
    ((AudioManagerState *)&D_800C7A50)->altBufferEnd =
        ((AudioManagerState *)&D_800C7A50)->bufferEnd;

    dmaMemory = alHeapDBAlloc(0, 0, (ALHeap *)configArg->field14, 1,
                              D_800C8644 * 0xC);
    for (i = 0; i < 3; i++) {
        ((AudioManagerState *)&D_800C7A50)->cmdLists[i] = dmaMemory;
        ((AudioManagerState *)&D_800C7A50)->cmdListsAlt[i] = dmaMemory;
        ((AudioManagerState *)&D_800C7A50)->frameSamples[i] = 0;
        dmaMemory += D_800C8644 * 4;
    }

    dmaMemory = alHeapDBAlloc(0, 0, (ALHeap *)configArg->field14, 1, 0xD200);
    D_800C7E08[0].node.prev = NULL;
    D_800C7E08[0].node.next = NULL;
    dmaPtr = &D_800C7E08[1];
    prevDmaPtr = &D_800C7E08[0];
    do {
        alLink(&dmaPtr->node, &prevDmaPtr->node);
        dmaPtr++;
        prevDmaPtr->ptr = dmaMemory;
        prevDmaPtr++;
        dmaMemory += 0x200;
    } while (dmaPtr != (AudioManagerDMABuffer *)&D_800C863C);
    prevDmaPtr->ptr = dmaMemory;

    osCreateMesgQueue(&D_800C7D9C, D_800C7DD4, 8);
    osCreateMesgQueue(&D_800C7D84, D_800C7DB4, 8);
    osCreateMesgQueue(&D_800C9020, D_800C9038, 0x69);
    osCreateThread((OSThread *)&D_800C7A50, -4, func_80001A84, NULL,
                   D_800C7A48, priority);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/audiomgr/func_80001740.s")
#endif

/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/audiomgr.c::__amMain; Mickey's queue globals and message flow remain authoritative. */
void func_80001A84(void *arg) {
    s16 *msg = NULL;
    s16 *doneMsg = NULL;
    s32 done = 0;

    (void)arg;
    osScAddClient(D_800BFA30, &D_800BFA38, &D_800C7D84, 1);
    do {
        osRecvMesg(&D_800C7D84, (OSMesg *)&msg, OS_MESG_BLOCK);
        switch (*msg) {
            case 4:
                break;
            case 1:
                func_80001BF4();
                osRecvMesg(&D_800C7D9C, (OSMesg *)&doneMsg, OS_MESG_BLOCK);
                func_80002134();
                break;
            case 10:
                done = 1;
                break;
        }
    } while (done == 0);
    n_alClose(&D_800C7C80);
}
void func_80001BA0(void) {
    osStartThread(&D_800C7A50);
}
void func_80001BC4(void) {
    osStopThread(&D_800C7A50);
}
extern s32 D_80078DEC;

s32 func_80001BE8(void) {
    return D_80078DEC;
}
/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/audiomgr.c::__amHandleFrameMsg and __clearAudioDMA; Mickey's manager
 * fields, schedule state, and task layout remain authoritative. */
/* Verdict: structure-mismatch; 350 differing sites, 372 candidate vs 336 target instructions. */
/* First mismatch: function offset +0x00; candidate frame -96 vs target -88. */
/* Gap: manager/task-base and cleanup/large-mode loop shapes remain displaced. */
#ifdef NON_MATCHING
void func_80001BF4(void) {
    register AudioManagerState *manager;
    AudioManagerDMABuffer *dmaBase;
    AudioManagerDMABuffer *dmaPtr;
    Acmd *cmdList;
    Acmd *cmdp;
    u8 *work;
    u8 *buffer;
    u8 *audioPtr;
    s32 samplesLeft;
    s32 cmdLen;
    s32 i;

    func_8000238C();
    manager = (AudioManagerState *)&D_800C7A50;
    samplesLeft = D_A4500004 >> 2;
    osAiSetNextBuffer(
        manager->cmdLists[D_80078DCC],
        manager->frameSamples[D_80078DCC] << 2);

    if (D_80078DDC == 1) {
        D_800C91DC += 2;
        if (D_800C91DC >= 6) {
            D_800C91DC = 6;
            D_80078DE8 = 1;
        } else {
            D_80078DE8 = 0;
        }
    }

    if (D_80078DE4 > 0) {
        D_80078DE4--;
        if (D_80078DE4 <= 0) {
            mmFree(manager->largeBufferStart);
            mmFree(manager->largeData[0]);
            for (i = 0; i != 0x834; i += sizeof(AudioManagerDMABuffer)) {
                dmaPtr = (AudioManagerDMABuffer *)((u8 *)D_80078DC0 + i);
                if (D_800C7DF8.firstUsed == dmaPtr) {
                    D_800C7DF8.firstUsed =
                        (AudioManagerDMABuffer *)dmaPtr->node.next;
                }
                if (D_800C7DF8.firstFree == dmaPtr) {
                    D_800C7DF8.firstFree =
                        (AudioManagerDMABuffer *)dmaPtr->node.next;
                }
                alUnlink(&dmaPtr->node);
            }
            mmFree(D_80078DC0);
            D_80078DC0 = NULL;
        }
    }

    if (D_80078DE0 != D_80078DDC) {
        D_80078DE8 = 0;
        if (((D_80078DDC ^ 1) == 0) && (D_80078DC0 == NULL)) {
            manager->largeBufferStart = func_8002B280(0x2C100, 0x82);
            manager->largeBufferEnd =
                (u8 *)manager->largeBufferStart + 0x16080;
            work = func_8002B280(D_800C8644 * 0x48, 0x82);
            for (i = 0; i < 2; i++) {
                manager->cmdLists[i] = work;
                manager->largeData[i] = work;
                work += D_800C8644 * 0x18;
            }
            manager->bufferStart = manager->largeBufferStart;
            manager->bufferEnd = manager->largeBufferEnd;
            dmaBase = func_8002B280(0xDA34, 0x82);
            D_80078DC0 = dmaBase;
            if (D_800C7DF8.firstFree != NULL) {
                alLink(&dmaBase->node, &D_800C7DF8.firstFree->node);
            } else {
                D_800C7DF8.firstFree = dmaBase;
                dmaBase->node.next = NULL;
                dmaBase->node.prev = NULL;
            }
            dmaPtr = dmaBase;
            buffer = (u8 *)dmaBase + 0x834;
            for (i = 0; i != 0x820; i += sizeof(AudioManagerDMABuffer)) {
                alLink(&(dmaPtr + 1)->node, &dmaPtr->node);
                dmaPtr->ptr = (char *)buffer;
                dmaPtr++;
                buffer += 0x200;
            }
            dmaPtr->ptr = (char *)buffer;
            D_800C91DC = 1;
        } else {
            for (i = 0; i < 2; i++) {
                manager->cmdLists[i] = manager->cmdListsAlt[i];
            }
            D_80078DE4 = 0xC;
            manager->bufferStart = manager->altBufferStart;
            manager->bufferEnd = manager->altBufferEnd;
        }
        D_80078DE0 = D_80078DDC;
    }

    if (D_80078DDC == 0) {
        D_80078DEC = 2;
    } else {
        D_80078DEC = D_800C91DC * 2;
    }

    audioPtr = (u8 *)osVirtualToPhysical(
        manager->cmdLists[D_80078DC8]);
    if (((samplesLeft >= 0x159) != 0) && (D_80078DD8 != 0)) {
        D_80078DD8 = 0;
        manager->frameSamples[D_80078DC8] = D_800C8640;
    } else {
        D_80078DD8 = 1;
        manager->frameSamples[D_80078DC8] = D_800C863C;
    }
    if (D_80078DDC == 1) {
        manager->frameSamples[D_80078DC8] *= D_800C91DC;
    }

    cmdList = (Acmd *)((u8 **)&manager->bufferStart)[D_80078DC4];
    cmdp = n_alAudioFrame(cmdList, &cmdLen, (s16 *)audioPtr,
                          manager->frameSamples[D_80078DC4]);

    manager->task.msgQ = &D_800C7D9C;
    manager->task.taskID = 1;
    manager->task.unk58 = -1;
    manager->task.flags = 2;
    manager->task.next = NULL;
    manager->task.msg = NULL;
    manager->task.unk5C = 0;
    manager->task.unk60 = 0xFF;
    manager->task.unk64 = 0;
    manager->task.list.t.type = M_AUDTASK;
    manager->task.list.t.flags = OS_TASK_DP_WAIT;
    manager->task.list.t.ucode_boot = D_80077950;
    manager->task.list.t.ucode_boot_size =
        (u8 *)D_80077AD0 - (u8 *)D_80077950;
    manager->task.list.t.ucode = D_80076110;
    manager->task.list.t.ucode_data = D_80084B00;
    manager->task.list.t.ucode_data_size = 0x800;
    manager->task.list.t.data_ptr = (u64 *)cmdList;
    manager->task.list.t.data_size = (cmdp - cmdList) * sizeof(Acmd);
    manager->task.list.t.yield_data_ptr = NULL;
    manager->task.list.t.yield_data_size = 0;

    osSendMesg(osScGetCmdQ(D_800BFA30), (OSMesg)&D_800C7CE8, 0);
    D_80078DC4 ^= 1;
    D_80078DCC = D_80078DC8;
    D_80078DC8 = (D_80078DC8 + 1) % 3;
    D_80078DD0++;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/audiomgr/func_80001BF4.s")
#endif
/* PROVENANCE: control-flow and audio-completion intent cross-checked against Jet Force Gemini's
 * public src/audiomgr.c::__amHandleDoneMsg; Mickey's ROM-derived globals remain authoritative. */
/* Matched 2026-09-17 (lane w6-audio), 9 -> 0 masked words at delta 0,
 * frame 0x18, seven relocations, unforced. L131: two pointer names for
 * &D_80078EFC split the address range so the load is a folded %hi/%lo and
 * the store rematerializes through $at. One name CSEs and keeps a pointer. */
extern volatile s32 D_80078EFC;

void func_80002134(void) {
    s32 *p;
    s32 *q;

    if ((osAiGetLength() >> 2) == 0) {
        p = &D_80078EFC;
        q = &D_80078EFC;
        if (*p == 0) {
            D_80078DF0 |= 8;
            *q = 0;
        }
    }
}
/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/audiomgr.c::__amDMA; Mickey's DMA state and queue globals remain authoritative. */
/* Configured IDO emits all 115 target instructions and 22 relocation records exactly. */
s32 func_80002188(s32 addr, s32 len, void *state)
{
  s32 addrEnd;
  AudioManagerDMABuffer *lastDmaPtr;
  void *foundBuffer;
  s32 delta;
  s32 buffEnd;
  AudioManagerDMABuffer *dmaPtr;
  s32 pad;
  lastDmaPtr = (void *) 0;
  delta = addr & 1;
  dmaPtr = D_800C7DF8.firstUsed;
  addrEnd = addr + len;
  while (dmaPtr != ((void *) 0))
  {
    buffEnd = dmaPtr->startAddr + 0x200;
    if (dmaPtr->startAddr > ((u32) addr))
    {
      break;
    }
    else
      if (addrEnd <= buffEnd)
    {
      dmaPtr->lastFrame = D_80078DD0;
      foundBuffer = (dmaPtr->ptr + addr) - dmaPtr->startAddr;
      return osVirtualToPhysical(foundBuffer);
    }
    lastDmaPtr = dmaPtr;
    dmaPtr = (AudioManagerDMABuffer *) dmaPtr->node.next;
  }

  dmaPtr = D_800C7DF8.firstFree;
  if (dmaPtr == ((void *) 0))
  {
    D_80078DF0 |= 2;
    if (lastDmaPtr == ((void *) 0))
    {
      lastDmaPtr = D_800C7DF8.firstUsed;
    }
  }
  if (dmaPtr == ((void *) 0))
  {
    return osVirtualToPhysical(lastDmaPtr->ptr) + delta;
  }
  D_800C7DF8.firstFree = (AudioManagerDMABuffer *) dmaPtr->node.next;
  alUnlink(&dmaPtr->node);
  if (lastDmaPtr != ((void *) 0))
  {
    alLink(&dmaPtr->node, &lastDmaPtr->node);
  }
  else
    if (D_800C7DF8.firstUsed != ((void *) 0))
  {
    lastDmaPtr = D_800C7DF8.firstUsed;
    D_800C7DF8.firstUsed = dmaPtr;
    dmaPtr->node.next = &lastDmaPtr->node;
    dmaPtr->node.prev = (void *) 0;
    lastDmaPtr->node.prev = &dmaPtr->node;
  }
  else
  {
 if (0) { }
    D_800C7DF8.firstUsed = dmaPtr;
    dmaPtr->node.next = (void *) 0;
    dmaPtr->node.prev = (void *) 0;
  }
  foundBuffer = dmaPtr->ptr;
  addr -= delta;
  dmaPtr->startAddr = addr;
  dmaPtr->lastFrame = D_80078DD0;
  osPiStartDma(&D_800C8648[D_80078DD4++], 1, 0, addr, foundBuffer, 0x200, &D_800C9020);
  return osVirtualToPhysical(foundBuffer) + delta;
}
/* PROVENANCE: body adapted from Banjo-Kazooie's public decomp,
 * src/core1/code_1D00.c::audioManager_DMAInitProc. */
ALDMAproc audioManager_DMAInitProc(void *state) {
    if (!D_800C7DF8.initialized) {
        D_800C7DF8.firstUsed = NULL;
        D_800C7DF8.firstFree = D_800C7E08;
        D_800C7DF8.initialized = 1;
    }
    *(void **)state = &D_800C7DF8;
    return func_80002188;
}
/* PROVENANCE: body adapted from Diddy Kong Racing's public decomp,
 * src/audiomgr.c::__clearAudioDMA; Mickey's DMA state and queue globals remain authoritative. */
void func_8000238C(void) {
    u32 i;
    OSIoMesg *iomsg = NULL;
    AudioManagerDMABuffer *dmaPtr;
    void *nextPtr;

    for (i = 0; i < D_80078DD4; i++) {
        if (osRecvMesg(&D_800C9020, (OSMesg *)&iomsg, OS_MESG_NOBLOCK) == -1) {
            D_80078DF0 |= 4;
        }
    }

    dmaPtr = D_800C7DF8.firstUsed;
    while (dmaPtr != NULL) {
        nextPtr = dmaPtr->node.next;
        if (dmaPtr->lastFrame + 1 < D_80078DD0) {
            if (D_800C7DF8.firstUsed == dmaPtr) {
                D_800C7DF8.firstUsed =
                    (AudioManagerDMABuffer *)dmaPtr->node.next;
            }
            alUnlink(&dmaPtr->node);
            if (D_800C7DF8.firstFree != NULL) {
                alLink(&dmaPtr->node, &D_800C7DF8.firstFree->node);
            } else {
                D_800C7DF8.firstFree = dmaPtr;
                dmaPtr->node.next = NULL;
                dmaPtr->node.prev = NULL;
            }
        }
        dmaPtr = (AudioManagerDMABuffer *) nextPtr;
    }

    D_80078DD4 = 0;
}

/* PLATEAU-HANDOFF:func_80001740:start
 * symbol: func_80001740
 * score: 153 differing words
 * frame: -0x158
 * relocations: 58
 * first-mismatch: +0x0
 * summary: JFG confirms 0x150 frame; best remains 203/209 at 0x158, masked 153. Missing sltu, two lui, addiu, or, nop. Next: emit those six with no new stack home.
 * PLATEAU-HANDOFF:func_80001740:end
 */

/* PLATEAU-HANDOFF:func_80001BF4:start
 * symbol: func_80001BF4
 * score: 350 differing words
 * frame: -0x60
 * relocations: 187
 * first-mismatch: +0x0
 * summary: Best DKR-derived frame handler; manager/task-base allocation and cleanup/large-mode loops remain structurally displaced.
 * PLATEAU-HANDOFF:func_80001BF4:end
 */
