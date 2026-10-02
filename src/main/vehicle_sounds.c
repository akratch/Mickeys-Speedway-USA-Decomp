/*
 * Racer/vehicle sound updater -- ROM 0x58E50-0x59B90
 * (VRAM 0x80058250-0x80058F90).
 *
 * The name is a Tier B/D description, not a borrowed JFG TU name. The large
 * updater walks active racer objects, owns two positional sound handles on
 * each one, and updates their position, volume and pitch from speed and
 * camera distance. No function in this boundary has an exact JFG skeleton
 * hit, so the existing splat boundary is intentionally not presented as a
 * measured cross-title file boundary.
 *
 * PROVENANCE: JFG's permitted src/audio_manager_36D0.c and audio.h were read
 * to identify the shared positional-sound API. No body is adapted from them.
 */

#include "PR/ultratypes.h"

typedef struct VehicleSoundSlot {
    void *handle;
    f32 previousDistance;
    f32 dopplerPitch;
    void *racerObject;
} VehicleSoundSlot;

typedef struct VehicleSoundProfile {
    s16 priority;
    s16 soundId;
    f32 minimumSpeed;
    f32 maximumSpeed;
    f32 volumeScale;
} VehicleSoundProfile;

typedef struct VehicleRacerState {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ s8 characterId;
    /* 0x002 */ u8 vehicleId;
    /* 0x003 */ u8 pad003;
    /* 0x004 */ f32 speed;
    /* 0x008 */ u8 pad008[0xA0 - 0x008];
    /* 0x0A0 */ s16 secondarySoundId;
    /* 0x0A2 */ u8 pad0A2[0xBC - 0xA2];
    /* 0x0BC */ void *engineSound;
    /* 0x0C0 */ void *secondarySound;
    /* 0x0C4 */ u8 pad0C4[0x1A8 - 0xC4];
    /* 0x1A8 */ u16 flags;
    /* 0x1AA */ u8 pad1AA[0x320 - 0x1AA];
    /* 0x320 */ u8 soundProfile[4];
    /* 0x324 */ u8 pad324[0x349 - 0x324];
    /* 0x349 */ u8 intensityOffsetDisabled;
    /* 0x34A */ u8 pad34A[0x3FA - 0x34A];
    /* 0x3FA */ s16 raceFinished;
    /* 0x3FC */ u8 pad3FC[0x418 - 0x3FC];
    /* 0x418 */ f32 engineIntensity;
} VehicleRacerState;

typedef struct VehicleObject {
    /* 0x00 */ u8 pad00[0x0C];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x64 - 0x18];
    /* 0x64 */ VehicleRacerState *racer;
} VehicleObject;

typedef struct VehicleCamera {
    /* 0x00 */ u8 pad00[0x0C];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x54 - 0x18];
} VehicleCamera;

VehicleSoundSlot D_800D78B0[4];
extern s32 D_800D78F0;
extern u8 D_8007BF04;
extern u8 D_8007BF0C;
extern VehicleSoundProfile D_8007F810[];
extern u16 D_8007F910[];
extern u16 D_8007F924[];
extern f32 D_8007F938[];
extern f32 D_8007F960[];
extern f32 D_8007F988[];
extern f32 D_8007F9B0[];
extern f32 D_8007F9D8[];
extern f32 D_8007FA00[];

f32 alCents2Ratio(s32 cents);
void func_80002FE0(u16 soundId, f32 x, f32 y, f32 z, u8 arg4,
                   void **soundHandle);
void func_8000309C(void *soundHandle, u8 volume);
void func_800030B4(void *soundHandle, u8 pitch);
void func_800031C0(void *soundHandle, f32 x, f32 y, f32 z);
void func_800031E8(void *soundHandle);
VehicleObject **func_80005750(s32 *count);
VehicleCamera *camGetListPtr(void);
s32 func_8003A550(void);
s32 mainGetNumberOfCameras(void);
s32 mathRnd(s32 minimum, s32 maximum);
f32 sqrtf(f32 value);
f32 func_80058EF4(f32 value);

