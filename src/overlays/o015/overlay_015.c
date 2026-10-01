#include "overlays/overlay_015.h"

/* The shipped LOCAL relocations address this block through the initialized
 * section base; text-side proxy symbols remain the relocation carriers. */
typedef struct Overlay15InitializedData {
    u32 prefix[6];
    Overlay15Gfx starSetup[5];
    f32 starFadeScale;
} Overlay15InitializedData;

Overlay15InitializedData gOverlay15InitializedData = {
    { 0, 0, 0, 1, 0, 0 },
    {
        { 0xE7000000U, 0 },
        { 0xB6000000U, 0x00010001U },
        { 0xFCFFFFFFU, 0xFFFDF6FBU },
        { 0xEF000C0FU, 0x0F0A4000U },
        { 0xB8000000U, 0 },
    },
    0.8732876777648926F,
};

/* The two particle fields are the whole of this unit's BSS: the starfield at
 * +0x00 and the rain field at +0x50. They are defined here, and `static`,
 * because the shipped code shares one high half between two adjacent bound
 * loads, which IDO only does for a locally-defined symbol; `static` also makes
 * every access a section-relative record whose addend is already the
 * module-relative offset the shipped word carries (mk/overlays.mk drops those
 * records so the static link does not adjust them a second time). Functions
 * not yet rewritten still reach these words through text-side proxies. */
typedef struct Overlay15Field {
    /* 0x00 */ Overlay15InitBounds bounds;
    /* 0x30 */ Overlay15Star movement;
    /* 0x3C */ Overlay15Star previous;
    /* 0x48 */ u32 *colors;
    /* 0x4C */ s32 pad4C;
} Overlay15Field;

static Overlay15Field sOverlay15Starfield;
static Overlay15Field sOverlay15Rain;

/*
 * Overlay 15, ADR 0006 consolidation. Functions remain in retail ROM order.
 * The R4300 multiply-hazard flag is harmless for the active resource/value
 * wrappers and is required by the fallback-heavy drawing translation unit.
 */

void *overlay15GetResource4(void) {
    return gOverlay15Resource4;
}

/* DKR v77/v80 has generic resource-release wrappers but no exact donor. */
void overlay15ReleaseResource(void) {
    if (gOverlay15Resource4 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource4);
        gOverlay15Resource4 = 0;
        gOverlay15Resource48 = 0;
    }
}

/* Plateau (2026-08-25, batch 36): canonical -O2 -mips2 is four bytes
 * short; best 230/247 words differ, first at +0x4. Field-order/count-address
 * lifetimes improved 238 to 230; lattice, playbook, and 40m permuter found no exact. */
/* s1-b (2026-09-16): the palette index inits ahead of the colour block and the
 * zero/zMax stores ahead of colorDivisor's: 70 to 60. The size definition is
 * forwarded into a type-4 temp (s3) where the target keeps starIndex's web (s0). */
/* s2-b (2026-09-16): 60 to 41. A self-redefinition of count (`count |= 0`)
 * in the entry block stops uopt forwarding `starIndex = count * 12` into its
 * uses, so the size stays in starIndex's symbol web (s0) and count keeps its
 * parameter copy (s2); uopt then deletes the redefinition (delta 0). On that
 * head the natural xMax-before-yRange order syncs the FP ring, and the count
 * store written to the global directly (no cast pointer) forms the address in
 * s7 itself. See docs/lastmile-forwarding-kill.md. */
/* w20-o015 (2026-09-18): 41 to 19. Guard-local starIndex=1 (41 to 20).
 * Same-line allocate + starsAddress + store (L59) keeps addiu+sw
 * adjacent, 20 to 19; first structural moves to +0xF8. */
