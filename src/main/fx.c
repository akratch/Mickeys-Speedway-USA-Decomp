/*
 * Resident visual effects -- ROM 0x47A70-0x4BC40 (VRAM 0x80046E70).
 *
 * PROVENANCE: the translation-unit identity and the descriptive cone/wake
 * names are adapted from Jet Force Gemini's public decompilation, src/fx.c.
 * Mickey begins at JFG's fxFreeCone portion of that TU; the matching sequence
 * of texture, allocator, trigonometry and draw calls establishes the named
 * routines below. Externally referenced functions and unresolved JFG
 * placeholders retain Mickey address names. The bodies remain Mickey's
 * extracted assembly.
 */

#include "game/fx.h"
#include "n_audio/mbi.h"

typedef struct FxConePoint {
    f32 x;
    f32 y;
    f32 z;
} FxConePoint;

typedef FxCone FxConeCoords;

typedef struct FxWakeRippleData {
    u8 pad0[0x70];
    void *texture;
    u8 mode;
    u8 active;
    s16 fade;
    s16 angle;
    s16 angleStep;
    f32 value7C;
    f32 value80;
    Wake *update;
} FxWakeRippleData;

typedef struct FxWakeTexture {
    u8 pad0[0x10];
    u16 length;
} FxWakeTexture;

typedef struct FxWakeLinked {
    u8 pad0[4];
    s16 flags;
} FxWakeLinked;

typedef struct FxWakeUpdateOwner {
    u8 pad0[0x0C];
    f32 valueC;
    u8 pad10[4];
    f32 value14;
    u8 pad18[4];
    f32 value1C;
    u8 pad20[4];
    f32 value24;
    u8 pad28[0x2C];
    FxWakeRippleData *ripple;
} FxWakeUpdateOwner;

typedef struct FxWakeSegment {
    s32 x;
    s32 y;
    s32 z;
    u8 padC[2];
    s16 length;
} FxWakeSegment;

extern void func_80048080(s32 count, s16 arg1, s16 arg2, s16 arg3,
                          s16 arg4, s16 arg5, FxConePoint *points,
                          u8 *vertices, s32 alpha);
extern void viGetCurrentSize(s32 *width, s32 *height);
extern s16 Arctanf(f32 x, f32 y);
extern s32 viGetVideoMode(void);
extern void wakeUpdate(Wake *wake, f32 x, f32 height, f32 z, s16 angle,
                       s32 delta);
extern void mathOneFloatPY(void *source, f32 *result, s16 angle);
extern void camSetScissor(Gfx **dlist);
extern void func_80034920();
extern void *func_8002B314(s32 size, s32 tag);
extern u8 D_7D310[];

void func_80046E70(FxCone *cone) {
    FxConeTextureInfo *texture;
    FxConeTextureInfo *alternateTexture;

    texture = cone->texture.pointer;
    if (texture != 0) {
        func_800347A0(texture);
    }
    alternateTexture = cone->alternateTexture.pointer;
    if (alternateTexture != 0) {
        func_800347A0(alternateTexture);
    }
    mmFree(cone);
}
/* Matched by writing the block as stores and re-reads of the cone's own
 * fields: the header size leads the allocation sum, each buffer address is
 * the previous field plus a size, and the mode test repeats `arg8 + 1`
 * instead of carrying it in a local. The inherited `cone + 0x38` carrier,
 * the `if (1) { }` region and the mode local were the whole residual. */
extern void *func_8002B280(s32 size, s32 tag);
extern void *func_80034448(s32 resourceId);
extern void func_800470B0(FxCone *, s16, s16, s16, s16, s16,
                          f32, f32, f32);
extern void func_80047304(FxCone *, s16, s16, s16, s16, s16,
                          f32, f32, f32);
extern void func_800475E8(FxCone *, s16);

void *func_80046EC4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                    f32 arg5, f32 arg6, f32 arg7, s32 arg8, s32 arg9,
                    s32 argA) {
    s32 vertexBytes;
    s32 bufferBytes;
    s32 pad;
    s32 flags;
    FxCone *cone;

    flags = arg8 & 0x80;
    arg8 = arg8 & 0x7F;
    if (arg8 == 0) {
        vertexBytes = 0x280;
        bufferBytes = 0x154;
    } else {
        vertexBytes = arg8 * 0x10;
        bufferBytes = (arg8 * 0xA) + 0xA;
    }
    cone = (FxCone *) func_8002B280(
        sizeof(FxCone) + vertexBytes + bufferBytes * 2, 0x87);
    if (cone != NULL) {
        if (arg9 >= 0) {
            cone->texture.value = (s32) func_80034448(arg9);
        } else {
            cone->texture.value = 0;
        }
        if (argA >= 0) {
            cone->alternateTexture.value = (s32) func_80034448(argA);
        } else {
            cone->alternateTexture.value = 0;
        }
        cone->vertices = (u8 *) (cone + 1);
        cone->addresses[0] = cone->vertices + vertexBytes;
        cone->addresses[1] = cone->addresses[0] + bufferBytes;
        cone->mode = arg8 + 1;
        cone->segmentCount = arg8;
        cone->addressIndex = 0;
        cone->flags = flags;
        cone->value22 = arg4;
        cone->value20 = arg3;
        cone->value24 = arg7;
        cone->value18 = arg5;
        cone->value1C = arg6;
        cone->value2A = arg2;
        cone->value28 = arg1;
        cone->value26 = arg0;
        if (arg8 + 1 == 1) {
            func_80047304(cone, arg0, arg1, arg2, arg3, arg4, arg5, arg6,
                          arg7);
        } else {
            func_800470B0(cone, arg0, arg1, arg2, arg3, arg4, arg5, arg6,
                          arg7);
        }
        func_800475E8(cone, 0);
    }
    return cone;
}
void func_8004707C(FxCone *cone, s32 value2C, s32 value2D, s32 value2E,
                   s32 value30, s32 value31, s32 value32) {
    if (cone != 0) {
        cone->primRed = value2C;
        cone->primGreen = value2D;
        cone->primBlue = value2E;
        cone->envRed = value30;
        cone->envGreen = value31;
        cone->envBlue = value32;
    }
}
/* PROVENANCE: JFG's public src/fx.c establishes the corresponding cone
 * routine and call roles (it carries the routine as assembly only); this body
 * is written from Mickey's own listing and typed layouts.
 * Matched by writing each loop in its plain form: `while (n--)` over the rim
 * points, `for (i = 0; i < 2; i++)` over the two address buffers (the scaled
 * counter and the cursor are both the compiler's), and a counted `for` over
 * the fan indices. The two pad locals place the point array's home. */
void func_800470B0(FxCone *cone, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                   s16 arg5, f32 arg6, f32 arg7, f32 arg8) {
    FxConePoint *point;
    u8 *vertex;
    s32 step;
    f32 z0;
    f32 z1;
    s32 angle;
    s32 i;
    s32 n;
    s32 pad0;
    s32 pad1;
    FxConePoint points[16];

    if (cone->flags != 0) {
        z0 = 0.0f;
        z1 = -arg8;
        step = -0x10000 / cone->segmentCount;
    } else {
        z1 = 0.0f;
        z0 = -arg8;
        step = 0x10000 / cone->segmentCount;
    }
    points[0].x = 0.0f;
    points[0].y = 0.0f;
    points[0].z = z0;
    point = &points[1];
    angle = 0;
    n = cone->segmentCount;
    while (n--) {
        point->x = func_8002A8C0(angle) * arg6;
        point->y = func_8002A8BC(angle) * arg7;
        point->z = z1;
        point++;
        angle += step;
    }
    for (i = 0; i < 2; i++) {
        func_80048080(cone->mode, arg1, arg2, arg3, arg4, arg5, points,
                      cone->addresses[i], 0xFF);
    }
    vertex = cone->vertices;
    for (i = 1; i <= cone->segmentCount; i++) {
        vertex[0] = 0;
        vertex[1] = i;
        vertex[2] = i + 1;
        vertex[3] = 0;
        vertex += 0x10;
    }
    vertex[-0xE] = 1;
}

/* PROVENANCE: JFG's public src/fx.c carries the corresponding cone routine as
 * assembly only; this body is written from Mickey's own listing in the shape
 * of its matched sibling func_800470B0.
 * Matched by discarding the inherited m2c shape: a 17-point array walked by
 * one `point` cursor (the second ring is `point[8]`), plain counted loops
 * (the third vertex loop is IDO's own four-way unroll), and 0.33f written as
 * the literal it is. The literal is the TU's own pool word (ROM 0x849E4); as a
 * global it cannot be hoisted past the trig calls, which put scaleZ's load
 * ahead of scaleX/scaleY in the schedule. `i` and the pad below the array
 * place its home at sp+0x98. */
void func_80047304(FxCone *cone, s16 arg1, s16 arg2, s16 arg3, s16 arg4,
                   s16 arg5, f32 arg6, f32 arg7, f32 arg8) {
    u8 *vertex;
    FxConePoint *point;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 sine;
    f32 cosine;
    FxConePoint points[17];
    s32 i;
    s32 pad;

    scaleX = arg6 * 4.0f;
    scaleY = arg7 * 4.0f;
    scaleZ = -(arg8 * 0.33f);
    points[0].x = 0.0f;
    points[0].y = 0.0f;
    points[0].z = -arg8;
    point = &points[1];
    for (i = 0; i < 8; i++) {
        sine = func_8002A8C0(i << 13);
        cosine = func_8002A8BC(i << 13);
        point->x = arg6 * sine;
        point->y = arg7 * cosine;
        point->z = 0.0f;
        point[8].x = 2.0f * (scaleX * sine);
        point[8].y = 2.0f * (scaleY * cosine);
        point[8].z = scaleZ;
        point++;
    }
    for (i = 0; i < 2; i++) {
        func_80048080(17, arg1, arg2, arg3, arg4, arg5, points,
                      cone->addresses[i], 0xFF);
    }
    vertex = cone->vertices;
    for (i = 1; i < 9; i++) {
        vertex[0] = 0;
        vertex[1] = i;
        vertex[2] = i + 8;
        vertex[3] = (i & 7) + 9;
        vertex[0x10] = 0;
        vertex[0x11] = i;
        vertex[0x12] = (i & 7) + 9;
        vertex[0x13] = (i & 7) + 1;
        vertex += 0x20;
    }
    for (i = 1; i < 9; i++) {
        vertex[0] = 0;
        vertex[1] = i;
        vertex[2] = (i & 7) + 9;
        vertex[3] = i + 8;
        vertex[0x10] = 0;
        vertex[0x11] = i;
        vertex[0x12] = (i & 7) + 1;
        vertex[0x13] = (i & 7) + 9;
        vertex += 0x20;
    }
    for (i = 1; i < 9; i++) {
        vertex[0] = 0;
        vertex[1] = i;
        vertex[2] = (i & 7) + 1;
        vertex[3] = 0;
        vertex += 0x10;
    }
}
/* PROVENANCE: JFG's fxMakeConeTextureCoords body is assembly-only; this is
 * written from Mickey's own listing.
 * Matched by a natural rewrite (plain loops IDO unrolls itself, 102.4f as
 * the TU's own literal) and then one ring draw. The common loop's preheader
 * emits its items in expression-number order, numbered at first occurrence:
 * with the edge scales spelled inline in the eight-point loop, the t[] base
 * was numbered ahead of them and its temporary took the first ring register.
 * Assigning widthEdge/heightEdge before that loop numbers (width - 1) and
 * (height - 1) first. segmentCount carries the eight-point angle and the
 * 32-vertex countdown as well as the count, which with the texture local
 * keeps the frame at 0xF8 and the arrays at sp+0xB8/sp+0x90. */
