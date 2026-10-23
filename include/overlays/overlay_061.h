#ifndef OVERLAY_061_H
#define OVERLAY_061_H

#include "ultra64.h"
#include "overlays/overlay_045.h"
#include "overlays/o045/resource_descriptor.h"

/* Overlay 61: the controller-pak ghost save menu. */

typedef struct Overlay61Entry {
    s16 kind;
    u8 field2;
    u8 field3;
    s8 field4;
    u8 field5;
    u8 field6;
    u8 field7;
    u8 text0[0x30];
    u8 text1[4];
    s32 field3C;
} Overlay61Entry;

typedef struct Overlay61Sprite {
    /* 0x00 */ void *texture;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 frame;
    /* 0x0C */ u8 pad0C[0x14];
} Overlay61Sprite;

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

typedef struct Overlay61RecordHeader {
    u32 magic;
    u8 pad4[4];
    s8 field8;
    u8 pad9;
    s16 fieldA; /* the record count */
    s16 fieldC;
} Overlay61RecordHeader;

extern s32 gOverlay61RepeatXReloc;
extern s32 gOverlay61RepeatYReloc;
extern s32 joyGetPressed(s32 controller);
extern s32 joyGetStickX(s32 controller);
extern s32 joyGetStickY(s32 controller);
extern s32 gOverlay61Counter98;
extern s32 gOverlay61Counter9C;
extern s32 gOverlay61CounterA0;
extern s32 gOverlay61EntryCountReloc;
extern Overlay61Entry gOverlay61EntriesReloc[];
extern u8 gOverlay61Text0Reloc[];
extern u8 gOverlay61Text1Reloc[];
extern s32 sprintf(u8 *, u8 *, ...);
extern u8 gOverlay61Format58Reloc[];
extern u8 gOverlay61Format64Reloc[];
extern u8 gOverlay61Format70Reloc[];
extern u8 gOverlay61Format78Reloc[];
extern u8 gOverlay61Format90Reloc[];
extern u8 gOverlay61Format9CReloc[];
extern u8 gOverlay61FormatA8Reloc[];
extern u8 gOverlay61LabelB0Reloc[];
extern u8 gOverlay61FormatBCReloc[];
extern void *gOverlay61NamesReloc[];
extern void fontColour(s32, s32, s32, s32, s32);
extern void frontDrawRectangle(s32, s32, s32, s32, s32, u32);
extern void fontPrintXY(s32, s32, s32, void *, s32);
extern void *levelGetName(s8);
extern s32 gOverlay61ItemCountReloc;
extern s32 gOverlay61SelectedIndexReloc;
extern s32 gOverlay61TopIndexReloc;
extern u8 gOverlay61ItemsReloc[];
extern u8 gOverlay61UpLabelReloc[];
extern u8 gOverlay61DownLabelReloc[];
/* Called with one and with two arguments. */
extern void fontUseFont();
extern void fontBackground(s32, s32, s32, s32);
extern u8 gOverlay61SetupData[];
extern u8 gOverlay61RecordD0[];
extern u8 gOverlay61RecordDC[];
extern u8 gOverlay61RecordE8[];
extern u8 gOverlay61RecordF4[];
extern u8 gOverlay61Record100[];
extern u8 gOverlay61Record10C[];
extern u8 gOverlay61Record114[];
extern u8 gOverlay61Record120[];
extern u8 gOverlay61Record12C[];
extern u8 gOverlay61Record130[];
extern u8 gOverlay61Record134[];
extern u8 gOverlay61Record13C[];
extern u8 gOverlay61Record144[];
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
extern s32 gOverlay61Config08;
extern s32 gOverlay61Config30;
extern s32 gOverlay61StateA4;
extern s32 gOverlay61StateA8;
extern s32 gOverlay61StateAC;
extern s32 gOverlay61StateB0;
extern s32 D_800D31C8[];
extern void loadFrontEndList(void *data);
extern Overlay45ResourceDescriptor *overlay61CreateDescriptorReloc(
    void *data, s32 width, s32 height, s32 flags);
/* Called with one and with two arguments. */
extern void rumbleProcessing();
extern Overlay61Sprite gOverlay61Sprite0;
extern s32 gOverlay61Animation0;
extern f32 gOverlay61Time0;
extern Overlay61Sprite gOverlay61Sprite1;
extern s32 gOverlay61Animation1;
extern f32 gOverlay61Time1;
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
extern Overlay61GhostEntry *D_8007A1F4;
extern void *D_800D3140;
extern void overlay45SetMode(Overlay45ResourceDescriptor *descriptor,
                                 s32 mode);
extern s32 packDirectory(s32 device, s32 count, char **names, char **exts,
                         u32 *sizes, u8 *types);
extern s32 packFreeSpace(s32 device, u32 *bytes, s32 *files);
extern s32 packDeleteFile(s32 device, s32 slot);
extern void amSndPlay(u16 soundId, void **handle);
extern void mainTitlePageInit(s32 mode);
extern void texAnimateTexSprite(void *texture, s32 *animation, s32 mode, f32 *time,
                          s32 updateRate);
extern void func_8002F618(void **commands, void *texture, s32 x, s32 y,
                          u8 red, u8 green, u8 blue, u8 alpha);
extern u8 gOverlay61ResourceBaseReloc[];
extern void *gOverlay61Resource58Reloc;
extern void *gOverlay61Resource5CReloc;
extern void *gOverlay61Resource60Reloc;
extern void *gOverlay61Resource64Reloc;
extern void *gOverlay61Resource68Reloc;
extern void *gOverlay61Resource6CReloc;
extern void *gOverlay61Resource70Reloc;
extern void *gOverlay61Resource74Reloc;
extern void *gOverlay61Resource78Reloc;
extern void *gOverlay61Resource7CReloc;
extern void *gOverlay61Resource80Reloc;
extern void *gOverlay61Resource84Reloc;
extern void *gOverlay61Resource88Reloc;
extern void freeFrontEndList(void *);
extern void packDirectoryFree(void);
extern u8 gOverlay61SavePathReloc[];
/* Called with and without the device argument. */
extern s32 packOpen();
extern s32 packOpenFile(s32 device, void *path, s32 *arg2, void **handle);
extern s32 packFileSize(s32, void *, s32 *);
extern void *mmAlloc(s32 size, s32 tag);
extern s32 packReadFile(s32, void *, void *, s32);
extern s32 overlay68PayloadLimit(s32);
extern void mmFree(void *);
extern void packClose(s32);
extern s32 packWriteFile(
    s32 arg0, s32 arg1, void *format, s32 arg3, void *data, s32 size);
extern u8 gOverlay61RecordFormatReloc[];
extern u8 gOverlay61RecordPathReloc[];
extern char *gOverlay61FileNames[];
extern u8 gOverlay61FileStatus[];

/* This module's exports (ORT rows). */
void overlay61InitResources(void);
void func_overlay_061_F0000B84_18BFF4C(s32 updateRate);
void overlay61ReleaseResources(void);

#endif