#ifdef NON_MATCHING
void overlay15InitStarsAndPalette(s32 count, s32 xRange, s32 yRange,
                                  s32 zRange, u32 startColor, u32 endColor,
                                  s32 colorDivisor) {
    Overlay15Star *stars;
    Overlay15Star **starsAddress;
    Overlay15InitBounds *bounds;
    u16 *palette;
    s32 starIndex;
    /* Reuse the completed star-loop counters for the first two palette indices. */

    s32 paletteIndex2;
    s32 paletteIndex3;
    s32 startR;
    s32 startG;
    s32 startB;
    s32 deltaR;
    s32 deltaG;
    s32 deltaB;
    s32 previousStarIndex;

    starIndex = count * 12;
    count |= 0; /* kills the multiply's operand: the definition is not forwarded */
    stars = overlay15Allocate(starIndex + 0x200, 0x87); starsAddress = &gOverlay15Stars; *starsAddress = stars;
    gOverlay15StarPalette = (u16 *) ((u8 *) *starsAddress + starIndex);

    bounds = &gOverlay15InitBounds;
    bounds->xRange = (f32) xRange;
    bounds->xMin = bounds->xRange * -0.5f;
    bounds->xMax = bounds->xRange * 0.5f;
    bounds->yRange = (f32) yRange;
    xRange <<= 7;
    bounds->yMin = bounds->yRange * -0.5f;
    yRange <<= 7;
    bounds->yMax = bounds->yRange * 0.5f;
    gOverlay15StarCount = count;
    bounds->zRange = (f32) zRange;
    zRange = (zRange + 1) << 8;
    bounds->zero = 0;
    bounds->zMax = bounds->zRange + 1.0f;
    bounds->colorDivisor = (f32) colorDivisor;
    bounds->zMin = 1.0f;
    bounds->colorStep = 255.0f / bounds->colorDivisor;

    previousStarIndex = 0;
    if (count > 0) {
        starIndex = 1; /* loop start; keeping this outside PRE-sinks palette's `starIndex = 1` onto the exit path */
        do {
            stars->x = (f32) overlay15RandomRange(-xRange, xRange) *
                       (1.0f / 256.0f);
            stars->y = (f32) overlay15RandomRange(-yRange, yRange) *
                       (1.0f / 256.0f);
            stars->z = (f32) overlay15RandomRange(0x100, zRange) *
                       (1.0f / 256.0f);
            previousStarIndex = starIndex;
            starIndex++;
            stars++;
        } while (previousStarIndex < gOverlay15StarCount);
        previousStarIndex = 0;
    }

    starIndex = 1;
    paletteIndex2 = 2;
    paletteIndex3 = 3;
    startR = (startColor >> 24) & 0xFF;
    startG = (startColor >> 16) & 0xFF;
    startB = (startColor >> 8) & 0xFF;
    deltaR = ((endColor >> 24) & 0xFF) - startR;
    deltaG = ((endColor >> 16) & 0xFF) - startG;
    deltaB = ((endColor >> 8) & 0xFF) - startB;

    palette = gOverlay15StarPalette;


    do {
        *palette++ =
            (((((deltaR * previousStarIndex) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * previousStarIndex) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * previousStarIndex) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * starIndex) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * starIndex) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * starIndex) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * paletteIndex2) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex2) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex2) >> 8) + startB) & 0xF8) >> 2) | 1;
        *palette++ =
            (((((deltaR * paletteIndex3) >> 8) + startR) & 0xF8) << 8) |
            (((((deltaG * paletteIndex3) >> 8) + startG) & 0xF8) << 3) |
            (((((deltaB * paletteIndex3) >> 8) + startB) & 0xF8) >> 2) | 1;
        previousStarIndex += 4;
        starIndex += 4;
        paletteIndex2 += 4;
        paletteIndex3 += 4;
    } while (previousStarIndex != 0x100);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F000004C_18723E4.s")
#endif

typedef struct Overlay15StarPointerView {
    u8 pad00[4];
    Overlay15Star *stars;
} Overlay15StarPointerView;