void func_800475E8(FxCone *cone, s16 angle) {
    FxConeTextureInfo *texture;
    FxConeVertex *vertex;
    s32 width;
    s32 height;
    s32 segmentCount;
    s32 step;
    s16 s[20];
    s16 t[20];
    f32 widthEdge;
    f32 heightEdge;
    s32 i;
    f32 sine;
    f32 cosine;

    if (cone == NULL) {
        return;
    }
    segmentCount = angle;
    texture = cone->texture.pointer;
    if (texture == NULL) {
        return;
    }
    width = texture->width * 16;
    height = texture->height * 16;
    vertex = (FxConeVertex *) cone->vertices;
    if (cone->segmentCount == 0) {
        widthEdge = width - 1;
        heightEdge = height - 1;
        for (i = 0; i != 8; i++) {
            sine = func_8002A8C0(segmentCount);
            cosine = func_8002A8BC(segmentCount);
            s[i + 1] = (s32) (102.4f * sine) + width;
            t[i + 1] = (s32) (102.4f * cosine) + height;
            s[i + 9] = (s32) (widthEdge * sine) + width;
            t[i + 9] = (s32) (heightEdge * cosine) + height;
            segmentCount += 0x2000;
        }
        segmentCount = 32;
        while (segmentCount--) {
            vertex->s0 = s[vertex->index0];
            vertex->t0 = t[vertex->index0];
            vertex->s1 = s[vertex->index1];
            vertex->t1 = t[vertex->index1];
            vertex->s2 = s[vertex->index2];
            vertex->t2 = t[vertex->index2];
            vertex++;
        }
        segmentCount = 8;
        step = 0x2000;
    } else {
        segmentCount = cone->segmentCount;
        step = 0x10000 / segmentCount;
    }
    i = 0;
    if (segmentCount >= 0) {
        do {
            s[i] = (s32) (func_8002A8C0(angle) * (f32) (width - 1)) + width;
            t[i] = (s32) (func_8002A8BC(angle) * (f32) (height - 1)) + height;
            angle += step;
            i++;
        } while (i <= segmentCount);
    }
    for (i = 0; i < segmentCount; i++) {
        vertex->s0 = s[i];
        vertex->t0 = t[i];
        vertex->s1 = s[i + 1];
        vertex->t1 = t[i + 1];
        vertex->s2 = width;
        vertex->t2 = height;
        vertex++;
    }
}
/* PROVENANCE: JFG's fxMakeConeLength role identifies the routine; this body is reconstructed from Mickey's target offsets and m2c control flow.
 * Matched 2026-09-23 (Track B, lane B-fx). What closed it, in order:
 *  - frame 0x150: six declared slots above points and five between points
 *    and vertices, whose spill home is +0x6C. Unused declarations
 *    (unused, addressIndex, scale) keep their homes, so they stay.
 *  - size: no addressIndex carrier (its copy was the extra word), and
 *    while (i--) for the first loop (the target keeps the post-decrement
 *    copy and tests the counter).
 *  - FP colours: the cast height in its own web (originZ), the scale
 *    written inline, and originZ *= before the negation. */
void func_800479D4(FxCone *cone, s16 height, f32 radius, f32 depth,
                   s32 alpha) {
    s32 unused;
    FxConePoint *point;
    u8 addressIndex;
    s32 angle;
    s32 i;
    s32 step;
    FxConePoint points[15];
    f32 originZ;
    f32 scale;
    f32 scaleX;
    f32 scaleY;
    f32 factor;
    u8 *vertices;
    f32 temp;

    if (cone != 0) {
        point = points;
        cone->addressIndex = cone->addressIndex ^ 1;
        vertices = cone->addresses[cone->addressIndex];
        if (cone->flags != 0) {
            angle = 0;
            i = cone->segmentCount;
            step = -0x10000 / i;
            while (i--) {
                point->x = func_8002A8C0(angle) * radius;
                temp = func_8002A8BC(angle) * depth;
                point->z = (f32) -height;
                point++;
                angle += step;
                point[-1].y = temp;
            }
            func_80048080(cone->segmentCount, cone->value26, cone->value28,
                          cone->value2A, (s32) cone->value20,
                          (s32) cone->value22, points, vertices + 0xA, 0);
            return;
        }
        point->z = (f32) -height;
        mathOneFloatPY((u8 *) cone + 0x20, &points[0].x, height);
        *(s16 *) vertices = (s16) ((s32) points[0].x + cone->value26);
        *(s16 *) (vertices + 2) =
            (s16) ((s32) points[0].y + cone->value28);
        *(s16 *) (vertices + 4) =
            (s16) ((s32) points[0].z + cone->value2A);
        if (cone->segmentCount == 0) {
            scaleX = cone->value18;
            scaleY = cone->value1C;
            originZ = (f32) cone->value24;
            if (alpha < 0x80) {
                factor = 0.0f;
            } else if (alpha >= 0x100) {
                factor = 1.0f;
            } else {
                factor = (f32) (alpha - 0x7F) * 0.0078125f;
            }
            scaleX *= 1.0f + (2.0f * factor);
            scaleY *= 1.0f + (2.0f * factor);
            i = 0;
            originZ *= 0.25f * factor;
            temp = -originZ;
            do {
                angle = i << 0xD;
                point->x = func_8002A8C0(angle) * scaleX;
                originZ = func_8002A8BC(angle) * scaleY;
                i++;
                point->z = temp;
                point++;
                point[-1].y = originZ;
            } while (i != 8);
            func_80048080(8, cone->value26, cone->value28, cone->value2A,
                          (s32) cone->value20, (s32) cone->value22, points,
                          vertices + 0x5A, 0xFF);
        }
    }
}

/*
 * PROVENANCE: the block-local display-list macro spelling below is adapted
 * from Jet Force Gemini include/f3ddkr.h. Mickey's own bytes establish every
 * invocation, argument, constant and operation order in func_80047CD8.
 */
#define FX_SHIFTL(value, shift, width) \
    ((u32)(((u32)(value) & ((1U << (width)) - 1U)) << (shift)))
#define FX_PIPE_SYNC(packet) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xE7, 24, 8); \
    _g->w1 = 0; \
}
#define FX_SET_PRIM(packet, red, green, blue, alpha) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xFA, 24, 8); \
    _g->w1 = FX_SHIFTL(red, 24, 8) | FX_SHIFTL(green, 16, 8) | \
             FX_SHIFTL(blue, 8, 8) | FX_SHIFTL(alpha, 0, 8); \
}
#define FX_SET_ENV(packet, red, green, blue, alpha) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(0xFB, 24, 8); \
    _g->w1 = FX_SHIFTL(red, 24, 8) | FX_SHIFTL(green, 16, 8) | \
             FX_SHIFTL(blue, 8, 8) | FX_SHIFTL(alpha, 0, 8); \
}
#define FX_VERTEX_JFG(packet, address, count, first) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL(4, 24, 8) | \
             FX_SHIFTL(((count) << 3) | ((u32)(address) & 6) | (first), \
                       16, 8) | \
             FX_SHIFTL(((count) << 3) + ((count) << 1) + 8, 0, 16); \
    _g->w1 = (u32)(address); \
}
#define FX_POLYGON(packet, address, count, textured) { \
    FxGfx *_g = (FxGfx *)(packet); \
    _g->w0 = FX_SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
             FX_SHIFTL(5, 24, 8) | FX_SHIFTL((count) * 16, 0, 16); \
    _g->w1 = (u32)(address); \
}

/* Mickey-derived body; JFG's corresponding fxDrawCone body is assembly-only.
 * The white-color block retains an unsigned XOR-zero allocation lever. */
