#include "overlays/overlay_060.h"

/* Tier B: the four rank labels reach fontPrintXY, whose converter reads
 * them into separate scratch; all six formats are read-only sprintf inputs.
 * Actual immutable backing is separate from the existing qualified imports
 * used by the host link. The retained pointer table still has its legacy
 * char* view and is not source-owned here.
 * Tier D: original declarations/names and source retention remain inferred.
 * The compiler supplies ordinary string and final section alignment. */
const char o60FirstPlaceLabel[] = "1st";
const char o60SecondPlaceLabel[] = "2nd";
const char o60ThirdPlaceLabel[] = "3rd";
const char o60EmptyPlaceLabel[] = " ";
const char o60ControlLabelFormat[] = "%s %s";
const char o60ControlValueFormat[] = "%s %s";
const char o60EmptyRecordFormat[] = "-----:--.--";
const char o60RecordFormat[] = "%c%c%c%2d:%02d.%02d";
const char o60StereoLabelFormat[] = "%s %s";
const char o60StereoValueFormat[] = "%s %s";

/* Tier B: the initializer writes ten/four/two actual character-pointer
 * caches; the updater reads those same cells and replaces menu slot seven.
 * Seven signed words hold opacity, label position, panel selection/scroll
 * and the menu path index. IDO supplies terminal section alignment only.
 * Tier D: original declarations/names and caller index bounds remain open.
 * The path word retains the existing big-endian byte-three read; this is
 * real module-lifetime storage, not complete native/header/API closure. */
char *o60MenuText[10];
char *o60StereoText[4];
char *o60ToggleText[2];
s32 o60MenuOpacity;
s32 o60PanelOpacity;
s32 o60MenuLabelY;
s32 o60ActivePanel;
s32 o60PanelSelection;
s32 o60PanelScroll;
s32 o60MenuPathIndex;

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static s32 func_overlay_060_F0002F54_18BCD2C(s32 left, s32 bottom, s32 width, s32 height, s32 progress, s32 alpha, s32 ticks);
static void overlay60DrawBorder(s32 left, s32 top, s32 right, s32 bottom);
static void overlay60DrawLine(s32 left, s32 top, s32 right, s32 bottom);
static void overlay60ReassignChoiceSlots(void);


/* Workbench: mixed structure/schedule/register, 17 words, first +0x108.
 * A fidelity-clean allocator trace could not attribute source webs or stack
 * homes, and coordinate carriers regressed the frame.
 *
 * 2026-09-10, lane c6-close: 21 -> 17 in two independent edits, both of them
 * ordinary C.
 *
 * The first is L59's structural form. The counter and the object cursor were
 * initialised on two statements ahead of a guarded `do`; moving BOTH into one
 * `for` header makes them share a source line, which is the tie-break as1
 * reaches last, and the two hoisted address materialisations then complete
 * before the counter is zeroed as they do in the target. That alone is 21 ->
 * 19. The bound was already register-resident, so this is not the counted-`for`
 * rewrite the earlier closure measured at 28 -- what pays here is the shared
 * line, not the loop shape: `i = 0, objects = ...;` on one statement before an
 * unchanged `do` scores 22, and putting the cursor first in the header scores
 * 27.
 *
 * The second is the descriptor store order, worth 19 -> 17. The residual is
 * schedule-sensitive to it: 23,649 orders of the nine loop-body statements
 * were measured and the floor is 17. The retained order is corroborated
 * independently of the search -- the target stores z, pad0C, alpha, pad0A in
 * that sequence, which is the contiguous block every 17-scoring order carries,
 * and one of objectId/size has to sit last. Nine orders reach 17; this is the
 * one closest to declaration order.
 *
 * 2026-09-11, lane f9-audit: 17 -> 0, byte-exact. The closure above held ONE
 * variable fixed while it varied the rest: the coordinate index was always
 * spelled as an integer add on a `(u32)` cast (or `i * 2` in the indexed
 * form, which is -4 bytes). The ring told the story: with objectId and size
 * stored first the target's coordinate shift takes the ring slot AFTER the one
 * the candidate gives it, so the target spends one pop that emits nothing
 * between the size store and the address. `&gOverlay60CoordsD8[i << 1]` is
 * that pop -- the doubled index folds into the one shift and still costs a
 * temp (L65's phantom, on a shift rather than a mask). With the ring phase
 * right, the named `inner` local takes v1 at save 20 / nocs 1 with no frame
 * growth, which is the "one more coloured pool web" the closure had correctly
 * seen and wrongly refuted: every refutation named `inner` while the phase was
 * still one behind, so the tail shifted and read as a 71-84 word regression.
 * The 0x68 frames it measured came from that same `(u32)` address spelling,
 * not from the sixth declaration. */
void func_overlay_060_F0000000_18B9DD8(void) {
    Overlay60Object **objects;
    Overlay60SpawnDesc desc;
    Overlay60Object *object;
    s16 *coordinate;
    Overlay60Inner *inner;
    s32 i;

    o60MenuText_o060Reloc[0] = D_8007C0B8_o060Reloc[0x44 / 4];
    o60MenuText_o060Reloc[1] = D_8007C0B8_o060Reloc[0x1C8 / 4];
    o60MenuText_o060Reloc[2] = D_8007C0B8_o060Reloc[0x1E4 / 4];
    o60MenuText_o060Reloc[3] = D_8007C0B8_o060Reloc[0x48 / 4];
    o60MenuText_o060Reloc[4] = D_8007C0B8_o060Reloc[0x40 / 4];
    o60MenuText_o060Reloc[5] = D_8007C0B8_o060Reloc[0x3C / 4];
    o60MenuText_o060Reloc[6] = D_8007C0B8_o060Reloc[0x4C / 4];
    o60MenuText_o060Reloc[7] = D_8007C0B8_o060Reloc[0x278 / 4];
    o60MenuText_o060Reloc[8] = D_8007C0B8_o060Reloc[0x1CC / 4];
    o60MenuText_o060Reloc[9] = D_8007C0B8_o060Reloc[0x1D0 / 4];

    o60StereoText_o060Reloc[0] = D_8007C0B8_o060Reloc[0x00 / 4];
    o60StereoText_o060Reloc[1] = D_8007C0B8_o060Reloc[0x04 / 4];
    o60StereoText_o060Reloc[2] = D_8007C0B8_o060Reloc[0x08 / 4];
    o60StereoText_o060Reloc[3] = D_8007C0B8_o060Reloc[0x0C / 4];
    o60ToggleText_o060Reloc[0] = D_8007C0B8_o060Reloc[0x1C / 4];
    o60ToggleText_o060Reloc[1] = D_8007C0B8_o060Reloc[0x18 / 4];

    o60MenuOpacity_o060Reloc = 0;
    o60PanelOpacity_o060Reloc = 0;
    o60MenuLabelY_o060Reloc = 0x9B;
    o60ActivePanel_o060Reloc = -1;

    for (i = 0, objects = &gOverlay60ObjectC8; i < 4; i++, objects++) {
        desc.objectId = 0x138;
        desc.size = 0xE;
        coordinate = &gOverlay60CoordsD8[i << 1];
        desc.x = coordinate[0];
        desc.y = coordinate[1];
        desc.z = 0;
        desc.pad0C = 0;
        desc.alpha = 0x80;
        desc.pad0A = 0;
        object = func_8000590C(&desc, 0);
        *objects = object;
        inner = *object->inner;
        inner->mode = 2;
        func_8005AD64(*objects, 0, 0, 0.0f);
    }

    loadFrontEndList((s16 *)gOverlay60Bss00);
    gOverlay60Bss20 = *(u32 *)(D_800D31C8 + 0x1E4);
    gOverlay60Bss40 = *(u32 *)(D_800D31C8 + 0x0C);
    gOverlay60Bss60 = *(u32 *)(D_800D31C8 + 0x144);
    gOverlay60Bss80 = *(u32 *)(D_800D31C8 + 0x1EC);

    i = frontGetScreenMode();
    if (i == 1) {
        o60MenuPathIndex_o060Reloc = 4;
        gOverlay60ObjectC8->x = (f32)gOverlay60CoordE8;
        gOverlay60ObjectC8->y = (f32)gOverlay60CoordEA;
        gOverlay60ObjectCC->x = (f32)gOverlay60CoordEC;
        gOverlay60ObjectCCY->y = (f32)gOverlay60CoordEE;
        gOverlay60ObjectD0->x = (f32)gOverlay60CoordF0;
        gOverlay60ObjectD0Y->y = (f32)gOverlay60CoordF2;
        gOverlay60ObjectD4->x = (f32)gOverlay60CoordF4;
        gOverlay60ObjectD4Y->y = (f32)gOverlay60CoordF6;
    } else {
        o60MenuPathIndex_o060Reloc = 0;
    }

    animseqStartPath(((u8 *)&o60MenuPathIndex_o060Reloc)[3]);
    object = (Overlay60Object *)func_800508B4(((u8 *)&o60MenuPathIndex_o060Reloc)[3]);
    if (object != 0) {
        object->flags |= 2;
    }
    o60CharacterSprite_o060Reloc = 0;
}

