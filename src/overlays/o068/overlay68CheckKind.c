#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no exact donor for this probe check. */

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    u8 pad00[0x10];
    void *slots[4];
    s16 values[4];
} Overlay68Probe;

typedef struct Overlay68ResidentEntry {
    s32 thresholdNumerator;
    u8 pad04[0x1C];
} Overlay68ResidentEntry;

/* Both aliases bind to the same overlay-local kind/amount table. */
extern const Overlay68KindPair gOverlay68KindMapInitial[];
extern volatile const s8 gOverlay68KindMapLoop;

/* The runtime relocation stream preserves these five distinct call roles. */
extern void *func_8002B280(s32 size, u32 colourTag);
extern s32 piRomLoadSection(u32 assetIndex, u32 address,
                                    s32 assetOffset, s32 size);
extern u8 *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 kind);
extern void mmFree(void *probe);

/* Matched 2026-09-17 (lane w5-o068), 10 -> 0 masked words at delta 0,
 * frame 0x48, nine relocations, unforced. L145/L160: delete the walking
 * valueCursor local and subscript probe->values[index] so IDO generates
 * the scaled cursor. A while, not a guarded do, keeps that index as a
 * preheader variable so as1 emits the scaled-zero init. L67: the map-loop
 * back-edge keeps the copy-propagated -1 first only when currentKind is
 * materialised at the test; the inert xor is tracked in
 * docs/cleanup-queue.md. */
s32 overlay68CheckKind(s32 kind) {
    s32 amount;
    const Overlay68KindPair *mapping;
    volatile const s8 *loopMapping;
    s32 currentKind;
    s32 result;
    s32 threshold;
    Overlay68Probe *probe;
    s32 value;
    Overlay68ResidentEntry *entries;
    s32 index;

    result = 0;
    amount = -1;
    mapping = gOverlay68KindMapInitial;

    if (mapping->kind != -1) {
        loopMapping = &gOverlay68KindMapLoop;
        currentKind = *loopMapping;
        do {
            if (kind == currentKind) {
                amount = mapping->amount;
                break;
            }
            mapping++;
            currentKind = mapping->kind;
            /* Legacy inert xor preserves IDO's comparison operand order;
             * tracked in docs/cleanup-queue.md. */
        } while ((currentKind ^ 0) != -1);
    }

    if (amount != -1) {
        probe = func_8002B280(sizeof(*probe), 0x85);
        if (probe != NULL) {
            piRomLoadSection(0x3F, (u32)probe,
                                    amount * (s32)sizeof(*probe),
                                    sizeof(*probe));
            entries = (Overlay68ResidentEntry *)func_800291C4();
            index = levelGetBlurEffect(kind);
            threshold = entries[index].thresholdNumerator / 5;
            if (threshold == 0) {
                threshold = 0x7080;
            }

            value = 0x8CA0;
            index = 0;
            while ((index < 4) && (probe->slots[index] != NULL)) {
                value = probe->values[index];
                index++;
            }

            if (threshold < value) {
                result = 1;
            }
        }
        mmFree(probe);
    }

    return result;
}
