#include "overlays/overlay_101.h"



/* Tier B: consumer strides and independent neighboring LOCAL identities
 * bound these storage families. The entry allocator/dispatcher scans 40.
 * Tier D: 16 root/order/node slots and 32 text slots are bounded storage
 * views, not recovered original declarations; constructors select 0..11.
 * Only ordinary compiler section alignment follows the final entry. */
O101View_D_0 D_0;
O101Elem_gOverlay101Order gOverlay101Order[16];
O101Elem_gOverlay101Resources20 gOverlay101Resources20[16];
O101Elem_D_340 D_340[16];
O101Elem_D_540 D_540[32];
s32 gOverlay101GlobalX;
s32 gOverlay101GlobalY;
O101Elem_gOverlay101Entries gOverlay101Entries[40];

typedef char O101RootSlotSize[(sizeof(O101Elem_D_0) == 28) ? 1 : -1];
typedef char O101RootStorageSize[(sizeof(O101View_D_0) == 448) ? 1 : -1];
typedef char O101OrderSlotSize[(sizeof(O101Elem_gOverlay101Order) == 4) ? 1 : -1];
typedef char O101Node20Size[(sizeof(O101Elem_gOverlay101Resources20) == 20) ? 1 : -1];
typedef char O101Node32Size[(sizeof(O101Elem_D_340) == 32) ? 1 : -1];
typedef char O101TextNodeSize[(sizeof(O101Elem_D_540) == 24) ? 1 : -1];
typedef char O101AnimationEntrySize[(sizeof(O101Elem_gOverlay101Entries) == 48) ? 1 : -1];

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101Initialize(void) {
    if (D_8007BF44 > 0) {
        mainChangeLevel(1, 0, 0, 15, 1, 0);
        return;
    }

    overlay101SetScaleReloc(0.5f, 0);
    if (gOverlay101CurrentConfig == -1) {
        mainTitlePageInit(1);
    } else {
        mainChangeLevel(gOverlay101CurrentConfig, 0, 0,
                                 gOverlay101ConfigValue, 1, 0);
        mainSetMode(gOverlay101CurrentResource);
    }
    gOverlay101Ready = 0;
    func_80001608();
}

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this pool scan. */
void *overlay101AllocateEntry(void) {
    O101Elem_gOverlay101Entries *entry;
    void *result;
    s32 count;
    register s32 remaining;

    count = gOverlay101EntryCount;
    entry = &gOverlay101Entries[0];
    remaining = 0x27;
    if (gOverlay101EntryCount < 0x28) {
        do {
            if (entry->m0.type == 0) {
                gOverlay101EntryCount = count + 1;
                return entry;
            }
            entry++;
        } while (remaining--);
    }
    result = NULL;
    return result;
}

Overlay101Node *overlay101FindEntry(Overlay101Node *head, s32 value,
                                    s32 occurrence) {
    s32 current;
    s32 found;
    Overlay101Node *node;

    found = 0;
    current = head->value;
    node = head->next;
    if ((node != NULL) && (current != 0)) {
        do {
            if (current == value) {
                if (found == occurrence) {
                    return node;
                }
                found++;
            }
            current = node->value;
            node = node->next;
        } while ((node != NULL) && (current != 0));
    }
    return NULL;
}

void overlay101UpdateEntry(Overlay101TimedEntry *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}

void overlay101SchedulePair(s32 index, s32 x, s32 y, f32 seconds,
                            s32 submitValue) {
    s32 duration;
    Overlay101PairEntry *entry;
    Overlay101PairSlot *slot;

    slot = &D_0.el[index].m0;
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0.el[index].m0;
            entry->output = slot;
            entry->type = 1;
            entry->startX = slot->x;
            entry->startY = slot->y;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateEntry((void *)entry, submitValue);
            }
        }
    } else {
        slot->x = x;
        slot->y = y;
    }
}

void overlay101UpdateEntry12(Overlay101TimedEntry12 *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output12 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}

void overlay101SchedulePair12(s32 index, s32 x, s32 y, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101PairEntry12 *entry;
    Overlay101PairSlot12 *slot;

    slot = &D_0.el[index].m1;
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0.el[index].m1;
            entry->output = slot;
            entry->type = 2;
            entry->startX = slot->x;
            entry->startY = slot->y;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateEntry12((void *)entry, submitValue);
            }
        }
    } else {
        slot->x = x;
        slot->y = y;
    }
}

void overlay101ActivateSlot(s32 index) {
    Overlay101Slot_ActivateSlot *slot;
    slot = &D_0.el[index].m2;
    if (slot->state == 0) {
        amSndPlay(0x1F7, 0);
        slot->state = 1;
        slot->value = 0;
    } else if (slot->state == 3) {
        slot->state = 1;
    }
}

void overlay101AdvanceSlot(s32 index) {
    Overlay101Slot_AdvanceSlot *slot;
    s32 state;

    slot = &D_0.el[index].m3;
    state = slot->state;
    if (state == 2) {
        slot->state = 3;
        slot->value = 0x40;
    } else if (state == 1) {
        slot->state = 3;
    }
}

void overlay101PromoteSlot(s32 index) {
    Overlay101Slot_PromoteSlot *target;
    Overlay101Slot_PromoteSlot *previous;
    Overlay101Slot_PromoteSlot *current;
    s32 count;
    s32 i;

    target = &D_0.el[index].m4;
    i = 1;
    previous = gOverlay101Order[0].m0;
    gOverlay101Order[0].m0 = target;
    count = o101OrderCounter_o101Reloc;
    while (i < count && previous != target) {
            current = gOverlay101Order[i].m0;
            gOverlay101Order[i].m0 = previous;
            previous = current;
            i++;
    }
}

void overlay101UpdateByte17(Overlay101ByteEntry17_UpdateByte17 *entry, s32 step) {
    s32 value;
    s32 elapsed;
    Overlay101ByteOutput17 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
            ((entry->end - entry->start) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) output->value = value;
}

void overlay101ScheduleByte17(s32 index, s32 value, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101ByteEntry17_ScheduleByte17 *entry;
    Overlay101ByteSlot17 *slot;

    slot = &D_0.el[index].m5;
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0.el[index].m5;
            entry->output = slot;
            entry->type = 6;
            entry->start = slot->value;
            entry->end = value;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) overlay101UpdateByte17((void *)entry, submitValue);
        }
    } else {
        slot->value = value;
    }
}

void overlay101UpdateByte16(Overlay101ByteEntry16_UpdateByte16 *entry, s32 step) {
    s32 value;
    s32 elapsed;
    Overlay101ByteOutput16 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
            ((entry->end - entry->start) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) output->value = value;
}

void overlay101ScheduleByte16(s32 index, s32 value, f32 seconds,
                              s32 submitValue) {
    s32 duration;
    Overlay101ByteEntry16_ScheduleByte16 *entry;
    Overlay101ByteSlot16 *slot;

    slot = &D_0.el[index].m6;
    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            slot = &D_0.el[index].m6;
            entry->output = slot;
            entry->type = 7;
            entry->start = slot->value;
            entry->end = value;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) overlay101UpdateByte16((void *)entry, submitValue);
        }
    } else {
        slot->value = value;
    }
}

void overlay101UpdateEntry8(Overlay101TimedEntry8 *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output8 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}

void overlay101ScheduleLinkedPair(s32 index, s32 key, s32 x, s32 y,
                                  f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedSlot *slot;
    Overlay101LinkedEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m7, 1, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 8;
                entry->startX = slot->x;
                entry->startY = slot->y;
                entry->endX = x;
                entry->endY = y;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->x = x;
            slot->y = y;
        }
    }
}

/* ---- overlay101UpdateEntry8B (+0xB28) ---- */
void overlay101UpdateEntry8B(Overlay101TimedEntry8 *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output8 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}

void overlay101ScheduleLinkedPair2(s32 index, s32 key, s32 x, s32 y,
                                   f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedSlot2 *slot;
    Overlay101LinkedEntry2 *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m8, 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 9;
                entry->startX = slot->x;
                entry->startY = slot->y;
                entry->endX = x;
                entry->endY = y;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->x = x;
            slot->y = y;
        }
    }
}

void overlay101UpdateFloat12(Overlay101FloatEntry *entry, s32 step) {
    s32 elapsed;
    f32 value;
    Overlay101Float12Output *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
                ((entry->end - entry->start) * (f32)elapsed) /
                    (f32)entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->value = value;
    }
}

void overlay101ScheduleLinkedFloat(s32 index, s32 key, f32 value, f32 seconds,
                                   s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedFloatSlot *slot;
    Overlay101LinkedFloatEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m9, 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 10;
                entry->start = slot->value;
                entry->end = value;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->value = value;
        }
    }
}

void overlay101UpdateDelta16(Overlay101DeltaEntry *entry, s32 step) {
    s32 elapsed;
    s32 value;
    Overlay101Delta16Output *output;

    elapsed = (entry->elapsed += step);
    value = entry->start;
    if (elapsed >= entry->duration) {
        entry->type = 0;
        value += entry->delta;
        gOverlay101EntryCount--;
    } else {
        value += (entry->delta * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->value = value;
    }
}

void overlay101ScheduleLinkedScaled(s32 index, s32 key, f32 value, f32 seconds,
                                    s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedScaledSlot *slot;
    Overlay101LinkedScaledEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m10, 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 11;
                entry->start = slot->value;
                entry->end = value * gOverlay101TimedScale_o101Reloc;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->value = value * gOverlay101ImmediateScale_o101Reloc;
        }
    }
}

void overlay101UpdateByte18(Overlay101Byte18Entry *entry, s32 step) {
    s32 elapsed;
    f32 value;
    Overlay101Byte18Output *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
                ((entry->end - entry->start) * (f32)elapsed) /
                    (f32)entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->value = value;
    }
}

void overlay101ScheduleLinkedByte(s32 index, s32 key, s32 value, f32 seconds,
                                  s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedByteSlot *slot;
    Overlay101LinkedByteEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m11, 2, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 12;
                entry->start = slot->value;
                entry->end = value;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->value = value;
        }
    }
}

/* ---- overlay101UpdateEntry8C (+0x11E0) ---- */
void overlay101UpdateEntry8C(Overlay101TimedEntry8 *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output8 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}

void overlay101ScheduleLinkedPair3(s32 index, s32 key, s32 x, s32 y,
                                   f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedSlot3 *slot;
    Overlay101LinkedEntry3 *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m12, 3, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 13;
                entry->startX = slot->x;
                entry->startY = slot->y;
                entry->endX = x;
                entry->endY = y;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->x = x;
            slot->y = y;
        }
    }
}

void overlay101UpdateColor(Overlay101ColorEntry *entry, s32 step) {
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 elapsed;
    Overlay101ColorOutput *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->type = 0;
        red = entry->endRed;
        green = entry->endGreen;
        blue = entry->endBlue;
        alpha = entry->endAlpha;
        gOverlay101EntryCount--;
    } else {
        red = entry->startRed +
            ((entry->endRed - entry->startRed) * elapsed) / entry->duration;
        green = entry->startGreen +
            ((entry->endGreen - entry->startGreen) * elapsed) / entry->duration;
        blue = entry->startBlue +
            ((entry->endBlue - entry->startBlue) * elapsed) / entry->duration;
        alpha = entry->startAlpha +
            ((entry->endAlpha - entry->startAlpha) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->red = red;
        output->green = green;
        output->blue = blue;
        output->alpha = alpha;
    }
}

void overlay101ScheduleLinkedColor(s32 index, s32 key, s32 red, s32 green,
                                   s32 blue, s32 alpha, f32 seconds,
                                   s32 submitValue) {
    volatile s16 reservation;
    Overlay101LinkedColorSlot *slot;
    Overlay101LinkedColorEntry *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m13, 3, key);
    if (slot != NULL) {
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 14;
                entry->startRed = slot->red;
                entry->startGreen = slot->green;
                entry->startBlue = slot->blue;
                entry->startAlpha = slot->alpha;
                entry->endRed = red;
                entry->endGreen = green;
                entry->endBlue = blue;
                entry->endAlpha = alpha;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101SubmitLinkedPairReloc(entry, submitValue);
                }
            }
        } else {
            slot->red = red;
            slot->green = green;
            slot->blue = blue;
            slot->alpha = alpha;
        }
    }
}

void overlay101UpdateFrames(Overlay101FrameEntry *entry, s32 step) {
    Overlay101FrameOutput *output;
    s32 delay;

    delay = (entry->delay -= step);
    if (delay < 0) {
        output = entry->output;
        if (output != NULL) {
            entry->delay = 0;
            entry->elapsed -= delay;
            if (entry->elapsed >= entry->duration) {
                do {
                    output->frame++;
                    entry->elapsed -= entry->duration;
                } while (entry->elapsed >= entry->duration);
            }
            if (output->frame >= output->frameCount) {
                output->frame = output->frameCount;
                entry->type = 0;
                gOverlay101EntryCount--;
            }
        } else {
            entry->type = 0;
            gOverlay101EntryCount--;
        }
    }
}

void overlay101ScheduleFrames(s32 index, s32 key, f32 fraction, f32 rate,
                              f32 seconds, s32 submitValue) {
    volatile s16 reservation;
    Overlay101FrameSlot *slot;
    Overlay101FrameSchedule *entry;
    s32 duration;

    slot = overlay101FindEntry(&D_0.el[index].m14, 3, key);
    if (slot != NULL) {
        if (fraction >= 0.0f && fraction <= 1.0f) {
            slot->frame = (s32)(slot->frameCount * fraction);
        }
        duration = seconds * 60.0f;
        if (duration > 0) {
            entry = overlay101AllocateEntry();
            if (entry != NULL) {
                entry->output = slot;
                entry->type = 15;
                entry->delay = rate * 60.0f;
                entry->elapsed = 0;
                entry->duration = duration;
                if (submitValue > 0) {
                    overlay101UpdateFrames((void *)entry, submitValue);
                }
            }
        }
    }
}

void overlay101UpdateGlobalPair(Overlay101GlobalPairEntry_UpdateGlobalPair *entry, s32 step) {
    s32 elapsed;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        gOverlay101GlobalX = entry->endX;
        gOverlay101GlobalY = entry->endY;
        entry->type = 0;
        gOverlay101EntryCount--;
        return;
    }
    gOverlay101GlobalX = entry->startX +
        ((entry->endX - entry->startX) * elapsed) / entry->duration;
    gOverlay101GlobalY = entry->startY +
        ((entry->endY - entry->startY) * entry->elapsed) / entry->duration;
}

void overlay101ScheduleGlobalPair(s32 x, s32 y, f32 seconds,
                                  s32 submitValue) {
    Overlay101GlobalPairEntry_ScheduleGlobalPair *entry;
    s32 duration;

    duration = seconds * 60.0f;
    if (duration > 0) {
        entry = overlay101AllocateEntry();
        if (entry != NULL) {
            entry->output = NULL;
            entry->type = 16;
            entry->startX = gOverlay101GlobalX;
            entry->startY = gOverlay101GlobalY;
            entry->endX = x;
            entry->endY = y;
            entry->elapsed = 0;
            entry->duration = duration;
            if (submitValue > 0) {
                overlay101UpdateGlobalPair((void *)entry, submitValue);
            }
        }
    } else {
        gOverlay101GlobalX = x;
        gOverlay101GlobalY = y;
    }
}

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101DispatchActive(s32 value) {
    O101Elem_gOverlay101Entries *entry;
    s32 remaining;

    if (gOverlay101EntryCount > 0) {
        entry = &gOverlay101Entries[0];
        remaining = 39;
        do {
            switch (entry->m1.type) {
                case 1:
                    overlay101UpdateEntry((void *)entry, value);
                    break;
                case 2:
                    overlay101UpdateEntry12((void *)entry, value);
                    break;
                case 6:
                    overlay101UpdateByte17((void *)entry, value);
                    break;
                case 7:
                    overlay101UpdateByte16((void *)entry, value);
                    break;
                case 8:
                    overlay101UpdateEntry8((void *)entry, value);
                    break;
                case 9:
                    overlay101UpdateEntry8B((void *)entry, value);
                    break;
                case 10:
                    overlay101UpdateFloat12((void *)entry, value);
                    break;
                case 11:
                    overlay101UpdateDelta16((void *)entry, value);
                    break;
                case 12:
                    overlay101UpdateByte18((void *)entry, value);
                    break;
                case 13:
                    overlay101UpdateEntry8C((void *)entry, value);
                    break;
                case 14:
                    overlay101UpdateColor((void *)entry, value);
                    break;
                case 15:
                    overlay101UpdateFrames((void *)entry, value);
                    break;
                case 16:
                    overlay101UpdateGlobalPair((void *)entry, value);
                    break;
                case 3:
                case 4:
                case 5:
                    break;
            }
            entry++;
        } while (remaining--);
    }
}

void overlay101Reset(void *value) {
    gOverlay101Value220 = value;
    gOverlay101Value224 = 0;
    gOverlay101Value228 = 0;
}