void func_80047CD8(FxGfx **dList, FxCone *cone, s32 flags, u8 alpha) {
    s32 hasTexture;

    if (cone != 0) {
        FX_PIPE_SYNC((*dList)++);
        if (flags & 0x200) {
            FX_SET_PRIM((*dList)++, cone->primRed, cone->primGreen,
                        cone->primBlue, alpha);
            FX_SET_ENV((*dList)++, cone->envRed, cone->envGreen,
                       cone->envBlue, 0);
        } else {
            /* Keep this display-list block on one line for IDO allocation. */
            { FxGfx *_g = (FxGfx *)((*dList)++); _g->w0 = FX_SHIFTL(0xFA, 24, 8); _g->w1 = ((FX_SHIFTL(0xFF, 24, 8) | FX_SHIFTL(0xFF, 16, 8) | FX_SHIFTL(0xFF, 8, 8)) ^ 0) | FX_SHIFTL(alpha, 0, 8); };
            FX_SET_ENV((*dList)++, 0xFF, 0xFF, 0xFF, 0);
        }

        if (cone->texture.value != 0) {
            hasTexture = 1;
        } else {
            hasTexture = 0;
        }
        if (!cone->segmentCount) {
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          17, 0);
            func_800349A4(dList, cone->alternateTexture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000, 16,
                       hasTexture);
            func_800349A4(dList, cone->texture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000200, 8,
                       hasTexture);
            func_800349A4(dList, cone->alternateTexture.value, flags, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000100, 16,
                       hasTexture);
        } else {
            func_800349A4(dList, cone->texture.value, flags, 0);
            FX_VERTEX_JFG((*dList)++,
                          cone->addresses[cone->addressIndex] + 0x80000000,
                          cone->mode, 0);
            FX_POLYGON((*dList)++, cone->vertices + 0x80000000,
                       cone->segmentCount, hasTexture);
        }

        FX_PIPE_SYNC((*dList)++);
        FX_SET_PRIM((*dList)++, 0xFF, 0xFF, 0xFF, 0xFF);
        FX_SET_ENV((*dList)++, 0xFF, 0xFF, 0xFF, 0);
    }
}
/* Transforms `count` cone points by two angles and writes the packed
 * position/colour records the display list draws.
 *
 * The two cursors are the *parameters*: IDO promotes both stack homes into
 * registers for the loop and writes them back at the loop exit, which is what
 * an earlier candidate's `volatile` pointers and explicit writeback were
 * imitating.
 *
 * The last six words were one floating-point colour swap -- the target holds
 * the loaded z in f14 and x in f2 -- and it is a web-count fact, not an
 * ordering one. With the three loads in x, y, z order and four live FP webs
 * in the loop, x is pinned to f14 and only y and z can trade f2 and f12; that
 * is flat over all six load orders, all six store orders that keep the
 * schedule, both spellings of the rotated-z subexpression, inlining it, and
 * 6,000 random permutations of the eight declarations (FP webs turn out to
 * ignore declaration order exactly as integer webs do). The rotated *y*
 * component is its own named value, like the rotated z beside it. That fifth
 * web costs no instruction -- IDO coalesces it -- and it is what moves x to
 * f2 and z to f14.
 *
 * PROVENANCE: JFG's assembly-only func_8006A224 confirms the same
 * cone-point-transform role, frame class, four trigonometric calls and packed
 * output loop; no donor C exists and none was adopted. */
typedef struct FxTransformInput {
    f32 x;
    f32 y;
    f32 z;
} FxTransformInput;

typedef struct FxTransformOutput {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    s8 alpha;
} FxTransformOutput;

void func_80048080(s32 count, s16 x, s16 y, s16 z, s16 angle0, s16 angle1,
                   FxConePoint *input, u8 *output, s32 alpha) {
    f32 cos1;
    f32 sin1;
    f32 cos0;
    f32 sin0;
    f32 inputZ;
    f32 inputY;
    f32 inputX;
    f32 rotatedZ;
    f32 rotatedY;

    cos1 = func_8002A8C0(angle1);
    sin1 = func_8002A8BC(angle1);
    cos0 = func_8002A8C0(angle0);
    sin0 = func_8002A8BC(angle0);
    while (count--) {
        inputX = input->x;
        inputY = input->y;
        inputZ = input->z;
        input++;
        output[6] = 0xFF;
        output[7] = 0xFF;
        output[8] = 0xFF;
        output[9] = alpha;
        output += 10;
        rotatedZ = (inputZ * sin1) + (inputY * cos1);
        rotatedY = (inputY * sin1) - (inputZ * cos1);
        ((s16 *)output)[-5] = (s16)((s32)((inputX * sin0) + (rotatedZ * cos0)) + x);
        ((s16 *)output)[-4] = (s16)((s32) rotatedY + y);
        ((s16 *)output)[-3] = (s16)((s32)((rotatedZ * sin0) - (inputX * cos0)) + z);
    }
}
#ifdef NON_MATCHING
typedef struct FxWakeAllocation {
    u8 flags;
    u8 segmentCount;
    u8 state;
    u8 textureIndex;
    f32 value4;
    s16 value8;
    s16 textureStep;
    f32 valueC;
    u8 *vertices;
    u8 *samples;
    u8 *sampleBuffers[4];
    u8 *vertexBuffers[2];
    FxConeTextureInfo *linked;
    s16 value34;
    s16 value36;
    u8 value38;
    u8 value39;
    u8 value3A;
    u8 value3B;
    s32 value3C;
} FxWakeAllocation;

/* Natural-source pass (2026-10-07, lane a-front): -32 bytes -> -8.
 *  - the triangle count is its own variable (segmentCount * 2 again): the
 *    target's second fill loop bounds on a separate copy;
 *  - the flags store is an if/else, the alpha default stays a plain
 *    default-then-override, and the resource id is a full word;
 *  - one pointer local walks both areas; no size/vertexBytes locals (frame
 *    0x98 -> 0x90 and the target's home ladder for groupCount/alpha/triCount).
 * Left: two dead copies the target makes before the first call (segment and
 * group counts into v0/v1) and frameCount's pre-call piece in s0; see shard. */
