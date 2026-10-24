/* Mickey-only storage recovery. Original declarations remain inferred.
 * DATA0..0x50: typed sentinel resource list, live animation flags and
 * texture-node grammar. Existing caller/import view debts are not changed. */
#include "PR/ultratypes.h"

typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct Overlay63TextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} Overlay63TextureNode;

s16 overlay63ResourceIds[] = {
    6, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 115, 116, 117, 118, 119, 120, -1
};
s32 overlay63AnimationFlags = 13;
Overlay63TextureNode overlay63TextureNodes[] = {
    { 0, 0, 0, -16, -16 },
    { 0, 0, 0, 0, 0 }
};

#ifdef TARGET_N64
typedef char O63PointerWidth[(sizeof(void *) == 4) ? 1 : -1];
typedef char O63TextureNodeSize[(sizeof(Overlay63TextureNode) == 16) ? 1 : -1];
typedef char O63ResourceListSize[(sizeof(overlay63ResourceIds) == 44) ? 1 : -1];
typedef char O63AnimationFlagsSize[(sizeof(overlay63AnimationFlags) == 4) ? 1 : -1];
typedef char O63TextureNodePairSize[(sizeof(overlay63TextureNodes) == 32) ? 1 : -1];
#endif
