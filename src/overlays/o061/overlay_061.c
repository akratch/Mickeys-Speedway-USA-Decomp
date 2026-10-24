#include "overlays/overlay_061.h"

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
        if (o61RepeatX_o061Reloc > 0) {
            o61RepeatX_o061Reloc--;
            *xDirection = 0;
        } else {
            o61RepeatX_o061Reloc = 10;
            *xDirection = 1;
        }
    } else if (joyGetStickX(0) < -30) {
        if (o61RepeatX_o061Reloc < 0) {
            o61RepeatX_o061Reloc++;
            *xDirection = 0;
        } else {
            o61RepeatX_o061Reloc = -10;
            *xDirection = -1;
        }
    } else {
        o61RepeatX_o061Reloc = 0;
        *xDirection = 0;
    }

    input = joyGetStickY(0);
    if (input >= 31) {
        if (o61RepeatY_o061Reloc > 0) {
            o61RepeatY_o061Reloc--;
            *yDirection = 0;
        } else {
            o61RepeatY_o061Reloc = 10;
            *yDirection = 1;
        }
    } else if (joyGetStickY(0) < -30) {
        if (o61RepeatY_o061Reloc < 0) {
            o61RepeatY_o061Reloc++;
            *yDirection = 0;
        } else {
            o61RepeatY_o061Reloc = -10;
            *yDirection = -1;
        }
    } else {
        o61RepeatY_o061Reloc = 0;
        *yDirection = 0;
    }
}

/*
 * Overlay 61 +0x1C0. Fresh DKR v77/v80 searches for the matching consecutive
 * +0x98/+0x9C/+0xA0 zeroing helper were negative.
 */
static void overlay61ResetCounters(void) {
    o61EntryCount_o061Reloc = 0;
    o61SelectedEntry_o061Reloc = 0;
    gOverlay61CounterA0 = 0;
}

