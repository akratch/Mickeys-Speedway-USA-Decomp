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
 * 214/253 relocation-masked words, exact 0x3F4 size, exact 0x68 frame, 51 of
 * 51 relocations.  The 39 differing words are six sites, and every one of them
 * is a carrier colour rather than a structure: at each, the shipped code
 * spends `a0` (or one lower ring temp) where this candidate spends the next
 * one.  `a0` and `a1` are pure scratch in this function -- every call here
 * passes its first two floats in f12/f14, so no argument ever lands in them.
 *
 * The frame is closed.  T = frame = 0x68 and the declaration chain runs
 * count/remaining/leafResult at 0x64/0x60/0x5C, hitX 0x58, hitY 0x54, line
 * 0x50, recursiveResult 0x4C, so S = 0x1C and an eighth local moves the frame
 * to 0x70.  That is why every attempt to name the short-circuit boolean in a
 * fresh local collapses, and why the L88 "name it and drop another local"
 * composition has nowhere to go: the seven that are there are all live.
 *
 * Eliminated by earlier lanes: leaf-zero return (151), a scalar in place of
 * the one-element array (99), all four or-operand orders (byte-identical --
 * uopt normalises them), naming the short-circuit boolean in five carriers
 * (78-107), and a 48-cell composition cross flooring at 39.  Added here, all
 * flat at 39 or size-changing: the ternary and short-circuit-or spellings of
 * both short-circuit blocks (93-97 where they compile); 42 adjacent
 * source-line joins across the whole body; a 25-cell cross of five leaf-head
 * and five leaf-tail spellings (including `count = remaining--`, the comma
 * operator in the loop condition, and an explicit post-loop reset); 20
 * named-boolean carrier x branch-polarity forms, every one of which changes
 * the size; a 1,115-cell randomised cross of declaration order, local
 * qualifiers and types, five or-block spellings and four node-selection forms;
 * and the callee return-type lattice that closed overlay 5 -- making
 * `overlay2IntersectBoundary` non-void, `overlay2IntersectSegments` void, or
 * changing this function's own return type -- which is inert here.
 *
 * Lane c6-band-b (2026-09-10). First, the allocator is exonerated: 168 single
 * web forces on the instrumented globalcolor profile -- all fourteen integer
 * allocator webs crossed with the twelve caller-saved colours -- leave the
 * object at 39 words or worse. The "carrier colour" reading above is a
 * description of the symptom, not of a reachable decision.
 *
 * Second, the shape of one residue is now known exactly. At each of the two
 * short-circuit blocks the shipped code emits
 * `sltu <t>,zero,v0 / bnez <t>,end / move v0,<t>`, which is the signature of
 * `a || b` and not of `if (a) { return 1; }`: it normalises the first call's
 * result to a boolean and returns that boolean, where this candidate branches
 * and materialises a literal 1. Spelling both blocks with `||` reproduces
 * that instruction triple exactly in the object -- and costs four bytes per
 * block, because the two-`if` form keeps two distinct `node->side` loads
 * either side of a `beqzl` branch-likely delay slot and the `||` form lets
 * uopt fold them into one. So the function comes out eight bytes short and
 * the trade is exact: the boolean shape and the duplicated load are wanted
 * together and no spelling found so far gives both. All four `||` spellings
 * (`(a != 0) || (b != 0)`, the bare form, and the two mixed ones) are
 * byte-identical, as are the single-block variants at four bytes short; and
 * twelve empty `if (1) {}` region markers placed at every statement boundary
 * of the tail (L97's join-point use, which is exactly the "stop uopt folding
 * two reads into one" lever) do not restore the load.
 *
 * Lane p9-mid (2026-09-12).  The eight-byte shortfall of the `||` spelling is
 * one named instruction.  At each mirror block the shipped code emits three
 * words -- normalise the call result to a boolean, branch on the boolean to the
 * epilogue, copy the boolean into the return register in the delay slot -- and
 * then issues BOTH of the second call's stack-passed float arguments
 * afterwards.  The two-`if` form emits four words there, one of which is a
 * hoisted load of the second call's third argument in the branch delay slot,
 * and so does not reissue that argument later.  The counts balance exactly, so
 * the requirement is the shipped triple TOGETHER WITH a separate reissue of
 * that argument, and that reissue is the whole of the eight bytes.  It is not
 * reachable by splitting a range: the argument is loaded from a stack slot the
 * frame already holds, not from the global, so the address form on either call,
 * the volatile address form, and `volatile` on the file-scope declarations of
 * both globals are all byte-identical to the plain `||` at eight bytes short.
 * Routing the first call's arguments through the dead float locals costs 24
 * bytes; a dead pointer local for the node selection is still eight short.
 *
 * Third, `node = node->side1/side0` before the tail call puts the selected
 * child in `node`'s own s1 where the target uses a scratch a0. The ternary
 * argument form is byte-identical to the assignment; both two-return forms,
 * with and without `else`, fail to tail-merge and grow the function by 20
 * bytes.
 *
 * Lane w10-o002 (2026-09-17). Identity-gated IDO (proc 0, 40 p1, 20 coloured).
 * On this shape count occupies a0; node is not offered a0. Generated subscript
 * and delete-count grow. The 41-shape force of the loop-tail temp onto a0 is
 * 32 and is not source-reachable: the temp is the only v0-offered web in its
 * block, and the intersect call that would deny v0 also loads a0.
 */
#ifdef NON_MATCHING
s32 overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1,
                      Overlay2Node *node) {
    register s32 count;
    s32 remaining;
    s32 leafResult;
    f32 hitX;
    f32 hitY;
    Overlay2Line *line;
    s32 recursiveResult[1];

    if (node->type == 1) {
        leafResult = 0;
        remaining = node->data.leaf.count;
        count = remaining;
        line = &D_3C[node->index];
        remaining--;
        if (count != 0) {
            do {
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
                count = remaining;
                line++;
            } while (remaining--);
        }
        if (D_60 != 0) {
            return leafResult;
        }
        return count;
    }

    D_50 = ((node->index == 0) ? y0 : x0) < node->data.boundary;
    D_54 = ((node->index == 0) ? y1 : x1) < node->data.boundary;

    if (D_50 == D_54) {
        if (D_50 != 0) {
            node = node->side1;
        } else {
            node = node->side0;
        }
        return overlay2QueryNode(x0, y0, x1, y1, node);
    }

    D_30 = node->index;
    D_34 = node->data.boundary;
    overlay2IntersectBoundary(x0, y0, x1, y1, &D_58, &D_5C);

    if (D_50 != 0) {
        if (D_60 != 0) {
            recursiveResult[0] =
                overlay2QueryNode(x0, y0, D_58, D_5C, node->side1);
            return overlay2QueryNode(D_58, D_5C, x1, y1, node->side0) |
                   recursiveResult[0];
        }
        if (overlay2QueryNode(x0, y0, D_58, D_5C, node->side1) != 0) {
            return 1;
        }
        return overlay2QueryNode(D_58, D_5C, x1, y1, node->side0) != 0;
    }

    if (D_60 != 0) {
        recursiveResult[0] =
            overlay2QueryNode(x0, y0, D_58, D_5C, node->side0);
        return overlay2QueryNode(D_58, D_5C, x1, y1, node->side1) |
               recursiveResult[0];
    }
    if (overlay2QueryNode(x0, y0, D_58, D_5C, node->side0) != 0) {
        return 1;
    }
    return overlay2QueryNode(D_58, D_5C, x1, y1, node->side1) != 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/overlay2QueryNode/func_overlay_002_F00016A0_1858498.s")
#endif

/* PLATEAU-HANDOFF:overlay2QueryNode:start
 * symbol: overlay2QueryNode
 * score: 39/253 words
 * frame: 0x68
 * relocations: 51
 * first-mismatch: +0x58
 * summary: IDO gated. Index/delete-count grow. 41-shape w69=a0 force is 32. Tail temp uniquely offered v0 in its block; denying v0 also denies a0.
 * PLATEAU-HANDOFF:overlay2QueryNode:end
 */
