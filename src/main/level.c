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
extern void piRomLoadSection(s32, void *, s32, s32);
extern void mmFree(void *);
extern void *func_8002B280(s32, s32);
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

#ifdef NON_MATCHING
/*
 * PROVENANCE: body adapted from public JFG src/level.c at commits
 * 773e313/1a92d81; the pinned JFG reference still uses GLOBAL_ASM, and Mickey
 * byte identity is decisive. The historically measured pre-endpoint full-TU
 * spelling was 255/259 raw and 256/259 relocation-normalized, exact 259-word
 * size/frame 0x58, with all 37 offsets/types but only 35 identities; no
 * attributable C object survives. Raw first +0x50 was the identity-wrong
 * D_800CF3E0+0x40 endpoint pair; normalized first +0x13C and +0x148/+0x154
 * were one v0/target-a0 world carrier. A reformatted isolated import measured
 * only 251/259 raw and 252/259 normalized. Current D_800CF420 source is
 * uncompiled, so score, frame, and all identities are unknown; linked equality
 * proves fallback only. Run current full-TU V0 and, only on material regression,
 * one historical 0348d2f control. If V0 reproduces only the three normalized
 * carrier sites, retain exactly 119 configurations, trace once, and try one
 * natural block-scoped carrier. A strict legal gain alone permits one annotated
 * batch capped at 2,000 candidates or 20 minutes; otherwise park.
 */
