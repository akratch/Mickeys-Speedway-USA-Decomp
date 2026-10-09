#include "PR/ultratypes.h"
#include "game/menu.h"
#include "game/font.h"
#include "game/charControl.h"
#include "game/anim.h"
#include "game/math.h"
#include "game/gameVi.h"
#include "n_audio/mbi.h"
extern int sprintf(char *buffer, const char *format, ...);

/* Tier B: the call sites and runtime relocation records identify these
 * resident interfaces. The local views below use Mickey's menu, save-record,
 * and RCP field layouts; unknown fields retain their offsets. */
typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;
typedef struct SavesPackedEntry {
    s32 time;
    u8 initials[3];
    u8 character;
} SavesPackedEntry;
typedef struct SavesSlot { SavesPackedEntry records[4]; } SavesSlot;
typedef struct Overlay60Point {
    s16 x;
    s16 y;
} Overlay60Point;

typedef struct MenuSpawnInner {
    u8 pad00[8];
    s16 mode;
} MenuSpawnInner;

extern ControlActor **func_8000572C(s32 *start, s32 *end);
extern void amSndPlay(u16 sound, void **handle);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void amTunePlay(u8 sequence);
extern void amTuneSetFade(f32 fade, u8 volume);
extern void fontUseFont(s32 font);
extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);
extern void fontPrintXY(Gfx **commands, s32 x, s32 y, char *text, s32 flags);
extern void func_8002F618(Gfx **commands, RcpTextureNode *texture,
                        s32 x, s32 y, s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8002FB34(Gfx **commands, RcpTextureNode *texture,
                        f32 x, f32 y, f32 scaleX, f32 scaleY, s32 alpha, s32 flags);
extern void texAnimateTexSprite(void *texture, s32 *state, s32 speed, f32 *frame, s32 ticks);
extern void frontSet2PlayerSplit(s32 split);
extern s32 func_8003A700(u8 initial);
extern void viChangeMode(s32 screenMode);
extern void viNoClear(void);
extern void camSetFOV(f32 fov, s32 force);
extern MtxF *func_8002468C(void);
extern void func_80029198(void);
extern SavesSlot *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 level);
extern void texDPInit(Gfx **commands);
extern void texDPTextureX(Gfx **commands, s32 arg1, s32 arg2, s32 arg3);
extern void camStandardPersp(Gfx **commands, Mtx **matrices);
extern void camStandardOrtho(Gfx **commands, Mtx **matrices);
extern void rcpClearZBuffer(Gfx **commands, s32 width, s32 height,
                           s32 left, s32 top, s32 right, s32 bottom);
extern void func_8005AD64(ControlActor *object, s32 frame, s32 arg2, f32 value);
extern s32 func_8005ABA8(ControlActor *object, f32 scale, f32 ticks);
extern void modSetTextureFrame(MenuSpawnInner *model, s32 index, s32 frame);
extern void func_80009E78(Gfx **commands, Mtx **matrices,
                        void **vertices, ControlActor *object);
extern void *texLoadSprite(s32 asset, s32 flags);
extern void texFreeSprite(void *sprite);
extern void camDo2DSprite(Gfx **commands, Mtx **matrices,
                        void **vertices, void *transform, void *sprite,
                        s32 flags, s32 alpha);
extern void mainChangeLevel(s32 level, s32 entrance, s32 cutscene,
                            s32 arg3, s32 arg4, s32 arg5);
extern s32 func_overlay_041_F000124C_1888584(s32 index);
extern void func_overlay_048_F0000000_1895408(void);
extern void func_overlay_056_F00000B8_18A2E30(s32 time, s32 *minutes,
                                           s32 *seconds, s32 *hundredths);
extern s32 func_overlay_068_F000146C_18C85CC(s32 index);
extern s32 func_overlay_082_F0000498_18CF618(ControlActor *object);
extern s32 func_overlay_082_F00004A4_18CF624(ControlActor *object);
extern void func_overlay_082_F00004B0_18CF630(ControlActor *object);
extern void func_overlay_082_F00004C0_18CF640(ControlActor *object);
extern s32 func_overlay_060_F0002F54_18BCD2C(s32 left, s32 bottom, s32 width,
                                         s32 height, s32 value, s32 alpha, s32 ticks);
extern void overlay60DrawBorder(s32 left, s32 top, s32 right, s32 bottom);
extern void overlay60DrawLine(s32 x1, s32 y1, s32 x2, s32 y2);
extern void overlay60ReassignChoiceSlots(void);

