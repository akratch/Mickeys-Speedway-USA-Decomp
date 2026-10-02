#ifndef OVERLAY_008_H
#define OVERLAY_008_H

#include "PR/ultratypes.h"

typedef struct Overlay8IndexedObject {
    u8 pad0;
    s8 index;
} Overlay8IndexedObject;

typedef struct O8P1294Owner O8P1294Owner;
typedef struct O8P1294State O8P1294State;

typedef struct O8Node {
    void *resource;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *items[1];
} O8Node;

typedef struct O8ChildFlag { u8 pad00[0x1e]; s8 gate; } O8ChildFlag;
typedef struct O8OwnerPeer { u8 pad00[0x63]; u8 gate63; } O8OwnerPeer;

typedef struct O8Owner {
    u8 pad00[0xc];
    s32 valueC, value10, value14;
    u8 pad18[0x10];
    f32 position28;
    u8 pad2c[0xf];
    s8 mode3B;
    u8 pad3c[4];
    u8 *children40;
    u8 pad44[4];
    O8OwnerPeer *peer48;
    u8 pad4c[4];
    void *value50;
    u8 pad54[0x14];
    O8Node **node68;
    u8 pad6c[0x14];
    s32 flags80;
    u8 pad84[0xf];
    u8 childIndex93;
} O8Owner;

typedef struct O8State {
    u8 pad00[2];
    u8 condition2;
    u8 pad03;
    f32 lateral4;
    u8 pad08[0x68];
    void *value70;
    f32 vector74, vector78, vector7C;
    f32 value80, value84;
    u8 pad88[0x30];
    void *resourceB8;
    u8 padBC[8];
    void *resourceC4;
    u8 padC8[0xc];
    s32 conditionD4;
    u8 padD8[0x2e];
    s16 angle106;
    u8 pad108[0x2c];
    void *resource134;
    void *resource138;
    u8 pad13c[0x36];
    s8 condition172;
    u8 pad173[0xe];
    u8 active181;
    u8 pad182[0x1a0];
    u8 selector322;
    u8 selector323;
    u8 pad324[0x26];
    u8 mode34A;
    u8 pad34B[0x68];
    u8 timer3B3;
    u8 pad3B4[0x88];
    s16 angle43C;
} O8State;

#define O8_F32(address) (*(f32 *)(address))
#define O8_S32(address) (*(s32 *)(address))
#define O8_S32_ARRAY(address) ((s32 *)(address))
#define O8_PTR(address) (*(void **)(address))
#define gOverlay8MotionOutsideValueReloc D_28C

typedef struct Overlay8MotionRecord {
    u8 pad000[0xC4];
    void *resource;
    u8 pad0C8[0x38];
    s16 direction;
    s16 activeDirection;
    u8 pad104[4];
    s16 fallbackSign;
    u8 pad10A[0x5E];
    s16 gate168;
    s16 gate16A;
} Overlay8MotionRecord;

typedef struct Overlay8ActivationState {
    s8 type;
    u8 pad001[0xC3];
    void *resource;
    u8 pad0C8[0x3A];
    s16 activeDirection;
    u8 pad104[0x54];
    s16 gate158;
    u8 pad15A[0x29];
    u8 active183;
    u8 pad184;
    u8 active185;
    u8 pad186;
    u8 timer187;
    u8 pad188[0x20];
    u16 flags1A8;
} Overlay8ActivationState;

typedef struct Overlay8ActivationOwner {
    u8 pad00[0x0C];
    s32 valueC;
    s32 value10;
    s32 value14;
    u8 pad18[0x4C];
    Overlay8ActivationState *state;
} Overlay8ActivationOwner;

typedef struct O8PhaseState {
    u8 pad000[0x16A];
    s16 timer;
    u8 pad16C[0x18];
    u8 forceEffect;
    u8 phase;
    u8 effect;
    u8 countdown;
    f32 weight;
    u8 pad18C[0x290];
    u32 flags;
} O8PhaseState;

typedef struct O8P2640Anchor {
    s16 helperInput0;
    u8 pad002[0x0A];
    f32 coordC;
    f32 coord10;
    f32 coord14;
} O8P2640Anchor;

typedef struct O8P2640Config {
    u8 pad00;
    s8 tuningIndex1;
} O8P2640Config;

typedef struct O8P2640Tuning {
    f32 extent0;
    f32 offset4;
    f32 spreadScale8;
} O8P2640Tuning;

