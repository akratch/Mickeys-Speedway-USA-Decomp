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

/* The primary bound scalars lead the BSS exactly as they do in the retail
 * object. Other particle modes alias these words through text-side proxies. */
f32 gOverlay15StarBound0;
f32 gOverlay15StarBound1;
f32 gOverlay15StarBound2;
f32 gOverlay15StarBound3;
f32 gOverlay15StarBound4;
f32 gOverlay15StarBound5;
f32 gOverlay15StarBound6;
f32 gOverlay15StarBound7;
f32 gOverlay15StarBound8;
u32 gOverlay15BssPad24;
u8 gOverlay15BssTail[0x78];

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
    s32 starCount;
    s32 previousStarIndex;
    s32 *countAddress;

    starIndex = count * 12;
    starCount = count;
    stars = overlay15Allocate(starIndex + 0x200, 0x87);
    starsAddress = &gOverlay15Stars;
    *starsAddress = stars;
    gOverlay15StarPalette = (u16 *) ((u8 *) *starsAddress + starIndex);

    bounds = &gOverlay15InitBounds;
    countAddress = (s32 *)(s32)&gOverlay15StarCount;
    bounds->xRange = (f32) xRange;
    bounds->xMin = bounds->xRange * -0.5f;
    bounds->yRange = (f32) yRange;
    xRange <<= 7;
    bounds->xMax = bounds->xRange * 0.5f;
    bounds->yMin = bounds->yRange * -0.5f;
    yRange <<= 7;
    bounds->yMax = bounds->yRange * 0.5f;
    bounds->zRange = (f32) zRange;
    zRange = (zRange + 1) << 8;
    *countAddress = starCount;
    bounds->zero = 0;
    bounds->zMax = bounds->zRange + 1.0f;
    bounds->colorDivisor = (f32) colorDivisor;
    bounds->zMin = 1.0f;
    bounds->colorStep = 255.0f / bounds->colorDivisor;

    previousStarIndex = 0; starIndex = 1;
    if (starCount > 0) {
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
        } while (previousStarIndex < *countAddress);
        previousStarIndex = 0;
    }

    startR = (startColor >> 24) & 0xFF;
    startG = (startColor >> 16) & 0xFF;
    startB = (startColor >> 8) & 0xFF;
    deltaR = ((endColor >> 24) & 0xFF) - startR;
    deltaG = ((endColor >> 16) & 0xFF) - startG;
    deltaB = ((endColor >> 8) & 0xFF) - startB;

    palette = gOverlay15StarPalette;

    starIndex = 1;
    paletteIndex2 = 2;
    paletteIndex3 = 3;
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

typedef struct Overlay15StarMovementView {
    u8 pad00[0x30];
    Overlay15Star movement;
} Overlay15StarMovementView;

typedef struct Overlay15StarPointerView {
    u8 pad00[4];
    Overlay15Star *stars;
} Overlay15StarPointerView;

