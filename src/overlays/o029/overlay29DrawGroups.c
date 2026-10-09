#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay29Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay29Group;

typedef struct Overlay29RenderState {
    u8 pad00[0xB];
    s8 enabled;
    u8 pad0C[0x1C];
    Overlay29Group group3;
    Overlay29Group group2;
    Overlay29Group group1;
    Overlay29Group group0;
} Overlay29RenderState;

typedef struct Overlay29Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay29Node;

typedef struct Overlay29ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay29ResourceChoice;

typedef struct Overlay29Context {
    u8 pad00[0x64];
    Overlay29RenderState *render;
    Overlay29Node **nodeTable;
} Overlay29Context;

extern void camPushModelMtx(Gfx **, s32, Overlay29Group *, f32, f32);
extern void camRestoreModelMtx(Gfx **);
extern void texDPInit(Gfx **);

/* PROVENANCE: command expansions use the existing SDK macros in
 * n_audio/mbi.h. Node layout, indexing, call order and command words are
 * reconstructed from Mickey's own overlay. */
void overlay29DrawGroups(Gfx **dl, s32 drawContext,
                         Overlay29Context *context) {
    Overlay29RenderState *render;
    Overlay29Group *group;
    Overlay29Node *node;
    Overlay29ResourceChoice *choice;
    Gfx *gfx;
    u32 resourceSegment;
    s32 groupIndex;

    render = context->render;
    if (render->enabled != 0) {
        groupIndex = 4;
        gDPPipeSync((*dl)++);
        gDPSetEnvColor((*dl)++, 0xFF, 0xFF, 0xFF, 0);

        while (groupIndex--) {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else if (groupIndex == 1) {
                group = &render->group1;
            } else {
                group = &render->group0;
            }

            node = context->nodeTable[4 - groupIndex];
            choice = (Overlay29ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, group->selector);
            camPushModelMtx(dl, drawContext, group, 1.0f, 0.0f);

            gfx = (*dl)++;
            gfx->words.w0 = 0xBF000000;
            gfx->words.w1 = node->segment + 0x80000000;
            gSPDisplayList((*dl)++, resourceSegment + 0x80000000);
            gSP1Triangle((*dl)++, 0, 0, 0, 0);
            camRestoreModelMtx(dl);

            node->consumed = 0;
        }

        texDPInit(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
