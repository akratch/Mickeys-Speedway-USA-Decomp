#include "overlays/overlay_048.h"

/* DKR v77/v80 and JFG have no exact matching three-call setup tuple. */
void overlay48Initialize(void) {
    overlay48ConfigureReloc(0x1C, 0, 0, 0xD, 1, 0);
    mainChangeCameras(1);
    overlay48ConfigureReloc(6, 6, 0, 2, 0, 0);
    gOverlay48State = 0;
}

Overlay48Entry gOverlay48Entries[6];

/* Matched 2026-10-02. One loop over the five entries with the entry pointer
 * walking beside the index: IDO runs the remainder iteration first (entry 0,
 * constant addresses) and unrolls the other four, and because the index is
 * the secondary variable it stays a run-time 1 for the seed address. */
void overlay48InitializeState(void) {
    Overlay48Entry *entry;
    s32 i;

    for (i = 0, entry = gOverlay48Entries; i < 5; i++, entry++) {
        entry->lifetime = 0;
        entry->active = 0;
        entry->seed = D_274.values[i];
        entry->handle = 0;
    }
    gOverlay48Timer = 0;
    gOverlay48ScriptIndex = 0;
    gOverlay48Finished = 0;
    gOverlay48Script = D_174;
    amTunePlay(0x16);
    func_800015F8();
}

/* A source-level switch preserves the script operation order while giving
 * IDO the retail control web and register allocation. */
void overlay48UpdateState(s32 updateRate) {
    Overlay48Entry *entry;
    s32 index;
    s32 allSettled;
    s32 maximum;
    u32 control;

    allSettled = 1;
    fontUseFont(3);
    entry = gOverlay48Entries;
    maximum = 0xFF;
    index = 0;
    do {
        if (entry->handle != 0) {
            if (entry->lifetime > 0) {
                if (entry->active == 0) {
                    entry->lifetime -= updateRate;
                    allSettled = 0;
                }
                entry->opacity += updateRate * 4;
                if (entry->opacity >= 0x100) {
                    entry->opacity = maximum;
                }
            } else if (entry->active == 0) {
                entry->opacity -= updateRate * 4;
                if (entry->opacity < 0) {
                    entry->opacity = 0;
                    overlay45ReleaseDescriptor(entry->handle);
                    entry->handle = 0;
                } else {
                    allSettled = 0;
                }
            }
            if (entry->handle != 0) {
                overlay45SetMode(entry->handle, entry->opacity);
            }
        }
        index++;
        entry++;
    } while (index < 5);

    if (gOverlay48Finished == 0 &&
        ((gOverlay48Timer -= updateRate) <= 0)) {
        control = *gOverlay48Script;
        switch ((s32) control) {
            case -1:
                if (allSettled != 0) {
                    gOverlay48Script++;
                    index = *gOverlay48Script;
                    gOverlay48Script++;
                    gOverlay48Timer = *gOverlay48Script;
                    gOverlay48Script++;

                    entry = gOverlay48Entries;
                    if (index != 0) {
                        do {
                            if (entry->handle == 0) {
                                entry->handle = overlay45CreateDescriptor(
                                    gOverlay48ResourceTable[
                                        gOverlay48ResourceIndex++],
                                    0xA0, entry->seed, 4);
                                overlay45SetMode(entry->handle, 0);
                                index--;
                                entry->lifetime = gOverlay48Timer;
                            }
                            entry++;
                        } while (index != 0);
                    }

                    while (*gOverlay48Script == -2) {
                        gOverlay48Script++;
                        gOverlay48Entries[*gOverlay48Script].active = 1;
                        gOverlay48Script++;
                    }
                }
                break;
            case -3:
                gOverlay48Script++;
                gOverlay48Entries[*gOverlay48Script].active = 0;
                gOverlay48Entries[*gOverlay48Script].lifetime = 0;
                gOverlay48Script++;
                break;
            case -4:
                gOverlay48Finished = 1;
                mainTitlePageInit(1);
                func_80001608();
                amTuneSetFade(gOverlay48TransitionValue, 0);
                break;
        }
    }
}

/* DKR v77/v80 only contain generic fixed-handle cleanup relatives. */
void overlay48ReleaseAll(void) {
    s32 index;
    Overlay48Entry *entry;

    entry = gOverlay48Entries;
    index = 0;
    do {
        if (entry->handle != 0) {
            overlay45ReleaseDescriptor(entry->handle);
            entry->handle = 0;
        }
        index++;
        entry++;
    } while (index != 5);
}
