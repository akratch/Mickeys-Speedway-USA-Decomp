#ifndef OVERLAY_014_H
#define OVERLAY_014_H

#include "PR/ultratypes.h"
#include "game/pi.h"
#include "game/menu.h"

/* A loaded value keyed by id; the module keeps 32 of them. */
typedef struct Overlay14ValueSlot {
    s32 key;
    void *value;
} Overlay14ValueSlot;

/* Tier D: layout from the rule loader's loads. A group is a count followed
 * by that many 8-byte rules; groups are stored back to back. */
typedef struct Overlay14Rule {
    u8 left;
    u8 operation;
    u8 right;
    u8 field114;
    s16 value;
    s16 field110;
} Overlay14Rule;

typedef struct Overlay14RuleGroup {
    s16 count;
    Overlay14Rule rules[1];
} Overlay14RuleGroup;

typedef struct Overlay14QueuedCommand {
    s16 value;
    s16 mode;
    s32 result;
} Overlay14QueuedCommand;

/* The pending-command record ResetMode replays (the queue's other view). */
typedef struct Overlay14PendingCommand {
    s16 pad0;
    s16 type;
    s32 value;
} Overlay14PendingCommand;

/* The menu command list: a header, then 16-byte entries. */
typedef struct Overlay14SelectionList {
    s16 count;
    s16 selected;
    s16 field4;
    s16 field6;
    s16 firstValue;
} Overlay14SelectionList;

typedef struct Overlay14SelectionEntry {
    s16 field0;
    s16 field2;
    s16 field4;
    s16 field6;
    s16 value;
    s16 fieldA;
    s16 fieldC;
    s16 fieldE;
} Overlay14SelectionEntry;

typedef struct Overlay14MenuCommand {
    u8 pad00[6];
    s16 checkValue;
    s16 encodedAction;
    s16 fallbackAction;
    s16 trailingAction;
    u8 pad0E[2];
} Overlay14MenuCommand;

typedef struct Overlay14Asset {
    s16 count;
    s16 pad2;
    void *pointer;
    s32 pad8;
    s16 marker;
    s16 padE;
} Overlay14Asset;

typedef struct Overlay14State {
    u8 pad00[0xF4];
    s32 *ranges;
} Overlay14State;

typedef struct Overlay14PackedRect {
    s8 useAlternateX0;
    s8 x0;
    s8 useAlternateY0;
    s8 y0;
    s8 useAlternateX1;
    s8 x1;
    s8 useAlternateY1;
    s8 y1;
    s8 intensity;
} Overlay14PackedRect;

typedef MenuRectangle Overlay14RenderRect;

typedef struct Overlay14Entry {
    s16 count;
    s16 selected;
    u8 *text;
    s16 kind;
    u8 padA[6];
} Overlay14Entry;

typedef struct Overlay14Owner {
    u8 pad00[0x84];
    s32 released;
} Overlay14Owner;

typedef struct Overlay14Object {
    s16 id;
    u8 pad2[10];
    f32 x;
    f32 y;
    f32 z;
} Overlay14Object;

/*
 * Overlay-local words reached at an addend from a section base. Each reader
 * was matched with the word as `.value` of a pad struct at that offset; the
 * Ref types carry the offsets once.
 */
#define OVERLAY14_REF(name, offset) \
    typedef struct name { u8 pad00[offset]; s32 value; } name
OVERLAY14_REF(O14ValueC0Ref, 0xC0);
OVERLAY14_REF(O14StateCCRef, 0xCC);
OVERLAY14_REF(O14PendingValueD0Ref, 0xD0);
OVERLAY14_REF(O14PendingModeD4Ref, 0xD4);
OVERLAY14_REF(O14TransitionD8Ref, 0xD8);
OVERLAY14_REF(O14CursorDCRef, 0xDC);
OVERLAY14_REF(O14ValueDCRef, 0xDC);
OVERLAY14_REF(O14PointerE0Ref, 0xE0);
OVERLAY14_REF(O14HandleE0Ref, 0xE0);
OVERLAY14_REF(O14ModeE4Ref, 0xE4);
OVERLAY14_REF(O14ValueECRef, 0xEC);
OVERLAY14_REF(O14ValueF8Ref, 0xF8);
OVERLAY14_REF(O14ResultFCRef, 0xFC);
OVERLAY14_REF(O14Value108Ref, 0x108);
OVERLAY14_REF(O14Value110Ref, 0x110);
OVERLAY14_REF(O14Value114Ref, 0x114);
#undef OVERLAY14_REF

