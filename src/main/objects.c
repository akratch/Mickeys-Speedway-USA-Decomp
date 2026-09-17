#include "PR/ultratypes.h"
#include "game/particles.h"

/* This TU uses -Wab,-r4300_mul; target func_8000A62C requires its three
 * R4300 multiply-hazard delay nops, and the flag is validated against the
 * existing exact object-system functions by the full-ROM verify. */

typedef struct AnimPathObject AnimPathObject;
typedef struct Gfx Gfx;
typedef struct Mtx Mtx;
typedef struct TrackVertex TrackVertex;

typedef struct TrackSkyObject {
    u8 pad00[6];
    s16 flags;
} TrackSkyObject;

typedef struct {
    u8 pad[0x58];
    s32 unk58;
} Objects06C40;

typedef struct {
    s16 unk0;
    u8 pad02[2];
    f32 unk4;
} Objects69C0Out;

typedef struct {
    u8 pad00[0x2C];
    f32 unk2C;
} Objects69C0Deep;

typedef struct {
    u8 pad00[0xE0];
    Objects69C0Deep *unkE0;
} Objects69C0Mid;

typedef struct {
    u8 pad00[0x40];
    Objects69C0Mid *unk40;
    u8 pad44[0x34];
    Objects69C0Out *unk78;
} Objects69C0In;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E[4];
    s8 unk22;
    u8 pad23[0xAD];
    f32 unkD0[4];
} Objects58C0Data;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
} Objects58C0Arg;

typedef struct {
    u8 pad00[0x44];
    s16 unk44;
    u8 pad46[0x22];
    s32 *unk68;
    u8 pad6C[0x1C];
    void *unk88;
} Objects08A20Arg;

typedef struct {
    u8 pad00[0x30];
    f32 unk30;
} Objects09F08Arg;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    s8 unk1E[4];
    s8 unk22;
    u8 pad23[0xB1];
    f32 unkD4[3];
} Objects09F74Data;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x22];
    s8 unk3A;
    u8 pad3B[5];
    Objects09F74Data *unk40;
    s16 unk44;
    u8 pad46[0x1E];
    void *unk64;
    u8 pad68[0x2B];
    u8 unk93;
} Objects09F74Object;

typedef struct {
    u8 pad00[0x4E];
    u8 unk4E;
} Objects09F74Camera;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects0BB84Vec3;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[4];
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
} Objects0BB84Plane;

typedef struct {
    u8 pad00[8];
    f32 x;
    f32 y;
    f32 z;
} Objects0BB84Output;

typedef struct {
    u8 pad00[0x10];
    f32 unk10;
} Objects0BB84Depth;

typedef struct {
    u8 pad00[0xE0];
    Objects0BB84Depth *unkE0;
} Objects0BB84Node;

typedef struct {
    u8 pad00[0x40];
    Objects0BB84Node *unk40;
    u8 pad44[0x34];
    Objects0BB84Output *unk78;
} Objects0BB84Object;

typedef struct {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    s32 unk14;
    u8 pad18[4];
    u16 unk1C;
    u16 unk1E;
    f32 unk20;
    f32 unk24;
    f32 unk28;
} Objects0B3CCConfig;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    u16 unk1E;
    u8 pad20[0x30];
    f32 unk50;
    u8 pad54[0x8C];
    Objects0B3CCConfig *unkE0;
} Objects0B3CCData;

typedef struct {
    s16 flags;
    s16 unk2;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    void *unk24;
} Objects0B3CCState;

typedef struct {
    u8 pad00[2];
    s16 unk2;
    s16 unk4;
    u8 pad06[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[4];
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    u8 pad28[0x18];
    Objects0B3CCData *unk40;
    u8 pad44[0x34];
    Objects0B3CCState *unk78;
    u8 pad7C[4];
    s32 unk80;
} Objects0B3CCObject;

extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern void func_8000309C(void *handle, u8 volume);
extern void trackMakePolylist(s32 count, Objects0BB84Vec3 *start,
                              Objects0BB84Vec3 *end, f32 *radius, s32 arg4,
                              s32 arg5);
extern s32 func_80010900(Objects0BB84Vec3 *start, Objects0BB84Vec3 *end,
                         f32 radius, s32 actor, void *callback);
extern u32 func_8001357C(f32 arg0, f32 arg1, f32 *arg2, s32 arg3,
                         void *arg4);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern void func_8000BB84(s32 arg0, Objects0BB84Vec3 *arg1,
                          Objects0BB84Vec3 *arg2, f32 arg3,
                          Objects0BB84Plane *arg4, Objects0BB84Object *arg5);
extern f32 D_8008152C;
extern f32 D_80081530;
extern f32 D_80081534;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
    u8 pad44[0x24];
    s32 *unk68;
} Objects06448Arg;

typedef struct {
    u8 pad00[8];
    s16 unk8;
    u8 pad0A[0x35];
    u8 unk3F;
} Objects08028Model;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
    u8 pad44[0x24];
    Objects08028Model **unk68;
} Objects08028Object;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
    u8 pad1F[0x35];
    f32 unk54;
    f32 unk58;
    u8 pad5C[5];
    u8 unk61;
    u8 unk62;
    u8 unk63;
    s16 unk64;
    u16 unk66;
} Objects069E8Source;

typedef struct {
    f32 unk0;
    f32 unk4;
    s32 unk8;
    u16 unkC;
    u16 unkE;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 pad14[8];
    s32 unk1C;
} Objects069E8Target;

typedef struct {
    u8 pad00[0x40];
    Objects069E8Source *unk40;
    u8 pad44[8];
    Objects069E8Target *unk4C;
} Objects069E8Object;

typedef struct {
    u8 pad00[0xA6];
    u8 unkA6;
    u8 padA7;
    u8 *unkA8;
    s32 *unkAC;
} Objects04B04Object;

typedef struct {
    s32 unk0;
    s32 unk4;
} Objects06868Entry;

typedef struct {
    u8 pad00[0x25];
    s8 unk25;
    u8 pad26[0x1E];
    Objects06868Entry *unk44;
} Objects06868Data;

typedef struct {
    u8 pad00[0x40];
    Objects06868Data *unk40;
    u8 pad44[0x28];
    ParticleTrigger *unk6C;
} Objects06868Object;

typedef struct {
    u8 pad00[0x14];
    u16 unk14;
} Objects0A244Header;

typedef struct {
    u8 pad00[0x40];
    Objects0A244Header *unk40;
} Objects0A244Object;

typedef struct {
    u8 pad00[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x2C];
    s16 unk44;
    u8 pad46[0x4B];
    u8 unk91;
} Objects04454Object;

typedef struct {
    u8 pad00[0x1B];
    u8 unk1B;
} Objects0471CData;

typedef struct {
    u8 pad00[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x28];
    Objects0471CData *unk40;
    u8 pad44[0x4D];
    u8 unk91;
} Objects0471CObject;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
} Objects06B04Source;

typedef struct {
    s32 unk0;
    s32 unk4;
} Objects0831CCommand;

typedef struct {
    u8 pad00[3];
    u8 unk3;
    u8 pad04[0xA];
    u16 unkE;
    u16 unk10;
    u8 pad12[0xE];
} Objects07C68Texture;

typedef struct {
    s16 unk0;
    s16 unk2;
    s32 unk4;
} Objects07C68Record;

typedef struct {
    u8 pad00[0x18];
    void **unk18;
    u8 pad1C[0x10];
    u8 unk2C;
} Objects07C68Source;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x40];
    Objects07C68Record *unk4C;
    u8 pad50[0x40];
    u8 unk90;
} Objects07C68Object;

typedef struct {
    u8 pad00[0x50];
    s16 *unk50;
} Objects07C68Indexed;

typedef struct {
    u16 unk0;
    s8 unk2;
    u8 unk3;
    u16 unk4;
    u16 unk6;
    f32 unk8;
} Objects06B04Entry;

typedef struct {
    u8 pad00[0x2F];
    u8 unk2F;
    u8 pad30[8];
    u8 *unk38;
} Objects06B04Asset;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects06B04Source *unk40;
    u8 pad44[4];
    void *unk48;
    u8 pad4C[0x1C];
    Objects06B04Asset ***unk68;
} Objects06B04Object;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x68];
    u8 *unk74;
} Objects06B04Output;

typedef struct {
    f32 unk0;
    u8 pad04[0x10];
    u16 unk14;
    u8 pad16[6];
    s16 unk1C;
    u8 pad1E[4];
    s8 unk22;
    u8 pad23[0x11];
    s32 *unk34;
} Objects06C4CAsset;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    f32 unk8;
    u8 pad0C[0x20];
    s16 unk2C;
    u8 pad2E[0x12];
    Objects06C4CAsset *unk40;
    u8 pad44[2];
    s16 unk46;
    u8 pad48[0x20];
    s32 *unk68;
    u8 pad6C[0x28];
    s32 unk94[1];
} Objects06C4CObject;

extern void *func_8000486C(s32 arg0);
extern void *func_80006C4C(s32 arg0);
extern void *func_8001F520(s32 assetId, s32 flags);
extern void *func_8002B314(s32 size, s32 tag);
extern void *func_800355A0(s32 assetId, s32 flags);

typedef struct {
    u8 pad00[6];
    s16 unk6;
    u8 pad08[4];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x18];
    f32 unk30;
} Objects0A39CObject;

typedef struct {
    f32 pad00[2];
    f32 unk8;
    f32 pad0C[3];
    f32 unk18;
    f32 pad1C[3];
    f32 unk28;
    f32 pad2C[3];
    f32 unk38;
} Objects0A39CMatrix;

typedef struct {
    f32 unk0;
    u8 pad04[0x18];
    s16 unk1C;
    u8 pad1E[0x16];
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    u8 pad48[4];
    s32 unk4C;
    s32 unk50;
    u8 pad54[0x52];
    u8 unkA6;
    u8 padA7;
    s32 unkA8;
    s32 unkAC;
    s32 unkB0;
    u8 padB4[0x2C];
    s32 unkE0;
} Objects0486CAsset;

typedef struct {
    u16 unk0;
    s8 unk2;
    u8 unk3;
    s16 unk4;
    u8 pad06[2];
    s32 unk8;
} Objects06534Record;

typedef struct {
    u8 pad00[0x23];
    s8 unk23;
    s8 unk24;
    u8 pad25[0x13];
    s32 *unk38;
    s32 unk3C;
    Objects06534Record *unk40;
} Objects06534Data;

typedef struct {
    s32 count;
    void *entries[10];
    s32 unk2C;
} Objects06534List;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects06534Data *unk40;
    u8 pad44[0x18];
    Objects06534List *unk5C;
    void *unk60;
    u8 pad64[0x28];
    u8 unk8C;
} Objects06534Object;

typedef struct {
    void *unk0;
    s8 unk4;
    u8 unk5;
    u8 pad06[2];
    f32 unk8;
    s32 unkC;
    f32 unk10;
} Objects06534Sprite;

typedef struct {
    u8 unk0;
    u8 pad01[7];
    s16 unk8;
    u8 pad0A[0x0E];
    s16 unk18;
} Objects07E40Group;

typedef struct {
    u8 pad00[4];
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
} Objects07E40Record;

typedef struct {
    u8 pad00[0x10];
    u8 unk10;
    u8 pad11[5];
    s16 unk16;
    u8 *unk18;
    u8 pad1C[4];
    u8 *unk20;
    u8 *unk24;
} Objects07E40Inner;

typedef struct {
    Objects07E40Inner *unk0;
} Objects07E40Outer;

typedef struct {
    u8 pad00[0x22];
    s8 unk22;
    u8 pad23[0x7F];
    u8 unkA2;
    u8 unkA3;
    s8 unkA4;
    s8 unkA5;
} Objects07E40Data;

typedef struct {
    u8 pad00[0x40];
    Objects07E40Data *unk40;
    u8 pad44[0x24];
    Objects07E40Outer **unk68;
} Objects07E40Object;

typedef struct {
    s16 pad00;
    s16 pad02;
    s16 pad04;
    s16 unk6;
    s16 unk8;
} Objects07E40Texture;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
} Objects08128Bounds;

typedef struct {
    u8 pad00[0x20];
    s16 unk20;
    s16 unk22;
    s16 unk24;
    s16 unk26;
    s16 unk28;
    s16 unk2A;
} Objects08128Track;

typedef struct {
    u8 pad00[0x0C];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x16];
    s16 unk2E;
} Objects08128Object;

extern Objects08128Track *trackGetTrack(void);
extern Objects08128Bounds *func_8000FEEC(s32);
extern s32 func_8000FAE0(f32, f32, f32);

typedef struct {
    u8 pad00[0xA6];
    u8 unkA6;
    u8 padA7;
    u8 *unkA8;
    void **unkAC;
} Objects09220Data;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects09220Data *unk40;
} Objects09220Object;

typedef struct {
    u8 pad00[0x26];
    s16 unk26;
    s16 unk28;
    s16 unk2A;
} Objects09220Source;

typedef struct {
    f32 pad00[2];
    f32 unk8;
    f32 pad0C[3];
    f32 unk18;
    f32 pad1C[3];
    f32 unk28;
} Objects09220Matrix;

typedef struct {
    u32 w0;
    u32 w1;
} Objects09220Gfx;

extern void *camGetRotationMtx(void);
extern void *camGetProjOrgMtx(void);
extern void mathOneFloatPY(void *, f32 *);
extern void mtxf_transform_point(void *, f32, f32, f32, f32 *, f32 *, f32 *);
extern s32 func_800246B0(f32, f32, f32, f32 *, f32 *, u8);
extern void func_80034DF0(u8, u8, u8, u8, u8, u8);
extern void func_80034E48(void);
extern void func_80023598(void **, void *, void *, void *, void *, s32);
extern void func_80023A08(void **, s32, s32, s16 *, s32, s32, s32);
extern f32 sqrtf(f32);

typedef struct {
    u8 pad00[0x1C];
    s16 unk1C;
} Objects0A830Data;

typedef struct {
    u8 pad00[0x40];
    Objects0A830Data *unk40;
    u8 pad44[0x20];
    void *unk64;
} Objects0A830Object;

extern void *D_800C94D8[];
extern s32 D_800C9470;
extern s32 D_800C9474;
extern s32 D_800C94A8;
extern s32 D_800C94AC;
extern s32 *D_800C9450;
extern s32 D_800C9454;
extern s32 D_800C945C;
extern u16 D_8007BF1C;
extern void *D_800C94A0;
extern Objects04B04Object **D_800C9488;
extern u16 *D_800C948C;
extern s32 *D_800C94A4;
extern s32 D_800C94B4;
extern s32 D_800C94B8;
extern s32 D_800C94BC;
extern s32 D_8007A210;
extern void *D_8007A214;
extern void *D_8007A218;
extern s32 D_8007A21C;
extern s16 D_800C9508[];
extern s16 D_800C94B0;
extern s16 D_800C94B2;
extern s32 D_80078F84;
extern Objects04454Object *D_80078F20;
extern s8 D_80078F88;
extern s8 D_80079004;
extern u8 D_8007BDA0;
extern u8 D_8007BEF8;
extern u8 D_8007BF0C;
extern s8 D_80079250;
extern f32 D_80080F84;
extern void **D_800C94F4;
extern s32 D_800C94F8;
extern void **D_800C9494;
extern s32 D_800C9498;
extern s32 D_800C949C;
extern s32 D_800C94FC;
extern s32 D_800C9490;
extern u8 *D_800C9460;
extern s32 D_800C9468;
extern s32 *D_800C9458;
extern s16 *D_800C94E0;
extern s32 D_800C94C0[];
extern s32 D_800C94C8[];
extern s32 D_800C94D0[];
extern s32 D_800C94D4[];
extern s32 D_800C9478;
extern f32 D_800C946C;
extern s32 D_80078F80;
extern s32 D_80078F78;
extern s32 D_800C94E8;
extern u8 D_800D3128[];
extern void **D_800C94EC;
extern s32 D_800C94F0;
extern void **D_800C9500;
extern s32 D_800C9504;
extern s32 D_8007A1F4;
extern s32 D_8007A1F8;
extern s32 D_8007A1FC;
extern u8 D_8007BEFC;
extern u8 D_8007BF04;
extern u8 D_8007BF10;
extern void *D_80078F7C;
extern s16 D_80078F8C[];
extern s16 D_80078FA0[];
extern s16 D_80078FB4[];
extern s16 D_80078FC8[];
extern s16 D_80078FDC[];
extern s16 D_80078FF0[];
extern s32 D_800C947C;
extern s32 D_800C9480;
extern s32 D_800C9484;
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
extern s32 *piRomLoad(s32 assetIndex);
extern s32 runlinkDownloadCode(s32 overlayIndex);
extern void *func_8000590C();
extern f32 sqrtf(f32 value);
extern void func_80006FA0(void);
extern void func_80007118();
extern s32 TrapDanglingJump();
extern void mmFree(void *data);
extern void modFreeModel(void *resource);
extern void func_800347A0(void *texture);
extern void func_800359D4(void *sprite);
extern s32 func_8000A6E8(s32 arg0);
extern void *func_8002B280(s32 size, s32 tag);
extern void *func_8002B4C0(void *slots, s32 size);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void func_80009F74(Objects09F74Object *object);
extern u8 *levelGetLevel(void);
extern s32 levelGetNumber(void);
extern s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3);
extern void func_80058250(void);
extern s32 camGetNo(void);
extern Objects09F74Camera *camGetPtr(void);
extern f32 camGetProjZ(f32 x, f32 y, f32 z);
extern u8 *levelGetColourCycling(void);
extern void func_80008B94(void *object);
typedef struct Objects09AA8Object Objects09AA8Object;
extern void func_80009AA8(Objects09AA8Object *object);
extern s32 func_800290A0(void);
extern void func_800367E8(Objects07C68Texture *texture, void *flags, s32 *frame,
                           s32 updateRate);
extern s32 D_80079008[];
extern s32 D_800790D0[];
extern f32 D_80080D24;
extern f32 D_80080D28;
extern void func_8000831C(void *arg0, void *arg1, s32 arg2, void *arg3, s32 arg4,
                          s32 arg5, s32 arg6, s32 arg7, f32 arg8, s32 arg9, s32 arg10);
typedef struct CameraScaledTransform CameraScaledTransform;
typedef struct FxGfx FxGfx;
extern void camPushModelMtx(Gfx **dlist, Mtx **mtx, CameraScaledTransform *transform,
                            f32 scale, f32 scaleY);
extern void camPopModelMtx(Gfx **dlist);
extern void camRestoreModelMtx(Gfx **dlist);
extern void func_80034920(Gfx **dlist);
extern void func_800349A4(FxGfx **dlist, s32 texture, s32 flags, s32 arg3);
extern s32 func_800291FC(void);
extern s32 func_80034448();
extern void func_8005AF14(void *arg0, void *arg1, void *arg2);
extern void func_80019AB8(void *arg0, void *arg1, s32 arg2, s32 arg3);
extern void func_80007C68(Objects07C68Object *arg0, Objects07C68Source *arg1,
                          Objects07C68Object *arg2, s32 arg3);
extern void **func_8000572C(s32 *start, s32 *end);
extern f32 func_8000BD0C(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5);
extern void partInitTrigger(ParticleTrigger *trigger, s32 type, s32 value);
extern void partInitTriggerSPPos(ParticleTrigger *trigger, s32 type, s32 value, s32 index);
extern void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
extern u8 *func_80028F54(void);

void func_80004340(void) {
    D_800C9470 = 0;
    D_800C9474 = 0;
    D_800C9498 = 0;
    D_800C949C = 0;
    D_800C94A8 = 0;
    D_800C94AC = 0;
    D_800C94F0 = 0;
    D_800C94F8 = 0;
    D_800C9504 = 0;
    D_800C94B0 = 0;
    D_800C94B2 = 0;
}
void func_8000439C(void) {
    s32 count;
    s32 i;
    s32 offset;

    D_800C94FC = 0;
    func_80006FA0();
    count = D_800C9498;
    i = 0;
    offset = 0;
    if (count > 0) {
        do {
            func_80007118(*(void **)((u8 *)D_800C9494 + offset), 1);
            i += 1;
            offset += 4;
        } while (i != count);
    }
    D_800C94F0 = 0;
    D_800C9498 = 0;
    D_800C949C = 0;
    func_80004340();
    mmFree(D_800C94D8[0]);
    mmFree(D_800C94D8[1]);
    D_80078F84 = 0;
}
/* The list base is advanced onto the start element and the cursor taken from
 * it, rather than the two being separate sums. The copy coalesces away, so it
 * emits nothing, but it leaves uopt a zero-cost web that interferes with the
 * initial list index and takes a0 -- which is what puts that index in a1. */
void *func_80004454(f32 arg0, f32 arg1, f32 arg2, u8 arg3) {
    f32 queryX;
    f32 queryY;
    s32 sp7C;
    s32 sp78;
    f32 temp_f0;
    f32 var_f20;
    s32 temp_v0;
    s32 var_s1;
    s32 var_v0;
    u8 *var_s2;
    Objects04454Object *temp_s0;
    void *var_s4;

    queryX = arg0;
    queryY = arg1;
    temp_v0 = (s32)func_8000572C(&sp7C, &sp78);
    var_f20 = D_80080D24;
    var_s4 = NULL;
    if (sp7C < sp78) {
        var_s1 = sp7C * 4; temp_v0 += var_s1; var_s2 = (u8 *)temp_v0; do {
            temp_s0 = *(Objects04454Object **)var_s2;
            if (temp_s0->unk91 != 0) {
                var_v0 = sp78 * 4;
            } else {
                if ((temp_s0 != D_80078F20) && (temp_s0->unk44 == arg3)) {
                    temp_f0 = func_8000BD0C(temp_s0->unkC, temp_s0->unk10,
                                            temp_s0->unk14, queryX, queryY, arg2);
                    if (temp_f0 < var_f20) {
                        var_f20 = temp_f0;
                        var_s4 = temp_s0;
                    }
                }
                var_v0 = sp78 * 4;
            }
            var_s1 += 4;
            var_s2 += 4;
        } while (var_s1 < var_v0);
    }
    return var_s4;
}
/* Keep the preheader's three assignments on one physical line with `do {`:
 * splitting them costs two words at +0x74/+0x78. See func_8000471C, which is
 * the same function against a different object list and needs the same edit. */
