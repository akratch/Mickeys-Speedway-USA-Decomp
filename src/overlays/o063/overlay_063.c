#include "overlays/overlay_063.h"

/* Module-private functions: no ORT row, so their callers use JUMP
 * records. */
static void overlay63UpdateSequence(s32 delta);

#pragma weak gO63ChainReload = gO63Chain

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

/*
 * Matched (lane j-o035, 2026-10-02). The draw-context calls take the
 * context address as a pointer, the particle cursor starts at the end of
 * gO63Particles (not at the fade word's address), the two packets use the
 * GBI macros, the loop is `count = 19; while (count--)`, the particle
 * position stores run x, y, z-plane, height, and the position reset is a
 * do-while(0) macro whose region ranks the render context ahead of the
 * draw context for the callee-saved registers.
 */

#define O63_RESET_POSITION(p) \
    do { \
        (p).x = 0; \
        (p).y = 0; \
        (p).z = 0; \
        (p).scale = 1.0f; \
        (p).scratch28 = 0.0f; \
    } while (0)

void overlay63UpdateEffects(s32 updateRate) {
    O63RenderPosition pos;
    O63Particle *particle;
    s32 count;
    s32 pad0; /* frame 0xF8: the target keeps 8 more bytes of locals */

    if (gO63Triggered == 0) {
        if ((D_800D3180 & 0x9000) && (gO63ExternalTimerReloc <= 0)) {
            if (func_80037664() == 1) {
                func_80037414(0, 0x3FC00000, 0x3F800000, 0, 0, 0, 1);
            }
            o63SetStateReloc(5);
            func_80001608();
            mainChangeLevel(0, 0, 0, 0x10, 1, 1);
            gO63Triggered = 1;
        } else {
            gO63TriggerTimer += updateRate;
            if (gO63TriggerTimer >= 0xE10) {
                func_8003A680(0);
                gO63ExternalStateAReloc = -1;
                gO63ExternalStateBReloc = 1;
                func_80001608();
                mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
                o63SetStateReloc(1);
                gO63Triggered = 1;
            }
        }
    }

    if (gO63FadeTimer != 0) {
        gO63FadeTimer -= updateRate;
        if (gO63FadeTimer < 0) {
            gO63FadeTimer = 0;
        }
        if (gO63Fade == 0xFF) {
            if (gO63ExternalTimerReloc > 0) {
                gO63ExternalTimerReloc -= updateRate;
            } else {
                gO63Opacity += updateRate * 4;
                if (gO63Opacity >= 0x100) {
                    gO63Opacity = 0xFF;
                }
            }
        }
    } else if (gO63FadeDirection == 1) {
        gO63Fade += updateRate * 4;
        if (gO63Fade >= 0x100) {
            gO63Fade = 0xFF;
        }
        if (gO63Fade == 0xFF) {
            gO63FadeDirection = 0;
            gO63FadeTimer = 0xF0;
        }
    } else {
        gO63Fade -= updateRate * 4;
        if (gO63Fade < 0) {
            gO63Fade = 0;
        }
        if (gO63Fade == 0) {
            gO63FadeDirection = 1;
            gO63FadeTimer = 0x1E0;
        }
    }

    if ((gO63Opacity != 0) && (func_800291E4() == 0)) {
        func_8002F618(&D_800D3140, &gO63Local30, 0x40, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        func_8002F618(&D_800D3140, &gO63Local30, 0x100, 0xCC, 0xFF, 0xFF, 0xFF, gO63Opacity);
        overlay45SetMode(gO63OpacityContextReloc, gO63Opacity);
        texAnimateTexSprite(gO63Local30.word0, &gO63Local2C, 2, &gO63ObjectFloat, updateRate);
        gO63Local30.fixed8 = (s32)(gO63ObjectFloat * 65536.0f);
    }

    texDPInit(&D_800D3140);
    camStandardOrtho(&D_800D3140, &D_800D3144);
    if (gO63Fade != 0) {

        O63_RESET_POSITION(pos);

        gDPPipeSync(D_800D3140++);
        gDPSetPrimColor(D_800D3140++, 0, 0, 0xFF, 0xFF, 0xFF, gO63Fade);

        particle = &gO63Particles[19];
        count = 19;
        while (count--) {
            particle--;
            particle->angle = particle->angle + particle->angleRate * updateRate;
            if (particle->angle >= 0x8001) {
                particle->angle -= 0x8000;
                particle->angleRate = mathRnd(0x600, 0xA00);
            }
            particle->height += particle->heightRate * updateRate;
            if (particle->height < -0x1000) {
                particle->height = -0x1000;
                particle->heightRate = mathRnd(0x100, 0x200);
            } else if (particle->height >= 0x1001) {
                particle->height = 0x1000;
                particle->heightRate = -mathRnd(0x100, 0x200);
            }
            particle->jitter = (s8)(s32)(func_8002A8C0(particle->angle) * 5.0f);
            pos.posX = (f32)particle->x;
            pos.posY = (f32)(particle->y + particle->jitter);
            pos.posZ = 0.0f;
            pos.z = particle->height;
            if (gO63Fade < 0xFF) {
                camDo2DSprite(&D_800D3140, &D_800D3144,
                    &D_800D3148, &pos, particle->resource, 5, gO63Fade);
            } else {
                camDo2DSprite(&D_800D3140, &D_800D3144,
                    &D_800D3148, &pos, particle->resource, 0x8001, gO63Fade);
            }
        }
    }
    overlay63UpdateSequence(updateRate);
}

void overlay63Release(void) {
    overlay63ReleaseReloc(gOverlay63Resource);
    overlay63ReleaseReloc(gOverlay63Block);
}

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 63 as none. */
static void overlay63UpdateSequence(s32 delta) {
    s32 pressedButtons;

    pressedButtons = joyGetPressed_o063Reloc(0);
    if (pressedButtons != 0) {
        if (gO63Sequence == 0) {
            gO63PendingState = gO63PendingStart;
            gO63Sequence = gO63SequenceStart;
            gO63SequenceIndex = 1;
        } else if (pressedButtons == gO63Sequence[gO63SequenceIndex]) {
            gO63SequenceIndex++;
            if (gO63Sequence[gO63SequenceIndex] == 0) {
                D_8007A1E0 = (D_8007A1E0 ^ 1) & 1;
                gO63Sequence = 0;
                gO63Timer = 0xB4;
                gO63ActiveState = gO63PendingState;
            }
        } else {
            gO63Sequence = 0;
        }
    }

    if (gO63Timer > 0) {
        gO63Timer -= delta;
        if (gO63Timer <= 0 && gO63TransitionEnabled != 0) {
            overlay48Initialize();
        }
        fontUseFont(2);
        fontColour(0, 0, 0, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140, 0xA0, 0x3C, gO63ActiveState, 0xC);
        fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
        fontPrintXY(&D_800D3140, 0xA1, 0x3D, gO63ActiveState, 0xC);
    }
}

/* Tier D storage model from the actual writes and phase order, not a recovered
 * original declaration. Initialization visits twenty slots; rendering visits
 * nineteen and the final slot becomes five control words. Existing imported
 * Config/Particle/control views remain legacy effective-type/lifetime debt.
 * The compiler naturally places this400-byte envelope after three prefix
 * globals with four alignment bytes; no reserved object supplies that gap. */
struct Overlay45ResourceDescriptor;
typedef struct Overlay63ParticleStorage {
    void *resource;
    s16 x;
    s16 y;
    s16 tilt;             /* current source calls this height; rendered as Z angle */
    u16 phase;
    s16 phaseStep;
    s16 tiltStep;
    s8 jitter;
} Overlay63ParticleStorage;

typedef struct Overlay63EffectControls {
    s32 fade;
    s32 fadeDirection;
    s32 triggered;
    s32 triggerTimer;
    s32 fadeTimer;
} Overlay63EffectControls;

typedef union Overlay63EffectsStorage {
    Overlay63ParticleStorage constructorSlots[20];
    struct {
        Overlay63ParticleStorage rendered[19];
        Overlay63EffectControls controls;
    } runtime;
} Overlay63EffectsStorage;

struct Overlay45ResourceDescriptor *overlay63DescriptorStorage;
s32 overlay63OpacityStorage;
f32 overlay63FrameStorage;
Overlay63EffectsStorage overlay63EffectsStorage;

#ifdef TARGET_N64
typedef char O63StoragePointerWidth[(sizeof(void *) == 4) ? 1 : -1];
typedef char O63ParticleStorageSize[(sizeof(Overlay63ParticleStorage) == 20) ? 1 : -1];
typedef char O63EffectControlsSize[(sizeof(Overlay63EffectControls) == 20) ? 1 : -1];
typedef char O63EffectsStorageSize[(sizeof(Overlay63EffectsStorage) == 400) ? 1 : -1];
#endif
