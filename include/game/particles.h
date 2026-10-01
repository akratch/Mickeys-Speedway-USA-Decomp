#ifndef GAME_PARTICLES_H
#define GAME_PARTICLES_H

#include "PR/ultratypes.h"

/*
 * Resident particle aggregates. The target load/store surface fixes the
 * offsets and access widths represented by these declarations; the named
 * vector and resource relationships are cross-checked against JFG and DKR.
 * PROVENANCE: aggregate names and shapes are informed by Jet Force Gemini
 * include/structs.h and Diddy Kong Racing src/particles.h. Mickey's target
 * assembly is authoritative for every retained padding byte and field width.
 */
typedef struct ParticleVec3f {
    union {
        struct {
            f32 x;
            f32 y;
            f32 z;
        };
        f32 f[3];
    };
} ParticleVec3f;

typedef struct ParticleVec3s {
    union {
        struct {
            s16 x;
            s16 y;
            s16 z;
        };
        struct {
            s16 rotationY;
            s16 rotationX;
            s16 rotationZ;
        };
        s16 s[3];
    };
} ParticleVec3s;

typedef struct ParticleVec3i {
    union {
        struct {
            s32 x;
            s32 y;
            s32 z;
        };
        s32 i[3];
    };
} ParticleVec3i;

typedef struct ParticleConfig {
    s32 flags;
    ParticleVec3f emitterPosition;
    f32 value10;
    ParticleVec3s sourceRotation;
    s16 value1A;
    ParticleVec3s sourceAngularVelocity;
    ParticleVec3s emissionDirection;
    s16 value28;
    ParticleVec3s emissionDirectionAngularVelocity;
    ParticleVec3f velocityModifier;
    f32 value3C;
    s16 value40;
    s16 value42;
    ParticleVec3s rotation;
    ParticleVec3s angularVelocity;
    f32 value50;
    f32 value54;
    f32 value58;
    s32 flags5C;
    s32 value60;
    ParticleVec3s sourceDirectionRange;
    ParticleVec3s emissionDirectionRange;
    s32 value70;
    ParticleVec3i velocityModifierRange;
    ParticleVec3s rotationRange;
    ParticleVec3s angularVelocityRange;
    s32 value8C;
    s32 value90;
    s32 value94;
    u8 value98;
    u8 value99;
    u8 value9A;
    u8 value9B;
} ParticleConfig;

typedef struct ParticleTrigger {
    ParticleConfig *config;
    s32 flags;
    union {
        s16 unk08;
        struct {
            u8 sourceRotationCounter;
            u8 directionRotationCounter;
        };
    };
    s16 value;
    s16 unk0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 x;
    s16 y;
    s16 z;
    s16 unk20;
    u8 alpha;
    s8 index;
    u8 active;
    u8 pad25[3];
} ParticleTrigger;

typedef struct ParticleTriggerSlot {
    ParticleConfig *config;
    s32 flags;
    u8 unk08;
    u8 unk09;
    s16 type;
    s16 unk0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    s16 value14;
    s16 value16;
    s16 value18;
    s16 value1A;
    s16 value1C;
    s16 value1E;
    s16 value20;
    u8 result;
    s8 index;
} ParticleTriggerSlot;

typedef struct ParticleObjectHeader {
    u8 pad00[0x25];
    s8 triggerCount;
} ParticleObjectHeader;

typedef struct ParticleObject {
    u8 pad00[0x1A];
    s16 activeTriggerCount;
    u8 pad1C[0x24];
    ParticleObjectHeader *header;
    u8 pad44[0x28];
    ParticleTriggerSlot *triggers;
    u8 pad70[0x10];
    u32 triggerBits;
} ParticleObject;

typedef struct ParticleResourceEntry {
    u8 pad00[0x2C];
    s16 type;
    u8 pad2E[0x1E];
    void *resource;
    u8 pad50[0x28];
} ParticleResourceEntry;

typedef struct ParticleResourceList {
    u8 pad00[0x14];
    ParticleResourceEntry *entries;
    s32 count;
} ParticleResourceList;

typedef struct CircularParticle CircularParticle;

typedef struct ParticleModelEntry {
    CircularParticle *particles[15];
    s32 particleCount;
    s32 vertexCount;
    ParticleVec3f points[8];
    u8 active;
    u8 padA5[3];
    s32 animationState;
    s32 configFlags;
    f32 textureFrame;
    s16 animationSpeed;
    u8 padB6[2];
    void *resource;
    volatile ParticleTriggerSlot *trigger;
} ParticleModelEntry;

