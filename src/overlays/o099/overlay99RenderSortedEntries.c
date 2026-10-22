#include "game/math.h"
#include "overlays/overlay_099.h"

/* Overlay 99 +0x800: sorted translucent-entry render pass. */

typedef struct Overlay99Transform {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay99Transform;

extern f32 camGetProjZ(f32 x, f32 y, f32 z);
extern Overlay99Transform *camGetPtr(void);
extern void func_80022E80(Overlay99RenderState *state);
extern void func_8002AA50(Overlay99RenderState *state, MtxF matrix);
extern void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                                             f32 *outX, f32 *outY, f32 *outZ);
extern void func_80022FD4(Gfx **displayList, Mtx **matrices,
                                       void *vertices,
                                       Overlay99RenderState *state,
                                       f32 *opacity,
                                       Overlay99CameraSprite *sprite,
                                       s32 mode, s32 selector);
extern f32 gOverlay99IntensityScale;
extern f32 gOverlay99TransformZ;

/*
 * The declaration list is a frame census, not a style choice.  uopt homes
 * every declared local at a descending word from the top of the 0x148 frame,
 * in declaration order, whether or not the local is ever touched through the
 * stack.  Reading the target's own homes back gives eighteen scalar slots
 * around the five aggregates: three above `savedX`, five between `savedZ` and
 * `distances`, six between `sprite` and `sorted`, four below it.  The body
 * uses nine of them; the rest are declared as `unused*` because the census is
 * what is known -- which scalar sat in which slot is not, and is provably
 * free: permuting the nine live names across the eighteen slots leaves the
 * object byte-identical.  Only the counts and the aggregate boundaries matter.
 */
void overlay99RenderSortedEntries(Gfx **displayList, Mtx **matrices,
                                  void *vertices, Overlay99RenderState *state,
                                  f32 intensityScale) {
    s32 count;
    s32 i;
    s32 j;
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    f32 invScale;
    Overlay99RenderEntry *entry;
    Overlay99TableOwner *owner;
    Overlay99Transform *transform;
    Overlay99Vec3 *vec;
    f32 distances[4];
    MtxF matrix;
    Overlay99CameraSprite sprite;
    Gfx *command;
    s32 unused0;
    s32 unused1;
    s32 unused2;
    s32 unused3;
    s32 unused4;
    Overlay99RenderEntry *sorted[4];
    s32 unused5;
    s32 unused6;
    s32 unused7;
    s32 unused8;

    entry = state->entries;
    owner = *state->tableOwner;
    count = 0;
    if (entry != 0) {
        i = 0;
        while ((i < state->entryCount) && (i != 4)) {
            vec = &owner->vectors[entry->tableIndex];
            distances[count] = camGetProjZ(vec->x, vec->y, vec->z);
            sorted[count] = entry;
            entry++;
            i++;
            count++;
        }
    }

    if (count <= 0) {
        return;
    }

    for (i = count - 1; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (distances[j + 1] < distances[j]) {
                invScale = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = invScale;
                entry = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = entry;
            }
        }
    }

    transform = camGetPtr();
    savedX = transform->x;
    savedY = transform->y;
    savedZ = transform->z;
    transform->x = 0.0f;
    transform->y = 0.0f;
    transform->z = gOverlay99TransformZ;
    func_80022E80(state);
    transform->x = savedX;
    transform->y = savedY;
    transform->z = savedZ;

    invScale = 1.0f / *state->unitScale;
    func_8002AA50(state, matrix);
    sprite.frame = (s16)(s32)(intensityScale * gOverlay99IntensityScale);
    sprite.angle = 0;
    sprite.divisor = 3;
    sprite.frameCount = 0x3333;
    sprite.matrixScale = 1.0f;

    command = *displayList;
    *displayList = command + 1;
    command->words.w1 = 0;
    command->words.w0 = 0xE7000000;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFB000000; command->words.w1 = 0xFFFFFF00;

    for (i = 0; i < count; i++) {
        entry = sorted[i];
        vec = &owner->vectors[entry->tableIndex];
        sprite.transformScale = entry->scale * invScale;
        sprite.spriteData = entry->spriteData;
        mtxf_transform_point(matrix, vec->x, vec->y, vec->z,
                                         &sprite.x, &sprite.y, &sprite.z);
        func_80022FD4(displayList, matrices, vertices, state,
                                   state->opacity, &sprite, 14,
                                   state->mode39);
    }
}
