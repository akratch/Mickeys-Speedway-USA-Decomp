#include "overlays/overlay_058.h"

/* Tier B: module-lifetime result-screen storage. Four heading/player slots
 * and six race-entry score/order/tie slots are independently used by the
 * current readers. The retained source operand imports below preserve the
 * loader representation while naming these actual backing definitions.
 * IDO supplies alignment before ColourCycle and after the player bytes;
 * there are no source padding objects. Original declarations and every
 * caller's entry-count preconditions remain separate obligations. */
s32 o58ScoreDeltas[6];
char *o58HeadingText[4];
s32 o58SelectedAlphabetRow;
s32 o58SelectedAlphabetColumn;
s32 o58PathState;
Overlay58AnimationState o58Animation;
s32 o58PathDelay;
s32 gOverlay58FloorChoice;
s32 gOverlay58InsertChoice;
s32 o58ScreenState;
s32 o58PrimaryX;
s32 o58RankX;
s32 o58ScoreX;
s32 o58HeadingX;
s32 o58SlideX;
s32 o58RowsY;
s32 o58NextScreen;
s32 o58ScoreTickDelay;
s32 o58PathAlpha;
s32 o58ActiveState;
s32 o58PathFinished;
s32 gOverlay58PrimaryOrderIndex;
Overlay58RaceEntry *o58TimeOrder[6];
Overlay58RaceEntry *o58RankOrder[6];
s32 o58TimeTies[6];
s32 o58RankTies[6];
s32 o58MenuChoice;
s32 o58PathPending;
s32 gOverlay58ResourceReady;
ColourCycle o58Highlight;
s8 o58ActivePlayers[4];

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void overlay58SetNodeValue(s32 id, s32 argument, f32 value);
static void func_overlay_058_F000138C_18B0574(s32 ticks);
static void overlay58DrawSegmentStrip(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 limit);
static void overlay58DrawPointQuad(s32 x, s32 y, s32 z);
static void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z);
static void overlay58RefreshRankSet(void);
static void overlay58EnsureResource(void);
static void overlay58FinalizePackedStatus(void);

/* Matched 2026-10-02 (lane x-o058) by rewriting the inherited shape, not by
 * any allocator work: 295 -> 0 masked words at size delta 0, frame 0x60.
 *
 * The previous candidate walked hand-made cursors over alias extern names
 * (D_7C for D_78 + 4, D_94, D_AC, D_C4, a separate "end" symbol for the tag
 * table) and declared nineteen locals.  The function is the matched whale's
 * vocabulary instead: every table is indexed by `i` and strength reduction
 * makes the cursors, the entry count is read from its global at each loop
 * test, and the entry type is the whale's race entry (so the 0x1C byte
 * increment is `counters[class]++`).  That one rewrite is 295 -> 77.
 *
 * The last 77 were one local: the rank-gap clamp reuses `swapped` instead of
 * declaring its own.  A separate `gap` costs two frame cells (0x68 against
 * 0x60) and leaves the constant 10 for initColourCycle unhoisted, which moves
 * the gap into v1 and the table base into a1.  -Wo,-loopunroll,0 was not
 * load-bearing (identical object either way) and is removed. */  /* (was on typedef RcpTextureInfo) */
/* Tier D: stride and the one field read here. */  /* (was on typedef Overlay58PlayerSlot) */
void func_overlay_058_F0000000_18AF1E8(void) {
    s32 i;
    s32 swapped;
    s32 minutesA;
    s32 minutesB;
    s32 secondsA;
    s32 secondsB;
    s32 centisA;
    s32 centisB;
    Overlay58RaceState *state;
    Overlay58RaceEntry *entry;

    state = func_80028F54();
    loadFrontEndList(D_o058_5AE8);
    D_o058_5B40[0].texture = D_800D31C8_o058Reloc[3];
    D_o058_5B60[0].texture = D_800D31C8_o058Reloc[4];
    D_o058_5B80[0].texture = D_800D31C8_o058Reloc[5];
    D_o058_5BA0[0].texture = D_800D31C8_o058Reloc[0x7B];
    o58PathState_o058Reloc = 0;
    joyResetMap();
    for (i = 0; i < 4; i++) {
        o58ActivePlayers_o058Reloc[i] = D_800D3058_o058Reloc[i].enabled;
    }
    o58ActiveState_o058Reloc = state->active;
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        o58TimeOrder_o058Reloc[i] = &state->entries[i];
        o58RankOrder_o058Reloc[i] = &state->entries[i];
    }
    if (state->mode == 5) {
        do {
            swapped = 0;
            for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
                if (o58TimeOrder_o058Reloc[i]->value < o58TimeOrder_o058Reloc[i + 1]->value) {
                    entry = o58TimeOrder_o058Reloc[i];
                    o58TimeOrder_o058Reloc[i] = o58TimeOrder_o058Reloc[i + 1];
                    o58TimeOrder_o058Reloc[i + 1] = entry;
                    swapped = 1;
                }
            }
        } while (swapped);
    } else {
        do {
            swapped = 0;
            for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
                if (o58TimeOrder_o058Reloc[i]->value > o58TimeOrder_o058Reloc[i + 1]->value) {
                    entry = o58TimeOrder_o058Reloc[i];
                    o58TimeOrder_o058Reloc[i] = o58TimeOrder_o058Reloc[i + 1];
                    o58TimeOrder_o058Reloc[i + 1] = entry;
                    swapped = 1;
                }
            }
        } while (swapped);
    }
    o58TimeTies_o058Reloc[0] = 0;
    for (i = 1; i < D_8007BEF8_o058Reloc; i++) {
        overlay56SplitTime(o58TimeOrder_o058Reloc[i - 1]->value, &minutesA, &secondsA, &centisA);
        overlay56SplitTime(o58TimeOrder_o058Reloc[i]->value, &minutesB, &secondsB, &centisB);
        if ((minutesA == minutesB) && (secondsA == secondsB) && (centisA == centisB)) {
            o58TimeTies_o058Reloc[i] = o58TimeTies_o058Reloc[i - 1];
        } else {
            o58TimeTies_o058Reloc[i] = i;
        }
    }
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        if (state->entries == o58TimeOrder_o058Reloc[i]) {
            gOverlay58PrimaryOrderIndex_o058Reloc = i;
            i = 6;
        }
    }
    if (((gOverlay58PrimaryOrderIndex_o058Reloc < 4) || (D_8007BF0C_o058Reloc != 0)) && (state->mode != 1)) {
        for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
            o58TimeOrder_o058Reloc[i]->counters[o58TimeTies_o058Reloc[i]]++;
            o58TimeOrder_o058Reloc[i]->rank += D_o058_5B28[o58TimeTies_o058Reloc[i]];
            if (o58TimeOrder_o058Reloc[i]->rank > 9999) {
                o58TimeOrder_o058Reloc[i]->rank = 9999;
            }
        }
    } else if ((state->active != 0) && !(D_8007BF1C_o058Reloc & 0x100)) {
        state->countdown--;
    }
    do {
        swapped = 0;
        for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
            if (o58RankOrder_o058Reloc[i]->rank < o58RankOrder_o058Reloc[i + 1]->rank) {
                entry = o58RankOrder_o058Reloc[i];
                o58RankOrder_o058Reloc[i] = o58RankOrder_o058Reloc[i + 1];
                o58RankOrder_o058Reloc[i + 1] = entry;
                swapped = 1;
            }
        }
    } while (swapped);
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        swapped = o58RankOrder_o058Reloc[0]->rank - o58RankOrder_o058Reloc[i]->rank;
        if (swapped > 100) {
            swapped = 100;
        }
        o58RankOrder_o058Reloc[i]->gap = swapped;
    }
    o58RankTies_o058Reloc[0] = 0;
    for (i = 1; i < D_8007BEF8_o058Reloc; i++) {
        if (o58RankOrder_o058Reloc[i - 1]->rank == o58RankOrder_o058Reloc[i]->rank) {
            o58RankTies_o058Reloc[i] = o58RankTies_o058Reloc[i - 1];
        } else {
            o58RankTies_o058Reloc[i] = i;
        }
    }
    o58MenuChoice_o058Reloc = 0;
    initColourCycle(&o58Highlight_o058Reloc, 10);
    o58SelectedAlphabetColumn_o058Reloc = 0;
    if (D_8007C1B4_o058Reloc == 3) {
        o58SelectedAlphabetRow_o058Reloc = 3;
    } else {
        o58SelectedAlphabetRow_o058Reloc = 0;
    }
    if (state->mode != 5) {
        o58HeadingText_o058Reloc[0] = D_8007C0B8_o058Reloc->text[0x24];
    } else {
        o58HeadingText_o058Reloc[0] = D_8007C0B8_o058Reloc->text[0x70];
    }
    o58HeadingText_o058Reloc[1] = D_8007C0B8_o058Reloc->text[0x25];
    o58HeadingText_o058Reloc[2] = D_8007C0B8_o058Reloc->text[0x26];
    o58HeadingText_o058Reloc[3] = D_8007C0B8_o058Reloc->text[0x14];
    overlay58FinalizePackedStatus();
    o58PathPending_o058Reloc = 1;
    gOverlay58ResourceReady_o058Reloc = 0;
    D_8007BF6C_o058Reloc++;
}

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay58ReleaseResources(void) {
    void *resource;

    overlay58ReleaseReloc(gOverlay58InlineResource);
    resource = gOverlay58OptionalResource;
    if (resource != NULL) {
        overlay58ReleaseReloc(resource);
    }
}

/*
 * Mickey-only reconstruction (no permitted donor counterpart).  Exact.
 *
 * What the shipped code fixes about the source, in the order it was found:
 *   - Plain `-O2`: the first `jal` carries the parameter home store in its
 *     delay slot, which `-g3` never schedules.
 *   - `o58HeadingX_o058Reloc = 0` is stored in both arms of the case-4 mode test.
 *   - The menu bits and the screen-mode nibble are one object (one ROM-table
 *     symbol, addends 0 and 0x13).
 *   - Case 3 reads its own resident table (ROM-table symbol 0x754), not the
 *     one the drawing loop, case 5 and the level calls read (symbol 0x666);
 *     the `= -1` stores in cases 0 and 3 go to overlay BSS +0x0
 *     (`o58ScoreDeltas_o058Reloc[0]`), not to the camera mode global.
 *   - The progress float (data +0x2C0) is read under two names.  The two
 *     compares (`< 1.0f` and `1.0f ==`) are not the same uopt variable as the
 *     strip argument, the `+=` and the clamp: the shipped code reloads the
 *     strip argument after the join instead of carrying the compare's read
 *     around the sound call, and the address is never hoisted into a ninth
 *     saved register (four references, not six).  The compares therefore use
 *     the module-offset spelling `D_o058_5CE0` for the same word.  Written
 *     through a pointer local instead, the `&` and the `==` take the wrong
 *     operand order and the frame gains a cell.
 *   - `increment = 0.02f; increment *= updateRate;` loads the literal before
 *     the conversion and keeps it the multiply's left operand; one
 *     expression is canonicalised the other way round in every spelling.
 *   - `1.0f == progress`, constant first, is the shipped `c.eq.s` order.
 *   - The large point quad's x and z are `s32` locals assigned before the
 *     call, vertex first (`vertex + offset`), so x and z are complete before
 *     y is read and the offset conversion is the add's left operand; the
 *     offsets are one flat `s16` table indexed `player * 2` and
 *     `player * 2 + 1`.
 *   - The frame is a declaration count: 40 bytes of locals, status third.
 *     The mapping scan is an indexed `for` over `D_1A0` using `marker` as
 *     its index (uopt creates the cursor), so there is no cursor local, and
 *     start/end are an `s16` pair.
 */
