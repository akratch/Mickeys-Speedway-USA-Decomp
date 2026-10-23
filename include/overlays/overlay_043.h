#ifndef OVERLAY_043_H
#define OVERLAY_043_H

#include "PR/ultratypes.h"

typedef f32 MtxF[4][4];

/* Called with a matrix by the projection builder and with the motion output
 * block by the motion helper; unprototyped so both views pass a pointer. */
extern void func_8002A82C();

/* ---- overlay43InitializeState ---- */
typedef struct Overlay43Command {
    s16 type;
    u8 mode;
    u8 byte03;
    s16 short04;
    s16 width;
    s16 height;
    s16 short0A;
    s16 short0C;
    s16 scale;
    s16 short10;
    s16 short12;
    void *data;
    s16 short18;
    u8 byte1A;
    u8 byte1B;
    u8 byte1C;
    u8 byte1D;
    u8 byte1E;
    u8 byte1F;
} Overlay43Command;

typedef struct Overlay43State {
    f32 x;
    f32 y;
    Overlay43Command *command;
    s16 short0C;
    s16 short0E;
    u8 flags;
    u8 byte11;
    u8 byte12;
    u8 byte13;
    u8 pad14[0x0C];
    u8 *bufferEnd;
    u8 pad24[0x3C];
    void **current;
    void **parent;
    u8 pad68[0x44];
    f32 alpha;
    u8 padB0[0x0A];
    u8 enabled;
} Overlay43State;

typedef struct Overlay43Source {
    f32 x;
    f32 y;
    u8 pad08[8];
    u8 flags;
    u8 byte11;
    u8 byte12;
    u8 pad13[9];
    Overlay43State *state;
} Overlay43Source;

typedef struct Overlay43Parent {
    u8 pad00[0x60];
    s8 linkIndex;
} Overlay43Parent;

typedef struct Overlay43Input {
    u8 pad00[0x3A];
    s8 currentIndex;
    u8 pad3B[5];
    Overlay43Parent *parent;
    u8 pad44[8];
    Overlay43Source *source;
    u8 pad50[0x18];
    void ***entries;
} Overlay43Input;

extern u8 D_0[];

/* ---- overlay43FlushPending ---- */
typedef struct Overlay43PendingEntry {
    u8 pad00[0x20];
    s32 arg20;
    u8 pad24[0x04];
    s32 arg28;
    u8 pad2C[0x04];
    s32 arg30;
    u8 pad34[0x85];
    u8 pending;
} Overlay43PendingEntry;

typedef struct Overlay43PendingState {
    u8 pad00[0xC8];
    s8 pendingCount;
} Overlay43PendingState;

