#include "ultra64.h"
#include "overlays/overlay_045.h"

typedef struct Overlay61Sprite {
    /* 0x00 */ void *texture;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 frame;
    /* 0x0C */ u8 pad0C[0x14];
} Overlay61Sprite;

/* Overlay 68's primary entry header: the ghost the menu can save. */
typedef struct Overlay61GhostEntry {
    /* 0x00 */ s32 active;
    /* 0x04 */ s8 character;
    /* 0x05 */ u8 timer;
    /* 0x06 */ s16 course;
    /* 0x08 */ s16 time;
} Overlay61GhostEntry;

typedef struct Overlay61ListEntry {
    /* 0x00 */ s16 nextState;
    /* 0x02 */ u8 slot;
    /* 0x03 */ u8 pad03[0x35];
    /* 0x38 */ u8 note[8];
} Overlay61ListEntry;

extern Overlay61Sprite gOverlay61Sprite0;
extern s32 gOverlay61Animation0;
extern f32 gOverlay61Time0;
extern Overlay61Sprite gOverlay61Sprite1;
extern s32 gOverlay61Animation1;
extern f32 gOverlay61Time1;
extern Overlay45ResourceDescriptor *gOverlay61Handle58;
extern Overlay45ResourceDescriptor *gOverlay61Handle5C;
extern Overlay45ResourceDescriptor *gOverlay61Handle60;
extern Overlay45ResourceDescriptor *gOverlay61Handle64;
extern Overlay45ResourceDescriptor *gOverlay61Handle68;
extern Overlay45ResourceDescriptor *gOverlay61Handle6C;
extern Overlay45ResourceDescriptor *gOverlay61Handle70;
extern Overlay45ResourceDescriptor *gOverlay61Handle74;
extern Overlay45ResourceDescriptor *gOverlay61Handle78;
extern Overlay45ResourceDescriptor *gOverlay61Handle7C;
extern Overlay45ResourceDescriptor *gOverlay61Handle80;
extern Overlay45ResourceDescriptor *gOverlay61Handle84;
extern Overlay45ResourceDescriptor *gOverlay61Handle88;
extern u32 gOverlay61FreeSpace;
extern s32 gOverlay61EntryCount;
extern s32 gOverlay61Selection;
extern s32 gOverlay61State;
extern s32 gOverlay61Error;
extern s32 gOverlay61ErrorNext;
extern s32 gOverlay61ErrorBack;
extern char *gOverlay61DirNames[16];
extern char *gOverlay61DirExts[16];
extern u32 gOverlay61DirSizes[16];
extern u8 gOverlay61DirTypes[16];
extern Overlay61ListEntry gOverlay61Entries[];
extern char gOverlay61NewFileName[];
extern char gOverlay61GhostLabel[];
extern char gOverlay61FreeLabel[];
extern Overlay61GhostEntry *gOverlay61GhostEntry;
extern void *gDisplayListHead;

extern void overlay61UpdateInput(s32 *x, s32 *y, s32 *confirm, s32 *cancel);
extern void overlay61ResetCounters(void);
extern void overlay61AddEntry(s32 kind, s32 slot, s32 name, s32 ext, s32 size,
                              s32 character, s32 course, s32 time);
extern s32 overlay61ReadCharacter(s32 device, s32 ext, s32 *character,
                                  s32 *course, s32 *time);
extern s32 overlay61RecordSize(Overlay61GhostEntry *entry);
extern void overlay61ChooseFileExtension(char *name);
extern s32 overlay61WriteCharacter(Overlay61GhostEntry *entry, s32 device,
                                   char *name);
extern void overlay61DrawList(void **commands);
extern s32 func_overlay_061_F0001648_18C0A10(Overlay61GhostEntry *entry,
                                             s32 device, u8 *note);
/* Overlay 45 +0x1BE0 (SYMBOL records): the shared descriptor mode setter. */
extern void overlay61SetModeReloc(Overlay45ResourceDescriptor *descriptor,
                                 s32 mode);
extern s32 packDirectory(s32 device, s32 count, char **names, char **exts,
                         u32 *sizes, u8 *types);
extern s32 packFreeSpace(s32 device, u32 *bytes, s32 *files);
extern s32 packDeleteFile(s32 device, s32 slot);
extern void amSndPlay(u16 soundId, void **handle);
extern void mainTitlePageInit(s32 mode);
extern void func_800367A4(void *texture, s32 *animation, s32 mode, f32 *time,
                          s32 updateRate);