/*
 * Matched 2026-10-01. The field is reached through a pointer taken at the top
 * of the function. In the entry block uopt keeps that pointer as the base of
 * the three movement stores; in the block behind the stars test it forwards
 * the address into each bound read instead, so the nine reads are direct and
 * as1 shares one high half per aligned pair. Nine separate bound scalars cost
 * four extra high halves, and naming the struct's members directly makes
 * every read go through one base register.
 */
void overlay15MoveStars(f32 movementX, f32 movementY, f32 movementZ,
                        s32 rate) {
    Overlay15Field *field = &sOverlay15Starfield;
    f32 scale;

    field->movement.x = movementX;
    field->movement.y = movementY;
    field->movement.z = movementZ;
    if (((Overlay15StarPointerView *)&gOverlay15Stars)->stars != 0) {
        scale = (f32)rate;
        movementX *= scale;
        movementY *= scale;
        movementZ *= scale;
        starfieldFastMove(gOverlay15StarCount,
                          ((Overlay15StarPointerView *)&gOverlay15Stars)->stars,
                          movementX, movementY, movementZ,
                          field->bounds.xMin, field->bounds.xMax,
                          field->bounds.xRange, field->bounds.yMin,
                          field->bounds.yMax, field->bounds.yRange,
                          field->bounds.zMin, field->bounds.zMax,
                          field->bounds.zRange);
    }
}

/*
 * Matched 2026-09-16 (lane nx-c). Two edits closed the last nine words:
 * the fade scale is the float literal 255.0f / 292.0f (255 over the visible
 * depth range 300 - 8) assigned to a local, not a data global -- the shipped
 * value sits in this unit's own literal pool (data_rodata +0x40, the last
 * word of gOverlay15InitializedData), and as a pool constant uopt hoists it
 * into the loop preheader below the synthesised zero-trip guard and colours
 * it after the two depth constants (single use, numbered last); and the two
 * setup-command stores share one physical line (L59), which is what let as1
 * order the entry block once the fade load had left it. The pool the compiler
 * emits for the literal is externalized back onto the retained data word in
 * mk/overlays.mk. See docs/matching-triage-handoffs/overlay15DrawScreenStars.md.
 */
void overlay15DrawScreenStars(Overlay15Gfx **displayList, f32 projectionScale) {
    Overlay15Gfx *command;
    Overlay15Star *star;
    s32 remaining;
    s32 screenX;
    s32 screenWidth;
    s32 screenHeight;
    s32 screenY;
    s32 shade;
    f32 inverseDepth;
    Overlay15Gfx *initialCommand;
    f32 fadeScale;

    overlay15GetDimensionsReloc(&screenWidth, &screenHeight);
    remaining = gOverlay15StarCount;
    command = *displayList;
    star = ((Overlay15StarPointerView *)&gOverlay15Stars)->stars;
    initialCommand = command++;
    initialCommand->w0 = 0x06000000; initialCommand->w1 = (u32) gOverlay15StarSetup;
    fadeScale = 255.0f / 292.0f;

    while (remaining--) {
        if ((star->z >= 8.0f) && (star->z < 300.0f)) {
            inverseDepth = projectionScale / star->z;
            screenX = (s32) (star->x * inverseDepth) +
                      (s32) (((u32) screenWidth) >> 1);
            screenY = (s32) (((u32) screenHeight) >> 1) -
                      (s32) (star->y * inverseDepth);
            if ((screenX >= 0) && (screenY >= 0) &&
                (screenX < screenWidth) && (screenY < screenHeight)) {
                shade = 255 - (s32) ((star->z - 8.0f) * fadeScale);
                command->w0 = 0xFA000000; command->w1 = (shade << 24) | (shade << 16) | (shade << 8) | 0xFF; command++; command->w0 = 0xF6000000 | ((screenX + 1) << 14) | ((screenY + 1) << 2); command->w1 = (screenX << 14) | (screenY << 2); command++;
            }
        }
        star++;
    }

    *displayList = command;
    overlay15FinishDisplayListReloc(displayList);
}

