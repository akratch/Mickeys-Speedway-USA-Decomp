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
/* charControl's BSS, in target address order (IDO lays .bss out in declaration
 * order): the collision callback state at 0x800CB2C0, the camera state pointer
 * at 0x800CB300, and the shared gravity scalar at 0x800CB304. The collision
 * state must be defined here: as1 shares one high half per aligned pair only
 * for a symbol its TU defines, which is the target's form in func_8001EC44. */
ControlCollisionState D_800CB2C0;
ControlCameraState *D_800CB300;
/* Shared gravity scalar; original resident/overlay uses are single precision. */
f32 D_800CB304;

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
/* Returns int: func_8001EC44 narrows the result itself before the call that
 * consumes it, and func_8001DCD0 stores it through s16 pointers. */
s32 Arctanf(f32 x, f32 y);
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
void func_8001BBB4(ControlActor *actor, ControlPlayer *player, f32 updateRate);
void func_8001C114(s32 slotIndex, f32 x, f32 y, f32 z);
void *func_80053420(s32 index, void *target);
void func_80024ED8();
s32 func_8003A550(void);
s32 func_8000FBD8(s32 segmentIndex, f32 x, f32 y, f32 z);
void fxMakeConeTextureCoords(void *cone, s16 angle);
void fxMakeConeLength(void *cone, s16 angle, f32 x, f32 y, s32 length);
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
void *fxAllocateCone(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
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
void func_8001EC44(s32 unused, ControlVector3 *pos, ControlVector3 *vel,
                   f32 radius, ControlCollisionPlane *plane);
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
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081840.s")

void func_8001BBB4(ControlActor *actor, ControlPlayer *player, f32 updateRate) {
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
                TrapDanglingJump(actor, player, D_800CB300, (s32) updateRate);
            } else if (player->controlKeys & 4) {
                func_80024ED8(actor, player, D_800CB300);
            } else if (D_8007BF10 != 0) {
                TrapDanglingJump(D_800CB300, actor, *(s32 *) &updateRate);
            } else {
                TrapDanglingJump(D_800CB300, actor, *(s32 *) &updateRate);
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
void controlPlayerReInit(ControlActor *actor, f32 x, f32 y, f32 z, s16 rotationX, s16 rotationY, s16 rotationZ) {
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
    state->arg4 = rotationX;
    state->arg5 = rotationY;
    state->arg6 = rotationZ;
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
/* PROVENANCE: JFG's corresponding character-control initialization role supplied the control-flow lead; fields and body are reconstructed from Mickey. */
/* Matched (was 121 masked words). The target reuses one i and one j across
 * the loops, which is what puts player and actor in s5/s6. On top of that:
 * the character model's first pointer is read through stateCursor before the
 * character data (the target holds it in v0, which removes a ring draw); the
 * point copy starts its byte offset inside the guard with i cleared outside;
 * the slot byte is `i << 3`, which uopt does not strength-reduce; and
 * unk10, unkFE, unk4C, unk3BA and the spawn packet's owner are written in
 * the target's store order, with `i = 0` folded onto the line its tie needs. */
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081844.s")

void func_8001C4C0(ControlActor *actor, ControlPlayerInitState *state, s32 mode) {
    ControlPlayer *player;
    CharControlEffectDefinition *effect;
    s32 effectIndex;
    CharControlParticleDefinition *particle;
    CharControlCharacterData *characterData;
    CharControlParticleSlot *slot;
    f32 *output;
    s16 *position;
    CharControlSpawnSetup packet;
    void **effectOwner;
    void *stateCursor;
    s32 pointIndex;
    s32 i;
    s32 j;

    player = actor->player;
    player->unk1B8 = 0x2000;
    player->playerIndex = *((u8 *) state + 0x10);
    player->unk1 = *((u8 *) state + 0x11);
    player->unk10 = 0.0f;
    actor->rotationX = state->arg4;
    actor->rotationY = state->arg5;
    actor->rotationZ = state->arg6;
    player->unkF0 = actor->rotationX;
    player->unkF2 = actor->rotationY;
    player->unkF4 = actor->rotationZ;
    player->unkFE = 0;
    player->unkDC = (s16) (0x8000 - actor->rotationX);
    func_8005AD64(actor, 0, -1, 0.0f);
    player->unk50 = 1.0f;
    player->unk54 = 1.0f;

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
    output = &player->unk2C0[0];
    i = 0;
    if (player->unk2BC > 0) {
        pointIndex = 0;
        do {
            i++;
            output += 3;
            output[-3] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex));
            output[-2] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex + 4));
            output[-1] = *((f32 *) ((u8 *) player->unk2B8 + pointIndex + 8));
            pointIndex += 0x10;
        } while (i < player->unk2BC);
    }
    func_8001EFFC(actor, player, &player->unk2F0);

    effectIndex = player->unk1;
    if ((effectIndex < 0) || (effectIndex >= 10)) {
        effectIndex = 0;
    }
    if (player->playerIndex < (D_8007BEF8 - D_8007BEFC)) {
        effect = D_8007980C[effectIndex].entries;
        if (effect != 0) {
            i = D_8007980C[effectIndex].count;
            j = 0;
            effectOwner = (void **) player;
            if (i > 0) {
                do {
                    if (effectOwner[0x134 / 4] == 0) {
                        effectOwner[0x134 / 4] =
                            charControlEffectSpawnTrap(
                                actor, effect->kind, effect->angle, effect->index,
                                effect->x, effect->y, effect->z, effect->w,
                                effect->arg14, effect->arg15, effect->arg16,
                                effect->arg17);
                    }
                    j++;
                    effectOwner++;
                    effect++;
                } while (j != i);
            }
        }
    }

    stateCursor = actor->unk68[actor->unk3A];
    characterData = (CharControlCharacterData *) *(void **) stateCursor;
    i = 0; if (levelGetType() == 3) {
        j = D_8007987C[effectIndex].count;
        particle = D_8007987C[effectIndex].entries;
    } else if (D_8007BF04 != 0) {
        j = D_800798DC[effectIndex].count;
        particle = D_800798DC[effectIndex].entries;
    } else {
        j = D_8007987C[effectIndex].count;
        particle = D_8007987C[effectIndex].entries;
    }
    slot = (CharControlParticleSlot *) player->particles;
    while (j--) {
        {
            if (slot->handle == 0) {
                if (particle->index < characterData->count) {
                    position = (s16 *) ((u8 *) characterData->positions +
                        (characterData->indexTable[particle->index].offset * 10));
                    slot->kind = particle->kind;
                    slot->index = particle->index;
                    slot->model = (s8)
                        characterData->indexTable[particle->index].value;
                    slot->handle = fxAllocateCone(
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
            i++;
        }
    }
    while (i < 4) {
        i++;
        slot->handle = 0;
        slot++;
    }

    player->unk38 = actor->x;
    player->unk3C = actor->y;
    player->unk40 = actor->z;
    player->unk44 = actor->x;
    player->unk48 = actor->y;
    player->unk4C = actor->z;
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
    i = 0;
    if (player->playerIndex != -1) {
        camSetNo(player->playerIndex);
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
    player->unk17C = 1.0;
    do {
        stateCursor = (u8 *) stateCursor + 1;
        *((u8 *) stateCursor + 0x12B) = 0;
        *((u8 *) stateCursor + 0x12F) = i << 3;
        i++;
    } while (i < 4);
    player->unk3EC = 0.0f;
    player->unk3F0 = D_80081844;
    player->unk43C = actor->rotationX;
    player->unk43E = actor->rotationY;
    player->unk440 = actor->rotationZ;
    player->unk444 = actor->unk8;
    player->unk448 = actor->x;
    player->unk44C = actor->y;
    player->unk450 = actor->z;
    player->unk3BA = 0xFF;
    if (levelGetType() == 3) {
        if (mode != 0) {
            packet.owner = actor;
            packet.kind = 0x124; i = 0;
            packet.arg04 = 0;
            packet.arg06 = 0;
            packet.arg08 = 0;
            do {
                packet.arg0A = i;
                func_8000590C((ControlSpawnPacket *) &packet, 1);
                i++;
            } while (i != 3);
        }
    }
    if ((D_8007BF1C & 2) && (*func_80028F54() != 1)) {
        player->unk192 = 0xA;
    } else {
        player->unk192 = 0;
    }
}
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
 * compiler-temp region below them.  decayFactor/levelInfo/character keep their positions
 * because those are the target's own displacements. */
/* bounce is ONE scratch float reused twice, and that is what closed this
 * function.  Naming the ballistic step keeps it in a coloured web where the
 * inline expression spent floating-point ring temps, which cost 40 register
 * words; declaring a *new* local for it instead of reusing bounce pushed
 * every compiler temp 4 bytes down the frame and cost 25 displacement words.
 * Both halves are needed: 53 -> 39 -> 0. */
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081848.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_8008184C.s")

void func_8001CB84(ControlActor *actor, s32 updateRate) {
    f32 decayFactor;
    f32 bounce;
    f32 tiltCos;
    CharControlLevelDescription *levelInfo;
    ControlPlayer *player;
    CharControlSpawnSetup packetD0;
    CharControlSpawnSetup packetD8;
    s32 fade;
    s32 character;

    player = actor->player;
    player->unk3 = player->unk2;
    if (player->unkC8 != NULL) {
        levelInfo = ((CharControlLevelRequest *) player->unkC8)->description;
        character = levelInfo->characterLow;
        if (levelInfo->characterHigh != 0xFF) {
            character |= levelInfo->characterHigh << 8;
        }
        mainChangeLevel(character, mainGetNextCharacter(),
                        levelInfo->nextLevel, frontGetMode(),
                        levelInfo->camera, 0);
        if (levelInfo->animGroup != -1) {
            mainSetAnimGroup(levelInfo->animGroup);
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
        bounce = (player->unk150 * (f32) updateRate) -
                  (0.5f * D_800CB304 * (f32) updateRate * (f32) updateRate);
        player->unk154 = player->unk154 + bounce;
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
            decayFactor = Powerf(D_80081848, updateRate);
            player->unk160 = dAngle(player->unk160, 0, decayFactor);
            player->unk164 = dAngle(player->unk164, 0, decayFactor);
            player->unk162 = dAngle(player->unk162, 0, decayFactor);
        } else if (player->unk158 == 0) {
            player->unk160 = 0;
            player->unk164 = 0;
            player->unk162 = 0;
            player->unk154 = 0.0f;
            player->unk150 = 0.0f;
        }
        tiltCos = func_8002A8BC(player->unk162) * func_8002A8BC(player->unk164);
        if (tiltCos < 0.0f) {
            bounce = 0.0f;
        } else {
            bounce = tiltCos * tiltCos;
        }
        player->unk14C = 30.0f - (30.0f * bounce);
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
            fade = player->unk190 - (updateRate * 4);
            if (fade <= 0) {
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
                player->unk190 = (u8) fade;
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
void func_8001D2A0(ControlActor *actor, s32 updateRate)
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
    TrapDanglingJump(actor, player, updateRate);
    D_800CB300 = D_800CB300;
  }
  if (player->unkD4 != 0)
  {
    TrapDanglingJump(player->unkD4, updateRate);
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
    func_8001BBB4(actor, player, (f32) updateRate);
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
s16 dAngle(s16 angle, s16 target, f32 factor) {
    s32 backward;
    s32 forward;

    forward = (target - angle) & 0xFFFF;
    backward = (angle - target) & 0xFFFF;
    if (backward < forward) {
        forward = -backward;
    }
    return (s16) (angle + (s32) ((f32) forward * factor));
}
/* PROVENANCE -- role and signature follow JFG's charControl controlMakeV,
 * which the donor carries as assembly only; the body is written from
 * Mickey's listing. Matched with one reused index, each argument reduced to
 * its fraction in place, the table read by subscript, and the difference
 * accumulated into `v`. */
f32 func_8001D880(f32 valueA, f32 valueB, f32 *table, f32 divisor) {
    s32 i;
    f32 base;
    f32 v;

    valueB *= 10.0f;
    i = valueB;
    valueB -= i;
    base = table[i];
    v = (table[i + 1] - base) * valueB + base;
    valueA *= 10.0f;
    i = valueA;
    valueA -= i;
    base = table[i];
    v -= base + (table[i + 1] - base) * valueA;
    return v / divisor;
}
/* PROVENANCE -- adapted from JFG's src/charControl.c controlFSUvels. */
void controlFSUvels(s16 *rotation, ControlPlayer *player) {
    s16 angles[3];

    angles[0] = rotation[0];
    angles[1] = rotation[1];
    angles[2] = 0;
    pointListRPY(3, angles, D_80079BD4, player->unk14);
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
 *    already in size; that CSE is what makes IDO keep the loaded value in a
 *    caller-saved register and copy it into the saved one,
 *  - case 2 spells the scaled angle inline, exactly as case 3 does. Naming the
 *    call result in a local made it a uopt-coloured web where the target pops a
 *    ugen ring temp, and that one class crossing rotated twelve downstream webs,
 *  - case 2 performs actor->unk80 |= triggerFlags AFTER the whole size product, so
 *    the product's four ring temps are drawn before the or's two loads.
 * The last two are one composition: neither alone is an improvement. */
/* PROVENANCE: JFG's public controlUpdateJetFlames role and Mickey's m2c/assembly establish
 * the state-machine order; no external body is copied into this reconstruction. */
void func_8001D960(ControlActor *actor, ControlPlayer *player, s32 triggerFlags, s32 spawnState,
                   s32 updateRate) {
    s32 slotIndex;
    ControlFlameSlot *slot;
    void *particle;
    s32 slotMask;
    s32 intensity;
    s32 phase;
    s32 size;
    f32 scaleX;
    f32 scaleY;

    slotIndex = 0;
    slotMask = 1;
    slot = (ControlFlameSlot *) ((u8 *) player + 0x34C);
    do {
        particle = slot->particle;
        if (particle != NULL) {
            size = *(s16 *) ((u8 *) particle + 0x24);
            intensity = slot->intensity;
            phase = slot->phase;
            scaleX = *(f32 *) ((u8 *) particle + 0x18);
            scaleY = *(f32 *) ((u8 *) particle + 0x1C);
            if (slot->state == 2) {
                if (slot->mode == 0) {
                    intensity -= updateRate << 5;
                    if (intensity < 0) {
                        intensity = 0;
                    }
                    phase = actor->rotationX;
                    if (player->unk186 & slotMask) {
                        slot->mode = 2;
                    }
                } else {
                    intensity += updateRate << 5;
                    if (intensity >= 0x100) {
                        intensity = 0xFF;
                    }
                    phase = actor->rotationX;
                    if (!(player->unk186 & slotMask)) {
                        slot->mode = 0;
                    }
                }
                scaleX *= (f32) intensity / 255.0f;
                scaleY *= (f32) intensity / 255.0f;
                if (actor->unk70 != NULL) {
                    s32 lightHandle;

                    lightHandle = *actor->unk70;
                    if (lightHandle != 0) {
                        changeLightIntensity((void *) lightHandle, intensity);
                    }
                }
            } else {
                switch (slot->mode) {
                case 0:
                    intensity = 0;
                    if (player->unk186 & slotMask) {
                        s32 savedState;

                        savedState = actor->unk80;
                        actor->unk80 = spawnState;
                        partUpdateTriggers(actor, 2);
                        actor->unk80 = savedState;
                        slot->mode = 1;
                    }
                    break;
                case 1:
                    intensity += updateRate << 5;
                    size = (s32) (*(s16 *) ((u8 *) particle + 0x24) * intensity) >> 7;
                    if (intensity >= 0x100) {
                        intensity = 0xFF;
                        if (player->unk186 & slotMask) {
                            slot->mode = 2;
                        } else {
                            slot->mode = 3;
                        }
                    }
                    break;
                case 2:
                    intensity += updateRate * 0x10;
                    if (intensity >= 0x100) {
                        intensity = 0xFF;
                    }
                    phase += updateRate << 0xC;
                    size = (s32) ((func_8002A204((s16) (phase << 8)) + 0x18000) *
                                    ((s32) (size * intensity) >> 8)) >> 0x10;
                    actor->unk80 |= triggerFlags;
                    if (!(player->unk186 & slotMask)) {
                        slot->mode = 3;
                    }
                    break;
                case 3:
                    intensity -= updateRate * 8;
                    if (intensity <= 0) {
                        intensity = 0;
                        slot->mode = 0;
                    } else {
                        phase += updateRate << 0xC;
                        size = (s32) ((func_8002A204((s16) (phase << 8)) + 0x18000) *
                                        ((s32) (size * intensity) >> 8)) >> 0x10;
                        if (player->unk186 & slotMask) {
                            slot->mode = 2;
                        }
                    }
                    break;
                }
            }
            slot->intensity = intensity;
            slot->phase = phase;
            if (intensity != 0) {
                fxMakeConeTextureCoords(particle, slot->phase);
                fxMakeConeLength(particle, size, scaleX, scaleY, slot->intensity);
            }
        }
        slotMask *= 2;
        slot++;
        slotIndex++;
    } while (slotIndex != 4);
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
s16 dAngle(s16 angle, s16 target, f32 factor);
void func_8001DCD0(s16 rotation, ControlVector3 *vector, s16 *pitch, s16 *yaw);
/* PROVENANCE: JFG's public charControl.c and hit.c identify the related
 * ground-hit and polygon-edge control family, but publish assembly only;
 * this body is reconstructed from Mickey's collision records and fields. */
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081850.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081854.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081858.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_8008185C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081860.s")

/* Matched 2026-10-07 (lane c-res) from the natural rewrite (lane a-char, 224
 * -> 150). What closed it: `bit` as a u8 (the target splits it across the
 * 0x24 block's calls and spills it to a compiler temp); vec, acc, down, end
 * and hit as f32[3] arrays (no dot copy); no `mask` or `scale` locals (the
 * collision result is masked and shifted in place), which with `bit`
 * declared beside `count` gives the target's homes and temps; nx divided
 * before nz and the dot nx-term first; the halving tail as unk88, unk181,
 * unk4, unk8; `hitResult |= 1` inside the non-bounce arm (the bounce arm
 * branches past it), which also stops the hit address being hoisted into s4;
 * and `(s32) timer + (s32) updateRate` for the add's operand order. The
 * float cells above are this function's literal pool; written as literals
 * the compiler would place them after every GLOBAL_ASM cell of the TU, so
 * they stay externs. That costs two things the literals gave for free: the
 * bounce scale is held in `speed` (as in func_8001E5C4) so the three tied
 * float webs keep f2/f12/f14, and the 0.9 cell is read once into `d` so it is
 * held in f22 across the two Powerf calls as the hoisted literal was. */
s32 func_8001DD70(ControlActor *actor, ControlPlayer *player, f32 updateRate) {
    CharControlGroundRecord *record;
    s32 i;
    ControlVector3 *p;
    CharControlGroundRecord records[4];
    ControlVector3 points[4];
    f32 radius[4];
    f32 acc[3];
    f32 down[3];
    f32 end[3];
    f32 hit[3];
    f32 vec[3];
    u32 result;
    f32 speed;
    f32 dist;
    f32 nz;
    f32 nx;
    f32 dirX;
    f32 dirZ;
    f32 dot;
    f32 d;
    s32 count;
    u8 bit;
    u32 timer;
    s32 hitResult;
    s16 rot[4];
    s16 pitch;
    s16 yaw;
    ControlVector3 *dst;

    func_8001EFFC((ControlTransform *) actor, player, &points[0].x);
    count = player->unk2BC;
    for (i = 0; i < count; i++) {
        radius[i] = player->unk2B8[i].w;
    }
    trackMakePolylist(count, (ControlVector3 *) &player->unk2F0, points, radius, player->unk33C, 1);
    result = func_80010B4C(count, &player->unk2F0, points, radius, records, &actor->x, actor);
    hitResult = 0;
    if ((result >> 30) != 0) {
        actor->x = player->unk38;
        actor->y = player->unk3C;
        actor->z = player->unk40;
        hitResult = 2;
    } else {
        player->unk349 = 0;
        player->unk34A = 0;
        player->unk34B = 0;
        player->unk18E = 0;
        player->unk334 = NULL;
        player->unk344 = 0;
        bit = 1;
        result &= 0x3FFFFFFF;
        for (i = 0; i < count; i++) {
            record = &records[i];
            if (result & 1) {
                if (record->unk3D & 0x12) {
                    player->unk349 |= bit;
                    if ((record->unk3D & 0x10) && (record->hitObject != NULL)) {
                        player->unk334 = record->hitObject;
                    }
                }
                if (record->unk3D & 0x48) {
                    player->unk34B |= bit;
                }
                if (record->unk3D & 0x24) {
                    vec[0] = 0.0f;
                    vec[1] = 0.0f;
                    vec[2] = -1.0f;
                    rot[0] = actor->rotationX;
                    rot[1] = 0;
                    rot[2] = 0;
                    mathOneFloatRPY((ControlTransform *) rot, vec);
                    dist = sqrtf(record->unk18 * record->unk18 + record->unk10 * record->unk10);
                    nx = record->unk10 / dist;
                    nz = record->unk18 / dist;
                    player->unk90 = nz * vec[0] - vec[2] * nx;
                    dot = vec[0] * nx + vec[2] * nz;
                    player->unk8C = dot;
                    if (dot < 0.0f) {
                        dot = -dot;
                    }
                    speed = sqrtf(actor->velocityX * actor->velocityX + actor->velocityZ * actor->velocityZ);
                    if (speed > 0.0f) {
                        dirX = actor->velocityX / speed;
                        dirZ = actor->velocityZ / speed;
                    }
                    timer = player->unk198;
                    if (timer == 0 && speed > 8.0f &&
                        ((d = nx * dirX + nz * dirZ) < D_80081850 || D_80081854 < d)) {
                        player->unk74 = 2.0f * -d * nx + dirX;
                        player->unk78 = 0.0f;
                        player->unk7C = 2.0f * -d * nz + dirZ;
                        speed = (D_80081858 * dot + 0.5f) * speed;
                        player->unk80 = speed;
                        player->unk84 = speed;
                        player->unk88 = D_8008185C;
                        player->unk181 = 1;
                        player->unk4 *= 0.5f;
                        player->unk8 *= 0.5f;
                    } else {
                        if (player->flags1A8 & 1) {
                            if ((f32) timer < 240.0f) {
                                player->unk198 = (s32) timer + (s32) updateRate;
                            } else {
                                player->unk198 = 0;
                                player->unk166 = 1;
                            }
                        } else {
                            player->unk198 = 1;
                        }
                        hitResult |= 1;
                    }
                    player->unk34A |= bit;
                }
            }
            (&player->unk320)[i] = record->unk3C;
            (&player->unk324)[i] = record->unk38;
            player->unk344 |= record->unk38;
            bit <<= 1;
            result >>= 1;
        }
    }
    acc[0] = 0.0f;
    acc[1] = 0.0f;
    acc[2] = 0.0f;
    if (player->unk16C != 1) {
        down[0] = 0.0f;
        down[2] = 0.0f;
        down[1] = -50.0f;
        mathOneFloatRPY((ControlTransform *) actor, down);
        p = points;
        for (i = 0; i < count; i++) {
            end[0] = down[0] + p->x;
            end[1] = down[1] + p->y;
            end[2] = down[2] + p->z;
            dist = 1.0f;
            if (func_80010654(p, (ControlVector3 *) end, (ControlVector3 *) hit, &dist) != 0) {
                acc[0] += hit[0];
                acc[1] += hit[1];
                acc[2] += hit[2];
            }
            p++;
        }
    }
    p = points;
    if (player->unk173 == 0) {
        func_8001DCD0(actor->rotationX, (ControlVector3 *) acc, &pitch, &yaw);
        d = D_80081860;
        actor->rotationZ = dAngle(actor->rotationZ, pitch, 1.0f - Powerf(d, (s32) updateRate));
        actor->rotationY = dAngle(actor->rotationY, yaw, 1.0f - Powerf(d, (s32) updateRate));
    }
    if (actor->rotationZ > 0x3000) {
        actor->rotationZ = 0x3000;
    } else if (actor->rotationZ < -0x3000) {
        actor->rotationZ = -0x3000;
    }
    if (actor->rotationY > 0x3000) {
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
    dst = (ControlVector3 *) &player->unk2F0;
    for (i = 0; i < count; i++) {
        dst->x = p->x;
        dst->y = p->y;
        dst->z = p->z;
        dst++;
        p++;
    }
    return hitResult;
}
/* PROVENANCE: JFG's public charControl.c identifies the corresponding
 * controlSquashCheckPrior routine, but publishes assembly only; this body is
 * reconstructed from Mickey's fields, calls, branch conditions, and stores. */
#ifndef NON_MATCHING
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081864.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081868.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_8008186C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081870.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081874.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081878.s")
#endif

/* Rewritten from the listing (lane z-fxchar, 2026-10-02): 408 at -16 to 157
 * at size delta 0, frame 0xD0 as the target. What moved it: the sixteen-word
 * clear as `while (n--)`; the collision state read through a pointer local
 * taken at entry (each read is then a by-name load, re-read after every
 * store through `player`); the three flag bytes set by OR; `(u8) flags & 1`,
 * whose deleted mask is the ring draw the target spends; `!player->unk198`,
 * which keeps the count in one register; and the six scalar locals placed in
 * the six cells the target leaves unused. What remains is in the handoff.
 * 2026-10-07, lane a-char: the scale written to unk80 and copied to unk84
 * from the field (157 -> 93 at delta 0); the dot web then splits at the
 * speed sqrtf as shipped.
 * Later the same day (resumed): forward and offset as f32[3] arrays, which
 * removes the dot copy (the target computes the dot straight into its
 * symbol), the dot written nx-term first, nz divided after nx, unk78 stored
 * after unk74, and the halving tail as unk88, unk181, unk4, unk8 (93 -> 8).
 * Matched 2026-10-07 by holding the bounce scale in `speed` (reused): the
 * scale web then numbers after -side and 2*-side, so the three tied float
 * webs take f2, f12, f14 as shipped. */
s32 func_8001E5C4(ControlActor *actor, ControlPlayer *player, f32 updateRate) {
    ControlCollisionState *state = &D_800CB2C0;
    s32 *p;
    ControlVector3 pos;
    f32 offset[3];
    ControlVector3 sum;
    ControlVector3 down;
    ControlVector3 end;
    ControlVector3 hit;
    f32 forward[3];
    f32 radius;
    f32 nx;
    f32 nz;
    f32 len;
    s32 n;
    u32 flags;
    f32 vxn;
    f32 vzn;
    f32 dot;
    f32 speed;
    f32 side;
    s32 result;
    s16 rot[3];
    s16 pitch;
    s16 yaw;

    p = (s32 *) state;
    n = 16;
    while (n--) {
        *p++ = 0;
    }
    pointListRPY(player->unk2BC, (s16 *) actor, player->unk2C0, offset);
    pos.x = offset[0] + actor->x;
    pos.y = offset[1] + actor->y;
    pos.z = offset[2] + actor->z;
    radius = player->unk2B8->w;
    trackMakePolylist(1, (ControlVector3 *) &player->unk2F0, &pos, &radius,
                      player->unk33C, 1);
    flags = (u32) func_80010900((ControlVector3 *) &player->unk2F0, &pos,
                                radius, (s32) actor, (void *) func_8001EC44);
    actor->x = pos.x - offset[0];
    actor->y = pos.y - offset[1];
    actor->z = pos.z - offset[2];
    result = 0;
    if ((flags >> 0x1E) != 0) {
        actor->x = player->unk38;
        actor->y = player->unk3C;
        actor->z = player->unk40;
        result = 2;
    } else {
        player->unk349 = 0;
        player->unk34A = 0;
        player->unk34B = 0;
        player->unk18E = 0;
        player->unk334 = 0;
        player->unk344 = 0;
        if ((u8) flags & 1) {
            if (state->state & 0x12) {
                player->unk349 |= 1;
                if ((state->state & 0x10) && (state->hitObject != NULL)) {
                    player->unk334 = (s32) state->hitObject;
                }
            }
            if (state->state & 0x48) {
                player->unk34B |= 1;
            }
            if (state->state & 0x24) {
                forward[0] = 0.0f;
                forward[1] = 0.0f;
                forward[2] = -1.0f;
                rot[0] = actor->rotationX;
                rot[1] = 0;
                rot[2] = 0;
                mathOneFloatRPY((ControlTransform *) rot, forward);
                len = sqrtf((state->unk18 * state->unk18) +
                            (state->unk10 * state->unk10));
                nx = state->unk10 / len;
                nz = state->unk18 / len;
                player->unk90 = (nz * forward[0]) - (forward[2] * nx);
                dot = (forward[0] * nx) + (forward[2] * nz);
                player->unk8C = dot;
                if (dot < 0.0f) {
                    dot = -dot;
                }
                speed = sqrtf((actor->velocityX * actor->velocityX) +
                              (actor->velocityZ * actor->velocityZ));
                if (speed > 0.0f) {
                    vxn = actor->velocityX / speed;
                    vzn = actor->velocityZ / speed;
                }
                side = (nx * vxn) + (nz * vzn);
                if (!player->unk198 && (speed > 8.0f) &&
                    ((side < D_80081864) || (D_80081868 < side))) {
                    player->unk74 = ((2.0f * -side) * nx) + vxn;
                    player->unk78 = 0.0f;
                    player->unk7C = ((2.0f * -side) * nz) + vzn;
                    speed = ((D_8008186C * dot) + 0.5f) * speed;
                    player->unk80 = speed;
                    player->unk84 = speed;
                    player->unk88 = D_80081870;
                    player->unk181 = 1;
                    player->unk4 *= 0.5f;
                    player->unk8 *= 0.5f;
                } else {
                    if ((f32) player->unk198 < 240.0f) {
                        player->unk198 += (s32) updateRate;
                    } else {
                        player->unk198 = 0;
                        player->unk166 = 1;
                    }
                    result = 1;
                }
                player->unk34A |= 1;
            }
        }
        player->unk320 = state->mode;
        player->unk324 = state->flags;
        player->unk344 |= state->flags;
    }
    sum.x = 0.0f;
    sum.y = 0.0f;
    sum.z = 0.0f;
    if (player->unk16C != 1) {
        down.x = 0.0f;
        down.y = -50.0f;
        down.z = 0.0f;
        mathOneFloatRPY((ControlTransform *) actor, &down.x);
        end.x = down.x + pos.x;
        end.y = down.y + pos.y;
        end.z = down.z + pos.z;
        len = 1.0f;
        if (func_80010654(&pos, &end, &hit, &len) != 0) {
            sum.x += hit.x;
            sum.y += hit.y;
            sum.z += hit.z;
        }
    }
    if (player->unk173 == 0) {
        func_8001DCD0(actor->rotationX, &sum, &pitch, &yaw);
        actor->rotationZ = dAngle(actor->rotationZ, pitch,
                                  1.0f - Powerf(D_80081874, (s32) updateRate));
        actor->rotationY = dAngle(actor->rotationY, yaw,
                                  1.0f - Powerf(D_80081878, (s32) updateRate));
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
    player->unk2F0 = pos.x;
    player->unk2F4 = pos.y;
    player->unk2F8 = pos.z;
    return result;
}
/* PROVENANCE -- JFG's public charControl.c identifies the corresponding
 * controlSquashCheckPrior routine, but publishes assembly only; this body is
 * reconstructed from Mickey's fields, calls, branch conditions, and stores. */
/* Matched by rewriting from the listing in the shape of the matched overlay
 * 26 sibling (func_overlay_026_F0000B18_187AF10). What decided it: the point
 * is read through pos-> at each use, so the three loads and the two shared
 * products are uopt's own temporaries (the stack cells at 0x4C..0x5C); the
 * projection is built in dx/dy/dz and normalised in place; the collision
 * state is written through a pointer local taken at entry; Arctanf returns
 * int, so the (s16) cast is the shipped sign extension; and in the last
 * branch `len` carries the plane offset while `delta` keeps a copy for after
 * the calls. `len` is overwritten by the square root, so that copy cannot be
 * propagated: the offset is a one-block web that takes f0 and `delta` lives
 * in its home, which is what pushes the three point temporaries to f18, a
 * split f0 and a split f12. */
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_8008187C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081880.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081884.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081888.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_8008188C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081890.s")

void func_8001EC44(s32 unused, ControlVector3 *pos, ControlVector3 *vel,
                   f32 radius, ControlCollisionPlane *plane) {
    ControlCollisionState *state = &D_800CB2C0;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 delta;
    f32 u;
    f32 v;
    f32 w;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 d;
    f32 value;
    f32 len;
    f32 angle;

    nx = plane->x;
    ny = plane->y;
    nz = plane->z;
    d = plane->distance;
    value = (pos->z * nz) + ((nx * pos->x) + (ny * pos->y)) + d;
    if ((D_8008187C <= ny) || (plane->flags & 0x10000000)) {
        u = vel->z * ny;
        v = -(vel->z * nx) + (nz * vel->x);
        w = -(vel->x * ny);
        dx = (v * nz) - (w * ny);
        dy = (w * nx) - (u * nz);
        dz = (u * ny) - (v * nx);
        len = (dx * dx) + (dy * dy) + (dz * dz);
        if (D_80081880 < len) {
            len = sqrtf(len);
            dx /= len;
            dy /= len;
            dz /= len;
            value = radius - plane->unk1C;
            pos->x = plane->unk10 + (value * dx);
            pos->y = plane->unk14 + (value * dy);
            pos->z = plane->unk18 + (value * dz);
        } else {
            pos->y = (-((pos->z * nz) + (nx * pos->x) + d) / ny) + D_80081884;
        }
        state->unk04 = nx;
        state->unk08 = ny;
        state->unk0C = nz;
        state->state |= 2;
    } else if (ny <= D_80081888) {
        value = D_8008188C - value;
        pos->x = pos->x + (value * nx);
        pos->y = pos->y + (value * ny);
        pos->z = pos->z + (value * nz);
        state->unk1C = nx;
        state->unk20 = ny;
        state->unk24 = nz;
        state->state |= 8;
    } else {
        len = D_80081890 - value;
        delta = len;
        u = pos->x + (len * nx);
        v = pos->y + (len * ny);
        w = pos->z + (len * nz);
        dx = pos->x - u;
        dy = pos->y - v;
        dz = pos->z - w;
        len = sqrtf((dx * dx) + (dz * dz));
        angle = func_8002A8BC((s16) Arctanf(dy, len));
        if (angle != 0.0f) {
            value = delta / angle;
            len = sqrtf((nx * nx) + (nz * nz));
            pos->x += value * (nx / len);
            pos->z += value * (nz / len);
        } else {
            pos->x = u;
            pos->y = v;
            pos->z = w;
        }
        state->unk10 = nx;
        state->unk14 = ny;
        state->unk18 = nz;
        state->state |= 4;
    }
    state->flags = plane->flags;
    state->mode = plane->kind;
}
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
/* Existing readonly scalar cells, in their owning function order. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081894.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/D_80081898.s")

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
void controlSetPlayerSetup(s16 x, s16 y, s16 z, s16 angle) {
    D_800CB470 = x;
    D_800CB472 = y;
    D_800CB474 = z;
    D_800CB476 = angle;
    D_80079BF8 = 1;
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlGetPlayerSetup
 * name/role. This body is reconstructed from Mickey's setup-state accesses.
 */
s32 controlGetPlayerSetup(s16 *x, s16 *y, s16 *z, s16 *angle) {
    if (D_80079BF8 != 0) {
        *x = D_800CB470;
        *y = D_800CB472;
        *z = D_800CB474;
        *angle = D_800CB476;
        D_80079BF8 = 0;
        return 1;
    }
    return 0;
}

/* PROVENANCE -- adapted from JFG's src/charControl.c controlClearPlayerSetup. */
void controlClearPlayerSetup(void) {
    D_80079BF8 = 0;
}
