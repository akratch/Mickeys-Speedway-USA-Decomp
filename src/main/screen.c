/*
 * Compressed screen loading and drawing -- ROM 0x2F0D0-0x2F400.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/screen.c. Mickey's load/decompress and draw/VI
 * call graphs establish the two-function correspondence. Mickey's own bytes
 * decide the display-list construction and split-screen control flow.
 */

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

typedef struct ScreenCommand {
    u32 w0;
    u32 w1;
} ScreenCommand;

extern ScreenCommand D_8007A330[];

s32 frontGet2PlayerSplit(void);
s32 mainGetNumberOfCameras(void);
void viGetCurrentSize(s32 *width, s32 *height);
s32 viGetVideoMode(void);
void texDPInit(ScreenCommand **dlist);

#define SCREEN_PRIMITIVE(command, color) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xFA000000; \
        cmd->w1 = (color); \
    }

#define SCREEN_FILL_RECT(command, ulx, uly, lrx, lry) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xF6000000 | (((lrx) & 0x3FF) << 14) | \
                  (((lry) & 0x3FF) << 2); \
        cmd->w1 = (((ulx) & 0x3FF) << 14) | (((uly) & 0x3FF) << 2); \
    }

#define SCREEN_PIPE_SYNC(command) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xE7000000; \
        cmd->w1 = 0; \
    }

/* Mickey-derived display-list command construction; JFG supplies the
 * existing screen TU/name correspondence, not this C body. */
void screenLoad(ScreenCommand **dlist, s32 ulx, s32 uly, s32 lrx, s32 lry,
                s32 drawOuter) {
    ScreenCommand *command;

    command = *dlist;
    if (drawOuter != 0) {
        SCREEN_PRIMITIVE(command++, 0);
        SCREEN_FILL_RECT(command++, ulx - 1, uly - 1, lrx + 1, lry + 1);
        SCREEN_PIPE_SYNC(command++);
    }
    SCREEN_PRIMITIVE(command++, 0xFF);
    SCREEN_FILL_RECT(command++, ulx, uly, lrx, lry);
    SCREEN_PIPE_SYNC(command++);
    *dlist = command;
}
/* Mickey-derived split-screen border flow; JFG supplies the screen TU/name
 * correspondence, not this C body. */
void screenDraw(ScreenCommand **dlist) {
    s32 cameraCount;
    s32 videoMode;
    u32 width;
    u32 height;
    u32 halfWidth;
    u32 halfHeight;
    u32 borderWidth;
    u32 borderHeight;
    cameraCount = mainGetNumberOfCameras();
    videoMode = viGetVideoMode();
    if (cameraCount >= 2) {
        viGetCurrentSize((s32 *) &width, (s32 *) &height);
        halfWidth = width >> 1;
        halfHeight = height >> 1;
        borderWidth = width / 20;
        borderHeight = height / 20;

        gSPDisplayList((*dlist)++, D_8007A330);

        if (cameraCount >= 3 ||
            (cameraCount == 2 && frontGet2PlayerSplit() != 0)) {
            screenLoad(dlist, halfWidth - 1, 0, halfWidth + 1, height, 1);
            screenLoad(dlist, 0, 0, borderWidth, height, 0);
            screenLoad(dlist, width - borderWidth, 0, width, height, 0);
        }
        if (cameraCount >= 3 ||
            (cameraCount == 2 && frontGet2PlayerSplit() == 0)) {
            screenLoad(dlist, 0, halfHeight - 1, width, halfHeight + 1, 1);
            if (!(videoMode & 1)) {
                screenLoad(dlist, 0, 0, width, borderHeight, 0);
                screenLoad(dlist, 0, height - borderHeight, width, height, 0);
            }
        }
        if (cameraCount == 3) {
            screenLoad(dlist, halfWidth, halfHeight, width, height, 0);
        }
        texDPInit(dlist);
    }
}