#define gOverlay101Events D_220
#define gOverlay101EventIndex D_224
#define gOverlay101EventClock D_228
#define gOverlay101PresentationDone D_218

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101DispatchEvents(s32 step) {
    Overlay101Event *events;
    Overlay101Event *event;
    s32 remaining;
    s32 delay;
    s32 finished;
    s32 packed;

    events = gOverlay101Events;
    if (events == 0) {
        return;
    }
    if ((s16)events[gOverlay101EventIndex].type == 0) {
        return;
    }
    gOverlay101EventClock += step;
    finished = 0;
    do {
        event = &gOverlay101Events[gOverlay101EventIndex];
        delay = (s32)(event->time * 60.0f);
        remaining = gOverlay101EventClock - delay;
        if (remaining >= 0) {
            switch ((u16)event->type) {
                case 0:
                    return;
                case 1:
                    overlay101SchedulePair(event->index, event->value8,
                        event->valueC, event->value18.f, remaining);
                    break;
                case 2:
                    overlay101SchedulePair12(event->index, event->value8,
                        event->valueC, event->value18.f, remaining);
                    break;
                case 3:
                    overlay101ActivateSlot(event->index);
                    break;
                case 4:
                    overlay101AdvanceSlot(event->index);
                    break;
                case 5:
                    overlay101PromoteSlot(event->index);
                    break;
                case 6:
                    overlay101ScheduleByte17(event->index, event->value8,
                        event->value18.f, remaining);
                    break;
                case 7:
                    overlay101ScheduleByte16(event->index, event->value8,
                        event->value18.f, remaining);
                    break;
                case 8:
                    overlay101ScheduleLinkedPair(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 9:
                    overlay101ScheduleLinkedPair2(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 10:
                    overlay101ScheduleLinkedFloat(event->index, event->key,
                        event->value10.f, event->value18.f, remaining);
                    break;
                case 11:
                    overlay101ScheduleLinkedScaled(event->index, event->key,
                        event->value10.f, event->value18.f, remaining);
                    break;
                case 12:
                    overlay101ScheduleLinkedByte(event->index, event->key,
                        event->value8, event->value18.f, remaining);
                    break;
                case 13:
                    overlay101ScheduleLinkedPair3(event->index, event->key,
                        event->value8, event->valueC, event->value18.f, remaining);
                    break;
                case 14:
                    packed = event->valueC;
                    overlay101ScheduleLinkedColor(event->index, event->key,
                        event->value8 >> 16, event->value8 & 0xFF,
                        packed >> 16, packed & 0xFF, event->value18.f, remaining);
                    break;
                case 15:
                    overlay101ScheduleFrames(event->index, event->key,
                        event->value10.f, event->value14.f, event->value18.f,
                        remaining);
                    break;
                case 16:
                    overlay101ScheduleGlobalPair(event->value8, event->valueC,
                        event->value18.f, remaining);
                    break;
                case 17:
                    overlay101Initialize();
                    gOverlay101PresentationDone = 1;
                    break;
                case 18:
                    D_8007BF60 = 1;
                    break;
            }
            gOverlay101EventIndex++;
        } else {
            finished = 1;
        }
    } while (finished == 0);
}
#undef gOverlay101Events
#undef gOverlay101EventIndex
#undef gOverlay101EventClock
#undef gOverlay101PresentationDone

/* DKR and JFG use the same G_SETSCISSOR macro semantics, but neither pinned
 * object set contains this clipping helper as an exact donor. */
void overlay101SetScissor(Overlay101Gfx **displayList, register s32 left, s32 top,
                          s32 right, s32 bottom) {
    s32 width;
    s32 height;

    if ((right >= left) && (bottom >= top)) {
        viGetCurrentSize(&width, &height);
        if ((right > 0) && (left < width) &&
            (bottom > 0) && (top < height)) {
            if (left < 0) {
                left = 0;
            }
            if ((u32)width < (u32)right) {
                right = width - 1;
            }
            if (top < 0) {
                top = 0;
            }
            if ((u32)height < (u32)bottom) {
                bottom = height - 1;
            }
        } else {
            left = 0;
            top = 0;
            right = 0;
            bottom = 0;
        }
    } else {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }

    {
        Overlay101Gfx *command;
        command = (*displayList)++;
        command->words.w0 = 0xE7000000;
        command->words.w1 = 0;
    }
    {
        Overlay101Gfx *command;
        command = (*displayList)++;
        command->words.w0 = 0xED000000 |
                      (((s32)((f32)left * 4.0f) & 0xFFF) << 12) |
                      ((s32)((f32)top * 4.0f) & 0xFFF);
        command->words.w1 = (((s32)((f32)right * 4.0f) & 0xFFF) << 12) |
                      ((s32)((f32)bottom * 4.0f) & 0xFFF);
    }
}

/* DKR v77/v80 and JFG have viewport-clipping relatives but no exact donor. */
s32 overlay101GetBounds(Overlay101BoundsNode_GetBounds *node, s32 *leftOut,
                         s32 *topOut, s32 *rightOut, s32 *bottomOut) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 valid;
    s32 screenWidth;
    s32 screenHeight;

    valid = 0;
    if ((node->type == 2) || (node->type == 4)) {
        viGetCurrentSize(&screenWidth, &screenHeight);
        left = node->x;
        top = node->y;
        right = node->x + node->width;
        bottom = node->y + node->height;
        if (node->type != 4) {
            left += 4;
            top += 0xC;
            right -= 4;
            bottom -= 4;
        }
        if ((left < screenWidth) && (top < screenHeight) && (right >= 0) &&
            (bottom >= 0)) {
            if (left < 0) {
                left = 0;
            }
            if ((u32)right >= (u32)screenWidth) {
                right = screenWidth - 1;
            }
            if (top < 0) {
                top = 0;
            }
            if ((u32)bottom >= (u32)screenHeight) {
                bottom = screenHeight - 1;
            }
            *leftOut = left;
            valid = 1;
            *topOut = top;
            *rightOut = right;
            *bottomOut = bottom;
        }
    }
    if (valid == 0) {
        *leftOut = 0;
        *topOut = 0;
        *rightOut = 0;
        *bottomOut = 0;
    }
}

/* DKR v77/v80 and JFG contain related sprite/HUD draw pipelines but no exact
 * object donor for this element renderer. */
void overlay101DrawElement(s32 displayList, Overlay101BoundsNode_DrawElement *node,
                           Overlay101Element *element) {
    s32 x;
    s32 y;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    u8 saved0;
    u8 saved1;
    u8 *pixels;
    u8 *savedPixel;

    if ((node->type == 2) || (node->type == 4)) {
        if ((element->index > 0) && (element->alpha > 0) &&
            ((pixels = element->pixels) != 0)) {
            savedPixel = 0;
            x = node->x + element->x;
            y = node->y + element->y;
            if (element->index < element->count) {
                savedPixel = pixels + element->index;
                saved0 = savedPixel[0];
                saved1 = savedPixel[1];
                if (gOverlay101RenderFlags & 8) {
                    savedPixel[0] = 0x24;
                    savedPixel[1] = 0;
                } else {
                    savedPixel[0] = 0;
                }
            } else {
                savedPixel = 0;
            }
            red = (element->red * node->opacity) / 255;
            green = (element->green * node->opacity) / 255;
            blue = (element->blue * node->opacity) / 255;
            alpha = (element->alpha * node->scale) / 255;

            fontUseFont(element->kind);
            fontBackground(0, 0, 0, 0);
            fontColour(red, green, blue, 0xFF, alpha);
            overlay101GetBounds(node, &left, &top, &right, &bottom);
            overlay101SetScissor(displayList, left, top, right, bottom);
            fontPrintXY(displayList, x, y, pixels,
                                       element->flags);
            overlay101SetScissor(displayList, 0, 0, 1000, 1000);

            if (savedPixel != 0) {
                savedPixel[0] = saved0;
                savedPixel[1] = saved1;
            }
        }
    }
}

/* ---- func_overlay_101_F0002510_18DDD30 (+0x2510) ---- */
/* The SDK GBI header takes _SHIFTL/_SHIFTR from mbi.h; spelled as the SDK does. */
#define _SHIFTL(v, s, w) \
    ((unsigned int)(((unsigned int)(v) & ((0x01 << (w)) - 1)) << (s)))
#define _SHIFTR(v, s, w) \
    ((unsigned int)(((unsigned int)(v) >> (s)) & ((0x01 << (w)) - 1)))

/*
 * Lanes p11-o101, x-o101, a-ovl3, k-2 and m-4 decoded the callees and wrote
 * the body with the SDK GBI macros, the y origin tested from its own local
 * and copied after the clip tests, and the bounds call's result consumed by
 * x (times zero: the call then delivers v0 into the clip block, so bottom
 * takes v0 and left s3, checklist item 38). Lane p-3 (2026-10-08) closed the
 * last 60 words: the display list read before the stride, an empty test of
 * y at the right clamp (y outranks node for s4), the row mask carried by
 * originY from right after rows (it outranks sourceX and drawWidth), the
 * scaling statements in the target's order, and each edge copied from its
 * origin and then grown, which gives the sums the target's operand order.
 */
void func_overlay_101_F0002510_18DDD30(Gfx **dList, Overlay101ClipNode *node,
                                       Overlay101TextureElement *element) {
    Overlay101Texture *texture;
    Gfx *gfx;
    u16 *source;
    s32 x;
    s32 y;
    s32 shift;
    s32 rows;
    s32 sourceX;
    s32 sourceY;
    s32 drawX;
    s32 drawY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 stride;
    s32 drawWidth;
    s32 drawHeight;
    s32 chunkRows;
    s32 edgeX;
    s32 edgeY;
    s32 originY;

    if ((node->type != 2) && (node->type != 4)) {
        return;
    }
    texture = element->texture;
    if (texture == NULL) {
        return;
    }
    x = overlay101GetBounds(node, &left, &top, &right, &bottom) * 0 + node->x + element->x;
    originY = node->y + element->y;
    y = originY;
    edgeX = x;
    edgeY = originY;
    edgeX += texture->width;
    edgeY += texture->height;
    if ((right < x) || (bottom < originY) || (edgeX < left) || (edgeY < top)) {
        return;
    }
    overlay101SetScissor(dList, left, top, right, bottom);
    rows = 0x800 / texture->width;
    if (rows >= 8) {
        shift = 3;
    } else if (rows >= 4) {
        shift = 2;
    } else {
        shift = 1;
    }
    rows = 1 << shift;
    originY = rows - 1;
    gfx = *dList;
    stride = texture->width * rows;
    if (x < left) {
        drawX = left;
        sourceX = left - x;
    } else {
        drawX = x;
        sourceX = 0;
    }
    drawWidth = texture->width - sourceX;
    if ((right - drawX) < drawWidth) {
        drawWidth = right - drawX; if (y) {}
    }
    if (y < top) {
        drawY = top;
        sourceY = top - y;
    } else {
        drawY = y;
        sourceY = 0;
    }
    drawHeight = texture->height - sourceY;
    if ((bottom - drawY) < drawHeight) {
        drawHeight = bottom - drawY;
    }
    source = &texture->pixels[stride * (sourceY >> shift)];
    drawX *= 4;
    drawY *= 4;
    drawWidth *= 4;
    sourceX <<= 5;
    sourceY = (sourceY & originY) << 5;
    gSPDisplayList(gfx++, D_230);
    gDPSetPrimColor(gfx++, 0, 0, node->intensity, node->intensity, node->intensity, node->alpha);
    while (drawHeight > 0) {
        gDPLoadTextureBlockS(gfx++, source, G_IM_FMT_RGBA, G_IM_SIZ_16b, texture->width, rows, 0,
                             G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        chunkRows = rows - (sourceY >> 5);
        if (drawHeight < chunkRows) {
            chunkRows = drawHeight;
        }
        gSPTextureRectangle(gfx++, drawX, drawY, drawX + drawWidth, drawY + chunkRows * 4, G_TX_RENDERTILE,
                            sourceX, sourceY, 1 << 10, 1 << 10);
        drawHeight -= chunkRows;
        sourceY = 0;
        drawY += chunkRows * 4;
        source += stride;
    }
    *dList = gfx;
    texDPInit(dList);
    overlay101SetScissor(dList, 0, 0, 1000, 1000);
}
#undef _SHIFTL
#undef _SHIFTR

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
 */
void overlay101DrawTransformed(Overlay101Gfx **displayList, void *matrix,
                               void *vertices, Overlay101DrawNode_DrawTransformed *node,
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
        viGetCurrentSize(&screenWidth, &screenHeight);
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
        command->words.w1 = 0;
        command->words.w0 = 0xE7000000;
        command = (primColorCommand = (*displayList)++);
        command->words.w0 = 0xFA000000;
        command->words.w1 = (element->color & 0xFF) | 0xFFFFFF00;
        command = (*displayList)++;
        command->words.w1 = 0xFFFFFF00;
        command->words.w0 = 0xFB000000;
        if (element->rotation != 0)
            rotated = 1;
        else
            rotated = 0;
        overlay101GetBounds(node, &bounds0, &bounds1, &bounds2, &bounds3);
        overlay101SetScissor(displayList, bounds0, bounds1, bounds2,
                                   bounds3);
        if (element->scale == 1.0f)
            sprSetTextureFilter(0);
        camDo2DSprite(displayList, matrix, vertices,
                                       &transform, element->object, rotated,
                                       element->color);
        if (element->scale == 1.0f)
            sprSetTextureFilter(1);
        command = (syncCommand = (*displayList)++);
        command->words.w1 = 0;
        command->words.w0 = 0xE7000000;
        command = (*displayList)++;
        command->words.w0 = 0xFA000000; command->words.w1 = 0xFFFFFFFF;
        overlay101SetScissor(displayList, 0, 0, 1000, 1000);
        (void)right;
        (void)bottom;
    }
}

/* DKR v77/v80 and JFG have no exact donor for this four-rectangle builder. */
void overlay101BuildFrame(s32 kind, s32 x, s32 y, s32 width, s32 height,
                          s32 color) {
    Overlay101FrameRect *rect;
    Overlay101FrameRect batch[4];

    rect = batch;
    rect[0].x0 = x;
    rect[0].y0 = y;
    rect[0].x1 = x + 1;
    rect[0].y1 = y + height;
    rect[0].color = color;
    rect[1].x0 = x;
    rect[1].y0 = y + height - 1;
    rect[1].x1 = x + width;
    rect[1].y1 = y + height;
    rect[1].color = color;
    rect[2].x0 = x + width - 1;
    rect[2].y0 = y + 1;
    rect[2].x1 = x + width;
    rect[2].y1 = y + height - 1;
    rect[2].color = color;
    rect += 3;
    rect->x0 = x + 1;
    rect->y0 = y;
    rect->x1 = x + width;
    rect->y1 = y + 1;
    rect->color = color;

    frontDrawRectangles(kind, 4, batch, 0);
}

/* ---- overlay101BuildIntensityColors (+0x2CE4) ---- */
void overlay101BuildIntensityColors(s32 intensity, s32 alpha, u32 *full,
                                    u32 *dim, u32 *dimmer, u32 *darkest) {
    s32 component;
    s32 blue;

    if (full != NULL) {
        component = (intensity * 0xFF) >> 8;
        *full = (component << 24) | (component << 16) | (component << 8) |
                (alpha & 0xFF);
    }
    if (dim != NULL) {
        component = (intensity * 0x78) >> 8;
        blue = (intensity * 0x40) >> 8;
        *dim = (component << 24) | (component << 16) | (blue << 8) |
               (alpha & 0xFF);
    }
    if (dimmer != NULL) {
        component = (intensity * 0xF0) >> 8;
        blue = (intensity * 0x80) >> 8;
        *dimmer = (component << 24) | (component << 16) | (blue << 8) |
                  (alpha & 0xFF);
    }
    if (darkest != NULL) {
        component = (intensity * 0x10) >> 8;
        *darkest = (component << 16) | (component << 8) | (alpha & 0xFF);
    }
}

/* The two `if (1)` blocks are uopt region boundaries, and each is load-bearing
 * for the shipped bytes rather than decoration. Only a control-flow construct
 * opens a region: a bare compound statement measures byte-identical to no
 * block at all. The first one renumbers every geometry web -- it is what puts
 * x, x + width, x + 1 and y + height in a0..a3 and lets a2 carry y + 1 once x
 * is dead -- and takes the candidate from 38 differing words to 10. The second
 * is a join point between the two `trailingColor` reads, which is what stops
 * uopt from folding them into one load; that is why trailingColor needs no
 * `volatile` while leadingColor, whose two reads have no join between them,
 * still does. Falsified with each measured: all sixteen commutative operand
 * orders of the four geometry sums are byte-inert, 1,888 statement orders and
 * 297 region spans reach 10 but no lower without the second boundary, and
 * dropping leadingColor's `volatile` folds its pair and costs an instruction.
 * The converse (trailingColor `volatile` too, second block deleted) keeps size
 * and registers but schedules two colour loads late: 4 words (lane c-1). */
void overlay101BuildBorder(Overlay101Gfx **displayList, s32 x, s32 y,
                           s32 width, s32 height, s32 intensity, s32 alpha,
                           s32 swapColors) {
    u32 trailingColor;
    volatile u32 leadingColor;
    volatile u32 interiorColor;
    Overlay101BorderRect *rect;
    Overlay101BorderRect rects[5];

    if (swapColors != 0) {
        overlay101BuildIntensityColors(
            intensity + 1, alpha, (u32 *)&leadingColor,
            (u32 *)&trailingColor, (u32 *)&interiorColor, NULL);
    } else {
        overlay101BuildIntensityColors(
            intensity + 1, alpha, (u32 *)&trailingColor,
            (u32 *)&leadingColor, (u32 *)&interiorColor, NULL);
    }

    rect = rects;
    rect[0].x0 = x;
    rect[0].y0 = y;
    rect[0].x1 = x + 1;
    rect[0].y1 = y + height - 1;
    rect[1].x0 = x;
    rects[0].color = leadingColor;
    rect[1].y0 = y + height - 1;
    rect[1].x1 = x + width;
    if (1) {
        rect[1].y1 = y + height;
        rects[1].color = leadingColor;
        rect[2].x0 = x + width - 1;
        rect[2].y0 = y + 1;
    }
    rect[2].x1 = x + width;
    rect[2].y1 = y + height - 1;
    rects[2].color = trailingColor;
    rect[3].x0 = x + 1;
    if (1) {
    }
    rect[3].y0 = y;
    rect[3].x1 = x + width;
    rect[3].y1 = y + 1;
    rects[3].color = trailingColor;

    rect += 4;
    rect->x0 = x + 1;
    rect->y0 = y + 1;
    rect->x1 = x + width - 1;
    rect->y1 = y + height - 1;
    rect->color = interiorColor;

    frontDrawRectangles(displayList, 5, rects, 0);
}

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor.
 *
 * Matched 2026-10-01 by discarding the inherited shape rather than tuning it.
 * The old candidate kept the colours, the dimensions and the records in one
 * padded work struct, forced the per-record colour reloads with `volatile`,
 * carried `right` and `bottom` in declared locals and steered uopt with two
 * `if (1)` regions. What the listing asks for is plainer on every point:
 *   - `out = records` is assigned before the BuildIntensityColors call, so
 *     the pointer's value reaches the record stores across a call. uopt then
 *     still folds every store address to a frame offset but no longer proves
 *     the stores miss the four address-taken colours, so each record reloads
 *     its colour with no qualifier, and the last record goes through the
 *     pointer's own register. Assigning it after the call folds the first
 *     three records' reads into one load.
 *   - x, y, width and height are ordinary locals shared by the mode 1/3 arm
 *     and the mode 2 arm, which is what colours the mode 1/3 height.
 *   - the right and bottom edges are the expressions `x + width` and
 *     `y + height` at each use; uopt commons them and spills the one that
 *     crosses calls to a compiler temporary below the record array.
 *   - `out` is declared between the colours and the array, which is the
 *     four-byte cell above the records. */
void overlay101DrawPanel(Overlay101Gfx **displayList, Overlay101Panel *panel) {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u32 full;
    u32 dimmer;
    u32 dim;
    u32 darkest;
    Overlay101PanelRect *out;
    Overlay101PanelRect records[20];

    overlay101SetScissor(displayList, 0, 0, 1000, 1000);

    if ((panel->mode == 1) || (panel->mode == 3)) {
        x = panel->currentX +
            (((panel->x - panel->currentX) * panel->phase) >> 6);
        y = panel->currentY +
            (((panel->y - panel->currentY) * panel->phase) >> 6);
        width = 32 + (((panel->width - 32) * panel->phase) >> 6);
        height = 32 + (((panel->height - 32) * panel->phase) >> 6);
        overlay101BuildFrame(displayList, x, y, width, height, 0xC0C0C0FF);
    } else if (panel->mode == 2) {
        out = records;
        x = panel->x;
        y = panel->y;
        width = panel->width;
        height = panel->height;

        overlay101BuildIntensityColors(panel->intensity + 1, panel->alpha,
                                            &full, &dim, &dimmer, &darkest);

        out->x0 = x + 1;
        out->y0 = y + 12;
        out->x1 = x + 3;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x + 1;
        out->y0 = y + height - 3;
        out->x1 = x + width - 1;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x + width - 3;
        out->y0 = y + 12;
        out->x1 = x + width - 1;
        out->y1 = y + height - 1;
        out->color = dimmer;
        out++;

        out->x0 = x;
        out->y0 = y + 12;
        out->x1 = x + 1;
        out->y1 = y + height;
        out->color = dim;
        out++;

        out->x0 = x + 1;
        out->y0 = y + height - 1;
        out->x1 = x + width;
        out->y1 = y + height;
        out->color = dim;
        out++;

        out->x0 = x + width - 4;
        out->y0 = y + 12;
        out->x1 = x + width - 3;
        out->y1 = y + height - 3;
        out->color = dim;
        out++;

        out->x0 = x + 3;
        out->y0 = y + 12;
        out->x1 = x + 4;
        out->y1 = y + height - 3;
        out->color = full;
        out++;

        out->x0 = x + 4;
        out->y0 = y + height - 4;
        out->x1 = x + width - 3;
        out->y1 = y + height - 3;
        out->color = full;
        out++;

        out->x0 = x + width - 1;
        out->y0 = y + 12;
        out->x1 = x + width;
        out->y1 = y + height;
        out->color = full;
        out++;

        out->x0 = x + 4;
        out->y0 = y + 12;
        out->x1 = x + width - 4;
        out->y1 = y + height - 4;
        out->color = darkest;
        out++;

        frontDrawRectangles(displayList, 10, records, 0);

        overlay101BuildBorder(displayList, x, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, x + 12, y, width - 36, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, x + width - 24, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, x + width - 12, y, 12, 12,
                                   panel->intensity, panel->alpha, 0);

        if (panel->content != NULL) {
            overlay101SetScissor(displayList, x + 14, y + 2,
                                      x + width - 26, y + 10);
            fontUseFont(2);
            fontBackground(0, 0, 0, 0);
            fontColour(0, 0, 0, 255, panel->alpha);
            fontPrintXY(displayList, x + 14, y + 2,
                                       panel->content, 0);
            overlay101SetScissor(displayList, 0, 0, 1000, 1000);
        }
    } else if (panel->mode == 0) {
        func_8002F618(displayList, D_1F4, panel->currentX,
                                        panel->currentY, 255, 255, 255,
                                        panel->alpha);
    }
}

#define O101_PRIM(packet, color)                                            \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->words.w0 = 0xFA000000;                                      \
        macroCommand->words.w1 = (color);                                         \
    }

#define O101_VERTICES(packet, address)                                      \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        void *macroAddress = (void *)(address);                             \
        macroCommand->words.w0 =                                                  \
            ((((((u32)macroAddress & 6U) | 0x60U) & 0xFFU) << 16) |        \
             0x04000000U) | 0x80U;                                         \
        macroCommand->words.w1 = (u32)macroAddress;                               \
    }

#define O101_DRAW(packet, address)                                          \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->words.w0 = 0x05510060;                                      \
        macroCommand->words.w1 = (u32)(address);                                  \
    }

#define O101_SYNC(packet)                                                   \
    {                                                                       \
        Overlay101Gfx *macroCommand = (Overlay101Gfx *)(packet);            \
        macroCommand->words.w0 = 0xE7000000;                                      \
        macroCommand->words.w1 = 0;                                               \
    }

/* Exact C: all 238 instruction words, the -152 frame, relocations, and linked
 * overlay range match after bounded permutation. DKR/JFG are donor-negative.
 *
 * Lane c-5 (2026-10-09): the permuter left the middle of the body as one
 * 1,553-character line of expanded packet macros, and a `new_var` copy of the
 * seconds angle. The copy was inert and is gone; the line is split with the
 * O101_* macros. One physical-line fact survives the split: `item = 2` must
 * share the line of the `do`. as1 breaks the tie between the hoisted alpha
 * 255 and the item 2 by source line, and on its own earlier line the 2 is
 * loaded first (2 words). */
