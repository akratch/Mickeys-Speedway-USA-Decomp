#include "PR/ultratypes.h"
typedef struct Overlay3State { u8 pad000[1]; s8 selector; u8 pad002[0x3B4]; s16 scoreHigh; s16 scoreLow; u8 pad3BA[0x40]; s16 blocked; } Overlay3State;
typedef struct Overlay3Object { u8 pad00[0x0C]; f32 x; f32 y; f32 z; u8 pad18[0x4C]; Overlay3State *state; } Overlay3Object;
typedef struct Overlay3Search { u8 pad000[0x38D]; u8 cachedIndex; u16 timer; u8 pad390[0x18]; u8 weights[1]; } Overlay3Search;
extern Overlay3Object **overlay3GetSearchObjectsReloc(s32 *count);
extern s32 overlay3ContainsValueReloc(Overlay3Object *anchor, Overlay3Object *object);
extern s32 overlay3RandomRangeReloc(s32 low, s32 high);
extern f32 overlay3SqrtReloc(f32 value);
/*
 * Matched 2026-09-16 (lane lm-a), 118/118 words at frame 0x80, unforced.
 * Two facts, both measured (docs/lastmile-region-boundary.md):
 *   - the loop subscripts `objects[index]` and lets strength reduction own
 *     the walking cursor (L160). With a declared cursor the helper result
 *     was one web coloured v0 unopposed; generated, uopt keeps the call
 *     result for the cached path, copies it into a1 before the guard, and
 *     both reloads of `count` take v0, which is the ROM's shape;
 *   - the frame is 0x80 only with one more pointer-typed declaration than
 *     the body uses: `cursor` is kept, unused, for that reason. An unused
 *     f32 in its place is eliminated and the frame falls to 0x78. Listed in
 *     docs/cleanup-queue.md.
 */
Overlay3Object *overlay3SelectScoredObject(Overlay3Object *anchor, Overlay3Search *search, s32 elapsed) {
    s32 count; Overlay3Object **objects; Overlay3Object **cursor; Overlay3Object *result;
    Overlay3Object *object; Overlay3State *state; s32 index; s32 bestScore;
    s32 bestIndex; s32 score; f32 dx; f32 dz; u16 timer; u8 cachedIndex;
    objects = overlay3GetSearchObjectsReloc(&count);
    result = 0;
    if ((elapsed < search->timer) && ((cachedIndex = search->cachedIndex) != 0x7F)) {
        search->timer = search->timer - elapsed;
        result = objects[cachedIndex];
    } else {
        bestScore = -1000000;
        index = count - 1;
        if (count != 0) {
            do {
                object = objects[index];
                if ((object != anchor) && (object->state->blocked == 0) &&
                    (overlay3ContainsValueReloc(anchor, object) == 0)) {
                    score = overlay3RandomRangeReloc(0, 2000);
                    state = object->state;
                    score += (state->scoreHigh - state->scoreLow) * 1000;
                    dx = anchor->x - object->x;
                    dz = anchor->z - object->z;
                    score -= ((s32)overlay3SqrtReloc((dx * dx) + (dz * dz))) / 8;
                    score += search->weights[state->selector] << 8;
                    if (bestScore < score) {
                        bestScore = score;
                        bestIndex = index;
                        result = object;
                    }
                }
            } while (index--);
        }
        if (result != 0) {
            search->cachedIndex = bestIndex;
            search->timer = overlay3RandomRangeReloc(3, 6) * 60;
        }
    }
    return result;
}
