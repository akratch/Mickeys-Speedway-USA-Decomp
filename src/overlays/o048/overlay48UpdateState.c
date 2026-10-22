#include "PR/ultratypes.h"

typedef struct Overlay48Entry {
    s32 handle;
    s16 lifetime;
    u8 pad06[2];
    s16 seed;
    s8 active;
    u8 pad0B;
    s16 opacity;
    u8 pad0E[2];
} Overlay48Entry;

extern Overlay48Entry gOverlay48Entries[5];
extern s16 gOverlay48Timer;
extern s16 *gOverlay48Script;
extern s16 gOverlay48ResourceIndex;
extern s16 gOverlay48Finished;
extern s32 gOverlay48ResourceTable[];
extern f32 gOverlay48TransitionValue;

extern void fontUseFont(s32 mode);
extern void overlay45ReleaseDescriptor(s32 handle);
extern void overlay45SetMode(s32 handle, s16 value);
extern s32 overlay45CreateDescriptor(s32 resource, s32 value, s16 seed, s32 kind);
extern void mainTitlePageInit(s32 mode);
extern void func_80001608(void);
extern void amTuneSetFade(f32 value, s32 mode);

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