void func_overlay_058_F00005FC_18AF7E4(s32 updateRate) {
    Overlay58AnimPath *path;
    Overlay58PathGeometry *geometry;
    Overlay58Status *status;
    Overlay58Gfx *command;
    s16 start;
    s16 end;
    s32 marker;
    s32 stage;
    f32 increment;
    s32 x;
    s32 z;

    status = func_80028F54();
    o58PathFinished_o058Reloc = 0;
    switch (o58PathState_o058Reloc) {
    case 0:
        o58Animation_o058Reloc.word = 0;
        path = (Overlay58AnimPath *)func_800508B4(o58Animation_o058Reloc.bytes[3]);
        if (path != 0) {
            animseqStartPath(o58Animation_o058Reloc.bytes[3]);
            path->flags |= 2;
        }
        o58PathState_o058Reloc = 4;
        if (status->mode == 1) {
            overlay58RefreshRankSet();
            overlay58EnsureResource();
            o58ScreenState_o058Reloc = 8;
        } else if (status->mode == 5) {
            o58ScreenState_o058Reloc = 2;
            o58ScoreDeltas_o058Reloc[0] = -1;
        } else {
            o58ScreenState_o058Reloc = 1;
        }
        break;
    case 4:
        if (D_2BC != 0) {
            amSndStop(D_2BC);
        }
        o58PathAlpha_o058Reloc -= updateRate * 4;
        if (overlay41IsUnitScale(o58Animation_o058Reloc.word) != 0) {
            o58PathState_o058Reloc = 1;
            amSndPlay(0x1FA, 0);
            if (status->mode == 5) {
                o58PrimaryX_o058Reloc = 0;
                o58RankX_o058Reloc = 0;
                o58ScoreX_o058Reloc = 0x140;
                o58HeadingX_o058Reloc = 0;
            } else {
                o58PrimaryX_o058Reloc = 0x140;
                o58ScoreX_o058Reloc = 0;
                o58HeadingX_o058Reloc = 0;
            }
            o58SlideX_o058Reloc = 0;
            o58RowsY_o058Reloc = 0;
            o58NextScreen_o058Reloc = 0;
        }
        break;
    case 1:
        o58PathAlpha_o058Reloc -= updateRate * 4;
        func_overlay_058_F000138C_18B0574(updateRate);
        break;
    case 2:
        if (o58PathDelay_o058Reloc > 0) {
            o58PathDelay_o058Reloc -= updateRate;
            if (o58PathDelay_o058Reloc <= 0) {
                overlay58SetNodeValue(3, 0, 0.02f);
            }
        } else if (o58PathDelay_o058Reloc >= 0) {
            o58PathDelay_o058Reloc -= updateRate;
            if (o58PathDelay_o058Reloc < 0) {
                amSndPlay(0x32A, 0);
            }
        }
        if (overlay41IsUnitScale(o58Animation_o058Reloc.word) != 0) {
            o58PathState_o058Reloc = 3;
            o58PathAlpha_o058Reloc = 0;
        }
        break;
    case 3:
        o58PathAlpha_o058Reloc += updateRate * 4;
        if (o58PathAlpha_o058Reloc >= 0xFF) {
            o58PathAlpha_o058Reloc = 0xFE;
        }
        if (D_2B0 == 0) {
            if (D_800D31B8 & 0x4000) {
                if (D_2BC != 0) {
                    amSndStop(D_2BC);
                }
                amSndPlay(0xD, 0);
                animseqStopPath(o58Animation_o058Reloc.bytes[3]);
                o58Animation_o058Reloc.word = 2;
                path = (Overlay58AnimPath *)func_800508B4(o58Animation_o058Reloc.bytes[3]);
                if (path != 0) {
                    func_8005055C(o58Animation_o058Reloc.bytes[3]);
                    animseqStartPath(o58Animation_o058Reloc.bytes[3]);
                    path->flags |= 2;
                }
                o58PathState_o058Reloc = 4;
                overlay58SetNodeValue(3, 0, -0.01f);
                if (status->mode == 1) {
                    overlay58RefreshRankSet();
                    overlay58EnsureResource();
                    o58ScreenState_o058Reloc = 8;
                } else if (status->mode == 5) {
                    o58ScreenState_o058Reloc = 2;
                    o58ScoreDeltas_o058Reloc[0] = -1;
                } else {
                    o58ScreenState_o058Reloc = 1;
                }
            } else if (D_800D31B8 & 0x9000) {
                if (D_2BC != 0) {
                    amSndStop(D_2BC);
                }
                if (D_8007BF0C == 0) {
                    if (status->active < 3) {
                        if (gOverlay58UnlockTableReloc[status->player][status->active + 1] != -1) {
                            if (!(((Overlay58Options *)D_800D3128)->bits13 &
                                  (1 << gOverlay58UnlockTableReloc[status->player][status->active + 1]))) {
                                ((Overlay58Options *)D_800D3128)->bits13 |=
                                    1 << gOverlay58UnlockTableReloc[status->player][status->active + 1];
                                func_800291B4();
                                func_8003A680(
                                    gOverlay58UnlockTableReloc[status->player][status->active + 1] +
                                    0xE);
                            }
                        }
                    }
                    if (D_8007BF44 > 0) {
                        o58PathState_o058Reloc = 5;
                        animseqStopPath(o58Animation_o058Reloc.bytes[3]);
                        o58Animation_o058Reloc.word = 9;
                        path = (Overlay58AnimPath *)func_800508B4(o58Animation_o058Reloc.bytes[3]);
                        if (path != 0) {
                            func_8005055C(o58Animation_o058Reloc.bytes[3]);
                            animseqStartPath(o58Animation_o058Reloc.bytes[3]);
                            path->flags |= 2;
                        }
                    }
                }
                if (o58PathState_o058Reloc != 5) {
                    o58PathFinished_o058Reloc = 1;
                    D_2B4 = 1;
                }
            }
        }
        break;
    case 5:
        if (overlay41IsUnitScale(o58Animation_o058Reloc.word) != 0) {
            status->active++;
            if (status->active == 4) {
                D_8007BF20 = o58RankOrder_o058Reloc[0]->character;
                D_8007BF24 = o58RankOrder_o058Reloc[1]->character;
                D_8007BF28 = o58RankOrder_o058Reloc[2]->character;
                D_8007BF2C = status->player;
                gOverlay58LevelReloc = 0x24;
                gOverlay58TrackReloc = 0;
                gOverlay58VehicleReloc = 8;
                gOverlay58TransitionModeReloc = 1;
            } else {
                D_8007BEF8 = 6;
                D_8007BEFC = 5;
                joyCreateMap(o58ActivePlayers_o058Reloc);
                gOverlay58LevelReloc =
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active];
                gOverlay58TrackReloc = status->value4;
                gOverlay58VehicleReloc = 5;
                gOverlay58TransitionModeReloc = 0;
            }
            if (o58PathPending_o058Reloc != 0) {
                mainChangeLevel(1, 0, 0, 0xF, 1, 0);
                o58PathPending_o058Reloc = 0;
            }
            o58PathState_o058Reloc = 6;
            o58PathAlpha_o058Reloc = 0;
        }
        break;
    case 6:
        break;
    }

    if (o58PathAlpha_o058Reloc < 0) {
        o58PathAlpha_o058Reloc = 0;
    }
    if ((o58PathFinished_o058Reloc != 0) && (D_2B0 == 0)) {
        if (D_2B4 != 0) {
            status->active++;
        }
        if (status->active == 4) {
            if (D_8007BF0C != 0) {
                if (o58PathPending_o058Reloc != 0) {
                    mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                    o58PathPending_o058Reloc = 0;
                }
            } else {
                D_8007BF20 = o58RankOrder_o058Reloc[0]->character;
                D_8007BF24 = o58RankOrder_o058Reloc[1]->character;
                D_8007BF28 = o58RankOrder_o058Reloc[2]->character;
                D_8007BF2C = status->player;
                if (o58PathPending_o058Reloc != 0) {
                    mainChangeLevel(0x24, 0, 0, 8, 1, 0);
                    o58PathPending_o058Reloc = 0;
                }
            }
        } else if (D_8007BF0C != 0) {
            mainChangeCameras(D_8007C1A0);
            if (((D_8007C1A0 == 2) ||
                 (D_8007C1A0 == 3)) &&
                (gOverlay58CameraGateReloc != 0)) {
                D_8007BEFC = 4 - D_8007C1A0;
            } else {
                D_8007BEFC = 0;
            }
            joyCreateMap(o58ActivePlayers_o058Reloc);
            if (o58PathPending_o058Reloc != 0) {
                mainChangeLevel(
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active],
                    status->value4, 0, 5, 1, 0);
                o58PathPending_o058Reloc = 0;
            }
        } else {
            D_8007BEF8 = 6;
            D_8007BEFC = 5;
            joyCreateMap(o58ActivePlayers_o058Reloc);
            if (o58PathPending_o058Reloc != 0) {
                mainChangeLevel(
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active],
                    status->value4, 0, 5, 1, 0);
                o58PathPending_o058Reloc = 0;
            }
        }
        amSndPlay(0xC, 0);
        D_2B0 = 1;
    }
    if (o58PathAlpha_o058Reloc < 0) {
        o58PathAlpha_o058Reloc = 0;
    }

    path = (Overlay58AnimPath *)func_800508B4(3);
    if ((path != 0) && (o58PathAlpha_o058Reloc > 0)) {
        geometry = *path->object->geometry;
        camSetView(&D_800D3140,
                      &D_800D3144);
        command = D_800D3140++;
        command->w0 = 0xFA000000;
        command->w1 = (o58PathAlpha_o058Reloc & 0xFF) | ~0xFF;
        for (stage = 0; stage <= o58ActiveState_o058Reloc; stage++) {
            start = 0;
            end = 0;
            for (marker = 0; D_1A0[marker] != -1; marker += 2) {
                if (D_1A0[marker] ==
                    gOverlay58SelectionTableReloc[status->player][stage]) {
                    start = D_1A0[marker + 1];
                }
                if (stage == 3) {
                    end = 0x13;
                } else if (D_1A0[marker] ==
                           gOverlay58SelectionTableReloc[status->player]
                                                        [stage + 1]) {
                    end = D_1A0[marker + 1];
                }
            }

            if (stage == o58ActiveState_o058Reloc) {
                increment = 0.02f;
                increment *= updateRate;
                if ((D_2BC == 0) & (D_o058_5CE0 < 1.0f)) {
                    if (o58PathState_o058Reloc == 3) {
                        amSndPlay(0x32B, &D_2BC, o58ActiveState_o058Reloc);
                    }
                }
                overlay58DrawSegmentStrip(
                    geometry->vertices[start].x, geometry->vertices[start].y,
                    geometry->vertices[start].z, geometry->vertices[end].x,
                    geometry->vertices[end].y, geometry->vertices[end].z,
                    D_2C0);
                D_2C0 += increment;
                if (D_2C0 > 1.0f) {
                    D_2C0 = 1.0f;
                    if (D_2BC != 0) {
                        amSndStop(D_2BC);
                    }
                    overlay58DrawPointQuad((s32)geometry->vertices[end].x,
                                           (s32)geometry->vertices[end].y,
                                           (s32)geometry->vertices[end].z);
                }
            } else {
                overlay58DrawSegmentStrip(
                    geometry->vertices[start].x, geometry->vertices[start].y,
                    geometry->vertices[start].z, geometry->vertices[end].x,
                    geometry->vertices[end].y, geometry->vertices[end].z,
                    1.0f);
            }

            if (!(D_2A8[status->player] &
                  ((((Overlay58Options *)D_800D3128)->flags << 5) >> 0x1C))) {
                marker = -1;
                if (status->player == 0) {
                    if (stage == 0) {
                        marker = start;
                    }
                } else if ((status->player > 0) && (status->player < 4)) {
                    if (stage == 3) {
                        marker = start;
                    } else if ((o58ActiveState_o058Reloc == 2) && (stage == 2) &&
                               (1.0f == D_o058_5CE0)) {
                        marker = end;
                    }
                }
                if (marker != -1) {
                    x = geometry->vertices[marker].x +
                        D_B8[status->player * 2];
                    z = geometry->vertices[marker].z +
                        D_B8[status->player * 2 + 1];
                    overlay58DrawLargePointQuad(
                        x, (s32)geometry->vertices[marker].y, z);
                }
            }
            overlay58DrawPointQuad((s32)geometry->vertices[start].x,
                                   (s32)geometry->vertices[start].y,
                                   (s32)geometry->vertices[start].z);
        }
    }
}

/* DKR v77/v80 and JFG have no exact donor for this node-update routine. */
static void overlay58SetNodeValue(s32 id, s32 argument, f32 value) {
    Overlay58Node *node;

    node = (Overlay58Node *)func_800508B4(id);
    if ((node != NULL) && (node->target != NULL)) {
        if (value < 0.0f) {
            func_8005AD64(node->target, argument, -1,
                                        node->target->parameter);
        } else {
            func_8005AD64(node->target, argument, -1, 0.0f);
        }
        node->value = value;
        node->active = 0;
    }
}

/* Matched reconstruction from Mickey's runtime relocations (Tier A, 2026-09-16,
 * lane wv-y: 3,614 of 3,614 words at frame 0x138 and 1,253 relocations).
 * Resident identities use the ROM-table exports and reserved section bases;
 * D_o058 names identify byte offsets in overlay 58, including its BSS.
 */
