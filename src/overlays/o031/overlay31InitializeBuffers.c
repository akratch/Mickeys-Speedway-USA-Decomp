#include "PR/ultratypes.h"

typedef struct Overlay31EffectRecord {
    u8 pad00[0x24];
    u8 active;
    u8 pad25[3];
} Overlay31EffectRecord;

typedef struct Overlay31Vertex {
    u8 bytes[10];
} Overlay31Vertex;

extern void reset_particles(void);
extern void *mmAlloc();
extern void *overlay31CreateConfig(s32 kind, void *source, s32 width,
                                   s32 height, s32 count);
extern void *overlay31CreateRecords(s32 count);
extern void *overlay31CreatePool(s32 count);
extern void func_overlay_031_F0000000_187F520(void);
extern void *piRomLoad(s32 assetId);
extern void *texLoadSprite();
extern void *texLoadTexture();
extern void *modLoadModel();
extern void mmFree(void *asset);

extern s32 D_8007C88C;
extern s32 D_8007C890;
extern void *D_8007C89C[2];
extern u8 gOverlay31TriangleSourceA[];
extern u8 gOverlay31TriangleSourceB[];
extern u8 gOverlay31RectangleSourceA[];
extern u8 gOverlay31RectangleSourceB[];
extern void *D_800D4120[4];
extern void *D_8007C898;
extern void *D_8007C894;
extern void **D_8007C884;
extern s32 D_8007C888;
extern Overlay31EffectRecord *D_8007C8BC;
extern s32 D_8007C8C0;

/* PROVENANCE: Diddy Kong Racing, src/particles.c
 * (init_particle_buffers); semantic source-shape analogue only. */
/* Exact 245-word match; declaration order preserves IDO's 0x48-byte frame. */
void overlay31InitializeBuffers(s32 maxTriangle, s32 maxRectangle,
                                s32 maxSprite, s32 maxLine, s32 maxPoint,
                                s32 maxExtra, s32 maxEffects) {
    s32 i;
    s32 vertexCount;
    s32 temp_var;
    s16 *assetBuffer;

    reset_particles();

    if (maxTriangle < 0) {
        maxTriangle = 0x10;
    }
    if (maxRectangle < 0) {
        maxRectangle = 0x10;
    }
    if (maxSprite < 0) {
        maxSprite = 0xD0;
    }
    if (maxExtra < 0) {
        maxExtra = 0xF;
    }
    if (maxLine < 0) {
        maxLine = 0xA0;
    }
    if (maxPoint < 0) {
        maxPoint = 0x40;
    }
    if (maxEffects < 0) {
        maxEffects = 0x20;
    }

    vertexCount = maxTriangle * 3 + maxRectangle * 4 + maxSprite +
                  maxLine * 0x12 + maxPoint * 0x78;
    D_8007C88C = maxLine;
    D_8007C890 = maxPoint;
    D_8007C89C[0] =
        mmAlloc(vertexCount * 2 * sizeof(Overlay31Vertex), 0x8C);
    D_8007C89C[1] =
        (u8 *)D_8007C89C[0] + vertexCount * 10;

    D_800D4120[0] = overlay31CreateConfig(
        (s32)gOverlay31TriangleSourceA, gOverlay31TriangleSourceB,
        3, 1, maxTriangle);
    D_800D4120[1] = overlay31CreateConfig(
        (s32)gOverlay31RectangleSourceA, gOverlay31RectangleSourceB,
        4, 2, maxRectangle);
    D_800D4120[2] = overlay31CreateConfig(0, 0, 0, 0, maxSprite);
    D_800D4120[3] = overlay31CreateConfig(0, 0, 0, 0, maxExtra);
    D_8007C898 = overlay31CreatePool(maxPoint);
    D_8007C894 = overlay31CreateRecords(D_8007C88C);
    func_overlay_031_F0000000_187F520();

    if (D_8007C884 == 0) {
        assetBuffer = piRomLoad(0x3A);
        D_8007C888 = 0;
        while (assetBuffer[D_8007C888] != -1) {
            D_8007C888++;
        }

        D_8007C884 =
            mmAlloc(D_8007C888 * 4, 0x8C);
        for (i = 0; i < D_8007C888; i++) {
            switch (assetBuffer[i] & 0xC000) {
                case 0x8000:
                    D_8007C884[i] =
                        texLoadSprite(assetBuffer[i] & 0x3FFF, 1);
                    break;
                case 0xC000:
                    D_8007C884[i] =
                        texLoadTexture(assetBuffer[i] & 0x3FFF);
                    break;
                case 0:
                    D_8007C884[i] =
                        modLoadModel(assetBuffer[i], 0);
                    break;
            }
        }
        mmFree(assetBuffer);
    }

    D_8007C8BC =
        mmAlloc(maxEffects * sizeof(Overlay31EffectRecord), 0x8C);
    for (i = 0; i < maxEffects; i++) {
        D_8007C8BC[i].active = 0;
    }
    D_8007C8C0 = maxEffects;
}
