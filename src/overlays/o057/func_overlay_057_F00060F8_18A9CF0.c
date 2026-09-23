#include "PR/ultratypes.h"
#include "overlays/overlay_045.h"

typedef struct Overlay57MenuSource {
    u8 pad00[0x28];
    s16 index;
    s8 active;
    u8 pad2B[9];
} Overlay57MenuSource;

typedef struct Overlay57MenuEntry {
    s32 type;
    s8 controller;
    u8 pad05[3];
    s32 value08;
    u8 pad0C[0x1C];
} Overlay57MenuEntry;

typedef struct Overlay57IndexLink {
    s32 index;
} Overlay57IndexLink;

extern s32 overlay84GetActive(void);
extern void overlay84ClearMode(void);
extern void overlay84ActivateCurrent(s32 kind);
extern void overlay84Mark(void);
extern void amSndPlay(s32 soundId, void *handle);
extern void animseqStartPath(s32 pathId);
extern void animseqStopPath(s32 pathId);
extern void joyCreateMap(s8 *activePlayers);
extern void mainSetMode(s32 mode);
extern void mainChangeCameras(s32 camera);
extern void mainChangeLevel(s32 level, s32 entrance, s32 vehicle,
                            s32 cutscene, s32 multiplayer, s32 arg5);

extern void func_overlay_057_F0001020_18A4C18(s32 updateRate);
extern void overlay57SetNodeValue(s32 id, s32 argument, s32 valueBits);

extern s32 gOverlay57State;
extern s32 gOverlay57DistanceState;
extern s32 gOverlay57Selection;
extern s32 gOverlay57PreviousSelection;
extern s32 gOverlay57LayoutValue;
extern s32 gOverlay57PreviousLayoutValue;
extern s32 gOverlay57LayoutBusy;
extern s32 gOverlay57Timer;
extern s32 gOverlay57ObjectReady;
extern u8 gOverlay57ObjectId;
extern Overlay45ResourceDescriptor *gOverlay57Layouts[];
extern Overlay57IndexLink gOverlay57StartLinks[];
extern Overlay57IndexLink gOverlay57StopLinks[];
extern s32 gOverlay57NodeArguments[];
extern s16 gOverlay57ControllerMap[];
extern Overlay57MenuEntry *gOverlay57MenuEntries;
extern s16 gOverlay57LevelBySelection[];
extern s16 gOverlay57EntranceByIndex[];
extern s16 gOverlay57EntranceIndex;
extern s16 gOverlay57DefaultEntrance;
extern s32 gOverlay57MenuCount;
extern s16 gOverlay57MenuSources[];
extern s32 gOverlay57MenuSourceIndex;
extern s16 gOverlay57MenuInputX;
extern s16 gOverlay57MenuInputY;
extern u32 gOverlay57MenuButtons;
extern s32 gOverlay57Camera;
extern u8 gOverlay57MirrorSelection;
extern u8 gOverlay57MirrorFlag;
extern u8 gOverlay57MenuByte0;
extern u8 gOverlay57MenuByte1;
extern u8 gOverlay57MenuByte2;
extern u8 gOverlay57MenuByte3;
extern u16 gOverlay57MenuHalf;
extern s16 gOverlay57MenuHalf2;
extern s8 gOverlay57MenuSourcesStart[];
extern s8 gOverlay57MenuSourcesEnd[];

