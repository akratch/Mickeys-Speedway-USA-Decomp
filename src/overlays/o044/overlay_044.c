#include "overlays/overlay_044.h"

/* Mickey-local reconstruction; no external donor body was used. */
Overlay44AnimationState *overlay44CreateAnimationState(
    s32 sourceIndex, s32 value8, s32 valueA, s32 subtype, s32 flags) {
    Overlay44AnimationState *state;
    Overlay44FrameSource *source;
    s32 remainingSlot;
    s32 storageScale;
    s32 allocationSize;
    s32 handleIndex;

    state = &D_800D76D8[0];
    remainingSlot = 2;
    while ((state->sourceIndex >= 0) && remainingSlot--) {
        state++;
    }
    if (remainingSlot < 0) {
        return 0;
    }

    source = &D_800D76D0[sourceIndex];
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
                mmAlloc(allocationSize, 0x87);
        }

        if ((state->handles[0] == 0) || (state->handles[1] == 0) ||
            (state->handles[2] == 0) || (state->handles[3] == 0)) {
            overlay44ReleaseHandles_o044Reloc(state);
            state = 0;
        } else {
            s32 frameData;

            frameData = source->dataOffset;
            state->cachedFrame[0] = 0;
            piRomLoadSection(0x42, state->handles[0], frameData,
                                      source->frameSize);
            {
                s32 secondData;

                secondData = source->dataOffset + source->frameSize;
                state->cachedFrame[1] = 1;
                piRomLoadSection(0x42, state->handles[1], secondData,
                                          source->frameSize);
            }
            viFrameRateReset();
        }
    }

    return state;
}

/* DKR v77/v80 has generic fixed-handle cleanup, but no exact donor. */
void overlay44ReleaseHandles(Overlay44AnimationState *state) {
    s32 index;

    if (state != 0) {
        if (state->sourceIndex != -1) {
            state->sourceIndex = -1;
            index = 0;
            do {
                if (state->handles[index] != 0) {
                    mmFree(state->handles[index]);
                    state->handles[index] = 0;
                }
                index++;
            } while (index != 4);
        }
    }
}

/* Mickey-local reconstruction, tier A: stock object and linked ROM exact.
 * A shared source offset preserves the frame without unused padding. The
 * frame-source reads need no volatile view in the whole module (measured at
 * the fold, 2026-10-23); neither does the allocator's. */
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
    Overlay44FrameSource *source;

    if ((state == 0) || (state->sourceIndex == -1)) {
        return;
    }

    source = &D_800D76D0[state->sourceIndex];
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
                piRomLoadSection(0x42, state->handles[slot], sourceOffset,
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
                piRomLoadSection(0x42, state->handles[slot], sourceOffset,
                                          source->frameSize);
                break;
            }
        } while (slot--);
    }

    state->protectedSlot0 = frameSlot;
    state->protectedSlot1 = nextSlot;
}

/*
 * Overlay 44: draw the animation's two 16-bit frames as one two-tile
 * texture rectangle per TMEM-sized strip.
 *
 * Rewritten 2026-10-02 from the listing (lane w2-capbuf): libultra-style
 * gDPLoadMultiBlockS / gSPTextureRectangle packet macros on (*dl)++, the
 * strip loop as in JFG screen.c's screenDraw (a semantic relative, no code
 * adapted), the frame-source fields read through the global at each use,
 * and an unsigned stride.
 *
 * The byte stride is established before the rectangle dimensions; its signed
 * conversion is repeated at the strip division. This preserves IDO's allocation
 * order. The TU uses the R4300 multiply scheduling mode.
 */

void func_overlay_044_F0000580_188BDE0(Overlay44AnimationState *state,
                                       Gfx **dl, f32 scale) {
    s32 width;
    s32 height;
    s32 rows;
    s32 maxRows;
    s32 x;
    s32 xh;
    s32 yPrev;
    s32 y;
    s32 dsdx;
    s32 stride;
    u8 *tex0;
    u8 *tex1;
    s32 alpha;

    if (state == NULL || state->sourceIndex == -1) {
        return;
    }
    width = D_800D76D0[state->sourceIndex].dimension0;
    height = D_800D76D0[state->sourceIndex].dimension1;
    tex0 = state->handles[state->protectedSlot0];
    tex1 = state->handles[state->protectedSlot1];
    alpha = state->phase & 0xFF;
    if (scale == 1.0f) {
        gSPDisplayList((*dl)++, D_0);
    } else {
        gSPDisplayList((*dl)++, D_28);
    }
    gDPSetPrimColor((*dl)++, 0, 0, state->subtype, state->subtype,
                    state->subtype, 255);
    gDPSetEnvColor((*dl)++, alpha, alpha, alpha, alpha);
    x = state->value8 * 4;
    y = yPrev = state->valueA << 16;
    stride = (u32)width * 2;
    xh = (s32)(width * scale * 4.0f) + x;
    dsdx = (s32)(1024.0f / scale);
    scale *= 65536.0f;
    if (height != 0) {
        maxRows = (0x800 / ((s32)((u32)width * 2))) & ~1;
        do {
            if (maxRows < height) {
                rows = maxRows;
                height -= maxRows;
            } else {
                rows = height;
                height = 0;
            }
            y += (s32)(rows * scale);
            gDPLoadMultiBlockS((*dl)++, tex1, 0x100, 1, G_IM_FMT_RGBA,
                               G_IM_SIZ_16b, width, rows, 0, G_TX_CLAMP,
                               G_TX_CLAMP, 0, 0, 0, 0);
            gDPLoadMultiBlockS((*dl)++, tex0, 0, 0, G_IM_FMT_RGBA,
                               G_IM_SIZ_16b, width, rows, 0, G_TX_CLAMP,
                               G_TX_CLAMP, 0, 0, 0, 0);
            gSPTextureRectangle((*dl)++, x, yPrev >> 14, xh, y >> 14, 0, 0, 0,
                                dsdx, dsdx);
            yPrev = y;
            tex0 += rows * stride;
            tex1 += rows * stride;
        } while (height != 0);
    }
    texDPInit(dl);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
}