typedef struct O8P2640Record {
    s16 value0;
    s16 value2;
    f32 magnitude4;
    f32 coord8;
    f32 coordC;
    f32 coord10;
    f32 phase14;
    s16 size18;
    u8 kind1A;
    u8 pad1B;
    u32 packed1C;
    u32 packed20;
    u32 packed24;
    u32 packed28;
    u32 packed2C;
    u32 packed30;
} O8P2640Record;

typedef struct O8P291CLink {
    u8 pad00[0x62];
    u8 gate62;
} O8P291CLink;

typedef struct O8P291CMotion {
    s16 heading0;
    u8 pad02[0x0A];
    f32 positionC;
    f32 position10;
    f32 position14;
    u8 pad18[4];
    f32 velocity1C;
    f32 velocity20;
    f32 velocity24;
    u8 pad28[6];
    s16 state2E;
    u8 pad30[0x18];
    O8P291CLink *link48;
} O8P291CMotion;

typedef struct O8P291CState {
    u8 pad00[4];
    f32 control4;
    f32 control8;
    f32 blendC;
    u8 pad10[0x28];
    f32 origin38;
    f32 origin3C;
    f32 origin40;
    u8 pad44[0x30];
    f32 axis74;
    f32 axis78;
    f32 axis7C;
    f32 speed80;
    f32 speed84;
    f32 accel88;
    u8 pad8C[8];
    f32 delta94;
    f32 delta98;
    f32 delta9C;
    u8 padA0[0x50];
    s16 angleF0;
    u8 padF2[0x0A];
    s16 angleFC;
    s16 angleFE;
    u8 pad100[4];
    s16 angle104;
    u8 pad106[0x64];
    s16 mode16A;
    u8 pad16C[4];
    u8 reset170;
    u8 pad171[0x10];
    u8 active181;
    u8 pad182[3];
    u8 suppress185;
    u8 pad186[0x296];
    u32 flags41C;
    u8 pad420[0x18];
    s32 mode438;
} O8P291CState;

typedef struct O8P291CBlendView {
    u8 pad00[0x0C];
    f32 blendC;
} O8P291CBlendView;

typedef struct Overlay8UpdateFlag {
    u8 flags;
} Overlay8UpdateFlag;

typedef struct Overlay8UpdateChild {
    u8 pad00[0x75];
    u8 near;
    u8 pad76[0x0A];
    f32 target80;
    Overlay8UpdateFlag *flag;
} Overlay8UpdateChild;

typedef struct Overlay8UpdateOwner {
    u8 pad00[0x10];
    f32 position10;
    u8 pad14[0x0C];
    f32 velocity20;
    u8 pad24[0x30];
    Overlay8UpdateChild *child;
} Overlay8UpdateOwner;

typedef struct Overlay8UpdateInput {
    u8 pad00[2];
    u8 active;
    u8 pad03;
    f32 lateral;
    u8 pad08[0x60];
    f32 position68;
    f32 delta6C;
} Overlay8UpdateInput;

typedef struct Overlay8ChannelState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x114];
    f32 values[4];
    u8 modes[4];
    u8 phases[4];
    u8 pad134[0x188];
    s32 selectorMode;
    u8 pad2C0[0x60];
    u8 selectors[4];
} Overlay8ChannelState;

typedef struct Overlay8ColorSource {
    u8 pad00[0x38];
    u8 red;
    u8 green;
    u8 blue;
} Overlay8ColorSource;

typedef struct Overlay8ColorOwner {
    u8 pad00[0x64];
    Overlay8ColorSource *colors;
} Overlay8ColorOwner;

typedef struct Overlay8ColorState {
    u8 pad000[0xD0];
    Overlay8ColorOwner *owner;
    u8 pad0D4[0x96];
    s16 timer16A;
    u8 pad16C[0x18];
    u8 alternate184;
    u8 pad185;
    u8 flags186;
    u8 pad187[0x1CD];
    void *target354;
    u8 pad358[8];
    void *target360;
} Overlay8ColorState;

typedef struct Overlay8ScaleRecord {
    u8 pad00[0x0E];
    u16 scale;
} Overlay8ScaleRecord;

typedef struct Overlay8ScaleSlot {
    Overlay8ScaleRecord *record;
    u32 pad04;
} Overlay8ScaleSlot;