extern Overlay43PendingState D_C8;
extern Overlay43PendingEntry *D_120[];
extern u8 ext_4d258[];
extern void func_8002E800(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void osRecvMesg(void *queue, s32 *message, s32 flags);

/* ---- overlay43ReleaseResources ---- */
typedef struct Overlay43ResourceOwner {
    u8 pad00[0x24];
    void *resource24;
    void *resource28;
    void *resource2C;
    u8 pad30[0x04];
    void *resource34;
    void *children[12];
    void *resource68;
    u8 pad6C[0x4C];
    u8 childCount;
} Overlay43ResourceOwner;

extern void func_8002B768(void *resource);
extern void func_80020184(void *resource);

/* ---- func_overlay_043_F0000324_188A2F4 ---- */
typedef struct Overlay43Node {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[9];
    u8 priority;
    u8 pad16[0xA];
} Overlay43Node;

typedef struct Overlay43NodeSet {
    u8 pad00[0xE];
    s16 count;
    Overlay43Node nodes[1];
} Overlay43NodeSet;

typedef struct Overlay43Record {
    MtxF matrix;
    Overlay43Node *node;
    s32 alpha;
} Overlay43Record;

typedef struct Overlay43VertexBound {
    u8 pad00[8];
    f32 radius;
} Overlay43VertexBound;

typedef struct Overlay43ModelDefinition {
    u8 pad00[0x2F];
    u8 vertexCount;
    u8 pad30[8];
    Overlay43VertexBound *bounds;
} Overlay43ModelDefinition;

typedef struct Overlay43VertexPosition {
    f32 x;
    f32 y;
    f32 z;
} Overlay43VertexPosition;

typedef struct Overlay43ModelInstance {
    Overlay43ModelDefinition *definition;
    u8 pad04[4];
    s16 active;
    u8 pad0A[0x3E];
    Overlay43VertexPosition *positions;
} Overlay43ModelInstance;

typedef struct Overlay43ProjectState {
    f32 scaleX;
    f32 scaleY;
    u8 pad08[9];
    u8 maximumRecords;
    u8 pad12[0x1E];
    void *displayListEnd;
    u8 pad34[0x2C];
    Overlay43ModelInstance *model;
    u8 pad64[8];
    MtxF transform;
    f32 fallbackScale;
    f32 diameter;
    s16 yaw;
    s16 pitch;
    u8 padB8;
    u8 pending;
    u8 boundsMode;
} Overlay43ProjectState;

typedef struct Overlay43Link {
    u8 pad00[0x1C];
    Overlay43ProjectState *state;
} Overlay43Link;

typedef struct Overlay43ScaleSource {
    u8 pad00[0x5C];
    f32 scale;
} Overlay43ScaleSource;

typedef struct Overlay43ProjectInput {
    u8 pad00[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x28];
    Overlay43ScaleSource *scaleSource;
    u8 pad44[8];
    Overlay43Link *link;
    Overlay43NodeSet *nodeSet;
} Overlay43ProjectInput;

typedef struct Overlay43Level {
    u8 pad00[0xD8];
    s16 yaw;
    s16 pitch;
    u8 padDC[6];
    u8 priority;
} Overlay43Level;

typedef struct Overlay43Transform {
    s16 yaw;
    s16 pitch;
    s16 roll;
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay43Transform;

/*
 * Overlay 43 ("clone.c"): build the per-node projection records for one
 * clone and size its 64x64 image, then hand the records to the renderer.
 *
 * Matched from 494 masked words at -4 by rewriting the inherited m2c shape
 * from the listing and the relocation records:
 *  - the record array and the queue are their own array symbols (the queue
 *    store is `symbol(index)`, not a base register), and the fallback's
 *    alpha is a direct store to element 0 by its own name, which is the
 *    word the old candidate was short;
 *  - the fallback record is read back from records[0] into a local before
 *    the identity call, so the copy that survives the call is a register
 *    move; alpha is stored before the count;
 *  - the yaw/pitch pair handed to the motion helper is an s16 array, so the
 *    store to element 1 makes the state stores re-read element 0;
 *  - priority is a local at all three helper calls and the vertex pointer
 *    is a local taken before the radius, which fixes the argument and ring
 *    draw order;
 *  - two float locals carry several short roles each (node x, vertex
 *    radius and diameter; node z, the blend fraction, the minimum x and
 *    the tolerance test), which is what lands the five saved float
 *    registers and the frame;
 *  - the three transform angles are cleared roll first as separate
 *    statements (a chained clear costs a frame cell);
 *  - the TU builds with -Wab,-r4300_mul: the target keeps the radius
 *    multiply out of the call's delay slot.
 *
 * PROVENANCE: no code adapted. Callee names are the tree's adopted names
 * for the resident functions the relocation records select.
 */
extern s8 gO43QueueCountReloc;
extern s32 D_8007A3C8;
extern Overlay43Record gO43RecordsReloc[];
extern Overlay43ProjectState *gO43QueueReloc[];
extern Overlay43Record gO43FallbackReloc;
#define gO43Records gO43RecordsReloc
#define gO43QueueCount gO43QueueCountReloc
#define gO43RenderPending D_8007A3C8
#define gO43Fallback gO43FallbackReloc

extern Overlay43Level *levelGetLevel(void);
extern s16 Arctanf(f32 x, f32 y);
extern f32 sqrtf(f32 value);
extern s16 dAngle(s16 from, s16 to, f32 fraction);
extern void func_8002AE10(Overlay43Transform *transform, MtxF matrix);
extern void mtxf_mul(MtxF a, MtxF b, MtxF out);
extern void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                                 f32 *outX, f32 *outY, f32 *outZ);

/* ---- func_overlay_043_F0000BE4_188ABB4 ---- */
/*
 * Overlay 43 ("clone.c"): render each queued clone into its own 64x64 8-bit
 * intensity image.  The two trace calls carry the source file name and the
 * original line numbers 596 and 675.
 *
 * Matched 2026-10-02 by rewriting the inherited m2c shape from the listing:
 * one GBI packet macro per command on the address-taken list cursor, a
 * counted loop over the entries indexed from the top, the K0-to-physical
 * conversion as libultra's subtraction, and the per-file -mips1 override
 * removed (the target fills no load-delay slots, so it is MIPS II).  The
 * last ring draw was the display-list start: the target holds it in a
 * register local (`start`) and copies it into the cursor, so the trace
 * call's argument draws nothing.  The frame's two extra cells are `start`
 * and one unused local below the matrices.
 *
 * PROVENANCE: no code adapted.  The GBI macro bodies follow the published
 * libultra gbi.h encodings (F3D movewords as gImmp21) that the shipped words
 * decode to.
 */

typedef struct {
    u32 w0;
    u32 w1;
} Gwords;

typedef union {
    Gwords words;
    long long force_structure_alignment;
} Gfx;


typedef struct {
    long m[4][4];
} Mtx;

#define _SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define OS_K0_TO_PHYSICAL(x) (u32)(((char *)(x) - 0x80000000))

#define gDma0p(pkt, c, s, l) {                                              \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8) | _SHIFTL((l), 0, 24);               \
    _g->words.w1 = (u32)(s);                                                \
}
#define gDma1p(pkt, c, s, l, p) {                                           \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p), 16, 8) |              \
                    _SHIFTL((l), 0, 16));                                   \
    _g->words.w1 = (u32)(s);                                                \
}
#define gImmp1(pkt, c, p0) {                                                \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8);                                     \
    _g->words.w1 = (u32)(p0);                                               \
}
#define gImmp21(pkt, c, p0, p1, dat) {                                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p0), 8, 16) |             \
                    _SHIFTL((p1), 0, 8));                                   \
    _g->words.w1 = (u32)(dat);                                              \
}
#define gMoveWd(pkt, index, offset, data) \
    gImmp21((pkt), 0xBC, (offset), (index), (data))
