#include "overlays/overlay_090.h"

/* DKR v77/v80 and JFG contain no exact donor for this state initializer. */
void overlay90Initialize(Overlay90Owner *owner, Overlay90Config *config) {
    Overlay90State *state;

    state = owner->state;
    state->active = 1;
    state->flag = 0;
    state->angle = config->angle;
    state->x = config->x;
    state->y = config->y;
    state->z = config->z;
    state->value26 = 0;
    state->value28 = 0;
    state->value14 = 0.0f;
    state->value24 = -0x8000;
    state->value2A = 0;
    state->value34 = 0;
    state->value36 = 0;
    state->value38 = 0;
    state->value3C = 0x7F;
    state->value26 = -0x1F00;
    state->value28 = -0x2040;
    state->value2C = 0x20;
    state->value2E = 0x80;
    state->value10 = 0.0f;
    state->value18 = 0.0f;
    state->value1C = 0.0f;
    state->value20 = 0.0f;
    state->value30 = 0.0f;
    state->value14 = 15.0f;
    state->value1C = gOverlay90Value1C;
    state->value20 = gOverlay90Value20;
    overlay90CommitReloc(owner, 0, -1, 0.0f);
}

typedef struct Overlay90Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay90Vector;

typedef struct Overlay90AttachmentHeader {
    u8 pad00[0x2C];
    u8 count;
} Overlay90AttachmentHeader;

typedef struct Overlay90AttachmentEntry {
    s16 value;
    u16 pad02;
    u32 flags;
} Overlay90AttachmentEntry;

struct Overlay90Attachment {
    Overlay90AttachmentHeader *header;
    u8 pad04[0x48];
    Overlay90AttachmentEntry *entries;
};

typedef struct Overlay90Level {
    u8 pad00[0x8E];
    u8 sequence;
} Overlay90Level;

typedef struct Overlay90Data {
    f32 value1C;
    f32 value20;
    f32 scaleDecay;
    f32 scaleStep;
    f32 maximumAnimation;
    f32 animationRate;
    f32 riseRate;
    f32 motionDecay;
    s32 modeTable[7];
    f32 animationScale;
} Overlay90Data;

#define OVERLAY90_DATA (*(Overlay90Data *)&gOverlay90Value1C)

extern s32 gOverlay90SequenceStateReloc;
extern s32 gOverlay90SequenceDoneReloc;

extern void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
extern s32 func_8005ABA8(Overlay90Owner *owner, f32 value, f32 updateRate);
extern void func_8005AD64(Overlay90Owner *owner, s32 mode, s32 index,
                          f32 startFrame);