/* Workbench: exact-size at 441 instructions and frame 0x60; 252 masked words.
 * `controller` is s8, not s32 -- as an s32 the two writes through it cost the
 * candidate 16 instructions of sign handling, and the size delta closes with
 * the narrower type alone.
 *
 * The +2 temp-ring offset is now measured instead of inferred.  From ring
 * position 4 onward -- the `addu` that indexes gOverlay57Layouts in the first
 * layout branch -- this candidate skips t0 AND t1 for the whole function,
 * because two loop-invariant webs hold them: the s16-table base and the
 * constant 40 that is the menu-entry stride.  In the target those same two
 * webs are s1 and s2, CALLEE-SAVED, and the reason is not how the loop is
 * written: it is that in the target both webs are still live AFTER
 * joyCreateMap / mainSetMode / mainChangeCameras.
 *
 *   - s1 is live to the mainChangeLevel entrance argument.  In the target the
 *     entrance table and the controller map are the SAME address: the fill
 *     loop's `lh ...,0(sN)` and the entrance argument's `lh ...,0(...)` both
 *     carry displacement 0 off one `lui`/`addiu` pair.
 *   - s2 is live to the value08 block, where the target multiplies 2 by it
 *     (`li v1,2; multu v1,s2; mflo v0`) and reaches entries[2..5] as
 *     base+80+{8,48,88,128} rather than as constant offsets.
 *
 * So the decision variable is CROSS-CALL LIVENESS of those two webs, not
 * indexed-versus-walked.  Both attempts to create it failed and are measured:
 * pointing the entrance argument at the map symbol costs 4 bytes (280 masked),
 * and every form of the value08 block that should keep the multiply is folded
 * back to constant offsets by uopt -- an `index = 2` variable, a `count`
 * variable, and `for (index = 2; index < 6; index++)` are all bit-identical to
 * the written-out stores (252); `for (index = 0; index < 6; index++)` costs 32
 * bytes and the downward `while (index--)` form saves 12.  What is wanted is a
 * form that keeps 40 in a register across the three calls at delta 0.
 *
 * Structure read out of the target, confirmed instruction by instruction but
 * NOT adoptable while the ring is 2 positions out -- every one of them scores
 * worse against a mis-registered baseline, so they are recorded here for the
 * lane that lands the webs first:
 *
 *   - the fill loop caches `source->active` in one `lb` and uses it for both
 *     the playerOrder store and the test, and then reads the controller map
 *     TWICE (`lh` of source->index, scale, index, twice over) rather than
 *     caching it in `controller`.  Adopting it alone costs 8 bytes.
 *   - the activePlayers init loop counts DOWN: `index = 10; while (index--)`,
 *     writing sp+0x4D down to sp+0x44, with the dead post-decrement `move`.
 *     Adopting it alone costs 4 bytes.
 *   - the second loop's `controller` is a plain s32 there: the target has no
 *     `sll`/`sra` sign extension in it at all.  Adopting it alone SAVES 16
 *     bytes, which says the target spends those 16 elsewhere.
 *   - `gOverlay57MenuEntries[0].type = 5` is an `sb` in the target, so the
 *     first field is a byte, not an s32.  Byte-neutral here.
 *
 * Two corrections are adopted above because each is a win at delta 0: the
 * third layout branch's second overlay45ConfigureLayout takes 4, not 0x104, as
 * its last argument (the target's `li a3,4`), and the second loop is written
 * `for (index = gOverlay57MenuCount; index < 6; index++)`, which reproduces the
 * target's strength reduction -- entry test on the count, back-edge test on the
 * byte offset -- for 3 words.  256 -> 252. */