void *overlay15GetResource10(void) {
    return gOverlay15Resource10;
}

void overlay15ReleaseResource10(void) {
    if (gOverlay15Resource10 != 0) {
        overlay15ReleaseReloc(gOverlay15Resource10);
        gOverlay15Resource10 = 0;
    }
}

/* Matched 2026-10-01. The rain field's bounds are the static struct's own, and
 * the loop is a for over the global count whose init clause sets the index and
 * both cursors; setup written to the globals is read back, as in the target.
 * Five unused s32 pads land the frame. The colour block starts at
 * count * 12 bytes (a byte-pointer sum, not `stars + count`, which commutes
 * the final add). */
void overlay15InitStars(s32 count, s32 xRange, s32 yRange, s32 zRange,
                        u32 startColor, u32 endColor, s32 colorDivisor) {
    Overlay15Star *stars;
    u32 *colors;
    s32 i;
    s32 startR;
    s32 startG;
    s32 startB;
    s32 startA;
    s32 deltaR;
    s32 deltaG;
    s32 deltaB;
    s32 deltaA;
    s32 colorTime;
    s32 pad0;
    s32 pad1;
    s32 pad2;
    s32 pad3;
    s32 pad4;

    gOverlay15MovingStars = overlay15Allocate(count << 4, 0x87);
    gOverlay15RainColors = (u32 *) ((u8 *) gOverlay15MovingStars + count * 12);
    gOverlay15MovingStarCount = count;
    sOverlay15Rain.bounds.zero = 0;
    sOverlay15Rain.bounds.xRange = (f32) xRange;
    sOverlay15Rain.bounds.xMin = sOverlay15Rain.bounds.xRange * -0.5f;
    sOverlay15Rain.bounds.xMax = sOverlay15Rain.bounds.xRange * 0.5f;
    sOverlay15Rain.bounds.yRange = (f32) yRange;
    sOverlay15Rain.bounds.yMin = sOverlay15Rain.bounds.yRange * -0.5f;
    sOverlay15Rain.bounds.yMax = sOverlay15Rain.bounds.yRange * 0.5f;
    sOverlay15Rain.bounds.zRange = (f32) zRange;
    sOverlay15Rain.bounds.zMin = sOverlay15Rain.bounds.zRange * -0.5f;
    sOverlay15Rain.bounds.zMax = sOverlay15Rain.bounds.zRange * 0.5f;
    sOverlay15Rain.bounds.colorDivisor = (f32) colorDivisor;
    sOverlay15Rain.bounds.colorStep = 255.0f / sOverlay15Rain.bounds.colorDivisor;
    xRange <<= 7;
    yRange <<= 7;
    zRange <<= 7;

    startR = (startColor >> 24) & 0xFF;
    startG = (startColor >> 16) & 0xFF;
    startB = (startColor >> 8) & 0xFF;
    startA = startColor & 0xFF;
    deltaR = ((endColor >> 24) & 0xFF) - startR;
    deltaG = ((endColor >> 16) & 0xFF) - startG;
    deltaB = ((endColor >> 8) & 0xFF) - startB;
    deltaA = (endColor & 0xFF) - startA;
    for (i = 0, stars = gOverlay15MovingStars, colors = gOverlay15RainColors;
         i < gOverlay15MovingStarCount; i++) {
        stars->x = (f32) overlay15RandomRange(-xRange, xRange) *
                   (1.0f / 256.0f);
        stars->y = (f32) overlay15RandomRange(-yRange, yRange) *
                   (1.0f / 256.0f);
        stars->z = (f32) overlay15RandomRange(-zRange, zRange) *
                   (1.0f / 256.0f);
        colorTime = overlay15RandomRange(0, 255);
        *colors = ((((deltaR * colorTime) >> 8) + startR) << 24) |
                  ((((deltaG * colorTime) >> 8) + startG) << 16) |
                  ((((deltaB * colorTime) >> 8) + startB) << 8) |
                  (((deltaA * colorTime) >> 8) + startA);
        stars++;
        colors++;
    }
}

