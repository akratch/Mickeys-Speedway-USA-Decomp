/*
 * Character and camera control -- ROM 0x1C790-0x20020
 * (VRAM 0x8001BB90-0x8001F420).
 *
 * The yaml boundaries were originally splat's aligned file-boundary
 * candidates. The content now supports the TU assignment independently: the
 * first routines follow JFG's camera-control cluster, exact skeleton anchors
 * identify func_8001C2D4 and controlSetPlayerSetup inside the block, the tail
 * is the same player-setup set/get/clear sequence, and the next yaml block
 * starts with a tier-A JFG models.c function. See docs/modules.md section 3.4.
 *
 * PROVENANCE -- Jet Force Gemini's public decomp src/charControl.c,
 * src/charControl.h, built charControl.c object, public symbol map, and
 * asm/nonmatchings/charControl filenames were consulted to identify the
 * translation unit and obtain comparison leads. Names not already supported
 * by tier-A evidence remain comments in docs/modules.md and are not adopted
 * here. Any future body adapted from JFG must carry its own PROVENANCE note
 * before that body and must be proved against Mickey's bytes.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul, measured on func_8001F09C.
 */

#include "PR/ultratypes.h"
#include "game/charControl.h"

extern u8 D_80079BF8;
extern u16 D_80079A0C[];
extern u16 D_80079A20[][4];
extern ControlGravityVector D_800799EC;
extern ControlGravityVector D_800799FC;
extern u16 D_8007BF1C;
extern f32 D_8008187C;
extern f32 D_80081880;
extern f32 D_80081884;
extern f32 D_80081888;
extern f32 D_8008188C;
extern f32 D_80081890;
extern f32 D_80081894;
extern f32 D_80081898;
extern s32 D_80079BCC;
extern s32 D_8007C1A0;
extern f32 D_80079BD4[];
extern CameraTrackedObject *D_800CB308[];
extern CameraOverrideSlot D_800CB368[];
extern CameraOverride D_800CB380[];
extern s16 D_800CB470;
extern s16 D_800CB472;
extern s16 D_800CB474;
extern s16 D_800CB476;
extern ControlCollisionState D_800CB2C0;
extern ControlCameraState *D_800CB300;
extern u8 D_8007BF10;
extern f32 D_80081840;
extern f32 D_80081844;
extern f32 D_80081848;
extern f32 D_8008184C;
extern f32 D_80081850;
extern f32 D_80081854;
extern f32 D_80081858;
extern f32 D_8008185C;
extern f32 D_80081860;
extern f32 D_800CB304;

typedef struct CharControlEffectDefinition {
    u8 kind;
    u8 index;
    s16 angle;
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    u8 arg14;
    u8 arg15;
    u8 arg16;
    u8 arg17;
} CharControlEffectDefinition;

typedef struct CharControlEffectList {
    s32 count;
    CharControlEffectDefinition *entries;
} CharControlEffectList;

typedef struct CharControlParticleDefinition {
    u8 kind;
    u8 index;
    s8 angle;
    s8 angleLow;
    s16 arg4;
    s16 arg6;
    s16 arg8;
    s16 argA;
    s16 argC;
    s16 argE;
} CharControlParticleDefinition;

typedef struct CharControlParticleList {
    s32 count;
    CharControlParticleDefinition *entries;
} CharControlParticleList;

typedef struct CharControlIndex {
    u16 offset;
    u16 value;
} CharControlIndex;

typedef struct CharControlCharacterData {
    u8 pad00[0x1C];
    s16 *positions;
    u8 pad20[0x2D - 0x20];
    u8 count;
    u8 pad2E[2];
    CharControlIndex *indexTable;
} CharControlCharacterData;

typedef struct CharControlParticleSlot {
    u8 kind;
    u8 active;
    u8 index;
    s8 model;
    u8 unk4;
    u8 pad05;
    s16 unk6;
    void *handle;
} CharControlParticleSlot;

typedef struct CharControlLevelDescription {
    u8 pad00[0x0A];
    u8 characterLow;
    u8 characterHigh;
    s8 nextLevel;
    u8 pad0D[0x16 - 0x0D];
    s8 animGroup;
    u8 pad17[0x1A - 0x17];
    s8 camera;
} CharControlLevelDescription;

typedef struct CharControlLevelRequest {
    u8 pad00[0x3C];
    CharControlLevelDescription *description;
} CharControlLevelRequest;

typedef struct CharControlSpawnSetup {
    s16 kind;
    s8 arg02;
    s8 arg03;
    s16 arg04;
    s16 arg06;
    s16 arg08;
    s8 arg0A;
    s8 arg0B;
    void *owner;
} CharControlSpawnSetup;

typedef struct CharControlGroundRecord {
    void *hitObject;
    u8 pad04[0x10 - 0x04];
    f32 unk10;
    f32 unk14;
    f32 unk18;
    u8 pad1C[0x38 - 0x1C];
    s32 unk38;
    u8 unk3C;
    u8 unk3D;
    u8 pad3E[0x40 - 0x3E];
} CharControlGroundRecord;

extern CharControlEffectList D_8007980C[];
extern CharControlParticleList D_8007987C[];
extern CharControlParticleList D_800798DC[];
extern CharControlParticleList D_8007992C[];
extern CharControlParticleList D_8007996C[];
extern CharControlParticleList D_800799AC[];
extern u8 D_8007BEF8;
extern u8 D_8007BEFC;
extern u8 D_8007BF04;
extern f32 D_80081864;
extern f32 D_80081868;
extern f32 D_8008186C;
extern f32 D_80081870;
extern f32 D_80081874;
extern f32 D_80081878;
extern f32 D_800CB2D8;

typedef struct ControlCollisionNormal {
    f32 x;
    f32 y;
    f32 z;
} ControlCollisionNormal;
extern f32 D_800CB2C4;
extern f32 D_800CB2C8;
extern f32 D_800CB2CC;
extern ControlCollisionNormal D_800CB2D0;
extern f32 D_800CB2D4;
extern f32 D_800CB2DC;
extern f32 D_800CB2E0;
extern f32 D_800CB2E4;
extern s32 D_800CB2F8;
extern u8 D_800CB2FC;
extern u8 D_800CB2FD;

void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
void func_8001EFFC(ControlTransform *transform, ControlPlayer *player, f32 *output);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
s16 Arctanf(f32 x, f32 y);
f32 sqrtf(f32 value);
void mathOneFloatRPY(ControlTransform *transform, f32 *output);
s32 mathRnd(s32 minimum, s32 maximum);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
void func_800031E8(void *handle);
void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority, void **handle);
void func_8001D690(ControlActor *actor, ControlPlayer *player);
void func_80006EA0(void *handle);
s32 func_8000FAE0(f32 x, f32 y, f32 z);
void func_8001C4C0(ControlActor *actor, ControlPlayerInitState *state, s32 mode);
s32 TrapDanglingJump();
#pragma weak charControlEffectSpawnTrap = TrapDanglingJump
extern void *charControlEffectSpawnTrap(
    ControlActor *actor, u8 kind, s16 angle, u8 index,
    f32 x, f32 y, f32 z, f32 w,
    u8 arg14, u8 arg15, u8 arg16, u8 arg17);
void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup,
                     s32 nextCamera, s32 fadeOut, s32 flags);
s32 mainGetNextCharacter(void);
void mainSetAnimGroup(s32 group);
u8 frontGetMode(void);
void func_800214AC(void);
void func_8001F09C(ControlPlayer *player, s32 updateRate);
void func_800031C0(void *soundHandle, f32 x, f32 y, f32 z);
void func_8001BBB4(ControlActor *actor, ControlPlayer *player, f32 arg2);
void func_8001C114(s32 slotIndex, f32 x, f32 y, f32 z);
void *func_80053420(s32 index, void *target);
void func_80024ED8();
s32 func_8003A550(void);
s32 func_8000FBD8(s32 segmentIndex, f32 x, f32 y, f32 z);
void func_800475E8(void *cone, s16 angle);
void func_800479D4(void *cone, s16 angle, f32 x, f32 y, s32 length);
void partUpdateTriggers(void *object, s32 updateRate);
void changeLightIntensity(void *light, u8 intensity);
s32 func_8002A204(s16 angle);
void camSetNo();
ControlCameraState *camGetPtr(void);
s32 camGetMode(void);
ControlCameraState *camGetListPtr(void);
ControlTrackState *trackGetTrack(void);
ControlLevelState *levelGetLevel(void);
s32 mainGetNumberOfCameras(void);
s32 func_800299E8(s32 minimum, s32 maximum);
ControlActor **func_8000572C(s32 *start, s32 *end);
s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode, ControlActor **hitActor);
void func_800282C8(void);
void func_8005AD64(void *instance, s32 frame, s32 arg2, f32 value);
void *func_80046EC4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                    f32 arg5, f32 arg6, f32 arg7, s32 arg8, s32 arg9,
                    s32 argA);
f32 Powerf(f32 value, s32 exponent);
void trackMakePolylist(s32 count, ControlVector3 *start,
                       ControlVector3 *end, f32 *radius, s32 arg4, s32 arg5);
s32 func_80010654(ControlVector3 *start, ControlVector3 *end,
                  ControlVector3 *result, f32 *maximum);
s32 func_80010B4C(s32 count, void *start, void *points, void *radius,
                  void *records, void *actorPosition, void *actor);
s32 func_80010900(ControlVector3 *start, ControlVector3 *end, f32 radius,
                  s32 actor, void *callback);
void func_8001EC44(s32 arg0, ControlVector3 *arg1, ControlVector3 *arg2,
                   f32 arg3, ControlCollisionPlane *arg4);
u8 levelGetType(void);
u8 *func_80028F54(void);
u32 joyGetButtons(s32 playerIndex);
u32 joyGetPressed(s32 playerIndex);
u32 joyGetReleased(s32 playerIndex);
u32 joyGetStickX(s32 playerIndex);
u32 joyGetAbsX(s32 playerIndex);
u32 joyGetStickY(s32 playerIndex);
u32 joyGetAbsY(s32 playerIndex);
void func_800291D8(s32 arg0);
s32 func_800291FC(void);
void rumbleStart(s32 playerIndex, s32 strength, f32 duration);