/* Tier B: the call sites and runtime relocation records identify these
 * resident interfaces. The local views below use Mickey's menu, save-record,
 * and RCP field layouts; unknown fields retain their offsets. */  /* (was on typedef RcpTextureInfo) */
/* Resident globals reached through the module's runtime relocation table.
 * Each is spelled with a per-module placeholder: the runtime SYMBOL records
 * that bind these sites store small section-relative addends, so the linker
 * must value each name from those stored addends rather than from the
 * resident definition (see docs/reloc-surface.md). The resident identity is
 * the address in the name; D_800D3128 is the settings block. */  /* (was on extern D_8007BF14_o060Reloc) */
/* Tier B/D: the shared menu view names language and stereo mode. The
 * remaining fields below are identified by their bit positions: their reads
 * and byte-preserving writes belong to the same resident settings word. */  /* (was on typedef Overlay60SettingsBits) */
/* Tier B: overlay-local identities come from overlay 60's LOCAL records.
 * Data and BSS are distinct even where their stored addends coincide. */  /* (was on extern gOverlay60Data020) */
/* Tier B: the sixteen-byte display list at data +0xB0. The runtime table
 * binds both halves of its address as LOCAL data records, and the shipped
 * words carry the physical-address bias (the module's KSEG0 base plus
 * 0x80000000 wraps to its physical address), so the DMA command takes the
 * list's address plus that bias, folded into the relocation addend. */  /* (was on extern gOverlay60Data0B0) */
#define O60_TEXT(offset) D_8007C0B8_o060Reloc[(offset) / 4]

/* Tier B/D: menu update and drawing reconstructed from Mickey's call graph,
 * field accesses, and ten-case dispatch. Matched: linked byte-identical. */
/* Declaration order and variable identity below are load-bearing.
 *
 * Tier D (frame census): IDO reserves a home for every declared local in
 * declaration order from the top of the local block down, so the list's
 * length and order fix each home. The target's homes are panel 404,
 * first 400, end 396, the spilled case-3 counter 372, minutes/seconds/
 * hundredths 348/344/340, enabled 316, text 188 and glyph 180, which
 * requires exactly fourteen scalar locals above `minutes` with the counter
 * ninth. `spare` is the preview panel's rank index: spelled as an index
 * into the rank table (`gOverlay60Data12C[spare - 1]`, `spare != 0`) uopt
 * keeps the table address plus index live across the two calls and
 * rematerialises the folded `data + 0x12C` constant at the compare, exactly
 * as the target; a pointer spelling makes the table address a CSE web that
 * takes s5 from `previewMode` (97 words). `row` also carries the wide-adjust
 * value in the screen-mode panel (a0, no copy). The records panel advances
 * the save block as byte arithmetic on the named blur index (`slots + i * 32`
 * gives the base-first addu; a scaled pointer add puts the index first) and
 * walks `record` as an explicit induction pointer beside `row`.
 *
 * Tier B (spill homes, uopt spilltemps): the records panel's name-table
 * pointer is spilled to 96(sp) and the hoisted (f32)ticks to 80(sp). uopt
 * hands its register temps frame slots in web order, reusing the lowest
 * slot of the same size not held by an interfering earlier temp, below the
 * block cfe laid out for locals and its own call-result temporaries. Two
 * spellings fix both homes: the nested volume getters land in `spare`
 * first (a nested u16 call result would cost cfe a halfword temporary
 * below the locals and push every slot down by four), and the preview
 * panel reads `gOverlay60Data0C8[i]` directly rather than through
 * `object` (the common subexpression is then a compiler temp that takes
 * a slot of its own, which is what keeps the float below it at 80).
 *
 * Tier B (register colouring, uopt priority allocator): the target keeps
 * &gOverlay60Data0A8 in s2 for the whole function, never promotes the
 * settings word, and leaves the case-3 counter in v1 spilled to 372(sp).
 * That colouring is reproduced only when (1) every settings access is one
 * object at D_800D3128_o060Reloc (the runtime relocation table resolves all 28 sites,
 * including +0x13 and +0x14, to that symbol), (2) the loop index and the
 * case-2/case-6 value are one variable `i`, and (3) `count` is also the
 * glyph-loop index in the records panel. Separating any of them gives the
 * settings word a saved register and shreds the menu-owner address into
 * 24 rematerialisations. */
