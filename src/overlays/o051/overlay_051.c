#include "overlays/overlay_051.h"

/*
 * Overlay 51, ADR 0006 consolidation: one translation unit in ROM order.
 * The pinned DKR v77/v80 and JFG scans found no exact donor for these
 * functions. The TU is built with -Wab,-r4300_mul (mk/overlays.mk) for the
 * HUD update's easing loop and clock-hand multiply; the other three
 * functions compile identically with and without it.
 */

void overlay51Initialize(void) {
    overlay51CreateReloc(gOverlay51Resource0);
    overlay51CreateReloc(gOverlay51Resource18);
    overlay51CreateReloc(4);
    overlay51CreateReloc(11);
    overlay51PrepareReloc(gOverlay51ClockGlyphs);
    overlay51PrepareReloc(gOverlay51TimeGlyphs);
    gOverlay51HudHeight = -80.0f;
    overlay51CreateReloc();
    gOverlay51Item = -1;
    gOverlay51Handle = overlay51CreateReloc();
}

void overlay51PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) gOverlay51Objects[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) gOverlay51Objects[entry->second];
        }
        entry++;
    }
}

/* Tier D: field widths and offsets from this function's loads and stores. */
typedef struct Overlay51Racer {
    u8 pad000[0x19A];
    u8 item;
    u8 pad19B;
    s32 itemState;
    u8 pad1A0[0x383 - 0x1A0];
    s8 laps;
    u8 pad384[0x400 - 0x384];
    s32 raceTime;
} Overlay51Racer;

typedef struct Overlay51Object {
    u8 pad00[0x64];
    Overlay51Racer *racer;
} Overlay51Object;

typedef struct Overlay51Level {
    u8 pad00[0x86];
    s8 laps;
} Overlay51Level;

/* Same 0x20-byte record as menu.c's MenuCurrentObject. */
typedef struct Overlay51MenuObject {
    s16 value00;
    s16 value02;
    s16 angle;
    s16 index;
    f32 value08;
    f32 x;
    f32 y;
    f32 value14;
    f32 frame;
    u8 pad1C[4];
} Overlay51MenuObject;

/* Tier B: callee identities decoded from overlay 51's runtime relocation
 * records (resident, overlay 56 +0xB8, overlay 59 +0x784/+0x84C). The
 * *_o051Reloc names are the generated relocation surface. */
u8 *func_80028F54_o051Reloc(void);
void camStandardOrtho_o051Reloc(void **, void **);
void overlay56SplitTime_o051Reloc(s32, s32 *, s32 *, s32 *);
Overlay51Level *levelGetLevel_o051Reloc(void);
s32 func_800290A0_o051Reloc(void);
void func_8002F618_o051Reloc(void **, void *, s32, s32, u8, u8, u8, u8);
void func_80034920_o051Reloc(void **);
void func_80039E34_o051Reloc(s32);
void freeFrontEndItem_o051Reloc(s32);
void loadFrontEndItem_o051Reloc(s32);
s32 overlay59Interpolate_o051Reloc(s32, s32, s32, s32, s32, s32 *, s32 *, s32);
void overlay59BuildList_o051Reloc(s32, void *);
s32 mainGetMode_o051Reloc(void);
void func_800016EC_o051Reloc(u8);
void func_8003A590_o051Reloc(void);
void func_80037414_o051Reloc(s32, f32, f32, s32, s32, s32, s32);
void mainChangeLevel_o051Reloc(s32, s32, s32, s32, s32, s32);
void func_800005CC_o051Reloc(f32, u8);

extern void *D_800D3140_o051Reloc;
extern void *D_800D3144_o051Reloc;
extern s32 D_800C947C_o051Reloc;
extern s16 D_8007C180_o051Reloc[];
extern Overlay51MenuObject D_800D3550_o051Reloc[];
extern s32 ext_o1_83e0_o051Reloc; /* overlay 1 +0x83E0 */

/* Overlay 51's single-player HUD update: the cut-down sibling of overlay 50's
 * func_overlay_050_F0000334_1896CA4, written the same way and matched as the
 * first draft of that copy. Kept from the sibling: plain for loops under
 * -Wab,-r4300_mul (the rotated easing loop and the negate-then-shift clock
 * hand), every local at function scope in the target's frame order (itemId's
 * home is the spill cell at 0xC0), the blink counter advanced after it is
 * added, and the item read from its global at every use. Removed: the
 * banner, lap-difference, speedometer and lap-time blocks, the position and
 * lap rows, the item-count badge and the frame callee. */
