#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101DrawNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
} Overlay101DrawNode;

typedef struct Overlay101TransformElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    f32 scale;
    s16 rotation;
    u8 color;
    u8 pad13;
    f32 depth;
    u8 pad18[4];
    void *object;
} Overlay101TransformElement;

typedef struct Overlay101Transform {
    s16 rotateZ;
    s16 rotateY;
    s16 rotateX;
    s16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 depth;
} Overlay101Transform;

void overlay101GetDimensions2Reloc(s32 *width, s32 *height);
void overlay101GetBounds2Reloc(Overlay101DrawNode *node, s32 *left, s32 *top,
                               s32 *right, s32 *bottom);
void overlay101SetScissor2Reloc(Overlay101Gfx **displayList, s32 left, s32 top,
                                s32 right, s32 bottom);
void overlay101SetTransformModeReloc(s32 mode);
void overlay101SubmitTransformReloc(Overlay101Gfx **displayList, void *matrix,
                                    void *vertices,
                                    Overlay101Transform *transform,
                                    void *object, s32 rotated, s32 color);

/*
 * Overlay 101 text +0x29A4..+0x2C3C, matched 2026-09-11 (lane p2-close).
 *
 * How the last 36 words came out, because two of the three edits look like
 * noise and are not:
 *
 * 1. `(element->color & 0xFF) | 0xFFFFFF00` -- the mask is a no-op on a `u8`
 *    and folds away at zero byte cost, but cfe still emits it, so ugen
 *    allocates a THIRD expression temporary at that statement instead of two.
 *    ugen hands out caller-saved temporaries from a FIFO free list, and this
 *    candidate was running exactly one place behind the shipped code from the
 *    `0xFB000000` constant onwards: the whole 30-word register-naming residual
 *    was one missing allocation in that list, not a colour and not a web the
 *    allocator refused.  Consuming one extra temporary here puts the list back
 *    in phase, and the residual falls 36 -> 4.
 * 2. Building the final command's `w0` constant before its `w1` constant fixes
 *    which of the two gets the earlier temporary (4 -> 2), and
 * 3. folding that pair onto one physical line retires as1's `lineno` tie so the
 *    two stores keep the shipped order anyway (2 -> 0). [L59]
 *
 * The earlier 62 -> 43 -> 36 work is unchanged and still load-bearing: the
 * rotation test sits after the third command build, which is where the target
 * has it and which frees the 0xFFFFFF00 constant onto a1, and the
 * nested-assignment carriers at the second and fourth commands restore the
 * temporary ring that move rotates.  `right` and `bottom` are declared and
 * unused on purpose -- they reserve the two stack homes the shipped frame has
 * between `syncCommand` and `command` [L121]. `primColorCommand` and
 * `syncCommand` are those two carriers, named for their packets (the
 * permuter's `new_var` and `new_var2`; lane c-5, 2026-10-09).
 */void overlay101DrawTransformed(Overlay101Gfx **displayList, void *matrix,
                               void *vertices, Overlay101DrawNode *node,
                               Overlay101TransformElement *element) {
    s32 bounds0;
    s32 bounds1;
    s32 bounds2;
    s32 bounds3;
    s32 screenWidth;
    s32 screenHeight;
    s32 rotated;
    Overlay101Transform transform;
    Overlay101Gfx *primColorCommand;
    Overlay101Gfx *syncCommand;
    s32 right;
    s32 bottom;
    Overlay101Gfx *command;

    if (((element->color != 0) && (!(element->scale <= 0.0f))) &&
        ((node->type == 2) || (node->type == 4))) {
        overlay101GetDimensions2Reloc(&screenWidth, &screenHeight);
        transform.rotateZ = 0;
        transform.rotateY = 0;
        transform.rotateX = element->rotation;
        transform.scale = element->scale;
        transform.x = ((f32)(node->x + element->x)) -
                      ((f32)(((u32)screenWidth) >> 1));
        transform.y = ((f32)(((u32)screenHeight) >> 1)) -
                      ((f32)(node->y + element->y));
        transform.z = 0.0f;
        transform.depth = element->depth;
        command = (*displayList)++;
        command->w1 = 0;
        command->w0 = 0xE7000000;
        command = (primColorCommand = (*displayList)++);
        command->w0 = 0xFA000000;
        command->w1 = (element->color & 0xFF) | 0xFFFFFF00;
        command = (*displayList)++;
        command->w1 = 0xFFFFFF00;
        command->w0 = 0xFB000000;
        if (element->rotation != 0)
            rotated = 1;
        else
            rotated = 0;
        overlay101GetBounds2Reloc(node, &bounds0, &bounds1, &bounds2, &bounds3);
        overlay101SetScissor2Reloc(displayList, bounds0, bounds1, bounds2,
                                   bounds3);
        if (element->scale == 1.0f)
            overlay101SetTransformModeReloc(0);
        overlay101SubmitTransformReloc(displayList, matrix, vertices,
                                       &transform, element->object, rotated,
                                       element->color);
        if (element->scale == 1.0f)
            overlay101SetTransformModeReloc(1);
        command = (syncCommand = (*displayList)++);
        command->w1 = 0;
        command->w0 = 0xE7000000;
        command = (*displayList)++;
        command->w0 = 0xFA000000; command->w1 = 0xFFFFFFFF;
        overlay101SetScissor2Reloc(displayList, 0, 0, 1000, 1000);
        (void)right;
        (void)bottom;
    }
}
