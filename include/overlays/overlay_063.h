#ifndef OVERLAY_063_H
#define OVERLAY_063_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "game/math.h"

/* Overlay 63. */

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

typedef struct O63Particle {
    void *resource;
    s16 x;
    s16 y;
    s16 height;
    u16 angle;
    s16 angleRate;
    s16 heightRate;
    s8 jitter;
    u8 pad11[3];
} O63Particle;

typedef struct O63RenderPosition {
    s16 x;
    s16 y;
    s16 z;
    u16 pad6;
    f32 scale;
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad18[0x10];
    f32 scratch28;
    u8 pad2C[0x68];
} O63RenderPosition;

typedef struct O63LocalObject {
    s32 word0;
    s32 word4;
    s32 fixed8;
} O63LocalObject;

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
extern s16 gO63ChainReload;
extern s32 gO63State18C;
extern s32 gO63State190;
extern s32 gO63State194;
extern s32 gO63State198;
extern s32 gO63State19C;
extern s32 func_80037664(void);
extern void func_80037414(s32, s32, s32, s32, s32, s32, s32);
extern void o63SetStateReloc(s32);
extern void func_80001608(void);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_8003A680(s32);
extern s32 func_800291E4(void);
extern void func_8002F618(void *, O63LocalObject *, s32, s32, s32, s32, s32, s32);
extern void texAnimateTexSprite(s32, void *, s32, f32 *, s32);
extern void texDPInit(void *);
extern void camStandardOrtho(void *, void *);
extern void camDo2DSprite(void *, void *, void *, O63RenderPosition *, void *, s32, s32);
extern u32 D_800D3180;
extern s32 gO63ExternalTimerReloc;
extern s32 gO63ExternalStateAReloc;
extern s32 gO63ExternalStateBReloc;
extern Gfx *D_800D3140;
extern void *gO63OpacityContextReloc;
extern void *D_800D3144;
extern void *D_800D3148;
extern s32 gO63Opacity;
extern f32 gO63ObjectFloat;
extern void *gO63Local2C;
extern O63LocalObject gO63Local30;
extern O63Particle gO63Particles[19];
extern s32 gO63Fade;
extern s32 gO63FadeDirection;
extern s32 gO63Triggered;
extern s32 gO63TriggerTimer;
extern s32 gO63FadeTimer;
extern void *gOverlay63Resource;
extern unsigned char gOverlay63Block[];
extern void overlay63ReleaseReloc(void *resource);
extern u16 joyGetPressed_o063Reloc(s32 arg0);
extern void overlay48Initialize(void);
extern void fontPrintXY(void *resource, s32 x, s32 y, void *state, s32 size);
extern u32 *gO63Sequence;
extern u8 gO63PendingStart[];
extern u32 gO63SequenceStart[];
extern void *gO63PendingState;
extern void *gO63ActiveState;
extern s32 gO63SequenceIndex;
extern s32 gO63Timer;
extern s32 gO63TransitionEnabled;
extern u32 D_8007A1E0;

/* This module's exports (ORT rows). */
void overlay63Initialize(void);
void overlay63UpdateEffects(s32 updateRate);
void overlay63Release(void);

#endif