/* Lane lm-obj: plain `for (i = start; i < end; i++)` over list[i]. The
 * hand-unrolled remainder-plus-4x body was +700 / 272 masked. Lane w2-obj:
 * the outer `if (start < end)` duplicated the for-loop test and cost two
 * words; removing it is +16 / 84 from +24 / 100. Remaining extras are the
 * type mask rematerialized at each compare because s0 holds unmodified arg0.
 * The 04454 preheader walk under-unrolls to -236. */
#ifdef NON_MATCHING
s32 func_80004590(s32 arg0) {
    s32 start;
    s32 end;
    s32 count;
    s32 i;
    s32 type;
    Objects04454Object **list;
    Objects04454Object *object;

    type = arg0 & 0xFF;
    count = 0;
    list = (Objects04454Object **)func_8000572C(&start, &end);
    for (i = start; i < end; i++) {
        object = list[i];
        if ((object->unk91 == 0) && (object != D_80078F20) &&
            (type == object->unk44)) {
            count += 1;
        }
    }
    return count;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004590.s")
#endif
/* Twin of func_80004454 against a different object list: the same advance of
 * the list base onto the start element, whose coalesced copy leaves the
 * zero-cost web that takes a0 and moves the initial list index to a1.
 * The preheader must stay on one physical line with `do {`. */
void *func_8000471C(f32 arg0, f32 arg1, f32 arg2, u8 arg3) {
    f32 queryX;
    f32 queryY;
    s32 sp7C;
    s32 sp78;
    f32 temp_f0;
    f32 var_f20;
    s32 temp_v0;
    s32 var_s1;
    s32 var_v0;
    u8 *var_s2;
    Objects0471CObject *temp_s0;
    void *var_s4;

    queryX = arg0;
    queryY = arg1;
    temp_v0 = (s32)func_8000572C(&sp7C, &sp78);
    var_f20 = D_80080D28;
    var_s4 = NULL;
    if (sp7C < sp78) {
        var_s1 = sp7C * 4; temp_v0 += var_s1; var_s2 = (u8 *)temp_v0; do {
            temp_s0 = *(Objects0471CObject **)var_s2;
            if (temp_s0->unk91 != 0) {
                var_v0 = sp78 * 4;
            } else {
                if ((temp_s0 != (Objects0471CObject *)D_80078F20) && (temp_s0->unk40->unk1B == arg3)) {
                    temp_f0 = func_8000BD0C(temp_s0->unkC, temp_s0->unk10, temp_s0->unk14, queryX, queryY, arg2);
                    if (temp_f0 < var_f20) {
                        var_f20 = temp_f0;
                        var_s4 = temp_s0;
                    }
                }
                var_v0 = sp78 * 4;
            }
            var_s1 += 4;
            var_s2 += 4;
        } while (var_s1 < var_v0);
    }
    return var_s4;
}
/* (Four lines held here so the physical line numbers of every function below
 * are unchanged by this promotion; IDO's schedule is sensitive to them, and a
 * shorter replacement moved an already-matched neighbour in another TU.) */
void func_8000485C(s8 arg0) {
    D_80078F88 = arg0;
}
void *func_8000486C(s32 arg0) {
    Objects0486CAsset *asset;
    s32 assetStart;
    s32 assetSize;
    s32 index;

    if (*(u16 *)((u8 *)D_800C948C + arg0 * 2) != 0) {
        *(u16 *)((u8 *)D_800C948C + arg0 * 2) += 1;
        return ((Objects0486CAsset **)D_800C9488)[arg0];
    }

    assetStart = ((s32 *)((u8 *)D_800C9458 + arg0 * 4))[0];
    assetSize = ((s32 *)((u8 *)D_800C9458 + arg0 * 4))[1] - assetStart;
    asset = (Objects0486CAsset *)func_8002B4C0(D_800C94A0, assetSize);
    if ((asset == NULL) && (D_80078F88 != 0)) {
        asset = (Objects0486CAsset *)func_8002B280(assetSize, 0x8B);
    }
    if (asset != NULL) {
        piRomLoadSection(0x2D, (u32)asset, assetStart, assetSize);
        asset->unk4C = (s32)((u8 *)asset + asset->unk4C);
        asset->unk50 = (s32)((u8 *)asset + asset->unk50);
        asset->unk44 = (s32)((u8 *)asset + asset->unk44);
        asset->unkB0 = (s32)((u8 *)asset + asset->unkB0);
        asset->unk38 = (s32)((u8 *)asset + asset->unk38);
        asset->unk3C = (s32)((u8 *)asset + asset->unk3C);
        asset->unk40 = (s32)((u8 *)asset + asset->unk40);
        asset->unk34 = (s32)((u8 *)asset + asset->unk34);
        asset->unkA8 = (s32)((u8 *)asset + asset->unkA8);
        asset->unkAC = (s32)((u8 *)asset + asset->unkAC);
        if (asset->unkE0 != 0) {
            asset->unkE0 = (s32)((u8 *)asset + asset->unkE0);
        }

        index = 0;
        if (asset->unkA6 > 0) {
            do {
                if ((((u16 *)asset->unkA8)[index] & 0xC000) == 0xC000) {
                    ((void **)asset->unkAC)[index] =
                        (void *)func_80034448(((u16 *)asset->unkA8)[index] & 0x3FFF);
                } else if (((u16 *)asset->unkA8)[index] & 0x8000) {
                    ((void **)asset->unkAC)[index] =
                        func_800355A0(((u16 *)asset->unkA8)[index] & 0x3FFF, 1);
                } else {
                    ((void **)asset->unkAC)[index] =
                        func_8001F520(((u16 *)asset->unkA8)[index], 0);
                }
                index += 1;
            } while (index < asset->unkA6);
        }

        if ((asset->unk1C == 1) && (D_8007BF1C & 8) &&
            (*func_80028F54() != 1)) {
            asset->unk0 *= 0.5f;
        }
        *(Objects0486CAsset **)((u8 *)D_800C9488 + arg0 * 4) = asset;
        *(u16 *)((u8 *)D_800C948C + arg0 * 2) = 1;
    } else {
        return NULL;
    }
    return asset;
}
/* Workbench verdict: register-permutation; 14 differing words (59/73). */
/* First mismatch: +0x4; size, frame, CFG, and relocation surface are exact. */
/* Structural gap: none; global-color pool allocation is reserved for the permuter. */
void func_80004B04(s32 arg0)
{
  s32 temp_v0;
  s32 var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 *temp_v1;
  u16 temp_a1;
  u16 temp_v0_2;
  Objects04B04Object *temp_s0;
  temp_v0 = arg0 * 2;
  temp_v1 = (u16 *) (((u8 *) D_800C948C) + temp_v0);
  temp_a1 = *temp_v1;
  if (temp_a1)
  {
    *temp_v1 = temp_a1 - 1;
    if ((*((u16 *) (((u8 *) D_800C948C) + temp_v0))) == 0)
    {
      var_s2 = 0;
      temp_s0 = D_800C9488[arg0];
      var_s3 = 0;
      var_s1 = 0;
      if (temp_s0->unkA6 > 0)
      {
        do
        {
          ;
          if (((*((u16 *) (temp_s0->unkA8 + var_s3))) & 0xC000) == 0xC000)
          {
            func_800347A0(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          else
            if ((*((u16 *) (temp_s0->unkA8 + var_s3))) & 0x8000)
          {
            func_800359D4(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          else
          {
            modFreeModel(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          var_s2 += 1;
          var_s3 += 2;
          var_s1 += 4;
        }
        while (var_s2 < ((s32) temp_s0->unkA6));
      }
      mmFree(temp_s0);
    }
  }
}
/* Loads one object-table section from ROM into a fresh 0x3000-byte heap and
 * rewrites its records in place for the current game options, leaving the
 * cursor back at the first record.
 *
 * Two spellings carry the whole match. The allocation goes into the global
 * slot and `heap` is a copy taken *after* the derived cursor is stored, which
 * is what puts the local's home store after both global stores rather than
 * before them; defining `heap` first and assigning the global from it emits
 * the home store one or two instructions early, and no ordering or line
 * grouping of those five statements recovers it. And the record length is the
 * field read spelled twice rather than a carrier: sharing `tableCount` with
 * the switch's keep/skip flag merges the two webs onto one colour, a dedicated
 * length local separates them but costs the frame (0x50 against 0x48, since
 * this function's seven declared scalars and five compiler temporaries already
 * fill it exactly), and reading the field again at both uses separates them
 * for free. The accumulate must precede the cursor store for that to hold.
 *
 * PROVENANCE: ROM-table scan and direct array-carrier spelling informed by
 * Diddy Kong Racing public src/objects.c track_spawn_objects. Mickey ROM
 * controls its boundaries, globals, record format and spawn conditions. */
void func_80004C28(s32 arg0, s32 arg1) {
    s32 size;
    s32 offset;
    s32 tableCount;
    s32 *heap;
    s16 *current;
    s32 *romTable;
    s32 start;

    D_800C94D8[arg1] = func_8002B280(0x3000, 0x8B);
    D_800C94C0[arg1] = (s32)((u8 *)D_800C94D8[arg1] + 0x10);
    heap = D_800C94D8[arg1];
    D_800C94C8[arg1] = 0;
    D_800C94D0[arg1] = arg0;

    romTable = piRomLoad(0x1C);
    for (tableCount = 0; romTable[tableCount] != -1; tableCount++) {
    }
    tableCount -= 1;
    if (arg0 >= tableCount) {
        arg0 = 0;
    }
    start = romTable[arg0];
    size = romTable[arg0 + 1] - start;
    mmFree(romTable);
    if (size == 0) {
        return;
    }

    piRomLoadSection(0x1D, (u32)heap, start, size);
    D_800C94C0[arg1] = (s32)((u8 *)D_800C94D8[arg1] + 0x10);
    D_800C94C8[arg1] = *heap;
    D_800C94E8 = arg1;
    romTable = &D_800C94D0[arg1];
    if (romTable == &D_800C94D4[0]) {
        D_80078F78 = 1;
        runlinkDownloadCode(8);
        runlinkDownloadCode(1);
        TrapDanglingJump();
        D_80078F78 = 0;
    }

    offset = 0;
    if (D_800C94C8[arg1] > 0) {
        do {
            tableCount = 1;
            if (D_8007BF0C != 0) {
                current = (s16 *)D_800C94C0[arg1];
                switch (*current) {
                case 0x1B:
                    *current = 0xEA;
                    break;
                case 0x86:
                    *current = 0xEB;
                    break;
                case 0x02:
                case 0x16:
                case 0x17:
                case 0x76:
                case 0x78:
                case 0x7D:
                case 0x104:
                case 0x105:
                case 0x107:
                case 0x12C:
                    if ((s32)D_8007BEF8 >= 3) {
                        tableCount = 0;
                    }
                    break;
                case 0x106:
                case 0x131:
                case 0x132:
                case 0x147:
                    tableCount = 0;
                    break;
                }
            }
            current = (s16 *)D_800C94C0[arg1];
            if ((*current == 0x3F) && ((D_8007BF1C & 0x20) != 0)) {
                tableCount = 0;
            }
            if ((*current == 1) || (*current == 0x155) || (*current == 0x156)) {
                if (D_800D3128[0x12] >= 0x15) {
                    *current = 0x156;
                    current = (s16 *)D_800C94C0[arg1];
                } else if ((*(u16 *)(D_800D3128 + 0x14) & 0x40) != 0) {
                    *current = 0x155;
                    current = (s16 *)D_800C94C0[arg1];
                } else {
                    *current = 1;
                    current = (s16 *)D_800C94C0[arg1];
                }
            }
            if (tableCount != 0) {
                func_8000590C(current, 1);
                current = (s16 *)D_800C94C0[arg1];
            }
            offset += *((u8 *)current + 2);
            D_800C94C0[arg1] = (s32)((u8 *)current + *((u8 *)current + 2));
        } while (offset < D_800C94C8[arg1]);
    }
    D_800C94C0[arg1] = (s32)((u8 *)D_800C94D8[arg1] + 0x10);
    if (D_800C9478 == 0) {
        func_80006FA0();
    }
    D_800C9478 = 1;
}
typedef struct {
    s16 unk0;
    u8 unk2;
    u8 pad03;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11;
} Objects04FE0Packet;

typedef struct {
    u8 unk0;
    u8 pad01[3];
    u8 unk4;
    s8 unk5;
    s8 unk6;
    u8 pad07[0x21];
} Objects04FE0ModeRecord;

typedef struct {
    u8 pad00[4];
    s16 unk4;
    s16 unk6;
    s16 unk8;
} Objects04FE0Source;

typedef struct {
    s16 unk0;
    u8 pad02[0x3A];
    void *unk3C;
    u8 pad40[4];
    s16 unk44;
    u8 pad46[0x3E];
    s32 unk84;
    s32 unk88;
} Objects04FE0Object;

typedef struct {
    s16 unk0;
    u8 unk2;
    u8 pad03;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    u8 unkA;
    u8 pad0B;
} Objects04FE0ExtraPacket;

typedef struct {
    s16 unk0;
    u8 unk2;
    u8 pad03;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
} Objects04FE0SpecialPacket;

#ifdef NON_MATCHING
void func_80004FE0(s32 arg0) {
    s32 i;
    s32 offset;
    s32 playerCount;
    s32 slot;
    s32 type;
    Objects04FE0Packet packets[6];
    Objects04FE0Object *category[6];
    Objects04FE0SpecialPacket specialPacket;
    Objects04FE0ExtraPacket extraPacket;
    Objects04FE0ModeRecord *modeState;
    Objects04FE0ModeRecord *records;
    Objects04FE0Object *object;
    Objects04FE0Source *source;
    s8 *level;

    modeState = func_80028F54();
    level = (s8 *)levelGetLevel();
    playerCount = func_800291FC();
    D_800C94F8 = 0;
    if ((level[0x83] != 1) && (level[0x83] != 2) && (playerCount > 0)) {
        for (i = 0; i < 6; i++) {
            category[i] = NULL;
        }
        for (offset = 0; offset < D_800C9498; offset++) {
            object = (Objects04FE0Object *)D_800C9494[offset];
            if ((object->unk44 == 5) && (arg0 == object->unk88)) {
                slot = object->unk84;
                if ((slot >= 0) && (slot < 6)) {
                    if (category[slot] == NULL) {
                        category[slot] = object;
                    }
                } else {
                    for (type = 0; type < 6; type++) {
                        category[type] = object;
                    }
                }
            }
        }
        for (i = 0, records = modeState; i < playerCount; i++, records++) {
            type = records->unk4;
            if (type >= 0xA) {
                type = 0;
            }
            if (D_8007BF10 != 0) {
                packets[i].unk0 = D_80078FF0[type];
            } else if (D_8007BF0C == 0) {
                if (level[0x83] == 3) {
                    packets[i].unk0 = D_80078FB4[type];
                } else if (D_8007BF04 != 0) {
                    packets[i].unk0 = D_80078FA0[type];
                } else {
                    packets[i].unk0 = D_80078F8C[type];
                }
            } else if (level[0x83] == 3) {
                packets[i].unk0 = D_80078FDC[type];
            } else {
                packets[i].unk0 = D_80078FC8[type];
            }
            packets[i].unk2 = 0x12;
            packets[i].unkA = 0;
            packets[i].unkC = 0;
            packets[i].unk10 = i;
            packets[i].unk11 = type;
            if ((modeState->unk0 == 5) || (modeState->unk0 == 6)) {
                slot = i;
            } else if ((modeState->unk0 == 1) || (modeState->unk0 == 2)) {
                slot = 0;
            } else if (D_8007BF0C != 0) {
                slot = (playerCount - records->unk6) - 1;
            } else {
                slot = records->unk6;
            }
            object = category[slot];
            if (object != NULL) {
                source = (Objects04FE0Source *)object->unk3C;
                packets[i].unk4 = source->unk4;
                packets[i].unk6 = source->unk6;
                packets[i].unk8 = source->unk8;
                packets[i].unkE = object->unk0;
                category[slot] = NULL;
            } else {
                packets[i].unk4 = 0;
                packets[i].unk6 = 0;
                packets[i].unk8 = 0;
                packets[i].unkE = 0;
            }
        }
        controlGetPlayerSetup(&packets[0].unk4, &packets[0].unk6,
                              &packets[0].unk8, &packets[0].unkE);
        for (offset = 0; offset < 8; offset++) {
            D_800C94F4[offset] = NULL;
        }
        for (i = 0, offset = 0; i < playerCount; i++, offset += 4) {
            object = (Objects04FE0Object *)func_8000590C(&packets[i], 1);
            *(void **)((u8 *)D_800C94F4 + offset) = object;
            if (object != NULL) {
                object->unk3C = NULL;
            }
        }
        D_800C94F8 = playerCount;
        for (i = 0; i < playerCount - D_8007BEFC; i++) {
            extraPacket.unk0 = 0x71;
            extraPacket.unk4 = 0;
            extraPacket.unk6 = 0;
            extraPacket.unk8 = 0;
            extraPacket.unkA = i;
            object = (Objects04FE0Object *)func_8000590C(&extraPacket, 1);
            if (object != NULL) {
                object->unk3C = NULL;
            }
        }
        if (D_8007BF0C == 0) {
            specialPacket.unk0 = 0x70;
        } else {
            specialPacket.unk0 = 0x77;
        }
        specialPacket.unk2 = 0xC;
        specialPacket.unk4 = packets[0].unk4;
        specialPacket.unk6 = packets[0].unk6;
        specialPacket.unk8 = packets[0].unk8;
        specialPacket.unkA = packets[0].unkE;
        object = (Objects04FE0Object *)func_8000590C(&specialPacket, 1);
        if (object != NULL) {
            object->unk3C = NULL;
            if (D_8007BF0C != 0) {
                D_80078F7C = object;
            }
        }
        if ((modeState->unk0 == 1) && (D_800C94F4[0] != NULL)) {
            if ((*(s32 *)D_800D3128 & 0x80000) != 0) {
                TrapDanglingJump(D_8007A1F4);
            }
            if ((*(s32 *)D_800D3128 & 0x100000) != 0) {
                TrapDanglingJump(levelGetNumber());
                TrapDanglingJump(D_8007A1F8);
            }
            D_8007A1FC = TrapDanglingJump(levelGetNumber());
            TrapDanglingJump(); TrapDanglingJump(D_800C94F4[0]);
        }
        D_800C947C = 1;
        D_800C9480 = 0x80;
        D_800C9484 = 0;
        D_80078F80 = 0;
    }
    runlinkDownloadCode(8);
    runlinkDownloadCode(1);
    TrapDanglingJump((s8)level[0x83]);
    func_80058250();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004FE0.s")
#endif
/* Lane lm-obj: the ROM's two rank-copy loops are IDO's unroller output of a
 * plain `for (i = 0; i < arg0; i++)` over each mode record. The hand-unrolled
 * remainder-plus-4x body grew 632 bytes under the unroller. unk5 is signed:
 * a u8 load left 10 structural words at delta 0. 87/87 words, frame 0x18,
 * 3 relocations. */
void func_80005548(s32 arg0) {
    s32 i;
    Objects04FE0ModeRecord *records;

    records = (Objects04FE0ModeRecord *)func_80028F54();
    if (D_8007BF0C != 0) {
        for (i = 0; i < arg0; i++) {
            records[i].unk6 = records[i].unk5;
        }
    } else {
        for (i = 0; i < arg0; i++) {
            records[i].unk6 = (arg0 - records[i].unk5) - 1;
        }
    }
}
/* D_800C9460 heads an eight-byte record: the object base pointer, then the
   pointer to the index table.  The second field's address reaches the compiler
   as &D_800C9460[1], which is why this reads through a pointer instead of a
   second named global -- splat could not pair that %hi/%lo across the
   early-return branch and spells it as a literal in the fallback, so the
   fallback carries two fewer relocations for identical linked words. */
void *func_800056A4(s32 tableIndex) {
    s32 **table = (s32 **)&D_800C9460;

    if ((tableIndex < 0) || (tableIndex >= D_800C9468)) {
        return D_800C9460;
    }
    return D_800C9460 + (table[1][tableIndex] * 4);
}
void *func_800056F0(s32 index) {
    if ((index < 0) || (index >= D_800C9498)) {
        return 0;
    }
    return D_800C9494[index];
}
void **func_8000572C(s32 *start, s32 *end) {
    *start = D_800C949C;
    *end = D_800C9498;
    return D_800C9494;
}
void **func_80005750(s32 *count) {
    *count = D_800C94F8;
    return D_800C94F4;
}
void func_80005768(AnimPathObject *object) {
    D_800C9500[D_800C9504] = object;
    D_800C9504 += 1;
}
void func_80005798(void *object) {
    s32 i;

    if (D_800C9504 > 0) {
        for (i = 0; i < D_800C9504; i++) {
            if (D_800C9500[i] == object) {
                D_800C9500[i] = D_800C9500[D_800C9504 - 1];
            }
        }
        D_800C9504 -= 1;
    }
}
void **func_80005808(s32 *count) {
    *count = D_800C9504;
    return D_800C9500;
}
s32 func_80005820(s32 arg0) {
    if (D_800C94F8 == 0) {
        return 0;
    }
    if ((arg0 < 0) || (arg0 >= D_800C94F8)) {
        return 0;
    }
    return D_800C94F4[arg0];
}
s16 func_80005868(s32 arg0) {
    s32 assetOffset;
    s16 *entry;
    u8 buffer[0xC0];

    assetOffset = D_800C9458[D_800C94E0[arg0]];
    piRomLoadSection(0x2D, (u32)buffer, assetOffset, 0xC0);
    entry = (s16 *)(buffer + 0x1C);
    return *entry;
}
s8 func_800058C0(Objects58C0Arg *arg0, s32 arg1) {
    if ((arg1 >= 4) || (arg0->unk40->unkD0[arg1] == 0.0f)) {
        return arg0->unk40->unk1E[0];
    }
    return arg0->unk40->unk1E[arg1];
}
typedef struct {
    s16 unk0;
    u8 pad02[2];
    s16 unk4;
    s16 unk6;
    s16 unk8;
} Objects0590CPacket;

typedef struct {
    f32 unk0;
    u8 pad04[0x10];
    u16 unk14;
    u8 pad16[2];
    s16 unk18;
    s8 unk1A;
    u8 pad1B[1];
    s16 unk1C;
    u8 pad1E[4];
    s8 unk22;
    s8 unk23;
    s8 unk24;
    s8 unk25;
    u8 pad26[2];
    s8 unk28;
    u8 unk29;
    u8 pad2A[6];
    u8 unk30;
    u8 pad31[3];
    s32 *unk34;
    u8 pad38[0x29];
    u8 unk61;
    u8 pad62[0x10];
    s8 unk72;
    u8 pad73[0x34];
    u8 unkA7;
    u8 padA8[0xC];
    s16 unkB4;
    s16 unkB6;
    s16 unkB8;
    u8 unkBA;
    u8 unkBB;
    f32 unkBC;
    f32 unkC0;
    f32 unkC4;
    f32 unkC8;
    s32 unkCC;
    u8 padD0[0x10];
    s32 unkE0;
} Objects0590CAsset;

typedef struct {
    u8 pad00[0x30];
    s32 unk30;
} Objects0590CParticleList;

typedef struct Objects0AA38Object Objects0AA38Object;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x14];
    s16 unk2C;
    s16 unk2E;
    u8 pad30[4];
    f32 unk34;
    u8 pad38[1];
    u8 unk39;
    u8 pad3A[2];
    void *unk3C;
    Objects0590CAsset *unk40;
    u8 pad44[2];
    s16 unk46;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s32 unk60;
    s32 unk64;
    s32 *unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 unk78;
    s32 unk7C;
    u8 pad80[0xD];
    u8 unk8D;
    u8 pad8E[3];
    u8 unk91;
    u8 unk92;
    u8 unk93;
    s32 unk94[1];
} Objects0590CObject;

extern u8 *align4(u8 *address);
extern void lightSetupLightSources(void *object);
extern void lightSetupFlareSources(void *object);
extern s32 func_8001A008(void *object, void *state);
extern void modelSetModelFlags(s32 flags);
extern s32 func_80048760(void *object, s32 state);
extern void func_80053550(void *source, s32 kind, s32 mode, s16 rotationX,
                           s16 rotationY, s16 rotationZ, f32 radius, f32 height,
                           f32 arg8, f32 arg9, s32 collisionType, u16 flags);
extern void func_8000AA38(Objects0AA38Object *object, void *arg1, s32 preserveState);
extern void func_80006448(void *object);
extern s32 func_80006534(Objects06534Object *object);
extern s32 func_80006868(Objects06868Object *object, void *data);
extern s32 func_800069C0(Objects69C0In *object, Objects69C0Out *data);
extern s32 func_800069E8(Objects069E8Object *object, Objects069E8Target *data);
extern s32 func_80006B04(Objects06B04Object *object, Objects06B04Output *data,
                          volatile s32 arg2);
extern s32 func_80006C40(Objects06C40 *object, s32 data);
extern s32 func_8000A830(Objects0A830Object *object, void *data);

/* PROVENANCE: screened Jet Force Gemini, src/objects.c, at public revision
 * efd5abb1c79636e297b831f7c2d5bf47eac39c0c. Its objSetupObject counterpart
 * remains assembly-backed. No donor C was adopted; the existing Mickey
 * candidate and the unresolved header-scheduling mechanism are unchanged.
 */
/* Lane s1-c (2026-09-16): 99 -> 26 masked at delta 0, frame 0x90, no force.
 * The nested fixups declare no pointer: `object->unk4C` is stored and
 * re-read with one spelling, so uopt forwards the stored value into a temp
 * that ugen keeps in a ring register across the branch (the target's t8/t4).
 * The `(u32)` on each nested STORE address is load-bearing: the forwarded
 * re-read is a phantom web (coloured a0, never emitted) that outranks the
 * nested value on web number; a second spelling for the store splits it.
 *
 * Lane s2-a (2026-09-16): 26 -> 2 masked at delta 0, frame 0x90, no force.
 * Three mechanisms, all read off the allocator records:
 *  - The tail's three-way tie (`arg1 & 1`, &D_800C9498, &D_800C94A8 at 3/7)
 *    is broken by an empty overflow check after the special-list append,
 *    `if (D_800C94A8 > 0x100) { }` -- the shape DKR's spawnObject carries
 *    after its own append (PROVENANCE: idiom only, no code adopted). It emits
 *    nothing and reorders the constants' webs; the failure-path
 *    &D_800C9498 def then lands at the join, as the ROM has it (26 -> 5).
 *  - The copy loop is the plain `while (resultSize < size)` loop and the
 *    TU no longer carries -Wo,-loopunroll,0 (see the Makefile): IDO's
 *    unroller emits the ROM's remainder loop, its bypass of the
 *    `resultSize != size` test on the no-remainder path, the 4x body and the
 *    strength-reduced `sll a0,zero,2` preheader init. The hand-unrolled
 *    loops could never reach that CFG, and with the plain loop `size` has
 *    its loop weight naturally, so the six `size = size | 0` probes are gone.
 *  - The zeroing loop is subscripted for the same reason (its `offset`
 *    carrier, shared with the copy loops, was what ranked `selected` and the
 *    constant 1 at the head), and the unk48 nested value is the expression
 *    itself, not `offset`: a declared symbol is numbered with the type-3
 *    webs and wins the 1.5 tie against the unk48 value web on number where
 *    the ROM decides the value first (v1) and the nested read second (a0).
 * Lane lm-0590c: both nested adds commute temp-first while the operand is
 * an ILOD off the forwarded relocated pointer (`uadd(cvt(ILOD(temp)),
 * isvar)`). Binding that operand to an isvar makes uopt emit object-first,
 * matching the plain fixups. A new local takes a home and moves the 0x90
 * frame; `offset` and the already-dead `aligned` reuse the existing 20
 * slots. Using `offset` at both sites reconnects as one type-3 web and
 * re-steals the 1.5 tie (14). */
void *func_8000590C(void *arg0, s32 arg1) {
    Objects0590CObject *object;
    Objects0590CObject *newObject;
    u8 *cursor;
    s32 loadFlags;
    s32 loadType;
    s16 selected;
    s32 packetId;
    s32 index;
    s32 offset;
    s32 size;
    u8 *aligned;
    s8 failed;
    s32 resultSize;
    Objects0590CAsset *asset;

    D_8007A210 = 1;
    D_8007A214 = NULL;
    D_8007A218 = NULL;
    D_8007A21C = 1;

    packetId = ((Objects0590CPacket *)arg0)->unk0;
    if (arg1 & 2) {
        selected = packetId;
    } else {
        selected = D_800C94E0[packetId];
    }
    if ((selected < 0) || (selected >= D_800C945C)) {
        D_8007A21C = 3;
        return NULL;
    }

    resultSize = 0;
    if ((D_800C9454 >> 2) > 0) {
        do {
            ((s32 *)D_800C9450)[resultSize] = 0;
            resultSize += 1;
        } while (resultSize < (D_800C9454 >> 2));
    }
    object = (Objects0590CObject *)D_800C9450;
    D_8007A214 = object;
    object->unk6 = 2;
    object->unk40 = (Objects0590CAsset *)func_8000486C(selected);
    if (object->unk40 == NULL) {
        D_8007A21C = 2;
        return NULL;
    }
    D_8007A218 = (u8 *)object->unk40 + 4;
    object->unk6 = (s16)(object->unk6 | (object->unk40->unk14 & 0x124C));
    object->unkC = (f32)((Objects0590CPacket *)arg0)->unk4;
    object->unk10 = (f32)((Objects0590CPacket *)arg0)->unk6;
    object->unk14 = (f32)((Objects0590CPacket *)arg0)->unk8;
    object->unk2E = func_8000FAE0(object->unkC, object->unk10, object->unk14);
    object->unk2C = selected;
    object->unk3C = arg0;
    object->unk46 = packetId;
    object->unk8 = object->unk40->unk0;
    object->unk34 = (f32)object->unk40->unk18 * object->unk8;
    object->unk39 = 0xFF;
    object->unk8D = object->unk40->unkA7;
    object->unk91 = 0;
    object->unk93 = 0;
    loadFlags = func_8000A6E8(object->unk40->unk1C);
    loadType = loadFlags & 3;
    object->unk40->unk1A += 1;
    object->unk68 = (s32 *)((u8 *)object + 0x94);
    failed = 0;

    for (index = 0; index < object->unk40->unk22; index++) {
        switch (func_800058C0((Objects58C0Arg *)object, index)) {
        case 2:
            object->unk68[index] = (s32)func_80034448(object->unk40->unk34[index]);
            if (object->unk68[index] == 0) {
                failed = 1;
            }
            break;
        case 0:
            if (object->unk40->unk30 != 0) {
                modelSetModelFlags(0);
            }
            object->unk68[index] = (s32)func_8001F520(object->unk40->unk34[index], loadType);
            if (object->unk68[index] == 0) {
                failed = 1;
            }
            modelSetModelFlags(8);
            break;
        case 1:
            object->unk68[index] = (s32)func_800355A0(object->unk40->unk34[index], 0xA);
            if (object->unk68[index] == 0) {
                failed = 1;
            }
            break;
        default:
            object->unk68 = NULL;
            break;
        }
    }
    if (failed != 0) {
        func_80006448(object);
        func_80004B04(selected);
        D_8007A21C = 2;
        return NULL;
    }

    cursor = (u8 *)object->unk68 + (object->unk40->unk22 * 4);
    cursor += func_8000A830((Objects0A830Object *)object, cursor);
    D_800C9490 = 0;
    if (loadFlags & 0x100) {
        cursor += func_8001A008(object, cursor);
        *(u8 *)((u8 *)object->unk50 + 0xC) = *(u8 *)((u8 *)object->unk50 + 0x25);
        *(u8 *)((u8 *)object->unk50 + 0xD) = *(u8 *)((u8 *)object->unk50 + 0x27);
    }
    resultSize = loadFlags & 0x200;
    if ((resultSize != 0) && (object->unk40->unk61 != 0)) {
        resultSize = func_800069E8((Objects069E8Object *)object,
                                   (Objects069E8Target *)cursor);
        cursor += resultSize;
        if (resultSize == 0) {
            func_80006448(object);
            func_80004B04(selected);
            D_8007A21C = 2;
            return NULL;
        }
    }
    if ((loadFlags & 0x400) && (object->unk40->unk72 != -1)) {
        resultSize = func_80048760(object, (s32)cursor);
        cursor += resultSize;
        if (resultSize == 0) {
            if (D_800C9490 != 0) {
                func_800347A0(D_800C9490);
            }
            func_80006448(object);
            func_80004B04(selected);
            D_8007A21C = 2;
            return NULL;
        }
    }
    if (loadFlags & 0x800) {
        cursor += func_80006B04((Objects06B04Object *)object,
                                (Objects06B04Output *)cursor,
                                ((Objects0590CPacket *)arg0)->unk0);
    }
    if (loadFlags & 0x1000) {
        cursor += func_80006C40((Objects06C40 *)object, (s32)cursor);
    }
    if ((object->unk40->unk23 > 0) && (object->unk40->unk23 < 0xA)) {
        object->unk5C = (s32)cursor;
        aligned = align4(cursor + 0x34);
        ((Objects0590CParticleList *)object->unk5C)->unk30 = (s32)aligned;
        cursor = aligned;
        cursor += object->unk40->unk23 * 0x40;
    }
    if ((object->unk40->unk24 > 0) && (object->unk40->unk24 < 0xA)) {
        aligned = align4(cursor);
        object->unk60 = (s32)aligned;
        cursor = aligned;
        cursor += object->unk40->unk24 * 0x14;
    }
    if (object->unk40->unk25 > 0) {
        cursor += func_80006868((Objects06868Object *)object, cursor);
    }
    if (object->unk40->unk28 > 0) {
        object->unk70 = (s32)cursor;
        cursor += object->unk40->unk28 * 4;
    }
    if (object->unk40->unk29 > 0) {
        object->unk74 = (s32)cursor;
        cursor += object->unk40->unk29 * 4;
    }
    if (object->unk40->unkE0 != 0) {
        cursor += func_800069C0((Objects69C0In *)object,
                                (Objects69C0Out *)cursor);
    }
    if (object->unk8D != 0) {
        aligned = align4(cursor);
        object->unk7C = (s32)aligned;
        cursor = aligned;
        cursor += object->unk40->unkA7 * 0x24;
    }

    size = (s32)(cursor - (u8 *)object);
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }
    newObject = object;
    object = (Objects0590CObject *)func_8002B4C0(D_800C94A0, size);
    if ((object == NULL) && (D_80078F88 != 0)) {
        object = (Objects0590CObject *)func_8002B280(size, 0x8B);
    }
    if (object == NULL) {
        if (D_800C9490 != 0) {
            func_800347A0(D_800C9490);
        }
        func_80006448(newObject);
        func_80004B04(selected);
        D_8007A21C = 2;
        return NULL;
    }

    resultSize = 0;
    size >>= 2;
    while (resultSize < size) {
        ((s32 *)object)[resultSize] = ((s32 *)D_800C9450)[resultSize];
        resultSize += 1;
    }
    if (object->unk54 != 0) {
        object->unk54 = (s32)((u32)object + (u32)object->unk54 - (u32)D_800C9450);
    }
    if (object->unk4C != 0) {
        object->unk4C = (s32)((u32)object + (u32)object->unk4C - (u32)D_800C9450);
        offset = *(s32 *)((u8 *)object->unk4C + 0x1C);
        if (offset != 0) {
            *(s32 *)((u8 *)(u32)object->unk4C + 0x1C) =
                (s32)((u32)object + offset - (u32)D_800C9450);
        }
    }
    if (object->unk50 != 0) {
        object->unk50 = (s32)((u32)object + (u32)object->unk50 - (u32)D_800C9450);
    }
    if (object->unk64 != 0) {
        object->unk64 = (s32)((u32)object + (u32)object->unk64 - (u32)D_800C9450);
    }
    if (object->unk48 != 0) {
        object->unk48 = (s32)((u32)object + (u32)object->unk48 - (u32)D_800C9450);
        aligned = (u8 *)*(s32 *)((u8 *)object->unk48 + 0x74);
        if (aligned != 0) {
            *(s32 *)((u8 *)(u32)object->unk48 + 0x74) =
                (s32)((u32)object + (s32)aligned - (u32)D_800C9450);
        }
    }
    if (object->unk58 != 0) {
        object->unk58 = (s32)((u32)object + (u32)object->unk58 - (u32)D_800C9450);
    }
    if (object->unk5C != 0) {
        object->unk5C = (s32)((u32)object + (u32)object->unk5C - (u32)D_800C9450);
    }
    if (object->unk60 != 0) {
        object->unk60 = (s32)((u32)object + (u32)object->unk60 - (u32)D_800C9450);
    }
    if (object->unk7C != 0) {
        object->unk7C = (s32)((u32)object + (u32)object->unk7C - (u32)D_800C9450);
    }
    if (object->unk6C != 0) {
        object->unk6C = (s32)((u32)object + (u32)object->unk6C - (u32)D_800C9450);
    }
    if (object->unk40->unk28 > 0) {
        object->unk70 = (s32)((u32)object + (u32)object->unk70 - (u32)D_800C9450);
    }
    if (object->unk40->unk29 > 0) {
        object->unk74 = (s32)((u32)object + (u32)object->unk74 - (u32)D_800C9450);
    }
    if (object->unk40->unkE0 != 0) {
        object->unk78 = (s32)((u32)object + (u32)object->unk78 - (u32)D_800C9450);
    }
    object->unk68 = (s32 *)((u8 *)object + 0x94);
    if (arg1 & 1) {
        D_800C9494[D_800C9498] = object;
        D_800C9498 += 1;
    } else {
        D_800C94A4[D_800C94A8] = (s32)object;
        D_800C94A8 += 1;
        if (D_800C94A8 > 0x100) { }
    }
    if (object->unk40->unk28 > 0) {
        lightSetupLightSources(object);
    }
    if (object->unk40->unk29 > 0) {
        lightSetupFlareSources(object);
    }
    func_8000AA38((Objects0AA38Object *)object, arg0, 0);
    if (object->unk58 != 0) {
        TrapDanglingJump(object);
    }
    if (object->unk48 != 0) {
        asset = object->unk40;
        func_80053550(object, asset->unkBA, asset->unkBB,
                      asset->unkB4, asset->unkB6,
                      asset->unkB8, asset->unkBC,
                      asset->unkC0, asset->unkC4,
                      asset->unkC8, asset->unkCC, 1);
    }
    if (func_80006534((Objects06534Object *)object) != 0) {
        if (D_800C9490 != 0) {
            func_800347A0(D_800C9490);
        }
        func_80006448(object);
        func_80004B04(selected);
        mmFree(object);
        if (arg1 & 1) {
            D_800C9498 -= 1;
        } else {
            D_800C94A8 -= 1;
        }
        D_8007A21C = 2;
        return NULL;
    }
    if ((loadFlags & 0x200) && (object->unk40->unk61 != 0)) {
        if ((*(s32 *)((u8 *)object->unk4C + 0x1C) != 0) &&
            ((*(u8 *)((u8 *)object->unk4C + 0x10) & 8) != 0)) {
            TrapDanglingJump(object);
        }
    }
    D_8007A21C = 4;
    return object;
}
/* Mickey-only reconstruction. Indexed accesses let IDO create the offset and
 * resource webs; the initializer shares the loop line to retain entry order. */
void func_80006448(void *arg0) {
    s32 index;
    s8 type;

    if (((Objects06448Arg *)arg0)->unk40->unk22 > 0) {
        index = 0; do {
            type = func_800058C0((Objects58C0Arg *)arg0, index);
            if (type == 0) {
                if (((Objects06448Arg *)arg0)->unk68[index] != 0) {
                    modFreeModel(((Objects06448Arg *)arg0)->unk68[index]);
                }
            } else if (type == 2) {
                if (((Objects06448Arg *)arg0)->unk68[index] != 0) {
                    func_800347A0(((Objects06448Arg *)arg0)->unk68[index]);
                }
            } else {
                if (((Objects06448Arg *)arg0)->unk68[index] != 0) {
                    func_800359D4(((Objects06448Arg *)arg0)->unk68[index]);
                }
            }
            index += 1;
        } while (index < ((Objects06448Arg *)arg0)->unk40->unk22);
    }
}
/* Configured C: 17 differences (188/205 exact), first +0x11C.
 * The region before the flag store raises the sprite web above the list cursor
 * without emitting instructions; the retained body remains NON_MATCHING. */
/* Earlier 63-word plateau: size, frame, CFG and all seven relocations exact. */
/* Three independent residuals, none of them structural:
 *   1. a two-web exchange, s1<->s2, between `sprite` and the compiler's
 *      induction pointer for `list->entries[index]` (21 + 10 sites);
 *   2. a caller-saved lane rotation, target [v1, a0, a1] against candidate
 *      [a0, a1, a2], over the record->unk8 carrier and the reference /
 *      frameOffset fields it feeds (7 + 5 + 5 sites);
 *   3. five ring-temp lane rotations downstream of (2).
 * Falsified (each measured, all still 63 words): every permutation of the
 * three NULL/zero initialisers; computing reference and frameOffset before
 * flags; all ten positions for `sprite` in the declaration list; an explicit
 * cursor advanced in place for list->entries; the arrow/deref and [0]-index
 * rewrite at every member access in the function, greedily composed.
 * Regressions: dropping the `flags` local and re-reading record->unk8 (144);
 * a separate-statement cursor increment (178). */
#ifdef NON_MATCHING
s32 func_80006534(Objects06534Object *object) {
    Objects06534Record *record;
    s32 count;
    Objects06534Sprite *sprite;
    Objects06534List *list;
    void *loaded;
    s32 failed;
    s32 index;
    s32 reference;
    s32 frameOffset;
    u32 flags;

    list = NULL;
    sprite = NULL;
    failed = 0;
    if ((object->unk40->unk23 > 0) && (object->unk40->unk23 < 0xA)) {
        list = object->unk5C;
        list->count = object->unk40->unk23;
        for (index = 0; index < list->count; index++) {
            loaded = func_80006C4C(object->unk40->unk38[index]);
            if (loaded != NULL) {
                *(f32 *)((u8 *)loaded + 8) *= object->unk8;
            }
            list->entries[index] = loaded;
            if (loaded == NULL) {
                failed = 1;
            }
        }
        list->unk2C = object->unk40->unk3C;
    }

    object->unk8C = object->unk40->unk24;
    if ((object->unk8C > 0) && (object->unk40->unk24 < 0xA)) {
        sprite = (Objects06534Sprite *)object->unk60;
        record = object->unk40->unk40;
        for (index = 0; index < object->unk8C; index++, sprite++, record++) {
            sprite->unk0 = func_800355A0(record->unk0, 1);
            sprite->unk4 = record->unk2;
            flags = record->unk8;
            frameOffset = ((u32)record->unk8 >> 16) & 0x3F;
            reference = ((u32)record->unk8 >> 22) & 0x3F;
            if (1) { }
            sprite->unkC = flags;
            sprite->unk8 = ((f32)record->unk4 / 500.0f) * object->unk8;
            sprite->unk5 = record->unk3;
            count = *(u8 *)sprite->unk0;
            if (flags & 0x80000000) {
                sprite->unk10 = (f32)mathRnd(0, count);
            } else if (flags & 0x40000000) {
                sprite->unk10 = ((Objects06534Sprite *)object->unk60)[reference].unk10 + (f32)frameOffset;
            } else if (flags & 0x20000000) {
                sprite->unk10 = ((Objects06534Sprite *)object->unk60)[reference].unk10 - (f32)frameOffset;
            }
            if ((f32)count < sprite->unk10) {
                sprite->unk10 -= (f32)count;
            } else if (sprite->unk10 < 0.0f) {
                sprite->unk10 += (f32)count;
            }
            if (sprite->unk0 == NULL) {
                failed = 1;
            }
        }
    }

    if (failed != 0) {
        if (list != NULL) {
            for (index = 0; index < list->count; index++) {
                loaded = list->entries[index];
                if (loaded != NULL) {
                    func_80006448(loaded);
                    func_80004B04(*(s16 *)((u8 *)loaded + 0x2C));
                    mmFree(loaded);
                }
            }
            list->unk2C = 0;
        }
        if (sprite != NULL) {
            sprite = (Objects06534Sprite *)object->unk60;
            for (index = 0; index < object->unk8C; index++) {
                if (sprite->unk0 != NULL) {
                    func_800359D4(sprite->unk0);
                    sprite->unk0 = NULL;
                }
            }
        }
    }
    return failed;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006534.s")
#endif
/* PROVENANCE: control-flow body adapted from Jet Force Gemini's public
 * src/objects.c func_80007494; Mickey's offsets, globals, and calls are authoritative. */
/* Workbench verdict: allocation-mismatch; 1 differing word (85/86). */
/* First mismatch: +0x7C; size, frame, CFG, and call/relocation shape are exact. */
/* Structural gap: none; s5/v1 comparison color is reserved for the permuter. */
s32 func_80006868(Objects06868Object *arg0, void *arg1)
{
  s32 temp_v0_2;
  s32 temp_v1;
  s32 temp_v1_2;
  s32 var_s1;
  s32 var_s3;
  s8 var_v1;
  Objects06868Entry *temp_a2;
  Objects06868Data *temp_v0;
  Objects06868Entry *var_s0;
  temp_v0 = arg0->unk40;
  arg0->unk6C = arg1;
  var_v1 = temp_v0->unk25;
  var_s3 = 0;
  temp_a2 = temp_v0->unk44;
  if (var_v1 <= 0)
  {
    goto done;
  }
  var_s0 = temp_a2;
  var_s1 = 0;
  do
  {
    temp_v0_2 = var_s0->unk0;
    temp_v1 = temp_v0_2 & 0xFFFF0000;
    if (temp_v1 == 0xFFFF0000)
    {
      partInitTrigger(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 8) & 0xFF, temp_v0_2 & 0xFF);
    }
    else
      if ((temp_v0_2 & 0xFFFF0000) == 0xFFFE0000)
    {
      partInitTriggerSPPos(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 8) & 0xFF, temp_v0_2 & 0xFF, var_s0->unk4 & 0xFF);
    }
    else
    {
      temp_v1_2 = var_s0->unk4;
      partInitTriggerPos(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 24) & 0xFF, (temp_v0_2 >> 16) & 0xFF, temp_v0_2 & 0xFFFF, (temp_v1_2 >> 16) & 0xFFFF, temp_v1_2 & 0xFFFF);
    }
    var_s3 += 1;
    var_s1 += 0x24;
    var_v1 = arg0->unk40->unk25;
    var_s0 += 1;
  }
  while (var_s3 < var_v1);
  done:
  return ((var_v1 * 0x24) + 3) & (~3);

}
s32 func_800069C0(Objects69C0In *arg0, Objects69C0Out *arg1) {
    arg0->unk78 = arg1;
    arg1->unk4 = arg0->unk40->unkE0->unk2C;
    arg0->unk78->unk0 = 2;
    return 0x2C;
}
s32 func_800069E8(Objects069E8Object *arg0, Objects069E8Target *arg1) {
    Objects069E8Source *source;
    Objects069E8Source *source2;
    s32 sp1C;
    s32 temp_v0_2;

    source2 = arg0->unk40;
    source = source2;
    arg0->unk4C = arg1;
    arg1->unk0 = source2->unk54;
    arg1->unk4 = source2->unk58;
    arg1->unk10 = source2->unk61;
    arg1->unk11 = source2->unk62;
    arg1->unk12 = source2->unk63;
    arg1->unk13 = 0;
    arg1->unkC = 0;
    arg1->unkE = source2->unk66;
    sp1C = 0x20;
    arg1->unk1C = 0;
    if (func_800291FC() == 0) {
        if (arg1->unk10 & 8) {
            /* The record's own aligned tail is the cursor.  Reading the
               stored field back at the second use is what keeps the +4 out
               of the size constant: spelled through a local, IDO reassociates
               (cursor + 4) - base + 0xBC into cursor - base + 0xC0. */
            arg1->unk1C = ((s32)((u8 *)arg1 + 0x20) & ~3) + 4;
            sp1C = (arg1->unk1C - (s32)arg1) + 0xBC;
        }
    } else {
        arg1->unk10 = 5;
        arg1->unk11 = 1;
    }
    if (arg0->unk40->unk1E == 0) {
        arg1->unk10 |= 0x20;
    }
    temp_v0_2 = func_80034448(source->unk64, arg1);
    arg1->unk8 = temp_v0_2;
    if (temp_v0_2 == 0) {
        return 0;
    }
    D_800C9490 = arg1->unk8;
    return (sp1C & ~3) + 4;
}
/* Workbench verdict: structure-mismatch; 63 differing words (78 candidate / 79 target). */
/* First mismatch: +0x20; both outputs are frameless and the candidate is one instruction shorter. */
/* Structural gap: asset/count carrier and late loop register shape remain unresolved. */
#ifdef NON_MATCHING
s32 func_80006B04(Objects06B04Object *arg0, Objects06B04Output *arg1, volatile s32 arg2) {
    s16 temp_t0;
    s32 var_a3;
    s32 var_t2;
    s32 var_v1;
    u8 temp_a3;
    u8 temp_t4;
    Objects06B04Asset *temp_a2;
    u8 *temp_t9;
    u8 *var_a1;
    Objects06B04Output *output;

    arg0->unk48 = arg1;
    var_v1 = 0x7C;
    if (arg0->unk40->unk1E == 0) {
        temp_t9 = (u8 *)arg1 + 0x7C;
        temp_a2 = **arg0->unk68;
        output = arg1;
        temp_a3 = temp_a2->unk2F;
        if ((s32)temp_a3 > 0) {
            output->unkA = temp_a3;
            temp_t0 = output->unkA;
            output->unk74 = temp_t9;
            var_t2 = 0;
            var_v1 = (temp_t0 * 0x34) + 0x7C;
            if (temp_t0 > 0) {
                var_a1 = temp_t9;
                var_a3 = 0;
                do {
                    *(u16 *)var_a1 = 0;
                    *(u16 *)(var_a1 + 2) = 0;
                    *(u16 *)(var_a1 + 4) = 0;
                    var_t2 += 1;
                    var_a1 += 0x34;
                    *(u16 *)(var_a1 - 0x2E) = *(u16 *)(temp_a2->unk38 + var_a3);
                    *(s8 *)(var_a1 - 0x2C) = *(s8 *)(temp_a2->unk38 + var_a3 + 2);
                    *(u16 *)(var_a1 - 0x2A) = 0;
                    *(u8 *)(var_a1 - 0x2B) = *(u8 *)(temp_a2->unk38 + var_a3 + 3);
                    *(f32 *)(var_a1 - 0x10) = *(f32 *)(temp_a2->unk38 + var_a3 + 8) * arg0->unk8;
                    *(f32 *)(var_a1 - 0xC) = *(f32 *)(temp_a2->unk38 + var_a3 + 8) * arg0->unk8;
                    *(u8 *)(var_a1 - 0x6) = (u8)*(u16 *)(temp_a2->unk38 + var_a3 + 4);
                    temp_t4 = *(u8 *)(var_a1 - 0x4) | 0x80;
                    *(u8 *)(var_a1 - 0x4) = temp_t4;
                    *(u16 *)(var_a1 - 0x8) = 0;
                    *(u8 *)(var_a1 - 0x4) = temp_t4 & 0xBF;
                    *(u8 *)(var_a1 - 0x5) = (u8)*(u16 *)(temp_a2->unk38 + var_a3 + 6);
                    var_a3 += 0xC;
                } while (var_t2 < output->unkA);
            }
        }
    }
    return var_v1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006B04.s")
#endif
s32 func_80006C40(Objects06C40 *arg0, s32 arg1) {
    arg0->unk58 = arg1;
    return 0x13C;
}
void *func_80006C4C(s32 arg0) {
    Objects06C4CAsset *asset;
    Objects06C4CObject *object;
    u8 *end;
    s32 count;
    s32 index;
    s32 loadType;
    s32 spriteModel;
    void *result;
    s32 failed;
    s32 size;
    s32 *destination;
    s32 *source;

    if (arg0 >= D_800C945C) {
        arg0 = 0;
    }
    asset = func_8000486C(arg0);
    if (asset == NULL) {
        return NULL;
    }

    object = (Objects06C4CObject *)D_800C9450;
    size = asset->unk22;
    size = size * 4 + 0x94;
    end = (u8 *)object + size;
    count = D_800C9454 >> 2;
    destination = (s32 *)D_800C9450;
    while (count--) {
        *destination++ = 0;
    }

    object->unk40 = asset;
    object->unk6 = 2;
    object->unk2C = (s16)arg0;
    object->unk46 = (s16)arg0;
    object->unk8 = asset->unk0;
    object->unk6 = (s16)(object->unk6 | (asset->unk14 & 0x20C));
    loadType = func_8000A6E8(asset->unk1C) & 3;
    failed = 0;
    index = 0;
    if (object->unk40->unk22 > 0) {
        do {
            spriteModel = func_800058C0((Objects58C0Arg *)object, index);
            object->unk68 = object->unk94;
            if (spriteModel == 0) {
                object->unk68[index] =
                    (s32)func_8001F520(
                        object->unk40->unk34[index], loadType);
            } else {
                object->unk68[index] =
                    (s32)func_800355A0(
                        object->unk40->unk34[index], 0xA);
            }
            if (object->unk68[index] == 0) {
                failed = 1;
            }
            index += 1;
        } while (index < object->unk40->unk22);
    }

    if (failed == 0) {
        size = (s32)(end - (u8 *)object);
        if (size & 0xF) {
            size = (size + 0xF) & ~0xF;
        }
        result = func_8002B314(size, 0x8B);
        if (result != NULL) {
            count = size >> 2;
            source = (s32 *)D_800C9450;
            destination = (s32 *)result;
            while (count--) {
                *destination++ = *source++;
            }
            ((Objects06C4CObject *)result)->unk68 =
                ((Objects06C4CObject *)result)->unk94;
        } else {
            failed = 1;
        }
    }
    if (failed != 0) {
        func_80006448(object);
        func_80004B04(arg0);
        result = NULL;
    }
    return result;
}
void func_80006EA0(void *ptr) {
    if (((u8 *) ptr)[0x91] == 0) {
        ((u8 *) ptr)[0x91] = 1;
        D_800C94EC[D_800C94F0] = ptr;
        D_800C94F0 += 1;
    }
}
/* Workbench verdict: structure-mismatch; 40 differing words, candidate 46/47. */
/* First mismatch: +0x08; the target has one direct global-pointer load extra. */
/* Structural candidate: global-pointer load/register shape remains unresolved. */
#ifdef NON_MATCHING
s32 func_80006EE4(s32 object) {
    s32 index;
    s32 *entry;

    index = 0;
    if (D_800C94A8 > 0) {
        entry = D_800C94A4;
        do {
            if (object == *entry) {
                D_800C94A8 -= 1;
                if (index < D_800C94A8) {
                    s32 shiftIndex = index;
                    s32 offset = shiftIndex * 4;

                    do {
                        shiftIndex += 1;
                        *(s32 *)((u8 *)D_800C94A4 + offset) =
                            *(s32 *)((u8 *)D_800C94A4 + offset + 4);
                        offset += 4;
                    } while (shiftIndex < D_800C94A8);
                }
                func_80007118(object, 0, index, &D_800C94A8);
                return index;
            }
            index += 1;
            entry += 1;
        } while (index < D_800C94A8);
    }
    return -1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006EE4.s")
#endif
/* Workbench verdict: structure-mismatch; 63 differing words (94/94). */
/* First mismatch: +0x0C; frame and instruction count are exact. */
/* Structural gap: active-list carriers and loop register allocation remain unresolved. */
#ifdef NON_MATCHING
void func_80006FA0(void) {
    s32 *temp_v0;
    s32 temp_a0;
    s32 temp_s2;
    s32 temp_t4;
    s32 temp_t7;
    s32 var_a0;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;
    s32 var_v0;
    s32 var_v1;

    var_s5 = 0;
    if (D_800C94F0 > 0) {
        var_s6 = 0;
        do {
            temp_a0 = D_800C9498;
            var_s4 = -1;
            temp_s2 = *(s32 *)((u8 *)D_800C94EC + var_s6);
            var_v0 = 0;
            if (temp_a0 > 0) {
                do {
                    temp_t4 = temp_a0 - 1;
                    if (temp_s2 == ((s32 *)D_800C9494)[var_v0]) {
                        var_s4 = var_v0;
                        var_a0 = var_v0 * 4;
                        if (var_v0 < D_800C94B2) {
                            D_800C94B2 -= 1;
                        }
                        D_800C9498 = temp_t4;
                        var_v1 = var_v0;
                        if (var_v0 < temp_t4) {
                            do {
                                var_v1 += 1;
                                temp_v0 = (s32 *)((u8 *)D_800C9494 + var_a0);
                                temp_t7 = temp_v0[1];
                                var_a0 += 4;
                                temp_v0[0] = temp_t7;
                            } while (var_v1 < D_800C9498);
                        }
                        func_80007118(temp_s2, 0);
                        var_v0 = D_800C9498;
                    }
                    var_v0 += 1;
                } while (var_v0 < temp_a0);
            }
            if (var_s4 == -1) {
                func_80006EE4(temp_s2);
            }
            var_s5 += 1;
            var_s6 += 4;
        } while (var_s5 < D_800C94F0);
    }
    D_800C94F0 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006FA0.s")
#endif
extern void func_800031E8(void *object);
extern void func_80005798(void *object);
extern void func_8000D728(void *object);
extern void camlightDelete(void *object);
extern void partObjFreeTriggers(void *object);
extern void partNullifyCircularParticleParents(void *object);
extern s32 lightKillGlowingLight();
extern void func_80048980(void *object);
extern void func_8001C088(void *object);
extern void killLight(void *light);
extern void amSndStop(void *sound);
extern void func_80046E70(void *object);

#ifdef NON_MATCHING
void func_80007118(u8 *object, s32 arg1) {
    s32 i;
    s32 offset;
    u8 *entry;
    u8 *payload;
    void *value;
    u8 *linkedPayload;
    u8 *owner;

    if (*(s16 *)(object + 0x44) == 1) {
        payload = *(u8 **)(object + 0x64);
        value = *(void **)(payload + 0xD0);
        if (value != NULL && *(u8 *)((u8 *)value + 0x91) == 0) {
            func_80006EE4((s32)value);
        }
        value = *(void **)(payload + 0xD4);
        if (value != NULL && *(u8 *)((u8 *)value + 0x91) == 0) {
            func_80006EE4((s32)value);
        }
    }
    D_8007A210 = 2;
    D_8007A21C = 1;
    D_8007A214 = object;
    D_8007A218 = (s32)(*(u8 **)(object + 0x40) + 4);

    if (*(void **)(object + 0x5C) != NULL) {
        for (i = 0, offset = 0; i < **(s32 **)(object + 0x5C); i++, offset += 4) {
            entry = *(u8 **)(*(u8 **)(object + 0x5C) + 4 + offset);
            func_80006448(entry);
            func_80004B04(*(s16 *)(entry + 0x2C));
            mmFree(entry);
        }
    }
    if (*(void **)(object + 0x60) != NULL) {
        for (i = 0, offset = 0; i < *(u8 *)(object + 0x8C); i++, offset += 0x14) {
            func_800359D4(*(void **)(*(u8 **)(object + 0x60) + offset));
        }
    }
    partObjFreeTriggers(object);
    if (*(void **)(object + 0x70) != NULL) {
        for (i = 0, offset = 0; i < *(s8 *)(*(u8 **)(object + 0x40) + 0x28); i++, offset += 4) {
            killLight(*(void **)(*(u8 **)(object + 0x70) + offset));
        }
    }
    if (*(void **)(object + 0x74) != NULL) {
        for (i = 0, offset = 0; i < *(u8 *)(*(u8 **)(object + 0x40) + 0x29); i++, offset += 4) {
            camlightDelete(*(void **)(*(u8 **)(object + 0x74) + offset));
        }
    }

    switch (*(s16 *)(object + 0x44)) {
        case 92:
            payload = *(u8 **)(object + 0x64);
            TrapDanglingJump(payload);
            break;
        case 65:
            payload = *(u8 **)(object + 0x64);
            func_800359D4(*(void **)(payload + 0x20));
            break;
        case 67:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x3C);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 72:
            TrapDanglingJump();
            break;
        case 76:
            TrapDanglingJump();
            break;
        case 78:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x3C);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 68:
            linkedPayload = *(u8 **)(object + 0x64);
            value = *(void **)(linkedPayload + 0x18);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(linkedPayload + 0x1C);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(linkedPayload + 0x20);
            if (value != NULL) {
                owner = *(u8 **)((u8 *)value + 0x64);
                if (*(void **)(owner + 0xD4) == object) {
                    *(void **)(owner + 0xD4) = NULL;
                }
            }
            break;
        case 66:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x0);
            if (value != NULL) {
                owner = *(u8 **)((u8 *)value + 0x64);
                if (*(void **)(owner + 0xD0) == object) {
                    *(void **)(owner + 0xD0) = NULL;
                }
            }
            value = *(void **)(payload + 0x14);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 89:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x10);
            if (value != NULL) {
                owner = *(u8 **)((u8 *)value + 0x64);
                if (*(void **)(owner + 0xD8) == object) {
                    *(void **)(owner + 0xD8) = NULL;
                }
            }
            break;
        case 90:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x0);
            if (value != NULL) {
                amSndStop(value);
            }
            break;
        case 64:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x3C);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 55:
            linkedPayload = *(u8 **)(object + 0x64);
            value = *(void **)(linkedPayload + 0x18);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(linkedPayload + 0x40);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 54:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0x38);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 71:
            D_80078F7C = NULL;
            break;
        case 63:
            value = (void *)D_8007A1F4;
            if (value != NULL && *(void **)value == object) {
                *(void **)value = NULL;
            }
            break;
        case 61:
            TrapDanglingJump(object);
            break;
        case 62:
            TrapDanglingJump(object);
            break;
        case 14:
            func_8001C088(object);
            break;
        case 15:
            payload = *(u8 **)(object + 0x64);
            killLight(payload);
            break;
        case 41:
            linkedPayload = *(u8 **)(object + 0x64);
            value = *(void **)(linkedPayload + 0x20);
            if (value != NULL) {
                killLight(value);
            }
            value = *(void **)(linkedPayload + 0x2C);
            if (value != NULL) {
                func_8000D728(value);
            }
            break;
        case 9:
            value = *(void **)(object + 0x84);
            if (value != NULL) {
                camlightDelete(value);
            }
            break;
        case 35:
            lightKillGlowingLight(*(void **)(object + 0x64));
            break;
        case 42:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 4);
            if (value != NULL) {
                mmFree(value);
            }
            break;
        case 29:
            func_80005798(object);
            value = *(void **)(object + 0x84);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 73:
            func_80005798(object);
            break;
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 79:
            value = *(void **)(object + 0x84);
            if (value != NULL) {
                func_800031E8(value);
            }
            break;
        case 1:
            payload = *(u8 **)(object + 0x64);
            value = *(void **)(payload + 0xA4);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xA8);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xAC);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xB0);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xB4);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xB8);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xBC);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xC0);
            if (value != NULL) {
                func_800031E8(value);
            }
            value = *(void **)(payload + 0xC4);
            if (value != NULL) {
                func_800031E8(value);
            }
            for (i = 0; i < 0x10; i += 4) {
                value = *(void **)(payload + 0x134 + i);
                if (value != NULL) {
                    TrapDanglingJump(value);
                }
            }
            for (i = 0; i != 0x30; i += 0xC) {
                value = *(void **)(payload + 0x354 + i);
                if (value != NULL) {
                    func_80046E70(value);
                }
            }
            break;
        default:
            break;
    }

    payload = *(u8 **)(object + 0x4C);
    if (payload != NULL) {
        if (*(void **)(payload + 0x1C) != NULL && ((*(u8 *)(*(u8 **)(object + 0x40) + 0x61) & 8) != 0)) {
            TrapDanglingJump(*(void **)(payload + 0x1C));
            payload = *(u8 **)(object + 0x4C);
        }
        value = *(void **)(payload + 8);
        if (value != NULL) {
            func_800347A0(value);
        }
    }
    value = *(void **)(object + 0x54);
    if (value != NULL) {
        func_80048980(value);
    }
    value = *(void **)(object + 0x78);
    if (value != NULL) {
        value = *(void **)((u8 *)value + 0x24);
        if (value != NULL) {
            func_800031E8(value);
        }
    }
    if (*(u8 *)(object + 0x92) != 0) {
        partNullifyCircularParticleParents(object);
    }
    func_80006448(object);
    func_80004B04(*(s16 *)(object + 0x2C));
    mmFree(object);
    D_8007A21C = 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007118.s")