static void overlay61AddEntry(s32 value0, s32 value1, char *text0, char *text1, s32 value4,
                       s32 slot, s32 value6, s32 totalTime) {
    s32 hours;
    s32 minutes;
    s32 hundredths;
    Overlay61Entry *entry;

    if (o61EntryCount_o061Reloc < 20) {
        entry = &o61MenuEntries_o061Reloc[o61EntryCount_o061Reloc++];

        if (totalTime != -1) {
            hours = totalTime / 3600;
            minutes = (totalTime - (hours * 3600)) / 60;
            hundredths = (((totalTime - (hours * 3600)) - (minutes * 60)) * 100) / 60;
        }

        if ((slot < 0) || (slot >= 11)) {
            slot = 0;
        }

        if (text0 != 0) {
            sprintf((char *)entry->text0, gOverlay61Text0Reloc, text0);
        } else {
            entry->text0[0] = 0;
        }

        if (text1 != 0) {
            sprintf((char *)entry->text1, gOverlay61Text1Reloc, text1);
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
            sprintf(text, gOverlay61Format58Reloc, entry->field2);
            fontPrintXY(context, 0x20, y, text, 0);
            /* fall through */
        case 5:
            sprintf(text, gOverlay61Format64Reloc, entry->field3C);
            fontPrintXY(context, 0x20, y + 9, text, 0);
            sprintf(text, gOverlay61Format70Reloc,
                                 (char *)entry->text0, (char *)entry->text1);
            fontPrintXY(context, 0x66, y, text, 0);
            fontPrintXY(context, 0xD0, y,
                                   gOverlay61NamesReloc[entry->field3], 0);
            fontPrintXY(context, 0x66, y + 9,
                                   levelGetName(entry->field4), 0);
            sprintf(text, gOverlay61Format78Reloc,
                                 entry->field5, entry->field6, entry->field7);
            fontPrintXY(context, 0xD0, y + 9, text, 0);
            return;

        case 6:
            sprintf(text, gOverlay61Format90Reloc, entry->field2);
            fontPrintXY(context, 0x20, y, text, 0);
            sprintf(text, gOverlay61Format9CReloc, entry->field3C);
            fontPrintXY(context, 0x20, y + 9, text, 0);
            sprintf(text, gOverlay61FormatA8Reloc,
                                 (char *)entry->text0, (char *)entry->text1);
            fontPrintXY(context, 0x66, y + 4, text, 0);
            return;

        default:
            fontPrintXY(context, 0x40, y, gOverlay61LabelB0Reloc, 4);
            sprintf(text, gOverlay61FormatBCReloc, entry->field3C);
            fontPrintXY(context, 0x40, y + 9, text, 4);
            fontPrintXY(context, 0x66, y + 4, entry->text0, 0);
            return;
    }
}

static void overlay61DrawList(s32 context) {
    s32 index;
    s32 y;
    s32 remaining;

    if (o61SelectedEntry_o061Reloc < gOverlay61TopIndexReloc) {
        gOverlay61TopIndexReloc = o61SelectedEntry_o061Reloc;
    } else if (o61SelectedEntry_o061Reloc >=
               gOverlay61TopIndexReloc + 7) {
        gOverlay61TopIndexReloc = o61SelectedEntry_o061Reloc - 6;
    }

    fontUseFont(2, &gOverlay61TopIndexReloc);
    fontBackground(0, 0, 0, 0);

    index = gOverlay61TopIndexReloc;
    y = 0x32;
    /* IDO schedules both relocated bases before the counter on this line. */
    remaining = 6; do { if (index >= o61EntryCount_o061Reloc) {
        overlay61DrawEntry(context, y, 0, 0);
        } else {
            overlay61DrawEntry(
                context, y, &o61MenuEntries_o061Reloc[index],
                index++ == o61SelectedEntry_o061Reloc);
        }
        y += 0x18;
    } while (remaining--);

    fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
    if (gOverlay61TopIndexReloc != 0) {
        fontPrintXY(
            context, 0xA0, 0x28, gOverlay61UpLabelReloc, 4);
    }
    if (gOverlay61TopIndexReloc + 7 < o61EntryCount_o061Reloc) {
        fontPrintXY(
            context, 0xA0, 0xD8, gOverlay61DownLabelReloc, 4);
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
    loadFrontEndList(o61ResourceIds_o061Reloc);
    fontUseFont(3);
    gOverlay61Handle58 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61RecordD0, 0xA0, 0xE, 4);
    gOverlay61Handle5C = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61RecordDC, 0xA0, 0xB0, 4);
    gOverlay61Handle60 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61RecordE8, 0xA0, 0xB0, 4);
    gOverlay61Handle64 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61RecordF4, 0xA0, 0xB0, 4);
    gOverlay61Handle68 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record100, 0xA0, 0xCC, 4);
    gOverlay61Handle6C = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record10C, 0xA0, 0xB0, 4);
    gOverlay61Handle70 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record114, 0xA0, 0xB0, 4);
    gOverlay61Handle74 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record120, 0xA0, 0xB0, 4);
    gOverlay61Handle78 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record12C, 0x34, 0xCC, 0);
    gOverlay61Handle7C = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record130, 0x10C, 0xCC, 1);
    gOverlay61Handle80 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record134, 0x34, 0xCC, 0);
    gOverlay61Handle84 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record13C, 0x10C, 0xCC, 1);
    gOverlay61Handle88 = (Overlay45ResourceDescriptor *)overlay61CreateDescriptorReloc(gOverlay61Record144, 0xA0, 0xB0, 4);
    gOverlay61Config08 = D_800D31C8[4];
    gOverlay61Config30 = D_800D31C8[5];
    o61State_o061Reloc = 2;
    o61Error_o061Reloc = 0;
    o61ErrorNextState_o061Reloc = 2;
    o61ErrorBackState_o061Reloc = 7;
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
    overlay45SetMode(gOverlay61Handle58, 0xFF);
    overlay45SetMode(gOverlay61Handle5C, 0);
    overlay45SetMode(gOverlay61Handle60, 0);
    overlay45SetMode(gOverlay61Handle64, 0);
    overlay45SetMode(gOverlay61Handle68, 0);
    overlay45SetMode(gOverlay61Handle6C, 0);
    overlay45SetMode(gOverlay61Handle70, 0);
    overlay45SetMode(gOverlay61Handle74, 0);
    overlay45SetMode(gOverlay61Handle78, 0);
    overlay45SetMode(gOverlay61Handle7C, 0);
    overlay45SetMode(gOverlay61Handle80, 0);
    overlay45SetMode(gOverlay61Handle84, 0);
    overlay45SetMode(gOverlay61Handle88, 0);

    switch (o61State_o061Reloc) {
    case 0:
        if (o61Error_o061Reloc == 1 || o61Error_o061Reloc == 8) {
            overlay45SetMode(gOverlay61Handle6C, 0xFF);
        } else if (o61Error_o061Reloc == 4) {
            overlay45SetMode(gOverlay61Handle74, 0xFF);
        } else {
            overlay45SetMode(gOverlay61Handle70, 0xFF);
        }
        overlay45SetMode(gOverlay61Handle80, 0xFF);
        overlay45SetMode(gOverlay61Handle84, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm) {
            o61State_o061Reloc = o61ErrorNextState_o061Reloc;
            amSndPlay(0xC, NULL);
        } else if (cancel) {
            o61State_o061Reloc = o61ErrorBackState_o061Reloc;
            amSndPlay(0xD, NULL);
        }
        break;
    case 1:
        overlay45SetMode(gOverlay61Handle68, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (confirm || cancel) {
            o61State_o061Reloc = 2;
            amSndPlay(0xC, NULL);
        }
        break;
    case 2:
        result = packDirectory(0, 16, o61DirectoryNames_o061Reloc, o61DirectoryExtensions_o061Reloc,
                               o61DirectorySizes_o061Reloc, o61DirectoryTypes_o061Reloc);
        if (result) {
            o61State_o061Reloc = 0;
            o61Error_o061Reloc = result;
            o61ErrorNextState_o061Reloc = 2;
            o61ErrorBackState_o061Reloc = 7;
            break;
        }
        overlay61ResetCounters();
        type = o61DirectoryTypes_o061Reloc;
        for (i = 0; i < 16; i++) {
            if (*type == 0 && D_8007A1F4 != NULL) {
                if (overlay61ReadCharacter(0, o61DirectoryExtensions_o061Reloc[i],
                                           &character, &course, &time) == 0) {
                    overlay61AddEntry(4, i, o61DirectoryNames_o061Reloc[i],
                                      o61DirectoryExtensions_o061Reloc[i],
                                      o61DirectorySizes_o061Reloc[i], character,
                                      course, time);
                } else {
                    overlay61AddEntry(6, i, o61DirectoryNames_o061Reloc[i],
                                      o61DirectoryExtensions_o061Reloc[i],
                                      o61DirectorySizes_o061Reloc[i], 0, 0, 0);
                }
            } else if (*type != 0xFF) {
                overlay61AddEntry(6, i, o61DirectoryNames_o061Reloc[i],
                                  o61DirectoryExtensions_o061Reloc[i],
                                  o61DirectorySizes_o061Reloc[i], 0, 0, 0);
            }
            type++;
        }
        if (D_8007A1F4 != NULL && D_8007A1F4->course != -1) {
            i = overlay61RecordSize(D_8007A1F4) + 4;
            if (i & 0xFF) {
                i = (i - (i & 0xFF)) + 0x100;
            }
            overlay61AddEntry(5, 0, gOverlay61GhostLabel, 0, i,
                              D_8007A1F4->character,
                              D_8007A1F4->course,
                              D_8007A1F4->time);
        }
        packFreeSpace(0, &o61FreeBytes_o061Reloc, NULL);
        overlay61AddEntry(7, 0, gOverlay61FreeLabel, 0,
                          o61FreeBytes_o061Reloc, 0, 0, 0);
        o61State_o061Reloc = 3;
        break;
    case 3:
        if (cancel) {
            o61State_o061Reloc = 7;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            o61State_o061Reloc = o61MenuEntries_o061Reloc[o61SelectedEntry_o061Reloc].kind;
            amSndPlay(0xC, NULL);
        } else if (y > 0) {
            o61SelectedEntry_o061Reloc--;
            if (o61SelectedEntry_o061Reloc < 0) {
                o61SelectedEntry_o061Reloc = o61EntryCount_o061Reloc - 1;
            }
            amSndPlay(0xF, NULL);
        } else if (y < 0) {
            o61SelectedEntry_o061Reloc++;
            if (o61SelectedEntry_o061Reloc >= o61EntryCount_o061Reloc) {
                o61SelectedEntry_o061Reloc = 0;
            }
            amSndPlay(0xF, NULL);
        }
        break;
    case 4:
        overlay45SetMode(gOverlay61Handle5C, 0xFF);
        overlay45SetMode(gOverlay61Handle78, 0xFF);
        overlay45SetMode(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State_o061Reloc = 6;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = func_overlay_061_F0001648_18C0A10(
                D_8007A1F4, 0,
                (char *)o61MenuEntries_o061Reloc[o61SelectedEntry_o061Reloc].text1);
            if (result) {
                o61State_o061Reloc = 0;
                o61Error_o061Reloc = result;
                o61ErrorNextState_o061Reloc = 4;
                o61ErrorBackState_o061Reloc = 2;
            } else {
                o61State_o061Reloc = 1;
            }
        }
        break;
    case 5:
        overlay45SetMode(gOverlay61Handle60, 0xFF);
        overlay45SetMode(gOverlay61Handle78, 0xFF);
        overlay45SetMode(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State_o061Reloc = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            overlay61ChooseFileExtension(gOverlay61NewFileName);
            result = overlay61WriteCharacter((s32 *)D_8007A1F4, 0,
                                             gOverlay61NewFileName);
            if (result) {
                o61State_o061Reloc = 0;
                o61Error_o061Reloc = result;
                o61ErrorNextState_o061Reloc = 5;
                o61ErrorBackState_o061Reloc = 2;
            } else {
                o61State_o061Reloc = 1;
            }
        }
        break;
    case 6:
        overlay45SetMode(gOverlay61Handle64, 0xFF);
        overlay45SetMode(gOverlay61Handle78, 0xFF);
        overlay45SetMode(gOverlay61Handle7C, 0xFF);
        alpha0 = 0xFF;
        alpha1 = 0xFF;
        if (cancel) {
            o61State_o061Reloc = 2;
            amSndPlay(0xD, NULL);
        } else if (confirm) {
            amSndPlay(0xC, NULL);
            result = packDeleteFile(0, o61MenuEntries_o061Reloc[o61SelectedEntry_o061Reloc].field2);
            if (result) {
                o61State_o061Reloc = 0;
                o61Error_o061Reloc = result;
                o61ErrorNextState_o061Reloc = 6;
                o61ErrorBackState_o061Reloc = 2;
            } else {
                o61State_o061Reloc = 1;
            }
        }
        break;
    case 7:
        mainTitlePageInit(0);
        o61State_o061Reloc = 8;
        break;
    case 8:
        break;
    }

    if (o61EntryCount_o061Reloc > 0) {
        overlay61DrawList((s32)&D_800D3140);
    }
    texAnimateTexSprite(gOverlay61Sprite0.texture, &gOverlay61Animation0, 2,
                  &gOverlay61Time0, updateRate);
    gOverlay61Sprite0.frame = gOverlay61Time0 * 65536.0f;
    texAnimateTexSprite(gOverlay61Sprite1.texture, &gOverlay61Animation1, 2,
                  &gOverlay61Time1, updateRate);
    gOverlay61Sprite1.frame = gOverlay61Time1 * 65536.0f;
    if (alpha0) {
        func_8002F618(&D_800D3140, &gOverlay61Sprite0, 0x24, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha0);
    }
    if (alpha1) {
        func_8002F618(&D_800D3140, &gOverlay61Sprite1, 0x11C, 0xCC,
                      0xFF, 0xFF, 0xFF, alpha1);
    }
}

