#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct Overlay26Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay26Group;

typedef struct Overlay26RenderState {
    u8 pad00[0x30];
    s8 enabled;
    u8 pad31[3];
    Overlay26Group group3;
    Overlay26Group group2;
    Overlay26Group group1;
    Overlay26Group group0;
} Overlay26RenderState;

typedef struct Overlay26Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay26Node;

typedef struct Overlay26ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay26ResourceChoice;

typedef struct Overlay26Context {
    u8 pad00[0x50];
    void *drawData;
    u8 pad54[0x10];
    Overlay26RenderState *render;
    Overlay26Node **nodeTable;
} Overlay26Context;

extern void o26PrepareNode(Overlay26Context *, Overlay26Node *, void *, s32);
extern void o26DrawReloc(Gfx **, void *, Overlay26Group *, f32, f32);
extern void o26FlushReloc(Gfx **);
extern void o26FinishReloc(Gfx **);

/* PROVENANCE: command expansions use the existing SDK macros in
 * n_audio/mbi.h. Node layout, indexing, call order and command words are
 * reconstructed from Mickey's own overlay. */
void func_overlay_026_F0001158_187B550(Gfx **dl, void *drawContext,
                                       Overlay26Context *context) {
    Overlay26RenderState *render;
    Overlay26Group *group;
    Overlay26Node *node;
    Overlay26ResourceChoice *choice;
    Gfx *gfx;
    u32 resourceSegment;
    s32 groupIndex;

    render = context->render;
    if (render->enabled != 0) {
        gDPPipeSync((*dl)++);
        groupIndex = 4;
        gDPSetEnvColor((*dl)++, 0xFF, 0xFF, 0xFF, 0);

        while (groupIndex--) {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else {
                if (groupIndex == 1) {
                    group = &render->group1;
                } else {
                    group = &render->group0;
                }
            }

            node = context->nodeTable[4 - groupIndex];
            choice = (Overlay26ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            o26PrepareNode(context, node, context->drawData, 0);
            gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, group->selector);
            o26DrawReloc(dl, drawContext, group, 1.0f, 0.0f);

            gfx = (*dl)++;
            gfx->words.w0 = 0xBF000000;
            gfx->words.w1 = node->segment + 0x80000000;
            gSPDisplayList((*dl)++, resourceSegment + 0x80000000);
            gSP1Triangle((*dl)++, 0, 0, 0, 0);
            o26FlushReloc(dl);

            node->consumed = 0;
        }

        o26FinishReloc(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