#endif
void func_80007844(void) {
}
typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
    u8 pad1F[0x82];
    u8 unkA1[1];
    u8 unkA2;
} Objects0784CData;

typedef struct {
    u8 pad00[0x38];
    f32 unk38;
    f32 unk3C;
    f32 unk40;
    u8 pad44[0xB2];
    s16 unkF6;
    s16 unkF8;
    s16 unkFA;
} Objects0784COutput;

typedef struct {
    u8 pad00[8];
    void *unk8;
    u16 unkC;
    u16 unkE;
} Objects0784CAnimation;

typedef struct {
    void *unk0;
    u8 pad04[1];
    u8 unk5;
    u8 pad06[6];
    s32 unkC;
    u8 pad10[4];
} Objects0784CEffect;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad06[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x28];
    Objects0784CData *unk40;
    s16 unk44;
    u8 pad46[6];
    Objects0784CAnimation *unk4C;
    u8 pad50[4];
    void *unk54;
    u8 pad58[8];
    Objects0784CEffect *unk60;
    Objects0784COutput *unk64;
    u8 pad68[0x24];
    u8 unk8C;
} Objects0784CObject;

extern s32 runlinkIsModuleLoaded(s32 module);
extern void func_8000AEEC(void *object, s32 updateRate);
extern void func_8000B3CC(void *object, s32 updateRate);
extern void spranimOnceControl(void *object, s32 updateRate);
extern void spranimControl(void *object, s32 updateRate);
extern void texscrollControl(void *object, s32 updateRate);
extern void effectboxControl(void *object, s32 updateRate);
extern void func_800148E0(void *object);
extern void func_8001B798(void *object, s32 updateRate);
extern void func_8001BB04(void *object, s32 updateRate);
extern void func_8001BB10(void *object, s32 updateRate);
extern void rangetriggerControl(void *object, s32 updateRate);
extern void func_80007E40(Objects07E40Object *object, s32 updateRate);
extern void func_80049000(void *object, s32 updateRate);
extern void func_80036544(void *entry, s32 *mode, s32 animationId, void *state,
                          s32 updateRate);