#ifdef NON_MATCHING
void overlay15MoveStars(f32 movementX, f32 movementY, f32 movementZ,
                        s32 rate) {
    f32 scale;

    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.x =
        movementX;
    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.y =
        movementY;
    ((Overlay15StarMovementView *)&gOverlay15StarMovement)->movement.z =
        movementZ;
    if (((Overlay15StarPointerView *)&gOverlay15Stars)->stars != 0) {
        scale = (f32)rate;
        movementX *= scale;
        movementY *= scale;
        movementZ *= scale;
        starfieldFastMove(gOverlay15StarCount,
                          ((Overlay15StarPointerView *)&gOverlay15Stars)->stars,
                          movementX, movementY, movementZ,
                          gOverlay15StarBound0, gOverlay15StarBound1,
                          gOverlay15StarBound2, gOverlay15StarBound3,
                          gOverlay15StarBound4, gOverlay15StarBound5,
                          gOverlay15StarBound6, gOverlay15StarBound7,
                          gOverlay15StarBound8);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F0000428_18727C0.s")
#endif

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

#ifdef NON_MATCHING
void overlay15InitStars(s32 count, s32 xRange, s32 yRange, s32 zRange,
                        u32 startColor, u32 endColor, s32 colorDivisor) {
    Overlay15Star *stars;
    u32 *colors;
    Overlay15Star *volatile *starsAddress;
    u32 *volatile *colorsAddress;
    volatile s32 *countAddress;
    Overlay15InitBounds *unusedBoundsAddress;
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

    unusedBoundsAddress = &gOverlay15InitBounds;
    stars = overlay15Allocate(count << 4, 0x87);

    gOverlay15InitBounds.xRange = (f32) xRange;
    gOverlay15InitBounds.xMin = gOverlay15InitBounds.xRange * -0.5f;
    gOverlay15InitBounds.xMax = gOverlay15InitBounds.xRange * 0.5f;
    gOverlay15InitBounds.yRange = (f32) yRange;
    gOverlay15InitBounds.yMin = gOverlay15InitBounds.yRange * -0.5f;
    gOverlay15InitBounds.yMax = gOverlay15InitBounds.yRange * 0.5f;
    gOverlay15InitBounds.zRange = (f32) zRange;
    gOverlay15InitBounds.zMin = gOverlay15InitBounds.zRange * -0.5f;
    gOverlay15InitBounds.zMax = gOverlay15InitBounds.zRange * 0.5f;
    gOverlay15InitBounds.colorDivisor = (f32) colorDivisor;
    gOverlay15InitBounds.colorStep = 255.0f /
                                     gOverlay15InitBounds.colorDivisor;

    i = 0;
    colors = (u32 *) (stars + count);
    starsAddress = &gOverlay15Stars;
    colorsAddress = &gOverlay15StarColors;
    countAddress = &gOverlay15StarCount;
    xRange <<= 7;
    yRange <<= 7;
    zRange <<= 7;
    *starsAddress = stars;
    *colorsAddress = colors;
    *countAddress = count;
    gOverlay15InitBounds.zero = 0;

    if (count > 0) {
        startR = (startColor >> 24) & 0xFF;
        startG = (startColor >> 16) & 0xFF;
        startB = (startColor >> 8) & 0xFF;
        startA = startColor & 0xFF;
        deltaR = ((endColor >> 24) & 0xFF) - startR;
        deltaG = ((endColor >> 16) & 0xFF) - startG;
        deltaB = ((endColor >> 8) & 0xFF) - startB;
        deltaA = (endColor & 0xFF) - startA;

        do {
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
            i++;
            stars++;
            colors++;
        } while (i < gOverlay15StarCount);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F00006E8_1872A80.s")
#endif

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative.
 * Plateau: canonical C is 110/103 words (+28 bytes), with 84 differing from
 * +0x30; scalar BSS symbols retain seven HIs after pair, flag, and permuter sweeps. */
#ifdef NON_MATCHING
void overlay15UpdateMovingStars(f32 positionX, f32 positionY, f32 positionZ,
                                s32 updateRate) {
    Overlay15MovingStarCamera *camera;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 scale;

    camera = overlay15GetActiveCameraReloc();
    if (gOverlay15CameraReadyRead != 0 && updateRate != 0) {
        scale = 1.0f / (f32)updateRate;
        deltaX = (gOverlay15PreviousCameraX - camera->x) * scale;
        deltaY = (gOverlay15PreviousCameraY - camera->y) * scale;
        deltaZ = (gOverlay15PreviousCameraZ - camera->z) * scale;
    } else {
        deltaX = 0.0f;
        deltaY = 0.0f;
        deltaZ = 0.0f;
    }

    gOverlay15PreviousCameraX = camera->x;
    gOverlay15PreviousCameraY = camera->y;
    gOverlay15PreviousCameraZ = camera->z;
    positionX += deltaX;
    positionY += deltaY;
    positionZ += deltaZ;
    gOverlay15CameraReadyWrite = 1;
    gOverlay15CurrentPositionX = positionX;
    gOverlay15CurrentPositionY = positionY;
    gOverlay15CurrentPositionZ = positionZ;

    if (gOverlay15MovingStars != 0) {
        scale = (f32)updateRate;
        positionX *= scale;
        positionZ *= scale;
        positionY *= scale;
        starfieldFastMove(gOverlay15MovingStarCount, gOverlay15MovingStars,
                          positionX, positionY, positionZ,
                          gOverlay15MovingBound0, gOverlay15MovingBound1,
                          gOverlay15MovingBound2, gOverlay15MovingBound3,
                          gOverlay15MovingBound4, gOverlay15MovingBound5,
                          gOverlay15MovingBound6, gOverlay15MovingBound7,
                          gOverlay15MovingBound8);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F00009E0_1872D78.s")
#endif

void overlay15SetValueC(s32 value) {
    gOverlay15ValueC = value;
}

void overlay15ClearValue7C(void) {
    gOverlay15Value7C = 0;
}

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
typedef struct Overlay15RainOffsets {
    u8 pad00[0x80];
    f32 x;
    f32 y;
    f32 z;
} Overlay15RainOffsets;

extern Overlay15RainOffsets gOverlay15RainOffsets;

/* Plateau (2026-08-25, ownership): linked full-BSS candidate is exact-size
 * 0xD8 with 13 differing words, first +0x74; the 119-flag lattice is flat.
 * Workbench: structure-mismatch; post-call load scheduling remains. */
#ifdef NON_MATCHING
void overlay15DrawRain(void *framebuffer, s32 width, s32 height,
                       f32 projectionScale, f32 intensity) {
    s32 visibleCount;
    Overlay15CameraState *camera;

    if (gOverlay15RainEnabled != 0) {
        visibleCount = (s32)((f32)gOverlay15RainCapacity * intensity);
        if ((gOverlay15RainPositions != 0) && (visibleCount > 0)) {
            camera = overlay15GetActiveCameraReloc();
            rainFastDraw(framebuffer, width, height, visibleCount,
                         gOverlay15RainPositions, gOverlay15RainColors,
                         camera->angle + 0x8000, gOverlay15RainOffsets.x,
                         gOverlay15RainOffsets.y, gOverlay15RainOffsets.z,
                         projectionScale);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o015/overlay_015/func_overlay_015_F0000B94_1872F2C.s")
#endif

/* PLATEAU-HANDOFF:overlay15DrawRain:start
 * symbol: overlay15DrawRain
 * score: 13/54 words
 * frame: 0x40
 * relocations: 17
 * first-mismatch: +0x74
 * summary: Current 17-draw census reproduces prior address-lowering plateau; existing differential covers the identified source routes.
 * PLATEAU-HANDOFF:overlay15DrawRain:end
 */

/* PLATEAU-HANDOFF:overlay15InitStars:start
 * symbol: overlay15InitStars
 * score: 89/190 words
 * frame: 0xb8
 * relocations: 15
 * first-mismatch: +0x7c
 * summary: Byte-offset normalization and folded setup stores are byte-inert; 74 draws and the existing spill/order residual remain.
 * PLATEAU-HANDOFF:overlay15InitStars:end
 */

/* PLATEAU-HANDOFF:overlay15InitStarsAndPalette:start
 * symbol: overlay15InitStarsAndPalette
 * score: 70/247 words
 * frame: 0x40
 * relocations: 14
 * first-mismatch: +0x4
 * summary: Counter reuse and measured store moves reach 198 aligned exact words; 70 masked remain at exact size/frame, with adjacent scheduling controls stalled.
 * PLATEAU-HANDOFF:overlay15InitStarsAndPalette:end
 */

/* PLATEAU-HANDOFF:overlay15UpdateMovingStars:start
 * symbol: overlay15UpdateMovingStars
 * score: 84/103 words
 * frame: 0x58
 * relocations: 46
 * first-mismatch: +0x30
 * summary: Inlining the trailing rate conversion removes only a location emission; 27 draws and the plus-28-byte address deficit remain.
 * PLATEAU-HANDOFF:overlay15UpdateMovingStars:end
 */

/* PLATEAU-HANDOFF:overlay15MoveStars:start
 * symbol: overlay15MoveStars
 * score: 30/54 words
 * frame: 0x40
 * relocations: 25
 * first-mismatch: +0x30
 * summary: Deleting scale removes only a location emission; fourteen draws, object text and shared-address lowering remain unchanged.
 * PLATEAU-HANDOFF:overlay15MoveStars:end
 */
