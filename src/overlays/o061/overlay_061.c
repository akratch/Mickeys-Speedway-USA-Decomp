#include "overlays/overlay_061.h"

typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

/* Tier B: the real resource-group load/free walks terminate at -1. The two
 * texture groups use the resident renderer's node fields and null-texture
 * sentinels, with independent signed animation counters and float times.
 * Descriptor handles and state/error/count words have actual typed writers.
 * Tier D: original names/declarations/qualifiers remain inferred. In
 * particular, the initial packed offsets -12 are retained exactly; they are
 * overwritten before the menu draws. Signed frame encoding is retained before unsigned storage; resident graphics,
 * cursor and asset/lifetime contracts remain separate source/native debt. */
s16 o61ResourceIds[4] = {3, 4, 5, -1};
RcpTextureNode o61ConfirmSprite[2] = {
    {0, 0, -12, -12, 0}, {0, 0, 0, 0, 0},
};
s32 o61ConfirmAnimation = 13;
f32 o61ConfirmTime = 0.0f;
RcpTextureNode o61CancelSprite[2] = {
    {0, 0, -12, -12, 0}, {0, 0, 0, 0, 0},
};
s32 o61CancelAnimation = 13;
f32 o61CancelTime = 0.0f;
Overlay45ResourceDescriptor *o61TitleDescriptor = 0;
Overlay45ResourceDescriptor *o61LoadDescriptor = 0;
Overlay45ResourceDescriptor *o61SaveDescriptor = 0;
Overlay45ResourceDescriptor *o61EraseDescriptor = 0;
Overlay45ResourceDescriptor *o61DoneDescriptor = 0;
Overlay45ResourceDescriptor *o61NoPakDescriptor = 0;
Overlay45ResourceDescriptor *o61PakErrorDescriptor = 0;
Overlay45ResourceDescriptor *o61PakFullDescriptor = 0;
Overlay45ResourceDescriptor *o61YesDescriptor = 0;
Overlay45ResourceDescriptor *o61NoDescriptor = 0;
Overlay45ResourceDescriptor *o61RetryDescriptor = 0;
Overlay45ResourceDescriptor *o61AbortDescriptor = 0;
Overlay45ResourceDescriptor *o61SaveToSgDescriptor = 0;
s32 o61RepeatX = 0;
s32 o61RepeatY = 0;
u32 o61FreeBytes = 0;
s32 o61EntryCount = 0;
s32 o61SelectedEntry = 0;
s32 o61TopEntry = 0;
s32 o61State = 0;
s32 o61Error = 0;
s32 o61ErrorNextState = 0;
s32 o61ErrorBackState = 0;

/* Tier C/D: this distinct stored identifier has a natural ten-byte bound.
 * No active use or original qualification is established; preserve mutable
 * storage. It is not an invented use, reserve or padding declaration. */
char o61PakIdentifier[] = "MSU-GHOST";

/* Read view0..10: ten actual name pointers and a legitimate NULL slot10.
 * Original declaration/capacity is inferred. */
const char *o61CharacterNames[11] = {
    o61MickeyName, o61MinnieName, o61DonaldName, o61DaisyName, o61GoofyName,
    o61PeteName, o61HueyName, o61DueyName, o61LouieName, o61LudwigName, NULL
};

/* packDirectory lends mutable cache slices (18-byte names/6-byte extensions).
 * Use only after success; replacement/free invalidates pointees without
 * clearing these module-owned slots. Shared-cache lifetime coordination remains open. */
/* Tier B: packDirectory receives capacity16 for each output and fills the
 * unused slots through that capacity. The menu inserts at most20 real64-byte
 * entries. Extension selection writes exactly a letter and its terminator.
 * Tier D: original declarations/names remain inferred. Error/formatter/ghost-view and shared-cache lifetime coordination
 * remain separate source/native preconditions. */
char *o61DirectoryNames[16];
char *o61DirectoryExtensions[16];
u32 o61DirectorySizes[16];
u8 o61DirectoryTypes[16];
Overlay61Entry o61MenuEntries[20];
char o61NewFileExtension[2];



