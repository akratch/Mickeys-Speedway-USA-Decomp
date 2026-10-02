#include "PR/ultratypes.h"
#include "overlays/overlay_056.h"

/* Matched 2026-10-02 (lane x-o058) by rewriting the inherited shape, not by
 * any allocator work: 295 -> 0 masked words at size delta 0, frame 0x60.
 *
 * The previous candidate walked hand-made cursors over alias extern names
 * (D_7C for D_78 + 4, D_94, D_AC, D_C4, a separate "end" symbol for the tag
 * table) and declared nineteen locals.  The function is the matched whale's
 * vocabulary instead: every table is indexed by `i` and strength reduction
 * makes the cursors, the entry count is read from its global at each loop
 * test, and the entry type is the whale's race entry (so the 0x1C byte
 * increment is `counters[class]++`).  That one rewrite is 295 -> 77.
 *
 * The last 77 were one local: the rank-gap clamp reuses `swapped` instead of
 * declaring its own.  A separate `gap` costs two frame cells (0x68 against
 * 0x60) and leaves the constant 10 for initColourCycle unhoisted, which moves
 * the gap into v1 and the table base into a1.  -Wo,-loopunroll,0 was not
 * load-bearing (identical object either way) and is removed. */

typedef struct RcpTextureInfo RcpTextureInfo;

typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct Overlay58RaceEntry {
    u8 character;
    u8 variant;
    u8 variantCopy;
    u8 gap;
    s32 value;
    s32 lapTimes[3];
    u8 pad14[8];
    u8 counters[6];
    u16 rank;
    u8 flags[4];
} Overlay58RaceEntry;

typedef struct Overlay58RaceState {
    u8 mode;
    u8 active;
    u8 player;
    u8 countdown;
    Overlay58RaceEntry entries[6];
} Overlay58RaceState;

/* Tier D: stride and the one field read here. */
typedef struct Overlay58PlayerSlot {
    u8 pad00[0x2A];
    s8 enabled;
    u8 pad2B[9];
} Overlay58PlayerSlot;

typedef struct Overlay58LanguageText {
    char *text[182];
} Overlay58LanguageText;

typedef struct ColourCycle {
    s32 frame;
    s32 time;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    struct ColourCycle *table;
} ColourCycle;

extern Overlay58RaceState *func_80028F54(void);
extern void loadFrontEndList(s16 *assetGroup);
extern void joyResetMap(void);
extern void initColourCycle(ColourCycle *cycle, s32 tableIndex);
extern void overlay58FinalizePackedStatus(void);

extern u8 D_8007BEF8_o058Reloc;
extern u8 D_8007BF0C_o058Reloc;
extern u16 D_8007BF1C_o058Reloc;
extern s32 D_8007BF6C_o058Reloc;
extern Overlay58LanguageText *D_8007C0B8_o058Reloc;
extern s32 D_8007C1B4_o058Reloc;
extern Overlay58PlayerSlot D_800D3058_o058Reloc[];
extern RcpTextureInfo *D_800D31C8_o058Reloc[];

extern s16 D_o058_5AE8[];
extern s32 D_o058_5B28[6];
extern RcpTextureNode D_o058_5B40[];
extern RcpTextureNode D_o058_5B60[];
extern RcpTextureNode D_o058_5B80[];
extern RcpTextureNode D_o058_5BA0[];
extern char *D_o058_5E68[4];
extern s32 D_o058_5E78;
extern s32 D_o058_5E7C;
extern s32 D_o058_5E80;
extern s32 D_o058_5EBC;
extern s32 D_o058_5EC4;
extern Overlay58RaceEntry *D_o058_5EC8[6];
extern Overlay58RaceEntry *D_o058_5EE0[6];
extern s32 D_o058_5EF8[6];
extern s32 D_o058_5F10[6];
extern s32 D_o058_5F28;
extern s32 D_o058_5F2C;
extern s32 D_o058_5F30;
extern ColourCycle D_o058_5F38;
extern s8 D_o058_5F48[4];

