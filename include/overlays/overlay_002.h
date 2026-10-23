#ifndef OVERLAY_002_H
#define OVERLAY_002_H

#include "PR/ultratypes.h"
#include "game/math.h"

/* A boundary line of a collision shape. */
typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

typedef struct Overlay2Point {
    s16 x;
    s16 y;
} Overlay2Point;

/* A region of the partition being built: a line range plus its split. */
typedef struct Overlay2Region {
    s32 boundaryAxis;
    f32 boundaryValue;
    struct Overlay2Region *side1;
    struct Overlay2Region *side0;
    u16 start;
    u16 count;
} Overlay2Region;

/* A built partition node: a split (type 0) or a leaf line range (type 1). */
typedef struct Overlay2Node {
    u16 type;
    u16 index;
    union {
        f32 boundary;
        struct {
            u16 count;
            u16 pad6;
        } leaf;
    } data;
    struct Overlay2Node *side1;
    struct Overlay2Node *side0;
} Overlay2Node;

/* A collision shape: its outline points and built partition. */
typedef struct Overlay2Shape {
    Overlay2Point *points;
    s32 pointCount;
    s32 pad8;
    struct Overlay2Shape *next;
    s32 flags;
    Overlay2Node *nodes;
    Overlay2Line *lines;
} Overlay2Shape;

typedef struct Overlay2BoundaryCandidate {
    s32 lineIndex;
    u16 side0Count;
    u16 side1Count;
    u16 crossingCount;
    u16 axis;
    f32 value;
} Overlay2BoundaryCandidate;

typedef struct Overlay2Hit {
    s16 x;
    s16 y;
    u16 index;
    s16 previous;
    s16 next;
    u16 owner;
    s32 flags;
} Overlay2Hit;

typedef struct Overlay2RouteState {
    u8 group;
    u8 order;
} Overlay2RouteState;

typedef struct Overlay2ObjectHeader {
    u8 pad00[0xB];
    u8 order;
} Overlay2ObjectHeader;

typedef struct Overlay2RouteObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay2ObjectHeader *header;
    u8 pad40[4];
    s16 type;
    u8 pad46[0x1E];
    Overlay2RouteState *route;
    u8 pad68[0x29];
    u8 disabled;
} Overlay2RouteObject;

typedef struct Overlay2RouteInput {
    u8 pad00[0xA];
    u8 group;
    u8 order;
} Overlay2RouteInput;

typedef struct Overlay2ColorObject {
    u8 pad0[0x64];
    u8 *state;
} Overlay2ColorObject;

extern Overlay2Line *gOverlay2Lines;
extern s32 gOverlay2LineCount;
extern Overlay2Region *gOverlay2Regions;
extern s32 gOverlay2RegionCount;
extern s32 gOverlay2BoundaryAxis;
extern f32 gOverlay2BoundaryValue;
extern s32 gOverlay2TemporaryMode;
extern Overlay2Region gOverlay2TemporaryRange;
extern Overlay2BoundaryCandidate *gOverlay2BoundaryCandidates;
extern s32 gOverlay2BoundaryCandidateCount[];
extern s32 gOverlay2SelectedBoundary;
extern s32 gOverlay2BuiltNodeCount;
extern s32 gOverlay2BuiltLineCount;

/* The query state block, under the names each reader was matched with. */
extern s32 D_30;
extern f32 D_34;
extern Overlay2Node *D_38;
extern Overlay2Line *D_3C;
extern f32 D_40;
extern f32 D_44;
extern f32 D_48;
extern f32 D_4C;
extern s32 D_50;
extern s32 D_54;
extern f32 D_58;
extern f32 D_5C;
extern s32 D_60;
extern f32 D_64;
extern f32 D_68;
extern f32 D_6C;
extern u16 D_70;
extern u16 D_72;
extern Overlay2Line *gOverlay2QueryLinesReloc;
extern f32 gOverlay2QueryX0Reloc;
extern f32 gOverlay2QueryY0Reloc;
extern f32 gOverlay2QueryX1Reloc;
extern f32 gOverlay2QueryY1Reloc;
extern Overlay2Hit *gOverlay2QueryHitReloc;
extern f32 gOverlay2QueryHitXReloc;
extern f32 gOverlay2QueryHitYReloc;
extern f32 gOverlay2QueryBestReloc;
extern u16 gOverlay2QueryIndexReloc;
extern f32 gOverlay2QueryLimitReloc;
extern f32 gOverlay2QueryResultReloc;

extern s32 func_8002A910(f32 y, f32 x);
extern s32 func_8002AA0C(s32 current, s32 target);
extern s32 mathDiffAngle(s32 first, s32 second);
extern void *mmAlloc(s32 size, s32 tag);
extern void _bzero(void *memory, int size);
extern s32 mmGetDelay(void);
extern void mmSetDelay(s32 state);
extern void mmFree(void *memory);
extern void mmAllocAtAddr(s32 size, void *memory, s32 tag);
extern Overlay2RouteObject **func_8000572C(s32 *start, s32 *end);
extern f32 func_8000BCB0(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                         f32 z1);
extern u32 joyGetButtons(s32 controller);
/* Cross-overlay callee, reached through a runtime relocation record. */
extern u16 overlay1GetEntryIndex(Overlay2Shape *object);
/* Overlay 1's function, called through a placeholder: a value line for
 * overlay1GetEntry itself would override its definition in every module. */
extern Overlay2Shape *overlay1GetEntry(u16 index);

/*
 * Calls to the module's own exports are SYMBOL relocation records: the
 * shipped word stores the 0xF0000000 addend, not the callee's offset, so
 * they name undefined placeholders until the overlay tables are linker
 * output.
 */
extern s32 overlay2ContainsPoint_o002Reloc(f32 x, f32 y, Overlay2Shape *shape);
extern void func_overlay_002_F0000C90_1857A88_o002Reloc(void *object, s32 enabled);
extern s32 func_overlay_002_F0001364_185815C_o002Reloc(f32 x0, f32 y0, f32 x1, f32 y1,
                                                     f32 x2, f32 y2, f32 x3, f32 y3,
                                                     f32 *outX, f32 *outY);

#endif