/*
 * Plateau (2026-09-10): 733 of 3,614 relocation-masked words differ, down from
 * 894.  Size delta 0, frame 0x138.  Under a shape-tolerant alignment 3,034
 * rows are byte-exact (up from 2,874) and 631 of 848 saved-register operand
 * slots agree (up from 434); the s7-with-s8 transposition that three grinds
 * named is GONE -- 0 transposed slots against 187.  Report all three numbers
 * for anything measured here: at 894 words the positional count is dominated
 * by displacement and does not order configurations on its own.
 *
 * The residual is p1 colour, not shape.  Every one of this procedure's 428
 * allocator decisions is phase one, so declaration order and definition
 * position are the wrong axis for a COLOUR (165 earlier forms were flat for
 * that reason).  **Statement order is not** -- see note 7 below; the phase
 * census retires L106 and nothing else, and statement order reaches as1's
 * scheduling and ugen's emission order, which the census does not touch.  It
 * is worth 733 -> 688 here on its own.  For the colour the axis is L100's
 * ratio,
 * `save = totalsave / nocs`, and there are two source dials on it:
 *
 *   1. Discarded-expression probes at loop depth 2 (three of them in case 9's
 *      grid inner loop, below).  100 of totalsave each, no instructions.
 *   2. WHICH DECLARED SCALAR PLAYS A LOOP'S INDEX.  A loop whose index is a
 *      carrier of its own takes its occurrences out of `i`'s web, and the
 *      target's own shape says it did this: it emits no `move <saved>,zero`
 *      in either the case-2 decrement loop's preheader or case 8's, which is
 *      what a loop whose index dies at strength reduction looks like.  Case 2
 *      is carried by `savedPosition` and case 8 by `opponent`; both are dead
 *      in their case, so the rename carries no meaning.  Worth 894 -> 755 and
 *      755 -> 733 respectively, and 197 saved-register slots between them.
 *
 * Still open, largest first: 119 words in case 9's grid-loop exit test (the
 * target tests `slti $at,<saved>,10` and the natural `opponent < 0xA` puts 10
 * in a saved register instead -- a second p1 contest, this one between the
 * constant's web and a global address web, and NOT reachable by respelling
 * either loop's bound: seven erase-loop spellings crossed with three grid
 * spellings are all exactly inert); 72 words in case 8; 40 in case 12; 34 in
 * case 1; 34 in case 2's draw loop; 29 in case 3; 22 at the case 1/2 boundary.
 *
 * Falsified: spelling a frame address as a cached pointer local (`char
 * *textPtr = &text[0];`, and the same for `nodes` and `character`) does not
 * reproduce the target's cached form -- all three measure one word WORSE at
 * unchanged size and frame, and caching `&character[0]` for the +0x3704 site
 * is byte-identical to not doing it.
 *
 *  7.  STATEMENT ORDER, 2026-09-12 (lane p11-big).  The claim above that
 *      statement order is "the wrong axis" is false as stated, and it had
 *      been closing the largest untried dimension on this function.  A
 *      move-one hill climb over EVERY run of three or more consecutive
 *      single-line statements in the body -- 67 such runs, each climbed to a
 *      local optimum and the whole sweep then re-run twice to a fixed point --
 *      takes 733 -> 688 masked at size delta 0, byte-exact 3120 -> 3144,
 *      register naming 379 -> 371, immediate 25 -> 24 and really different
 *      103 -> 88, with both frame ladders unchanged.  Five orders moved:
 *      `rowY = rowBase;` to the top of case 1, `i = 0;` after `x = ...` in
 *      case 1, `columnStep` after `i = 0;` in case 5, `nodes[0].texture` to
 *      the head of its store group in the portrait loop, `cursor++` after
 *      `textY += 0x1E;` in case 6, and `nodes[0].y` before `nodes[0].x` in
 *      case 12.  672 is reachable but only through three moves that are
 *      SEMANTICALLY WRONG -- two put `nodes[1].texture = 0;` after the
 *      func_8002F618 call that walks the node list to its NULL terminator,
 *      and one moves `x = -x;` in front of a call that takes `x + 0xA0` as an
 *      argument.  They are rejected; 688 is the safe move-one optimum, and
 *      re-climbing from it finds only those same three moves again.
 *
 *  8.  THE COUNTED-LOOP EXIT REWRITE, 2026-09-12 (lane p12-whale).  688 ->
 *      634 at size delta 0, byte-exact 3144 -> 3260, naming 371 -> 247,
 *      immediate 24 -> 31, really different 88 -> 89, frame 0x138 both sides.
 *      uopt rewrites a provably counted loop's `< CONST` exit test into `!=`
 *      against a constant hoisted into a register, and it does so
 *      UNCONDITIONALLY at this loop's depth -- measured at bounds 0x9, 0xA,
 *      0xB and 0x40, and still with every other `0xA` in the procedure
 *      removed.  So no spelling of `opponent < 0xA` reaches the target's
 *      `slti $at,<index>,10`: `< 0xA`, `<= 9`, `< 10`, `!(>= 0xA)`,
 *      `0xA > opponent`, `< 0x000A`, `< 5 + 5`, `< 0xAL`, `< (s32) 0xAU` and
 *      `< 0xA && 1` all compile to ONE object, and so do the erase loop's
 *      `!= 0xA`, `< 0xA` and `<= 9`.  What the rewrite needs is a known
 *      initial value, so an OPAQUE ZERO retires it at no instruction cost.
 *      The old `(opponent - 1) < (0xA - 1)` bought the same `slti` by
 *      comparing a derived value, but its `addiu` spent one temp-ring draw
 *      and rotated the whole free list for the remaining 1,196 bytes: a
 *      closed nine-cycle t1->t5->t2->t6->t7->t8->t3->t4->t9->t1 carrying 150
 *      of the 212 differing register slots after +0x3148, and none before it.
 *
 *      The statement-order climb was re-run from the new shape (L146: an
 *      order optimum belongs to the shape) and found 634 -> 623 in two
 *      moves, both in case 10's node fill: `nodes[0].texture` to the head of
 *      the store group, and `nodes[0].x` before `nodes[0].y`.  No call sits
 *      between them and the NULL terminator is still stored before
 *      func_8002F618.  An opaque zero at each of the other 17 index resets
 *      x four carriers (69 cells) is inert or worse everywhere: the lever is
 *      site-specific to this one loop.
 *
 * The `if (i != 0);` statements below are discarded-expression probes
 * (ido-5.3 L37) -- zero instructions, one web occurrence each.  They were
 * found by a two-pass climb over 11,304 variants and are a local optimum;
 * a further 144-variant sweep (one probe at the top of each case body, for
 * twelve different locals) found every size-preserving one inert.
 */

/* Runtime relocation evidence, not a resident-address naming inference:
 * all six halfword reads bind the same reserved-selector identity.
 * The former D_800D3050 spelling therefore shares this import; it does
 * not denote a second object. Each of the six owned HI/LO pairs agrees
 * in selector and base, with a zero candidate addend.
 * Coalescing the two spellings preserves the compiled sections and the
 * relocation map after the proved import renames. The older inference
 * of distinct storage from compiler behavior is superseded.
 * The _o058Reloc imports keep reserved runtime identities separate from
 * resident-address names. See the handoff for the scoped proof. */  /* (was on extern D_o058_5BA0) */
