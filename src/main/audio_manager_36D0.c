/*
 * Resident positional-audio manager -- ROM 0x3100-0x45F0
 * (VRAM 0x80002500).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_36D0.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * func_80002500 has JFG amInitAudioMap's initialization role and starts at a
 * 16-byte boundary. The following twenty functions preserve JFG's TU order;
 * the last ends exactly where the byte-identified amVibratoInit begins.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"
#include "game/math.h"

typedef struct AudioPoint {
    f32 x;
    f32 y;
    f32 z;
    u16 soundId;
    u8 volume;
    u8 pitch;
    u8 minVolume;
    u8 flags;
    u8 inRange;
    u8 pad13;
    s32 range;
    void *soundHandle;
    struct AudioPoint **handle;
    u8 fastFalloff;
    u8 priority;
    u8 triggeredOnce;
    u8 unk23;
} AudioPoint;

typedef struct AudioCamera {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x3C];
} AudioCamera;

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

typedef struct AudioEchoSurface {
    f32 height;
    u8 pad04[4];
    u32 flags;
} AudioEchoSurface;

typedef struct AudioVector3 {
    f32 x;
    f32 y;
    f32 z;
} AudioVector3;

typedef struct AudioUpdateEntry {
    AudioPoint *point;
    f32 pitch;
    u8 volume;
    u8 effects;
    u8 pad0A[2];
    s32 pan;
} AudioUpdateEntry;

extern AudioSoundData *D_800C91E0;
extern AudioPoint **D_800C91E4;
extern AudioPoint *D_800C91E8;
extern AudioPoint **D_800C91F0;
extern u8 D_800C91EC;
extern s8 D_800C91F4;
extern MtxF D_800C91F8;
extern u16 D_80078F00;
extern s32 D_80078F14;
extern s32 D_80078F04[];
extern f32 D_80080B58;
extern AudioUpdateEntry D_800C9238[][3];
extern AudioUpdateEntry D_800C9248;
extern AudioUpdateEntry D_800C9258;
extern AudioUpdateEntry D_800C9268;
extern AudioUpdateEntry D_800C9278;
extern AudioUpdateEntry D_800C9288;
extern AudioUpdateEntry D_800C9298;
extern AudioUpdateEntry D_800C92A8;
extern AudioUpdateEntry D_800C92B8;
extern AudioUpdateEntry D_800C92C8;
extern AudioUpdateEntry D_800C92D8;
extern AudioUpdateEntry D_800C92E8;
void amAmbientStop(void);
void amSndPlayDirect(u16 soundId, u8 volume, u8 pan, f32 pitch, u8 effects,
                     void **handle);
void amSndStop(void *sound);
void camGetPlayerProjMtx(s32 player, MtxF matrix);
AudioCamera *camGetListPtr(void);
void gsSndpSetParam(void *sound, s32 parameter, u32 value);
void gsSndpSetPriority(void *sound, u8 priority);
s32 runlinkIsModuleLoaded(s32 moduleId);
s32 scalevol(s32 volume);
f32 sqrtf(f32 value);
void TrapDanglingJump();
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z, f32 *outX, f32 *outY, f32 *outZ);
s32 Arctanf(f32 x, f32 z);
s32 mainGetNumberOfCameras(void);
void amGetSfxSettings(AudioSoundData **table, s32 *size, s32 *count);
void *mmAlloc(s32 size, s32 tag);
void amResetAudioMap(void);
s32 func_8001398C(f32 x, f32 z, s32 range,
                  AudioEchoSurface ***surfaces);
s32 amCalcSfxStereo(f32 x, f32 y, f32 z);
void amCreateAudioPoint(u16 soundId, f32 x, f32 y, f32 z, u8 flags, u8 minVolume, u8 volume, u16 distance, u8 fastFalloff,
                   u8 pitch, u8 priority, s32 soundMode, AudioPoint **point);
void func_800037C4(s32 index);
u8 amSndSetEcho(void *sound, f32 x, f32 y, f32 z);
void func_80003480(AudioPoint *point, s32 volume, f32 pitch, s32 pan,
                   s32 effects);
void func_800035F8(s32 index);
void func_80003760(AudioPoint *point);

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * amInitAudioMap; body and pool layout use Mickey-only evidence.
 */
