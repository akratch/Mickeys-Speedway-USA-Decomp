#include "PR/ultratypes.h"
struct Sprite;

/* Tier B: actual mutable preview mode, CPU sound handle, ten sound IDs,
 * seven panel/transition/settings words, six sprite IDs and a CPU Sprite
 * pointer. The random selector is inclusive0..9; the sprite selector wraps
 * through0..5. IDO supplies final section alignment, not another object.
 * Tier D: original declarations/names and caller/lifetime bounds remain
 * separate. The sound engine retains the handle-cell address; this storage
 * does not close that lifetime or the deeper provider/header views. */
s32 o60PreviewMode = 0;
void *o60PreviewSound = 0;
u16 o60PreviewSoundIds[10] = {4, 9, 58, 417, 532, 451, 449, 443, 504, 524};
s32 o60SubmenuState = 0;
s32 o60PanelValue = 0;
s32 o60PanelPhase = 1;
s32 o60VideoTransition = 0;
s32 o60PreviewTune = 0;
s32 o60SettingsPending = 0;
s32 o60ExitRequested = 0;
s16 o60CharacterSpriteIds[6] = {162, 163, 164, 165, 166, 167};
struct Sprite *o60CharacterSprite = 0;
