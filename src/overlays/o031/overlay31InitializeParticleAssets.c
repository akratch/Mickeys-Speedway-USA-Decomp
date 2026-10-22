#include "PR/ultratypes.h"

typedef struct Overlay31Behaviour {
    u8 pad00[0x9C];
    void *colourLoop;
} Overlay31Behaviour;

extern s32 D_800D4120[];
extern s32 gOverlay31ParticleAssetCount;
extern u8 *gOverlay31ParticleAssets;
extern void **D_8007C8AC;
extern s32 D_8007C8B0;
extern u8 *gOverlay31BehaviourAssets;
extern Overlay31Behaviour **D_8007C8B8;

extern void *piRomLoad(s32 assetId);
extern void overlay31BuildPalettes(void);
extern void *func_800056A4(s32 assetId);
extern void func_overlay_031_F0000000_187F520(void);

/* Preserve the raw split carrier identity in a removable private island. */
static void *const overlay31RuntimeCarrier =
    func_overlay_031_F0000000_187F520;

/* PROVENANCE: Diddy Kong Racing, src/particles.c (init_particle_assets);
 * semantic source-shape analogue only. Mickey's ROM decides every detail. */
void func_overlay_031_F00002E8_187F808(void) {
    s32 i;

    for (i = 0; i < 5; i++) {
        D_800D4120[i] = 0;
    }

    D_8007C8AC = piRomLoad(0x33);
    while ((s32)D_8007C8AC[gOverlay31ParticleAssetCount + 1] != -1) {
        gOverlay31ParticleAssetCount++;
    }

    gOverlay31ParticleAssets = piRomLoad(0x34);
    for (i = 0; i < gOverlay31ParticleAssetCount; i++) {
        D_8007C8AC[i] =
            gOverlay31ParticleAssets + (s32)D_8007C8AC[i];
    }

    overlay31BuildPalettes();

    D_8007C8B8 = piRomLoad(0x35);
    while ((s32)D_8007C8B8[D_8007C8B0 + 1] != -1) {
        D_8007C8B0++;
    }

    gOverlay31BehaviourAssets = piRomLoad(0x36);
    for (i = 0; i < D_8007C8B0; i++) {
        D_8007C8B8[i] =
            (Overlay31Behaviour *)(gOverlay31BehaviourAssets +
                                   (s32)D_8007C8B8[i]);
        if ((u32)D_8007C8B8[i]->colourLoop != 0xFFFFFFFF) {
            D_8007C8B8[i]->colourLoop =
                func_800056A4(
                    (s32)D_8007C8B8[i]->colourLoop);
        } else {
            D_8007C8B8[i]->colourLoop = NULL;
        }
    }
}