typedef struct Overlay8ScaleContext {
    u8 pad00[0x18];
    Overlay8ScaleSlot *slots;
    u8 pad1C[0x10];
    u8 count;
} Overlay8ScaleContext;

typedef struct Overlay8ScalePair {
    s16 first;
    s16 second;
    u32 selector;
} Overlay8ScalePair;

typedef struct Overlay8ScaleOutput {
    u8 pad00[0x0A];
    s16 outputIndex;
    u8 pad0C[0x40];
    Overlay8ScalePair *pairs;
    s16 *outputs[];
} Overlay8ScaleOutput;

typedef struct Overlay8ScaleState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x414];
    u32 flags41C;
    u8 pad420[0x0C];
    s32 value42C;
} Overlay8ScaleState;

typedef struct O8P42A8Actor {
    s16 angle0;
    s16 angle2;
    s16 angle4;
    u8 pad06[6];
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0xC];
    f32 motion24;
    f32 motion28;
} O8P42A8Actor;

typedef struct O8P42A8State {
    s8 mode0;
    u8 pad001[3];
    f32 velocity4;
    u8 pad008[0xC];
    f32 offset14;
    f32 offset18;
    f32 offset1C;
    u8 pad020[0xBC];
    s16 directionDC;
    u8 pad0DE[2];
    f32 steeringE0;
    f32 accelerationE4;
    f32 heightE8;
    f32 tiltEC;
    s16 angleF0;
    u8 pad0F2[0xE];
    s16 modifier100;
    s16 sign102;
    u8 pad104[4];
    s16 magnitude108;
    u8 pad10A[0x66];
    u8 reset170;
    u8 pad171[0x13];
    u8 double184;
    u8 force185;
    u8 pad186[7];
    s8 special18D;
    u8 pad18E[3];
    s8 lock191;
    u8 pad192[0x1B7];
    u8 lowering349;
    u8 pad34A[0xD2];
    u32 flags41C;
    u32 cycleFlags420;
} O8P42A8State;

typedef struct O8P42A8Owner {
    u8 pad00[2];
    s16 angle2;
    s16 angle4;
    u8 pad06[6];
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0x4C];
    O8P42A8State *state64;
} O8P42A8Owner;

typedef struct O8P0058Owner O8P0058Owner;
typedef struct O8P0058State O8P0058State;

typedef struct O8P0058PeerState {
    u8 pad00[0x14];
    f32 blend14;
} O8P0058PeerState;

typedef struct O8P0058Peer {
    u8 pad00[0x64];
    O8P0058PeerState *state64;
} O8P0058Peer;

struct O8P0058Owner {
    s16 angle0;
    s16 angle2;
    s16 angle4;
    u8 pad06[6];
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0x23];
    s8 mode3B;
    u8 pad3C[0x28];
    O8P0058State *state64;
    u8 pad68[0x18];
    s32 flags80;
};

struct O8P0058State {
    s8 mode0;
    u8 pad001;
    u8 surfaceActive2;
    u8 surfaceMode3;
    f32 lower4;
    f32 upper8;
    u8 pad00C[0x44];
    f32 position50;
    f32 bounce54;
    u8 pad058[4];
    f32 direction5C;
    f32 direction60;
    f32 direction64;
    f32 surface68;
    f32 surface6C;
    f32 value70;
    u8 pad074[0x44];
    void *resourceB8;
    u8 pad0BC[0x18];
    O8P0058Peer *peerD4;
    u8 pad0D8[0x18];
    s16 angleF0;
    u8 pad0F2[0x66];
    s16 gate158;
    u8 pad15A[0x10];
    s16 active16A;
    u8 mode16C;
    u8 pad16D;
    s8 counter16E;
    u8 pad16F;
    u8 reset170;
    u8 pad171;
    s8 override172;
    u8 pad173[9];
    f32 override17C;
    u8 pad180[3];
    u8 active183;
    u8 alternate184;
    u8 pad185[7];
    u8 bounceActive18C;
    s8 disabled18D;
    u8 pad18E[0x192];
    u8 selectors320[4];
    u8 pad324[0x25];
    u8 lowering349;
    u8 pad34A[0xB0];
    s16 gate3FA;
    f32 bounceVelocity3FC;
    u8 pad400[0x1C];
    u32 flags41C;
    u32 modeFlags420;
    u8 pad424[4];
    s32 signed428;
    s32 value42C;
    s32 signed430;
    s32 value434;
};