static void func_overlay_058_F000138C_18B0574(s32 ticks) {
    s32 i;
    s32 opponent;
    s32 textY;
    s32 columnX;
    s32 x;
    char **cursor;   /* unused: one frame cell, exactly as the scalar it replaced (wv-x) */
    s32 portraitX;   /* unused: the loop indices below ride on letter1 and textY (wv-v) */
    s32 columnStep;
    s32 portraitIndex;
    s32 countdownX;
    s32 seconds;
    s32 minutes;
    s32 centiseconds;
    s32 rowY;
    s32 highlighted;
    s32 rowHeight;
    s32 letter0;
    s32 letter1;
    s32 savedX;
    s32 savedPosition;
    s32 savedOffset;
    s32 rowBase;
    s32 columnCount;
    char character[2];
    char text[24];
    s32 erase;
    Overlay58RaceState *state;
    RcpTextureNode nodes[4];
    AnimPath *path;
    SavesSlot *saves;
    SavesSlot *slot;

    state = func_80028F54();
    updateColourCycle(&o58Highlight_o058Reloc, ticks);
    fontUseFont(0);
    switch (o58ScreenState_o058Reloc) {
    case 1:
    case 2:
        if ((o58ScoreDeltas_o058Reloc[0] == -1) && (D_8007BEF8_o058Reloc > 0)) {
            /* `letter1`, the same name case 2's decrement and draw loops
             * subscript with: uopt keeps one web per IR name (L131), so the
             * o58ScoreDeltas_o058Reloc cursor spans the draw loop's calls and takes s0 on
             * its own, as the target has it in all three loops.  Any other
             * index here leaves this loop's cursor in a0 (wv-r).
             * WHY letter1 (wv-v): the index's seven folded resets are dead
             * defs that uopt still colours, as one web per symbol.  A
             * symbol of its own (portraitX) takes a0, the lowest free
             * colour, and a0 here collides with o58ScreenState_o058Reloc's value web,
             * which cascades a1->a0/a2->a1/a3->a2 through the function.
             * Written on a local that is dead in cases 1/2 and already
             * coloured s1 by case 10 (letter1, save 10.0), the resets join
             * that web and take s1, which nothing at blocks 9/17/85/93
             * wants.  The case-1 draw loop must share the name (the
             * o58TimeTies_o058Reloc address web spans entry + both draw loops), and
             * letter1 is dead throughout it; textY is not (it is case 1's
             * text y), so cases 3/13 ride on textY and cases 1/2 on letter1.
             * s0 carriers (letter0, portraitIndex, countdownX) flip their
             * own case; savedPosition and erase cost width. */
            letter1 = 0;
            do {
                o58ScoreDeltas_o058Reloc[letter1] = D_o058_5B28[o58TimeTies_o058Reloc[letter1]];
                letter1++;
            } while (letter1 < D_8007BEF8_o058Reloc);
        }
        break;
    }

    rowHeight = D_o058_5C74[D_8007BEF8_o058Reloc - 1];
    rowBase = D_o058_5C68[D_8007BEF8_o058Reloc - 1];
    switch (o58ScreenState_o058Reloc) {
    case 1:
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0))); /* Keep reset in the last argument value. */

        fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58HeadingX_o058Reloc + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x27], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        x = o58PrimaryX_o058Reloc + o58SlideX_o058Reloc;
        letter1 = 0; rowY = rowBase;
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            do {
                x = -x;
                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + o58TimeOrder_o058Reloc[letter1]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = x + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                fontPrintXY(&D_800D3140_o058Reloc, x + 0x28, rowY, D_o058_5C98[o58TimeTies_o058Reloc[letter1]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == o58TimeOrder_o058Reloc[letter1])) {
                    fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                fontPrintXY(&D_800D3140_o058Reloc, x + 0x71, rowY, D_8007C0B8_o058Reloc->text[o58TimeOrder_o058Reloc[letter1]->character + 0x1A], 0);
                if (state->mode == 5) {
                    if (i == 0) {
                        overlay56SplitTime(o58TimeOrder_o058Reloc[letter1]->value, &minutes, &seconds, &centiseconds);
                    } else {
                        overlay56SplitTime(o58TimeOrder_o058Reloc[0]->value - o58TimeOrder_o058Reloc[letter1]->value, &minutes, &seconds, &centiseconds);
                    }
                } else if (i == 0) {
                    overlay56SplitTime(o58TimeOrder_o058Reloc[letter1]->value, &minutes, &seconds, &centiseconds);
                } else {
                    overlay56SplitTime(o58TimeOrder_o058Reloc[letter1]->value - o58TimeOrder_o058Reloc[0]->value, &minutes, &seconds, &centiseconds);
                }
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                textY = rowY + o58RowsY_o058Reloc;
                if ((i == 0) || (minutes != 0)) {
                    sprintf(&text[0], D_o058_5D2C, minutes);
                    fontPrintXY(&D_800D3140_o058Reloc, x + 0xDC, textY, &text[0], 1);
                    fontPrintXY(&D_800D3140_o058Reloc, x + 0xDF, textY, D_o058_5D30, 0);
                }
                if (i != 0) {
                    if (state->mode == 5) {
                        fontPrintXY(&D_800D3140_o058Reloc, x + 0xBA, textY, D_o058_5D34, 0);
                    } else {
                        fontPrintXY(&D_800D3140_o058Reloc, x + 0xBA, textY, D_o058_5D38, 0);
                    }
                }
                sprintf(&text[0], D_o058_5D3C, seconds);
                fontPrintXY(&D_800D3140_o058Reloc, x + 0xE6, textY, &text[0], 0);
                fontPrintXY(&D_800D3140_o058Reloc, x + 0xFD, textY, D_o058_5D44, 0);
                sprintf(&text[0], D_o058_5D48, centiseconds);
                fontPrintXY(&D_800D3140_o058Reloc, x + 0x104, textY, &text[0], 0);
                letter1 += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        o58PrimaryX_o058Reloc -= ticks * 0xF;
        if (o58PrimaryX_o058Reloc < 0) {
            o58PrimaryX_o058Reloc = 0;
            if ((o58NextScreen_o058Reloc == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                if ((gOverlay58PrimaryOrderIndex_o058Reloc >= 4) && (D_8007BF0C_o058Reloc == 0)) {
                    if (state->countdown == 0) {
                        o58NextScreen_o058Reloc = 5;
                    } else {
                        o58NextScreen_o058Reloc = 6;
                    }
                } else {
                    *o58ScoreDeltas_o058Reloc = -1;
                    o58NextScreen_o058Reloc = 2;
                }
                amSndPlay(0xCU, NULL);
            }
            if (o58NextScreen_o058Reloc != 0) {
                o58HeadingX_o058Reloc += ticks * 0xF;
                if ((o58NextScreen_o058Reloc == 5) || (o58NextScreen_o058Reloc == 6)) {
                    o58SlideX_o058Reloc += ticks * 0xF;
                    if (o58SlideX_o058Reloc >= 0x141) {
                        o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                        amSndPlay(0x1FAU, NULL);
                        return;
                    }
                } else {
                    o58RowsY_o058Reloc += ticks * 0xF;

                    if ((o58RowsY_o058Reloc >= 0xB5) && (o58HeadingX_o058Reloc >= 0x141)) {
                        o58RankX_o058Reloc = 0x6E;
                        o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                        amSndPlay(0x1FAU, NULL);
                        o58NextScreen_o058Reloc = 0;
                        o58ScoreTickDelay_o058Reloc = 0x4B;
                        o58ScoreX_o058Reloc = 0;
                        return;
                    }
                }
            }
        }
        break;
    case 2:
        i = 0;
        if (*o58ScoreDeltas_o058Reloc > 0) {
            o58ScoreTickDelay_o058Reloc = o58ScoreTickDelay_o058Reloc - ticks;
            if (o58ScoreTickDelay_o058Reloc < 0) {
                o58ScoreTickDelay_o058Reloc = 0xF;
                amSndPlay(0x1BU, NULL);
                if ((s32) D_8007BEF8_o058Reloc > 0) {
                    /* Two inductions on purpose.  `letter1` subscripts and
                     * tests, so `o58ScoreDeltas_o058Reloc[letter1]` is the entry loop's
                     * and the draw loop's IR name (one web, callee-saved, s0);
                     * `i` is counted beside it so the post-loop `i = 0` is a
                     * real reset and survives.  Indexing by `i` alone makes
                     * the draw loop initialise its cursors from `i * 4` (two
                     * reaching definitions, no constant folding); any other
                     * lone index lets uopt delete the reset.  See
                     * docs/whale-role-carriers.md. */
                    i = 0;
                    letter1 = 0;
                    do {
                        if (o58ScoreDeltas_o058Reloc[letter1] > 0) {
                            o58ScoreDeltas_o058Reloc[letter1] -= 1;
                        }
                        letter1++;
                        i++;
                    } while (letter1 < D_8007BEF8_o058Reloc);
                    i = 0;
                }
            }
        }
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58ScoreX_o058Reloc + o58HeadingX_o058Reloc + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x28], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        savedX = o58PrimaryX_o058Reloc;
        savedOffset = o58ScoreX_o058Reloc;
        letter1 = 0; /* Independent array induction; visible i stays early. */
        rowY = rowBase;
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            do {
                o58PrimaryX_o058Reloc = -o58PrimaryX_o058Reloc;
                o58ScoreX_o058Reloc = -o58ScoreX_o058Reloc;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + o58TimeOrder_o058Reloc[letter1]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = o58PrimaryX_o058Reloc + o58ScoreX_o058Reloc + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58ScoreX_o058Reloc + 0x28, rowY, D_o058_5C98[o58TimeTies_o058Reloc[letter1]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == o58TimeOrder_o058Reloc[letter1])) {
                    fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58ScoreX_o058Reloc + 0x71, rowY, D_8007C0B8_o058Reloc->text[o58TimeOrder_o058Reloc[letter1]->character + 0x1A], 0);
                /* `columnX`, not `textY`: uopt colours a symbol as ONE web across
                 * every case (textY is 29 blocks over cases 1/2/8/9/10 and takes
                 * s4 early), and the target's rank difference here sits in s1 with
                 * the format-string address in s4 -- the colours of a web decided
                 * late, i.e. the variable that also carries column X in cases 9/12/13.
                 * Fourteen other carriers measured 151-3209; this one is -10 at delta 0. */
                columnX = o58TimeOrder_o058Reloc[letter1]->rank - o58ScoreDeltas_o058Reloc[letter1];
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                sprintf(&text[0], D_o058_5D50, columnX);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0xBE, rowY, &text[0], 0);
                sprintf(&text[0], D_o058_5D54, o58ScoreDeltas_o058Reloc[letter1]);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0xEB, rowY, &text[0], 0);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0x113, rowY, D_o058_5D5C, 0);
                letter1 += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        o58PrimaryX_o058Reloc = savedX;
        o58ScoreX_o058Reloc = savedOffset;
        o58RankX_o058Reloc -= ticks * 0xF;
        o58ScoreX_o058Reloc -= ticks * 0xF;
        o58HeadingX_o058Reloc -= ticks * 0xF;

        if ((o58RankX_o058Reloc < 0) && (o58ScoreX_o058Reloc < 0) && (o58HeadingX_o058Reloc < 0)) {
            if (o58NextScreen_o058Reloc == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    o58NextScreen_o058Reloc = 3;
                    amSndPlay(0xCU, NULL);
                } else if ((D_800D31B8_o058Reloc & 0x4000) && (state->mode != 5)) {
                    o58NextScreen_o058Reloc = 1;
                    amSndPlay(0xDU, NULL);
                }
            } else {
                o58PrimaryX_o058Reloc += ticks * 0xF;
                if (o58PrimaryX_o058Reloc >= 0x141) {
                    o58RowsY_o058Reloc = 0;
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    amSndPlay(0x1FAU, NULL);
                    o58NextScreen_o058Reloc = 0;
                    o58HeadingX_o058Reloc = 0;
                }
            }
        }
        if (o58RankX_o058Reloc < 0) {
            o58RankX_o058Reloc = 0;
        }
        if (o58ScoreX_o058Reloc < 0) {
            o58ScoreX_o058Reloc = 0;
        }
        if (o58HeadingX_o058Reloc < 0) {
            o58HeadingX_o058Reloc = 0;
            return;
        }
        break;
    case 3:
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x29], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        savedX = o58PrimaryX_o058Reloc;

        savedPosition = o58RankX_o058Reloc;
        textY = 0; /* the case-3/13 row index rides on textY (s4, dead here): see case 13 */
        /* Keep the row home in the nonempty block, beside its first use. */
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            rowY = rowBase; do {
                o58PrimaryX_o058Reloc = -o58PrimaryX_o058Reloc;
                o58RankX_o058Reloc = -o58RankX_o058Reloc;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + o58RankOrder_o058Reloc[textY]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0x28, rowY, D_o058_5C98[o58RankTies_o058Reloc[textY]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == o58RankOrder_o058Reloc[textY])) {
                    fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0x71, rowY, D_8007C0B8_o058Reloc->text[o58RankOrder_o058Reloc[textY]->character + 0x1A], 0);
                sprintf(&text[0], D_o058_5D60, o58RankOrder_o058Reloc[textY]->rank);
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0xD2, rowY, &text[0], 0);
                if (o58RankOrder_o058Reloc[textY]->rank == 1) {
                    fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0x104, rowY, D_o058_5D64, 0);
                } else {
                    fontPrintXY(&D_800D3140_o058Reloc, o58PrimaryX_o058Reloc + o58RankX_o058Reloc + 0x104, rowY, D_o058_5D68, 0);
                }
                textY += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        o58PrimaryX_o058Reloc = savedX;
        o58RankX_o058Reloc = savedPosition;

        o58PrimaryX_o058Reloc -= ticks * 0xF;
        if (o58PrimaryX_o058Reloc < 0) {
            o58PrimaryX_o058Reloc = 0;
            if (o58NextScreen_o058Reloc == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 0) {
                        o58NextScreen_o058Reloc = 4;
                    } else {
                        o58NextScreen_o058Reloc = 0xC;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    o58NextScreen_o058Reloc = 2;
                    *o58ScoreDeltas_o058Reloc = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                o58RankX_o058Reloc += ticks * 0xF;
                if (o58RankX_o058Reloc >= 0x141) {
                    if (o58NextScreen_o058Reloc == 4) {
                        animseqStopPath(o58Animation_o058Reloc.bytes[3]);
                        o58Animation_o058Reloc.word = 1;
                        path = func_800508B4(o58Animation_o058Reloc.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(o58Animation_o058Reloc.bytes[3]);
                            animseqStartPath(o58Animation_o058Reloc.bytes[3]);
                            path->flags |= 2;
                        }
                        o58PathState_o058Reloc = 2;
                        o58PathDelay_o058Reloc = 0x50;
                        return;
                    }
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    amSndPlay(0x1FAU, NULL);
                    o58NextScreen_o058Reloc = 0;
                    o58RankX_o058Reloc = 0;
                    o58ScoreX_o058Reloc = 0x140;
                    return;
                }
            }
        }
        break;
    case 12:
        /* The visible index is reset through the last title-colour argument
         * (the value-producing spelling wv-i found: the reset then lands in
         * the call's delay slot), as case 13 already does.
         *
         * `i &= 0` is the last fact of the match (wv-y).  The title loop
         * below subscripts D_o058_5C98 by `i`, and its cursor init folds to
         * the bare address only from a reset in the guard's own block, so
         * the loop is preceded by a second `i = 0` there.  uopt keeps that
         * second reset out of the code only as a redundant store, which
         * needs the delay-slot reset to still be live when dead stores are
         * removed -- and every plain read of `i` between the two resets is
         * folded to 0 first.  A self-reading def is what survives: it is
         * not a dead-store candidate, its read cannot be folded because it
         * precedes a def of the same variable in its block, and once the
         * liveness pass has run it is a store of the value `i` already
         * holds and is deleted.  Zero instructions.  Placed in any of
         * blocks 181-183, on its own line or the call's, and spelled
         * `i = i & 0`, the result is byte-identical; without it the
         * delay-slot reset dies and the guard reset is emitted (46).
         * Measured on a two-loop mini TU first; docs/whale-self-read.md. */
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        i &= 0;
        fontPrintXY(&D_800D3140_o058Reloc, o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x6E], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        /* Roles, not colours: `opponent` is s0 and `columnX` s1 in cases
         * 8-10, and the target keeps column X in s0 and the inner index in
         * s1 here and in case 13.  So `opponent` carries column X and
         * `columnX` the inner index; neither web changes colour (wv-r). */
        opponent = D_o058_5C80[D_8007BEF8_o058Reloc - 1] + o58RankX_o058Reloc + o58ScoreX_o058Reloc;
        columnStep = D_o058_5C8C[D_8007BEF8_o058Reloc - 1];
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            /* The title array is subscripted by `i`, so strength reduction
             * owns the cursor and the load carries its `.noalias` fact
             * (a walking pointer has none and the load stays above the
             * argument store: wv-x's six).  This reset gives the cursor
             * its constant init and is then deleted as redundant, so no
             * index web touches blocks 183/184/190: W (the `&o58ScoreX_o058Reloc`
             * case-12 piece) accepts the call block 185, rejects 191, keeps
             * 202 at margin 0 and colours t0, while the `&o58RankX_o058Reloc`
             * piece accepts 191 at margin 0 and rejects 202 (wv-t, wv-x).
             * Any dead carrier here is one web too many for both tests. */
            i = 0;
            do {
                fontPrintXY(&D_800D3140_o058Reloc, opponent, 0x37, D_o058_5C98[i], 4);
                i += 1;
                opponent += columnStep;
                if (columnStep != 0); /* +10 to the stride web: decided before the count (see case 13). */
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        savedPosition = o58RankX_o058Reloc;
        savedOffset = o58ScoreX_o058Reloc;
        i = 0;
        rowY = rowBase;
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            do {
                o58RankX_o058Reloc = -o58RankX_o058Reloc;
                o58ScoreX_o058Reloc = -o58ScoreX_o058Reloc;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + o58RankOrder_o058Reloc[(u32)i]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0x28;
                nodes[0].y = rowY + 0x12;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                columnX = 0;
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);

                opponent = D_o058_5C80[D_8007BEF8_o058Reloc - 1] + o58RankX_o058Reloc + o58ScoreX_o058Reloc;
                if ((s32) D_8007BEF8_o058Reloc > 0) {
                    do {
                        sprintf(&text[0], D_o058_5D6C, o58RankOrder_o058Reloc[(u32)i]->counters[columnX]);
                        fontPrintXY(&D_800D3140_o058Reloc, opponent, rowY + 0x16, &text[0], 4);
                        columnX += 1;
                        opponent += columnStep;
                    } while (columnX < (s32) D_8007BEF8_o058Reloc);
                }
                i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        o58RankX_o058Reloc = savedPosition;
        o58ScoreX_o058Reloc = savedOffset;

        o58ScoreX_o058Reloc -= ticks * 0xF;
        if (o58ScoreX_o058Reloc < 0) {
            /* An empty `if (columnCount != 0);` read stood here to keep the
             * `&o58ScoreX_o058Reloc` fragment from adjoining this block (-4 when it
             * was added).  On the current source shape it is byte-inert and
             * was removed (lane c-1, 2026-10-08). */
            o58ScoreX_o058Reloc = 0;
            if (o58NextScreen_o058Reloc == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 3) {
                        o58NextScreen_o058Reloc = 4;
                    } else {
                        o58NextScreen_o058Reloc = 7;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    o58NextScreen_o058Reloc = 3;
                    *o58ScoreDeltas_o058Reloc = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                o58RankX_o058Reloc += ticks * 0xF;
                if (o58RankX_o058Reloc >= 0x141) {
                    if (o58NextScreen_o058Reloc == 4) {
                        animseqStopPath(o58Animation_o058Reloc.bytes[3]);
                        o58Animation_o058Reloc.word = 1;
                        path = func_800508B4(o58Animation_o058Reloc.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(o58Animation_o058Reloc.bytes[3]);
                            animseqStartPath(o58Animation_o058Reloc.bytes[3]);
                            path->flags |= 2;
                        }
                        o58PathState_o058Reloc = 2;
                        o58PathDelay_o058Reloc = 0x50;
                        return;
                    }
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    amSndPlay(0x1FAU, NULL);
                    o58NextScreen_o058Reloc = 0;
                    o58PrimaryX_o058Reloc = 0x140;
                    o58RankX_o058Reloc = 0;
                    o58ScoreX_o058Reloc = 0x140;
                    return;
                }
            }
        }
        break;
    case 13:
        /* Initialize the visible index through the last title-colour argument. */
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        fontPrintXY(&D_800D3140_o058Reloc, o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x6F], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        if (D_8007BEF8_o058Reloc == 4) {
            columnCount = 4;
        } else {
            columnCount = D_8007BEF8_o058Reloc + 1;
        }

        opponent = D_o058_5C80[D_8007BEF8_o058Reloc - 1] + o58RankX_o058Reloc + o58ScoreX_o058Reloc;
        columnStep = D_o058_5C8C[columnCount - 1];
        if (columnCount > 0) {
            /* Loop 1 is indexed by the same variable that indexes case 3
             * and the row loop below, with the reset folded here in the
             * preheader (wv-u): that keeps the `&o58RankX_o058Reloc` a2 piece at 28
             * interferers at its 254 step, so the head lui/lw pair and the
             * transition keep their shared register.  `opponent` is the
             * coordinate (only its 99.4 beats the cursor's 15.5, so the
             * cursor lands s1 and the coordinate s0).  The variable is
             * `textY` because the reset web must not be a0/a1/a2: textY is
             * dead in cases 3 and 13, its web is decided early into s4 by
             * case 1, and s4 is harmless at 136/235/241 (wv-v; the forced
             * witnesses a3/t0/s4 all measured the same 11).  `cursor` stays
             * declared for its frame cell.  The subscript keeps the 0x51 so
             * the cursor register holds the bare symbol and the load spends
             * the 0x144 in its displacement, as the target does. */
            textY = 0;
            do {
                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + textY];
                nodes[0].alternate = NULL;
                nodes[0].x = opponent;
                /* Stride in `columnStep` (shared with case 12, s5) and count
                 * in `columnCount` (s6), as the target colours them.  The
                 * stride probe in case 12 keeps the stride above `i`.  A
                 * matching `if (columnCount != 0);` count probe stood here;
                 * on the current source shape it is byte-inert and was
                 * removed (lane c-1, 2026-10-08). */
                nodes[0].y = 0x37;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                i += 1;
                textY += 1;
                opponent += columnStep;
            /* `!=`, not `<`: with the cursor carrying the subscript, uopt
             * no longer normalises `i < columnCount` into the target's
             * `bne`, and the `<` spelling costs an extra slt.  The converse
             * of what the indexed form wanted; see the handoff. */
            } while (i != columnCount);
        }
        savedPosition = o58RankX_o058Reloc;
        savedOffset = o58ScoreX_o058Reloc;
        textY = 0; i = 0;
        rowY = rowBase;
        if ((s32) D_8007BEF8_o058Reloc > 0) {
            do {
                o58RankX_o058Reloc = -o58RankX_o058Reloc;
                o58ScoreX_o058Reloc = -o58ScoreX_o058Reloc;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + o58RankOrder_o058Reloc[textY]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = o58RankX_o058Reloc + o58ScoreX_o058Reloc + 0x28;
                nodes[0].y = rowY + 0x12;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                columnX = 0;
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);

                opponent = D_o058_5C80[D_8007BEF8_o058Reloc - 1] + o58RankX_o058Reloc + o58ScoreX_o058Reloc;
                if (columnCount > 0) {
                    do {
                        sprintf(&text[0], D_o058_5D70, o58RankOrder_o058Reloc[textY]->flags[columnX]);
                        fontPrintXY(&D_800D3140_o058Reloc, opponent + 8, rowY + 0x16, &text[0], 4);
                        columnX += 1;
                        opponent += columnStep;
                    } while (columnX != columnCount);
                }
                textY += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8_o058Reloc);
        }
        o58RankX_o058Reloc = savedPosition;
        o58ScoreX_o058Reloc = savedOffset;

        o58ScoreX_o058Reloc -= ticks * 0xF;
        if (o58ScoreX_o058Reloc < 0) {
            o58ScoreX_o058Reloc = 0;
            if (o58NextScreen_o058Reloc == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 3) {
                        o58NextScreen_o058Reloc = 4;
                    } else {
                        o58NextScreen_o058Reloc = 7;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    o58NextScreen_o058Reloc = 0xC;
                    *o58ScoreDeltas_o058Reloc = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                o58RankX_o058Reloc += ticks * 0xF;
                if (o58RankX_o058Reloc >= 0x141) {
                    if (o58NextScreen_o058Reloc == 4) {
                        animseqStopPath(o58Animation_o058Reloc.bytes[3]);
                        o58Animation_o058Reloc.word = 1;
                        path = func_800508B4(o58Animation_o058Reloc.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(o58Animation_o058Reloc.bytes[3]);
                            animseqStartPath(o58Animation_o058Reloc.bytes[3]);
                            path->flags |= 2;
                        }
                        o58PathState_o058Reloc = 2;
                        o58PathDelay_o058Reloc = 0x50;
                        return;
                    }
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    amSndPlay(0x1FAU, NULL);
                    o58NextScreen_o058Reloc = 0;
                    o58RankX_o058Reloc = 0;
                    o58ScoreX_o058Reloc = 0x140;
                    return;
                }
            }
        }
        break;
    case 7:
    case 11:
        i = 0;
        if (o58NextScreen_o058Reloc == 0) {
            if ((D_800D31BE_o058Reloc >= 0x11) && (o58MenuChoice_o058Reloc > 0)) {
                o58MenuChoice_o058Reloc -= 1;
                amSndPlay(0xFU, NULL);
            } else if ((D_800D31BE_o058Reloc < -0x10) && (o58MenuChoice_o058Reloc < 3)) {
                o58MenuChoice_o058Reloc += 1;
                amSndPlay(0xFU, NULL);
            }
        }
        x = o58RankX_o058Reloc + o58ScoreX_o058Reloc;
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        if (o58ScreenState_o058Reloc == 7) {
            if (D_8007C1A0_o058Reloc == 1) {
                fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0xB5], 4);
            } else {
                fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x2A], 4);
            }
        } else {
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x2B], 4);
        }
        /* Indexed Y and text let strength reduction supply both carriers.
         * Preserve the early visible index across the menu input calls. */
        rowBase = 0;

        do {
            if (i == o58MenuChoice_o058Reloc) {
                fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            fontPrintXY(&D_800D3140_o058Reloc, 0xA0 - x, 0x50 + rowBase * 0x1E, o58HeadingText_o058Reloc[rowBase], 4);
            i += 1;
            rowBase++;

            x = -x;
        } while (i < 4);
        o58ScoreX_o058Reloc -= ticks * 0xF;

        if (o58ScoreX_o058Reloc < 0) {
            if (o58NextScreen_o058Reloc == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    o58NextScreen_o058Reloc = 4;
                    amSndPlay(0xCU, NULL);

                } else if (D_800D31B8_o058Reloc & 0x4000) {
                    if (o58ScreenState_o058Reloc == 7) {
                        o58NextScreen_o058Reloc = 0xC;
                    } else {
                        o58NextScreen_o058Reloc = 0xA;
                    }
                    amSndPlay(0xDU, NULL);

                }
            } else {
                o58RankX_o058Reloc += ticks * 0xF;
                if (o58RankX_o058Reloc >= 0x141) {
                    if (o58NextScreen_o058Reloc == 4) {
                        switch (o58MenuChoice_o058Reloc) {
                        case 0:
                            mainChangeCameras(D_8007C1A0_o058Reloc);
                            joyCreateMap(o58ActivePlayers_o058Reloc);
                            if (((D_8007C1A0_o058Reloc == 2) || (D_8007C1A0_o058Reloc == 3)) && (D_8007BF74_o058Reloc != 0)) {
                                D_8007BEFC_o058Reloc = 4 - D_8007C1A0_o058Reloc;
                                D_8007BEF8_o058Reloc = 4;
                            } else if ((D_8007C1A0_o058Reloc == 1) && (state->mode == 5)) {
                                D_8007BEFC_o058Reloc = 3;
                                D_8007BEF8_o058Reloc = 4;
                            } else {
                                D_8007BEFC_o058Reloc = 0;
                                D_8007BEF8_o058Reloc = (u8) D_8007C1A0_o058Reloc;
                            }
                            for (i = 0; i < 6; i++) {
                                state->entries[i].value = 0;
                            }
                            if (o58PathPending_o058Reloc != 0) {
                                mainChangeLevel((s32) D_800D304E_o058Reloc, 0, 0, 5, 1, 0);
                                o58PathPending_o058Reloc = 0;
                            }

                            break;
                        case 1:
                            if (o58PathPending_o058Reloc != 0) {
                                mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
                                o58PathPending_o058Reloc = 0;

                            }
                            break;
                        case 2:
                            if (o58PathPending_o058Reloc != 0) {
                                if (state->mode == 5) {
                                    mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                                } else {
                                    mainChangeLevel(0xC, 0, 0, 0x11, 1, 0);
                                }
                                o58PathPending_o058Reloc = 0;

                            }
                            break;
                        case 3:
                            if (o58PathPending_o058Reloc != 0) {
                                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                                o58PathPending_o058Reloc = 0;

                            }
                            break;
                        }
                    } else {
                        o58RowsY_o058Reloc = 0;
                        o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                        amSndPlay(0x1FAU, NULL);
                        o58NextScreen_o058Reloc = 0;
                        o58PrimaryX_o058Reloc = 0x140;
                        o58RankX_o058Reloc = 0;
                        o58ScoreX_o058Reloc = 0x140;

                        o58SlideX_o058Reloc = 0;
                    }
                }
            }
        }
        if (o58ScoreX_o058Reloc < 0) {
            o58ScoreX_o058Reloc = 0;
            return;
        }
        break;
    case 5:
        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, o58SlideX_o058Reloc + 0xA0, 0x50, D_8007C0B8_o058Reloc->text[0x2C], 4);
        o58SlideX_o058Reloc = -o58SlideX_o058Reloc;
        fontColour(0xFF, 0, 0, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, o58SlideX_o058Reloc + 0xA0, 0x8C, D_8007C0B8_o058Reloc->text[0x2D], 4);
        o58SlideX_o058Reloc = -o58SlideX_o058Reloc;

        o58SlideX_o058Reloc = o58SlideX_o058Reloc - (ticks * 0xF);
        if (o58SlideX_o058Reloc < 0) {
            if (D_800D31B8_o058Reloc & 0x9000) {
                func_8003A754();
                if ((D_8007BF44_o058Reloc > 0) && (state->mode == 0)) {
                    D_8007BF48_o058Reloc = -1;
                    if (o58PathPending_o058Reloc != 0) {
                        mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
                        o58PathPending_o058Reloc = 0;
                    }
                    mainSetAnimGroup(1);
                } else if (o58PathPending_o058Reloc != 0) {
                    mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                    o58PathPending_o058Reloc = 0;
                }
            }
            o58SlideX_o058Reloc = 0;
            return;
        }
        break;
    case 6:
        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, o58SlideX_o058Reloc + 0xA0, 0x50, D_8007C0B8_o058Reloc->text[0x2C], 4);
        o58SlideX_o058Reloc = -o58SlideX_o058Reloc;
        if (state->active == 0) {
            fontColour(0, 0xFF, 0, 0xFF, 0xFF);
            fontPrintXY(&D_800D3140_o058Reloc, o58SlideX_o058Reloc + 0xA0, 0x8C, D_8007C0B8_o058Reloc->text[0x6C], 4);
            o58SlideX_o058Reloc = -o58SlideX_o058Reloc;
        } else {
            i = 0;
            if (D_8007BF1C_o058Reloc & 0x100) {
                sprintf(&text[0], D_8007C0B8_o058Reloc->text[0xB3]);
                countdownX = 0x6C;
            } else {

                if (state->countdown == 1) {
                    sprintf(&text[0], D_8007C0B8_o058Reloc->text[0x2E], state->countdown);
                    countdownX = 0x94;
                } else {
                    sprintf(&text[0], D_8007C0B8_o058Reloc->text[0x2F], state->countdown);
                    countdownX = 0x80;
                }
            }
            fontColour(0, 0xFF, 0, 0xFF, 0xFF);
            fontPrintXY(&D_800D3140_o058Reloc, o58SlideX_o058Reloc + 0xA0, 0x8C, &text[0], 4);
            o58SlideX_o058Reloc = -o58SlideX_o058Reloc;

            if ((s32) state->countdown > 0) {
                /* Indexed X: strength reduction generates the walker (L160). */
                /* Adjacent index def; see the note in case 7/11. */
                i = 0;
                do {
                    nodes[i].texture = (RcpTextureInfo *) D_800D31C8_o058Reloc[0x51 + state->entries[0].character];
                    nodes[i].alternate = 0;
                    nodes[i].x = countdownX + o58SlideX_o058Reloc + i * 0x28;
                    nodes[i].y = 0xAA;
                    nodes[i].packedOffset = 0;
                    /* No declared walker: it kept portraitX out of s0 here. */
                    i += 1;
                } while (i < (s32) state->countdown);
            }
            nodes[i].texture = 0;
            func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
        }
        o58SlideX_o058Reloc = o58SlideX_o058Reloc - (ticks * 0xF);
        if (o58SlideX_o058Reloc < 0) {
            if (D_800D31B8_o058Reloc & 0x9000) {
                if ((D_8007BF44_o058Reloc > 0) && (state->mode == 0)) {
                    D_8007BEF8_o058Reloc = 6;
                    D_8007BEFC_o058Reloc = 5;
                    joyCreateMap(o58ActivePlayers_o058Reloc);
                    D_8007BF48_o058Reloc = (s32) D_8007C0C0_o058Reloc[state->player][state->active];
                    D_8007BF4C_o058Reloc = (s32) state->entries[0].character;
                    D_8007BF50_o058Reloc = 5;
                    D_8007BF54_o058Reloc = 0;
                    mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
                    mainSetAnimGroup(1);
                } else {
                    o58PathFinished_o058Reloc = 1;
                }
            }
            o58SlideX_o058Reloc = 0;
            return;
        }
        break;
    case 8:
        x = o58PrimaryX_o058Reloc + o58SlideX_o058Reloc;

        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, func_8003A5A0((s32) D_800D304E_o058Reloc), 4);
        x = -x;
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x39, D_8007C0B8_o058Reloc->text[0x27], 4);

        if (D_o058_5CD8 != 0) {
            if ((D_o058_5CD8 >= 2) && (x == 0)) {
                amSndPlay(0x27CU, NULL);
                D_o058_5CD8 = 1;
            }
            if (D_o058_5CD8 == 1) {
                func_8002F618(&D_800D3140_o058Reloc, D_o058_5BA0, x + 0x30, 0x24, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            }
        }
        /* The visible row index starts with the title font call.
         * This separate induction starts beside the loop so uopt can fold
         * both the lap-array address and the row-coordinate stride.
         * Both indices visit zero through two once; `opponent` is otherwise
         * dead here.  Splitting only one induction leaves the other blocked.
         * This split increases `i`'s occurrence-block count; the fourth
         * grid probe below restores its saved-register priority.
         * The visible increment goes first to match the loop-tail schedule. */
        opponent = 0;
        do {
            x = -x;
            textY = 0x5B + o58RowsY_o058Reloc + opponent * 0x1B; /* This order puts the global first in the add. */
            if (i == gOverlay58FloorChoice_o058Reloc) {
                fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            sprintf(&text[0], D_8007C0B8_o058Reloc->text[0x30], i + 1);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0x82, textY, &text[0], 1);
            overlay56SplitTime(state->entries[0].lapTimes[opponent], &minutes, &seconds, &centiseconds);
            if (i != gOverlay58FloorChoice_o058Reloc) {
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
            }
            sprintf(&text[0], D_o058_5D74, minutes);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xB4, textY, &text[0], 1);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xB7, textY, D_o058_5D78, 0);
            sprintf(&text[0], D_o058_5D7C, seconds);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xBE, textY, &text[0], 0);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xD5, textY, D_o058_5D84, 0);
            sprintf(&text[0], D_o058_5D88, centiseconds);
            fontPrintXY(&D_800D3140_o058Reloc, x + 0xDC, textY, &text[0], 0);
            i += 1;
            opponent += 1;
        } while (i != 3);
        x = -x;
        textY = textY + 0x2C;
        if (gOverlay58InsertChoice_o058Reloc != -1) {
            fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
        } else {
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        fontPrintXY(&D_800D3140_o058Reloc, x + 0x82, textY, D_8007C0B8_o058Reloc->text[0x31], 1);
        overlay56SplitTime(state->entries[0].value, &minutes, &seconds, &centiseconds);
        if (gOverlay58InsertChoice_o058Reloc == -1) {
            fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        }
        sprintf(&text[0], D_o058_5D90, minutes);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xB4, textY, &text[0], 1);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xB7, textY, D_o058_5D94, 0);
        sprintf(&text[0], D_o058_5D98, seconds);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xBE, textY, &text[0], 0);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xD5, textY, D_o058_5DA0, 0);
        sprintf(&text[0], D_o058_5DA4, centiseconds);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xDC, textY, &text[0], 0);
        o58PrimaryX_o058Reloc -= ticks * 0xF;
        if (o58PrimaryX_o058Reloc < 0) {
            o58PrimaryX_o058Reloc = 0;
            if ((o58NextScreen_o058Reloc == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                if ((gOverlay58FloorChoice_o058Reloc != -1) || (gOverlay58InsertChoice_o058Reloc != -1)) {
                    o58NextScreen_o058Reloc = 9;
                } else {
                    o58NextScreen_o058Reloc = 0xA;
                }
                amSndPlay(0xCU, NULL);
            }
            if (o58NextScreen_o058Reloc != 0) {
                o58SlideX_o058Reloc += ticks * 0xF;
                if (o58SlideX_o058Reloc >= 0x141) {
                    o58SlideX_o058Reloc = 0;
                    o58PrimaryX_o058Reloc = 0x140;
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    o58NextScreen_o058Reloc = 0;
                    amSndPlay(0x1FAU, NULL);
                    return;
                }
            }
        }
        break;
    case 9:
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        x = o58PrimaryX_o058Reloc + o58SlideX_o058Reloc;
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, D_8007C0B8_o058Reloc->text[0x32], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        o58PrimaryX_o058Reloc -= ticks * 0xF;
        if (o58PrimaryX_o058Reloc < 0) {
            o58PrimaryX_o058Reloc = 0;
            if (o58NextScreen_o058Reloc == 0) {
                erase = 0;
                if (D_800D31B8_o058Reloc & 0x4000) {
                    erase = 1;
                } else if (D_800D31B8_o058Reloc & 0x9000) {
                    if ((o58SelectedAlphabetRow_o058Reloc == 2) && (o58SelectedAlphabetColumn_o058Reloc == 9)) {
                        erase = 1;
                    } else if ((D_8007C1B4_o058Reloc == 3) || (o58SelectedAlphabetRow_o058Reloc == 3)) {
                        if (o58SelectedAlphabetRow_o058Reloc == 3) {
                            amSndPlay(0xCU, NULL);
                            o58NextScreen_o058Reloc = 0xA;
                            saves = func_800291C4();
                            /* Slot address as overlay60Prefix spells it: the index
                             * through a dead local AND byte arithmetic.  Either alone is
                             * byte-identical to `&saves[f()]` (wv-v cycle 10); together
                             * they put `saves` first in the addu, as the target has it
                             * (wv-w cycle 1: the operand order is the tree's evaluation
                             * order, not a canonical sum). */
                            portraitIndex = levelGetBlurEffect(D_800D304E_o058Reloc); slot = (SavesSlot *) ((u8 *) saves + portraitIndex * 32);
                            if (gOverlay58FloorChoice_o058Reloc != -1) {
                                slot->records[3].name[0] = func_8003A6B0(D_800D31C4_o058Reloc[0]);
                                slot->records[3].name[1] = func_8003A6B0(D_800D31C4_o058Reloc[1]);
                                slot->records[3].name[2] = func_8003A6B0(D_800D31C4_o058Reloc[2]);
                            }
                            if (gOverlay58InsertChoice_o058Reloc != -1) {
                                slot->records[gOverlay58InsertChoice_o058Reloc].name[0] = func_8003A6B0(D_800D31C4_o058Reloc[0]);
                                slot->records[gOverlay58InsertChoice_o058Reloc].name[1] = func_8003A6B0(D_800D31C4_o058Reloc[1]);
                                slot->records[gOverlay58InsertChoice_o058Reloc].name[2] = func_8003A6B0(D_800D31C4_o058Reloc[2]);
                            }
                            func_80029120(levelGetBlurEffect(D_800D304E_o058Reloc));
                            if (gOverlay58ResourceReady_o058Reloc != 0) {
                                func_8002917C();
                            }
                        } else {
                            amSndPlay(0xEU, NULL);
                        }
                    } else {
                        D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc] = (u8) D_o058_5C5C[o58SelectedAlphabetRow_o058Reloc][o58SelectedAlphabetColumn_o058Reloc];
                        if (D_8007C1B4_o058Reloc == 2) {
                            o58SelectedAlphabetRow_o058Reloc = 3;
                        }
                        D_8007C1B4_o058Reloc += 1;
                        amSndPlay(0xCU, NULL);
                    }
                }
                if (erase != 0) {
                    if (D_8007C1B4_o058Reloc == 0) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc] = 0x20;
                        D_8007C1B4_o058Reloc -= 1;
                        columnX = 0; do { /* Keep the preheader scheduling tie. */

                            /* The outer row uses columnX and the inner
                             * character index uses i, as in the grid below.
                             * Both counters are reinitialized before each
                             * scan and retain the original bounds. */
                            i = 0;
                            do {
                                if ((u8) D_o058_5C5C[columnX][i] == D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc]) {
                                    o58SelectedAlphabetRow_o058Reloc = columnX;
                                    o58SelectedAlphabetColumn_o058Reloc = i;
                                }
                                i += 1;
                            } while (i != 0xA);
                            columnX += 1;
                        } while (columnX != 3);
                        amSndPlay(0xDU, NULL);
                    }
                }

                if (D_800D31BE_o058Reloc >= 0x11) {
                    if (o58SelectedAlphabetRow_o058Reloc == 0) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        o58SelectedAlphabetRow_o058Reloc -= 1;
                    }

                }
                if (D_800D31BE_o058Reloc < -0x10) {
                    if ((o58SelectedAlphabetRow_o058Reloc == 3) || (o58SelectedAlphabetRow_o058Reloc == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        o58SelectedAlphabetRow_o058Reloc += 1;
                    }
                }

                if (D_800D31BC_o058Reloc < -0x10) {
                    if ((o58SelectedAlphabetColumn_o058Reloc == 0) || (o58SelectedAlphabetRow_o058Reloc == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        o58SelectedAlphabetColumn_o058Reloc -= 1;
                    }

                }
                if (D_800D31BC_o058Reloc >= 0x11) {
                    if ((o58SelectedAlphabetColumn_o058Reloc == 9) || (o58SelectedAlphabetRow_o058Reloc == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        o58SelectedAlphabetColumn_o058Reloc += 1;
                    }
                }
            }
            if (o58NextScreen_o058Reloc != 0) {
                o58SlideX_o058Reloc += ticks * 0xF;
                if (o58SlideX_o058Reloc >= 0x141) {
                    o58SlideX_o058Reloc = 0;
                    o58PrimaryX_o058Reloc = 0x140;
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    o58NextScreen_o058Reloc = 0;
                    amSndPlay(0x1FAU, NULL);
                }
            }
        }
        textY = 0x78;
        columnX = 0; do { /* One physical line: as1 then emits the hoisted addresses first (L59). */
            /* Keep the increment opaque to the counted-loop proof.  The
             * discarded product below is defined and zero on every pass;
             * the index still starts at zero and advances by exactly one.
             * uopt therefore retains the signed comparison without making
             * its bound a saved-register web, while ugen folds the product.
             * Unlike the former opaque initial value, this does not need a
             * temporary stack slot or an extra executable instruction.
             * The record and character index choices in case 10 complete
             * the paired shape correction; either end alone misprices the
             * intervening displacement.  See the current handoff for the
             * controlled comparisons and the remaining local residual. */
            i = 0;
            x = -x;
            opponent = 0x34 + x;
            do {
                /* The erase scan and this grid share their row induction.
                 * columnX selects the row, i selects its character, and
                 * opponent carries the drawing coordinate.  Each role has
                 * its own initialization and visits the original bounds.
                 * The matching row carriers retain the shared table cursor.
                 * Together with the count/stride carrier exchange in case
                 * 13, this keeps the visible index in its target colour.
                 * The former four discarded index probes are unnecessary
                 * on this source shape and have been removed, and so has
                 * the opaque `i += (i * 0) + 1` unit increment: plain `i++`
                 * is byte-identical (lane c-1, 2026-10-08).
                 */
                sprintf(&text[0], D_o058_5DAC, (u8) D_o058_5C5C[columnX][i]);
                if ((i == o58SelectedAlphabetColumn_o058Reloc) && (columnX == o58SelectedAlphabetRow_o058Reloc)) {
                    fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                fontPrintXY(&D_800D3140_o058Reloc, opponent, textY, &text[0], 4);
                i++;
                opponent += 0x18;
            } while (i < 0xA);
            columnX += 1;
            textY += 0x1B;
        } while (columnX < 3);
        x = -x;
        if (o58SelectedAlphabetRow_o058Reloc == 3) {
            fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
        } else {
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, textY, D_8007C0B8_o058Reloc->text[0x33], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        i = 0;
        columnX = x + 0x78;
        if (D_8007C1B4_o058Reloc > 0) {
            do {
                sprintf(&text[0], D_o058_5DB0, D_800D31C4_o058Reloc[i]);
                fontPrintXY(&D_800D3140_o058Reloc, columnX, 0x50, &text[0], 4);
                i += 1;
                columnX += 0x1E;
            } while (i < D_8007C1B4_o058Reloc);
        }
        if ((D_8007C1B4_o058Reloc != 3) && (o58SelectedAlphabetRow_o058Reloc != 3)) {
            sprintf(&text[0], D_o058_5DB4, (u8) D_o058_5C5C[o58SelectedAlphabetRow_o058Reloc][o58SelectedAlphabetColumn_o058Reloc]);
            fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
            fontPrintXY(&D_800D3140_o058Reloc, columnX, 0x50, &text[0], 4);
            return;
        }
        break;
    case 10:
        x = o58PrimaryX_o058Reloc + o58SlideX_o058Reloc;
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));

        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x1E, func_8003A5A0((s32) D_800D304E_o058Reloc), 4);
        x = -x;
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, 0x39, D_8007C0B8_o058Reloc->text[0x34], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        saves = func_800291C4();

        erase = levelGetBlurEffect(D_800D304E_o058Reloc); slot = (SavesSlot *) ((u8 *) saves + erase * 32); /* see case 9 (wv-w) */

        if (D_o058_5CD8 != 0) {
            if ((D_o058_5CD8 >= 2) && (x == 0)) {
                amSndPlay(0x27CU, NULL);
                D_o058_5CD8 = 1;
            }
            if (D_o058_5CD8 == 1) {
                func_8002F618(&D_800D3140_o058Reloc, D_o058_5BA0, x + 0x30, 0x24, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            }
        }
        textY = 0x5B;
        /* Separate record induction. */
        opponent = 0;
        do {
            x = -x;
            overlay56SplitTime(slot->records[opponent].value, &minutes, &seconds, &centiseconds);
            if (slot->records[opponent].value == 0) {
                sprintf(&text[0], D_o058_5DB8);
                portraitIndex = 0x4A;
            } else {
                letter0 = func_8003A700(slot->records[opponent].name[0]) & 0xFF;
                letter1 = func_8003A700(slot->records[opponent].name[1]) & 0xFF;
                sprintf(&text[0], D_o058_5DC4, letter0, letter1, func_8003A700(slot->records[opponent].name[2]), minutes, seconds, centiseconds);
                portraitIndex = slot->records[opponent].character + 0x51;
            }
            highlighted = (i == gOverlay58InsertChoice_o058Reloc) ||
                       ((i == 3) && (gOverlay58FloorChoice_o058Reloc != -1));
            if (highlighted != 0) {
                fontColour((s32) o58Highlight_o058Reloc.red, (s32) o58Highlight_o058Reloc.green, (s32) o58Highlight_o058Reloc.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            if (i < 3) {
                fontPrintXY(&D_800D3140_o058Reloc, x + 0x28, textY, D_o058_5C98[i], 0);
            }
            nodes[0].texture = D_800D31C8_o058Reloc[portraitIndex];
            nodes[0].alternate = NULL;
            nodes[0].x = x + 0x58;
            nodes[0].y = textY - 4;
            nodes[0].packedOffset = 0;
            nodes[1].texture = 0;
            func_8002F618(&D_800D3140_o058Reloc, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            /* `rowY` is dead on entry to case 10; this independent index
             * avoids the former carrier's interference with character. */
            rowY = 0;
            do {
                if (highlighted == 0) {
                    if (rowY == 0) {
                        fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    if (rowY == 3) {
                        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                    }
                }
                character[0] = text[rowY];
                character[1] = 0;
                fontPrintXY(&D_800D3140_o058Reloc, D_o058_5CB0[rowY] + x, textY, &character[0], 0);
                rowY += 1;
            } while (rowY != 11);
            if (i == 2) {
                textY += 0x1B;
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                fontPrintXY(&D_800D3140_o058Reloc, x + 0xA0, textY, D_8007C0B8_o058Reloc->text[0x35], 4);
            }
            opponent += 1;
            i += 1;
            textY += 0x1B;
        } while (i != 4);
        o58PrimaryX_o058Reloc -= ticks * 0xF;
        if (o58PrimaryX_o058Reloc < 0) {
            o58PrimaryX_o058Reloc = 0;
            if ((o58NextScreen_o058Reloc == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                o58NextScreen_o058Reloc = 0xB;
                amSndPlay(0xCU, NULL);
            }
            if (o58NextScreen_o058Reloc != 0) {
                o58SlideX_o058Reloc += ticks * 0xF;
                if (o58SlideX_o058Reloc >= 0x141) {
                    o58ScreenState_o058Reloc = o58NextScreen_o058Reloc;
                    o58RankX_o058Reloc = 0;
                    o58ScoreX_o058Reloc = 0x140;
                    o58NextScreen_o058Reloc = 0;
                    amSndPlay(0x1FAU, NULL);
                }
            }
        }
        break;
    default:
        break;
    }
}

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDma1p and include/PR/mbi.h _SHIFTL,
 * include/f3ddkr.h gSPVertexJFG and gSPPolygon, include/PR/os_convert.h
 * OS_PHYSICAL_TO_K0), a permitted source under docs/CLEANROOM.md, exactly as
 * in this overlay's two point-quad routines.
 */

#define O58_SHIFTL(v, s, w) ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))

#define O58_DMA1P(pkt, c, s, l, p) { Overlay58Gfx *_g = (Overlay58Gfx *)(pkt); _g->w0 = (O58_SHIFTL((c), 24, 8) | O58_SHIFTL((p), 16, 8) | O58_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }

#define O58_VERTEX(pkt, v, n, v0) O58_DMA1P(pkt, 4, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))

#define O58_POLYGON(dl, ptr, numTris, texEnabled) { Overlay58Gfx *_g = (Overlay58Gfx *)(dl); _g->w0 = O58_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O58_SHIFTL(5, 24, 8) | O58_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }

#define O58_PHYSICAL_TO_K0(x) (void *)(((u32)(x)+0x80000000))

/*
 * Matched 2026-10-01 by shape, from a 68-word plateau whose named blocker was
 * the ranking of the 0xFF constant against the vertex cursor's address.  The
 * four vertex colours are set by a four-pass loop over a walking pointer; the
 * compiler unrolls it completely, which is where the second, third, fourth,
 * first store order, the reversed third group and the dead pointer advance
 * all come from, and the constant keeps its inner-loop weight so it is
 * coloured ahead of the address.  The two packets are the JFG macros, the Y
 * coordinate is the same cast at each use, and the parameter is initialised
 * before the prepare call with no probe statement.
 */
static void overlay58DrawSegmentStrip(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                               f32 z1, f32 limit) {
    f32 dy;
    f32 dz;
    f32 distance;
    f32 t;
    f32 dx;
    f32 next;
    f32 startX;
    f32 startZ;
    f32 endX;
    f32 zPerpendicular;
    f32 xPerpendicular;
    f32 endZ;
    f32 stripStep;
    f32 quadSpan;
    Overlay58Vertex *vertices;
    s32 i;

    dx = x1 - x0;
    dy = y1 - y0;
    dz = z1 - z0;
    distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));

    zPerpendicular = (1.25f * dx) / distance;
    xPerpendicular = (1.25f * dz) / distance;
    stripStep = 12.0f / distance;
    quadSpan = 8.0f / distance;
    t = 0.0f;

    texDPTextureX(&D_800D3140, (void *)0,
                               5, 0);
    while (t < limit) {
        vertices = D_800D3148;
        for (i = 0; i < 4; i++) {
            vertices->r = 0xFF;
            vertices->g = 0xFF;
            vertices->b = 0xFF;
            vertices->a = 0xFF;
            vertices++;
        }
        next = t + quadSpan;
        if (next > 1.0f) {
            next = 1.0f;
        }
        if (next <= limit) {
            O58_VERTEX(D_800D3140++, O58_PHYSICAL_TO_K0(D_800D3148), 4, 0);
            O58_POLYGON(D_800D3140++, D_80000058, 2, 1);
            startX = (t * dx) + x0;
            startZ = (t * dz) + z0;
            D_800D3148->x = (s16)(startX - xPerpendicular);
            D_800D3148->y = (s16)y0;
            D_800D3148->z = (s16)(startZ + zPerpendicular);
            D_800D3148++;
            D_800D3148->x = (s16)(startX + xPerpendicular);
            D_800D3148->y = (s16)y0;
            D_800D3148->z = (s16)(startZ - zPerpendicular);
            D_800D3148++;
            endX = (next * dx) + x0;
            endZ = (next * dz) + z0;
            D_800D3148->x = (s16)(endX - xPerpendicular);
            D_800D3148->y = (s16)y0;
            D_800D3148->z = (s16)(endZ + zPerpendicular);
            D_800D3148++;
            D_800D3148->x = (s16)(endX + xPerpendicular);
            D_800D3148->y = (s16)y0;
            D_800D3148->z = (s16)(endZ - zPerpendicular);
            D_800D3148++;
        }
        t += stripStep;
    }
}

/*
 * The shipped table-1 SYMBOL record names the resident texDPTextureX (its
 * `jal` stores 0, the runtime patches it).  Its first argument is the display
 * list head, the same identity as D_800D3140 below.
 */  /* (was on extern texDPTextureX) */
/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDma1p and include/PR/mbi.h _SHIFTL,
 * include/f3ddkr.h gSPVertexJFG and gSPPolygon, include/PR/os_convert.h
 * OS_PHYSICAL_TO_K0), a permitted source under docs/CLEANROOM.md.  The two
 * commands are gSPVertexJFG(dl++, OS_PHYSICAL_TO_K0(cursor), 4, 0) and
 * gSPPolygon(dl++, payload, 2, TRIN_ENABLE_TEXTURE): the vertex command's
 * `v` argument is used twice, which is why the cursor is loaded twice, and
 * each macro is one physical line, which is what retires the store-order
 * tie at +0x88.  Each macro's block-scoped `_g` is a phantom symbol web
 * (decided after the shared `dl++` expression web once both appends sit in
 * one block, which is what gives that web v0).
 */