#define gDPNoParam(pkt, cmd) {                                              \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((cmd), 24, 8);                                   \
    _g->words.w1 = 0;                                                       \
}
#define gDPSetColor(pkt, c, d) {                                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL((c), 24, 8);                                     \
    _g->words.w1 = (u32)(d);                                                \
}
#define DPRGBColor(pkt, cmd, r, g, b, a)                                    \
    gDPSetColor(pkt, cmd, (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) |          \
                           _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8)))
#define gDPSetEnvColor(pkt, r, g, b, a) DPRGBColor(pkt, 0xFB, r, g, b, a)
#define gDPSetPrimColor(pkt, m, l, r, g, b, a) {                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xFA, 24, 8) | _SHIFTL((m), 8, 8) |             \
                    _SHIFTL((l), 0, 8));                                    \
    _g->words.w1 = (_SHIFTL(r, 24, 8) | _SHIFTL(g, 16, 8) |                 \
                    _SHIFTL(b, 8, 8) | _SHIFTL(a, 0, 8));                   \
}
#define gDPSetFillColor(pkt, d) gDPSetColor(pkt, 0xF7, d)
#define gSPSetOtherMode(pkt, cmd, sft, len, data) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL((cmd), 24, 8) | _SHIFTL((sft), 8, 8) |          \
                    _SHIFTL((len), 0, 8));                                  \
    _g->words.w1 = (u32)(data);                                             \
}
#define G_CYC_FILL 0x00300000
#define gDPSetCycleType(pkt, type) gSPSetOtherMode(pkt, 0xBA, 20, 2, type)
#define G_IM_FMT_I 4
#define G_IM_SIZ_8b 1
#define gDPSetColorImage(pkt, fmt, siz, width, i) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xFF, 24, 8) | _SHIFTL((fmt), 21, 3) |           \
                   _SHIFTL((siz), 19, 2) | _SHIFTL((width) - 1, 0, 12);     \
    _g->words.w1 = (u32)(i);                                                \
}
#define G_SC_NON_INTERLACE 0
#define gDPSetScissor(pkt, mode, ulx, uly, lrx, lry) {                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = _SHIFTL(0xED, 24, 8) |                                   \
                   _SHIFTL((int)((float)(ulx) * 4.0F), 12, 12) |            \
                   _SHIFTL((int)((float)(uly) * 4.0F), 0, 12);              \
    _g->words.w1 = _SHIFTL((mode), 24, 2) |                                 \
                   _SHIFTL((int)((float)(lrx) * 4.0F), 12, 12) |            \
                   _SHIFTL((int)((float)(lry) * 4.0F), 0, 12);              \
}
#define gDPFillRectangle(pkt, ulx, uly, lrx, lry) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xF6, 24, 8) | _SHIFTL((lrx), 14, 10) |         \
                    _SHIFTL((lry), 2, 10));                                 \
    _g->words.w1 = (_SHIFTL((ulx), 14, 10) | _SHIFTL((uly), 2, 10));        \
}
#define gSPClearGeometryMode(pkt, word) gImmp1(pkt, 0xB6, word)
#define gSPTexture(pkt, s, t, level, tile, on) {                            \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (_SHIFTL(0xBB, 24, 8) | _SHIFTL((level), 11, 3) |        \
                    _SHIFTL((tile), 8, 3) | _SHIFTL((on), 0, 8));           \
    _g->words.w1 = (_SHIFTL((s), 16, 16) | _SHIFTL((t), 0, 16));            \
}
#define gSPViewport(pkt, v) gDma1p((pkt), 0x03, (v), 16, 0x80)
#define gSPMatrix(pkt, m, p) gDma1p((pkt), 0x01, (m), 64, (p))
#define gSPDisplayList(pkt, dl) gDma1p(pkt, 0x06, dl, 0, 0)
#define gDPFullSync(pkt) gDPNoParam(pkt, 0xE9)
#define gSPEndDisplayList(pkt) gDPNoParam(pkt, 0xB8)
#define FRUSTRATIO_2 2
#define gSPClipRatio(pkt, r) {                                              \
    gMoveWd(pkt, 0x04, 0x04, (u32)(r) & 0xFFFF);                            \
    gMoveWd(pkt, 0x04, 0x0C, (u32)(r) & 0xFFFF);                            \
    gMoveWd(pkt, 0x04, 0x14, (u32)(-(r)) & 0xFFFF);                         \
    gMoveWd(pkt, 0x04, 0x1C, (u32)(-(r)) & 0xFFFF);                         \
}