void overlay101DrawClock(Overlay101Gfx **displayList, Mtx **matrices, Overlay101Vertex **vertexCursor, s32 originX, s32 originY)
{
  Overlay101ClockPoint *source;
  Overlay101Vertex *vertex;
  f32 major;
  f32 middle;
  Overlay101PhaseSpill phaseSpill;
  f32 scaledA;
  f32 scaledB;
  s32 majorWhole;
  s32 middleWhole;
  s32 remainder;
  s32 timeValue;
  s32 angle;
  s32 item;
  s32 point;
  s32 baseX;
  s32 baseY;
  u32 spent;
  originX *= 10;
  originY *= 10;
  baseX = originX;
  baseY = originY;
  func_80022A44(0.1f);
  camStandardOrtho(displayList, matrices);
  func_80022A44(1.0f);
  timeValue = D_338;
  major = ((f32) timeValue) / 216000.0f;
  majorWhole = (s32) major;
  remainder = timeValue - (majorWhole * 216000);
  middle = ((f32) remainder) / 3600.0f;
  middleWhole = (s32) middle;
  spent = ((u32) (majorWhole * 216000)) + ((u32) (3600 * middleWhole));
  phaseSpill.minor.write = ((f32) (timeValue - ((s32) spent))) / 60.0f;
  vertex = *vertexCursor;
  texDPTextureX(displayList, D_33C, 4, 0);
  O101_PRIM((*displayList)++, 0xFFFFFFA0);
  O101_VERTICES((*displayList)++, (void *) (((u32) vertex) + 0x80000000U));
  O101_DRAW((*displayList)++, D_800002D8);
  O101_SYNC((*displayList)++);
  O101_PRIM((*displayList)++, 0xFFFFFFFF);
  source = D_278;
  item = 2; do /* same line: see above */
  {
    if (item == 2)
    {
      angle = (s32) ((major * 65536.0f) / 24.0f);
      scaledA = func_8002A8BC(angle) * 10.0f;
      scaledB = func_8002A8C0(angle) * 10.0f;
      point = 4;
    }
    else if (item == 1)
    {
      angle = (s32) ((middle * 65536.0f) / 60.0f);
      scaledA = func_8002A8BC(angle) * 10.0f;
      scaledB = 10.0f;
      scaledB = func_8002A8C0(angle) * scaledB;
      point = 4;
    }
    else
    {
      angle = (((s32) phaseSpill.minor.read) << 16) / 60;
      scaledA = func_8002A8BC(angle) * 10.0f;
      scaledB = func_8002A8C0(angle) * 10.0f;
      point = 4;
    }
    if (point--)
    {
      do
      {
        vertex->x = baseX + ((s32) ((source->y * scaledB) + (scaledA * source->x)));
        vertex->y = baseY + ((s32) ((scaledA * source->y) - ((*source).x * scaledB)));
        vertex->z = 0;
        vertex->red = 0;
        vertex->green = 0;
        vertex->blue = 0;
        vertex->alpha = 255;
        source++;
        vertex++;
      }
      while (point--);
    }
  }
  while (item--);
  *vertexCursor = vertex;
}
#undef O101_PRIM
#undef O101_VERTICES
#undef O101_DRAW
#undef O101_SYNC