/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void overlay61UpdateInput(s32 *xDirection, s32 *yDirection, s32 *confirm, s32 *cancel);
static void overlay61ResetCounters(void);
static void overlay61AddEntry(s32 value0, s32 value1, char *text0, char *text1, s32 value4, s32 slot, s32 value6, s32 totalTime);
static void overlay61DrawEntry(s32 context, s32 y, Overlay61Entry *entry, s32 selected);
static void overlay61DrawList(s32 context);
static s32 func_overlay_061_F0001648_18C0A10(s32 *output, s32 device, char *extension);
static s32 overlay61WriteCharacter(s32 *source, s32 destination, char *extension);
static s32 overlay61ReadCharacter( s32 device, char *extension, s32 *out0, s32 *out1, s32 *out2);
static void overlay61ChooseFileExtension(char *output);
/* Called with and without the record (the definition ends the module). */
static s32 overlay61RecordSize();

static void overlay61UpdateInput(s32 *xDirection, s32 *yDirection, s32 *confirm,
                          s32 *cancel) {
    s32 input;

    if ((joyGetPressed(0) & 0x9000) != 0) {
        *confirm = 1;
    } else {
        *confirm = 0;
    }

    if ((joyGetPressed(0) & 0x4000) != 0) {
        *cancel = 1;
    } else {
        *cancel = 0;
    }

    input = joyGetStickX(0);
    if (input >= 31) {
        if (o61RepeatX > 0) {
            o61RepeatX--;
            *xDirection = 0;
        } else {
            o61RepeatX = 10;
            *xDirection = 1;
        }
    } else if (joyGetStickX(0) < -30) {
        if (o61RepeatX < 0) {
            o61RepeatX++;
            *xDirection = 0;
        } else {
            o61RepeatX = -10;
            *xDirection = -1;
        }
    } else {
        o61RepeatX = 0;
        *xDirection = 0;
    }

    input = joyGetStickY(0);
    if (input >= 31) {
        if (o61RepeatY > 0) {
            o61RepeatY--;
            *yDirection = 0;
        } else {
            o61RepeatY = 10;
            *yDirection = 1;
        }
    } else if (joyGetStickY(0) < -30) {
        if (o61RepeatY < 0) {
            o61RepeatY++;
            *yDirection = 0;
        } else {
            o61RepeatY = -10;
            *yDirection = -1;
        }
    } else {
        o61RepeatY = 0;
        *yDirection = 0;
    }
}

/*
 * Overlay 61 +0x1C0. Fresh DKR v77/v80 searches for the matching consecutive
 * +0x98/+0x9C/+0xA0 zeroing helper were negative.
 */
static void overlay61ResetCounters(void) {
    o61EntryCount = 0;
    o61SelectedEntry = 0;
    o61TopEntry = 0;
}

static void overlay61AddEntry(s32 value0, s32 value1, char *text0, char *text1, s32 value4,
                       s32 slot, s32 value6, s32 totalTime) {
    s32 hours;
    s32 minutes;
    s32 hundredths;
    Overlay61Entry *entry;

    if (o61EntryCount < 20) {
        entry = &o61MenuEntries[o61EntryCount++];

        if (totalTime != -1) {
            hours = totalTime / 3600;
            minutes = (totalTime - (hours * 3600)) / 60;
            hundredths = (((totalTime - (hours * 3600)) - (minutes * 60)) * 100) / 60;
        }

        if ((slot < 0) || (slot >= 11)) {
            slot = 0;
        }

        if (text0 != 0) {
            sprintf((char *)entry->text0, o61CopyNameFormat, text0);
        } else {
            entry->text0[0] = 0;
        }

        if (text1 != 0) {
            sprintf((char *)entry->text1, o61CopyExtensionFormat, text1);
        } else {
            entry->text1[0] = 0;
        }

        entry->kind = value0;
        entry->field2 = value1;
        entry->field3 = slot;
        entry->field4 = value6;
        entry->field5 = hours;
        entry->field6 = minutes;
        entry->field7 = hundredths;
        entry->field3C = value4;
    }
}

