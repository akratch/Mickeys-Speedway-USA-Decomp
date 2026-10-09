#include "ultra64.h"
#include "game/level.h"

/*
 * Resident level lifecycle and metadata, ROM 0x263F0-0x27760.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/level.c and built src/level.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern s32 D_800CF3C4;
typedef struct LevelHeaderSummarySource LevelHeaderSummarySource;
extern LevelHeaderSummarySource *D_800CF3C8;
extern s32 *D_800CF3C0;
extern s32 D_800CF3D4;
extern s32 D_800CF3D8;
/* The level TU owns the adjacent count and colour-cycle BSS ranges. */
s32 D_800CF3E0[16];
u8 D_800CF420[0x70];
extern u8 **D_800CF3D0;
extern u8 *D_8007A0D0;
extern u8 D_8007BF08;
extern void *D_8007A0E0[3];
extern s16 D_8007A0EC[3];
extern s16 *D_8007A0F4;
extern u8 D_8007BF0C;
extern s16 *D_800C94E0;
extern void *D_800CF490[];
extern s32 D_800CF508;
extern s32 D_800C947C;
extern s32 D_80078DF0;

/* PROVENANCE: field roles adapted from JFG src/level.c; Mickey layout is decisive. */
typedef struct LevelSummary {
    u8 type;
    s8 world;
    u8 objectFlag;
    u8 gfxIndex : 3;
    u8 screenMode : 2;
    u8 region : 3;
    u8 tune;
    u8 blur;
} LevelSummary;

typedef struct LevelHeaderColour {
    u8 pad0[0x94];
    s16 weatherType[7];
} LevelHeaderColour;

struct LevelHeaderSummarySource {
    u8 pad0[0x20];
    s8 world;
    u8 pad21[2];
    s8 gfxIndex;
    u8 pad24[0x28];
    s16 trackArg0;
    s16 trackArg3;
    s16 trackArg1;
    s8 resourceMode;
    u8 pad53;
    u8 trackData[0x24];
    s16 fogNear;
    s16 fogFar;
    s16 fogTargetNear;
    u8 fogR;
    u8 fogG;
    u8 fogB;
    u8 fogA;
    u8 pad82;
    s8 type;
    u8 pad84[0xA];
    u8 tune;
    u8 pad8F[5];
    s16 colourCycles[7];
    s16 weatherCount;
    u8 padA4;
    u8 weatherType;
    u8 weatherColourA;
    u8 weatherColourB;
    s16 weatherVelX;
    s16 weatherVelY;
    s16 weatherVelZ;
    s8 cameraFov;
    u8 screenR;
    u8 screenG;
    u8 screenB;
    u8 padB2[6];
    s32 resourceB8;
    s16 valueBC;
    s16 valueBE;
    s32 resourceC0;
    u8 padC4[2];
    s8 featureC6;
    u8 voiceLimit;
    u8 padC8[4];
    s8 screenMode;
    u8 region;
    s16 trackArg4;
    u8 padD0[0x16];
    u8 camera;
    u8 padE7;
    s32 resourceE8;
    s16 trackArg5;
    u8 padEE[0xD];
    u8 featureFB;
    u8 padFC[9];
    u8 weatherScale;
    u8 pad106;
    u8 blur;
    u8 lightCount;
    u8 pad109[3];
    s16 tunes[3];
    u8 pad112[0x1E];
};