void func_overlay_060_F0000334_18BA10C(s32 ticks) {
    s32 panel;
    s32 first;
    s32 end;
    ControlActor **objects;
    ControlActor *menuObject;
    ControlActor *object;
    s32 i;
    s32 spare;
    s32 count;
    s32 stereoMode;
    s32 screenMode;
    s32 previewMode;
    s32 showArrows;
    s32 limit;
    s32 minutes;
    s32 seconds;
    s32 hundredths;
    s32 row;
    s32 y;
    u8 enabled[16];
    char text[128];
    s32 icon;
    char glyph[2];
    s32 initial0;
    s32 initial1;
    SavesSlot *slot;
    SavesSlot *slots;
    SavesPackedEntry *record;
    MtxF *projection;
    AnimPath *path;
    MenuSpawnInner *model;
    s32 left;
    s32 right;

    if (gOverlay60Data0A8 == NULL) {
        objects = func_8000572C(&first, &end);
        while (first < end) {
            menuObject = objects[first++];
            if (menuObject->kind == 0x54) {
                gOverlay60Data0A8 = menuObject;
                first = end;
            }
        }
    }
    if (o60SettingsPending_o060Reloc != 0) {
        frontSetLanguage(D_800D3128_o060Reloc.bits.language);
        frontSetStereoMode(D_800D3128_o060Reloc.bits.stereoMode);
        frontSetSfxVolume(D_800D3128_o060Reloc.sfxVolume);
        frontSetBgmVolume(D_800D3128_o060Reloc.bgmVolume);
        o60SettingsPending_o060Reloc = 0;
    }
    if (gOverlay60Data0A8 != NULL &&
        overlay41IsUnitScale(o60MenuPathIndex_o060Reloc) != 0) {
        fontUseFont(2);
        o60MenuOpacity_o060Reloc += ticks * 2;
        if (o60MenuOpacity_o060Reloc >= 0x100) {
            o60MenuOpacity_o060Reloc = 0xFF;
        }
        fontColour(0, 0xFF, 0, 0xFF, o60MenuOpacity_o060Reloc);
        panel = overlay82GetSelection(gOverlay60Data0A8);
        if (o60ActivePanel_o060Reloc == -1 &&
            overlay82IsActive(gOverlay60Data0A8) != 0) {
            o60ActivePanel_o060Reloc = panel;
            o60SubmenuState_o060Reloc = 0;
            o60PanelSelection_o060Reloc = 0;
            o60PanelScroll_o060Reloc = 0;
            o60PanelValue_o060Reloc = 0;
            o60PanelPhase_o060Reloc = 1;
            o60VideoTransition_o060Reloc = 0;
        }
        if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
            panel == o60ActivePanel_o060Reloc) {
            o60PanelOpacity_o060Reloc += ticks * 8;
            if (o60PanelOpacity_o060Reloc >= 0x100) {
                o60PanelOpacity_o060Reloc = 0xFF;
            }
        } else {
            o60PanelOpacity_o060Reloc -= ticks * 8;
            if (o60PanelOpacity_o060Reloc < 0) {
                o60PanelOpacity_o060Reloc = 0;
                o60ActivePanel_o060Reloc = -1;
            }
        }
        if (overlay82IsActive(gOverlay60Data0A8) != 0) {
            o60MenuLabelY_o060Reloc -= ticks * 4;
            if (o60MenuLabelY_o060Reloc < 0x69) {
                o60MenuLabelY_o060Reloc = 0x69;
            }
        } else {
            o60MenuLabelY_o060Reloc += ticks * 4;
            if (o60MenuLabelY_o060Reloc >= 0x9C) {
                o60MenuLabelY_o060Reloc = 0x9B;
            }
        }
        if (o60PanelOpacity_o060Reloc != 0) {
            switch (o60ActivePanel_o060Reloc) {
            case 0:
                if (o60PreviewSound_o060Reloc == NULL) {
                    amSndPlay(o60PreviewSoundIds_o060Reloc[mathRnd(0, 9)],
                              &o60PreviewSound_o060Reloc);
                }
                spare = frontGetSfxVolume();
                frontSetSfxVolume(func_overlay_060_F0002F54_18BCD2C(
                    0x5F, 0xB9, 0x64, 0x28, spare, o60PanelOpacity_o060Reloc, ticks));
                break;
            case 1:
                if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        if (o60PanelSelection_o060Reloc == 0) {
                            o60PanelSelection_o060Reloc = 1;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        if (o60PanelSelection_o060Reloc == 1) {
                            o60PanelSelection_o060Reloc = 0;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    }
                }
                if (o60PanelSelection_o060Reloc == 0) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field9 = (D_800D3128_o060Reloc.bits.field9 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                }
                sprintf(text, o60ControlLabelFormat_o060Reloc, O60_TEXT(0x254),
                        o60ToggleText_o060Reloc[D_800D3128_o060Reloc.bits.field9]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x96, text, 0xC);
                if (o60PanelSelection_o060Reloc == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field8 = (D_800D3128_o060Reloc.bits.field8 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                }
                sprintf(text, o60ControlValueFormat_o060Reloc, O60_TEXT(0x258),
                        o60ToggleText_o060Reloc[D_800D3128_o060Reloc.bits.field8]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAA, text, 0xC);
                break;
            case 2:
                i = frontGet2PlayerSplit();
                if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                    if (D_800D31BC_o060Reloc != 0) {
                        i ^= 1;
                        frontSet2PlayerSplit(i);
                        amSndPlay(0xF, NULL);
                    }
                }
                fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B,
                              D_8007C0B8_o060Reloc[0x1E8 / 4 + i], 0xC);
                break;
            case 3:
                for (i = 0, count = 0; i < 16; i++) {
                    if (D_800D3128_o060Reloc.enabledMask & (1 << i)) {
                        enabled[count++] = i;
                    }
                }
                if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
                    count > 0) {
                    if (D_800D31BE_o060Reloc > 0 && o60PanelSelection_o060Reloc > 0) {
                        o60PanelSelection_o060Reloc--;
                        if (o60PanelSelection_o060Reloc < o60PanelScroll_o060Reloc) {
                            o60PanelScroll_o060Reloc = o60PanelSelection_o060Reloc;
                        }
                        amSndPlay(0xF, NULL);
                    } else if (D_800D31BE_o060Reloc < 0 && o60PanelSelection_o060Reloc < count - 1) {
                        o60PanelSelection_o060Reloc++;
                        if (o60PanelScroll_o060Reloc < o60PanelSelection_o060Reloc - 5) {
                            o60PanelScroll_o060Reloc = o60PanelSelection_o060Reloc - 5;
                        }
                        amSndPlay(0xF, NULL);
                    } else if (D_800D31BC_o060Reloc != 0) {
                        D_8007BF1C_o060Reloc ^= 1 << enabled[o60PanelSelection_o060Reloc];
                        amSndPlay(0xF, NULL);
                    }
                }
                y = 0x8C;
                for (i = o60PanelScroll_o060Reloc;
                     i < o60PanelScroll_o060Reloc + 6 && i < count; i++) {
                    if (i == o60PanelSelection_o060Reloc) {
                        fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x4D, y,
                                  D_8007C0B8_o060Reloc[0x2A4 / 4 + enabled[i]], 8);
                    fontPrintXY(&D_800D3140_o060Reloc, 0xC1, y,
                        o60ToggleText_o060Reloc[(D_8007BF1C_o060Reloc >> enabled[i]) & 1], 8);
                    y += 0xC;
                }
                break;
            case 4:
                spare = frontGetBgmVolume();
                frontSetBgmVolume(func_overlay_060_F0002F54_18BCD2C(
                    0x5F, 0xB9, 0x64, 0x28, spare, o60PanelOpacity_o060Reloc, ticks));
                if (D_8007BF1C_o060Reloc & 0x80) {
                    previewMode = o60PanelValue_o060Reloc;
                    if (previewMode > 0) {
                        o60PanelValue_o060Reloc = previewMode - ticks;
                        if (o60PanelValue_o060Reloc <= 0) {
                            o60PanelValue_o060Reloc = 0;
                            amTunePlay(((u8 *)&o60PreviewTune_o060Reloc)[3]);
                        }
                    }
                    if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                        if ((D_800D31B8_o060Reloc & 4) && o60PanelSelection_o060Reloc > 0) {
                            o60PanelSelection_o060Reloc--;
                            amSndPlay(0xF, NULL);
                        } else if ((D_800D31B8_o060Reloc & 8) && o60PanelSelection_o060Reloc < 0x2A) {
                            o60PanelSelection_o060Reloc++;
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31B8_o060Reloc & 0x9000) {
                            if (o60PanelValue_o060Reloc == 0) {
                                amTuneSetFade(0.5f, 0);
                                o60PanelValue_o060Reloc = 0x1E;
                            }
                            o60PreviewTune_o060Reloc = o60PanelSelection_o060Reloc + 1;
                            amSndPlay(0xC, NULL);
                        }
                    }
                    sprintf(text, O60_TEXT(0x2C8), o60PanelSelection_o060Reloc + 1);
                    fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x7F, text, 0xC);
                    fontColour(0xFF, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x75, gOverlay60Data0C0, 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x89, gOverlay60Data0C4, 0xC);
                }
                break;
            case 5:
                stereoMode = frontGetStereoMode();
                if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                    if (D_800D31BC_o060Reloc < 0) {
                        stereoMode--;
                        if (stereoMode < 0) {
                            stereoMode = 3;
                        }
                        amSndPlay(0xF, NULL);
                    } else if (D_800D31BC_o060Reloc > 0) {
                        stereoMode++;
                        if (stereoMode >= 4) {
                            stereoMode = 0;
                        }
                        amSndPlay(0xF, NULL);
                    }
                }
                frontSetStereoMode(stereoMode);
                if (stereoMode == 3) {
                    func_8002F618(&D_800D3140_o060Reloc, &gOverlay60Data020,
                                  0x68, 0x8C, 0, 0xFF, 0, o60PanelOpacity_o060Reloc);
                    fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xB9, O60_TEXT(0x298), 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xC3, O60_TEXT(0x29C), 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xCD, O60_TEXT(0x2A0), 0xC);
                } else {
                    fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, o60StereoText_o060Reloc[stereoMode], 0xC);
                }
                break;
            case 6:
                screenMode = frontGetScreenMode();
                if (o60VideoTransition_o060Reloc == 0 &&
                    overlay82IsActive(gOverlay60Data0A8) != 0 &&
                    D_800D31BC_o060Reloc != 0) {
                    overlay82Disable(gOverlay60Data0A8);
                    o60VideoTransition_o060Reloc = 1;
                    amSndPlay(0xF, NULL);
                }
                if (o60VideoTransition_o060Reloc != 0) {
                    if (o60VideoTransition_o060Reloc == 1) {
                        if (viDisplayingScreen0() == 0) {
                            projection = func_8002468C();
                            animseqStopPath(((u8 *)&o60MenuPathIndex_o060Reloc)[3]);
                            if (screenMode == 0) {
                                animseqStopPath(2);
                                animseqStartPath(3);
                                path = func_800508B4(3);
                                if (path != NULL) {
                                    path->flags |= 2;
                                }
                                for (i = 0; i < 4; i++) {
                                    gOverlay60Data0C8[i]->x = gOverlay60Data0E8[i].x;
                                    gOverlay60Data0C8[i]->y = gOverlay60Data0E8[i].y;
                                }
                                (*projection)[0][0] *= 0.75f;
                            } else {
                                animseqStopPath(3);
                                animseqStartPath(2);
                                path = func_800508B4(2);
                                if (path != NULL) {
                                    path->flags |= 2;
                                }
                                for (i = 0; i < 4; i++) {
                                    gOverlay60Data0C8[i]->x = gOverlay60Data0D8[i].x;
                                    gOverlay60Data0C8[i]->y = gOverlay60Data0D8[i].y;
                                }
                                (*projection)[0][0] /= 0.75f;
                            }
                            o60VideoTransition_o060Reloc = 2;
                        }
                    } else {
                        if (screenMode == 0) {
                            screenMode = 1;
                        } else {
                            screenMode = 0;
                        }
                        viNoClear();
                        D_800D2FC0_o060Reloc = 1;
                        viChangeMode(screenMode);
                        camSetFOV(60.0f, 1);
                        o60VideoTransition_o060Reloc = 0;
                        overlay82Enable(gOverlay60Data0A8);
                    }
                }
                if (screenMode == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x91, O60_TEXT(0x1DC), 0xC);
                    row = frontGetWideAdjust();
                    if (D_800D31B4_o060Reloc & 8) {
                        row--;
                    }
                    if (D_800D31B4_o060Reloc & 4) {
                        row++;
                    }
                    frontSetWideAdjust(row);
                    fontColour(0xFF, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x6A, 0xB4, gOverlay60Data0C0, 4);
                    fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x97, 0xB4, O60_TEXT(0x34), 4);
                    fontColour(0xFF, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x6A, 0xC3, gOverlay60Data0C4, 4);
                    fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x97, 0xC3, O60_TEXT(0x38), 4);
                } else {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, O60_TEXT(0x1E0), 0xC);
                }
                frontSetScreenMode(screenMode);
                break;
            case 7:
                showArrows = 0;
                if (o60SubmenuState_o060Reloc == 0) {
                    overlay82Enable(gOverlay60Data0A8);
                    if (o60PanelPhase_o060Reloc == 0 &&
                        overlay82IsActive(gOverlay60Data0A8) != 0) {
                        if (D_800D31B8_o060Reloc & 0x9000) {
                            o60SubmenuState_o060Reloc = 1;
                            o60PanelValue_o060Reloc = 0;
                            overlay82Disable(gOverlay60Data0A8);
                            amSndPlay(0xC, NULL);
                        } else if (D_800D31BE_o060Reloc > 0 && o60PanelSelection_o060Reloc > 0) {
                            o60PanelSelection_o060Reloc--;
                            if (o60PanelSelection_o060Reloc == 2 && !(D_800D3128_o060Reloc.unlocked & 0x3F)) {
                                o60PanelSelection_o060Reloc = 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BE_o060Reloc < 0) {
                            if (o60PanelSelection_o060Reloc < 3) {
                                o60PanelSelection_o060Reloc++;
                                if (o60PanelSelection_o060Reloc == 2 && !(D_800D3128_o060Reloc.unlocked & 0x3F)) {
                                    o60PanelSelection_o060Reloc = 3;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        }
                    }
                    o60PanelPhase_o060Reloc = 0;
                    o60MenuText_o060Reloc[7] = O60_TEXT(0x278);
                    if (o60PanelSelection_o060Reloc == 0) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x8F, O60_TEXT(0x280), 0xC);
                    if (o60PanelSelection_o060Reloc == 1) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, O60_TEXT(0x288), 0xC);
                    if (o60PanelSelection_o060Reloc == 2) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                    } else if (D_800D3128_o060Reloc.unlocked != 0) {
                        fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0x90, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xA7, O60_TEXT(0x27C), 0xC);
                    if (o60PanelSelection_o060Reloc == 3) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xB3, O60_TEXT(0x28C), 0xC);
                } else {
                    switch (o60PanelSelection_o060Reloc) {
                    case 0:
                        previewMode = o60PanelValue_o060Reloc;
                        o60MenuText_o060Reloc[7] = D_8007C0B8_o060Reloc[D_8007C11C_o060Reloc[previewMode]];
                        if (((D_800D3128_o060Reloc.progress[0] & 0x1C0) >> 6) >= 3 &&
                            ((D_800D3128_o060Reloc.progress[1] & 0x1C0) >> 6) >= 3 &&
                            ((D_800D3128_o060Reloc.progress[2] & 0x1C0) >> 6) >= 3) {
                            if (D_800D3128_o060Reloc.bits.field26_23 == 0xF) {
                                if (D_800D3128_o060Reloc.bits.field18) {
                                    limit = 0x15;
                                } else {
                                    limit = 0x14;
                                }
                            } else {
                                limit = 0x10;
                            }
                        } else {
                            limit = 0xC;
                        }
                        if (D_800D31B8_o060Reloc & 0x4000) {
                            o60SubmenuState_o060Reloc = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            o60PanelValue_o060Reloc = previewMode - 1;
                            if (o60PanelValue_o060Reloc < 0) {
                                o60PanelValue_o060Reloc = limit - 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0) {
                            o60PanelValue_o060Reloc = previewMode + 1;
                            if (o60PanelValue_o060Reloc >= limit) {
                                o60PanelValue_o060Reloc = 0;
                            }
                            amSndPlay(0xF, NULL);
                        }
                        slots = func_800291C4();
                        i = levelGetBlurEffect(D_8007C0E8_o060Reloc[o60PanelValue_o060Reloc]);
                        slot = (SavesSlot *)((u8 *)slots + i * 32);
                        fontUseFont(2);
                        fontBackground(0, 0, 0, 0);
                        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x7A, O60_TEXT(0xD0), 4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xBA, O60_TEXT(0xD4), 4);
                        if (overlay68CheckKind(
                                D_8007C0E8_o060Reloc[o60PanelValue_o060Reloc]) != 0) {
                            /* 0.67f is a literal: its HI16/LO16 pair is a LOCAL record
                             * against the module's rodata, ahead of the texture
                             * address in the shipped table. */
                            func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data080,
                                81.0f, 115.0f, 0.67f, 0.67f, -2, 0);
                        }
                        fontColour(0xC0, 0xFF, 0, 0xFF, 0xFF);
                        for (row = 0, record = slot->records; row < 4; record++, row++) {
                            overlay56SplitTime(record->time,
                                &minutes, &seconds, &hundredths);
                            if (record->time == 0) {
                                sprintf(text, o60EmptyRecordFormat_o060Reloc);
                                icon = 0x4A;
                            } else {
                                initial0 = func_8003A700(record->initials[0]) & 0xFF;
                                initial1 = func_8003A700(record->initials[1]) & 0xFF;
                                sprintf(text, o60RecordFormat_o060Reloc, initial0, initial1,
                                    func_8003A700(record->initials[2]),
                                    minutes, seconds, hundredths);
                                icon = record->character + 0x51;
                            }
                            gOverlay60Data060.texture = D_800D31C8_o060Reloc[icon];
                            fontPrintXY(&D_800D3140_o060Reloc, o60RecordColumns_o060Reloc[0],
                                o60RecordRows_o060Reloc[row], gOverlay60Data1A4[row], 0xC);
                            func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data060,
                                o60RecordColumns_o060Reloc[1], o60RecordRows_o060Reloc[row], 0.5f, 0.5f, -1, 0);
                            for (count = 0; count < 11; count++) {
                                glyph[0] = text[count];
                                glyph[1] = '\0';
                                fontPrintXY(&D_800D3140_o060Reloc, o60RecordColumns_o060Reloc[count + 2],
                                    o60RecordRows_o060Reloc[row], glyph, 0xC);
                            }
                        }
                        showArrows = 1;
                        break;
                    case 1:
                        texDPInit(&D_800D3140_o060Reloc);
                        camStandardPersp(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc);
                        previewMode = o60PanelValue_o060Reloc;
                        o60MenuText_o060Reloc[7] = D_8007C0B8_o060Reloc[0x220 / 4 + previewMode];
                        if (((D_800D3128_o060Reloc.progress[0] & 0x1C0) >> 6) >= 3 &&
                            ((D_800D3128_o060Reloc.progress[1] & 0x1C0) >> 6) >= 3 &&
                            ((D_800D3128_o060Reloc.progress[2] & 0x1C0) >> 6) >= 3) {
                            if (D_800D3128_o060Reloc.bits.field26_23 == 0xF) {
                                limit = 5;
                            } else {
                                limit = 4;
                            }
                        } else {
                            limit = 3;
                        }
                        if (D_800D31B8_o060Reloc & 0x4000) {
                            o60SubmenuState_o060Reloc = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            o60PanelValue_o060Reloc = previewMode - 1;
                            if (o60PanelValue_o060Reloc < 0) {
                                o60PanelValue_o060Reloc = limit - 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0) {
                            o60PanelValue_o060Reloc = previewMode + 1;
                            if (o60PanelValue_o060Reloc >= limit) {
                                o60PanelValue_o060Reloc = 0;
                            }
                            amSndPlay(0xF, NULL);
                        }
                        previewMode = o60PanelValue_o060Reloc;
                        o60PreviewMode_o060Reloc = previewMode;
                        fontColour(0, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0x8C, O60_TEXT(0x234), 8);
                        fontColour(0xFF, 0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0x9B, O60_TEXT(0x238), 8);
                        fontColour(0xFF, 0xFF, 0, 0xFF, o60PanelOpacity_o060Reloc);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0xAA, O60_TEXT(0x23C), 8);
                        fontColour(0, 0xFF, 0xFF, 0xFF, o60PanelOpacity_o060Reloc);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0xB9, O60_TEXT(0x240), 8);
                        texDPTextureX(&D_800D3140_o060Reloc, 0, 0, 0);
                        gDma1p(D_800D3140_o060Reloc++, 7, (u8 *)gOverlay60Data0B0 + 0x80000000, 0x10, 2);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 0, 255, 0,
                                        o60PanelOpacity_o060Reloc);
                        overlay60DrawBorder(0x39, 0x86, 0x82, 0x92);
                        overlay60DrawBorder(0x91, 0x74, 0xAF, 0x9D);
                        overlay60DrawLine(0x82, 0x8D, 0x91, 0x8D);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 0, 0,
                                        o60PanelOpacity_o060Reloc);
                        overlay60DrawBorder(0x39, 0x95, 0x82, 0xA1);
                        overlay60DrawBorder(0xB5, 0x74, 0xD4, 0x9D);
                        overlay60DrawLine(0xC7, 0x9E, 0xC7, 0xA2);
                        overlay60DrawLine(0x82, 0xA1, 0xC7, 0xA1);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 255, 0,
                                        o60PanelOpacity_o060Reloc);
                        overlay60DrawBorder(0x39, 0xA4, 0x82, 0xB0);
                        overlay60DrawBorder(0x91, 0xA6, 0xAF, 0xCF);
                        overlay60DrawLine(0x82, 0xAB, 0x91, 0xAB);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 0, 255, 255,
                                        o60PanelOpacity_o060Reloc);
                        overlay60DrawBorder(0x39, 0xB3, 0x82, 0xBF);
                        overlay60DrawBorder(0xB5, 0xA6, 0xD4, 0xCF);
                        overlay60DrawLine(0xC5, 0xD0, 0xC5, 0xD3);
                        overlay60DrawLine(0x80, 0xD3, 0xC6, 0xD3);
                        overlay60DrawLine(0x80, 0xC0, 0x80, 0xD3);
                        texDPInit(&D_800D3140_o060Reloc);
                        rcpClearZBuffer(&D_800D3140_o060Reloc, 0x140, 0xF0, 0x8C, 0x64, 0xD7, 0xC8);
                        for (i = 0; i < 4; i++) {
                            previewMode = o60PanelValue_o060Reloc;
                            if (previewMode != gOverlay60Data0C8[i]->unk3A) {
                                gOverlay60Data0C8[i]->unk3A = previewMode;
                                func_8005AD64(gOverlay60Data0C8[i], 0, 0, 0.0f);
                            }
                            previewMode = o60PanelValue_o060Reloc;
                            gOverlay60Data0C8[i]->unk8 = gOverlay60Data0F8[previewMode];
                            model = (MenuSpawnInner *)(gOverlay60Data0C8[i])->unk68[o60PanelValue_o060Reloc];
                            model->mode = ticks;
                            spare = ((u32)(D_800D3128_o060Reloc.progress[o60PanelValue_o060Reloc] & gOverlay60Data10C[i]) >> gOverlay60Data11C[i]);
                            modSetTextureFrame(model, 0, gOverlay60Data12C[spare - 1] * 256);
                            func_8005ABA8(gOverlay60Data0C8[i], 0.003f, ticks);
                            if (spare != 0) {
                                (gOverlay60Data0C8[i])->alpha = o60PanelOpacity_o060Reloc;
                                func_80009E78(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc,
                                    &D_800D3148_o060Reloc, gOverlay60Data0C8[i]);
                            }
                        }
                        showArrows = 1;
                        break;
                    case 2:
                        o60MenuText_o060Reloc[7] = O60_TEXT(0x27C);
                        if (D_800D31B8_o060Reloc & 0x4000) {
                            o60SubmenuState_o060Reloc = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            previewMode = o60PanelValue_o060Reloc;
                            i = previewMode;
                            do {
                                i--;
                                if (i < 0) {
                                    i = 5;
                                }
                            } while (!(D_800D3128_o060Reloc.unlocked & (1 << i)));
                            if (i != previewMode) {
                                o60PanelValue_o060Reloc = i;
                                if (o60CharacterSprite_o060Reloc != NULL) {
                                    texFreeSprite(o60CharacterSprite_o060Reloc);
                                    o60CharacterSprite_o060Reloc = NULL;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        } else if (D_800D31BC_o060Reloc > 0) {
                            previewMode = o60PanelValue_o060Reloc;
                            i = previewMode;
                            do {
                                i++;
                                if (i >= 6) {
                                    i = 0;
                                }
                            } while (!(D_800D3128_o060Reloc.unlocked & (1 << i)));
                            if (i != previewMode) {
                                o60PanelValue_o060Reloc = i;
                                if (o60CharacterSprite_o060Reloc != NULL) {
                                    texFreeSprite(o60CharacterSprite_o060Reloc);
                                    o60CharacterSprite_o060Reloc = NULL;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        }
                        if (o60SubmenuState_o060Reloc != 0 && o60CharacterSprite_o060Reloc == NULL) {
                            o60CharacterSprite_o060Reloc = texLoadSprite(
                                o60CharacterSpriteIds_o060Reloc[o60PanelValue_o060Reloc], 0);
                        }
                        if (o60CharacterSprite_o060Reloc != NULL) {
                            camStandardOrtho(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc);
                            gDPPipeSync(D_800D3140_o060Reloc++);
                            gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 255, 255, 255);
                            gDPSetEnvColor(D_800D3140_o060Reloc++, 255, 255, 255, 0);
                            camDo2DSprite(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc, &D_800D3148_o060Reloc,
                                gOverlay60Data178, o60CharacterSprite_o060Reloc, 0, 0xFF);
                            if (o60SubmenuState_o060Reloc == 0) {
                                texFreeSprite(o60CharacterSprite_o060Reloc);
                                o60CharacterSprite_o060Reloc = NULL;
                            }
                        }
                        for (i = 0; i < 6; i++) {
                            if (D_800D3128_o060Reloc.unlocked & (1 << i)) {
                                showArrows++;
                            }
                        }
                        if (showArrows < 2) { showArrows = 0; }
                        break;
                    case 3:
                        o60MenuText_o060Reloc[7] = O60_TEXT(0x28C);
                        if (D_800D31B8_o060Reloc & 0x9000) {
                            if (o60PanelValue_o060Reloc != 0) {
                                o60SubmenuState_o060Reloc = 0;
                                o60SettingsPending_o060Reloc = 1;
                                overlay60ReassignChoiceSlots();
                                D_8007BF1C_o060Reloc = 0;
                                func_80029198();
                                amSndPlay(0xC, NULL);
                            } else {
                                o60SubmenuState_o060Reloc = 0;
                                amSndPlay(0xD, NULL);
                            }
                        } else if (D_800D31B8_o060Reloc & 0x4000) {
                            o60SubmenuState_o060Reloc = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0 && o60PanelValue_o060Reloc == 0) {
                            o60PanelValue_o060Reloc = 1;
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0 && o60PanelValue_o060Reloc != 0) {
                            o60PanelValue_o060Reloc = 0;
                            amSndPlay(0xF, NULL);
                        }
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x8F, O60_TEXT(0x290), 0xC);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x99, O60_TEXT(0x294), 0xC);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAD, O60_TEXT(0x54), 0xC);
                        if (o60PanelValue_o060Reloc != 0) {
                            fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                        } else {
                            fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                        }
                        fontPrintXY(&D_800D3140_o060Reloc, 0x71, 0xBC, O60_TEXT(0x58), 0xC);
                        if (o60PanelValue_o060Reloc == 0) {
                            fontColour(0x80, 0xFF, 0x80, 0xFF, o60PanelOpacity_o060Reloc);
                        } else {
                            fontColour(0, 0xD0, 0, 0xFF, o60PanelOpacity_o060Reloc);
                        }
                        fontPrintXY(&D_800D3140_o060Reloc, 0xB1, 0xBC, O60_TEXT(0x5C), 0xC);
                        break;
                    }
                }
                if (showArrows != 0) {
                    if (frontGetScreenMode() == 1) {
                        left = 0x2E;
                        right = 0xE4;
                    } else {
                        left = 0x24;
                        right = 0xF4;
                    }
                    func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data040, left,
                        155.0f, 1.0f, 1.0f, -2, 3);
                    func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data040, right,
                        155.0f, 1.0f, 1.0f, -2, 0x1003);
                    texAnimateTexSprite(gOverlay60Data040.texture, &gOverlay60Data0A0,
                        0xC, &gOverlay60Data0A4, ticks);
                    gOverlay60Data040.packedOffset = (s32)(gOverlay60Data0A4 * 65536.0f);
                }
                break;
            case 8:
                if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        if (o60PanelSelection_o060Reloc == 0) {
                            o60PanelSelection_o060Reloc = 1;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        if (o60PanelSelection_o060Reloc == 1) {
                            o60PanelSelection_o060Reloc = 0;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    }
                }
                if (o60PanelSelection_o060Reloc == 0) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field20 = (D_800D3128_o060Reloc.bits.field20 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                }
                sprintf(text, o60StereoLabelFormat_o060Reloc, O60_TEXT(0x1D4),
                    o60ToggleText_o060Reloc[D_800D3128_o060Reloc.bits.field20]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x96, text, 0xC);
                if (o60PanelSelection_o060Reloc == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    if (overlay82IsActive(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field19 = (D_800D3128_o060Reloc.bits.field19 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                }
                sprintf(text, o60StereoValueFormat_o060Reloc, O60_TEXT(0x1D8),
                    o60ToggleText_o060Reloc[D_800D3128_o060Reloc.bits.field19]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAA, text, 0xC);
                break;
            case 9:
                if (overlay82IsActive(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        o60PanelSelection_o060Reloc++;
                        if (o60PanelSelection_o060Reloc >= 0xC) {
                            o60PanelSelection_o060Reloc = 0xB;
                            amSndPlay(0xE, NULL);
                        } else {
                            amSndPlay(0xF, NULL);
                        }
                        if (o60PanelScroll_o060Reloc < o60PanelSelection_o060Reloc - 5) {
                            o60PanelScroll_o060Reloc++;
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        o60PanelSelection_o060Reloc--;
                        if (o60PanelSelection_o060Reloc < 0) {
                            o60PanelSelection_o060Reloc = 0;
                            amSndPlay(0xE, NULL);
                        } else {
                            amSndPlay(0xF, NULL);
                        }
                        if (o60PanelSelection_o060Reloc < o60PanelScroll_o060Reloc) {
                            o60PanelScroll_o060Reloc = o60PanelSelection_o060Reloc;
                        }
                    }
                    if (D_800D31BC_o060Reloc != 0) {
                        D_8007BF14_o060Reloc ^= 1 << (o60PanelSelection_o060Reloc + 2);
                        amSndPlay(0xF, NULL);
                    }
                }
                y = 0x8C;
                for (i = o60PanelScroll_o060Reloc; i < o60PanelScroll_o060Reloc + 6; i++) {
                    if (i == o60PanelSelection_o060Reloc) {
                        fontColour(0x64, 0xFF, 0x64, 0xFF, o60PanelOpacity_o060Reloc);
                    } else {
                        fontColour(0, 0xBE, 0, 0xFF, o60PanelOpacity_o060Reloc);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x4D, y,
                        D_8007C0B8_o060Reloc[0x1F0 / 4 + i], 8);
                    fontPrintXY(&D_800D3140_o060Reloc, 0xC1, y,
                        o60ToggleText_o060Reloc[(D_8007BF14_o060Reloc >> (i + 2)) & 1], 8);
                    y += 0xC;
                }
                break;
            }
        }
        if ((D_800D31B8_o060Reloc & 0x4000) && o60ActivePanel_o060Reloc == -1 &&
            o60ExitRequested_o060Reloc == 0) {
            o60ExitRequested_o060Reloc = 1;
            if (D_8007BF1C_o060Reloc & 0x40) {
                D_8007BF1C_o060Reloc &= ~0x40;
                overlay48Initialize();
            } else {
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
            }
            amSndPlay(0xD, NULL);
        }
        fontColour(0, 0xFF, 0, 0xFF, o60MenuOpacity_o060Reloc);
        fontPrintXY(&D_800D3140_o060Reloc, 0x91, o60MenuLabelY_o060Reloc,
            o60MenuText_o060Reloc[panel], 0xC);
    }
}

