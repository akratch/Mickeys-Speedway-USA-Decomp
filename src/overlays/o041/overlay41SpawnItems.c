#include "PR/ultratypes.h"

typedef struct Overlay41Root {
    u8 pad0[8];
    void *item;
} Overlay41Root;

typedef struct Overlay41Item {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    s32 kind;
    u8 pad18[0x38];
    f32 *opacity;
} Overlay41Item;

typedef struct Overlay41ShortPair {
    s16 x;
    s16 y;
    s16 z;
} Overlay41ShortPair;

typedef struct Overlay41FloatVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay41FloatVector;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor. */
extern Overlay41Root **D_800D6B00;
extern s32 mathRnd(s32 lower, s32 upper);
extern void mathOneFloatPY(Overlay41ShortPair *input,
                           Overlay41FloatVector *output);
extern void func_overlay_012_F00001B4_186D434(
    f32 x, f32 y, s32 kind, f32 outX, f32 outY, f32 z, s32 opacity, s32 mode,
    f32 scale);

/* The compiler-generated switch table and scalar constant are linked against
 * the overlay's retained data through an ABS symbol. Their duplicate payload
 * is discarded after a digest check; no instruction word is changed. */
void overlay41SpawnItems(s32 rootIndex, s32 count, s32 mode, s32 centerX,
                         s32 centerY, s32 radiusX, s32 radiusY, s32 centerZ,
                         s32 radiusZ) {
    s32 opacity;
    Overlay41ShortPair input;
    Overlay41FloatVector output;
    Overlay41Root *root;
    Overlay41Item *item;
    f32 scale;

    root = D_800D6B00[rootIndex];
    scale = 1.0f;
    if (root == 0) {
        return;
    }
    item = root->item;
    if (item == 0) {
        return;
    }

    switch (mode) {
        case 0:
            mode = 0;
            break;
        case 1:
            mode = 1;
            break;
        case 2:
            mode = 1;
            scale = 0.5f;
            break;
        case 3:
            mode = 5;
            scale = 0.75f;
            break;
        case 4:
            mode = 4;
            break;
        default:
            mode = 2;
            break;
    }

    if (item->opacity != 0) {
        opacity = *item->opacity * 255.0f;
    } else {
        opacity = 255;
    }

    while (count--) {
        input.x = mathRnd(-radiusX, radiusX) + centerX;
        input.y = mathRnd(-radiusY, radiusY) + centerY;
        output.z =
            (f32)(mathRnd(-radiusZ, radiusZ) + centerZ) * -0.01f;
        mathOneFloatPY(&input, &output);
        func_overlay_012_F00001B4_186D434(
            item->x, item->y, item->kind, output.x, output.y, output.z, opacity,
            mode, scale);
    }
}