typedef struct ParticleVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} ParticleVertex;

typedef struct ParticlePointStreamEntry {
    u8 pad00[0x40];
    s32 pointCount;
    f32 points[8][3];
    u8 active;
    u8 padA5[3];
    s32 animationState;
    s32 configFlags;
    f32 textureFrame;
    s16 animationSpeed;
    u8 padB6[2];
    void *resource;
    ParticleTriggerSlot *trigger;
} ParticlePointStreamEntry;

typedef struct ParticleLinePoint ParticleLinePoint;
typedef struct ParticleTexture ParticleTexture;

typedef struct ParticleLineEntry {
    s32 pointCount;
    ParticleLinePoint *points[15];
    u8 pad40[0xE4];
    u8 active;
    u8 pad125[3];
    ParticleTexture *texture;
    ParticleConfig *config;
    union {
        u32 *colorTable;
        s32 unk130;
    };
    s16 colorCount;
    u8 pad136[2];
    s32 descriptorWord;
    s32 configFlags;
    f32 textureFrame;
    s16 value144;
    u8 pad146[2];
} ParticleLineEntry;

struct ParticleTexture {
    u8 pad00[4];
    s16 flags;
    u8 pad06[0x0A];
    u16 frameCount;
};

typedef struct ParticleSprite {
    u8 frameCount;
    u8 pad01[0x0F];
    ParticleTexture **textures;
} ParticleSprite;

struct ParticleLinePoint {
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
    s16 intensityTimer;
    s16 intensity;
    s16 colorTimer;
    s16 colorIndex;
    s16 lifetime;
    s16 intensityVelocity;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
};

typedef struct ParticleLineVertex {
    s16 x0;
    s16 y0;
    s16 z0;
    u8 red0;
    u8 green0;
    u8 blue0;
    u8 alpha0;
    s16 x1;
    s16 y1;
    s16 z1;
    u8 red1;
    u8 green1;
    u8 blue1;
    u8 alpha1;
} ParticleLineVertex;

typedef ParticleLineEntry ParticleLineRenderEntry;

typedef struct ParticleModelPartConfig {
    s16 type;
    s16 triggerType;
    s16 particleType;
} ParticleModelPartConfig;

typedef struct ParticleModelObjectHeader {
    u8 pad00[0xB0];
    ParticleModelPartConfig *parts;
} ParticleModelObjectHeader;

typedef struct ParticleModelObject {
    u8 pad00[0x40];
    ParticleModelObjectHeader *header;
    u8 pad44[0x38];
    ParticleTriggerSlot *triggers;
    u8 pad80[0x0D];
    u8 triggerCount;
} ParticleModelObject;

typedef struct ParticleTypeDescriptor {
    s32 descriptorWord;
    union {
        s32 flags;
        struct {
            u8 pad04;
            u8 pointCount;
            s16 resourceId;
        };
    };
    s16 animationSpeed;
    s16 lifetime;
    s16 lifetimeRange;
    u8 intensity;
    u8 targetIntensity;
    s16 intensityTimer;
    s16 colorTimer;
    f32 scale;
    u8 colorMode;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad1C[3];
    u8 colorCount;
    u32 *colorTable;
    u8 pad24[2];
    u8 triggerValue;
    u8 triggerType;
} ParticleTypeDescriptor;

typedef struct BasicParticle {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 transformFlags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 scaleVelocity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[6];
    u8 type;
    u8 pad2F;
    f32 localX;
    f32 localY;
    f32 localZ;
    f32 movementValue;
    f32 gravity;
    u8 pad44[4];
    void *parent;
    u8 pad4C[0x12];
    s16 angularVelocityY;
    s16 angularVelocityX;
    s16 angularVelocityZ;
} BasicParticle;

typedef struct ParticleEmitterTransformIndex {
    u16 pad00;
    u16 matrixIndex;
} ParticleEmitterTransformIndex;

typedef struct ParticleEmitterHeader {
    u8 pad00[0x30];
    ParticleEmitterTransformIndex *transformIndices;
    u8 pad34[0x1A];
    s8 transformedPoints;
} ParticleEmitterHeader;

typedef struct ParticleEmitterPointSet {
    u8 pad00[0x1E];
    s8 transformedPoints;
} ParticleEmitterPointSet;