typedef struct Overlay43Model {
    u8 pad00[0x0A];
    s16 partIndex;
    void *parts[1];
} Overlay43Model;

typedef struct Overlay43VertexData {
    u8 pad00[4];
    void *vertices;
} Overlay43VertexData;

typedef struct Overlay43RenderState {
    u8 pad00[0x20];
    void *framebuffer;
    u8 pad24[4];
    Gfx *displayList;
    Mtx *matrixHeap;
    Gfx *displayListEnd;
    void *modelDisplayList;
    u8 pad38[0x28];
    Overlay43Model *model;
    Overlay43VertexData *vertexData;
    u8 pad68[4];
    MtxF transform;
    u8 padAC[0xD];
    u8 pending;
} Overlay43RenderState;

typedef struct Overlay43RenderLink {
    u8 pad00[0x1C];
    Overlay43RenderState *state;
} Overlay43RenderLink;

typedef struct Overlay43RenderInput {
    u8 pad00[0x4C];
    Overlay43RenderLink *link;
} Overlay43RenderInput;

typedef struct Overlay43RenderEntry {
    MtxF matrix;
    u8 pad40[4];
    s32 alpha;
} Overlay43RenderEntry;

/* Resident callees: diRcpTrace (diRcpTrace), rsp_segment,
 * rcpInitDpNoSize, mtxf_mul and mtxf_to_mtx. */
