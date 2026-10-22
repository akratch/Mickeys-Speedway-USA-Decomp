#include "PR/ultratypes.h"

typedef struct Overlay57Node {
    u8 pad00[0x28];
    s32 value28;
    u8 pad2C[0xE];
    u8 byte3A;
    u8 pad3B[0x4D];
    s32 value88;
} Overlay57Node;

typedef struct Overlay57Result {
    u8 pad00[8];
    Overlay57Node *node;
    f32 valueC;
    u8 pad10[4];
    u8 byte14;
} Overlay57Result;

extern u8 gOverlay57UpdateIndex;
extern u8 D_8007BF2C;
extern s32 gOverlay57UpdateSelection;
extern u16 gOverlay57UpdateFlags[];
extern u32 gOverlay57UpdateMasks[];
extern u32 gOverlay57UpdateShifts[];
extern s8 gOverlay57UpdateValues[];
extern f32 gOverlay57UpdateScale;
extern void animseqStopPath(s32 id);
extern void *func_8005077C(s32 id);
extern void animseqStartPath(s32 id);
extern Overlay57Result *func_800508B4(s32 id);
extern void func_8005AD64(Overlay57Node *node, s32, s32, s32 value);

/* DKR v77/v80 and JFG contain only unrelated generic mask/shift idioms. */
void overlay57UpdateNode(void) {
    register u32 flag;
    void *first;
    Overlay57Result *result;

    flag = (gOverlay57UpdateFlags[gOverlay57UpdateSelection] &
            gOverlay57UpdateMasks[gOverlay57UpdateIndex]) >>
           gOverlay57UpdateShifts[gOverlay57UpdateIndex];
    if (flag == 0) {
        animseqStopPath(0x4D);
        return;
    }
    first = func_8005077C(0x4D);
    animseqStartPath(0x4D);
    result = func_800508B4(0x4D);
    if (result != NULL && result->node != NULL) {
        if (first != NULL) {
            result->valueC = gOverlay57UpdateScale;
            result->byte14 = 0;
            result->node->byte3A = D_8007BF2C;
            func_8005AD64(result->node, 0, 0, result->node->value28);
        }
        result->node->value88 = gOverlay57UpdateValues[flag];
    }
}