f32 func_8001BB90(s32 cameraIndex) {
    return D_800CB380[cameraIndex].blend;
}
/* Matched from a 38-word plateau in four edits, three of which are why the body reads oddly.
 * (1) Declaring surfaceValid ahead of level gives the declared block a fifth carrier and closes the
 *     0x30 frame onto the target's 0x38, retiring 16 stack-displacement words. Homes descend from the
 *     frame top in declaration order and which locals become carriers is emergent, so the fix is to
 *     reorder rather than to add.
 * (2) The surface scan runs EIGHT iterations, not seven: the target tests the counter and decrements
 *     in the back-edge delay slot.
 * (3) Reading the halfword directly rather than through an `angle` local spends the ring pop the
 *     target spends there (L88), retiring a uniform -1 rotation over nine downstream webs.
 * (4) The last two words were the dead loop-exit copy ordered against the pointer advance. Carrying
 *     the decrement in `mask` and advancing `surface` below it -- so the test reads +0x112 rather than
 *     +0x114 -- emits them in the target's order.
 */
/* PROVENANCE: JFG's corresponding character-control routine supplied the control-flow role;
 * all field offsets, calls, and the body below are reconstructed from Mickey. */
void func_8001BBB4(ControlActor *actor, ControlPlayer *player, f32 arg2) {
    ControlTrackState *track;
    s32 surfaceIndex;
    void *cameraSource;
    s32 surfaceValid;
    ControlLevelState *level;
    s32 i;
    s32 mask;
    s16 angle;
    u8 *surface;
    if (player->playerIndex < mainGetNumberOfCameras()) {
        func_8001C114(player->playerIndex, actor->x, actor->y, actor->z);
        cameraSource = func_80053420(0, D_800CB300);
        if (cameraSource == NULL) {
            if ((player->flags1A8 & 1) && (func_8003A550() == 0)) {
                TrapDanglingJump(actor, player, D_800CB300, (s32) arg2);
            } else if (player->controlKeys & 4) {
                func_80024ED8(actor, player, D_800CB300);
            } else if (D_8007BF10 != 0) {
                TrapDanglingJump(D_800CB300, actor, *(s32 *) &arg2);
            } else {
                TrapDanglingJump(D_800CB300, actor, *(s32 *) &arg2);
            }
        }
        track = trackGetTrack();
        if (track != NULL) {
            if (D_800CB300->y < ((f32) track->unk24 - 100.0f)) {
                D_800CB300->y += (((f32) track->unk24 - 100.0f) - D_800CB300->y) * D_80081840;
            }
        }
        level = levelGetLevel();
        surfaceIndex = func_8000FAE0(D_800CB300->x, D_800CB300->y, D_800CB300->z);
        if (surfaceIndex != -1) {
            surfaceValid = 1;
            i = 7;
            surface = (u8 *) level + 0xE;
            do {
                if (surfaceIndex == *(s16 *) (surface + 0x112)) {
                    surfaceValid = func_8000FBD8(surfaceIndex, D_800CB300->x,
                                                 D_800CB300->y, D_800CB300->z);
                    break;
                }
                mask = i--;
                surface -= 2;
            } while (mask);
            if (surfaceValid != 0) {
                D_800CB300->unk3E = (s16) surfaceIndex;
            }
        }
        if (*(void **)((u8 *) actor + 0x50) != NULL && level->unk0E3 == 0) {
            *(f32 *) ((u8 *) *(void **)((u8 *) actor + 0x50) + 0x10) = D_800CB300->x - actor->x;
            *(f32 *) ((u8 *) *(void **)((u8 *) actor + 0x50) + 0x14) = D_800CB300->y - actor->y;
            *(f32 *) ((u8 *) *(void **)((u8 *) actor + 0x50) + 0x18) = D_800CB300->z - actor->z;
        }
    }
}
/* PROVENANCE: JFG's corresponding character-control routine supplied the control-flow role; fields and body are reconstructed from Mickey. */
void func_8001BE0C(ControlActor *actor, ControlPlayer *player) {
    s32 i;

    D_800CB300 = camGetPtr();
    D_800CB300->unk4 = 0;
    D_800CB300->unk2 = 0;
    D_800CB300->unk0 = 0;
    D_800CB300->unk24 = 600.0f;
    D_800CB300->unk28 = 150.0f;
    D_800CB300->unk3D = 0;
    D_800CB300->unk44 = 0xFF;
    D_800CB300->unk45 = 0xFF;
    D_800CB300->unk46 = 0xFF;
    D_800CB300->unk47 = 0xFF;
    D_800CB300->unk40 = 0.0f;
    D_800CB300->y = actor->y + 80.0f;
    D_800CB300->unk18 = D_800CB300->x;
    D_800CB300->unk1C = D_800CB300->y;
    D_800CB300->unk20 = D_800CB300->z;
    D_800CB300->unk49 = 1;
    if (camGetMode() >= 2) {
        D_800CB300->unk24 = 400.0f;
    }
    if ((player->playerIndex >= 0) && (player->playerIndex < 4)) {
        D_800CB368[player->playerIndex].object = 0;
        D_800CB368[player->playerIndex].unk08 = 0.0f;
        D_800CB368[player->playerIndex].unk0C = 0.0f;
        D_800CB368[player->playerIndex].unk10 = 1.0f;
        D_800CB368[player->playerIndex].unk14 = 1.0f;
        D_800CB368[player->playerIndex].unk18 = 0.0f;
        D_800CB368[player->playerIndex].unk1C = 0.0f;
        D_800CB368[player->playerIndex].unk20 = 0.0f;
        D_800CB368[player->playerIndex].unk24 = 1.0f;
        D_800CB368[player->playerIndex].unk28 = 0.0f;
    }
    player->unk16F = 0;
    i = 0;
    do {
        func_8001BBB4(actor, player, 1.0f);
        i++;
    } while (i != 8);
    player->unk16F = 1;
}
void func_8001C054(CameraTrackedObject *value) {
    if (D_80079BCC < 24) {
        D_800CB308[D_80079BCC] = value;
        D_80079BCC++;
    }
}
void func_8001C088(CameraTrackedObject *value) {
    s32 index;
    s32 foundIndex;

    index = 0;
    foundIndex = -1;
    if (D_80079BCC > 0) {
        do {
            if (D_800CB308[index] == value) {
                foundIndex = index;
            }
            index++;
        } while (index < D_80079BCC);
    }
    if (foundIndex != -1) {
        for (index = foundIndex; index < D_80079BCC - 1; index++) {
            D_800CB308[index] = D_800CB308[index + 1];
        }
        D_80079BCC--;
    }
}
void func_8001C114(s32 slotIndex, f32 x, f32 y, f32 z) {
    CameraOverrideSlot *slot;
    CameraTrackedObject *object;
    CameraTrackedObject *searchObject;
    CameraBounds *bounds;
    f32 deltaX;
    f32 deltaZ;
    f32 trackedRadius;
    f32 radius;

    if (slotIndex >= 0 && slotIndex < 4) {
        slot = &D_800CB368[slotIndex];
        object = slot->object;
        if (object != 0) {
            bounds = slot->bounds;
            if (bounds != 0) {
                trackedRadius = bounds->trackedRadius;
                deltaX = object->x - x;
                deltaZ = object->z - z;
                trackedRadius *= trackedRadius;
                if (trackedRadius <
                    ((deltaX * deltaX) + (deltaZ * deltaZ))) {
                    slot->object = 0;
                    object = 0;
                } else if ((bounds->flags & 0x8000) &&
                           ((y < bounds->trackedUpper) || (bounds->trackedLower < y))) {
                    slot->object = 0;
                    object = 0;
                }
            }
        }
        if (object == 0) {
            if (D_80079BCC > 0) {
                CameraTrackedObject **current;
                s32 index;

                index = 0, current = D_800CB308;
                do {
                    searchObject = *current;
                    bounds = searchObject->bounds;
                    radius = bounds->radius;
                    deltaX = searchObject->x - x;
                    deltaZ = searchObject->z - z;
                    radius *= radius;
                    if (((deltaX * deltaX) + (deltaZ * deltaZ)) < radius) {
                        slot->object = searchObject;
                        slot->bounds = bounds;
                        if ((bounds->flags & 0x4000) &&
                            ((y < bounds->upper) || (bounds->lower < y))) {
                            slot->object = 0;
                        }
                    }
                    index++;
                    current++;
                } while (index < D_80079BCC);
            }
        }
    }
}
void func_8001C2C4(void) {
}
void func_8001C2CC(void) {
}
void func_8001C2D4(u8 *start, u8 *end) {
    u8 *current = start;

    if (start < end) {
        do {
            *current++ = 0;
        } while (current != end);
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols and assembly supplied the
 * controlPlayerReInit name/role. This Mickey-specific save, clear, initialize,
 * and restore body is independently reconstructed from Mickey's code.
 */
void controlPlayerReInit(ControlActor *actor, f32 x, f32 y, f32 z, s16 arg4, s16 arg5, s16 arg6) {
    ControlPlayer *player;
    s32 saved192;
    s32 saved1A8;
    s32 saved3BA;
    s32 saved45C;
    s32 saved45D;
    ControlPlayerInitState stateStorage;
    ControlPlayerInitState *state;

    player = actor->player;
    state = &stateStorage;
    state->playerIndex = player->playerIndex;
    state->unk11 = player->unk1;
    state->arg4 = arg4;
    state->arg5 = arg5;
    state->arg6 = arg6;
    saved192 = player->unk192;
    saved1A8 = player->flags1A8;
    saved3BA = player->unk3BA;
    saved45C = player->unk45C;
    saved45D = player->unk45D;
    actor->x = x;
    actor->y = y;
    actor->flags &= ~0x400;
    actor->z = z;
    actor->velocityX = 0.0f;
    actor->velocityY = 0.0f;
    actor->velocityZ = 0.0f;
    actor->positionTag = func_8000FAE0(actor->x, actor->y, actor->z);
    actor->alpha = 0xFF;
    actor->unk80 = 0;
    if (player->unkD0 != 0) {
        func_80006EA0(player->unkD0);
    }
    if (player->unkD4 != 0) {
        func_80006EA0(player->unkD4);
    }
    if (player->unkD8 != 0) {
        func_80006EA0(player->unkD8);
    }
    func_8001C2D4((u8 *) player, (u8 *) player + 0xA4);
    func_8001C2D4((u8 *) player + 0xC8, (u8 *) player + 0x134);
    func_8001C2D4((u8 *) player + 0x144, (u8 *) player + 0x19A);
    func_8001C2D4((u8 *) player + 0x1A4, (u8 *) player + 0x34C);
    func_8001C2D4((u8 *) player + 0x3E4, (u8 *) player + 0x400);
    func_8001C4C0(actor, state, 0);
    player->unk11C[0] = -2.0f;
    player->unk11C[1] = -2.0f;
    player->unk11C[2] = -2.0f;
    player->unk11C[3] = -2.0f;
    player->unk192 = saved192;
    player->flags1A8 = saved1A8;
    player->unk3BA = saved3BA;
    player->unk45C = saved45C;
    player->unk45D = saved45D;
}
/* Bounded plateau: 401/403 words, 386 differing words, first mismatch +0x0. */
/* Candidate frame is -0xB0 versus target -0xA8; candidate/target relocations are 40/38. */
/* The typed direct effect-spawn alias is proven; remaining particle/effect lifetimes miss the target register web. */
/* Frame split, measured: target 0x24 save + 0x84 non-save, candidate 0x1C save
 * + 0x94 non-save. The two axes pull opposite ways -- the declaration census
 * that closed func_8001CB84 and func_8001D960 removed 8 non-save bytes here
 * (one redundant s8 copy of player->playerIndex, one redundant loop bound),
 * but the target ALSO holds two more values in callee-saved registers than
 * this candidate does, so the save area is 8 bytes short. Removing further
 * declarations (pointIndex, effectCount, packetIndex, stateCursor) was flat.
 * The `register` qualifiers previously carried here were no-ops: dropping them
 * produced a byte-identical object, so the next lever is the uopt callee-saved
 * tie-break, not more source-level pruning.
 * Named, 2026-09-09: the target uses s0-s6, this candidate only s0-s4. Both
 * agree on the two obvious carriers -- the target holds `player` in s5 and
 * `actor` in s6, this candidate in s3 and s4 -- so the two missing saved webs
 * are the ones the target puts in s3 and s4, and they are the effect/particle
 * list walk.
 * Identity-gated 2026-09-17 (proc 10, instrumented .text identical): two
 * type-2 symbol webs split because totalsave 10 and 11 is not strictly below
 * callee-saved bestcost 16.25, so s5/s6 are never allocated. Split webs have
 * no colour and cannot be forced. L109 identity probes on player,
 * effectOwner, and particleCount did not move those totalsaves. L99 unused
 * pointer/f32 grows non-save, not the save area. Per-arm particle count and
 * entries loads reproduce the target's three-arm entries load but leave the
 * count in a caller-saved temp and grow the function. Next: loop-weighted
 * references that survive copy-prop onto those leftover symbol webs. */
/* PROVENANCE: JFG's corresponding character-control initialization role supplied the control-flow lead; fields and body are reconstructed from Mickey. */
#ifdef NON_MATCHING
void func_8001C4C0(ControlActor *actor, ControlPlayerInitState *state, s32 mode) {
    ControlPlayer *player;
    CharControlEffectList *effectList;
    CharControlEffectDefinition *effect;
    CharControlParticleList *particleList;
    CharControlParticleDefinition *particle;
    CharControlCharacterData *characterData;
    CharControlParticleSlot *slot;
    CharControlSpawnSetup packet;
    f32 *output;
    s16 *position;
    void **effectOwner;
    void *stateCursor;
    s32 effectIndex;
    s32 effectCount;
    s32 effectSlot;
    s32 pointIndex;
    s32 particleCount;
    s32 particleSlotCount;
    s32 packetIndex;

    player = actor->player;
    player->unk1B8 = 0x2000;
    player->playerIndex = *((u8 *) state + 0x10);
    player->unk10 = 0.0f;
    player->unk1 = *((u8 *) state + 0x11);
    actor->rotationX = state->arg4;
    actor->rotationY = state->arg5;
    actor->rotationZ = state->arg6;
    player->unkF0 = actor->rotationX;
    player->unkF2 = actor->rotationY;
    player->unkFE = 0;
    player->unkF4 = actor->rotationZ;
    player->unkDC = (s16) (0x8000 - actor->rotationX);
    func_8005AD64(actor, 0, -1, 0.0f);
    player->unk50 = 1.0f;
    player->unk54 = 1.0f;

    output = &player->unk2C0[0];
    if (D_8007BF10 != 0) {
        player->unk2BC = 4;
        player->unk2B8 = (ControlGravityVector *) &D_800799AC;
    } else {
        player->unk2BC = 4;
        if (D_8007BF1C & 8) {
            player->unk2B8 = (ControlGravityVector *) &D_8007996C;
        } else {
            player->unk2B8 = (ControlGravityVector *) &D_8007992C;
        }
    }
    player->unk33C = 0;
    player->unk340 = 0;
    pointIndex = 0;
    if (player->unk2BC > 0) {
        effectSlot = 0;
        do {
            effectSlot++;
            output += 3;
            output[-3] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex));
            output[-2] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex + 4));
            output[-1] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex + 8));
            pointIndex += 0x10;
        } while (effectSlot < player->unk2BC);
    }
    func_8001EFFC(actor, player, &player->unk2F0);

    effectIndex = (s32) player->unk1;
    if ((player->unk1 < 0) || (player->unk1 >= 10)) {
        effectIndex = 0;
    }
    if (player->playerIndex < (D_8007BEF8 - D_8007BEFC)) {
        effectList = &D_8007980C[effectIndex];
        effect = effectList->entries;
        if (effect != 0) {
            effectCount = effectList->count;
            effectSlot = 0;
            effectOwner = (void **) player;
            if (effectCount > 0) {
                do {
                    if (effectOwner[0x134 / 4] == 0) {
                        effectOwner[0x134 / 4] =
                            charControlEffectSpawnTrap(
                                actor, effect->kind, effect->angle, effect->index,
                                effect->x, effect->y, effect->z, effect->w,
                                effect->arg14, effect->arg15, effect->arg16,
                                effect->arg17);
                    }
                    effectSlot++;
                    effectOwner++;
                    effect++;
                } while (effectSlot != effectCount);
            }
        }
    }

    characterData = (CharControlCharacterData *)
        *(*(actor->unk68 + actor->unk3A));
    particleCount = effectIndex * 8;
    particleSlotCount = 0;
    if (levelGetType() == 3) {
        particleList = (CharControlParticleList *)
            ((u8 *) D_8007987C + particleCount);
    } else if (D_8007BF04 != 0) {
        particleList = (CharControlParticleList *)
            ((u8 *) D_800798DC + particleCount);
    } else {
        particleList = (CharControlParticleList *)
            ((u8 *) D_8007987C + particleCount);
    }
    particleCount = particleList->count;
    particle = particleList->entries;
    slot = (CharControlParticleSlot *) player->particles;
    if (particleCount != 0) {
        particleCount--;
        do {
            if (slot->handle == 0) {
                if (particle->index < characterData->count) {
                    slot->kind = particle->kind;
                    slot->index = particle->index;
                    position = (s16 *) ((u8 *) characterData->positions +
                        (characterData->indexTable[particle->index].offset * 10));
                    slot->model = (s8)
                        characterData->indexTable[particle->index].value;
                    slot->handle = func_80046EC4(
                        position[0], position[1], position[2],
                        (s16) (particle->angle << 8),
                        (s16) (particle->angleLow << 8),
                        (f32) particle->arg4, (f32) particle->arg6,
                        (f32) particle->arg8, particle->argA, particle->argC,
                        particle->argE);
                }
            }
            slot->active = 0;
            slot->unk4 = 0;
            slot->unk6 = 0;
            particle++;
            slot++;
            particleSlotCount++;
        } while (particleCount-- != 0);
    }
    if (particleSlotCount < 4) {
        do {
            particleSlotCount++;
            slot->handle = 0;
            slot++;
        } while (particleSlotCount < 4);
    }

    player->unk38 = actor->x;
    player->unk3C = actor->y;
    player->unk40 = actor->z;
    player->unk44 = actor->x;
    player->unk48 = actor->y;
    player->unk190 = 0xFF;
    player->unk18D = 0;
    player->unk338 = 0;
    player->unk348 = 0;
    player->unk456 = 0;
    player->unk387 = 0xFF;
    player->unk388 = 0;
    player->unk183 = 0x80;
    player->unk184 = 0;
    player->unk185 = 0;
    player->unk186 = 0;
    player->unk187 = 0;
    player->unk188 = 0.0f;
    player->unk4C = actor->z;
    if (player->playerIndex != -1) {
        camSetNo(player->playerIndex, 0, &D_800CB300);
        func_8001BE0C(actor, player);
    }
    player->unk16C = 0;
    player->unk2 = 0;
    player->unk3 = 0;
    player->unk16D = 0;
    if (player->actions == 0) {
        player->unk19A = 0xFF;
        player->unk19B = 0;
        player->unk19C = 0;
        player->actions = 0;
    }
    player->unk1A4 = 0;
    player->unk1A5 = 0;
    player->unk1A6 = 0;
    player->unk172 = 0;
    player->unk173 = 0;
    stateCursor = (u8 *) player;
    player->unk174 = 1.0f;
    player->unk178 = 2.0f;
    player->unk17C = 1.0f;
    particleSlotCount = 0;
    do {
        packetIndex = particleSlotCount * 8;
        particleSlotCount++;
        stateCursor = (u8 *) stateCursor + 1;
        *((u8 *) stateCursor + 0x12B) = 0;
        *((u8 *) stateCursor + 0x12F) = (u8) packetIndex;
    } while (particleSlotCount < 4);
    player->unk3EC = 0.0f;
    player->unk3F0 = D_80081844;
    player->unk43C = actor->rotationX;
    player->unk43E = actor->rotationY;
    player->unk440 = actor->rotationZ;
    player->unk444 = actor->unk8;
    player->unk448 = actor->x;
    player->unk44C = actor->y;
    player->unk3BA = 0xFF;
    player->unk450 = actor->z;
    if (levelGetType() == 3) {
        packetIndex = 0;
        if (mode != 0) {
            packet.kind = 0x124;
            packet.arg04 = 0;
            packet.arg06 = 0;
            packet.arg08 = 0;
            packet.owner = actor;
            do {
                packet.arg0A = packetIndex;
                func_8000590C((ControlSpawnPacket *) &packet, 1);
                packetIndex++;
            } while (packetIndex != 3);
        }
    }
    if ((D_8007BF1C & 2) && (*func_80028F54() != 1)) {
        player->unk192 = 0xA;
        return;
    }
    player->unk192 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C4C0.s")
