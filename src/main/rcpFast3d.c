/*
 * Fast3D/RCP task and clear helpers -- ROM 0x2F400-0x30CD0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/rcpFast3d.c. Mickey's exact
 * rcpFast3d/rcpInit skeleton anchors, ordered init helpers and RCP call graph
 * establish the boundary. Adapted C bodies carry point-of-use provenance;
 * Mickey's own code and data decide every promoted implementation.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"
#include "game/gameVi.h"
#include "game/sched_internal.h"
#include "n_audio/mbi.h"

typedef struct RcpCommand {
    u32 w0;
    u32 w1;
} RcpCommand;

typedef struct RcpTextureInfo {
    u8 pad00[6];
    u16 width;
    u16 height;
    u8 pad0A[4];
    u16 tileRows;
    u8 pad10[4];
    u32 *data;
    u16 count;
} RcpTextureInfo;

typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct RcpGradientColour {
    u8 red;
    u8 green;
    u8 blue;
    u8 interpolate;
} RcpGradientColour;

#define RCP_DISPLAY_LIST(command, list) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0x06000000; \
        cmd->w1 = (u32) (list); \
    }

#define RCP_PIPE_SYNC(command) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xE7000000; \
        cmd->w1 = 0; \
    }

#define RCP_SET_COLOR_IMAGE(command, width, address) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xFF100000 | (((width) - 1) & 0xFFF); \
        cmd->w1 = (address); \
    }

#define RCP_SET_DEPTH_IMAGE(command, address) \
    { \
        RcpCommand *cmd = (command); \
        cmd->w0 = 0xFE000000; \
        cmd->w1 = (address); \
    }

#define RCP_SET_FILL_CYCLE(command) \
    { \
        RcpCommand *cycleCmd = (command); \
        cycleCmd->w0 = 0xEF30000F; \
        cycleCmd->w1 = 0; \
    }

extern u8 D_8007A3A0;
extern u8 D_8007A3A4;
extern u8 D_8007A3A8;
extern u32 D_8007A3B0;
extern u32 D_8007A3AC;
extern s32 D_8007A3B4;
extern s32 D_8007A3B8;
extern s32 D_8007A3BC;
extern s32 D_8007A3C0;
extern s32 D_8007A3C4;
extern s32 D_8007A3C8;
extern s32 D_8007A3EC;
extern s32 D_8007A410;
extern OSMesg D_8007A3CC;
extern OSMesg D_8007A3F0;
extern OSMesg D_8007A414;
extern RcpCommand D_8007A438[];
extern RcpCommand D_8007A4B8[];
extern OSMesgQueue D_800D2880;
extern OSMesg D_800D2898;
extern OSMesgQueue D_800D28A0;
extern OSMesgQueue D_800D28B8;
extern OSMesg D_800D28D0[];
extern OSMesg D_800D28F0[];
extern OSMesgQueue *D_800D2C90;
extern OSMesgQueue D_800D2C98;
extern OSMesg D_800D2CB0[];
extern OSMesgQueue D_800D2CD0;
extern OSMesg D_800D2CE8[];
extern OSMesgQueue D_800D2D08;
extern OSMesg D_800D2D20[];
extern u16 *D_800D2FAC;
extern OSScTask D_800D2910[];
extern OSScTask D_800D29F0[];
extern OSScTask D_800D2AD0[];
extern OSScTask D_800D2BB0[];
extern u64 D_800D2480[];
extern u64 D_800D3670[];
extern u64 D_80077950[];
extern u64 D_80077AD0[];
extern u64 D_80085240[];
extern u64 rspbootTextEnd[];
#pragma weak rspbootTextEnd = D_80077AD0

extern u8 D_8007A4F8[];
extern u8 D_8007A540[];
extern u8 D_8007A588[];
extern u8 D_8007A5C0[];
extern u8 D_8007A600[];
extern s32 texFrame(void *texture, s32 frame, ...);
extern void func_80034910();

OSMesgQueue *osScGetInterruptQ(OSSched *scheduler);
void osWritebackDCacheAll(void);
s32 TrapDanglingJump(void);
s32 camIsUserView(s32 arg0);
s32 camGetVisibleUserView(s32 arg0, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
s32 camGetMode(void);
s32 frontGet2PlayerSplit(void);
void camSetScissor(RcpCommand **dlist);
void func_8002EBE0(RcpCommand **dlist, s32 width, s32 height, u32 value);
void rcpClearZBuffer(RcpCommand **dlist, u32 width, u32 height, s32 x1,
                     s32 y1, s32 x2, s32 y2);

/* Mickey-derived task construction. JFG supplies the function name and the
 * OSScTask field correspondence, while its public C file retains assembly;
 * JFG's SDK ucode header supplies the official rspbootTextEnd symbol name. */