static void overlay61DrawEntry(s32 context, s32 y, Overlay61Entry *entry,
                        s32 selected) {
    u32 color;
    char text[128];

    if (entry == 0) {
        frontDrawRectangle(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x200000FF);
        frontDrawRectangle(context, 0x64, y - 2, 0x122, y + 0x14,
                             0x000020FF);
        return;
    }

    if (entry->kind == 5) {
        color = 0x00FF00FF;
    } else if ((entry->kind == 4) || (entry->kind == 6)) {
        color = 0x0000FFFF;
    } else {
        color = 0xFF0000FF;
    }

    if (selected) {
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        frontDrawRectangle(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x800000FF);
        frontDrawRectangle(context, 0x64, y - 2, 0x122, y + 0x14,
                             color & 0x808080FF);
    } else {
        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        frontDrawRectangle(context, 0x1E, y - 2, 0x62, y + 0x14,
                             0x500000FF);
        frontDrawRectangle(context, 0x64, y - 2, 0x122, y + 0x14,
                             color & 0x505050FF);
    }

    switch (entry->kind) {
        case 4:
            sprintf(text, o61GhostSlotFormat, entry->field2);
            fontPrintXY(context, 0x20, y, text, 0);
            /* fall through */
        case 5:
            sprintf(text, o61GhostSizeFormat, entry->field3C);
            fontPrintXY(context, 0x20, y + 9, text, 0);
            sprintf(text, o61GhostNameFormat,
                                 (char *)entry->text0, (char *)entry->text1);
            fontPrintXY(context, 0x66, y, text, 0);
            fontPrintXY(context, 0xD0, y,
                                   (char *)o61CharacterNames[entry->field3], 0);
            fontPrintXY(context, 0x66, y + 9,
                                   levelGetName(entry->field4), 0);
            sprintf(text, o61GhostTimeFormat,
                                 entry->field5, entry->field6, entry->field7);
            fontPrintXY(context, 0xD0, y + 9, text, 0);
            return;

        case 6:
            sprintf(text, o61OtherSlotFormat, entry->field2);
            fontPrintXY(context, 0x20, y, text, 0);
            sprintf(text, o61OtherSizeFormat, entry->field3C);
            fontPrintXY(context, 0x20, y + 9, text, 0);
            sprintf(text, o61OtherNameFormat,
                                 (char *)entry->text0, (char *)entry->text1);
            fontPrintXY(context, 0x66, y + 4, text, 0);
            return;

        default:
            fontPrintXY(context, 0x40, y, (char *)o61PakFreeLabel, 4);
            sprintf(text, o61FreeByteFormat, entry->field3C);
            fontPrintXY(context, 0x40, y + 9, text, 4);
            fontPrintXY(context, 0x66, y + 4, entry->text0, 0);
            return;
    }
}

static void overlay61DrawList(s32 context) {
    s32 index;
    s32 y;
    s32 remaining;

    if (o61SelectedEntry < o61TopEntry) {
        o61TopEntry = o61SelectedEntry;
    } else if (o61SelectedEntry >=
               o61TopEntry + 7) {
        o61TopEntry = o61SelectedEntry - 6;
    }

    fontUseFont(2, &o61TopEntry);
    fontBackground(0, 0, 0, 0);

    index = o61TopEntry;
    y = 0x32;
    /* IDO schedules both relocated bases before the counter on this line. */
    remaining = 6; do { if (index >= o61EntryCount) {
        overlay61DrawEntry(context, y, 0, 0);
        } else {
            overlay61DrawEntry(
                context, y, &o61MenuEntries[index],
                index++ == o61SelectedEntry);
        }
        y += 0x18;
    } while (remaining--);

    fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
    if (o61TopEntry != 0) {
        fontPrintXY(
            context, 0xA0, 0x28, (char *)o61MoreAboveLabel, 4);
    }
    if (o61TopEntry + 7 < o61EntryCount) {
        fontPrintXY(
            context, 0xA0, 0xD8, (char *)o61MoreBelowLabel, 4);
    }
}

/*
 * Overlay 61 +0x968. Fresh DKR v77/v80 source searches for the full
 * (0xA0/0xB0/0xCC, 0x34/0x10C) thirteen-record call sequence and final
 * 2,0,2,7 state tuple were negative. The nearby DKR ghost/controller-pak
 * source remains a semantic navigation lead only, not a donor or naming
 * basis.
 */