void overlay101DrawChain(Gfx **context, Mtx **mtx, s32 *vertices,
                         Overlay101DrawChain *chain) {
    register Overlay101DrawNode_DrawChain *node;
    s32 type;

    if (chain->enabled != 0) {
        camStandardOrtho(context, mtx);
        if (chain->mode != 4) {
            overlay101DrawPanel(context, chain);
        }
        node = chain->next;
        type = chain->type;
        while (node != NULL) {
            switch (type) {
                case 1: {
                    Overlay101DrawNode_DrawChain *current = node;
                    func_overlay_101_F0002510_18DDD30(context, chain, node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 2: {
                    Overlay101DrawNode_DrawChain *current = node;
                    overlay101DrawTransformed(context, mtx, vertices, chain,
                                                  node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                case 3: {
                    Overlay101DrawNode_DrawChain *current = node;
                    overlay101DrawElement(context, chain, node);
                    type = current->type;
                    node = current->next;
                    break;
                }
                default:
                    node = NULL;
                    break;
            }
        }
    }
}

void overlay101UpdateChains(s32 step) {
    s32 count;
    s32 type;
    O101Elem_gOverlay101Order *order;
    Overlay101UpdateChain *chain;
    Overlay101UpdateNode *node;

    gOverlay101Time214 += step;
    gOverlay101Time338 += step;
    count = o101OrderCounter_o101Reloc;
    if (count--) {
        order = &gOverlay101Order[count];
        do {
            chain = order->m1;
            if (chain->state == 1) {
                chain->amount += step * 4;
                if (chain->amount >= 0x40) {
                    chain->state = 2;
                }
            } else if (chain->state == 3) {
                chain->amount -= step * 4;
                if (chain->amount <= 0) {
                    chain->state = 0;
                }
            }

            node = chain->next;
            type = chain->type;
            if ((node != NULL) && (type != 0)) {
                do {
                    if ((type == 2) && (node->flags & 1)) {
                        texAnimateSprite(
                            node->resource, &node->flags, node->value13,
                            &node->flags - 1, step);
                    }
                    type = node->type;
                    node = node->next;
                } while ((node != NULL) && (type != 0));
            }
            order--;
        } while (count--);
    }
}

void overlay101DrawSlots(Gfx **context, Mtx **matrices, s32 y) {
    s32 count;
    s32 first;
    O101Elem_gOverlay101Order *slot;

    count = o101OrderCounter_o101Reloc;
    first = 1;
    if (count--) {
        slot = &gOverlay101Order[count];
        do {
            overlay101DrawChain(context, matrices, y, slot->m2);
            if (first != 0) {
                overlay101DrawClock(context, matrices, y, 0x6E, 0x4D);
                first = 0;
            }
            slot--;
        } while (count--);
    }
}

/* ---- func_overlay_101_F0003A58_18DF278 (+0x3A58) ---- */
#define D_1C D_1C.m0
#define D_38 D_38.m0
#define D_54 D_54.m0
#define D_8C D_8C.m0
#define D_A8 D_A8.m0
#define D_INPUT D_INPUT.m0

/* Tier A: stock IDO with this TU's R4300 multiply-scheduler flag reproduces
 * the owned text. Array expressions preserve the builder family's shared
 * address webs; group mode is initialized before geometry. See the matching
 * handoff for the compiler-flag control and linked promotion evidence. */
void func_overlay_101_F0003A58_18DF278(void) {
    s32 length;
    void *handle;

#define INIT_GROUP(group, px, pwidth, py, pheight, value0, value1, input, owner) \
    D_0.el[1 + group].m15.mode = 0; \
    D_0.el[1 + group].m15.x = (px); \
    D_0.el[1 + group].m15.width = (pwidth); \
    D_0.el[1 + group].m15.y = (py); \
    D_0.el[1 + group].m15.height = (pheight); \
    D_0.el[1 + group].m15.value12 = (value0); \
    D_0.el[1 + group].m15.value14 = (value1); \
    D_0.el[1 + group].m15.color16 = 0xFF; \
    D_0.el[1 + group].m15.color17 = 0xFF; \
    D_0.el[1 + group].m15.childType = 0; \
    D_0.el[1 + group].m15.child = 0; \
    D_0.el[1 + group].m15.data18 = D_INPUT.input; \
    gOverlay101Order[o101OrderCounter_o101Reloc].m2 = &(owner); \
    o101OrderCounter_o101Reloc = o101OrderCounter_o101Reloc + 1

#define ADD_NODE32(group, nodeX, nodeY, nodeScale, nodeValue, nodeColor, imageId) \
    D_340[o101Node32Counter_o101Reloc].m0.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m0.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m0.scale = (nodeScale); \
    D_340[o101Node32Counter_o101Reloc].m0.value10 = (nodeValue); \
    D_340[o101Node32Counter_o101Reloc].m0.color12 = (nodeColor); \
    D_340[o101Node32Counter_o101Reloc].m0.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m0.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m0.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m0.previousType = D_0.el[1 + group].m15.childType; \
    D_340[o101Node32Counter_o101Reloc].m0.previous = D_0.el[1 + group].m15.child; \
    D_340[o101Node32Counter_o101Reloc].m0.handle = handle; \
    D_0.el[1 + group].m15.childType = 2; \
    D_0.el[1 + group].m15.child = &D_340[o101Node32Counter_o101Reloc].m0; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

#define ADD_NODE20(group, nodeX, nodeY, imageId) \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.x = (nodeX); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.y = (nodeY); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.scale = 1.0f; \
    handle = func_80036DD0(imageId); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.previousType = D_0.el[1 + group].m15.childType; \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.previous = D_0.el[1 + group].m15.child; \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m0.handle = handle; \
    D_0.el[1 + group].m15.childType = 1; \
    D_0.el[1 + group].m15.child = &gOverlay101Resources20[o101Node20Counter_o101Reloc].m0; \
    o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1

#define ADD_TEXT(group, input, textX, textY, c0, c1, c2, c3) \
    D_540[D_1D0].m0.x = (textX); \
    D_540[D_1D0].m0.y = (textY); \
    length = overlay101ByteLength(D_INPUT.input); \
    D_540[D_1D0].m0.length = (u8)length; \
    D_540[D_1D0].m0.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m0.mode = 2; \
    D_540[D_1D0].m0.color0 = (c0); \
    D_540[D_1D0].m0.color1 = (c1); \
    D_540[D_1D0].m0.color2 = (c2); \
    D_540[D_1D0].m0.color3 = (c3); \
    D_540[D_1D0].m0.kind = 4; \
    D_540[D_1D0].m0.text = D_INPUT.input; \
    D_540[D_1D0].m0.previousType = D_0.el[1 + group].m15.childType; \
    D_540[D_1D0].m0.previous = D_0.el[1 + group].m15.child; \
    D_0.el[1 + group].m15.childType = 3; \
    D_0.el[1 + group].m15.child = &D_540[D_1D0].m0; \
    D_1D0 = D_1D0 + 1

#define INIT_GROUP0() \
    D_0.el[1 + 0].m15.mode = 4; \
    D_0.el[1 + 0].m15.value12 = 0x140; \
    D_0.el[1 + 0].m15.value14 = 0xF0; \
    D_0.el[1 + 0].m15.data18 = &D_C64; \
    D_0.el[1 + 0].m15.color16 = 0xFF; \
    D_0.el[1 + 0].m15.color17 = 0xFF; \
    D_0.el[1 + 0].m15.x = 0; \
    D_0.el[1 + 0].m15.width = 0; \
    D_0.el[1 + 0].m15.y = 0; \
    D_0.el[1 + 0].m15.height = 0; \
    D_0.el[1 + 0].m15.childType = 0; \
    D_0.el[1 + 0].m15.child = 0; \
    gOverlay101Order[o101OrderCounter_o101Reloc].m2 = &D_1C; \
    o101OrderCounter_o101Reloc = o101OrderCounter_o101Reloc + 1

    INIT_GROUP0();
    ADD_NODE32(0, 242, 334, 1.0f, 0, 0xFF, 145);

    INIT_GROUP(1, 32, 24, 48, 36, 224, 168, data0, D_38);
    ADD_TEXT(1, text4, 112, 144, 255, 255, 255, 255);
    ADD_TEXT(1, text8, 112, 154, 255, 255, 255, 255);
    ADD_NODE20(1, -100, 14, 4);

    INIT_GROUP(2, 32, 104, 80, 57, 160, 126, dataC, D_54);
    ADD_TEXT(2, text10, 80, 112, 255, 255, 255, 255);
    ADD_NODE20(2, 32, 14, 11);

    INIT_GROUP(3, 32, 64, 58, 24, 204, 192, data14, D_0.el[4].m15);
    ADD_NODE20(3, 6, 14, 6);

    INIT_GROUP(4, 40, -132, 40, -132, 240, 132, data18, D_8C);
    ADD_TEXT(4, text1C, 120, 96, 255, 255, 255, 255);
    ADD_TEXT(4, text20, 120, 106, 255, 255, 255, 255);
    ADD_TEXT(4, text24, 120, 116, 255, 255, 255, 255);
    ADD_NODE20(4, 80, 14, 14);

    INIT_GROUP(5, -64, -12, 64, 16, 96, 104, data28, D_A8);
    ADD_TEXT(5, text2C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(5, 8, 13, 3);

    INIT_GROUP(6, 320, -12, 160, 16, 96, 104, data30, D_C4);
    ADD_TEXT(6, text34, 48, 90, 192, 192, 255, 255);
    ADD_NODE20(6, 8, 13, 5);

    INIT_GROUP(7, 320, 240, 160, 120, 96, 104, data38, D_E0);
    ADD_TEXT(7, text3C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(7, 8, 13, 18);

    INIT_GROUP(8, -64, 240, 64, 120, 96, 104, data40, D_FC);
    ADD_TEXT(8, text44, 48, 90, 255, 255, 192, 255);
    ADD_NODE20(8, 8, 13, 8);

    INIT_GROUP(9, 76, -162, 76, -162, 172, 162, data48, D_118);
    ADD_TEXT(9, text4C, 86, 138, 192, 255, 192, 255);
    ADD_TEXT(9, text50, 86, 148, 192, 255, 192, 255);
    ADD_NODE20(9, 6, 14, 10);

    INIT_GROUP(10, 32, 144, 32, 91, 192, 62, data54, D_134);
    ADD_TEXT(10, text58, 96, 14, 0, 0, 0, 0);
    ADD_TEXT(10, text5C, 96, 24, 0, 0, 0, 0);
    ADD_TEXT(10, text60, 96, 34, 0, 0, 0, 0);
    ADD_TEXT(10, text64, 96, 48, 0, 0, 0, 0);

    overlay101Reset(D_o101ResetData340);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
#undef INIT_GROUP0
}
#undef D_1C
#undef D_38
#undef D_54
#undef D_8C
#undef D_A8
#undef D_INPUT

/* ---- func_overlay_101_F000512C_18E094C (+0x512C) ---- */
#define D_0 D_0.m0
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_54 D_54.m1
#define D_INPUT D_INPUT.m1

/* Four source-shape levers took this family from 296/298 masked words to
 * 130/136. None costs an instruction.
 * L59 -- every per-element assignment group is ONE physical line. as1 minimises
 *   (start_time, -aftercycles, -latency, addr, lineno, list position); with a
 *   group's stores on separate lines `lineno` is the deciding key and orders them
 *   in source order, while the ROM emits that group reversed. Folding the group
 *   retires the key, the raw list order supplies the reversal, and the prologue
 *   goes byte-exact. Worth 26 words and 3 of the 25 insertion sites on its own.
 * L100/L115 -- `index` carries the panel-order counter D_1C4 and the text-row
 *   counter D_1D0. That is what puts `index` in the ROM's callee-saved home with
 *   the ROM's save placement; one shared counter gives that home to the node
 *   counter instead and rotates the ugen ring for the rest of the function.
 * L115/L101 -- the two node blocks read their counter global DIRECTLY in the
 *   pointer expression, on BOTH sides of the call, and the `slot` local is gone.
 *   This CORRECTS what this comment used to say. It recorded "dropping `slot`
 *   for a direct counter read anywhere" as regressive, and each half alone does
 *   regress -- pre-call only and post-call only are both far worse. Doing both
 *   halves together is not: it reads 197 against 211. The pre-call direct read
 *   turns the pre-call counter from a globalcolor web into a ugen ring temp,
 *   which is what the ROM has, and because both definitions of the node pointer
 *   are then the same expression the pointer keeps the ROM's callee-saved home
 *   instead of splitting. The bump must sit immediately after the recomputed
 *   pointer; behind any store it makes uopt re-read the counter.
 * The byte-length local is `u8`, not `s32`. Worth 197 to 130, the largest single
 *   lever on the function, and it is semantically exact rather than lucky: both
 *   uses already truncate to eight bits -- the node field is `u8` and the opacity
 *   expression masks with 0xFF -- so the declaration only tells the compiler what
 *   the code already guarantees, and the mask becomes redundant. A 360-cell
 *   lattice over the callee's declared return type, the local's type, the store
 *   cast and the opacity mask puts every floor cell at LOCAL `u8`, with the
 *   return type indifferent across all six spellings. The naming bucket falls
 *   169 to 36 on this edit: the pre-edit residual splits into ten windows whose
 *   sizes repeat 14 then 7 once per text row, a per-iteration ring-consumption
 *   difference, and after it what is left is one clean ring shift.
 * Exhausted, with what was varied. The counter-bump STORE: 659 constrained
 *   permutations of the whole post-call block -- the bump, the three node stores
 *   and the two root stores under their two real dependences -- across four bump
 *   spellings, all flat. The block order was re-measured at the post-u8 plateau
 *   and the adopted form is still the floor, so it is not a stale result.
 * The no-op paragraph that used to close this comment is answered below, and it
 *   was right that L127 does not reach the residual -- the lever wanted is the
 *   opposite one. */
/* 130 -> 99 -> 0. Lane p9-o101 took this family to 99 by deleting every local
 * that held an element address or an element index and spelling each store
 * through the array subscript of the counter global itself (L131: the subscript
 * written on both sides of a call is ONE IR name, so it is one range and both
 * occurrences take the caller-saved colour the shipped code uses, while the
 * counter read becomes its own range on the callee-saved register and the
 * pre-call read lands in a ring temporary -- L130, a local is a symbol and a
 * symbol is never a ring temporary). The counter bump moves to the end of its
 * group as part of that edit, because the stores then read the counter instead
 * of a captured address. Every statement order was re-climbed afterwards, since
 * a statement-order optimum belongs to the shape and not to the function (L146).
 *
 * The last 99 were the four text rows, and they were ONE COUNT. ugen drew
 * sixteen integer ring temporaries per row where the shipped code draws fifteen,
 * so the phase ran one position ahead and fell one further behind per row --
 * row 1 ours t6 against t5, then t4, t3, t2. The instrumented ugen's
 * DKWB-FREELIST records name every draw with its source line, and the extra one
 * was the second of two masks: with the length local declared `u8`, reading it
 * already emits a mask of the call result, so `(u32)(length & 0xFF)` masks
 * twice and ugen draws a ring temporary for each, after which as1's peephole
 * folds the pair into the single `andi` the object shows. The draw is spent but
 * the instruction is gone, which is why no register census could see the cause,
 * and why 155 L127 no-op cells measured on this family were all flat -- a no-op
 * CONSUMES a ring temporary and this residual needed one FEWER. Deleting the
 * redundant mask is semantically exact, since a `u8` is already inside 0xFF,
 * and it spends fifteen draws per row: every register in the text rows lands,
 * the first mismatch moves four words later, and what is left is schedule alone.
 *
 * That schedule was one statement move. `.kind = 4` belongs between `.color3`
 * and `.text`, which is where the four 2,100-byte builders in this overlay
 * already had it; on the two-mask shape the same move read 103, so it is L146
 * twice over. Walking the kind store across the macro on the one-mask shape
 * reads 20, 5, 4, 3, 2, 0, 28, 27, 123 from just after `.opacity` back to its
 * old place -- a single clean optimum at 0. Size delta 0, frame 0x38, and the
 * identical two edits close all three 1,520-byte siblings.
 * Lane p10-o101, 2026-09-12. */
void func_overlay_101_F000512C_18E094C(void) {
    s32 index;
    u8 length;
    void *handle;
    Node20 *node20;
    Node24 *node24;
    Node32 *node32;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_C78; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 = D_1C4 + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x93, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x5A; D_0.height48 = 0x20; D_0.value4A = 0x4C; D_0.value4C = 0x54; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = 0; D_0.data50 = D_INPUT.data68; D_1C0[D_1C4] = &D_38; D_1C4 = D_1C4 + 1;

    D_200[o101Node20Counter_o101Reloc].x = 6; D_200[o101Node20Counter_o101Reloc].y = 0xE; D_200[o101Node20Counter_o101Reloc].scale = 1.0f; handle = func_80036DD0(7); D_200[o101Node20Counter_o101Reloc].handle = handle; D_200[o101Node20Counter_o101Reloc].previousType = D_0.childType; D_200[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &D_200[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_0.x5E = 0x20; D_0.width60 = 0x40; D_0.y62 = 0x20; D_0.height64 = 0x78; D_0.value66 = 0xC0; D_0.value68 = 0x46; D_0.mode5C = 0; D_0.color6A = 0xFF; D_0.color6B = 0xFF; D_0.secondChildType = 0; D_0.secondChild = 0; D_0.data6C = D_INPUT.data6C; D_1C0[D_1C4] = &D_54; D_1C4 = D_1C4 + 1;

#define ADD_TEXT_ROW(field, rowY)                                            \
    D_540[D_1D0].m1.x = 0x60; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(D_INPUT.field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = D_INPUT.field; \
    D_540[D_1D0].m1.previousType = D_0.secondChildType; \
    D_540[D_1D0].m1.previous = D_0.secondChild; \
    D_0.secondChildType = 3; \
    D_0.secondChild = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(text70, 0x10);
    ADD_TEXT_ROW(text74, 0x1E);
    ADD_TEXT_ROW(text78, 0x28);
    ADD_TEXT_ROW(text7C, 0x36);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_CE0);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_54
#undef D_INPUT

/* ---- func_overlay_101_F000571C_18E0F3C (+0x571C) ---- */
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_54 D_54.m1
#define D_INPUT D_INPUT.m2

#define D_0 D_0.m17

/* Mickey-local structural sibling: func_overlay_101_F00063F8_18E1C18. */
/* Two source-shape levers took this from 263 masked words to 222; the frame,
 * the size and the whole callee-saved ladder were already exact.
 *
 * L59 -- the four non-macro assignment groups are ONE physical line each. as1
 *   minimises (start_time, -aftercycles, -latency, addr, lineno, ready-list
 *   position); with a group's stores on separate lines `lineno` decides and
 *   emits them in source order, while the ROM emits each group reversed.
 *   Folding retires the key and the raw ready-list order supplies the reversal.
 *   Worth 27 words on its own: 263 to 236, structural 57 to 28, and the first
 *   thirty-three instructions go byte-exact.
 * L110/L115 -- the FIRST node group reads the pool counter inline,
 *   `node32 = &D_340[D_1CC].m1`, and only the post-call half names the index
 *   local. The ROM puts that pre-call read in a scratch register and the
 *   post-call one in the counter's callee-saved home; giving both the same
 *   local forces one web across both and mis-colours the pre-call half.
 *   Worth a further 13 words, and it is specific to the first group: the same
 *   change inside the image macro regresses to 297 and inside the text macro
 *   to 364.
 *
 * Refuted here, each measured against the folded 236 baseline. The
 *   call-spanning edit that closed the missing sixth web on the F78F4
 *   quadruplet -- storing handle, previousType and previous through the
 *   pre-call pointer -- is a REGRESSION on this function (305 for all three
 *   moved, 242 for handle alone, 248 for the other two), because this
 *   function's ladder already homes nine slots and there is no web to win.
 *   The L100 counter partition likewise does not pay: splitting the node and
 *   text counters reads 346, the F63F8 shape (owner+text in one local, node in
 *   another) 383, one local for all three 283, and giving the owner counter
 *   its own local 236, against 236 for the incumbent.
 * Measured inert: the x/y store order in the first node group once the pool
 *   read is inline.
 * Remaining: size, frame and the s0..s8 slot map are all exact; 178 of the 222
 *   are register naming and, by L114, almost all of it is ugen ring phase
 *   rather than colour -- the instrumented uopt shows every remaining split
 *   web preferring a caller-saved colour, so no missing web is left to win. */
/* Matched 2026-09-12 by lane p9-o101, 222 -> 29 -> 0, with the lever that
 * closed the four 2,100-byte builders in this overlay: DELETE EVERY LOCAL THAT
 * HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX and spell each store through the
 * array subscript of the counter global itself.
 *
 * A uopt live range is formed per IR name (L131), so `D_340[D_1CC].m1` written on
 * both sides of a call is ONE range spanning the call and both occurrences take
 * the caller-saved colour the shipped code uses; the counter load becomes its
 * own range covering every element and takes the callee-saved register the
 * index local used to hold; and the pre-call counter read lands in a ring
 * temporary rather than a coloured symbol, which is the free ring draw of L129.
 * A local reaches none of the three, because a local is a symbol and a symbol is
 * never a ring temporary (L130). That single edit is worth 222 -> 29 here.
 *
 * The counter bump moves to the end of each group as part of the edit: without
 * the pointer local the stores read the counter rather than a captured address,
 * so the bump can no longer sit early. That is semantics, not schedule.
 *
 * The last 29 were statement order, re-climbed from scratch because a
 * statement-order optimum is never portable across a shape change -- the four
 * one-line groups and the two macros, hill-climbed under the read-before-write
 * dependences with each call held as a barrier. A second round is flat. */
void func_overlay_101_F000571C_18E0F3C(void) {
    s32 length;
    void *handle;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_C8C; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 += 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0x4E; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x92, 0); D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x5A; D_0.height48 = 0x20; D_0.value4A = 0xCC; D_0.value4C = 0x54; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = D_INPUT.data80; D_0.childType = 0; D_0.child = 0; D_1C0[D_1C4] = &D_38; D_1C4 += 1;

#define ADD_IMAGE_NODE(nodeX, imageScale, imageId)                            \
    D_340[o101Node32Counter_o101Reloc].m1.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m1.y = 0xE; \
    D_340[o101Node32Counter_o101Reloc].m1.scale = (imageScale); \
    D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; \
    D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m1.handle = handle; \
    D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.childType; \
    D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.child; \
    D_0.childType = 2; \
    D_0.child = &D_340[o101Node32Counter_o101Reloc].m1; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

    ADD_IMAGE_NODE(6, 0.8f, 0xA1);
    ADD_IMAGE_NODE(0x46, 0.8f, 0xA0);
    ADD_IMAGE_NODE(0x86, 0.8f, 0xA0);

#undef ADD_IMAGE_NODE

    D_0.x5E = 0x20; D_0.width60 = 0x40; D_0.y62 = 0x60; D_0.height64 = 0x78; D_0.value66 = 0xC0; D_0.value68 = 0x46; D_0.mode5C = 0; D_0.color6A = 0xFF; D_0.color6B = 0xFF; D_0.secondChildType = 0; D_0.secondChild = 0; D_0.data6C = D_INPUT.data84; D_1C0[D_1C4] = &D_54; D_1C4 += 1;

#define ADD_TEXT_ROW(field, rowY)                                            \
    D_540[D_1D0].m1.x = 0x60; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = (field); \
    D_540[D_1D0].m1.previousType = D_0.secondChildType; \
    D_540[D_1D0].m1.previous = D_0.secondChild; \
    D_0.secondChildType = 3; \
    D_0.secondChild = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.text88, 0x10);
    ADD_TEXT_ROW(D_INPUT.text8C, 0x1E);
    ADD_TEXT_ROW(D_INPUT.text90, 0x28);
    ADD_TEXT_ROW(D_INPUT.text94, 0x36);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_10EC);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_54
#undef D_INPUT

/* ---- func_overlay_101_F0005E08_18E1628 (+0x5E08) ---- */
#define D_0 D_0.m1
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_54 D_54.m1
#define D_INPUT D_INPUT.m3

/* Four source-shape levers took this family from 296/298 masked words to
 * 130/136. None costs an instruction.
 * L59 -- every per-element assignment group is ONE physical line. as1 minimises
 *   (start_time, -aftercycles, -latency, addr, lineno, list position); with a
 *   group's stores on separate lines `lineno` is the deciding key and orders them
 *   in source order, while the ROM emits that group reversed. Folding the group
 *   retires the key, the raw list order supplies the reversal, and the prologue
 *   goes byte-exact. Worth 26 words and 3 of the 25 insertion sites on its own.
 * L100/L115 -- `index` carries the panel-order counter D_1C4 and the text-row
 *   counter D_1D0. That is what puts `index` in the ROM's callee-saved home with
 *   the ROM's save placement; one shared counter gives that home to the node
 *   counter instead and rotates the ugen ring for the rest of the function.
 * L115/L101 -- the two node blocks read their counter global DIRECTLY in the
 *   pointer expression, on BOTH sides of the call, and the `slot` local is gone.
 *   This CORRECTS what this comment used to say. It recorded "dropping `slot`
 *   for a direct counter read anywhere" as regressive, and each half alone does
 *   regress -- pre-call only and post-call only are both far worse. Doing both
 *   halves together is not: it reads 197 against 211. The pre-call direct read
 *   turns the pre-call counter from a globalcolor web into a ugen ring temp,
 *   which is what the ROM has, and because both definitions of the node pointer
 *   are then the same expression the pointer keeps the ROM's callee-saved home
 *   instead of splitting. The bump must sit immediately after the recomputed
 *   pointer; behind any store it makes uopt re-read the counter.
 * The byte-length local is `u8`, not `s32`. Worth 197 to 130, the largest single
 *   lever on the function, and it is semantically exact rather than lucky: both
 *   uses already truncate to eight bits -- the node field is `u8` and the opacity
 *   expression masks with 0xFF -- so the declaration only tells the compiler what
 *   the code already guarantees, and the mask becomes redundant. A 360-cell
 *   lattice over the callee's declared return type, the local's type, the store
 *   cast and the opacity mask puts every floor cell at LOCAL `u8`, with the
 *   return type indifferent across all six spellings. The naming bucket falls
 *   169 to 36 on this edit: the pre-edit residual splits into ten windows whose
 *   sizes repeat 14 then 7 once per text row, a per-iteration ring-consumption
 *   difference, and after it what is left is one clean ring shift.
 * Exhausted, with what was varied. The counter-bump STORE: 659 constrained
 *   permutations of the whole post-call block -- the bump, the three node stores
 *   and the two root stores under their two real dependences -- across four bump
 *   spellings, all flat. The block order was re-measured at the post-u8 plateau
 *   and the adopted form is still the floor, so it is not a stale result.
 * The no-op paragraph that used to close this comment is answered below, and it
 *   was right that L127 does not reach the residual -- the lever wanted is the
 *   opposite one. */
/* 130 -> 99 -> 0. Lane p9-o101 took this family to 99 by deleting every local
 * that held an element address or an element index and spelling each store
 * through the array subscript of the counter global itself (L131: the subscript
 * written on both sides of a call is ONE IR name, so it is one range and both
 * occurrences take the caller-saved colour the shipped code uses, while the
 * counter read becomes its own range on the callee-saved register and the
 * pre-call read lands in a ring temporary -- L130, a local is a symbol and a
 * symbol is never a ring temporary). The counter bump moves to the end of its
 * group as part of that edit, because the stores then read the counter instead
 * of a captured address. Every statement order was re-climbed afterwards, since
 * a statement-order optimum belongs to the shape and not to the function (L146).
 *
 * The last 99 were the four text rows, and they were ONE COUNT. ugen drew
 * sixteen integer ring temporaries per row where the shipped code draws fifteen,
 * so the phase ran one position ahead and fell one further behind per row --
 * row 1 ours t6 against t5, then t4, t3, t2. The instrumented ugen's
 * DKWB-FREELIST records name every draw with its source line, and the extra one
 * was the second of two masks: with the length local declared `u8`, reading it
 * already emits a mask of the call result, so `(u32)(length & 0xFF)` masks
 * twice and ugen draws a ring temporary for each, after which as1's peephole
 * folds the pair into the single `andi` the object shows. The draw is spent but
 * the instruction is gone, which is why no register census could see the cause,
 * and why 155 L127 no-op cells measured on this family were all flat -- a no-op
 * CONSUMES a ring temporary and this residual needed one FEWER. Deleting the
 * redundant mask is semantically exact, since a `u8` is already inside 0xFF,
 * and it spends fifteen draws per row: every register in the text rows lands,
 * the first mismatch moves four words later, and what is left is schedule alone.
 *
 * That schedule was one statement move. `.kind = 4` belongs between `.color3`
 * and `.text`, which is where the four 2,100-byte builders in this overlay
 * already had it; on the two-mask shape the same move read 103, so it is L146
 * twice over. Walking the kind store across the macro on the one-mask shape
 * reads 20, 5, 4, 3, 2, 0, 28, 27, 123 from just after `.opacity` back to its
 * old place -- a single clean optimum at 0. Size delta 0, frame 0x38, and the
 * identical two edits close all three 1,520-byte siblings.
 * Lane p10-o101, 2026-09-12. */
void func_overlay_101_F0005E08_18E1628(void) {
    s32 index;
    u8 length;
    void *handle;
    Node20 *node20;
    Node24 *node24;
    Node32 *node32;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_CA0; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 = D_1C4 + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x93, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x5A; D_0.height48 = 0x20; D_0.value4A = 0x4C; D_0.value4C = 0x54; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = 0; D_0.data50 = D_INPUT.data98; D_1C0[D_1C4] = &D_38; D_1C4 = D_1C4 + 1;

    D_200[o101Node20Counter_o101Reloc].x = 6; D_200[o101Node20Counter_o101Reloc].y = 0xE; D_200[o101Node20Counter_o101Reloc].scale = 1.0f; handle = func_80036DD0(7); D_200[o101Node20Counter_o101Reloc].handle = handle; D_200[o101Node20Counter_o101Reloc].previousType = D_0.childType; D_200[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &D_200[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_0.x5E = 0x20; D_0.width60 = 0x40; D_0.y62 = 0x20; D_0.height64 = 0x78; D_0.value66 = 0xC0; D_0.value68 = 0x46; D_0.mode5C = 0; D_0.color6A = 0xFF; D_0.color6B = 0xFF; D_0.secondChildType = 0; D_0.secondChild = 0; D_0.data6C = D_INPUT.data9C; D_1C0[D_1C4] = &D_54; D_1C4 = D_1C4 + 1;

#define ADD_TEXT_ROW(field, rowY)                                            \
    D_540[D_1D0].m1.x = 0x60; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(D_INPUT.field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = D_INPUT.field; \
    D_540[D_1D0].m1.previousType = D_0.secondChildType; \
    D_540[D_1D0].m1.previous = D_0.secondChild; \
    D_0.secondChildType = 3; \
    D_0.secondChild = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(textA0, 0x10);
    ADD_TEXT_ROW(textA4, 0x1E);
    ADD_TEXT_ROW(textA8, 0x28);
    ADD_TEXT_ROW(textAC, 0x36);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_1514);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_54
#undef D_INPUT

/* ---- func_overlay_101_F00063F8_18E1C18 (+0x63F8) ---- */
#define D_0 D_0.m2
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_54 D_54.m1
#define D_INPUT D_INPUT.m4

/* Three source-shape levers took this function from 296 masked words to 203.
 * L59 -- every per-element assignment group is ONE physical line. as1 minimises
 *   (start_time, -aftercycles, -latency, addr, lineno, list position); with a
 *   group's stores on separate lines `lineno` is the deciding key and orders them
 *   in source order, while the ROM emits that group reversed. Folding the group
 *   retires the key, the raw list order supplies the reversal, and the prologue
 *   goes byte-exact.
 * L100/L115 -- the counter locals are partitioned, not shared. `index` carries the
 *   panel-order counter D_1C4 and the text-row counter D_1D0. That is what puts
 *   `index` in the ROM's callee-saved home with the ROM's save placement.
 * L115/L101 -- the two node blocks read their counter global directly in the
 *   pointer expression, on BOTH sides of the call, and the `slot` local is gone.
 *   This is the same mechanism the F78F4 quadruplet needed and it does two
 *   different things here at once. The pre-call counter read stops being a
 *   globalcolor web and becomes a ugen ring temp, which is what the ROM has --
 *   `lw t5,0(s3)`, `sll t6,t5,0x5`, `addu s0,s4,t6` go byte-exact and the first
 *   mismatch moves from +0x8C to +0x9C -- and because both definitions of the
 *   node pointer are now the same symbol expression, the pointer keeps the ROM's
 *   callee-saved s0 instead of splitting into two caller-saved webs. The
 *   post-call counter then takes the ROM's a1. Worth 213 to 203 and the naming
 *   bucket 167 to 104. Doing only half of it is worse than either: pre-call only
 *   reads 215 because the node pointer splits, post-call only reads 314.
 * The counter bump must stay in front of every store. `D_1CC = D_1CC + 1`
 *   immediately after the pointer expression reuses the load; placed behind any
 *   store it makes uopt re-read the counter, at one extra load per block.
 * L114, corrected for this procedure, and it is per procedure. Do NOT classify a
 *   naming row by register bank here. This procedure's own p1cost rows decode as
 *   c2 v1, c4 a1, c5 a2, c6 a3, c7 t0, c8 t1, c9 t2, c10 t3, c11 t4, c12 t5,
 *   c13 unnamed, c14-c22 s0-s8, and globalcolor really does assign three t-bank
 *   colours in this compilation (two webs to c7 t0, one to c8 t1) alongside 22
 *   webs it declines to ugen. So "t-bank means ring phase" is false here. What
 *   makes the lever above work is narrower and measured: t5, the register the
 *   pre-call counter needs, is NOT among the colours globalcolor assigns, so
 *   that value has to reach the ring, and the direct counter read is what puts
 *   it there. A later lane must re-decode the table from its own procedure's
 *   records rather than inherit this one.
 * Measured inert: declaration order, `register`, the local's type, statement
 *   order inside the text-row macro, and dropping the now-unused `slot`
 *   declaration (L99).
 * Measured regressive: splitting the counter per block or per call side, and
 *   moving the opacity store later.
 * The byte-length local is `u8`, not `s32`, and the callee is declared `s32`
 *   rather than `s8`. Worth 203 to 136, the largest single lever on the
 *   function, and semantically exact rather than lucky: both uses already
 *   truncate to eight bits -- the node field is `u8` and the opacity expression
 *   masks with 0xFF -- so the declaration only states what the code already
 *   guarantees. A 360-cell lattice over the callee's return type, the local's
 *   type, the store cast and the opacity mask puts every floor cell at LOCAL
 *   `u8`, with the return type indifferent across all six spellings.
 * The counter-bump STORE, which this comment used to name as the next lever, is
 *   exhausted. 659 constrained permutations of the whole post-call block -- the
 *   bump, the three node stores and the two root stores under their two real
 *   dependences -- across four bump spellings, all flat, measured on the
 *   identical F000512C sibling. The reading that the axis is not statement
 *   order holds, now over the whole block rather than six statements. The block
 *   order was re-measured after the type lever and the adopted form is still
 *   the floor, so it is not a stale plateau.
 * The no-op paragraph that used to close this comment is answered below, and it
 *   was right that L127 does not reach the residual -- the lever wanted is the
 *   opposite one. */
/* 130 -> 99 -> 0. Lane p9-o101 took this family to 99 by deleting every local
 * that held an element address or an element index and spelling each store
 * through the array subscript of the counter global itself (L131: the subscript
 * written on both sides of a call is ONE IR name, so it is one range and both
 * occurrences take the caller-saved colour the shipped code uses, while the
 * counter read becomes its own range on the callee-saved register and the
 * pre-call read lands in a ring temporary -- L130, a local is a symbol and a
 * symbol is never a ring temporary). The counter bump moves to the end of its
 * group as part of that edit, because the stores then read the counter instead
 * of a captured address. Every statement order was re-climbed afterwards, since
 * a statement-order optimum belongs to the shape and not to the function (L146).
 *
 * The last 99 were the four text rows, and they were ONE COUNT. ugen drew
 * sixteen integer ring temporaries per row where the shipped code draws fifteen,
 * so the phase ran one position ahead and fell one further behind per row --
 * row 1 ours t6 against t5, then t4, t3, t2. The instrumented ugen's
 * DKWB-FREELIST records name every draw with its source line, and the extra one
 * was the second of two masks: with the length local declared `u8`, reading it
 * already emits a mask of the call result, so `(u32)(length & 0xFF)` masks
 * twice and ugen draws a ring temporary for each, after which as1's peephole
 * folds the pair into the single `andi` the object shows. The draw is spent but
 * the instruction is gone, which is why no register census could see the cause,
 * and why 155 L127 no-op cells measured on this family were all flat -- a no-op
 * CONSUMES a ring temporary and this residual needed one FEWER. Deleting the
 * redundant mask is semantically exact, since a `u8` is already inside 0xFF,
 * and it spends fifteen draws per row: every register in the text rows lands,
 * the first mismatch moves four words later, and what is left is schedule alone.
 *
 * That schedule was one statement move. `.kind = 4` belongs between `.color3`
 * and `.text`, which is where the four 2,100-byte builders in this overlay
 * already had it; on the two-mask shape the same move read 103, so it is L146
 * twice over. Walking the kind store across the macro on the one-mask shape
 * reads 20, 5, 4, 3, 2, 0, 28, 27, 123 from just after `.opacity` back to its
 * old place -- a single clean optimum at 0. Size delta 0, frame 0x38, and the
 * identical two edits close all three 1,520-byte siblings.
 * Lane p10-o101, 2026-09-12. */
void func_overlay_101_F00063F8_18E1C18(void) {
    s32 index;
    u8 length;
    void *handle;
    Node32 *node32;
    Node24 *node24;
    Node20 *node20;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_CB4; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 = D_1C4 + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0x4E; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x92, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x9A; D_0.height48 = 0x20; D_0.value4A = 0x4C; D_0.value4C = 0x54; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = 0; D_0.data50 = D_INPUT.dataB0; D_1C0[D_1C4] = &D_38; D_1C4 = D_1C4 + 1;

    D_200[o101Node20Counter_o101Reloc].x = 6; D_200[o101Node20Counter_o101Reloc].y = 0xE; D_200[o101Node20Counter_o101Reloc].scale = 1.0f; handle = func_80036DD0(7); D_200[o101Node20Counter_o101Reloc].handle = handle; D_200[o101Node20Counter_o101Reloc].previousType = D_0.childType; D_200[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &D_200[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_0.x5E = 0x20; D_0.width60 = 0x40; D_0.y62 = 0x60; D_0.height64 = 0x78; D_0.value66 = 0xC0; D_0.value68 = 0x46; D_0.mode5C = 0; D_0.color6A = 0xFF; D_0.color6B = 0xFF; D_0.secondChildType = 0; D_0.secondChild = 0; D_0.data6C = D_INPUT.dataB4; D_1C0[D_1C4] = &D_54; D_1C4 = D_1C4 + 1;

#define ADD_TEXT_ROW(field, rowY)                                            \
    D_540[D_1D0].m1.x = 0x60; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(D_INPUT.field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = D_INPUT.field; \
    D_540[D_1D0].m1.previousType = D_0.secondChildType; \
    D_540[D_1D0].m1.previous = D_0.secondChild; \
    D_0.secondChildType = 3; \
    D_0.secondChild = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(textB8, 0x10);
    ADD_TEXT_ROW(textBC, 0x1E);
    ADD_TEXT_ROW(textC0, 0x28);
    ADD_TEXT_ROW(textC4, 0x36);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_1920);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_54
#undef D_INPUT

/* ---- func_overlay_101_F00069E8_18E2208 (+0x69E8) ---- */
#define D_1C D_1C.m2
#define D_38 D_38.m2
#define D_54 D_54.m2
#define D_8C D_8C.m1
#define D_A8 D_A8.m1
#define D_INPUT D_INPUT.m5

/* Tier B: this function's runtime relocation records resolve ten sprite
 * loads to resident texLoadSprite, four screen loads to func_80036DD0,
 * seven local calls to overlay101ByteLength, and the final call to
 * overlay101Reset. The matched local callees establish their prototypes. */
/* The tail call is a SYMBOL relocation record, not a JUMP one: the shipped
 * word carries the 0xF0000000 addend rather than offset >> 2, so it must go
 * through the generated surface name even though the callee is defined in
 * this same overlay. Naming the in-module definition scores 0 masked words
 * and still builds a different ROM, by exactly this one word. */

/*
 * Mickey-local reconstruction from this overlay's extracted function and
 * the typed node builders immediately before and after it. The pinned DKR
 * v77/v80 and JFG overlay scans report no donor for overlay 101.
 *
 * 613 masked words -> 0, lane p10-o101 2026-09-12, on three edits and no
 * new identity. The shape is the one that closed the rest of this overlay's
 * builder family, and the previous handoff's structural reading -- an
 * exchanged saved-register pair plus a scratch ring one slot behind from
 * +0x88 -- was a consequence of the carrier, not a separate problem.
 *
 * L145. DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX
 *   and spell each store through the array subscript of the counter global
 *   itself. `index`, `orderIndex`, `node20`, `node24` and `node32` are all
 *   gone. A uopt live range is formed per IR name (L131), so the subscript
 *   written on both sides of a call is ONE range spanning the call and both
 *   occurrences take the caller-saved colour the shipped code uses; the
 *   counter read becomes its own range and takes the callee-saved register
 *   the index local held; and the pre-call read lands in a ring temporary,
 *   which a local can never be (L130, a local is a symbol). Worth 613 -> 13
 *   at size delta 0 on its own. The four builder blocks that were written out
 *   longhand are folded back into their macros as part of it, so every group
 *   is one physical line again and as1's `lineno` tie-break is retired (L59,
 *   L132); on the local-carrier shape that same folding measured 623, ten
 *   words WORSE, which is L146 exactly -- an order or a layout measured
 *   before a shape change says nothing after it.
 * L146, twice more, and both directions were already written down here as
 *   settled. The first sprite node wants x BEFORE y: the comment this one
 *   replaces recorded the opposite, measured on the carrier shape, and it was
 *   right there and wrong here. 13 -> 9. The group header wants `mode` FIRST,
 *   ahead of the geometry stores, where it used to sit seventh; sweeping it
 *   across the header reads 0, 0, 6, 7, 9, 9, 9, 16, 21, 26, 31, 31 from
 *   first to last, and only groups 2, 4 and 5 were mis-scheduled at the old
 *   position. 9 -> 0.
 *
 * Tier B identities are unchanged from the reconstruction: the runtime table
 * resolves ten sprite loads to texLoadSprite, four screen loads to
 * func_80036DD0, seven local calls to overlay101ByteLength and the tail call
 * to overlay101Reset; ROM-exact overlay101Cleanup and overlay101DrawSlots
 * witness the order records, the order list and count, and the two resource
 * arrays with their counters. 963 of 963 words byte-exact, frame 0x50,
 * 110 of 110 relocations aligned.
 */
void func_overlay_101_F00069E8_18E2208(void) {
    register s32 commonY;
    s32 length;
    void *handle;

#define INIT_GROUP(group, px, pwidth, py, pheight, value0, value1, input, owner) \
    D_0.el[1 + group].m16.mode = 0; \
    D_0.el[1 + group].m16.x = (px); \
    D_0.el[1 + group].m16.width = (pwidth); \
    D_0.el[1 + group].m16.y = (py); \
    D_0.el[1 + group].m16.height = (pheight); \
    D_0.el[1 + group].m16.value12 = (value0); \
    D_0.el[1 + group].m16.value14 = (value1); \
    D_0.el[1 + group].m16.color16 = 0xFF; \
    D_0.el[1 + group].m16.color17 = 0xFF; \
    D_0.el[1 + group].m16.childType = 0; \
    D_0.el[1 + group].m16.child = 0; \
    D_0.el[1 + group].m16.data18 = D_INPUT.input; \
    gOverlay101Order[o101OrderCounter_o101Reloc].m2 = &(owner); \
    o101OrderCounter_o101Reloc = o101OrderCounter_o101Reloc + 1

#define ADD_NODE32(group, nodeX, nodeY, nodeScale, nodeValue, nodeColor, imageId) \
    D_340[o101Node32Counter_o101Reloc].m2.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m2.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m2.scale = (nodeScale); \
    D_340[o101Node32Counter_o101Reloc].m2.value10 = (nodeValue); \
    D_340[o101Node32Counter_o101Reloc].m2.color12 = (nodeColor); \
    D_340[o101Node32Counter_o101Reloc].m2.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m2.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m2.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m2.previousType = D_0.el[1 + group].m16.childType; \
    D_340[o101Node32Counter_o101Reloc].m2.previous = D_0.el[1 + group].m16.child; \
    D_340[o101Node32Counter_o101Reloc].m2.handle = handle; \
    D_0.el[1 + group].m16.childType = 2; \
    D_0.el[1 + group].m16.child = &D_340[o101Node32Counter_o101Reloc].m2; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

#define ADD_NODE20(group, nodeX, nodeY, imageId) \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.x = (nodeX); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.y = (nodeY); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.scale = 1.0f; \
    handle = func_80036DD0(imageId); \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.previousType = D_0.el[1 + group].m16.childType; \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.previous = D_0.el[1 + group].m16.child; \
    gOverlay101Resources20[o101Node20Counter_o101Reloc].m1.handle = handle; \
    D_0.el[1 + group].m16.childType = 1; \
    D_0.el[1 + group].m16.child = &gOverlay101Resources20[o101Node20Counter_o101Reloc].m1; \
    o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1

#define ADD_TEXT(group, input, textX, textY) \
    D_540[D_1D0].m2.x = (textX); \
    D_540[D_1D0].m2.y = (textY); \
    length = overlay101ByteLength(D_INPUT.input); \
    D_540[D_1D0].m2.length = (u8)length; \
    D_540[D_1D0].m2.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m2.mode = 2; \
    D_540[D_1D0].m2.color0 = 0xFF; \
    D_540[D_1D0].m2.color1 = 0xFF; \
    D_540[D_1D0].m2.color2 = 0xFF; \
    D_540[D_1D0].m2.color3 = 0xFF; \
    D_540[D_1D0].m2.kind = 4; \
    D_540[D_1D0].m2.text = D_INPUT.input; \
    D_540[D_1D0].m2.previousType = D_0.el[1 + group].m16.childType; \
    D_540[D_1D0].m2.previous = D_0.el[1 + group].m16.child; \
    D_0.el[1 + group].m16.childType = 3; \
    D_0.el[1 + group].m16.child = &D_540[D_1D0].m2; \
    D_1D0 = D_1D0 + 1

#define INIT_GROUP0() \
    D_0.el[1 + 0].m16.mode = 4; \
    D_0.el[1 + 0].m16.value12 = 0x140; \
    D_0.el[1 + 0].m16.value14 = 0xF0; \
    D_0.el[1 + 0].m16.data18 = &D_CC8; \
    D_0.el[1 + 0].m16.color16 = 0xFF; \
    D_0.el[1 + 0].m16.color17 = 0xFF; \
    D_0.el[1 + 0].m16.x = 0; \
    D_0.el[1 + 0].m16.width = 0; \
    D_0.el[1 + 0].m16.y = 0; \
    D_0.el[1 + 0].m16.height = 0; \
    D_0.el[1 + 0].m16.childType = 0; \
    D_0.el[1 + 0].m16.child = 0; \
    gOverlay101Order[o101OrderCounter_o101Reloc].m2 = &D_1C; \
    o101OrderCounter_o101Reloc = o101OrderCounter_o101Reloc + 1

    INIT_GROUP0();
    ADD_NODE32(0, 0xF2, 0x14E, 1.0f, 0, 0xFF, 0x91);
    commonY = 0x78;
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x94);

    INIT_GROUP(1, 0x20, 0x18, 0x30, 0x10, 0xE0, 0xC0, dataC8, D_38);
    ADD_NODE20(1, 0xC, 0xE, 0xD);
    ADD_TEXT(1, textCC, 0x70, 0xA8);
    ADD_TEXT(1, textD0, 0x70, 0xB2);

    INIT_GROUP(2, 0x20, 0x40, 0x62, 0x18, 0x7C, 0x84, dataD4, D_54);
    ADD_NODE20(2, 0x16, 0x1E, 0x10);

    INIT_GROUP(3, 0x20, 0x68, 0x40, 0xA0, 0xC0, 0x26, dataD8, D_0.el[4].m16);
    ADD_TEXT(3, textDC, 0x60, 0xE);
    ADD_TEXT(3, textE0, 0x60, 0x18);

    INIT_GROUP(4, 0x20, 0x90, 0x2F, 0x10, 0xE2, 0xC0, dataE4, D_8C);
    ADD_NODE20(4, 0xD, 0xE, 0x14);
    ADD_TEXT(4, textE8, 0x71, 0xA8);
    ADD_TEXT(4, textEC, 0x71, 0xB2);

    INIT_GROUP(5, 0x90, -0x50, 0x30, 0x30, 0xE0, 0x8C, dataF0, D_A8);
    ADD_NODE20(5, 0xC, 0x12, 0xC);
    ADD_TEXT(5, textF4, 0x70, 0x7B);

    overlay101Reset(&D_1D2C);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
#undef INIT_GROUP0
}
#undef D_1C
#undef D_38
#undef D_54
#undef D_8C
#undef D_A8
#undef D_INPUT

/* ---- func_overlay_101_F00078F4_18E3114 (+0x78F4) ---- */
#define D_0 D_0.m3
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_INPUT D_INPUT.m6

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C
 * TU; this consumer uses that owner for the root and node pools. */
/* Matched by deleting every local that held an element address or an element
 * index, and spelling all six store groups -- the chain node, the eight image
 * nodes and the three text rows -- through the array subscript of the counter
 * global itself.
 *
 * A uopt live range is formed per IR name (L131), so `&D_340[D_1CC].m1` written
 * on both sides of a call is ONE range whose hull spans the call, and a
 * pointer or index LOCAL is a symbol, which is coloured from the procedure's
 * table or given a stack home and is never a ring temporary.  That is what
 * held the earlier shapes apart from the shipped code in two ways at once.
 * Writing `D_340[D_1CC].m1.field` instead gives, per element:
 *   - one range for the address expression, pre-call and post-call, so both
 *     occurrences take the one caller-saved colour the shipped code uses;
 *   - one range for the counter load, spanning all three rows, which is what
 *     takes the callee-saved register the earlier shapes gave to the address;
 *   - and the pre-call read of the counter lands in a ring temporary instead
 *     of a coloured symbol, which is the ring draw (L129) the index-local
 *     shapes were missing -- they ran a FIFO position behind from the head of
 *     the text region onward.
 * Measured on this function: index locals everywhere 179, one row reverted to
 * the direct read 155, all rows direct but the post-call stores still through
 * a shared index local 165, no locals at all 98.
 *
 * The remaining 98 were two statement orders, both of which had to be
 * re-climbed because a statement-order optimum is not portable across a shape
 * change (L59: each group is one physical line, so as1's `lineno` tie is
 * retired and the raw ready-list order supplies the reversal):
 *   - the image node's post-call group wants the handle store FIRST, ahead of
 *     the two old-link reads: 98 -> 26.  The earlier 630-order exhaustion was
 *     an optimum of the pointer-local shape and does not survive this one.
 *   - the second root header's nine geometry stores want width, height, mode,
 *     x, colour, colour, y, then the two values: 26 -> 0.
 *
 * All four siblings in this cluster take the identical edit at 0 words. */
void func_overlay_101_F00078F4_18E3114(void) {
    void *handle;
    s32 length;
    s32 ownerIndex;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_CDC; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_1C; D_1C4 = ownerIndex + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0x4E; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x92, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; D_0.chainType = 2; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.width44 = 0x18; D_0.height48 = 0x14; D_0.mode40 = 0; D_0.x42 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.y46 = 0x20; D_0.value4A = 0x100; D_0.value4C = 0xB4; D_0.data50 = D_INPUT.dataF8; D_0.childType = 0; D_0.child = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_38; D_1C4 = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId) \
    D_340[o101Node32Counter_o101Reloc].m1.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m1.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; \
    D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m1.handle = handle; \
    D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.childType; \
    D_0.childType = 2; \
    D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.child; \
    D_0.child = &D_340[o101Node32Counter_o101Reloc].m1; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

    if (((D_800D3128 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x98);
    }
    if (((D_800D3128 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x9A);
    }
    if (((D_800D3128 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9E);
    }
    if (((D_800D3128 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY) \
    D_540[D_1D0].m1.x = 0x80; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = (field); \
    D_540[D_1D0].m1.previousType = D_0.childType; \
    D_540[D_1D0].m1.previous = D_0.child; \
    D_0.childType = 3; \
    D_0.child = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.textFC, 0x92);
    ADD_TEXT_ROW(D_INPUT.text100, 0x9C);
    ADD_TEXT_ROW(D_INPUT.text104, 0xA6);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_2608);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_INPUT

/* ---- func_overlay_101_F0008128_18E3948 (+0x8128) ---- */
#define D_0 D_0.m4
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_INPUT D_INPUT.m7

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C
 * TU; this consumer uses that owner for the root and node pools. */
/* Matched by deleting every local that held an element address or an element
 * index, and spelling all six store groups -- the chain node, the eight image
 * nodes and the three text rows -- through the array subscript of the counter
 * global itself.
 *
 * A uopt live range is formed per IR name (L131), so `&D_340[D_1CC].m1` written
 * on both sides of a call is ONE range whose hull spans the call, and a
 * pointer or index LOCAL is a symbol, which is coloured from the procedure's
 * table or given a stack home and is never a ring temporary.  That is what
 * held the earlier shapes apart from the shipped code in two ways at once.
 * Writing `D_340[D_1CC].m1.field` instead gives, per element:
 *   - one range for the address expression, pre-call and post-call, so both
 *     occurrences take the one caller-saved colour the shipped code uses;
 *   - one range for the counter load, spanning all three rows, which is what
 *     takes the callee-saved register the earlier shapes gave to the address;
 *   - and the pre-call read of the counter lands in a ring temporary instead
 *     of a coloured symbol, which is the ring draw (L129) the index-local
 *     shapes were missing -- they ran a FIFO position behind from the head of
 *     the text region onward.
 * Measured on this function: index locals everywhere 179, one row reverted to
 * the direct read 155, all rows direct but the post-call stores still through
 * a shared index local 165, no locals at all 98.
 *
 * The remaining 98 were two statement orders, both of which had to be
 * re-climbed because a statement-order optimum is not portable across a shape
 * change (L59: each group is one physical line, so as1's `lineno` tie is
 * retired and the raw ready-list order supplies the reversal):
 *   - the image node's post-call group wants the handle store FIRST, ahead of
 *     the two old-link reads: 98 -> 26.  The earlier 630-order exhaustion was
 *     an optimum of the pointer-local shape and does not survive this one.
 *   - the second root header's nine geometry stores want width, height, mode,
 *     x, colour, colour, y, then the two values: 26 -> 0.
 *
 * All four siblings in this cluster take the identical edit at 0 words. */
void func_overlay_101_F0008128_18E3948(void) {
    void *handle;
    s32 length;
    s32 ownerIndex;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_CF0; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_1C; D_1C4 = ownerIndex + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x93, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; D_0.chainType = 2; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.width44 = 0x18; D_0.height48 = 0x14; D_0.mode40 = 0; D_0.x42 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.y46 = 0x20; D_0.value4A = 0x100; D_0.value4C = 0xB4; D_0.data50 = D_INPUT.dataF8; D_0.childType = 0; D_0.child = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_38; D_1C4 = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId) \
    D_340[o101Node32Counter_o101Reloc].m1.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m1.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; \
    D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m1.handle = handle; \
    D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.childType; \
    D_0.childType = 2; \
    D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.child; \
    D_0.child = &D_340[o101Node32Counter_o101Reloc].m1; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

    if (((D_800D3128 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x26, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x26, 0x2E, 0x98);
    }
    if (((D_800D3128 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0x68, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0x68, 0x2E, 0x9A);
    }
    if (((D_800D3128 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x26, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x26, 0x70, 0x9E);
    }
    if (((D_800D3128 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0x68, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0x68, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY) \
    D_540[D_1D0].m1.x = 0x80; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = (field); \
    D_540[D_1D0].m1.previousType = D_0.childType; \
    D_540[D_1D0].m1.previous = D_0.child; \
    D_0.childType = 3; \
    D_0.child = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.text108, 0x92);
    ADD_TEXT_ROW(D_INPUT.text10C, 0x9C);
    ADD_TEXT_ROW(D_INPUT.text110, 0xA6);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_2934);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_INPUT

/* ---- func_overlay_101_F000895C_18E417C (+0x895C) ---- */
#define D_0 D_0.m5
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_INPUT D_INPUT.m8

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C
 * TU; this consumer uses that owner for the root and node pools. */
/* Matched by deleting every local that held an element address or an element
 * index, and spelling all six store groups -- the chain node, the eight image
 * nodes and the three text rows -- through the array subscript of the counter
 * global itself.
 *
 * A uopt live range is formed per IR name (L131), so `&D_340[D_1CC].m1` written
 * on both sides of a call is ONE range whose hull spans the call, and a
 * pointer or index LOCAL is a symbol, which is coloured from the procedure's
 * table or given a stack home and is never a ring temporary.  That is what
 * held the earlier shapes apart from the shipped code in two ways at once.
 * Writing `D_340[D_1CC].m1.field` instead gives, per element:
 *   - one range for the address expression, pre-call and post-call, so both
 *     occurrences take the one caller-saved colour the shipped code uses;
 *   - one range for the counter load, spanning all three rows, which is what
 *     takes the callee-saved register the earlier shapes gave to the address;
 *   - and the pre-call read of the counter lands in a ring temporary instead
 *     of a coloured symbol, which is the ring draw (L129) the index-local
 *     shapes were missing -- they ran a FIFO position behind from the head of
 *     the text region onward.
 * Measured on this function: index locals everywhere 179, one row reverted to
 * the direct read 155, all rows direct but the post-call stores still through
 * a shared index local 165, no locals at all 98.
 *
 * The remaining 98 were two statement orders, both of which had to be
 * re-climbed because a statement-order optimum is not portable across a shape
 * change (L59: each group is one physical line, so as1's `lineno` tie is
 * retired and the raw ready-list order supplies the reversal):
 *   - the image node's post-call group wants the handle store FIRST, ahead of
 *     the two old-link reads: 98 -> 26.  The earlier 630-order exhaustion was
 *     an optimum of the pointer-local shape and does not survive this one.
 *   - the second root header's nine geometry stores want width, height, mode,
 *     x, colour, colour, y, then the two values: 26 -> 0.
 *
 * All four siblings in this cluster take the identical edit at 0 words. */
void func_overlay_101_F000895C_18E417C(void) {
    void *handle;
    s32 length;
    s32 ownerIndex;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_D04; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_1C; D_1C4 = ownerIndex + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0x4E; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x92, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; D_0.chainType = 2; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.width44 = 0x18; D_0.height48 = 0x14; D_0.mode40 = 0; D_0.x42 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.y46 = 0x20; D_0.value4A = 0x100; D_0.value4C = 0xB4; D_0.data50 = D_INPUT.dataF8; D_0.childType = 0; D_0.child = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_38; D_1C4 = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId) \
    D_340[o101Node32Counter_o101Reloc].m1.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m1.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; \
    D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m1.handle = handle; \
    D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.childType; \
    D_0.childType = 2; \
    D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.child; \
    D_0.child = &D_340[o101Node32Counter_o101Reloc].m1; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

    if (((D_800D3128 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x98);
    }
    if (((D_800D3128 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x9A);
    }
    if (((D_800D3128 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9E);
    }
    if (((D_800D3128 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY) \
    D_540[D_1D0].m1.x = 0x80; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = (field); \
    D_540[D_1D0].m1.previousType = D_0.childType; \
    D_540[D_1D0].m1.previous = D_0.child; \
    D_0.childType = 3; \
    D_0.child = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.text114, 0x92);
    ADD_TEXT_ROW(D_INPUT.text118, 0x9C);
    ADD_TEXT_ROW(D_INPUT.text11C, 0xA6);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_2C60);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_INPUT

/* ---- func_overlay_101_F0009190_18E49B0 (+0x9190) ---- */
#define D_0 D_0.m6
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_INPUT D_INPUT.m9

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C
 * TU; this consumer uses that owner for the root and node pools. */
/* Matched by deleting every local that held an element address or an element
 * index, and spelling all six store groups -- the chain node, the eight image
 * nodes and the three text rows -- through the array subscript of the counter
 * global itself.
 *
 * A uopt live range is formed per IR name (L131), so `&D_340[D_1CC].m1` written
 * on both sides of a call is ONE range whose hull spans the call, and a
 * pointer or index LOCAL is a symbol, which is coloured from the procedure's
 * table or given a stack home and is never a ring temporary.  That is what
 * held the earlier shapes apart from the shipped code in two ways at once.
 * Writing `D_340[D_1CC].m1.field` instead gives, per element:
 *   - one range for the address expression, pre-call and post-call, so both
 *     occurrences take the one caller-saved colour the shipped code uses;
 *   - one range for the counter load, spanning all three rows, which is what
 *     takes the callee-saved register the earlier shapes gave to the address;
 *   - and the pre-call read of the counter lands in a ring temporary instead
 *     of a coloured symbol, which is the ring draw (L129) the index-local
 *     shapes were missing -- they ran a FIFO position behind from the head of
 *     the text region onward.
 * Measured on this function: index locals everywhere 179, one row reverted to
 * the direct read 155, all rows direct but the post-call stores still through
 * a shared index local 165, no locals at all 98.
 *
 * The remaining 98 were two statement orders, both of which had to be
 * re-climbed because a statement-order optimum is not portable across a shape
 * change (L59: each group is one physical line, so as1's `lineno` tie is
 * retired and the raw ready-list order supplies the reversal):
 *   - the image node's post-call group wants the handle store FIRST, ahead of
 *     the two old-link reads: 98 -> 26.  The earlier 630-order exhaustion was
 *     an optimum of the pointer-local shape and does not survive this one.
 *   - the second root header's nine geometry stores want width, height, mode,
 *     x, colour, colour, y, then the two values: 26 -> 0.
 *
 * All four siblings in this cluster take the identical edit at 0 words. */
void func_overlay_101_F0009190_18E49B0(void) {
    void *handle;
    s32 length;
    s32 ownerIndex;

    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &D_D18; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_1C; D_1C4 = ownerIndex + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m1.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x93, 0); D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; D_0.chainType = 2; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.width44 = 0x18; D_0.height48 = 0x14; D_0.mode40 = 0; D_0.x42 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.y46 = 0x20; D_0.value4A = 0x100; D_0.value4C = 0xB4; D_0.data50 = D_INPUT.dataF8; D_0.childType = 0; D_0.child = 0; ownerIndex = D_1C4; D_1C0[ownerIndex] = &D_38; D_1C4 = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId) \
    D_340[o101Node32Counter_o101Reloc].m1.x = (nodeX); \
    D_340[o101Node32Counter_o101Reloc].m1.y = (nodeY); \
    D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; \
    D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; \
    D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; \
    D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; \
    handle = texLoadSprite((imageId), 0); \
    D_340[o101Node32Counter_o101Reloc].m1.handle = handle; \
    D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.childType; \
    D_0.childType = 2; \
    D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.child; \
    D_0.child = &D_340[o101Node32Counter_o101Reloc].m1; \
    o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1

    if (((D_800D3128 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x26, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x26, 0x2E, 0x98);
    }
    if (((D_800D3128 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0x68, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0x68, 0x2E, 0x9A);
    }
    if (((D_800D3128 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x26, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x26, 0x70, 0x9E);
    }
    if (((D_800D3128 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0x68, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0x68, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY) \
    D_540[D_1D0].m1.x = 0x80; \
    D_540[D_1D0].m1.y = (rowY); \
    length = overlay101ByteLength(field); \
    D_540[D_1D0].m1.length = (u8)length; \
    D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); \
    D_540[D_1D0].m1.mode = 2; \
    D_540[D_1D0].m1.color0 = 0; \
    D_540[D_1D0].m1.color1 = 0; \
    D_540[D_1D0].m1.color2 = 0; \
    D_540[D_1D0].m1.color3 = 0; \
    D_540[D_1D0].m1.kind = 4; \
    D_540[D_1D0].m1.text = (field); \
    D_540[D_1D0].m1.previousType = D_0.childType; \
    D_540[D_1D0].m1.previous = D_0.child; \
    D_0.childType = 3; \
    D_0.child = &D_540[D_1D0].m1; \
    D_1D0 = D_1D0 + 1

    ADD_TEXT_ROW(D_INPUT.text120, 0x92);
    ADD_TEXT_ROW(D_INPUT.text124, 0x9C);
    ADD_TEXT_ROW(D_INPUT.text128, 0xA6);

#undef ADD_TEXT_ROW

    overlay101Reset(&D_2F8C);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_INPUT

/* ---- overlay101BuildPresentationA (+0x99C4) ---- */
#define D_0 D_0.m7

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */

/* Matched 2026-10-02 (lane x-o101), from 136 masked words (130 on D) at
 * delta 0, by discarding the inherited shape after decoding the relocation
 * table and writing the body in the matched tail builders' form: one symbol
 * per object (the A/B alias pairs were each one address), one-name counter
 * subscripts with each creator result stored straight into its node, the
 * calls at their real arities (the "dim colour in a2" and the four-argument
 * closing call were the leftover a1-a3 of a one-argument overlay101Reset),
 * and the opacity read back from the stored length. The four presentation
 * builders are one function with different constants. The last two orders
 * were the second root header's data, childType, child and the node-24
 * childType before child. */
void overlay101BuildPresentationA(void) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101PresAAssetD2C; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101PresAOrderSlots[gO101PresAOrderCount] = &D_0.chainType; gO101PresAOrderCount = gO101PresAOrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m3.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m3.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m3.value10 = 0; D_340[o101Node32Counter_o101Reloc].m3.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m3.color13 = 0; D_340[o101Node32Counter_o101Reloc].m3.value18 = 0; D_340[o101Node32Counter_o101Reloc].m3.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m3.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m3.handle = texLoadSprite(0x91, NULL); D_340[o101Node32Counter_o101Reloc].m3.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m3.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m3; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x50; D_0.value4A = 0xA0; D_0.value4C = 0xAE; D_0.width44 = 0x18; D_0.height48 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = gO101PresAInputs.data12C; D_0.childType = 0; D_0.child = NULL; gO101PresAOrderSlots[gO101PresAOrderCount] = &D_0.childType; gO101PresAOrderCount = gO101PresAOrderCount + 1;

    gO101PresANodes20[o101Node20Counter_o101Reloc].x = 0x10; gO101PresANodes20[o101Node20Counter_o101Reloc].y = 0x16; gO101PresANodes20[o101Node20Counter_o101Reloc].scale = 1.0f; gO101PresANodes20[o101Node20Counter_o101Reloc].handle = func_80036DD0(0x15, &gO101PresANodes20[o101Node20Counter_o101Reloc]); gO101PresANodes20[o101Node20Counter_o101Reloc].previousType = D_0.childType; gO101PresANodes20[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &gO101PresANodes20[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_540[gO101PresANode24Count].m3.x = 0x50; D_540[gO101PresANode24Count].m3.y = 0x9C; D_540[gO101PresANode24Count].m3.length = overlay101ByteLength(gO101PresAInputs.text130); D_540[gO101PresANode24Count].m3.opacity = (s8)(s32)((f32)(u32)D_540[gO101PresANode24Count].m3.length * (f32)(s32)1); D_540[gO101PresANode24Count].m3.mode = 2; D_540[gO101PresANode24Count].m3.color0 = 0xFF; D_540[gO101PresANode24Count].m3.color1 = 0xC0; D_540[gO101PresANode24Count].m3.color2 = 0xC0; D_540[gO101PresANode24Count].m3.color3 = 0xFF; D_540[gO101PresANode24Count].m3.kind = 4; D_540[gO101PresANode24Count].m3.text = gO101PresAInputs.text130; D_540[gO101PresANode24Count].m3.previousType = D_0.childType; D_540[gO101PresANode24Count].m3.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101PresANode24Count].m3; gO101PresANode24Count = gO101PresANode24Count + 1;

    overlay101Reset(&gO101PresAFinalObject32B8);
}
#undef D_0

/* ---- overlay101BuildPresentationB (+0x9D04) ---- */
#define D_0 D_0.m8

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */

/* Matched 2026-10-02 (lane x-o101), from 136 masked words (130 on D) at
 * delta 0, by discarding the inherited shape after decoding the relocation
 * table and writing the body in the matched tail builders' form: one symbol
 * per object (the A/B alias pairs were each one address), one-name counter
 * subscripts with each creator result stored straight into its node, the
 * calls at their real arities (the "dim colour in a2" and the four-argument
 * closing call were the leftover a1-a3 of a one-argument overlay101Reset),
 * and the opacity read back from the stored length. The four presentation
 * builders are one function with different constants. The last two orders
 * were the second root header's data, childType, child and the node-24
 * childType before child. */
void overlay101BuildPresentationB(void) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101PresBAssetD40; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101PresBOrderSlots[gO101PresBOrderCount] = &D_0.chainType; gO101PresBOrderCount = gO101PresBOrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m4.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m4.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m4.value10 = 0; D_340[o101Node32Counter_o101Reloc].m4.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m4.color13 = 0; D_340[o101Node32Counter_o101Reloc].m4.value18 = 0; D_340[o101Node32Counter_o101Reloc].m4.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m4.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m4.handle = texLoadSprite(0x91, NULL); D_340[o101Node32Counter_o101Reloc].m4.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m4.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m4; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x50; D_0.value4A = 0xA0; D_0.value4C = 0xAE; D_0.width44 = 0x18; D_0.height48 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = gO101PresBInputs.data134; D_0.childType = 0; D_0.child = NULL; gO101PresBOrderSlots[gO101PresBOrderCount] = &D_0.childType; gO101PresBOrderCount = gO101PresBOrderCount + 1;

    gO101PresBNodes20[o101Node20Counter_o101Reloc].x = 0x10; gO101PresBNodes20[o101Node20Counter_o101Reloc].y = 0x16; gO101PresBNodes20[o101Node20Counter_o101Reloc].scale = 1.0f; gO101PresBNodes20[o101Node20Counter_o101Reloc].handle = func_80036DD0(0x16, &gO101PresBNodes20[o101Node20Counter_o101Reloc]); gO101PresBNodes20[o101Node20Counter_o101Reloc].previousType = D_0.childType; gO101PresBNodes20[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &gO101PresBNodes20[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_540[gO101PresBNode24Count].m4.x = 0x50; D_540[gO101PresBNode24Count].m4.y = 0x9C; D_540[gO101PresBNode24Count].m4.length = overlay101ByteLength(gO101PresBInputs.text138); D_540[gO101PresBNode24Count].m4.opacity = (s8)(s32)((f32)(u32)D_540[gO101PresBNode24Count].m4.length * (f32)(s32)1); D_540[gO101PresBNode24Count].m4.mode = 2; D_540[gO101PresBNode24Count].m4.color0 = 0xC0; D_540[gO101PresBNode24Count].m4.color1 = 0xFF; D_540[gO101PresBNode24Count].m4.color2 = 0xC0; D_540[gO101PresBNode24Count].m4.color3 = 0xFF; D_540[gO101PresBNode24Count].m4.kind = 4; D_540[gO101PresBNode24Count].m4.text = gO101PresBInputs.text138; D_540[gO101PresBNode24Count].m4.previousType = D_0.childType; D_540[gO101PresBNode24Count].m4.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101PresBNode24Count].m4; gO101PresBNode24Count = gO101PresBNode24Count + 1;

    overlay101Reset(&gO101PresBFinalObject34E8);
}
#undef D_0

/* ---- overlay101BuildPresentationC (+0xA044) ---- */
#define D_0 D_0.m9

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */

/* Matched 2026-10-02 (lane x-o101), from 136 masked words (130 on D) at
 * delta 0, by discarding the inherited shape after decoding the relocation
 * table and writing the body in the matched tail builders' form: one symbol
 * per object (the A/B alias pairs were each one address), one-name counter
 * subscripts with each creator result stored straight into its node, the
 * calls at their real arities (the "dim colour in a2" and the four-argument
 * closing call were the leftover a1-a3 of a one-argument overlay101Reset),
 * and the opacity read back from the stored length. The four presentation
 * builders are one function with different constants. The last two orders
 * were the second root header's data, childType, child and the node-24
 * childType before child. */
void overlay101BuildPresentationC(void) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101PresCAssetD54; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101PresCOrderSlots[gO101PresCOrderCount] = &D_0.chainType; gO101PresCOrderCount = gO101PresCOrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m5.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m5.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m5.value10 = 0; D_340[o101Node32Counter_o101Reloc].m5.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m5.color13 = 0; D_340[o101Node32Counter_o101Reloc].m5.value18 = 0; D_340[o101Node32Counter_o101Reloc].m5.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m5.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m5.handle = texLoadSprite(0x91, NULL); D_340[o101Node32Counter_o101Reloc].m5.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m5.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m5; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x50; D_0.value4A = 0xA0; D_0.value4C = 0xAE; D_0.width44 = 0x18; D_0.height48 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = gO101PresCInputs.data13C; D_0.childType = 0; D_0.child = NULL; gO101PresCOrderSlots[gO101PresCOrderCount] = &D_0.childType; gO101PresCOrderCount = gO101PresCOrderCount + 1;

    gO101PresCNodes20[o101Node20Counter_o101Reloc].x = 0x10; gO101PresCNodes20[o101Node20Counter_o101Reloc].y = 0x16; gO101PresCNodes20[o101Node20Counter_o101Reloc].scale = 1.0f; gO101PresCNodes20[o101Node20Counter_o101Reloc].handle = func_80036DD0(0x17, &gO101PresCNodes20[o101Node20Counter_o101Reloc]); gO101PresCNodes20[o101Node20Counter_o101Reloc].previousType = D_0.childType; gO101PresCNodes20[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &gO101PresCNodes20[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_540[gO101PresCNode24Count].m5.x = 0x50; D_540[gO101PresCNode24Count].m5.y = 0x9C; D_540[gO101PresCNode24Count].m5.length = overlay101ByteLength(gO101PresCInputs.text140); D_540[gO101PresCNode24Count].m5.opacity = (s8)(s32)((f32)(u32)D_540[gO101PresCNode24Count].m5.length * (f32)(s32)1); D_540[gO101PresCNode24Count].m5.mode = 2; D_540[gO101PresCNode24Count].m5.color0 = 0xC0; D_540[gO101PresCNode24Count].m5.color1 = 0xC0; D_540[gO101PresCNode24Count].m5.color2 = 0xFF; D_540[gO101PresCNode24Count].m5.color3 = 0xFF; D_540[gO101PresCNode24Count].m5.kind = 4; D_540[gO101PresCNode24Count].m5.text = gO101PresCInputs.text140; D_540[gO101PresCNode24Count].m5.previousType = D_0.childType; D_540[gO101PresCNode24Count].m5.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101PresCNode24Count].m5; gO101PresCNode24Count = gO101PresCNode24Count + 1;

    overlay101Reset(&gO101PresCFinalObject3718);
}
#undef D_0

/* ---- overlay101BuildPresentationD (+0xA384) ---- */
#define D_0 D_0.m10

/* Every call but the byte-length one is a SYMBOL relocation record, so
 * each callee is a placeholder. The sprite creator is the resident routine
 * the tail builders (overlay101TailA6BC/AB4C/C6E8) call with two arguments,
 * the compact creator the one overlay101TailC6E8 calls with (key, node),
 * and the closing call is overlay101Reset (overlay 101 +0x1BB4) with one
 * argument; the a1-a3 the shipped calls carry are leftovers. */

/* Matched 2026-10-02 (lane x-o101), from 136 masked words (130 on D) at
 * delta 0, by discarding the inherited shape after decoding the relocation
 * table and writing the body in the matched tail builders' form: one symbol
 * per object (the A/B alias pairs were each one address), one-name counter
 * subscripts with each creator result stored straight into its node, the
 * calls at their real arities (the "dim colour in a2" and the four-argument
 * closing call were the leftover a1-a3 of a one-argument overlay101Reset),
 * and the opacity read back from the stored length. The four presentation
 * builders are one function with different constants. The last two orders
 * were the second root header's data, childType, child and the node-24
 * childType before child. */
void overlay101BuildPresentationD(void) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101PresDAssetD68; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101PresDOrderSlots[gO101PresDOrderCount] = &D_0.chainType; gO101PresDOrderCount = gO101PresDOrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m6.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m6.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m6.value10 = 0; D_340[o101Node32Counter_o101Reloc].m6.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m6.color13 = 0; D_340[o101Node32Counter_o101Reloc].m6.value18 = 0; D_340[o101Node32Counter_o101Reloc].m6.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m6.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m6.handle = texLoadSprite(0x91, NULL); D_340[o101Node32Counter_o101Reloc].m6.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m6.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m6; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x50; D_0.value4A = 0xA0; D_0.value4C = 0xAE; D_0.width44 = 0x18; D_0.height48 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.data50 = gO101PresDInputs.data144; D_0.childType = 0; D_0.child = NULL; gO101PresDOrderSlots[gO101PresDOrderCount] = &D_0.childType; gO101PresDOrderCount = gO101PresDOrderCount + 1;

    gO101PresDNodes20[o101Node20Counter_o101Reloc].x = 0x10; gO101PresDNodes20[o101Node20Counter_o101Reloc].y = 0x16; gO101PresDNodes20[o101Node20Counter_o101Reloc].scale = 1.0f; gO101PresDNodes20[o101Node20Counter_o101Reloc].handle = func_80036DD0(0x18, &gO101PresDNodes20[o101Node20Counter_o101Reloc]); gO101PresDNodes20[o101Node20Counter_o101Reloc].previousType = D_0.childType; gO101PresDNodes20[o101Node20Counter_o101Reloc].previous = D_0.child; D_0.childType = 1; D_0.child = &gO101PresDNodes20[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

    D_540[gO101PresDNode24Count].m6.x = 0x50; D_540[gO101PresDNode24Count].m6.y = 0x9C; D_540[gO101PresDNode24Count].m6.length = overlay101ByteLength(gO101PresDInputs.text148); D_540[gO101PresDNode24Count].m6.opacity = (s8)(s32)((f32)(u32)D_540[gO101PresDNode24Count].m6.length * (f32)(s32)1); D_540[gO101PresDNode24Count].m6.mode = 2; D_540[gO101PresDNode24Count].m6.color0 = 0xFF; D_540[gO101PresDNode24Count].m6.color1 = 0xFF; D_540[gO101PresDNode24Count].m6.color2 = 0; D_540[gO101PresDNode24Count].m6.color3 = 0xFF; D_540[gO101PresDNode24Count].m6.kind = 4; D_540[gO101PresDNode24Count].m6.text = gO101PresDInputs.text148; D_540[gO101PresDNode24Count].m6.previousType = D_0.childType; D_540[gO101PresDNode24Count].m6.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101PresDNode24Count].m6; gO101PresDNode24Count = gO101PresDNode24Count + 1;

    overlay101Reset(&gO101PresDFinalObject3948);
}
#undef D_0

/* ---- overlay101TailA6BC (+0xA6BC) ---- */
#define D_0 D_0.m11
/* All three sprite nodes are created by one resident routine (one SYMBOL
 * target in the relocation table, the same one overlay101TailAB4C calls),
 * and it takes two arguments: the a2 the shipped calls carry is the node
 * count's leftover register, not an argument. The finalizer is a
 * same-module SYMBOL record, so it is a placeholder too. */

/* Matched 2026-10-02 (lane x-o101), 53 -> 0 at delta 0, by copying the
 * matched sibling overlay101TailAB4C's shape: the creator called with its
 * real two arguments and the function taking only `base` (53 -> 8; the old
 * third argument and the dead second parameter were the leftover a2), the
 * root header stored kind, width2E, height30 (8 -> 2), and no handle or
 * length local: the creator result is stored straight into the node and the
 * opacity reads the stored length back (2 -> 0, the frame 0x40 -> 0x38, since
 * each declared local had held a frame cell above the base + 0xA2 spill). */
void overlay101TailA6BC(s32 base) {
    D_0.kind = 4; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101TailAssetD7C; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailOrderSlots[gO101TailOrderCount] = &D_0.chainType; gO101TailOrderCount = gO101TailOrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m7.x = 0xA0; D_340[o101Node32Counter_o101Reloc].m7.y = 0x78; D_340[o101Node32Counter_o101Reloc].m7.value10 = 0xE38; D_340[o101Node32Counter_o101Reloc].m7.color12 = 0; D_340[o101Node32Counter_o101Reloc].m7.color13 = 0; D_340[o101Node32Counter_o101Reloc].m7.value18 = 0; D_340[o101Node32Counter_o101Reloc].m7.scale = 5.0f; D_340[o101Node32Counter_o101Reloc].m7.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m7.handle = texLoadSprite(base + 0xA2, NULL); D_340[o101Node32Counter_o101Reloc].m7.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m7.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m7; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_340[o101Node32Counter_o101Reloc].m7.x = 0xA0; D_340[o101Node32Counter_o101Reloc].m7.y = 0x78; D_340[o101Node32Counter_o101Reloc].m7.value10 = 0x71C; D_340[o101Node32Counter_o101Reloc].m7.color12 = 0; D_340[o101Node32Counter_o101Reloc].m7.color13 = 0; D_340[o101Node32Counter_o101Reloc].m7.value18 = 0; D_340[o101Node32Counter_o101Reloc].m7.scale = 5.0f; D_340[o101Node32Counter_o101Reloc].m7.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m7.handle = texLoadSprite(base + 0xA2, NULL); D_340[o101Node32Counter_o101Reloc].m7.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m7.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m7; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_340[o101Node32Counter_o101Reloc].m7.x = 0xA0; D_340[o101Node32Counter_o101Reloc].m7.y = 0x78; D_340[o101Node32Counter_o101Reloc].m7.value10 = 0; D_340[o101Node32Counter_o101Reloc].m7.color12 = 0; D_340[o101Node32Counter_o101Reloc].m7.color13 = 0; D_340[o101Node32Counter_o101Reloc].m7.value18 = 0; D_340[o101Node32Counter_o101Reloc].m7.scale = 5.0f; D_340[o101Node32Counter_o101Reloc].m7.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m7.handle = texLoadSprite(base + 0xA2, NULL); D_340[o101Node32Counter_o101Reloc].m7.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m7.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m7; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.y46 = 0x3C; D_0.height48 = 0x50; D_0.value4A = 0xC8; D_0.value4C = 0x2A; D_0.width44 = 0x18; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailInputs.data14C; gO101TailOrderSlots[gO101TailOrderCount] = &D_0.childType; gO101TailOrderCount = gO101TailOrderCount + 1;

    D_540[gO101TailNode24Count].m7.x = 0x64; D_540[gO101TailNode24Count].m7.y = 0x10; D_540[gO101TailNode24Count].m7.length = overlay101ByteLength(gO101TailInputs.text150); D_540[gO101TailNode24Count].m7.opacity = (s8)(s32)((f32)(u32)D_540[gO101TailNode24Count].m7.length * (f32)(s32)1); D_540[gO101TailNode24Count].m7.mode = 2; D_540[gO101TailNode24Count].m7.color0 = 0xFF; D_540[gO101TailNode24Count].m7.color1 = 0xFF; D_540[gO101TailNode24Count].m7.color2 = 0xFF; D_540[gO101TailNode24Count].m7.color3 = 0xFF; D_540[gO101TailNode24Count].m7.kind = 4; D_540[gO101TailNode24Count].m7.text = gO101TailInputs.text150; D_540[gO101TailNode24Count].m7.previousType = D_0.childType; D_540[gO101TailNode24Count].m7.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailNode24Count].m7; gO101TailNode24Count = gO101TailNode24Count + 1;

    D_540[gO101TailNode24Count].m7.x = 0x64; D_540[gO101TailNode24Count].m7.y = 0x1A; D_540[gO101TailNode24Count].m7.length = overlay101ByteLength(gO101TailInputs.text154); D_540[gO101TailNode24Count].m7.opacity = (s8)(s32)((f32)(u32)D_540[gO101TailNode24Count].m7.length * (f32)(s32)1); D_540[gO101TailNode24Count].m7.mode = 2; D_540[gO101TailNode24Count].m7.color0 = 0xFF; D_540[gO101TailNode24Count].m7.color1 = 0xFF; D_540[gO101TailNode24Count].m7.color2 = 0xFF; D_540[gO101TailNode24Count].m7.color3 = 0xFF; D_540[gO101TailNode24Count].m7.kind = 4; D_540[gO101TailNode24Count].m7.text = gO101TailInputs.text154; D_540[gO101TailNode24Count].m7.previousType = D_0.childType; D_540[gO101TailNode24Count].m7.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailNode24Count].m7; gO101TailNode24Count = gO101TailNode24Count + 1;

    overlay101Reset(&gO101TailFinalObject3B78);
}
#undef D_0

/* ---- overlay101TailAB4C (+0xAB4C) ---- */
#define D_0 D_0.m12

/* All three sprite nodes are created by the same resident routine (one
 * SYMBOL target in the relocation table), and it takes two arguments: the
 * third register the shipped calls carry is a leftover, not an argument. */

/* Matched 2026-10-02 (lane q-ovl10), 499 -> 0 at delta 0. Over the
 * one-name counter-subscript shape of lanes p10/w13 (L145/L131), three
 * edits: the creator called with its real two arguments (499 -> 100, the
 * old three-argument image was a leftover a2), value14 = 0.0f on all three
 * sprite nodes as ONE pool web with only the scales split by spelling
 * (L103; 100 -> 13), and the line order kind before width2E in the root
 * header and value14 before value18 in nodes 2 and 3 (13 -> 0). */
void func_overlay_101_F000AB4C_18E636C(void) {
    void *handle;
    u8 length;

    D_0.height30 = 0xF0; D_0.kind = 4; D_0.width2E = 0x140; D_0.asset34 = &gO101TailAB4CAssetD90; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &D_0.chainType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m8.x = 0xF0; D_340[o101Node32Counter_o101Reloc].m8.y = 0x154; D_340[o101Node32Counter_o101Reloc].m8.value10 = 0; D_340[o101Node32Counter_o101Reloc].m8.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m8.color13 = 0; D_340[o101Node32Counter_o101Reloc].m8.value18 = 0; D_340[o101Node32Counter_o101Reloc].m8.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m8.value14 = 0.0f; handle = texLoadSprite(0x91, NULL); D_340[o101Node32Counter_o101Reloc].m8.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m8.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m8.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m8; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_340[o101Node32Counter_o101Reloc].m8.x = 0x8C; D_340[o101Node32Counter_o101Reloc].m8.y = 0x88; D_340[o101Node32Counter_o101Reloc].m8.value10 = 0; D_340[o101Node32Counter_o101Reloc].m8.color12 = 0; D_340[o101Node32Counter_o101Reloc].m8.color13 = 0; D_340[o101Node32Counter_o101Reloc].m8.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m8.value18 = 0; D_340[o101Node32Counter_o101Reloc].m8.scale = 0.00f; handle = texLoadSprite(0xA8, NULL); D_340[o101Node32Counter_o101Reloc].m8.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m8.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m8.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m8; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_340[o101Node32Counter_o101Reloc].m8.x = 0xB4; D_340[o101Node32Counter_o101Reloc].m8.y = 0x88; D_340[o101Node32Counter_o101Reloc].m8.value10 = 0; D_340[o101Node32Counter_o101Reloc].m8.color12 = 0; D_340[o101Node32Counter_o101Reloc].m8.color13 = 0; D_340[o101Node32Counter_o101Reloc].m8.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m8.value18 = 0; D_340[o101Node32Counter_o101Reloc].m8.scale = 0.000f; handle = texLoadSprite(0xA8, NULL); D_340[o101Node32Counter_o101Reloc].m8.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m8.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m8.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m8; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x28; D_0.height48 = 0x18; D_0.value4A = 0xF0; D_0.value4C = 0xAA; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailAB4CInputs.data158; gO101TailAB4COrderSlots[gO101TailAB4COrderCount] = &D_0.childType; gO101TailAB4COrderCount = gO101TailAB4COrderCount + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x10; length = overlay101ByteLength(gO101TailAB4CInputs.text15C); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0x80; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text15C; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x1E; length = overlay101ByteLength(gO101TailAB4CInputs.text160); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text160; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x28; length = overlay101ByteLength(gO101TailAB4CInputs.text164); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text164; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x36; length = overlay101ByteLength(gO101TailAB4CInputs.text168); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text168; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x40; length = overlay101ByteLength(gO101TailAB4CInputs.text16C); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text16C; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x4A; length = overlay101ByteLength(gO101TailAB4CInputs.text170); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text170; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x54; length = overlay101ByteLength(gO101TailAB4CInputs.text174); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text174; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x84; length = overlay101ByteLength(gO101TailAB4CInputs.text178); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text178; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x8E; length = overlay101ByteLength(gO101TailAB4CInputs.text17C); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text17C; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    D_540[gO101TailAB4CNode24Count].m8.x = 0x78; D_540[gO101TailAB4CNode24Count].m8.y = 0x98; length = overlay101ByteLength(gO101TailAB4CInputs.text180); D_540[gO101TailAB4CNode24Count].m8.length = (u8)length; D_540[gO101TailAB4CNode24Count].m8.opacity = (s8)(s32)((f32)(u32)length * (f32)(s32)1); D_540[gO101TailAB4CNode24Count].m8.mode = 2; D_540[gO101TailAB4CNode24Count].m8.color0 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color1 = 0xFF; D_540[gO101TailAB4CNode24Count].m8.color2 = 0; D_540[gO101TailAB4CNode24Count].m8.color3 = 0; D_540[gO101TailAB4CNode24Count].m8.kind = 4; D_540[gO101TailAB4CNode24Count].m8.text = gO101TailAB4CInputs.text180; D_540[gO101TailAB4CNode24Count].m8.previousType = D_0.childType; D_540[gO101TailAB4CNode24Count].m8.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[gO101TailAB4CNode24Count].m8; gO101TailAB4CNode24Count = gO101TailAB4CNode24Count + 1;

    overlay101Reset(&gO101TailAB4CFinalObject3F14);
}
#undef D_0

/* ---- overlay101TailB544 (+0xB544) ---- */
#define D_0 D_0.m13
#define D_1C D_1C.m1
#define D_38 D_38.m1
#define D_INPUT D_INPUT.m10

/* Ownership: the shared builder BSS (+0x0..+0xFD0) is defined by the F571C TU;
 * this consumer uses that owner for the root and node pools. */
/* 158 -> 0, lane p10-o101 2026-09-12, on the lever that closed the rest of
 * this overlay's builder family, and the residual was never four separate
 * problems -- it was one carrier.
 * L145 -- DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX.
 *   `index`, `node32` and `node24` are gone and every store is spelled through
 *   the array subscript of the counter global itself. A uopt live range is
 *   formed per IR name (L131), so the subscript written on both sides of the
 *   call is ONE range spanning it, and both occurrences take the caller-saved
 *   colour the shipped code uses; the counter read becomes its own range on the
 *   callee-saved register the index local used to hold; and the pre-call read
 *   lands in a ring temporary, which a local can never be (L130). The bump
 *   moves to the end of its group, since the stores now read the counter
 *   rather than a captured address. 158 -> 2 at size delta 0.
 * L146 -- every field order this file carried was an optimum of the carrier
 *   shape and none of them survives it. All four text rows take the declared
 *   field order now, one physical line each, including the two that had been
 *   expanded one statement per line and the one that carried a scrambled
 *   colour order; the two root headers likewise. The only order that is not
 *   the declared one is in the sprite node, where color13 precedes value18,
 *   and that single swap is the last 2 words. A dependence-respecting climb
 *   over all four groups with the call held as a barrier reaches 0 and a
 *   second round is flat. */
void func_overlay_101_F000B544_18E6D64(void) {
    s32 length;
    void *handle;

    D_0.kind = 4; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.width2E = 0x140; D_0.value2C = 0; D_0.chainType = 0; D_0.height30 = 0xF0; D_0.asset34 = &D_DA4; D_0.chain = 0; D_1C0[D_1C4] = &D_1C; D_1C4 = D_1C4 + 1;

    D_340[o101Node32Counter_o101Reloc].m1.x = 0x108; D_340[o101Node32Counter_o101Reloc].m1.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m1.value10 = 0; D_340[o101Node32Counter_o101Reloc].m1.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m1.color13 = 0; D_340[o101Node32Counter_o101Reloc].m1.value18 = 0; D_340[o101Node32Counter_o101Reloc].m1.y = 0x154; D_340[o101Node32Counter_o101Reloc].m1.value14 = 0.0f; handle = texLoadSprite(0x91, 0); D_340[o101Node32Counter_o101Reloc].m1.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m1.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m1.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m1; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x1C; D_0.height48 = 0x50; D_0.value4A = 0xD8; D_0.value4C = 0x42; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = 0; D_0.data50 = D_INPUT.data184; D_1C0[D_1C4] = &D_38; D_1C4 = D_1C4 + 1;

    D_540[D_1D0].m1.x = 0x6C; D_540[D_1D0].m1.y = 0x10; length = overlay101ByteLength(D_INPUT.text188); D_540[D_1D0].m1.length = (u8)length; D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].m1.mode = 2; D_540[D_1D0].m1.color0 = 0xFF; D_540[D_1D0].m1.color1 = 0x80; D_540[D_1D0].m1.color2 = 0; D_540[D_1D0].m1.color3 = 0xFF; D_540[D_1D0].m1.kind = 4; D_540[D_1D0].m1.text = D_INPUT.text188; D_540[D_1D0].m1.previousType = D_0.childType; D_540[D_1D0].m1.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0].m1; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].m1.x = 0x6C; D_540[D_1D0].m1.y = 0x1E; length = overlay101ByteLength(D_INPUT.text18C); D_540[D_1D0].m1.length = (u8)length; D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].m1.mode = 2; D_540[D_1D0].m1.color0 = 0xFF; D_540[D_1D0].m1.color1 = 0xFF; D_540[D_1D0].m1.color2 = 0; D_540[D_1D0].m1.color3 = 0xFF; D_540[D_1D0].m1.kind = 4; D_540[D_1D0].m1.text = D_INPUT.text18C; D_540[D_1D0].m1.previousType = D_0.childType; D_540[D_1D0].m1.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0].m1; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].m1.x = 0x6C; D_540[D_1D0].m1.y = 0x28; length = overlay101ByteLength(D_INPUT.text190); D_540[D_1D0].m1.length = (u8)length; D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].m1.mode = 2; D_540[D_1D0].m1.color0 = 0xFF; D_540[D_1D0].m1.color1 = 0xFF; D_540[D_1D0].m1.color2 = 0; D_540[D_1D0].m1.color3 = 0xFF; D_540[D_1D0].m1.kind = 4; D_540[D_1D0].m1.text = D_INPUT.text190; D_540[D_1D0].m1.previousType = D_0.childType; D_540[D_1D0].m1.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0].m1; D_1D0 = D_1D0 + 1;

    D_540[D_1D0].m1.x = 0x6C; D_540[D_1D0].m1.y = 0x32; length = overlay101ByteLength(D_INPUT.text194); D_540[D_1D0].m1.length = (u8)length; D_540[D_1D0].m1.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0); D_540[D_1D0].m1.mode = 2; D_540[D_1D0].m1.color0 = 0xFF; D_540[D_1D0].m1.color1 = 0xFF; D_540[D_1D0].m1.color2 = 0; D_540[D_1D0].m1.color3 = 0xFF; D_540[D_1D0].m1.kind = 4; D_540[D_1D0].m1.text = D_INPUT.text194; D_540[D_1D0].m1.previousType = D_0.childType; D_540[D_1D0].m1.previous = D_0.child; D_0.childType = 3; D_0.child = &D_540[D_1D0].m1; D_1D0 = D_1D0 + 1;

    overlay101Reset(&D_4358);
}
#undef D_0
#undef D_1C
#undef D_38
#undef D_INPUT

/* ---- overlay101TailBA34 (+0xBA34) ---- */
#define D_0 D_0.m14

/* The three creator calls and finalizer are packet-local opaque identities. */

/* Exact typed owner for overlay 101 +0xBA34..+0xC144. */
/* 393 -> 0, lane p10-o101 2026-09-12, on three edits and in that order. The
 * residual read as a prologue problem -- first mismatch at +0 with a frame
 * 0x10 too large -- and it was two carriers and one constant.
 * L145, but PARTIALLY, and the boundary is the point. Delete the locals that
 *   held an element address or an element index and spell those stores through
 *   the array subscript of the counter global itself, so that the subscript
 *   written on both sides of a call is ONE IR name (L131) and the pre-call read
 *   lands in a ring temporary a local can never be (L130). That works for the
 *   sprite nodes and the text rows, whose counter has ONE name. It does NOT
 *   work for the three root headers: this reconstruction models the shipped
 *   relocation identity by giving the order counter two names, Call0 and Call1,
 *   so the pre-call and post-call subscripts are different expressions, L131
 *   cannot fuse them, and going direct there costs instructions -- whole-body
 *   direct measures 444 at delta +8 against 393. Mixed, it is 333 at delta 0.
 *   THE TEST IS WHETHER BOTH SIDES OF THE CALL CAN BE SPELLED THE SAME.
 * L144 -- `register volatile f32 scale` was the second carrier and the larger
 *   one. volatile pins a home and re-loads it at every use: the frame census
 *   showed our +0x40 slot taking five loads where the target takes one, plus a
 *   whole extra slot. The shipped code has no carrier at all -- the multiplier
 *   is the `(f32)(s32)1` literal written in place, exactly as in the rest of
 *   this overlay's builders. 304 -> 263 at delta 0, and the first divergence
 *   moves from +0 to +0x144, which is the frame going exact.
 * L146 -- then re-climb, because every order measured before those two edits is
 *   void. The climb moves one statement: kind ahead of width2E in the first
 *   root header. 4 -> 0. Two further rounds are flat. */
void func_overlay_101_F000BA34_18E7254(s32 variant) {
    void *handle;
    s32 length;
    s32 orderIndex;

    orderIndex = gO101TailBA34OrderCountCall0; D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.kind = 4; D_0.asset34 = &gO101TailBA34AssetDB8; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailBA34OrderSlots[orderIndex] = &D_0.chainType; gO101TailBA34OrderCountCall0 = orderIndex + 1;

    D_340[o101Node32Counter_o101Reloc].m9.x = 0x4E; D_340[o101Node32Counter_o101Reloc].m9.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m9.value10 = 0; D_340[o101Node32Counter_o101Reloc].m9.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m9.color13 = 0; D_340[o101Node32Counter_o101Reloc].m9.value18 = 0; D_340[o101Node32Counter_o101Reloc].m9.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m9.value14 = 0.0f; handle = texLoadSprite(0x92, NULL, &gO101TailBA34OrderCountCall0); D_340[o101Node32Counter_o101Reloc].m9.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m9.previous = D_0.chain; D_340[o101Node32Counter_o101Reloc].m9.handle = handle; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m9; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    orderIndex = gO101TailBA34OrderCountCall1; D_0.x42 = 0x20; D_0.width44 = 0x18; D_0.y46 = 0x8C; D_0.height48 = 0x18; D_0.value4A = 0x5B; D_0.value4C = 0x63; D_0.mode40 = 0; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailBA34Inputs.data198; gO101TailBA34OrderSlots[orderIndex] = &D_0.childType; gO101TailBA34OrderCountCall1 = orderIndex + 1;

    if (variant == 0) {
        D_340[o101Node32Counter_o101Reloc].m9.x = 6; D_340[o101Node32Counter_o101Reloc].m9.y = 12; D_340[o101Node32Counter_o101Reloc].m9.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m9.value10 = 0; D_340[o101Node32Counter_o101Reloc].m9.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m9.color13 = 0; D_340[o101Node32Counter_o101Reloc].m9.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m9.value18 = 0; handle = texLoadSprite(0xA0, NULL, &gO101TailBA34OrderCountCall1); D_340[o101Node32Counter_o101Reloc].m9.previousType = D_0.childType; D_340[o101Node32Counter_o101Reloc].m9.previous = D_0.child; D_340[o101Node32Counter_o101Reloc].m9.handle = handle; D_0.childType = 2; D_0.child = &D_340[o101Node32Counter_o101Reloc].m9; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;
    } else {
        D_340[o101Node32Counter_o101Reloc].m9.x = 6; D_340[o101Node32Counter_o101Reloc].m9.y = 12; D_340[o101Node32Counter_o101Reloc].m9.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m9.value10 = 0; D_340[o101Node32Counter_o101Reloc].m9.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m9.color13 = 0; D_340[o101Node32Counter_o101Reloc].m9.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m9.value18 = 0; handle = texLoadSprite(0x9F, NULL, &gO101TailBA34OrderCountCall1); D_340[o101Node32Counter_o101Reloc].m9.previousType = D_0.childType; D_340[o101Node32Counter_o101Reloc].m9.previous = D_0.child; D_340[o101Node32Counter_o101Reloc].m9.handle = handle; D_0.childType = 2; D_0.child = &D_340[o101Node32Counter_o101Reloc].m9; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;
    }

    orderIndex = gO101TailBA34OrderCountCall1; D_0.x5E = 0x20; D_0.width60 = 0x40; D_0.y62 = 0x5A; D_0.height64 = 0x7E; D_0.value66 = 0xCC; D_0.value68 = 0x4A; D_0.mode5C = 0; D_0.color6A = 0xFF; D_0.color6B = 0xFF; D_0.tertiaryType = 0; D_0.tertiary = NULL; D_0.data6C = gO101TailBA34Inputs.data084; gO101TailBA34OrderSlots[orderIndex] = &D_0.tertiaryType; gO101TailBA34OrderCountCall1 = orderIndex + 1;

    D_540[gO101TailBA34Node24Count].m9.x = 0x66; D_540[gO101TailBA34Node24Count].m9.y = 0x10; length = overlay101ByteLength(gO101TailBA34Inputs.text19C); D_540[gO101TailBA34Node24Count].m9.length = (u8)length; D_540[gO101TailBA34Node24Count].m9.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailBA34Node24Count].m9.mode = 2; D_540[gO101TailBA34Node24Count].m9.color0 = 0; D_540[gO101TailBA34Node24Count].m9.color1 = 0; D_540[gO101TailBA34Node24Count].m9.color2 = 0; D_540[gO101TailBA34Node24Count].m9.color3 = 0; D_540[gO101TailBA34Node24Count].m9.kind = 4; D_540[gO101TailBA34Node24Count].m9.text = gO101TailBA34Inputs.text19C; D_540[gO101TailBA34Node24Count].m9.previousType = D_0.tertiaryType; D_540[gO101TailBA34Node24Count].m9.previous = D_0.tertiary; D_0.tertiaryType = 3; D_0.tertiary = &D_540[gO101TailBA34Node24Count].m9; gO101TailBA34Node24Count = gO101TailBA34Node24Count + 1;

    D_540[gO101TailBA34Node24Count].m9.x = 0x66; D_540[gO101TailBA34Node24Count].m9.y = 0x1E; length = overlay101ByteLength(gO101TailBA34Inputs.text1A0); D_540[gO101TailBA34Node24Count].m9.length = (u8)length; D_540[gO101TailBA34Node24Count].m9.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailBA34Node24Count].m9.mode = 2; D_540[gO101TailBA34Node24Count].m9.color0 = 0; D_540[gO101TailBA34Node24Count].m9.color1 = 0; D_540[gO101TailBA34Node24Count].m9.color2 = 0; D_540[gO101TailBA34Node24Count].m9.color3 = 0; D_540[gO101TailBA34Node24Count].m9.kind = 4; D_540[gO101TailBA34Node24Count].m9.text = gO101TailBA34Inputs.text1A0; D_540[gO101TailBA34Node24Count].m9.previousType = D_0.tertiaryType; D_540[gO101TailBA34Node24Count].m9.previous = D_0.tertiary; D_0.tertiaryType = 3; D_0.tertiary = &D_540[gO101TailBA34Node24Count].m9; gO101TailBA34Node24Count = gO101TailBA34Node24Count + 1;

    D_540[gO101TailBA34Node24Count].m9.x = 0x66; D_540[gO101TailBA34Node24Count].m9.y = 0x28; length = overlay101ByteLength(gO101TailBA34Inputs.text1A4); D_540[gO101TailBA34Node24Count].m9.length = (u8)length; D_540[gO101TailBA34Node24Count].m9.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailBA34Node24Count].m9.mode = 2; D_540[gO101TailBA34Node24Count].m9.color0 = 0; D_540[gO101TailBA34Node24Count].m9.color1 = 0; D_540[gO101TailBA34Node24Count].m9.color2 = 0; D_540[gO101TailBA34Node24Count].m9.color3 = 0; D_540[gO101TailBA34Node24Count].m9.kind = 4; D_540[gO101TailBA34Node24Count].m9.text = gO101TailBA34Inputs.text1A4; D_540[gO101TailBA34Node24Count].m9.previousType = D_0.tertiaryType; D_540[gO101TailBA34Node24Count].m9.previous = D_0.tertiary; D_0.tertiaryType = 3; D_0.tertiary = &D_540[gO101TailBA34Node24Count].m9; gO101TailBA34Node24Count = gO101TailBA34Node24Count + 1;

    D_540[gO101TailBA34Node24Count].m9.x = 0x66; D_540[gO101TailBA34Node24Count].m9.y = 0x32; length = overlay101ByteLength(gO101TailBA34Inputs.text1A8); D_540[gO101TailBA34Node24Count].m9.length = (u8)length; D_540[gO101TailBA34Node24Count].m9.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailBA34Node24Count].m9.mode = 2; D_540[gO101TailBA34Node24Count].m9.color0 = 0; D_540[gO101TailBA34Node24Count].m9.color1 = 0; D_540[gO101TailBA34Node24Count].m9.color2 = 0; D_540[gO101TailBA34Node24Count].m9.color3 = 0; D_540[gO101TailBA34Node24Count].m9.kind = 4; D_540[gO101TailBA34Node24Count].m9.text = gO101TailBA34Inputs.text1A8; D_540[gO101TailBA34Node24Count].m9.previousType = D_0.tertiaryType; D_540[gO101TailBA34Node24Count].m9.previous = D_0.tertiary; D_0.tertiaryType = 3; D_0.tertiary = &D_540[gO101TailBA34Node24Count].m9; gO101TailBA34Node24Count = gO101TailBA34Node24Count + 1;

    D_540[gO101TailBA34Node24Count].m9.x = 0x66; D_540[gO101TailBA34Node24Count].m9.y = 0x3C; length = overlay101ByteLength(gO101TailBA34Inputs.text1AC); D_540[gO101TailBA34Node24Count].m9.length = (u8)length; D_540[gO101TailBA34Node24Count].m9.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailBA34Node24Count].m9.mode = 2; D_540[gO101TailBA34Node24Count].m9.color0 = 0; D_540[gO101TailBA34Node24Count].m9.color1 = 0; D_540[gO101TailBA34Node24Count].m9.color2 = 0; D_540[gO101TailBA34Node24Count].m9.color3 = 0; D_540[gO101TailBA34Node24Count].m9.kind = 4; D_540[gO101TailBA34Node24Count].m9.text = gO101TailBA34Inputs.text1AC; D_540[gO101TailBA34Node24Count].m9.previousType = D_0.tertiaryType; D_540[gO101TailBA34Node24Count].m9.previous = D_0.tertiary; D_0.tertiaryType = 3; D_0.tertiary = &D_540[gO101TailBA34Node24Count].m9; gO101TailBA34Node24Count = gO101TailBA34Node24Count + 1;

    overlay101Reset(&gO101TailBA34FinalObject4630);
}
#undef D_0

/* ---- overlay101TailC144 (+0xC144) ---- */
#define D_0 D_0.m15

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative. */
/* 246 -> 209 -> 178 -> 0. The last step, lane p10-o101 2026-09-12, is L145 and
 * it also voids the three levers this comment used to record.
 * L145 -- DELETE EVERY LOCAL THAT HELD AN ELEMENT ADDRESS OR AN ELEMENT INDEX,
 *   `node32`, `node24`, `nodeIndex`, `orderIndex` and the `orderCount` pointer
 *   alike, and spell every store through the array subscript of the counter
 *   global itself. A uopt live range is formed per IR name (L131), so the
 *   subscript written on both sides of the call is ONE range spanning it and
 *   both occurrences take the caller-saved colour the shipped code uses; the
 *   counter read becomes its own range on the callee-saved register the index
 *   local used to hold; and the pre-call read lands in a ring temporary, which
 *   a local can never be, because a local is a symbol (L130). The counter bump
 *   moves to the end of its group as a consequence, since the stores now read
 *   the counter rather than a captured address. 178 -> 36 at size delta 0.
 * L146 -- every order recorded before that edit is void, and all three of the
 *   ones this comment used to carry were. The text row wanted its text store
 *   just after the opacity store, ahead of mode, from 17,481 constrained tail
 *   orders; on this shape it wants `mode` first and `text` second, which is
 *   the single swap that takes 36 -> 0. The second root header wanted height48
 *   before value4A from 18,996 head orders; on this shape the declared field
 *   order is the floor and the climb moves nothing. The extra physical line
 *   inside rows 1..4 is likewise gone: every group is one line again.
 * A four-group dependence-respecting climb -- the text row, both root headers
 * and the sprite node, with the call held as a barrier -- reaches 0 and a
 * second round is flat.
 * The promotion blocker this comment used to name, the gO101TailC144OrderCount
 * relocation identity, is resolved by the ROM: 361 of 361 words byte-exact and
 * gmake verify unchanged. */
void func_overlay_101_F000C144_18E7964(void) {
    void *handle;
    s32 length;

    D_0.width2E = 0x140; D_0.height30 = 0xF0; D_0.asset34 = &gO101TailC144AssetDCC; D_0.kind = 4; D_0.color32 = 0xFF; D_0.color33 = 0xFF; D_0.value26 = 0; D_0.value28 = 0; D_0.value2A = 0; D_0.value2C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailC144OrderSlots[gO101TailC144OrderCount] = &D_0.chainType; gO101TailC144OrderCount = gO101TailC144OrderCount + 1;

    D_340[o101Node32Counter_o101Reloc].m10.x = 0xF2; D_340[o101Node32Counter_o101Reloc].m10.y = 0x14E; D_340[o101Node32Counter_o101Reloc].m10.value10 = 0; D_340[o101Node32Counter_o101Reloc].m10.color12 = 0xFF; D_340[o101Node32Counter_o101Reloc].m10.color13 = 0; D_340[o101Node32Counter_o101Reloc].m10.value18 = 0; D_340[o101Node32Counter_o101Reloc].m10.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m10.value14 = 0.0f; handle = texLoadSprite(0x93, 0); D_340[o101Node32Counter_o101Reloc].m10.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m10.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m10; D_340[o101Node32Counter_o101Reloc].m10.handle = handle; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

    D_0.height48 = 0x51; D_0.value4A = 0xCC; D_0.value4C = 0x4E; D_0.width44 = 0x18; D_0.x42 = 0x20; D_0.y46 = 0x20; D_0.color4E = 0xFF; D_0.color4F = 0xFF; D_0.mode40 = 0; D_0.childType = 0; D_0.child = NULL; D_0.data50 = gO101TailC144Inputs.data084; gO101TailC144OrderSlots[gO101TailC144OrderCount] = &D_0.childType; gO101TailC144OrderCount = gO101TailC144OrderCount + 1;

    D_540[gO101TailC144Node24Count].m10.x = 0x66; D_540[gO101TailC144Node24Count].m10.y = 0x10; length = overlay101ByteLength(gO101TailC144Inputs.text1B0); D_540[gO101TailC144Node24Count].m10.length = (u8)length; D_540[gO101TailC144Node24Count].m10.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailC144Node24Count].m10.mode = 2; D_540[gO101TailC144Node24Count].m10.text = gO101TailC144Inputs.text1B0; D_540[gO101TailC144Node24Count].m10.color0 = 0; D_540[gO101TailC144Node24Count].m10.color1 = 0; D_540[gO101TailC144Node24Count].m10.color2 = 0; D_540[gO101TailC144Node24Count].m10.color3 = 0; D_540[gO101TailC144Node24Count].m10.kind = 4; D_540[gO101TailC144Node24Count].m10.previousType = D_0.childType; D_540[gO101TailC144Node24Count].m10.previous = D_0.child; D_0.child = &D_540[gO101TailC144Node24Count].m10; D_0.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    D_540[gO101TailC144Node24Count].m10.x = 0x66; D_540[gO101TailC144Node24Count].m10.y = 0x1E; length = overlay101ByteLength(gO101TailC144Inputs.text1B4); D_540[gO101TailC144Node24Count].m10.length = (u8)length; D_540[gO101TailC144Node24Count].m10.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailC144Node24Count].m10.mode = 2; D_540[gO101TailC144Node24Count].m10.text = gO101TailC144Inputs.text1B4; D_540[gO101TailC144Node24Count].m10.color0 = 0; D_540[gO101TailC144Node24Count].m10.color1 = 0; D_540[gO101TailC144Node24Count].m10.color2 = 0; D_540[gO101TailC144Node24Count].m10.color3 = 0; D_540[gO101TailC144Node24Count].m10.kind = 4; D_540[gO101TailC144Node24Count].m10.previousType = D_0.childType; D_540[gO101TailC144Node24Count].m10.previous = D_0.child; D_0.child = &D_540[gO101TailC144Node24Count].m10; D_0.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    D_540[gO101TailC144Node24Count].m10.x = 0x66; D_540[gO101TailC144Node24Count].m10.y = 0x28; length = overlay101ByteLength(gO101TailC144Inputs.text1B8); D_540[gO101TailC144Node24Count].m10.length = (u8)length; D_540[gO101TailC144Node24Count].m10.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailC144Node24Count].m10.mode = 2; D_540[gO101TailC144Node24Count].m10.text = gO101TailC144Inputs.text1B8; D_540[gO101TailC144Node24Count].m10.color0 = 0; D_540[gO101TailC144Node24Count].m10.color1 = 0; D_540[gO101TailC144Node24Count].m10.color2 = 0; D_540[gO101TailC144Node24Count].m10.color3 = 0; D_540[gO101TailC144Node24Count].m10.kind = 4; D_540[gO101TailC144Node24Count].m10.previousType = D_0.childType; D_540[gO101TailC144Node24Count].m10.previous = D_0.child; D_0.child = &D_540[gO101TailC144Node24Count].m10; D_0.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    D_540[gO101TailC144Node24Count].m10.x = 0x66; D_540[gO101TailC144Node24Count].m10.y = 0x36; length = overlay101ByteLength(gO101TailC144Inputs.text1BC); D_540[gO101TailC144Node24Count].m10.length = (u8)length; D_540[gO101TailC144Node24Count].m10.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailC144Node24Count].m10.mode = 2; D_540[gO101TailC144Node24Count].m10.text = gO101TailC144Inputs.text1BC; D_540[gO101TailC144Node24Count].m10.color0 = 0; D_540[gO101TailC144Node24Count].m10.color1 = 0; D_540[gO101TailC144Node24Count].m10.color2 = 0; D_540[gO101TailC144Node24Count].m10.color3 = 0; D_540[gO101TailC144Node24Count].m10.kind = 4; D_540[gO101TailC144Node24Count].m10.previousType = D_0.childType; D_540[gO101TailC144Node24Count].m10.previous = D_0.child; D_0.child = &D_540[gO101TailC144Node24Count].m10; D_0.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    D_540[gO101TailC144Node24Count].m10.x = 0x66; D_540[gO101TailC144Node24Count].m10.y = 0x40; length = overlay101ByteLength(gO101TailC144Inputs.text1C0); D_540[gO101TailC144Node24Count].m10.length = (u8)length; D_540[gO101TailC144Node24Count].m10.opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1); D_540[gO101TailC144Node24Count].m10.mode = 2; D_540[gO101TailC144Node24Count].m10.text = gO101TailC144Inputs.text1C0; D_540[gO101TailC144Node24Count].m10.color0 = 0; D_540[gO101TailC144Node24Count].m10.color1 = 0; D_540[gO101TailC144Node24Count].m10.color2 = 0; D_540[gO101TailC144Node24Count].m10.color3 = 0; D_540[gO101TailC144Node24Count].m10.kind = 4; D_540[gO101TailC144Node24Count].m10.previousType = D_0.childType; D_540[gO101TailC144Node24Count].m10.previous = D_0.child; D_0.child = &D_540[gO101TailC144Node24Count].m10; D_0.childType = 3; gO101TailC144Node24Count = gO101TailC144Node24Count + 1;

    overlay101Reset(&gO101TailC144FinalObject4A90);
}
#undef D_0

/* ---- overlay101TailC6E8 (+0xC6E8) ---- */
#define D_0 D_0.m16

/* Matched 2026-10-02 (lane x-o101), 106 -> 0 at delta 0, by rewriting the
 * inherited shape in the matched siblings' (overlay101TailAB4C/A6BC) form
 * after decoding the relocation table: the selector is the queue's first
 * byte (one symbol, not two), the queue shift is a plain indexed for loop
 * whose own guard is the second count test, every node row is a one-name
 * counter subscript with the creator result stored straight into the node,
 * and the sprite creators take two arguments (the a2/a3 the shipped calls
 * carry are the root and order-count addresses left over). The first
 * candidate in this shape scored 274 at +8; dropping the redundant inner
 * count test closed it to 0. */
void func_overlay_101_F000C6E8_18E7F08(void) {
    u8 selector;
    s32 i;

    if (levelGetNumber() == 0x12) {
        overlay101Reset(NULL);
    } else {
        selector = D_8007BF3C[0];
        if (D_8007BF44 > 0) {
            D_8007BF44--;
            for (i = 0; i < D_8007BF44; i++) {
                D_8007BF3C[i] = D_8007BF3C[i + 1];
            }
        }

        switch (selector) {
            case 0:  func_overlay_101_F0003A58_18DF278(); break;
            case 1:  func_overlay_101_F000512C_18E094C(); break;
            case 2:  func_overlay_101_F000571C_18E0F3C(); break;
            case 3:  func_overlay_101_F0005E08_18E1628(); break;
            case 4:  func_overlay_101_F00063F8_18E1C18(); break;
            case 5:  func_overlay_101_F00069E8_18E2208(); break;
            case 6:  func_overlay_101_F00078F4_18E3114(); break;
            case 7:  func_overlay_101_F0008128_18E3948(); break;
            case 8:  func_overlay_101_F000895C_18E417C(); break;
            case 9:  func_overlay_101_F0009190_18E49B0(); break;
            case 10: overlay101BuildPresentationA(); break;
            case 11: overlay101BuildPresentationC(); break;
            case 12: overlay101BuildPresentationB(); break;
            case 13: overlay101BuildPresentationD(); break;
            case 14: overlay101TailA6BC(0); break;
            case 15: overlay101TailA6BC(1); break;
            case 16: overlay101TailA6BC(2); break;
            case 17: overlay101TailA6BC(3); break;
            case 18: overlay101TailA6BC(4); break;
            case 19: overlay101TailA6BC(5); break;
            case 20: func_overlay_101_F000AB4C_18E636C(); break;
            case 21: func_overlay_101_F000B544_18E6D64(); break;
            case 22: func_overlay_101_F000BA34_18E7254(0); break;
            case 23: func_overlay_101_F000BA34_18E7254(1); break;
            case 24: func_overlay_101_F000C144_18E7964(); break;
            default: overlay101Reset(NULL); break;
        }

        if (gO101TailC6E8PresentationActive != 0) {
            D_0.kind = 2; D_0.x = -0x20; D_0.y = -0x18; D_0.width = 0x180; D_0.height = 0x120; D_0.asset = gO101TailC6E8AssetDF0_o101Reloc; D_0.color0 = 0xFF; D_0.color1 = 0xFF; D_0.value0A = 0; D_0.value0C = 0; D_0.chainType = 0; D_0.chain = NULL; gO101TailC6E8OrderSlots[gO101TailC6E8OrderCount] = &D_0; gO101TailC6E8OrderCount = gO101TailC6E8OrderCount + 1;

            D_340[o101Node32Counter_o101Reloc].m11.x = 0x35; D_340[o101Node32Counter_o101Reloc].m11.y = 0xE0; D_340[o101Node32Counter_o101Reloc].m11.value10 = 0; D_340[o101Node32Counter_o101Reloc].m11.color0 = 0xFF; D_340[o101Node32Counter_o101Reloc].m11.color1 = 0; D_340[o101Node32Counter_o101Reloc].m11.value18 = 0; D_340[o101Node32Counter_o101Reloc].m11.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m11.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m11.handle = texLoadSprite(0x8F, NULL); D_340[o101Node32Counter_o101Reloc].m11.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m11.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m11; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

            D_340[o101Node32Counter_o101Reloc].m11.x = 0xC0; D_340[o101Node32Counter_o101Reloc].m11.y = 0xE0; D_340[o101Node32Counter_o101Reloc].m11.value10 = 0; D_340[o101Node32Counter_o101Reloc].m11.color0 = 0xFF; D_340[o101Node32Counter_o101Reloc].m11.color1 = 0; D_340[o101Node32Counter_o101Reloc].m11.value18 = 0; D_340[o101Node32Counter_o101Reloc].m11.scale = 1.0f; D_340[o101Node32Counter_o101Reloc].m11.value14 = 0.0f; D_340[o101Node32Counter_o101Reloc].m11.handle = texLoadSprite(0x90, NULL); D_340[o101Node32Counter_o101Reloc].m11.previousType = D_0.chainType; D_340[o101Node32Counter_o101Reloc].m11.previous = D_0.chain; D_0.chainType = 2; D_0.chain = &D_340[o101Node32Counter_o101Reloc].m11; o101Node32Counter_o101Reloc = o101Node32Counter_o101Reloc + 1;

            gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].x = 0x20; gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].y = 0x18; gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].scale = 1.0f; gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].handle = func_80036DD0(1, &gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc]); gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].previousType = D_0.chainType; gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc].previous = D_0.chain; D_0.chainType = 1; D_0.chain = &gO101TailC6E8Nodes20[o101Node20Counter_o101Reloc]; o101Node20Counter_o101Reloc = o101Node20Counter_o101Reloc + 1;

            gO101TailC6E8Handle1D4[0].texture = texLoadTexture(0x5F1);
            gO101TailC6E8Handle1F4[0].texture = texLoadTexture(0x5F2);
        }

        gO101TailC6E8Handle33C = texLoadTexture(0x58);
        gO101TailC6E8Handle338 = mathRnd(0, 0x278D00);
    }

    gO101TailC6E8PresentationDone = 0;
    amTunePlay(0x27);
    func_800015F8();
}
#undef D_0

