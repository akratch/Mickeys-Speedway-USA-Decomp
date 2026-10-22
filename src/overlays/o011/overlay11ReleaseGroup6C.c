#include "ultra64.h"

extern s32 gOverlay11GroupsActive;
extern void *gOverlay11GroupHandles[];
extern void *gOverlay11GroupEnd6[];
extern void overlay45ReleaseDescriptor(void *handle);
extern void overlay11ReleaseHandles(void);

/* DKR v77/v80 and JFG checks found only generic fixed-handle cleanup. */
void overlay11ReleaseGroup6C(void) {
    void **handle;
    void **end;

    if (gOverlay11GroupsActive != 0) {
        handle = gOverlay11GroupHandles; end = gOverlay11GroupEnd6;
        do {
            overlay45ReleaseDescriptor(*handle);
            handle++;
            handle[-1] = NULL;
        } while (handle != end);
        overlay11ReleaseHandles();
        gOverlay11GroupsActive = 0;
    }
}
