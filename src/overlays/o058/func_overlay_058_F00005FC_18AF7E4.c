#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58Status {
    u8 mode;
    u8 active;
    u8 player;
    u8 value3;
    u8 value4;
} Overlay58Status;

typedef struct Overlay58OrderEntry {
    u8 mode;
} Overlay58OrderEntry;

typedef struct Overlay58Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay58Vec3f;

typedef struct Overlay58PathGeometry {
    u8 pad00[0x40];
    Overlay58Vec3f *vertices;
} Overlay58PathGeometry;

typedef struct Overlay58PathObject {
    u8 pad00[0x68];
    Overlay58PathGeometry **geometry;
} Overlay58PathObject;

typedef struct Overlay58AnimPath {
    u8 pad00[8];
    Overlay58PathObject *object;
    u8 pad0C[0xA];
    u8 flags;
} Overlay58AnimPath;

typedef struct Overlay58Gfx {
    u32 w0;
    u32 w1;
} Overlay58Gfx;

typedef union Overlay58PathSelection {
    s32 value;
    struct {
        u8 pad00[3];
        u8 path;
    } bytes;
} Overlay58PathSelection;

extern s32 D_30;
extern Overlay58PathSelection D_34;
extern s32 D_38;
extern s32 D_44;
extern s32 D_48;
extern s32 D_4C;
extern s32 D_50;
extern s32 D_54;
extern s32 D_58;
extern s32 D_5C;
extern s32 D_60;
extern s32 D_68;
extern s32 D_6C;
extern s32 D_70;
extern Overlay58OrderEntry *D_90[];
extern s16 D_B8[][2];
extern s8 D_F8[];
extern f32 D_120;
extern s16 D_1A0[];
extern u8 D_2A8[];
extern s32 D_2B0;
extern s32 D_2B4;
extern s32 D_2BC;
extern f32 D_2C0;
extern s32 D_DC;

extern u8 gOverlay58MenuGateReloc;
extern s16 gOverlay58SelectionTableReloc[][4];
extern u8 gOverlay58MenuBitsReloc[];
extern s32 gOverlay58MenuReadyReloc;
extern u8 gOverlay58ConfigAReloc;
extern u8 gOverlay58ConfigBReloc;
extern u8 gOverlay58ConfigCReloc;
extern u8 gOverlay58PlayerReloc;
extern s32 gOverlay58LevelReloc;
extern s32 gOverlay58TrackReloc;
extern s32 gOverlay58VehicleReloc;
extern s32 gOverlay58TransitionModeReloc;
extern u8 gOverlay58ControllerCountReloc;
extern u8 gOverlay58ControllerModeReloc;
extern s32 gOverlay58CameraModeReloc;
extern u8 gOverlay58CameraGateReloc;
extern s32 gOverlay58InputReloc;
extern u8 gOverlay58ScreenModeReloc[];
extern Overlay58Gfx *gOverlay58DisplayListReloc;
extern void *gOverlay58MatrixReloc;

extern Overlay58Status *func_80028F54(void);
extern Overlay58AnimPath *func_800508B4(u8 path);
extern void func_8005055C(u8 path);
extern void animseqStartPath(u8 path);
extern void animseqStopPath(u8 path);
extern void amSndPlay();
extern void amSndStop();
extern s32 overlay41IsUnitScale(s32 index);
extern void func_800291B4(void);
extern void func_8003A680(s32 value);
extern void joyCreateMap(s8 *activePlayers);
extern void mainChangeLevel(s32 level, s32 track, s32 vehicle, s32 mode,
                            s32 arg4, s32 arg5);
extern void mainChangeCameras(s32 mode);
extern void func_800221E8(Overlay58Gfx **displayList, void **matrix);
extern void overlay58SetNodeValue(s32 index, s32 component, f32 value);
extern void func_overlay_058_F000138C_18B0574(s32 updateRate);
extern void overlay58EnsureResource(void);