void overlay101UpdatePresentation(void *context, void *data, void *vertices,
                                  s32 step) {
    s32 state;

    if (gOverlay101PresentationActive != 0) {
        overlay101DispatchActive(step);
        overlay101DispatchEvents(step);
        overlay101UpdateChains(step);
        overlay101DrawSlots(context, data, vertices);
        func_8002FB34(
            context, gOverlay101Handle1D4, (f32)gOverlay101GlobalX,
            (f32)gOverlay101GlobalY, 1.0f, 1.0f, -2, 3);
        state = gOverlay101PresentationDone;
    } else {
        state = gOverlay101PresentationDone;
        if (state == 0) {
            gOverlay101PresentationTimer += step;
            if (gOverlay101PresentationTimer >= 0xD3) {
                overlay101Initialize();
                gOverlay101PresentationTimer = 0;
                state = gOverlay101PresentationDone;
            }
        }
    }

    if (state == 0) {
        if (gOverlay101InputMode != 0) {
            if (overlay101ReadInputReloc(0) & 0x9000) {
                overlay101Initialize();
                gOverlay101PresentationDone = 1;
            }
        } else if ((overlay101ReadInputReloc(0) & 0x820) == 0x820) {
            overlay101Initialize();
            gOverlay101PresentationDone = 1;
        }
    }
}

