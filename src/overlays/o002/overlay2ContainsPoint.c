#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
    u16 sourceIndex;
    u16 ownerIndex;
} Overlay2Line;

typedef struct Overlay2Node {
    u16 type;
    u16 index;
    union {
        struct {
            f32 value;
            struct Overlay2Node *side1;
            struct Overlay2Node *side0;
        } branch;
        struct {
            u16 count;
            u16 pad6;
            u32 pad8;
            u32 padC;
        } leaf;
    } data;
} Overlay2Node;

typedef struct Overlay2Shape {
    void *points;
    s32 pointCount;
    s32 pad8;
    struct Overlay2Shape *next;
    u32 flags;
    Overlay2Node *nodes;
    Overlay2Line *lines;
} Overlay2Shape;

extern s32 Arctanf(f32 y, f32 x);
extern s32 mathDiffAngle(s32 first, s32 second);

/* Pinned DKR v77/v80 and JFG scans found no exact point-containment donor. */
s32 overlay2ContainsPoint(f32 x, f32 y, Overlay2Shape *shape) {
    Overlay2Node *node;
    Overlay2Line *line;
    f32 coordinate;
    s32 count;
    s16 firstAngle;
    s16 secondAngle;
    s32 hasMore;

    node = shape->nodes;
    if (node->type == 0) {
        do {
            if (node->index == 0) {
                coordinate = y;
            } else {
                coordinate = x;
            }
            if (coordinate < node->data.branch.value) {
                node = node->data.branch.side1;
            } else {
                node = node->data.branch.side0;
            }
        } while (node->type == 0);
    }

    count = node->data.leaf.count;
    line = &shape->lines[node->index];
    hasMore = count;
    count--;
    if (hasMore != 0) {
        do {
            firstAngle = Arctanf(line->y0 - y, line->x0 - x);
            secondAngle = Arctanf(line->y1 - y, line->x1 - x);
            if (mathDiffAngle(firstAngle, secondAngle) < 0) {
                return 0;
            }
            line++;
            hasMore = count;
            count--;
        } while (hasMore != 0);
    }
    return 1;
}