/* DKR v77/v80 and JFG contain only generic resource-cleanup idioms. */
void overlay60ReleaseResources(void) {
    void **resource;
    void **end;

    resource = gOverlay60Resources; end = gOverlay60ResourcesEnd;
    do {
        if (*resource != NULL) {
            overlay60ReleaseReloc(*resource);
            *resource = NULL;
        }
        resource++;
    } while (resource != end);

    if (o60CharacterSprite_o060Reloc != NULL) {
        overlay60ReleaseReloc(o60CharacterSprite_o060Reloc);
        o60CharacterSprite_o060Reloc = NULL;
    }
    overlay60ReleaseReloc(&gOverlay60PrimaryResource);
    func_8002917C();
}

/* Cross-overlay call through the loader's relocation surface: the shipped
 * word is the 0xF0000000 addend (a SYMBOL record, not a JUMP), so the
 * callee must be named by its o060 placeholder, as overlay60Prefix does. */  /* (was on extern overlay82IsActive_o060Reloc) */
/* One display-list append the way the SDK's packet macros write it: a
 * block-scoped pointer per expansion, on one physical line. */

#define O60_APPEND(a, b) { Overlay60Command *_g = D_800D3140++; _g->w0 = (a); _g->w1 = (b); }

/* The SDK's _SHIFTL: the mask is redundant on a u8 field and as1 folds it
 * into the lbu, but ugen has already drawn a ring temp for it (L149). */