void overlay61ReleaseResources(void) {
    freeFrontEndList(o61ResourceIds_o061Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource58Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource5CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource60Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource64Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource68Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource6CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource70Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource74Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource78Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource7CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource80Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource84Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource88Reloc);
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
            device, (char *)gOverlay61SavePathReloc, extension, &fileNumber);
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
            destination, -1, (char *)gOverlay61RecordFormatReloc, extension, (u8 *)buffer, size);
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
            device, (char *)gOverlay61RecordPathReloc, extension, &fileNumber);
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
        if (o61DirectoryTypes_o061Reloc[i] == 0) {
            if ((o61DirectoryExtensions_o061Reloc[i][0] >= 'A') &&
                (o61DirectoryExtensions_o061Reloc[i][0] <= 'Z')) {
                used[o61DirectoryExtensions_o061Reloc[i][0] - 'A'] = 1;
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

/* Tier B: packDirectory receives capacity16 for each output and fills the
 * unused slots through that capacity. The menu inserts at most20 real64-byte
 * entries. Extension selection writes exactly a letter and its terminator.
 * Tier D: original declarations/names remain inferred. Alternate legacy
 * list/note views and the shared directory allocation lifetime remain open. */
char *o61DirectoryNames[16];
char *o61DirectoryExtensions[16];
u32 o61DirectorySizes[16];
u8 o61DirectoryTypes[16];
Overlay61Entry o61MenuEntries[20];
char o61NewFileExtension[2];

/* Tier B/C: these real literals are read by the font conversion/copy path,
 * descriptor allocation/copy, sprintf formats, or pak filename conversion.
 * The complete current consumer chains do not write or retain their inputs.
 * Tier D: original const declarations remain inferred. Legacy mutable alias
 * views and callable/pointer-integer ABI debt are not unified by this owner.
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