#endif
void func_8001CB0C(ControlTransform *transform, ControlPlayer *player) {
    player->unk2BC = 1;
    if (D_8007BF1C & 8) {
        player->unk2B8 = &D_800799FC;
    } else {
        player->unk2B8 = &D_800799EC;
    }
    player->unk33C = 0;
    player->unk340 = 0;
    player->unk2C0[0] = player->unk2B8->x;
    player->unk2C0[1] = player->unk2B8->y;
    player->unk2C0[2] = player->unk2B8->z;
    func_8001EFFC(transform, player, &player->unk2C0[12]);
}
/* PROVENANCE: JFG's public charControl.c identifies the corresponding
 * controlSquashCheckPost-adjacent character-control routine, but publishes
 * assembly only; this body is reconstructed from Mickey's fields, calls,
 * and branch conditions. */
/* Frame law used here (see docs/ido-learnings.md): declared locals occupy the
 * TOP of the local region in declaration order, first-declared highest; every
 * value written as an expression instead of a named local is homed in the
 * compiler-temp region below them.  sp7C/sp70/character keep their m2c names
 * because those are the target's own displacements. */
/* var_f12 is ONE scratch float reused twice, and that is what closed this
 * function.  Naming the ballistic step keeps it in a coloured web where the
 * inline expression spent floating-point ring temps, which cost 40 register
 * words; declaring a *new* local for it instead of reusing var_f12 pushed
 * every compiler temp 4 bytes down the frame and cost 25 displacement words.
 * Both halves are needed: 53 -> 39 -> 0. */