/* The four camera sound slots are one array: a plain loop that IDO unrolls
 * gives the target's shared high half per pair of stores, which the sixteen
 * separately named scalars this TU used to define could not (2026-10-02). */
void func_80058250(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        D_800D78B0[i].handle = 0;
        D_800D78B0[i].dopplerPitch = 0.0f;
        D_800D78B0[i].racerObject = 0;
    }
}

void func_800582A8(void) {
    VehicleSoundSlot *slot = D_800D78B0, *end = (VehicleSoundSlot *)&D_800D78F0;
    do {
        if (slot->handle != 0) {
            func_800031E8(slot->handle);
        }
        if (slot->racerObject != 0) {
            slot->racerObject = 0;
        }
        slot++;
    } while (slot != end);
}

/*
 * PROVENANCE: source-level organization and terminology are adapted from
 * Diddy Kong Racing's permitted published src/audio_vehicle.c functions
 * racer_sound_update_all and racer_sound_doppler_effect. Mickey's own field
 * offsets, tables, control flow, constants and positional-audio calls decide
 * this body.
 *
 * 2026-10-02 (lane p-tex2), 697 at -16 to 6 at delta 0: rewritten from the
 * target listing. The D_800842F0..D_80084314 externs were this function's
 * own float literals; the loops are `while (i--)` over indexed arrays (uopt
 * makes the pointers); the profile scan is a 4-trip for loop IDO unrolls;
 * the slots are one array; min/max/volume scale and the engine intensity
 * are plain locals the target reads uninitialised from their homes;
 * relative velocity reuses `speed` (one home at 0xBC); ratio and range are
 * spill temps, not homes. The doppler smoothing divides by 2.0f: written
 * as `* 0.5f` it shares the pitch's "0.5f" constant, and that one ucode
 * constant spans the racer loop and loses f12 (409 to 8). The empty test on
 * cameras after the camera loop extends its range by one block, so the
 * 0x54 stride constant outranks it for s6 (save 200/19 against 201/20); the
 * three pads keep the 0x118 frame. Matched.
 */