void func_overlay_051_F00000D0_18999D0(Overlay51Object *object, s32 updateRate) {
    s32 i;
    s32 messageX;
    s32 messageY;
    s32 itemId;
    Overlay51Glyph messageGlyphs[5];
    Overlay51Glyph itemGlyphs[2];
    Overlay51Level *level;
    Overlay51Racer *racer;
    u8 *modeFlag;
    s32 minutes;
    s32 seconds;
    s32 centiseconds;
    s32 hudY;

    modeFlag = func_80028F54_o051Reloc();
    if (object != NULL) {
        racer = object->racer;
        camStandardOrtho_o051Reloc(&D_800D3140_o051Reloc, &D_800D3144_o051Reloc);
        if (D_800C947C_o051Reloc == 0) {
            for (i = 0; i < updateRate; i++) {
                gOverlay51HudHeight += (0.0f - gOverlay51HudHeight) * 0.125f;
            }
        }
        hudY = (s32) gOverlay51HudHeight;
        if (*modeFlag == 6) {
            overlay56SplitTime_o051Reloc(racer->raceTime, &minutes, &seconds, &centiseconds);
            level = levelGetLevel_o051Reloc();
            if ((D_800C947C_o051Reloc == 0) && (racer->laps != level->laps) &&
                (func_800290A0_o051Reloc() == 0) && (racer->raceTime != 0x83D60)) {
                centiseconds -= centiseconds % 10;
                centiseconds += gOverlay51BlinkCounter;
                gOverlay51BlinkCounter++;
                gOverlay51BlinkCounter = (s8) ((s8) gOverlay51BlinkCounter % 10);
            }
            gOverlay51TimeGlyphs[0].glyph = (minutes / 10) << 0x10;
            gOverlay51TimeGlyphs[1].glyph = (minutes % 10) << 0x10;
            gOverlay51TimeGlyphs[3].glyph = (seconds / 10) << 0x10;
            gOverlay51TimeGlyphs[4].glyph = (seconds % 10) << 0x10;
            gOverlay51TimeGlyphs[6].glyph = (centiseconds / 10) << 0x10;
            gOverlay51TimeGlyphs[7].glyph = (centiseconds % 10) << 0x10;
            func_8002F618_o051Reloc(&D_800D3140_o051Reloc, gOverlay51TimeGlyphs, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            func_80034920_o051Reloc(&D_800D3140_o051Reloc);
            D_800D3550_o051Reloc[4].x = 42.0f;
            D_800D3550_o051Reloc[4].y = (f32) (0x54 - hudY);
            D_800D3550_o051Reloc[4].angle = (s16) ((s32) (racer->raceTime * -0x10000) / 300);
            func_80039E34_o051Reloc(4);
            func_8002F618_o051Reloc(&D_800D3140_o051Reloc, gOverlay51ClockGlyphs, 0, hudY, 0xFF, 0xFF, 0xFF, 0xFF);
            func_80034920_o051Reloc(&D_800D3140_o051Reloc);
        }
        if (racer->item != 0xFF) {
            gOverlay51ItemAlpha += updateRate * 16;
            if (gOverlay51ItemAlpha >= 256) {
                gOverlay51ItemAlpha = 255;
            }
        } else {
            gOverlay51ItemAlpha -= updateRate * 8;
            if (gOverlay51ItemAlpha < 0) {
                gOverlay51ItemAlpha = 0;
            }
        }
        if (gOverlay51ItemAlpha > 0) {
            if (racer->itemState != 0) {
                itemId = 0x35;
            } else if (racer->item != 0xFF) {
                itemId = D_8007C180_o051Reloc[racer->item];
            } else {
                itemId = gOverlay51Item;
            }
            if (itemId != gOverlay51Item) {
                if (gOverlay51Item != -1) {
                    freeFrontEndItem_o051Reloc(gOverlay51Item);
                }
                gOverlay51Item = itemId;
                if (gOverlay51Item != -1) {
                    loadFrontEndItem_o051Reloc(gOverlay51Item);
                }
            }
            if (gOverlay51Item != -1) {
                if (gOverlay51Item == 0x35) {
                    itemGlyphs[0].x = 0x8A;
                    itemGlyphs[0].y = 0xF;
                } else {
                    itemGlyphs[0].x = 0x90;
                    itemGlyphs[0].y = 0x15;
                }
                itemGlyphs[0].glyph = 0;
                itemGlyphs[0].alternate = 0;
                itemGlyphs[1].texture = 0;
                itemGlyphs[0].texture = gOverlay51Objects[gOverlay51Item];
                func_8002F618_o051Reloc(&D_800D3140_o051Reloc, itemGlyphs, 0, 0, 0xFF, 0xFF, 0xFF, gOverlay51ItemAlpha);
            }
        } else {
            if (gOverlay51Item != -1) {
                freeFrontEndItem_o051Reloc(gOverlay51Item);
                gOverlay51Item = -1;
            }
        }
        if (overlay59Interpolate_o051Reloc(0, -0x18, 0xBE, 0x30, 0xBE, &messageX, &messageY, 1) != 0) {
            overlay59BuildList_o051Reloc(0, messageGlyphs);
            if (messageGlyphs[0].texture != 0) {
                func_8002F618_o051Reloc(&D_800D3140_o051Reloc, messageGlyphs, messageX, messageY, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
        if ((mainGetMode_o051Reloc() == 0) && (*modeFlag == 5) && (ext_o1_83e0_o051Reloc == 0) &&
            (gOverlay51TransitionDone == 0)) {
            func_800016EC_o051Reloc(1);
            func_8003A590_o051Reloc();
            func_80037414_o051Reloc(2, 4.0f, -1.0f, 0, 0, 0, 0);
            mainChangeLevel_o051Reloc(0x12, 0, 0, 7, 1, 1);
            func_800005CC_o051Reloc(3.0f, 0);
            gOverlay51TransitionDone = 1;
        }
    }
}

void overlay51ReleaseState(void) {
    s32 index;

    overlay51ReleaseReloc(gOverlay51Resource0);
    overlay51FinalizeReloc();
    index = gOverlay51Item;
    if (index != -1) {
        overlay51ReleaseIndexReloc(index);
        gOverlay51Item = -1;
    }
}
