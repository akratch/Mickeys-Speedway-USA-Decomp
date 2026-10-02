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
typedef struct Overlay58Options {
    u32 flags;
    u8 pad04[0xF];
    u8 bits13;
} Overlay58Options;
extern Overlay58Options gOverlay58MenuBitsReloc;
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
 * Mickey-only reconstruction (no permitted donor counterpart).
 *
 * 2026-10-02, lane q-ovl9: 509 -> 137 masked at size delta +4.
 *   - The per-file `-O2 -g3` override was inherited and is refuted by the
 *     target's own prologue: the first `jal` carries the parameter home store
 *     in its delay slot, which `-g3` never schedules.  The overlay's matched
 *     whale builds at plain `-O2`, and so does this TU now.
 *   - The ninth callee-saved web was `&D_2C0`.  Its two comparisons against
 *     1.0f read through a pointer taken at entry, which keeps them out of the
 *     address web; with four direct references left the web costs less than
 *     a ninth saved register and is not coloured, so s8 and the 0x10 frame
 *     surplus disappear and the arg load after the sound call is fresh, as
 *     in the target.
 *   - `D_54 = 0` is stored in both arms of the mode test (the target's `b`
 *     carries that store in its delay slot).
 *   - The menu bits and the screen-mode nibble are one object (one ROM-table
 *     symbol, addends 0 and 0x13).
 *   - The case-3 selection row is addressed as `&table[0][0] + player * 4`.
 *   - Locals are declared in frame order (status first at 0x7C, increment
 *     at 0x58); the unused `verts`, `buttons` and `mode` are gone.
 *
 * Open: the case-3 table base and the `mainChangeCameras` mode address are
 * saved registers across their calls in the target (s0, s1) and
 * rematerialised here; the large-point-quad offsets convert before the
 * vertex loads in the target (two f32 temporaries do it at delta 0 but take
 * two frame slots); the `D_2BC == 0` load is scheduled later here.
 */
#ifdef NON_MATCHING
void func_overlay_058_F00005FC_18AF7E4(s32 updateRate) {
    Overlay58AnimPath *path;
    Overlay58PathGeometry *geometry;
    Overlay58Status *status;
    Overlay58Gfx *command;
    s16 *mapping;
    s16 start;
    s16 end;
    s32 marker;
    s32 selection;
    s32 stage;
    f32 *progressPtr;
    f32 progress;
    f32 increment;

    progressPtr = &D_2C0;
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
                D_54 = 0;
            } else {
                D_48 = 0x140;
                D_50 = 0;
                D_54 = 0;
            }
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
            if (gOverlay58InputReloc & 0x4000) {
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
            } else if (gOverlay58InputReloc & 0x9000) {
                if (D_2BC != 0) {
                    amSndStop(D_2BC);
                }
                if (gOverlay58MenuGateReloc == 0) {
                    if (status->active < 3) {
                        selection =
                            (&gOverlay58SelectionTableReloc[0][0] + status->player * 4)[status->active + 1];
                        if (selection != -1) {
                            if (!(gOverlay58MenuBitsReloc.bits13 & (1 << selection))) {
                                gOverlay58MenuBitsReloc.bits13 |= 1 << selection;
                                func_800291B4();
                                func_8003A680(
                                    (&gOverlay58SelectionTableReloc[0][0] + status->player * 4)[status->active + 1] +
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
            if (((gOverlay58CameraModeReloc == 2) ||
                 (gOverlay58CameraModeReloc == 3)) &&
                (gOverlay58CameraGateReloc != 0)) {
                gOverlay58ControllerModeReloc = 4 - gOverlay58CameraModeReloc;
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
                if ((D_2BC == 0) & (*progressPtr < 1.0f)) {
                    if (D_30 == 3) {
                        amSndPlay(0x32B, &D_2BC, D_6C);
                    }
                }
                overlay58DrawSegmentStrip(
                    geometry->vertices[start].x, geometry->vertices[start].y,
                    geometry->vertices[start].z, geometry->vertices[end].x,
                    geometry->vertices[end].y, geometry->vertices[end].z,
                    D_2C0);
                progress = D_2C0 + increment;
                D_2C0 = progress;
                if (progress > 1.0f) {
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
                  ((gOverlay58MenuBitsReloc.flags << 5) >> 0x1C))) {
                marker = -1;
                if (status->player == 0) {
                    if (stage == 0) {
                        marker = start;
                    }
                } else if ((status->player > 0) && (status->player < 4)) {
                    if (stage == 3) {
                        marker = start;
                    } else if ((D_6C == 2) && (stage == 2) &&
                               (*progressPtr == 1.0f)) {
                        marker = end;
                    }
                }
                if (marker != -1) {
                    overlay58DrawLargePointQuad(
                        (s32)((f32)D_B8[status->player][0] +
                              geometry->vertices[marker].x),
                        (s32)geometry->vertices[marker].y,
                        (s32)((f32)D_B8[status->player][1] +
                              geometry->vertices[marker].z));
                }
            }
            overlay58DrawPointQuad((s32)geometry->vertices[start].x,
                                   (s32)geometry->vertices[start].y,
                                   (s32)geometry->vertices[start].z);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/func_overlay_058_F00005FC_18AF7E4/func_overlay_058_F00005FC_18AF7E4.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_058_F00005FC_18AF7E4:start
 * symbol: func_overlay_058_F00005FC_18AF7E4
 * score: 137/829 words
 * frame: 0x88
 * relocations: 278
 * first-mismatch: +0x41C
 * summary: Plain -O2 (no -g3); &D_2C0 compares via entry pointer drop s8; frame order. Open: s0/s1 address webs, quad offsets.
 * PLATEAU-HANDOFF:func_overlay_058_F00005FC_18AF7E4:end
 */
