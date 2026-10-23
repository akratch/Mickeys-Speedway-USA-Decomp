#ifndef OVERLAY_089_H
#define OVERLAY_089_H

#include "PR/ultratypes.h"

/* Overlay 89: a flickering light effect with a drifting aim and a particle
 * burst. Pinned DKR v77/v80 and JFG object scans found no exact donor. */

/* The effect state, also read as the rule by overlay89Evaluate and as the
 * motion record by overlay89Update. */
typedef struct Overlay89EffectState {
    u8 mode;
    u8 motionFlags;
    u8 index;
    u8 particleCount;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 enabled;
    u8 pad09;
    s16 timer;
    f32 squaredRange;
    s16 xVelocity;
    s16 yVelocity;
    s16 left;
    s16 right;
    s16 top;
    s16 bottom;
    s32 motionWord;
    void *primaryHandle;
    f32 radius;
    f32 scale;
    void *secondaryHandle;
} Overlay89EffectState;

typedef struct Overlay89ColorEntry {
    u8 pad00[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad09;
} Overlay89ColorEntry;

typedef struct Overlay89NestedHeader {
    u8 pad00[0x12];
    s16 count;
    u8 pad14[8];
    Overlay89ColorEntry *entries;
} Overlay89NestedHeader;

typedef struct Overlay89NestedRoot {
    Overlay89NestedHeader *header;
    Overlay89ColorEntry *colors;
} Overlay89NestedRoot;

typedef struct Overlay89Status {
    u8 pad00[0x132];
    s16 initialized;
} Overlay89Status;

typedef struct Overlay89Object {
    s16 angleA;
    s16 angleB;
    u8 pad04[4];
    f32 size;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x22];
    u8 active;
    u8 pad3B[0x1D];
    Overlay89Status *status;
    u8 pad5C[8];
    Overlay89EffectState *state;
    Overlay89NestedRoot **nested;
    u8 pad6C[0x18];
    s32 clearWord;
} Overlay89Object;

typedef struct Overlay89Init {
    u8 pad00[0xA];
    u8 angleA;
    u8 angleB;
    s8 xVelocity;
    s8 yVelocity;
    u8 halfWidth;
    u8 halfHeight;
    u8 mode;
    u8 index;
    u8 motionFlags;
    u8 particleCount;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 range;
    u8 flags;
    u8 descriptorMode;
    u8 pad1B;
    u16 squaredRange;
    u8 pad1E;
    u8 size;
    u8 radius;
    u8 scale;
} Overlay89Init;

typedef struct Overlay89CreateDescriptor {
    u8 mode;
    s8 kind;
    u8 flags;
    s8 sentinelByte;
    s16 zero04;
    s16 zero06;
    s16 zero08;
    s16 zero0A;
    s16 sizeA;
    s16 sizeB;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    s16 sentinel;
    s8 zero16;
    s8 zero17;
} Overlay89CreateDescriptor;

typedef struct Overlay89Particle {
    s16 angleA;
    s16 angleB;
    f32 speed;
    f32 x;
    f32 y;
    f32 z;
    f32 size;
    s16 lifetime;
    s8 type;
    u8 pad1B;
    s32 colorA;
    s32 colorB;
    s32 rotation;
    u32 flagsA;
    u32 flagsB;
    u32 flagsC;
} Overlay89Particle;

extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern void trackLightMove(void *handle, f32 x, f32 y, f32 z);
extern void *trackLightAdd(f32 x, f32 y, f32 z, f32 scale,
                           f32 scaledSize, s32 red, s32 green, s32 blue);
extern s32 func_8002905C(u8, Overlay89EffectState *);
extern void *addObjectLight(Overlay89Object *object,
                            Overlay89CreateDescriptor *descriptor,
                            Overlay89EffectState *state,
                            Overlay89Init *init);
extern void changeLightIntensity(void *handle, u8 value);
extern void trackLightColour(void *handle, u8 red, u8 green, u8 blue,
                             s32 intensity);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 overlay34CreateRecord(Overlay89Particle *particle);

void overlay89InitializeEffect(Overlay89Object *object, Overlay89Init *init);
void overlay89UpdateStateAndParticles(Overlay89Object *object,
                                      s32 updateRate);

#endif
