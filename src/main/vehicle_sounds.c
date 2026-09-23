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
 * to identify the shared positional-sound API. No body is adapted from them;
 * all four functions remain Mickey's generated assembly.
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

void *D_800D78B0;
f32 sVehicleSoundPreviousDistance0;
f32 D_800D78B8;
void *D_800D78BC;
void *D_800D78C0;
f32 sVehicleSoundPreviousDistance1;
f32 D_800D78C8;
void *D_800D78CC;
void *D_800D78D0;
f32 sVehicleSoundPreviousDistance2;
f32 D_800D78D8;
void *D_800D78DC;
void *D_800D78E0;
f32 sVehicleSoundPreviousDistance3;
f32 D_800D78E8;
void *D_800D78EC;
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
extern f32 D_800842F0;
extern f32 D_800842F4;
extern f32 D_800842F8;
extern f32 D_800842FC;
extern f32 D_80084300;
extern f32 D_80084304;
extern f32 D_80084308;
extern f32 D_8008430C;
extern f32 D_80084310;
extern f32 D_80084314;
extern f32 D_80084318;

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

#ifdef NON_MATCHING
/* Bounded plateau: owns ROM 0x58E50..0x58EA8, 22 frameless words with no
 * padding. Configured full-TU C is 26 words, matches 3/22 positionally, first
 * +0x0, and carries 24 relocations against the target's 20. The four-word
 * excess is one redundant high-half load per slot: the target uses two direct
 * absolute bases for three stores while retaining separately named LO16s.
 * The previous 119-row lattice, aggregate/volatile-slot families, and this
 * pass's target ordering, TU-local-scalar, comma-expression, racer-base pointer,
 * and volatile split-tail forms are all nonexact; none strictly improves V0.
 * The sole caller is func_80004FE0+0x54C and no credible donor exists. Resume
 * only with an original declaration/TU model that naturally emits that mixed
 * relocation shape, or evidence that this initializer was handwritten; do
 * not repeat flags or these storage forms. Assembly remains canonical. */
