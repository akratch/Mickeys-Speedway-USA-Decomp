/*
 * Shadow buffers and projection -- ROM 0x16A90-0x18FF0.
 *
 * PROVENANCE -- the TU attribution and the five descriptive names below are
 * borrowed from Jet Force Gemini's public retail-derived decompilation:
 * src/shadows.c and asm/nonmatchings/shadows/*.s.  They are supported here at
 * tier B by the same call graph and at tier D by function order and masked
 * instruction shape.  This is not a whole-object tier-A match; Mickey's ROM
 * remains the authority for every body.
 *
 * The matched leaf bodies below were reconstructed from Mickey's own
 * instructions and globals; no JFG body has been adapted.  The remaining
 * pragmas preserve the original ROM bytes.  JFG's address-placeholder helper
 * names are deliberately not imported.
 */

#include "PR/ultratypes.h"

/* Shadow buffer state occupies one contiguous compiler-owned .data input
 * section. Keep the retail labels at their measured offsets for all users;
 * the terminal halfword pair is part of IDO's measured 0x50-byte section. */
u8 *D_80079410[1] = { 0 };
u8 *D_80079414[3] = { 0 };
u8 *D_80079420[1] = { 0 };
u8 *D_80079424[3] = { 0 };
u8 *D_80079430[1] = { 0 };
u8 *D_80079434[3] = { 0 };
u8 *D_80079440 = 0;
u8 *D_80079444 = 0;
u8 *D_80079448 = 0;
s32 D_8007944C = 0;
s32 D_80079450 = 0;
s32 D_80079454 = 0;
s32 D_80079458 = 0;
u16 D_8007945C[2] = { 0, 0x4000 };
extern s32 D_800CB278;
extern s32 D_800CB27C;
extern s32 D_800CB280;
extern s32 D_800CB268;
extern s32 D_800CB26C;
extern s32 D_800C9D40;
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern s32 D_800CAF58;
extern u8 D_800CAF60[];
extern u8 D_800C9D48[];
extern u8 D_800C9F58[];
extern s32 D_800C9F48[];
extern f32 D_800CB260;
extern f32 D_800CB270;
extern f32 D_800CB274;
extern s32 D_800CB284;
extern s32 D_800CB288;
extern void *func_8002B280(s32 size, s32 tag);
extern void mmFree(void *ptr);
extern s32 getXZCompareMask(void *grid, s32 xMin, s32 zMin, s32 xMax, s32 zMax);
extern s32 mathXZInTri(s32 x, s32 z, void *a, void *b, void *c);
extern f32 D_80079464[];
extern f32 D_800CB28C;

typedef struct ShadowQueryVolume {
    u8 pad0[0x6C];
    s16 minY6C;
    s16 maxY6E;
} ShadowQueryVolume;

typedef struct ShadowQuery {
    u8 pad0[0xC];
    f32 x0C;
    f32 y10;
    f32 z14;
    u8 pad18[0x16];
    s16 sector2E;
    u8 pad30[0x10];
    ShadowQueryVolume *volume40;
    u8 pad44[0xC];
    f32 *value50;
} ShadowQuery;

typedef struct ShadowBox {
    s32 words[3];
} ShadowBox;

/* One 0x10-byte batch; a batch's faces end where the next batch's begin. */
typedef struct ShadowBlock {
    u8 pad0[6];
    s16 verticesOffset;
    s16 facesOffset;
    u8 padA[2];
    u32 flags;
} ShadowBlock;

typedef struct ShadowTriangle {
    u8 verticesArray[4];
    u8 pad4[0xC];
} ShadowTriangle;

typedef struct ShadowPoint {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[4];
} ShadowPoint;

typedef struct ShadowSector {
    ShadowPoint *vertices;
    ShadowTriangle *triangles;
    u8 pad8[4];
    ShadowBlock *batches;
    u32 *faceMasks;
    u8 pad14[0x10];
    s16 numberOfBatches;
    u8 pad26[0x1A];
} ShadowSector;

typedef struct ShadowWorld {
    u8 pad0[4];
    ShadowSector *sectors;
    ShadowBox *boundingBoxes;
} ShadowWorld;
extern s32 func_80017660(void *arg0, s32 arg1, void *arg2, s32 arg3, s32 arg4);
extern void func_80018544(void *arg0, void *arg1);
extern s32 shadowBoxPolyOverlap(f32 arg0, f32 arg1, f32 arg2, f32 arg3,
                                s32 arg4, void *arg5);

/* PROVENANCE: adapted from JFG's public asm/nonmatchings/shadows/shadowInitBuffers.s; Mickey globals are authoritative.
 * The C body emits all 75 linked instruction words and the owning 0x50-byte
 * .data section exactly. Its sentinel pair still binds D_80079434 + 0xC where
 * the target relocation metadata names D_80079440, so relocation identity is
 * not exact. */
