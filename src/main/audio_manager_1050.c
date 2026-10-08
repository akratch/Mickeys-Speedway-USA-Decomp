/*
 * Resident audio front end -- ROM 0x1050-0x2340 (VRAM 0x80000450).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_1050.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * The end is the 16-byte-aligned start of Mickey's audiomgr-shaped run:
 * func_80001740 has the allocator, message-queue, and audio-thread setup shape
 * of JFG's first audiomgr routine. The two preceding Mickey-only routines stay
 * with this TU; no unaligned boundary is asserted from JFG's layout.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"

typedef struct AudioSequencePlayer {
    u8 pad0[0x2C];
    s32 state;
    u16 channelMask;
} AudioSequencePlayer;

typedef struct AudioSoundData {
    u16 soundBite;
    u8 volume;
    u8 minVolume;
    u8 pitch;
    u8 unk5;
    u16 range;
    u8 priority;
    u8 unk9;
} AudioSoundData;

typedef struct AudioEnvelope {
    s32 attackTime;
    s32 decayTime;
} AudioEnvelope;

typedef struct AudioSound {
    AudioEnvelope *envelope;
} AudioSound;

typedef struct AudioInstrument {
    u8 pad0[0xE];
    s16 soundCount;
    AudioSound *soundArray[1];
} AudioInstrument;

typedef struct AudioBank {
    u8 pad0[0xC];
    AudioInstrument *instArray[1];
} AudioBank;

typedef struct AudioBankFile {
    s32 revision;
    AudioBank *bankArray[1];
} AudioBankFile;

typedef struct AudioSequenceRomEntry {
    u16 unk0;
    s16 sequenceCount;
    u8 *romAddress;
} AudioSequenceRomEntry;

typedef struct AudioMusicData {
    u8 volume;
    u8 tempo;
    u8 reverb;
} AudioMusicData;

typedef struct AudioDelayedSound {
    u16 soundId;
    s16 timer;
    void **handle;
} AudioDelayedSound;

extern s32 D_80078D7C;
extern s32 D_80078D80;
extern s32 D_80078D78;
extern s32 D_80078D8C;
extern s32 D_80078D90;
extern void *D_80078D60;
extern void *D_80078D64;
extern u8 D_80078D68;
extern u8 D_80078D6C;
extern u8 D_80078D70;
extern u8 D_80078D74;
extern u8 D_80078D88;
extern u8 D_80078D84;
extern u8 D_80078D94;
extern u8 D_80078D98;
extern s8 D_80078DA0;
extern u8 D_80078DA8;
extern u8 D_80078DB0;
extern s8 D_80078DB4;
extern u8 D_80078DAC;
extern s32 D_80078DA4;
extern AudioSequenceRomEntry *D_800BF790;
extern u8 D_800BF794;
extern u8 D_800BF795;
extern u32 *D_800BF798;
extern AudioBankFile *D_800BF79C;
extern AudioSoundData *D_800BF7A0;
extern s32 D_800BF7A8;
extern s32 D_800BF7B0;
extern s32 D_800BF7B8;
extern s32 D_800BF7BC;
extern s32 D_800BF7C0;
extern s32 D_800BF7C4;
extern AudioDelayedSound D_800BF7C8[];
extern s32 D_800BFA00;
extern s32 D_800BFA04;
extern u8 D_800BFA08;
extern OSMesgQueue D_800BFA10;
extern void *D_800BF900;
extern u8 D_800BF908;
extern u8 D_80085A40[];
extern u8 D_8008DA40[];
extern AudioMusicData *D_800BF7A4;
extern s32 osTvType;
extern void gsSndpSetParam();
extern void gsSndpSetMasterVolume(u8 group, u16 volume);
extern u32 gsSndpGetGlobalVolume(void);
extern void gsSndpStop(void *sound);
extern void n_alCSPSetChlVol(void *player, u8 channel, u8 volume);
extern void n_alCSPSetVol(void *player, s16 volume);
extern void n_alCSPNew(void *player, void *config);
extern void n_alCSPSetMessageQ(void *player, void *queue);
extern void n_alCSPStop(void *player);
extern void n_alCSPVoiceLimit(void *player, u8 value);
extern s32 n_alCSPGetState(void *player);
extern void n_alCSeqNew(void *sequence, u8 *data);
extern void n_alCSPSetSeq(void *player, void *sequence);
extern void n_alCSPPlay(void *player);
extern void mainPreNMI(void);
extern u8 *piRomGetSectionPtr(u32 assetIndex, u32 assetOffset);
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
extern s32 amDittyPlaying(void);
extern void func_80001308(u8 value, void *player);
extern void stop_ALSeqp(void *player);
extern u16 amGetSfxCount(void);
void amTuneStop(void);
void amTuneSetVolume(u8 volume);
void amAmbientSetVolume(u8 volume);
void amTuneSetReverbOnOff(s32 enabled);
void amTuneMuteChl(s32 channel);
void amTuneUnmuteChl(s32 channel);
void amSndPlay(u16 soundId, void **handle);
void func_8000137C(void *player, u8 *sequenceData, u8 *sequenceId,
                   void *sequence);
extern void *ad_sndp_play(void *bank, s16 soundBite, u16 volume, u8 pan,
                          f32 pitch, u8 arg5, void **handle);

/* PROVENANCE: body adapted from JFG src/audio_manager_1050.c amSetMuteMode. */
void func_80000450(s32 behavior) {
    switch (behavior) {
        case 1:
            gsSndpSetMasterVolume(0, 0);
            gsSndpSetMasterVolume(1, 0x7FFF);
            n_alCSPSetVol(D_80078D64, 0);
            break;
        case 2:
            gsSndpSetMasterVolume(0, 0);
            break;
        default:
            gsSndpSetMasterVolume(0, 0x7FFF);
            gsSndpSetMasterVolume(1, 0x7FFF);
            n_alCSPSetVol(D_80078D64, (s16)(gsSndpGetGlobalVolume() * D_80078D6C));
            break;
    }
    D_80078DA0 = behavior;
}

