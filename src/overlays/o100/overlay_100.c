#include "overlays/overlay_100.h"

Overlay100Motion *overlay100InitializeMotion(
    f32 x, f32 y, f32 z, s32 count, s32 colorB0, s32 colorB1, s32 colorB2,
    s32 colorA0, s32 colorA1, s32 colorA2, f32 durationSeconds) {
    Overlay100Motion *motion;
    Overlay100Vec3 *velocity;
    s32 remaining;
    s32 frameIndex;
    s16 angles[2];
    s32 velocityBytes;
    f32 velocityScale;

    if (D_80079278 >= 16) {
        return 0;
    }

    velocityBytes = count * (s32)sizeof(Overlay100Vec3);
    motion = mmAlloc(sizeof(Overlay100Motion) + velocityBytes * 4,
                                  0x87);
    if (motion != 0) {
        motion->bank = 1;
        motion->nextBank = 0;
        motion->phase = 0;
        motion->count = count;
        motion->duration = (s16)(s32)(durationSeconds * 60.0f);
        motion->remaining = motion->duration;
        motion->colorB0 = colorB0;
        motion->colorB1 = colorB1;
        motion->colorB2 = colorB2;
        motion->colorA0 = colorA0;
        motion->colorA1 = colorA1;
        motion->colorA2 = colorA2;
        motion->velocity = velocity = (Overlay100Vec3 *)(motion + 1);

        remaining = motion->count;
        if (remaining--) {
            velocityScale = gOverlay100VelocityScaleReloc[0];
            do {
                angles[0] = func_800299E8(0, 0xFFFF);
                angles[1] = func_800299E8(0, 0xFFFF);
                velocity->z =
                    -(f32)func_800299E8(0x180, 0x7F) * velocityScale;
                mathOneFloatPY(angles, velocity);
                velocity++;
            } while (remaining--);
        }

        velocity = (Overlay100Vec3 *)((u8 *)motion->velocity + velocityBytes);
        frameIndex = 0;
        do {
            remaining = motion->count;
            motion->frames[frameIndex] = velocity;
            frameIndex++;
            if (remaining--) {
                do {
                    velocity->x = x;
                    velocity->y = y;
                    velocity->z = z;
                    velocity++;
                } while (remaining--);
            }
        } while (frameIndex != 3);

        gOverlay100EntriesReloc[D_80079278] = motion;
        D_80079278++;
    }
    return motion;
}

/* DKR v77/v80 and JFG contain only generic object-list cleanup relatives. */
void overlay100ReleaseAll(void) {
    s32 count;

    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100RemoveEntry(gOverlay100Entries[count]);
        }
    }
}

/*
 * DKR v77/v80 and JFG contain generic array-compaction relatives, but no
 * exact object donor for this exact registry removal routine.
 */
void overlay100RemoveEntry(void *entry) {
    s32 i;

    if (D_80079278 > 0) {
        for (i = 0; i < D_80079278; i++) {
            if (gOverlay100Entries[i] == entry) {
                D_80079278--;
                break;
            }
        }

        while (i < D_80079278) {
            gOverlay100Entries[i] = gOverlay100Entries[i + 1];
            i++;
        }
    }
    mmFree(entry);
}

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyValue(s32 value) {
    s32 count;
    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100UpdateMotion(gOverlay100Entries[count], value);
        }
    }
}


Overlay100Motion *overlay100UpdateMotion(Overlay100Motion *motion, s32 step) {
    s32 oldPhase, nextPhase, count;
    Overlay100Vec3 *velocity, *oldFrame, *newFrame;
    f32 timeStep, blendScale, gravityStep, verticalBias;

    if (motion == 0) return 0;
    motion->remaining -= step;
    if (motion->remaining <= 0) {
        overlay100RemoveEntry(motion);
        return 0;
    }
    blendScale = step;
    oldPhase = motion->phase;
    nextPhase = oldPhase + 1;
    if (nextPhase >= 3) nextPhase = 0;
    timeStep = blendScale;
    motion->phase = nextPhase;
    if (motion->bank < 3) motion->bank++;
    else {
        motion->nextBank++;
        if (motion->nextBank >= 3) motion->nextBank = 0;
    }
    count = motion->count;
    velocity = motion->velocity;
    oldFrame = motion->frames[oldPhase];
    newFrame = motion->frames[nextPhase];
    if (count--) {
        gravityStep = -(gOverlay100GravityReloc[1] * timeStep);
        verticalBias = ((f32)(step + 1) * gravityStep) * 0.5f;
        do {
            newFrame->x = oldFrame->x + velocity->x * blendScale;
            newFrame->y = oldFrame->y + velocity->y * blendScale + verticalBias;
            newFrame->z = oldFrame->z + velocity->z * blendScale;
            velocity->y += gravityStep;
            velocity++;
            oldFrame++;
            newFrame++;
        } while (count--);
    }
    return motion;
}

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyToValue(void *displayList) {
    s32 count;
    count = D_80079278;
    if (count != 0) {
        while (count--) {
            overlay100DrawMotion(displayList, gOverlay100Entries[count]);
        }
    }
}

