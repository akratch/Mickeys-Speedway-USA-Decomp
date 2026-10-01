#include "PR/ultratypes.h"

typedef struct Overlay57SelectionChild {
    u8 reserved00[0x28];
    f32 value28;
    u8 reserved2C[0x0F];
    s8 selector3B;
} Overlay57SelectionChild;

typedef struct Overlay57SelectionResult {
    u8 reserved00[8];
    Overlay57SelectionChild *child;
} Overlay57SelectionResult;

typedef union Overlay57LocalWord {
    s32 word;
    f32 value;
} Overlay57LocalWord;

extern s32 gO57SelectionList8Reloc[];
extern s32 gO57SelectionList30Reloc[];
extern s32 gO57SelectionCurrent100Reloc[];
extern s32 gO57SelectionPending104Reloc;
extern s32 gO57SelectionPrevious108Reloc;
extern Overlay57LocalWord gO57SelectionValue10CReloc;
extern s32 gO57SelectionState118Reloc;
extern s32 gO57SelectionMode11CReloc;
extern s32 gO57SelectionTimer120Reloc;
extern s32 gO57SelectionIds134Reloc[];
extern s32 gO57SelectionActive144Reloc;
extern s32 gO57SelectionValues158Reloc[];
extern s32 gO57SelectionValues17CReloc[];
extern s32 gO57SelectionPrimary4E8Reloc;
extern s32 gO57SelectionSecondary4ECReloc;
extern s32 gO57SelectionChanging4F0Reloc;
extern s32 gO57SelectionDistance50CReloc;

extern u32 gOverlay57Flags37ECReloc;
extern s32 gOverlay57Mode38C4Reloc;
extern s16 gOverlay57Threshold38D8Reloc;
extern s16 gOverlay57Threshold3970Reloc;
extern s32 gOverlay57PublishedIndex3A28Reloc;

extern s32 overlay57Call35F8Reloc(void);
extern void overlay57Call3618Reloc(s32 *primary, s32 *secondary);
extern Overlay57SelectionResult *overlay57Call3684Reloc(u8 id);
extern void *overlay57Call36F4Reloc(u8 id);
extern Overlay57SelectionResult *overlay57Call3758Reloc(u8 id);
extern s32 overlay57Call37E0Reloc(void);
extern void overlay57Call3880Reloc(s32 command, s32 argument);
extern void overlay57Call3888Reloc(s32 argument);
extern s32 overlay57Call38ACReloc(void);
extern void overlay57Call3920Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call3940Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call3960Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39B0Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39D0Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39F0Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);

/* Overlay 57 text +0x35E0..+0x3A4C. Matched 2026-10-01 (lane d-o057) from 162
 * masked words by discarding the inherited shape: no volatile primary-state
 * pointer and no sentinel local (uopt shares the one read across the head
 * comparisons itself), the id walks are `while (*list != -1)` with the list
 * read at each use, the tail is three early returns rather than an else-if
 * chain (which is what keeps the float literals and the table base out of
 * callee-saved registers), and the published index is read back after the
 * store. The two trailing locals are frame cells (L99). */