typedef struct O8P0058Query {
    f32 initial0;
    u8 pad004[0x144];
    f32 heights148[1];
} O8P0058Query;

typedef struct O8P0058Surface {
    f32 height0;
    u32 flags4;
} O8P0058Surface;

typedef struct O8P34A0Peer {
    u8 pad00[0x63];
    u8 gate63;
} O8P34A0Peer;

typedef struct O8P34A0Owner {
    u8 pad00[0xC];
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0x10];
    f32 scale28;
    u8 pad2C[0xF];
    s8 mode3B;
    u8 pad3C[0xC];
    O8P34A0Peer *peer48;
} O8P34A0Owner;

typedef struct O8P34A0State {
    u8 pad000;
    s8 kind1;
    u8 pad002[2];
    f32 motion4;
    u8 pad008[0x84];
    f32 value8C;
    f32 value90;
    u8 pad094[0x6C];
    s16 direction100;
    s16 secondary102;
    u8 pad104[4];
    s16 steering108;
    u8 pad10A[6];
    s16 angle110;
    s16 angle112;
    s16 angles114[4];
    u8 pad11C[0x28];
    s16 angle144;
    s16 angle146;
    u8 pad148[0x24];
    u8 mode16C;
    u8 pad16D[5];
    s8 override172;
    u8 pad173[0xE];
    u8 active181;
    u8 pad182[3];
    u8 force185;
    u8 pad186[0xD];
    u8 overrideMode193;
    u8 raw194[7];
    u8 gate19B;
    s32 gate19C;
    u8 pad1A0[8];
    u16 flags1A8;
    u8 pad1AA[0x19F];
    u8 lowering349;
    u8 motion34A;
    u8 pad34B[0x3F];
    s16 smoothed38A;
    u8 pad38C[0x3C];
    f32 activity3C8;
    u8 pad3CC[0x20];
    f32 phase3EC;
    f32 phase3F0;
    s16 output3F4;
    s16 output3F6;
    s16 output3F8;
    u8 pad3FA[0x22];
    u32 flags41C;
    u8 pad420[8];
    s32 steering428;
} O8P34A0State;

typedef struct O8P34A0Query {
    f32 **samples0;
    s32 scratch04;
    s32 scratch08;
    s32 scratch0C;
    s32 scratch10;
} O8P34A0Query;

typedef struct Overlay8MotionRow {
    u8 firstSelector;
    u8 secondSelector;
    u8 pad002[2];
    s16 firstScale;
    s16 secondScale;
} Overlay8MotionRow;

typedef struct Overlay8MotionAnchor {
    s16 helperInput;
    u8 pad002[0x0A];
    f32 x;
    u8 pad010[4];
    f32 y;
} Overlay8MotionAnchor;

typedef struct Overlay8MotionState {
    u8 pad000;
    s8 rowIndex;
    u8 pad002[0x191];
    u8 outsideLatch;
    f32 outsideValue;
    u8 pad198[0x24C];
    Overlay8MotionAnchor *target;
    s16 primary;
    s16 secondary;
} Overlay8MotionState;

typedef struct O8P4CF0Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O8P4CF0Vec3f;

typedef struct O8P4CF0Normal {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} O8P4CF0Normal;

typedef struct O8P4CF0Actor {
    s16 angle000;
    s16 angle002;
    s16 angle004;
    u8 pad006[6];
    f32 x00C;
    f32 y010;
    f32 z014;
    u8 pad018[8];
    f32 vertical020;
} O8P4CF0Actor;

typedef struct O8P4CF0State {
    u8 pad000[4];
    f32 motion004;
    u8 pad008[0x16A];
    s8 timer172;
    u8 activated173;
    f32 blend174;
    f32 height178;
    f32 derived17C;
} O8P4CF0State;

typedef struct O8P4CF0Bounds {
    s16 minX000;
    s16 unused002;
    s16 minZ004;
    s16 extentX006;
    s16 extentZ008;
} O8P4CF0Bounds;

typedef struct O8P4CF0SceneItem {
    u8 pad000[0x44];
    s16 category044;
    u8 pad046[0x3E];
    O8P4CF0Bounds *bounds084;
    s32 callbackGate088;
} O8P4CF0SceneItem;

extern u8 gOverlay8IndexMode;

extern void *gOverlay8Primary[];

extern void *gOverlay8Secondary[];