extern void func_8002F618(void **commands, void *texture, s32 x, s32 y,
                          u8 red, u8 green, u8 blue, u8 alpha);

/*
 * Controller pak menu. Rewritten from the listing (476 -> 0, lane w2-ovle,
 * 2026-10-02): the two sprites, their animation words and times are six
 * separate objects; the cases carry the sound calls the inherited candidate
 * lacked; the second sprite's frame is updated like the first. The final
 * alpha arguments are narrowed to u8 by func_8002F618's prototype, which is
 * what makes the shipped code re-read each alpha after its test. Frame
 * homes follow declaration order: i and result are the two otherwise unused
 * words above and below the pak character outputs.
 */
void func_overlay_061_F0000B84_18BFF4C(s32 updateRate) {
    s32 i;
    s32 character;
    s32 course;
    s32 time;
    s32 result;
    s32 x;
    s32 y;
    s32 confirm;
    s32 cancel;
    s32 alpha0;
    s32 alpha1;
    u8 *type;

    overlay61UpdateInput(&x, &y, &confirm, &cancel);
    alpha0 = 0;
    alpha1 = 0;
    overlay61SetModeReloc(gOverlay61Handle58, 0xFF);
    overlay61SetModeReloc(gOverlay61Handle5C, 0);
    overlay61SetModeReloc(gOverlay61Handle60, 0);
    overlay61SetModeReloc(gOverlay61Handle64, 0);
    overlay61SetModeReloc(gOverlay61Handle68, 0);
    overlay61SetModeReloc(gOverlay61Handle6C, 0);
    overlay61SetModeReloc(gOverlay61Handle70, 0);
    overlay61SetModeReloc(gOverlay61Handle74, 0);
    overlay61SetModeReloc(gOverlay61Handle78, 0);
    overlay61SetModeReloc(gOverlay61Handle7C, 0);
    overlay61SetModeReloc(gOverlay61Handle80, 0);
    overlay61SetModeReloc(gOverlay61Handle84, 0);
    overlay61SetModeReloc(gOverlay61Handle88, 0);

    switch (gOverlay61State) {
    case 0:
        if (gOverlay61Error == 1 || gOverlay61Error == 8) {
            overlay61SetModeReloc(gOverlay61Handle6C, 0xFF);
        } else if (gOverlay61Error == 4) {
            overlay61SetModeReloc(gOverlay61Handle74, 0xFF);
        } else {
            overlay61SetModeReloc(gOverlay61Handle70, 0xFF);
        }
        overlay61SetModeReloc(gOverlay61Handle80, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle84, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm) {
            gOverlay61State = gOverlay61ErrorNext;
            amSndPlay(0xC, NULL);
        } else if (cancel) {
            gOverlay61State = gOverlay61ErrorBack;
            amSndPlay(0xD, NULL);
        }
        break;
    case 1:
        overlay61SetModeReloc(gOverlay61Handle68, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm || cancel) {
            gOverlay61State = 2;
            amSndPlay(0xC, NULL);
        }
        break;
    case 2:
        result = packDirectory(0, 16, gOverlay61DirNames, gOverlay61DirExts,
                               gOverlay61DirSizes, gOverlay61DirTypes);
        if (result) {
            gOverlay61State = 0;
            gOverlay61Error = result;
            gOverlay61ErrorNext = 2;
            gOverlay61ErrorBack = 7;
            break;
        }
        overlay61ResetCounters();
        type = gOverlay61DirTypes;
        for (i = 0; i < 16; i++) {
            if (*type == 0 && gOverlay61GhostEntry != NULL) {
                if (overlay61ReadCharacter(0, (s32)gOverlay61DirExts[i],
                                           &character, &course, &time) == 0) {
                    overlay61AddEntry(4, i, (s32)gOverlay61DirNames[i],
                                      (s32)gOverlay61DirExts[i],
                                      gOverlay61DirSizes[i], character,
                                      course, time);
                } else {
                    overlay61AddEntry(6, i, (s32)gOverlay61DirNames[i],
                                      (s32)gOverlay61DirExts[i],
                                      gOverlay61DirSizes[i], 0, 0, 0);
                }
            } else if (*type != 0xFF) {
                overlay61AddEntry(6, i, (s32)gOverlay61DirNames[i],
                                  (s32)gOverlay61DirExts[i],
                                  gOverlay61DirSizes[i], 0, 0, 0);
            }
            type++;
        }
        if (gOverlay61GhostEntry != NULL && gOverlay61GhostEntry->course != -1) {
            i = overlay61RecordSize(gOverlay61GhostEntry) + 4;
            if (i & 0xFF) {
                i = (i - (i & 0xFF)) + 0x100;
            }
            overlay61AddEntry(5, 0, (s32)gOverlay61GhostLabel, 0, i,
                              gOverlay61GhostEntry->character,
                              gOverlay61GhostEntry->course,
                              gOverlay61GhostEntry->time);
        }
        packFreeSpace(0, &gOverlay61FreeSpace, NULL);
        overlay61AddEntry(7, 0, (s32)gOverlay61FreeLabel, 0,
                          gOverlay61FreeSpace, 0, 0, 0);
        gOverlay61State = 3;
        break;
    case 3:
        if (cancel) {
            gOverlay61State = 7;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            gOverlay61State = gOverlay61Entries[gOverlay61Selection].nextState;
            amSndPlay(0xC, NULL);
        } else if (y > 0) {
            gOverlay61Selection--;
            if (gOverlay61Selection < 0) {
                gOverlay61Selection = gOverlay61EntryCount - 1;
            }
            amSndPlay(0xF, NULL);
        } else if (y < 0) {
            gOverlay61Selection++;
            if (gOverlay61Selection >= gOverlay61EntryCount) {
                gOverlay61Selection = 0;
            }
            amSndPlay(0xF, NULL);
        }
        break;
    case 4:
        overlay61SetModeReloc(gOverlay61Handle5C, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle78, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            gOverlay61State = 6;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = func_overlay_061_F0001648_18C0A10(
                gOverlay61GhostEntry, 0,
                gOverlay61Entries[gOverlay61Selection].note);
            if (result) {
                gOverlay61State = 0;
                gOverlay61Error = result;
                gOverlay61ErrorNext = 4;
                gOverlay61ErrorBack = 2;
            } else {
                gOverlay61State = 1;
            }
        }
        break;
    case 5:
        overlay61SetModeReloc(gOverlay61Handle60, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle78, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            gOverlay61State = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            overlay61ChooseFileExtension(gOverlay61NewFileName);
            result = overlay61WriteCharacter(gOverlay61GhostEntry, 0,
                                             gOverlay61NewFileName);
            if (result) {
                gOverlay61State = 0;
                gOverlay61Error = result;
                gOverlay61ErrorNext = 5;
                gOverlay61ErrorBack = 2;
            } else {
                gOverlay61State = 1;
            }
        }
        break;
    case 6:
        overlay61SetModeReloc(gOverlay61Handle64, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle78, 0xFF);
        overlay61SetModeReloc(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            gOverlay61State = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = packDeleteFile(0, gOverlay61Entries[gOverlay61Selection].slot);
            if (result) {
                gOverlay61State = 0;
                gOverlay61Error = result;
                gOverlay61ErrorNext = 6;
                gOverlay61ErrorBack = 2;
            } else {
                gOverlay61State = 1;
            }
        }
        break;
    case 7:
        mainTitlePageInit(0);
        gOverlay61State = 8;
        break;
    case 8:
        break;
    }

    if (gOverlay61EntryCount > 0) {
        overlay61DrawList(&gDisplayListHead);
    }
    func_800367A4(gOverlay61Sprite0.texture, &gOverlay61Animation0, 2,
                  &gOverlay61Time0, updateRate);
    gOverlay61Sprite0.frame = gOverlay61Time0 * 65536.0f;
    func_800367A4(gOverlay61Sprite1.texture, &gOverlay61Animation1, 2,
                  &gOverlay61Time1, updateRate);
    gOverlay61Sprite1.frame = gOverlay61Time1 * 65536.0f;
    if (alpha0) {
        func_8002F618(&gDisplayListHead, &gOverlay61Sprite0, 0x24, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha0);
    }
    if (alpha1) {
        func_8002F618(&gDisplayListHead, &gOverlay61Sprite1, 0x11C, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha1);
    }
}