/* Resident globals reached through the module's runtime relocation table.
 * Each is spelled with a per-module placeholder: the runtime SYMBOL records
 * that bind these sites store small section-relative addends, so the linker
 * must value each name from those stored addends rather than from the
 * resident definition (see docs/reloc-surface.md). The resident identity is
 * the address in the name; D_800D3128 is the settings block. */
extern u16 D_8007BF14_o060Reloc;
extern u16 D_8007BF1C_o060Reloc;
extern char **D_8007C0B8_o060Reloc;
extern s16 D_8007C0E8_o060Reloc[];
extern s16 D_8007C11C_o060Reloc[];
extern s32 D_800D2FC0_o060Reloc;
/* Tier B/D: the shared menu view names language and stereo mode. The
 * remaining fields below are identified by their bit positions: their reads
 * and byte-preserving writes belong to the same resident settings word. */
typedef struct Overlay60SettingsBits {
    u32 upper : 5;
    u32 field26_23 : 4;
    u32 stereoMode : 2;
    u32 field20 : 1;
    u32 field19 : 1;
    u32 field18 : 1;
    u32 gap17_16 : 2;
    u32 language : 6;
    u32 field9 : 1;
    u32 field8 : 1;
    u32 lower : 8;
} Overlay60SettingsBits;
typedef struct Overlay60Settings {
    Overlay60SettingsBits bits;
    u16 sfxVolume;
    u16 bgmVolume;
    u16 progress[5];
    u8 pad12;
    u8 unlocked;
    u16 enabledMask;
} Overlay60Settings;
extern Overlay60Settings D_800D3128_o060Reloc;
extern Gfx *D_800D3140_o060Reloc;
extern Mtx *D_800D3144_o060Reloc;
extern void *D_800D3148_o060Reloc;
extern s32 D_800D31B4_o060Reloc;
extern s32 D_800D31B8_o060Reloc;
extern s16 D_800D31BC_o060Reloc;
extern s16 D_800D31BE_o060Reloc;
extern RcpTextureInfo *D_800D31C8_o060Reloc[];

/* Tier B: overlay-local identities come from overlay 60's LOCAL records.
 * Data and BSS are distinct even where their stored addends coincide. */
extern RcpTextureNode gOverlay60Data020;
extern RcpTextureNode gOverlay60Data040;
extern RcpTextureNode gOverlay60Data060;
extern RcpTextureNode gOverlay60Data080;
extern s32 gOverlay60Data0A0;
extern f32 gOverlay60Data0A4;
extern ControlActor *gOverlay60Data0A8;
extern char gOverlay60Data0C0[];
extern char gOverlay60Data0C4[];
extern ControlActor *gOverlay60Data0C8[];
extern Overlay60Point gOverlay60Data0D8[];
extern Overlay60Point gOverlay60Data0E8[];
extern f32 gOverlay60Data0F8[];
extern s32 gOverlay60Data10C[];
extern s32 gOverlay60Data11C[];
extern s8 gOverlay60Data12B[];
extern s8 gOverlay60Data12C[];
extern s32 gOverlay60Data130;
extern void *gOverlay60Data134;
extern u16 gOverlay60Data138[];
extern s32 gOverlay60Data14C;
extern s32 gOverlay60Data150;
extern s32 gOverlay60Data154;
extern s32 gOverlay60Data158;
extern s32 gOverlay60Data15C;
extern s32 gOverlay60Data160;
extern s32 gOverlay60Data164;
extern s16 gOverlay60Data168[];
extern void *gOverlay60Data174;
extern u8 gOverlay60Data178[];
extern char *gOverlay60Data1A4[];
extern s16 gOverlay60Data1B4[];
extern s16 gOverlay60Data1D0[];
extern char gOverlay60Data1F0[];
extern char gOverlay60Data1F8[];
extern char gOverlay60Data200[];
extern char gOverlay60Data20C[];
extern char gOverlay60Data220[];
extern char gOverlay60Data228[];
extern f32 gOverlay60Data258;
extern char *gOverlay60Data260[];
extern char *gOverlay60Data288[];
extern char *gOverlay60Data298[];
extern s32 gOverlay60Data2A0;
extern s32 gOverlay60Data2A4;
extern s32 gOverlay60Data2A8;
extern s32 gOverlay60Data2AC;
extern s32 gOverlay60Data2B0;
extern s32 gOverlay60Data2B4;
extern s32 gOverlay60Data2B8;
/* Tier B: the sixteen-byte display list at data +0xB0. The runtime table
 * binds both halves of its address as LOCAL data records, and the shipped
 * words carry the physical-address bias (the module's KSEG0 base plus
 * 0x80000000 wraps to its physical address), so the DMA command takes the
 * list's address plus that bias, folded into the relocation addend. */