void func_overlay_058_F0000000_18AF1E8(void) {
    s32 i;
    s32 swapped;
    s32 minutesA;
    s32 minutesB;
    s32 secondsA;
    s32 secondsB;
    s32 centisA;
    s32 centisB;
    Overlay58RaceState *state;
    Overlay58RaceEntry *entry;

    state = func_80028F54();
    loadFrontEndList(D_o058_5AE8);
    D_o058_5B40[0].texture = D_800D31C8_o058Reloc[3];
    D_o058_5B60[0].texture = D_800D31C8_o058Reloc[4];
    D_o058_5B80[0].texture = D_800D31C8_o058Reloc[5];
    D_o058_5BA0[0].texture = D_800D31C8_o058Reloc[0x7B];
    D_o058_5E80 = 0;
    joyResetMap();
    for (i = 0; i < 4; i++) {
        D_o058_5F48[i] = D_800D3058_o058Reloc[i].enabled;
    }
    D_o058_5EBC = state->active;
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        D_o058_5EC8[i] = &state->entries[i];
        D_o058_5EE0[i] = &state->entries[i];
    }
    if (state->mode == 5) {
        do {
            swapped = 0;
            for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
                if (D_o058_5EC8[i]->value < D_o058_5EC8[i + 1]->value) {
                    entry = D_o058_5EC8[i];
                    D_o058_5EC8[i] = D_o058_5EC8[i + 1];
                    D_o058_5EC8[i + 1] = entry;
                    swapped = 1;
                }
            }
        } while (swapped);
    } else {
        do {
            swapped = 0;
            for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
                if (D_o058_5EC8[i]->value > D_o058_5EC8[i + 1]->value) {
                    entry = D_o058_5EC8[i];
                    D_o058_5EC8[i] = D_o058_5EC8[i + 1];
                    D_o058_5EC8[i + 1] = entry;
                    swapped = 1;
                }
            }
        } while (swapped);
    }
    D_o058_5EF8[0] = 0;
    for (i = 1; i < D_8007BEF8_o058Reloc; i++) {
        overlay56SplitTime(D_o058_5EC8[i - 1]->value, &minutesA, &secondsA, &centisA);
        overlay56SplitTime(D_o058_5EC8[i]->value, &minutesB, &secondsB, &centisB);
        if ((minutesA == minutesB) && (secondsA == secondsB) && (centisA == centisB)) {
            D_o058_5EF8[i] = D_o058_5EF8[i - 1];
        } else {
            D_o058_5EF8[i] = i;
        }
    }
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        if (state->entries == D_o058_5EC8[i]) {
            D_o058_5EC4 = i;
            i = 6;
        }
    }
    if (((D_o058_5EC4 < 4) || (D_8007BF0C_o058Reloc != 0)) && (state->mode != 1)) {
        for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
            D_o058_5EC8[i]->counters[D_o058_5EF8[i]]++;
            D_o058_5EC8[i]->rank += D_o058_5B28[D_o058_5EF8[i]];
            if (D_o058_5EC8[i]->rank > 9999) {
                D_o058_5EC8[i]->rank = 9999;
            }
        }
    } else if ((state->active != 0) && !(D_8007BF1C_o058Reloc & 0x100)) {
        state->countdown--;
    }
    do {
        swapped = 0;
        for (i = 0; i < D_8007BEF8_o058Reloc - 1; i++) {
            if (D_o058_5EE0[i]->rank < D_o058_5EE0[i + 1]->rank) {
                entry = D_o058_5EE0[i];
                D_o058_5EE0[i] = D_o058_5EE0[i + 1];
                D_o058_5EE0[i + 1] = entry;
                swapped = 1;
            }
        }
    } while (swapped);
    for (i = 0; i < D_8007BEF8_o058Reloc; i++) {
        swapped = D_o058_5EE0[0]->rank - D_o058_5EE0[i]->rank;
        if (swapped > 100) {
            swapped = 100;
        }
        D_o058_5EE0[i]->gap = swapped;
    }
    D_o058_5F10[0] = 0;
    for (i = 1; i < D_8007BEF8_o058Reloc; i++) {
        if (D_o058_5EE0[i - 1]->rank == D_o058_5EE0[i]->rank) {
            D_o058_5F10[i] = D_o058_5F10[i - 1];
        } else {
            D_o058_5F10[i] = i;
        }
    }
    D_o058_5F28 = 0;
    initColourCycle(&D_o058_5F38, 10);
    D_o058_5E7C = 0;
    if (D_8007C1B4_o058Reloc == 3) {
        D_o058_5E78 = 3;
    } else {
        D_o058_5E78 = 0;
    }
    if (state->mode != 5) {
        D_o058_5E68[0] = D_8007C0B8_o058Reloc->text[0x24];
    } else {
        D_o058_5E68[0] = D_8007C0B8_o058Reloc->text[0x70];
    }
    D_o058_5E68[1] = D_8007C0B8_o058Reloc->text[0x25];
    D_o058_5E68[2] = D_8007C0B8_o058Reloc->text[0x26];
    D_o058_5E68[3] = D_8007C0B8_o058Reloc->text[0x14];
    overlay58FinalizePackedStatus();
    D_o058_5F2C = 1;
    D_o058_5F30 = 0;
    D_8007BF6C_o058Reloc++;
}