void overlay101Cleanup(void) {
    s32 count;
    O101Elem_gOverlay101Resources20 *resource20;
    O101Elem_D_340 *resource32;
    O101Elem_D_0 *order;

    if (gOverlay101Handle1D4[0].texture != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle1D4[0].texture);
        gOverlay101Handle1D4[0].texture = NULL;
    }
    if (gOverlay101Handle1F4[0].texture != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle1F4[0].texture);
        gOverlay101Handle1F4[0].texture = NULL;
    }

    count = o101Node20Counter_o101Reloc;
    resource20 = &gOverlay101Resources20[0];
    if (count--) {
        do {
            if (resource20->m2.resource != NULL) {
                overlay101ReleaseReloc(resource20->m2.resource);
                resource20->m2.resource = NULL;
            }
            resource20++;
        } while (count--);
    }

    o101Node20Counter_o101Reloc = 0;
    count = o101Node32Counter_o101Reloc;
    resource32 = &D_340[0];
    if (count--) {
        do {
            if (resource32->m12.resource != NULL) {
                overlay101ReleaseReloc(resource32->m12.resource);
                resource32->m12.resource = NULL;
            }
            resource32++;
        } while (count--);
    }
    o101Node32Counter_o101Reloc = 0;

    count = o101OrderCounter_o101Reloc;
    order = &D_0.el[0];
    if (count--) {
        do {
            order->m17.first = NULL;
            order->m17.second = NULL;
            order++;
        } while (count--);
    }
    o101OrderCounter_o101Reloc = 0;

    if (gOverlay101Handle33C != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle33C,
                               &o101OrderCounter_o101Reloc);
        gOverlay101Handle33C = NULL;
    }
}

/* ---- overlay101ByteLength (+0xCEA8) ---- */
/* This byte-string loop has no exact counterpart in the pinned DKR objects. */
s32 overlay101ByteLength(u8 *text) {
    s32 length = 0;
    u8 value = *text++;

    if (value != 0) {
        do {
            value = *text;
            length++;
            text++;
        } while (value != 0);
    }
    return length;
}
