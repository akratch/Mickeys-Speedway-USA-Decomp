#include "PR/ultratypes.h"

typedef struct Overlay101Resource20 {
    u8 pad00[0x10];
    void *resource;
} Overlay101Resource20;

typedef struct Overlay101Resource32 {
    u8 pad00[0x1C];
    void *resource;
} Overlay101Resource32;

typedef struct Overlay101OrderEntry {
    void *first;
    void *second;
    u8 pad08[0x14];
} Overlay101OrderEntry;

extern void *gOverlay101Handle1D4;
extern void *gOverlay101Handle1F4;
extern s32 gOverlay101Resource20Count;
extern s32 gOverlay101Resource32Count;
extern s32 gOverlay101OrderCount;
extern void *gOverlay101Handle33C;
extern Overlay101Resource20 gOverlay101Resources20[];
extern Overlay101Resource32 gOverlay101Resources32[];
extern Overlay101OrderEntry gOverlay101OrderEntries[];
extern void overlay101ReleaseReloc(void *resource, ...);

void overlay101Cleanup(void) {
    s32 count;
    Overlay101Resource20 *resource20;
    Overlay101Resource32 *resource32;
    Overlay101OrderEntry *order;

    if (gOverlay101Handle1D4 != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle1D4);
        gOverlay101Handle1D4 = NULL;
    }
    if (gOverlay101Handle1F4 != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle1F4);
        gOverlay101Handle1F4 = NULL;
    }

    count = gOverlay101Resource20Count;
    resource20 = gOverlay101Resources20;
    if (count--) {
        do {
            if (resource20->resource != NULL) {
                overlay101ReleaseReloc(resource20->resource);
                resource20->resource = NULL;
            }
            resource20++;
        } while (count--);
    }

    gOverlay101Resource20Count = 0;
    count = gOverlay101Resource32Count;
    resource32 = gOverlay101Resources32;
    if (count--) {
        do {
            if (resource32->resource != NULL) {
                overlay101ReleaseReloc(resource32->resource);
                resource32->resource = NULL;
            }
            resource32++;
        } while (count--);
    }
    gOverlay101Resource32Count = 0;

    count = gOverlay101OrderCount;
    order = gOverlay101OrderEntries;
    if (count--) {
        do {
            order->first = NULL;
            order->second = NULL;
            order++;
        } while (count--);
    }
    gOverlay101OrderCount = 0;

    if (gOverlay101Handle33C != NULL) {
        overlay101ReleaseReloc(gOverlay101Handle33C,
                               &gOverlay101OrderCount);
        gOverlay101Handle33C = NULL;
    }
}
