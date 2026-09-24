/*
 * Resident camera -- ROM 0x21EE0-0x25C20 (VRAM 0x800212E0).
 *
 * The 69 functions cover camera state, user viewports, projection setup,
 * sprite/model matrices, projection helpers and screen shake. The final
 * function ends four bytes before the existing 16-byte subsegment boundary.
 * No function uses an odd single-precision floating-point register, so this
 * block has no handwritten-assembly classification under docs/modules.md 6.2.
 *
 * PROVENANCE DISCLOSURE. The camera TU identification and the adopted names in
 * this file were read from Jet Force Gemini's public retail-derived decomp,
 * src/camera.c, as permitted by docs/CLEANROOM.md. Mickey's own bytes decide
 * every name and body: docs/modules.md records which names are tier A, which
 * are tier B role/order arguments, and which functions remain unresolved.
 * The matched functions below carry adapted bodies from the named permitted
 * JFG or DKR public decomps, each with its own PROVENANCE note at the point of
 * use; everything else in this split is still GLOBAL_ASM.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul; the projection-depth dot product
 * fixes the TU's multiply scheduler mode.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "game/gameVi.h"
#include "game/math.h"
#include "n_audio/mbi.h"

typedef struct {
    s16 yRotation;
    s16 xRotation;
    s16 zRotation;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} CameraTransform;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} CameraPoint;

typedef struct {
    CameraTransform transform;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    f32 fov;
    f32 shakeX;
    f32 shakeY;
    f32 shakeZ;
    u8 unk3C;
    u8 unk3D;
    s16 unk3E;
    f32 unk40;
    u8 pad44[6];
    s16 pitchOffset;
    u8 stateA;
    u8 stateB;
    u8 pad4E[6];
} Camera;

typedef struct {
    s16 attackEnd;
    s16 sustainEnd;
    s16 totalEnd;
    s16 timer;
    s32 magnitude;
} CameraShake;

typedef struct {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} CameraSpriteAnchor;

typedef struct {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    u8 *spriteData;
} CameraSprite;

typedef struct {
    s16 yRotation;
    s16 xRotation;
    s16 zRotation;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} CameraScaledTransform;

typedef union {
    CameraScaledTransform transform;
    f64 align;
} CameraAlignedScaledTransform;

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} CameraVertex;

typedef struct {
    CameraScaledTransform transform;
    u8 pad18[0x10];
    f32 frame;
} CameraObjectSegment;

typedef struct {
    s32 flags;
    u8 pad04[0x30];
} CameraViewportFlags;

typedef struct {
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 posX;
    s32 posY;
    s32 width;
    s32 height;
    s32 scissorX1;
    s32 scissorY1;
    s32 scissorX2;
    s32 scissorY2;
    s32 flags;
} CameraViewport;

typedef struct {
    s8 state;
    u8 pad01[0x53];
} CameraState3D;

typedef struct {
    s16 zRotation;
    u8 pad02[0x52];
} CameraZRotationEntry;

typedef struct {
    u8 pad00[0x50];
    f32 unk50;
    u8 pad54[0x43C - 0x54];
    s16 xRotation;
    u8 pad43E[2];
    s16 zRotation;
    u8 pad442[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} CameraSpritePlayer;

typedef struct {
    s16 xRotation;
    u8 pad02[2];
    s16 zRotation;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x30 - 0x18];
    f32 distance;
    u8 pad34[0x39 - 0x34];
    u8 alpha;
    u8 pad3A[0x40 - 0x3A];
    f32 *baseScale;
    s16 kind;
    s16 spriteType;
    u8 pad48[0x50 - 0x48];
    f32 *opacity;
    u8 pad54[0x64 - 0x54];
    CameraSpritePlayer *player;
} CameraSpriteActor;

typedef struct {
    s16 spriteType;
    s16 scaleIndex;
} CameraSpriteScaleEntry;

extern u8 D_80079F94;
extern s32 D_80079F8C;
extern s32 D_80079D48;
extern f32 D_80079F60;
extern f32 D_80079F48;
extern f32 D_80079F4C;
extern f32 D_80079F50;
extern f32 D_80079F54;
extern f32 D_80079F58[];
extern f32 D_80079F90;
extern CameraViewport D_80079C10[];
extern CameraViewportFlags D_80079C40[];
extern Vp D_80079D58[];
extern Vp D_80079E98[];
extern u8 D_80079F98[];
extern CameraState3D D_800CEA5D[];
extern u8 D_80079FA0[];
extern s32 D_800CEC84;
extern s32 D_800CEC88;
extern f32 D_800CEC8C;
extern f32 D_800CEC90;
extern u8 D_80079FA8[];
extern f32 D_80079FB0[];
extern Mtx D_800CED60[];
extern MtxF D_800CEC98;
extern CameraTransform D_800CEC68;
extern CameraTransform D_80079F18;
extern u16 D_800CEC94;
extern MtxF D_800CED18;
extern MtxF D_800CECD8;
extern Mtx *D_800CED58;
extern Mtx D_800CF160;
extern s32 D_800CEC60;
extern s32 D_800CEC64;
extern s8 D_800CEC80;
extern MtxF D_800CF1A0;
extern MtxF D_800CF1E0;
extern MtxF D_800CF220;
extern f32 D_800CF2A4;
extern f32 D_800CF2A8;
extern f32 D_800CF2AC;
extern f32 D_800CF2B0;
extern MtxF D_800CF2B8;
extern MtxF D_800CF2F8;
extern MtxF D_800CF260;
extern f32 D_800CF2A0;
extern f32 D_800D2FB4;
extern f32 D_80081A1C;
extern f32 D_80081A20;
extern f32 D_80081A24;
extern f32 D_80081A28;
extern f32 D_80081A2C;
extern f32 D_80081A30;
extern f32 D_80081A34;
extern f32 D_80081A38;
extern f32 D_80081A3C;
extern f32 D_80081A40;
extern f32 D_80081A44;
extern f32 D_80081A48;
extern Camera D_800CEA20[];
extern CameraZRotationEntry D_800CEA24[];
extern CameraShake D_800CEC18[];
extern s32 D_8007C854;
extern s32 D_8007C85C;
extern s32 D_80079FC8;
extern f32 D_80079FD8[];
extern CameraSpriteScaleEntry D_80079FF0[];
extern u8 D_8007BF0C;
extern u8 D_79FCC[];

void mtxf_mul(MtxF lhs, MtxF rhs, MtxF dest);
void mtxf_to_mtx(MtxF src, Mtx *dest);
void mtxf_translate_y(MtxF matrix, f32 y);
void matrixScale(f32 x, f32 y, f32 z, MtxF matrix);
void mathScaleMtx(MtxF matrix, f32 x, f32 y, f32 z);
void mathRSMtx(s32 rotation, f32 scale, f32 aspect, MtxF matrix);
void func_8004FAD0(MtxF matrix, u16 *perspNorm, f32 fovy, f32 aspect,
                   f32 nearPlane, f32 farPlane, f32 scale);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
void func_80029AB8(MtxF matrix, f32 scale);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
s16 Arctanf(f32 x, f32 y);
void func_8002AA50(CameraScaledTransform *transform, MtxF matrix);
void func_8002AB78(CameraTransform *transform, MtxF matrix);
void func_8002AE10(CameraTransform *transform, MtxF matrix);
void func_80024978(MtxF matrix);
void func_80034E54(Gfx **dlist, u8 *spriteData, s32 flags,
                   f32 frame, s32 alpha);
void func_80034434(s32 enabled);
void func_80023CCC(Gfx **dlist, Mtx **mtx, CameraVertex **vertices,
                   u8 *spriteData, s16 x, s16 y, s16 z, s16 angle, f32 scale,
                   f32 matrixScale, f32 frame, s32 flags, u8 alpha);
f32 sqrtf(f32 value);
s32 mathRnd(s32 minimum, s32 maximum);
s32 levelGetNumber(void);
u8 levelGetType(void);
s32 frontGet2PlayerSplit(void);
extern s32 levelInitRegionFlags(void);
extern void func_80021504(f32 fov, s32 force);
void func_80021838(s32 x, s32 y, s32 z, s32 zRotation, s32 xRotation,
                   s32 yRotation);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);
extern void camSetViewport(Gfx **dlist, s32 halfWidth, s32 halfHeight,
                           s32 centerX, s32 centerY, s32 regionFlags);
extern void func_80022794(Gfx **dlist, Mtx **mtx);
void camStopShakes(void);

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camInit. */
void camInit(void) {
    s32 i;

    D_800CEC64 = 0;
    D_800CEC60 = 0;
    D_800CEC80 = 0;
    _bzero(D_800CEA20, sizeof(Camera) * 6);
    for (i = 0; i < 6; i++) {
        D_800CEC64 = i;
        func_80021838(200, 200, 200, 0, 0, 180);
        D_800CEA20[D_800CEC64].fov = 60.0f;
    }
    camStopShakes();
    D_800CF2A0 = 60.0f;
    func_8004FAD0(D_800CEC98, &D_800CEC94, D_800CF2A0, 1.3333334f,
                  10.0f, D_80081A1C, 1.0f);
    D_80079F94 = (D_80079F94 + 1) & 0xF;
    mtxf_to_mtx(D_800CEC98, &D_800CED60[D_80079F94]);
}
f32 func_80021438(void) {
    return D_800D2FB4;
}
void func_80021444(s32 cameraIndex, s32 state) {
    if ((cameraIndex >= 0) && (cameraIndex < 6)) {
        D_80079F98[cameraIndex] = state;
        D_800CEA5D[cameraIndex].state = state;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camUseShake. */
void camUseShake(void) {
    D_800CEC84 = 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camIgnoreShake. */
void camIgnoreShake(void) {
    D_800CEC84 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetFOV. */
f32 camGetFOV(void) {
    return D_800CF2A0;
}
void func_800214AC(void) {
    D_800CEA20[D_800CEC64].stateA ^= 1;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camOverrideProjScales.
 */
void camOverrideProjScales(f32 scaleX, f32 scaleY) {
    D_800CEC8C = scaleX;
    D_800CEC90 = scaleY;
    D_800CEC88 = 1;
}
/*
 * PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetFOV,
 * with Mickey's camera-state mirror and region-specific projection scaling.
 */
/*
 * Retained evidence (audited 2026-08-29): configured C owns 0x214 bytes /
 * 133 words at .text+0x224 with a 0x28 frame and 43 candidate relocation
 * tuples. All 90 non-relocation words agree with the retained linked ELF;
 * the other 43 object-to-linked differences occur exactly at those tuples.
 * The linked function, complete 0x3D40-byte camera TU, and resident .main
 * section are byte-identical to ROM.
 *
 * The executable source tokens are unchanged since this object. The exact
 * retained full .bin predates it, and no independent target relocation
 * object survives, so one fresh target/object/link/full-bin reproof remains.
 */
void func_80021504(f32 fov, s32 force) {
    Camera *camera;
    s32 videoMode;
    s32 type;

    camera = &D_800CEA20[D_800CEC64];
    camera->fov = fov;
    if ((fov > 0.0f) && (fov < 90.0f) &&
        ((force != 0) || (fov != D_800CF2A0) ||
         (camera->stateB != camera->stateA))) {
        D_800CF2A0 = fov;
        if (camera->stateA != 0) {
            D_800CF2A0 = -D_800CF2A0;
            camera->stateB = camera->stateA;
        }
        func_8004FAD0(D_800CEC98, &D_800CEC94, D_800CF2A0, 1.3333334f,
                      10.0f, D_80081A20, 1.0f);
        D_80079F90 = D_800CEC98[0][0] / D_80081A24;
        videoMode = viGetVideoMode();
        if (D_800CEC88 != 0) {
            D_800CEC98[0][0] *= D_800CEC8C;
            D_800CEC98[1][1] *= D_800CEC90;
            D_800CEC88 = 0;
        } else if (videoMode & 1) {
            type = levelGetType();
            if (((type == 1) || (type == 2)) &&
                (levelGetNumber() != 0x2A)) {
                D_800CEC98[1][1] *= D_80081A28;
            } else {
                D_800CEC98[0][0] *= 0.75f;
            }
        }
        D_80079F94 = (D_80079F94 + 1) & 0xF;
        mtxf_to_mtx(D_800CEC98, &D_800CED60[D_80079F94]);
    }
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:cam_reset_fov.
 * Retained evidence (reviewed 2026-08-29): configured C owns 37 words
 * with a 0x28 frame and 14 candidate relocation tuples. Applying those
 * tuples at the linked symbol addresses reproduces all 37 words in the
 * retained post-object ELF; ROM 0x22318..0x223AC and the complete camera
 * TU are byte-identical to the baserom.
 *
 * No source/object reference, resident direct jump, main-reloc entry,
 * overlay SYMBOL relocation, or absolute pointer is retained. ROM-table
 * row 453 merely exports resident offset +0x212C8; it is not inbound
 * evidence. The exact full .bin predates the object, and no independent
 * target relocation object survives, so run one fresh target/object/link/
 * full-bin reproof.
 */
void func_80021718(void) {
    func_8004FAD0(D_800CEC98, &D_800CEC94, 60.0f, 1.3333334f, 10.0f,
                  D_80081A2C, 1.0f);
    D_80079F94 = (D_80079F94 + 1) & 0xF;
    mtxf_to_mtx(D_800CEC98, &D_800CED60[D_80079F94]);
}
MtxF *func_800217AC(void) {
    return &D_800CF260;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camDistance. */
f32 camDistance(f32 x, f32 y, f32 z) {
    Camera *cam;
    f32 dx;
    f32 dy;
    f32 dz;

    cam = &D_800CEA20[D_800CEC64];
    dx = x - cam->transform.x;
    dy = y - cam->transform.y;
    dz = z - cam->transform.z;
    return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:camera_reset, with Mickey's own field layout and store order.
 */
void func_80021838(s32 x, s32 y, s32 z, s32 zRotation, s32 xRotation,
                   s32 yRotation) {
    Camera *camera;
    u8 *states;
    f32 floatX;
    f32 floatY;
    f32 sourceX;
    f32 floatZ;

    camera = &D_800CEA20[D_800CEC64];
    camera->transform.zRotation = zRotation * 182;
    floatX = x;
    sourceX = floatX;
    floatY = y;
    camera->transform.xRotation = xRotation * 182;
    floatZ = z;
    camera->transform.yRotation = yRotation * 182;
    states = D_80079F98;
    camera->pitchOffset = 0;
    camera->transform.x = sourceX;
    camera->transform.y = floatY;
    camera->unk18 = sourceX;
    camera->transform.z = floatZ;
    camera->unk1C = floatY;
    camera->unk20 = floatZ;
    camera->unk3C = 0;
    camera->shakeX = 0.0f;
    camera->shakeY = 0.0f;
    camera->shakeZ = 0.0f;
    camera->unk24 = 128.0f;
    camera->unk28 = 32.0f;
    camera->unk3D = states[D_800CEC64];
    camera->unk3E = -1;
    camera->unk40 = 0.0f;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetMode. */
s32 camGetMode(void) {
    return D_800CEC60;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetMode. */
s32 camSetMode(s32 mode) {
    if ((mode < 0) || (mode >= 4)) {
        mode = 0;
    }
    D_800CEC60 = mode;
    if (D_800CEC60 < D_800CEC64) {
        D_800CEC64 = 0;
    }
    return mode + 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetNo. */
s32 camGetNo(void) {
    return D_800CEC64;
}
Camera *func_80021970(s32 cameraIndex) {
    return &D_800CEA20[cameraIndex];
}
/*
 * PROVENANCE: name and role from JFG's public decomp,
 * src/camera.c:camSetNo; body reconstructed from Mickey-only evidence.
 */
void camSetNo(s32 camNo) {
    D_800CEC64 = camNo;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetWaterLine. */
u8 camGetWaterLine(s32 camNo) {
    return D_80079FA8[camNo];
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetWaterLine. */
void camSetWaterLine(s32 camNo, s32 waterLine) {
    if ((camNo >= 0) && (camNo < 4)) {
        D_80079FA8[camNo] = waterLine;
    }
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:copy_viewports_to_stack; JFG's public src/camera.c supplies
 * the camUserViewTick role while Mickey supplies the six-camera bound.
 *
 * Retained evidence (audited 2026-08-29): the pre-comment configured object
 * owns 0x1A0 bytes / 104 words at .text+0x6F0, uses an 0x8 frame saving s0,
 * and carries eight HI16/LO16 records: two pairs for D_80079D48 and one pair
 * each for D_80079C10 and D_80079D58. Its linked range and complete camera TU
 * are byte-identical to ROM. The current executable source tokens are
 * unchanged; fresh current-source object/link/full-bin proof remains due.
 *
 * D_80079D48 initializes to zero and is toggled only here, so port spans
 * 10..20. Linked storage gives D_80079D58 exactly twenty Vp entries and places
 * D_80079E98 immediately afterward; port 20 therefore resolves to
 * D_80079E98[0]. The physical alias is proven, but original intent is not.
 */
void func_800219D0(void) {
    s32 width;
    s32 height;
    s32 port;
    s32 yPos;
    s32 xPos;
    s32 i;

    D_80079D48 = 1 - D_80079D48;
    for (i = 0; i < 6; i++) {
        if (D_80079C10[i].flags & 4) {
            D_80079C10[i].flags &= ~1;
        } else if (D_80079C10[i].flags & 2) {
            D_80079C10[i].flags |= 1;
        }
        D_80079C10[i].flags &= ~6;
        if (D_80079C10[i].flags & 1) {
            if (!(D_80079C10[i].flags & 8)) {
                xPos = (((D_80079C10[i].x2 - D_80079C10[i].x1) + 1) << 1) +
                       (D_80079C10[i].x1 * 4);
            } else {
                xPos = D_80079C10[i].posX;
                xPos *= 4;
            }
            if (!(D_80079C10[i].flags & 0x10)) {
                yPos = (((D_80079C10[i].y2 - D_80079C10[i].y1) + 1) << 1) +
                       (D_80079C10[i].y1 * 4);
            } else {
                yPos = D_80079C10[i].posY;
                yPos *= 4;
            }
            if (!(D_80079C10[i].flags & 0x20)) {
                width = D_80079C10[i].x2 - D_80079C10[i].x1;
                width += 1;
                width *= 2;
            } else {
                width = D_80079C10[i].width;
                width *= 2;
            }
            if (!(D_80079C10[i].flags & 0x40)) {
                height = (D_80079C10[i].y2 - D_80079C10[i].y1) + 1;
                height *= 2;
            } else {
                height = D_80079C10[i].height;
                height *= 2;
            }
            port = i;
            port += (D_80079D48 * 5) + 10;
            D_80079D58[port].vp.vtrans[0] = xPos;
            D_80079D58[port].vp.vtrans[1] = yPos;
            D_80079D58[port].vp.vscale[0] = width;
            D_80079D58[port].vp.vscale[1] = height;
        }
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camEnableUserView. */
void camEnableUserView(s32 camNo, s32 immediate) {
    CameraViewport *viewport;

    if (immediate != 0) {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 1;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 2;
    }
    viewport->flags &= ~4;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camDisableUserView. */
void camDisableUserView(s32 camNo, s32 immediate) {
    CameraViewport *viewport;

    if (immediate != 0) {
        viewport = &D_80079C10[camNo];
        viewport->flags &= ~1;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 4;
    }
    viewport->flags &= ~2;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camIsUserView. */
s32 camIsUserView(s32 camNo) {
    return D_80079C40[camNo].flags & 1;
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:viewport_menu_set; JFG's public src/camera.c supplies the
 * camSetUserView role while Mickey supplies the video-size call and layout.
 */
void func_80021C88(s32 camNo, s32 x1, s32 y1, s32 x2, s32 y2) {
    s32 swap;
    struct {
        u32 height;
        u32 width;
    } videoSize;

    viGetCurrentSize((s32 *) &videoSize.width, (s32 *) &videoSize.height);
    if (x2 < x1) {
        swap = x1;
        x1 = x2;
        x2 = swap;
    }
    if (y2 < y1) {
        swap = y1;
        y1 = y2;
        y2 = swap;
    }
    if (((u32) x1 >= videoSize.width) || (x2 < 0) ||
        ((u32) y1 >= videoSize.height) ||
        (y2 < 0)) {
        D_80079C10[camNo].scissorX1 = 0;
        D_80079C10[camNo].scissorY1 = 0;
        D_80079C10[camNo].scissorX2 = 0;
        D_80079C10[camNo].scissorY2 = 0;
    } else {
        if (x1 < 0) {
            D_80079C10[camNo].scissorX1 = 0;
        } else {
            D_80079C10[camNo].scissorX1 = x1;
        }
        if (y1 < 0) {
            D_80079C10[camNo].scissorY1 = 0;
        } else {
            D_80079C10[camNo].scissorY1 = y1;
        }
        if ((u32) x2 >= videoSize.width) {
            D_80079C10[camNo].scissorX2 = videoSize.width - 1;
        } else {
            D_80079C10[camNo].scissorX2 = x2;
        }
        if ((u32) y2 >= videoSize.height) {
            D_80079C10[camNo].scissorY2 = videoSize.height - 1;
        } else {
            D_80079C10[camNo].scissorY2 = y2;
        }
    }
    D_80079C10[camNo].x1 = x1;
    D_80079C10[camNo].x2 = x2;
    D_80079C10[camNo].y1 = y1;
    D_80079C10[camNo].y2 = y2;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camSetUserViewSpecial.
 */
void camSetUserViewSpecial(s32 camNo, s32 posX, s32 posY, s32 width,
                           s32 height) {
    CameraViewport *viewport;

    if (posX != 0x8000) {
        viewport = &D_80079C10[camNo];
        viewport->posX = posX;
        viewport->flags |= 8;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags &= ~8;
    }
    if (posY != 0x8000) {
        viewport->posY = posY;
        viewport->flags |= 0x10;
    } else {
        viewport->flags &= ~0x10;
    }
    if (width != 0x8000) {
        viewport->width = width;
        viewport->flags |= 0x20;
    } else {
        viewport->flags &= ~0x20;
    }
    if (height != 0x8000) {
        viewport->height = height;
        viewport->flags |= 0x40;
        return;
    }
    viewport->flags &= ~0x40;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camGetVisibleUserView.
 */
s32 camGetVisibleUserView(s32 camNo, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    CameraViewport *viewport = &D_80079C10[camNo];

    *x1 = viewport->scissorX1;
    *x2 = viewport->scissorX2;
    *y1 = viewport->scissorY1;
    *y2 = viewport->scissorY2;
    if ((*x1 | *x2 | *y1 | *y2) == 0) {
        return 0;
    }
    return 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetUserView. */
void camGetUserView(s32 camNo, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    CameraViewport *viewport = &D_80079C10[camNo];

    *x1 = viewport->x1;
    *y1 = viewport->y1;
    *x2 = viewport->x2;
    *y2 = viewport->y2;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camGetWindowLimits; Mickey's draft supplies the inset margins
 * and split-orientation behavior.
 */
void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1, u32 *x2,
                   u32 *y2) {
    s32 videoMode;
    u32 halfWidth;
    u32 halfHeight;
    u32 marginWidth;
    u32 marginHeight;

    if (D_80079C10[camNo].flags & 1) {
        *x1 = D_80079C10[camNo].scissorX1;
        *y1 = D_80079C10[camNo].scissorY1;
        *x2 = D_80079C10[camNo].scissorX2;
        *y2 = D_80079C10[camNo].scissorY2;
        return;
    }

    videoMode = viGetVideoMode();
    viGetCurrentSize((s32 *) x2, (s32 *) y2);
    *x1 = 0;
    *y1 = 0;
    halfWidth = *x2 >> 1;
    halfHeight = *y2 >> 1;
    marginWidth = *x2 / 20U;
    marginHeight = *y2 / 20U;

    switch (mode) {
        case 1:
            if (videoMode & 1) {
                marginHeight = 0;
            }
            if (frontGet2PlayerSplit() != 0) {
                if (D_80079C10 == &D_80079C10[camNo]) {
                    *x1 = marginWidth;
                    *x2 = halfWidth;
                } else {
                    *x1 = halfWidth;
                    *x2 -= marginWidth;
                }
            } else if (D_80079C10 == &D_80079C10[camNo]) {
                *y1 = marginHeight;
                *y2 = halfHeight;
            } else {
                *y1 = halfHeight;
                *y2 -= marginHeight;
            }
            break;
        case 2:
        case 3:
            if (videoMode & 1) {
                marginHeight = 0;
            }
            if (camNo & 1) {
                *x1 = halfWidth;
                *x2 -= marginWidth;
            } else {
                *x1 = marginWidth;
                *x2 = halfWidth;
            }
            if (camNo & 2) {
                *y1 = halfHeight;
                *y2 -= marginHeight;
            } else {
                *y1 = marginHeight;
                *y2 = halfHeight;
            }
            break;
        case 0:
            break;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetView. */
void func_800221E8(Gfx **dlist, Mtx **mtx) {
    u32 halfWidth;
    struct {
        u32 pad;
        u32 lry;
        u32 lrx;
        u32 uly;
        u32 ulx;
    } win;
    u32 halfHeight;
    s32 videoMode;

    func_80021FB0(D_800CEC60, D_800CEC64, (s32 *)&win.ulx,
                  (s32 *)&win.uly, &win.lrx, &win.lry);

    videoMode = viGetVideoMode();
    if ((videoMode == 2) || (videoMode == 3)) {
        halfWidth = 224;
        halfHeight = 168;
    } else {
        halfWidth = 160;
        halfHeight = 120;
    }

    if (D_800CEC60 >= 2) {
        halfWidth >>= 1;
        halfHeight >>= 1;
    }

    if (D_80079F8C != 0) {
        win.lrx >>= 1;
        win.lry >>= 1;
        halfWidth >>= 1;
        halfHeight >>= 1;
    }

    if (D_80079FA0[D_800CEC64] != 0) {
        halfWidth *= D_80079FB0[D_800CEC64];
        halfHeight *= D_80079FB0[D_800CEC64];
        gSPClipRatio((*dlist)++, FRUSTRATIO_1);
    }

    gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE,
                  win.ulx, win.uly, win.lrx, win.lry);
    camSetViewport(dlist, halfWidth, halfHeight, (win.lrx + win.ulx) >> 1,
                   (win.lry + win.uly) >> 1, levelInitRegionFlags());

    if (mtx != NULL) {
        func_80022794(dlist, mtx);
    }
}
void func_80022604(s32 arg0) {
    D_80079F8C = arg0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetScissor. */
void camSetScissor(Gfx **dlist) {
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;

    func_80021FB0(D_800CEC60, D_800CEC64, (s32 *) &x1, (s32 *) &y1,
                  &x2, &y2);
    gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE, x1, y1, x2, y2);
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camGetPlayerProjMtx.
 */
void camGetPlayerProjMtx(s32 player, MtxF dest) {
    mtxf_mul(D_800CF1A0, D_800CEC98, dest);
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetProjMtx. */
void func_80022794(Gfx **dlist, Mtx **mtx) {
    Camera *camera;

    camera = &D_800CEA20[D_800CEC64];
    if ((D_800CF2A0 != camera->fov) ||
        (camera->stateA != camera->stateB) ||
        (D_80079FA0[D_800CEC64] != 0)) {
        func_80021504(camera->fov, 0);
    }

    if (dlist != NULL) {
        gSPPerspNormalize((*dlist)++, D_800CEC94);
    }

    D_800CEC68.yRotation = camera->transform.yRotation + 0x8000;
    D_800CEC68.xRotation = camera->transform.xRotation + camera->pitchOffset;
    D_800CEC68.zRotation = camera->transform.zRotation;
    D_800CEC68.x = -camera->transform.x;
    D_800CEC68.y = -camera->transform.y;
    D_800CEC68.z = -camera->transform.z;
    if (D_800CEC84) {
        D_800CEC68.x -= camera->shakeX;
        D_800CEC68.y -= camera->shakeY;
        D_800CEC68.z -= camera->shakeZ;
    }
    func_8002AE10(&D_800CEC68, D_800CF1A0);
    mtxf_mul(D_800CF1A0, D_800CEC98, D_800CED18);

    if (dlist != NULL) {
        mtxf_to_mtx(D_800CED18, *mtx);
        gSPMatrix((*dlist)++, (u32)*mtx + 0x80000000,
                  G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        (*mtx)++;
    }

    D_800CEC68.yRotation = -0x8000 - camera->transform.yRotation;
    D_800CEC68.xRotation =
        -(camera->transform.xRotation + camera->pitchOffset);
    D_800CEC68.zRotation = -camera->transform.zRotation;
    D_800CEC68.x = camera->transform.x;
    D_800CEC68.y = camera->transform.y;
    D_800CEC68.z = camera->transform.z;
    if (D_800CEC84) {
        D_800CEC68.x += camera->shakeX;
        D_800CEC68.y += camera->shakeY;
        D_800CEC68.z += camera->shakeZ;
    }
    func_8002AB78(&D_800CEC68, D_800CF1E0);
    mtxf_to_mtx(D_800CF1E0, &D_800CF160);
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camOrthoYAspect. */
void camOrthoYAspect(f32 aspect) {
    D_80079F60 = aspect;
}
void func_80022A44(f32 arg0) {
    D_80079F48 = arg0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStandardOrtho. */
void camStandardOrtho(Gfx **dlist, Mtx **mtx) {
    u32 width;
    u32 height;
    Vp *viewport;

    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    if (D_80079F8C != 0) {
        width >>= 1;
        height >>= 1;
        viewport = &D_80079E98[(D_800CEC64 & 3) | 4];
    } else {
        viewport = &D_80079D58[D_800CEC64 + 5];
    }
    viewport->vp.vscale[0] = width * 2;
    viewport->vp.vscale[1] = width * 2;
    viewport->vp.vtrans[0] = width * 2;
    viewport->vp.vtrans[1] = height * 2;
    gSPViewport((*dlist)++, (u32) viewport + 0x80000000);
    func_80024978(D_800CED18);
    mtxf_to_mtx(D_800CED18, *mtx);
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 0);
    (*mtx)++;
}

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStandardPersp. */
void camStandardPersp(Gfx **dlist, Mtx **mtx) {
    gSPPerspNormalize((*dlist)++, D_800CEC94);
    func_8002AE10(&D_80079F18, D_800CF220);
    mtxf_mul(D_800CF220, D_800CEC98, D_800CED18);
    mtxf_to_mtx(D_800CED18, *mtx);
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 0);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetViewport. */
void camSetViewport(Gfx **dlist, s32 halfWidth, s32 halfHeight, s32 centerX,
                    s32 centerY, s32 regionFlags) {
    s32 camNo;
    Vp *viewport;

    camNo = D_800CEC64;
    if (D_80079F8C != 0) {
        viewport = &D_80079E98[camNo & 3];
    } else {
        viewport = &D_80079D58[camNo];
    }
    if (!(D_80079C40[camNo].flags & 1)) {
        viewport->vp.vtrans[0] = centerX * 4;
        viewport->vp.vtrans[1] = centerY * 4;
        viewport->vp.vscale[0] = halfWidth * 4;
        viewport->vp.vscale[1] = halfHeight * 4;
        if (regionFlags != 0) {
            viewport->vp.vscale[0] = -viewport->vp.vscale[0];
        }
    }
    gSPViewport((*dlist)++, (u32) viewport + 0x80000000);
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camResetView. */
void func_80022D20(Gfx **dlist) {
    u32 height;
    u32 width;

    D_800CEC64 = 4;
    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    if (!(D_80079C40[D_800CEC64].flags & 1)) {
        gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE, 0, 0, width - 1,
                      height - 1);
        camSetViewport(dlist, width >> 1, height >> 1, width >> 1,
                       height >> 1, 0);
    } else {
        camSetScissor(dlist);
        camSetViewport(dlist, 0, 0, 0, 0, 0);
    }
    D_800CEC64 = 0;
}
/* Mickey-only camera-relative billboard offset reconstruction. */
void func_80022E80(CameraScaledTransform *transform) {
    f32 x;
    f32 y;
    f32 z;
    CameraAlignedScaledTransform inverse;

    inverse.transform.yRotation = -transform->yRotation;
    inverse.transform.xRotation = -transform->xRotation;
    inverse.transform.zRotation = -transform->zRotation;
    inverse.transform.x = 0.0f;
    inverse.transform.y = 0.0f;
    inverse.transform.z = 0.0f;
    inverse.transform.scale = 1.0f;
    func_8002AA50(transform, D_800CF2B8);
    func_8002AE10((CameraTransform *) &inverse.transform, D_800CF2F8);
    D_800CF2B0 = 1.0f / transform->scale;
    x = D_800CEA20[D_800CEC64].transform.x - transform->x;
    y = D_800CEA20[D_800CEC64].transform.y - transform->y;
    z = D_800CEA20[D_800CEC64].transform.z - transform->z;
    mtxf_transform_point(D_800CF2F8, x, y, z, &x, &y, &z);
    D_800CF2A4 = x * D_800CF2B0;
    D_800CF2A8 = y * D_800CF2B0;
    D_800CF2AC = z * D_800CF2B0;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: JFG's public src/camera.c identifies the camDoSprite role;
 * this substantially different body is reconstructed from Mickey-only data.
 *
 * The eight-byte frame excess and the twenty-byte transformed-coordinate home
 * shift were ONE fault, and tools/frame_census.py separated them from the
 * scheduling residual they were hiding behind.  The candidate's ladder was the
 * target's ladder plus eight, slot for slot, once the declarations were put in
 * the target's order; the two numbers were the same declaration list read two
 * ways.  What the census forced:
 *
 *  a. angle, pitch and frameStep move ABOVE transformedX.  The target's homed
 *     block starts transformedX three slots further down than a naive order
 *     gives, and those three slots are register-class -- they never reach
 *     memory, so nothing but the census could see them.
 *
 *  b. sine moves between rotatedX and rotatedZ, one slot the target reserves
 *     and never writes.
 *
 *  c. `transform` is the LAST homed object.  Everything m2c declared after it
 *     -- wrappedFrame, color -- has to go, because the block has to be exactly
 *     0x68 bytes of declarations plus the 0x18 struct.  Both of those live
 *     ranges are disjoint from `pitch`, which is dead from the angleProduct
 *     store onwards, so one s32 carries all three (L115: reusing a local that
 *     is already dead imports no interference at zero width).
 *
 * Measured with tools/align_symbol.py: 203 -> 157 masked words, frame
 * 0xB8 -> 0xB0 exact, the ladders identical slot for slot, immediate-only
 * 60 -> 1, byte-exact 172 -> 219 of 369, really different 62 -> 45.
 *
 * 2026-09-12, lane p11-mid.  The previous line here recorded `sprite->frame &
 * 0xFFFF` as REJECTED at +4 bytes and 263 words.  That rejection is about the
 * spelling in isolation and it should be reopened, because the +1 it costs has
 * a -1 waiting for it in the same function: we emit `move a0,v0` at +0x1CC,
 * copying the divisor the target never copies, and the target emits lh plus
 * andi 0xFFFF at +0x24C and +0x254 where we emit one lhu.  Paired with whatever
 * removes the divisor copy the spelling is size-neutral; alone it is not.
 *
 * The naming residual is one ugen draw, read off DKWB_UGEN_SCHED plus
 * DKWB_UGEN_TRACE.  This procedure's GP free list is t6 t7 t8 t9 t2 t3 t4 t5 --
 * v0, v1, t0 and t1 are REMOVEd before the first draw -- and it is FIFO.
 * Everything from +0x0 to +0x98 is byte-identical, and the first naming
 * difference at +0x9C is the `sprite` reload `lw 196(sp)`, which takes t2 for us
 * and t3 for the target: exactly one free-list position later.  The target
 * therefore spends one more GP draw in the transformed-coordinate block, and
 * since its emitted code up to +0x98 agrees word for word, that draw is folded
 * away in its own output -- the [L149] case, which no register census can see.
 *
 * Swept and flat against it: the five statements from `divisor = sprite->divisor`
 * through the volatile `angleProduct` store admit 30 orders once `divisor`
 * precedes `frameStep` and `quadrant` precedes `angle &= 0x3FFF`; ten tie at 157
 * and twenty are 167.  Moving `horizontal = sqrtf(...)` to six earlier positions
 * gives 263 to 277 at size deltas +4 to +36 -- the target does schedule that
 * multiply pair before the quadrant branch where we schedule it after, but every
 * source position that moves it also spills localX and localZ.  Head spellings:
 * interleaving each scale with its own subtraction 330 at -4, compound `*=` 157,
 * reversing the three local subtractions 287 at -4, swapping the multiply
 * operands 330 at +8, deleting the `register` spriteEarly carrier 215 at 0.
 */
void func_80022FD4(Gfx **dlist, Mtx **mtx, void *vertices,
                   CameraSpriteAnchor *anchor, f32 *opacity,
                   CameraSprite *sprite, s32 flags, s32 alpha) {
    register CameraSprite *spriteEarly;
    volatile s32 angleProduct;
    s32 quadrant;
    s32 angle;
    s32 pitch;
    s32 frameStep;
    f32 transformedX;
    f32 transformedY;
    f32 transformedZ;
    f32 localX;
    f32 localY;
    f32 localZ;
    f32 rotatedX;
    f32 sine;
    f32 rotatedZ;
    f32 cosine;
    f32 matrixScale;
    f32 horizontal;
    CameraScaledTransform transform;
    u16 divisor;
    Gfx *cmd;

    spriteEarly = sprite;
    transformedX = spriteEarly->x - anchor->x;
    transformedY = spriteEarly->y - anchor->y;
    transformedZ = spriteEarly->z - anchor->z;
    mtxf_transform_point(D_800CF2F8, transformedX, transformedY,
                         transformedZ, &transformedX, &transformedY,
                         &transformedZ);
    transformedX = transformedX * D_800CF2B0;
    transformedY = transformedY * D_800CF2B0;
    transformedZ = transformedZ * D_800CF2B0;
    localX = D_800CF2A4 - transformedX;
    localY = D_800CF2A8 - transformedY;
    localZ = D_800CF2AC - transformedZ;

    cosine = func_8002A8C0(sprite->angle);
    sine = func_8002A8BC(sprite->angle);
    rotatedX = (localX * sine) + (localZ * cosine);
    rotatedZ = (localZ * sine) - (localX * cosine);
    angle = Arctanf(rotatedX,
                    sqrtf((localY * localY) + (rotatedZ * rotatedZ)));
    cosine = -func_8002A8C0(Arctanf(rotatedX, rotatedZ));
    if (rotatedZ < 0.0f) {
        rotatedZ = -rotatedZ;
        cosine = -cosine;
        angle = -angle;
    }

    pitch = Arctanf(localY, rotatedZ);
    if (pitch >= 0x8001) {
        pitch += 0xFFFF0000;
    }

    divisor = sprite->divisor;
    quadrant = angle & 0x4000;
    frameStep = (s32)sprite->spriteData[0] / (s32)divisor;
    angle &= 0x3FFF;
    angleProduct = (s32)((f32)pitch * cosine);
    if (quadrant != 0) {
        angle = 0x3FFF - angle;
    }
    angle = (angle * frameStep) >> 14;
    if ((s32)divisor >= 2) {
        pitch = (u16)sprite->frame;
        while (pitch >= sprite->frameCount) {
            pitch -= sprite->frameCount;
        }
        angle += frameStep * (((s32)divisor * pitch) /
                              sprite->frameCount);
    }

    horizontal = sqrtf((localX * localX) + (localZ * localZ));
    transform.yRotation = Arctanf(localX, localZ);
    transform.xRotation = -Arctanf(localY, horizontal);
    transform.zRotation = angleProduct;
    transform.scale = sprite->transformScale;
    transform.x = transformedX;
    transform.y = transformedY;
    transform.z = transformedZ;
    func_8002AA50(&transform, D_800CF220);

    if (quadrant != 0) {
        D_800CF220[0][0] = -D_800CF220[0][0];
        D_800CF220[0][1] = -D_800CF220[0][1];
        D_800CF220[0][2] = -D_800CF220[0][2];
    }
    matrixScale = sprite->matrixScale;
    if (matrixScale != 1.0f) {
        func_80029AB8(D_800CF220, matrixScale);
    }
    mtxf_mul(D_800CF220, D_800CF2B8, D_800CECD8);
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF220);
    mtxf_to_mtx(D_800CF220, *mtx);
    D_800CED58 = *mtx;

    if (flags & 4) {
        flags |= 1;
    } else {
        flags &= ~1;
    }

    if (D_8007C854 != 0) {
        if (opacity != NULL) {
            pitch = *opacity * D_8007C85C;
        } else {
            pitch = D_8007C85C;
        }
    } else {
        if (opacity != NULL) {
            pitch = *opacity * 255.0f;
        } else {
            pitch = 255;
        }
    }

    pitch &= 0xFF;
    gDPSetPrimColor((*dlist)++, 0, 0, pitch, pitch, pitch, alpha);

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w0 = 0x01020040;
    cmd->words.w1 = (u32)*mtx + 0x80000000;
    (*mtx)++;

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w0 = (((((u32)D_79FCC & 6) | 8) & 0xFF) << 16) |
                    0x04000012;
    cmd->words.w1 = (u32)D_79FCC;

    func_80034E54(dlist, sprite->spriteData, flags & 0xF,
                  (f32)angle, alpha);

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w1 = 0;
    cmd->words.w0 = 0xBC00000A;

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w1 = -1;
    cmd->words.w0 = 0xFA000000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022FD4.s")
#endif
#ifdef NON_MATCHING
/* Workbench: size delta 0 and frame 0x90 both closed; 13 words remain.
 * Closed by: func_80034434 takes one argument (the varargs call spilled its
 * extra operands); GBI colour macros with the raw colour (drops the Gfx local
 * and the separate mask); declarations laid out so every home lands on the
 * target's ladder; the index reassigned to the scale index (x = f(x)); angle
 * finished after frameCount is read; an empty region at the join, which
 * raises the callee toll past dlist's total save so dlist stays in its home
 * (no s1).
 * Remains: baseScale and distanceScale tie at total save 6 and baseScale wins
 * f16 (a force puts it at 3 words), the propagated copy in the distance
 * multiply, and the order of the two Arctanf argument loads. */
void func_80023598(Gfx **dlist, Mtx **mtx, CameraVertex **vertices,
                   CameraSpriteActor *actor, u8 *spriteData, s32 alpha) {
    CameraSpritePlayer *player;
    s32 spriteTypeIndex;
    f32 baseScale;
    s32 angle;
    s32 mirroredFrame;
    s32 frameCount;
    s32 doubledFrameCount;
    s32 frame;
    s32 color;
    f32 threshold;
    f32 scale;
    f32 matrixScale;
    f32 multiplier;
    f32 x;
    f32 y;
    f32 z;
    f32 distanceScale;
    s16 xRotation;
    s16 zRotation;

    if (actor->kind == 1) {
        player = actor->player;
        viGetVideoMode();
        spriteTypeIndex = 0;
        while ((D_80079FF0[spriteTypeIndex].spriteType !=
                actor->spriteType) &&
               (D_80079FF0[spriteTypeIndex].spriteType != -1)) {
            spriteTypeIndex++;
        }
        spriteTypeIndex = D_80079FF0[spriteTypeIndex].scaleIndex;
        baseScale = *actor->baseScale;
        scale = D_80079FD8[spriteTypeIndex] * baseScale;
        matrixScale = player->unk50;
        xRotation = player->xRotation;
        zRotation = player->zRotation;
        distanceScale = player->scale;
        x = player->x;
        y = player->y;
        z = player->z;
        if (D_8007BF0C != 0) {
            distanceScale = baseScale;
            if (D_800CEC60 == 1) {
                threshold = 400.0f;
                multiplier = D_80081A30;
            } else {
                threshold = 250.0f;
                multiplier = D_80081A34;
            }
            if (threshold < actor->distance) {
                multiplier = ((actor->distance - threshold) * multiplier) +
                             1.0f;
                if (multiplier > 2.0f) {
                    multiplier = 2.0f;
                }
                distanceScale *= multiplier;
            }
        }
    } else {
        xRotation = actor->xRotation;
        zRotation = actor->zRotation;
        distanceScale = actor->scale;
        x = actor->x;
        y = actor->y;
        z = actor->z;
        scale = distanceScale;
        matrixScale = 1.0f;
        baseScale = *actor->baseScale;
    }

    do {
    } while (0);
    scale *= distanceScale / baseScale;
    angle = Arctanf(D_800CEA20[D_800CEC64].transform.x - x,
                    D_800CEA20[D_800CEC64].transform.z - z);
    frameCount = spriteData[0] - 1;
    angle = xRotation - angle;
    doubledFrameCount = frameCount * 2;
    frame = ((((0x8000 / doubledFrameCount) + angle) & 0xFFFF) *
             doubledFrameCount) >> 16;
    mirroredFrame = frame;
    if (frameCount < frame) {
        mirroredFrame = doubledFrameCount - frame;
        D_80079FC8 = 1;
    }

    if (D_8007C854 != 0) {
        if (actor->opacity != NULL) {
            color = *actor->opacity * D_8007C85C;
        } else {
            color = D_8007C85C;
        }
    } else {
        if (actor->opacity != NULL) {
            color = *actor->opacity * 255.0f;
        } else {
            color = 255;
        }
    }

    gDPSetPrimColor((*dlist)++, 0, 0, color, color, color, actor->alpha);
    gDPSetEnvColor((*dlist)++, 0, 0, 0, 0);

    func_80034434(1);
    func_80023CCC(dlist, mtx, vertices, spriteData, x, y, z,
                  func_8002A8BC(angle) * zRotation, scale, matrixScale,
                  mirroredFrame, 0x10E, alpha);
    func_80034434(0);

    gDPSetPrimColor((*dlist)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dlist)++, 255, 255, 255, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023598.s")
#endif
/*
 * PROVENANCE: adapted from JFG's public decomp, src/camera.c:camDoSprite;
 * Mickey supplies the resident projection flip and display-list encoding.
 */
void func_80023A08(Gfx **dlist, Mtx **mtx, CameraVertex **vertices,
                   CameraObjectSegment *segment, u8 *spriteData, s32 flags,
                   u8 alpha) {
    s32 rotation;
    f32 scale;
    f32 aspect;
    register CameraVertex *vertex;

    vertex = *vertices;
    vertex->x = segment->transform.x;
    vertex->y = segment->transform.y;
    vertex->z = segment->transform.z;
    vertex->r = 255;
    vertex->g = 255;
    vertex->b = 255;
    vertex->a = 255;
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = ((((((u32)*vertices + 0x80000000) & 6) | 8) &
                           0xFF) << 16) | 0x04000000 | 0x12;
        cmd->words.w1 = (u32)*vertices + 0x80000000;
    }
    (*vertices)++;

    rotation = D_800CEA24[D_800CEC64].zRotation +
               segment->transform.zRotation;
    scale = segment->transform.scale * D_80079F90;
    aspect = func_80021438();
    if (viGetVideoMode() & 1) {
        scale *= 0.75f;
        aspect *= D_80081A38;
    }
    mathRSMtx(rotation, scale, aspect, D_800CECD8);

    if (D_80079FC8 != 0) {
        D_80079FC8 = 0;
        D_800CECD8[0][0] = -D_800CECD8[0][0];
        D_800CECD8[0][1] = -D_800CECD8[0][1];
    }
    if (flags & 0x8000) {
        func_80029AB8(D_800CECD8, -segment->transform.scale);
    }
    mtxf_to_mtx(D_800CECD8, *mtx);
    D_800CED58 = *mtx;
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = 0x01020040;
        cmd->words.w1 = (u32)*mtx + 0x80000000;
    }
    (*mtx)++;
    gMoveWd((*dlist)++, 2, 0, 1);

    flags &= ~1;
    if (flags & 4) {
        flags |= 1;
    }
    func_80034E54(dlist, spriteData, flags & 0xF, segment->frame, alpha);
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC000002;
    }
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camDoSpriteDirect; Mickey supplies the secondary matrix scale,
 * resident projection state and display-list encoding.
 */
void func_80023CCC(Gfx **dlist, Mtx **mtx, CameraVertex **vertices,
                   u8 *spriteData, s16 x, s16 y, s16 z, s16 angle, f32 scale,
                   f32 matrixScale, f32 frame, s32 flags, u8 alpha) {
    s32 rotation;
    f32 aspect;
    register CameraVertex *vertex;

    vertex = *vertices;
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    vertex->r = 255;
    vertex->g = 255;
    vertex->b = 255;
    vertex->a = 255;
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = ((((((u32)*vertices + 0x80000000) & 6) | 8) &
                           0xFF) << 16) | 0x04000000 | 0x12;
        cmd->words.w1 = (u32)*vertices + 0x80000000;
    }
    (*vertices)++;

    rotation = D_800CEA24[D_800CEC64].zRotation + angle;
    scale *= D_80079F90;
    aspect = func_80021438();
    if (viGetVideoMode() & 1) {
        scale *= 0.75f;
        aspect *= D_80081A3C;
    }
    mathRSMtx(rotation, scale, aspect, D_800CECD8);

    if (D_80079FC8 != 0) {
        D_80079FC8 = 0;
        D_800CECD8[0][0] = -D_800CECD8[0][0];
        D_800CECD8[0][1] = -D_800CECD8[0][1];
    }
    if (flags & 0x8000) {
        func_80029AB8(D_800CECD8, -scale);
    }
    if (matrixScale != 1.0f) {
        func_80029AB8(D_800CECD8, matrixScale);
    }
    mtxf_to_mtx(D_800CECD8, *mtx);
    D_800CED58 = *mtx;
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = 0x01020040;
        cmd->words.w1 = (u32)*mtx + 0x80000000;
    }
    (*mtx)++;
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 1;
        cmd->words.w0 = 0xBC000002;
    }

    flags &= ~1;
    if (flags & 4) {
        flags |= 1;
    }
    func_80034E54(dlist, spriteData, flags & 0xF, frame, alpha);
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC000002;
    }
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camDo2DSprite; Mickey supplies the display-list encoding and
 * its resident camera globals.
 */
void func_80023F84(Gfx **dlist, Mtx **mtx, CameraVertex **vertices,
                   CameraObjectSegment *segment, u8 *spriteData, s32 flags,
                   u8 alpha) {
    CameraVertex *vertex;
    f32 scale;
    struct {
        MtxF scaleMatrix;
        MtxF aspectMatrix;
    } matrices;

    if (spriteData == NULL) {
        return;
    }

    vertex = *vertices;
    vertex->x = segment->transform.x;
    vertex->y = segment->transform.y;
    vertex->z = segment->transform.z;
    vertex->r = 255;
    vertex->g = 255;
    vertex->b = 255;
    vertex->a = 255;

    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = ((((((u32)*vertices + 0x80000000) & 6) | 8) &
                           0xFF) << 16) | 0x04000000 | 0x12;
        cmd->words.w1 = (u32)*vertices + 0x80000000;
    }
    (*vertices)++;

    D_800CEC68.yRotation = -segment->transform.yRotation;
    D_800CEC68.xRotation = -segment->transform.xRotation;
    D_800CEC68.zRotation = segment->transform.zRotation;
    D_800CEC68.x = 0.0f;
    D_800CEC68.y = 0.0f;
    D_800CEC68.z = 0.0f;

    if (D_800CEC80 != 0) {
        scale = segment->transform.scale;
        mathScaleMtx(matrices.scaleMatrix, scale, scale, 1.0f);
        mathRSMtx(0, 1.0f, func_80021438(), matrices.aspectMatrix);
        mtxf_mul(matrices.aspectMatrix, matrices.scaleMatrix, D_800CF220);
    } else {
        scale = segment->transform.scale;
        mathScaleMtx(D_800CF220, scale, scale, 1.0f);
    }

    func_8002AE10(&D_800CEC68, matrices.aspectMatrix);
    mtxf_mul(D_800CF220, matrices.aspectMatrix, D_800CECD8);
    mtxf_to_mtx(D_800CECD8, *mtx);
    D_800CED58 = *mtx;

    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w0 = 0x01020040;
        cmd->words.w1 = (u32)*mtx + 0x80000000;
    }
    (*mtx)++;

    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 1;
        cmd->words.w0 = 0xBC000002;
    }

    func_80034E54(dlist, spriteData, flags, segment->frame, alpha);

    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }

    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC000002;
    }
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camPushFloatModelMtx.
 */
void camPushFloatModelMtx(Gfx **dlist, Mtx **mtx, MtxF matrix) {
    s32 i;
    s32 j;

    i = 0;
    do {
        j = 0;
        do {
            D_800CECD8[i][j] = matrix[i][j];
            j++;
        } while (j < 4);
        i++;
    } while (i < 4);
    mtxf_mul(matrix, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPushMuzzleMtx. */
void camPushMuzzleMtx(Gfx **dlist, Mtx **mtx, CameraPoint *point, MtxF matrix) {
    f32 outZ;
    f32 outY;
    f32 outX;
    MtxF temp;
    s32 i;
    s32 j;

    D_800CECD8[3][0] = 0.0f;
    D_800CECD8[3][1] = 0.0f;
    D_800CECD8[3][2] = 0.0f;
    mtxf_mul(matrix, D_800CECD8, temp);
    i = 0;
    do {
        j = 0;
        do {
            D_800CECD8[i][j] = temp[i][j];
            j++;
        } while (j < 4);
        i++;
    } while (i < 4);
    mtxf_transform_point(D_800CECD8, point->x, point->y, point->z,
                         &outX, &outY, &outZ);
    D_800CECD8[3][0] = point->x;
    D_800CECD8[3][1] = point->y;
    D_800CECD8[3][2] = point->z;
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camScaleModelMtx. */
void camScaleModelMtx(Gfx **dlist, Mtx **mtx, f32 scale) {
    if (scale != 1.0f) {
        matrixScale(scale, scale, scale, D_800CECD8);
        mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
        mtxf_to_mtx(D_800CF260, *mtx);
        D_800CED58 = *mtx;
        gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
        (*mtx)++;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPushModelMtx. */
void camPushModelMtx(Gfx **dlist, Mtx **mtx, CameraScaledTransform *transform,
                     f32 scale, f32 scaleY) {
    func_8002AA50(transform, D_800CECD8);
    if (scaleY != 0.0f) {
        mtxf_translate_y(D_800CECD8, scaleY);
    }
    if (scale != 1.0f) {
        func_80029AB8(D_800CECD8, scale);
    }
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camRestoreModelMtx. */
void camRestoreModelMtx(Gfx **dlist) {
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPopModelMtx. */
void camPopModelMtx(Gfx **dlist) {
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetPtr. */
Camera *camGetPtr(void) {
    return &D_800CEA20[D_800CEC64];
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetListPtr. */
Camera *camGetListPtr(void) {
    return D_800CEA20;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetInvProjMtx. */
MtxF *camGetInvProjMtx(void) {
    return &D_800CF1E0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjOrgMtx. */
Mtx *camGetProjOrgMtx(void) {
    return &D_800CED60[D_80079F94];
}
MtxF *func_8002468C(void) {
    return &D_800CEC98;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetRotationMtx. */
MtxF *camGetRotationMtx(void) {
    return &D_800CF1A0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjectionMtx. */
MtxF *camGetProjectionMtx(void) {
    return &D_800CED18;
}
/*
 * PROVENANCE: JFG's public src/camera.c identifies the camProjectPoint role;
 * this body is reconstructed from Mickey's matrix and viewport dataflow.
 */
s32 func_800246B0(f32 x, f32 y, f32 z, f32 *outX, f32 *outY,
                  u8 transform) {
    s32 visible;
    f32 viewportScaleX;
    f32 viewportScaleY;
    f32 projectedX;
    f32 projectedY;
    f32 depth;
    Vp *viewport;

    visible = 0;
    if (transform != 0) {
        mtxf_transform_point(D_800CF1A0, x, y, z, &x, &y, &z);
    }
    projectedX = D_800CEC98[0][0] * x;
    projectedY = D_800CEC98[1][1] * y;
    depth = -(D_800CEC98[2][3] * z);
    if (depth < -2.0f) {
        viewport = &D_80079D58[D_800CEC64];
        if (D_80079C40[D_800CEC64].flags & 1) {
            viewport += (D_80079D48 * 5) + 10;
        }
        viewportScaleX = (f32) (viewport->vp.vscale[0] >> 2);
        viewportScaleY = (f32) (viewport->vp.vscale[1] >> 2);
        visible = 1;
        *outX = (f32) (viewport->vp.vtrans[0] >> 2) -
                ((projectedX * viewportScaleX) / depth);
        *outY = (f32) (viewport->vp.vtrans[1] >> 2) +
                ((projectedY * viewportScaleY) / depth);
    }
    return visible;
}

/*
 * PROVENANCE: body adapted from JFG's public src/camera.c:camReversePoint
 * after that source was matched upstream in commit 24d61fe. Its repeated
 * transX load is material to IDO's allocation. With Mickey's symbols and ABI,
 * all 65 instructions and nine relocations are exact.
 */
void func_80024834(f32 screenX, f32 screenY, f32 *x, f32 *y, f32 *z,
                   u8 transform) {
    f32 transX;
    f32 scaleY;
    f32 scaleX;
    f32 transY;
    f32 clipZ;
    Vp *viewport;

    clipZ = (*z * D_800CEC98[2][2]) * D_800CEC98[2][3];
    transX = D_80079D58[D_800CEC64].vp.vtrans[0] >> 2;
    scaleX = D_80079D58[D_800CEC64].vp.vscale[0] >> 2;
    scaleY = D_80079D58[D_800CEC64].vp.vscale[1] >> 2;
    transX = D_80079D58[D_800CEC64].vp.vtrans[0] >> 2;
    transY = D_80079D58[D_800CEC64].vp.vtrans[1] >> 2;
    *x = ((transX - screenX) * clipZ) /
         (D_800CEC98[0][0] * scaleX);
    *y = ((screenY - transY) * clipZ) /
         (D_800CEC98[1][1] * scaleY);
    if (transform != 0) {
        mtxf_transform_point(D_800CF1E0, *x, *y, *z, x, y, z);
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjZ. */
f32 camGetProjZ(f32 x, f32 y, f32 z) {
    f32 temp;
    f32 out;

    temp = D_800CF1A0[3][2];
    out = temp +
          (z * D_800CF1A0[2][2] +
           (y * D_800CF1A0[1][2] + D_800CF1A0[0][2] * (temp = x)));
    temp = y * D_800CF1A0[1][2] + x * D_800CF1A0[0][2];
    temp = z * D_800CF1A0[2][2] + temp;
    out = D_800CF1A0[3][2] + temp;

    return out;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camCopyOrthoMatrix.
 *
 * JFG's 15-slot copy with each slot scaled by D_80079F48. The body is the
 * JFG loop unchanged; what closed it was the .data layout. as1 reuses a
 * `lui $at` for a second load only when both addresses provably share a
 * high half, and it can prove that only inside one 16-byte block of the
 * section (the section's own alignment). The target re-materialises the
 * high half between the first and second peeled loads and shares it for
 * the second and third, which puts D_80079F4C at section offset 0xC: this
 * TU's .data starts at 0x80079F40, not 0x80079F48. See the two leading
 * words below.
 */
void func_80024978(MtxF matrix) {
    s32 i;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    for (i = 0; i < 15; i++) {
        ((f32 *) matrix)[i] = ((f32 *) &D_80079F4C)[i] * D_80079F48;
    }
    matrix[3][3] = (u32) width >> 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStartShake. */
void camStartShake(s32 camNo, f32 attack, f32 sustain, f32 decay,
                   s32 magnitude) {
    CameraShake *shake;

    if ((camNo >= 0) && (camNo < 6)) {
        shake = &D_800CEC18[camNo];
        shake->attackEnd = (s16) (s32) (attack * 60.0f);
        shake->sustainEnd =
            (s16) (shake->attackEnd + (s32) (sustain * 60.0f));
        shake->totalEnd =
            (s16) (shake->sustainEnd + (s32) (decay * 60.0f));
        shake->timer = 0;
        shake->magnitude = magnitude;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStopShakes. */
void camStopShakes(void) {
    s32 i;
    Camera *cam;
    CameraShake *shake;

    D_800CEC84 = 0;
    cam = D_800CEA20;
    shake = D_800CEC18;

    /* IDO: line-join preserves the target store/increment schedule. */
    for (i = 6; i--; cam++, shake++) { \
        cam->shakeX = 0.0f; \
        cam->shakeY = 0.0f; \
        cam->shakeZ = 0.0f; \
        shake->magnitude = 0; \
    }
}
/*
 * PROVENANCE: role from JFG's public decomp, src/camera.c:camScreenShake;
 * body reconstructed from Mickey-only evidence.
 */
void func_80024BA0(f32 x, f32 y, f32 z, f32 radius, f32 magnitude) {
    Camera *cam;
    f32 distance;
    f32 dz;
    f32 dy;
    f32 attack;
    f32 sustain;
    s32 i;

    i = 0;
    if (D_800CEC60 >= 0) {
        sustain = D_80081A40;
        /* IDO: line-join keeps the paired constant/pointer setup schedulable. */
        attack = D_80081A44; cam = D_800CEA20;
        do {
            distance = x - cam->transform.x;
            dy = y - cam->transform.y;
            dz = z - cam->transform.z;
            distance = sqrtf((distance * distance) + (dy * dy) + (dz * dz));
            if (distance < radius) {
                camStartShake(i, attack, sustain, attack,
                              (s32) (((radius - distance) * magnitude) /
                                     radius));
            }
            i++;
            cam++;
        } while (i <= D_800CEC60);
    }
}

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetZoom. */
void camSetZoom(s32 camNo, f32 zoom) {
    if ((camNo >= 0) && (camNo < 6)) {
        D_80079FA0[camNo] = 1;
        D_80079FB0[camNo] = zoom;
    }
}
/*
 * PROVENANCE: JFG's public src/camera.c identifies the camTick role/order;
 * the body is reconstructed from Mickey-only camera and shake-envelope data.
 */
void func_80024D00(s32 updateRate) {
    Camera *camera;
    CameraShake *shake;
    s32 magnitude;
    s32 i;

    camera = D_800CEA20;
    shake = D_800CEC18;
    D_800CEC84 = 0;
    for (i = 6; i--; camera++, shake++) {
        D_80079FA0[i] = 0;
        D_80079FA8[i] = 1;
        camera->shakeX = 0.0f;
        camera->shakeY = 0.0f;
        camera->shakeZ = 0.0f;
        if (shake->magnitude != 0) {
            shake->timer += updateRate;
            if (shake->timer >= shake->totalEnd) {
                shake->magnitude = 0;
            } else {
                D_800CEC84 = 1;
                magnitude = shake->magnitude;
                if (shake->sustainEnd < shake->timer) {
                    magnitude = ((shake->totalEnd - shake->timer) * magnitude) /
                                (shake->totalEnd - shake->sustainEnd);
                } else if (shake->timer < shake->attackEnd) {
                    magnitude = (shake->timer * magnitude) / shake->attackEnd;
                }
                camera->shakeY = mathRnd(0, magnitude);
            }
        }
    }
}
/* Mickey-only fixed-distance camera-transform reconstruction. */
void func_80024ED8(CameraTransform *source, s32 unused, Camera *dest) {
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    if (D_800CEC60 == 0) {
        distance = 100.0f;
    } else {
        distance = 150.0f;
    }
    targetX = source->x - (func_8002A8C0(source->yRotation) * distance);
    targetY = source->y + (distance * D_80081A48);
    targetZ = source->z - (func_8002A8BC(source->yRotation) * distance);
    dest->transform.x = targetX;
    dest->unk18 = targetX;
    dest->unk1C = targetY;
    dest->transform.y = targetY;
    dest->unk20 = targetZ;
    dest->transform.z = targetZ;
    deltaX = targetX - source->x;
    deltaY = (targetY - source->y) - 20.0f;
    deltaZ = targetZ - source->z;
    dest->transform.yRotation = Arctanf(deltaZ, deltaX) + 0x4000;
    dest->transform.xRotation =
        Arctanf(deltaY, sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)));
    dest->transform.zRotation = 0;
}

/* The projection scale and matrix constants are owned by this TU.
 * The section starts 16-byte aligned at 0x80079F40, eight bytes before the
 * scale: func_80024978's shared `lui $at` pattern places D_80079F4C at
 * section offset 0xC (see the comment there). The two leading words are
 * zero in the ROM and nothing references them. IDO rounds the section to
 * 16 bytes, so it also owns the next two matrix floats at 0x80079F58. */
s32 D_80079F40 = 0;
s32 D_80079F44 = 0;
f32 D_80079F48 = 1.0f;
f32 D_80079F4C = 1.0f;
f32 D_80079F50 = 0.0f;
f32 D_80079F54 = 0.0f;
f32 D_80079F58[2] = { 0.0f, 0.0f };

/* PLATEAU-HANDOFF:func_80022FD4:start
 * symbol: func_80022FD4
 * score: 157/369 words
 * frame: 0xB0
 * relocations: 55
 * first-mismatch: +0x9C
 * summary: first +0x9C remains the folded GP-draw schedule; field-access probes moved no census lines; retained body is best
 * PLATEAU-HANDOFF:func_80022FD4:end
 */

/* PLATEAU-HANDOFF:func_80023598:start
 * symbol: func_80023598
 * score: 13 differing words
 * frame: 0x90
 * relocations: 32
 * first-mismatch: +0xA8
 * summary: Delta 0, frame 0x90; baseScale wins the f16 tie with distanceScale (force w30=c29 gives 3), plus a propagated copy and Arctanf load order
 * PLATEAU-HANDOFF:func_80023598:end
 */