/*
 * Matched by lane w1-b (2026-09-16) from the 21-word register-only plateau;
 * see docs/lastmile-block-budget-globals.md.  Three things had to hold at
 * once, all of them block structure rather than colour:
 *
 *  1. Both packet appends in ONE uopt block, and `vertices = cursor` in that
 *     same block: the `dl++` expression web then has one occurrence set
 *     (save 6.0) and is coloured v0 before the two `_g` phantoms (3.0 each,
 *     a1/a2), and `vertices` spanning that block and the colour block keeps
 *     v1 busy there so the display-list address falls to a3.
 *  2. The colour fill written as the four-vertex loop it is.  IDO unrolls
 *     the constant-count loop (vertices 1-3 at positive offsets, the cursor
 *     advanced by three, vertex 0 at -3), and the loop's entry and exit
 *     are the block boundaries the match needs: the 0xFF constant is a
 *     single-block web (save 15, v0) that shares no block with the `dl++`
 *     web, and the four coordinate expressions are single-block webs
 *     coloured v0/v1/a1/a2 in first-occurrence order.  An earlier
 *     straight-line spelling needed two empty `if (vertices != 0);`
 *     probes to cut those blocks (lane c-1, 2026-10-08, byte-identical).
 *  3. The call in its own `if (1)` region, arguments in source order.
 *     Spelled plainly the display-list `lui` is scheduled first (4 words
 *     differ).  Reading the render-state field in a region of its own
 *     before the call also matches the bytes, but it reverses the two
 *     HI16/LO16 pairs in the relocation table against the shipped order
 *     (display list first), so the region holds the whole call instead.
 */