void func_8005830C(s32 updateRate) {
    VehicleObject *object;
    VehicleObject *candidate;
    VehicleRacerState *racer;
    s32 racerCount;
    VehicleSoundProfile *profile;
    VehicleSoundSlot *slot;
    VehicleCamera *cameras;
    s32 i;
    s32 j;
    s32 k;
    VehicleObject **racers;
    s32 soundId;
    s32 bestPriority;
    s32 volume;
    f32 engineIntensity;
    f32 nearestDistance;
    f32 distance;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 minimumSpeed;
    f32 maximumSpeed;
    f32 speed;
    f32 pad_rv;
    f32 cents;
    f32 pitch;
    f32 basePitch;
    f32 ratio;
    f32 volumeScale;
    f32 secondaryVolumeScale;
    f32 range;
    s32 pad0;
    s32 pad1;
    s32 pad2;

    nearestDistance = 1000000000.0f;
    racers = func_80005750(&racerCount);
    i = racerCount;
    while (i--) {
        object = racers[i];
        racer = object->racer;
        if (racer->raceFinished != 0) {
            if (racer->engineSound != 0) {
                func_800031E8(racer->engineSound);
            }
            if (racer->secondarySound != 0) {
                func_800031E8(racer->secondarySound);
            }
        } else {
            engineIntensity = racer->speed;
            if (D_8007BF04 == 0) {
                engineIntensity *= D_8007F938[racer->characterId];
                basePitch = D_8007F988[racer->characterId];
                volumeScale = D_8007F9D8[racer->characterId];
                soundId = D_8007F910[racer->characterId];
            } else {
                engineIntensity *= D_8007F960[racer->characterId];
                basePitch = D_8007F9B0[racer->characterId];
                volumeScale = D_8007FA00[racer->characterId];
                soundId = D_8007F924[racer->characterId];
            }
            engineIntensity += mathRnd(-10, 10) * 0.1f;
            if (engineIntensity < 0.0f) {
                engineIntensity = -engineIntensity;
            }
            if (engineIntensity > 21.0f) {
                engineIntensity = 21.0f;
            }
            racer->engineIntensity = engineIntensity;
            if (racer->intensityOffsetDisabled == 0) {
                racer->engineIntensity += 3.0f;
            }
            if ((racer->flags & 0x20) && (racer->flags & 1) && (racer->flags & 0x10)) {
                if (racer->engineSound != 0) {
                    func_800031E8(racer->engineSound);
                }
            }
            if ((!(racer->flags & 1) || func_8003A550() != 0) && racer->raceFinished == 0) {
                if (racer->engineSound == 0) {
                    func_80002FE0(soundId, object->x, object->y, object->z, 1, &racer->engineSound);
                }
                func_800031C0(racer->engineSound, object->x, object->y, object->z);
                func_800030B4(racer->engineSound, (s32)(racer->engineIntensity * 6.0f + basePitch));
                if (engineIntensity < 0.0f) {
                    engineIntensity = -engineIntensity;
                }
                if (engineIntensity > 21.0f) {
                    engineIntensity = 21.0f;
                }
                volume = 85.0f - engineIntensity * volumeScale;
                if (volume > 60) {
                    volume = 60;
                }
                func_8000309C(racer->engineSound, volume);
            }
            if (((racer->flags & 0x20) || func_8003A550() != 0) && racer->raceFinished == 0) {
                speed = racer->speed;
                soundId = 0;
                bestPriority = 0;
                if (speed < 0.0f) {
                    speed = -speed;
                }
                if (racer->vehicleId != 0 && speed > 3.0f) {
                    minimumSpeed = 3.0f;
                    maximumSpeed = 18.0f;
                    secondaryVolumeScale = 0.5f;
                    soundId = 0x23;
                } else {
                    for (k = 0; k < 4; k++) {
                        profile = &D_8007F810[racer->soundProfile[k] & 0xF];
                        if (bestPriority < profile->priority && profile->minimumSpeed < speed) {
                            bestPriority = profile->priority;
                            soundId = profile->soundId;
                            minimumSpeed = profile->minimumSpeed;
                            maximumSpeed = profile->maximumSpeed;
                            secondaryVolumeScale = profile->volumeScale;
                        }
                    }
                }
                if (racer->secondarySound != 0 && soundId != racer->secondarySoundId) {
                    func_800031E8(racer->secondarySound);
                    racer->secondarySoundId = 0;
                }
                if (soundId != 0) {
                    racer->secondarySoundId = soundId;
                    range = maximumSpeed - minimumSpeed;
                    if (maximumSpeed < speed) {
                        speed = maximumSpeed;
                    }
                    if (racer->secondarySound == 0) {
                        func_80002FE0(soundId, object->x, object->y, object->z, 1, &racer->secondarySound);
                    }
                    func_800031C0(racer->secondarySound, object->x, object->y, object->z);
                    ratio = (speed - minimumSpeed) / range;
                    func_800030B4(racer->secondarySound, (s32)((ratio * 0.5f + 0.5f) * 100.0f));
                    func_8000309C(racer->secondarySound, (s32)(ratio * 100.0f * secondaryVolumeScale) + 20);
                }
            }
        }
    }

    if (func_8003A550() == 0) {
        i = mainGetNumberOfCameras();
        cameras = camGetListPtr();
        while (i--) {
            slot = &D_800D78B0[i];
            object = slot->racerObject;
            candidate = 0;
            if (object != 0 && object == slot->handle) {
                racer = object->racer;
                if (racer->raceFinished != 0) {
                    func_800031E8(slot->handle);
                }
            }
            if (D_8007BF0C != 0) {
                for (j = 0; j < racerCount; j++) {
                    object = racers[j];
                    racer = object->racer;
                    if ((racer->flags & 1) && (racer->flags & 0x20) && racer->raceFinished == 0 &&
                        i == racer->playerIndex) {
                        deltaX = object->x - cameras[i].x;
                        deltaY = object->y - cameras[i].y;
                        deltaZ = object->z - cameras[i].z;
                        nearestDistance = sqrtf(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
                        candidate = object;
                        j = racerCount;
                    }
                }
            } else {
                j = racerCount;
                while (j--) {
                    object = racers[j];
                    racer = object->racer;
                    if ((racer->flags & 1) && racer->raceFinished == 0) {
                        deltaX = object->x - cameras[i].x;
                        deltaY = object->y - cameras[i].y;
                        deltaZ = object->z - cameras[i].z;
                        distance = sqrtf(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
                        if (distance < nearestDistance && distance < 5000.0f) {
                            nearestDistance = distance;
                            candidate = object;
                        }
                    }
                }
            }
            if (candidate != 0) {
                if (candidate == slot->racerObject) {
                    racer = candidate->racer;
                    speed = (nearestDistance - slot->previousDistance) / updateRate;
                    if (speed > 15.0f) {
                        speed = 15.0f;
                    } else if (speed < -15.0f) {
                        speed = -15.0f;
                    }
                    if (D_8007BF04 == 0) {
                        basePitch = D_8007F988[racer->characterId];
                        volumeScale = D_8007F9D8[racer->characterId];
                        soundId = D_8007F910[racer->characterId];
                    } else {
                        basePitch = D_8007F9B0[racer->characterId];
                        volumeScale = D_8007FA00[racer->characterId];
                        soundId = D_8007F924[racer->characterId];
                    }
                    cents = func_80058EF4(racer->engineIntensity) * 1731.234f;
                    if (6.99f < speed && speed <= 7.0f) {
                        speed = 6.99f;
                    } else if (speed >= 7.0f && speed < 7.01f) {
                        speed = 7.01f;
                    }
                    ratio = alCents2Ratio((7.0f + speed) / (7.0f - speed) * cents);
                    slot->dopplerPitch += (ratio - slot->dopplerPitch) / 2.0f;
                    if (0.3f < slot->dopplerPitch) {
                        slot->dopplerPitch = 0.3f;
                    } else if (slot->dopplerPitch < 0.0f) {
                        slot->dopplerPitch = 0.0f;
                    }
                    if (slot->handle == 0) {
                        func_80002FE0(soundId, candidate->x, candidate->y, candidate->z, 1, &slot->handle);
                    }
                    func_800031C0(slot->handle, candidate->x, candidate->y, candidate->z);
                    pitch = slot->dopplerPitch * 100.0f + basePitch + racer->engineIntensity * 6.0f;
                    if (pitch > 200.0f) {
                        pitch = 200.0f;
                    }
                    func_800030B4(slot->handle, (s32)pitch);
                    if (engineIntensity < 0.0f) {
                        engineIntensity = -engineIntensity;
                    }
                    if (engineIntensity > 21.0f) {
                        engineIntensity = 21.0f;
                    }
                    volume = 85.0f - engineIntensity * volumeScale;
                    if (volume > 60) {
                        volume = 60;
                    }
                    func_8000309C(slot->handle, volume);
                } else if (slot->handle != 0) {
                    func_800031E8(slot->handle);
                }
                slot->racerObject = candidate;
                slot->previousDistance = nearestDistance;
            } else if (slot->handle != 0) {
                func_800031E8(slot->handle);
            }
        }
        if (cameras) {
        }
    }
}

/*
 * Exact under -O2 -mips2 -32 -Wab,-r4300_mul. Naming the loop-invariant
 * square preserves the target's FP lifetimes and direct multiplication in
 * the return expression preserves its return-register coalescing.
 */
f32 func_80058EF4(f32 arg0) {
    f32 one;
    f32 squared;
    f32 previous;
    f32 term;
    f32 result;
    s32 divisor;

    one = 1.0f;
    previous = -1.0f;
    result = 0.0f;
    divisor = 1;
    arg0 = (arg0 - one) / (one + arg0);
    term = arg0;
    squared = arg0 * arg0;
    if (0.001f < (result - previous)) {
        do {
            previous = result;
            result += term / divisor;
            divisor += 2;
            term *= squared;
        } while (0.001f < (result - previous));
    }
    return result * (s32)2;
}