extern void o8Call0894Reloc(O8Node *, void *, O8Owner *);

extern void ext_o0_19668(O8Owner *, O8Node *, void *, void *);

extern void ext_o8_3368(O8Owner *, O8State *, void *, O8Node *, s32);

extern f32 gO8FloatCC, gO8FloatD0, gO8FloatD4, gO8FloatD8;

extern f32 gO8FloatDC, gO8FloatE0;

extern void *gO8Pointer14, *gO8Pointer18;

extern s32 gO8Table360[], gO8Table3A0[];

extern s32 gO8Value364, gO8Value3A4, gO8Value370, gO8Value3B0;

extern void ext_o0_1eed0_target(O8State *, s32, u32);

extern f32 ext_o0_2a46c(s16);

extern f32 ext_o0_2a470(s16);

extern void o8Call0894EmitReloc(O8Owner *, O8State *, s32,
                                               s32, s32, f32, f32, f32, s32);

extern void ext_o8_3278(O8Owner *, O8State *, s32);

extern void ext_o8_2ec0(O8Owner *, O8State *, s32, s32, s32);

extern void ext_o0_1d510(O8Owner *, O8State *, s32);

extern void ext_o0_2d98(void *);

extern void ext_o0_2b90(s32, s32, s32, s32, s32, void **);

extern void ext_o7_ccc(O8Owner *, s32);

extern void ext_o8_3018(O8Owner *, O8State *, void *, s32);

extern void ext_o0_3e990(f32);

extern void ext_o0_3e99c(O8Owner *, s32);

extern void ext_o17_668(void *, void *);

extern void ext_o0_2d70(void *, s32, s32, s32);

extern void o8StartMotionResourceReloc(void *resource);

extern s32 gOverlay8ActivationGateTimerReloc;

extern void overlay8ReleaseResourceReloc(void *resource);

extern void overlay8CreateResourceReloc(s32 kind, s32 valueC, s32 value10,
                                        s32 value14, s32 count,
                                        void **resourceOut);

extern void overlay8FinalizeActivationReloc(Overlay8ActivationOwner *owner,
                                            s32 code);

extern u8 gO8RolloverControlReloc;

extern u8 gO8Phase2ScaleControlReloc;

extern u8 gO8Phase3ScaleControlReloc;

extern const f32 gO8Phase2TargetReloc;

extern const f32 gO8Phase2ScaleReloc;

extern const f32 gO8Phase3DecayReloc;

extern const f32 gO8RetireThresholdReloc;

extern const f32 gO8Phase3ScaleReloc;

extern f32 o8RolloverSampleReloc(void);

extern void o8Phase1EmitReloc(O8PhaseState *state, s32 kind, f32 scale);

extern const O8P2640Tuning D_2110[];

extern f32 O8P2640_call_26AC(f32 squaredDistance);

extern s32 O8P2640_call_26F0(f32 negX, f32 negZ);

extern f32 O8P2640_call_26FC(s32 anchorHalfword);

extern f32 O8P2640_call_2708(s32 anchorHalfword);

extern s32 O8P2640_call_27BC(s32 low, s32 high);

extern s32 O8P2640_call_27CC(s32 low, s32 high);

extern s32 O8P2640_call_27DC(s32 low, s32 high);

/* Declared with a result: the caller's dead count copy takes v1, not v0. */
extern s32 O8P2640_call_28C0(O8P2640Record *record);

extern f32 O8P291C_data_1A0;

extern f32 O8P291C_data_1A4;

extern f32 O8P291C_data_1A8;

extern f32 O8P291C_data_1AC;

extern f32 O8P291C_gravity;

extern f32 D_10;

extern f32 o8Approach291CReloc(f32 value, s32 updateRate);

extern f32 O8P291C_call_sin(s16 angle);

extern f32 O8P291C_call_cos(s16 angle);

extern void o8Surface291CReloc(
    O8P291CMotion *motion, O8P291CState *state, s32 updateRate);

extern s32 O8P291C_call_037C(O8P291CMotion *motion, O8P291CState *state,
                             f32 update);

extern s32 O8P291C_call_039C(O8P291CMotion *motion, f32 x, f32 y, f32 z);

extern const f32 gOverlay8UpdateLowerReloc;

extern const f32 gOverlay8UpdateUpperReloc;

extern const f32 gOverlay8UpdateDecayReloc;

