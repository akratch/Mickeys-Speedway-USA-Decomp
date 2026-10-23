#include "overlays/overlay_005.h"

/*
 * Overlay 5: audio start-up. One translation unit at the overlay flag set.
 * Only overlay5InitializeAudio is exported. The libaudio bank patcher
 * (alSeqFileNew .. _bnkfPatchWaveTable) and overlay5CreatePlayer are static;
 * overlay5InitSequence has no caller at all and stays global, because IDO
 * deletes an unreferenced static.
 */

static void _bnkfPatchBank(ALBank *bank, s32 offset, s32 table);
static void _bnkfPatchInst(ALInstrument *inst, s32 offset, s32 table);
static void _bnkfPatchSound(ALSound *sound, s32 offset, s32 table);
static void _bnkfPatchWaveTable(ALWaveTable *wave, s32 offset, s32 table);
static void *overlay5CreatePlayer(s32 voiceCount, s32 value);

/* PROVENANCE: adapted from DKR us.v77, libultra/src/audio/bnkf.c::alSeqFileNew. */
static void alSeqFileNew(ALSeqFile *file, u8 *base) {
    s32 offset = (s32)base;
    s32 i;

    for (i = 0; i < file->seqCount; i++) {
        file->seqArray[i].offset =
            (u8 *)((u8 *)file->seqArray[i].offset + offset);
    }
}

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::alBnkfNew.
 */
static void alBnkfNew(ALBankFile *file, u8 *table) {
    s32 offset = (s32)file;
    s32 tableOffset = (s32)table;
    s32 i;

    if (file->revision != AL_BANK_VERSION) {
        return;
    }

    for (i = 0; i < file->bankCount; i++) {
        file->bankArray[i] = (ALBank *)((u8 *)file->bankArray[i] + offset);
        if (file->bankArray[i] != 0) {
            _bnkfPatchBank(file->bankArray[i], offset, tableOffset);
        }
    }
}

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchBank.
 */
static void _bnkfPatchBank(ALBank *bank, s32 offset, s32 table) {
    s32 i;

    if (bank->flags != 0) {
        return;
    }
    bank->flags = 1;

    if (bank->percussion != 0) {
        bank->percussion = (ALInstrument *)((u8 *)bank->percussion + offset);
        _bnkfPatchInst(bank->percussion, offset, table);
    }

    for (i = 0; i < bank->instCount; i++) {
        bank->instArray[i] = (ALInstrument *)((u8 *)bank->instArray[i] + offset);
        if (bank->instArray[i] != 0) {
            _bnkfPatchInst(bank->instArray[i], offset, table);
        }
    }
}

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchInst.
 */
static void _bnkfPatchInst(ALInstrument *inst, s32 offset, s32 table) {
    s32 i;

    if (inst->flags != 0) {
        return;
    }
    inst->flags = 1;

    for (i = 0; i < inst->soundCount; i++) {
        inst->soundArray[i] = (ALSound *)((u8 *)inst->soundArray[i] + offset);
        _bnkfPatchSound(inst->soundArray[i], offset, table);
    }
}

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchSound.
 */
static void _bnkfPatchSound(ALSound *sound, s32 offset, s32 table) {
    if (sound->flags != 0) {
        return;
    }
    sound->flags = 1;

    sound->envelope = (ALEnvelope *)((u8 *)sound->envelope + offset);
    sound->keyMap = (ALKeyMap *)((u8 *)sound->keyMap + offset);
    sound->wavetable = (ALWaveTable *)((u8 *)sound->wavetable + offset);
    _bnkfPatchWaveTable(sound->wavetable, offset, table);
}

/*
 * PROVENANCE: adapted from DKR us.v77,
 * libultra/src/audio/bnkf.c::_bnkfPatchWaveTable.
 */
static void _bnkfPatchWaveTable(ALWaveTable *wave, s32 offset, s32 table) {
    if (wave->flags != 0) {
        return;
    }
    wave->flags = 1;
    wave->base += table;

    if (wave->type == AL_ADPCM_WAVE) {
        wave->waveInfo.adpcmWave.book =
            (ALADPCMBook *)((u8 *)wave->waveInfo.adpcmWave.book + offset);
        if (wave->waveInfo.adpcmWave.loop != 0) {
            wave->waveInfo.adpcmWave.loop =
                (ALADPCMloop *)((u8 *)wave->waveInfo.adpcmWave.loop + offset);
        }
    } else if (wave->type == AL_RAW16_WAVE) {
        if (wave->waveInfo.rawWave.loop != 0) {
            wave->waveInfo.rawWave.loop =
                (ALRawLoop *)((u8 *)wave->waveInfo.rawWave.loop + offset);
        }
    }
}

