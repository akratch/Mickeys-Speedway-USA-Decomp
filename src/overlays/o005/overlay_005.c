#include "overlays/overlay_005.h"

/* Overlay 5, ADR 0006 consolidation: the -O2 game-code portion. */

void overlay5InitSequence(void *owner, s32 value) {
    Overlay5SequenceHeader header;

    header.count = 14;
    header.value = value;
    overlay5SequenceInitReloc((u8 *)owner + 0x48, &header, 0);
}

/*
 * The declaration list is Diddy Kong Racing's `audio_init` list, `pad`
 * included (see the PROVENANCE note on `Overlay5SoundConfig`).  IDO packs the
 * declared locals of every scope into one chain, top-down in declaration
 * order, from T = frame, with frame = align8(below + S); reading the shipped
 * 0x98 frame and the 0x70/0x68/0x4C homes back through that law pins the list
 * exactly, and it only closes with the sound config at libaudio's real 0x24.
 */
void overlay5InitializeAudio(void *context) {
    s32 index;
    Overlay5SoundConfig soundConfig;
    Overlay5Resource *resource;
    u32 bankSize;
    u32 maxValue;
    u32 pad;
    Overlay5SequenceConfig sequenceConfig;

    maxValue = 0;
    gOverlay5AudioOwner = gOverlay5OwnerSoundState;
    alHeapInit(gOverlay5HeapState, gOverlay5HeapMemory, 0x30D40);

    resource = func_8002E148(0x31);
    gOverlay5Span0Size = resource->end - resource->span1End;
    gOverlay5Span0 = mmAlloc(gOverlay5Span0Size, 0x82);
    func_8002E2E0(0x32, gOverlay5Span0,
                  (void *)resource->span1End, gOverlay5Span0Size);
    gOverlay5Span0ScaleValue = gOverlay5Span0Size / 10U;

    gOverlay5Span1Size = resource->span1End - resource->span1Start;
    gOverlay5Span1 = mmAlloc(gOverlay5Span1Size, 0x82);
    func_8002E2E0(0x32, gOverlay5Span1,
                  (void *)resource->span1Start, gOverlay5Span1Size);
    gOverlay5Span1ScaleValue = gOverlay5Span1Size / 3U;

    gOverlay5Span2 = mmAlloc(resource->span0Start, 0x82);
    func_8002E2E0(0x32, gOverlay5Span2, 0, resource->span0Start);
    alBnkfNew(gOverlay5Span2,
              func_8002E35C(0x32, (void *)resource->span0Start));

    gOverlay5Bank = alHeapDBAlloc(0, 0, gOverlay5HeapState, 1, 4);
    func_8002E2E0(0x32, gOverlay5Bank,
                  (void *)resource->span0End, 4);

    bankSize = (u32)gOverlay5Bank->count * sizeof(Overlay5BankEntry) + 4;
    gOverlay5Bank = mmAlloc(bankSize, 0x82);
    func_8002E2E0(0x32, gOverlay5Bank,
                  (void *)resource->span0End, bankSize);
    alSeqFileNew(gOverlay5Bank,
                 func_8002E35C(0x32, (void *)resource->span0End));

    gOverlay5EntryValues = mmAlloc(
        (u32)gOverlay5Bank->count * sizeof(*gOverlay5EntryValues), 0x82);
    {
        Overlay5Bank *bank = gOverlay5Bank;
        u32 *destination;
        u32 destinationOffset;
        u32 sourceOffset;

        index = 0;
        if (bank->count > 0) {
            destinationOffset = 0;
            destination = gOverlay5EntryValues;
            sourceOffset = 0;
            do {
                u32 value;

                *destination = *(u32 *)((u8 *)bank + sourceOffset + 8);
                destination = (u32 *)((u8 *)gOverlay5EntryValues +
                                      destinationOffset);
                value = *destination;
                if ((value & 1) != 0) {
                    *destination = value + 1;
                    destination = (u32 *)((u8 *)gOverlay5EntryValues +
                                          destinationOffset);
                    value = *destination;
                }
                if (maxValue < value) {
                    maxValue = value;
                }
                bank = gOverlay5Bank;
                index++;
                destinationOffset += 4;
                destination++;
                sourceOffset += 8;
            } while (index < bank->count);
        }
    }

    soundConfig.maxVVoices = 0x2C;
    soundConfig.maxPVoices = 0x28;
    soundConfig.maxUpdates = 0x80;
    soundConfig.dmaproc = NULL;
    soundConfig.fxType[0] = 6;
    soundConfig.maxFXbusses = 1;
    soundConfig.outputRate = 0;
    soundConfig.heap = gOverlay5HeapState;
    func_80001740(&soundConfig, 0x0C, context);

    gOverlay5Player0 = overlay5CreatePlayer(0x20, 0x96);
    gOverlay5Player1 = overlay5CreatePlayer(0x10, 0x32);

    sequenceConfig.maxEvents = 0xC8;
    sequenceConfig.maxSounds = 0x20;
    sequenceConfig.maxChannels = 0x10;
    sequenceConfig.numGroups = 5;
    sequenceConfig.heap = gOverlay5HeapState;
    gsSndpNew(&sequenceConfig);

    func_80001BA0();
    amSetMuteMode(0);
    func_8002B768(resource);
    func_800039F0();
    alSurround_OutputType(4);
    alSurround_ReverbSetup(0, 3);
    osCreateMesgQueue(gOverlay5MessageQueue, gOverlay5MessageBuffer, 1);
    n_alCSPSetMessageQ(gOverlay5Player0, gOverlay5MessageQueue);
}

void *overlay5CreatePlayer(s32 voiceCount, s32 value) {
    void *player;
    Overlay5PlayerConfig config;

    config.arg0 = voiceCount;
    config.arg1 = value;
    config.voiceCount = voiceCount;
    config.channels = 0x10;
    config.heap = gOverlay5AudioHeap;
    config.initQueue = gOverlay5InitQueue;
    config.eventQueue = gOverlay5EventQueue;
    config.frameCallback = gOverlay5FrameCallback;
    player = overlay5AllocPlayerReloc(0, 0, gOverlay5AudioHeap, 1, 0x90);
    overlay5InitPlayerReloc(player, &config);
    overlay5AttachBankReloc(player, gOverlay5AudioState->sequenceBank);
    return player;
}