extern LevelSummary *D_800CF3DC;
extern s32 mainGetNumberOfCameras(void);
extern void func_80000510(u8);
extern void amTuneResetFade(void);
extern void func_80000B48(u16);
extern void amTuneResetChls(void);
extern void amTuneStop(void);
extern u8 amTuneGetSeqNo(void);
extern void func_80036AB0(void *, s32);
extern s32 *piRomLoad(s32);
extern s32 piRomLoadSection(s32, void *, s32, s32);
extern void mmFree(void *);
extern void *mmAlloc(s32, s32);
extern u8 *align4(u8 *);
extern void rumbleKill(s32);
extern s8 func_800291FC(void);
extern void gsSndpLimitVoices(s32);
extern void initColourCycle(void *, s16);
extern void amTuneVoiceLimit(u8);
extern void func_80000450(s32);
extern void setupLights(s32, s32, s32);
extern void func_8000A6DC(s32);
extern void func_80051004(s32);
extern void *func_80034448(s32);
extern void *func_800355A0(s32, s32);
extern void *func_8000486C(s32);
extern void *func_8001F520(s32, s32);
extern s32 runlinkDownloadCode(s32);
extern void trackSetFogOff(s32);
extern void trackSetFog(s32, s16, s16, s16, s32, s32, s32, s32);
extern void setupWeather(s32, s32, s32, s32, s32, s32, s32);
extern void weather_clip_planes(s16, s16);
extern void func_8002EBD4(void *);
extern void *func_800056A4(s32);
extern void func_80036C60(void *);
extern void rcpSetScreenColour(u8, u8, u8);
extern void viFrameRateReset(void);
extern void levelTunePlay(void);

#pragma weak levelTrackInitTrap = TrapDanglingJump
extern void levelTrackInitTrap(s32, s32, s32, s32, s32, s32);
#pragma weak levelOverlay7InitPoolTrap = TrapDanglingJump
extern void levelOverlay7InitPoolTrap(void);
#pragma weak levelOverlay34InitStorageTrap = TrapDanglingJump
extern void levelOverlay34InitStorageTrap(s32);
#pragma weak levelOverlay33InitializeBuffersTrap = TrapDanglingJump
extern void levelOverlay33InitializeBuffersTrap(void);
#pragma weak levelOverlay42InitTrap = TrapDanglingJump
extern void levelOverlay42InitTrap(void);
#pragma weak levelOverlay16InitializeBufferTrap = TrapDanglingJump
extern void levelOverlay16InitializeBufferTrap(u8 *);
#pragma weak levelOverlay103CheckSignatureTrap = TrapDanglingJump
extern s32 levelOverlay103CheckSignatureTrap(void);
extern void camSetNo(s32);
extern void func_80021504(f32, s32);
extern void func_8003C770(s32, s32);
extern void runlinkFreeCode(s32);
extern void amSndStop(void *);
extern void amSndPlay(u16, void **);
extern void func_80001708(void);
extern void amAmbientStop(void);
extern void camlightFlush(void);
extern void amResetAudioMap(void);
extern void func_800582A8(void);
extern void freeWeather(void);
extern void func_800347A0(void *);
extern void func_800359D4(void *);
extern void func_80004B04(s16);
extern void modFreeModel(void *);
extern void runlinkFlushModules(void);

/*
 * PROVENANCE: body adapted from public JFG src/level.c at commits
 * 773e313/1a92d81; the pinned JFG reference still uses GLOBAL_ASM, and Mickey
 * byte identity is decisive. piRomLoadSection returns s32; keeping that
 * result in count lets the following world value coalesce into a0.
 */