void overlay5InitSequence(void *owner, s32 value) {
    Overlay5SequenceHeader header;

    header.count = 14;
    header.value = value;
    n_alEvtqPostEvent((u8 *)owner + 0x48, &header, 0);
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
    D_800BF900 = gOverlay5OwnerSoundState;
    alHeapInit(gOverlay5HeapState, gOverlay5HeapMemory, 0x30D40);

    resource = func_8002E148(0x31);
    D_800BF7B0 = resource->end - resource->span1End;
    D_800BF7A0 = mmAlloc(D_800BF7B0, 0x82);
    func_8002E2E0(0x32, D_800BF7A0,
                  (void *)resource->span1End, D_800BF7B0);
    D_800BF7A8 = D_800BF7B0 / 10U;

    gOverlay5Span1Size = resource->span1End - resource->span1Start;
    D_800BF7A4 = mmAlloc(gOverlay5Span1Size, 0x82);
    func_8002E2E0(0x32, D_800BF7A4,
                  (void *)resource->span1Start, gOverlay5Span1Size);
    gOverlay5Span1ScaleValue = gOverlay5Span1Size / 3U;

    gOverlay5Span2 = mmAlloc(resource->span0Start, 0x82);
    func_8002E2E0(0x32, gOverlay5Span2, 0, resource->span0Start);
    alBnkfNew(gOverlay5Span2,
              func_8002E35C(0x32, (void *)resource->span0Start));

    D_800BF790 = alHeapDBAlloc(0, 0, gOverlay5HeapState, 1, 4);
    func_8002E2E0(0x32, D_800BF790,
                  (void *)resource->span0End, 4);

    bankSize = (u32)D_800BF790->count * sizeof(Overlay5BankEntry) + 4;
    D_800BF790 = mmAlloc(bankSize, 0x82);
    func_8002E2E0(0x32, D_800BF790,
                  (void *)resource->span0End, bankSize);
    alSeqFileNew(D_800BF790,
                 func_8002E35C(0x32, (void *)resource->span0End));

    D_800BF798 = mmAlloc(
        (u32)D_800BF790->count * sizeof(*D_800BF798), 0x82);
    {
        Overlay5Bank *bank = D_800BF790;
        u32 *destination;
        u32 destinationOffset;
        u32 sourceOffset;

        index = 0;
        if (bank->count > 0) {
            destinationOffset = 0;
            destination = D_800BF798;
            sourceOffset = 0;
            do {
                u32 value;

                *destination = *(u32 *)((u8 *)bank + sourceOffset + 8);
                destination = (u32 *)((u8 *)D_800BF798 +
                                      destinationOffset);
                value = *destination;
                if ((value & 1) != 0) {
                    *destination = value + 1;
                    destination = (u32 *)((u8 *)D_800BF798 +
                                          destinationOffset);
                    value = *destination;
                }
                if (maxValue < value) {
                    maxValue = value;
                }
                bank = D_800BF790;
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

    D_80078D60 = overlay5CreatePlayer(0x20, 0x96);
    D_80078D64 = overlay5CreatePlayer(0x10, 0x32);

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
    osCreateMesgQueue(D_800BFA10, gOverlay5MessageBuffer, 1);
    n_alCSPSetMessageQ(D_80078D60, D_800BFA10);
}

static void *overlay5CreatePlayer(s32 voiceCount, s32 value) {
    void *player;
    Overlay5PlayerConfig config;

    config.arg0 = voiceCount;
    config.arg1 = value;
    config.voiceCount = voiceCount;
    config.channels = 0x10;
    config.heap = gOverlay5AudioHeap;
    config.initQueue = amInitOsc;
    config.eventQueue = amStopOsc;
    config.frameCallback = amUpdateOsc;
    player = overlay5AllocPlayerReloc(0, 0, gOverlay5AudioHeap, 1, 0x90);
    n_alCSPNew(player, &config);
    overlay5AttachBankReloc(player, gOverlay5AudioState->sequenceBank);
    return player;
}
