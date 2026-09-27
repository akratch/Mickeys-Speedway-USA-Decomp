#include "PR/ultratypes.h"

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    s32 dataOffset;
    s32 frameSize;
} Overlay44FrameSource;

typedef struct Overlay44AnimationState {
    s8 sourceIndex;
    u8 storageMode;
    u8 flags;
    u8 subtype;
    s32 phase;
    s16 value8;
    s16 valueA;
    u8 pad0C[2];
    s8 protectedSlot0;
    s8 protectedSlot1;
    s8 cachedFrame[4];
    void *handles[4];
} Overlay44AnimationState;

extern Overlay44FrameSource *gOverlay44FrameSources;
extern s32 overlay44UploadFrameReloc(u32 assetIndex, void *address,
                                     s32 assetOffset, s32 size);

/* Mickey-local reconstruction, tier A: stock object and linked ROM exact.
 * A shared source offset preserves the frame without unused padding.
 * The resource data offset is ordinary storage; only the scalar metadata
 * reads retain the volatile view required by this compiler schedule. */
void overlay44UpdateFrameCache(Overlay44AnimationState *state,
                               s32 updateRate) {
    s32 sourceOffset;
    s32 limit;
    s32 delta;
    s32 nextFrame;
    s32 frameSlot;
    s32 nextSlot;
    s32 slot;
    s32 frame;
    volatile Overlay44FrameSource *source;

    if ((state == 0) || (state->sourceIndex == -1)) {
        return;
    }

    source = &gOverlay44FrameSources[state->sourceIndex];
    if (state->flags & 4) {
        limit = (source->frameCount << 8) - 1;
    } else {
        limit = (source->frameCount - 1) << 8;
    }

    if (state->flags & 0x80) {
        state->flags &= ~0x80;
    } else {
        delta = ((source->speed << 8) / 60) * updateRate;
        do {
            if (state->flags & 1) {
                state->phase -= delta;
                if (state->phase < 0) {
                    if (state->flags & 4) {
                        if (state->flags & 2) {
                            state->phase = -state->phase;
                            state->flags &= ~1;
                        } else {
                            state->phase += limit;
                        }
                    } else {
                        state->phase = 0;
                    }
                }
            } else {
                state->phase += delta;
                if (state->phase > limit) {
                    if (state->flags & 2) {
                        state->phase = limit - (state->phase - limit);
                        state->flags |= 1;
                    } else if (state->flags & 4) {
                        state->phase -= limit;
                    } else {
                        state->phase = limit;
                    }
                }
            }
        } while ((state->phase < 0) || (state->phase > limit));
    }

    frameSlot = -1;
    frame = state->phase >> 8;
    nextFrame = frame + 1;
    nextSlot = -1;
    if (nextFrame >= source->frameCount) {
        nextFrame = 0;
    }

    slot = 3;
    do {
        if (state->cachedFrame[slot] == frame) frameSlot = slot;
        if (state->cachedFrame[slot] == nextFrame) nextSlot = slot;
    } while (slot--);

    if (frameSlot < 0) {
        slot = 3;
        do {
            if ((slot != nextSlot) && (slot != state->protectedSlot0) &&
                (slot != state->protectedSlot1)) {
                sourceOffset = ((Overlay44FrameSource *)source)->dataOffset + (frame * source->frameSize);
                state->cachedFrame[slot] = frame;
                frameSlot = slot;
                overlay44UploadFrameReloc(0x42, state->handles[slot], sourceOffset,
                                          source->frameSize);
                break;
            }
        } while (slot--);
    }

    if (nextSlot < 0) {
        slot = 3;
        do {
            if ((slot != frameSlot) && (slot != state->protectedSlot0) &&
                (slot != state->protectedSlot1)) {
                sourceOffset = ((Overlay44FrameSource *)source)->dataOffset + (nextFrame * source->frameSize);
                state->cachedFrame[slot] = nextFrame;
                nextSlot = slot;
                overlay44UploadFrameReloc(0x42, state->handles[slot], sourceOffset,
                                          source->frameSize);
                break;
            }
        } while (slot--);
    }

    state->protectedSlot0 = frameSlot;
    state->protectedSlot1 = nextSlot;
}