void func_8001CB84(ControlActor *actor, s32 updateRate) {
    f32 sp7C;
    f32 var_f12;
    f32 temp_f2_2;
    CharControlLevelDescription *sp70;
    ControlPlayer *player;
    CharControlSpawnSetup packetD0;
    CharControlSpawnSetup packetD8;
    s32 temp_v1_3;
    s32 character;

    player = actor->player;
    player->unk3 = player->unk2;
    if (player->unkC8 != NULL) {
        sp70 = ((CharControlLevelRequest *) player->unkC8)->description;
        character = sp70->characterLow;
        if (sp70->characterHigh != 0xFF) {
            character |= sp70->characterHigh << 8;
        }
        mainChangeLevel(character, mainGetNextCharacter(),
                        sp70->nextLevel, frontGetMode(),
                        sp70->camera, 0);
        if (sp70->animGroup != -1) {
            mainSetAnimGroup(sp70->animGroup);
        }
        func_80006EA0(player->unkC8);
        player->unkC8 = NULL;
    }
    if (player->unk1A6 != 0) {
        if (player->unk1A5 == 0) {
            player->unk1A5 = 1;
            if (player->unk1A4 == 1) {
                camSetNo(player->playerIndex);
                func_800214AC();
            }
        }
        player->unk1A6 = (s16) (player->unk1A6 - updateRate);
        if (player->unk1A6 <= 0) {
            player->unk1A6 = 0;
            if (player->unk1A4 == 1) {
                camSetNo(player->playerIndex);
                func_800214AC();
            }
        }
    }
    if (player->unk18D != 0) {
        player->unk18D = (s8) (player->unk18D - updateRate);
        if (player->unk18D <= 0) {
            player->unk18D = 0;
            player->unk54 = 1.0f;
            if ((player->unk338 != NULL) && (player->unk338->unk44 == 0x52)) {
                *(s16 *) player->unk338->state &= 0xFFFD;
                player->unk338->unk20 = (f32) player->unk338->state->unk18;
                player->unk338 = NULL;
            }
        }
    }
    if (player->unk50 != player->unk54) {
        func_8001F09C(player, updateRate);
    }
    actor->unk48->unk54 = 0.0f;
    if (player->unk158 != 0) {
        if (player->unk158 & 0x8000) {
            if (player->unkB4 != NULL) {
                func_800031E8(player->unkB4);
            }
            func_80002FE0(0x21, actor->x, actor->y, actor->z, 4,
                          &player->unkB4);
            player->unk158 = (s16) (player->unk158 & 0x7FFF);
        }
        var_f12 = (player->unk150 * (f32) updateRate) -
                  (0.5f * D_800CB304 * (f32) updateRate * (f32) updateRate);
        player->unk154 = player->unk154 + var_f12;
        player->unk150 = player->unk150 - (D_800CB304 * (f32) updateRate);
        if (player->unk154 < 0.0f) {
            player->unk154 = -player->unk154;
            player->unk158 = (s16) (player->unk158 - 1);
            player->unk150 = -player->unk150 * 0.5f;
        }
        if ((player->unk158 >= 2) ||
            ((player->unk158 == 1) && (player->unk150 > 0.0f))) {
            player->unk160 = (s16) (player->unk160 + player->unk15A * updateRate);
            player->unk164 = (s16) (player->unk164 + player->unk15E * updateRate);
            player->unk162 = (s16) (player->unk162 + player->unk15C * updateRate);
        } else if (player->unk158 == 1) {
            sp7C = Powerf(D_80081848, updateRate);
            player->unk160 = dAngle(player->unk160, 0, sp7C);
            player->unk164 = dAngle(player->unk164, 0, sp7C);
            player->unk162 = dAngle(player->unk162, 0, sp7C);
        } else if (player->unk158 == 0) {
            player->unk160 = 0;
            player->unk164 = 0;
            player->unk162 = 0;
            player->unk154 = 0.0f;
            player->unk150 = 0.0f;
        }
        temp_f2_2 = func_8002A8BC(player->unk162) * func_8002A8BC(player->unk164);
        if (temp_f2_2 < 0.0f) {
            var_f12 = 0.0f;
        } else {
            var_f12 = temp_f2_2 * temp_f2_2;
        }
        player->unk14C = 30.0f - (30.0f * var_f12);
        actor->unk48->unk54 = player->unk154;
        player->unk185 = 0;
        player->unk188 = 0.0f;
    }
    if (D_8007BF10 != 0) {
        if (player->unk191 == 0) {
            TrapDanglingJump(actor, updateRate);
        }
        TrapDanglingJump(actor, updateRate);
    } else if (player->flags1A8 & 1) {
        TrapDanglingJump(actor, player, updateRate);
        if (player->unk191 == 0) {
            TrapDanglingJump(actor, updateRate);
        }
    } else {
        if (player->unk191 == 0) {
            if ((player->unk1 == 0) || (player->unk1 == 1) ||
                (player->unk1 == 2) || (player->unk1 == 3)) {
                TrapDanglingJump(actor, updateRate);
            } else {
                TrapDanglingJump(actor, updateRate);
            }
        }
        TrapDanglingJump(actor, updateRate);
    }
    if ((player->unk168 != 0) && (player->unk3FA == 0)) {
        player->unk168 = (s16) (player->unk168 - updateRate);
        if (player->unk168 < 0) {
            player->unk168 = 0;
        }
        if (player->unk168 & 8) {
            player->unk190 = 0x40;
        } else {
            player->unk190 = 0xFF;
        }
    }
    if (player->unk16A != 0) {
        player->unk16A = (s16) (player->unk16A - updateRate);
        if (player->unk16A <= 0) {
            player->unk16A = 0;
        } else if (player->unkD0 == NULL) {
            packetD0.kind = 0xB8;
            packetD0.arg04 = 0;
            packetD0.arg06 = 0xE;
            packetD0.arg08 = 7;
            packetD0.arg0A = 0x14;
            packetD0.arg0B = 0x14;
            packetD0.owner = actor;
            player->unkD0 = func_8000590C(
                (ControlSpawnPacket *) &packetD0, 1);
            if (player->unkD0 != NULL) {
                ((ControlSpawned *) player->unkD0)->unk3C = 0;
            }
        }
    }
    if (!(player->flags1A8 & 1)) {
        if ((player->unkD8 == NULL) &&
            (TrapDanglingJump((void *) player->playerIndex) != 0)) {
            packetD8.kind = 0x14C;
            packetD8.arg02 = 0x10;
            packetD8.arg03 = 0;
            packetD8.arg04 = 0;
            packetD8.arg06 = 0xE;
            packetD8.arg08 = 7;
            packetD8.owner = actor;
            player->unkD8 = func_8000590C(
                (ControlSpawnPacket *) &packetD8, 1);
            if (player->unkD8 != NULL) {
                ((ControlSpawned *) player->unkD8)->unk3C = 0;
            }
        } else if ((player->unkD8 != NULL) &&
                   (TrapDanglingJump((void *) player->playerIndex) == 0)) {
            func_80006EA0(player->unkD8);
        }
    }
    controlDisableJoypad(player, 0);
    if (player->unk3FA != 0) {
        if ((s32) player->unk190 > 0) {
            temp_v1_3 = player->unk190 - (updateRate * 4);
            if (temp_v1_3 <= 0) {
                if (player->unkAC != NULL) {
                    func_800031E8(player->unkAC);
                    actor->unk80 = 0;
                }
                player->unk191 = 1;
                player->unk190 = 0;
                actor->x = player->unk44;
                actor->y = player->unk48 + D_8008184C;
                actor->z = player->unk4C;
            } else {
                player->unk190 = (u8) temp_v1_3;
            }
        }
    }
    if (player->unkA4 != NULL) {
        func_800031C0(player->unkA4, actor->x, actor->y, actor->z);
    }
}
/*
 * Workbench: structure-mismatch, 96/95 instructions, 40 raw/9 normalized differences, first +0xE0; frame exact.
 * Levers tried: prior flags/commutative/volatile/prototype forms plus fresh pointer scope, lvalue, and typed-stride forms.
 * Remains: candidate CSE hoists the global base address before the camera-count call; target keeps it split.
 */