typedef struct ParticleEmitterResource {
    ParticleEmitterHeader *header;
    u8 pad04[4];
    s16 disableTransform;
    s16 matrixTableIndex;
    void *matrices[13];
    f32 (*points)[3];
} ParticleEmitterResource;

typedef struct ParticleEmitterObject {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad28[0x12];
    s8 resourceIndex;
    u8 pad3B[5];
    u8 *header;
    u8 pad44[0x0C];
    f32 *shading;
    u8 pad54[0x14];
    ParticleEmitterResource **resources;
    u8 pad6C[0x26];
    u8 hasMovingParticle;
    u8 pointSetIndex;
} ParticleEmitterObject;

typedef struct EmittedParticle {
    u8 pad00[0x1C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad28[0x28];
    s32 flags;
} EmittedParticle;

typedef struct ParticleParent {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} ParticleParent;

struct CircularParticle {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 transformFlags;
    f32 scale;
    f32 renderX;
    f32 renderY;
    f32 renderZ;
    f32 scaleVelocity;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 textureFrame;
    s16 type;
    u8 kind;
    u8 state;
    f32 x;
    f32 y;
    f32 z;
    f32 movementValue;
    f32 gravity;
    u8 *colorTable;
    void *parent;
    void *resource;
    s32 flags;
    s32 value54;
    ParticleTrigger *trigger;
    s16 lifetime;
    s16 angularVelocityY;
    s16 angularVelocityX;
    s16 angularVelocityZ;
    s16 intensity;
    s16 intensityVelocity;
    s16 intensityTimer;
    s16 colorIndex;
    s16 colorTimer;
    u8 colorCount;
    u8 alpha;
    u8 colorMode;
    u8 red;
    u8 green;
    u8 blue;
    u8 alternateRed;
    u8 alternateGreen;
    u8 alternateBlue;
    u8 updateTexture;
};

typedef struct ParticleRenderTransform {
    s16 rotationY;
    s16 rotationX;
    s16 rotationZ;
    s16 flags;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} ParticleRenderTransform;

typedef struct ParticleRenderResourceHeader {
    u8 pad00[0x68];
    void *displayList;
} ParticleRenderResourceHeader;

typedef struct ParticleRenderResource {
    ParticleRenderResourceHeader *header;
    void *vertices;
    u8 pad08[2];
    s16 triangleIndex;
    void *triangles[1];
} ParticleRenderResource;

typedef struct ParticleSpriteResource {
    u8 pad00[6];
    s16 flags;
    u8 red;
    u8 green;
    u8 blue;
    u8 alternateRed;
    u8 alternateGreen;
    u8 alternateBlue;
} ParticleSpriteResource;

typedef struct CircularParticlePool {
    u8 pad00[0x14];
    CircularParticle *particles;
    s32 count;
    s32 activeCount;
    u32 *freeBits;
    s32 lastBitWord;
    s32 exhausted;
} CircularParticlePool;

typedef struct ParticlePosition {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} ParticlePosition;

extern f32 D_8007C8F8;
extern f32 D_8007C8F0;
extern f32 D_8007C8F4;
extern s32 D_8007C854;
extern s32 D_8007C85C;
extern f32 D_80082A48;
extern f32 D_80082A4C;
extern f32 D_80082A50;
extern f32 D_80082A54;
extern f32 D_8007C8C8[];
extern void **D_8007C884;
extern s32 D_8007C888;
extern s32 D_8007C88C;
extern ParticleLineEntry *D_8007C894;
extern void *D_8007C89C[2];
extern s32 D_8007C8B0;
extern ParticleConfig **D_8007C8B8;
extern ParticleTrigger *D_8007C8BC;
extern s32 D_8007C8C0;
extern s32 D_8007C8C4;
extern s32 D_8007C890;
extern s32 D_8007C8E8;
extern s32 D_8007C8EC;
extern ParticleModelEntry *D_8007C898;
extern ParticleTypeDescriptor **D_8007C8AC;
extern s32 D_800D4140;
extern CircularParticlePool *D_800D4120[];
extern CircularParticlePool *D_800D4124;
extern CircularParticlePool *D_800D4128;
extern CircularParticlePool *D_800D412C;
extern CircularParticlePool *D_800D4130[];
extern f32 D_800D4134;
extern f32 D_800D4138;
extern f32 D_800D413C;
extern void *D_8007CA60;
extern f32 *D_8007CA90[];
extern void *D_8007CA98;
extern u8 D_7C900[];


#endif