void func_80058250(void) {
    D_800D78B0 = 0;
    D_800D78B8 = 0.0f;
    D_800D78BC = 0;
    D_800D78C0 = 0;
    D_800D78C8 = 0.0f;
    D_800D78CC = 0;
    D_800D78D0 = 0;
    D_800D78D8 = 0.0f;
    D_800D78DC = 0;
    D_800D78E0 = 0;
    D_800D78E8 = 0.0f;
    D_800D78EC = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_80058250.s")
#endif

void func_800582A8(void) {
    VehicleSoundSlot *slot = (VehicleSoundSlot *)&D_800D78B0, *end = (VehicleSoundSlot *)&D_800D78F0;
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

#ifdef NON_MATCHING
/*
 * PROVENANCE: source-level organization and terminology are adapted from
 * Diddy Kong Racing's permitted published src/audio_vehicle.c functions
 * racer_sound_update_all and racer_sound_doppler_effect. Mickey's own field
 * offsets, tables, control flow, constants and positional-audio calls decide
 * this body.
 *
 * Fresh p10 workbench: 758/762 candidate/target instructions and frame 0x118.
 * The retained unsigned vehicle ID and pointer setup order reduce opcode
 * mismatches without changing the positional score; DKR organization remains
 * provenance-only.
 *
 * Frame closed (2026-09-10).  `unused0` above is a frame-census slot, not a
 * value: the declared-local list was one 4-byte slot short, and adding one
 * takes the frame from 0x110 to the target's 0x118 exactly (the local block
 * quantizes in 8-byte steps).  Identical rows go 273 -> 278 of 762 and the
 * register-masked alignment 484 -> 490.  Its identity is unknown; whatever
 * real local it stands for, the census says there is one.
 *
 * With the frames equal the stack census separates cleanly, and that is the
 * useful part.  Every one of the twenty slots from 0x10 to 0x84 -- the
 * saved-register block and all of the spill temporaries -- now matches the
 * target exactly, by offset and by use count.  The entire residual lives in
 * the declared-local block above 0xA0, and it is an ORDER problem, not a size
 * one: the local with eight uses sits at frame_top-4 here and at frame_top-16
 * in the target, so the target declares roughly three more 4-byte locals
 * ahead of it.  The next lever is to permute the declaration list against the
 * target's use-count fingerprint, which is (8,4,3,2,2,8,6,2) reading down from
 * its first homed local against (8,6,2,1,1,1,4) here.
 *
 * The loop idiom question is settled, against the obvious guess.  The
 * candidate emitted thirteen `sltu reg,zero,counter` boolean materializations
 * that the target does not have -- the target has none at all -- and the
 * temptation is to reach for a different loop form.  Do not: the target's own
 * back-edge tests the counter and decrements it in the branch delay slot,
 * which is exactly `while (x-- != 0)`, the form already committed here.
 * Rewriting the three loops as `--x != -1` does remove all thirteen `sltu`,
 * but replaces them with thirteen equally spurious `addiu x,x,-1` duplicated
 * into the same branch-likely slots, and costs a register-masked identical
 * row; `--x >= 0` is worse again at -6.  All sixty-four combinations of four
 * loop forms across the three loops were measured and none is exact.  What
 * fills those slots in the target is float stores: the target has twenty
 * `swc1` against twelve here, and eight more homed float values is also what
 * would give ugen something real to sink into each likely slot.  Chase the
 * float homes, not the loop condition.
 *
 * The flag lattice was re-run over seven configurations; the shipped
 * `-Wab,-r4300_mul -O2 -mips2` is the best on the alignment and -mips1 costs
 * about a hundred instructions.
 */
void func_8005830C(s32 updateRate) {
    s32 racerCount;
    s32 racerIndex;
    s32 cameraCount;
    s32 cameraIndex;
    s32 scanIndex;
    s16 bestPriority;
    s32 secondarySoundId;
    s32 volume;
    f32 speed;
    f32 minimumSpeed;
    f32 maximumSpeed;
    f32 volumeScale;
    f32 range;
    f32 ratio;
    f32 nearestDistance;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 relativeVelocity;
    f32 cents;
    f32 engineIntensity;
    f32 basePitch;
    f32 storedMinimumSpeed;
    f32 storedMaximumSpeed;
    f32 storedVolumeScale;
    f32 storedEngineIntensity;
    f32 unused0;
    VehicleObject **racers;
    VehicleObject **racerPtr;
    VehicleObject *object;
    VehicleObject *candidate;
    VehicleRacerState *racer;
    VehicleSoundProfile *profile;
    VehicleSoundSlot *slot;
    VehicleCamera *cameras;

    nearestDistance = D_800842F0;
    racers = func_80005750(&racerCount);
    engineIntensity = storedEngineIntensity;
    racerIndex = racerCount - 1;
    if (racerCount != 0) {
        racerPtr = racers + racerIndex;
        volumeScale = storedVolumeScale;
        minimumSpeed = storedMinimumSpeed;
        maximumSpeed = storedMaximumSpeed;
        do {
            object = *racerPtr;
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
                    basePitch = D_8007F988[racer->characterId];
                    engineIntensity *= D_8007F938[racer->characterId];
                    secondarySoundId = D_8007F910[racer->characterId];
                    volumeScale = D_8007F9D8[racer->characterId];
                } else {
                    basePitch = D_8007F9B0[racer->characterId];
                    engineIntensity *= D_8007F960[racer->characterId];
                    secondarySoundId = D_8007F924[racer->characterId];
                    volumeScale = D_8007FA00[racer->characterId];
                }
                engineIntensity += (f32)mathRnd(-10, 10) * D_800842F4;
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

                if ((racer->flags & 0x20) && (racer->flags & 1) &&
                    (racer->flags & 0x10)) {
                    if (racer->engineSound != 0) {
                        func_800031E8(racer->engineSound);
                    }
                }
                if ((!(racer->flags & 1) || func_8003A550() != 0) &&
                    racer->raceFinished == 0) {
                    if (racer->engineSound == 0) {
                        func_80002FE0(secondarySoundId, object->x, object->y,
                                       object->z, 1, &racer->engineSound);
                    }
                    func_800031C0(racer->engineSound, object->x, object->y,
                                   object->z);
                    func_800030B4(
                        racer->engineSound,
                        (s32)((racer->engineIntensity * 6.0f) + basePitch) &
                            0xFF);
                    engineIntensity = racer->engineIntensity;
                    if (engineIntensity < 0.0f) {
                        engineIntensity = -engineIntensity;
                    }
                    if (engineIntensity > 21.0f) {
                        engineIntensity = 21.0f;
                    }
                    volume = 85.0f - (engineIntensity * volumeScale);
                    if (volume >= 61) {
                        volume = 60;
                    }
                    func_8000309C(racer->engineSound, volume & 0xFF);
                }

                if (((racer->flags & 0x20) || func_8003A550() != 0) &&
                    racer->raceFinished == 0) {
                    speed = racer->speed;
                    secondarySoundId = 0;
                    bestPriority = 0;
                    if (speed < 0.0f) {
                        speed = -speed;
                    }
                    if (racer->vehicleId != 0 && speed > 3.0f) {
                        minimumSpeed = 3.0f;
                        maximumSpeed = 18.0f;
                        volumeScale = 0.5f;
                        secondarySoundId = 0x23;
                    } else {
                        profile = &D_8007F810[racer->soundProfile[0] & 0xF];
                        if (profile->priority > 0 &&
                            profile->minimumSpeed < speed) {
                            bestPriority = profile->priority;
                            secondarySoundId = profile->soundId;
                            minimumSpeed = profile->minimumSpeed;
                            maximumSpeed = profile->maximumSpeed;
                            volumeScale = profile->volumeScale;
                        }
                        profile = &D_8007F810[racer->soundProfile[1] & 0xF];
                        if (bestPriority < profile->priority &&
                            profile->minimumSpeed < speed) {
                            bestPriority = profile->priority;
                            secondarySoundId = profile->soundId;
                            minimumSpeed = profile->minimumSpeed;
                            maximumSpeed = profile->maximumSpeed;
                            volumeScale = profile->volumeScale;
                        }
                        profile = &D_8007F810[racer->soundProfile[2] & 0xF];
                        if (bestPriority < profile->priority &&
                            profile->minimumSpeed < speed) {
                            bestPriority = profile->priority;
                            secondarySoundId = profile->soundId;
                            minimumSpeed = profile->minimumSpeed;
                            maximumSpeed = profile->maximumSpeed;
                            volumeScale = profile->volumeScale;
                        }
                        profile = &D_8007F810[racer->soundProfile[3] & 0xF];
                        if (bestPriority < profile->priority &&
                            profile->minimumSpeed < speed) {
                            secondarySoundId = profile->soundId;
                            minimumSpeed = profile->minimumSpeed;
                            maximumSpeed = profile->maximumSpeed;
                            volumeScale = profile->volumeScale;
                        }
                    }
                    if (racer->secondarySound != 0 &&
                        secondarySoundId != racer->secondarySoundId) {
                        func_800031E8(racer->secondarySound);
                        racer->secondarySoundId = 0;
                    }
                    if (secondarySoundId != 0) {
                        racer->secondarySoundId = secondarySoundId;
                        range = maximumSpeed - minimumSpeed;
                        if (maximumSpeed < speed) {
                            speed = maximumSpeed;
                        }
                        if (racer->secondarySound == 0) {
                            func_80002FE0(secondarySoundId, object->x, object->y,
                                           object->z, 1,
                                           &racer->secondarySound);
                        }
                        func_800031C0(racer->secondarySound, object->x,
                                       object->y, object->z);
                        ratio = (speed - minimumSpeed) / range;
                        func_800030B4(
                            racer->secondarySound,
                            (s32)(((ratio * 0.5f) + 0.5f) * 100.0f) & 0xFF);
                        func_8000309C(
                            racer->secondarySound,
                            ((s32)(ratio * 100.0f * volumeScale) + 20) &
                                0xFF);
                    }
                }
            }
            racerPtr--;
        } while (racerIndex-- != 0);
        storedVolumeScale = volumeScale;
        storedMinimumSpeed = minimumSpeed;
        storedMaximumSpeed = maximumSpeed;
        storedEngineIntensity = engineIntensity;
    }

    if (func_8003A550() == 0) {
        cameraCount = mainGetNumberOfCameras();
        cameras = camGetListPtr();
        cameraIndex = cameraCount - 1;
        if (cameraCount != 0) {
            do {
                slot = &((VehicleSoundSlot *)&D_800D78B0)[cameraIndex];
                object = slot->racerObject;
                candidate = 0;
                if (object != 0 && object == slot->handle &&
                    object->racer->raceFinished != 0) {
                    func_800031E8(slot->handle);
                }

                if (D_8007BF0C != 0) {
                    scanIndex = 0;
                    if (racerCount > 0) {
                        do {
                            object = racers[scanIndex];
                            racer = object->racer;
                            if ((racer->flags & 1) && (racer->flags & 0x20) &&
                                racer->raceFinished == 0 &&
                                cameraIndex == racer->playerIndex) {
                                deltaX = object->x - cameras[cameraIndex].x;
                                deltaY = object->y - cameras[cameraIndex].y;
                                deltaZ = object->z - cameras[cameraIndex].z;
                                nearestDistance = sqrtf(
                                    (deltaX * deltaX) + (deltaY * deltaY) +
                                    (deltaZ * deltaZ));
                                candidate = object;
                                scanIndex = racerCount;
                            }
                            scanIndex++;
                        } while (scanIndex < racerCount);
                    }
                } else {
                    scanIndex = racerCount - 1;
                    if (racerCount != 0) {
                        racerPtr = racers + scanIndex;
                        do {
                            object = *racerPtr;
                            racer = object->racer;
                            if ((racer->flags & 1) &&
                                racer->raceFinished == 0) {
                                deltaX = object->x - cameras[cameraIndex].x;
                                deltaY = object->y - cameras[cameraIndex].y;
                                deltaZ = object->z - cameras[cameraIndex].z;
                                range = sqrtf((deltaX * deltaX) +
                                              (deltaY * deltaY) +
                                              (deltaZ * deltaZ));
                                if (range < nearestDistance &&
                                    range < D_800842F8) {
                                    nearestDistance = range;
                                    candidate = object;
                                }
                            }
                            racerPtr--;
                        } while (scanIndex-- != 0);
                    }
                }

                if (candidate != 0) {
                    if (candidate == slot->racerObject) {
                        racer = candidate->racer;
                        relativeVelocity =
                            (nearestDistance - slot->previousDistance) /
                            (f32)updateRate;
                        if (relativeVelocity > 15.0f) {
                            relativeVelocity = 15.0f;
                        } else if (relativeVelocity < -15.0f) {
                            relativeVelocity = -15.0f;
                        }
                        if (D_8007BF04 == 0) {
                            basePitch = D_8007F988[racer->characterId];
                            secondarySoundId =
                                D_8007F910[racer->characterId];
                            volumeScale = D_8007F9D8[racer->characterId];
                        } else {
                            basePitch = D_8007F9B0[racer->characterId];
                            secondarySoundId =
                                D_8007F924[racer->characterId];
                            volumeScale = D_8007FA00[racer->characterId];
                        }
                        cents = func_80058EF4(racer->engineIntensity) *
                                D_800842FC;
                        if (D_80084300 < relativeVelocity &&
                            relativeVelocity <= 7.0f) {
                            relativeVelocity = D_80084304;
                        } else if (relativeVelocity >= 7.0f &&
                                   relativeVelocity < D_80084308) {
                            relativeVelocity = D_8008430C;
                        }
                        ratio = alCents2Ratio(
                            (s32)(((7.0f + relativeVelocity) /
                                   (7.0f - relativeVelocity)) *
                                  cents));
                        slot->dopplerPitch +=
                            (ratio - slot->dopplerPitch) * 0.5f;
                        if (D_80084310 < slot->dopplerPitch) {
                            slot->dopplerPitch = D_80084314;
                        } else if (slot->dopplerPitch < 0.0f) {
                            slot->dopplerPitch = 0.0f;
                        }
                        if (slot->handle == 0) {
                            func_80002FE0(secondarySoundId, candidate->x,
                                           candidate->y, candidate->z, 1,
                                           &slot->handle);
                        }
                        func_800031C0(slot->handle, candidate->x,
                                       candidate->y, candidate->z);
                        ratio = (slot->dopplerPitch * 100.0f) + basePitch +
                                (racer->engineIntensity * 6.0f);
                        if (ratio > 200.0f) {
                            ratio = 200.0f;
                        }
                        func_800030B4(slot->handle, (s32)ratio & 0xFF);
                        engineIntensity = storedEngineIntensity;
                        if (engineIntensity < 0.0f) {
                            engineIntensity = -engineIntensity;
                        }
                        if (engineIntensity > 21.0f) {
                            engineIntensity = 21.0f;
                        }
                        volume = 85.0f - (engineIntensity * volumeScale);
                        if (volume >= 61) {
                            volume = 60;
                        }
                        func_8000309C(slot->handle, volume & 0xFF);
                    } else if (slot->handle != 0) {
                        func_800031E8(slot->handle);
                    }
                    slot->racerObject = candidate;
                    slot->previousDistance = nearestDistance;
                } else if (slot->handle != 0) {
                    func_800031E8(slot->handle);
                }
            } while (cameraIndex-- != 0);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/vehicle_sounds/func_8005830C.s")
#endif

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
    if (D_80084318 < (result - previous)) {
        do {
            previous = result;
            result += term / divisor;
            divisor += 2;
            term *= squared;
        } while (D_80084318 < (result - previous));
    }
    return result * (s32)2;
}

/* PLATEAU-HANDOFF:func_8005830C:start
 * symbol: func_8005830C
 * score: 697 differing words
 * frame: 0x118
 * relocations: 88
 * first-mismatch: +0x8
 * summary: 758 vs 762 instructions; frame now exact at 0x118 and the whole saved-register and spill-temp block matches by offset and use count, so the residual is the declared-local ORDER plus eight unhomed float values
 * PLATEAU-HANDOFF:func_8005830C:end
 */

/* PLATEAU-HANDOFF:func_80058250:start
 * symbol: func_80058250
 * score: 19/22 words
 * frame: frameless
 * relocations: 24
 * first-mismatch: +0x0
 * summary: O2 address-commoning is the +16 (addiu vs $at). O1 same-symbol tails are 22 words but f4 not f0. Separate globals never share $at.
 * PLATEAU-HANDOFF:func_80058250:end
 */