void overlay57UpdateSelection(s32 updateRate) {
    s32 newPrimary;
    s32 newSecondary;
    Overlay57SelectionResult *result;
    s32 *list;
    s32 entry;
    s32 unused1;
    s32 unused2;

    if (overlay57Call35F8Reloc() != 2) {
        return;
    }
    gO57SelectionActive144Reloc = 1;
    overlay57Call3618Reloc(&newPrimary, &newSecondary);
    if ((newPrimary != gO57SelectionPrimary4E8Reloc) ||
        (newSecondary != gO57SelectionSecondary4ECReloc)) {
        if ((gO57SelectionPrimary4E8Reloc != 0xFF) &&
            (newPrimary != gO57SelectionPrimary4E8Reloc)) {
            list = gO57SelectionIds134Reloc;
            while (*list != -1) {
                if (*list == gO57SelectionPrimary4E8Reloc) {
                    result = overlay57Call3684Reloc(*list);
                    if ((result != NULL) && (result->child != NULL) &&
                        (newSecondary != result->child->selector3B)) {
                        overlay57SetNodeValue(*list, gO57SelectionValues17CReloc[*list], 0.007f);
                    }
                }
                list++;
            }
        }
        if (newPrimary != 0xFF) {
            if (overlay57Call36F4Reloc(newPrimary) != NULL) {
                overlay57SetNodeValue(newPrimary, newSecondary, 0.01f);
                gO57SelectionChanging4F0Reloc = 1;
            }
        }
        gO57SelectionPrimary4E8Reloc = newPrimary;
        gO57SelectionSecondary4ECReloc = newSecondary;
    }

    if ((gO57SelectionChanging4F0Reloc != 0) && (gO57SelectionPrimary4E8Reloc != 0xFF)) {
        result = overlay57Call3758Reloc(gO57SelectionPrimary4E8Reloc);
        if ((result != NULL) && (gO57SelectionValue10CReloc.value < result->child->value28)) {
            gO57SelectionChanging4F0Reloc = 0;
            list = gO57SelectionIds134Reloc;
            while (*list != -1) {
                if (*list == gO57SelectionPrimary4E8Reloc) {
                    overlay57SetNodeValue(*list, gO57SelectionValues17CReloc[*list], 0.007f);
                }
                list++;
            }
        }
    }

    if (overlay57Call37E0Reloc() != -1) {
        list = gO57SelectionIds134Reloc;
        while (*list != -1) {
            overlay57SetNodeValue(*list, gO57SelectionValues158Reloc[*list], 0.01f);
            list++;
        }
        gO57SelectionState118Reloc = 4;
        gO57SelectionTimer120Reloc = 10;
        return;
    }
    if ((gOverlay57Flags37ECReloc & 0x4000) && (gO57SelectionDistance50CReloc == 0)) {
        overlay57Call3880Reloc(0xD, 0);
        overlay57Call3888Reloc(0);
        gO57SelectionState118Reloc = 5;
        gO57SelectionMode11CReloc = 0;
        return;
    }
    entry = overlay57Call38ACReloc();
    if (entry == gO57SelectionCurrent100Reloc[0]) {
        return;
    }
    if (gOverlay57Mode38C4Reloc == 1) {
        if ((gOverlay57Threshold38D8Reloc < -16) && (gO57SelectionDistance50CReloc == 0)) {
            overlay57Call3920Reloc(((void **)gO57SelectionList8Reloc)[entry], -0xA0, 0xBE, 4);
        } else {
            overlay57Call3940Reloc(((void **)gO57SelectionList8Reloc)[entry], 0x1E0, 0xBE, 4);
        }
        overlay57Call3960Reloc(((void **)gO57SelectionList8Reloc)[gO57SelectionCurrent100Reloc[0]], 0xA0, 0x104, 0x104);
    } else {
        if ((gOverlay57Threshold3970Reloc < -16) && (gO57SelectionDistance50CReloc == 0)) {
            overlay57Call39B0Reloc(((void **)gO57SelectionList30Reloc)[entry], -0xA0, 0xBE, 4);
        } else {
            overlay57Call39D0Reloc(((void **)gO57SelectionList30Reloc)[entry], 0x1E0, 0xBE, 4);
        }
        overlay57Call39F0Reloc(((void **)gO57SelectionList30Reloc)[gO57SelectionCurrent100Reloc[0]], 0xA0, 0x104, 0x104);
    }
    gO57SelectionPrevious108Reloc = gO57SelectionCurrent100Reloc[0];
    gO57SelectionCurrent100Reloc[0] = entry;
    gO57SelectionValue10CReloc.word = gO57SelectionPending104Reloc;
    gO57SelectionPending104Reloc = 0xFF;
    gOverlay57PublishedIndex3A28Reloc = gO57SelectionCurrent100Reloc[0];
}