/* PROVENANCE: Mickey's own target accesses and caller ABI supply this reconstruction; JFG supplies only the published role/name. */
Wake *wakeAllocate(s32 wakeType, f32 wakeValue88, f32 wakeValue80,
                   f32 wakeValue84, s32 wakeValue8C, f32 wakeValue8E) {
    s32 i;
    FxWakeAllocation *wake;
    u8 *cursor;
    s32 frameCount;
    s32 segmentCount;
    s32 segmentBytes;
    s32 sampleBytes;
    s32 textureBytes;
    s32 groupCount;
    s32 alpha;
    s32 j;
    s32 triCount;
    s32 bufferCount;

    frameCount = (s32) (wakeValue88 * 60.0f);
    segmentCount = (frameCount + 5) >> 1;
    groupCount = segmentCount * 2;
    triCount = segmentCount * 2;
    alpha = 2;
    bufferCount = 2;
    if (wakeType == 0) {
        alpha = 4;
    }
    segmentBytes = groupCount * 0xA;
    sampleBytes = segmentCount * 0x10;
    textureBytes = triCount * 0x10;
    wake = func_8002B314(sampleBytes + (segmentCount * 0x14) + (alpha * segmentBytes) +
                         (textureBytes * 2) + 0x40, 0x87);
    if (wake != NULL) {
        cursor = (u8 *) wake + 0x40;
        for (i = 0; i < bufferCount; i++) {
            wake->vertexBuffers[i] = cursor + (i * textureBytes);
        }
        cursor = wake->vertexBuffers[1] + textureBytes;
        wake->vertices = cursor;
        wake->sampleBuffers[2] = NULL;
        wake->sampleBuffers[3] = NULL;
        cursor += sampleBytes;
        wake->samples = cursor;
        cursor += (segmentCount * 0x14);
        for (i = 0; i < alpha; i++) {
            wake->sampleBuffers[i] = cursor + (i * segmentBytes);
        }
        wake->linked = func_80034448(wakeValue8C);
        if (wake->linked != NULL) {
            if (wakeType != 0) {
                wake->flags = 1;
            } else {
                wake->flags = 0;
            }
            wake->state = 0;
            wake->segmentCount = segmentCount;
            wake->value8 = 0;
            wake->value3C = 0;
            wake->value4 = wakeValue80;
            wake->textureIndex = (s8) frameCount;
            wake->textureStep =
                (s16) (((wake->linked->height - 1) << 8) /
                       (u8) frameCount);
            wake->valueC =
                (wakeValue84 - wakeValue80) / (u8) frameCount;
            for (i = 0; i < alpha; i++) {
                for (j = 0; j < groupCount; j++) {
                    wake->sampleBuffers[i][(j * 0xA) + 6] = 0xFF;
                    wake->sampleBuffers[i][(j * 0xA) + 7] = 0xFF;
                    wake->sampleBuffers[i][(j * 0xA) + 8] = 0xFF;
                }
            }
            for (i = 0; i < bufferCount; i++) {
                for (j = 0; j < triCount; j++) {
                    wake->vertexBuffers[i][j * 0x10] = 0x40;
                }
            }
            wake->value34 = 0;
            wake->value36 =
                (s16) ((wakeValue8E * 256.0f) / 60.0f);
            wake->value38 = 0;
            wake->value39 = 0;
            wake->value3A = 0;
            wake->value3B = 0;
        } else {
            mmFree(wake);
            wake = NULL;
        }
    }
    return (Wake *) wake;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeAllocate.s")
#endif
/* Builds the ripple's two display frames in the caller's buffer and allocates
 * its wake.
 *
 * Two source facts closed the last twenty words, both the same kind of error:
 * writing down something the compiler produces.
 *
 * The two texture extents are `s32`, not `s16`. As `s16` locals they are
 * numbered ahead of the constants 1 and 2 the frame stores materialise, and
 * the whole ugen temp ring rotates behind them -- sixteen words, none of them
 * reachable by moving the two definitions (all four positions among the frame
 * stores were measured, and inlining them costs twelve instructions).
 *
 * The white-fill loop indexes the record from `i`; it does not carry a cursor.
 * With `p += 0x28` in source the loop's four preheader values come out as two
 * ugen copies and then two hoisted constants, and no ordering or grouping of
 * the three initialisations moves them. Written as `i * 0x28` the cursor
 * becomes uopt's own induction variable, its initialisation is inserted after
 * the constants instead, and the preheader is the target's `li a1, 2` /
 * `li v1, 255` / `move a0, zero` / `move v0, s0`.
 *
 * PROVENANCE: Mickey field layouts and control flow are reconstructed from the
 * target's accesses; JFG's wakeSetupRipple is assembly-only and supplies only
 * TU and name context. */
typedef struct FxRippleSource {
    u8 pad00[0x73];
    s8 wakeType;
    u8 pad74[4];
    f32 textureScale;
    s16 textureId;
    s16 wakeValue7E;
    f32 wakeValue80;
    f32 wakeValue84;
    f32 wakeValue88;
    s16 wakeValue8C;
    s16 wakeValue8E;
} FxRippleSource;

typedef struct FxRippleSetup {
    u8 pad00[0xC];
    f32 valueC;
    u8 pad10[4];
    f32 value14;
    u8 pad18[0x28];
    FxRippleSource *source;
    u8 pad44[0x10];
    u8 *output;
} FxRippleSetup;

typedef struct FxRippleFrame {
    u8 value0;
    u8 value1;
    u8 value2;
    u8 value3;
    s16 value4;
    s16 value6;
    s16 value8;
    s16 valueA;
    s16 valueC;
    s16 valueE;
    u8 value10;
    u8 value11;
    u8 value12;
    u8 value13;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 value1A;
    s16 value1C;
    s16 value1E;
    u8 pad20[8];
} FxRippleFrame;

typedef struct FxRippleOutput {
    FxRippleFrame frames[2];
    u8 pad50[0x20];
    FxConeTextureInfo *texture;
    u8 value74;
    u8 value75;
    s16 value76;
    s16 value78;
    s16 value7A;
    f32 value7C;
    f32 value80;
    Wake *wake;
} FxRippleOutput;

extern void func_8001357C(f32 valueC, f32 value14, void *output,
                          s32 value, s32 zero);
extern Wake *wakeAllocate(s32 wakeType, f32 wakeValue88, f32 wakeValue80,
                          f32 wakeValue84, s32 wakeValue8C,
                          f32 wakeValue8E);

s32 func_80048760(void *arg0, s32 arg1) {
    u8 pad[16];
    s32 size;
    s32 i;
    FxRippleOutput *output;
    FxRippleSource *source;
    FxConeTextureInfo *texture;
    s32 extentX;
    s32 extentY;
    u8 fill;
    FxRippleFrame *frame;

    if ((arg1 & 7) != 0) {
        size = 8 - (arg1 & 7);
        arg1 += size;
    } else {
        size = 0;
    }
    output = (FxRippleOutput *) arg1;
    size += (s32) align4((u8 *) 0x88);
    source = ((FxRippleSetup *) arg0)->source;
    ((FxRippleSetup *) arg0)->output = (u8 *) output;
    output->texture = func_80034448(source->textureId);
    if (output->texture == 0) {
        return 0;
    }
    texture = output->texture;
    extentX = (texture->width - 1) << 5;
    extentY = (texture->height - 1) << 5;
    frame = (FxRippleFrame *) output;
    frame->value0 = 0x40;
    frame->value1 = 0;
    frame->value4 = extentX;
    frame->value6 = 0;
    frame->value2 = 1;
    frame->value8 = 0;
    frame->valueA = 0;
    frame->value3 = 2;
    frame->valueC = extentX;
    frame->valueE = extentY;
    frame->value10 = 0x40;
    frame->value11 = 1;
    frame->value14 = 0;
    frame->value16 = 0;
    frame->value12 = 2;
    frame->value18 = extentX;
    frame->value1A = extentY;
    frame->value13 = 3;
    frame->value1C = 0;
    frame->value1E = extentY;

    fill = 0xFF;
    for (i = 0; i != 2; i++) {
        ((u8 *) output)[(i * 0x28) + 0x30] = fill;
        ((u8 *) output)[(i * 0x28) + 0x31] = fill;
        ((u8 *) output)[(i * 0x28) + 0x32] = fill;
        ((u8 *) output)[(i * 0x28) + 0x33] = fill;
        ((u8 *) output)[(i * 0x28) + 0x3A] = fill;
        ((u8 *) output)[(i * 0x28) + 0x3B] = fill;
        ((u8 *) output)[(i * 0x28) + 0x3C] = fill;
        ((u8 *) output)[(i * 0x28) + 0x3D] = fill;
        ((u8 *) output)[(i * 0x28) + 0x44] = fill;
        ((u8 *) output)[(i * 0x28) + 0x45] = fill;
        ((u8 *) output)[(i * 0x28) + 0x46] = fill;
        ((u8 *) output)[(i * 0x28) + 0x47] = fill;
        ((u8 *) output)[(i * 0x28) + 0x26] = fill;
        ((u8 *) output)[(i * 0x28) + 0x27] = fill;
        ((u8 *) output)[(i * 0x28) + 0x28] = fill;
        ((u8 *) output)[(i * 0x28) + 0x29] = fill;
    }

    output->value74 = 0;
    output->value75 = 0;
    output->value76 = 0;
    output->value78 = 0;
    output->value7A = source->wakeValue7E;
    output->value7C = source->textureScale;
    func_8001357C(((FxRippleSetup *) arg0)->valueC,
                  ((FxRippleSetup *) arg0)->value14,
                  (u8 *) output + 0x80,
                  0x10000, 0);
    output->wake = 0;
    if (source->wakeType != -1) {
        output->wake = wakeAllocate(source->wakeType, source->wakeValue88,
                                    source->wakeValue80, source->wakeValue84,
                                    source->wakeValue8C,
                                    (f32) source->wakeValue8E);
    }
    return size;
}
void wakeFree(Wake *wake) {
    void *linked = wake->linked;

    if (linked != 0) {
        func_800347A0(linked);
    }
    mmFree(wake);
}
void func_80048980(WakeRipple *ripple) {
    void *linked = ripple->linked;

    if (linked != 0) {
        func_800347A0(linked);
    }
    if (ripple->wake != 0) {
        wakeFree(ripple->wake);
    }
}
#ifdef NON_MATCHING
/* B3-fx (2026-09-23): size delta 0 and frame 0x90, 257 masked. Two
 * semantic fixes against the target (a separate polygon counter stored at
 * +0xE, and stripIndex advancing by 2 per sample with polygon[0x12] set),
 * index as s32, a while (count--) scan, and the 0x20 buffer read spelled as
 * a subscript so uopt keeps it apart from the 0x18/0x28 address (the target
 * computes that address twice). The rest is p1 colour order: wake and
 * secondaryVertices, index and stripIndex swap; see the handoff shard. */
void wakeUpdate(Wake *wake, f32 arg1, f32 arg2, f32 arg3, s16 angle, s32 arg5) {
    u8 *sample;
    s32 index;
    s32 count;
    s32 outputOffset;
    s32 mark;
    s32 stripIndex;
    s32 polyCount;
    s32 outputCount;
    s32 value;
    s32 polygonOffset;
    s32 vertexCount;
    u8 *vertices;
    u8 *secondaryVertices;
    u8 *polygon;
    f32 sine;
    f32 cosine;

    vertexCount = wake->value39;
    count = wake->value3B;
    index = vertexCount;
    while (count--) {
        sample = (u8 *) wake->samples + (index * 0x14);
        index++;
        if (index >= wake->segmentCount) {
            index = 0;
        }
        if (arg5 >= sample[0]) {
            wake->value39 = index;
            wake->value3B--;
        } else {
            count = 0;
        }
    }
    vertexCount = wake->value39;
    mark = 0;
    stripIndex = 0;
    polyCount = 0;
    outputCount = 0;
    sample = (u8 *) wake->samples + (vertexCount * 0x14);
    sample[1] |= 0x80;
    if (wake->flags & 2) {
        if (wake->value8 == 0) {
            mark = 1;
        }
        if (wake->value8 < 0xBF) {
            wake->value8 += 0x40;
        } else {
            wake->value8 = 0xFF;
        }
        wake->value3C += arg5 * 0x10;
        if (wake->value3C >= 0x100) {
            wake->value3C = 0xFF;
        }
    } else {
        if (wake->value8 >= 0x41) {
            wake->value8 -= 0x40;
        } else {
            wake->value8 = 0;
        }
        wake->value3C -= arg5 * 0x10;
        if (wake->value3C < 0) {
            wake->value3C = 0;
        }
    }
    if ((wake->value8 != 0) && (wake->value3B < wake->segmentCount)) {
        sample = (u8 *) wake->samples + (wake->value3A * 0x14);
        sample[0] = wake->textureIndex;
        value = wake->value8 >> 1;
        sample[1] = value;
        if (mark != 0) {
            sample[1] = value | 0x80;
        }
        *(s16 *) (sample + 2) = angle;
        *(s16 *) (sample + 4) = (*(u16 *) ((u8 *) wake->linked + 8) - 1) << 8;
        *(s16 *) (sample + 6) = arg2;
        *(f32 *) (sample + 8) = arg1;
        *(f32 *) (sample + 0xC) = arg3;
        *(f32 *) (sample + 0x10) = wake->value4;
        wake->value3A++;
        if (wake->value3A >= wake->segmentCount) {
            wake->value3A = 0;
        }
        wake->value3B++;
    }
    wake->state = 1 - wake->state;
    wake->value38 = 0;
    if (wake->value3B != 0) {
        vertices = *(u8 **) ((u8 *) wake + 0x18 + (wake->state * 4));
        secondaryVertices = ((u8 **) ((u8 *) wake + 0x20))[wake->state];
        polygon = *(u8 **) ((u8 *) wake + 0x28 + (wake->state * 4));
        polygonOffset = (*(u16 *) ((u8 *) wake->linked + 6) - 1) << 5;
        outputOffset = 0;
        index = wake->value39;
        if (index != wake->value3A) {
            do {
                value = index * 5;
                sample = (u8 *) wake->samples + (value * 4);
                if (++index >= wake->segmentCount) {
                    index = 0;
                }
                if (sample[1] & 0x80) {
                    stripIndex = 0;
                    if (outputOffset != 0) {
                        *(s16 *) ((u8 *) wake->vertices +
                                  (wake->value38 * 0x10) + 0xC) = outputCount;
                        *(s16 *) ((u8 *) wake->vertices +
                                  (wake->value38 * 0x10) + 0xE) = polyCount;
                        wake->value38++;
                    }
                    outputOffset = 1;
                    polyCount = 0;
                    *(u8 **) ((u8 *) wake->vertices +
                              (wake->value38 * 0x10) + 0x0) = vertices;
                    outputCount = 0;
                    *(u8 **) ((u8 *) wake->vertices +
                              (wake->value38 * 0x10) + 0x4) = secondaryVertices;
                    *(u8 **) ((u8 *) wake->vertices +
                              (wake->value38 * 0x10) + 0x8) = polygon;
                }
                sample[0] -= arg5;
                *(f32 *) (sample + 0x10) += wake->valueC * (f32) arg5;
                *(s16 *) (sample + 4) -= *(s16 *) ((u8 *) wake + 0xA) * arg5;
                sine = func_8002A8C0(*(s16 *) (sample + 2));
                sine *= *(f32 *) (sample + 0x10);
                cosine = func_8002A8BC(*(s16 *) (sample + 2));
                cosine *= *(f32 *) (sample + 0x10);
                value = ((sample[1] & 0x7F) * wake->value3C) >> 7;
                vertices += 0xA;
                *(s16 *) (vertices - 0xA) = *(f32 *) (sample + 8) - cosine;
                *(s16 *) (vertices - 8) = *(s16 *) (sample + 6);
                *(s8 *) (vertices - 1) = value;
                *(s16 *) (vertices - 6) = *(f32 *) (sample + 0xC) + sine;
                if (secondaryVertices == NULL) {
                    *(s16 *) (vertices + 0) = *(f32 *) (sample + 8) + cosine;
                    *(s16 *) (vertices + 4) = *(f32 *) (sample + 0xC) - sine;
                } else {
                    secondaryVertices += 0x14;
                    *(s16 *) (vertices + 0) = *(f32 *) (sample + 8);
                    *(s16 *) (vertices + 4) = *(f32 *) (sample + 0xC);
                    *(s16 *) (secondaryVertices - 0x14) = *(f32 *) (sample + 8) + cosine;
                    *(s16 *) (secondaryVertices - 0x12) = *(s16 *) (sample + 6);
                    *(s8 *) (secondaryVertices - 0xB) = value;
                    *(s16 *) (secondaryVertices - 0x10) = *(f32 *) (sample + 0xC) - sine;
                    *(s16 *) (secondaryVertices - 0xA) = *(s16 *) (vertices + 0);
                    *(s16 *) (secondaryVertices - 8) = *(s16 *) (sample + 6);
                    *(s8 *) (secondaryVertices - 1) = value;
                    *(s16 *) (secondaryVertices - 6) = *(s16 *) (vertices + 4);
                }
                *(s8 *) (vertices + 9) = value;
                vertices += 0xA;
                *(s16 *) (vertices - 8) = *(s16 *) (sample + 6);
                outputCount += 2;
                vertexCount = *(s16 *) (sample + 4) >> 3;
                if (stripIndex != 0) {
                    polygon[3] = stripIndex;
                    *(s16 *) (polygon + 0xC) = 0;
                    *(s16 *) (polygon + 0xE) = vertexCount;
                    polygon[0x12] = stripIndex;
                    *(s16 *) (polygon + 0x18) = 0;
                    *(s16 *) (polygon + 0x1A) = vertexCount;
                    polygon[0x13] = stripIndex + 1;
                    *(s16 *) (polygon + 0x1C) = polygonOffset;
                    *(s16 *) (polygon + 0x1E) = vertexCount;
                    polygon += 0x20;
                    polyCount += 2;
                    if ((stripIndex + 2) >= 0x11) {
                        stripIndex = 0;
                    }
                }
                polygon[1] = stripIndex;
                *(s16 *) (polygon + 4) = 0;
                *(s16 *) (polygon + 6) = vertexCount;
                polygon[2] = stripIndex + 1;
                *(s16 *) (polygon + 8) = polygonOffset;
                *(s16 *) (polygon + 0xA) = vertexCount;
                polygon[0x11] = stripIndex + 1;
                *(s16 *) (polygon + 0x14) = polygonOffset;
                *(s16 *) (polygon + 0x16) = vertexCount;
                stripIndex += 2;
            } while (index != wake->value3A);
        }
        *(s16 *) ((u8 *) wake->vertices + (wake->value38 * 0x10) + 0xC) = outputCount;
        *(s16 *) ((u8 *) wake->vertices + (wake->value38 * 0x10) + 0xE) = polyCount;
        wake->value38++;
        wake->value34 += wake->value36 * arg5;
        while (wake->value34 >= *(u16 *) ((u8 *) wake->linked + 0x10)) {
            wake->value34 -= *(u16 *) ((u8 *) wake->linked + 0x10);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/wakeUpdate.s")
#endif
/* The height carrier is `v1` because `mode` dies before it, not because of any
 * spelling of the carrier itself. uopt colours pool webs by descending
 * `references / bucket(references + spanning statements)`, and `mode` outranks
 * `height`; whichever is coloured first takes `v1` and the other takes `a0`.
 * Reading the field back for the vertex base -- `ripple->mode`, which the store
 * on the line above has just written, so uopt forwards it and it costs no
 * instruction -- ends `mode`'s live range at that store instead of at the
 * vertex multiply. `mode` then no longer interferes with the truncated height,
 * and `height` takes `v1`. Lever 45 used for its live range rather than for a
 * register-to-register copy or a ring slot.
 *
 * The height read must also stand before the vertex base: ugen schedules the
 * `l.s`/`trunc.w.s`/`mfc1` ahead of the multiply either way, and with the two
 * statements in the other order the conversion lands inside `mode`'s range
 * again (five words, at +0xF8 onward rather than +0x108). Keeping the local
 * `mode` in the multiply is the previous five-word state.
 *
 * The lever that got here first was the vertex writeback: the twelve stores are
 * all addressed from the *post*-increment `vertex`, including the first one,
 * which the earlier draft wrote as `vertex[+0x22]` before `vertex += 0x3E` and
 * which IDO schedules ahead of the increment anyway. That took the residual
 * 69 -> 5. */
/* PROVENANCE: Jet Force Gemini public decomp src/fx.c at efd5abb1c79636e297b831f7c2d5bf47eac39c0c
 * still leaves wakeUpdateRipple assembly-only; src/fx.h adds no ripple source
 * context. JFG supplies only the role/name; this retained body uses Mickey's
 * target offsets and calls. No new donor body was available or adopted. */
void func_80049000(FxWakeUpdateOwner *owner, s32 delta) {
    FxWakeTexture *texture;
    u8 mode;
    FxWakeRippleData *ripple;
    u8 *vertex;
    s16 step;
    s32 height;
    s16 angle;

    ripple = owner->ripple;
    if (ripple != 0) {
        if (ripple->active != 0) {
            ripple->fade = (s16) (ripple->fade + 0x20);
            if (ripple->fade >= 0x100) {
                ripple->fade = 0xFF;
            }
        } else {
            ripple->fade = (s16) (ripple->fade - 0x20);
            if (ripple->fade < 0) {
                ripple->fade = 0;
            }
        }
        step = ripple->angleStep;
        if (step != 0) {
            texture = ripple->texture;
            if (texture != 0) {
                ripple->angle = (s16) (ripple->angle + (step * delta));
                if (ripple->angle >= (s32) texture->length) {
                    do {
                        ripple->angle = (s16) (ripple->angle - (s32) texture->length);
                    } while (ripple->angle >= (s32) texture->length);
                }
            }
        }
        if (ripple->fade != 0) {
            mode = 1 - ripple->mode;
            ripple->mode = mode;
            height = (s32) ripple->value80;
            vertex = (u8 *) ripple + (ripple->mode * 0x28);
            vertex += 0x3E;
            *(s16 *) (vertex - 0x1E) = (s16) (s32) (owner->valueC + ripple->value7C);
            *(s16 *) (vertex - 0x1C) = (s16) height;
            *(s16 *) (vertex - 0x1A) = (s16) (s32) (owner->value14 - ripple->value7C);
            *(s16 *) (vertex - 0x14) = (s16) (s32) (owner->valueC - ripple->value7C);
            *(s16 *) (vertex - 0x12) = (s16) height;
            *(s16 *) (vertex - 0x10) = (s16) (s32) (owner->value14 - ripple->value7C);
            *(s16 *) (vertex - 0xA) = (s16) (s32) (owner->valueC + ripple->value7C);
            *(s16 *) (vertex - 8) = (s16) height;
            *(s16 *) (vertex - 6) = (s16) (s32) (owner->value14 + ripple->value7C);
            *(s16 *) vertex = (s16) (s32) (owner->valueC - ripple->value7C);
            *(s16 *) (vertex + 2) = (s16) height;
            *(s16 *) (vertex + 4) = (s16) (s32) (owner->value14 + ripple->value7C);
        }
        angle = Arctanf(owner->value1C, owner->value24);
        if (ripple->update != 0) {
            wakeUpdate(ripple->update, owner->valueC, ripple->value80,
                       owner->value14, angle, delta);
        }
    }
}
/* PROVENANCE: JFG's wakeDraw role supplies the display-list idiom; this body is reconstructed from Mickey's target offsets and FxGfx type.
 * Matched 2026-09-23 (Track B, lane B-fx). What closed it, in order:
 *  - frame: the target's 0x88 frame is fourteen declared slots between alpha
 *    and outerOffset, and outerOffset declared last (its spill home is +0x44).
 *    Unused declarations keep their homes here, so the list is the frame.
 *  - size: shiftedX and shiftedZ written inline instead of through carriers
 *    (L160); their carriers took a2/t0 and pushed the outer index into a
 *    register the target spills.
 *  - colour order: x += xStep after the first polygon, the outer bound read
 *    from wake->value38 directly (no outerLimit carrier), both index updates on
 *    one line (as1 store order), and the second vertex's packet pointer taken
 *    before shiftedY so that web numbers after it.
 *  - tail: FX_PIPE_SYNC + FX_SET_ENV instead of hand-written command words. */
void wakeDraw(Wake *wake, FxGfx **dlist) {
    s32 outerIndex;
    s32 alpha;
    s32 remaining;
    s32 chunk;
    s32 shiftedY;
    s32 xStep;
    s32 shiftedX;
    s32 shiftedZ;
    s32 zStep;
    s32 x;
    s32 y;
    s32 z;
    u8 outerLimit;
    FxGfx *cmd;
    FxWakeSegment *segment;
    s32 unusedSlot;
    s32 outerOffset;

    if ((s32) wake->value38 > 0) {
        func_800349A4(dlist, (s32) wake->linked, 0x1F,
                      (s32) wake->value34 << 8);
        if ((((FxWakeLinked *) wake->linked)->flags & 0x40) != 0) {
            alpha = wake->value34 & 0xFF;
        } else {
            alpha = 0xFF;
        }
        FX_SET_ENV((*dlist)++, alpha, alpha, alpha, alpha);
        FX_SET_PRIM((*dlist)++, 0xFF, 0xFF, 0xFF, 0xFF);
        outerIndex = 0;
        if (wake->value38 > 0) {
            outerOffset = 0;
            do {
                segment = (FxWakeSegment *) ((u8 *) wake->vertices + outerOffset);
                remaining = segment->length;
                x = segment->x;
                y = segment->y;
                z = segment->z;
                if (remaining != 0) {
                    do {
                        if (remaining >= 0x11) {
                            remaining -= 0x10;
                            chunk = 0x10;
                        } else {
                            chunk = remaining;
                            remaining = 0;
                        }
                        FX_VERTEX_JFG((*dlist)++, x + 0x80000000, chunk + 2, 0);
                        FX_POLYGON((*dlist)++, z + 0x80000000, chunk, 1);
                        xStep = chunk * 0xA;
                        x += xStep;
                        zStep = chunk * 0x10;
                        if (y != 0) {
                            {
                                FxGfx *_g = (*dlist)++;
                                shiftedY = y + 0x80000000;
                                _g->w0 = FX_SHIFTL(4, 24, 8) |
                                         FX_SHIFTL(((chunk + 2) << 3) | ((u32) shiftedY & 6) | 0, 16, 8) |
                                         FX_SHIFTL(((chunk + 2) << 3) + ((chunk + 2) << 1) + 8, 0, 16);
                                _g->w1 = (u32) shiftedY;
                            }
                            y += xStep;
                            FX_POLYGON((*dlist)++, z + 0x80000000, chunk, 1);
                        }
                        z += zStep;
                    } while (remaining != 0);
                }
                outerIndex++; outerOffset += 0x10;
            } while (outerIndex < wake->value38);
        }
        if (alpha != 0xFF) {
            FX_PIPE_SYNC((*dlist)++);
            FX_SET_ENV((*dlist)++, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }
}
/* Workbench: schedule-mismatch, 2/138 differing words, first mismatch +0x60. */
/* Exact 138-word geometry/frame -0x20; one D_7D310 LO16 schedule slot remains. */
/* All five relocation identities agree; the LO16 offset is nonexact. */
void func_80049518(WakeRipple *ripple, FxGfx **dlist)
{
  s32 alpha;
  void *linked;
  FxGfx *cmd;
  if (ripple != ((void *) 0))
  {
 do { if ((linked = ripple->linked) != ((void *) 0)) { if (ripple->value76 != 0) { func_800349A4(dlist, (s32) linked, 0xF, ((s32) ripple->value78) << 8); { FxGfx *textureCmd = (*dlist)++; textureCmd->w0 = 0x07020010; textureCmd->w1 = (u32) D_7D310; } if (ripple->wake != ((void *) 0)) { alpha = 0xFF - (((s32) ripple->wake->value3C) >> 1); } else { alpha = 0xFF; } cmd = *dlist; *dlist = cmd + 1; cmd->w0 = 0xFA000000; cmd->w1 = (((alpha * ((s32) ripple->value76)) >> 8) & 0xFF) | (~0xFF); if ((((FxWakeLinked *) ripple->linked)->flags & 0x40) != 0) { alpha = ripple->value78 & 0xFF; } else { alpha = 0xFF; } { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (u32) ((((u32) 0xFB) & ((1U << 8) - 1U)) << 24); _g->w1 = ((((u32) ((((u32) alpha) & ((1U << 8) - 1U)) << 24)) | ((u32) ((((u32) alpha) & ((1U << 8) - 1U)) << 16))) | ((u32) ((((u32) alpha) & ((1U << 8) - 1U)) << 8))) | ((u32) ((((u32) alpha) & ((1U << 8) - 1U)) << 0)); } ; { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (((u32) ((((u32) 4) & ((1U << 8) - 1U)) << 24)) | ((u32) ((((u32) (((4 << 3) | (((u32) ((((u8 *) ripple) + (ripple->value74 * 0x28)) + 0x80000020)) & 6)) | 0)) & ((1U << 8) - 1U)) << 16))) | ((u32) ((((u32) (((4 << 3) + (4 << 1)) + 8)) & ((1U << 16) - 1U)) << 0)); _g->w1 = (u32) ((((u8 *) ripple) + (ripple->value74 * 0x28)) + 0x80000020); } ; { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (((u32) ((((u32) (((2 - 1) << 4) | 1)) & ((1U << 8) - 1U)) << 16)) | ((u32) ((((u32) 5) & ((1U << 8) - 1U)) << 24))) | ((u32) ((((u32) (2 * 16)) & ((1U << 16) - 1U)) << 0)); _g->w1 = (u32) (((u8 *) ripple) + 0x80000000); } ; func_80034920(dlist); } if (ripple->wake != ((void *) 0)) { wakeDraw(ripple->wake, dlist); } { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (u32) ((((u32) 0xE7) & ((1U << 8) - 1U)) << 24); _g->w1 = 0; } ; { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (u32) ((((u32) 0xFA) & ((1U << 8) - 1U)) << 24); _g->w1 = ((((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 24)) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 16))) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 8))) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 0)); } ; { FxGfx *_g = (FxGfx *) ((*dlist)++); _g->w0 = (u32) ((((u32) 0xFB) & ((1U << 8) - 1U)) << 24); _g->w1 = ((((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 24)) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 16))) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 8))) | ((u32) ((((u32) 0xFF) & ((1U << 8) - 1U)) << 0)); } ; } } while (0);
  }
}
void fxInit(void) {
    FxRecord *record;
    s32 i;

    record = D_800D5F58;
    i = 5;
    while (i--) {
        record->state = 0;
        record->flags = 0;
        record->status = 0;
        record++;
    }
    D_800D5F50 = 0;
    func_8004ACC4();
}
/* Mickey-derived body; JFG's corresponding fx.c routine is assembly-only. */
void func_8004978C(s32 index, s32 mask, s32 enable) {
    s32 unused[2];
    FxRecord *record;
    s32 count = 0;
    s32 andMask;
    s32 orMask;

    if (index == -1) {
        record = D_800D5F58;
        count = 5;
    } else if (index >= 0 && index < 5) {
        record = &D_800D5F58[index];
        count = 1;
    }
    if (count != 0) {
        andMask = ~mask;
        if (enable != 0) {
            andMask = -1;
            orMask = mask;
        } else {
            orMask = 0;
        }
        while (count--) {
            record->flags = (record->flags & andMask) | orMask;
            record++;
        }
    }
}
s32 func_80049828(s32 index, s32 mask) {
    if (index >= 0 && index < 5 && (D_800D5F5A[index].value & mask) != 0) {
        return 1;
    }
    return 0;
}
s32 func_80049864(s32 index) {
    if (index >= 0 && index < 5 && D_800D5F59[index].value != 0) {
        return 1;
    }
    return 0;
}
s32 func_8004989C(s32 index) {
    FxRecord *record;
    s32 color;

    if (index < 0 || index >= 5) {
        return 0;
    }
    record = &D_800D5F58[index];
    color = ((record->red & 0xF8) << 8) |
            ((record->green & 0xF8) << 3) |
            ((record->blue & 0xF8) >> 2);
    color |= color << 16;
    return color;
}
extern s32 camGetMode(void);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);