void levelGetCounts(void) {
    s32 i;
    s32 count;
    LevelHeaderSummarySource *header;
    u8 *nameData;

    header = func_8002B280(sizeof(LevelHeaderSummarySource), 0x8F);
    D_800CF3C0 = piRomLoad(0x1E);

    for (i = 0; &D_800CF3E0[i] != (s32 *)D_800CF420; i++) { D_800CF3E0[i] = 0;
    }

    D_800CF3D4 = 0;
    while (D_800CF3C0[D_800CF3D4] != -1) {
        D_800CF3D4++;
    }
    D_800CF3D4--;

    D_800CF3DC = func_8002B280(D_800CF3D4 * sizeof(LevelSummary), 0x8F);
    D_800CF3D8 = -1;
    D_800CF3C8 = header;
    for (i = 0; i < D_800CF3D4; i++) {
        piRomLoadSection(0x1F, D_800CF3C8, D_800CF3C0[i], sizeof(LevelHeaderSummarySource));
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

    D_8007A0D0 = func_8002B280(0x20, 0x8F);
    D_800CF3C0 = piRomLoad(0x22);
    i = 0;
    while (D_800CF3C0[i] != -1) {
        i++;
    }
    i--;
    count = D_800CF3C0[i] - D_800CF3C0[0];
    nameData = func_8002B280((s32) align4((u8 *) count) + (i * 4), 0x8F);
    D_800CF3D0 = (u8 **) ((u32) nameData + (s32) align4((u8 *) count));
    piRomLoadSection(0x23, nameData, 0, count);
    for (count = 0; count < i; count++) {
        D_800CF3D0[count] = nameData + D_800CF3C0[count];
    }
    mmFree(D_800CF3C0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetCounts.s")
#endif

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelNGetType(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].type;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetTune(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].tune;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetWorld(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].world;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetRegionNo(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].region;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetScreenMode(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].screenMode;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetBlurEffect(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].blur;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u32 levelGetGfxIndex(s32 arg0) {
    u32 result;

    result = mainGetNumberOfCameras() - 1;
    if ((arg0 >= 0) && (arg0 < D_800CF3D4) && (result == 0) &&
        (D_800CF3DC[arg0].gfxIndex != 0)) {
        result = 4;
    }
    return result;
}

#ifdef NON_MATCHING
/* PROVENANCE: pinned JFG c82afff keeps levelInit assembly-only; later public
 * JFG 773e313/1a92d81 C provides structural and lifetime evidence only.
 * Mickey's call graph, fields, boundaries, and bytes remain decisive. */
/* Fresh pad-free configured V0 is exactly 516 words, 389/516 positional,
 * frame 0x78 versus target 0x80, with all 110 offsets/types and 102 stable
 * identities. The eight typed trap aliases retain authenticated ABIs but stay
 * fail-closed against the target TrapDanglingJump symbol. JFG's separate s16
 * tune lifetime is a strict gain to 394/516. A fidelity-clean proc-8 allocator
 * trace proves the first mechanism is the eight-byte frame deficit; extending
 * the donor's meaningful lvlCount lifetime through setupLights is byte-flat.
 * Preserve this pad-free basin. ORT 526 has sole caller func_80028564+0x5F8;
 * do not restore the rejected volatile pad or repeat broad search. */
void levelInit(s32 lvlIdx, s32 arg1, s32 arg2, s32 arg3) {
    s16 tune;
    s32 i;
    s32 lvlStart;
    u32 lvlSize;
    s32 lvlCount;
    s32 j;
    s32 shouldPlay;
    s32 off;
    s32 freeSlot;

    rumbleKill(1);
    D_800CF3C0 = piRomLoad(0x1E);
    if (arg3 < 0) {
        arg3 = 0;
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
    D_800CF3C8 = func_8002B280(lvlSize, 0x85);
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

    func_8000A6DC(arg3);

    mainPreNMI();
    levelTrackInitTrap(D_800CF3C8->trackArg0, D_800CF3C8->trackArg1, arg1,
                       D_800CF3C8->trackArg3, D_800CF3C8->trackArg4,
                       D_800CF3C8->trackArg5);

    mainPreNMI();

    func_80051004(arg3);

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
        /* The three doubled masks below are ugen phantom-pop carriers, not
         * evidence: each redundant `& M` on an already-masked value folds to
         * no instruction and still costs one integer temp-ring pop (L65), and
         * the target's loop body allocates three temps more than the plain
         * spelling does.  Removing any of them costs 87 words of pure ring
         * phase in the whole tail of the function.  The `*(s16 *)((i << 1) +
         * (u32)base)` access, by contrast, IS evidence: levelFreeAll, in this
         * same file and one word-pair from exact, reads this same table that
         * way, and it is what fixes the operand order of the address add.
         *
         * 2026-09-10, lane c6-named: the 16-word colour term is now closed
         * against reordering as well as against reservation, from the traced
         * interference list rather than from a sweep.  Web 104's twelve
         * interferers are 288, 289, 290, 291, 295, 296, 297, 298, 307, 115,
         * 106 and 99; exactly ONE of them (298) holds a caller-saved colour,
         * and its save is 2.5 against web 104's 26.7, so raising it above the
         * target needs eleven more loop references.  The rest are callee-saved
         * because they are live across the loop and this loop calls, so no
         * reordering of existing webs can supply the two caller-saved
         * interferers on c1 and c2 that the target's a2 requires -- the two
         * webs would have to live inside the loop body and not cross a call,
         * and only a new web can do that.  The mirror lever found on overlay
         * 86 the same day does not apply either: a web whose span reaches a
         * call result has v0 struck from its `p1cost` candidate list, but web
         * 104's list begins at colour 1, so v0 is offered and taken and the
         * span is already short.  Naming the masked index or the store address
         * as a loop-body local to manufacture the missing webs fails for a
         * third reason: uopt re-materialises a cheap masked value at each use
         * instead of keeping the local, so the loop body comes out
         * byte-identical and the only effect is the frame cell the twelfth
         * declaration costs (24 words, delta 0, both carriers, and 29 with
         * both).
         *
         * 2026-09-10, lane w8-bigclose: the colour term is confirmed from the
         * allocator's own records, and one axis it was never measured against
         * is now retired outright.  An instrumented uopt (CDX log; its object
         * is byte-identical to the tree's, which is the identity gate) records
         * 1655 p1 records for this procedure and ZERO p2 -- level.c does emit
         * 70 p2 records, but all of them belong to the file's small procedures,
         * none to this one.  So [L106]'s ascending-web-number colouring never
         * runs here and moving a defining statement cannot move a colour in
         * this function; only [L100]'s save = totalsave/nocs orders anything.
         * That is [L108]'s question and it costs one compile: ask it before
         * spending a pass on definition position, declaration order or
         * statement order, all three of which are the same dead axis here.
         *
         * The trace also confirms every number above and adds the reason the
         * two terms move together.  Web 104 is save 26.666666 (nocs 3,
         * totalsave 80) at cost 0 and is decided TWELFTH of this procedure's
         * decisions; of the eleven decided before it, only three interfere with
         * it, and their colours are exactly the two the closure names, which is
         * why its forbidden set is that pair and it takes the first free
         * colour.  The web that carries the resource table's address is decided
         * far LATER, at save 2.0, which is why it currently takes the colour
         * the target gives web 104: fix web 104 and the address web finds that
         * colour taken and falls one further, which is the target's pair
         * exactly.  The 16 words really are one decision, and the two
         * interfering caller-saved webs it needs must be decided in the first
         * eleven -- a constraint on their save, not on their position.
         *
         * 2026-09-11, lane f9-audit: those two webs were already in the
         * function.  uopt's web is the SYMBOL, not the def-use chain --
         * f_intfering intersects two live-BLOCK bit vectors, and a symbol's
         * live blocks are the union over all of its uses -- so carrying the
         * resource id in `shouldPlay` (the first tune loop's flag: a2, save
         * 40, decided seventh, live in the blocks where the v0/v1 tune-loop
         * temps 223/224 live) hands the loop value a web whose forbidden set
         * is c1..c4.  It takes a2 and the address web falls to a3 behind it:
         * 22 -> 6 at delta 0, frame unchanged.  Every other existing local
         * was measured as the carrier (tune 17, lvlCount 30, j 31, freeSlot
         * and i move the frame); a fresh local cannot do it because it has
         * no second use.  `s16 resourceId` stays declared only to hold the
         * frame cell the original spent on some local this function no
         * longer reads; nothing in the object says which local that was.
         * The same carrier trick does NOT reach the 6-word order term: a
         * masked index named into j/lvlCount/i at the top of the body is
         * copy-propagated back into every arm before live ranges form (88
         * and 92), so L102 holds for a shared symbol too -- a carrier only
         * works on a value that survives copy propagation.
         *
         * 2026-09-11, lane s1-one: three mechanisms retired against the order
         * term, measured on levelFreeAll (the cheaper twin) and confirmed
         * here.  Carrier identity is inert: the masked index named into every
         * local that function already has, including the one with its own
         * earlier live range, crossed with five address spellings, is either
         * byte-identical to the inline form or costs four to eight bytes of
         * narrowing.  L109's discarded-expression probes are inert: all three
         * named forms are byte-identical, so the probe never survives to give
         * the mask a second read.  And the phantom cannot be moved after the
         * address add: a redundant AND takes its ring slot before its own
         * operand subtree is evaluated, so it pops first wherever inside the
         * index expression it is written -- a triple mask, the redundant mask
         * on the shifted value, on the base and on the whole address, the
         * all-ones spellings, `* 2` and the self-addition were each measured.
         * Deleting the pop is not available either: the arm then spends four
         * temps and costs 88 to 95 words, with the first mismatch moving back
         * to +0x238, so the fifth pop is load-bearing well before this loop.
         * Carrying the table read in the declared-and-unread `resourceId` is
         * byte-identical, so that may be the local the frame cell belongs to,
         * but it is not a lever. */
        for (off = 0; off < D_800CF508; off++) {
            s16 resourceId;

            shouldPlay = D_8007A0F4[off];
            if ((shouldPlay & 0xC000) == 0xC000) {
                D_800CF490[off] = func_80034448(shouldPlay & 0x3FFF);
            } else if ((shouldPlay & 0x8000) & 0x8000) {
                D_800CF490[off] = func_800355A0(shouldPlay & 0x3FFF, 0);
            } else if ((shouldPlay & 0x4000) & 0x4000) {
                D_800CF490[off] = func_8000486C(
                    *(s16 *) ((((shouldPlay & 0x3FFF) & 0x3FFF) << 1) +
                              (u32) D_800C94E0));
            } else {
                D_800CF490[off] = func_8001F520(shouldPlay & 0x3FFF, 0);
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInit.s")
#endif

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
void levelUpdateColourCycling(s32 arg0) {
    s32 i;

    for (i = 0; i < 7; i++) {
        if (((LevelHeaderColour *) D_800CF3C8)->weatherType[i] != -1) {
            func_80036AB0(&D_800CF420[i * 16], arg0);
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
u8 *levelGetName(s32 arg0) {
    *D_8007A0D0 = 0;
    if (arg0 < D_800CF3D4) {
        D_800CF3C0 = piRomLoad(0x1E);
        if (D_800CF3C0 != NULL) {
            piRomLoadSection(0x1F, D_8007A0D0, D_800CF3C0[arg0], 0x20);
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
    s32 temp_v0_2;
    s32 i;
    s8 temp_v1;
    void *temp_v0;

    rcpSetScreenColour(0, 0, 0);
    mmFree(D_800CF3C8);
    amAmbientStop();
    freeLights();
    camlightFlush();
    func_80013EC0();
    amResetAudioMap();
    func_800582A8();
    temp_v0 = D_800CF3C8;
    if (*(s16 *) ((u8 *) temp_v0 + 0xA2) > 0) {
        freeWeather();
    }
    temp_v1 = *(s8 *) ((u8 *) D_800CF3C8 + 0x52);
    if (temp_v1 == 3) {
        func_800347A0(*(void **) ((u8 *) D_800CF3C8 + 0xB8));
    } else if ((temp_v1 == 4) || (temp_v1 == 5)) {
        func_8002EBD4(0);
    }
    if (D_8007A0F4 != NULL) {
        for (i = 0; i < D_800CF508; i++) {
            temp_v0_2 = D_8007A0F4[i];
            if ((temp_v0_2 & 0xC000) == 0xC000) {
                func_800347A0(D_800CF490[i]);
            } else if (temp_v0_2 & 0x8000) {
                func_800359D4(D_800CF490[i]);
            } else if (temp_v0_2 & 0x4000) {
                temp_v0_2 &= 0x3FFF;
                func_80004B04(*(s16 *) ((u32) D_800C94E0 + (temp_v0_2 << 1)));
            } else {
                modFreeModel(D_800CF490[i]);
            }
            D_800CF490[i] = NULL;
        }
        mmFree(D_8007A0F4);
        D_8007A0F4 = NULL;
    }
    runlinkFlushModules();
    func_8004AF68();
    D_800C947C = 0;
    D_80078DF0 = 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNextOfWorld(s32 arg0, s8 arg1) {
    s32 next;

    next = arg0 + 1;
    if (next >= D_800CF3D4) {
        next = 0;
    }
    while ((next != arg0) && (arg1 != D_800CF3DC[next].world)) {
        next++;
        if (next >= D_800CF3D4) {
            next = 0;
        }
    }
    return next;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetPrevOfWorld(s32 arg0, s8 arg1) {
    s32 prev;

    prev = arg0 - 1;
    if (prev < 0) {
        prev = D_800CF3D4 - 1;
    }
    while ((prev != arg0) && (arg1 != D_800CF3DC[prev].world)) {
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

/* PLATEAU-HANDOFF:levelInit:start
 * symbol: levelInit
 * score: 510/516 words
 * frame: 0x80
 * relocations: 110
 * first-mismatch: +0x328
 * summary: Separate ring-mask evaluation, fifth draw kept: statement double-mask stayed 6, use-site mask scored 19, comma-before-base scored 94. Phantom still leads.
 * PLATEAU-HANDOFF:levelInit:end
 */