/*
 * Written from the listing (lane c-o066, 2026-10-07): GBI packet macros, the
 * two getters called with no arguments (their relocations name
 * func_8002468C(void) and camGetPtr(void); what the target leaves in a0-a3
 * at those calls are leftover webs, not arguments), plain colour locals,
 * `while (row--)` / `while (count--)`, the alpha-step clamp as an if/else
 * (the target's branch over an empty arm), the count read before the frame
 * pointer, and the alpha product inline in the colour packet (so the packed
 * colour word is numbered, and emitted in the setup, first). In the inner
 * loop the depth and the reciprocal are expressions written at each use, so
 * uopt shares them as single webs (the reciprocal computed straight into
 * f16), and point->y is read into a local at the top of the visible branch,
 * which is the shipped early f18 load. The unreferenced locals place the
 * frame (0xC0) and the green/blue/command spill homes.
 */
void overlay100DrawMotion(Gfx **dList, Overlay100Motion *motion) {
    void *unused0; /* unreferenced: frame 0xC0 and the target's homes */
    void *unused1;
    void *unused2;
    void *unused3;
    f32 sinAngle, cosAngle, xScale, yScale;
    s32 green;
    s32 blue;
    f32 py;
    void *unused4;
    f32 depthScale, unused5;
    O100View *view;
    Overlay100Vec3 *point;
    s16 *angle;
    s32 phase, row, count;
    s32 alphaStep;
    s32 red;
    s32 x, y, progress;
    Gfx *commands;

    if (motion == NULL) {
        return;
    }
    commands = *dList;
    view = func_8002468C();
    xScale = view->scaleX * 320.0f * 0.5f;
    yScale = view->scaleY * 240.0f * 0.5f;
    depthScale = view->depthScale;
    gSPDisplayList(commands++, gOverlay100SegmentReloc);
    progress = (motion->remaining << 16) / motion->duration;
    red = motion->colorA0 + (((motion->colorB0 - motion->colorA0) * progress) >> 16);
    green = motion->colorA1 + (((motion->colorB1 - motion->colorA1) * progress) >> 16);
    blue = motion->colorA2 + (((motion->colorB2 - motion->colorA2) * progress) >> 16);
    if (motion->remaining >= 64) {
        alphaStep = 255;
    } else {
        alphaStep = motion->remaining * 4;
    }
    angle = camGetPtr();
    sinAngle = func_8002A8BC(*angle + 0x8000);
    cosAngle = func_8002A8C0(*angle + 0x8000);
    row = motion->bank;
    phase = motion->nextBank;
    while (row--) {
        gDPPipeSync(commands++);
        gDPSetPrimColor(commands++, 0, 0, red, green, blue, (alphaStep * (3 - row)) / 3);
        count = motion->count;
        point = motion->frames[phase];
        phase++;
        if (phase >= 3) {
            phase = 0;
        }
        while (count--) {
            if ((-(point->x * cosAngle) + point->z * sinAngle) < -10.0f) {
                py = point->y;
                x = (s32)((point->x * sinAngle + point->z * cosAngle) * xScale * (1.0f / ((-(point->x * cosAngle) + point->z * sinAngle) * depthScale))) + 160;
                if ((u32)x < 320) {
                    y = 120 - (s32)(py * yScale * (1.0f / ((-(point->x * cosAngle) + point->z * sinAngle) * depthScale)));
                    if ((u32)y < 240) {
                        gDPFillRectangle(commands++, x, y, x + 1, y + 1);
                    }
                }
            }
            point++;
        }
    }
    *dList = commands;
    texDPInit(dList);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
}
