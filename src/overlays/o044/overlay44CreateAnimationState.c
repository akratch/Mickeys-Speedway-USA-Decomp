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

extern Overlay44AnimationState gOverlay44StatePool[3];
extern Overlay44FrameSource *gOverlay44FrameSources;
extern void *overlay44AllocateFrameReloc(s32 size, s32 tag);
extern void overlay44CleanupStateReloc(Overlay44AnimationState *state);
extern s32 overlay44UploadFrameReloc(u32 assetIndex, void *address,
                                     s32 assetOffset, s32 size);
extern void overlay44FinishFramesReloc(void);

/* Mickey-local reconstruction; no external donor body was used. */
Overlay44AnimationState *overlay44CreateAnimationState(
    s32 sourceIndex, s32 value8, s32 valueA, s32 subtype, s32 flags) {
    Overlay44AnimationState *state;
    volatile Overlay44FrameSource *source;
    s32 remainingSlot;
    s32 storageScale;
    s32 allocationSize;
    s32 handleIndex;

    state = &gOverlay44StatePool[0];
    remainingSlot = 2;
    while ((state->sourceIndex >= 0) && remainingSlot--) {
        state++;
    }
    if (remainingSlot < 0) {
        return 0;
    }

    source = &gOverlay44FrameSources[sourceIndex];
    switch (source->storageMode) {
        case 0:
            storageScale = 0x20;
            state->storageMode = 0;
            break;
        case 1:
            storageScale = 4;
            state->storageMode = 3;
            break;
        case 2:
            storageScale = 8;
            state->storageMode = 2;
            break;
        case 3:
            storageScale = 0x10;
            state->storageMode = 1;
            break;
        default:
            storageScale = 0;
            state->storageMode = 0;
            break;
    }

    allocationSize =
        (source->dimension0 * storageScale * source->dimension1) >> 3;
    if (allocationSize != 0) {
        state->sourceIndex = sourceIndex;
        state->flags = flags | 0x80;
        state->subtype = subtype;
        state->phase = 0;
        state->value8 = value8;
        state->valueA = valueA;
        state->protectedSlot0 = -1;
        state->protectedSlot1 = -1;

        for (handleIndex = 0; handleIndex < 4; handleIndex++) {
            state->cachedFrame[handleIndex] = -1;
            state->handles[handleIndex] =
                overlay44AllocateFrameReloc(allocationSize, 0x87);
        }

        if ((state->handles[0] == 0) || (state->handles[1] == 0) ||
            (state->handles[2] == 0) || (state->handles[3] == 0)) {
            overlay44CleanupStateReloc(state);
            state = 0;
        } else {
            s32 frameData;

            frameData = source->dataOffset;
            state->cachedFrame[0] = 0;
            overlay44UploadFrameReloc(0x42, state->handles[0], frameData,
                                      source->frameSize);
            {
                s32 secondData;

                secondData = source->dataOffset + source->frameSize;
                state->cachedFrame[1] = 1;
                overlay44UploadFrameReloc(0x42, state->handles[1], secondData,
                                          source->frameSize);
            }
            overlay44FinishFramesReloc();
        }
    }

    return state;
}