void overlay61InitResources(void) {
    loadFrontEndList(o61ResourceIds);
    fontUseFont(3);
    o61TitleDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61PakPageLabel, 0xA0, 0xE, 4);
    o61LoadDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61LoadGhostLabel, 0xA0, 0xB0, 4);
    o61SaveDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61SaveGhostLabel, 0xA0, 0xB0, 4);
    o61EraseDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61EraseFileLabel, 0xA0, 0xB0, 4);
    o61DoneDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61TaskDoneLabel, 0xA0, 0xCC, 4);
    o61NoPakDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61NoPakLabel, 0xA0, 0xB0, 4);
    o61PakErrorDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61PakErrorLabel, 0xA0, 0xB0, 4);
    o61PakFullDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61PakFullLabel, 0xA0, 0xB0, 4);
    o61YesDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61YesLabel, 0x34, 0xCC, 0);
    o61NoDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61NoLabel, 0x10C, 0xCC, 1);
    o61RetryDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61RetryLabel, 0x34, 0xCC, 0);
    o61AbortDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61AbortLabel, 0x10C, 0xCC, 1);
    o61SaveToSgDescriptor = (Overlay45ResourceDescriptor *)overlay45CreateDescriptor(o61SaveToSgLabel, 0xA0, 0xB0, 4);
    o61ConfirmSprite[0].texture = (RcpTextureInfo *)D_800D31C8[4];
    o61CancelSprite[0].texture = (RcpTextureInfo *)D_800D31C8[5];
    o61State = 2;
    o61Error = 0;
    o61ErrorNextState = 2;
    o61ErrorBackState = 7;
    rumbleProcessing(0, 2);
}