s32 rcpFast3d(u64 *dataStart, u64 *dataEnd, s32 taskType,
              void *framebuffer) {
    OSScTask *task;
    s32 taskFlags;

    D_8007A3C4 = 1;
    taskFlags = 3;
    switch (taskType) {
        case 0:
            task = &D_800D2910[D_8007A3B4];
            D_8007A3B4 ^= 1;
            taskFlags = 0x23;
            task->msgQ = &D_800D28B8;
            task->unk58 = 0xFF0000FF;
            task->unk5C = 0xFF0000FF;
            task->taskID = 2;
            break;
        case 3:
            task = &D_800D29F0[D_8007A3B8];
            D_8007A3B8 ^= 1;
            task->msgQ = &D_800D2C98;
            task->msg = &D_8007A3CC;
            task->unk58 = 0xFF00FFFF;
            task->unk5C = 0xFF00FFFF;
            task->taskID = 5;
            break;
        case 4:
            task = &D_800D2AD0[D_8007A3BC];
            D_8007A3BC ^= 1;
            task->msgQ = &D_800D2CD0;
            task->msg = &D_8007A3F0;
            task->unk58 = 0xFFFF00FF;
            task->unk5C = 0xFFFF00FF;
            task->taskID = 6;
            break;
        case 5:
            task = &D_800D2BB0[D_8007A3C0];
            D_8007A3C0 ^= 1;
            task->msgQ = &D_800D2D08;
            task->msg = &D_8007A414;
            task->unk58 = 0x00FF00FF;
            task->unk5C = 0x00FF00FF;
            task->taskID = 7;
            break;
    }

    task->unk68 = 0;
    task->list.t.dram_stack = D_800D2480;
    task->list.t.dram_stack_size = 0x400;
    task->list.t.yield_data_ptr = D_800D3670;
    task->list.t.yield_data_size = 0xA00;
    task->flags = taskFlags;
    task->list.t.data_ptr = dataStart;
    task->list.t.data_size = dataEnd - dataStart;
    task->list.t.ucode_boot = D_80077950;
    task->list.t.type = 1;
    task->list.t.flags = 2;
    task->list.t.ucode_boot_size = (u32) rspbootTextEnd -
                                   (u32) D_80077950;
    task->list.t.ucode = D_80077AD0;
    task->list.t.ucode_data = D_80085240;
    task->list.t.ucode_data_size = 0x800;
    task->list.t.output_buff = NULL;
    task->list.t.output_buff_size = NULL;
    task->next = NULL;
    task->framebuffer = framebuffer;
    task->unk60 = 0xFF;
    task->unk64 = 0xFF;
    osWritebackDCacheAll();
    osSendMesg(D_800D2C90, task, OS_MESG_BLOCK);
    return 0;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/rcpFast3d.c:rcpWaitDP. */
s32 rcpWaitDP(void) {
    OSMesg message = NULL;
    OSMesg refractDoneMessage = NULL;
    OSMesg blurDoneMessage = NULL;

    if (D_8007A3C4 == 0) {
        return 0;
    }
    osRecvMesg(&D_800D28B8, &message, OS_MESG_BLOCK);
    if (D_8007A410 != 0) {
        osRecvMesg(&D_800D2D08, &blurDoneMessage, OS_MESG_BLOCK);
        D_8007A410 = 0;
    }
    if (D_8007A3EC != 0) {
        osRecvMesg(&D_800D2CD0, &refractDoneMessage, OS_MESG_BLOCK);
        D_8007A3EC = 0;
    }
    if (D_8007A3C8 != 0) {
        TrapDanglingJump();
        D_8007A3C8 = 0;
    }
    D_8007A3C4 = 0;
    return ((s32 *) message)[1];
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpSetScreenColour. */
void rcpSetScreenColour(u8 red, u8 green, u8 blue) {
    D_8007A3A0 = red;
    D_8007A3A4 = green;
    D_8007A3A8 = blue;
}
/* PROVENANCE: body and name adapted from Diddy Kong Racing's public decomp, src/rcp_dkr.c:bgdraw_fillcolour. */
void bgdraw_fillcolour(s32 red, s32 green, s32 blue) {
    D_8007A3AC = ((red << 8) & 0xF800) | ((green << 3) & 0x7C0) | ((blue >> 2) & 0x3E) | 1;
    D_8007A3AC |= D_8007A3AC << 16;
}
void func_8002EBD4(u32 value) {
    D_8007A3B0 = value;
}
/*
 * Draws the sky gradient: eight bands per screen, split for two players. A
 * band either fills flat or steps its colour every two lines toward the next
 * entry's. Written from the listing on 2026-10-02 (lane w2-front) and matched
 * the same day (lane z-res) by three edits: the gradient step packs its
 * colour inline twice in the fill-colour packet, while the flat band names
 * it in an s32 local (so uopt saves the packed value only in the loop); the
 * step loop sits in an `if (1)` region, which puts its first test in a block
 * of its own so the post-decrement copy is of a variable and survives; and
 * `i` is cleared before `bandStart`. The pad cells reproduce the 0x88 frame
 * and are not claimed as the original's declarations.
 */
void func_8002EBE0(RcpCommand **dlist, s32 width, s32 height, u32 colours) {
    s32 pad[15];
    s32 screens;
    s32 pad2;
    RcpCommand *cmd;
    s32 mode;
    s32 screenHeight;
    s32 y;
    s32 i;
    s32 bandStart;
    s32 steps;
    s32 redStep;
    s32 greenStep;
    s32 blueStep;
    s32 redOffset;
    s32 greenOffset;
    s32 blueOffset;
    s32 r;
    s32 g;
    s32 b;
    s32 colour;
    RcpGradientColour *entry;

    cmd = *dlist;
    screens = 1;
    mode = camGetMode();
    if (mode >= 2 || (mode == 1 && frontGet2PlayerSplit() == 0)) {
        screens = 2;
    }
    gDPPipeSync(cmd++);
    gDPSetScissor(cmd++, G_SC_NON_INTERLACE, 0, 0, width - 1, height - 1);
    RCP_SET_FILL_CYCLE(cmd++);
    screenHeight = height >> (screens - 1);
    y = 0;
    while (screens--) {
        entry = (RcpGradientColour *) colours;
        i = 0;
        bandStart = 0;
        do {
            i++;
            if (entry->interpolate) {
                steps = ((bandStart + screenHeight) >> 4) - (bandStart >> 4);
                redStep = ((entry[1].red - entry->red) << 16) / steps;
                greenStep = ((entry[1].green - entry->green) << 16) / steps;
                blueStep = ((entry[1].blue - entry->blue) << 16) / steps;
                redOffset = 0;
                greenOffset = 0;
                blueOffset = 0;
                if (1) while (steps--) {
                    r = entry->red + (redOffset >> 16);
                    g = entry->green + (greenOffset >> 16);
                    b = entry->blue + (blueOffset >> 16);
                    gDPSetFillColor(cmd++, (GPACK_RGBA5551(r, g, b, 1) << 16) | GPACK_RGBA5551(r, g, b, 1));
                    gDPFillRectangle(cmd++, 0, y, width, y + 2);
                    redOffset += redStep;
                    greenOffset += greenStep;
                    blueOffset += blueStep;
                    y += 2;
                }
            } else {
                r = entry->red;
                g = entry->green;
                b = entry->blue;
                colour = GPACK_RGBA5551(r, g, b, 1);
                gDPSetFillColor(cmd++, (colour << 16) | colour);
                gDPFillRectangle(cmd++, 0, y, width,
                                 y + (((bandStart + screenHeight) >> 4) - (bandStart >> 4)) * 2);
                y += (((bandStart + screenHeight) >> 4) - (bandStart >> 4)) * 2;
            }
            bandStart += screenHeight;
            entry++;
        } while (i != 8);
    }
    gDPPipeSync(cmd++);
    *dlist = cmd;
}
/* PROVENANCE: command sequence adapted from DKR's public src/rcp_dkr.c:bgdraw_render. */
void rcpClearZBuffer(RcpCommand **dlistPtr, u32 width, u32 height, s32 left,
                     s32 top, s32 right, s32 bottom) {
    RcpCommand *dlist;
    s32 alignedX1;
    s32 alignedX2;

    if ((D_800D2FAC != 0) && (left < right) && (top < bottom)) {
        alignedX1 = left & ~3;
        dlist = *dlistPtr;
        alignedX2 = (right + 3) & ~3;
        left = alignedX1;
        right = alignedX2;
        RCP_PIPE_SYNC(dlist++);
        gDPSetScissor(dlist++, G_SC_NON_INTERLACE, 0, 0, width - 1,
                      height - 1);
        RCP_SET_FILL_CYCLE(dlist++);
        RCP_SET_COLOR_IMAGE(dlist++, width, 0x02000000);
        gDPSetFillColor(dlist++, 0xFFFCFFFC);
        gDPFillRectangle(dlist++, left, top, right, bottom);
        RCP_PIPE_SYNC(dlist++);
        RCP_SET_COLOR_IMAGE(dlist++, width, 0x01000000);
        *dlistPtr = dlist;
    }
}
/* PROVENANCE: display-list command spelling adapted from Diddy Kong Racing's
 * public decomp, src/rcp_dkr.c:bgdraw_render. Mickey's enable flag, helpers,
 * coordinates, and branch structure decide the implementation; JFG supplies
 * the ordered rcpClearScreen correspondence while retaining assembly. */
void rcpClearScreen(RcpCommand **dlist, s32 unused, s32 drawBackground) {
    s32 width;
    s32 height;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    viGetCurrentSize(&width, &height);
    if (TrapDanglingJump() == 0) {
        rcpClearZBuffer(dlist, width, height, 0, 0, width, height);
    }
    if (drawBackground != 0) {
        if (camIsUserView(0) != 0) {
            gDPSetFillColor((*dlist)++, D_8007A3AC);
            gDPFillRectangle((*dlist)++, 0, 0, width - 1, height - 1);

            if (camGetVisibleUserView(0, &x1, &y1, &x2, &y2) != 0) {
                gDPSetCycleType((*dlist)++, G_CYC_1CYCLE);
                gDPSetPrimColor((*dlist)++, 0, 0, D_8007A3A0,
                                D_8007A3A4, D_8007A3A8, 0xFF);
                gDPSetCombineMode((*dlist)++, G_CC_PRIMITIVE,
                                  G_CC_PRIMITIVE);
                gDPSetRenderMode((*dlist)++, G_RM_OPA_SURF,
                                 G_RM_OPA_SURF2);
                gDPFillRectangle((*dlist)++, x1, y1, x2, y2);
            }
        } else if (D_8007A3B0 != 0) {
            func_8002EBE0(dlist, width, height, D_8007A3B0);
        } else {
            gDPSetFillColor(
                (*dlist)++,
                (GPACK_RGBA5551(D_8007A3A0, D_8007A3A4, D_8007A3A8, 1)
                 << 16) |
                    GPACK_RGBA5551(D_8007A3A0, D_8007A3A4, D_8007A3A8, 1));
            gDPFillRectangle((*dlist)++, 0, 0, width - 1, height - 1);
        }
    }
    gDPPipeSync((*dlist)++);
    camSetScissor(dlist);
}
void rcpInitDp(RcpCommand **dlist) {
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    RCP_PIPE_SYNC((*dlist)++);
    RCP_SET_COLOR_IMAGE((*dlist)++, width, 0x01000000);
    RCP_SET_DEPTH_IMAGE((*dlist)++, 0x02000000);
    RCP_DISPLAY_LIST((*dlist)++, D_8007A438);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpInitDpNoSize. */
void rcpInitDpNoSize(RcpCommand **dlist) {
    RCP_DISPLAY_LIST((*dlist)++, D_8007A438);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/rcpFast3d.c:rcpInitSp. */
void rcpInitSp(RcpCommand **dlist) {
    RCP_DISPLAY_LIST((*dlist)++, D_8007A4B8);
}
/* Mickey-derived reconstruction; JFG supplies the name, prototype, and exact
 * object-skeleton anchor, while its public src/rcpFast3d.c retains assembly. */
void rcpInit(OSSched *scheduler) {
    D_800D2C90 = osScGetInterruptQ(scheduler);
    osCreateMesgQueue(&D_800D2880, &D_800D2898, 1);
    osCreateMesgQueue(&D_800D28A0, D_800D28D0, 8);
    osCreateMesgQueue(&D_800D28B8, D_800D28F0, 8);
    osCreateMesgQueue(&D_800D2C98, D_800D2CB0, 8);
    osCreateMesgQueue(&D_800D2CD0, D_800D2CE8, 8);
    osCreateMesgQueue(&D_800D2D08, D_800D2D20, 8);
}
/* PROVENANCE -- the indexed texture loop is adapted from Diddy Kong Racing's
 * public src/rcp_dkr.c:texrect_draw. Mickey's command stream, extra texture
 * fields, and helper calls remain the controlling evidence. */
/* Four edits closed this, each measured alone: the raw command word's tile
 * field is spelled unsigned so it is the same constant as the load block's
 * _SHIFTL(G_TX_LOADTILE, 24, 3) and both share one materialisation; the tile
 * size arguments shift rather than multiply, so the -1 is not reassociated
 * past the scale; the texture rectangle is written out longhand because its
 * third command's cursor IS lastCmd, which the macro form cannot express; and
 * the second command of the plain-texture branch takes its cursor before the
 * texel count is computed, which is what orders those two webs. */
void func_8002F618(RcpCommand **dlistPtr, RcpTextureNode *nodes, s32 xPos,
                   s32 yPos, u8 red, u8 green, u8 blue, u8 alpha) {
    RcpTextureInfo *tex;
    RcpTextureInfo *alternate;
    RcpCommand *dlist;
    RcpCommand *blockCmd;
    RcpCommand *rectCmd;
    RcpCommand *halfCmd;
    RcpCommand *lastCmd;
    /* no node cursor: see the note above func_8002F618 */
    s32 i;
    s32 uly;
    s32 ulx;
    s32 lry;
    s32 lrx;
    s32 t;
    s32 s;
    s32 loadCount;

    i = 0;
    if (nodes->texture != NULL) {
        dlist = *dlistPtr;
        if (nodes->alternate != NULL) {
            lastCmd = (RcpCommand *)D_8007A540;
        } else {
            lastCmd = (RcpCommand *)D_8007A4F8;
        }

        RCP_DISPLAY_LIST(dlist++, lastCmd);
        gDPSetPrimColor((Gfx *)dlist++, 0, 0, red, green, blue, alpha);

        xPos *= 4;
        yPos *= 4;
        tex = nodes[i].texture;

        while (tex != NULL) {
            ulx = (nodes[i].x * 4) + xPos;
            uly = (nodes[i].y * 4) + yPos;
            lrx = (tex->width * 4) + ulx;
            lry = (tex->height * 4) + uly;
            if (lrx > 0 && lry > 0) {
                s = 0;
                t = 0;
                if (ulx < 0) {
                    s = -(ulx * 8);
                    ulx = 0;
                }
                if (uly < 0) {
                    t = -(uly * 8);
                    uly = 0;
                }

                alternate = nodes[i].alternate;
                if (alternate != NULL) {
                    gDPSetTextureImage(
                        (Gfx *)dlist++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1,
                        ((((s32)nodes[i].packedOffset >> 16) * tex->tileRows) +
                         (s32)tex + 0x20));
                    gDPSetTile((Gfx *)dlist++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                               0, 0, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               8, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 8,
                               G_TX_NOLOD);
                    gDPLoadSync((Gfx *)dlist++);
                    gDPLoadBlock((Gfx *)dlist++, G_TX_LOADTILE, 0, 0,
                                 (tex->width * tex->height) - 1, 0);
                    gDPPipeSync((Gfx *)dlist++);
                    gDPSetTile((Gfx *)dlist++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                               ((tex->width * 2) + 7) >> 3, 0, G_TX_RENDERTILE,
                               0, G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD,
                               G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD);
                    gDPSetTileSize((Gfx *)dlist++, G_TX_RENDERTILE, 0, 0,
                                   (tex->width - 1) << 2,
                                   (tex->height - 1) << 2);
                    gDPSetTextureImage(
                        (Gfx *)dlist++, G_IM_FMT_I, G_IM_SIZ_16b, 1,
                        ((((s32)nodes[i].packedOffset >> 16) *
                          alternate->tileRows) + (s32)alternate + 0x20));
                    gDPSetTile((Gfx *)dlist++, G_IM_FMT_I, G_IM_SIZ_16b, 0,
                               0x100, G_TX_LOADTILE, 0,
                               G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD,
                               G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD);
                    gDPLoadSync((Gfx *)dlist++);
                    gDPLoadBlock(
                        (Gfx *)dlist++, G_TX_LOADTILE, 0, 0,
                        (((s32)(alternate->width * alternate->height) + 3) >> 2) - 1,
                        0);
                    gDPPipeSync((Gfx *)dlist++);
                    gDPSetTile((Gfx *)dlist++, G_IM_FMT_I, G_IM_SIZ_4b,
                               (((s32)alternate->width >> 1) + 7) >> 3, 0x100,
                               1, 0, G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD,
                               G_TX_NOMIRROR | G_TX_WRAP, 8, G_TX_NOLOD);
                    gDPSetTileSize((Gfx *)dlist++, 1, 0, 0,
                                   (alternate->width - 1) << 2,
                                   (alternate->height - 1) << 2);
                } else {
                    dlist->w0 = *tex->data;
                    dlist->w1 = (u32)(texFrame(
                        tex, nodes[i].packedOffset) + 0x80000000U);
                    dlist++;
                    blockCmd = dlist++; loadCount = tex->count - 1; blockCmd->w0 = (((loadCount & 0xFF) << 16) | 0x07000000U | ((loadCount * 8) & 0xFFFF)); blockCmd->w1 = (u32)tex->data + 0x80000008U;
                }

                rectCmd = dlist++; rectCmd->w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(lrx, 12, 12) | _SHIFTL(lry, 0, 12)); rectCmd->w1 = (_SHIFTL(G_TX_RENDERTILE, 24, 3) | _SHIFTL(ulx, 12, 12) | _SHIFTL(uly, 0, 12));
                halfCmd = dlist++; halfCmd->w0 = _SHIFTL(G_RDPHALF_1, 24, 8); halfCmd->w1 = (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16));
                lastCmd = dlist++; lastCmd->w0 = _SHIFTL(G_RDPHALF_2, 24, 8); lastCmd->w1 = (_SHIFTL(1024, 16, 16) | _SHIFTL(1024, 0, 16));
            }
            i++;
            tex = nodes[i].texture;
        }

        gDPPipeSync((Gfx *)dlist++);
        gDPSetPrimColor((Gfx *)dlist++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        *dlistPtr = dlist;
        func_80034910(lastCmd);
    }
}
/*
 * PROVENANCE: the scaled rectangle loop is adapted from Diddy Kong Racing's
 * public src/rcp_dkr.c:texrect_draw_scaled, including its `(u8) flags & 0xFF`
 * table select. Mickey's texture fetch helper, its DMA command and its prim
 * colour reset differ from DKR. Mickey's target decides those.
 *
 * Matched 2026-10-02 (lane z-res) from lane w2-front's 155-word body by:
 * - the double mask on the table index, which spends the ring draw the
 *   target spends before the masked index (155 -> 39);
 * - each packet's cursor and two stores on one source line, as a macro
 *   expansion would be, which gives as1 the target's store order (39 -> 20);
 * - an indexed node loop with a three-argument helper call, so the node
 *   cursor is uopt's own induction temporary and is saved in a temporary
 *   cell after the four hoisted scalars, not on a declared home;
 * - the display-list packet written through lastCmd, which removes one
 *   declared cell and puts those five cells on the target's offsets;
 * - `-Wab,-r4300_mul` on the unit (Makefile), which places the two scale
 *   multiplies.
 * The flip and position setup stays inside `if (tex != NULL)`: it removes a
 * basic block, so xScale and yScale take f20/f22 (lane w2-front).
 */
void func_8002FB34(RcpCommand **dlistPtr, RcpTextureNode *nodes, f32 xPos, f32 yPos,
                   f32 xScale, f32 yScale, u32 colour, s32 flags) {
    RcpTextureInfo *tex;
    u8 *dmaDlist;
    RcpCommand *dlist;
    s32 s;
    s32 t;
    s32 dsdx;
    s32 width;
    s32 height;
    s32 dtdy;
    s32 ulx;
    s32 uly;
    s32 lrx;
    s32 lry;
    s32 count;
    RcpCommand *blockCmd;
    RcpCommand *rectCmd;
    RcpCommand *halfCmd;
    RcpCommand *lastCmd;
    s32 xPos4x;
    s32 yPos4x;
    s32 bFlipX;
    s32 bFlipY;
    s32 i;

    width = 0;
    height = 0;
    dlist = *dlistPtr;
    viGetCurrentSize(&width, &height);
    height *= 4;
    width *= 4;
    if ((colour & 0xFF) == 0xFF) {
        dmaDlist = D_8007A5C0 + ((u8) (flags & 0xFF) * 0x10);
    } else {
        dmaDlist = D_8007A600 + ((u8) (flags & 0xFF) * 0x10);
    }
    xScale *= 4.0f;
    yScale *= 4.0f;
    lastCmd = dlist++; lastCmd->w0 = 0x06000000; lastCmd->w1 = (u32) (D_8007A588);
    { RcpCommand *_g = dlist++; _g->w0 = 0x07020010; _g->w1 = (u32) dmaDlist + 0x80000000U; }
    { RcpCommand *_g = dlist++; _g->w0 = 0xFA000000; _g->w1 = colour; }
    tex = nodes->texture;
    halfCmd = NULL;
    if (tex != NULL) {
        xPos4x = xPos * 4.0f;
        yPos4x = yPos * 4.0f;
        bFlipX = flags & 0x1000;
        bFlipY = flags & 0x2000;
        i = 0;
        do {
            if (!bFlipX) {
                ulx = (s32) (nodes[i].x * xScale) + xPos4x;
            } else {
                lrx = xPos4x - (s32) (nodes[i].x * xScale);
                ulx = lrx - (s32) (tex->width * xScale);
            }
            if (!bFlipY) {
                uly = (s32) (nodes[i].y * yScale) + yPos4x;
            } else {
                lry = yPos4x - (s32) (nodes[i].y * yScale);
                uly = lry - (s32) (tex->height * yScale);
            }
            if (ulx < width && uly < height) {
                if (!bFlipX) {
                    lrx = (s32) (tex->width * xScale) + ulx;
                }
                if (!bFlipY) {
                    lry = (s32) (tex->height * yScale) + uly;
                }
                if (lrx > 0 && lry > 0 && ulx < lrx && uly < lry) {
                    dsdx = ((tex->width - 1) << 12) / (lrx - ulx);
                    if (bFlipX) {
                        s = (tex->width - 1) << 5;
                        dsdx = -dsdx;
                    } else {
                        s = 0;
                    }
                    dtdy = ((tex->height - 1) << 12) / (lry - uly);
                    if (bFlipY) {
                        t = (tex->height - 1) << 5;
                        dtdy = -dtdy;
                    } else {
                        t = 0;
                    }
                    if (ulx < 0) {
                        s += (-ulx * dsdx) >> 7;
                        ulx = 0;
                    }
                    if (uly < 0) {
                        t += (-uly * dtdy) >> 7;
                        uly = 0;
                    }
                    dmaDlist = (u8 *) tex->data;
                    dlist->w0 = *(u32 *) dmaDlist;
                    dlist->w1 = texFrame(tex, nodes[i].packedOffset, halfCmd) + 0x80000000U;
                    dlist++;
                    dmaDlist += 8;
                    blockCmd = dlist++; count = tex->count - 1; blockCmd->w0 = ((count & 0xFF) << 16) | 0x07000000 | ((count * 8) & 0xFFFF); blockCmd->w1 = (u32) dmaDlist + 0x80000000U;
                    rectCmd = dlist++; rectCmd->w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(lrx, 12, 12) | _SHIFTL(lry, 0, 12)); rectCmd->w1 = (_SHIFTL(G_TX_RENDERTILE, 24, 3) | _SHIFTL(ulx, 12, 12) | _SHIFTL(uly, 0, 12));
                    halfCmd = dlist++; halfCmd->w0 = _SHIFTL(G_RDPHALF_1, 24, 8); halfCmd->w1 = (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16));
                    lastCmd = dlist++; lastCmd->w0 = _SHIFTL(G_RDPHALF_2, 24, 8); lastCmd->w1 = (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16));
                }
            }
            tex = nodes[i + 1].texture;
            i++;
        } while (tex != NULL);
    }
    gDPPipeSync((Gfx *) dlist++);
    { RcpCommand *_g = dlist++; _g->w0 = 0xFA000000; _g->w1 = 0xFFFFFFFF; }
    *dlistPtr = dlist;
    func_80034910();
}
