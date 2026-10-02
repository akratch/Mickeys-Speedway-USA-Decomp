#include "PR/ultratypes.h"

typedef struct Overlay73Command {
    u32 w0;
    u32 w1;
} Overlay73Command;

typedef struct Overlay73Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay73Vertex;

typedef struct Overlay73DrawState {
    Overlay73Vertex vertices[12];
    void *resource;
    u8 vertexBank;
} Overlay73DrawState;

typedef struct Overlay73DrawObject {
    u8 pad00[0x39];
    u8 alpha;
    u8 pad3A[0x2A];
    Overlay73DrawState *state;
} Overlay73DrawObject;

extern u8 D_80000000[];
extern void func_8002409C(Overlay73Command **commands, s32 context,
                         Overlay73DrawObject *object, f32 scale, f32 extra);
extern void func_80034554(Overlay73Command **commands, void *resource,
                         s32 mode, s32 flags);
extern void func_800241BC(Overlay73Command **commands);

/*
 * PROVENANCE: the packet macros below are adapted from the Jet Force Gemini
 * decompilation (include/PR/gbi.h gDPSetPrimColor and gDma1p, include/PR/mbi.h
 * _SHIFTL, include/f3ddkr.h gSPVertexJFG and gSPPolygon,
 * include/PR/os_convert.h OS_PHYSICAL_TO_K0), a permitted source under
 * docs/CLEANROOM.md, by way of the matched overlay 71 renderer
 * func_overlay_071_F0000870_18CA390, whose shape this function copies.  No
 * function body was imported.
 *
 * Matched 2026-10-02 as a sibling copy of that overlay 71 renderer: every
 * command is its packet macro, and the vertex bank is a subscript into an
 * array of ten-byte vertices, `vertices[vertexBank * 6]`, so the address is
 * the index times six times the element size -- the shipped 3*2 then 5*2
 * multiply chain, which a single `* 60` byte offset folds into 15*4.  The
 * unused `pad` between `vertices` and `state` lands the 0x30 frame and the
 * vertices spill at +0x2C.
 */
#define O73_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O73_RGBA(r, g, b, a) (O73_SHIFTL(r, 24, 8) | O73_SHIFTL(g, 16, 8) | O73_SHIFTL(b, 8, 8) | O73_SHIFTL(a, 0, 8))
#define O73_SET_PRIM_COLOR(pkt, m, l, r, g, b, a) { Overlay73Command *_g = (Overlay73Command *)(pkt); _g->w0 = (O73_SHIFTL(0xFA, 24, 8) | O73_SHIFTL(m, 8, 8) | O73_SHIFTL(l, 0, 8)); _g->w1 = O73_RGBA(r, g, b, a); }
#define O73_DMA1P(pkt, c, s, l, p) { Overlay73Command *_g = (Overlay73Command *)(pkt); _g->w0 = (O73_SHIFTL((c), 24, 8) | O73_SHIFTL((p), 16, 8) | O73_SHIFTL((l), 0, 16)); _g->w1 = (unsigned int)(s); }
#define O73_VERTEX(pkt, v, n, v0) O73_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1))) + 8, ((n))<<3|(((u32)(v) & 6))|(v0))
#define O73_POLYGON(dl, ptr, numTris, texEnabled) { Overlay73Command *_g = (Overlay73Command *)(dl); _g->w0 = O73_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | O73_SHIFTL(0x05, 24, 8) | O73_SHIFTL(((numTris)*16), 0, 16); _g->w1 = (unsigned int)(ptr); }
#define O73_PHYSICAL_TO_K0(x) (void *)(((u32)(x) + 0x80000000))

void func_overlay_073_F0000D70_18CB830(Overlay73Command **commands,
                                       s32 context,
                                       Overlay73DrawObject *object) {
    Overlay73Vertex *vertices;
    s32 pad;
    Overlay73DrawState *state;

    state = object->state;
    if (state->resource != NULL) {
        vertices = &state->vertices[state->vertexBank * 6];
        func_8002409C(commands, context, object, 1.0f, 0.0f);
        O73_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, object->alpha);
        func_80034554(commands, state->resource, 0xE, 0);
        O73_VERTEX((*commands)++, O73_PHYSICAL_TO_K0(vertices), 6, 0);
        O73_POLYGON((*commands)++, D_80000000, 8, 1);
        O73_SET_PRIM_COLOR((*commands)++, 0, 0, 255, 255, 255, object->alpha);
        func_800241BC(commands);
    }
}