extern void overlay8FinishUpdateReloc(Overlay8UpdateOwner *owner,
                                      s32 updateRate);

extern const f32 gOverlay8PhaseScales[];

extern const f32 gOverlay8SelectorScales[];

extern f32 overlay8SampleChannel(f32 argument, void *sampleState);

extern void overlay8EmitChannel(Overlay8ChannelState *state, s32 kind,
                                f32 scale);

extern void o8ApplyColorsReloc(void *target, s32 alpha0,
                                               s32 alpha1, s32 alpha2,
                                               s32 red, s32 green, s32 blue);

extern const f32 gOverlay8ScaleLowerReloc;

extern const f32 gOverlay8ScaleUpperReloc;

extern u8 D_0[];

extern f32 D_2210[];
extern f32 D_2188[];
extern f32 D_21C8[];
extern s16 D_2208[];
extern f32 D_2220[];

extern f32 o8P42A8SampleReloc(f32 value, s32 mode);

extern s32 o8P42A8RandomReloc(void);

extern s32 o8P42A8ApproachReloc(s32 current, s32 target);

extern f32 o8P42A8TrigAReloc(s32 angle);

extern f32 o8P42A8TrigBReloc(s32 angle);

extern f32 D_8;

extern f32 D_C;

extern s32 D_14;

extern s32 D_18;

extern f32 D_B0;

extern f32 D_B4;

extern f32 D_B8;

extern f32 D_BC;

extern f32 D_C0;

extern f32 D_C4;

extern f32 D_C8;

extern f32 D_310[];

extern u8 D_350[];

extern u8 gO8P0058MirrorGateReloc;

extern s32 gO8P0058PresentReloc;

extern f32 gO8P0058ResultReloc;

extern s32 gO8P0058ActiveReloc;

extern u8 gO8P0058SpawnGateReloc;

extern void o8P0058ResetReloc(O8P0058State *state, s32 mode);

extern void o8P0058ModeReloc(O8P0058State *state, s32 mode);

extern O8P0058Query *o8P0058AcquireReloc(O8P0058State *state);

/* Resident +0x5A914, the same mode setter o8P34A0SetModeReloc calls. */
extern void o8P0058SpawnReloc(O8P0058Owner *owner, s32 kind, s32 index,
                              f32 blend);

extern void o8P0058OrientReloc(O8P0058Owner *owner, O8P0058State *state);

extern void o8P0058RotateReloc(s16 *angles, f32 *vector);

extern s32 o8P0058SurfaceReloc(f32 x, f32 z, s32 unused, u32 flags,
                               O8P0058Surface *surface);

extern void o8P0058CollisionReloc(O8P0058Owner *owner,
                                  O8P0058State *state);

extern void o8P0058EffectReloc(O8P0058State *state, s32 kind, f32 scale,
                               f32 *value);

extern f32 o8P0058SampleReloc(O8P0058Owner *owner, O8P0058State *state,
                              f32 limit, f32 update);

extern void o8P0058UpdateReloc(O8P0058Owner *owner, O8P0058State *state,
                               s32 updateRate);

extern void o8P0058ReleaseReloc(void *resource);

extern void o8P0058CreateReloc(s32 kind, f32 x, f32 y, f32 z, s32 mode,
                               void **resource);

extern void o8P0058BounceReloc(O8P0058State *state, s32 kind, f32 scale);

/* The +0x1294 update's two same-module calls are SYMBOL records (the shipped
 * words carry the module base), so they go through placeholders as the
 * +0x0058 body's do: the indexed tuning lookup and the +0x1000 phase helper. */
extern void *o8P1294IndexedReloc(Overlay8IndexedObject *object);

extern f32 o8P1294PhaseReloc(void *unused, O8PhaseState *state, f32 input);

extern void func_overlay_008_F0001294_185EFEC(O8P1294Owner *owner,
                                              O8P1294State *state,
                                              f32 update);

extern void func_overlay_008_F000291C_1860674(O8P291CMotion *motion,
                                              O8P291CState *state,
                                              f32 update);

