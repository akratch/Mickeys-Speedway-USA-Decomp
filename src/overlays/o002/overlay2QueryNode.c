#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

typedef struct Overlay2Node {
    u16 type;
    u16 index;
    union {
        struct {
            u16 count;
            u16 pad6;
        } leaf;
        f32 boundary;
    } data;
    struct Overlay2Node *side1;
    struct Overlay2Node *side0;
} Overlay2Node;

extern s32 D_30;
extern f32 D_34;
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

extern s32 overlay2IntersectSegmentsReloc(f32 x0, f32 y0, f32 x1, f32 y1,
                                     f32 x2, f32 y2, f32 x3, f32 y3,
                                     f32 *outX, f32 *outY);
extern void overlay2IntersectBoundary(f32 x0, f32 y0, f32 x1, f32 y1,
                                      f32 *outX, f32 *outY);

/* Mickey-local reconstruction; pinned DKR/JFG object scans found no donor.
 *
 * Matched by writing the natural shape with no carriers at all: the leaf scan
 * is `while (remaining--)`, the leaf result, the child selection and the
 * all-hits union are plain expressions, and the first-hit case is `||`. The
 * function returns `int`, not `s32`: with a `long` result the `||` value is
 * converted through a named temporary, which is what every earlier carrier
 * shape was imitating. One unreferenced leading local supplies the frame cell
 * above the scan counter.
 */
int overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1, Overlay2Node *node) {
    s32 pad;
    s32 remaining;
    s32 leafResult;
    f32 hitX;
    f32 hitY;
    Overlay2Line *line;

    if (node->type == 1) {
        leafResult = 0;
        remaining = node->data.leaf.count;
        line = &D_3C[node->index];
        while (remaining--) {
            if (overlay2IntersectSegmentsReloc(D_40, D_44, D_48, D_4C,
                                               line->x1, line->y1,
                                               line->x2, line->y2,
                                               &hitX, &hitY) != 0) {
                if (((((D_40 - hitX) * (D_40 - hitX)) +
                      ((D_44 - hitY) * (D_44 - hitY))) > 1.0f) &&
                    ((((D_40 - hitX) * (D_40 - hitX)) +
                      ((D_44 - hitY) * (D_44 - hitY))) < D_6C)) {
                    D_6C = ((D_40 - hitX) * (D_40 - hitX)) +
                           ((D_44 - hitY) * (D_44 - hitY));
                    D_64 = hitX;
                    D_68 = hitY;
                    D_70 = line->value1;
                    D_72 = line->value2;
                }
                if (D_60 != 0) {
                    leafResult = 1;
                } else {
                    return 1;
                }
            }
            line++;
        }
        return (D_60 != 0) ? leafResult : 0;
    }

    D_50 = ((node->index == 0) ? y0 : x0) < node->data.boundary;
    D_54 = ((node->index == 0) ? y1 : x1) < node->data.boundary;

    if (D_50 == D_54) {
        return overlay2QueryNode(x0, y0, x1, y1,
                                 (D_50 != 0) ? node->side1 : node->side0);
    }

    D_30 = node->index;
    D_34 = node->data.boundary;
    overlay2IntersectBoundary(x0, y0, x1, y1, &D_58, &D_5C);

    if (D_50 != 0) {
        if (D_60 != 0) {
            return overlay2QueryNode(x0, y0, D_58, D_5C, node->side1) |
                   overlay2QueryNode(D_58, D_5C, x1, y1, node->side0);
        }
        return overlay2QueryNode(x0, y0, D_58, D_5C, node->side1) ||
               overlay2QueryNode(D_58, D_5C, x1, y1, node->side0);
    }

    if (D_60 != 0) {
        return overlay2QueryNode(x0, y0, D_58, D_5C, node->side0) |
               overlay2QueryNode(D_58, D_5C, x1, y1, node->side1);
    }
    return overlay2QueryNode(x0, y0, D_58, D_5C, node->side0) ||
           overlay2QueryNode(D_58, D_5C, x1, y1, node->side1);
}
