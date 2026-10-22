#include "PR/ultratypes.h"

typedef struct O63Config {
    void *resource;
    s16 value4;
    s16 value6;
    s16 pad8;
    s16 randomA;
    s16 randomB;
    s16 randomC;
    u8 pad10[4];
} O63Config;

extern void amTunePlay(s32 value);
extern void func_800015F8(void);
extern void loadFrontEndList(void *resource);
extern void fontUseFont(s32 mode);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 intensity);
extern void *overlay45CreateDescriptor(void *resource, s32 x, s32 y, s32 kind);
extern void overlay45SetMode(void *resource, s32 value);
extern s16 mathRnd(s32 lower, s32 upper);
extern void joyEnable(s32 slot);

extern u8 gO63InitResource[];
extern u8 gO63CreateResource[];
extern void *D_800D31C8[];
extern s32 D_8007BF44;
extern u8 gO63ExternalByteReloc;
extern void *gO63LocalResource;
extern s32 gO63LocalWord;
extern f32 gO63LocalFloat;
extern s32 gO63SavedTableWord;
extern O63Config gO63Configs[];
extern s16 gO63Chain;
#pragma weak gO63ChainReload = gO63Chain
extern s16 gO63ChainReload;
extern s32 gO63State18C;
extern s32 gO63State190;
extern s32 gO63State194;
extern s32 gO63State198;
extern s32 gO63State19C;

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 63 as none. */
/*
 * Fresh maintenance plateau: a separate typed alias plus the address-preserving
 * integer round trip restores the target's second chain relocation pair and
 * reduces the configured 117-word candidate from 22 differences to one at
 * +0x154.  Both frames are 0x30 and both relocation surfaces have 46 records.
 * The remaining difference is the commutative loop-back comparison order; a
 * bounded permuter found only an inert-expression spelling, which is rejected.
 */
void overlay63Initialize(void)
{
  s16 *chain;
  O63Config *config;
  s16 index;
  s32 slot;
  amTunePlay(0x16);
  func_800015F8();
  loadFrontEndList(gO63InitResource);
  gO63SavedTableWord = (s32) D_800D31C8[6];
  fontUseFont(3);
  fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
  gO63LocalResource = overlay45CreateDescriptor(gO63CreateResource, 0xA0, 0xC0, 0x204);
  gO63LocalWord = 0;
  gO63LocalFloat = 0.0f;
  overlay45SetMode(gO63LocalResource, 0);
  chain = &gO63Chain;
  config = gO63Configs;
  if (gO63Chain != (-1))
  {
    index = *((s16 *) ((s32) (&gO63ChainReload)));
    do
    {
      config->resource = D_800D31C8[index];
      config->value4 = chain[1];
      config->value6 = chain[2];
      config->randomA = mathRnd(0, 0x8000);
      config->randomB = mathRnd(0x600, 0xA00);
      config->randomC = mathRnd(0x100, 0x300);
      if (mathRnd(0, 1) == 0)
      {
        config->randomC = -config->randomC;
      }
      index = chain[3];
      chain += 3;
      config++;
    }
    while ((index ^ 0) != (-1));
  }
  gO63State18C = 0;
  gO63State190 = 1;
  gO63State194 = 0;
  gO63State198 = 0;
  gO63State19C = 0xF0;
  for (slot = 0; slot < 4; slot++)
  {
    joyEnable(slot);
  }

  D_8007BF44 = 0;
  gO63ExternalByteReloc = 0;
}

