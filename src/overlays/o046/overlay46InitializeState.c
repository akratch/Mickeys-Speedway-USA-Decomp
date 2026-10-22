#include "PR/ultratypes.h"

typedef struct Overlay46Resource {
    u8 pad00[0x3C];
    s32 value3C;
} Overlay46Resource;

typedef struct Overlay46Descriptor {
    s16 type;
    s16 pad02;
    s16 value4;
    s16 value6;
    s16 value8;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay46Descriptor;

extern s16 gOverlay46Ids[];
extern s32 gOverlay46Value0;
extern s32 gOverlay46Value4;
extern s32 gOverlay46Value8;
extern void *gOverlay46Value2C;
extern void *gOverlay46Resource4C;
extern void *gOverlay46Resource50;
extern void *gOverlay46Group54;
extern s32 gOverlay46Mode58;
extern s32 gOverlay46Timer5C;
extern Overlay46Resource *gOverlay46Resource184;
extern void *gOverlay46Value1E4;

extern void overlay46ReleaseIdsReloc(s16 *ids);
extern void *func_80036DD0(s32 kind);
extern void *overlay44CreateAnimationState(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                                       s32 arg4);
extern void func_80037414(s32 arg0, f32 arg1, f32 arg2, s32 arg3,
                                    s32 arg4, s32 arg5, s32 arg6);
extern Overlay46Resource *func_8000590C(
    Overlay46Descriptor *descriptor, s32 arg1);
extern void overlay46SetModeReloc(s32 mode);
extern void func_800015F8(void);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay46InitializeState(void) {
    Overlay46Descriptor descriptor;
    Overlay46Resource *resource;

    overlay46ReleaseIdsReloc(gOverlay46Ids);
    gOverlay46Value2C = gOverlay46Value1E4;
    gOverlay46Resource4C = func_80036DD0(2);
    gOverlay46Resource50 = func_80036DD0(0);
    gOverlay46Group54 = overlay44CreateAnimationState(0, 0x7C, 0x41, 0, 0);
    gOverlay46Mode58 = 1;
    gOverlay46Timer5C = 100;
    gOverlay46Value0 = 0xFF;
    gOverlay46Value4 = 0;
    gOverlay46Value8 = 0;
    func_80037414(1, 2.5f, 0.0f, 0, 0, 0, 0);

    descriptor.type = 0x101;
    descriptor.value4 = 0;
    descriptor.value6 = 0;
    descriptor.value8 = 0;
    descriptor.red = 0;
    descriptor.green = 0x40;
    descriptor.blue = 0;
    descriptor.alpha = 0;
    if ((resource = gOverlay46Resource184 =
             func_8000590C(&descriptor, 0)) != NULL) {
        resource->value3C = 0;
    }
    overlay46SetModeReloc(1);
    func_800015F8();
}