void func_800498FC(s32 index, f32 value16, f32 value18, s32 red, s32 green,
                   s32 blue, s32 flags) {
    FxRecord *record;

    if (index < 0 || index >= 5) {
        return;
    }
    record = &D_800D5F58[index];
    if ((record->flags & 2) != 0 ||
        (record->state != 0 && (record->flags & 1) != 0)) {
        return;
    }
    if (index == 4) {
        func_80021FB0(0, 0, &record->value4, &record->value8,
                      (u32 *)&record->valueC, (u32 *)&record->value10);
    } else {
        func_80021FB0(camGetMode(), index, &record->value4, &record->value8,
                      (u32 *)&record->valueC, (u32 *)&record->value10);
    }
    record->flags = 0;
    record->value14 = 0;
    record->value16 = (s16)(value16 * 60.0f);
    record->value18 = (s16)(value18 * 60.0f);
    record->red = red;
    record->green = green;
    record->blue = blue;
    record->value1D = flags & 0xFF3F;
    record->value1E = flags & 0x80;
    record->value1F = flags & 0x40;
    if (record->value1E != 0) {
        if (record->value1F != 0) {
            record->state = 3;
        } else {
            record->state = 2;
        }
        record->status = 0xFF;
        return;
    }
    record->state = 1;
    record->status = 0;
}
void func_80049A8C(s32 index) {
    s32 count = 0;
    FxRecord *record;

    if (index == -1) {
        count = 5;
        record = D_800D5F58;
    } else if (index >= 0 && index < 5) {
        count = 1;
        record = &D_800D5F58[index];
    }
    while (count--) {
        record->state = 0;
        record->flags &= ~5;
        record->status = 0;
        record++;
    }
}
/* Lane a-front (2026-10-07): 181 masked at delta 0 -> 105 at +4.
 *  - the state byte is read into a local at the top of each record and the
 *    switch re-reads it into that local: the ten hoisted constants then
 *    become constrained webs (22 interferences) and globalcolor gives them
 *    v1..t5 ahead of the variables, which take s0..s4 as shipped;
 *  - cases 1 and 3 read the duration after the counter update.
 * Left: the switch-value copy and case 0's block, and case 2's duration
 * colour (s4 in the target); see the shard. */