#define OVERLAY60_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

/* Matched 2026-09-16 (lane s1-c), 222/222 words, frame 0xC0, from 84 in six
 * measured cycles, no colour forced. What each edit bought, measured alone:
 *  - the first append through the block-scoped macro (84 -> 78): the target
 *    loads three appends into v0 and the loop's last into v1, which one
 *    declared symbol web cannot do;
 *  - OVERLAY60_SHIFTL on the raw colour bytes (78 -> 19): each folded mask is
 *    a spent ring draw, and the target's arm-1 ring skips t2/t5/t8 exactly
 *    where the three byte reads sit; arm 2 needs it on red only;
 *  - no `offset` local, `(f32)(x - left)` strength-reduced in the loop
 *    (19 -> 7): x's web then outranks the display-list address web
 *    (61/3 against 62/3 with the declared offset) and takes s0;
 *  - `axis` declared and `topFloat` inlined (7 -> 0): every declared local,
 *    live or dead, takes a frame slot in declaration order, and the reduced
 *    form reserves one temp word more, so color's home at 0x9C needs six
 *    slots above it and one fewer below at frame 0xC0.
 */
static s32 func_overlay_060_F0002F54_18BCD2C(s32 left, s32 bottom, s32 width,
                                       s32 height, s32 progress, s32 alpha,
                                       s32 ticks) {
    s16 axis;
    Overlay60Command *command;
    s32 right;
    s32 x;
    s32 split;
    u32 top;
    Overlay60Color color;
    f32 widthFloat;
    f32 fraction;
    f32 verticalOffset;

    if (overlay82IsActive_o060Reloc(
            *(void **)(gOverlay60LocalDataReloc + 0xA8)) != 0) {
        axis = D_800D31C0;
        if (axis < -0x10) {
            progress -= ticks * 2;
            if (progress < 0) {
                progress = 0;
            }
        } else if (axis >= 0x11) {
            progress += ticks * 2;
            if (progress >= 0x101) {
                progress = 0x100;
            }
        }
    }

    right = left + width;
    texDPTextureX(&D_800D3140, 0, 0, 0);
    O60_APPEND(0x07020010, (u32)D_800000B0);

    x = left;
    if (left < right) {
        widthFloat = (f32)width;
        split = (s32)((f32)left +
                      widthFloat * ((f32)progress * 0.00390625f));
        do {
            fraction = (f32)(x - left) / widthFloat;
            verticalOffset = (f32)height * fraction;
            initColourCycle(&color, 0xB);
            updateColourCycle(&color, (s32)(fraction * 60.0f));

            if (x < split) {
                command = D_800D3140++;
                command->w0 = 0xFA000000;
                command->w1 = OVERLAY60_SHIFTL(color.red, 24, 8) |
                              OVERLAY60_SHIFTL(color.green, 16, 8) |
                              OVERLAY60_SHIFTL(color.blue, 8, 8) |
                              OVERLAY60_SHIFTL(alpha, 0, 8);
            } else {
                command = D_800D3140++;
                command->w0 = 0xFA000000;
                command->w1 = OVERLAY60_SHIFTL(color.red >> 1, 24, 8) |
                              OVERLAY60_SHIFTL(color.green >> 1, 16, 8) |
                              OVERLAY60_SHIFTL(color.blue >> 1, 8, 8) |
                              OVERLAY60_SHIFTL(alpha >> 1, 0, 8);
            }

            command = D_800D3140++;
            command->w0 = 0xF6000000 |
                          OVERLAY60_SHIFTL(x + 2, 14, 10) |
                          OVERLAY60_SHIFTL(bottom, 2, 10);
            top = (u32)((f32)bottom - verticalOffset);
            command->w1 = OVERLAY60_SHIFTL(x, 14, 10) |
                          OVERLAY60_SHIFTL(top, 2, 10);
            x += 4;
        } while (x < right);
    }
    return progress;
}