static void overlay58DrawPointQuad(s32 x, s32 y, s32 z) {
    Overlay58Vertex *vertices;
    s32 i;

    if (1) { texDPTextureX(&D_800D3140, D_800D31C8[0x44 / 4], 5, 0); }
    O58_VERTEX(D_800D3140++, O58_PHYSICAL_TO_K0(D_800D3148), 4, 0);
    O58_POLYGON(D_800D3140++, D_80000078, 2, 1);
    vertices = D_800D3148;
    for (i = 0; i < 4; i++) {
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
    }
    D_800D3148->x = (s16)(x - 8);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 8);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 8);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 8);
    D_800D3148++;
    D_800D3148->x = (s16)(x - 8);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 8);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 8);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 8);
    D_800D3148++;
}

/*
 * The shipped table-1 SYMBOL record names the resident texDPTextureX (its
 * `jal` stores 0, the runtime patches it).  Its first argument is the display
 * list head, the same identity as D_800D3140 below.
 */  /* (was on extern texDPTextureX) */
/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDma1p and include/PR/mbi.h _SHIFTL,
 * include/f3ddkr.h gSPVertexJFG and gSPPolygon, include/PR/os_convert.h
 * OS_PHYSICAL_TO_K0), a permitted source under docs/CLEANROOM.md.  The two
 * commands are gSPVertexJFG(dl++, OS_PHYSICAL_TO_K0(cursor), 4, 0) and
 * gSPPolygon(dl++, payload, 2, TRIN_ENABLE_TEXTURE): the vertex command's
 * `v` argument is used twice, which is why the cursor is loaded twice, and
 * each macro is one physical line, which is what retires the store-order
 * tie at +0x88.  Each macro's block-scoped `_g` is a phantom symbol web
 * (decided after the shared `dl++` expression web once both appends sit in
 * one block, which is what gives that web v0).
 */