extern void diRcpTrace(Gfx *dl, char *file, s32 line);
extern void rsp_segment(Gfx **dl, s32 segment, void *base);
extern void rcpInitDpNoSize(Gfx **dl);
extern void mtxf_mul(MtxF a, MtxF b, MtxF out);
extern void mtxf_to_mtx(MtxF src, Mtx *dest);

/* Module-relative bases: .rodata (the two "clone.c" strings), .data (the
 * two fixed matrices at +0x38/+0x78 and the viewport at +0xB8) and .bss
 * (the queue at +0x120).  The queue count at .data+0xC8 is addressed as
 * its own symbol: the target materialises its full address once and
 * reads and writes it at displacement 0.  Two resident data words are
 * reached through the reserved data selector. */
extern char gO43RodataBaseReloc[];
extern u8 gO43DataBaseReloc[];
extern u8 gO43BssBaseReloc[];
extern u8 gO43VertexSourceReloc[];
extern s32 D_8007A3C8;
extern s8 gO43QueueCountReloc;

#define gO43TraceFile0 (gO43RodataBaseReloc)
#define gO43TraceFile1 (gO43RodataBaseReloc + 0x10)
#define gO43CloneMatrixA (*(MtxF *)(gO43DataBaseReloc + 0x38))
#define gO43CloneMatrixB (*(MtxF *)(gO43DataBaseReloc + 0x78))
#define gO43Viewport (gO43DataBaseReloc + 0xB8)
#define gO43QueueCount gO43QueueCountReloc
#define gO43Queue ((Overlay43RenderState **)(gO43BssBaseReloc + 0x120))

/* ---- overlay43ComputeMotion ---- */
typedef struct Overlay43RotationInput {
    s16 pad00;
    s16 angle;
} Overlay43RotationInput;

typedef struct Overlay43MotionOutput {
    f32 unk00;
    u8 pad04[0x0C];
    f32 unk10;
    f32 unk14;
    f32 unk18;
    u8 pad1C[0x0C];
    f32 unk28;
    u8 pad2C[0x18];
    s32 owner;
} Overlay43MotionOutput;

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

extern void func_80029FE4(Overlay43RotationInput *input, Vec3f *direction);
extern void func_8002A82C(Overlay43MotionOutput *output);

/* ---- overlay43AllocateResources ---- */
typedef struct Overlay43AllocInput {
    u8 pad00[0x44];
    s16 mode;
} Overlay43AllocInput;

typedef struct Overlay43Resources {
    u8 pad00[0x20];
    u8 *aligned;
    void *primary;
    void *secondary;
    void *tertiary;
} Overlay43Resources;

extern void *mmAlloc(s32 size, s32 tag);
extern void func_8002B768(void *pointer);

/* ---- overlay43SubmitChildren ---- */
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

typedef struct Overlay43ChildState {
    u8 pad00[0x34];
    void *primarySlot;
    void *childSlots[11];
    Overlay43Renderable **primaryRef;
    u8 pad68[0x50];
    u8 childCount;
} Overlay43ChildState;

extern s32 makeModelGfx(
    void **slot,
    Overlay43Renderable *renderable,
    s32 renderFlags,
    s32 renderContext,
    s32 arg4,
    s32 alpha,
    s32 enabled);

/* ---- overlay43FilterImage ---- */
typedef struct Overlay43Image {
    u8 pad0[0x20];
    u8 *pixels;
} Overlay43Image;

#endif
