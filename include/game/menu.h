#ifndef _GAME_MENU_H_
#define _GAME_MENU_H_

#include "PR/ultratypes.h"

typedef struct MenuCommand MenuCommand;

/* PROVENANCE: base layout adapted from JFG's public decomp,
 * src/menu.h::Resbitfield; twoPlayerSplit and stereoMode are Mickey-derived
 * from their paired getters and byte-preserving setters. */
typedef struct MenuScreenModeBits {
    u32 unused : 1;
    u32 modeBit0 : 1;
    u32 modeBit1 : 1;
    u32 twoPlayerSplit : 1;
    u32 unusedStereoGap : 5;
    u32 stereoMode : 2;
    u32 unusedLanguageGap : 5;
    u32 language : 6;
    u32 rest : 10;
} MenuScreenModeBits;

typedef union MenuScreenModeState {
    MenuScreenModeBits bits;
    u8 raw;
} MenuScreenModeState;

/* Mickey-derived from the adjacent per-controller accesses in func_80039720. */
typedef struct MenuControllerRepeatState {
    s8 repeatX[4];
    s8 repeatY[4];
    u32 previousButtons[4];
} MenuControllerRepeatState;

void frontSetMode(s32 mode);
u8 frontGetMode(void);
void frontDrawRectangle(MenuCommand **displayList, s32 left, s32 top, s32 right, s32 bottom, u32 colour);
void frontPlayerScreenLimits(s32 player, s32 *left, s32 *top, s32 *right, s32 *bottom);
void frontDemoMessage(MenuCommand **displayList, s32 updateRate);
void freeFrontEndList(s16 *assetGroup);
void freeFrontEndItem(s32 assetId);
void loadFrontEndList(s16 *assetGroup);
void loadFrontEndItem(s32 assetId);
void setupFrontEndList(s16 *objectGroup);
void setupFrontEndObject(s32 objectId);
s32 frontGetLanguage(void);
void frontSetLanguage(s32 language);
s32 frontGetScreenMode(void);
void frontSetScreenMode(s32 screenMode);
void frontStoreScreenMode(void);
u8 frontRecallScreenMode(void);
s32 frontGetLevelScreenMode();
s8 frontGetWideAdjust(void);
void frontSetWideAdjust(s32 offset);
u32 frontGetStereoMode(void);
void frontSetStereoMode(s32 mode);
u16 frontGetSfxVolume(void);
void frontSetSfxVolume(s32 volume);
u16 frontGetBgmVolume(void);
void frontSetBgmVolume(s32 volume);
s32 frontGet2PlayerSplit(void);
void func_8003A544(s32 value);
s32 func_8003A550(void);
void func_8003A55C(s32 value);
void func_8003A590(void);

#endif