/*
 * Matched by lane w1-b (2026-09-16) from the 21-word register-only plateau;
 * see docs/lastmile-block-budget-globals.md.  Three things had to hold at
 * once, all of them block structure rather than colour:
 *
 *  1. Both packet appends in ONE uopt block, and `vertices = cursor` in that
 *     same block: the `dl++` expression web then has one occurrence set
 *     (save 6.0) and is coloured v0 before the two `_g` phantoms (3.0 each,
 *     a1/a2), and `vertices` spanning that block and the colour block keeps
 *     v1 busy there so the display-list address falls to a3.
 *  2. The colour fill written as the four-vertex loop it is.  IDO unrolls
 *     the constant-count loop (vertices 1-3 at positive offsets, the cursor
 *     advanced by three, vertex 0 at -3), and the loop's entry and exit
 *     are the block boundaries the match needs: the 0xFF constant is a
 *     single-block web (save 15, v0) that shares no block with the `dl++`
 *     web, and the four coordinate expressions are single-block webs
 *     coloured v0/v1/a1/a2 in first-occurrence order.  An earlier
 *     straight-line spelling needed two empty `if (vertices != 0);`
 *     probes to cut those blocks (lane c-1, 2026-10-08, byte-identical).
 *  3. The call in its own `if (1)` region, arguments in source order.
 *     Spelled plainly the display-list `lui` is scheduled first (4 words
 *     differ).  Reading the render-state field in a region of its own
 *     before the call also matches the bytes, but it reverses the two
 *     HI16/LO16 pairs in the relocation table against the shipped order
 *     (display list first), so the region holds the whole call instead.
 */