/*
 * Mickey-only reconstruction. The donor scan found no close permitted
 * skeleton, and skeleton_scan cannot yet address an assembly ownership range.
 *
 * Closed hole (2026-09-10).  Between `func_800291B4()` and `func_8003A680()`
 * the target executes eight instructions that reload `status` from its home
 * and recompute the selection-table element from `status->player` and
 * `status->active` -- a memory load no compiler may re-materialize across a
 * call from a cached local, so the source spells that access a second time
 * rather than reusing `selection`.  Adopting the re-read closes the single
 * largest structural hole in the body: the register-masked alignment goes
 * from 672/829 identical rows to 715/829 (81.1% -> 86.2%) and the exact
 * alignment from 415 to 419.
 *
 * That also corrects what the extent measurement was saying.  The previous
 * -3 was a net across a -8 hole and a +6 surplus, and reads as "three
 * instructions short" only because the two nearly cancel (lever 48).  With
 * the hole closed the surplus is exposed and localized: the candidate is
 * +6, and every remaining count-changing block is one of three causes.
 *
 *   1. One extra callee-saved web (+2).  The candidate saves s0-s8 where the
 *      target saves s0-s7, which is also the whole 16-byte frame surplus
 *      (0x98 against 0x88).  The webs do not correspond: the target parks
 *      the literal 3 in s4 and `geometry` in s7 for seven `64(s7)` reads,
 *      while the candidate parks `geometry` on the stack at 0x8C, hoists 12
 *      into s8 for the five vertex `multu`s, and materializes -1 a fifth
 *      time into s4 inside the strip loop where the target reuses the one it
 *      already has.
 *   2. Branch-likely selection (+4 across four blocks).  The target reaches
 *      its early exits with `bltzl`/`beqzl` carrying `lw ra` in the likely
 *      slot; the candidate emits the plain branch and a separate restore.
 *      The epilogue carries one more `lw` than the target's because of (1),
 *      so this is very likely a consequence of the extra saved web rather
 *      than an independent defect.
 *   3. The strip-loop cursor materialization.  The target lands `D_1A0`
 *      straight in the cursor (`lui`, then `addiu v0,reg,%lo`); the
 *      candidate completes the address in its own register and copies it
 *      (`lui`, `addiu`, `move`), which is lever 51 read from the other side.
 *
 * Flat under measurement here, do not repeat: all six orderings of the
 * `start`/`end`/`mapping` initializers, `stage < D_6C + 1` for the loop
 * guard, and reversed operand order in both sentinel tests -- every one is
 * byte-identical to the form above.  The flag lattice was re-run and the
 * shipped `-O2 -g3 -mips2` is the best of eight configurations on the
 * alignment; `-mips1` costs about a hundred instructions, `-O1` more, and
 * dropping `-g3` changes the count by one without moving identical rows.
 *
 * Prior ABI/prologue, branch-order, marker-lifetime, geometry, and
 * split-path forms remain exhausted, as does the Overlay34 conditional-start
 * precedent.
 *
 * Remeasured 2026-09-11 (lane/p9-oneoff) with tools/align_symbol.py. Three
 * corrections to the numbers above, and one fork worth recording:
 *
 *   - The frame surplus is now 24 bytes, not 16. The candidate's prologue is
 *     `addiu sp,sp,-160` against the target's `-136` (0xA0 against 0x88).
 *     The `0x98 against 0x88` in cause (1) is stale.
 *   - The uncached form is +8 bytes and 723 masked words today, not the
 *     +24/727 recorded at the point of use above. The cached local is still
 *     the better of the two, but by 4 bytes and 1 word, not by 20 and 5.
 *   - The named `geometry` local is FREE. Removing it entirely --
 *     `verts = (*path->object->geometry)->vertices;` with the declaration
 *     deleted -- is byte-identical to the form here. So the "parks
 *     `geometry` on the stack at 0x8C" in cause (1) is a uopt temporary,
 *     not the declared local, and no source lever reaches it by renaming or
 *     removing that local.
 *
 * What the size delta actually is. The aligner puts a 2-word insertion at
 * candidate +0x0 and nothing else in the prologue: the candidate saves s8
 * where the target does not, so the save/restore pair is +8 bytes on its own,
 * and the body is -4 against it. The net +4 is a cancellation, which is why
 * hunting for "one instruction" in the body cannot work -- there is no single
 * surplus instruction to find. The lever is the ninth callee-saved web, and
 * that is cause (1), already named.
 *
 * A measured fork, NOT adopted. Replacing the hoisted `verts` with a
 * block-scoped `Overlay58Vec3f *v = geometry->vertices;` at the head of each
 * of the five drawing blocks -- which is what the target's seven `64(s7)`
 * reads look like -- moves the aligned split from 314/301/230 to 418/265/162
 * (byte-exact 37.9% -> 49.5%) and the positional residual from 722 to 551.
 * It also takes the size to +28 and the frame to 0xB0, because each block
 * local is its own web and the tenth one spills. It is recorded here because
 * it is the only form measured that moves the byte-exact count by a hundred
 * rows; it becomes adoptable the moment the callee-saved surplus is solved,
 * and not before. Partial forms (block locals on the two strips only, or on
 * the three quads only) are 554/+36 and 552/+32, both worse than the whole.
 * Reloading the cache at the top of the stage loop is 590/+12.
 */