extern void amSndPlay(s32 soundId, void **handle);
extern void overlay90SequenceReloc(s32 sequenceId);
extern s32 camGetMode(void);
extern Overlay90Level *levelGetLevel(void);
extern void amTunePlay(u8 sequenceId);
extern void amSndStopXYZ(void *handle);
extern void func_80006EA0(void *object);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 func_8000FAE0(f32 x, f32 y, f32 z);
extern void amSndPlayXYZ(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern s32 mathRnd(s32 lower, s32 upper);
extern void amSndSetXYZ(void *handle, f32 x, f32 y, f32 z);
extern void amSndSetVolXYZ(void *handle, u8 volume);
extern void amSndSetPitchXYZ(void *handle, u8 pitch);

/* Matched 2026-10-02 (lane w2-ovlc) from 327 masked at -8. On top of lane
 * q-ovl10's edits (literal floats, func_8005ABA8 returning a value, field
 * rereads, the s16 sound pointer, locals in frame order), what closed it:
 * - the state machine is a do-while with `updateRate = 0` before its test
 *   and no `default:` arm: the out-of-range branch then reaches the loop
 *   test directly, so uopt keeps the reset there instead of hoisting it into
 *   every case, and with no back-edge block it reloads state->active at the
 *   loop head (327 -> 130);
 * - -Wab,-r4300_mul (mk/overlays.mk): the nop between the owner->x products;
 * - the sound pitch reuses animationValue, whose merged web is spilled
 *   through its home across the sound calls, declared after delta so the
 *   home lands at 0x90 (130 -> 0). */
void func_overlay_090_F00000FC_18D4BF4(Overlay90Owner *owner,
                                        s32 updateRate) {
    Overlay90State *state;
    f32 sine;
    f32 cosine;
    s32 originalUpdateRate;
    f32 radial;
    f32 side;
    f32 forward;
    s32 transitioned;
    s32 remaining;
    s32 displayIndex;
    Overlay90Vector delta;
    f32 animationValue;
    s32 value;
    s16 *sound;

    state = owner->state;
    originalUpdateRate = updateRate;
    state->value34 += 0x200;
    displayIndex = 0;

    if ((state->active == 1) || (state->active == 7)) {
        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                state->value30 *= 0.9f;
            } while (remaining--);
        }
    } else {
        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                state->value30 += (8.0f - state->value30) * 0.05f;
            } while (remaining--);
        }
    }

    do {
        transitioned = 0;
        switch (state->active) {
        case 1:
            displayIndex = 0;
            remaining = 0xF0 - state->flag;
            if (updateRate < remaining) {
                remaining = updateRate;
            }
            while (remaining--) {
                state->value26 += state->value2C;
                state->value28 += state->value2E;
                state->value14 *= 0.981f;
                if (state->value2E > 0) {
                    state->value2E--;
                }
                delta.x = 0.0f;
                delta.y = 0.0f;
                delta.z = -state->value14;
                pointListRPY(1, &state->value24, &delta.x, &delta.x);
                state->value18 += delta.x;
                state->value1C += delta.y;
                state->value20 += delta.z;
            }
            animationValue = state->value14 * 0.003f + 0.01f;
            if (animationValue > 0.025f) {
                animationValue = 0.025f;
            }
            func_8005ABA8(owner, animationValue, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0xF0) {
                state->flag -= 0xF0;
                state->active = 2;
                state->value14 = 0.0f;
                func_8005AD64(owner, 2, -1, 0.0f);
                transitioned = 1;
            }
            break;
        case 2:
            displayIndex = 0;
            func_8005ABA8(owner, 0.0167f, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0x78) {
                state->flag -= 0x78;
                state->active = 3;
                func_8005AD64(owner, 1, -1, 0.0f);
                amSndPlay(0x12, 0);
                overlay90SequenceReloc(0x1D);
            }
            break;
        case 3:
            displayIndex = 1;
            func_8005ABA8(owner, 0.01f, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0x3C) {
                state->flag -= 0x3C;
                state->active = 4;
                amSndPlay(0x13, 0);
                transitioned = 1;
            }
            break;
        case 4:
            displayIndex = 2;
            func_8005ABA8(owner, 0.01f, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0x3C) {
                state->flag -= 0x3C;
                state->active = 5;
                amSndPlay(0x14, 0);
                overlay90SequenceReloc(0x1E);
                gOverlay90SequenceStateReloc = 0;
                transitioned = 1;
            }
            break;
        case 5:
            displayIndex = 3;
            func_8005ABA8(owner, 0.01f, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0x1E) {
                gOverlay90SequenceStateReloc = 0x83;
            } else if (state->flag >= 0xF) {
                gOverlay90SequenceStateReloc = 0x84;
            }
            if (state->flag >= 0x3C) {
                state->flag -= 0x3C;
                state->active = 6;
                amSndPlay(0x15, 0);
                overlay90SequenceReloc(0x1F);
                gOverlay90SequenceDoneReloc = 0;
                transitioned = 1;
            }
            break;
        case 6:
            displayIndex = 4;
            func_8005ABA8(owner, 0.01f, (f32)updateRate);
            state->flag += updateRate;
            if (state->flag >= 0x3C) {
                state->flag -= 0x3C;
                state->active = 7;
                state->value2E = 0;
                state->value2C = -0x40;
                func_8005AD64(owner, 0, -1, 0.0f);
                if (camGetMode() == 0) {
                    Overlay90Level *level;

                    level = levelGetLevel();
                    if (level->sequence == 0) {
                        amTunePlay(2);
                    } else {
                        amTunePlay(level->sequence);
                    }
                }
                transitioned = 1;
            }
            break;
        case 7:
            displayIndex = 4;
            remaining = updateRate - 1;
            if (updateRate != 0) {
                do {
                    state->value10 += 0.050f;
                    state->value14 += state->value10;
                    state->value24 += state->value2A;
                    state->value26 += state->value2C;
                    state->value28 += state->value2E;
                    if (state->value2A < 0x40) {
                        state->value2A += 2;
                    }
                    if (state->value2C < 0x80) {
                        state->value2C += 8;
                    }
                    if (state->value2E < 0x80) {
                        state->value2E += 8;
                    }
                } while (remaining--);
            }
            if (state->value14 > 5.0f) {
                state->value14 = 5.0f;
            }
            if (state->value26 > 0x4000) {
                state->value26 = 0x4000;
            }
            if (state->value28 > 0x2000) {
                state->value28 = 0x2000;
            }
            delta.x = 0.0f;
            delta.y = 0.0f;
            delta.z = -state->value14 * (f32)updateRate;
            pointListRPY(1, &state->value24, &delta.x, &delta.x);
            state->value18 += delta.x;
            state->value1C += delta.y;
            state->value20 += delta.z;
            animationValue = state->value14 * 0.01f;
            if (animationValue > 0.025f) {
                animationValue = 0.025f;
            }
            func_8005ABA8(owner, animationValue, (f32)updateRate);
            state->value3C -= updateRate;
            if (state->value3C <= 0) {
                state->value3C = 1;
            }
            state->flag += updateRate;
            if (state->flag >= 0xB5) {
                if (state->value38 != 0) {
                    amSndStopXYZ(state->value38);
                }
                func_80006EA0(owner);
                return;
            }
            break;
        }

        updateRate = 0;
    } while (transitioned != 0);

    {
        Overlay90Attachment *attachment;
        Overlay90AttachmentEntry *entry;

        attachment = *owner->attachment;
        if (attachment != 0) {
            entry = attachment->entries;
            if (entry != 0) {
                remaining = attachment->header->count;
                while (remaining--) {
                    if (entry->flags & 0x100000) {
                        entry->value = displayIndex << 8;
                    }
                    entry++;
                }
            }
        }
    }

    sound = &state->value3E;
    value = (s32)(state->value14 * 1024.0f) + 0x800;
    if (value > 0x1800) {
        value = 0x1800;
    }
    state->value36 += value * originalUpdateRate;
    *sound++ = 0xD;
    *sound++ = state->value36;
    *sound++ = 0x13;
    *sound++ = state->value36;
    *sound = 0x2000;

    sine = func_8002A8BC(state->angle);
    cosine = func_8002A8C0(state->angle);
    side = state->value18;
    radial = func_8002A8C0(state->value34) * state->value30 +
             (state->value1C + 80.0f);
    forward = state->value20 + -100.0f;
    owner->x = side * sine + forward * cosine + state->x;
    owner->y = state->y + radial;
    owner->z = forward * sine - side * cosine + state->z;
    owner->rotationX = state->value24 + state->angle + 0x800;
    owner->rotationY = state->value26;
    owner->rotationZ = state->value28;
    owner->positionTag = func_8000FAE0(owner->x, owner->y, owner->z);

    if (state->value38 == 0) {
        amSndPlayXYZ(0x16, owner->x, owner->y, owner->z, 1,
                      &state->value38);
    }
    if (state->value38 != 0) {
        animationValue = state->value14 * 10.0f + 100.0f;
        if (animationValue > 150.0f) {
            animationValue = 150.0f;
        }
        animationValue += (f32)mathRnd(-5, 5);
        amSndSetXYZ(state->value38, owner->x, owner->y, owner->z);
        amSndSetVolXYZ(state->value38, ((u8 *)&state->value3C)[1]);
        amSndSetPitchXYZ(state->value38, (u8)animationValue);
    }
}