/* Overlay 68's primary entry header: the ghost the menu can save. */  /* (was on typedef Overlay61GhostEntry) */
/* Overlay 45 +0x1BE0 (SYMBOL records): the shared descriptor mode setter. */  /* (was on extern overlay45SetMode) */
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
    overlay45SetMode(o61TitleDescriptor, 0xFF);
    overlay45SetMode(o61LoadDescriptor, 0);
    overlay45SetMode(o61SaveDescriptor, 0);
    overlay45SetMode(o61EraseDescriptor, 0);
    overlay45SetMode(o61DoneDescriptor, 0);
    overlay45SetMode(o61NoPakDescriptor, 0);
    overlay45SetMode(o61PakErrorDescriptor, 0);
    overlay45SetMode(o61PakFullDescriptor, 0);
    overlay45SetMode(o61YesDescriptor, 0);
    overlay45SetMode(o61NoDescriptor, 0);
    overlay45SetMode(o61RetryDescriptor, 0);
    overlay45SetMode(o61AbortDescriptor, 0);
    overlay45SetMode(o61SaveToSgDescriptor, 0);

    switch (o61State) {
    case 0:
        if (o61Error == 1 || o61Error == 8) {
            overlay45SetMode(o61NoPakDescriptor, 0xFF);
        } else if (o61Error == 4) {
            overlay45SetMode(o61PakFullDescriptor, 0xFF);
        } else {
            overlay45SetMode(o61PakErrorDescriptor, 0xFF);
        }
        overlay45SetMode(o61RetryDescriptor, 0xFF);
        overlay45SetMode(o61AbortDescriptor, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm) {
            o61State = o61ErrorNextState;
            amSndPlay(0xC, NULL);
        } else if (cancel) {
            o61State = o61ErrorBackState;
            amSndPlay(0xD, NULL);
        }
        break;
    case 1:
        overlay45SetMode(o61DoneDescriptor, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm || cancel) {
            o61State = 2;
            amSndPlay(0xC, NULL);
        }
        break;
    case 2:
        result = packDirectory(0, 16, o61DirectoryNames, o61DirectoryExtensions,
                               o61DirectorySizes, o61DirectoryTypes);
        if (result) {
            o61State = 0;
            o61Error = result;
            o61ErrorNextState = 2;
            o61ErrorBackState = 7;
            break;
        }
        overlay61ResetCounters();
        type = o61DirectoryTypes;
        for (i = 0; i < 16; i++) {
            if (*type == 0 && D_8007A1F4 != NULL) {
                if (overlay61ReadCharacter(0, o61DirectoryExtensions[i],
                                           &character, &course, &time) == 0) {
                    overlay61AddEntry(4, i, o61DirectoryNames[i],
                                      o61DirectoryExtensions[i],
                                      o61DirectorySizes[i], character,
                                      course, time);
                } else {
                    overlay61AddEntry(6, i, o61DirectoryNames[i],
                                      o61DirectoryExtensions[i],
                                      o61DirectorySizes[i], 0, 0, 0);
                }
            } else if (*type != 0xFF) {
                overlay61AddEntry(6, i, o61DirectoryNames[i],
                                  o61DirectoryExtensions[i],
                                  o61DirectorySizes[i], 0, 0, 0);
            }
            type++;
        }
        if (D_8007A1F4 != NULL && D_8007A1F4->course != -1) {
            i = overlay61RecordSize(D_8007A1F4) + 4;
            if (i & 0xFF) {
                i = (i - (i & 0xFF)) + 0x100;
            }
            overlay61AddEntry(5, 0, (char *)o61NewGhostName, 0, i,
                              D_8007A1F4->character,
                              D_8007A1F4->course,
                              D_8007A1F4->time);
        }
        packFreeSpace(0, &o61FreeBytes, NULL);
        overlay61AddEntry(7, 0, (char *)o61ExitLabel, 0,
                          o61FreeBytes, 0, 0, 0);
        o61State = 3;
        break;
    case 3:
        if (cancel) {
            o61State = 7;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            o61State = o61MenuEntries[o61SelectedEntry].kind;
            amSndPlay(0xC, NULL);
        } else if (y > 0) {
            o61SelectedEntry--;
            if (o61SelectedEntry < 0) {
                o61SelectedEntry = o61EntryCount - 1;
            }
            amSndPlay(0xF, NULL);
        } else if (y < 0) {
            o61SelectedEntry++;
            if (o61SelectedEntry >= o61EntryCount) {
                o61SelectedEntry = 0;
            }
            amSndPlay(0xF, NULL);
        }
        break;
    case 4:
        overlay45SetMode(o61LoadDescriptor, 0xFF);
        overlay45SetMode(o61YesDescriptor, 0xFF);
        overlay45SetMode(o61NoDescriptor, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State = 6;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = func_overlay_061_F0001648_18C0A10(
                D_8007A1F4, 0,
                (char *)o61MenuEntries[o61SelectedEntry].text1);
            if (result) {
                o61State = 0;
                o61Error = result;
                o61ErrorNextState = 4;
                o61ErrorBackState = 2;
            } else {
                o61State = 1;
            }
        }
        break;
    case 5:
        overlay45SetMode(o61SaveDescriptor, 0xFF);
        overlay45SetMode(o61YesDescriptor, 0xFF);
        overlay45SetMode(o61NoDescriptor, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            overlay61ChooseFileExtension(o61NewFileExtension);
            result = overlay61WriteCharacter((s32 *)D_8007A1F4, 0,
                                             o61NewFileExtension);
            if (result) {
                o61State = 0;
                o61Error = result;
                o61ErrorNextState = 5;
                o61ErrorBackState = 2;
            } else {
                o61State = 1;
            }
        }
        break;
    case 6:
        overlay45SetMode(o61EraseDescriptor, 0xFF);
        overlay45SetMode(o61YesDescriptor, 0xFF);
        overlay45SetMode(o61NoDescriptor, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = packDeleteFile(0, o61MenuEntries[o61SelectedEntry].field2);
            if (result) {
                o61State = 0;
                o61Error = result;
                o61ErrorNextState = 6;
                o61ErrorBackState = 2;
            } else {
                o61State = 1;
            }
        }
        break;
    case 7:
        mainTitlePageInit(0);
        o61State = 8;
        break;
    case 8:
        break;
    }

    if (o61EntryCount > 0) {
        overlay61DrawList((s32)&D_800D3140);
    }
    texAnimateTexSprite(o61ConfirmSprite[0].texture, &o61ConfirmAnimation, 2,
                  &o61ConfirmTime, updateRate);
    o61ConfirmSprite[0].packedOffset = (s32)(o61ConfirmTime * 65536.0f);
    texAnimateTexSprite(o61CancelSprite[0].texture, &o61CancelAnimation, 2,
                  &o61CancelTime, updateRate);
    o61CancelSprite[0].packedOffset = (s32)(o61CancelTime * 65536.0f);
    if (alpha0) {
        func_8002F618(&D_800D3140, o61ConfirmSprite, 0x24, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha0);
    }
    if (alpha1) {
        func_8002F618(&D_800D3140, o61CancelSprite, 0x11C, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha1);
    }
}