/* PROVENANCE: Mickey's own FxRecord layout and m2c draft supply the state transitions; no external body is adapted here. */
#ifdef NON_MATCHING
s32 func_80049B14(s32 delta) {
    FxRecord *record;
    s16 current;
    s16 duration;
    s32 next;
    s32 carry;
    s32 bit;
    u16 flags;
    s32 mode;

    D_800D5F50 = 0;
    record = D_800D5F58;
    bit = 4;
    do {
        mode = record->state;
        if (mode != 0) {
            flags = record->flags;
            carry = delta;
            if ((flags & 4) != 0) {
                record->flags = flags & ~4;
            } else if (delta != 0) {
                do {
                    switch (mode = record->state) {
                    case 0:
                        carry = 0;
                        record->status = 0;
                        break;
                    case 1:
                        record->value14 = (s16) (record->value14 + carry);
                        current = record->value14;
                        duration = record->value16;
                        if (current >= duration) {
                            if (record->value18 != 0) {
                                next = current - duration;
                                carry = next;
                                record->state = 2;
                                record->value14 = next;
                                record->status = 0xFF;
                            } else {
                                carry = 0;
                                if ((record->value1F != 0) &&
                                    (record->value1E == 0)) {
                                    next = current - duration;
                                    carry = next;
                                    record->state = 3;
                                    record->value14 = next;
                                    record->status = 0xFF;
                                } else {
                                    record->state = 0;
                                    record->status = 0;
                                }
                            }
                        } else {
                            carry = 0;
                            record->status =
                                (u8) ((current * 0xFF) / duration);
                        }
                        break;
                    case 2:
                        duration = record->value18;
                        if (duration < 0) {
                            carry = 0;
                        } else {
                            record->value14 = (s16) (record->value14 + carry);
                            current = record->value14;
                            carry = 0;
                            if (current >= duration) {
                                /* carry doubles as the 0x1E mode byte: the
                                 * target loads it into carry's register, and
                                 * this spelling is what brings size delta to 0. */
                                carry = record->value1E;
                                if (((carry != 0) && (record->value1F == 0)) ||
                                    ((carry == 0) && (record->value1F != 0))) {
                                    carry = current - duration;
                                    record->state = 3;
                                    record->value14 = carry;
                                } else {
                                    if (record->value1F != 0) {
                                        carry = current - duration;
                                        record->state = 1;
                                        record->value14 = carry;
                                    } else {
                                        record->state = 0;
                                        record->status = 0;
                                    }
                                }
                            }
                        }
                        break;
                    case 3:
                        record->value14 = (s16) (record->value14 + carry);
                        current = record->value14;
                        duration = record->value16;
                        if (current >= duration) {
                            carry = 0;
                            if ((record->value1E != 0) &&
                                (record->value1F != 0) &&
                                (record->value18 != 0)) {
                                next = current - duration;
                                carry = next;
                                record->state = 2;
                                record->value14 = next;
                                record->status = 0;
                            } else {
                                record->state = 0;
                                record->status = 0;
                            }
                        } else {
                            carry = 0;
                            record->status =
                                (u8) (((duration - current) * 0xFF) / duration);
                        }
                        break;
                    }
                } while (carry != 0);
            }
            if (record->state != 0) {
                D_800D5F50 |= 0x10 >> bit;
            }
        }
        record++;
    } while (bit--);
    return D_800D5F50;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/fx/func_80049B14.s")
#endif
/* PROVENANCE: Mickey's target commands, globals, and CFG supply this
 * reconstruction; the GBI macros are project SDK headers. */
#define FX_SET_SCREEN_RENDER(packet, mode) { \
    Gfx *_g = (packet); \
    _g->words.w0 = 0xEF002C0F; \
    _g->words.w1 = (mode); \
}
void func_80049E4C(Gfx **dlist, s32 arg1) {
    s32 count;
    s32 width;
    s32 height;
    FxRecord *record;

    if (D_800D5F50 != 0) {
        viGetCurrentSize(&width, &height);
        gDPPipeSync((*dlist)++);
        gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE, 0, 0,
                      (u32) width, (u32) height);
        gSPClearGeometryMode((*dlist)++, G_ZBUFFER | G_FOG);
        gDPSetCombineMode((*dlist)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        if (arg1 == 0) {
            record = D_800D5F58;
            count = 4;
        } else {
            record = (FxRecord *) D_800D5FD8;
            count = 1;
        }
        while (count--) {
            if (record->status != 0) {
                if (record->status == 0xFF) {
                    FX_SET_SCREEN_RENDER((*dlist)++, 0x0F0A4000);
                } else {
                    FX_SET_SCREEN_RENDER((*dlist)++, 0x00504340);
                }
                gDPSetPrimColor((*dlist)++, 0, 0, record->red,
                                record->green, record->blue, record->status);
                gDPFillRectangle((*dlist)++, record->value4, record->value8,
                                 record->valueC, record->value10);
                gDPPipeSync((*dlist)++);
            }
            record++;
        }
        func_80034920(dlist);
        camSetScissor(dlist);
        gDPSetPrimColor((*dlist)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
#undef FX_SET_SCREEN_RENDER
void func_8004A0F0(void) {
    D_800D6038[0] = 0;
    D_800D6038[1] = 0;
    D_800D6040 = 0;
}
/* PROVENANCE: JFG's corresponding routine is assembly-only; this body is
 * reconstructed from Mickey's own listing.
 *
 * Matched by rewriting the inherited hand-unrolled m2c shape: the 9-column loop
 * is a plain counted `for` that IDO's default unroller expands (one remainder
 * copy, then four per pass); the mask/shift pair is an if/else (the target's
 * branch over an empty arm); the pixel offset is added to the base, not the
 * base to the offset; the masked channel is held in `value` itself, with the
 * old channel XORed out before the increment is added (that one reuse is what
 * orders the colours: value v1, old pixel a0); and the pattern cursor is
 * started on the `do` line, which is the as1 tie-break that hoists the end
 * pointer's address before the cursor's. */
void func_8004A10C(s32 *screen, u8 glyph, s32 x, s32 y, s32 arg4) {
    u16 *pixel;
    s32 colorMask;
    s32 shift;
    u32 *pattern;
    s32 column;
    s32 rowBits;
    s32 bit;
    s32 intensity;
    s32 oldPixel;
    s32 value;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    pixel = (y * width + x) + (u16 *) screen;
    if (arg4 != 0) {
        colorMask = 0xF800;
        shift = 11;
    } else {
        colorMask = 0x7C0;
        shift = 6;
    }
    pattern = D_8007D320; do {
        rowBits = *pattern;
        for (column = 0; column < 9; column++) {
            bit = rowBits & 7;
            rowBits >>= 3;
            if (bit != 0) {
                intensity = 4;
                if (glyph & (1 << bit)) {
                    intensity = 16;
                }
                oldPixel = *pixel;
                value = oldPixel & colorMask;
                oldPixel ^= value;
                value += intensity << shift;
                if (value & ~colorMask) {
                    value = colorMask;
                }
                *pixel = oldPixel | value;
            }
            pixel++;
        }
        pixel += width - 9;
        pattern++;
    } while (pattern != (u32 *) D_8007D364);
}
/* PROVENANCE: role adapted from JFG src/fx.c::func_8006DF90; both bodies are
 * assembly-only, so this reconstruction is Mickey-derived. */
/* Workbench: instruction-words-identical, 0 differing words; 76 instructions/frame -0x80. */
void func_8004A380(s32 x, s32 y, s32 value, s32 minimumWidth, s32 arg4) {
    s32 length;
    s32 index;
    u8 glyph;
    u8 character;
    char text[32];

    length = 0;
    index = 0;
    sprintf(text, "%d", value);
    if (text[length] != '\0') {
        do {
            length++;
        } while (text[length] != '\0');
    }
    if (minimumWidth >= length) {
        do {
            glyph = D_8007D364[11];
            if (length < minimumWidth) {
                length++;
            } else {
                character = text[index++];
                if (character == '-') {
                    glyph = D_8007D364[10];
                } else if (character >= '0' && character < ':') {
                    glyph = D_8007D364[character - '0'];
                }
            }
            func_8004A10C(D_800D2FA0, glyph, x, y, arg4);
            x += 10;
        } while (text[index] != '\0');
    }
}
/* Mickey-derived body; JFG's corresponding fx.c function is assembly-only. */
void func_8004A4B0(s32 value0, s32 value2, s32 value4, s32 value6,
                   s32 value7) {
    s32 group;
    s32 *countPtr;
    FxSpdRecord *record;

    group = D_800D6040;
    countPtr = &D_800D6038[group];
    if (*countPtr < 4) {
        record = &D_800D5FF8[group][(*countPtr)++];
        record->value0 = value0;
        record->value2 = value2;
        record->value4 = value4;
        record->value6 = value6;
        record->value7 = value7;
    }
}
/* Mickey-derived body; JFG's corresponding fx.c function is assembly-only. */
void func_8004A51C(void) {
    s32 group;
    s32 count;
    FxSpdRecord *record;

    group = D_800D6040;
    count = D_800D6038[group];
    record = D_800D5FF8[group];
    D_800D6040 = group ^ 1;
    D_800D6038[D_800D6040] = 0;
    while (count--) {
        func_8004A380(record->value0, record->value2, record->value4,
                      record->value6, record->value7);
        record++;
    }
}
/* PROVENANCE -- Jet Force Gemini's public fx.c places the same-named
 * fxSPDPRipple routine at this TU boundary, but publishes assembly only.
 * Mickey's target assembly supplies the fields, constants, and call order.
 *
 * Matched by discarding the inherited shape. What decided it, in order:
 * the three wave phases are this function's own statics (ROM 0x7DF70, the
 * TU's .data), so each is a register web with by-name loads and stores and
 * no address web; the packets are the SDK macros, whose unused `_g` copies
 * still take colours (a0 in the setup block, v1 in the colour block);
 * rippleEnabled is read from the level at each use; the three samples are
 * one expression, so each result is saved in the next call's delay slot; the
 * clamp is a conditional expression, whose empty arm is the extra block that
 * ranks the hoisted fill-rectangle word above alphaHigh and alphaLow, which
 * then live in their homes; alpha is rescaled in place; and `top`, a copy of
 * the row taken before the colour packet, is the web that keeps t1 out of
 * the temporary ring. */
typedef struct FxRippleLevel {
    u8 pad00[0xFA];
    u8 rippleEnabled;
} FxRippleLevel;

extern FxRippleLevel *levelGetLevel(void);
extern s32 func_8002A204(s16 angle);

void fxSPDPRipple(FxGfx **dList, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                  s32 arg5) {
    static s16 sPhaseA = 0;
    static s16 sPhaseB = 0x5555;
    static s16 sPhaseC = 0xAAAA;
    FxRippleLevel *level;
    s32 angleA;
    s32 angleB;
    s32 angleC;
    s32 i;
    s32 pad;
    s32 alphaHigh;
    s32 alphaMid;
    s32 alphaLow;
    s32 wave;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 top;
    s32 next;

    level = levelGetLevel();
    if ((level != NULL) && (level->rippleEnabled != 0)) {
        func_800349A4(dList, 0, 4, 0);
        gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        sPhaseA += (arg5 << 0xD) >> 4;
        sPhaseB += (arg5 * -0x3C00) >> 4;
        sPhaseC += (arg5 * 0x1800) >> 4;
        angleA = sPhaseA + (arg2 << 0xA);
        angleB = sPhaseB + (arg2 * 0xBA2);
        angleC = sPhaseC + (arg2 * 0x28F);
        alphaHigh = (level->rippleEnabled * 0x58) >> 7;
        alphaMid = (level->rippleEnabled * 0x50) >> 7;
        alphaLow = (level->rippleEnabled * 0x48) >> 7;
        for (i = arg2; i < arg4; i++) {
            wave = ((func_8002A204(angleA) * 0xC0) +
                    (func_8002A204(angleB) * 0x60) +
                    (func_8002A204(angleC) << 6)) >> 8;
            if (wave < 0) {
                wave = -wave;
                red = 8;
                green = 0x20;
                blue = 0xA0;
                alpha = alphaLow;
            } else {
                red = 0x80;
                green = 0xC0;
                blue = 0xFF;
                alpha = alphaHigh;
            }
            wave = (wave > 0x10000) ? 0x10000 : wave;
            red = (((red - 0x20) * wave) >> 0x10) + 0x20;
            green = (((green - 0x78) * wave) >> 0x10) + 0x78;
            blue = (((blue - 0xFF) * wave) >> 0x10) + 0xFF;
            alpha = (((alpha - alphaMid) * wave) >> 0x10) + alphaMid;
            angleA += 0x400;
            angleB += 0xBA2;
            angleC += 0x28F;
            top = i;
            gDPSetPrimColor((*dList)++, 0, 0, red, green, blue, alpha);
            next = i + 1;
            gDPFillRectangle((*dList)++, arg1, top, arg3, next);
            gDPPipeSync((*dList)++);
        }
        func_80034920(dList);
    }
}
void fxQueueScreenEffect(s32 type, s32 value4, s32 value6, s32 value8,
                         s32 valueA, s32 valueC, s32 valueE, s32 value10) {
    FxScreenEffect *effect;

    if (D_8007D478 < 4) {
        effect = &D_800D6048[D_8007D478++];
        effect->type = type;
        effect->value4 = value4;
        effect->value6 = value6;
        effect->value8 = value8;
        effect->valueA = valueA;
        effect->valueC = valueC;
        effect->valueE = valueE;
        effect->value10 = value10;
    }
}
void func_8004A9CC(FxGfx **dList) {
    FxScreenEffect *effect;
    s32 index;

    effect = D_800D6048;
    index = 0;
    if (D_8007D478 > 0) {
        do {
            fxScreenEffect(dList, effect->type, effect->value4,
                           effect->value6, effect->value8, effect->valueA,
                           effect->valueC, effect->valueE, effect->value10);
            index++;
            effect++;
        } while (index < D_8007D478);
    }
    D_8007D478 = 0;
}
/* PROVENANCE: the screen-copy loop follows the matched sibling func_80036F08
 * (adapted there from JFG src/screen.c:screenDraw); this body is written from
 * Mickey's own listing.
 * Matched by discarding the inherited m2c shape (eight packet carriers, a
 * copied cursor and limit) for the sibling's: one packet macro per command on
 * (*dList)++, gSPTextureRectangle for the E4/B3/B2 triple, the four
 * coordinates rescaled in place with arg5 itself as the row cursor, the
 * texture s taken from arg4 before the rescale (that is the unfolded
 * (arg4 << 5) << 16), the offset added to the base, case 3 laid out before
 * case 2, and the DMA command's w1 written before its w0. */
void fxScreenEffect(FxGfx **dList, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                    s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    FxGfx *table;
    u16 *screen;
    s32 top;
    s32 s;

    switch (viGetVideoMode()) {
    case 3:
        table = D_8007D408;
        break;
    case 2:
        table = D_8007D408;
        break;
    default:
        table = D_8007D3D0;
        break;
    }
    gSPDisplayList((*dList)++, D_8007D380);
    gDPSetColor((*dList)++, G_SETPRIMCOLOR, arg8);
    screen = arg2 * arg5 + (u16 *) arg1;
    s = arg4 << 5;
    arg4 <<= 2;
    arg5 <<= 2;
    arg6 <<= 2;
    arg7 <<= 2;
    while (arg5 < arg7) {
        (*dList)->w0 = table->w0;
        (*dList)->w1 = (u32) screen;
        (*dList)++;
        {
            FxGfx *_g = (*dList)++;
            _g->w1 = (u32) (table + 1) + 0x80000000;
            _g->w0 = 0x07060030;
        }
        screen += arg2 * 4;
        top = arg5;
        arg5 += 16;
        if (arg5 > arg7) {
            arg5 = arg7;
        }
        gSPTextureRectangle((*dList)++, arg4, top, arg6, arg5, 0, s, 0,
                            1 << 10, 1 << 10);
    }
    func_80034920(dList);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dList)++, 255, 255, 255, 255);
}
/* Resets the four texture slots and records, per slot, whether its callback
 * is still the dangling-jump trap.
 *
 * The four cursors in the emitted code are IDO's own strength-reduced
 * induction variables, not source pointers: the source indexes three parallel
 * four-element arrays with one loop variable and uopt walks them backwards.
 * That is what puts the now-dead copy of the source index in the *first* pool
 * colour and the synthesised trip counter in the second -- with hand-written
 * cursors the copy is compiler-generated and takes the last colour instead, a
 * 10-word residual that no ordering of the setup statements can reach (720
 * statement orders x 32 physical-line groupings, all flat).
 *
 * The relocations name D_800D60BC/D_800D60CC/D_8007D488 in the target's
 * disassembly and the array bases plus an addend here; both resolve to the
 * same linked words.
 */
void func_8004ACC4(void) {
    s32 i;

    D_800D60A8 = 0;
    i = 4;
    while (i--) {
        D_800D60B0[i] = 0;
        D_800D60C0[i] = 0;
        D_800D60D0[i] = (s32) TrapDanglingJump == (s32) D_8007D47C[i];
    }
}
s32 func_8004AD34(void) {
    FxTextureCallback callback;
    s32 index;

    index = 4;
    while (index--) {
        if ((1 << index) & D_800D60A8) {
            func_800320F0((s32)D_8007D47C + (index << 2));
            callback = (FxTextureCallback)D_8007D47C[index];
            if (callback != 0) {
                callback(index, D_800D6098[index], 0);
            }
        }
    }
    D_800D60A8 = 0;
}
extern void *func_8002B280(s32 size, s32 tag);

/*
 * PROVENANCE: the source topology is informed by Jet Force Gemini's public
 * src/fx.c::fxCpuTextureRequired placeholder and its retail-derived assembly;
 * Mickey's target establishes all types, expressions, and final codegen.
 */
void func_8004ADE8(s32 index, FxConeTextureInfo *texture) {
    s8 *first;
    s32 i;
    s8 *second;

    index--;
    D_800D60A8 |= 1 << index;
    D_800D6098[index] = (s32)texture;
    if (D_800D60B0[index] == 0) {
        first = func_8002B280(texture->width * texture->height, 0x87);
        D_800D60B0[index] = first;
        D_800D60C0[index] = second = func_8002B280(
            texture->width * texture->height, 0x87);
        if (D_800D60B0[index] == 0 || D_800D60C0[index] == 0) {
            D_800D60B0[index] = 0;
            D_800D60C0[index] = 0;
            return;
        }
        i = texture->width * texture->height;
        while (i--) {
            *first = 0;
            first++;
            *second = 0;
            second++;
        }
        func_800320F0((s32)&D_8007D47C[index]);
        second = (s8 *)D_8007D47C[index];
        if (second != 0) {
            ((FxTextureCallback)second)(index, D_800D6098[index], 1);
        }
    }
}
/* Frees both allocations for every texture slot and re-arms the slots whose
 * callback is still live with the dangling-jump trap.
 *
 * Same shape as func_8004ACC4 above and for the same reason: one loop
 * variable over four parallel four-element arrays, with every cursor in the
 * emitted code built by uopt. It reduces `D_800D60B0[i]` and `D_800D60D0[i]`
 * to backwards-walking pointers, shares ONE byte offset between
 * `D_800D60C0[i]` and `D_8007D47C[i]` (`addu s0, s1, t6` and
 * `addu t8, s5, s1`), materialises `D_800D60C0`'s base inside the loop, and
 * leaves the source index dead but still coloured -- the `move v0, s3`.
 * Hand-written cursors could reach none of that: sharing an offset in source
 * hoists the base into an eighth saved register (+3 words) and indexing both
 * arrays separately emits two shifts (+3). Both were measured, at a floor of
 * 15 words.
 *
 * JFG's fxCpuTextureFlush counterpart is still assembly-only; no donor body
 * was available or used.
 */
void func_8004AF68(void) {
    s32 i;
    void *allocation;

    i = 4;
    while (i--) {
        allocation = D_800D60B0[i];
        if (allocation != 0) {
            mmFree(allocation);
            mmFree(D_800D60C0[i]);
            D_800D60B0[i] = 0;
            D_800D60C0[i] = 0;
        }
        if (D_800D60D0[i] != 0) {
            D_8007D47C[i] = (FxTextureCallback) TrapDanglingJump;
        }
    }
    D_800D60A8 = 0;
}



/* PLATEAU-HANDOFF:func_80049B14:start
 * symbol: func_80049B14
 * score: 105/206 words
 * frame: 0x18
 * relocations: 4
 * first-mismatch: +0x30
 * summary: State read into a local at each record top and in the switch: constants become p1 webs (v1..t5), variables s0..s4. 181 at 0 to 105 at +4.
 * PLATEAU-HANDOFF:func_80049B14:end
 */

/* PLATEAU-HANDOFF:wakeAllocate:start
 * symbol: wakeAllocate
 * score: 325/351 words
 * frame: 0x90
 * relocations: 3
 * first-mismatch: +0x10
 * summary: Natural rewrite (own triangle count, if/else flags, word id, one cursor): -32 to -8, frame 0x90. Left: dead v0/v1 copies before the call.
 * PLATEAU-HANDOFF:wakeAllocate:end
 */

/* PLATEAU-HANDOFF:wakeUpdate:start
 * symbol: wakeUpdate
 * score: 253/398 words
 * frame: 0x90
 * relocations: 2
 * first-mismatch: +0x34
 * summary: Second-loop wrap test as ++index (257 to 253, delta 0). Left: p1 order wake/secondaryVertices, index/stripIndex, polygonOffset over polyCount.
 * PLATEAU-HANDOFF:wakeUpdate:end
 */
