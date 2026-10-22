#include "PR/ultratypes.h"

/* Runtime ORT 518 resolves this placeholder call to resident levelGetCounts. */
extern void levelGetCounts(void);
extern void *mmAlloc(s32, s32);
extern void func_80028EA0(void *);
extern void func_80028F3C(void);
extern void *D_800CF5B0;
extern void *D_800D18E0;
extern u32 D_8007A1CC;

/* DKR v77/v80 and JFG have no exact donor for this initialization sequence. */
void overlay18Initialize(void) {
    void *handle;
    register void **slot;

    levelGetCounts();
    D_800CF5B0 = NULL;
    handle = mmAlloc(0xF4, 0x8F);
    slot = &D_800D18E0;
    *slot = handle;
    func_80028EA0(*slot);
    func_80028F3C();
    D_8007A1CC = 0x10000000;
}
