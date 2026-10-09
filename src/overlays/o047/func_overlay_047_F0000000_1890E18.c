#include "PR/ultratypes.h"
#include "overlays/overlay_045.h"

typedef struct O47SpawnPacket {
    s16 selector;
    u8 type;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 byte0A;
    u8 byte0B;
    s8 state;
} O47SpawnPacket;

/* Tier D: field widths and offsets from this function's loads and stores. */
typedef struct O47Entry {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[0x14];
    s16 pathSelector;
    u8 pad22[2];
    void *handle;
    s16 selection;
    u8 enabled;
    s8 active;
    u8 initialized;
    u8 pad2D[3];
    s32 field30;
} O47Entry;

typedef struct O47Choice {
    s16 field00;
    s16 field02;
    s16 field04;
    u8 pad06[2];
    f32 field08;
    f32 x;
    f32 y;
    f32 field14;
    u8 pad18[0x10];
    f32 value;
} O47Choice;

typedef struct O47Timer {
    s32 field00;
    s32 field04;
    s32 field08;
    u8 pad0C[2];
    s16 duration;
} O47Timer;

typedef struct O47PathPoint {
    f32 x;
    f32 y;
    f32 z;
    s16 selector;
    u8 pad0E[2];
} O47PathPoint;

typedef struct O47PathNode {
    s16 selector;
    u8 pad02[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    struct O47PathNode *next;
} O47PathNode;

typedef struct O47PathObject {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[9];
    O47PathNode *path;
} O47PathObject;

typedef struct O47TextTable {
    u8 pad00[0x58];
    void *text58;
    void *text5C;
    u8 pad60[0x128];
    void *text188;
    void *text18C;
} O47TextTable;

/* Bit 27 is tested by shift-and-sign, which is a bitfield read. */
typedef struct O47Save {
    u32 pad0 : 4;
    u32 unlocked : 1;
    u32 pad5 : 27;
    u8 pad04[4];
    u16 configuration[4];
    u16 field10;
} O47Save;

/* Overlay 47 was one translation unit: its runtime records address this
 * function's .data, .bss and .rodata through LOCAL (section-relative)
 * records, and as1 shares one high half across the stores to adjacent
 * fields only for a TU-local symbol. So this TU defines the overlay's data
 * at the offsets the records address. The bytes belong to the retained
 * overlay image (overlay47ReleaseResources.c owns the initialized data):
 * these zero initializers only fix the layout, and the object's .data, .bss
 * and .rodata are dropped at POSTPROCESS with their records rebound to
 * zero-valued bases (mk/overlays.mk). The D_ names keep their section
 * offsets. */
static u8 sO47Data000[0x358] = { 0 };
static s16 D_358[12] = { 0 };
static s16 D_370[2] = { 0 };
static s16 D_374[12] = { 0 };
static void *D_38C[10] = { 0 };
static u8 sO47Data3B4[0x50] = { 0 };
static f32 D_404[15] = { 0 };
static s8 D_440[12] = { 0 };
static s8 D_44C[12] = { 0 };
static s8 D_458[12] = { 0 };
static s8 D_464[12] = { 0 };
static u8 sO47Data470[0x80] = { 0 };
static s32 D_4F0 = 0;
static u8 sO47Data4F4[0x6C] = { 0 };

static s8 sO47ChoiceCount;
static O47Choice D_8[10];
static O47Timer D_1C0[5];
static O47PathPoint D_210[7];
static O47PathPoint D_280[7];
static O47PathPoint D_2F0;
static s8 D_300[10];
static s8 D_30A;
static s8 sO47Bss30B;
static void *D_30C;
static s32 D_310;
static void *D_314;
static void *D_318;
static void *D_31C;
static void *D_320;
static s32 D_324;
static s32 D_328[4];
static s32 D_338;
static s32 sO47Bss33C;

/* Tier B: runtime export identities (resident and overlay 45); the
 * *_o047Reloc names are the generated relocation surface. */
extern O47Entry D_800D3058_o047Reloc[];
extern O47Save D_800D3128_o047Reloc;
extern s32 D_800D31C8_o047Reloc[];
extern O47TextTable *D_8007C0B8_o047Reloc;
extern s32 D_8007C1A0_o047Reloc;
extern u8 D_8007BF68_o047Reloc;

void loadFrontEndList_o047Reloc(s16 *list); /* 0:+0x39738 */
void setupFrontEndList_o047Reloc(s16 *list); /* 0:+0x39900 */
void fontUseFont_o047Reloc(s32); /* 0:+0x4AC54 */
void fontColour_o047Reloc(s32, s32, s32, s32, s32); /* 0:+0x4AC68 */
void joyResetMap_o047Reloc(void); /* 0:+0x24FA4 */
void *func_8000590C_o047Reloc(O47SpawnPacket *packet, s32 mode); /* 0:+0x54BC */
O47PathObject *func_800508B4_o047Reloc(s32 selector); /* 0:+0x50464 */
s32 frontGetMode_o047Reloc(void); /* 0:+0x389C0 */
void func_8003A754_o047Reloc(void); /* 0:+0x3A304 */
void animseqStartPath_o047Reloc(s32 selector); /* 0:+0x50238 */
void amSndPlay_o047Reloc(u16, void **); /* 0:+0xB44 */
Overlay45ResourceDescriptor *overlay45CreateDescriptor_o047Reloc(
    const char *text, s16 width, s16 height, s32 flags); /* 45:+0xC */

/* DKR v77/v80 and JFG contain no matching donor for this initializer.
 * Matched 2026-10-02 from a 548-word plateau. What it took: the default
 * loop unroll (the inherited -loopunroll,0 override is retired) with plain
 * index loops; the overlay's data defined in the TU (see above); the save
 * word's flag as a bitfield; s8 unlock flags; the argument-free resident
 * call; the x step read at each use (no carrier, which also lands the
 * frame); and the entry cursor taken before the outer loop. */
void func_overlay_047_F0000000_1890E18(void) {
    O47SpawnPacket packet;
    O47PathObject *pathObject;
    O47PathNode *node;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 yStep;
    O47Entry *entry;
    O47Choice *choice;

    loadFrontEndList_o047Reloc(D_358);
    setupFrontEndList_o047Reloc(D_370);
    fontUseFont_o047Reloc(3);
    fontColour_o047Reloc(0xFF, 0xFF, 0xFF, 0, 0xFF);
    D_30C = overlay45CreateDescriptor_o047Reloc(D_8007C0B8_o047Reloc->text188, 0xA0, 0x18, 4);
    D_314 = overlay45CreateDescriptor_o047Reloc("OK", 0xA0, 0x110, 4);
    D_318 = overlay45CreateDescriptor_o047Reloc(D_8007C0B8_o047Reloc->text18C, 0xA0, -0x28, 4);
    D_31C = overlay45CreateDescriptor_o047Reloc(D_8007C0B8_o047Reloc->text58, 0x78, 0x104, 4);
    D_320 = overlay45CreateDescriptor_o047Reloc(D_8007C0B8_o047Reloc->text5C, 0xC8, 0x104, 4);
    joyResetMap_o047Reloc();

    packet.type = 0xE;
    packet.x = 0;
    packet.y = 0;
    packet.z = 0;
    packet.state = 0;
    packet.byte0B = 0x4B;
    packet.byte0A = 0;
    i = 0;
    while (D_374[i] != -1) {
        packet.selector = D_374[i];
        D_38C[i] = func_8000590C_o047Reloc(&packet, 0);
        i++;
    }

    for (i = 0; i < 4; i++) {
        D_1C0[i].field04 = 0;
        D_1C0[i].field08 = 0;
        D_1C0[i].duration = 0x96;
    }
    D_1C0[4].field00 = 0;
    D_1C0[4].field04 = 0;

    pathObject = func_800508B4_o047Reloc(0x48);
    if (pathObject != NULL) {
        node = pathObject->path;
        for (i = 0; i < 7; i++) {
            D_210[i].x = node->x;
            D_210[i].y = node->y;
            D_210[i].z = node->z;
            D_210[i].selector = node->selector;
            node = node->next;
        }
    }

    pathObject = func_800508B4_o047Reloc(0x52);
    if (pathObject != NULL) {
        node = pathObject->path;
        for (i = 0; i < 7; i++) {
            D_280[i].x = node->x;
            D_280[i].y = node->y;
            D_280[i].z = node->z;
            D_280[i].selector = node->selector;
            node = node->next;
        }
    }

    pathObject = func_800508B4_o047Reloc(0x49);
    if (pathObject != NULL) {
        node = pathObject->path;
        D_2F0.x = node->x;
        D_2F0.y = node->y;
        D_2F0.z = node->z;
        D_2F0.selector = node->selector;
    }

    for (i = 0; i < 4; i++) {
        D_800D3058_o047Reloc[i].handle = NULL;
        D_800D3058_o047Reloc[i].field30 = 0;
        D_800D3058_o047Reloc[i].x = D_2F0.x;
        D_800D3058_o047Reloc[i].y = D_2F0.y;
        D_800D3058_o047Reloc[i].z = D_2F0.z;
        D_800D3058_o047Reloc[i].pathSelector = D_2F0.selector;
        D_800D3058_o047Reloc[i].enabled = 0;
        D_800D3058_o047Reloc[i].initialized = 0;
    }

    for (i = 0; i < 10; i++) {
        D_300[i] = 0;
    }

    for (i = 0; i < 6; i++) {
        D_8[i].value = i;
    }

    sO47ChoiceCount = 6;
    if (D_800D3128_o047Reloc.unlocked) {
        D_300[6] = 0;
        D_8[sO47ChoiceCount].value = 6.0f;
        sO47ChoiceCount++;
    } else {
        D_300[6] = 1;
    }

    if (((D_800D3128_o047Reloc.configuration[0] & 7) >= 3) &&
        ((D_800D3128_o047Reloc.configuration[1] & 7) >= 3) &&
        ((D_800D3128_o047Reloc.configuration[2] & 7) >= 3)) {
        D_300[7] = 0;
        D_8[sO47ChoiceCount].value = 7.0f;
        sO47ChoiceCount++;
    } else {
        D_300[7] = 1;
    }

    if ((((D_800D3128_o047Reloc.configuration[0] & 0x38) >> 3) >= 3) &&
        (((D_800D3128_o047Reloc.configuration[1] & 0x38) >> 3) >= 3) &&
        (((D_800D3128_o047Reloc.configuration[2] & 0x38) >> 3) >= 3)) {
        D_300[8] = 0;
        D_8[sO47ChoiceCount].value = 8.0f;
        sO47ChoiceCount++;
    } else {
        D_300[8] = 1;
    }

    if ((((D_800D3128_o047Reloc.configuration[0] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128_o047Reloc.configuration[1] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128_o047Reloc.configuration[2] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128_o047Reloc.configuration[3] & 0x1C0) >> 6) == 4) &&
        (((D_800D3128_o047Reloc.field10 & 0x1C0) >> 6) == 4)) {
        D_300[9] = 0;
        D_8[sO47ChoiceCount].value = 9.0f;
        sO47ChoiceCount++;
    } else {
        D_300[9] = 1;
    }

    if (frontGetMode_o047Reloc() == 9) {
        D_8007C1A0_o047Reloc = 1;
        D_30A = 1;
        for (i = 0; i < 4; i++) {
            D_800D3058_o047Reloc[i].selection = 0;
            D_800D3058_o047Reloc[i].active = 0;
        }
        func_8003A754_o047Reloc();
        D_800D3058_o047Reloc[0].active = 1;
        D_300[0] = 1;
        D_8007BF68_o047Reloc = 1;
    } else {
        D_30A = D_8007C1A0_o047Reloc;
        for (i = 0; i < 4; i++) {
            if (D_800D3058_o047Reloc[i].active != 0) {
                D_300[D_800D3058_o047Reloc[i].selection] = 1;
            }
        }
    }

    animseqStartPath_o047Reloc(0x4A);
    pathObject = func_800508B4_o047Reloc(0x4A);
    if (pathObject != NULL) {
        pathObject->flags |= 2;
    }
    amSndPlay_o047Reloc(0x19, NULL);

    for (i = 0; i < 4; i++) {
        D_800D3058_o047Reloc[i].enabled = 0;
    }
    D_310 = 0;
    D_324 = 0;
    D_4F0 = D_800D31C8_o047Reloc[4];

    x = D_440[sO47ChoiceCount + 3];
    y = D_458[sO47ChoiceCount + 3];
    yStep = D_464[sO47ChoiceCount + 3];
    for (i = 0; i < 10; i++) {
        D_8[i].field14 = 0.0f;
        D_8[i].field00 = 0;
        D_8[i].x = x;
        D_8[i].field02 = 0;
        D_8[i].field04 = 0;
        D_8[i].field08 = D_404[sO47ChoiceCount];
        x += D_44C[sO47ChoiceCount + 3];
        D_8[i].y = y;
        y += yStep;
        yStep = -yStep;
    }

    entry = D_800D3058_o047Reloc;
    for (i = 0; i < 4; i++, entry++) {
        for (choice = D_8, j = 0; j < sO47ChoiceCount; j++, choice++) {
            /* `(*p).f`, not `p->f`: the arrow form swaps the c.eq.s operands. */
            if (((*choice).value == (*entry).selection) && ((*entry).active != 0)) {
                D_328[i] = ((s32)choice->x + 0xA0) << 4;
            }
        }
    }
    D_338 = 0;
}
