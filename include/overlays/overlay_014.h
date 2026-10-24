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
 * Legacy F8/FC address carriers retain their offset-bearing word views.
 * Their pointer-domain/native-width contracts are outside scalar cleanup.
 */
#define OVERLAY14_REF(name, offset) \
    typedef struct name { u8 pad00[offset]; s32 value; } name
OVERLAY14_REF(O14ValueF8Ref, 0xF8);
OVERLAY14_REF(O14ResultFCRef, 0xFC);
#undef OVERLAY14_REF

/* Real BSS definitions use typed private loader operand imports. */
extern s32 gOverlay14InputFlag0_o014Reloc;
extern s32 gOverlay14InputFlag4_o014Reloc;
extern s32 gOverlay14InputFlag8_o014Reloc;
extern s32 gOverlay14PulseC_o014Reloc;
extern s32 gOverlay14Pulse10_o014Reloc;
extern s32 gOverlay14Pulse14_o014Reloc;
extern s32 gOverlay14Pulse18_o014Reloc;
extern s32 gOverlay14Timer1C_o014Reloc;
extern s32 gOverlay14Timer20_o014Reloc;
extern Overlay14ValueSlot gOverlay14ValueSlots_o014Reloc[32];
/* Legacy one-past boundary operand only: no separate slot allocation.
 * The genuine cache+32 expression changes the stock initializer shape.
 * This binding remains explicitly borrowed until that context is resolved. */
extern Overlay14ValueSlot gOverlay14ValueSlotsEnd128[];
extern Overlay14QueuedCommand gOverlay14CommandQueue_o014Reloc[16];

extern s32 o14Opacity_o014Reloc, o14Active_o014Reloc, o14State_o014Reloc;
extern s32 o14PendingValue_o014Reloc, o14PendingMode_o014Reloc;
extern s32 o14Transition_o014Reloc, o14Cursor_o014Reloc, o14TextHasMore_o014Reloc;
extern s32 o14ValueCount_o014Reloc, o14CommandCount_o014Reloc;
extern s32 o14Mode_o014Reloc;
extern Overlay14RuleGroup *o14RuleGroups_o014Reloc;
extern u32 *o14Ranges_o014Reloc;
extern s32 o14ResultAddress_o014Reloc, o14SelectionAddress_o014Reloc;
extern s32 o14Current_o014Reloc, o14Next_o014Reloc, o14Context_o014Reloc;
extern s32 o14RuleField110_o014Reloc, o14RuleField114_o014Reloc;
extern O14ResultFCRef gOverlay14ResultFCBase;
extern O14ValueF8Ref gOverlay14ValueF8Base;
extern s32 D_FC;
extern char o14ReturnLabel_o014Reloc[];
extern char o14ExitLabel_o014Reloc[];
extern s32 gOverlay14ValueF8;
extern Overlay14SelectionList *gOverlay14CommandHeader;
extern Overlay14State gOverlay14State;
extern s32 o14ExitRequested_o014Reloc;
extern Overlay14SelectionList *gOverlay14CommandHeaderBase[];
extern s32 gOverlay14EnabledF8Base[];
extern const char o14ScrollUpLabel_o014Reloc[];
extern const char o14ScrollDownLabel_o014Reloc[];
extern const char o14SelectionMoreLabel_o014Reloc[];
extern Overlay14PackedRect o14WindowRects_o014Reloc[18];
extern Overlay14Entry *gOverlay14Entries;
extern Overlay14Owner *o14Owner_o014Reloc;

extern void mmFree(void *ptr);
extern void *mmAlloc(s32 size, u32 colourTag);
extern u32 *piRomLoad_o014Reloc(u32 resourceId);
extern s32 frontGetLanguage_o014Reloc(void);
extern s32 frontGetLanguage(void);
extern u16 joyGetPressed(s32 player);
extern s8 joyGetStickX(s32 player);
extern s8 joyGetStickY(s32 player);
/* The height API returns its stored unsigned byte as a word (0..255). */
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
/*
 * ResetMode's call to the exported overlay14ReleaseOwner is a SYMBOL
 * relocation record: the shipped word stores the 0xF0000000 addend, not the
 * callee's offset, so it names this undefined placeholder until the overlay
 * tables are linker output.
 */
extern void overlay14ReleaseOwner_o014Reloc(void);

#endif