/*
 * Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative.
 * Matched 2026-10-01 on the form that closed overlay15MoveStars: the rain
 * field is reached through a pointer taken at entry, and because every use
 * sits behind the camera call uopt forwards the address into each access, so
 * all of them are direct and as1 shares one high half per aligned pair. That
 * removes the seven extra high halves of the scalar-symbol candidate. The
 * pointer is declared last, which leaves deltaZ's spill home where the target
 * has it, and the three position products are in x, y, z order.
 */
void overlay15UpdateMovingStars(f32 positionX, f32 positionY, f32 positionZ,
                                s32 updateRate) {
    Overlay15MovingStarCamera *camera;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 scale;
    Overlay15Field *field = &sOverlay15Rain;

    camera = overlay15GetActiveCameraReloc();
    if (field->bounds.zero != 0 && updateRate != 0) {
        scale = 1.0f / (f32)updateRate;
        deltaX = (field->previous.x - camera->x) * scale;
        deltaY = (field->previous.y - camera->y) * scale;
        deltaZ = (field->previous.z - camera->z) * scale;
    } else {
        deltaX = 0.0f;
        deltaY = 0.0f;
        deltaZ = 0.0f;
    }

    field->previous.x = camera->x;
    field->previous.y = camera->y;
    field->previous.z = camera->z;
    positionX += deltaX;
    positionY += deltaY;
    positionZ += deltaZ;
    field->bounds.zero = 1;
    field->movement.x = positionX;
    field->movement.y = positionY;
    field->movement.z = positionZ;

    if (gOverlay15MovingStars != 0) {
        scale = (f32)updateRate;
        positionX *= scale;
        positionY *= scale;
        positionZ *= scale;
        starfieldFastMove(gOverlay15MovingStarCount, gOverlay15MovingStars,
                          positionX, positionY, positionZ,
                          field->bounds.xMin, field->bounds.xMax,
                          field->bounds.xRange, field->bounds.yMin,
                          field->bounds.yMax, field->bounds.yRange,
                          field->bounds.zMin, field->bounds.zMax,
                          field->bounds.zRange);
    }
}

void overlay15SetValueC(s32 value) {
    gOverlay15ValueC = value;
}

void overlay15ClearValue7C(void) {
    gOverlay15Value7C = 0;
}

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
/* Matched 2026-10-01 on the overlay15MoveStars form: the rain field is a static
 * struct reached through a pointer local, so the colour and offset reads are
 * direct and share high halves. */
void overlay15DrawRain(void *framebuffer, s32 width, s32 height,
                       f32 projectionScale, f32 intensity) {
    s32 visibleCount;
    Overlay15Field *field = &sOverlay15Rain;
    Overlay15CameraState *camera;

    if (gOverlay15RainEnabled != 0) {
        visibleCount = (s32)((f32)gOverlay15RainCapacity * intensity);
        if ((gOverlay15RainPositions != 0) && (visibleCount > 0)) {
            camera = overlay15GetActiveCameraReloc();
            rainFastDraw(framebuffer, width, height, visibleCount,
                         gOverlay15RainPositions, field->colors,
                         camera->angle + 0x8000, field->movement.x,
                         field->movement.y, field->movement.z,
                         projectionScale);
        }
    }
}

/* PLATEAU-HANDOFF:overlay15InitStarsAndPalette:start
 * symbol: overlay15InitStarsAndPalette
 * score: 19/247 words
 * frame: 0x40
 * relocations: 14
 * first-mismatch: +0x70
 * summary: Same-line allocate+starsAddress+store keeps addiu+sw adjacent, 20 to 19; stars address still a1 (force p1:w317=c5 is 16), block 1 tail order and palette index inits remain.
 * PLATEAU-HANDOFF:overlay15InitStarsAndPalette:end
 */