/* PROVENANCE: body adapted from JFG src/audio_manager_1050.c amTunePlay. */
void func_80000510(u8 sequenceId) {
    if (D_80078D78 == 0 && D_800BF798[sequenceId] <= 0x8000) {
        if (D_80078D70 != 0) {
            amTuneStop();
            func_80001308(sequenceId, D_80078D60);
        }
        D_800BFA00 = -1;
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_1050.c amTuneVoiceLimit. */
void amTuneVoiceLimit(u8 voiceLimit) {
    if (D_80078DA8 == 0) {
        n_alCSPVoiceLimit(D_80078D60, voiceLimit);
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_1050.c amTuneSetFade. */
void func_800005CC(f32 fade, u8 volume) {
    if (volume > 0x7F) {
        volume = 0x7F;
    }
    D_800BF7B8 = volume;
    if (osTvType == 0) {
        D_80078D7C = fade * 50.0f;
    } else {
        D_80078D7C = fade * 60.0f;
    }
    if (D_80078D7C > 0) {
        D_800BF7BC = ((D_80078D68 - volume) << 16) / D_80078D7C;
    } else {
        amTuneSetVolume(volume);
    }
}

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c.
 */
void amTuneSetFadeScaled(f32 fade, u8 volume) {
    u32 scaled;

    if (volume > 0x7F) {
        volume = 0x7F;
    }
    scaled =
        (s32)((u32)D_800BF7A4[D_800BF794].volume * volume) / 0x7F;
    func_800005CC(fade, scaled);
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneResetFade(void) {
    D_80078D7C = 0;
}
/* PROVENANCE: body adapted from JFG src/audio_manager_1050.c amAmbientSetFade. */
void amAmbientSetFade(f32 fade, u8 volume) {
    if (volume > 0x7F) {
        volume = 0x7F;
    }
    D_800BF7C0 = volume;
    if (osTvType == 0) {
        D_80078D80 = fade * 50.0f;
    } else {
        D_80078D80 = fade * 60.0f;
    }
    if (D_80078D80 > 0) {
        D_800BF7C4 = ((D_80078D68 - volume) << 16) / D_80078D80;
    } else {
        amTuneSetVolume(volume);
    }
}

/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amAmbientResetFade(void) {
    D_80078D80 = 0;
}
/*
 * PROVENANCE: body adapted from JFG src/audio_manager_1050.c amAudioTick;
 * Mickey's resident sequence-init calls and master-volume fade tail remain
 * authoritative.
 */
void amAudioTick(u8 updateRate) {
    s32 i;
    s32 j;
    s32 volume;
    OSMesg message;
    s32 fadeStep;

    if (osRecvMesg(&D_800BFA10, &message, OS_MESG_NOBLOCK) == 0) {
        D_800BFA04 = 1;
    }

    if (D_80078D7C > 0 || D_80078DA4 != -1) {
        D_80078D7C -= updateRate;
        if (D_80078D7C < 0) {
            D_80078D7C = 0;
        }
        volume = ((D_800BF7BC * D_80078D7C) >> 16) + D_800BF7B8;
        amTuneSetVolume(volume);
    }
    if (D_80078D80 > 0) {
        D_80078D80 -= updateRate;
        if (D_80078D80 < 0) {
            D_80078D80 = 0;
        }
        volume = ((D_800BF7C4 * D_80078D80) >> 16) + D_800BF7C0;
        amAmbientSetVolume(volume);
    }

    if (D_80078D90 > 0) {
        for (i = 0; i < D_80078D90;) {
            D_800BF7C8[i].timer -= updateRate;
            if (D_800BF7C8[i].timer <= 0) {
                j = i;
                amSndPlay(D_800BF7C8[i].soundId, D_800BF7C8[i].handle);
                D_80078D90--;
                if (i < D_80078D90) {
                    D_800BF7C8[i].soundId = D_800BF7C8[i + 1].soundId;
                    D_800BF7C8[i].timer = D_800BF7C8[i + 1].timer;
                    D_800BF7C8[i].handle = D_800BF7C8[i + 1].handle;
                    do {
                        j++;
                    } while (j < D_80078D90);
                }
            } else {
                i++;
            }
        }
    }

    func_8000137C(D_80078D60, D_80085A40, &D_80078D94, D_800BF900);
    func_8000137C(D_80078D64, D_8008DA40, &D_80078D98, &D_800BF908);
    D_80078DB0 = 0;
    D_80078DB4 = 0;

    if (D_80078DAC != 0) {
        fadeStep = D_80078DAC * updateRate;
        if (fadeStep < D_800BFA08) {
            D_800BFA08 -= fadeStep;
        } else {
            D_800BFA08 = 0;
        }
        gsSndpSetMasterVolume(0, D_800BFA08 << 7);
        gsSndpSetMasterVolume(1, D_800BFA08 << 7);
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c
 * amWaitForMidiSync; body uses Mickey-only evidence.
 */
void amWaitForMidiSync(void) {
    OSMesg message;

    if (D_800BFA04 != 0) {
        D_800BFA04 = 0;
    } else {
        while (osRecvMesg(&D_800BFA10, &message, OS_MESG_NOBLOCK) != 0) {
            mainPreNMI();
        }
    }
}

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c
 * amResetMidiSync; body uses Mickey-only evidence.
 */
void amResetMidiSync(void) {
    D_800BFA04 = 0;
}

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c
 * amTuneSetChlMask; body uses Mickey-only evidence.
 */
void func_80000B48(u16 channelMask) {
    s32 channel;

    if (D_80078D94 != 0) {
        D_800BFA00 = channelMask;
        return;
    }
    ((AudioSequencePlayer *)D_80078D60)->channelMask = channelMask;
    for (channel = 0; channel < 16; channel++) {
        if (channelMask & (1 << channel)) {
            amTuneUnmuteChl(channel & 0xFF);
        } else {
            amTuneMuteChl(channel & 0xFF);
        }
    }
}

/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneMuteChl(s32 channel) {
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneUnmuteChl(s32 channel) {
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetChlVolume(u8 channel, u8 volume) {
    if (channel < 16) {
        n_alCSPSetChlVol(D_80078D60, channel, volume);
    }
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneResetChls(void) {
    s32 channel;
    u8 maskedChannel;

    if (D_80078D78 == 0) {
        channel = 0;
        do {
            maskedChannel = channel;
            amTuneUnmuteChl(maskedChannel & 0xFF);
            amTuneSetChlVolume(maskedChannel, 0x7F);
            channel++;
        } while (channel != 16);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c
 * music_jingle_play_safe; JFG src/audio_manager_1050.c supplies the official
 * amAmbientPlay name. Mickey's globals and call target remain authoritative.
 */
void amAmbientPlay(u8 value) {
    if (amDittyPlaying() == 0) {
        func_80001308(D_800BF795 = value, D_80078D64);
        D_80078D88 = 1;
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_stop; JFG
 * src/audio_manager_1050.c supplies the official amTuneStop name.
 */
void amTuneStop(void) {
    if (D_80078D78 == 0) {
        stop_ALSeqp(D_80078D60);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_stop; JFG
 * src/audio_manager_1050.c supplies the official amAmbientStop name.
 */
void amAmbientStop(void) {
    if (amDittyPlaying() == 0) {
        D_800BF795 = 0;
        stop_ALSeqp(D_80078D64);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_current_sequence;
 * JFG src/audio_manager_1050.c supplies the official amTuneGetSeqNo name.
 */
u8 amTuneGetSeqNo(void) {
    if (D_800BF794 != 0 &&
        ((AudioSequencePlayer *)D_80078D60)->state == 1) {
        return D_800BF794;
    }
    return 0;
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_current;
 * JFG src/audio_manager_1050.c supplies the official amAmbientGetSeqNo name.
 */
u8 amAmbientGetSeqNo(void) {
    return D_800BF795;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetVolume(u8 volume) {
    s32 scaledVolume;

    if (volume > 0x7F) {
        volume = 0x7F;
    }
    D_80078D68 = volume;
    scaledVolume = D_80078D8C * D_80078D68;
    n_alCSPSetVol(D_80078D60, scaledVolume);
    D_80078DB0 = 1;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetGlobalVolume(u32 volume) {
    s32 scaledVolume;

    if (volume > 0x100) {
        volume = 0x100;
    }
    D_80078D8C = volume;
    scaledVolume = D_80078D8C * D_80078D68;
    n_alCSPSetVol(D_80078D60, scaledVolume);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_volume; JFG
 * src/audio_manager_1050.c supplies the official amTuneGetVolume name.
 */
u8 amTuneGetVolume(void) {
    return D_80078D68;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amAmbientSetVolume(u8 volume) {
    D_80078D6C = volume;
    n_alCSPSetVol(D_80078D64,
                  (s16)(gsSndpGetGlobalVolume() * D_80078D6C));
}
/*
 * PROVENANCE: body shape and official name adapted from JFG
 * asm/nonmatchings/audio_manager_1050/amDittyPlay.s; Mickey's operands and
 * boundaries remain authoritative.
 */
void amDittyPlay(u8 sequenceId) {
    if (D_800BF798[sequenceId] <= 0x1000) {
        D_80078D74 = 1;
        func_80001308(D_800BF795 = sequenceId, D_80078D64);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_playing;
 * JFG src/audio_manager_1050.c supplies the official amDittyPlaying name.
 */
s32 amDittyPlaying(void) {
    if (D_800BF795 != 0 && D_80078D74 != 0 &&
        ((AudioSequencePlayer *)D_80078D64)->state == 1) {
        return D_800BF795;
    }
    D_80078D74 = 0;
    return 0;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amSndStop(void *sound) {
    gsSndpStop(sound);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c sound_play; JFG's
 * src/audio.h and asm/nonmatchings/audio_manager_1050/amSndPlay.s supply the
 * official name, SoundData layout, exact boundary, and direct-player call.
 */
void amSndPlay(u16 soundId, void **handle) {
    f32 pitch;
    s32 soundBite;

    if (soundId > D_800BF7A8) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    soundBite = D_800BF7A0[soundId].soundBite;
    if (soundBite == 0) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    pitch = (f32)D_800BF7A0[soundId].pitch / 100.0f;
    ad_sndp_play(D_800BF79C->bankArray[0], soundBite,
                 scalevol(D_800BF7A0[soundId].volume << 8), 0x40, pitch,
                 0, handle);
}
/*
 * PROVENANCE: official name, parameter role, and body shape adapted from JFG
 * asm/nonmatchings/audio_manager_1050/amSndPlayDirect.s; Mickey's boundary,
 * branch form, globals, and calls remain authoritative.
 */
void amSndPlayDirect(u16 soundBite, u8 volume, u8 pan, f32 pitch, u8 effects,
                     void **handle) {
    if (soundBite <= 0 || amGetSfxCount() < soundBite) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    ad_sndp_play(D_800BF79C->bankArray[0], soundBite,
                 scalevol(volume << 8), pan, pitch, effects, handle);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c
 * sound_volume_set_relative; JFG supplies the official amSndSetVol name.
 */
void amSndSetVol(u16 soundId, void *sound, u8 volume) {
    s32 scaledVolume;

    scaledVolume =
        (s32)(D_800BF7A0[soundId].volume * (volume / 127.0f)) * 256;
    if (sound != NULL) {
        gsSndpSetParam(sound, 8, scalevol(scaledVolume));
    }
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amSndSetPan(void *sound, u32 pan) {
    if (sound != NULL) {
        gsSndpSetParam(sound, 4, pan);
    }
}
/*
 * PROVENANCE: body and official name adapted from DKR src/audio.c
 * sound_pitch_set and JFG src/audio_manager_1050.c.
 */
void amSndSetPitchDirect(void *sound, u32 pitch) {
    u32 *pitchAddress = &pitch;

    if (sound != NULL) {
        gsSndpSetParam(sound, 16, *pitchAddress);
    }
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
u16 amGetSfxCount(void) {
    return D_800BF79C->bankArray[0]->instArray[0]->soundCount;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amGetSfxSettings(AudioSoundData **table, s32 *size, s32 *count) {
    if (table != NULL) {
        *table = D_800BF7A0;
    }
    if (size != NULL) {
        *size = D_800BF7B0;
    }
    if (count != NULL) {
        *count = D_800BF7A8;
    }
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
u8 amSoundIsLooped(u16 soundId) {
    if (soundId <= 0 ||
        D_800BF79C->bankArray[0]->instArray[0]->soundCount < soundId) {
        return 0;
    }
    return ((u32)(1 + D_800BF79C->bankArray[0]->instArray[0]
                          ->soundArray[soundId - 1]
                          ->envelope->decayTime) == 0);
}
/*
 * PROVENANCE: role and order compared with JFG src/audio_manager_1050.c;
 * body and resident sequence-header layout use Mickey-only evidence.
 */
void func_80001308(u8 sequenceId, void *player) {
    stop_ALSeqp(player);
    if (sequenceId < D_800BF790->sequenceCount) {
        if (player == D_80078D60) {
            D_80078D94 = sequenceId;
        } else {
            D_80078D98 = sequenceId;
        }
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c
 * music_sequence_init; body uses Mickey-only evidence.
 */
void func_8000137C(void *player, u8 *sequenceData, u8 *sequenceId, void *sequence) {
    s32 channel;

    if (n_alCSPGetState(player) == 0 && *sequenceId != 0) {
        piRomLoadSection(0x32, (u32)sequenceData,
                         D_800BF790[*sequenceId].romAddress - piRomGetSectionPtr(0x32, 0),
                         D_800BF798[*sequenceId]);
        n_alCSeqNew(sequence, sequenceData);
        n_alCSPSetSeq(player, sequence);
        n_alCSPPlay(player);
        if (player == D_80078D60) {
            D_800BF794 = *sequenceId;
            D_80078D84 = 1;
            if (D_80078DB0 != 0) {
                amTuneSetVolume(D_80078D68);
            } else {
                amTuneSetVolume(D_800BF7A4[*sequenceId].volume);
            }
            amTuneSetReverbOnOff(D_800BF7A4[*sequenceId].reverb);
            if (D_800BFA00 != -1) {
                for (channel = 0; channel < 16; channel++) {
                    if ((1 << channel) & D_800BFA00) {
                        amTuneUnmuteChl(channel & 0xFF);
                    } else {
                        amTuneMuteChl(channel & 0xFF);
                    }
                }
            }
        } else {
            amAmbientSetVolume(D_800BF7A4[*sequenceId].volume);
            D_800BF795 = *sequenceId;
        }
        *sequenceId = 0;
    }
    D_80078DB0 = 0;
    D_80078DB4 = 0;
}

/* PROVENANCE: body and name adapted from JFG stop_ALSeqp assembly. */
void stop_ALSeqp(void *player) {
    if (player == D_80078D60 && D_80078D84 != 0) {
        n_alCSPStop(player);
        D_80078D84 = 0;
        D_80078D94 = 0;
        return;
    }
    if (player == D_80078D64 && D_80078D88 != 0) {
        n_alCSPStop(player);
        D_80078D88 = 0;
        D_80078D98 = 0;
    }
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetReverbOnOff(s32 enabled) {
}
void func_800015F8(void) {
    D_80078D78 = 1;
}
void func_80001608(void) {
    D_80078D78 = 0;
}
s32 func_80001614(void) {
    return D_80078D78;
}
u8 func_80001620(u16 soundId) {
    if (D_800BF7A8 < soundId) {
        return 0;
    }
    return D_800BF7A0[soundId].volume;
}
void func_80001668(void *sound, u8 volume) {
    if (sound != NULL) {
        gsSndpSetParam(sound, 8, volume);
    }
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void forcelink(void) {
    n_alCSPNew(NULL, NULL);
    n_alCSPSetMessageQ(NULL, NULL);
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
int scalevol(volume)
int volume;
{
    return volume * 0.5f;
}
void func_800016EC(u8 mode) {
    D_80078DAC = mode;
    D_800BFA08 = 0xFF;
}
void func_80001708(void) {
    D_80078DAC = 0;
    gsSndpSetMasterVolume(0, 0x7FFF);
    gsSndpSetMasterVolume(1, 0x7FFF);
}