extern void func_8001CB84(void *object, s32 updateRate);
extern void func_8001D2A0(void *object, s32 updateRate);
extern void func_80053868(s32 updateRate);
extern void lightUpdateLights(s32 updateRate);
extern void lightUpdateObjects(void);
extern void amPlayAudioMap(void **objects, s32 count, s32 updateRate);

/* The per-frame object update pass: refreshes the render records of the
 * always-on object list, walks the active object range calling each object's
 * update, animation and effect work, defers four object classes to a bounded
 * pending list, and finishes with the lighting and audio passes.
 *
 * The animation period is a *named* local re-read on every test of the wrap
 * loop, which is the whole difference between this and a plain
 * `while (unkC >= period_field)` with the field spelled at both uses. Under
 * -O2 the frame is an arithmetic identity,
 *
 *     frame = roundup8(outgoing + saved + 4*temps + 4*scalars + aggregate)
 *
 * with the aggregate based at `frame - 4*scalars - aggregate`, so the array's
 * home reads out the split between declared scalars and compiler temporaries
 * even when neither side's swapped slots are ever touched. The plain spelling
 * spends two compiler temporaries on the wrap loop's common subexpression and
 * lands `pending` four bytes high; naming the period pays those two back and
 * costs one scalar, and re-reading the field in the condition keeps the
 * in-loop store and reload the target emits. Adding a ninth scalar without
 * removing a temporary moves the array the wrong way, which is why sixteen
 * earlier declaration probes all landed at +4. */