#ifdef NON_MATCHING
void func_overlay_058_F00005FC_18AF7E4(s32 updateRate) {
    Overlay58Status *status;
    Overlay58AnimPath *path;
    Overlay58PathGeometry *geometry;
    Overlay58Vec3f *verts;
    Overlay58Gfx *command;
    s16 *mapping;
    s16 start;
    s16 end;
    s32 marker;
    s32 selection;
    s32 stage;
    s32 buttons;
    s32 mode;
    f32 increment;

    status = func_80028F54();
    D_70 = 0;
    switch (D_30) {
    case 0:
        D_34.value = 0;
        path = func_800508B4(D_34.bytes.path);
        if (path != 0) {
            animseqStartPath(D_34.bytes.path);
            path->flags |= 2;
        }
        D_30 = 4;
        if (status->mode == 1) {
            overlay58RefreshRankSet();
            overlay58EnsureResource();
            D_44 = 8;
        } else if (status->mode == 5) {
            D_44 = 2;
            gOverlay58CameraModeReloc = -1;
        } else {
            D_44 = 1;
        }
        break;
    case 4:
        if (D_2BC != 0) {
            amSndStop(D_2BC);
        }
        D_68 -= updateRate * 4;
        if (overlay41IsUnitScale(D_34.value) != 0) {
            D_30 = 1;
            amSndPlay(0x1FA, 0);
            if (status->mode == 5) {
                D_48 = 0;
                D_4C = 0;
                D_50 = 0x140;
            } else {
                D_48 = 0x140;
                D_50 = 0;
            }
            D_54 = 0;
            D_58 = 0;
            D_5C = 0;
            D_60 = 0;
        }
        break;
    case 1:
        D_68 -= updateRate * 4;
        func_overlay_058_F000138C_18B0574(updateRate);
        break;
    case 2:
        if (D_38 > 0) {
            D_38 -= updateRate;
            if (D_38 <= 0) {
                overlay58SetNodeValue(3, 0, 0.02f);
            }
        } else if (D_38 >= 0) {
            D_38 -= updateRate;
            if (D_38 < 0) {
                amSndPlay(0x32A, 0);
            }
        }
        if (overlay41IsUnitScale(D_34.value) != 0) {
            D_30 = 3;
            D_68 = 0;
        }
        break;
    case 3:
        D_68 += updateRate * 4;
        if (D_68 >= 0xFF) {
            D_68 = 0xFE;
        }
        if (D_2B0 == 0) {
            buttons = gOverlay58InputReloc;
            if (buttons & 0x4000) {
                if (D_2BC != 0) {
                    amSndStop(D_2BC);
                }
                amSndPlay(0xD, 0);
                animseqStopPath(D_34.bytes.path);
                D_34.value = 2;
                path = func_800508B4(D_34.bytes.path);
                if (path != 0) {
                    func_8005055C(D_34.bytes.path);
                    animseqStartPath(D_34.bytes.path);
                    path->flags |= 2;
                }
                D_30 = 4;
                overlay58SetNodeValue(3, 0, -0.01f);
                if (status->mode == 1) {
                    overlay58RefreshRankSet();
                    overlay58EnsureResource();
                    D_44 = 8;
                } else if (status->mode == 5) {
                    D_44 = 2;
                    gOverlay58CameraModeReloc = -1;
                } else {
                    D_44 = 1;
                }
            } else if (buttons & 0x9000) {
                if (D_2BC != 0) {
                    amSndStop(D_2BC);
                }
                if (gOverlay58MenuGateReloc == 0) {
                    if (status->active < 3) {
                        selection =
                            gOverlay58SelectionTableReloc[status->player]
                                                         [status->active + 1];
                        if (selection != -1) {
                            buttons = gOverlay58MenuBitsReloc[0x13];
                            mode = 1 << selection;
                            if (!(buttons & mode)) {
                                gOverlay58MenuBitsReloc[0x13] = buttons | mode;
                                func_800291B4();
                                func_8003A680(
                                    gOverlay58SelectionTableReloc
                                            [status->player]
                                            [status->active + 1] +
                                    0xE);
                            }
                        }
                    }
                    if (gOverlay58MenuReadyReloc > 0) {
                        D_30 = 5;
                        animseqStopPath(D_34.bytes.path);
                        D_34.value = 9;
                        path = func_800508B4(D_34.bytes.path);
                        if (path != 0) {
                            func_8005055C(D_34.bytes.path);
                            animseqStartPath(D_34.bytes.path);
                            path->flags |= 2;
                        }
                    }
                }
                if (D_30 != 5) {
                    D_70 = 1;
                    D_2B4 = 1;
                }
            }
        }
        break;
    case 5:
        if (overlay41IsUnitScale(D_34.value) != 0) {
            status->active++;
            if (status->active == 4) {
                gOverlay58ConfigAReloc = D_90[0]->mode;
                gOverlay58ConfigBReloc = D_90[1]->mode;
                gOverlay58ConfigCReloc = D_90[2]->mode;
                gOverlay58PlayerReloc = status->player;
                gOverlay58LevelReloc = 0x24;
                gOverlay58TrackReloc = 0;
                gOverlay58VehicleReloc = 8;
                gOverlay58TransitionModeReloc = 1;
            } else {
                gOverlay58ControllerCountReloc = 6;
                gOverlay58ControllerModeReloc = 5;
                joyCreateMap(D_F8);
                gOverlay58LevelReloc =
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active];
                gOverlay58TrackReloc = status->value4;
                gOverlay58VehicleReloc = 5;
                gOverlay58TransitionModeReloc = 0;
            }
            if (D_DC != 0) {
                mainChangeLevel(1, 0, 0, 0xF, 1, 0);
                D_DC = 0;
            }
            D_30 = 6;
            D_68 = 0;
        }
        break;
    case 6:
        break;
    }

    if (D_68 < 0) {
        D_68 = 0;
    }
    if ((D_70 != 0) && (D_2B0 == 0)) {
        if (D_2B4 != 0) {
            status->active++;
        }
        if (status->active == 4) {
            if (gOverlay58MenuGateReloc != 0) {
                if (D_DC != 0) {
                    mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                    D_DC = 0;
                }
            } else {
                gOverlay58ConfigAReloc = D_90[0]->mode;
                gOverlay58ConfigBReloc = D_90[1]->mode;
                gOverlay58ConfigCReloc = D_90[2]->mode;
                gOverlay58PlayerReloc = status->player;
                if (D_DC != 0) {
                    mainChangeLevel(0x24, 0, 0, 8, 1, 0);
                    D_DC = 0;
                }
            }
        } else if (gOverlay58MenuGateReloc != 0) {
            mainChangeCameras(gOverlay58CameraModeReloc);
            mode = gOverlay58CameraModeReloc;
            if (((mode == 2) || (mode == 3)) &&
                (gOverlay58CameraGateReloc != 0)) {
                gOverlay58ControllerModeReloc = 4 - mode;
            } else {
                gOverlay58ControllerModeReloc = 0;
            }
            joyCreateMap(D_F8);
            if (D_DC != 0) {
                mainChangeLevel(
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active],
                    status->value4, 0, 5, 1, 0);
                D_DC = 0;
            }
        } else {
            gOverlay58ControllerCountReloc = 6;
            gOverlay58ControllerModeReloc = 5;
            joyCreateMap(D_F8);
            if (D_DC != 0) {
                mainChangeLevel(
                    gOverlay58SelectionTableReloc[status->player]
                                                 [status->active],
                    status->value4, 0, 5, 1, 0);
                D_DC = 0;
            }
        }
        amSndPlay(0xC, 0);
        D_2B0 = 1;
    }
    if (D_68 < 0) {
        D_68 = 0;
    }

    path = func_800508B4(3);
    if ((path != 0) && (D_68 > 0)) {
        geometry = *path->object->geometry;
        verts = geometry->vertices;
        func_800221E8(&gOverlay58DisplayListReloc,
                      &gOverlay58MatrixReloc);
        command = gOverlay58DisplayListReloc++;
        command->w0 = 0xFA000000;
        command->w1 = (D_68 & 0xFF) | ~0xFF;
        for (stage = 0; stage <= D_6C; stage++) {
            start = 0;
            end = 0;
            /*
             * The initial sentinel is a direct load of D_1A0[0]. The cursor
             * is a second address, born inside the taken path, so the skip
             * branch can carry that lui and marker's -1 is not hoisted
             * across the draws.
             */
            if (D_1A0[0] != -1) {
                mapping = D_1A0;
                do {
                    if (mapping[0] ==
                        gOverlay58SelectionTableReloc[status->player][stage]) {
                        start = mapping[1];
                    }
                    if (stage == 3) {
                        end = 0x13;
                    } else if (
                        mapping[0] ==
                        gOverlay58SelectionTableReloc[status->player]
                                                     [stage + 1]) {
                        end = mapping[1];
                    }
                    mapping += 2;
                } while (mapping[0] != -1);
            }

            if (stage == D_6C) {
                increment = D_120 * (f32)updateRate;
                if ((D_2BC == 0) & (D_2C0 < 1.0f)) {
                    if (D_30 == 3) {
                        amSndPlay(0x32B, &D_2BC, D_6C);
                    }
                }
                overlay58DrawSegmentStrip(
                    verts[start].x, verts[start].y,
                    verts[start].z, verts[end].x,
                    verts[end].y, verts[end].z,
                    D_2C0);
                D_2C0 += increment;
                if (D_2C0 > 1.0f) {
                    D_2C0 = 1.0f;
                    if (D_2BC != 0) {
                        amSndStop(D_2BC);
                    }
                    overlay58DrawPointQuad((s32)verts[end].x,
                                           (s32)verts[end].y,
                                           (s32)verts[end].z);
                }
            } else {
                overlay58DrawSegmentStrip(
                    verts[start].x, verts[start].y,
                    verts[start].z, verts[end].x,
                    verts[end].y, verts[end].z,
                    1.0f);
            }

            if (!(D_2A8[status->player] &
                  ((u32)(gOverlay58ScreenModeReloc[0] << 5) >> 0x1C))) {
                marker = -1;
                if (status->player == 0) {
                    if (stage == 0) {
                        marker = start;
                    }
                } else if ((status->player > 0) && (status->player < 4)) {
                    if (stage == 3) {
                        marker = start;
                    } else if ((D_6C == 2) && (stage == 2) &&
                               (D_2C0 == 1.0f)) {
                        marker = end;
                    }
                }
                if (marker != -1) {
                    overlay58DrawLargePointQuad(
                        (s32)((f32)D_B8[status->player][0] +
                              verts[marker].x),
                        (s32)verts[marker].y,
                        (s32)((f32)D_B8[status->player][1] +
                              verts[marker].z));
                }
            }
            overlay58DrawPointQuad((s32)verts[start].x,
                                   (s32)verts[start].y,
                                   (s32)verts[start].z);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/func_overlay_058_F00005FC_18AF7E4/func_overlay_058_F00005FC_18AF7E4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_058_F00005FC_18AF7E4:start
 * symbol: func_overlay_058_F00005FC_18AF7E4
 * score: 536 differing words
 * frame: 0xA0
 * relocations: 269
 * first-mismatch: +0x0
 * summary: Pair 5 extra-ILOD (-12), lines 514, 525, 548; pair 1 s8 line 207. Split D_1A0 cursor; marker=-1 inside bit test. Stall: neg1, volatile scale, per-draw reloads.
 * PLATEAU-HANDOFF:func_overlay_058_F00005FC_18AF7E4:end
 */