extern Gfx gOverlay60Data0B0[];

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
    if (gOverlay60Data160 != 0) {
        frontSetLanguage(D_800D3128_o060Reloc.bits.language);
        frontSetStereoMode(D_800D3128_o060Reloc.bits.stereoMode);
        frontSetSfxVolume(D_800D3128_o060Reloc.sfxVolume);
        frontSetBgmVolume(D_800D3128_o060Reloc.bgmVolume);
        gOverlay60Data160 = 0;
    }
    if (gOverlay60Data0A8 != NULL &&
        func_overlay_041_F000124C_1888584(gOverlay60Data2B8) != 0) {
        fontUseFont(2);
        gOverlay60Data2A0 += ticks * 2;
        if (gOverlay60Data2A0 >= 0x100) {
            gOverlay60Data2A0 = 0xFF;
        }
        fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A0);
        panel = func_overlay_082_F0000498_18CF618(gOverlay60Data0A8);
        if (gOverlay60Data2AC == -1 &&
            func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
            gOverlay60Data2AC = panel;
            gOverlay60Data14C = 0;
            gOverlay60Data2B0 = 0;
            gOverlay60Data2B4 = 0;
            gOverlay60Data150 = 0;
            gOverlay60Data154 = 1;
            gOverlay60Data158 = 0;
        }
        if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
            panel == gOverlay60Data2AC) {
            gOverlay60Data2A4 += ticks * 8;
            if (gOverlay60Data2A4 >= 0x100) {
                gOverlay60Data2A4 = 0xFF;
            }
        } else {
            gOverlay60Data2A4 -= ticks * 8;
            if (gOverlay60Data2A4 < 0) {
                gOverlay60Data2A4 = 0;
                gOverlay60Data2AC = -1;
            }
        }
        if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
            gOverlay60Data2A8 -= ticks * 4;
            if (gOverlay60Data2A8 < 0x69) {
                gOverlay60Data2A8 = 0x69;
            }
        } else {
            gOverlay60Data2A8 += ticks * 4;
            if (gOverlay60Data2A8 >= 0x9C) {
                gOverlay60Data2A8 = 0x9B;
            }
        }
        if (gOverlay60Data2A4 != 0) {
            switch (gOverlay60Data2AC) {
            case 0:
                if (gOverlay60Data134 == NULL) {
                    amSndPlay(gOverlay60Data138[mathRnd(0, 9)],
                              &gOverlay60Data134);
                }
                spare = frontGetSfxVolume();
                frontSetSfxVolume(func_overlay_060_F0002F54_18BCD2C(
                    0x5F, 0xB9, 0x64, 0x28, spare, gOverlay60Data2A4, ticks));
                break;
            case 1:
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        if (gOverlay60Data2B0 == 0) {
                            gOverlay60Data2B0 = 1;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        if (gOverlay60Data2B0 == 1) {
                            gOverlay60Data2B0 = 0;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    }
                }
                if (gOverlay60Data2B0 == 0) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field9 = (D_800D3128_o060Reloc.bits.field9 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                }
                sprintf(text, gOverlay60Data1F0, O60_TEXT(0x254),
                        gOverlay60Data298[D_800D3128_o060Reloc.bits.field9]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x96, text, 0xC);
                if (gOverlay60Data2B0 == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field8 = (D_800D3128_o060Reloc.bits.field8 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                }
                sprintf(text, gOverlay60Data1F8, O60_TEXT(0x258),
                        gOverlay60Data298[D_800D3128_o060Reloc.bits.field8]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAA, text, 0xC);
                break;
            case 2:
                i = frontGet2PlayerSplit();
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                    if (D_800D31BC_o060Reloc != 0) {
                        i ^= 1;
                        frontSet2PlayerSplit(i);
                        amSndPlay(0xF, NULL);
                    }
                }
                fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B,
                              D_8007C0B8_o060Reloc[0x1E8 / 4 + i], 0xC);
                break;
            case 3:
                for (i = 0, count = 0; i < 16; i++) {
                    if (D_800D3128_o060Reloc.enabledMask & (1 << i)) {
                        enabled[count++] = i;
                    }
                }
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                    count > 0) {
                    if (D_800D31BE_o060Reloc > 0 && gOverlay60Data2B0 > 0) {
                        gOverlay60Data2B0--;
                        if (gOverlay60Data2B0 < gOverlay60Data2B4) {
                            gOverlay60Data2B4 = gOverlay60Data2B0;
                        }
                        amSndPlay(0xF, NULL);
                    } else if (D_800D31BE_o060Reloc < 0 && gOverlay60Data2B0 < count - 1) {
                        gOverlay60Data2B0++;
                        if (gOverlay60Data2B4 < gOverlay60Data2B0 - 5) {
                            gOverlay60Data2B4 = gOverlay60Data2B0 - 5;
                        }
                        amSndPlay(0xF, NULL);
                    } else if (D_800D31BC_o060Reloc != 0) {
                        D_8007BF1C_o060Reloc ^= 1 << enabled[gOverlay60Data2B0];
                        amSndPlay(0xF, NULL);
                    }
                }
                y = 0x8C;
                for (i = gOverlay60Data2B4;
                     i < gOverlay60Data2B4 + 6 && i < count; i++) {
                    if (i == gOverlay60Data2B0) {
                        fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x4D, y,
                                  D_8007C0B8_o060Reloc[0x2A4 / 4 + enabled[i]], 8);
                    fontPrintXY(&D_800D3140_o060Reloc, 0xC1, y,
                        gOverlay60Data298[(D_8007BF1C_o060Reloc >> enabled[i]) & 1], 8);
                    y += 0xC;
                }
                break;
            case 4:
                spare = frontGetBgmVolume();
                frontSetBgmVolume(func_overlay_060_F0002F54_18BCD2C(
                    0x5F, 0xB9, 0x64, 0x28, spare, gOverlay60Data2A4, ticks));
                if (D_8007BF1C_o060Reloc & 0x80) {
                    previewMode = gOverlay60Data150;
                    if (previewMode > 0) {
                        gOverlay60Data150 = previewMode - ticks;
                        if (gOverlay60Data150 <= 0) {
                            gOverlay60Data150 = 0;
                            amTunePlay(((u8 *)&gOverlay60Data15C)[3]);
                        }
                    }
                    if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                        if ((D_800D31B8_o060Reloc & 4) && gOverlay60Data2B0 > 0) {
                            gOverlay60Data2B0--;
                            amSndPlay(0xF, NULL);
                        } else if ((D_800D31B8_o060Reloc & 8) && gOverlay60Data2B0 < 0x2A) {
                            gOverlay60Data2B0++;
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31B8_o060Reloc & 0x9000) {
                            if (gOverlay60Data150 == 0) {
                                amTuneSetFade(0.5f, 0);
                                gOverlay60Data150 = 0x1E;
                            }
                            gOverlay60Data15C = gOverlay60Data2B0 + 1;
                            amSndPlay(0xC, NULL);
                        }
                    }
                    sprintf(text, O60_TEXT(0x2C8), gOverlay60Data2B0 + 1);
                    fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x7F, text, 0xC);
                    fontColour(0xFF, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x75, gOverlay60Data0C0, 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x89, gOverlay60Data0C4, 0xC);
                }
                break;
            case 5:
                stereoMode = frontGetStereoMode();
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
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
                                  0x68, 0x8C, 0, 0xFF, 0, gOverlay60Data2A4);
                    fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xB9, O60_TEXT(0x298), 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xC3, O60_TEXT(0x29C), 0xC);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xCD, O60_TEXT(0x2A0), 0xC);
                } else {
                    fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, gOverlay60Data288[stereoMode], 0xC);
                }
                break;
            case 6:
                screenMode = frontGetScreenMode();
                if (gOverlay60Data158 == 0 &&
                    func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                    D_800D31BC_o060Reloc != 0) {
                    func_overlay_082_F00004B0_18CF630(gOverlay60Data0A8);
                    gOverlay60Data158 = 1;
                    amSndPlay(0xF, NULL);
                }
                if (gOverlay60Data158 != 0) {
                    if (gOverlay60Data158 == 1) {
                        if (viDisplayingScreen0() == 0) {
                            projection = func_8002468C();
                            animseqStopPath(((u8 *)&gOverlay60Data2B8)[3]);
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
                            gOverlay60Data158 = 2;
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
                        gOverlay60Data158 = 0;
                        func_overlay_082_F00004C0_18CF640(gOverlay60Data0A8);
                    }
                }
                if (screenMode == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x91, O60_TEXT(0x1DC), 0xC);
                    row = frontGetWideAdjust();
                    if (D_800D31B4_o060Reloc & 8) {
                        row--;
                    }
                    if (D_800D31B4_o060Reloc & 4) {
                        row++;
                    }
                    frontSetWideAdjust(row);
                    fontColour(0xFF, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x6A, 0xB4, gOverlay60Data0C0, 4);
                    fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x97, 0xB4, O60_TEXT(0x34), 4);
                    fontColour(0xFF, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x6A, 0xC3, gOverlay60Data0C4, 4);
                    fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x97, 0xC3, O60_TEXT(0x38), 4);
                } else {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, O60_TEXT(0x1E0), 0xC);
                }
                frontSetScreenMode(screenMode);
                break;
            case 7:
                showArrows = 0;
                if (gOverlay60Data14C == 0) {
                    func_overlay_082_F00004C0_18CF640(gOverlay60Data0A8);
                    if (gOverlay60Data154 == 0 &&
                        func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                        if (D_800D31B8_o060Reloc & 0x9000) {
                            gOverlay60Data14C = 1;
                            gOverlay60Data150 = 0;
                            func_overlay_082_F00004B0_18CF630(gOverlay60Data0A8);
                            amSndPlay(0xC, NULL);
                        } else if (D_800D31BE_o060Reloc > 0 && gOverlay60Data2B0 > 0) {
                            gOverlay60Data2B0--;
                            if (gOverlay60Data2B0 == 2 && !(D_800D3128_o060Reloc.unlocked & 0x3F)) {
                                gOverlay60Data2B0 = 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BE_o060Reloc < 0) {
                            if (gOverlay60Data2B0 < 3) {
                                gOverlay60Data2B0++;
                                if (gOverlay60Data2B0 == 2 && !(D_800D3128_o060Reloc.unlocked & 0x3F)) {
                                    gOverlay60Data2B0 = 3;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        }
                    }
                    gOverlay60Data154 = 0;
                    gOverlay60Data260[7] = O60_TEXT(0x278);
                    if (gOverlay60Data2B0 == 0) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x8F, O60_TEXT(0x280), 0xC);
                    if (gOverlay60Data2B0 == 1) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x9B, O60_TEXT(0x288), 0xC);
                    if (gOverlay60Data2B0 == 2) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                    } else if (D_800D3128_o060Reloc.unlocked != 0) {
                        fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0x90, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xA7, O60_TEXT(0x27C), 0xC);
                    if (gOverlay60Data2B0 == 3) {
                        fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xB3, O60_TEXT(0x28C), 0xC);
                } else {
                    switch (gOverlay60Data2B0) {
                    case 0:
                        previewMode = gOverlay60Data150;
                        gOverlay60Data260[7] = D_8007C0B8_o060Reloc[D_8007C11C_o060Reloc[previewMode]];
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
                            gOverlay60Data14C = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            gOverlay60Data150 = previewMode - 1;
                            if (gOverlay60Data150 < 0) {
                                gOverlay60Data150 = limit - 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0) {
                            gOverlay60Data150 = previewMode + 1;
                            if (gOverlay60Data150 >= limit) {
                                gOverlay60Data150 = 0;
                            }
                            amSndPlay(0xF, NULL);
                        }
                        slots = func_800291C4();
                        i = levelGetBlurEffect(D_8007C0E8_o060Reloc[gOverlay60Data150]);
                        slot = (SavesSlot *)((u8 *)slots + i * 32);
                        fontUseFont(2);
                        fontBackground(0, 0, 0, 0);
                        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x7A, O60_TEXT(0xD0), 4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xBA, O60_TEXT(0xD4), 4);
                        if (func_overlay_068_F000146C_18C85CC(
                                D_8007C0E8_o060Reloc[gOverlay60Data150]) != 0) {
                            func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data080,
                                81.0f, 115.0f, gOverlay60Data258, gOverlay60Data258, -2, 0);
                        }
                        fontColour(0xC0, 0xFF, 0, 0xFF, 0xFF);
                        for (row = 0, record = slot->records; row < 4; record++, row++) {
                            func_overlay_056_F00000B8_18A2E30(record->time,
                                &minutes, &seconds, &hundredths);
                            if (record->time == 0) {
                                sprintf(text, gOverlay60Data200);
                                icon = 0x4A;
                            } else {
                                initial0 = func_8003A700(record->initials[0]) & 0xFF;
                                initial1 = func_8003A700(record->initials[1]) & 0xFF;
                                sprintf(text, gOverlay60Data20C, initial0, initial1,
                                    func_8003A700(record->initials[2]),
                                    minutes, seconds, hundredths);
                                icon = record->character + 0x51;
                            }
                            gOverlay60Data060.texture = D_800D31C8_o060Reloc[icon];
                            fontPrintXY(&D_800D3140_o060Reloc, gOverlay60Data1B4[0],
                                gOverlay60Data1D0[row], gOverlay60Data1A4[row], 0xC);
                            func_8002FB34(&D_800D3140_o060Reloc, &gOverlay60Data060,
                                gOverlay60Data1B4[1], gOverlay60Data1D0[row], 0.5f, 0.5f, -1, 0);
                            for (count = 0; count < 11; count++) {
                                glyph[0] = text[count];
                                glyph[1] = '\0';
                                fontPrintXY(&D_800D3140_o060Reloc, gOverlay60Data1B4[count + 2],
                                    gOverlay60Data1D0[row], glyph, 0xC);
                            }
                        }
                        showArrows = 1;
                        break;
                    case 1:
                        texDPInit(&D_800D3140_o060Reloc);
                        camStandardPersp(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc);
                        previewMode = gOverlay60Data150;
                        gOverlay60Data260[7] = D_8007C0B8_o060Reloc[0x220 / 4 + previewMode];
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
                            gOverlay60Data14C = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            gOverlay60Data150 = previewMode - 1;
                            if (gOverlay60Data150 < 0) {
                                gOverlay60Data150 = limit - 1;
                            }
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0) {
                            gOverlay60Data150 = previewMode + 1;
                            if (gOverlay60Data150 >= limit) {
                                gOverlay60Data150 = 0;
                            }
                            amSndPlay(0xF, NULL);
                        }
                        previewMode = gOverlay60Data150;
                        gOverlay60Data130 = previewMode;
                        fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0x8C, O60_TEXT(0x234), 8);
                        fontColour(0xFF, 0, 0, 0xFF, gOverlay60Data2A4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0x9B, O60_TEXT(0x238), 8);
                        fontColour(0xFF, 0xFF, 0, 0xFF, gOverlay60Data2A4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0xAA, O60_TEXT(0x23C), 8);
                        fontColour(0, 0xFF, 0xFF, 0xFF, gOverlay60Data2A4);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x3C, 0xB9, O60_TEXT(0x240), 8);
                        texDPTextureX(&D_800D3140_o060Reloc, 0, 0, 0);
                        gDma1p(D_800D3140_o060Reloc++, 7, (u8 *)gOverlay60Data0B0 + 0x80000000, 0x10, 2);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 0, 255, 0,
                                        gOverlay60Data2A4);
                        overlay60DrawBorder(0x39, 0x86, 0x82, 0x92);
                        overlay60DrawBorder(0x91, 0x74, 0xAF, 0x9D);
                        overlay60DrawLine(0x82, 0x8D, 0x91, 0x8D);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 0, 0,
                                        gOverlay60Data2A4);
                        overlay60DrawBorder(0x39, 0x95, 0x82, 0xA1);
                        overlay60DrawBorder(0xB5, 0x74, 0xD4, 0x9D);
                        overlay60DrawLine(0xC7, 0x9E, 0xC7, 0xA2);
                        overlay60DrawLine(0x82, 0xA1, 0xC7, 0xA1);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 255, 0,
                                        gOverlay60Data2A4);
                        overlay60DrawBorder(0x39, 0xA4, 0x82, 0xB0);
                        overlay60DrawBorder(0x91, 0xA6, 0xAF, 0xCF);
                        overlay60DrawLine(0x82, 0xAB, 0x91, 0xAB);
                        gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 0, 255, 255,
                                        gOverlay60Data2A4);
                        overlay60DrawBorder(0x39, 0xB3, 0x82, 0xBF);
                        overlay60DrawBorder(0xB5, 0xA6, 0xD4, 0xCF);
                        overlay60DrawLine(0xC5, 0xD0, 0xC5, 0xD3);
                        overlay60DrawLine(0x80, 0xD3, 0xC6, 0xD3);
                        overlay60DrawLine(0x80, 0xC0, 0x80, 0xD3);
                        texDPInit(&D_800D3140_o060Reloc);
                        rcpClearZBuffer(&D_800D3140_o060Reloc, 0x140, 0xF0, 0x8C, 0x64, 0xD7, 0xC8);
                        for (i = 0; i < 4; i++) {
                            previewMode = gOverlay60Data150;
                            if (previewMode != gOverlay60Data0C8[i]->unk3A) {
                                gOverlay60Data0C8[i]->unk3A = previewMode;
                                func_8005AD64(gOverlay60Data0C8[i], 0, 0, 0.0f);
                            }
                            previewMode = gOverlay60Data150;
                            gOverlay60Data0C8[i]->unk8 = gOverlay60Data0F8[previewMode];
                            model = (MenuSpawnInner *)(gOverlay60Data0C8[i])->unk68[gOverlay60Data150];
                            model->mode = ticks;
                            spare = ((u32)(D_800D3128_o060Reloc.progress[gOverlay60Data150] & gOverlay60Data10C[i]) >> gOverlay60Data11C[i]);
                            modSetTextureFrame(model, 0, gOverlay60Data12C[spare - 1] * 256);
                            func_8005ABA8(gOverlay60Data0C8[i], 0.003f, ticks);
                            if (spare != 0) {
                                (gOverlay60Data0C8[i])->alpha = gOverlay60Data2A4;
                                func_80009E78(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc,
                                    &D_800D3148_o060Reloc, gOverlay60Data0C8[i]);
                            }
                        }
                        showArrows = 1;
                        break;
                    case 2:
                        gOverlay60Data260[7] = O60_TEXT(0x27C);
                        if (D_800D31B8_o060Reloc & 0x4000) {
                            gOverlay60Data14C = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0) {
                            previewMode = gOverlay60Data150;
                            i = previewMode;
                            do {
                                i--;
                                if (i < 0) {
                                    i = 5;
                                }
                            } while (!(D_800D3128_o060Reloc.unlocked & (1 << i)));
                            if (i != previewMode) {
                                gOverlay60Data150 = i;
                                if (gOverlay60Data174 != NULL) {
                                    texFreeSprite(gOverlay60Data174);
                                    gOverlay60Data174 = NULL;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        } else if (D_800D31BC_o060Reloc > 0) {
                            previewMode = gOverlay60Data150;
                            i = previewMode;
                            do {
                                i++;
                                if (i >= 6) {
                                    i = 0;
                                }
                            } while (!(D_800D3128_o060Reloc.unlocked & (1 << i)));
                            if (i != previewMode) {
                                gOverlay60Data150 = i;
                                if (gOverlay60Data174 != NULL) {
                                    texFreeSprite(gOverlay60Data174);
                                    gOverlay60Data174 = NULL;
                                }
                                amSndPlay(0xF, NULL);
                            }
                        }
                        if (gOverlay60Data14C != 0 && gOverlay60Data174 == NULL) {
                            gOverlay60Data174 = texLoadSprite(
                                gOverlay60Data168[gOverlay60Data150], 0);
                        }
                        if (gOverlay60Data174 != NULL) {
                            camStandardOrtho(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc);
                            gDPPipeSync(D_800D3140_o060Reloc++);
                            gDPSetPrimColor(D_800D3140_o060Reloc++, 0, 0, 255, 255, 255, 255);
                            gDPSetEnvColor(D_800D3140_o060Reloc++, 255, 255, 255, 0);
                            camDo2DSprite(&D_800D3140_o060Reloc, &D_800D3144_o060Reloc, &D_800D3148_o060Reloc,
                                gOverlay60Data178, gOverlay60Data174, 0, 0xFF);
                            if (gOverlay60Data14C == 0) {
                                texFreeSprite(gOverlay60Data174);
                                gOverlay60Data174 = NULL;
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
                        gOverlay60Data260[7] = O60_TEXT(0x28C);
                        if (D_800D31B8_o060Reloc & 0x9000) {
                            if (gOverlay60Data150 != 0) {
                                gOverlay60Data14C = 0;
                                gOverlay60Data160 = 1;
                                overlay60ReassignChoiceSlots();
                                D_8007BF1C_o060Reloc = 0;
                                func_80029198();
                                amSndPlay(0xC, NULL);
                            } else {
                                gOverlay60Data14C = 0;
                                amSndPlay(0xD, NULL);
                            }
                        } else if (D_800D31B8_o060Reloc & 0x4000) {
                            gOverlay60Data14C = 0;
                            amSndPlay(0xD, NULL);
                        } else if (D_800D31BC_o060Reloc < 0 && gOverlay60Data150 == 0) {
                            gOverlay60Data150 = 1;
                            amSndPlay(0xF, NULL);
                        } else if (D_800D31BC_o060Reloc > 0 && gOverlay60Data150 != 0) {
                            gOverlay60Data150 = 0;
                            amSndPlay(0xF, NULL);
                        }
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x8F, O60_TEXT(0x290), 0xC);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x99, O60_TEXT(0x294), 0xC);
                        fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAD, O60_TEXT(0x54), 0xC);
                        if (gOverlay60Data150 != 0) {
                            fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                        } else {
                            fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
                        }
                        fontPrintXY(&D_800D3140_o060Reloc, 0x71, 0xBC, O60_TEXT(0x58), 0xC);
                        if (gOverlay60Data150 == 0) {
                            fontColour(0x80, 0xFF, 0x80, 0xFF, gOverlay60Data2A4);
                        } else {
                            fontColour(0, 0xD0, 0, 0xFF, gOverlay60Data2A4);
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
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        if (gOverlay60Data2B0 == 0) {
                            gOverlay60Data2B0 = 1;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        if (gOverlay60Data2B0 == 1) {
                            gOverlay60Data2B0 = 0;
                            amSndPlay(0xF, NULL);
                        } else {
                            amSndPlay(0xE, NULL);
                        }
                    }
                }
                if (gOverlay60Data2B0 == 0) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field20 = (D_800D3128_o060Reloc.bits.field20 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                }
                sprintf(text, gOverlay60Data220, O60_TEXT(0x1D4),
                    gOverlay60Data298[D_800D3128_o060Reloc.bits.field20]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0x96, text, 0xC);
                if (gOverlay60Data2B0 == 1) {
                    fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0 &&
                        D_800D31BC_o060Reloc != 0) {
                        D_800D3128_o060Reloc.bits.field19 = (D_800D3128_o060Reloc.bits.field19 ^ 1) & 1;
                        amSndPlay(0xF, NULL);
                    }
                } else {
                    fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                }
                sprintf(text, gOverlay60Data228, O60_TEXT(0x1D8),
                    gOverlay60Data298[D_800D3128_o060Reloc.bits.field19]);
                fontPrintXY(&D_800D3140_o060Reloc, 0x91, 0xAA, text, 0xC);
                break;
            case 9:
                if (func_overlay_082_F00004A4_18CF624(gOverlay60Data0A8) != 0) {
                    if (D_800D31BE_o060Reloc < 0) {
                        gOverlay60Data2B0++;
                        if (gOverlay60Data2B0 >= 0xC) {
                            gOverlay60Data2B0 = 0xB;
                            amSndPlay(0xE, NULL);
                        } else {
                            amSndPlay(0xF, NULL);
                        }
                        if (gOverlay60Data2B4 < gOverlay60Data2B0 - 5) {
                            gOverlay60Data2B4++;
                        }
                    } else if (D_800D31BE_o060Reloc > 0) {
                        gOverlay60Data2B0--;
                        if (gOverlay60Data2B0 < 0) {
                            gOverlay60Data2B0 = 0;
                            amSndPlay(0xE, NULL);
                        } else {
                            amSndPlay(0xF, NULL);
                        }
                        if (gOverlay60Data2B0 < gOverlay60Data2B4) {
                            gOverlay60Data2B4 = gOverlay60Data2B0;
                        }
                    }
                    if (D_800D31BC_o060Reloc != 0) {
                        D_8007BF14_o060Reloc ^= 1 << (gOverlay60Data2B0 + 2);
                        amSndPlay(0xF, NULL);
                    }
                }
                y = 0x8C;
                for (i = gOverlay60Data2B4; i < gOverlay60Data2B4 + 6; i++) {
                    if (i == gOverlay60Data2B0) {
                        fontColour(0x64, 0xFF, 0x64, 0xFF, gOverlay60Data2A4);
                    } else {
                        fontColour(0, 0xBE, 0, 0xFF, gOverlay60Data2A4);
                    }
                    fontPrintXY(&D_800D3140_o060Reloc, 0x4D, y,
                        D_8007C0B8_o060Reloc[0x1F0 / 4 + i], 8);
                    fontPrintXY(&D_800D3140_o060Reloc, 0xC1, y,
                        gOverlay60Data298[(D_8007BF14_o060Reloc >> (i + 2)) & 1], 8);
                    y += 0xC;
                }
                break;
            }
        }
        if ((D_800D31B8_o060Reloc & 0x4000) && gOverlay60Data2AC == -1 &&
            gOverlay60Data164 == 0) {
            gOverlay60Data164 = 1;
            if (D_8007BF1C_o060Reloc & 0x40) {
                D_8007BF1C_o060Reloc &= ~0x40;
                func_overlay_048_F0000000_1895408();
            } else {
                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
            }
            amSndPlay(0xD, NULL);
        }
        fontColour(0, 0xFF, 0, 0xFF, gOverlay60Data2A0);
        fontPrintXY(&D_800D3140_o060Reloc, 0x91, gOverlay60Data2A8,
            gOverlay60Data260[panel], 0xC);
    }
}