void func_8000784C(s32 arg0) {
    Objects0784CObject *object;
    Objects0784COutput *output;
    Objects0784CAnimation *animation;
    Objects0784CEffect *effect;
    s32 i;
    s32 count;
    s32 objectOffset;
    s32 pendingCount;
    s32 animPeriod;
    Objects0784CObject *pending[0x20];

    count = 0;
    if (D_800C94F8 > 0) {
        do {
            output = ((Objects0784CObject *)D_800C94F4[count])->unk64;
            output->unk38 = ((Objects0784CObject *)D_800C94F4[count])->unkC;
            output->unk3C =
                ((Objects0784CObject *)D_800C94F4[count])->unk10;
            output->unk40 =
                ((Objects0784CObject *)D_800C94F4[count])->unk14;
            output->unkF6 =
                ((Objects0784CObject *)D_800C94F4[count])->unk0;
            output->unkF8 =
                ((Objects0784CObject *)D_800C94F4[count])->unk2;
            output->unkFA =
                ((Objects0784CObject *)D_800C94F4[count])->unk4;
            count += 1;
        } while (count < D_800C94F8);
    }


    if (runlinkIsModuleLoaded(0x14) != 0) {
        TrapDanglingJump(arg0);
    }

    count = D_800C9498;
    objectOffset = D_800C949C;
    pendingCount = 0;
    if (objectOffset < count) {
        do {
            object = ((Objects0784CObject **)D_800C9494)[objectOffset];
            if ((object->unk44 == 7) || (object->unk44 == 0x1C) ||
                (object->unk44 == 0x50) || (object->unk44 == 0x42)) {
                if (pendingCount < 0x20) {
                    pending[pendingCount] = object;
                    pendingCount += 1;
                }
            } else if (object->unk44 != 0xF) {
                func_8000AEEC(object, arg0);
                if ((object->unk40->unk1E == 0) && (object->unk40->unkA2 != 0xFF)) {
                    func_80007E40((Objects07E40Object *)object, arg0);
                }
                if ((object->unk44 != 1) && (object->unk54 != NULL)) {
                    func_80049000(object, arg0);
                }
                animation = (Objects0784CAnimation *)object->unk4C;
                if (animation != NULL) {
                    if (animation->unkE != 0) {
                        if (animation->unk8 != NULL) {
                            animation->unkC += (u32)animation->unkE * arg0;
                            while (animation->unkC >=
                                   (animPeriod = ((u16 *)animation->unk8)[8])) {
                                animation->unkC -= animPeriod;
                            }
                        }
                    }
                }
                if (object->unk60 != NULL) {
                    effect = object->unk60;
                    i = 0;
                    while (i < object->unk8C) {
                        func_80036544(effect->unk0, &effect->unkC,
                                      effect->unk5, &effect->pad10,
                                      arg0);
                        i += 1;
                        effect += 1;
                    }
                }
            }
            objectOffset += 1;
        } while (objectOffset < count);
    }

    if (D_80078F80 != 0) {
        D_80078F80 -= arg0;
        if (D_80078F80 < 0) {
            D_80078F80 = 0;
        }
    }

    i = 0;
    for (objectOffset = 0; objectOffset < D_800C94F8; objectOffset++) {
        func_8001CB84(D_800C94F4[objectOffset], arg0);
    }
    func_80053868(arg0);
    for (objectOffset = 0; objectOffset < D_800C94F8; objectOffset++) {
        func_8001D2A0(D_800C94F4[objectOffset], arg0);
    }
    if (pendingCount > 0) {
        objectOffset = (s32)pending;
        do {
            func_8000AEEC(*(Objects0784CObject **)objectOffset, arg0);
            i++;
            objectOffset += sizeof(void *);
        } while (i != pendingCount);
    }
    lightUpdateLights(arg0);
    lightUpdateObjects();
    amPlayAudioMap(D_800C94F4, D_800C94F8, arg0);
    if (*func_80028F54() == 1) {
        TrapDanglingJump(arg0);
    }
    if (D_800C9478 == 0) {
        func_80006FA0();
    }
    D_800C9478 = 1;
    D_800C946C = (f32)arg0;
}
/* Workbench verdict: allocation; 25 differing words (93/118). */
/* First mismatch: +0x94; size, frame, stack homes and opcode schedule are exact. */
/* Structural gap: none; one ugen ring rotation from +0x94 remains. */
#ifdef NON_MATCHING
void func_80007C68(Objects07C68Object *arg0, Objects07C68Source *arg1,
                   Objects07C68Object *arg2, s32 arg3) {
    s16 temp_v0_2;
    s32 sp58;
    s32 temp_t3;
    s32 temp_v0;
    s32 var_s3;
    Objects07C68Record *var_s0;
    s16 *var_s2;
    Objects07C68Texture *texture;

    if (func_800290A0() != 0) {
        arg3 = 0;
    }
    var_s0 = arg2->unk4C;
    if (var_s0 != NULL) {
        var_s3 = 0;
        var_s2 = ((Objects07C68Indexed *)((u8 *)arg2 + (arg2->unkA * 4)))->unk50;
        if ((s32)arg1->unk2C > 0) {
            do {
                temp_v0 = var_s0->unk4;
                texture = (Objects07C68Texture *)arg1->unk18[(temp_v0 & 0xFF) * 2];
                if (temp_v0 & 0x100000) {
                    sp58 = (s32)var_s0->unk0;
                    if (var_s0->unk4 & 0x200000) {
                        D_8007BDA0 = arg0->unk90;
                    }
                    func_800367E8(texture, &var_s0->unk4, &sp58, arg3);
                    var_s0->unk0 = (s16)sp58;
                    if (var_s0->unk2 >= 0) {
                        temp_t3 = sp58 + 0x100;
                        sp58 = temp_t3;
                        if (temp_t3 >= (s32)texture->unk10) {
                            if (texture->unk3 & 2) {
                                sp58 = 0;
                            } else {
                                sp58 -= 0x100;
                            }
                        }
                        var_s0->unk2 = (s16)sp58;
                    }
                }
                temp_v0_2 = var_s0->unk2;
                if (temp_v0_2 >= 0) {
                    var_s2 += 1;
                    /* `(*texture).unkE`, not `texture->unkE`: the two spellings
                     * are semantically identical but cfe emits a different
                     * expression-temp order for them, and this one is the
                     * target's (26 -> 25 differing words, measured). */
                    var_s2[-1] = (s16)((temp_v0_2 >> 8) * (*texture).unkE);
                }
                var_s3 += 1;
                var_s2 += 1;
                var_s2[-1] = (s16)(((s16)var_s0->unk0 >> 8) * texture->unkE);
                var_s0 += 1;
            } while (var_s3 < (s32)arg1->unk2C);
        }
        if (arg0->unk90 == 1) {
            arg0->unk90 = 2;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007C68.s")
#endif
/* Workbench verdict: structure-mismatch; 92 differing words (122/122). */
/* First mismatch: +0x18; extent and frame 0x18 are exact, with no relocations. */
/* Blocker: pool/temp allocation and dimension-load/offset-initialization schedule. */
#ifdef NON_MATCHING
void func_80007E40(Objects07E40Object *arg0, s32 arg1) {
    Objects07E40Object *object;
    Objects07E40Outer *model;
    Objects07E40Data *temp_v0;
    s16 textureWidth;
    s16 textureHeight;
    s16 temp_lo;
    s16 temp_lo_2;
    s16 temp_s3;
    s32 edge1;
    s32 edge2;
    s32 var_s2;
    s32 var_a3_2;
    s32 var_t2;
    s32 var_t4;
    s32 var_t3;
    s32 temp_t0;
    s32 var_a3;
    s32 var_t1;
    Objects07E40Texture *temp_t4;
    Objects07E40Record *temp_t5;
    Objects07E40Inner *temp_v0_2;
    u8 *var_s1;

    object = arg0;
    temp_v0 = arg0->unk40;
    var_a3 = temp_v0->unkA3;
    temp_lo = temp_v0->unkA4 * arg1;
    temp_t0 = temp_v0->unkA2;
    temp_lo_2 = temp_v0->unkA5 * arg1;
    if ((var_a3 == 0xFF) || ((s32) var_a3 < temp_v0->unk22)) {
        if (var_a3 == 0xFF) {
            var_a3 = 0;
            var_t3 = temp_v0->unk22;
        } else {
            var_t3 = var_a3 + 1;
        }
        var_t1 = var_a3;
        if ((s32) var_a3 < var_t3) {
            var_t2 = var_a3 * 4;
            do {
                var_t1 += 1;
                model = *(Objects07E40Outer **)((u8 *)object->unk68 + var_t2);
                temp_v0_2 = model->unk0;
                if ((s32) temp_t0 < (s32) temp_v0_2->unk10) {
                    temp_t4 = *(Objects07E40Texture **)(temp_v0_2->unk18 + (temp_t0 * 8));
                    textureWidth = temp_t4->unk6 << 5;
                    textureHeight = temp_t4->unk8 << 5;
                    var_a3_2 = 0;
                    var_t4 = 0;
                    if (temp_v0_2->unk16 > 0) {
                        var_s1 = temp_v0_2->unk24;
                        do {
                            var_a3_2 += 1;
                            if (temp_t0 == *(u8 *)var_s1) {
                                var_s2 = *(s16 *)(var_s1 + 8);
                                if (var_s2 < *(s16 *)(var_s1 + 0x18)) {
                                    do {
                                        temp_t5 = (Objects07E40Record *)temp_v0_2->unk20 + var_s2;
                                        if (temp_lo != 0) {
                                            temp_s3 = temp_t5->unk4;
                                            edge1 = temp_t5->unk8 - temp_s3;
                                            edge2 = temp_t5->unkC - temp_s3;
                                            temp_t5->unk4 = (temp_s3 + temp_lo) &
                                                (textureWidth - 1);
                                            temp_s3 = temp_t5->unk4;
                                            temp_t5->unk8 = temp_s3 + edge1;
                                            temp_t5->unkC = temp_s3 + edge2;
                                        }
                                        if (temp_lo_2 != 0) {
                                            temp_s3 = temp_t5->unk6;
                                            edge1 = temp_t5->unkA - temp_s3;
                                            edge2 = temp_t5->unkE - temp_s3;
                                            temp_t5->unk6 = (temp_s3 + temp_lo_2) &
                                                (textureHeight - 1);
                                            temp_s3 = temp_t5->unk6;
                                            temp_t5->unkA = temp_s3 + edge1;
                                            temp_t5->unkE = temp_s3 + edge2;
                                        }
                                        var_s2 += 1;
                                        var_s1 = temp_v0_2->unk24 + var_t4;
                                    } while (var_s2 < *(s16 *)(var_s1 + 0x18));
                                }
                            }
                            var_t4 += 0x10;
                            var_s1 += 0x10;
                        } while (var_a3_2 < temp_v0_2->unk16);
                    }
                }
                var_t2 += 4;
            } while (var_t1 != var_t3);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007E40.s")
#endif
/* Workbench verdict: structure-mismatch; 28 differing words (60/60). */
/* First mismatch: +0x24; size and frame are exact, with a near-identical CFG. */
/* Structural gap: outer object/offset carriers and inner model-index allocation differ. */
#ifdef NON_MATCHING
void func_80008028(s32 arg0) {
    s32 objectIndex;
    Objects08028Object *object;
    s32 objectOffset;
    s32 modelIndex;
    s32 updateModels;
    Objects58C0Data *data;
    Objects08028Model *model;
    u8 modelReferences;

    objectIndex = D_800C949C;
    if (objectIndex < D_800C9498) {
        objectOffset = objectIndex * 4;
        do {
            objectIndex += 1;
            object = *(Objects08028Object **)((u8 *)D_800C9494 + objectOffset);
            data = object->unk40;
            if (data->unk1E[0] == 0) {
                updateModels = 0;
                if (data->unkD0[1] != 0.0f) {
                    updateModels = 1;
                }
                if (data->unk22 > 0) {
                    modelIndex = 0;
                    do {
                        if ((updateModels == 0) || (data->unk1E[modelIndex] == 0)) {
                            model = object->unk68[modelIndex];
                            modelReferences = model->unk3F;
                            model->unk8 = arg0;
                            if (modelReferences != 0) {
                                model->unk3F = modelReferences - 1;
                            }
                        }
                        modelIndex += 1;
                    } while (modelIndex < object->unk40->unk22);
                }
            }
            objectOffset += 4;
        } while (objectIndex < D_800C9498);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80008028.s")
#endif
void func_80008118(void) {
    D_80079004 = 1;
}
s32 func_80008128(Objects08128Object *arg0, f32 arg1, f32 arg2, f32 arg3) {
    Objects08128Track *track;
    Objects08128Bounds *bounds;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    s32 minX;
    s32 x;
    s32 y;
    s32 z;
    s32 result;
    s32 minY;
    s32 minZ;
    s32 maxX;
    s32 maxY;
    s32 maxZ;

    track = trackGetTrack();
    deltaX = arg1;
    deltaY = arg2;
    deltaZ = arg3;
    x = (s32) (arg0->unkC + deltaX);
    result = 0;
    y = (s32) (arg0->unk10 + deltaY);
    z = (s32) (arg0->unk14 + deltaZ);
    if ((D_80079004 == 0) && (track != NULL)) {
        minX = track->unk20 - 1000;
        minY = track->unk24 - 1000;
        minZ = track->unk28 - 1000;
        maxX = track->unk22 + 1000;
        maxY = track->unk26 + 1000;
        maxZ = track->unk2A + 1000;
        if ((x < minX) || (y < minY) ||
            (z < minZ) || (maxX < x) ||
            (maxY < y) || (maxZ < z)) {
            result = 1;
        }
    }
    D_80079004 = 0;
    if (result == 0) {
        arg0->unkC += deltaX;
        arg0->unk10 += deltaY;
        arg0->unk14 += deltaZ;
        if (track != NULL) {
            bounds = func_8000FEEC(arg0->unk2E);
            if ((bounds != NULL) &&
                ((x < bounds->unk0) || (y < bounds->unk2) ||
                 (z < bounds->unk4) || (bounds->unk6 < x) ||
                 (bounds->unk8 < y) || (bounds->unkA < z))) {
                bounds = NULL;
            }
            if (bounds == NULL) {
                arg0->unk2E = func_8000FAE0(arg0->unkC, arg0->unk10,
                                            arg0->unk14);
            }
        }
    }
    return result;
}
#ifdef NON_MATCHING
void func_8000831C(void *arg0, void *arg1, s32 arg2, void *arg3, s32 arg4,
                   s32 arg5, s32 arg6, s32 arg7, f32 arg8, s32 arg9, s32 arg10) {
    s32 sp24;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_t3;
    Objects0831CCommand *temp_v0;

    sp24 = 0;
    camPushModelMtx((Gfx **)&D_800C94B4, (Mtx **)&D_800C94B8,
                    (CameraScaledTransform *)arg0, arg8, 0.0f);
    if ((arg6 & 0x240) == 0) {
        temp_v0 = (Objects0831CCommand *)D_800C94B4;
        D_800C94B4 = (s32)(temp_v0 + 1);
        temp_v0->unk0 = 0xFA000000;
        temp_t3 = arg9 & 0xFF;
        temp_v0->unk4 = (temp_t3 << 24) | (temp_t3 << 16) |
                        (temp_t3 << 8) | (arg10 & 0xFF);
        temp_v0 = (Objects0831CCommand *)D_800C94B4;
        D_800C94B4 = (s32)(temp_v0 + 1);
        temp_v0->unk4 = -0x100;
        temp_v0->unk0 = 0xFB000000;
    }
    if (arg5 != 0) {
        sp24 = 1;
    }
    if (arg10 < 0xFF) {
        arg6 |= 4;
    }
    func_800349A4((FxGfx **)&D_800C94B4, arg5, arg6, arg7);
    temp_v0 = (Objects0831CCommand *)D_800C94B4;
    temp_a2 = (s32)arg1 + 0x80000000;
    D_800C94B4 = (s32)(temp_v0 + 1);
    temp_a1 = arg2 * 8;
    temp_v0->unk0 = ((((temp_a1 | (temp_a2 & 6)) & 0xFF) << 16) |
                     0x04000000 | (((arg2 * 0xA) + 8) & 0xFFFF));
    temp_v0->unk4 = temp_a2;
    temp_v0 = (Objects0831CCommand *)D_800C94B4;
    D_800C94B4 = (s32)(temp_v0 + 1);
    temp_v0->unk0 = (((((arg4 - 1) * 0x10) | sp24) & 0xFF) << 16) |
                     0x05000000 | ((arg4 * 0x10) & 0xFFFF);
    temp_v0->unk4 = (s32)arg3 + 0x80000000;
    camPopModelMtx((Gfx **)&D_800C94B4);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000831C.s")
#endif
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects084C4Point;

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Objects084C4Vertex;

typedef union {
    struct {
        unsigned int w0;
        unsigned int w1;
    } words;
    unsigned long long force_alignment;
} Objects084C4Gfx;

#ifdef NON_MATCHING
void func_800084C4(Objects084C4Gfx **arg0, Objects084C4Vertex **arg1,
                   s32 arg2, s32 arg3, Objects084C4Point *arg4,
                   Objects084C4Point *arg5, f32 arg6, s32 arg7, s32 arg8,
                   u32 arg9) {
    f32 pointBX;
    f32 pointBY;
    f32 pointBZ;
    f32 pointAX;
    f32 pointAY;
    f32 pointAZ;
    f32 clippedBX;
    f32 clippedBY;
    f32 clippedBZ;
    f32 clippedAX;
    f32 clippedAY;
    f32 clippedAZ;
    f32 projectedBX;
    f32 projectedBY;
    f32 projectedAX;
    f32 projectedAY;
    f32 deltaLengthSquared;
    f32 deltaX;
    f32 deltaY;
    void *rotationMatrix;
    Objects084C4Gfx *displayList;
    Objects084C4Vertex *vertices;

    if ((arg5->x == arg4->x) && (arg5->y == arg4->y) &&
        (arg5->z == arg4->z)) {
        return;
    }
    rotationMatrix = camGetRotationMtx();
    mtxf_transform_point(rotationMatrix, arg5->x, arg5->y, arg5->z,
                         &pointBX, &pointBY, &pointBZ);
    mtxf_transform_point(rotationMatrix, arg4->x, arg4->y, arg4->z,
                         &pointAX, &pointAY, &pointAZ);
    if ((pointBZ > -10.0f) && (pointAZ > -10.0f)) {
        return;
    }
    clippedBX = pointBX;
    clippedBY = pointBY;
    clippedBZ = pointBZ;
    clippedAX = pointAX;
    clippedAY = pointAY;
    clippedAZ = pointAZ;
    if (pointBZ > -10.0f) {
        clippedBZ = -10.0f;
        deltaLengthSquared = (-10.0f - pointBZ) / (pointAZ - pointBZ);
        clippedBX += (pointAX - pointBX) * deltaLengthSquared;
        clippedBY += (pointAY - pointBY) * deltaLengthSquared;
    } else if (pointAZ > -10.0f) {
        clippedAZ = -10.0f;
        deltaLengthSquared = (-10.0f - pointAZ) / (pointBZ - pointAZ);
        clippedAX += (pointBX - pointAX) * deltaLengthSquared;
        clippedAY += (pointBY - pointAY) * deltaLengthSquared;
    }
    displayList = *arg0;
    vertices = *arg1;
    if (func_800246B0(clippedBX, clippedBY, clippedBZ,
                      &projectedBX, &projectedBY, 0) != 0) {
        if (func_800246B0(clippedAX, clippedAY, clippedAZ,
                          &projectedAX, &projectedAY, 0) != 0) {
            deltaX = projectedAX - projectedBX;
            deltaY = projectedAY - projectedBY;
            deltaLengthSquared = (deltaX * deltaX) + (deltaY * deltaY);
            if (deltaLengthSquared > 0.0f) {
                deltaLengthSquared = arg6 / sqrtf(deltaLengthSquared);
                deltaX *= deltaLengthSquared;
                deltaY *= deltaLengthSquared;
            }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0x01010040; command->words.w1 = (unsigned int)camGetProjOrgMtx() - 0x80000000U; }
            func_800349A4((FxGfx **)&displayList, arg2, arg9 | 6, 0);
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xFA000000; command->words.w1 = arg7; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xFB000000; command->words.w1 = arg8; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0x04000000U | (((unsigned int)((((unsigned int)vertices - 0x80000000U) & 6U) | 0x20U) & 0xFFU) << 16) | 0x30U; command->words.w1 = ((unsigned int)vertices - 0x80000000U); }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0x05110020; command->words.w1 = (unsigned int)arg3 - 0x80000000U; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xE7000000; command->words.w1 = 0; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xFA000000; command->words.w1 = (u32)-1; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xFB000000; command->words.w1 = (u32)-1; }
            { Objects084C4Gfx *command = displayList++; command->words.w0 = 0xBC00000A; command->words.w1 = 0; }
            vertices[0].x = (s16)(pointBX + deltaY);
            vertices[0].y = (s16)(pointBY + deltaX);
            vertices[0].z = (s16)pointBZ;
            vertices[0].r = 0xFF;
            vertices[0].g = 0xFF;
            vertices[0].b = 0xFF;
            vertices[0].a = 0xFF;
            vertices[1].x = (s16)(pointBX - deltaY);
            vertices[1].y = (s16)(pointBY - deltaX);
            vertices[1].z = (s16)pointBZ;
            vertices[1].r = 0xFF;
            vertices[1].g = 0xFF;
            vertices[1].b = 0xFF;
            vertices[1].a = 0xFF;
            vertices[2].x = (s16)(pointAX + deltaY);
            vertices[2].y = (s16)(pointAY + deltaX);
            vertices[2].z = (s16)pointAZ;
            vertices[2].r = 0xFF;
            vertices[2].g = 0xFF;
            vertices[2].b = 0xFF;
            vertices[2].a = 0xFF;
            vertices[3].x = (s16)(pointAX - deltaY);
            vertices[3].y = (s16)(pointAY - deltaX);
            vertices[3].z = (s16)pointAZ;
            vertices[3].r = 0xFF;
            vertices[3].g = 0xFF;
            vertices[3].b = 0xFF;
            vertices[3].a = 0xFF;
            vertices += 4;
        }
    }
    *arg0 = displayList;
    *arg1 = vertices;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_800084C4.s")
#endif
void func_80008A20(Objects08A20Arg *arg0) {
    func_8000831C(arg0, D_80079008, 0x14, D_800790D0, 0x18, *arg0->unk68, 2, 0, 1.0f, 0xFF, 0xFF);
}
void func_80008A8C(Objects08A20Arg *arg0) {
    switch (arg0->unk44) {
        case 0x3D:
            if (arg0->unk88 != NULL) {
                TrapDanglingJump(arg0);
            }
            TrapDanglingJump(&D_800C94B4, arg0);
            break;
        case 0x24:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
        case 0x3B:
            func_80008A20(arg0);
            break;
        case 0x41:
            TrapDanglingJump(arg0, &D_800C94B4, &D_800C94B8, &D_800C94BC);
            break;
        case 0x55:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
        case 0x5B:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
    }
}

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
    u8 pad1F[0x11];
    u8 unk30;
} Objects08B94Data;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
} Objects08B94Resource;

typedef struct {
    f32 unk0;
    u8 pad04;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} Objects08B94Multiplier;

typedef struct {
    u8 pad00[0xD];
    u8 unkD;
} Objects08B94Palette;

typedef struct {
    u8 pad00[8];
    u8 r;
    u8 g;
    u8 b;
    u8 pad0B[5];
} Objects08B94Colour;

typedef struct {
    u8 pad00[0x64];
    u8 *unk64;
} Objects08B94Camera;

typedef struct {
    u8 pad00[4];
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 pad0A[0xA];
    Objects08B94Camera *unk14;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
} Objects08B94Info;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x21];
    u8 unk39;
    s8 unk3A;
    u8 pad3B;
    Objects08B94Palette *unk3C;
    Objects08B94Data *unk40;
    s16 unk44;
    u8 pad46[0xA];
    Objects08B94Multiplier *unk50;
    u8 pad54[0x10];
    Objects08B94Info *unk64;
    Objects08B94Resource **unk68;
} Objects08B94Object;

/* Matched from 111 words with six edits, none of them declaration order or
 * statement placement (the function issues calls, so it is p1-only): the
 * multiplier is defined by re-reading object->unk50 rather than by a copy, so
 * its arm use counts as an occurrence and its p1 save (6/7) outranks alpha's
 * (10/12), which is what puts it in t3 and alpha in t4; the flags OR-assign
 * precedes the useColourState store; the arm-5 pointer has its own local
 * (reusing cameraData imports its interference); the multiplier word carries
 * _SHIFTL-style byte masks, which fold away and set the ugen ring phase; each
 * two-word command is written unk0 then unk4 on one physical line, the shape
 * of the gDPSetPrimColor/gDPSetEnvColor expansion; and paletteIndex is s32 so
 * its web ties the a0 webs at save 3.0 and wins on web number. 0.0025f is a
 * literal because it is this function's own rodata slot in the ROM.
 */
void func_80008B94(void *arg0) {
    s32 flags;
    s32 colourState;
    s32 alpha;
    s32 red;
    s32 green;
    s32 blue;
    s32 computedAlpha;
    s32 extraGreen;
    s32 extraBlue;
    s32 useColourState;
    s32 useMultiplier;
    s32 specialColour;
    Objects08B94Resource *resource;
    Objects08B94Multiplier *multiplier;
    f32 savedScale;
    Objects08B94Object *object;
    s8 *cameraData;
    Objects08B94Colour *colourA;
    Objects08B94Colour *colourB;
    Objects08B94Palette *palette;
    s32 white;
    Objects08B94Info *info;
    s8 *cameraData2;

    object = (Objects08B94Object *)arg0;
    specialColour = 0;
    if (((Objects08B94Object *)arg0)->unk44 == 0x45) {
        TrapDanglingJump(&D_800C94B4, &D_800C94B8, &D_800C94BC, object);
        return;
    }

    flags = object->unk6 & 6;
    if (object->unk40->unk30 == 0) {
        flags |= 8;
    }
    if (D_8007C854 != 0) {
        colourState = D_8007C85C;
        useColourState = colourState != 0xFF;
    } else {
        colourState = 0xFF;
        useColourState = 0;
    }
    useMultiplier = 0;
    if (object->unk8 < 0.0f) {
        flags |= 0x8000;
        object->unk8 = -object->unk8;
    }

    if (object->unk50 != NULL) {
        multiplier = object->unk50;
        colourState = (s32)((f32)colourState * multiplier->unk0);
        useColourState = 1;
        useMultiplier = 1;
    }
    alpha = object->unk39;
    if (alpha >= 0x100) {
        alpha = 0xFF;
    }
    if (object->unk44 == 0x50) {
        info = object->unk64;
        cameraData = (s8 *)info->unk14->unk64;
        if (camGetNo() == (s32)cameraData[0]) {
            alpha >>= 1;
        }
    }
    if (alpha < 0xFF) {
        flags |= 4;
        useColourState = 1;
    }

    resource = object->unk68[object->unk3A];
    if (object->unk44 == 0x44) {
        info = object->unk64;
        func_80034DF0(info->unk4, info->unk5,
                      info->unk6, info->unk7,
                      info->unk8, info->unk9);
        specialColour = 1;
    } else if (object->unk44 == 0x3C) {
        info = object->unk64;
        func_80034DF0(0xFF, 0xFF, 0xFF, info->unk18,
                      info->unk19, info->unk1A);
        specialColour = 1;
    } else if (object->unk44 == 0x20 && (palette = object->unk3C) != NULL &&
               (palette->unkD & 0x80) != 0) {
        Objects08B94Colour *colours;
        s32 paletteIndex;

        colours = (Objects08B94Colour *)levelGetColourCycling();
        colourA = colours;
        palette = object->unk3C;
        if (palette != NULL) {
            paletteIndex = palette->unkD;
            colourA = colours + (paletteIndex & 7);
            colourB = colours + ((paletteIndex >> 3) & 7);
        } else {
            colourB = colours;
        }
        func_80034DF0(colourA->r, colourA->g, colourA->b,
                      colourB->r, colourB->g, colourB->b);
        specialColour = 1;
    } else if (object->unk50 != NULL &&
               (resource->unk6 & 0x200) != 0) {
        red = ((s32)resource->unk8 * colourState) >> 8;
        green = ((s32)resource->unk9 * colourState) >> 8;
        blue = ((s32)resource->unkA * colourState) >> 8;
        computedAlpha = (((s32)resource->unkB * multiplier->unk5) * colourState) >> 16;
        extraGreen = (((s32)resource->unkC * multiplier->unk6) * colourState) >> 16;
        extraBlue = (((s32)resource->unkD * multiplier->unk7) * colourState) >> 16;
        func_80034DF0((u8)red, (u8)green, (u8)blue,
                      (u8)computedAlpha, extraGreen, extraBlue);
        specialColour = 1;
    } else if (object->unk44 == 0x50) {
        info = object->unk64;
        cameraData2 = (s8 *)info->unk14->unk64;
        TrapDanglingJump((s32)cameraData2[1], &computedAlpha,
                         &extraGreen, &extraBlue);
        white = 0xFF;
        func_80034DF0(white, white, white, (u8)computedAlpha,
                      extraGreen, extraBlue);
        specialColour = 1;
    } else {
        if (useColourState || alpha < 0xFF) {
            Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
            D_800C94B4 += 8;
            command->unk0 = 0xFA000000; command->unk4 = ((colourState & 0xFF) << 24) | ((colourState & 0xFF) << 16) | ((colourState & 0xFF) << 8) | (alpha & 0xFF);
        } else {
            Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
            D_800C94B4 += 8;
            command->unk0 = 0xFA000000; command->unk4 = (u32)-1;
        }
        if (useMultiplier) {
            Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
            D_800C94B4 += 8;
            command->unk0 = 0xFB000000;
            command->unk4 = (((u32)multiplier->unk5 & 0xFF) << 24) |
                            (((u32)multiplier->unk6 & 0xFF) << 16) |
                            (((u32)multiplier->unk7 & 0xFF) << 8);
        } else {
            Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
            D_800C94B4 += 8;
            command->unk0 = 0xFB000000; command->unk4 = (u32)-0x100;
        }
    }

    savedScale = object->unk8;
    if (object->unk44 == 0x50) {
        object->unk8 = 1.5f -
                       (camGetProjZ(object->unkC, object->unk10, object->unk14) *
                        0.0025f);
    }
    if (object->unk40->unk1E == 0) {
        func_80023598((void **)&D_800C94B4, &D_800C94B8, &D_800C94BC,
                      object, resource, alpha);
    } else {
        func_80023A08((void **)&D_800C94B4, (s32)&D_800C94B8,
                      (s32)&D_800C94BC,
                      (s16 *)object, resource, flags, alpha);
    }
    if (specialColour) {
        func_80034E48();
    }
    if (useColourState) {
        Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
        D_800C94B4 += 8;
        command->unk0 = 0xFA000000; command->unk4 = (u32)-1;
    }
    if (useMultiplier) {
        Objects0831CCommand *command = (Objects0831CCommand *)D_800C94B4;
        D_800C94B4 += 8;
        command->unk0 = 0xFB000000; command->unk4 = (u32)-0x100;
    }
    object->unk8 = savedScale;
}
/* The sprite renderer reads the transform and frame from this segment layout. */
void func_80009220(void **arg0, s32 arg1, s32 arg2, Objects09220Object *arg3,
                   s32 arg4, Objects09220Source *arg5, s32 arg6) {
    f32 distance;
    f32 direction[3];
    struct {
        s16 yRotation;
        s16 xRotation;
        s16 zRotation;
        u8 pad06[2];
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
        u8 pad18[0x10];
        f32 frame;
    } segment;
    Objects09220Matrix *matrix;
    Objects09220Gfx *command;

    if (arg3->unk40->unkA6 > 0) {
        direction[2] = -1.0f;
        mathOneFloatPY(arg3, direction);
        matrix = camGetRotationMtx();
        distance = ((matrix->unk8 * direction[0]) +
                    (matrix->unk18 * direction[1])) +
                   (direction[2] * matrix->unk28);
        if (distance > 0.0f) {
            distance = sqrtf(distance);
            segment.yRotation = 0;
            segment.xRotation = 0;
            segment.zRotation = 0;
            segment.frame = 0.0f;
            segment.scale = arg3->unk8 * distance * (f32)arg6 * 0.0225f;
            mtxf_transform_point((void *)arg4, (f32)arg5->unk26,
                                 (f32)arg5->unk28, (f32)arg5->unk2A,
                                 &segment.x, &segment.y, &segment.z);
            arg6 = (s32)(distance * 320.0f);
            if (arg6 >= 0x100) {
                arg6 = 0xFF;
            }
            func_80034DF0(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0);
            func_80023A08(arg0, arg1, arg2, (s16 *)&segment,
                          *arg3->unk40->unkAC, 4, arg6);
            func_80034E48();
            command = (*(Objects09220Gfx **)arg0)++;
            command->w0 = 0xFB000000; command->w1 = -0x100;
        }
    }
}
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects09414Vector;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x34];
    Objects09414Vector *unk40;
    u8 pad44[0xC];
    f32 unk50;
    u8 pad54[0x104];
    s16 unk158;
} Objects09414Resource;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    s8 unk1E[4];
} Objects09414Data;

