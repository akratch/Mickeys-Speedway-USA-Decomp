#include "PR/ultratypes.h"

typedef struct Overlay71Command {
    u32 w0;
    u32 w1;
} Overlay71Command;

typedef struct Overlay71DrawState {
    u8 pad00[0xCC];
    u8 active;
    u8 vertexBank;
    u16 flags;
} Overlay71DrawState;

typedef struct Overlay71DrawObject {
    u8 pad00[0x64];
    Overlay71DrawState *state;
    s32 *resourceIndex;
} Overlay71DrawObject;

extern void *gOverlay71InitialResourceReloc;
extern u8 D_80000008[];
extern u8 D_80000028[];
extern u8 D_800000D8[];

extern void func_80032BF0(void *resource, s32 mode, s32 flags);
extern void func_8002409C(Overlay71Command **commands, s32 context,
                         Overlay71DrawObject *object, f32 scale, f32 extra);
extern void func_80034554(Overlay71Command **commands, s32 resource, s32 mode,
                         s32 flags);
extern void func_800241BC(Overlay71Command **commands);

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDPNoParam, gDPSetColor, gDPSetPrimColor
 * and gDma1p, include/PR/mbi.h _SHIFTL, include/f3ddkr.h gSPVertexJFG and
 * gSPPolygon, include/PR/os_convert.h OS_PHYSICAL_TO_K0), a permitted source
 * under docs/CLEANROOM.md.  No function body was imported; DKR v77/v80 and
 * JFG contain no exact donor for this renderer.
 *
 * Matched 2026-10-01 by writing every display-list command as its macro.
 * Each expansion declares its own block-scoped `_g`, so the twelve command
 * sites are twelve separate symbols instead of the two shared pointer locals
 * (`command`, `head`) the earlier candidate carried, and the `u16 flags`
 * carrier, the `if (1)` region and the hand-folded store lines are gone: a
 * macro is one physical line already.  With those webs in place all four
 * `state->flags` loads take the shipped register with no further lever.
 */
#define O71_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O71_NO_PARAM(pkt, cmd) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = O71_SHIFTL(cmd, 24, 8); _g->w1 = 0; }
#define O71_SET_COLOR(pkt, c, d) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = O71_SHIFTL(c, 24, 8); _g->w1 = (u32)(d); }
#define O71_RGBA(r, g, b, a) (O71_SHIFTL(r, 24, 8) | O71_SHIFTL(g, 16, 8) | O71_SHIFTL(b, 8, 8) | O71_SHIFTL(a, 0, 8))
#define O71_PIPE_SYNC(pkt) O71_NO_PARAM(pkt, 0xE7)
#define O71_SET_ENV_COLOR(pkt, r, g, b, a) O71_SET_COLOR(pkt, 0xFB, O71_RGBA(r, g, b, a))
#define O71_SET_PRIM_COLOR(pkt, m, l, r, g, b, a) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = (O71_SHIFTL(0xFA, 24, 8) | O71_SHIFTL(m, 8, 8) | O71_SHIFTL(l, 0, 8)); _g->w1 = O71_RGBA(r, g, b, a); }
#define O71_DMA1P(pkt, c, s, l, p) { Overlay71Command *_g = (Overlay71Command *)(pkt); _g->w0 = (O71_SHIFTL((c), 24, 8) | O71_SHIFTL((p), 16, 8) | O71_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O71_VERTEX(pkt, v, n, v0) O71_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O71_POLYGON(dl, ptr, numTris, texEnabled) { Overlay71Command *_g = (Overlay71Command *)(dl); _g->w0 = O71_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O71_SHIFTL(0x05, 24, 8) | O71_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O71_PHYSICAL_TO_K0(x) (void *)(((u32)(x) + 0x80000000))
#define O71_VERTEX_BANK(state) O71_PHYSICAL_TO_K0((u32)(state) + (state)->vertexBank * 0x50)

void func_overlay_071_F0000870_18CA390(Overlay71Command **commands,
                                       s32 context,
                                       Overlay71DrawObject *object) {
    Overlay71DrawState *state;

    func_80032BF0(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    if (state->active != 0) {
        func_8002409C(commands, context, object, 1.0f, 0.0f);
        O71_PIPE_SYNC((*commands)++);
        O71_SET_ENV_COLOR((*commands)++, 255, 255, 255, 255);
        if (state->flags & 1) {
            func_80034554(commands, 0, 0x17, 0);
            O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 0xD0);
            O71_VERTEX((*commands)++, O71_VERTEX_BANK(state), 4, 0);
            O71_POLYGON((*commands)++, D_80000008, 2, 0);
            O71_PIPE_SYNC((*commands)++);
        }
        if (state->flags & 6) {
            func_80034554(commands, *object->resourceIndex, 0x17, 0);
            O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 255);
            O71_VERTEX((*commands)++, O71_VERTEX_BANK(state), 8, 0);
            if (state->flags & 4) {
                O71_POLYGON((*commands)++, D_800000D8, 8, 1);
            }
            if (state->flags & 2) {
                O71_POLYGON((*commands)++, D_80000028, 8, 1);
            }
            O71_PIPE_SYNC((*commands)++);
        }
        O71_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, 255);
        func_800241BC(commands);
    }
}