extern f32 D_1D8;
extern f32 D_1DC;
extern f32 D_1E0;
extern f32 D_1E4;
extern f32 D_1E8;
extern f32 D_1EC;
extern f32 D_1F0;
extern f32 D_1F4;
extern f32 D_1F8;
extern f32 D_1FC;
extern f32 D_200;
extern f32 D_204;
extern f32 D_208;
extern f32 D_20C;
extern f32 D_210;
extern f32 D_214;
extern f32 D_218;
extern f32 D_21C;
extern f32 D_220;
extern f32 D_224;
extern f32 D_228;
extern f32 D_22C;
extern f32 D_230;
extern f32 D_234;
extern f32 D_238;
extern f32 D_23C;
extern f32 D_240;
extern f32 D_244;
extern f32 D_248;
extern f32 D_24C;
extern f32 D_250;
extern f32 D_254;
extern f32 D_258;
extern f32 D_25C;

extern f32 gO8P34A0ScaleReloc;

extern u8 gO8P34A0ModeReloc;

extern s32 o8P34A0RandomReloc(s32 low, s32 high);

extern s32 o8P34A0TerrainReloc(f32 x, f32 z, s32 flags,
                               f32 ***samples);

/* overlay7DispatchSelection(owner, selection): two arguments. */
extern void o8P34A0EffectReloc(O8P34A0Owner *owner, s32 kind);

extern void o8P34A0SetModeReloc(O8P34A0Owner *owner, s32 mode, s32 index,
                                f32 blend);

extern s32 o8P34A0AnimateReloc(O8P34A0Owner *owner, f32 value,
                               f32 update);

extern void o8P34A0EventReloc(O8P34A0Owner *owner, s32 kind, s32 mode);

extern void o8P34A0StateEffectReloc(O8P34A0State *state, s32 kind,
                                    f32 scale);

extern s32 o8P34A0ApproachReloc(s32 current, s32 target);

extern f32 o8P34A0TrigAReloc(s32 angle);

extern f32 o8P34A0TrigBReloc(s32 angle);

extern f32 o8P34A0DecayReloc(f32 coefficient, s32 updateRate);

extern s32 o8P34A0BlendReloc(s32 current, s32 target, f32 blend);

extern void func_overlay_008_F00049E8_1862740(O8P34A0Owner *owner,
                                              O8P34A0State *state,
                                              f32 update);

extern s16 *gOverlay8Buffer;
/* The same command cursor (the BSS owner's commandBuffer) under the second
 * name the +0x34A0 body uses from its second command pair on. */
extern s16 *gOverlay8CommandBuffer;

extern s16 gOverlay8Value;

extern const f32 D_28C;

extern const Overlay8MotionRow D_2230[];

extern s32 overlay8MeasureDirectionReloc(f32 deltaX, f32 deltaY);

extern s32 overlay8ConvertDirectionReloc(s32 anchorValue,
                                         s32 directionCode);

extern f32 overlay8ApproachMotionReloc(s32 difference,
                                       f32 doubledSecondary, f32 limit);

extern s32 O8P4CF0_call_4D14(s32 selector);

extern O8P4CF0SceneItem **O8P4CF0_call_4D24(s32 *start, s32 *end);

extern void O8P4CF0_call_4D54(s32 mode, O8P4CF0Actor *actor,
                              O8P4CF0Vec3f *input, O8P4CF0Vec3f *output);

extern void O8P4CF0_call_4E50(O8P4CF0SceneItem *item);

extern f32 O8P4CF0_call_4E64(O8P4CF0Bounds *bounds, f32 x, f32 z,
                             O8P4CF0Normal *normal);

extern f32 O8P4CF0_call_4E9C(s32 angle);

extern f32 O8P4CF0_call_4EAC(s32 angle);

extern s32 O8P4CF0_call_4EE8(f32 horizontal, f32 vertical);

extern s32 O8P4CF0_call_4EF8(f32 horizontal, f32 vertical);

extern f32 O8P4CF0_call_4F0C(f32 amount, s32 updateRate);

extern s32 O8P4CF0_call_4F34(s32 current, s32 target, f32 factor);

extern f32 O8P4CF0_call_4F48(f32 amount, s32 updateRate);

extern s32 O8P4CF0_call_4F68(s32 current, s32 target, f32 factor);

extern const f32 O8P4CF0_data_290;

extern const f32 O8P4CF0_data_294;

extern const f32 O8P4CF0_data_298;

extern const f32 O8P4CF0_data_29C;

extern const f32 O8P4CF0_data_2A0;

extern const f32 O8P4CF0_data_2A4;

extern const f32 O8P4CF0_data_2A8;

extern f32 O8P4CF0_data_4FD4;

#endif
