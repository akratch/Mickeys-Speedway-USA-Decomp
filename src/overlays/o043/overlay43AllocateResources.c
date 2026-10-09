#include "PR/ultratypes.h"

typedef struct Overlay43Input {
    u8 pad00[0x44];
    s16 mode;
} Overlay43Input;

typedef struct Overlay43Resources {
    u8 pad00[0x20];
    u8 *aligned;
    void *primary;
    void *secondary;
    void *tertiary;
} Overlay43Resources;

extern void *mmAlloc(s32 size, s32 tag);
extern void func_8002B768(void *pointer);

s32 func_overlay_043_F0001184_188B154(Overlay43Input *input,
                                      Overlay43Resources *resources) {
    s32 pad[2];
    s32 secondaryCount;
    s32 primaryCount;

    resources->primary = mmAlloc(0x10B4, 0x87);
    if (resources->primary == NULL) {
        return 0;
    }

    resources->aligned = (u8 *)
        ((((u32)resources->primary + 0x20) & ~0x3F) + 0x80);
    if (input->mode == 1) {
        primaryCount = 0x28;
        secondaryCount = 6;
    } else {
        primaryCount = 0x3C;
        secondaryCount = 0xA;
    }

    resources->secondary = mmAlloc(primaryCount * 8, 0x87);
    if (resources->secondary == NULL) {
        func_8002B768(resources->primary);
        return 0;
    }

    resources->tertiary = mmAlloc(secondaryCount << 6, 0x87);
    if (resources->tertiary == NULL) {
        func_8002B768(resources->primary);
        func_8002B768(resources->secondary);
        return 0;
    }

    return 1;
}