void overlay61ReleaseResources(void) {
    freeFrontEndList(o61ResourceIds);
    overlay45ReleaseDescriptor(o61TitleDescriptor);
    overlay45ReleaseDescriptor(o61LoadDescriptor);
    overlay45ReleaseDescriptor(o61SaveDescriptor);
    overlay45ReleaseDescriptor(o61EraseDescriptor);
    overlay45ReleaseDescriptor(o61DoneDescriptor);
    overlay45ReleaseDescriptor(o61NoPakDescriptor);
    overlay45ReleaseDescriptor(o61PakErrorDescriptor);
    overlay45ReleaseDescriptor(o61PakFullDescriptor);
    overlay45ReleaseDescriptor(o61YesDescriptor);
    overlay45ReleaseDescriptor(o61NoDescriptor);
    overlay45ReleaseDescriptor(o61RetryDescriptor);
    overlay45ReleaseDescriptor(o61AbortDescriptor);
    overlay45ReleaseDescriptor(o61SaveToSgDescriptor);
    packDirectoryFree();
    rumbleProcessing(1);
}

/*
 * Exact 92-word match. The one-iteration do/while grouping is semantically
 * inert but preserves IDO's shipped caller-saved allocation; track a cleaner
 * spelling in the cleanup queue.
 */
