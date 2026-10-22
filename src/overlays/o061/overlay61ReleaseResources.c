#include "PR/ultratypes.h"

extern u8 gOverlay61ResourceBaseReloc[];
extern void *gOverlay61Resource58Reloc;
extern void *gOverlay61Resource5CReloc;
extern void *gOverlay61Resource60Reloc;
extern void *gOverlay61Resource64Reloc;
extern void *gOverlay61Resource68Reloc;
extern void *gOverlay61Resource6CReloc;
extern void *gOverlay61Resource70Reloc;
extern void *gOverlay61Resource74Reloc;
extern void *gOverlay61Resource78Reloc;
extern void *gOverlay61Resource7CReloc;
extern void *gOverlay61Resource80Reloc;
extern void *gOverlay61Resource84Reloc;
extern void *gOverlay61Resource88Reloc;

extern void freeFrontEndList(void *);
extern void overlay45ReleaseDescriptor(void *);
extern void packDirectoryFree(void);
extern void rumbleProcessing(s32);

void overlay61ReleaseResources(void) {
    freeFrontEndList(gOverlay61ResourceBaseReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource58Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource5CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource60Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource64Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource68Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource6CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource70Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource74Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource78Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource7CReloc);
    overlay45ReleaseDescriptor(gOverlay61Resource80Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource84Reloc);
    overlay45ReleaseDescriptor(gOverlay61Resource88Reloc);
    packDirectoryFree();
    rumbleProcessing(1);
}
