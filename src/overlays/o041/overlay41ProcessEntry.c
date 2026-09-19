#include "PR/ultratypes.h"

typedef struct Overlay41Input {
    u8 pad00[0xC];
    f32 scale;
    u8 pad10[4];
    u8 alternateMode;
} Overlay41Input;

typedef struct Overlay41State {
    u8 pad00[0x1E];
    s8 status;
    u8 pad1F[0xB5];
    f32 selector;
} Overlay41State;

typedef struct Overlay41Child {
    u8 pad00[0x11];
    u8 switchEnabled;
    u8 pad12[0x3C];
    s8 refreshEnabled;
} Overlay41Child;

typedef struct Overlay41Entry {
    Overlay41Child *child;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *values[1];
} Overlay41Entry;

typedef struct Overlay41Context {
    u8 pad00[0x28];
    u8 work[0x12];
    s8 entryIndex;
    u8 pad3B[5];
    Overlay41State *state;
    u8 pad44[0xC];
    void *argument;
    u8 pad54[0x14];
    Overlay41Entry **entries;
    u8 pad6C[0x27];
    u8 stateIndex;
} Overlay41Context;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor. */
extern void overlay41StartEntry(Overlay41Entry *, s32 *, s32, void *, s32);
extern void overlay41RefreshEntry(Overlay41Entry *, Overlay41Child *,
                                  Overlay41Context *, Overlay41Input *);
extern void overlay41ApplyEntry(Overlay41Context *, Overlay41Entry *, void *,
                                void *);
extern void overlay41FinishEntry(Overlay41Context *, Overlay41Child *,
                                 Overlay41Entry *, s32);

/* Matched 2026-09-19 (lane w31-o041p), 120 -> 0 masked words at delta 0.
 *
 *   - `scaled = (s32)(input->scale * 60.0f)` before the alternateMode test
 *     (delete remat): input stays in a3 instead of its incoming home, which
 *     was the extra word at +0x10 and the five-word overrun (L155).
 *   - unused `pad` declared first (L99): an unused pointer is not eliminated
 *     and occupies +0x44 so mode/child sit at +0x40/+0x38. Listed in
 *     docs/cleanup-queue.md.
 */
void func_overlay_041_F0001464_188879C(Overlay41Input *input,
                                       Overlay41Context *context,
                                       s32 argument) {
    Overlay41Input *pad;
    s32 mode;
    Overlay41Entry *entry;
    Overlay41Child *child;
    Overlay41State *state;
    s32 selected;
    Overlay41Entry *firstEntry;
    s32 scaled;

    state = context->state;
    selected = state->selector != 0.0f;
    if (((selected != 0) &&
         (((Overlay41State *)((u8 *)state + context->stateIndex))->status == 1)) ||
        ((selected == 0) && (state->status == 1))) {
        firstEntry = context->entries[context->entryIndex];
        if (firstEntry != 0) {
            scaled = (s32)(input->scale * 60.0f);
            mode = 9;
            if (input->alternateMode != 0) {
                mode = 11;
            }
            overlay41StartEntry(firstEntry, &mode, scaled, context->work, argument);
        }
    } else if (((selected != 0) &&
                (((Overlay41State *)((u8 *)state + context->stateIndex))->status == 0)) ||
               ((selected == 0) && (state->status == 0))) {
        entry = context->entries[context->entryIndex];
        if (entry->active != 0) {
            child = entry->child;
            if (child->refreshEnabled != 0) {
                overlay41RefreshEntry(entry, child, context, input);
                overlay41ApplyEntry(context, entry, context->argument,
                                    entry->values[entry->index]);
            } else {
                if (child->switchEnabled != 0) {
                    entry->index = 1 - entry->index;
                }
                overlay41ApplyEntry(context, entry, context->argument,
                                    entry->values[entry->index]);
            }
            if (child->switchEnabled > 0) {
                overlay41FinishEntry(context, child, entry, entry->active);
            }
            entry->active = 0;
        }
    }
}
