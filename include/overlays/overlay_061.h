#ifndef OVERLAY_061_H
#define OVERLAY_061_H

#include "ultra64.h"
#include "overlays/overlay_045.h"
#include "overlays/o045/resource_descriptor.h"

/* Overlay 61: the controller-pak ghost save menu. */

/* Tier B/D: current ownership/accesses establish this 64-byte view; the
 * original declaration is inferred. Formatting may cross text1[4] into field3C.
 * Sentinel-time initialization, failed read allocations and formatter-state
 * assumptions remain source/native debt. */
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

typedef struct Overlay61GhostEntry {
    /* 0x00 */ s32 active;
    /* 0x04 */ s8 character;
    /* 0x05 */ u8 timer;
    /* 0x06 */ s16 course;
    /* 0x08 */ s16 time;
} Overlay61GhostEntry;

typedef struct Overlay61RecordHeader {
    u32 magic;
    u8 pad4[4];
    s8 field8;
    u8 pad9;
    s16 fieldA; /* the record count */
    s16 fieldC;
} Overlay61RecordHeader;


/* Current source-owned literals, in their authenticated declaration order. */
extern const char o61MickeyName[];
extern const char o61MinnieName[];
extern const char o61DonaldName[];
extern const char o61DaisyName[];
extern const char o61GoofyName[];
extern const char o61PeteName[];
extern const char o61HueyName[];
extern const char o61DueyName[];
extern const char o61LouieName[];
extern const char o61LudwigName[];
extern const char o61CopyNameFormat[];
extern const char o61CopyExtensionFormat[];
extern const char o61GhostSlotFormat[];
extern const char o61GhostSizeFormat[];
extern const char o61GhostNameFormat[];
extern const char o61GhostTimeFormat[];
extern const char o61OtherSlotFormat[];
extern const char o61OtherSizeFormat[];
extern const char o61OtherNameFormat[];
extern const char o61PakFreeLabel[];
extern const char o61FreeByteFormat[];
extern const char o61MoreAboveLabel[];
extern const char o61MoreBelowLabel[];
extern const char o61PakPageLabel[];
extern const char o61LoadGhostLabel[];
extern const char o61SaveGhostLabel[];
extern const char o61EraseFileLabel[];
extern const char o61TaskDoneLabel[];
extern const char o61NoPakLabel[];
extern const char o61PakErrorLabel[];
extern const char o61PakFullLabel[];
extern const char o61YesLabel[];
extern const char o61NoLabel[];
extern const char o61RetryLabel[];
extern const char o61AbortLabel[];
extern const char o61SaveToSgLabel[];
extern const char o61NewGhostName[];
extern const char o61ExitLabel[];
extern const char o61LoadGhostPath[];
extern const char o61WriteGhostFormat[];
extern const char o61ReadGhostPath[];

extern s32 joyGetPressed(s32 controller);
extern s32 joyGetStickX(s32 controller);
extern s32 joyGetStickY(s32 controller);
extern s32 sprintf(char *buffer, const char *format, ...);
extern void fontColour(s32, s32, s32, s32, s32);
extern void frontDrawRectangle(s32, s32, s32, s32, s32, u32);
extern void fontPrintXY(s32, s32, s32, void *, s32);
extern void *levelGetName(s8);
/* Called with one and with two arguments. */
extern void fontUseFont();
extern void fontBackground(s32, s32, s32, s32);
/* Real resident pointer-bank footprint; resident storage/native ownership remains open. */
struct MenuFrontObject;
extern struct MenuFrontObject *D_800D31C8[];
extern void loadFrontEndList(s16 *data);
/* Called with one and with two arguments. */
extern void rumbleProcessing();
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
extern void freeFrontEndList(s16 *);
extern void packDirectoryFree(void);
/* Called with and without the device argument. */
extern s32 packOpen();
extern s32 packOpenFile(s32 device, char *name, char *extension, s32 *fileNumber);
extern s32 packFileSize(s32 device, s32 fileNumber, s32 *size);
extern void *mmAlloc(s32 size, s32 tag);
extern s32 packReadFile(s32 device, s32 fileNumber, u8 *data, s32 size);
extern s32 overlay68PayloadLimit(s32);
extern void mmFree(void *);
extern void packClose(s32);
extern s32 packWriteFile(
    s32 device, s32 fileNumber, char *name, char *extension, u8 *data, s32 size);

/* This module's exports (ORT rows). */
void overlay61InitResources(void);
void func_overlay_061_F0000B84_18BFF4C(s32 updateRate);
void overlay61ReleaseResources(void);

#endif