typedef struct {
    void *unk0;
    s8 unk4;
    u8 pad05[3];
    f32 unk8;
    u8 pad0C[8];
} Objects09414Entry;

typedef struct {
    u8 unk0;
    u8 pad01;
    s8 unk2;
    s8 unk3;
    u8 unk4;
    u8 pad05[3];
    void *unk8;
} Objects09414StaticEntry;

typedef struct {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    void *spriteData;
} Objects09414Sprite;

typedef struct {
    u32 w0;
    u32 w1;
} Objects09414Gfx;

typedef struct {
    u8 pad00[0x39];
    u8 unk39;
    u8 pad3A[6];
    Objects09414Data *unk40;
    u8 pad44[0xC];
    f32 *unk50;
    u8 pad54[0xC];
    Objects09414Entry *unk60;
    Objects09414Resource *unk64;
    Objects09414Resource **unk68;
    u8 pad6C[0x20];
    u8 unk8C;
    u8 pad8D[6];
    u8 unk93;
} Objects09414Object;

extern void func_80022E80(void *transform);
extern void func_80022FD4(void **displayList, s32 matrices, s32 vertices,
                          void *transform, f32 *opacity,
                          Objects09414Sprite *sprite, s32 flags, s32 alpha);
extern void func_80047CD8(void **displayList, void *cone, s32 flags, u8 alpha);
extern f32 func_80009F08(Objects09F08Arg *arg0);