#define OVERLAY60_FILL_RECTANGLE(packet, upperLeftX, upperLeftY, lowerRightX, \
                                 lowerRightY)                              \
    {                                                                      \
        Overlay60Command *macroCommand = (Overlay60Command *)(packet);      \
        macroCommand->w0 = 0xF6000000 |                                    \
                           OVERLAY60_SHIFTL(lowerRightX, 14, 10) |          \
                           OVERLAY60_SHIFTL(lowerRightY, 2, 10);            \
        macroCommand->w1 = OVERLAY60_SHIFTL(upperLeftX, 14, 10) |           \
                           OVERLAY60_SHIFTL(upperLeftY, 2, 10);             \
    }

/* DKR v77/v80 and JFG have no exact donor; DKR borders.c confirms the idiom. */
static void overlay60DrawBorder(s32 left, s32 top, s32 right, s32 bottom) {
    OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, right,
                             top + 1);
    OVERLAY60_FILL_RECTANGLE(D_800D3140++, right, top, right + 1,
                             bottom + 1);
    OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, bottom, right,
                             bottom + 1);
    OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, left + 1,
                             bottom);
}

/* DKR v77/v80 and JFG have no exact donor; DKR borders.c confirms the idiom. */
static void overlay60DrawLine(s32 left, s32 top, s32 right, s32 bottom) {
    if (left == right) {
        OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, right + 1,
                                 bottom);
    } else {
        OVERLAY60_FILL_RECTANGLE(D_800D3140++, left, top, right,
                                 bottom + 1);
    }
}