extern Overlay14ValueSlot gOverlay14ValueSlots28[], gOverlay14ValueSlotsEnd128[];
extern Overlay14ValueSlot gOverlay14Slots28[32];
extern s32 gOverlay14SlotCountE8;
extern s32 gOverlay14StateC0, gOverlay14StateC4, gOverlay14StateC8;
extern s32 gOverlay14PendingValueD0, gOverlay14PendingModeD4;
extern s32 gOverlay14TransitionD8, gOverlay14CursorDC, gOverlay14PointerE0;
extern s32 gOverlay14ValueCountE8, gOverlay14CommandCountEC;
extern Overlay14RuleGroup *gOverlay14AssetF0;
extern u32 *gOverlay14OffsetsF4;
extern s32 gOverlay14ResultF8, gOverlay14ResultFC;
extern s32 gOverlay14Current104, gOverlay14Next108, gOverlay14Context10C;
extern s32 gOverlay14Field110, gOverlay14Field114;
extern O14PendingValueD0Ref gOverlay14PendingValueD0Base;
extern O14PendingModeD4Ref gOverlay14PendingModeD4Base;
extern O14TransitionD8Ref gOverlay14TransitionD8Base;
extern O14CursorDCRef gOverlay14CursorDCBase;
extern O14PointerE0Ref gOverlay14PointerE0Base;
extern O14ModeE4Ref gOverlay14ModeE4;
extern O14ResultFCRef gOverlay14ResultFCBase;
extern O14Value108Ref gOverlay14Value108;
extern O14StateCCRef gOverlay14StateCC;
extern O14ValueECRef gOverlay14ValueEC;
extern O14Value110Ref gOverlay14Value110;
extern O14Value114Ref gOverlay14Value114;
extern O14ValueC0Ref gOverlay14ValueC0Base;
extern O14ValueDCRef gOverlay14ValueDCBase;
extern O14HandleE0Ref gOverlay14HandleE0;
extern O14ValueF8Ref gOverlay14ValueF8Base;
extern Overlay14QueuedCommand gOverlay14QueuedCommands128[];
extern Overlay14PendingCommand D_128[];
extern s32 D_EC;
extern s32 D_E4;
extern s32 D_FC;
extern s32 D_A4;
extern s32 D_B4;
extern s32 gOverlay14ValueF8;
extern s32 gOverlay14ValueD8;
extern s32 gOverlay14ValueDC;
extern s32 gOverlay14ValueE0;
extern s32 gOverlay14ValueC0;
extern Overlay14SelectionList *gOverlay14CommandHeader;
extern Overlay14State gOverlay14State;
extern s32 gOverlay14FlagC4;
extern s32 gOverlay14FlagC8;
extern s32 gOverlay14FlagCC;
extern s32 gOverlay14ResourceD0;
extern s32 gOverlay14ValueD4;
extern s32 gOverlay14ResetCC;
extern s32 gOverlay14Transition;
extern s32 gOverlay14Cursor;
extern s32 gOverlay14Flag0[];
extern s32 gOverlay14Flag4[];
extern s32 gOverlay14FlagC[];
extern s32 gOverlay14Flag10[];
extern Overlay14SelectionList *gOverlay14CommandHeaderBase[];
extern s32 gOverlay14ActiveHandle;
extern s32 gOverlay14TransitionValue[];
extern s32 gOverlay14FlagC4Base[];
extern s32 gOverlay14FlagC8Base[];
extern s32 gOverlay14ModeE4Base[];
extern s32 gOverlay14EnabledF8Base[];
extern char gOverlay14Args2C[];
extern char gOverlay14Args30[];
extern char gOverlay14Args34[];
extern s32 gOverlay14DataBase;
extern Overlay14Entry *gOverlay14Entries;
extern Overlay14Owner *gOverlay14Owner;

extern void overlay14InitializeReloc(void);
extern void mmFree(void *ptr);
extern void *mmAlloc(s32 size, u32 colourTag);
extern u32 *piRomLoad_o014Reloc(u32 resourceId);
extern s32 frontGetLanguage_o014Reloc(void);
extern s32 frontGetLanguage(void);
extern u16 joyGetPressed(s32 player);
extern s8 joyGetStickX(s32 player);
extern s8 joyGetStickY(s32 player);
/* The resident definition (main/font.c) returns u8; the module calls it as
 * returning int, as an undeclared call would. As u8 the zero-extension
 * becomes a second web copied out of v0 (one extra move, frame +8). */
extern s32 func_8004D5C0(s32 font);
extern void fontSetButtonMode(s32 active);
extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha,
                       s32 opacity);
extern void fontUseFont(s32 font);
extern u8 *func_8004D40C(s32 font, char *text, s32 maxWidth,
                         u8 **lineStart, s32 *outWidth);
extern void fontPrintXY(Gfx **displayList, s32 x, s32 y, char *text,
                        s32 alignmentFlags);
extern void frontDrawRectangles(Gfx **context, s32 count,
                                Overlay14RenderRect *rects, s32 flags);
extern Overlay14Object *func_80005820(s32 index);
extern void controlSetPlayerSetup(s16 x, s16 y, s16 z, s16 angle);
extern s32 mainGetNextCharacter(void);
extern u8 frontGetMode(void);
extern void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup,
                            s32 nextCamera, s32 fadeOut, s32 flags);
/* Unprototyped placeholders for SYMBOL calls whose callees the module's
 * relocation records name (offset-zero stored addend). */
extern s32 overlay14Dispatch();

/*
 * ResetMode's call to the exported overlay14ReleaseOwner is a SYMBOL
 * relocation record: the shipped word stores the 0xF0000000 addend, not the
 * callee's offset, so it names this undefined placeholder until the overlay
 * tables are linker output.
 */
extern void overlay14ReleaseOwner_o014Reloc(void);

#endif