static void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z) {
    Overlay58Vertex *vertices;
    s32 i;

    if (1) { texDPTextureX(&D_800D3140, D_800D31C8[0x1E8 / 4], 5, 0); }
    O58_VERTEX(D_800D3140++, O58_PHYSICAL_TO_K0(D_800D3148), 4, 0);
    O58_POLYGON(D_800D3140++, D_80000098, 2, 1);
    vertices = D_800D3148;
    for (i = 0; i < 4; i++) {
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
    }
    D_800D3148->x = (s16)(x - 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z - 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x - 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 18);
    D_800D3148++;
    D_800D3148->x = (s16)(x + 18);
    D_800D3148->y = (s16)y;
    D_800D3148->z = (s16)(z + 18);
    D_800D3148++;
}

/* Pinned DKR v77/v80 and JFG scans found no matching donor. */
static void overlay58RefreshRankSet(void) {
    Overlay58RankSet *set;
    Overlay58RankSet *sets;
    Overlay58PrioritySource *source;
    s32 setIndex;
    s32 i;
    s32 j;
    s32 candidateFloor;
    s32 existingFloor;
    u8 *sourceMeta;
    s32 *floorCursor;

    sets = func_800291C4();
    setIndex = levelGetBlurEffect(gOverlay58RankSetIdReloc);
    set = (Overlay58RankSet *)((u8 *)sets + (setIndex << 5));
    source = func_80028F54();
    sourceMeta = &source->tag;
    gOverlay58FloorChoice_o058Reloc = -1;
    floorCursor = (s32 *)sourceMeta;

    for (i = 0; i < 3; i++) {
        existingFloor = set->floorValue;
        candidateFloor = floorCursor[2];
        if (((candidateFloor / 3) < (existingFloor / 3)) ||
            (existingFloor == 0)) {
            set->floorValue = candidateFloor;
            set->floorTag = *sourceMeta;
            gOverlay58FloorChoice_o058Reloc = i;
        }
        floorCursor++;
    }

    gOverlay58InsertChoice_o058Reloc = -1;
    for (i = 0; i < 3; i++) {
        if (((*(s32 *)(sourceMeta + 4) / 3) <
             (set->entries[i].value / 3)) ||
            (set->entries[i].value == 0)) {
            gOverlay58InsertChoice_o058Reloc = i;

            for (j = 2; j > i; j--) {
                set->entries[j].reserved4 = set->entries[j - 1].reserved4;
                set->entries[j].reserved5 = set->entries[j - 1].reserved5;
                set->entries[j].reserved6 = set->entries[j - 1].reserved6;
                set->entries[j].value = set->entries[j - 1].value;
                set->entries[j].tag = set->entries[j - 1].tag;
            }

            set->entries[i].value = *(s32 *)(sourceMeta + 4);
            set->entries[i].tag = *sourceMeta;

            /* Keep the ordinary increment/backedge while forcing loop exit. */
            i = 3;
        }
    }
}

/* Pinned DKR v77/v80 and JFG scans found no matching donor. */
static void overlay58EnsureResource(void) {
    if (D_8007A1FC == NULL) {
        D_8007A1FC = overlay68CheckKind(gOverlay58ResourceId);
        if (D_8007A1FC != NULL) {
            gOverlay58ResourceState = 2;
            ((Overlay58Resource *)D_800D3128)->count++;
            gOverlay58ResourceReady_o058Reloc = 1;
        } else {
            gOverlay58ResourceState = 0;
        }
    } else {
        gOverlay58ResourceState = 1;
    }
}

/* The gate, the two mode bytes and the packed status words are resident
 * objects reached through runtime relocation records. */  /* (was on extern D_8007BF0C) */
/* overlay-local BSS +0x74 */  /* (was on extern D_8007BF04) */
/* Exact body for executable range +0x5554..+0x5A14.
 *
 * Matched 2026-10-01 by discarding the probes, volatile reads and comma
 * assignments of the plateau shape and writing what the relocation records
 * and the frame name:
 * - all twenty-four call sites are three resident functions;
 * - nine locals, with the records pointer declared last. The packed mode
 *   and the selected player are not locals: each is read from its object at
 *   every use, so the compiler keeps one shared load;
 * - `self` is assigned from the record inside each counting loop. The
 *   assignment is hoisted to the preheader as a copy of the shared load,
 *   and the in-loop reference is what ranks that load ahead of the other
 *   caller-saved values;
 * - the decoded field of the third loop reuses `mask`, which keeps it in the
 *   mask's register;
 * - the posted count is the expression `count + 1` at both uses, so it is a
 *   compiler temporary with its own home rather than the count's. */
static void overlay58FinalizePackedStatus(void) {
    s32 i;
    s32 count;
    s32 fourCount;
    s32 desired;
    s32 self;
    s32 current;
    s32 mask;
    s32 shift;
    Overlay58InputRecord *records;

    records = func_80028F54();

    if (D_8007BF0C != 0) {
        return;
    }
    if (records[0].state != 0) {
        return;
    }
    if (records[0].phase != 3) {
        return;
    }
    if (gOverlay58PrimaryOrderIndex_o058Reloc >= 4) {
        return;
    }

    desired = 3;
    if (records[0].rank == 0x24) {
        desired = 4;
    } else {
        for (i = 1; (desired > 0) && (i != 6);) {
            if (records[0].rank < records[i++].rank) {
                desired--;
            }
        }
    }

    if (D_8007BF04 == 0) {
        shift = 0;
        mask = 0x7;
    } else if (D_8007BF04 == 1) {
        shift = 3;
        mask = 0x38;
    } else if (D_8007BF08 == 0) {
        shift = 6;
        mask = 0x1C0;
    } else {
        shift = 9;
        mask = 0xE00;
    }

    current = (D_800D3128[records[0].player + 4] & mask) >> shift;
    if (desired < 3) {
        return;
    }
    if (current >= desired) {
        return;
    }

    if (D_8007BF04 == 0) {
        if (current >= 3) {
            return;
        }
        i = 0;
        count = 0;
        for (; i < 3; i++) {
            self = records[0].player;
            if ((i != self) && ((D_800D3128[i + 4] & 0x7) >= 3)) {
                count++;
            }
        }
        if (count == 0) {
            if (desired == 4) {
                func_8003A680(0x17);
            } else {
                func_8003A680(0x16);
            }
        } else if (count == 2) {
            if ((self >= 0) && (self < 3)) {
                func_8003A680(0x0B);
                D_800D3128[10] |= 0x10;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if (D_8007BF04 == 1) {
        if (current >= 3) {
            return;
        }
        i = 0;
        count = 0;
        for (; i < 3; i++) {
            self = records[0].player;
            if ((i != self) &&
                (((D_800D3128[i + 4] & 0x38) >> 3) >= 3)) {
                count++;
            }
        }
        if (count == 0) {
            if (desired == 4) {
                func_8003A680(0x17);
            } else {
                func_8003A680(0x16);
            }
        } else if (count == 2) {
            if ((self >= 0) && (self < 3)) {
                func_8003A680(0x0C);
                D_800D3128[10] |= 0x04;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if (D_8007BF08 == 0) {
        i = 0;
        count = 0;
        fourCount = 0;
        for (; i < 5; i++) {
            self = records[0].player;
            if (i != self) {
                mask = (D_800D3128[i + 4] & 0x1C0) >> 6;
                if (mask >= 3) {
                    count++;
                }
                if (mask == 4) {
                    fourCount++;
                }
            }
        }

        if ((fourCount == 4) && (desired == 4)) {
            func_8003A680(0x0D);
        }

        if (current < 3) {
            func_8003A680(count + 1);
            if ((count + 1) == 5) {
                func_8003A680(0x18);
                if ((D_800D3128[10] & 0x40) == 0) {
                    D_800D3128[10] |= 0x40;
                    func_800291B4();
                    func_8003A680(0x15);
                }
            }
        }

        if (records[0].player == 3) {
            if ((D_800D3128[10] & 0x100) == 0) {
                D_800D3128[10] |= 0x100;
                func_800291B4();
                func_8003A680(0x15);
            }
        }
        return;
    }

    if ((desired != 4) || (current == 4)) {
        return;
    }

    switch (records[0].player) {
        case 0:
            D_800D3128[10] |= 0x01;
            func_800291B4();
            break;
        case 1:
            D_800D3128[10] |= 0x02;
            func_800291B4();
            break;
        case 2:
            D_800D3128[10] |= 0x08;
            func_800291B4();
            break;
        case 3:
            D_800D3128[10] |= 0x20;
            func_800291B4();
            break;
        default:
            D_800D3128[10] |= 0x80;
            func_800291B4();
            break;
    }
    func_8003A680(0x15);
}