/* The four-entry resident player block. Two pass names keep the two
 * address-materialization lifetimes; the pass-2 end sentinel is a distinct
 * symbol so the back-edge comparison keeps End first (overlay 57's form). */  /* (was on extern D_800D3058_o060Reloc) */
/* Matched 2026-09-17 (lane w8-o060), 35 -> 0 masked words at delta 0,
 * frame 0x20, eight relocations, unforced. L160: delete the walking choice
 * pointer and subscript the player block from a while-index so IDO generates
 * the cursor. available[14] holds the index home at the 0x20 frame. */
static void overlay60ReassignChoiceSlots(void) {
    s32 i;
    u8 available[14];

    i = 0;
    while (i < 10) {
        available[i] = 1;
        i++;
    }

    i = 0;
    do {
        if (D_800D3058_o060Reloc[i].active != 0) {
            available[D_800D3058_o060Reloc[i].slot & 0xF] = 0;
        }
        i++;
    } while (&D_800D3058_o060Reloc[i] < &D_800D3058_o060Reloc[4]);

    i = 0;
    do {
        if ((D_800D3058Pass2_o060Reloc[i].active != 0) && (D_800D3058Pass2_o060Reloc[i].slot >= 6)) {
            D_800D3058Pass2_o060Reloc[i].slot = 0;
            while (available[D_800D3058Pass2_o060Reloc[i].slot] == 0) {
                D_800D3058Pass2_o060Reloc[i].slot++;
            }
            available[D_800D3058Pass2_o060Reloc[i].slot] = 0;
        }
        i++;
    } while (D_800D3058Pass2End_o060Reloc != &D_800D3058Pass2_o060Reloc[i]);
}
