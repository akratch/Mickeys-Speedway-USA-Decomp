#include "PR/ultratypes.h"

typedef struct O50Glyph {
    void *texture;
    void *alternate;
    s32 glyph;
    s16 x;
    s16 y;
} O50Glyph;

/* Tier D: the layout func_overlay_050_F0000334_1896CA4 gives this array. */
typedef struct O50MenuObject {
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
} O50MenuObject;

/* Overlay 50 was one translation unit (see func_overlay_050_F0000334_1896CA4):
 * its runtime records address this function's .data and .bss through LOCAL
 * (section-relative) records, and as1 shares one high half across the stores
 * to adjacent fields of record 0 only for a TU-local symbol. So this TU, like
 * that one, defines the overlay's data at the offsets the records address.
 * The bytes belong to the retained overlay image: these zero initializers only
 * fix the layout, and the object's .data, .bss and .rodata are dropped at
 * POSTPROCESS with their records rebound to zero-valued bases
 * (mk/overlays.mk). */
static s32 sOverlay50Data000[27] = { 0 };
static O50Glyph D_6C[4] = { 0 };
static O50Glyph D_AC[3] = { 0 };
static O50Glyph D_DC[2] = { 0 };
static O50Glyph D_FC[3] = { 0 };
static O50Glyph D_12C[10] = { 0 };
static O50Glyph D_1CC[5] = { 0 };
static s16 D_21C = 0;
static O50Glyph sOverlay50LapTemplate[10] = { 0 };
static O50Glyph D_2C0[2] = { 0 };
static O50Glyph D_2E0[2] = { 0 };
static O50Glyph D_300[2] = { 0 };
static s8 o50Data320[8] = { 0 };
static s32 D_328 = 0;
static s8 D_32C[4] = { 0 };
static s8 o50Data330[4] = { 0 };
static s32 D_334 = 0;
static s32 sOverlay50Data338 = 0;
static s32 D_33C = 0;
static s32 sOverlay50Data340[12] = { 0 };

static O50Glyph o50LapGlyphs[10];
static s32 sOverlay50BssA0;
static f32 D_A4;
static f32 D_A8;
static s32 sOverlay50BssAC;
static s32 D_B0[3];
static s32 D_BC;
static s32 D_C0;
static void *o50BssC4;
static s16 D_C8;
static s16 D_CA;

/* Tier B: runtime export identities; the *_o050Reloc names are the generated
 * relocation surface. */
extern O50MenuObject D_800D3550_o050Reloc[];
extern s16 D_800D304E_o050Reloc;

u8 *func_80028F54_o050Reloc(void); /* 0:+0x28B04 */
void loadFrontEndList_o050Reloc(void *list); /* 0:+0x39738 */
void setupFrontEndList_o050Reloc(void *list); /* 0:+0x39900 */
void amTunePlay_o050Reloc(s32); /* 0:+0xC0 */
void runlinkDownloadCode_o050Reloc(s32); /* 0:+0x31828 */
void overlay56LoadResource_o050Reloc(void); /* 56:+0x118 */
s32 levelGetNumber_o050Reloc(void); /* 0:+0x2630C */
void fontUseFont_o050Reloc(s32); /* 0:+0x4AC54 */
void *func_8003A5A0_o050Reloc(s32); /* 0:+0x3A150 */
void *overlay45CreateDescriptor_o050Reloc(void *, s32, s32, s32); /* 45:+0xC */
void overlay45SetMode_o050Reloc(void *, s32); /* 45:+0x1BE0 */
void overlay50PatchIndices(void *entry);

/* Matched 2026-10-02 from a 104-word plateau. What it took:
 *   - the overlay's data defined in the TU (see above), so the copy loop
 *     indexes two real arrays and IDO's default unroller emits the shipped
 *     one-plus-two-by-four copy;
 *   - the descriptor stored to its global and read back from it, with no
 *     carrier local: the global's address is then one web hoisted above the
 *     branch, and as1 puts the NULL store in a branch-likely delay slot
 *     (no -Wab,-r4300_mul needed);
 *   - one resident callee (func_80028F54) called twice and levelGetNumber
 *     called twice, where the inherited candidate had four names for them;
 *   - 0.7f as a literal and an unused leading local for the saved-flag home.
 */
void func_overlay_050_F0000000_1896970(void) {
    s32 pad;
    u8 *saved;
    s32 i;

    saved = func_80028F54_o050Reloc();
    loadFrontEndList_o050Reloc(sOverlay50Data000);
    setupFrontEndList_o050Reloc(&sOverlay50Data000[18]);
    amTunePlay_o050Reloc(4);
    sOverlay50BssA0 = 0x104;
    runlinkDownloadCode_o050Reloc(0xB);

    overlay50PatchIndices(D_6C);
    overlay50PatchIndices(D_12C);
    overlay50PatchIndices(D_AC);
    overlay50PatchIndices(D_DC);
    overlay50PatchIndices(D_FC);
    overlay50PatchIndices(D_300);
    overlay50PatchIndices(D_2C0);
    overlay50PatchIndices(D_2E0);
    overlay50PatchIndices(D_1CC);

    D_A4 = -100.0f;
    D_A8 = 120.0f;
    D_800D3550_o050Reloc[4].x = 42.0f;
    D_800D3550_o050Reloc[1].x = -78.0f;
    D_800D3550_o050Reloc[1].value08 = 0.7f;
    D_800D3550_o050Reloc[0].angle = 0x4000;
    for (i = 0; i < 3; i++) {
        D_B0[i] = 0xA0;
    }

    overlay56LoadResource_o050Reloc();
    o50Data330[0] = -1;
    D_800D304E_o050Reloc = levelGetNumber_o050Reloc();
    D_BC = -0x500;
    D_C0 = -0x140;
    for (i = 0; i < 9; i++) {
        o50LapGlyphs[i].x = sOverlay50LapTemplate[i].x;
        o50LapGlyphs[i].y = sOverlay50LapTemplate[i].y;
        o50LapGlyphs[i].glyph = sOverlay50LapTemplate[i].glyph;
    }

    if (*func_80028F54_o050Reloc() == 1) {
        D_800D3550_o050Reloc[1].x += -40.0f;
        D_FC[0].x -= 0x28;
        D_FC[1].x -= 0x28;
    }

    if (*saved == 0) {
        fontUseFont_o050Reloc(3);
        o50BssC4 = overlay45CreateDescriptor_o050Reloc(
            func_8003A5A0_o050Reloc(levelGetNumber_o050Reloc()), 0xA0, 0x78, 0xC);
        overlay45SetMode_o050Reloc(o50BssC4, 0);
    } else {
        o50BssC4 = NULL;
    }
    D_CA = 0;
    D_C8 = 0;
}
