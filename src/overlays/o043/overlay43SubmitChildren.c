#include "PR/ultratypes.h"

typedef struct Overlay43Renderable {
    u8 pad00[0x24];
    void *drawData;
} Overlay43Renderable;

typedef struct Overlay43CountRecord {
    u8 pad00[0x22];
    s8 childCount;
    s8 enabled;
} Overlay43CountRecord;

typedef struct Overlay43ChildRoot {
    u8 pad00[0x40];
    Overlay43CountRecord *counts;
    u8 pad44[0x24];
    Overlay43Renderable ***childRefs;
} Overlay43ChildRoot;

typedef struct Overlay43ChildContainer {
    u8 pad00[0x04];
    Overlay43ChildRoot *root;
} Overlay43ChildContainer;

typedef struct Overlay43Owner {
    u8 pad00[0x40];
    Overlay43CountRecord *gate;
    s16 mode;
    u8 pad46[0x16];
    Overlay43ChildContainer *children;
} Overlay43Owner;

typedef struct Overlay43RenderState {
    u8 pad00[0x34];
    void *primarySlot;
    void *childSlots[11];
    Overlay43Renderable **primaryRef;
    u8 pad68[0x50];
    u8 childCount;
} Overlay43RenderState;

extern s32 makeModelGfx(
    void **slot,
    Overlay43Renderable *renderable,
    s32 renderFlags,
    s32 renderContext,
    s32 arg4,
    s32 alpha,
    s32 enabled);

void overlay43SubmitChildren(Overlay43Owner *owner,
                             Overlay43RenderState *state,
                             s32 renderFlags,
                             s32 renderContext) {
    Overlay43ChildRoot *root;
    Overlay43Renderable **childRef;
    Overlay43Renderable *primary;
    s32 sourceCount;
    u8 normalizedCount;
    s32 index;
    s32 childOffset;

    primary = *state->primaryRef;

    makeModelGfx(
        &state->primarySlot,
        primary,
        renderFlags,
        renderContext,
        0,
        0xFF,
        1);

    if (owner->mode == 1) {
        return;
    }
    if (owner->gate->enabled <= 0) {
        return;
    }

    root = owner->children->root;
    index = 0;
    sourceCount = root->counts->childCount;
    normalizedCount = sourceCount & 0xFF;
    state->childCount = sourceCount;
    if (normalizedCount <= 0) {
        return;
    }

    childOffset = 0;
    do {
        childRef = *(Overlay43Renderable ***)
            ((u8 *)root->childRefs + childOffset);
        makeModelGfx(
            &state->childSlots[index],
            *childRef,
            renderFlags,
            renderContext,
            0,
            0xFF,
            1);

        childOffset += 4;
        index++;
    } while (index < state->childCount);
}