void shadowInitBuffers(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 i;
    s32 stride0;
    s32 stride1;
    s32 stride2;

    D_800CB284 = arg0;
    D_800CB288 = arg1;
    D_800CB278 = arg2;
    D_800CB27C = arg3;
    stride0 = arg2 * 10;
    D_800CB280 = arg4;
    D_80079410[0] = func_8002B280(stride0 * 4, 0x8D);
    stride1 = arg3 * 16;
    D_80079420[0] = func_8002B280(stride1 * 4, 0x8D);
    stride2 = arg4 * 8;
    D_80079430[0] = func_8002B280(stride2 * 4, 0x8D);

    for (i = 0; i < 3; i++) {
        D_80079414[i] = D_80079414[i - 1] + stride0;
        D_80079424[i] = D_80079424[i - 1] + stride1;
        D_80079434[i] = D_80079434[i - 1] + stride2;
    }
    D_80079458 = 0;
}
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/shadows/shadowFreeBuffers.s. */
void shadowFreeBuffers(void) {
    if (D_80079410[0] != NULL) {
        mmFree(D_80079410[0]);
        D_80079410[0] = NULL;
    }
    if (D_80079420[0] != NULL) {
        mmFree(D_80079420[0]);
        D_80079420[0] = NULL;
    }
    if (D_80079430[0] != NULL) {
        mmFree(D_80079430[0]);
        D_80079430[0] = NULL;
    }
}
void shadowChangeBuffer(void) {
    D_80079458 ^= 1;
}
void shadowGetBuffers(s32 arg0, void **arg1, void **arg2, void **arg3) {
    s32 index = D_80079458;

    if (arg0 & 2) {
        index += 2;
    }
    *arg1 = D_80079410[index];
    *arg2 = D_80079420[index];
    *arg3 = D_80079430[index];
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: Mickey's m2c control-flow draft and resident shadow
 * declarations reconstruct this pipeline; no external function body is
 * adapted.
 */
extern s16 Arctanf(f32 x, f32 y);
extern f32 Powerf(f32 value);
extern s32 TrapDanglingJump();
extern f32 camDistance(f32 x, f32 y, f32 z);
extern s32 camGetMode(void);
extern f32 sqrtf(f32 value);
extern void **func_8000572C(s32 *start, s32 *end);
extern s32 func_8000FD68(s32 *result, s16 xMin, s16 zMin, s16 xMax,
                         s32 yMin, s32 yMax, s32 yMax2);
extern void shadowBoundingBox(s32 count, f32 *points, f32 *xMin,
                              f32 *zMin, f32 *xMax, f32 *zMax);
extern void func_80016890(void *object, void *angles, void *surface,
                          f32 x, f32 y, f32 z, s16 type);
extern void func_800180B4(ShadowQuery *query);
extern u8 *levelGetLevel(void);
extern s16 D_80079460;
extern f32 D_800817A0;

typedef struct ShadowGenerateAngle {
    s16 horizontal;
    s16 vertical;
    s16 material;
} ShadowGenerateAngle;

#define SG_U8(p, o) (*(u8 *) ((u8 *) (p) + (o)))
#define SG_S16(p, o) (*(s16 *) ((u8 *) (p) + (o)))
#define SG_U16(p, o) (*(u16 *) ((u8 *) (p) + (o)))
#define SG_S32(p, o) (*(s32 *) ((u8 *) (p) + (o)))
#define SG_F32(p, o) (*(f32 *) ((u8 *) (p) + (o)))
#define SG_PTR(p, o) (*(void **) ((u8 *) (p) + (o)))

/* Workbench verdict: structure-mismatch, 445 differing words, first mismatch +0xC. */
/* Candidate is 520/510 instructions with frame -0x150 versus target -0x138; it is not shape-exact. */
/* Relocation count is exact at 63; local lifetimes and branch spelling still control the register web. */
void shadowGenerate(s32 arg0, s32 arg1) {
    s32 first;
    s32 selected;
    ShadowGenerateAngle angles[4];
    ShadowGenerateAngle *anglePointers[4];
    void **objects;
    void *object;
    void *info;
    void *surface;
    void *model;
    void *part;
    void *partData;
    void *angleSource;
    u8 *level;
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
    f32 limit;
    f32 scale;
    f32 a;
    f32 b;
    f32 c;
    f32 d;
    f32 e;
    f32 f;
    f32 g;
    f32 h;
    s16 type;
    s16 objectType;
    s16 lowAngle;
    s32 angleCount;
    s32 i;
    s32 j;
    s32 k;
    s32 value;

    selected = (arg0 & 2) | D_80079458;
    D_80079440 = D_80079410[selected];
    D_8007944C = 0;
    D_80079444 = D_80079420[selected];
    D_80079450 = 0;
    D_80079448 = D_80079430[selected];
    D_80079454 = 0;
    D_800CB28C = 1.0f - Powerf(D_800817A0);

    level = levelGetLevel();
    D_8007945C[0] = (u16) SG_S16(level, 0xD8);
    D_8007945C[1] = (u16) SG_S16(level, 0xDA);
    *(s16 *) ((u8 *) D_8007945C + 4) = SG_U8(level, 0xE2);

    objects = func_8000572C(&first, &selected);
    if (first < selected) {
        do {
            object = objects[first++];
            surface = SG_PTR(object, 0x4C);
            if (surface != NULL) {
                value = SG_U8(surface, 0x10) & arg0;
                if (value != 0) {
                    distance = 0.0f;
                    info = SG_PTR(object, 0x40);
                    x = SG_F32(object, 0xC);
                    y = SG_F32(object, 0x10);
                    z = SG_F32(object, 0x14);
                    type = SG_S16(object, 0x0);

                    if (value == 1) {
                        objectType = SG_S16(object, 0x44);
                        if (camGetMode() == 0) {
                            distance = camDistance(x, y, z);
                        } else if ((objectType != 1) &&
                                   (objectType != 0x35) &&
                                   (objectType != 0x3C)) {
                            distance = 32768.0f;
                        }
                        if (objectType == 1) {
                            partData = SG_PTR(object, 0x64);
                            x = SG_F32(partData, 0x448);
                            y = SG_F32(partData, 0x44C);
                            z = SG_F32(partData, 0x450);
                            type = SG_S16(partData, 0x43C);
                        } else if (objectType == 0x43) {
                            partData = SG_PTR(object, 0x64);
                            x = SG_F32(partData, 0x30);
                            y = SG_F32(partData, 0x34);
                            z = SG_F32(partData, 0x38);
                        } else if (objectType == 0x1D) {
                            partData = SG_PTR(object, 0x64);
                            x = SG_F32(partData, 0x44);
                            y = SG_F32(partData, 0x48);
                            z = SG_F32(partData, 0x4C);
                            type = SG_S16(partData, 0x50);
                        } else if (objectType == 0x49) {
                            partData = SG_PTR(object, 0x64);
                            x = SG_F32(partData, 0x44);
                            y = SG_F32(partData, 0x48);
                            z = SG_F32(partData, 0x4C);
                            type = SG_S16(partData, 0x50);
                        }
                    }

                    angleSource = NULL;
                    if ((SG_U8(surface, 0x10) & 8) != 0) {
                        angleSource = SG_PTR(surface, 0x1C);
                        if (angleSource != NULL) {
                            SG_U8(angleSource, 0x13) = 0;
                        }
                    }
                    SG_U8(surface, 0x13) = 0;
                    if (((SG_U16(object, 0x6) & 0x400) == 0) &&
                        ((SG_S32(info, 0x14) & 1) == 0) &&
                        (SG_F32(surface, 0) > 0.0f) &&
                        (SG_F32(surface, 4) > 0.0f)) {
                        limit = (f32) SG_S16(info, 0x68);
                        if (distance < limit) {
                            lowAngle = SG_S16(info, 0x6A);
                            if ((f32) lowAngle < distance) {
                                D_800CB260 = (limit - distance) /
                                             (f32) (SG_S16(info, 0x68) -
                                                    lowAngle);
                            } else {
                                D_800CB260 = 1.0f;
                            }
                            if ((SG_U8(surface, 0x10) & 4) != 0) {
                                value = 0;
                                if ((angleSource != NULL) &&
                                    ((SG_U8(angleSource, 0x10) & 8) != 0)) {
                                    value = TrapDanglingJump(object, 0,
                                                              (f32) arg1);
                                }
                                if (value != 0) {
                                    func_80016890(object, NULL, angleSource,
                                                  x, y, z, type);
                                } else {
                                    func_80016890(object, NULL, surface,
                                                  x, y, z, type);
                                }
                            } else if ((angleSource != NULL) &&
                                       ((SG_U8(angleSource, 0x10) & 8) != 0)) {
                                if (TrapDanglingJump(object, 1,
                                                     (f32) arg1) != 0) {
                                    func_80016890(object, NULL, angleSource,
                                                  x, y, z, type);
                                } else {
                                    func_80016890(object, NULL, surface,
                                                  x, y, z, type);
                                }
                            } else {
                                angleCount = (D_80079460 > 0) ? 1 : 0;
                                if (angleCount != 0) {
                                    anglePointers[0] =
                                        (ShadowGenerateAngle *) D_8007945C;
                                }
                                model = SG_PTR(object, 0x50);
                                if (model != NULL) {
                                    k = 1;
                                    part = (u8 *) model + 0x20;
                                    if (SG_S16(model, 0xE) >= 2) {
                                        do {
                                            partData = (u8 *) part + 0x10;
                                            if (SG_F32(part, 0x14) > 0.0f) {
                                                anglePointers[angleCount] =
                                                    &angles[angleCount];
                                                a = SG_F32(partData, 0);
                                                c = SG_F32(partData, 8);
                                                b = SG_F32(partData, 4);
                                                angles[angleCount].horizontal =
                                                    Arctanf(-a, -c);
                                                angles[angleCount].vertical =
                                                    Arctanf(b, sqrtf((a * a) +
                                                                     (c * c)));
                                                angleCount++;
                                                angles[angleCount - 1].material =
                                                    SG_U8(partData, 0x15);
                                            }
                                            k++;
                                            part = (u8 *) part + 0x20;
                                        } while (k < SG_S16(model, 0xE));
                                    }
                                }
                                k = angleCount - 1;
                                if (((s32) SG_U8(surface, 0x11) < angleCount) &&
                                    (k > 0)) {
                                    do {
                                        j = 0;
                                        while (j < k) {
                                            if (anglePointers[j]->material <
                                                anglePointers[j + 1]->material) {
                                                ShadowGenerateAngle *tmp = anglePointers[j];
                                                anglePointers[j] = anglePointers[j + 1];
                                                anglePointers[j + 1] = tmp;
                                            }
                                            j++;
                                        }
                                        k--;
                                    } while (k != 0);
                                }
                                i = 0;
                                while (((s32) SG_U8(surface, 0x13) <
                                        (s32) SG_U8(surface, 0x11)) &&
                                       (i < angleCount)) {
                                    func_80016890(object, anglePointers[i],
                                                  surface, x, y, z, type);
                                    i++;
                                }
                            }
                        }
                    }
                    model = SG_PTR(object, 0x50);
                    if (model != NULL) {
                        if (SG_U8(model, 4) >= 2) {
                            scale = D_800CB28C;
                            D_800CB28C = 1.0f;
                            func_800180B4((ShadowQuery *) object);
                            D_800CB28C = scale;
                        } else if (SG_U8(model, 4) == 0) {
                            func_800180B4((ShadowQuery *) object);
                        }
                        SG_U8(model, 4) = 0;
                    }
                }
            }
        } while (first < selected);
    }
    if (D_80079448 != NULL) {
        *(s16 *) (D_80079448 + (D_80079454 * 8) + 4) =
            (s16) D_80079450;
        *(s16 *) (D_80079448 + (D_80079454 * 8) + 6) =
            (s16) D_8007944C;
    }
}
#undef SG_U8
#undef SG_S16
#undef SG_U16
#undef SG_S32
#undef SG_F32
#undef SG_PTR
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowGenerate.s")
#endif
#ifdef NON_MATCHING
/*
 * PROVENANCE: the query/polygon organization follows DKR's public
 * src/tracks.c shadow pipeline and JFG's public func_8001DF5C assembly.
 * Mickey's own body, offsets, branches, and relocation surface remain the
 * authority for this candidate.
 */
extern s32 func_8000FD68(s32 *result, s16 xMin, s16 zMin, s16 xMax,
                         s32 yMin, s32 yMax, s32 yMax2);
extern void shadowBoundingBox(s32 count, f32 *points, f32 *xMin,
                              f32 *zMin, f32 *xMax, f32 *zMax);
extern void func_80017140(void *query, f32 *points, void *sector, s32 gridMask);
extern s32 func_80017BCC(void *query, void *angles, void *surface);
extern void func_80018654();
extern f32 D_800817A4;
extern s32 D_800CB264;

typedef struct Shadow168Angle {
    s16 horizontal;
    s16 vertical;
    s16 material;
} Shadow168Angle;

#define SH168_U8(p, o) (*(u8 *) ((u8 *) (p) + (o)))
#define SH168_S8(p, o) (*(s8 *) ((u8 *) (p) + (o)))
#define SH168_S16(p, o) (*(s16 *) ((u8 *) (p) + (o)))
#define SH168_U16(p, o) (*(u16 *) ((u8 *) (p) + (o)))
#define SH168_S32(p, o) (*(s32 *) ((u8 *) (p) + (o)))
#define SH168_F32(p, o) (*(f32 *) ((u8 *) (p) + (o)))
#define SH168_PTR(p, o) (*(void **) ((u8 *) (p) + (o)))

/* Workbench verdict: structure-mismatch, 553 differing words; first mismatch +0x4. */
/* Candidate is 563/556 instructions with the exact 0x190-byte target frame. */
/* Relocation count is exact at 48; allocation/CFG order and identities remain unresolved. */
void func_80016890(void *arg0, void *arg1, void *arg2p, f32 arg3, f32 arg4,
                   f32 arg5, s16 arg6) {
    typedef struct Shadow168Query {
        s32 surface0;
        u8 pad4[4];
        f32 x8;
        f32 yC;
        f32 z10;
        s16 type14;
        s16 lowerY16;
        s16 upperY18;
        u8 pad1A[2];
        f32 scale1C;
        f32 minimum20;
        f32 height24;
        f32 heightRange28;
        f32 inverseScale2C;
        f32 area30;
        f32 halfX34;
        f32 halfZ38;
        f32 expanded3C;
        f32 bounds40[4];
    } Shadow168Query;

    s32 result[32];
    void *matrix;
    s32 value;
    f32 radius;
    f32 base;
    f32 distance;
    f32 sine;
    f32 cosine;
    f32 ratio;
    f32 point0;
    f32 point2;
    f32 point4;
    f32 point6;
    f32 objectScale;
    Shadow168Query query;
    f32 points[8];
    s32 count;
    s32 i;
    s32 active;
    void *arg2 = arg2p;

    query.x8 = arg3;
    query.yC = arg4;
    query.z10 = arg5;
    query.type14 = arg6;
    query.scale1C = 2.0f;
    SH168_S16((u8 *) arg2 + (SH168_U8(arg2, 0x13) * 2), 0x14) =
        (s16) D_80079454;
    query.surface0 = SH168_S32(arg2, 8);
    query.lowerY16 = (s16) ((s32) SH168_S16(SH168_PTR(arg0, 0x40), 0x6C) +
                            (s32) arg4);
    query.upperY18 = (s16) ((s32) SH168_S16(SH168_PTR(arg0, 0x40), 0x6E) +
                            (s32) arg4);

    if (SH168_S16(arg0, 0x44) != 1) {
        distance = SH168_F32(arg0, 0x30);
        if (distance < 0.0f) {
            distance = -distance;
        }
        distance -= 250.0f;
        if (distance < 0.0f) {
            distance = 0.0f;
        }
        if (distance > 1024.0f) {
            distance = 1024.0f;
        }
        query.scale1C += distance * D_800817A4;
    }

    query.inverseScale2C = SH168_F32(arg2, 0);
    query.halfX34 = query.inverseScale2C * 10.0f;
    query.halfZ38 = query.halfX34;
    query.expanded3C = 1.0f;
    if (arg1 != NULL) {
        cosine = func_8002A8BC(SH168_S16(arg1, 2));
        if (cosine > 0.0f) {
            sine = func_8002A8C0(SH168_S16(arg1, 2));
            if (sine != 0.0f) {
                ratio = cosine / sine;
                if (ratio > 2.0f) {
                    ratio = 2.0f;
                }
            } else {
                ratio = 2.0f;
            }
            query.expanded3C +=
                (0.25f * ratio * (f32) SH168_U8(arg2, 0x12)) /
                query.halfZ38;
        }
    }
    query.expanded3C *= query.halfZ38;
    query.area30 = 2.0f * query.halfX34 * (query.halfZ38 + query.expanded3C);

    query.height24 =
        (f32) SH168_S16(SH168_PTR(arg0, 0x40), 0x6C) * 0.125f;
    if (query.height24 < 0.0f) {
        query.height24 = -query.height24;
    }
    query.heightRange28 = 7.0f * query.height24;
    query.minimum20 = -32768.0f;
    query.inverseScale2C = 144.0f / query.inverseScale2C;

    for (i = 0; i < 4; i++) {
        points[i * 2] = arg3;
        points[(i * 2) + 1] = arg5;
    }

    if (arg1 != NULL) {
        sine = func_8002A8C0(SH168_S16(arg1, 0));
        cosine = func_8002A8BC(SH168_S16(arg1, 0));
        point0 = query.halfX34 * cosine;
        point2 = query.halfZ38 * sine;
        point4 = query.halfZ38 * cosine;
        point6 = query.expanded3C * sine;
        points[0] += -point0 - point2;
        points[1] += (query.halfX34 * sine) - point4;
        points[2] += point0 - point2;
        points[3] += -point4 - (query.halfX34 * sine);
        points[4] += point0 + point6;
        points[5] += (query.expanded3C * cosine) - (query.halfX34 * sine);
        points[6] += -point0 + point6;
        points[7] += (query.expanded3C * cosine) + (query.halfX34 * sine);
    } else {
        value = SH168_U8(arg2, 0x10) & 0x20;
        point2 = SH168_F32(arg2, 4);
        point0 = SH168_F32(arg2, 0);
        if ((value != 0) || (point2 != point0)) {
            if (value != 0) {
                objectScale = SH168_F32(arg0, 8);
                matrix = SH168_PTR(SH168_PTR(arg0, 0x68), 0);
                point0 *= (f32) SH168_S16(matrix, 0x42) * objectScale;
                point2 *= (f32) SH168_S16(matrix, 0x46) * objectScale;
                point4 = SH168_F32(arg2, 0) *
                         ((f32) SH168_S16(matrix, 0x3C) * objectScale);
                point6 = SH168_F32(arg2, 4) *
                         ((f32) SH168_S16(matrix, 0x40) * objectScale);
            } else {
                point0 *= 10.0f;
                point2 *= 10.0f;
                point4 = SH168_F32(arg2, 0) * -10.0f;
                point6 = SH168_F32(arg2, 4) * -10.0f;
            }
            sine = func_8002A8C0(SH168_S16(arg0, 0));
            cosine = func_8002A8BC(SH168_S16(arg0, 0));
            query.halfX34 = (point0 - point4) * 0.5f;
            query.halfZ38 = (point2 - point6) * 0.5f;
            query.expanded3C = query.halfZ38;
            points[0] += (point0 * cosine) + (point2 * sine);
            points[1] += (point2 * cosine) - (point0 * sine);
            points[2] += (point4 * cosine) + (point2 * sine);
            points[3] += (point2 * cosine) - (point4 * sine);
            points[4] += (point4 * cosine) + (point6 * sine);
            points[5] += (point6 * cosine) - (point4 * sine);
            points[6] += (point0 * cosine) + (point6 * sine);
            points[7] += (point6 * cosine) - (point0 * sine);
        } else {
            points[0] += query.halfX34;
            points[1] += query.halfZ38;
            points[2] -= query.halfX34;
            points[3] += query.halfZ38;
            points[4] -= query.halfX34;
            points[5] -= query.halfZ38;
            points[6] += query.halfX34;
            points[7] -= query.halfZ38;
        }
    }

    D_800CB270 = (points[6] + points[0] + points[2] + points[4]) * 0.25f;
    D_800CB274 = (points[7] + points[1] + points[3] + points[5]) * 0.25f;
    shadowBoundingBox(4, points, &query.bounds40[0], &query.bounds40[1],
                      &query.bounds40[2], &query.bounds40[3]);
    count = func_8000FD68(result, (s16) (s32) query.bounds40[0],
                          query.lowerY16, (s16) (s32) query.bounds40[1],
                          (s32) query.bounds40[2], query.upperY18,
                          (s32) query.bounds40[3]);
    D_800CAF58 = 0;
    D_800C9D40 = 0;
    for (i = 0; i < 4; i++) {
        D_800C9F48[i] = 0;
    }
    D_800CB268 = -1;
    D_800CB26C = -1;
    D_800CB264 = 0;
    for (i = 0; i < count; i++) {
        if (result[i] >= 0) {
            value = getXZCompareMask(
                *(u8 **) ((u8 *) (s32) D_800CB284 + 8) +
                    (result[i] * 0xC),
                (s32) query.bounds40[0], (s32) query.bounds40[1],
                (s32) query.bounds40[2], (s32) query.bounds40[3]);
            func_80017140(&query, &points[0],
                          *(u8 **) ((u8 *) (s32) D_800CB284 + 4) +
                              (result[i] << 6),
                          value);
        }
    }
    active = 1;
    if (D_800CAF58 > 0) {
        func_80018654(D_800C9D40, D_800C9D48, D_800C9F48, D_800C9F58);
        if (func_80017BCC(&query, arg1, arg2) == 0) {
            active = 0;
        }
    }
    SH168_S16((u8 *) arg2 + (SH168_U8(arg2, 0x13) * 2), 0x18) =
        (s16) D_80079454;
    if (active != 0) {
        SH168_U8(arg2, 0x13) = (u8) (SH168_U8(arg2, 0x13) + 1);
    }
}
#undef SH168_U8
#undef SH168_S8
#undef SH168_S16
#undef SH168_U16
#undef SH168_S32
#undef SH168_F32
#undef SH168_PTR
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80016890.s")
#endif
/*
 * PROVENANCE: organized from the public JFG shadow polygon pipeline and
 * Mickey's own m2c control flow; all field offsets and buffer limits remain
 * Mickey-only evidence.
 */
#ifdef NON_MATCHING
/* Workbench verdict: 285 masked words at size delta 0 (was 300 at -12).
 * Track B, 2026-09-23:
 *   - the polygon buffer keeps its offset (ten declared words precede it);
 *     below it the declaration order now lands the target's homes: var_t1
 *     +0xBC, var_ra +0xB8, temp_a1 +0xB4, surfaceId +0x94, var_v1 +0x80,
 *     sp7C +0x7C, var_a0 +0x78. The two s16 locals share one word.
 *   - surfaceId read through its address after the calls is stored at its
 *     definition (as the target does) instead of being sunk to its use with
 *     temp_a0 kept live: -12 to -4.
 *   - sp7C at function scope read through its address stays home-resident,
 *     as the target's per-iteration reload shows: delta 0.
 * Measured and not adopted: holding D_800CAF58 in a local for the slot
 * address and the increment (the target never reloads it) gives 98
 * byte-exact words against 88 here but size delta -16; the fill loop
 * written with the byte index (not the scaled offset) carried across the
 * rotation matches the target's body shape but is +12; a declared
 * polygon-end pointer unrolls the loop (+744). Left: the literal 3 hoisted
 * into s7 (the target keeps slti and re-materialises 3 and the polygon
 * address at each call, hoisting only polygon+0x30), and arg2 held in t5. */
void func_80017140(void *arg0, f32 *arg1, void *arg2, s32 arg3) {
    u8 *var_a3;
    u8 *temp_a3;
    u8 *temp_t1;
    u8 *temp_t2;
    u8 *temp_v0;
    u8 *var_v0;
    u8 *var_v1_2;
    u8 *var_v1_3;
    u8 *var_v1_4;
    f32 pointHeight;
    u8 polygon[0x58];
    s32 var_t1;
    s32 var_ra;
    s32 temp_a1;
    s16 temp_v0_3;
    s16 temp_v1;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_t0;
    s32 var_lo;
    s32 surfaceId;
    s32 temp_s1;
    s32 temp_t9;
    s32 var_a0_2;
    s32 var_a2_2;
    s32 var_v1;
    s32 sp7C;
    s32 var_a0;
    s32 var_t0_2;
    s32 vertexOffset;
    s32 temp_v0_4;
    u32 temp_a0;
    u32 temp_v0_2;

    var_t0 = *(s16 *) ((u8 *) arg2 + 0x24);
    var_t1 = 0;
    if (var_t0 > 0) {
        sp7C = 0;
        do {
            temp_v0 = *(u8 **) ((u8 *) arg2 + 0xC) + *(s32 *) &sp7C;
            temp_a0 = *(u32 *) (temp_v0 + 0xC);
            if (!(temp_a0 & 0x08013880)) {
                var_ra = *(s16 *) (temp_v0 + 0x8);
                temp_a1 = *(s16 *) (temp_v0 + 0x18);
                surfaceId = (temp_a0 >> 24) & 7;
                vertexOffset = *(s16 *) (temp_v0 + 0x6);
                temp_s1 = (s32) (*(u8 **) ((u8 *) arg2 + 0x0) +
                                  (((vertexOffset << 2) + vertexOffset) << 1));
                if (var_ra < temp_a1) {
                    var_v1 = var_ra * 8;
                    var_a0 = var_ra * 4;
                    do {
                        temp_t9 = *(u16 *) (*(u8 **) ((u8 *) arg2 + 0x18) + var_v1) * 4;
                        temp_v0_2 = *(u32 *)
                            (*(u8 **) ((u8 *) arg2 + 0x10) + var_a0) & arg3;
                        if ((temp_v0_2 & 0xFFFF) &&
                            ((temp_v0_2 >> 0x10) != 0) &&
                            (*(f32 *) (*(u8 **) ((u8 *) arg2 + 0x1C) +
                                       (temp_t9 * 4) + 0x4) > 0.5f)) {
                            var_a0_2 = 1;
                            temp_a3 = *(u8 **) ((u8 *) arg2 + 0x4) + (var_ra * 0x10);
                            var_v1_2 = temp_a3 + 1;
                            var_a1 = *(s16 *)
                                (temp_s1 + (*(u8 *) (temp_a3 + 1) * 0xA) + 0x2);
                            var_a2 = var_a1;
                            do {
                                var_a0_2 += 1;
                                temp_v0_3 = *(s16 *)
                                    (temp_s1 + (*(u8 *) (var_v1_2 + 1) * 0xA) + 0x2);
                                if (temp_v0_3 < var_a1) {
                                    var_a1 = temp_v0_3;
                                } else if (var_a2 < temp_v0_3) {
                                    var_a2 = temp_v0_3;
                                }
                                var_v1_2 += 1;
                            } while (var_a0_2 < 3);
                            if (*(s16 *) ((u8 *) arg0 + 0x18) >= var_a1) {
                                var_v1_3 = temp_a3;
                                if (var_a2 >= *(s16 *) ((u8 *) arg0 + 0x16)) {
                                    var_v0 = polygon;
                                    var_v0 += 0x10;
                                    var_lo = 0xA * *(u8 *) (var_v1_3 + 1);
                                    while (var_v0 != polygon + 0x30) {
                                        var_v0 += 0x10;
                                        var_v1_3 += 1;
                                        *(f32 *) (var_v0 - 0x20) =
                                            (f32) *(s16 *) (temp_s1 + var_lo);
                                        *(s16 *) (var_v0 - 0x12) = -1;
                                        *(f32 *) (var_v0 - 0x18) =
                                            (f32) *(s16 *)
                                                (temp_s1 + (*(u8 *) (var_v1_3 + 0x0) * 0xA) + 0x4);
                                        var_lo = *(u8 *) (var_v1_3 + 1) * 0xA;
                                    }
                                    *(f32 *) (var_v0 - 0x10) =
                                        (f32) *(s16 *) (temp_s1 + var_lo);
                                    *(s16 *) (var_v0 - 0x2) = -1;
                                    *(f32 *) (var_v0 - 0x8) =
                                        (f32) *(s16 *)
                                            (temp_s1 + (*(u8 *) (var_v1_3 + 1) * 0xA) + 0x4);
                                    if (shadowBoxPolyOverlap(
                                            *(f32 *) ((u8 *) arg0 + 0x40),
                                            *(f32 *) ((u8 *) arg0 + 0x44),
                                            *(f32 *) ((u8 *) arg0 + 0x48),
                                            *(f32 *) ((u8 *) arg0 + 0x4C), 3,
                                            polygon) != 0) {
                                        *(u8 **) ((u8 *) arg0 + 0x4) =
                                            *(u8 **) ((u8 *) arg2 + 0x1C) +
                                            (temp_t9 * 4);
                                        if (*(f32 *) ((u8 *) arg0 + 0x24) > 0.0f) {
                                            func_80018544(arg0, polygon);
                                        }
                                        temp_v0_4 = func_80017660(arg0, 3, polygon, 4, arg1);
                                        if (temp_v0_4 >= 3) {
                                            temp_t2 = D_800CAF60 + (D_800CAF58 * 0xC);
                                            *(u8 *) (temp_t2 + 1) = 0;
                                            var_t0_2 = 0;
                                            if (temp_v0_4 > 0) {
                                                var_a3 = polygon;
                                                do {
                                                    temp_v1 = *(s16 *) (var_a3 + 0xE);
                                                    var_a1_2 = -1;
                                                    var_a0_3 = 0;
                                                    if (temp_v1 < 0) {
                                                        var_a2_2 = D_800C9D40;
                                                        temp_t1 = temp_t2 + var_t0_2;
                                                        if (var_a2_2 > 0) {
                                                            var_v1_4 = D_800C9D48;
loop_27:
                                                            if ((*(f32 *) (var_v1_4 + 0x0) ==
                                                                 *(f32 *) (var_a3 + 0x0)) &&
                                                                (*(f32 *) (var_v1_4 + 0x8) ==
                                                                 *(f32 *) (var_a3 + 0x8))) {
                                                                var_a1_2 = var_a0_3;
                                                            }
                                                            var_a0_3 += 1;
                                                            var_v1_4 += 0x10;
                                                            if ((var_a0_3 < var_a2_2) &&
                                                                (var_a1_2 == -1)) {
                                                                goto loop_27;
                                                            }
                                                        }
                                                        if (var_a1_2 == -1) {
                                                            if (var_a2_2 >= 0x20) {
                                                                D_800C9D40 = 0x1F;
                                                                var_a2_2 = 0x1F;
                                                            }
                                                            var_v1_4 = D_800C9D48 + (var_a2_2 * 0x10);
                                                            *(f32 *) (var_v1_4 + 0x0) = *(f32 *) (var_a3 + 0x0);
                                                            *(f32 *) (var_v1_4 + 0x8) = *(f32 *) (var_a3 + 0x8);
                                                            D_800C9D40 = var_a2_2 + 1;
                                                            *(s8 *) (temp_t1 + 0x2) = var_a2_2;
                                                            *(s32 *) (var_v1_4 + 0xC) =
                                                                *(s32 *) ((u8 *) arg0 + 0x4);
                                                        } else {
                                                            *(s8 *) (temp_t1 + 0x2) = var_a1_2;
                                                        }
                                                    } else {
                                                        *(s8 *) (temp_t2 + var_t0_2 + 0x2) = temp_v1;
                                                        *(u8 *) (temp_t2 + 1) |= (1 << var_t0_2);
                                                    }
                                                    var_t0_2 += 1;
                                                    var_a3 += 0x10;
                                                } while (var_t0_2 != temp_v0_4);
                                            }
                                            *(u8 *) (temp_t2 + 0x0) = temp_v0_4;
                                            *(s16 *) (temp_t2 + 0xA) = *(s32 *) &surfaceId;
                                            D_800CAF58 += 1;
                                            if ((D_800CB268 >= 0) &&
                                                (*(s32 *) &surfaceId != D_800CB268)) {
                                                D_800CB26C = 0;
                                            }
                                            D_800CB268 = *(s32 *) &surfaceId;
                                        }
                                    }
                                }
                            }
                        }
                        var_ra += 1;
                        var_v1 += 8;
                        var_a0 += 4;
                    } while (var_ra < temp_a1);
                    var_t0 = *(s16 *) ((u8 *) arg2 + 0x24);
                }
            }
            var_t1 += 1;
            sp7C += 0x10;
        } while (var_t1 < var_t0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017140.s")
#endif
/*
 * PROVENANCE: adapted from Diddy Kong Racing's public matched
 * src/tracks.c:func_8002FF6C. JFG's public assembly-only func_8001F288 is the
 * closest sibling and corroborates the shared frame and control-flow shape.
 * Mickey's target assembly, extra owner argument, globals, and output limits
 * determine every local revision below, and the body is exact against them.
 */
typedef struct ShadowClipVertex {
    f32 x;
    f32 y;
    f32 z;
    s16 padC;
    s16 edgeIndex;
} ShadowClipVertex;

typedef struct ShadowClipPlane {
    f32 x;
    f32 z;
} ShadowClipPlane;

typedef struct ShadowClipEdge {
    f32 x;
    f32 y;
    f32 z;
    void *owner;
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
} ShadowClipEdge;

s32 func_80017660(void *arg0, s32 arg1, void *arg2, s32 arg3, s32 arg4) {
    ShadowClipVertex clipped[8];
    f32 temp_f12;
    f32 temp_f14;
    f32 temp_f16;
    f32 temp_f22;
    f32 temp_f24;
    f32 var_f2;
    s32 edgeIndex;
    s32 edgeOffset;
    s32 edgeCount;
    s32 outputCount;
    s32 planeIndex;
    s32 next;
    s32 var_v0;
    s32 vertexCount;
    ShadowClipVertex *output;
    ShadowClipVertex *vertices;
    ShadowClipVertex *swap;
    ShadowClipEdge *edge;
    ShadowClipEdge *edges;

    vertices = (ShadowClipVertex *) arg2;
    output = clipped;
    vertexCount = arg1;
    planeIndex = 0;
    edges = (ShadowClipEdge *) D_800C9F58;

    while ((planeIndex < arg3) && (vertexCount >= 3)) {
        next = planeIndex + 1;
        if (next >= arg3) {
            next = 0;
        }

        temp_f12 = ((ShadowClipPlane *) arg4)[next].z -
                   ((ShadowClipPlane *) arg4)[planeIndex].z;
        temp_f14 = -(((ShadowClipPlane *) arg4)[next].x -
                     ((ShadowClipPlane *) arg4)[planeIndex].x);
        if (((ShadowClipPlane *) arg4)[planeIndex].x <
            ((ShadowClipPlane *) arg4)[next].x) {
            var_f2 = -((temp_f12 * ((ShadowClipPlane *) arg4)[planeIndex].x) +
                       (((ShadowClipPlane *) arg4)[planeIndex].z * temp_f14));
        } else {
            var_f2 = -((temp_f12 * ((ShadowClipPlane *) arg4)[next].x) +
                       (((ShadowClipPlane *) arg4)[next].z * temp_f14));
        }

        for (var_v0 = 0, outputCount = 0; var_v0 < vertexCount; var_v0++) {
            next = var_v0 + 1;
            if (next >= vertexCount) {
                next = 0;
            }
            temp_f16 = (temp_f12 * vertices[var_v0].x) +
                       (vertices[var_v0].z * temp_f14) + var_f2;
            temp_f22 = (temp_f12 * vertices[next].x) +
                       (vertices[next].z * temp_f14) + var_f2;
            if (((temp_f16 >= 0.0f) && (temp_f22 < 0.0f)) ||
                ((temp_f16 < 0.0f) && (temp_f22 >= 0.0f))) {
                edgeIndex = -1;
                edgeCount = D_800C9F48[planeIndex];
                edgeOffset = planeIndex << 5;
                edge = edges;
                edge += edgeOffset;
                for (; edgeCount > 0; edge++, edgeOffset++) {
                    if (!((edge->x0 == vertices[var_v0].x) &&
                          (edge->z0 == vertices[var_v0].z) &&
                          (edge->x1 == vertices[next].x) &&
                          (edge->z1 == vertices[next].z))) {
                        edgeCount--;
                        if (!((edge->x0 == vertices[next].x) &&
                              (edge->z0 == vertices[next].z) &&
                              (edge->x1 == vertices[var_v0].x) &&
                              (edge->z1 == vertices[var_v0].z))) {
                            continue;
                        }
                    }
                    edgeIndex = edgeOffset;
                    break;
                }
                if (edgeIndex >= 0) {
                    output[outputCount].edgeIndex = edgeIndex;
                    output[outputCount].x = edge->x;
                    output[outputCount].z = edge->z;
                    outputCount++;
                    if (outputCount >= 8) {
                        return 0;
                    }
                } else {
                    temp_f24 = temp_f16 / (temp_f16 - temp_f22);
                    output[outputCount].x = vertices[var_v0].x +
                        ((vertices[next].x - vertices[var_v0].x) * temp_f24);
                    output[outputCount].z = vertices[var_v0].z +
                        ((vertices[next].z - vertices[var_v0].z) * temp_f24);
                    edge->x0 = vertices[var_v0].x;
                    edge->z0 = vertices[var_v0].z;
                    edge->x1 = vertices[next].x;
                    edge->z1 = vertices[next].z;
                    edge->x = output[outputCount].x;
                    edge->z = output[outputCount].z;
                    edge->owner = *(void **) ((u8 *) arg0 + 4);
                    output[outputCount].edgeIndex = edgeOffset;
                    outputCount++;
                    if (outputCount >= 8) {
                        return 0;
                    }
                    D_800C9F48[planeIndex]++;
                }
            }
            if (temp_f22 <= 0) {
                output[outputCount].edgeIndex = vertices[next].edgeIndex;
                output[outputCount].x = vertices[next].x;
                output[outputCount].z = vertices[next].z;
                outputCount++;
                if (outputCount >= 8) {
                    return 0;
                }
            }
        }

        vertexCount = outputCount;
        planeIndex++;
        swap = vertices;
        vertices = output;
        output = swap;
    }

    if (vertexCount >= 3) {
        if ((void *) vertices != arg2) {
            for (var_v0 = 0; var_v0 < vertexCount; var_v0++) {
                ((ShadowClipVertex *) arg2)[var_v0].x = vertices[var_v0].x;
                ((ShadowClipVertex *) arg2)[var_v0].z = vertices[var_v0].z;
                ((ShadowClipVertex *) arg2)[var_v0].edgeIndex =
                    vertices[var_v0].edgeIndex;
            }
        }
    } else {
        vertexCount = 0;
    }
    return vertexCount;
}
/*
 * PROVENANCE: adapted from the public Diddy Kong Racing/JFG shadow-buffer
 * and projected-triangle organization; Mickey's target bytes, globals, and
 * resident buffer layouts determine the field bindings below.
 */
#ifdef NON_MATCHING
/* 2026-10-01 (lane d-res1): the three m2c goto loops written as do-while
 * loops are 217 masked at delta 0 (was 221): same statements, same order.
 * Workbench verdict: 221 masked words at size delta 0 (was 270 at +8).
 * Track B, 2026-09-23:
 *   - the +8 was one extra callee-saved FP register. The loop carried the two
 *     rotation deltas as fresh temps where the target reassigns the loaded
 *     coordinates (x -= cx; z -= cz), which freed a caller-saved FP register
 *     and stopped 1.0f being hoisted into f20.
 *   - the cosine result is copied into var_f16 inside each arm, after the
 *     sine reload, which is the target's duplicated arm tail (delta 0).
 *   - the frame is a declaration count: nine words before projected, eleven
 *     between it and spA8 (home +0xA8), and two unreferenced pads (L99).
 *   - var_a1 is a plain int shifted in place (no u8 mask); z is loaded in
 *     each arm; the rotated pair is two named results.
 * Left: the sine is spilled by the allocator in the target (store in the
 * second jal's delay slot) where this form stores at the assignment; 0.0f
 * shares f20 with var_f0 in the target while here it is a separate web in
 * f12, and var_f18/var_f0 take f20/f18 swapped (forcing p1 var_f18=c29,
 * var_f0=c30, zero=c30 prices 215); a separate x-delta web, which the
 * target's f12 suggests, costs +24 here by re-hoisting 1.0f. */
s32 func_80017BCC(void *arg0, void *arg1, void *arg2) {
    s32 var_fp;
    s32 var_s4;
    s32 var_s7;
    s32 var_t2;
    s32 var_t3;
    s32 var_v1;
    s32 var_a0_2;
    s32 var_a1_2;
    s32 var_t5;
    u32 projected[3];
    u8 *var_a0;
    u8 *var_a2;
    u8 *var_a3;
    u8 *var_s6;
    u8 *var_t4;
    u8 *var_v0;
    u8 *temp_v0;
    s32 var_a1;
    u8 vertexCount;
    f32 temp_f0;
    f32 temp_f12;
    f32 temp_f2;
    f32 spA8;
    f32 temp_f2_2;
    f32 var_f0;
    f32 var_f0_2;
    f32 var_f12;
    f32 var_f14;
    f32 var_f16;
    f32 var_f18;
    f32 var_f22;
    f32 var_f24;
    f32 var_f26;
    f32 var_f28;
    s32 pad0;
    s32 pad1;

    if ((*(u8 *) ((u8 *) arg2 + 0x10) & 0x10) != 0) {
        var_f14 = 0.0f;
        var_f16 = 1.0f;
    } else {
        if (arg1 != NULL) {
            spA8 = func_8002A8C0(*(s16 *) ((u8 *) arg1 + 0x0));
            var_f0 = func_8002A8BC(*(s16 *) ((u8 *) arg1 + 0x0));
            var_f14 = *(f32 *) &spA8;
            var_f16 = var_f0;
        } else {
            spA8 = func_8002A8C0(*(s16 *) ((u8 *) arg0 + 0x14));
            var_f0 = func_8002A8BC(*(s16 *) ((u8 *) arg0 + 0x14));
            var_f14 = *(f32 *) &spA8;
            var_f16 = var_f0;
        }
    }
    temp_v0 = *(u8 **) ((u8 *) arg0 + 0x0);
    var_f26 = *(f32 *) ((u8 *) arg0 + 0x34);
    temp_f12 = *(f32 *) ((u8 *) arg0 + 0x24);
    var_f28 = *(f32 *) ((u8 *) arg0 + 0x38);
    var_f18 = 255.0f;
    var_t5 = 0x19;
    var_f22 = (f32) (*(u16 *) (temp_v0 + 0x6) * 0x10) / var_f26;
    var_t4 = D_800CAF60;
    var_s7 = 0;
    var_f24 = (f32) (*(u16 *) (temp_v0 + 0x8) << 5) /
              (var_f28 + *(f32 *) ((u8 *) arg0 + 0x3C));
    if (temp_f12 > 0.0f) {
        temp_f2 = *(f32 *) ((u8 *) arg0 + 0xC) -
                  *(f32 *) ((u8 *) arg0 + 0x20);
        if (temp_f12 < temp_f2) {
            var_f18 = 255.0f *
                      (1.0f - ((temp_f2 - temp_f12) /
                               *(f32 *) ((u8 *) arg0 + 0x28)));
            if (var_f18 < 0.0f) {
                var_f18 = 0.0f;
            }
        }
        if (temp_f2 > 0.0f) {
            temp_f0 = (temp_f2 / 200.0f) + 1.0f;
            var_f22 *= temp_f0;
            var_f26 /= temp_f0;
            var_f24 *= temp_f0;
            var_f28 /= temp_f0;
        }
    }
    var_s4 = (s32) (var_f18 * D_800CB260);
    if (arg1 != NULL) {
        var_s4 = (s32) (*(s16 *) ((u8 *) arg1 + 0x4) * var_s4) >> 8;
    }
    var_f18 = D_800CB270;
    var_f0 = D_800CB274;
    var_t2 = D_8007944C;
    var_t3 = D_80079450;
    var_a3 = D_80079444 + (var_t3 * 0x10);
    var_fp = D_80079454;
    var_a2 = D_80079440 + (var_t2 * 0xA);
    var_s6 = D_80079448 + (var_fp * 8);
    if (D_800CAF58 > 0) {
        do {
            var_a0 = var_t4;
            if ((*(u8 *) (var_t4 + 0x0) + var_t5) >= 0x18) {
                *(s16 *) (var_s6 + 0x6) = var_t2;
                *(s16 *) (var_s6 + 0x4) = var_t3;
                var_s6 += 8;
                var_fp += 1;
                var_t5 = 0;
                *(u32 *) (var_s6 - 0x8) = *(u32 *) ((u8 *) arg0 + 0x0);
            }
            if (var_fp >= D_800CB280) {
                return 0;
            }
            var_a1 = *(u8 *) (var_t4 + 0x1);
            var_v1 = 0;
            vertexCount = *(u8 *) (var_t4 + 0x0);
            if ((s32) vertexCount > 0) {
                do {
                    if (var_a1 & 1) {
                        var_v0 = D_800C9F58 + (*(u8 *) (var_a0 + 0x2) << 5);
                        var_f0_2 = *(f32 *) (var_v0 + 0x0);
                        var_f12 = *(f32 *) (var_v0 + 0x4);
                        temp_f2_2 = *(f32 *) (var_v0 + 0x8);
                    } else {
                        var_v0 = D_800C9D48 + (*(u8 *) (var_a0 + 0x2) * 0x10);
                        var_f0_2 = *(f32 *) (var_v0 + 0x0);
                        var_f12 = *(f32 *) (var_v0 + 0x4);
                        temp_f2_2 = *(f32 *) (var_v0 + 0x8);
                    }
                    var_a1 = var_a1 >> 1;
                    var_t2 += 1;
                    var_a2 += 0xA;
                    *(s16 *) (var_a2 - 0xA) = (s32) var_f0_2;
                    *(u8 *) (var_a2 - 0x4) = 0xFF;
                    *(u8 *) (var_a2 - 0x3) = 0xFF;
                    *(u8 *) (var_a2 - 0x2) = 0xFF;
                    *(s8 *) (var_a2 - 0x1) = (s8) var_s4;
                    *(s16 *) (var_a2 - 0x6) = (s32) temp_f2_2;
                    *(s16 *) (var_a2 - 0x8) =
                        (s32) (*(f32 *) ((u8 *) arg0 + 0x1C) + var_f12);
                    if (var_t2 >= D_800CB278) {
                        return 0;
                    }
                    var_f0_2 -= var_f18;
                    var_a0 += 1;
                    temp_f2_2 -= var_f0;
                    temp_f0 = (var_f0_2 * var_f16) - (temp_f2_2 * var_f14);
                    temp_f2_2 = (temp_f2_2 * var_f16) + (var_f0_2 * var_f14);
                    projected[var_v1] =
                        ((s32) ((temp_f2_2 + var_f28) * var_f24) & 0xFFFF) |
                        ((s32) (var_f22 * (temp_f0 + var_f26)) << 0x10);
                    var_v1 += 1;
                } while (var_v1 < (s32) *(u8 *) (var_t4 + 0x0));
            }
            var_v1 = 1;
            if ((*(u8 *) (var_t4 + 0x0) - 1) >= 2) {
                var_a0_2 = var_t5 + 1;
                var_a1_2 = var_a0_2 + 1;
                var_v0 = (u8 *) &projected[1];
                do {
                    *(u8 *) (var_a3 + 0x0) = 0;
                    *(u8 *) (var_a3 + 0x1) = var_a0_2;
                    *(u8 *) (var_a3 + 0x2) = var_a1_2;
                    *(u8 *) (var_a3 + 0x3) = var_t5;
                    var_t3 += 1;
                    var_v1 += 1;
                    *(u32 *) (var_a3 + 0x4) = *(u32 *) (var_v0 + 0x0);
                    var_a3 += 0x10;
                    *(u32 *) (var_a3 - 0x8) = *(u32 *) (var_v0 + 0x4);
                    *(u32 *) (var_a3 - 0x4) = projected[0];
                    if (var_t3 >= D_800CB27C) {
                        return 0;
                    }
                    var_v0 += 4;
                    var_a0_2 += 1;
                    var_a1_2 += 1;
                } while (var_v1 < (*(u8 *) (var_t4 + 0x0) - 1));
            }
            var_s7 += 1;
            var_t5 += *(u8 *) (var_t4 + 0x0);
            var_t4 += 0xC;
        } while (var_s7 < D_800CAF58);
    }
    D_8007944C = var_t2;
    D_80079450 = var_t3;
    D_80079454 = var_fp;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017BCC.s")
#endif
/*
 * PROVENANCE: adapted from the public Diddy Kong Racing decompilation,
 * src/tracks.c func_8002DE30 (the object-under-shadow shade update): same
 * loops, same per-face mask test and the same reuse of the sector index as
 * the mask temporary, including its empty `if`.  Mickey's offsets, flag mask,
 * shade shift and globals are authoritative.
 *
 * Matched 2026-10-02 (lane x-shad) from 61 masked words: the m2c-derived
 * shape (explicit block offset carrier, triangle-vertex cursor, address-form
 * sector index) was replaced by DKR's indexed `sector->batches[i]` loops.
 * Two facts were load-bearing: the sector index reassigned to the face mask
 * inside the face loop (that reuse is what spills it across the
 * getXZCompareMask call instead of giving it s0), and the declaration order,
 * which puts the index at home +0x84, `i` at +0x74 and the mask at +0x64.
 */
void func_800180B4(ShadowQuery *query) {
    s32 yMax;
    s32 yMin;
    s32 sectorIndex;
    s32 k;
    u32 shade;
    s32 done;
    s32 i;
    ShadowSector *sector;
    ShadowTriangle *triangle;
    ShadowPoint *vertices;
    s32 mask;
    s32 lowY;
    s32 highY;
    s32 j;

    yMax = (s32) query->y10 + query->volume40->maxY6E;
    yMin = (s32) query->y10 + query->volume40->minY6C;
    sectorIndex = query->sector2E;
    done = FALSE;
    if (sectorIndex != -1) {
        mask = getXZCompareMask(&((ShadowWorld *) D_800CB284)->boundingBoxes[sectorIndex],
                                query->x0C - 16.0f, query->z14 - 16.0f,
                                query->x0C + 16.0f, query->z14 + 16.0f);
        sector = &((ShadowWorld *) D_800CB284)->sectors[sectorIndex];
        for (i = 0; i < sector->numberOfBatches && !done; i++) {
            if (!(sector->batches[i].flags & 0x08013880)) {
                shade = (sector->batches[i].flags >> 24) & 7;
                vertices = &sector->vertices[sector->batches[i].verticesOffset];
                for (j = sector->batches[i].facesOffset;
                     j < sector->batches[i + 1].facesOffset && !done; j++) {
                    sectorIndex = sector->faceMasks[j] & mask;
                    if (sectorIndex) {}
                    if (((sector->faceMasks[j] & mask) & 0xFFFF) &&
                        ((sector->faceMasks[j] & mask) >> 16)) {
                        triangle = &sector->triangles[j];
                        lowY = vertices[triangle->verticesArray[1]].y;
                        highY = lowY;
                        for (k = 1; k < 3; k++) {
                            if (vertices[triangle->verticesArray[k + 1]].y < lowY) {
                                lowY = vertices[triangle->verticesArray[k + 1]].y;
                            } else if (highY < vertices[triangle->verticesArray[k + 1]].y) {
                                highY = vertices[triangle->verticesArray[k + 1]].y;
                            }
                        }
                        if (highY >= yMin && yMax >= lowY) {
                            if (mathXZInTri(query->x0C, query->z14,
                                            &vertices[triangle->verticesArray[1]],
                                            &vertices[triangle->verticesArray[2]],
                                            &vertices[triangle->verticesArray[3]])) {
                                done = TRUE;
                                *query->value50 += ((1.0f - D_80079464[shade]) - *query->value50) *
                                                   D_800CB28C;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* PLATEAU-HANDOFF:func_80017140:start
 * symbol: func_80017140
 * score: 285/328 words
 * frame: 0x140
 * relocations: 19
 * first-mismatch: +0x44
 * summary: Delta 0 (was -12) via target home order and address-form surfaceId/sp7C; left: hoisted literal 3 and polygon base, uncached D_800CAF58
 * PLATEAU-HANDOFF:func_80017140:end
 */

/* PLATEAU-HANDOFF:func_80017BCC:start
 * symbol: func_80017BCC
 * score: 217/314 words
 * frame: 0x108
 * relocations: 44
 * first-mismatch: +0x58
 * summary: goto loops as do-while 221 to 217; open: zero constant in f20 vs f12 and the s7/fp batch-counter swap (p1 ranking)
 * PLATEAU-HANDOFF:func_80017BCC:end
 */

/* PLATEAU-HANDOFF:shadowGenerate:start
 * symbol: shadowGenerate
 * score: 445 differing words
 * frame: 0x150
 * relocations: 63
 * first-mismatch: +0xC
 * summary: Fresh V0 is 520/510 words with 445 differences; frames 0x150/0x138. Both have 63 relocations; 29 sites and identities align. Prior mechanisms closed.
 * PLATEAU-HANDOFF:shadowGenerate:end
 */

/* PLATEAU-HANDOFF:func_80016890:start
 * symbol: func_80016890
 * score: 534/563 words
 * frame: 0x190
 * relocations: 48
 * first-mismatch: +0x8
 * summary: 563->534, delta +60->+28: radius as query field, arg2 copied to a local (s3), query stores first, f32* points arg
 * PLATEAU-HANDOFF:func_80016890:end
 */
