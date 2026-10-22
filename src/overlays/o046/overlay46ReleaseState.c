#include "PR/ultratypes.h"

typedef struct Overlay46Resource Overlay46Resource;
typedef struct Overlay46Group Overlay46Group;

typedef struct Overlay46Data {
    s16 resourceIds[0x26];
    Overlay46Resource *resource4C;
    Overlay46Resource *resource50;
    Overlay46Group *group54;
    u8 pad58[0x12C];
    Overlay46Resource *resource184;
} Overlay46Data;

extern Overlay46Data gOverlay46Data;
extern void overlay46ReleaseIdsReloc(s16 *ids);
extern void overlay44ReleaseHandles(Overlay46Group *group);
extern void mmFree(Overlay46Resource *resource);
extern void func_80006EA0(Overlay46Resource *resource);
extern u8 frontRecallScreenMode(void);
extern void overlay46SetModeReloc(s32 mode);

void overlay46ReleaseState(void) {
    Overlay46Resource *resource;

    overlay46ReleaseIdsReloc(gOverlay46Data.resourceIds);
    overlay44ReleaseHandles(gOverlay46Data.group54);

    if (gOverlay46Data.resource50 != NULL) {
        resource = gOverlay46Data.resource50;
        mmFree(resource);
    }
    if (gOverlay46Data.resource4C != NULL) {
        resource = gOverlay46Data.resource4C;
        mmFree(resource);
    }
    if (gOverlay46Data.resource184 != NULL) {
        func_80006EA0(gOverlay46Data.resource184);
    }

    overlay46SetModeReloc(frontRecallScreenMode());
}
