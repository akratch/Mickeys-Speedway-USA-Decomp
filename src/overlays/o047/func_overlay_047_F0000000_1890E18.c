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

typedef struct O47Entry {
    s32 state;
    u8 pad04[4];
    u16 configuration[4];
    s32 field10;
    u8 pad14[0x10];
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
    u8 pad00[4];
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
    u8 pad00[0x20];
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

extern O47Entry D_0[];
extern O47Entry D_D0[];
extern O47Choice D_8[];
extern O47Choice D_1C0[];
extern s32 D_200;
extern O47PathPoint D_210[];
extern O47PathPoint D_280[];
extern O47PathPoint D_2F0;
extern u8 D_300[];
extern s8 D_30A;
extern void *D_30C;
extern s32 D_310;
extern void *D_314;
extern void *D_318;
extern void *D_31C;
extern void *D_320;
extern s32 D_324;
extern s32 D_328[];
extern s32 D_338;
extern u8 D_358[];
extern u8 D_370[];
extern s16 D_374;
/* The selector walk deliberately carries a distinct relocation name for the
 * same overlay-local address as D_374. */
extern s16 D_374_list[];
extern void *D_38C[];
extern f32 D_404[];
extern s8 D_443[];
extern s8 D_44F[];
extern s8 D_45B[];
extern s8 D_467[];
extern s32 D_4F0;
extern s8 D_DATA_0;
extern u8 D_TEXT_314[];
extern O47TextTable *gO47TextTable;

extern void o47LoadMenuDataReloc(void *data);
extern void o47ConfigureMenuDataReloc(void *data);
extern void o47SetMenuModeReloc(s32 mode);
extern void o47SetMenuColorReloc(s32 red, s32 green, s32 blue, s32 alpha,
                                  s32 intensity);
extern void o47FinalizeMenuSetupReloc(void);
extern void *func_8000590C(O47SpawnPacket *packet, s32 mode);
extern O47PathObject *o47FindPathObjectReloc(s32 selector);
extern s32 o47ReadModeReloc(void);
extern void o47SetModeReloc(s32 mode);
extern void o47LoadObjectReloc(s32 selector);
extern void o47ConfigureObjectReloc(s32 selector, s32 mode);

#define O47_COPY_POINT(destination, source) \
    do { \
        (destination).x = (source)->x; \
        (destination).y = (source)->y; \
        (destination).z = (source)->z; \
        (destination).selector = (source)->selector; \
        (source) = (source)->next; \
    } while (0)

/* DKR v77/v80 and JFG contain no matching donor for this initializer. */
/* Workbench p5: structure-mismatch; 632/628 candidate/target instructions, 548 differing words from +0x0, frame -0x68 exact.
 * No new safe lever: flag lattice and path-copy/addressing probes are exhausted.
 * Remains: retail path-copy split and overlay-local BSS alias scheduling. */
#ifdef NON_MATCHING
void func_overlay_047_F0000000_1890E18(void) {
    O47SpawnPacket packet;
    O47PathObject *pathObject;
    O47PathNode *node;
    O47PathPoint *point;
    O47Entry *entry;
    O47Choice *choice;
    O47Timer *timer;
    s32 index;
    s32 entryIndex;
    s32 choiceIndex;
    s8 count;
    s8 x;
    s8 y;
    s8 xStep;
    s8 yStep;

    o47LoadMenuDataReloc(D_358);
    o47ConfigureMenuDataReloc(D_370);
    o47SetMenuModeReloc(3);
    o47SetMenuColorReloc(0xFF, 0xFF, 0xFF, 0, 0xFF);
    D_30C = overlay45CreateDescriptor(gO47TextTable->text188, 0xA0, 0x18, 4);
    D_314 = overlay45CreateDescriptor(D_TEXT_314, 0xA0, 0x110, 4);
    D_318 = overlay45CreateDescriptor(gO47TextTable->text18C, 0xA0, -0x28, 4);
    D_31C = overlay45CreateDescriptor(gO47TextTable->text58, 0x78, 0x104, 4);
    D_320 = overlay45CreateDescriptor(gO47TextTable->text5C, 0xC8, 0x104, 4);
    o47FinalizeMenuSetupReloc();

    packet.type = 0xE;
    packet.x = 0;
    packet.y = 0;
    packet.z = 0;
    packet.byte0A = 0;
    packet.byte0B = 0x4B;
    packet.state = 0;
    if (D_374 != -1) {
        s16 *selector;
        void **handle;

        selector = D_374_list;
        handle = D_38C;
        do {
            packet.selector = *selector;
            *handle = func_8000590C(&packet, 0);
            selector++;
            handle++;
        } while (*selector != -1);
    }

    timer = (O47Timer *)D_1C0;
    do {
        timer->field04 = 0;
        timer->field08 = 0;
        timer->duration = 0x96;
        timer++;
    } while (timer < (O47Timer *)&D_200);
    ((s32 *)D_1C0)[0x40 / 4] = 0;
    ((s32 *)D_1C0)[0x44 / 4] = 0;

    pathObject = o47FindPathObjectReloc(0x48);
    if (pathObject != NULL) {
        node = pathObject->path;
        O47_COPY_POINT(D_210[0], node);
        O47_COPY_POINT(D_210[1], node);
        O47_COPY_POINT(D_210[2], node);
        point = &D_210[3];
        O47_COPY_POINT(point[0], node);
        O47_COPY_POINT(point[1], node);
        O47_COPY_POINT(point[2], node);
        O47_COPY_POINT(point[3], node);
    }

    pathObject = o47FindPathObjectReloc(0x52);
    if (pathObject != NULL) {
        node = pathObject->path;
        O47_COPY_POINT(D_280[0], node);
        O47_COPY_POINT(D_280[1], node);
        O47_COPY_POINT(D_280[2], node);
        point = &D_280[3];
        O47_COPY_POINT(point[0], node);
        O47_COPY_POINT(point[1], node);
        O47_COPY_POINT(point[2], node);
        O47_COPY_POINT(point[3], node);
    }

    pathObject = o47FindPathObjectReloc(0x49);
    if (pathObject != NULL) {
        node = pathObject->path;
        D_2F0.x = node->x;
        D_2F0.y = node->y;
        D_2F0.z = node->z;
        D_2F0.selector = node->selector;
    }

    entry = D_0;
    do {
        entry->handle = NULL;
        entry->field30 = 0;
        entry->state = *(s32 *)&D_2F0.x;
        *(f32 *)&entry->pad04[0] = D_2F0.y;
        *(f32 *)&entry->pad04[4] = D_2F0.z;
        *(s16 *)&entry->pad14[0xC] = D_2F0.selector;
        entry->enabled = 0;
        entry->initialized = 0;
        entry++;
    } while (entry < D_D0);

    index = 0;
    do {
        D_300[index] = 0;
        index++;
    } while (index < 10);

    choice = D_8;
    index = 0;
    do {
        choice->value = (f32)index;
        choice++;
        index++;
    } while (index < 6);

    count = 6;
    if (D_0[0].state & 0x08000000) {
        D_300[6] = 0;
        D_8[count].value = 6.0f;
        count++;
    } else {
        D_300[6] = 1;
    }

    if (((D_0[0].configuration[0] & 7) >= 3) &&
        ((D_0[0].configuration[1] & 7) >= 3) &&
        ((D_0[0].configuration[2] & 7) >= 3)) {
        D_300[7] = 0;
        D_8[count].value = 7.0f;
        count++;
    } else {
        D_300[7] = 1;
    }

    if ((((D_0[0].configuration[0] & 0x38) >> 3) >= 3) &&
        (((D_0[0].configuration[1] & 0x38) >> 3) >= 3) &&
        (((D_0[0].configuration[2] & 0x38) >> 3) >= 3)) {
        D_300[8] = 0;
        D_8[count].value = 8.0f;
        count++;
    } else {
        D_300[8] = 1;
    }

    if ((((D_0[0].configuration[0] & 0x1C0) >> 6) == 4) &&
        (((D_0[0].configuration[1] & 0x1C0) >> 6) == 4) &&
        (((D_0[0].configuration[2] & 0x1C0) >> 6) == 4) &&
        (((D_0[0].configuration[3] & 0x1C0) >> 6) == 4) &&
        (((*(u16 *)&D_0[0].field10 & 0x1C0) >> 6) == 4)) {
        D_300[9] = 0;
        D_8[count].value = 9.0f;
        count++;
    } else {
        D_300[9] = 1;
    }

    if (o47ReadModeReloc() == 9) {
        D_0[0].state = 1;
        D_30A = 1;
        D_0[0].selection = 0;
        D_0[0].active = 0;
        D_0[1].selection = 0;
        D_0[1].active = 0;
        D_0[2].selection = 0;
        D_0[2].active = 0;
        D_0[3].selection = 0;
        D_0[3].active = 0;
        o47SetModeReloc(1);
        D_0[0].active = 1;
        D_300[0] = 1;
        D_DATA_0 = 1;
    } else {
        D_30A = D_0[0].state;
        for (entryIndex = 0; entryIndex < 4; entryIndex++) {
            if (D_0[entryIndex].active != 0) {
                D_300[D_0[entryIndex].selection] = 1;
            }
        }
    }

    o47LoadObjectReloc(0x4A);
    pathObject = o47FindPathObjectReloc(0x4A);
    if (pathObject != NULL) {
        *((u8 *)pathObject + 0x16) |= 2;
    }
    o47ConfigureObjectReloc(0x19, 0);

    entry = D_0;
    do {
        entry->enabled = 0;
        entry++;
    } while (entry < D_D0);
    D_310 = 0;
    D_324 = 0;
    D_4F0 = D_0[0].field10;

    count = D_DATA_0;
    x = D_443[count];
    y = D_45B[count];
    xStep = D_44F[count];
    yStep = D_467[count];
    choice = D_8;
    do {
        choice->field14 = 0.0f;
        choice->field00 = 0;
        choice->x = (f32)x;
        choice->field02 = 0;
        choice->field04 = 0;
        choice->field08 = D_404[count];
        x += xStep;
        choice->y = (f32)y;
        y += yStep;
        yStep = -yStep;
        choice++;
    } while (choice < D_1C0);

    entry = D_0;
    entryIndex = 0;
    do {
        choice = D_8;
        choiceIndex = 0;
        if (count > 0) {
            do {
                if ((choice->value == (f32)entry->selection) &&
                    (entry->active != 0)) {
                    D_328[entryIndex] = ((s32)choice->x + 0xA0) << 4;
                }
                choiceIndex++;
                choice++;
            } while (choiceIndex < count);
        }
        entryIndex++;
        entry++;
    } while (entryIndex != 4);
    D_338 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o047/func_overlay_047_F0000000_1890E18/func_overlay_047_F0000000_1890E18.s")
#endif

#undef O47_COPY_POINT

/* PLATEAU-HANDOFF:func_overlay_047_F0000000_1890E18:start
 * symbol: func_overlay_047_F0000000_1890E18
 * score: 548/628 words
 * frame: 0x68
 * relocations: 189
 * first-mismatch: +0xC8
 * summary: V0 overruns its 2512-byte owner by 16 bytes (632/628; 553 raw differences); relocation identity proof is unavailable; candidate has 189 records.
 * PLATEAU-HANDOFF:func_overlay_047_F0000000_1890E18:end
 */