#ifdef NON_MATCHING
void func_overlay_057_F00060F8_18A9CF0(s32 updateRate) {
    Overlay57MenuSource *source;
    Overlay57IndexLink *link;
    s8 playerOrder[4];
    u8 activePlayers[10];
    s32 selection;
    s32 index;
    s32 count;
    s8 controller;

    if (overlay84GetActive() != 0) {
        return;
    }

    gOverlay57State = 6;
    if (gOverlay57MenuInputX < -16 && gOverlay57Selection > 0 &&
        gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection--;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], -0xA0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputX >= 17 && gOverlay57Selection < 3 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection++;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0x1E0, 0xBE,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputY < -16 && gOverlay57Selection < 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection += 2;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    } else if (gOverlay57MenuInputY >= 17 && gOverlay57Selection >= 2 &&
               gOverlay57DistanceState == 0) {
        gOverlay57LayoutBusy = 1;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 0x104);
        gOverlay57PreviousSelection = gOverlay57Selection;
        gOverlay57Selection -= 2;
        overlay45ConfigureLayout(gOverlay57Layouts[gOverlay57Selection], 0xA0, 0x104,
                                 4);
        gOverlay57PreviousLayoutValue = gOverlay57LayoutValue;
        gOverlay57LayoutValue = 0xFF;
    }

    if ((gOverlay57MenuButtons & 0x4000) && gOverlay57DistanceState == 0 &&
        gOverlay57ObjectReady != 0) {
        amSndPlay(13, 0);
        overlay84ClearMode();
        animseqStopPath(gOverlay57ObjectId);
        gOverlay57Timer = 7;
        gOverlay57State = 0;
        overlay84ActivateCurrent(1);

        link = gOverlay57StartLinks;
        if (link->index != -1) {
            index = link->index;
            do {
                animseqStartPath(index & 0xFF);
                overlay57SetNodeValue(index, gOverlay57NodeArguments[index],
                                      0x3BE56042);
                index = (++link)->index;
            } while (index != -1);
        }
        link = gOverlay57StopLinks;
        if (link->index != -1) {
            index = link->index;
            do {
                animseqStopPath(index & 0xFF);
                index = (++link)->index;
            } while (index != -1);
        }
    }

    func_overlay_057_F0001020_18A4C18(updateRate);
    if ((gOverlay57MenuButtons & 0x9000) && gOverlay57DistanceState == 0) {
        amSndPlay(12, 0);

        for (index = 0; index < 10; index++) {
            activePlayers[index] = 1;
        }

        source = (Overlay57MenuSource *)gOverlay57MenuSourcesStart;
        index = 0;
        count = 0;
        while ((s8 *)source < gOverlay57MenuSourcesEnd) {
            playerOrder[index++] = source->active;
            if (source->active != 0) {
                controller = gOverlay57ControllerMap[source->index];
                gOverlay57MenuEntries[count++].controller = controller;
                activePlayers[controller] = 0;
            }
            source++;
        }

        controller = 0;
        for (index = gOverlay57MenuCount; index < 6; index++) {
            while (activePlayers[controller] == 0) {
                controller++;
            }
            gOverlay57MenuEntries[index].controller = controller++;
        }

        joyCreateMap(playerOrder);
        mainSetMode(0);
        mainChangeCameras(gOverlay57Camera);
        gOverlay57MenuEntries[0].type = 5;
        selection = gOverlay57Selection;
        if (selection == 1) {
            gOverlay57MenuByte0 = 4;
            gOverlay57MenuByte1 = 3;
        } else if (gOverlay57MirrorFlag != 0) {
            gOverlay57MenuByte0 = 4;
            gOverlay57MenuByte1 = 4 - selection;
        } else {
            gOverlay57MenuByte0 = selection;
            gOverlay57MenuByte1 = 0;
        }
        gOverlay57MenuByte2 = selection >= 2;
        gOverlay57MenuByte3 = 2;
        gOverlay57MenuEntries[0].value08 = 0;
        gOverlay57MenuEntries[1].value08 = 0;
        gOverlay57MenuEntries[2].value08 = 0;
        gOverlay57MenuEntries[3].value08 = 0;
        gOverlay57MenuEntries[4].value08 = 0;
        gOverlay57MenuEntries[5].value08 = 0;
        gOverlay57MenuHalf = 0x3FC;
        gOverlay57MirrorSelection = 0;
        mainChangeLevel(gOverlay57LevelBySelection[gOverlay57Selection],
                        gOverlay57EntranceByIndex[gOverlay57EntranceIndex],
                        0, 5, 1, 0);
        gOverlay57DistanceState = 1;
        overlay84Mark();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F00060F8_18A9CF0/func_overlay_057_F00060F8_18A9CF0.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_057_F00060F8_18A9CF0:start
 * symbol: func_overlay_057_F00060F8_18A9CF0
 * score: 252/441 words
 * frame: 0x60
 * relocations: 175
 * first-mismatch: +0x7C
 * summary: The +2 ring offset is two loop-invariant webs, the s16-table base and the constant 40 menu-entry stride, which hold t0 and t1 here and are callee-saved s1 and s2 in the target because there both are still live after joyCreateMap mainSetMode and mainChangeCameras: s1 reaches the mainChangeLevel entrance argument off the same address as the controller map, and s2 is multiplied by 2 in the value08 block; so the variable is cross-call liveness, not indexed versus walked, and every form tried so far is either folded back to constant offsets by uopt or costs size. Adopted here: the third layout branch's last argument is 4 not 0x104, and the second loop is a for over the count which reproduces the target's strength reduction, taking 256 to 252 at delta 0.
 * PLATEAU-HANDOFF:func_overlay_057_F00060F8_18A9CF0:end
 */