#ifdef NON_MATCHING
void func_80009414(void **arg0, s32 arg1, s32 arg2, void *arg3) {
    s32 i;
    s32 j;
    s32 count;
    s32 type;
    s32 base;
    s32 swap;
    s32 mode;
    s16 sortIndex[8];
    s16 alphas[8];
    s16 kindOrEntry[8];
    f32 inverseScale;
    Objects09414Sprite sprite;
    u8 *textures[8];
    f32 depths[8];
    void *cone;
    void *cones[8];
    Objects09414Resource *root;
    Objects09414Data *data;
    Objects09414Resource *resource;
    Objects09414Object *object;
    Objects09414Gfx *command;
    Objects09414Entry *entries[8];
    Objects09414StaticEntry *staticEntry;
    Objects09414Entry *entry;
    Objects09414Vector *vector;

    object = (Objects09414Object *)arg3;
    resource = object->unk64;
    root = *object->unk68;

    command = (Objects09414Gfx *)*arg0;
    *arg0 = (void *)(command + 1);
    command->w1 = 0;
    command->w0 = 0xE7000000;
    command = (Objects09414Gfx *)*arg0;
    *arg0 = (void *)(command + 1);
    command->w1 = (u32)-0x100;
    command->w0 = 0xFB000000;

    for (i = 0; i < 4; i++) {
        if (*(s32 *)((u8 *)resource + 0x134 + (i * 4)) != 0) {
            TrapDanglingJump(arg0, *(s32 *)((u8 *)resource + 0x134 + (i * 4)));
        }
    }

    data = object->unk40;
    if (data->unk1E[object->unk93] == 0) {
        count = 0;
        for (i = 0; i < 4; i++) {
            staticEntry = (Objects09414StaticEntry *)
                ((u8 *)resource + 0x34C + (i * 0xC));
            if ((staticEntry->unk4 != 0) && (staticEntry->unk8 != NULL)) {
                cone = staticEntry->unk8;
                vector = &root->unk40[staticEntry->unk2];
                depths[count] = camGetProjZ(vector->x, vector->y, vector->z);
                cones[count] = cone;
                base = *(s32 *)((u8 *)root + (root->unkA * 4) + 0xC);
                textures[count] = (u8 *)base + (staticEntry->unk3 << 6);
                alphas[count] = staticEntry->unk4;
                kindOrEntry[count] = staticEntry->unk0 | 0x80;
                sortIndex[count] = count;
                count += 1;
            }
        }

        if ((resource->unk158 == 0) && (object->unk60 != NULL)) {
            entry = object->unk60;
            for (i = 0; (i < object->unk8C) && (i != 4); i++, entry++) {
                vector = &root->unk40[entry->unk4];
                depths[count] = camGetProjZ(vector->x, vector->y, vector->z);
                entries[count] = entry;
                kindOrEntry[count] = i;
                sortIndex[count] = count;
                count += 1;
            }
        }

        if (count > 0) {
            for (i = count - 1; i > 0; i--) {
                for (j = 0; j < i; j++) {
                    if (depths[sortIndex[j + 1]] < depths[sortIndex[j]]) {
                        swap = sortIndex[j];
                        sortIndex[j] = sortIndex[j + 1];
                        sortIndex[j + 1] = swap;
                    }
                }
            }

            func_80022E80((void *)((u8 *)resource + 0x43C));
            inverseScale = func_80009F08((Objects09F08Arg *)object) / object->unk40->unk0;
            sprite.divisor = 3;
            sprite.frameCount = 0x3333;
            for (i = 0; i < count; i++) {
                j = sortIndex[i];
                type = kindOrEntry[j];
                if ((type & 0x80) != 0) {
                    type &= 0x7F;
                    if (type == 0) {
                        mode = 0x206;
                    } else if (type == 1) {
                        mode = 6;
                    } else if (type == 2) {
                        mode = 0x16;
                        func_80009220(arg0, arg1, arg2,
                                      (Objects09220Object *)object,
                                      (s32)textures[j],
                                      (Objects09220Source *)cones[j],
                                      alphas[j]);
                    } else {
                        /* Baseline placeholder: default-mode lifetime remains unproved. */
                    mode = 0x3333;
                    }
                    command = (Objects09414Gfx *)*arg0;
                    *arg0 = (void *)(command + 1);
                    command->w0 = 0x01810040;
                    command->w1 = (u32)textures[j] + 0x80000000;
                    func_80047CD8(arg0, cones[j], mode,
                                  (u8)alphas[j]);
                    command = (Objects09414Gfx *)*arg0;
                    *arg0 = (void *)(command + 1);
                    command->w0 = 0xBC00000A;
                    command->w1 = 0;
                } else {
                    entry = entries[j];
                    sprite.angle = *(s16 *)((u8 *)resource + 0x10C +
                                             (type * 2));
                    sprite.frame = *(s16 *)((u8 *)resource + 0x114 +
                                             (type * 2));
                    sprite.transformScale = entry->unk8 * inverseScale;
                    sprite.matrixScale = resource->unk50;
                    vector = &root->unk40[entry->unk4];
                    sprite.x = vector->x;
                    sprite.y = *(f32 *)((u8 *)resource + 0x11C +
                                        (type * 4)) + vector->y;
                    sprite.z = vector->z;
                    sprite.spriteData = entry->unk0;
                    func_80022FD4(arg0, arg1, arg2,
                                  (void *)((u8 *)resource + 0x43C), object->unk50,
                                  &sprite, 0xE, object->unk39);
                }
            }
        }

        if (*(s32 *)((u8 *)resource + 0xD0) != 0) {
            TrapDanglingJump(arg0, arg1, *(s32 *)((u8 *)resource + 0xD0));
        }
        if (*(s32 *)((u8 *)resource + 0xD4) != 0) {
            TrapDanglingJump(arg0, arg1, arg2, *(s32 *)((u8 *)resource + 0xD4));
        }
        if (*(s32 *)((u8 *)resource + 0xD8) != 0) {
            TrapDanglingJump(arg0, arg1, *(s32 *)((u8 *)resource + 0xD8));
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80009414.s")
#endif
typedef struct {
    u8 pad00[0xD4];
    f32 unkD4;
} Objects09AA8Data;

typedef struct {
    u8 pad00[0x11];
    u8 unk11;
    u8 pad12[0x3C];
    s8 unk4E;
    u8 pad4F[0x21];
    s32 unk70;
    u8 pad74[4];
    u8 *unk78;
} Objects09AA8Material;

typedef struct {
    void *unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    u8 pad10[0x40];
    s32 unk50;
} Objects09AA8Entry;

typedef struct {
    void *unk0;
    u8 pad04[0x64];
    s32 unk68;
    s32 unk6C;
} Objects09AA8Root;

struct Objects09AA8Object {
    u8 pad00[0x39];
    u8 unk39;
    s8 unk3A;
    u8 pad3B[5];
    Objects09AA8Data *unk40;
    u8 pad44[0xC];
    s32 unk50;
    u8 pad54[0x14];
    Objects09AA8Entry **unk68;
    u8 pad6C[0x27];
    u8 unk93;
};

typedef union {
    struct {
        unsigned int w0;
        unsigned int w1;
    } words;
    unsigned long long force_alignment;
} Objects09AA8Command;

extern u8 D_78F28[];

void func_80009AA8(Objects09AA8Object *object) {
    s32 var_v0;
    Objects09AA8Entry **temp_v1;
    Objects09AA8Entry *temp_a1;
    Objects09AA8Entry *var_s2;
    Objects09AA8Material *temp_s1;
    u32 var_t3;
    void *temp_s0;
    s32 var_a0;
    s32 var_a1;
    s32 sp38;
    s32 var_t2;
    Objects09AA8Command *command;

    var_v0 = object->unk40->unkD4 != 0.0f;
    /* One definition, not a zero-then-one pair. uopt counts the pair as two
     * web occurrences, which halves this web's priority and drops it behind
     * the list and selected-entry webs; the single definition keeps it ahead
     * of them, and the selected entry then takes a1 instead of a0. */
    temp_v1 = object->unk68;
    temp_a1 = temp_v1[(s32)object->unk3A];
    if (var_v0 != 0) {
        var_s2 = temp_v1[0];
        sp38 = object->unk93;
    } else {
        var_s2 = temp_v1[(s32)object->unk3A];
        sp38 = 0;
    }
    /* Declared after the selection: initialized at the top, the spill IDO
       emits in the func_8005AF14 delay slot carries a register instead of
       the target's `sw zero`. */
    var_t2 = 0;
    temp_s1 = var_s2->unk0;
    temp_s0 = (Objects09AA8Root *)temp_a1->unk0;
    if (var_s2->unk8 != 0) {
        if (temp_s1->unk4E != 0) {

            func_8005AF14(var_s2, temp_s1, object);
        } else if (temp_s1->unk11 != 0) {
            var_s2->unkA = (s16)(var_s2->unkA ^ 1);
        }

        func_80019AB8(object, var_s2, object->unk50,
                      ((Objects09AA8Entry *)((u8 *)var_s2 +
                                             (var_s2->unkA * 4)))->unkC);
        if (temp_s1->unk11 != 0) {

            func_80007C68((Objects07C68Object *)object,
                          (Objects07C68Source *)temp_s1,
                          (Objects07C68Object *)var_s2, var_s2->unk8);
        }
        var_s2->unk8 = 0;
    }
    if (temp_s1->unk11 != 0) {
        var_t2 = 1;
    }
    if ((sp38 != 0) && (temp_a1->unk8 != 0)) {

        func_80019AB8(object, temp_a1, object->unk50,
                      ((Objects09AA8Entry *)((u8 *)var_s2 +
                                             (var_s2->unkA * 4)))->unkC);
        temp_a1->unk8 = 0;
    }
    if (object->unk39 == 0xFF) {
        var_t3 = ((Objects09AA8Root *)temp_s0)->unk68;
    } else {
        var_t3 = ((Objects09AA8Root *)temp_s0)->unk6C;
    }

    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0xFA000000; command->words.w1 = ((255U << 24) | (255U << 16) | (255U << 8) | ((unsigned int)object->unk39 & 255U));
    if (temp_s1->unk4E == 0) {

        camPushModelMtx((Gfx **)&D_800C94B4, (Mtx **)&D_800C94B8,
                        (CameraScaledTransform *)object, 1.0f, 0.0f);
    }
    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = (u32)((((u32)(((Objects09AA8Entry *)((u8 *)var_s2 + (var_s2->unkA * 4)))->unkC + 0x80000000) & 0xFFFFFF)) | 0xBF000000); command->words.w1 = (u32)(temp_a1->unk4 + 0x80000000);
    if (var_t2 != 0) {
        if (sp38 != 0) {
            command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0x02000050; command->words.w1 = (u32)D_78F28;
        } else {
            command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0x02000050; command->words.w1 = (u32)(((Objects09AA8Entry *)((u8 *)var_s2 + (var_s2->unkA * 4)))->unk50 + 0x80000000);
        }
    }
    if (temp_s1->unk70 != 0) {
        var_a0 = 0;
        if ((temp_s1->unk70 + 1) > 0) {
            var_a1 = 0;
            do {
                if (var_t2 != 0) {
                    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0x02000050; command->words.w1 = (u32)D_78F28;
                }
                command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; var_a0 += 1; command->words.w0 = 0x06000000; command->words.w1 = (u32)(*(s32 *)((u8 *)temp_s1->unk78 + var_a1) + 0x80000000);
                var_a1 += 4;
            } while (temp_s1->unk70 >= var_a0);
        }
    } else {
        command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0x06000000; command->words.w1 = (u32)(var_t3 + 0x80000000);
    }
    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0xBF000000; command->words.w1 = 0;
    camRestoreModelMtx((Gfx **)&D_800C94B4);
    func_80034920((Gfx **)&D_800C94B4);
    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0xFA000000; command->words.w1 = (u32)-1;
    command = (Objects09AA8Command *)D_800C94B4; D_800C94B4 += 8; command->words.w0 = 0xFB000000; command->words.w1 = (u32)-0x100;
    D_80079250 = 0;
}
void func_80009E78(Gfx **displayList, Mtx **matrix, TrackVertex **vertices,
                   TrackSkyObject *object) {
    if ((object->flags & 0xC00) == 0) {
        D_800C94B4 = (s32) *displayList;
        D_800C94B8 = (s32) *matrix;
        D_800C94BC = (s32) *vertices;
        func_80009F74(object);
        *displayList = (Gfx *) D_800C94B4;
        *matrix = (Mtx *) D_800C94B8;
        *vertices = (TrackVertex *) D_800C94BC;
    }
}
f32 func_80009F08(Objects09F08Arg *arg0) {
    f32 temp_f0;
    f32 var_f2;

    var_f2 = 1.0f;
    if (D_8007BF0C == 0) {
        temp_f0 = arg0->unk30;
        if (temp_f0 > 250.0f) {
            var_f2 += (temp_f0 - 250.0f) * 0.00134f;
            if (var_f2 > 2.0f) {
                var_f2 = 2.0f;
            }
        }
    }
    return var_f2;
}
/* Mickey-only distance-tier selection and object-renderer dispatch. */
void func_80009F74(Objects09F74Object *object) {
    s32 variant;
    Objects09F74Data *data;
    s8 *racer;
    f32 projection;

    if (object->unk40->unkD4[0] == 0.0f) {
        object->unk93 = object->unk3A;
    } else {
        projection = -camGetProjZ(object->unkC, object->unk10, object->unk14);
        data = object->unk40;
        variant = 0;
        while ((variant < 3) && (variant < data->unk22) &&
               (data->unkD4[variant] != 0.0f) &&
               (data->unkD4[variant] < projection)) {
            variant++;
        }
        if ((object->unk44 == 1) && (D_8007BF0C != 0) &&
            (variant == 0) && ((s32)D_8007BEF8 >= 3)) {
            racer = object->unk64;
            if (camGetNo() != *racer) {
                variant = 1;
            }
        }
        object->unk93 = (s8)variant;
    }

    if ((object->unk44 == 1) || (object->unk44 == 0x3F)) {
        if (camGetPtr()->unk4E >= 2) {
            object->unk93 = 0;
        } else if (object->unk44 == 0x3F) {
            object->unk8 = func_80009F08((Objects09F08Arg *)object) *
                           object->unk40->unk0;
        } else {
            *(f32 *)((u8 *)object->unk64 + 0x444) =
                func_80009F08((Objects09F08Arg *)object) * object->unk8;
        }
    }

    data = object->unk40;
    if (data->unkD4[0] == 0.0f) {
        variant = data->unk1E[0];
        switch (variant) {
        case 0:
            func_80009AA8(object);
            return;
        case 1:
            func_80008B94(object);
            return;
        case 2:
            func_80008A8C((Objects08A20Arg *)object);
            return;
        }
    } else {
        variant = data->unk1E[object->unk93];
        switch (variant) {
        case 0:
            object->unk3A = object->unk93;
            func_80009AA8(object);
            object->unk3A = 0;
            return;
        case 1:
            object->unk3A = object->unk93;
            func_80008B94(object);
            object->unk3A = 0;
            return;
        case 2:
            object->unk3A = object->unk93;
            func_80008A8C((Objects08A20Arg *)object);
            return;
        }
    }
}
/* PROVENANCE: partition loop adapted from Diddy Kong Racing's public
 * src/objects.c get_first_active_object; Mickey's list and header offsets are authoritative. */
s32 func_8000A244(s32 *arg0) {
    s32 i;
    s32 j;
    s32 minIndex;
    s32 maxIndex;
    s32 breakLoop;
    Objects0A244Object *tempObject;

    *arg0 = D_800C9498;
    if (D_800C94B2 != 0) {
        return D_800C94B2;
    }
    i = D_800C949C;
    j = D_800C9498 - 1;
    minIndex = i;
    maxIndex = j;
    while (i <= j) {
        breakLoop = 0;
        while ((i <= maxIndex) && (breakLoop == 0)) {
            if (((Objects0A244Object **)D_800C9494)[i]->unk40->unk14 & 1) {
                i += 1;
            } else {
                breakLoop = -1;
            }
        }
        breakLoop = 0;
        while ((j >= minIndex) && (breakLoop == 0)) {
            if (!(((Objects0A244Object **)D_800C9494)[j]->unk40->unk14 & 1)) {
                j -= 1;
            } else {
                breakLoop = -1;
            }
        }
        if (i < j) {
            tempObject = ((Objects0A244Object **)D_800C9494)[i];
            ((Objects0A244Object **)D_800C9494)[i] = ((Objects0A244Object **)D_800C9494)[j];
            ((Objects0A244Object **)D_800C9494)[j] = tempObject;
            i += 1;
            j -= 1;
        }
    }
    D_800C94B2 = i;
    return i;
}
/* Lane lm-obj: plain counted walk `current = *objects++` over the depth
 * update. The hand-unrolled remainder-plus-4x body was +300 / 231 masked;
 * this is delta 0 / 154. Entry still colours arg0 instead of spilling it. */
#ifdef NON_MATCHING
void func_8000A39C(s32 arg0, s32 arg1) {
    s32 passCount;
    Objects0A39CObject *current;
    Objects0A39CObject *next;
    Objects0A39CMatrix *matrix;
    f32 matrixX;
    f32 matrixY;
    Objects0A39CObject **objects;
    s32 sortOffset;
    f32 matrixZ;
    f32 matrixW;
    f32 currentDepth;
    f32 nextDepth;
    s32 difference;
    s32 updateCount;
    s32 i;
    s32 sorted;

    difference = arg0;
    difference = arg1 - difference;
    if (difference > 0) {
        passCount = difference;
        matrix = (Objects0A39CMatrix *)camGetRotationMtx();
        matrixX = matrix->unk8;
        matrixY = matrix->unk18;
        matrixZ = matrix->unk28;
        matrixW = matrix->unk38;
        arg0 *= 4;
        objects = (Objects0A39CObject **)(arg0 + (u8 *)D_800C9494);

        difference += 1;
        sortOffset = arg0;
        for (i = 0; i < difference; i++) {
            current = *objects++;
            if (current != NULL) {
                current->unk30 = -((current->unkC * matrixX) +
                                  (current->unk10 * matrixY) +
                                  (current->unk14 * matrixZ) + matrixW);
            }
        }

        do {
            objects = (Objects0A39CObject **)(sortOffset + (u8 *)D_800C9494);
            updateCount = passCount;
            passCount -= 1;
            current = objects[0];
            sorted = 1;
            currentDepth = current->unk30;
            if (current->unk6 & 0x800) {
                currentDepth += 32768.0f;
            }
            while (updateCount--) {
                next = objects[1];
                nextDepth = next->unk30;
                if (next->unk6 & 0x800) {
                    nextDepth += 32768.0f;
                }
                if (nextDepth < currentDepth) {
                    *objects++ = next;
                    sorted = 0;
                } else {
                    *objects++ = current;
                    current = next;
                    currentDepth = nextDepth;
                }
            }
            *objects = current;
        } while ((passCount != 0) && (sorted == 0));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000A39C.s")
#endif
/* PROVENANCE: body adapted from Jet Force Gemini's public src/objects.c
 * setObjectViewNormal; Mickey's target globals and byte output are authoritative. */
void func_8000A62C(f32 x, f32 y, f32 z) {
    f32 vecLength = sqrtf((x * x) + (y * y) + (z * z));
    f32 normalizedLength;

    if (vecLength != 0.0f) {
        normalizedLength = -8192.0f / vecLength;
        x *= normalizedLength;
        y *= normalizedLength;
        z *= normalizedLength;
    }
    D_800C9508[0] = x;
    D_800C9508[1] = y;
    D_800C9508[2] = z;
}
void func_8000A6DC(s32 arg0) {
    D_800C94B0 = arg0;
}
s32 func_8000A6E8(s32 arg0) {
    s32 flags;

    switch (arg0) {
        case 1:
            flags = 0xF01;
            break;
        case 4:
            flags = 0x301;
            break;
        case 3:
            flags = 0xB01;
            break;
        case 31:
            flags = 0x1101;
            break;
        case 24:
            flags = 0x1301;
            break;
        case 25:
            flags = 0x1200;
            break;
        case 26:
            flags = 0xB01;
            break;
        case 22:
        case 29:
        case 73:
        case 79:
            flags = 0x301;
            break;
        case 23:
            flags = 0x200;
            break;
        case 11:
        case 12:
        case 49:
            flags = 0x200;
            break;
        case 33:
            flags = 0x301;
            break;
        case 41:
            flags = 0x1101;
            break;
        case 53:
            flags = 0x200;
            break;
        case 54:
            flags = 0x101;
            break;
        case 55:
            flags = 0x101;
            break;
        case 56:
            flags = 0x101;
            break;
        case 57:
            flags = 0xB01;
            break;
        case 58:
            flags = 0xA00;
            break;
        case 60:
            flags = 0x200;
            break;
        case 63:
            flags = 0x101;
            break;
        case 64:
            flags = 0xB01;
            break;
        case 65:
            flags = 0x301;
            break;
        case 67:
            flags = 0xB01;
            break;
        case 71:
            flags = 0x101;
            break;
        case 72:
            flags = 0x301;
            break;
        case 74:
            flags = 0x301;
            break;
        case 75:
            flags = 0x101;
            break;
        case 77:
            flags = 0x101;
            break;
        case 78:
            flags = 0x301;
            break;
        case 82:
            flags = 0xB01;
            break;
        case 84:
            flags = 0x301;
            break;
        case 85:
            flags = 0x200;
            break;
        case 86:
            flags = 0x101;
            break;
        case 87:
            flags = 0x301;
            break;
        case 88:
            flags = 0x301;
            break;
        default:
            flags = 0;
            break;
    }
    return flags;
}
/* Tier A: Mickey-only switch destinations, field widths, and linked bytes. */
s32 func_8000A830(Objects0A830Object *arg0, void *arg1) {
    s32 size;

    switch (arg0->unk40->unk1C - 1) {
        case 0: size = 0x460; break;
        case 5: size = 0x18; break;
        case 19: size = 0xC; break;
        case 20: size = 0x10; break;
        case 13: size = 0x30; break;
        case 72: size = 0x54; break;
        case 28: size = 0x54; break;
        case 27: size = 0x1C; break;
        case 35: size = 0xD0; break;
        case 40: size = 0x30; break;
        case 11: size = 0xE; break;
        case 41: size = 8; break;
        case 49: size = 0xCC; break;
        case 42: size = 2; break;
        case 43: size = 4; break;
        case 32: size = 0xC; break;
        case 45: size = 6; break;
        case 46: size = 0xC; break;
        case 47: size = 0xC; break;
        case 48: size = 0xC; break;
        case 52: size = 0xC; break;
        case 53: size = 0x50; break;
        case 54: size = 0x44; break;
        case 55: size = 0; break;
        case 56: size = 0xE4; break;
        case 57: size = 0xD8; break;
        case 58: size = 6; break;
        case 59: size = 0x20; break;
        case 62: size = 0x10; break;
        case 63: size = 0x40; break;
        case 64: size = 0x24; break;
        case 65: size = 0x198; break;
        case 66: size = 0x40; break;
        case 67: size = 0x24; break;
        case 68: size = 0x14; break;
        case 69: size = 0x14; break;
        case 70: size = 0x50; break;
        case 71: size = 0x38; break;
        case 73: size = 4; break;
        case 79: size = 0x18; break;
        case 74: size = 4; break;
        case 75: size = 0x1C; break;
        case 76: size = 4; break;
        case 77: size = 0x44; break;
        case 80: size = 0xC; break;
        case 81: size = 0x18; break;
        case 82: size = 8; break;
        case 83: size = 0x38; break;
        case 84: size = 0x9C; break;
        case 85: size = 0x28; break;
        case 86: size = 8; break;
        case 87: size = 8; break;
        case 88: size = 0x14; break;
        case 89: size = 8; break;
        case 90: size = 0x238; break;
        case 91: size = 0xC0; break;
        default: size = 0; break;
    }
    if (size != 0) {
        arg0->unk64 = arg1;
    }
    return (size & -4) + 4;
}
struct Objects0AA38Object {
    u8 pad00[0x40];
    Objects0A830Data *unk40;
    s16 unk44;
};

extern void func_8001C4C0(void *object, void *arg1, s32 mode);
extern void spranimInit(void *object, void *arg1);
extern void sprasjiInit(void *object, void *arg1);
extern void func_8001A154(void *object);
extern void func_8001BAE4(void *object, void *arg1);
extern void func_8001BAF8(void *object, void *arg1);

/* Tier B: Mickey's resident loader records identify each overlay export.
 * The dispatch ABI forwards object, entry, and preserveState. Each call
 * retains the resident trap that the runtime loader patches at that site. */
void func_8000AA38(Objects0AA38Object *arg0, void *arg1, s32 preserveState) {
    arg0->unk44 = arg0->unk40->unk1C;
    switch (arg0->unk44 - 1) {
        case 0:
            func_8001C4C0(arg0, arg1, 1);
            break;
        case 28:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 69 +0x0. */
            break;
        case 72:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 88 +0x0. */
            break;
        case 1:
        case 2:
        case 3:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 85 +0x0. */
            break;
        case 30:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x508. */
            break;
        case 31:
        case 33:
            spranimInit(arg0, arg1);
            break;
        case 58:
            sprasjiInit(arg0, arg1);
            break;
        case 19:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x36C. */
            break;
        case 5:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x0. */
            break;
        case 4:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x14C. */
            break;
        case 10:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x130. */
            break;
        case 11:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 92 +0x0. */
            break;
        case 13:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x748. */
            break;
        case 14:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x3F4. */
            break;
        case 40:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 89 +0x270. */
            break;
        case 34:
            func_8001A154(arg0);
            break;
        case 35:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 71 +0x0. */
            break;
        case 8:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x420. */
            break;
        case 41:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 83 +0x53C. */
            break;
        case 7:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 93 +0x0. */
            break;
        case 6:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x1A8. */
            break;
        case 27:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x944. */
            break;
        case 23:
        case 24:
            func_8001BAE4(arg0, arg1);
            break;
        case 25:
            func_8001BAF8(arg0, arg1);
            break;
        case 32:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 36 +0x0. */
            break;
        case 45:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 1 +0x6788. */
            break;
        case 46:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 1 +0x69A0. */
            break;
        case 47:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 1 +0x6B28. */
            break;
        case 48:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 4 +0x0. */
            break;
        case 49:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 84 +0x0. */
            break;
        case 50:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 84 +0xDBC. */
            break;
        case 20:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 97 +0x1E8. */
            break;
        case 42:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 2 +0x1DF8. */
            break;
        case 43:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 2 +0x2528. */
            break;
        case 44:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 1 +0x36A0. */
            break;
        case 52:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 72 +0x0. */
            break;
        case 53:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 90 +0x0. */
            break;
        case 54:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 86 +0x444. */
            break;
        case 56:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 26 +0x0. */
            break;
        case 57:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 29 +0x42C. */
            break;
        case 59:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 25 +0x0. */
            break;
        case 60:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 20 +0x204. */
            break;
        case 61:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 20 +0xF78. */
            break;
        case 62:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 68 +0x8E0. */
            break;
        case 63:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 22 +0x0. */
            break;
        case 64:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 23 +0x208. */
            break;
        case 65:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 28 +0x1B8. */
            break;
        case 66:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 75 +0x0. */
            break;
        case 67:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 27 +0x0. */
            break;
        case 68:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 24 +0x0. */
            break;
        case 69:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 21 +0x0. */
            break;
        case 70:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 91 +0x0. */
            break;
        case 71:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 87 +0x0. */
            break;
        case 73:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 74 +0x0. */
            break;
        case 79:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 70 +0x0. */
            break;
        case 74:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 76 +0x0. */
            break;
        case 75:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 77 +0x0. */
            break;
        case 76:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 78 +0x0. */
            break;
        case 77:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 79 +0x0. */
            break;
        case 80:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 80 +0x0. */
            break;
        case 81:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 81 +0x0. */
            break;
        case 82:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 81 +0x240. */
            break;
        case 83:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 82 +0x0. */
            break;
        case 84:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 73 +0x0. */
            break;
        case 85:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 94 +0x0. */
            break;
        case 86:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 79 +0x1280. */
            break;
        case 87:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 79 +0x147C. */
            break;
        case 88:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 37 +0x0. */
            break;
        case 89:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 95 +0x0. */
            break;
        case 90:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 38 +0x0. */
            break;
        case 91:
            TrapDanglingJump(arg0, arg1, preserveState); /* Overlay 96 +0xF8. */
            break;
        case 9:
            break;
        case 12:
            break;
        case 15:
            break;
        case 16:
            break;
        case 17:
            break;
        case 18:
            break;
        case 21:
            break;
        case 22:
            break;
        case 26:
            break;
        case 29:
            break;
        case 36:
            break;
        case 37:
            break;
        case 38:
            break;
        case 39:
            break;
        case 51:
            break;
        case 55:
            break;
        case 78:
            break;
    }
}
typedef struct {
    u8 pad00[0x40];
    u8 *unk40;
    s16 unk44;
    u8 pad46[0x32];
    void *unk78;
} Objects0AEECObject;

/* Tier B: Mickey resident control-type switch and runtime overlay call sites. */
void func_8000AEEC(void *arg0, s32 arg1) {
    Objects0AEECObject *object;

    D_8007A210 = 3;
    D_8007A21C = 1;
    D_8007A214 = arg0;
    object = (Objects0AEECObject *)arg0;
    D_8007A218 = object->unk40 + 4;
    switch (object->unk44 - 3) {
        case 0:
            TrapDanglingJump(object, arg1); /* Overlay 85 +0xC0. */
            break;
        case 31:
            spranimOnceControl(object, arg1);
            break;
        case 29:
            spranimControl(object, arg1);
            break;
        case 17:
            texscrollControl(object, arg1);
            break;
        case 3:
            effectboxControl(object, arg1);
            break;
        case 38:
            TrapDanglingJump(object, arg1); /* Overlay 89 +0x5A4. */
            break;
        case 33:
            TrapDanglingJump(object); /* Overlay 71 +0x278. */
            break;
        case 39:
            TrapDanglingJump(object, arg1); /* Overlay 83 +0x7DC. */
            break;
        case 5:
            TrapDanglingJump(object, arg1); /* Overlay 93 +0x1C. */
            break;
        case 4:
            func_800148E0(object);
            break;
        case 26:
            TrapDanglingJump(object, arg1); /* Overlay 69 +0x4C. */
            break;
        case 70:
            TrapDanglingJump(object, arg1); /* Overlay 88 +0x4C. */
            break;
        case 25:
            func_8001B798(object, arg1);
            break;
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
            func_8001BB04(object, arg1);
            break;
        case 76:
            func_8001BB10(object, arg1);
            break;
        case 30:
            TrapDanglingJump(object, arg1); /* Overlay 36 +0x1D0. */
            break;
        case 43:
            TrapDanglingJump(object, arg1); /* Overlay 1 +0x67C0. */
            break;
        case 44:
            TrapDanglingJump(object, arg1); /* Overlay 1 +0x6A14. */
            break;
        case 45:
            TrapDanglingJump(object, arg1); /* Overlay 1 +0x6B6C. */
            break;
        case 46:
            TrapDanglingJump(object, arg1); /* Overlay 4 +0x138. */
            break;
        case 47:
            TrapDanglingJump(object, arg1); /* Overlay 84 +0x48. */
            break;
        case 50:
            TrapDanglingJump(object, arg1); /* Overlay 72 +0xB4. */
            break;
        case 51:
            TrapDanglingJump(object, arg1); /* Overlay 90 +0xFC. */
            break;
        case 52:
            TrapDanglingJump(object, arg1); /* Overlay 86 +0x474. */
            break;
        case 54:
            TrapDanglingJump(object, arg1); /* Overlay 26 +0x1A0. */
            break;
        case 55:
            TrapDanglingJump(object, arg1); /* Overlay 29 +0x5C4. */
            break;
        case 57:
            TrapDanglingJump(object, arg1); /* Overlay 25 +0x17C. */
            break;
        case 58:
            TrapDanglingJump(object); /* Overlay 20 +0xE0C. */
            break;
        case 60:
            TrapDanglingJump(object, arg1); /* Overlay 68 +0x96C. */
            break;
        case 61:
            TrapDanglingJump(object, arg1); /* Overlay 22 +0x2B0. */
            break;
        case 62:
            TrapDanglingJump(object, arg1); /* Overlay 23 +0x350. */
            break;
        case 63:
            TrapDanglingJump(object, arg1); /* Overlay 28 +0x318. */
            break;
        case 64:
            TrapDanglingJump(object, arg1); /* Overlay 75 +0x214. */
            break;
        case 65:
            TrapDanglingJump(object, arg1); /* Overlay 27 +0x64. */
            break;
        case 66:
            TrapDanglingJump(object, arg1); /* Overlay 24 +0x1C. */
            break;
        case 68:
            TrapDanglingJump(object, arg1); /* Overlay 91 +0x4C. */
            break;
        case 69:
            TrapDanglingJump(object, arg1); /* Overlay 87 +0x128. */
            break;
        case 71:
            TrapDanglingJump(object, arg1); /* Overlay 74 +0xB8. */
            break;
        case 72:
            TrapDanglingJump(object, arg1); /* Overlay 76 +0x38. */
            break;
        case 73:
            TrapDanglingJump(object, arg1); /* Overlay 77 +0x130. */
            break;
        case 74:
            TrapDanglingJump(object, arg1); /* Overlay 78 +0x70. */
            break;
        case 75:
            TrapDanglingJump(object, arg1); /* Overlay 79 +0x134. */
            break;
        case 78:
            TrapDanglingJump(object, arg1); /* Overlay 80 +0x11C. */
            break;
        case 79:
            TrapDanglingJump(object, arg1); /* Overlay 81 +0xCC. */
            break;
        case 80:
            TrapDanglingJump(object, arg1); /* Overlay 81 +0x274. */
            break;
        case 77:
            TrapDanglingJump(object, arg1); /* Overlay 70 +0xD8. */
            break;
        case 81:
            TrapDanglingJump(object, arg1); /* Overlay 82 +0x40. */
            break;
        case 82:
            TrapDanglingJump(object, arg1); /* Overlay 73 +0x190. */
            break;
        case 83:
            TrapDanglingJump(object, arg1); /* Overlay 94 +0x110. */
            break;
        case 84:
            TrapDanglingJump(object, arg1); /* Overlay 79 +0x1290. */
            break;
        case 85:
            TrapDanglingJump(object, arg1); /* Overlay 79 +0x149C. */
            break;
        case 86:
            TrapDanglingJump(object, arg1); /* Overlay 37 +0x88. */
            break;
        case 15:
            rangetriggerControl(object, arg1);
            break;
        case 87:
            TrapDanglingJump(object, arg1); /* Overlay 95 +0xC. */
            break;
        case 88:
            TrapDanglingJump(object, arg1); /* Overlay 38 +0x154. */
            break;
        case 1:
        case 2:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 16:
        case 18:
        case 27:
        case 28:
        case 32:
        case 34:
        case 35:
        case 36:
        case 37:
        case 40:
        case 41:
        case 42:
        case 48:
        case 49:
        case 53:
        case 56:
        case 59:
        case 67:
            break;
        default:
            break;
    }
    if (object->unk78 != NULL) {
        func_8000B3CC(object, arg1);
    }
    D_8007A21C = 4;
}
#ifdef NON_MATCHING
void func_8000B3CC(void *arg0, s32 arg1) {
    Objects0B3CCObject *object;
    Objects0B3CCConfig *config;
    f32 start[3];
    f32 end[3];
    f32 acceleration;
    f32 damping;
    Objects0B3CCState *state;
    f32 savedY;
    f32 negativeDot;
    f32 bottom;
    f32 volume;
    f32 dot;
    f32 radius;
    f32 step;
    f32 speed;
    s32 bounced;
    u32 collision;
    f32 moveZ;
    s16 savedAngle2;
    s16 savedAngle4;
    f32 factor;

    object = (Objects0B3CCObject *)arg0;
    state = object->unk78;
    state->unk2 = 0;
    if ((state->flags & 2) != 0) {
        return;
    }

    step = (f32)arg1;
    config = object->unk40->unkE0;
    if ((state->flags & 1) != 0) {
        acceleration = -config->unk8;
        damping = config->unkC;
    } else {
        acceleration = -config->unk0;
        damping = config->unk4;
    }

    start[0] = object->unkC + object->unk40->unkE0->unk20;
    start[1] = object->unk10 + object->unk40->unkE0->unk24;
    start[2] = object->unk14 + object->unk40->unkE0->unk28;
    end[0] = start[0] + (object->unk1C * step) + state->unk1C;
    end[2] = start[2] + (object->unk24 * step) + state->unk20;
    end[1] = start[1] + (object->unk20 * step) +
            (0.5f * acceleration * step * step);
    radius = state->unk4;

    trackMakePolylist(1, (Objects0BB84Vec3 *)start, (Objects0BB84Vec3 *)end, &radius, 0x10000, 0);
    collision = (u32)func_80010900((Objects0BB84Vec3 *)start, (Objects0BB84Vec3 *)end, radius, (s32)object,
                                    (void *)func_8000BB84);
    if ((collision >> 30) != 0) {
        object->unk1C = 0.0f;
        object->unk20 = 0.0f;
        object->unk24 = 0.0f;
        state->flags |= 2;
        return;
    }

    volume = end[0] - start[0];
    savedY = end[1] - start[1];
    moveZ = end[2] - start[2];
    func_80008128((Objects08128Object *)object, volume, savedY, moveZ);
    object->unk20 += acceleration * step;
    volume = object->unk1C;
    savedY = object->unk20;
    moveZ = object->unk24;
    speed = sqrtf((volume * volume) + (savedY * savedY) + (moveZ * moveZ));
    state->unk18 = speed;

    bottom = end[1] - radius;
    if (((func_8001357C(object->unkC, object->unk14, &state->unk14,
                        0x10000, NULL) & 0x10000) != 0) &&
        (bottom < state->unk14)) {
        damping = config->unkC;
        if ((state->flags & 1) == 0) {
            state->flags |= 1;
            if ((config->unk1C != 0) && (state->unk24 == NULL)) {
                func_80002FE0(config->unk1C, object->unkC, object->unk10,
                              object->unk14, 4, &state->unk24);
                if (state->unk24 != NULL) {
                    volume = state->unk18 * D_8008152C;
                    if (volume > 1.0f) {
                        volume = 1.0f;
                    }
                    func_8000309C(state->unk24,
                                  (u8)(127.0f * volume));
                }
            }
            if (config->unk14 != 0) {
                savedY = object->unk10;
                savedAngle2 = object->unk2;
                savedAngle4 = object->unk4;
                object->unk2 = 0;
                object->unk4 = 0;
                object->unk10 = state->unk14;
                object->unk80 = config->unk14;
                partUpdateTriggers(object, arg1);
                object->unk80 = 0;
                object->unk10 = savedY;
                object->unk2 = savedAngle2;
                object->unk4 = savedAngle4;
            }
        }
    } else {
        state->flags &= ~1;
    }

    object->unk1C *= damping;
    object->unk20 *= damping;
    object->unk24 *= damping;
    volume = object->unk1C;
    savedY = object->unk20;
    moveZ = object->unk24;
    state->unk18 = sqrtf((volume * volume) + (savedY * savedY) + (moveZ * moveZ));

    if ((collision << 2) != 0) {
        state->unk2 = 1;
        bounced = 0;
        if ((config->unk10 == 0.0f) || ((speed = state->unk18) == 0.0f)) {
            object->unk1C = 0.0f;
            object->unk20 = 0.0f;
            object->unk24 = 0.0f;
            state->flags |= 2;
        } else {
            object->unk1C /= speed;
            object->unk20 /= state->unk18;
            object->unk24 /= state->unk18;
            state->unk18 *= config->unk10;
            dot = (state->unk8 * object->unk1C) +
                  (state->unkC * object->unk20) +
                  (state->unk10 * object->unk24);
            negativeDot = -dot;
            factor = negativeDot + negativeDot;
            object->unk1C = ((factor * state->unk8) + object->unk1C) *
                            state->unk18;
            object->unk20 = ((factor * state->unkC) + object->unk20) *
                            state->unk18;
            object->unk24 = ((factor * state->unk10) + object->unk24) *
                            state->unk18;
            volume = object->unk1C;
            savedY = object->unk20;
            moveZ = object->unk24;
            speed = sqrtf((volume * volume) + (savedY * savedY) + (moveZ * moveZ));
            if (speed < 1.0f) {
                if (state->unkC < D_80081530) {
                    object->unk1C = state->unk8;
                    object->unk20 = state->unkC;
                    object->unk24 = state->unk10;
                } else {
                    object->unk1C = 0.0f;
                    object->unk20 = 0.0f;
                    object->unk24 = 0.0f;
                    state->unk18 = 0.0f;
                    state->flags |= 2;
                }
            } else {
                bounced = 1;
            }
        }
        if ((bounced != 0) && (config->unk1E != 0) &&
            ((state->flags & 1) == 0) && (state->unk24 == NULL)) {
            func_80002FE0(config->unk1E, object->unkC, object->unk10,
                          object->unk14, 4, &state->unk24);
            if (state->unk24 != NULL) {
                volume = state->unk18 * D_80081534;
                if (volume > 1.0f) {
                    volume = 1.0f;
                }
                func_8000309C(state->unk24,
                              (u8)(127.0f * volume));
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000B3CC.s")
#endif
/* Workbench verdict: structure-mismatch; 61 differing words (65/65). */
/* First mismatch: +0x0; target frame is 0x28, candidate frame is 0x30. */
/* Structural gap: FP register/stack allocation and argument homes differ. */
#ifdef NON_MATCHING
void func_8000BB84(s32 arg0, Objects0BB84Vec3 *arg1, Objects0BB84Vec3 *arg2,
                   f32 arg3, Objects0BB84Plane *arg4, Objects0BB84Object *arg5) {
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 inputX;
    f32 inputY;
    f32 inputZ;
    f32 dot;
    f32 factor;
    f32 reflectedX;
    f32 reflectedY;
    f32 reflectedZ;
    f32 scale;
    Objects0BB84Output *output;

    output = arg5->unk78;
    normalX = arg4->x;
    normalY = arg4->y;
    normalZ = arg4->z;
    inputX = arg2->x;
    inputY = arg2->y;
    inputZ = arg2->z;
    dot = (normalX * inputX) + (normalY * inputY) + (normalZ * inputZ);
    factor = 2.0f * -dot;
    reflectedX = inputX + (factor * normalX);
    reflectedY = inputY + (factor * normalY);
    reflectedZ = inputZ + (factor * normalZ);
    scale = (arg3 - arg4->unk1C) * arg5->unk40->unkE0->unk10;
    arg1->x = arg4->unk10 + (scale * reflectedX);
    arg1->y = arg4->unk14 + (scale * reflectedY);
    arg1->z = arg4->unk18 + (scale * reflectedZ);
    output->x = normalX;
    output->y = normalY;
    output->z = normalZ;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000BB84.s")
#endif
void GetRomlistInfo(s32 *romlist, s32 *size, s32 index) {
    *romlist = D_800C94C0[index];
    *size = D_800C94C8[index];
}
/* Workbench verdict: schedule-mismatch; 2 differing words. */
/* First mismatch: +0x3C. */
/* Shape-exact candidate; FP scheduling is reserved for the permuter. */
f32 func_8000BCB0(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5)
{
  f32 temp_f0;
  f32 temp_f16;
  f32 temp_f2;
  temp_f0 = arg0 - arg3;
  temp_f2 = arg1 - arg4;
  temp_f16 = arg2 - arg5;
  return sqrtf(((temp_f0 * temp_f0) + (temp_f2 * temp_f2)) + (temp_f16 * temp_f16));
}
/* Workbench verdict: schedule-mismatch; 2 differing words. */
/* First mismatch: +0x2C. */
/* Shape-exact candidate; instruction ordering is reserved for the permuter. */
f32 func_8000BD0C(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5)
{
  f32 temp_f16;
  f32 temp_f18;
  f32 temp_f2;
  temp_f2 = arg0 - arg3;
  temp_f16 = arg1 - arg4;
  temp_f18 = arg2 - arg5;
  return ((temp_f2 * temp_f2) + (temp_f16 * temp_f16)) + (temp_f18 * temp_f18);
}



/* PLATEAU-HANDOFF:func_80006EE4:start
 * symbol: func_80006EE4
 * score: 46/47 words
 * frame: 0x28
 * relocations: 7
 * first-mismatch: +0x8
 * summary: One target direct global-pointer load and allocator shape remain unresolved after bounded source attempts.
 * PLATEAU-HANDOFF:func_80006EE4:end
 */


/* PLATEAU-HANDOFF:func_80008028:start
 * symbol: func_80008028
 * score: 28/60 words
 * frame: 0x8
 * relocations: 8
 * first-mismatch: +0x24
 * summary: outer offset/interference schedule remains; indexed-loop and typed-offset families are closed
 * PLATEAU-HANDOFF:func_80008028:end
 */

/* PLATEAU-HANDOFF:func_8000BB84:start
 * symbol: func_8000BB84
 * score: 4/65 words
 * frame: 0x30
 * relocations: 0
 * first-mismatch: +0x0
 * summary: Arithmetic/control-flow size is exact, but the target's 0x28 frame and FP register/stack allocation remain unresolved.
 * PLATEAU-HANDOFF:func_8000BB84:end
 */


/* PLATEAU-HANDOFF:func_80006B04:start
 * symbol: func_80006B04
 * score: 63 differing words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x20
 * summary: Asset/count carrier and late loop register shape remain structural; next lever is a typed source record/output layout.
 * PLATEAU-HANDOFF:func_80006B04:end
 */



/* PLATEAU-HANDOFF:func_80006FA0:start
 * symbol: func_80006FA0
 * score: 63 differing words
 * frame: 0x40
 * relocations: 16
 * first-mismatch: +0x0C
 * summary: Active-list carriers and loop register allocation remain structural after restoring the two-argument destroy call.
 * PLATEAU-HANDOFF:func_80006FA0:end
 */

/* PLATEAU-HANDOFF:func_80004590:start
 * symbol: func_80004590
 * score: 84 differing words
 * frame: 0x50
 * relocations: 5
 * first-mismatch: +0x8
 * summary: Outer start less-than-end test duplicated the for-loop check. Remaining plus-16 is five per-compare type masks minus the missing incoming-arg0 store.
 * PLATEAU-HANDOFF:func_80004590:end
 */

/* PLATEAU-HANDOFF:func_8000831C:start
 * symbol: func_8000831C
 * score: 81 differing words
 * frame: 0x28
 * relocations: 13
 * first-mismatch: +0x0
 * summary: Still 81 masked at size delta -4, and this pass is a precise negative that redirects the axis. The transfer from func_8005A7A0 is refuted at its precondition: that lead's finding is that the frame is the variable, and here frame_census reports 0x28 on both sides, fourteen slots each, identical ladders, and not one slot either side uses alone. There is no frame cell for a lever to move. The one measured asymmetry is traffic, not layout: the home of the seventh argument is read three times and written once by the target, and read once and never written here, so the target never register-promotes that parameter while this candidate keeps it in a caller-saved register across the whole middle of the function. That is the structural fact to attack. The L100 divisor lever does not reach it. Discarded-expression probes on that parameter, OR-with-zero, AND-with-minus-one and XOR-with-zero, at five placements and applied once, twice and three times, are byte-inert in all of them, masked staying 81 with the slot traffic unchanged, so L109's narrowing extends from an already-read global to an already-read stack parameter. The one probe placed ahead of the first call is not inert and is worse, 100 masked with the frame growing 0x28 to 0x30. Two respellings of the tenfold multiply as an eightfold plus a twofold, the form the target emits reusing the eightfold it already has, leave masked at 81 and move the size the wrong way, -4 to -8, because cfe reassociates them back. Naming the decrement through the existing temp is flat. Six physical-line arrangements of the four-way OR that builds the colour word are byte-identical, so L59's tie-break does not reach inside a single statement. Call test: p1-only, three calls, so the axis is the save ratio and L106 is not available. What is left is 47 naming words carrying a closed three-cycle over t8, t9 and t7, one L127 ring-phase fact rather than 47 colour questions, plus 29 structural words.
 * PLATEAU-HANDOFF:func_8000831C:end
 */

/* PLATEAU-HANDOFF:func_80007C68:start
 * symbol: func_80007C68
 * score: 93/118 words
 * frame: 0x60
 * relocations: 4
 * first-mismatch: +0x94
 * summary: 25 words of pure register naming from +0x94, frame and stack homes exact. The target colours one value this body leaves to ugen, and a full phase-one force sweep over every web against every colour and the split path (522 compiles) never beats 25, which is what a missing web looks like. The ugen freelist trace shows the ring two draws ahead: v1 is held by the frame -12 local's web, and there is a phantom pop on the texture-fetch line. Naming the index costs the frame and still does not make a web, including across an L97 region; 23 region placements never improve.
 * PLATEAU-HANDOFF:func_80007C68:end
 */

/* PLATEAU-HANDOFF:func_80007E40:start
 * symbol: func_80007E40
 * score: 92 differing words
 * frame: 0x18
 * relocations: 0
 * first-mismatch: 0x18
 * summary: Generated outer-model index controls leave 39 draws and the model-load line unchanged; both regress to 95 masked; retained 92.
 * PLATEAU-HANDOFF:func_80007E40:end
 */

/* PLATEAU-HANDOFF:func_8000A39C:start
 * symbol: func_8000A39C
 * score: 37 differing words
 * frame: 0x58
 * relocations: 3
 * first-mismatch: +0x4
 * summary: 37 words and one surplus instruction. The target spills the unmodified first parameter to its incoming home and keeps the return address at +0x14; a spelling that takes the object base from an inline shift and the sort offset from a separate product reproduces that entry block exactly, but overshoots the frame by eight bytes and loses the copy the target emits after the matrix loads. Those are now two separable decision variables rather than one coupled pair.
 * PLATEAU-HANDOFF:func_8000A39C:end
 */

/* PLATEAU-HANDOFF:func_80006534:start
 * symbol: func_80006534
 * score: 17/205 words
 * frame: 0x38
 * relocations: 7
 * first-mismatch: +0x11C
 * summary: Bitfield decoding adds a capture draw but loses one word; ghost identity and local mask/load schedule remain at the guarded 17-word baseline.
 * PLATEAU-HANDOFF:func_80006534:end
 */

/* PLATEAU-HANDOFF:func_800084C4:start
 * symbol: func_800084C4
 * score: 168/343 words
 * frame: 0xB0
 * relocations: 8
 * first-mismatch: +0x2C8
 * summary: One candidate-only word at +0x2D4 drives a coherent seven-register ring rotation; explicit cast and signed parameter forms are flat.
 * PLATEAU-HANDOFF:func_800084C4:end
 */

/* PLATEAU-HANDOFF:func_80004FE0:start
 * symbol: func_80004FE0
 * score: 189 differing words
 * frame: 0x100
 * relocations: 83
 * first-mismatch: +0x38
 * summary: Workbench structure-mismatch: structure-buckets. Next: resolve category-fill unrolling and packet/index carriers with the frame held exact.
 * PLATEAU-HANDOFF:func_80004FE0:end
 */

/* PLATEAU-HANDOFF:func_80009414:start
 * symbol: func_80009414
 * score: 388 differing words
 * frame: 0x198
 * relocations: 11
 * first-mismatch: +0x40
 * summary: Workbench structure-mismatch: structure-buckets. Next: authenticate the default-mode lifetime and sort-prefix source before further scheduling work.
 * PLATEAU-HANDOFF:func_80009414:end
 */

/* PLATEAU-HANDOFF:func_80007118:start
 * symbol: func_80007118
 * score: 186 differing words
 * frame: 0x38
 * relocations: 71
 * first-mismatch: +0x2C
 * summary: Owner interference removal exposes infinite v0 cost for payload; address reads add sixteen words. The 186-word baseline is retained.
 * PLATEAU-HANDOFF:func_80007118:end
 */

/* PLATEAU-HANDOFF:func_8000B3CC:start
 * symbol: func_8000B3CC
 * score: 202 differing words
 * frame: 0x98
 * relocations: 20
 * first-mismatch: +0x148
 * summary: Workbench structure-mismatch: constant-audit then register-role. Next: resolve time-step/speed homes and the floating zero/reflection allocation.
 * PLATEAU-HANDOFF:func_8000B3CC:end
 */