void levelGetCounts(void) {
    s32 i;
    s32 count;
    LevelHeaderSummarySource *header;
    u8 *nameData;

    header = mmAlloc(sizeof(LevelHeaderSummarySource), 0x8F);
    D_800CF3C0 = piRomLoad(0x1E);

    for (i = 0; i != 16; i++) { D_800CF3E0[i] = 0;
    }

    D_800CF3D4 = 0;
    while (D_800CF3C0[D_800CF3D4] != -1) {
        D_800CF3D4++;
    }
    D_800CF3D4--;

    D_800CF3DC = mmAlloc(D_800CF3D4 * sizeof(LevelSummary), 0x8F);
    D_800CF3D8 = -1;
    D_800CF3C8 = header;
    for (i = 0; i < D_800CF3D4; i++) {
        count = piRomLoadSection(0x1F, D_800CF3C8, D_800CF3C0[i], sizeof(LevelHeaderSummarySource));
        if (D_800CF3C8->world > D_800CF3D8) {
            D_800CF3D8 = D_800CF3C8->world;
        }
        if ((D_800CF3C8->type >= 0) && (D_800CF3C8->type < 16)) { D_800CF3E0[D_800CF3C8->type]++;
        }
        D_800CF3DC[i].type = D_800CF3C8->type;
        D_800CF3DC[i].world = D_800CF3C8->world;
        D_800CF3DC[i].gfxIndex = D_800CF3C8->gfxIndex;
        D_800CF3DC[i].screenMode = D_800CF3C8->screenMode;
        D_800CF3DC[i].region = D_800CF3C8->region;
        D_800CF3DC[i].tune = D_800CF3C8->tune;
        D_800CF3DC[i].blur = D_800CF3C8->blur;
    }
    D_800CF3D8++;
    mmFree(D_800CF3C0);
    mmFree(header);

    D_8007A0D0 = mmAlloc(0x20, 0x8F);
    D_800CF3C0 = piRomLoad(0x22);
    i = 0;
    while (D_800CF3C0[i] != -1) {
        i++;
    }
    i--;
    count = D_800CF3C0[i] - D_800CF3C0[0];
    nameData = mmAlloc((s32) align4((u8 *) count) + (i * 4), 0x8F);
    D_800CF3D0 = (u8 **) ((u32) nameData + (s32) align4((u8 *) count));
    piRomLoadSection(0x23, nameData, 0, count);
    for (count = 0; count < i; count++) {
        D_800CF3D0[count] = nameData + D_800CF3C0[count];
    }
    mmFree(D_800CF3C0);
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelNGetType(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].type;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetTune(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].tune;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetWorld(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].world;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetRegionNo(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].region;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetScreenMode(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].screenMode;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetBlurEffect(s32 levelIndex) {
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4)) {
        return D_800CF3DC[levelIndex].blur;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u32 levelGetGfxIndex(s32 levelIndex) {
    u32 result;

    result = mainGetNumberOfCameras() - 1;
    if ((levelIndex >= 0) && (levelIndex < D_800CF3D4) && (result == 0) &&
        (D_800CF3DC[levelIndex].gfxIndex != 0)) {
        result = 4;
    }
    return result;
}

/* PROVENANCE: pinned JFG c82afff keeps levelInit assembly-only; later public
 * JFG 773e313/1a92d81 C provides structural and lifetime evidence only.
 * Mickey's call graph, fields, boundaries, and bytes remain decisive. */
void levelInit(s32 lvlIdx, s32 trackParam, s32 unused, s32 mode) {
    s16 tune;
    s32 i;
    s32 lvlStart;
    u32 lvlSize;
    s32 lvlCount;
    s32 j;
    s32 shouldPlay;
    s32 off;
    s32 freeSlot;
    s32 resourceId;

    rumbleKill(1);
    D_800CF3C0 = piRomLoad(0x1E);
    if (mode < 0) {
        mode = 0;
    }

    switch (func_800291FC()) {
        case 4:
            gsSndpLimitVoices(16);
            break;
        case 3:
            gsSndpLimitVoices(16);
            break;
        case 2:
            gsSndpLimitVoices(12);
            break;
        default:
            gsSndpLimitVoices(8);
            break;
    }

    lvlCount = 0;
    while (D_800CF3C0[lvlCount] != -1) {
        lvlCount++;
    }
    lvlCount--;
    if (lvlIdx >= lvlCount) {
        lvlIdx = 0;
    }

    lvlStart = D_800CF3C0[lvlIdx];
    lvlSize = D_800CF3C0[lvlIdx + 1] - lvlStart;
    D_800CF3C8 = mmAlloc(lvlSize, 0x85);
    piRomLoadSection(0x1F, D_800CF3C8, lvlStart, lvlSize);
    mainPreNMI();
    mmFree(D_800CF3C0);
    D_800CF3C4 = lvlIdx;

    for (i = 0; i < 7; i++) {
        if (D_800CF3C8->colourCycles[i] != -1) {
            initColourCycle(&D_800CF420[i * 16], D_800CF3C8->colourCycles[i]);
        }
    }
    amTuneVoiceLimit(D_800CF3C8->voiceLimit);
    amTuneResetFade();
    func_80000450(0);
    mainPreNMI();
    setupLights(D_800CF3C8->lightCount, 8, 0x10);
    mainPreNMI();

    func_8000A6DC(mode);

    mainPreNMI();
    levelTrackInitTrap(D_800CF3C8->trackArg0, D_800CF3C8->trackArg1, trackParam,
                       D_800CF3C8->trackArg3, D_800CF3C8->trackArg4,
                       D_800CF3C8->trackArg5);

    mainPreNMI();

    func_80051004(mode);

    if ((D_800CF3C8->type == 0) || (D_800CF3C8->type == 3)) {
        if (D_8007BF0C != 0) {
            D_8007A0F4 = piRomLoad(0x3C);
        } else {
            D_8007A0F4 = piRomLoad(0x3B);
        }
        D_800CF508 = 0;
        while (D_8007A0F4[D_800CF508] != -1) {
            D_800CF508++;
        }
        /* Each arm strips the two type bits from the id in place before it
         * uses it. That one statement per arm is what the allocator needed:
         * the id is a web of its own (no borrowed carrier, no unread pad), the
         * tests are single masks, and the table read is a plain subscript. */
        for (off = 0; off < D_800CF508; off++) {
            resourceId = D_8007A0F4[off];
            if ((resourceId & 0xC000) == 0xC000) {
                resourceId &= 0x3FFF;
                D_800CF490[off] = func_80034448(resourceId);
            } else if (resourceId & 0x8000) {
                resourceId &= 0x3FFF;
                D_800CF490[off] = func_800355A0(resourceId, 0);
            } else if (resourceId & 0x4000) {
                resourceId &= 0x3FFF;
                D_800CF490[off] = func_8000486C(D_800C94E0[resourceId]);
            } else {
                resourceId &= 0x3FFF;
                D_800CF490[off] = func_8001F520(resourceId, 0);
            }
        }
        runlinkDownloadCode(0x17);
        runlinkDownloadCode(0x16);
        runlinkDownloadCode(0x1A);
        runlinkDownloadCode(0x1D);
        runlinkDownloadCode(0x19);
        runlinkDownloadCode(0x1C);
        runlinkDownloadCode(0x1B);
        runlinkDownloadCode(0x18);
        runlinkDownloadCode(7);
        levelOverlay7InitPoolTrap();
        runlinkDownloadCode(0x25);
        runlinkDownloadCode(0x26);
        if (D_8007BF0C != 0) {
            levelOverlay34InitStorageTrap(0x28);
        } else {
            levelOverlay34InitStorageTrap(0x50);
        }
    }

    if ((D_800CF3C8->fogNear == 0) && (D_800CF3C8->fogFar == 0) &&
        (D_800CF3C8->fogR == 0) && (D_800CF3C8->fogG == 0) &&
        (D_800CF3C8->fogB == 0)) {
        for (i = 0; i < 4; i++) {
            trackSetFogOff(i);
        }
    } else {
        for (i = 0; i < 4; i++) {
            trackSetFog(i, D_800CF3C8->fogNear, D_800CF3C8->fogFar,
                        D_800CF3C8->fogTargetNear, D_800CF3C8->fogR,
                        D_800CF3C8->fogG, D_800CF3C8->fogB, D_800CF3C8->fogA);
        }
    }

    if (D_800CF3C8->weatherCount > 0) {
        setupWeather(D_800CF3C8->weatherType, D_800CF3C8->weatherCount,
                     D_800CF3C8->weatherVelX << 8, D_800CF3C8->weatherVelY << 8,
                     D_800CF3C8->weatherVelZ << 8,
                     D_800CF3C8->weatherColourA * 0x101,
                     D_800CF3C8->weatherColourB * 0x101);
        weather_clip_planes(-1, -0x200);
    }
    if (D_800CF3C8->resourceMode == 3) {
        D_800CF3C8->resourceB8 = (s32) func_80034448(D_800CF3C8->resourceB8);
        D_800CF3C8->valueBC = 0;
        D_800CF3C8->valueBE = 0;
    } else if ((D_800CF3C8->resourceMode == 4) || (D_800CF3C8->resourceMode == 5)) {
        func_8002EBD4(&D_800CF3C8->trackData);
    }
    if (D_800CF3C8->resourceC0 != -1) {
        D_800CF3C8->resourceC0 = (s32) func_800056A4(D_800CF3C8->resourceC0);
        func_80036C60((void *) D_800CF3C8->resourceC0);
    }
    rcpSetScreenColour(D_800CF3C8->screenR, D_800CF3C8->screenG,
                       D_800CF3C8->screenB);
    viFrameRateReset();
    levelTunePlay();

    for (i = 0; i < 4; i++) {
        camSetNo(i);
        func_80021504((f32) D_800CF3C8->cameraFov, 1);
    }
    camSetNo(0);

    if (D_800CF3C8->resourceE8 == -1) {
        D_800CF3C8->resourceE8 = 0;
    } else {
        D_800CF3C8->resourceE8 = (s32) func_800056A4(D_800CF3C8->resourceE8);
    }
    mainPreNMI();

    if (D_800CF3C8->featureC6 != 0) {
        levelOverlay33InitializeBuffersTrap();
    }
    if (D_800CF3C8->screenMode != 0) {
        levelOverlay42InitTrap();
    }
    if (D_800CF3C8->featureFB != 0) {
        levelOverlay16InitializeBufferTrap((u8 *)D_800CF3C8);
    }
    func_8003C770(0, D_800CF3C8->weatherScale * 0xF0);
    mainPreNMI();
    runlinkFreeCode(0x23);
    runlinkFreeCode(0x13);

    for (off = 0; off < 3; off++) {
        shouldPlay = 1;
        for (j = 0; j < 3; j++) {
            if (D_800CF3C8->tunes[j] == D_8007A0EC[off]) {
                shouldPlay = 0;
            }
        }
        if (shouldPlay && (D_8007A0E0[off] != NULL)) {
            D_8007A0EC[off] = -1;
            amSndStop(D_8007A0E0[off]);
        }
    }

    if (levelOverlay103CheckSignatureTrap() == 0) {
        D_800CF3C8->fogNear = 0x384;
        D_800CF3C8->fogFar = 0x398;
    }

    for (off = 0; off < 3; off++) {
        tune = 1;
        if (D_800CF3C8->tunes[off] != -1) {
            for (j = 0; j < 3; j++) {
                if (D_800CF3C8->tunes[off] == D_8007A0EC[j]) {
                    tune = 0;
                } else if (D_8007A0EC[j] == -1) {
                    freeSlot = j;
                }
            }
            if (tune) {
                amSndPlay(D_800CF3C8->tunes[off], &D_8007A0E0[freeSlot]);
                D_8007A0EC[freeSlot] = D_800CF3C8->tunes[off];
            }
        }
    }
    func_80001708();
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
void levelTunePlay(void) {
    if (((u8 *) D_800CF3C8)[0x8E] != 0) {
        if (((u8 *) D_800CF3C8)[0x8E] != amTuneGetSeqNo()) {
            amTuneResetChls();
            func_80000510(((u8 *) D_800CF3C8)[0x8E]);
            amTuneResetFade();
            func_80000B48(*(u16 *) &((u8 *) D_800CF3C8)[0x90]);
        }
    } else {
        amTuneStop();
    }
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
void levelUpdateColourCycling(s32 updateRate) {
    s32 i;

    for (i = 0; i < 7; i++) {
        if (((LevelHeaderColour *) D_800CF3C8)->weatherType[i] != -1) {
            func_80036AB0(&D_800CF420[i * 16], updateRate);
        }
    }
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetColourCycling(void) {
    return D_800CF420;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNumber(void) {
    return D_800CF3C4;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 levelGetType(void) {
    return ((u8 *) D_800CF3C8)[0x83];
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 levelGetCamera(void) {
    if (D_800CF3C8 != NULL) {
        return ((u8 *) D_800CF3C8)[0xE6];
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetLevel(void) {
    return (u8 *) D_800CF3C8;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetName(s32 levelIndex) {
    *D_8007A0D0 = 0;
    if (levelIndex < D_800CF3D4) {
        D_800CF3C0 = piRomLoad(0x1E);
        if (D_800CF3C0 != NULL) {
            piRomLoadSection(0x1F, D_8007A0D0, D_800CF3C0[levelIndex], 0x20);
            mmFree(D_800CF3C0);
        }
    }
    return D_8007A0D0;
}

/* PROVENANCE: structure adapted from JFG src/level.c:levelFreeAll; Mickey
 * layout is decisive. The world-index arm mutates the existing s32 entry
 * carrier with the 14-bit mask, then adds the table pointer first. A fresh
 * mask local is forward-substituted into one expression and cannot emit the
 * mask before the table load and the scale after it; assigning the mask back
 * into the carrier is a surviving evaluation, and 32-bit width avoids a
 * narrowing chain. */
void levelFreeAll(void) {
    s32 assetId;
    s32 i;
    s8 levelType;
    void *levelData;

    rcpSetScreenColour(0, 0, 0);
    mmFree(D_800CF3C8);
    amAmbientStop();
    freeLights();
    camlightFlush();
    func_80013EC0();
    amResetAudioMap();
    func_800582A8();
    levelData = D_800CF3C8;
    if (*(s16 *) ((u8 *) levelData + 0xA2) > 0) {
        freeWeather();
    }
    levelType = *(s8 *) ((u8 *) D_800CF3C8 + 0x52);
    if (levelType == 3) {
        func_800347A0(*(void **) ((u8 *) D_800CF3C8 + 0xB8));
    } else if ((levelType == 4) || (levelType == 5)) {
        func_8002EBD4(0);
    }
    if (D_8007A0F4 != NULL) {
        for (i = 0; i < D_800CF508; i++) {
            assetId = D_8007A0F4[i];
            if ((assetId & 0xC000) == 0xC000) {
                func_800347A0(D_800CF490[i]);
            } else if (assetId & 0x8000) {
                func_800359D4(D_800CF490[i]);
            } else if (assetId & 0x4000) {
                assetId &= 0x3FFF;
                func_80004B04(*(s16 *) ((u32) D_800C94E0 + (assetId << 1)));
            } else {
                modFreeModel(D_800CF490[i]);
            }
            D_800CF490[i] = NULL;
        }
        mmFree(D_8007A0F4);
        D_8007A0F4 = NULL;
    }
    runlinkFlushModules();
    fxCpuTextureFlush();
    D_800C947C = 0;
    D_80078DF0 = 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNextOfWorld(s32 levelIndex, s8 world) {
    s32 next;

    next = levelIndex + 1;
    if (next >= D_800CF3D4) {
        next = 0;
    }
    while ((next != levelIndex) && (world != D_800CF3DC[next].world)) {
        next++;
        if (next >= D_800CF3D4) {
            next = 0;
        }
    }
    return next;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetPrevOfWorld(s32 levelIndex, s8 world) {
    s32 prev;

    prev = levelIndex - 1;
    if (prev < 0) {
        prev = D_800CF3D4 - 1;
    }
    while ((prev != levelIndex) && (world != D_800CF3DC[prev].world)) {
        prev--;
        if (prev < 0) {
            prev = D_800CF3D4 - 1;
        }
    }
    return prev;
}

s32 levelInitRegionFlags(void) {
    if (((s8) ((u8 *) D_800CF3C8)[0x83] == 0) && (D_8007BF08 != 0)) {
        return 1;
    }
    return 0;
}