void func_80002500(void) {
    s32 i;

    amGetSfxSettings(&D_800C91E0, NULL, NULL);
    D_800C91E8 = mmAlloc(sizeof(AudioPoint) * 40, 0x82);
    D_800C91F0 = mmAlloc(sizeof(AudioPoint *) * 40, 0x82);
    D_800C91E4 = mmAlloc(sizeof(AudioPoint *) * 40, 0x82);
    D_80078F00 = 0;
    for (i = 0; i < 40; i++) {
        D_800C91E8[i].soundHandle = NULL;
    }
    amResetAudioMap();
}
/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amAmbientPause. */
void audspat_jingle_off(void) {
    amAmbientStop();
    D_800C91F4 = 1;
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amAmbientRestart. */
void amAmbientRestart(void) {
    D_800C91F4 = 0;
}

/*
 * PROVENANCE: role, order, and source shape compared with JFG
 * src/audio_manager_36D0.c amResetAudioMap; Mickey's pool layout, module ID,
 * and body remain authoritative.
 */
void amResetAudioMap(void) {
    AudioPoint *freePoint;
    AudioPoint *point;
    s32 i;

    freePoint = D_800C91E8;
    D_800C91EC = 0;
    while (D_800C91EC < 40) {
        D_800C91F0[D_800C91EC] = freePoint;
        freePoint++;
        D_800C91EC++;
    }
    D_800C91EC--;

    for (i = 0; i < D_80078F00; i++) {
        point = D_800C91E4[i];
        point->inRange = 0;
        if (point->soundHandle != NULL) {
            amSndStop(point->soundHandle);
        }
    }

    D_80078F00 = 0;
    D_800C91F4 = 0;
    D_800C9238[0][0].point = NULL;
    D_800C9248.point = NULL;
    D_800C9258.point = NULL;
    D_800C9268.point = NULL;
    D_800C9278.point = NULL;
    D_800C9288.point = NULL;
    D_800C9298.point = NULL;
    D_800C92A8.point = NULL;
    D_800C92B8.point = NULL;
    D_800C92C8.point = NULL;
    D_800C92D8.point = NULL;
    D_800C92E8.point = NULL;

    if (runlinkIsModuleLoaded(6) != 0) {
        TrapDanglingJump();
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio_spatial.c
 * audspat_update_all and compared with JFG src/audio_manager_36D0.c
 * amPlayAudioMap; Mickey's reduced point-only update remains authoritative.
 */
void amPlayAudioMap(void *players, s32 playerCount, s32 updateRate) {
    s32 cameraCount;
    s32 pointIndex;
    s32 cameraIndex;
    s32 volume;
    s32 pan;
    s32 specialPan;
    s32 effects;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 distance;
    s32 adjustedVolume;
    f32 pitch;
    f32 factor;
    f32 minimumDistance;
    s32 unused[2];
    AudioPoint *point;
    AudioCamera *cameras;

    cameraCount = mainGetNumberOfCameras();
    if (cameraCount == 1) {
        camGetPlayerProjMtx(0, D_800C91F8);
    }
    cameras = camGetListPtr();

    for (pointIndex = 0; pointIndex < D_80078F00; pointIndex++) {
        point = D_800C91E4[pointIndex];
        volume = 0;

        if (point->flags & 2) {
            if (cameraCount == 1) {
                dx = point->x - cameras[0].x;
                dy = point->y - cameras[0].y;
                dz = point->z - cameras[0].z;
                distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                if (distance < point->range && point->inRange == 0) {
                    pitch = point->pitch / 100.0f;
                    specialPan =
                        amCalcSfxStereo(point->x, point->y, point->z);
                    if ((u32)specialPan >> 24) {
                        specialPan &= 0xFF;
                        effects = 0x80;
                    } else {
                        effects = 0;
                    }
                    if (cameraCount != 1) {
                        specialPan = 64;
                    }
                    if (point->soundHandle == NULL &&
                        (point->triggeredOnce == 0 || !(point->flags & 4))) {
                        amSndPlayDirect(point->soundId, point->volume,
                                        specialPan, pitch, effects,
                                        &point->soundHandle);
                        point->triggeredOnce = 1;
                    } else if (point->soundHandle != NULL) {
                        gsSndpSetParam(point->soundHandle, 8,
                                      scalevol(point->volume << 8));
                        gsSndpSetParam(point->soundHandle, 0x10,
                                      *(u32 *)&pitch);
                        gsSndpSetParam(point->soundHandle, 4, specialPan);
                        effects |= amSndSetEcho(point->soundHandle, point->x,
                                                point->y, point->z);
                        gsSndpSetParam(point->soundHandle, 0x100, effects);
                    }
                    point->inRange = 1;
                } else if (point->range < distance && point->inRange != 0) {
                    point->inRange = 0;
                }
            }
        } else {
            for (cameraIndex = 0; cameraIndex < cameraCount; cameraIndex++) {
                dx = point->x - cameras[cameraIndex].x;
                dy = point->y - cameras[cameraIndex].y;
                dz = point->z - cameras[cameraIndex].z;
                distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                if (distance < point->range) {
                    if (point->fastFalloff == 0) {
                        adjustedVolume =
                            (1.0f - (f32)distance / (f32)point->range) *
                            point->volume;
                    } else {
                        factor = (f32)(point->range - distance) /
                                 (f32)point->range;
                        adjustedVolume = factor * factor * point->volume;
                    }
                    if (volume < adjustedVolume) {
                        volume = adjustedVolume;
                        pan = amCalcSfxStereo(point->x, point->y, point->z);
                    }
                }
            }

            if (volume < point->minVolume) {
                minimumDistance = D_80080B58;
                for (cameraIndex = 0; cameraIndex < cameraCount;
                     cameraIndex++) {
                    dx = point->x - cameras[cameraIndex].x;
                    dy = point->y - cameras[cameraIndex].y;
                    dz = point->z - cameras[cameraIndex].z;
                    distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                    if ((f32)distance < minimumDistance) {
                        pan = amCalcSfxStereo(point->x, point->y, point->z);
                        minimumDistance = distance;
                    }
                }
                volume = point->minVolume;
            }

            if (volume >= 11) {
                effects = 0;
                pitch = point->pitch / 100.0f;
                if (cameraCount != 1) {
                    pan = 64;
                }
                if ((u32)pan >> 24) {
                    pan &= 0xFF;
                    effects = 0x80;
                } else {
                    effects = 0;
                }
                if (point->unk23 == 0) {
                    if (point->soundHandle == NULL &&
                        (point->triggeredOnce == 0 || !(point->flags & 4))) {
                        amSndPlayDirect(point->soundId, volume, pan, pitch,
                                        effects, &point->soundHandle);
                        point->triggeredOnce = 1;
                    } else if (point->soundHandle != NULL) {
                        gsSndpSetParam(point->soundHandle, 8,
                                      scalevol(volume << 8));
                        gsSndpSetParam(point->soundHandle, 0x10,
                                      *(u32 *)&pitch);
                        gsSndpSetParam(point->soundHandle, 4, pan);
                        gsSndpSetPriority(point->soundHandle,
                                         point->priority);
                        effects |= amSndSetEcho(point->soundHandle, point->x,
                                                point->y, point->z);
                        gsSndpSetParam(point->soundHandle, 0x100, effects);
                    }
                } else {
                    func_80003480(point, volume, pitch, pan, effects);
                }
            } else if (point->soundHandle != NULL) {
                amSndStop(point->soundHandle);
                if (point->unk23 != 0) {
                    func_80003760(point);
                }
            } else {
                point->triggeredOnce = 1;
            }
        }
    }

    for (pointIndex = 0; pointIndex < 4; pointIndex++) {
        func_800035F8(pointIndex);
    }

    for (pointIndex = 0; pointIndex < D_80078F00; pointIndex++) {
        point = D_800C91E4[pointIndex];
        if (point->unk23 != 0) {
            point->triggeredOnce = 1;
        }
        if ((point->flags & 4) && point->triggeredOnce != 0 &&
            point->soundHandle == NULL) {
            func_800037C4(pointIndex);
        }
    }

    if (runlinkIsModuleLoaded(6)) {
        TrapDanglingJump(cameras, cameraCount);
    }
}
/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amCalcSfxStereo. */
s32 amCalcSfxStereo(f32 x, f32 y, f32 z) {
    s16 arctan;
    s32 result;
    f32 transformedX;
    f32 transformedY;
    f32 transformedZ;

    if (mainGetNumberOfCameras() >= 2) {
        return 64;
    }
    mtxf_transform_point(D_800C91F8, x, y, z, &transformedX, &transformedY, &transformedZ);
    arctan = Arctanf(transformedX, transformedZ);
    if (arctan >= -0x2000 && arctan <= 0x2000) {
        result = 64 + ((arctan * 0x3F) / 8192);
    } else if (arctan < -0x2000 && arctan > -0x6000) {
        result = (64 + ((s32)((-arctan * 0x3F) + 0xFFE86000) / 16384)) | 0xFF000000;
    } else if (arctan > 0x2000 && arctan < 0x6000) {
        result = (64 + (((-arctan * 0x3F) + 0x17A000) / 16384)) | 0xFF000000;
    } else {
        result = 64 | 0xFF000000;
    }
    return result;
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndPlayXYZ. */
void amSndPlayXYZ(u16 soundId, f32 x, f32 y, f32 z, u8 flags, AudioPoint **point) {
    if (D_800C91E0[soundId].soundBite != 0 && (point == NULL || *point == NULL)) {
        amCreateAudioPoint(D_800C91E0[soundId].soundBite, x, y, z, flags, D_800C91E0[soundId].minVolume,
                      D_800C91E0[soundId].volume, D_800C91E0[soundId].range, 0,
                      D_800C91E0[soundId].pitch, D_800C91E0[soundId].priority,
                      D_800C91E0[soundId].unk5, point);
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndSetVolXYZ. */
void amSndSetVolXYZ(AudioPoint *point, u8 volume) {
    if (point != NULL) {
        point->volume = volume;
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndSetPitchXYZ. */
void amSndSetPitchXYZ(AudioPoint *point, u8 pitch) {
    if (point != NULL) {
        point->pitch = pitch;
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndPlayDirectXYZ. */
void amSndPlayDirectXYZ(u16 soundId, f32 x, f32 y, f32 z, u8 flags, u8 volume, f32 pitch, u8 unused,
                   AudioPoint **point) {
    amCreateAudioPoint(soundId, x, y, z, flags, 100, volume, 15000, 0, pitch, 0x3F, 0, point);
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndSetXYZ. */
void amSndSetXYZ(AudioPoint *point, f32 x, f32 y, f32 z) {
    if (point != NULL) {
        point->x = x;
        point->y = y;
        point->z = z;
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndStopXYZ. */
void amSndStopXYZ(AudioPoint *point) {
    s32 index;

    if (point != NULL) {
        for (index = 0; index < D_80078F00; index++) {
            if (point == D_800C91E4[index]) {
                func_800037C4(index);
                break;
            }
        }
    }
}

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * amSndUnlinkHandleXYZ; body uses Mickey-only evidence.
 */
void amSndUnlinkHandleXYZ(AudioPoint *point) {
    s32 index;

    if (point != NULL) {
        for (index = 0; index < D_80078F00; index++) {
            if (point == D_800C91E4[index]) {
                D_800C91E4[index]->handle = NULL;
                return;
            }
        }
    }
}

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * amCreateAudioPoint; body and pool layout use Mickey-only evidence.
 */
void amCreateAudioPoint(u16 soundId, f32 x, f32 y, f32 z, u8 flags, u8 minVolume,
                   u8 volume, u16 distance, u8 fastFalloff, u8 pitch, u8 priority,
                   s32 soundMode, AudioPoint **pointHandle) {
    AudioPoint *point;

    if (D_80078F14 < D_80078F00) {
        D_80078F14 = D_80078F00;
    }
    if (D_80078F00 == 40) {
        if (pointHandle != NULL) {
            *pointHandle = NULL;
        }
        return;
    }
    point = D_800C91F0[D_800C91EC];
    D_800C91EC--;
    point->x = x;
    point->y = y;
    point->z = z;
    point->soundId = soundId;
    point->flags = flags;
    point->minVolume = minVolume;
    point->volume = volume;
    point->pitch = pitch;
    point->range = distance;
    point->fastFalloff = fastFalloff;
    point->priority = priority;
    point->triggeredOnce = 0;
    point->unk23 = soundMode;
    point->handle = pointHandle;
    D_800C91E4[D_80078F00] = point;
    D_80078F00++;
    if (pointHandle != NULL) {
        *pointHandle = point;
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * amSndSetEcho; body and surface layout use Mickey-only evidence.
 */
u8 amSndSetEcho(void *sound, f32 x, f32 y, f32 z) {
    AudioEchoSurface **surfaces;
    AudioEchoSurface *closest = NULL;
    AudioEchoSurface *surface;
    s32 count;
    s32 closestDistance = 0;
    s32 i;
    s32 distance;

    count = func_8001398C(x, z, 0x1800, &surfaces);
    i = count - 1;
    x = y + 10.0f;
    if (count != 0) {
        do {
            surface = surfaces[i];
            distance = (s32)(x - surface->height);
            if ((distance < closestDistance || closest == NULL) &&
                distance > 0) {
                closestDistance = distance;
                closest = surface;
            }
        } while (i--);
    }
    if (closest != NULL && (closest->flags & 0x20000000)) {
        return 0x3C;
    }
    return 0;
}
/* Four locals only: every declared local reserves a frame home, and the
 * target's 0x30 frame has room for exactly these. The group is read from the
 * point at each use instead of being held in a carrier or a base pointer. */
void func_80003480(AudioPoint *point, s32 volume, f32 pitch, s32 pan,
                   s32 effects) {
    s32 index;
    s32 bestVolume;
    s32 bestIndex;
    AudioUpdateEntry *entry;

    entry = D_800C9238[point->unk23 - 1];
    for (index = 0; index < ((s32 *) &D_80078F00)[point->unk23]; index++) {
        if (point == entry->point) {
            entry->volume = volume;
            entry->pitch = pitch;
            entry->pan = pan;
            entry->effects = effects;
            return;
        }
        entry++;
    }

    entry = D_800C9238[point->unk23 - 1];
    index = 0;
    bestVolume = entry->volume;
    for (; index < ((s32 *) &D_80078F00)[point->unk23]; index++) {
        if (entry->point == NULL) {
            entry->point = point;
            entry->volume = volume;
            entry->pitch = pitch;
            entry->pan = pan;
            entry->effects = effects;
            return;
        }
        if (bestVolume >= entry->volume) {
            bestVolume = entry->volume;
            bestIndex = index;
        }
        entry++;
    }

    if (bestVolume < volume) {
        entry = D_800C9238[point->unk23] + bestIndex - 3;
        if (entry->point->soundHandle != NULL) {
            amSndStop(entry->point->soundHandle);
        }
        entry->point = point;
        entry->volume = volume;
        entry->pitch = pitch;
        entry->pan = pan;
        entry->effects = effects;
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * func_80003994_4594; body and update-entry layout use Mickey-only evidence.
 */
void func_800035F8(s32 group) {
    AudioUpdateEntry *entry;
    AudioPoint *point;
    s32 *count;
    s32 countValue;
    s32 index;

    count = &D_80078F04[group];
    entry = &D_800C9238[group][0];
    index = 0;
    countValue = *count;
    if (countValue > 0) {
        do {
            if (entry->point != NULL) {
                point = entry->point;
                if (point->soundHandle == NULL &&
                    (!(point->flags & 4) || point->triggeredOnce == 0)) {
                    amSndPlayDirect(point->soundId, entry->volume,
                                    entry->pan, entry->pitch, entry->effects,
                                    &point->soundHandle);
                }
                if (point->soundHandle != NULL) {
                    gsSndpSetParam(point->soundHandle, 8,
                                   scalevol(entry->volume << 8));
                    gsSndpSetParam(point->soundHandle, 0x10,
                                   *(u32 *)&entry->pitch);
                    gsSndpSetParam(point->soundHandle, 4, entry->pan);
                    gsSndpSetPriority(point->soundHandle, point->priority);
                    gsSndpSetParam(
                        point->soundHandle, 0x100,
                        amSndSetEcho(point->soundHandle, point->x, point->y,
                                      point->z) |
                            entry->effects);
                }
                point->triggeredOnce = 1;
            }
            index++;
            entry++;
            countValue = *count;
        } while (index < countValue);
    }
}
/* Removes `point` from its group's update list. The group number is 1-based. */
void func_80003760(AudioPoint *point) {
    AudioUpdateEntry *entry;
    s32 index;

    entry = D_800C9238[point->unk23 - 1];
    for (index = 0; index < D_80078F04[point->unk23 - 1]; index++) {
        if (point == entry->point) {
            entry->point = NULL;
            return;
        }
        entry++;
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * func_80003B74_4774; body and pool layout use Mickey-only evidence.
 */
void func_800037C4(s32 index) {
    AudioPoint *point;

    if (D_80078F00 != 0) {
        point = D_800C91E4[index];
        if (point->soundHandle != NULL) {
            amSndStop(point->soundHandle);
        }
        if (point->handle != NULL) {
            *point->handle = NULL;
        }
        if (point->unk23 != 0) {
            func_80003760(point);
        }
        D_800C91EC++;
        D_800C91F0[D_800C91EC] = D_800C91E4[index];
        D_800C91E4[index] = D_800C91E4[D_80078F00 - 1];
        D_80078F00--;
        D_800C91E4[D_80078F00] = NULL;
    }
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_36D0.c
 * amSndGetXYZVolume; body and settings layout use Mickey-only evidence.
 */
s32 amSndGetXYZVolume(u16 soundId, AudioVector3 *position,
                  AudioVector3 *listener) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 range;
    s32 volume;
    s32 attenuated;

    dx = position->x - listener->x;
    dy = position->y - listener->y;
    dz = position->z - listener->z;
    distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
    volume = D_800C91E0[soundId].minVolume;
    range = D_800C91E0[soundId].range;
    if (distance < range) {
        attenuated = (s32)(D_800C91E0[soundId].volume *
                           (1.0f - (distance / range)));
        if (volume < attenuated) {
            volume = attenuated;
        }
    }
    return volume;
}