void func_8001D2A0(ControlActor *actor, s32 arg1)
{
  s32 cameraIndex;
  ControlPlayer *player;
  player = actor->player;
  player->unk43C = actor->rotationX;
  player->unk43E = actor->rotationY;
  player->unk440 = actor->rotationZ;
  player->unk444 = actor->unk8;
  player->unk448 = actor->x;
  player->unk44C = actor->y;
  player->unk450 = actor->z;
  if (player->unk158 != 0)
  {
    player->unk43C += player->unk160;
    player->unk43E += player->unk164;
    player->unk440 += player->unk162;
    player->unk44C += player->unk154 + player->unk14C;
  }
  if (!(player->flags1A8 & 1))
  {
    TrapDanglingJump(actor, player, arg1);
    D_800CB300 = D_800CB300;
  }
  if (player->unkD4 != 0)
  {
    TrapDanglingJump(player->unkD4, arg1);
  }
  D_800CB300 = camGetListPtr();
  cameraIndex = mainGetNumberOfCameras() - 1;
  if (player->playerIndex < cameraIndex)
  {
    cameraIndex = player->playerIndex;
  }
  D_800CB300 = (ControlCameraState *) ((cameraIndex * (sizeof(ControlCameraState))) + ((u8 *) D_800CB300));
  camSetNo(player->playerIndex, cameraIndex, &D_800CB300);
  if ((player->unk190 != 0) || (player->unk3FA == 0))
  {
    func_8001BBB4(actor, player, (f32) arg1);
  }
}
void func_8001D41C(ControlActor *actor, ControlPlayer *player, s32 updateRate) {
    ControlPlayerActions *actions;
    ControlPlayerAction action;
    s32 effectIndex;

    if (player->unk19C > 0) {
        player->unk19C -= updateRate;
        if (player->unk19C <= 0) {
            player->unk19C = 0;
            if ((D_8007C1A0 == 1) && !(player->flags1A8 & 1)) {
                effectIndex = player->unk19A;
                if ((effectIndex >= 2) && (effectIndex < 10)) {
                    TrapDanglingJump(effectIndex + 30);
                }
            }
        }
    }

    actions = player->actions;
    if ((actions == 0) || (player->unk19C != 0) ||
        (TrapDanglingJump(player->unkD4) != 0)) {
        if (player->controlDkeys & 0x2000) {
            if ((player->unk1 >= 0) && (player->unk1 < 10)) {
                if (D_8007BF1C & 4) {
                    if (player->unkA4 != 0) {
                        func_800031E8(player->unkA4);
                    }
                    func_80002FE0(
                        D_80079A20[player->unk1][func_800299E8(0, 3)],
                        actor->x, actor->y, actor->z, 4, &player->unkA4);
                    return;
                }
                if (player->unkA8 != 0) {
                    func_800031E8(player->unkA8);
                }
                func_80002FE0(D_80079A0C[player->unk1], actor->x, actor->y,
                              actor->z, 4, &player->unkA8);
            }
        }
    } else if (player->controlDkeys & 0x2000) {
        action = actions->positive;
        if ((action != 0) && (player->controlYjoy >= 65)) {
            action(actor);
            return;
        }
        action = actions->negative;
        if ((action != 0) && (player->controlYjoy < -64)) {
            action(actor);
            return;
        }
        action = actions->fallback;
        if (action != 0) {
            action(actor);
        }
    }
}
void controlFrozen(ControlActor *actor, ControlPlayer *player) {
    if (func_800291FC() == 1) {
        func_800291D8(10);
    }
    if (joyGetPressed(player->playerIndex) & 0xF00F) {
        func_8001D690(actor, player);
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols and assembly supplied the
 * controlRestartPlayer name/role. This Mickey-specific respawn-point search
 * and reinitialization body is independently reconstructed from Mickey's code.
 */
void func_8001D690(ControlActor *actor, ControlPlayer *player) {
    s32 start;
    s32 end;
    ControlActor **objects;
    ControlActor *current;
    s32 maxIndex;
    s32 playerCount;
    f32 radius;
    s32 mode;
    s32 candidateIndex;
    ControlActor *hitActor;
    ControlActor *candidates[8];
    s32 count;
    s32 hit;

    playerCount = func_800291FC();
    if (playerCount >= 2) {
        objects = func_8000572C(&start, &end);
        count = 0;
        radius = 32.0f;
        mode = 0;
        if (start < end) {
            do {
                current = objects[start++];
                if (current->kind == 5) {
                    hit = func_8005776C(current->x, current->y, current->z,
                                       radius, mode, &hitActor);
                    if ((hit == 0) || ((hit == 1) && (actor == hitActor))) {
                        candidates[count++] = current;
                    }
                }
            } while (start < end);
        }
        if (count == 0) {
            current = actor;
        } else if (count == 1) {
            current = candidates[0];
        } else {
            maxIndex = count - 1;
            candidateIndex = mathRnd(0, maxIndex);
            current = candidates[candidateIndex];
        }
        controlPlayerReInit(actor, current->x, current->y, current->z,
                            current->rotationX, current->rotationY,
                            current->rotationZ);
    } else {
        func_800282C8();
    }
}
/* PROVENANCE -- adapted from JFG's src/charControl.c dAngle. */
s16 dAngle(s16 arg0, s16 arg1, f32 arg2) {
    s32 temp_t1;
    s32 var_v1;

    var_v1 = (arg1 - arg0) & 0xFFFF;
    temp_t1 = (arg0 - arg1) & 0xFFFF;
    if (temp_t1 < var_v1) {
        var_v1 = -temp_t1;
    }
    return (s16) (arg0 + (s32) ((f32) var_v1 * arg2));
}
/* PROVENANCE -- adapted from JFG's charControl controlMakeV implementation. */
#ifdef NON_MATCHING
f32 func_8001D880(f32 arg0, f32 arg1, f32 *table, f32 divisor) {
    f32 base;
    f32 value;
    s32 index;

    arg1 *= 10.0f;
    index = (s32) arg1;
    base = table[index];
    value = table[index + 1];
    value = ((value - base) * (arg1 - (f32) index)) + base;
    arg0 *= 10.0f;
    index = (s32) arg0;
    base = table[index];
    return (value - (base + ((table[index + 1] - base) * (arg0 - (f32) index)))) / divisor;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D880.s")
#endif
/* PROVENANCE -- adapted from JFG's src/charControl.c controlFSUvels. */
void controlFSUvels(s16 *rotation, ControlPlayer *player) {
    s16 sp18[3];

    sp18[0] = rotation[0];
    sp18[1] = rotation[1];
    sp18[2] = 0;
    pointListRPY(3, sp18, D_80079BD4, player->unk14);
}
typedef struct ControlFlameSlot {
    u8 state;
    u8 mode;
    u8 pad02[2];
    u8 intensity;
    u8 pad05;
    s16 phase;
    void *particle;
} ControlFlameSlot;

typedef struct ControlFlameParticle {
    u8 pad00[0x18];
    f32 value18;
    f32 value1C;
    s16 value20;
    s16 value22;
    s16 value24;
} ControlFlameParticle;

/* Workbench verdict: register-only residual, 16 differing words, first mismatch +0x164. */
/* Candidate: 220/220 instructions, frame -0x60 on both sides, all six relocations
 * at the target's own instruction indexes, and every stack displacement equal. */
/* Four source facts recovered from the target:
 *  - the loop counter is ONE variable spilled to its own home each iteration,
 *    not an m2c sp5C/var_v0 pair; declaring it first is what puts its home at
 *    the target's displacement and keeps the frame at 0x60,
 *  - case 1 re-reads the particle's angle field rather than reading the value
 *    already in var_s6; that CSE is what makes IDO keep the loaded value in a
 *    caller-saved register and copy it into the saved one,
 *  - case 2 spells the scaled angle inline, exactly as case 3 does. Naming the
 *    call result in a local made it a uopt-coloured web where the target pops a
 *    ugen ring temp, and that one class crossing rotated twelve downstream webs,
 *  - case 2 performs actor->unk80 |= arg2 AFTER the whole var_s6 product, so
 *    the product's four ring temps are drawn before the or's two loads.
 * The last two are one composition: neither alone is an improvement. */
/* PROVENANCE: JFG's public controlUpdateJetFlames role and Mickey's m2c/assembly establish
 * the state-machine order; no external body is copied into this reconstruction. */
void func_8001D960(ControlActor *actor, ControlPlayer *player, s32 arg2, s32 arg3,
                   s32 arg4) {
    s32 var_v0;
    ControlFlameSlot *var_s1;
    void *temp_s7;
    s32 var_s5;
    s32 var_s0;
    s32 var_s3;
    s32 var_s6;
    f32 var_f20;
    f32 var_f22;

    var_v0 = 0;
    var_s5 = 1;
    var_s1 = (ControlFlameSlot *) ((u8 *) player + 0x34C);
    do {
        temp_s7 = var_s1->particle;
        if (temp_s7 != NULL) {
            var_s6 = *(s16 *) ((u8 *) temp_s7 + 0x24);
            var_s0 = var_s1->intensity;
            var_s3 = var_s1->phase;
            var_f20 = *(f32 *) ((u8 *) temp_s7 + 0x18);
            var_f22 = *(f32 *) ((u8 *) temp_s7 + 0x1C);
            if (var_s1->state == 2) {
                if (var_s1->mode == 0) {
                    var_s0 -= arg4 << 5;
                    if (var_s0 < 0) {
                        var_s0 = 0;
                    }
                    var_s3 = actor->rotationX;
                    if (player->unk186 & var_s5) {
                        var_s1->mode = 2;
                    }
                } else {
                    var_s0 += arg4 << 5;
                    if (var_s0 >= 0x100) {
                        var_s0 = 0xFF;
                    }
                    var_s3 = actor->rotationX;
                    if (!(player->unk186 & var_s5)) {
                        var_s1->mode = 0;
                    }
                }
                var_f20 *= (f32) var_s0 / 255.0f;
                var_f22 *= (f32) var_s0 / 255.0f;
                if (actor->unk70 != NULL) {
                    s32 temp_a0;

                    temp_a0 = *actor->unk70;
                    if (temp_a0 != 0) {
                        changeLightIntensity((void *) temp_a0, var_s0);
                    }
                }
            } else {
                switch (var_s1->mode) {
                case 0:
                    var_s0 = 0;
                    if (player->unk186 & var_s5) {
                        s32 temp_s2;

                        temp_s2 = actor->unk80;
                        actor->unk80 = arg3;
                        partUpdateTriggers(actor, 2);
                        actor->unk80 = temp_s2;
                        var_s1->mode = 1;
                    }
                    break;
                case 1:
                    var_s0 += arg4 << 5;
                    var_s6 = (s32) (*(s16 *) ((u8 *) temp_s7 + 0x24) * var_s0) >> 7;
                    if (var_s0 >= 0x100) {
                        var_s0 = 0xFF;
                        if (player->unk186 & var_s5) {
                            var_s1->mode = 2;
                        } else {
                            var_s1->mode = 3;
                        }
                    }
                    break;
                case 2:
                    var_s0 += arg4 * 0x10;
                    if (var_s0 >= 0x100) {
                        var_s0 = 0xFF;
                    }
                    var_s3 += arg4 << 0xC;
                    var_s6 = (s32) ((func_8002A204((s16) (var_s3 << 8)) + 0x18000) *
                                    ((s32) (var_s6 * var_s0) >> 8)) >> 0x10;
                    actor->unk80 |= arg2;
                    if (!(player->unk186 & var_s5)) {
                        var_s1->mode = 3;
                    }
                    break;
                case 3:
                    var_s0 -= arg4 * 8;
                    if (var_s0 <= 0) {
                        var_s0 = 0;
                        var_s1->mode = 0;
                    } else {
                        var_s3 += arg4 << 0xC;
                        var_s6 = (s32) ((func_8002A204((s16) (var_s3 << 8)) + 0x18000) *
                                        ((s32) (var_s6 * var_s0) >> 8)) >> 0x10;
                        if (player->unk186 & var_s5) {
                            var_s1->mode = 2;
                        }
                    }
                    break;
                }
            }
            var_s1->intensity = var_s0;
            var_s1->phase = var_s3;
            if (var_s0 != 0) {
                func_800475E8(temp_s7, var_s1->phase);
                func_800479D4(temp_s7, var_s6, var_f20, var_f22, var_s1->intensity);
            }
        }
        var_s5 *= 2;
        var_s1++;
        var_v0++;
    } while (var_v0 != 4);
}
void func_8001DCD0(s16 rotation, ControlVector3 *vector, s16 *pitch, s16 *yaw) {
    f32 cosine;
    f32 pitchX;
    s32 angle;
    f32 y;
    f32 transformedX;

    angle = -rotation;
    cosine = func_8002A8C0(angle);
    transformedX = func_8002A8BC(angle);
    y = vector->y;
    pitchX = (vector->z * cosine) + (vector->x * transformedX);
    transformedX = (vector->z * transformedX) - (vector->x * cosine);
    *pitch = Arctanf(-pitchX, y);
    *yaw = Arctanf(transformedX, y);
}
s16 dAngle(s16 arg0, s16 arg1, f32 arg2);
void func_8001DCD0(s16 rotation, ControlVector3 *vector, s16 *pitch, s16 *yaw);
/* PROVENANCE: JFG's public charControl.c and hit.c identify the related
 * ground-hit and polygon-edge control family, but publish assembly only;
 * this body is reconstructed from Mickey's collision records and fields. */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 433 differing words, first mismatch +0x0. */
/* Candidate shape: 533 instructions and frame -0x268, both exact; 260 alignment gaps. */
/* Declared to the target's own stack ladder, read off tools/frame_census.py.
 * The frame was already exact at 0x268 and the census still reported 19 slots
 * only this candidate used and 24 only the target did, so the excess was never
 * a size question -- it was ORDER, and the four rules the census pinned down
 * are worth stating because none of them is visible in a frame total:
 *
 *  a. EVERY declared local reserves a home, in declaration order, descending
 *     from the frame top and aligned to its own type, whether or not it ever
 *     reaches memory.  The seven register-class locals this function used to
 *     declare first reserved 0x1C bytes above `points` and pushed every
 *     homed object 0xF8 too low.  That is L99 read forwards: the ladder is a
 *     linear readout of the declaration list, so the list can be solved for.
 *
 *  b. The target's order is records, points, radius -- largest first, and
 *     three register-class slots above records -- then the accumulator triple,
 *     the RPY triple, endpoint, the spEC triple, the spE0 triple, spD4, the
 *     three spilled floats, spB8, spAC, the rotation shorts.
 *
 *  c. A three-float local whose address escapes only through its FIRST member
 *     loses the other two: they are separate symbols, nothing escapes them,
 *     and uopt constant-propagates their stores away.  The target stores all
 *     three, so the target declares them as ONE object.  spE0 and sp104 are
 *     ControlVector3 here for that reason, and spA4 is an s16 array.  This is
 *     L118 in the other direction -- not "does the target declare it at all"
 *     but "does the target declare it SEPARATELY at all".
 *
 *  d. spEC/spF0/spF4 must stay three separate locals: the target hoists the
 *     spF0 and spF4 loads out of the accumulate loop exactly as separate
 *     non-escaping locals do, and making them one aggregate keeps them inside.
 *     Making spEC an aggregate and reverting was worth 109 words on its own.
 *
 * Two further edits were measured here, both structural rather than home:
 * `record = records` moved inside the `spB8 > 0` preheader so uopt cannot
 * merge the array's address with the call argument's (L110), worth 9 words
 * and the second `addiu ...,sp,348` the target emits; and the two `!= spB8`
 * loop guards respelled `< spB8` for the target's `blez`.
 *
 * Measured with tools/align_symbol.py: 433 -> 299 masked words, byte-exact
 * 146 -> 390 of 533, register naming 241 -> 79, really different 131 -> 57,
 * size delta 0 throughout.  The home set is now exact except for two compiler
 * temps (0x7C here against the target's 0x84 and 0x8C).
 *
 * Measured and REJECTED: merging pointIndex and collisionIndex into a single
 * index -- the target's `move s2,zero` sits on the collision loop's exit path,
 * which reads like one shared index, but the merge drops ten instructions
 * (size delta -40) and byte-exact to 288, so the target has at least two.
 * Hoisting either index reset to the previous loop's exit costs +4 words. */
s32 func_8001DD70(ControlActor *actor, ControlPlayer *player, f32 updateRate) {
    ControlVector3 *pointSource;
    u8 *pointDest;
    u32 temp_v0;
    u8 records[0x100];
    u8 points[0x30];
    f32 radius[4];
    volatile f32 sp118;
    volatile f32 sp114;
    f32 sp110;
    ControlVector3 sp104;
    ControlVector3 endpoint;
    ControlVector3 spEC;
    ControlVector3 spE0;
    CharControlGroundRecord *record;
    u8 *player320;
    f32 spD4;
    u8 *player324;
    u32 bit;
    f32 directionX;
    f32 directionZ;
    f32 normalZ;
    f32 var_f18;
    s32 spB8;
    f32 normalX;
    u32 collisionMask;
    s32 spAC;
    s16 spA4[4];
    s16 spA2;
    s16 spA0;
    s32 pointIndex;
    s32 collisionIndex;
    f32 var_f4;
    f32 temp_f14;
    u32 temp_v0_2;

    func_8001EFFC((ControlTransform *) actor, player, (f32 *) points);
    spB8 = player->unk2BC;
    {
        {

        pointIndex = 0;
        while (pointIndex < spB8) {
            radius[pointIndex] = player->unk2B8[pointIndex].w;
            pointIndex += 1;
        }
        trackMakePolylist(spB8, (ControlVector3 *) &player->unk2F0, points, radius,
                          player->unk33C, 1);
        temp_v0 = (u32) func_80010B4C(
            spB8, &player->unk2F0, points, radius, records, &actor->x, actor);
        spAC = 0;
        if ((temp_v0 >> 30) != 0) {
            actor->x = player->unk38;
            actor->y = player->unk3C;
            actor->z = player->unk40;
            spAC = 2;
        } else {
            player->unk349 = 0;
            player->unk34A = 0;
            player->unk34B = 0;
            player->unk18E = 0;
            player->unk334 = NULL;
            player->unk344 = 0;
            bit = 1;
            collisionMask = temp_v0 & 0x3FFFFFFF;
            collisionIndex = 0;
            if (spB8 > 0) {
                record = (CharControlGroundRecord *) records;
                player320 = (u8 *) player;
                player324 = (u8 *) player;
                do {
                    if ((collisionMask & 1) != 0) {
                        if (record->unk3D & 0x12) {
                            player->unk349 = (u8) (player->unk349 | bit);
                            if ((record->unk3D & 0x10) &&
                                (record->hitObject != NULL)) {
                                player->unk334 = record->hitObject;
                            }
                        }
                        if (record->unk3D & 0x48) {
                            player->unk34B = (u8) (player->unk34B | bit);
                        }
                        if (record->unk3D & 0x24) {
                            spE0.x = 0.0f;
                        spE0.y = 0.0f;
                        spE0.z = -1.0f;
                        spA4[1] = 0;
                        spA4[2] = 0;
                        spA4[0] = actor->rotationX;
                        mathOneFloatRPY((ControlTransform *) spA4, &spE0.x);
                        spD4 = sqrtf((record->unk18 * record->unk18) +
                                     (record->unk10 * record->unk10));
                        normalZ = record->unk18 / spD4;
                        normalX = record->unk10 / spD4;
                        player->unk90 = (normalZ * spE0.x) -
                                        (spE0.z * normalX);
                        var_f18 = (spE0.z * normalZ) +
                                  (spE0.x * normalX);
                        player->unk8C = var_f18;
                        if (var_f18 < 0.0f) {
                            var_f18 = -var_f18;
                        }
                        var_f4 = sqrtf((actor->velocityX * actor->velocityX) +
                                       (actor->velocityZ * actor->velocityZ));
                        if (var_f4 > 0.0f) {
                            directionX = actor->velocityX / var_f4;
                            directionZ = actor->velocityZ / var_f4;
                        }
                        temp_v0_2 = player->unk198;
                        if ((temp_v0_2 == 0) && (var_f4 > 8.0f) &&
                            ((((normalX * directionX) + (normalZ * directionZ)) < D_80081850) ||
                             (D_80081854 < ((normalX * directionX) + (normalZ * directionZ))))) {
                            player->unk78 = 0.0f;
                            player->unk74 = ((2.0f * -((normalX * directionX) + (normalZ * directionZ))) * normalX) +
                                            directionX;
                            player->unk7C = ((2.0f * -((normalX * directionX) + (normalZ * directionZ))) * normalZ) +
                                            directionZ;
                            temp_f14 = ((D_80081858 * var_f18) + 0.5f) *
                                       var_f4;
                            player->unk80 = temp_f14;
                            player->unk84 = temp_f14;
                            player->unk181 = 1;
                            player->unk4 *= 0.5f;
                            player->unk88 = D_8008185C;
                            player->unk8 *= 0.5f;
                        } else if (player->flags1A8 & 1) {
                            var_f4 = (f32) temp_v0_2;
                            if (var_f4 < 240.0f) {
                                player->unk198 = (u8) (temp_v0_2 +
                                                       (s32) updateRate);
                            } else {
                                player->unk198 = 0;
                                player->unk166 = 1;
                            }
                        } else {
                            player->unk198 = 1;
                        }
                            spAC |= 1;
                            player->unk34A = (u8) (player->unk34A | bit);
                        }
                    }
                    player320[0x320] = record->unk3C;
                    *(s32 *) (player324 + 0x324) = record->unk38;
                    player->unk344 |= record->unk38;
                    record += 1;
                    collisionIndex += 1;
                    player320 += 1;
                    player324 += 4;
                    bit = (bit * 2) & 0xFF;
                    collisionMask >>= 1;
                } while (collisionIndex != spB8);
            }
        }
    }
    {
        sp110 = 0.0f;
        sp114 = 0.0f;
        sp118 = 0.0f;
        if (player->unk16C != 1) {
            sp104.x = 0.0f;
            sp104.z = 0.0f;
            sp104.y = -50.0f;
            mathOneFloatRPY((ControlTransform *) actor, &sp104.x);
            collisionIndex = 0;
            if (spB8 > 0) {
                pointSource = (ControlVector3 *) points;
                do {
                    endpoint.x = sp104.x + pointSource->x;
                    endpoint.y = sp104.y + pointSource->y;
                    endpoint.z = sp104.z + pointSource->z;
                    spD4 = 1.0f;
                    if (func_80010654(pointSource, &endpoint,
                                      &spEC, &spD4) != 0) {
                        sp110 += spEC.x;
                        sp114 += spEC.y;
                        sp118 += spEC.z;
                    }
                    pointSource += 1;
                    collisionIndex += 1;
                } while (collisionIndex != spB8);
            }
        }
        pointSource = (ControlVector3 *) points;
        if (player->unk173 == 0) {
            func_8001DCD0(actor->rotationX, (ControlVector3 *) &sp110,
                          &spA2, &spA0);
            actor->rotationZ = dAngle(
                actor->rotationZ, spA2,
                1.0f - Powerf(D_80081860, (s32) updateRate));
            actor->rotationY = dAngle(
                actor->rotationY, spA0,
                1.0f - Powerf(D_80081860, (s32) updateRate));
        }
    }
    if (actor->rotationZ >= 0x3001) {
        actor->rotationZ = 0x3000;
    } else if (actor->rotationZ < -0x3000) {
        actor->rotationZ = -0x3000;
    }
    if (actor->rotationY >= 0x3001) {
        actor->rotationY = 0x3000;
    } else if (actor->rotationY < -0x3000) {
        actor->rotationY = -0x3000;
    }
    if (player->unk349 != 0) {
        actor->velocityY = (actor->y - player->unk3C) / updateRate;
    }
    if (player->unk34A == 0) {
        player->unk198 = 0;
    }
    pointIndex = 0;
    pointDest = (u8 *) &player->unk2F0;
    while (pointIndex < spB8) {
        *(f32 *) (pointDest + 0x00) = pointSource->x;
        *(f32 *) (pointDest + 0x04) = pointSource->y;
        *(f32 *) (pointDest + 0x08) = pointSource->z;
        pointDest += 0x0C;
        pointSource += 1;
        pointIndex += 1;
    }
        return spAC;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001DD70.s")
#endif
/* PROVENANCE: JFG's public charControl.c identifies the corresponding
 * controlSquashCheckPrior routine, but publishes assembly only; this body is
 * reconstructed from Mickey's fields, calls, branch conditions, and stores. */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 410 differing words, first mismatch +0x0. */
/* Candidate shape: 412 instructions/frame -0xE0 versus target 416/-0xD0. */
/* Explicit vector locals repair semantics; loop/frame allocation remains.
 * Declaration census, 2026-09-09: nine m2c-only carriers went into the stack
 * slots the target already writes (sp64, sp68/sp6C, sp50) or straight into
 * their uses, taking the frame -0x100 -> -0xE0 at 410 -> 408 words with the
 * instruction count untouched.  The exact target frame -0xD0 is reachable
 * from here -- reading actor->velocityX/Z directly and inlining var_f8 lands
 * it -- but it is NOT the binding constraint: that spelling costs +4 words,
 * 46 more alignment gaps and takes the instruction count from 4 short of the
 * target to 6 short.  A frame that is too LARGE while the instruction count
 * is too SMALL means this candidate homes locals the target does not and
 * misses spills the target has; close the four missing instructions first. */
s32 func_8001E5C4(ControlActor *actor, ControlPlayer *player, f32 updateRate) {
    s16 sp40;
    s16 sp3E;
    s16 sp3C;
    s16 sp3A;
    s16 sp38;
    s32 sp44;
    ControlVector3 spBC;
    ControlVector3 spB0;
    ControlVector3 spA4;
    ControlVector3 sp98;
    f32 sp94;
    f32 sp90;
    f32 sp8C;
    ControlVector3 sp80;
    f32 sp7C;
    f32 sp78;
    f32 sp74;
    f32 sp70;
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    f32 sp58;
    f32 sp54;
    f32 sp50;
    s32 sp2C;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f16_2;
    f32 temp_f2;
    f32 var_f8;
    s32 *var_v1;
    s32 var_a2;
    u32 temp_v0;
    var_v1 = (s32 *) &D_800CB2C0;
    var_a2 = 0xF;
    do {
        *var_v1 = 0;
        var_v1++;
        var_a2--;
    } while (var_a2 != 0);
    pointListRPY(player->unk2BC, (s16 *) actor, player->unk2C0, &spB0.x);
    spBC.x = spB0.x + actor->x;
    spBC.y = spB0.y + actor->y;
    spBC.z = spB0.z + actor->z;
    sp70 = player->unk2B8->w;
    sp2C = (s32) &player->unk2F0;
    trackMakePolylist(1, (ControlVector3 *) sp2C,
                      &spBC, &sp70,
                      player->unk33C, 1);
    temp_v0 = (u32) func_80010900(
        (ControlVector3 *) sp2C, &spBC, sp70,
        (s32) actor, (void *) func_8001EC44);
    actor->x = spBC.x - spB0.x;
    actor->y = spBC.y - spB0.y;
    actor->z = spBC.z - spB0.z;
    sp44 = 0;
    if ((temp_v0 >> 0x1E) != 0) {
        actor->x = player->unk38;
        actor->y = player->unk3C;
        actor->z = player->unk40;
        sp44 = 2;
    } else {
        player->unk349 = 0;
        player->unk34A = 0;
        player->unk34B = 0;
        player->unk18E = 0;
        player->unk334 = 0;
        player->unk344 = 0;
        if ((temp_v0 & 1) != 0) {
            if (D_800CB2FD & 0x12) {
                player->unk349 = 1;
                if ((D_800CB2FD & 0x10) &&
                    (D_800CB2C0.hitObject != 0)) {
                    player->unk334 = D_800CB2C0.hitObject;
                }
            }
            if (D_800CB2FD & 0x48) {
                player->unk34B = (u8) (player->unk34B | 1);
            }
            if (D_800CB2FD & 0x24) {
                sp74 = 0.0f;
                sp78 = 0.0f;
                sp7C = -1.0f;
                sp3E = 0;
                sp40 = 0;
                sp3C = actor->rotationX;
                mathOneFloatRPY((ControlTransform *) &sp3C, &sp74);
                sp64 = sqrtf((D_800CB2D8 * D_800CB2D8) +
                             (D_800CB2D0.x * D_800CB2D0.x));
                sp6C = D_800CB2D0.x / sp64;
                sp68 = D_800CB2D8 / sp64;
                player->unk90 = (sp68 * sp74) -
                                (sp7C * sp6C);
                sp50 = (sp7C * sp68) +
                       (sp74 * sp6C);
                player->unk8C = sp50;
                if (sp50 < 0.0f) {
                    sp50 = -sp50;
                }
                temp_f0_2 = actor->velocityX;
                temp_f2 = actor->velocityZ;
                temp_f0_3 = sqrtf((temp_f0_2 * temp_f0_2) +
                                  (temp_f2 * temp_f2));
                if (temp_f0_3 > 0.0f) {
                    sp58 = temp_f0_2 / temp_f0_3;
                    sp54 = temp_f2 / temp_f0_3;
                }
                temp_f16_2 = (sp6C * sp58) + (sp68 * sp54);
                if ((player->unk198 == 0) && (temp_f0_3 > 8.0f) &&
                    ((temp_f16_2 < D_80081864) ||
                     (D_80081868 < temp_f16_2))) {
                    player->unk78 = 0.0f;
                    player->unk74 = ((2.0f * -temp_f16_2) * sp6C) + sp58;
                    player->unk7C = ((2.0f * -temp_f16_2) * sp68) + sp54;
                    player->unk80 = ((D_8008186C * sp50) + 0.5f) *
                                    temp_f0_3;
                    player->unk84 = player->unk80;
                    player->unk181 = 1;
                    player->unk4 = (f32) (player->unk4 * 0.5f);
                    player->unk88 = D_80081870;
                    player->unk8 = (f32) (player->unk8 * 0.5f);
                } else {
                    var_f8 = (f32) player->unk198;
                    if (var_f8 < 240.0f) {
                        player->unk198 = (u8) (player->unk198 +
                                                (s32) updateRate);
                    } else {
                        player->unk198 = 0;
                        player->unk166 = 1;
                    }
                    sp44 = 1;
                }
                player->unk34A = (u8) (player->unk34A | 1);
            }
        }
        player->unk320 = D_800CB2FC;
        player->unk324 = D_800CB2F8;
        player->unk344 = (s32) (player->unk344 | D_800CB2F8);
    }
    spA4.x = 0.0f;
    spA4.y = 0.0f;
    spA4.z = 0.0f;
    if (player->unk16C != 1) {
        sp98.x = 0.0f;
        sp98.y = -50.0f;
        sp98.z = 0.0f;
        mathOneFloatRPY((ControlTransform *) actor, &sp98.x);
        sp8C = sp98.x + spBC.x;
        sp90 = sp98.y + spBC.y;
        sp64 = 1.0f;
        sp94 = sp98.z + spBC.z;
        if (func_80010654(&spBC, (ControlVector3 *) &sp8C,
                          &sp80, &sp64) != 0) {
            spA4.x += sp80.x;
            spA4.y += sp80.y;
            spA4.z += sp80.z;
        }
    }
    if (player->unk173 == 0) {
        sp2C = (s32) updateRate;
        func_8001DCD0(actor->rotationX, &spA4,
                      &sp3A, &sp38);
        actor->rotationZ = dAngle(
            actor->rotationZ, sp3A,
            1.0f - Powerf(D_80081874, sp2C));
        actor->rotationY = dAngle(
            actor->rotationY, sp38,
            1.0f - Powerf(D_80081878, sp2C));
    }
    if (actor->rotationZ >= 0x3001) {
        actor->rotationZ = 0x3000;
    } else if (actor->rotationZ < -0x3000) {
        actor->rotationZ = -0x3000;
    }
    if (actor->rotationY >= 0x3001) {
        actor->rotationY = 0x3000;
    } else if (actor->rotationY < -0x3000) {
        actor->rotationY = -0x3000;
    }
    if (player->unk349 != 0) {
        actor->velocityY = (actor->y - player->unk3C) / updateRate;
    }
    if (player->unk34A == 0) {
        player->unk198 = 0;
    }
    player->unk2F0 = spBC.x;
    player->unk2F4 = spBC.y;
    player->unk2F8 = spBC.z;
    return sp44;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001E5C4.s")
#endif
/* PROVENANCE -- JFG's public charControl.c identifies the corresponding
 * controlSquashCheckPrior routine, but publishes assembly only; this body is
 * reconstructed from Mickey's fields, calls, branch conditions, and stores. */
#ifdef NON_MATCHING
/* Track B (B3-char): size delta 0 and frame 0xA0 as the target. The reused
 * x/y/z and u/v/w carriers and the in-place "x -= u" updates are what keep
 * u/v/w from being copy-propagated away; see the handoff for what remains. */
void func_8001EC44(s32 arg0, ControlVector3 *pos, ControlVector3 *vel,
                   f32 radius, ControlCollisionPlane *plane) {
    f32 nx, ny, nz;
    f32 x, y, z;
    f32 value;
    f32 vx, vz;
    f32 u, v, w;
    f32 len;
    f32 pad; /* unreferenced: without it the frame is not 0xA0 (L99) */
    f32 angle;

    nx = plane->x;
    ny = plane->y;
    nz = plane->z;
    x = pos->x;
    y = pos->y;
    z = pos->z;
    value = nx * x + ny * y + z * nz + plane->distance;
    if ((D_8008187C <= ny) || (plane->flags & 0x10000000)) {
        vz = vel->z;
        vx = vel->x;
        u = vz * ny;
        v = (nz * vx) - (vz * nx);
        w = -(vx * ny);
        x = (v * nz) - (w * ny);
        v = (u * ny) - (v * nx);
        u = (w * nx) - (u * nz);
        len = (x * x) + (u * u) + (v * v);
        if (D_80081880 < len) {
            len = sqrtf(len);
            value = radius - plane->unk1C;
            pos->x = plane->unk10 + (value * (x / len));
            pos->y = plane->unk14 + (value * (u / len));
            pos->z = plane->unk18 + (value * (v / len));
        } else {
            pos->y = (-(pos->z * nz + nx * pos->x + plane->distance) / ny) + D_80081884;
        }
        D_800CB2C4 = nx;
        D_800CB2C8 = ny;
        D_800CB2CC = nz;
        D_800CB2FD |= 2;
    } else if (ny <= D_80081888) {
        value = D_8008188C - value;
        pos->x = x + (value * nx);
        pos->y = y + (value * ny);
        pos->z = z + (value * nz);
        D_800CB2DC = nx;
        D_800CB2E0 = ny;
        D_800CB2E4 = nz;
        D_800CB2FD |= 8;
    } else {
        value = D_80081890 - value;
        u = x + (value * nx);
        v = y + (value * ny);
        w = z + (value * nz);
        x -= u;
        y -= v;
        z -= w;
        angle = func_8002A8BC(Arctanf(y, sqrtf((x * x) + (z * z))));
        if (angle != 0.0f) {
            value = value / angle;
            len = sqrtf((nx * nx) + (nz * nz));
            pos->x += value * (nx / len);
            pos->z += value * (nz / len);
        } else {
            pos->x = u;
            pos->y = v;
            pos->z = w;
        }
        D_800CB2D0.x = nx;
        D_800CB2D0.y = ny;
        D_800CB2D8 = nz;
        D_800CB2FD |= 4;
    }
    /* Diagnostic stand-in, not the original: the target's procedure has two
     * more uopt basic blocks than this body, which lifts the callee-save toll
     * from 4.75/4.85 to 5.0+ so pos and plane stay in a1/a3 and are saved
     * around the calls, as the target does. This dead if supplies them. */
    if (arg0 == 0x7FFF) { arg0 = 0; }
    D_800CB2F8 = plane->flags;
    D_800CB2FC = plane->kind;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001EC44.s")
#endif
void func_8001EFFC(ControlTransform *transform, ControlPlayer *player, f32 *output) {
    f32 *current;
    s32 index;

    pointListRPY(player->unk2BC, (s16 *) transform, player->unk2C0, output);
    current = output;
    index = 0;
    if (player->unk2BC > 0) {
        do {
            current[0] += transform->x;
            current[1] += transform->y;
            current[2] += transform->z;
            current += 3;
            index++;
        } while (index < player->unk2BC);
    }
}
void func_8001F09C(ControlPlayer *player, s32 updateRate) {
    f32 rate;

    rate = (f32) updateRate;
    if (player->unk50 < player->unk54) {
        player->unk50 += ((player->unk54 - player->unk50) * 0.125f * rate) + D_80081894;
        if (player->unk54 <= player->unk50) {
            player->unk50 = player->unk54;
        }
    } else {
        player->unk50 += ((player->unk54 - player->unk50) * 0.125f * rate) - D_80081898;
        if (player->unk50 <= player->unk54) {
            player->unk50 = player->unk54;
        }
    }
}
void func_8001F14C(ControlTransform *transform, ControlCeilingContext *context) {
    register ControlSpawned *spawned;
    ControlSpawnPacket packet;
    f32 offset[3];
    f32 x;
    f32 y;
    f32 z;

    offset[0] = 0.0f;
    offset[1] = 0.0f;
    offset[2] = 10.0f;
    mathOneFloatRPY(transform, offset);
    x = offset[0] + transform->x;
    y = context->height;
    z = offset[2] + transform->z;
    packet.kind = 0x157;
    packet.mode = 0xC;
    packet.flags = 0;
    packet.x = (s16) x;
    packet.y = (s16) y;
    packet.z = (s16) z;
    packet.unkA = mathRnd(-0x7FFF, 0x7FFF);
    spawned = func_8000590C(&packet, 1);
    if (spawned != 0) {
        spawned->unk3C = 0;
    }
    if (context->handle != 0) {
        func_800031E8(context->handle);
    }
    func_80002FE0(0x329, x, y, z, 4, &context->handle);
}
/*
 * PROVENANCE -- JFG's src/charControl.c supplied the controlDisableJoypad
 * name/role. Mickey's two-argument field store independently determines this
 * per-player body and differs from JFG's one-argument global implementation.
 */
void controlDisableJoypad(ControlPlayer *player, s32 disabled) {
    player->joypadDisabled = disabled;
}
/* PROVENANCE -- adapted from JFG's src/charControl.c controlReadJoypad. */
void controlReadJoypad(ControlPlayer *player, s32 playerIndex) {
    if ((playerIndex >= 0) && (playerIndex < 4) && (player->joypadDisabled == 0)) {
        player->controlXjoy = joyGetStickX(playerIndex);
        player->controlAbsXjoy = joyGetAbsX(playerIndex);
        player->controlYjoy = joyGetStickY(playerIndex);
        player->controlAbsYjoy = joyGetAbsY(playerIndex);
        player->controlKeys = joyGetButtons(playerIndex);
        player->controlDkeys = joyGetPressed(playerIndex);
        player->controlReleasedKeys = joyGetReleased(playerIndex);
    } else {
        player->controlXjoy = 0;
        player->controlAbsXjoy = 0;
        player->controlYjoy = 0;
        player->controlAbsYjoy = 0;
        player->controlKeys = 0;
        player->controlDkeys = 0;
        player->controlReleasedKeys = 0;
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlSetRumble
 * name/role. Mickey's smaller wrapper independently determines this body.
 */
void controlSetRumble(ControlPlayer *player, s32 strength, f32 duration) {
    if ((player->unk191 == 0) && !(player->flags1A8 & 1)) {
        rumbleStart(player->playerIndex, strength, duration);
    }
}
void func_8001F364(void) {
}
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800CB470 = arg0;
    D_800CB472 = arg1;
    D_800CB474 = arg2;
    D_800CB476 = arg3;
    D_80079BF8 = 1;
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlGetPlayerSetup
 * name/role. This body is reconstructed from Mickey's setup-state accesses.
 */
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3) {
    if (D_80079BF8 != 0) {
        *arg0 = D_800CB470;
        *arg1 = D_800CB472;
        *arg2 = D_800CB474;
        *arg3 = D_800CB476;
        D_80079BF8 = 0;
        return 1;
    }
    return 0;
}

/* PROVENANCE -- adapted from JFG's src/charControl.c controlClearPlayerSetup. */
void controlClearPlayerSetup(void) {
    D_80079BF8 = 0;
}


/* PLATEAU-HANDOFF:func_8001EC44:start
 * symbol: func_8001EC44
 * score: 224 differing words
 * frame: 0xA0
 * relocations: 43
 * first-mismatch: +0xC
 * summary: Delta 0, frame 0xA0. Blocker: callee toll 4.75/4.85 vs caller 5.0 puts pos/plane in s0/s1; target has 2 more uopt blocks.
 * PLATEAU-HANDOFF:func_8001EC44:end
 */

/* PLATEAU-HANDOFF:func_8001E5C4:start
 * symbol: func_8001E5C4
 * score: 408/416 words
 * frame: 0xE0
 * relocations: 53
 * first-mismatch: +0x0
 * summary: Nine m2c-only carriers folded into the stack slots the target already writes took the frame 0x100 -> 0xE0 at an unchanged instruction count. The exact 0xD0 frame is reachable but costs +4 words and 46 more gaps, so the frame is not the binding constraint -- the candidate is four instructions SHORT of the target while its frame is too large, which means missing spills. Close the instruction count first.
 * PLATEAU-HANDOFF:func_8001E5C4:end
 */


/* PLATEAU-HANDOFF:func_8001C4C0:start
 * symbol: func_8001C4C0
 * score: 386/403 words
 * frame: 0xB0
 * relocations: 40
 * first-mismatch: +0x0
 * summary: Identity-gated proc 10: two type-2 webs split at totalsave 10 and 11 vs callee cost 16.25 so s5/s6 never allocate. L99 unused pointer is not the save area.
 * PLATEAU-HANDOFF:func_8001C4C0:end
 */

/* PLATEAU-HANDOFF:func_8001D880:start
 * symbol: func_8001D880
 * score: 28/36 words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x4
 * summary: Indexing the table closed the address operand order (a2,t7 both sides) and naming the upper table entry in `value` before the lerp takes it to 28. The residual is a uopt colour rotation: the target colours only 10.0f from {f16,f18} and runs a five-wide FP ring, this candidate colours both and runs four. Twenty-five source forms plus 210 permuter candidates were flat or worse.
 * PLATEAU-HANDOFF:func_8001D880:end
 */

/* PLATEAU-HANDOFF:func_8001DD70:start
 * symbol: func_8001DD70
 * score: 299/533 words
 * frame: 0x268
 * relocations: 23
 * first-mismatch: +0x54
 * summary: tools/frame_census.py named the cause the frame total hid: 19 slots only this candidate used and 24 only the target did, at an already-exact 0x268. Declaring to the target's ladder closed the home set -- one flat list in the target's order, records before points before radius, the spE0/sp104/spA4 triples spelled as single objects so uopt cannot constant-propagate the members whose address never escapes, spEC left as three separate locals so its tail members still hoist out of the accumulate loop -- and with record = records moved into the loop preheader (L110) and both != spB8 guards respelled < spB8, 433 falls to 299 with byte-exact 146 -> 390 and really different 131 -> 57. Only two compiler temps differ now, 0x7C against 0x84 and 0x8C. Rejected with measurements: one shared loop index is -40 bytes and 288 byte-exact, so the target has at least two; hoisting either index reset to the previous loop's exit is +4 words.
 * PLATEAU-HANDOFF:func_8001DD70:end
 */