static s32 func_overlay_061_F0001648_18C0A10(s32 *output, s32 device, char *extension) {
    s32 copySize;
    s32 result;
    s32 size;
    s32 fileNumber;
    s32 *buffer;
    s32 *src;
    s32 *dst;

    result = packOpen(device);
    if (result == 0) {
        result = packOpenFile(
            device, (char *)o61LoadGhostPath, extension, &fileNumber);
        if (result == 0) {
            result = packFileSize(device, fileNumber, &size);
            if (size >= 4) {
                buffer = mmAlloc(size, 0x85);
                if (buffer != 0) {
                    result = packReadFile(device, fileNumber, (u8 *)buffer, size);
                    if (result == 0) {
                        if (buffer[0] == 0x43484152) {
                            copySize = size - 4;
                            do {
                                if (overlay68PayloadLimit(copySize) < copySize) {
                                    copySize = overlay68PayloadLimit(copySize);
                                }
                                copySize = (copySize + 3) >> 2;
                                src = buffer + 1;
                                dst = output;
                                while (copySize--) {
                                    *dst++ = *src++;
                                }
                            } while (0);
                            output[3] = (s32)(output + 4);
                        } else {
                            result = 5;
                        }
                    }
                    mmFree(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    packClose(device);
    return result;
}

static s32 overlay61WriteCharacter(s32 *source, s32 destination, char *extension) {
    s32 size;
    s32 words;
    s32 result;
    s32 *buffer;

    size = overlay61RecordSize() + 4;
    if (size % 32 != 0) {
        size = (size - (size % 32)) + 32;
    }

    buffer = mmAlloc(size, 0x85);
    if (buffer != 0) {
        s32 count;
        s32 *src;
        s32 *dst;

        buffer[0] = 0x43484152;
        words = (size - 1) >> 2;
        src = source;
        dst = buffer + 1;
        count = words - 1;
        if (words != 0) {
            do {
                *dst++ = *src++;
            } while (count--);
        }
        result = packWriteFile(
            destination, -1, (char *)o61WriteGhostFormat, extension, (u8 *)buffer, size);
        mmFree(buffer);
    } else {
        result = 6;
    }
    return result;
}

/* IDO 5.3 -O2 -mips2 reproduces 0x110 bytes; -mips1 grows them to 0x11C. */  /* (was on typedef Overlay61RecordHeader) */
static s32 overlay61ReadCharacter(
    s32 device, char *extension, s32 *out0, s32 *out1, s32 *out2) {
    s32 result;
    s32 size;
    s32 fileNumber;
    Overlay61RecordHeader *buffer;
    volatile s32 padding[2];

    result = packOpen();
    if (result == 0) {
        result = packOpenFile(
            device, (char *)o61ReadGhostPath, extension, &fileNumber);
        if (result == 0) {
            result = packFileSize(device, fileNumber, &size);
            if (size >= 0x40) {
                buffer = mmAlloc(0x40, 0x85);
                if (buffer != 0) {
                    result = packReadFile(device, fileNumber, (u8 *)buffer, 0x40);
                    if (result == 0) {
                        if (buffer->magic == 0x43484152) {
                            *out0 = buffer->field8;
                            *out1 = buffer->fieldA;
                            *out2 = buffer->fieldC;
                        } else {
                            result = 5;
                        }
                    }
                    mmFree(buffer);
                }
            } else {
                result = 6;
            }
        }
    }
    packClose(device);
    return result;
}

/* DKR save_data.c has the same controller-pak extension-selection semantics,
 * but pinned DKR v77/v80 and JFG contain no exact object donor. */
static void overlay61ChooseFileExtension(char *output) {
    s32 i;
    u8 used[26];

    for (i = 0; i < 26; i++) {
        used[i] = 0;
    }

    for (i = 0; i < 16; i++) {
        if (o61DirectoryTypes[i] == 0) {
            if ((o61DirectoryExtensions[i][0] >= 'A') &&
                (o61DirectoryExtensions[i][0] <= 'Z')) {
                used[o61DirectoryExtensions[i][0] - 'A'] = 1;
            }
        }
    }

    i = 0;
    while (used[i] != 0) {
        i++;
    }
    output[0] = i + 'A';
    output[1] = '\0';
}

static s32 overlay61RecordSize(Overlay61RecordHeader *record) {
    return (record->fieldA * 10) + 0x10;
}

/* Tier B/C: these real literals are read by the font conversion/copy path,
 * descriptor allocation/copy, sprintf formats, or pak filename conversion.
 * The complete current consumer chains do not write or retain their inputs.
 * Tier D: original const declarations remain inferred. External callable/cursor/ghost-view contracts remain separate source/native debt.
 * Their natural RO context also supplies the following compiler switch table. */
const char o61MickeyName[] = "Mickey";
const char o61MinnieName[] = "Minnie";
const char o61DonaldName[] = "Donald";
const char o61DaisyName[] = "Daisy";
const char o61GoofyName[] = "Goofy";
const char o61PeteName[] = "Pete";
const char o61HueyName[] = "Huey";
const char o61DueyName[] = "Duey";
const char o61LouieName[] = "Louie";
const char o61LudwigName[] = "Ludwig";
const char o61CopyNameFormat[] = "%s";
const char o61CopyExtensionFormat[] = "%s";
const char o61GhostSlotFormat[] = "Slot : %d";
const char o61GhostSizeFormat[] = "Size : %d";
const char o61GhostNameFormat[] = "%s.%s";
const char o61GhostTimeFormat[] = "Time: %02d:%02d.%02d\n";
const char o61OtherSlotFormat[] = "Slot : %d";
const char o61OtherSizeFormat[] = "Size : %d";
const char o61OtherNameFormat[] = "%s.%s";
const char o61PakFreeLabel[] = "Pak Free :";
const char o61FreeByteFormat[] = "%d";
const char o61MoreAboveLabel[] = "MORE";
const char o61MoreBelowLabel[] = "MORE";
const char o61PakPageLabel[] = "PAK PAGE";
const char o61LoadGhostLabel[] = "LOAD GHOST";
const char o61SaveGhostLabel[] = "SAVE GHOST";
const char o61EraseFileLabel[] = "ERASE FILE";
const char o61TaskDoneLabel[] = "TASK DONE";
const char o61NoPakLabel[] = "NO PAK";
const char o61PakErrorLabel[] = "PAK ERROR";
const char o61PakFullLabel[] = "PAK FULL";
const char o61YesLabel[] = "YES";
const char o61NoLabel[] = "NO";
const char o61RetryLabel[] = "RETRY";
const char o61AbortLabel[] = "ABORT";
const char o61SaveToSgLabel[] = "SAVE TO SG";
const char o61NewGhostName[] = "SAVE GHOST";
const char o61ExitLabel[] = "EXIT";
const char o61LoadGhostPath[] = "MSU-GHOST";
const char o61WriteGhostFormat[] = "MSU-GHOST";
const char o61ReadGhostPath[] = "MSU-GHOST";
