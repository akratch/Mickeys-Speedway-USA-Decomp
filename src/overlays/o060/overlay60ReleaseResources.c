#include "PR/ultratypes.h"

extern void *gOverlay60Resources[];
extern void *gOverlay60ResourcesEnd[];
extern void *gOverlay60OptionalResource;
extern u8 gOverlay60PrimaryResource;
extern void overlay60ReleaseReloc(void *resource);
extern void func_8002917C(void);

/* DKR v77/v80 and JFG contain only generic resource-cleanup idioms. */
void overlay60ReleaseResources(void) {
    void **resource;
    void **end;

    resource = gOverlay60Resources; end = gOverlay60ResourcesEnd;
    do {
        if (*resource != NULL) {
            overlay60ReleaseReloc(*resource);
            *resource = NULL;
        }
        resource++;
    } while (resource != end);

    if (gOverlay60OptionalResource != NULL) {
        overlay60ReleaseReloc(gOverlay60OptionalResource);
        gOverlay60OptionalResource = NULL;
    }
    overlay60ReleaseReloc(&gOverlay60PrimaryResource);
    func_8002917C();
}
