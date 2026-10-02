#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/*
 * PROVENANCE: the texture-TU order and direct helper bodies below were
 * compared with Jet Force Gemini's public src/textures.c. Mickey's field
 * offsets, globals, boundaries, and compiler output remain authoritative.
 */

extern s32 D_8007BD84;
extern s32 D_8007BD80;
extern s32 D_8007BD88;
extern s32 D_8007BD8C;
extern s32 D_8007BD90;
extern void *D_800D3020;
extern u8 *D_800D3024;
extern u8 *D_800D3028;
extern s32 D_800D302C;
extern s32 D_800D3030;
extern s32 D_800D3034;
extern s32 D_800D3004;
extern void *D_800D3000;
extern s32 D_800D3008;
extern s32 D_800D300C;
extern s32 D_800D2FF4;
extern s32 *D_800D2FF8;
extern s32 *D_800D2FFC;
extern struct SpriteVertex *D_800D3010;
extern Gfx *D_800D3014;
extern struct SpriteTriangle *D_800D3018;
extern u8 D_800D3038;
extern u8 D_800D3039;
extern u8 D_800D303A;
extern u8 D_800D303B;
extern u8 D_800D303C;
extern u8 D_800D303D;
extern s32 D_8007BD9C;
extern void func_8004ADE8();

typedef struct TextureFrameHeader {
    u8 pad00[2];
    u8 format;
    u8 spriteFlags;
    s16 flags;
    u16 width;
    u16 height;
    u8 pad0A;
    u8 posX;
    u8 pad0C;
    u8 posY;
    u16 textureSize;
    u16 numOfTextures;
    u16 frameAdvanceDelay;
    Gfx *cmd;
    u16 numberOfCommands;
    u8 pad1A;
    u8 unk1B;
    u8 unk1C;
    u8 isCompressed;
    u8 unk1E;
    u8 unk1F;
} TextureFrameHeader;

typedef struct Sprite {
    u8 numberOfFrames;
    u8 spriteFlags;
    s16 numberOfTextures;
    s16 numberOfInstances;
    s16 drawFlags;
    u8 primRed;
    u8 primGreen;
    u8 primBlue;
    u8 envRed;
    u8 envGreen;
    u8 envBlue;
    u8 pad0E[2];
    TextureFrameHeader **textures;
    u8 *commandOffsets;
    Gfx *frameDisplayLists[1];
} Sprite;

typedef struct SpriteAsset {
    s16 baseTextureId;
    s16 numberOfFrames;
    s16 anchorX;
    s16 anchorY;
    u8 metadata[6];
    s16 flags;
    u8 pad10[4];
    u8 frameTexOffsets[1];
} SpriteAsset;

typedef struct SpriteVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} SpriteVertex;

typedef struct SpriteTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    s16 uv0U;
    s16 uv0V;
    s16 uv1U;
    s16 uv1V;
    s16 uv2U;
    s16 uv2V;
} SpriteTriangle;

typedef struct TextureRenderSettings {
    Gfx *upper;
    Gfx *lower;
    s32 mask;
    s32 flags;
} TextureRenderSettings;

extern TextureRenderSettings D_8007BA80[];

#ifdef NON_MATCHING
#define TEXTURE_FIELD(expr, type_ptr, offset) (*(type_ptr)((u8 *)(expr) + (offset)))
#endif

extern u8 D_8007BDA0;
extern TextureRenderSettings D_8007B680[];
extern TextureRenderSettings D_8007B980[];
extern u8 D_8007BD94;
extern u8 D_8007BD98;
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void mmFree(void *ptr);
extern void *func_8002B314(s32 size, u32 colourTag);
extern u8 *align16(u8 *address);
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset,
                            s32 size);
extern TextureFrameHeader *func_80034448(s32 textureId);
extern void func_800347A0(TextureFrameHeader *texture);

void func_80035F48(u8 **dlist, TextureFrameHeader *tex, s32 rtile, s32 tmem);
void func_80035ADC(SpriteAsset *spriteAsset, Sprite *sprite, s32 frameId);

void func_800348C8(s32 tagId) {
    D_8007BD84 = tagId;
}

TextureFrameHeader *func_800348D4(TextureFrameHeader *arg0, s32 arg1) {
    TextureFrameHeader *ret = arg0 + 1;
    if ((arg1 > 0) && (arg1 < arg0->numOfTextures << 8)) {
        ret = (TextureFrameHeader *)(((u8 *)arg0) +
                                     ((arg1 >> 16) * arg0->textureSize)) + 1;
    }
    return ret;
}

void func_80034910(void) {
    D_8007BD8C = 1;
}

void func_80034920(Gfx **dlist) {
    D_8007BD90 = 0;
    D_800D3024 = 0;
    D_800D3028 = 0;
    D_800D3020 = 0;
    D_800D302C = 0;
    D_800D3030 = 1;
    D_800D3034 = 1;
    if (dlist != NULL) {
        gDPPipeSync((*dlist)++);
        gSPSetGeometryMode((*dlist)++, G_FOG | G_SHADING_SMOOTH | G_SHADE | G_ZBUFFER);
    }
    D_8007BD8C = 0;
}
#ifdef NON_MATCHING
/* PROVENANCE: gDkrDmaDisplayList as defined in Jet Force Gemini's public
 * include/f3ddkr.h (G_DMADL is command 7). */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                     \
    {                                                                          \
        Gfx *_g = (Gfx *)(pkt);                                                \
                                                                               \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) | \
                        _SHIFTL((numberOfCommands * 8), 0, 16));               \
        _g->words.w1 = (unsigned int)(address);                                \
    }
/*
 * PROVENANCE: Jet Force Gemini's public texDPTextureX establishes the related
 * texture/render-state role.  This body's fields, tables, control flow, and
 * display-list commands were reconstructed from Mickey's own function.
 */
void func_800349A4(Gfx **dlist, TextureFrameHeader *tex, s32 flags,
                   s32 frame) {
    TextureRenderSettings *settings;
    Gfx *dl;
    Gfx *textureCommands;
    u8 *currentTexture;
    u8 *nextTexture;
    s32 numTextures;
    s32 oldBlockedFlags;
    s32 frameIndex;
    s32 nextFrame;
    s32 hasTexture;
    s32 settingsIndex;
    s32 tableFlags;
    s32 *cachedState;

    if (D_8007BD8C != 0) {
        oldBlockedFlags = D_8007BD90;
        func_80034920(dlist);
        D_8007BD90 = oldBlockedFlags;
    }

    hasTexture = 0;
    dl = *dlist;
    if (tex != NULL) {
        settings = D_8007B680;
        numTextures = tex->numOfTextures >> 8;
        frameIndex = frame >> 16;
        if ((numTextures >= 2) && (frameIndex < numTextures) &&
            (D_8007BD94 == 0)) {
            currentTexture = ((u8 *)tex) + (frameIndex * tex->textureSize) +
                             sizeof(TextureFrameHeader);
            if ((tex->flags & 0x40) && (tex->unk1B < 2)) {
                nextFrame = frameIndex + 1;
                if (nextFrame >= numTextures) {
                    if (tex->spriteFlags & 2) {
                        nextFrame = 0;
                    } else {
                        nextFrame = numTextures - 1;
                    }
                }
                nextTexture = ((u8 *)tex) +
                              (nextFrame * tex->textureSize) +
                              sizeof(TextureFrameHeader);
            } else {
                nextTexture = currentTexture;
            }
        } else {
            currentTexture = (u8 *)(tex + 1);
            nextTexture = currentTexture;
        }

        flags |= tex->flags;
        hasTexture = 1;
        if ((currentTexture != D_800D3024) ||
            (nextTexture != D_800D3028)) {
            D_800D3024 = currentTexture;
            D_800D3028 = nextTexture;
            textureCommands = tex->cmd;
            dl->words.w0 = textureCommands->words.w0;
            dl->words.w1 = (u32)currentTexture;
            dl++;
            textureCommands++;
            if (tex->unk1B >= 2) {
                gSPDisplayList(dl++, textureCommands);
            } else {
                gDkrDmaDisplayList(dl++, (u32)textureCommands + 0x80000000, 6);
                if ((tex->flags & 0x40) && (tex->unk1B < 2)) {
                    dl->words.w0 = textureCommands[6].words.w0;
                    dl->words.w1 = (u32)nextTexture;
                    dl++;
                    textureCommands += 7;
                    gDkrDmaDisplayList(dl++, (u32)textureCommands + 0x80000000, 6);
                }
            }
        }
    } else {
        settings = D_8007B980;
    }

    if ((flags & 0x80) && (D_8007BD98 != 0)) {
        flags &= ~0x80;
        flags |= 4;
    }
    flags &= ~D_8007BD90;
    settingsIndex = (flags & 0x70) >> 4;
    if (hasTexture != 0) {
        if (tex->unk1B >= 2) {
            settingsIndex += 0x20;
            if (flags & 0x80) {
                settingsIndex += 8;
            }
        } else if (flags & 0x80) {
            settingsIndex += 8;
        } else if (flags & 0x100) {
            settingsIndex += 0x10;
        } else if (flags & 0x200) {
            settingsIndex += 0x18;
        }
    } else if (flags & 0x800) {
        settingsIndex += 8;
    }

    settings += settingsIndex;
    tableFlags = settings->flags | (flags & settings->mask);
    cachedState = &D_800D302C;
    if ((*cachedState != ((settingsIndex << 8) | tableFlags)) ||
        (D_800D3020 != settings)) {
        *cachedState = (settingsIndex << 8) | tableFlags;
        D_800D3020 = settings;
        gDPPipeSync(dl++);
        if (tableFlags & 2) {
            if (D_800D3030 == 0) {
                gSPSetGeometryMode(dl++, G_ZBUFFER);
            }
            D_800D3030 = 1;
        } else {
            if (D_800D3030 != 0) {
                gSPClearGeometryMode(dl++, G_ZBUFFER);
            }
            D_800D3030 = 0;
        }
        if (tableFlags & 8) {
            if (D_800D3034 == 0) {
                gSPSetGeometryMode(dl++, G_FOG);
            }
            D_800D3034 = 1;
        } else {
            if (D_800D3034 != 0) {
                gSPClearGeometryMode(dl++, G_FOG);
            }
            D_800D3034 = 0;
        }
        dl->words.w0 = settings->upper[tableFlags >> 3].words.w0;
        dl->words.w1 = settings->upper[tableFlags >> 3].words.w1;
        dl++;
        dl->words.w0 = settings->lower[tableFlags].words.w0;
        dl->words.w1 = settings->lower[tableFlags].words.w1;
        dl++;
    }
    *dlist = dl;
}
/* Size delta 0 and frame 0x40 closed (from +8 and -16). flags reaches s0
 * naturally once `flags = (flags & ~0x80) | 4` is two statements: its total
 * save goes 14 to 16 against the callee toll 15.85 (L56). The two DMA
 * commands are gSPDisplayList and gDkrDmaDisplayList; the wrap takes an else
 * arm; numTextures is declared ahead of oldBlockedFlags so that home lands at
 * -0x1C; settings is assigned at the head of the texture arm. A pointer to
 * D_800D302C, live across its load and store, stops the early address hold
 * and restores the target store order. Reusing an integer for that address
 * grew by 4 bytes. Left: register naming from +0x24. */
/* PLATEAU-HANDOFF:func_800349A4:start
 * symbol: func_800349A4
 * score: 175 differing words
 * frame: 0x40
 * relocations: 39
 * first-mismatch: +0x24
 * summary: Delta 0, 175 masked, frame 0x40, 39 relocs. A D_800D302C pointer rematerializes the load and store; integer reuse grew by 4. Left: naming from +0x24.
 * PLATEAU-HANDOFF:func_800349A4:end
 */
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800349A4.s")
#endif

void func_80034DE4(s32 value) {
    D_8007BD88 = value;
}

void func_80034DF0(u8 red, u8 green, u8 blue, u8 alternateRed,
                   u8 alternateGreen, u8 alternateBlue) {
    D_800D3038 = red;
    D_800D3039 = green;
    D_800D303A = blue;
    D_800D303B = alternateRed;
    D_800D303C = alternateGreen;
    D_800D303D = alternateBlue;
    D_8007BD9C = 1;
}

void func_80034E48(void) {
    D_8007BD9C = 0;
}
#ifdef NON_MATCHING
/* PROVENANCE: control-flow shape adapted from Jet Force Gemini's public
 * asm/nonmatchings/textures/sprDPset.s. Mickey's fields, globals, calls, and
 * compiler output remain authoritative.
 *
 * 2026-10-02 (lane o-tex), 424 to 43 at delta 0: the wrap quotient is its
 * own local (it takes a0, not a ring temp); frameIndex is assigned once after
 * the wrap and before the cursor read, and the 0x40 arm subtracts it from
 * frame in place, so uopt keeps the one truncation in the join block; the
 * frame counts and the per-frame texture count are read from the sprite at
 * each use; frameIndex is reused for the next frame's texture base, which
 * keeps currentTexture a live variable spilled at its home; the next-frame
 * wrap is if/else (the target's branch over an empty else); the second DMA
 * adds 0x80000038 directly; the six colour bytes are named fields (array
 * subscripts reassociate the colour OR chain). Left: the opacity fraction's
 * float temps, the call's argument copies, a v0/v1 swap in the settings
 * copy, and frameIndex's spill slot. */
void func_80034E54(Gfx **dlist, Sprite *sprite, s32 flags, f32 frame, u8 alpha) {
    TextureRenderSettings *settings;
    TextureFrameHeader *texture;
    TextureFrameHeader *nextTex;
    Gfx *frameCommands;
    s32 settingsIndex;
    s32 opacity;
    s32 texturesPerFrame;
    s32 currentTexture;
    s32 tableFlags;
    s32 stateKey;
    Gfx *dl;
    s32 nextTexture;
    s32 nextFrame;
    s32 i;
    s32 j;
    s32 frameIndex;

    if ((f32)(u32)sprite->numberOfFrames <= frame) {
        i = frame / (u32)sprite->numberOfFrames;
        frame -= (s32)(i * (u32)sprite->numberOfFrames);
    } else if (frame < 0.0f) {
        frame = 0.0f;
    }
    frameIndex = frame;
    dl = *dlist;
    flags |= sprite->drawFlags;
    flags &= ~D_8007BD90;
    settingsIndex = 0;
    switch (flags & 0xC000) {
    case 0x4000:
        settingsIndex = 0x10;
        break;
    case 0x8000:
        settingsIndex = 0x20;
        break;
    }
    if (flags & 0x40) {
        settingsIndex |= 1;
        frame -= frameIndex;
        opacity = (u8)(frame * 255.0f);
    } else {
        opacity = 0xFF;
    }
    if (D_8007BD88 == 0) {
        settingsIndex |= 2;
    }
    if (D_8007BD80 == 0) {
        if (flags & 0x200) {
            settingsIndex |= 4;
            if (D_8007BD9C == 0) {
                gDPSetPrimColor(dl++, 0, 0, sprite->primRed, sprite->primGreen, sprite->primBlue, alpha);
                gDPSetEnvColor(dl++, sprite->envRed, sprite->envGreen, sprite->envBlue, opacity);
            } else {
                gDPSetPrimColor(dl++, 0, 0, D_800D3038, D_800D3039, D_800D303A, alpha);
                gDPSetEnvColor(dl++, D_800D303B, D_800D303C, D_800D303D, opacity);
            }
        } else if (flags & 0x400) {
            settingsIndex |= 8;
            gDPSetPrimColor(dl++, 0, 0, sprite->primRed, sprite->primGreen, sprite->primBlue, alpha);
            gDPSetEnvColor(dl++, 255, 255, 255, opacity);
        } else if (flags & 0x40) {
            gDPSetEnvColor(dl++, 255, 255, 255, opacity);
        }
    }
    settings = &D_8007BA80[settingsIndex];
    tableFlags = settings->flags | (flags & settings->mask);
    stateKey = (settingsIndex << 8) | tableFlags;
    if ((D_800D302C != stateKey) || (D_800D3020 != D_8007BA80)) {
        D_800D302C = stateKey;
        D_800D3020 = D_8007BA80;
        gDPPipeSync(dl++);
        if (tableFlags & 2) {
            if (D_800D3030 == 0) {
                gSPSetGeometryMode(dl++, G_ZBUFFER);
            }
            D_800D3030 = 1;
        } else {
            if (D_800D3030 != 0) {
                gSPClearGeometryMode(dl++, G_ZBUFFER);
            }
            D_800D3030 = 0;
        }
        if (tableFlags & 8) {
            if (D_800D3034 == 0) {
                gSPSetGeometryMode(dl++, G_FOG);
            }
            D_800D3034 = 1;
        } else {
            if (D_800D3034 != 0) {
                gSPClearGeometryMode(dl++, G_FOG);
            }
            D_800D3034 = 0;
        }
        dl->words.w0 = settings->upper[tableFlags >> 3].words.w0;
        dl->words.w1 = settings->upper[tableFlags >> 3].words.w1;
        dl++;
        dl->words.w0 = settings->lower[tableFlags].words.w0;
        dl->words.w1 = settings->lower[tableFlags].words.w1;
        dl++;
    }
    D_800D3024 = 0;
    D_800D3028 = 0;
    texture = sprite->textures[0];
    if (texture->pad1A) {
        func_8004ADE8(texture->pad1A, texture);
    }
    if (sprite->drawFlags & 0x40) {
        nextFrame = frameIndex + 1;
        frameCommands = sprite->frameDisplayLists[0];
        texturesPerFrame = sprite->numberOfTextures / sprite->numberOfFrames;
        currentTexture = texturesPerFrame * frameIndex;
        if (nextFrame >= sprite->numberOfFrames) {
            if (sprite->spriteFlags != 0) {
                nextFrame = 0;
            } else {
                nextFrame--;
            }
        }
        frameIndex = texturesPerFrame * nextFrame;
        for (i = 0; i < texturesPerFrame; i++) {
            texture = sprite->textures[currentTexture + i];
            nextTex = sprite->textures[frameIndex + i];
            gDkrDmaDisplayList(dl++, (u32)texture->cmd + 0x80000000, 7);
            gDkrDmaDisplayList(dl++, (u32)nextTex->cmd + 0x80000038, 7);
            for (j = 0; j < sprite->commandOffsets[i]; j++) {
                dl->words.w0 = frameCommands->words.w0;
                dl->words.w1 = frameCommands->words.w1;
                dl++;
                frameCommands++;
            }
        }
        gDPPipeSync(dl++);
    } else {
        gSPDisplayList(dl++, sprite->frameDisplayLists[frameIndex]);
    }
    if (flags & 0x200) {
        gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
    }
    *dlist = dl;
}
/* PLATEAU-HANDOFF:func_80034E54:start
 * symbol: func_80034E54
 * score: 43/467 words
 * frame: 0xB0 (target 0xB0)
 * relocations: 43
 * first-mismatch: +0x108
 * summary: 424 to 43 at delta 0: quotient local, one frameIndex web reused, sprite fields re-read, if/else wrap, named colour bytes. Left: opacity float temps.
 * PLATEAU-HANDOFF:func_80034E54:end
 */
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80034E54.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: control-flow shape adapted from Diddy Kong Racing's public
 * src/textures_sprites.c::tex_load_sprite and cross-checked against Jet Force
 * Gemini's public texLoadSprite object. Mickey's allocation layout, fields,
 * globals, calls, and compiler output remain authoritative. */
/* The six word locals are declared in the order of their stack homes
 * (cacheNum 0x5C, then the offsets down to commandOffset at 0x48); the small
 * scalars follow so they pack below them. triangleOffset is never read: the
 * triangle pointer is rebuilt from displayListOffset, which is the target's
 * shape, and the declaration holds the home. Plateau 2026-10-01 (lane d-res1):
 * 109 -> 106, natural source with no allocator cues. 2026-10-02 (lane e-res3):
 * 106 -> 105 by reading the ROM table through one entry pointer. */
Sprite *func_800355A0(s32 spriteId, s32 flags) {
    Sprite *refSprite;
    Sprite *newSprite;
    s32 cacheNum;
    s32 triangleOffset;
    s32 displayListOffset;
    s32 vertexOffset;
    s32 textureOffset;
    s32 commandOffset;
    SpriteAsset *spriteAsset;
    TextureFrameHeader *texture;
    s32 i;
    s32 size;
    s8 allocFailed;
    s8 cacheFull;
    s16 numTextures;

    D_800D300C = flags;
    if (spriteId < 0 || spriteId >= D_800D3004) {
        return NULL;
    }

    for (i = 0, cacheFull = 0; i < D_800D3008; i++) {
        if (spriteId == D_800D2FFC[i << 1]) {
            refSprite = (Sprite *)D_800D2FFC[(i << 1) + 1];
            refSprite->numberOfInstances++;
            return refSprite;
        }
    }

    cacheNum = -1;
    for (i = 0; i < D_800D3008; i++) {
        s32 *node = &D_800D2FFC[i << 1];
        if (node[0] == -1) {
            cacheNum = i;
        }
    }

    if (cacheNum == -1) {
        cacheFull = 1;
        cacheNum = D_800D3008;
        D_800D3008++;
    }

    {
        s32 *entry = &D_800D2FF8[spriteId];
        size = entry[0];
        spriteAsset = D_800D3000;
        piRomLoadSection(0x15, (u32)spriteAsset, size, entry[1] - size);
    }

    numTextures = spriteAsset->frameTexOffsets[spriteAsset->numberOfFrames];
    i = numTextures;
    if (numTextures < spriteAsset->numberOfFrames) {
        i = spriteAsset->numberOfFrames;
    }

    triangleOffset = (s32)align16((u8 *)(spriteAsset->numberOfFrames * 4 + 0x18));
    displayListOffset = triangleOffset + ((i * 2) * 16);
    textureOffset = displayListOffset + ((i * 4) * 8) +
                    (spriteAsset->numberOfFrames * sizeof(Gfx));
    vertexOffset = textureOffset + (i * 4);
    commandOffset = vertexOffset + ((i * 4) * 10);
    size = (s32)align16((u8 *)(commandOffset + (i * 2)));
    newSprite = func_8002B314(size, 0x8E);
    if (newSprite == NULL) {
        if (cacheFull) {
            D_800D3008--;
        }
        return NULL;
    }

    D_800D3018 = (SpriteTriangle *)((u8 *)newSprite + (displayListOffset - ((i * 2) * 16)));
    D_800D3014 = (Gfx *)((u8 *)newSprite + displayListOffset);
    D_800D3010 = (SpriteVertex *)((u8 *)newSprite + vertexOffset);
    newSprite->textures =
        (TextureFrameHeader **)((u8 *)newSprite + textureOffset);
    newSprite->commandOffsets = (u8 *)newSprite + commandOffset;

    allocFailed = 0;
    for (i = 0; i < numTextures; i++) {
        D_8007BD84 = 0x8E;
        texture = func_80034448(spriteAsset->baseTextureId + i);
        newSprite->textures[i] = texture;
        if (newSprite->textures[i] == NULL) {
            allocFailed = 1;
        }
        D_8007BD84 = 0x90;
        D_800D2FF4 = 1;
    }

    D_800D2FF4 = 0;
    if (allocFailed) {
        for (i = 0; i < numTextures; i++) {
            texture = newSprite->textures[i];
            if (texture != NULL) {
                func_800347A0(texture);
            }
        }
        if (cacheFull) {
            D_800D3008--;
        }
        mmFree(newSprite);
        return NULL;
    }

    newSprite->numberOfTextures = numTextures;
    newSprite->primRed = spriteAsset->metadata[0];
    newSprite->primGreen = spriteAsset->metadata[1];
    newSprite->primBlue = spriteAsset->metadata[2];
    newSprite->envRed = spriteAsset->metadata[3];
    newSprite->envGreen = spriteAsset->metadata[4];
    newSprite->envBlue = spriteAsset->metadata[5];
    newSprite->numberOfFrames = spriteAsset->numberOfFrames;
    for (i = 0; i < spriteAsset->numberOfFrames; i++) {
        newSprite->frameDisplayLists[i] = D_800D3014;
        func_80035ADC(spriteAsset, newSprite, i);
        if (newSprite->drawFlags & 0x40) {
            i = spriteAsset->numberOfFrames;
        }
    }
    newSprite->drawFlags |= spriteAsset->flags & 0x600;

    if (D_800D3008 >= 100) {
        return NULL;
    }
    D_800D2FFC[cacheNum << 1] = spriteId;
    D_800D2FFC[(cacheNum << 1) + 1] = (s32)newSprite;
    newSprite->numberOfInstances = 1;
    return newSprite;
}
/* PLATEAU-HANDOFF:func_800355A0:start
 * symbol: func_800355A0
 * score: 105 differing words
 * frame: 0x68
 * relocations: 44
 * first-mismatch: +0x48
 * summary: Table entry pointer (106->105); open: target hoists the D_800D2FF8 lui and the id shift into block 2, here the shift lands in block 1
 * PLATEAU-HANDOFF:func_800355A0:end
 */
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800355A0.s")
#endif

void func_800359D4(Sprite *sprite) {
    s32 i;
    s32 frame;

    if (sprite != NULL) {
        sprite->numberOfInstances--;
        if (sprite->numberOfInstances <= 0) {
            for (i = 0; i < D_800D3008; i++) {
                if (sprite == (Sprite *)D_800D2FFC[(i << 1) + 1]) {
                    for (frame = 0; frame < sprite->numberOfTextures; frame++) {
                        func_800347A0(sprite->textures[frame]);
                    }
                    mmFree(sprite);
                    D_800D2FFC[i << 1] = -1;
                    D_800D2FFC[(i << 1) + 1] = -1;
                    break;
                }
            }
        }
    }
}
#define SPRITE_PHYSICAL(address) ((u32)((u8 *)(address) - 0x80000000))
#define SPRITE_DMA(packet, address, count)                                  \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = (0x07000000 | (((count) & 0xFF) << 16) |             \
                        (((count) << 3) & 0xFFFF));                          \
        _g->words.w1 = (u32)(address);                                      \
    }
#define SPRITE_VERTEX(packet, address, count)                               \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = (0x04000000 |                                        \
                        (((((count) << 3) | ((u32)(address) & 6)) & 0xFF)   \
                         << 16) |                                           \
                        (((((count) << 3) + ((count) << 1) + 8) | 0x200)   \
                         & 0xFFFF));                                        \
        _g->words.w1 = (u32)(address);                                      \
    }
#define SPRITE_POLYGON(packet, address)                                     \
    {                                                                       \
        Gfx *_g = (Gfx *)(packet);                                          \
        _g->words.w0 = 0x05110020;                                          \
        _g->words.w1 = (u32)(address);                                      \
    }

/* PROVENANCE: source shape adapted from Diddy Kong Racing's public
 * src/textures_sprites.c::sprite_init_frame and cross-checked against Jet
 * Force Gemini's public func_800577D8 object. Mickey's fields, globals, and
 * compiler output remain authoritative. */
void func_80035ADC(SpriteAsset *spriteAsset, Sprite *sprite, s32 frameId) {
    s32 pad[2];
    s32 anchorX;
    s32 anchorY;
    s32 tileEnd;
    s32 tileOffsetX;
    s32 tileOffsetY;
    s32 left;
    s32 numQuads;
    s32 curVertIndex;
    s32 texWidth;
    s32 texHeight;
    s32 tileIndex;
    SpriteVertex *vertex;
    SpriteVertex *curVerts;
    SpriteTriangle *triangle;
    Gfx *dlptr;
    Gfx *batchStart;
    TextureFrameHeader *tex;
    s32 commandOffsetCount;

    anchorX = spriteAsset->anchorX;
    anchorY = spriteAsset->anchorY;
    dlptr = D_800D3014;
    vertex = D_800D3010;
    triangle = D_800D3018;
    tileIndex = spriteAsset->frameTexOffsets[frameId];
    tileEnd = spriteAsset->frameTexOffsets[frameId + 1];

    if (frameId == 0 || tileIndex < tileEnd) {
        tex = sprite->textures[tileIndex];
        ((u8 *)sprite)[1] = tex->spriteFlags & 2;
        sprite->drawFlags = tex->flags & 0xC07B;
    }

    curVertIndex = 0;
    numQuads = 0;
    batchStart = dlptr;
    commandOffsetCount = 0;
    while (tileIndex < tileEnd) {
        curVerts = vertex;
        tex = sprite->textures[tileIndex];
        vertex += 4;
        texWidth = tex->width;
        texHeight = tex->height;
        tileOffsetX = tex->posX - anchorX;
        tileOffsetY = anchorY - tex->posY;

        vertex[-4].x = tileOffsetX;
        vertex[-4].y = tileOffsetY - 1;
        vertex[-4].z = 0;
        vertex[-4].r = 255;
        vertex[-4].g = 255;
        vertex[-4].b = 255;
        vertex[-4].a = 255;
        vertex[-3].x = tileOffsetX + texWidth - 1;
        vertex[-3].y = tileOffsetY - 1;
        vertex[-3].z = 0;
        vertex[-3].r = 255;
        vertex[-3].g = 255;
        vertex[-3].b = 255;
        vertex[-3].a = 255;
        vertex[-2].x = tileOffsetX + texWidth - 1;
        vertex[-2].y = tileOffsetY - texHeight;
        vertex[-2].z = 0;
        vertex[-2].r = 255;
        vertex[-2].g = 255;
        vertex[-2].b = 255;
        vertex[-2].a = 255;
        vertex[-1].x = tileOffsetX;
        vertex[-1].y = tileOffsetY - texHeight;
        vertex[-1].z = 0;
        vertex[-1].r = 255;
        vertex[-1].g = 255;
        vertex[-1].b = 255;
        vertex[-1].a = 255;

        if (sprite->drawFlags & 0x40) {
            if (batchStart != dlptr) {
                sprite->commandOffsets[commandOffsetCount++] =
                    (dlptr - batchStart);
                batchStart = dlptr;
            }
        } else {
            SPRITE_DMA(dlptr++, SPRITE_PHYSICAL(tex->cmd),
                       tex->numberOfCommands);
        }

        if (numQuads == 0) {
            left = tileEnd - tileIndex;
            if (left > 5) {
                left = 5;
            }
            SPRITE_VERTEX(dlptr++, SPRITE_PHYSICAL(curVerts), left * 4);
        }

        SPRITE_POLYGON(dlptr++, SPRITE_PHYSICAL(triangle));
        triangle[0].flags = 0x40;
        triangle[0].vi0 = curVertIndex + 3;
        triangle[0].vi1 = curVertIndex + 2;
        triangle[0].vi2 = curVertIndex + 1;
        triangle[0].uv0U = (texWidth - 1) << 5;
        triangle[0].uv0V = (texHeight - 1) << 5;
        triangle[0].uv1U = (texWidth - 1) << 5;
        triangle[0].uv1V = 0;
        triangle[0].uv2U = 1;
        triangle[0].uv2V = 0;
        triangle[1].flags = 0x40;
        triangle[1].vi0 = curVertIndex + 4;
        triangle[1].vi1 = curVertIndex + 3;
        triangle[1].vi2 = curVertIndex + 1;
        /* Inert IDO allocation cue found by the bounded permuter. */
        triangle++;
        triangle--;
        triangle[1].uv0U = 1;
        triangle[1].uv0V = (texHeight - 1) << 5;
        triangle[1].uv1U = (texWidth - 1) << 5;
        triangle[1].uv1V = (texHeight - 1) << 5;
        triangle[1].uv2U = 1;
        triangle[1].uv2V = 0;
        triangle += 2;

        curVertIndex += 4;
        numQuads++;
        tileIndex++;
        if (numQuads >= 5) {
            numQuads = 0;
            curVertIndex = 0;
        }
    }

    if (sprite->drawFlags & 0x40) {
        sprite->commandOffsets[commandOffsetCount++] = dlptr - batchStart;
        sprite->commandOffsets[commandOffsetCount] = 0;
    }
    gDPPipeSync(dlptr++);
    gSPEndDisplayList(dlptr++);
    D_800D3014 = dlptr;
    D_800D3010 = vertex;
    D_800D3018 = triangle;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public
 * src/textures.c::func_80057B8C; Mickey's fields, calls, and compiled bytes
 * remain authoritative. The donor's empty condition is retained because it
 * advances IDO's temporary FIFO without emitting an instruction. */
void func_80035E88(TextureFrameHeader *tex, Gfx *displayList) {
    Gfx *dlist = displayList;

    if (tex) {
    }
    tex->cmd = dlist;
    func_80035F48((u8 **)&dlist, tex, 0, 0);
    if (tex->unk1B < 2 && (tex->flags & 0x40)) {
        if (!(tex->format & 0xF)) {
            func_80035F48((u8 **)&dlist, tex, 1,
                          (0x1000 - tex->textureSize) >> 3);
        } else {
            func_80035F48((u8 **)&dlist, tex, 1, 0x100);
        }
    }
    tex->numberOfCommands = dlist - tex->cmd;
}
/*
 * PROVENANCE: adapted from Jet Force Gemini's public src/textures.c
 * func_8005719C_57D9C (the texture-load display-list builder: the two format
 * switches, mipmap size sum, explicit load commands and per-level tiles).
 * Mickey's 4-bit line width, swapped-load LoadBlock (dxt 0), field offsets
 * and compiled bytes are authoritative.
 *
 * Matched 2026-10-02 (from 371 masked at -84): the donor body written with
 * the standard gbi macros on a local cursor, the tile line inlined at both
 * uses, and fourteen locals laid out so the four switch-assigned sizes land
 * at the target's homes (two before them, one between them and imgFmt).
 */
#define OS_PHYSICAL_TO_K0(x) (void *)(((u32)(x) + 0x80000000))
void func_80035F48(u8 **dlist, TextureFrameHeader *tex, s32 rtile,
                   s32 tmem) {
    s32 texHeight;
    s32 texWidth;
    s32 tileImgSiz;
    s32 imgSiz;
    s32 imgSizIncr;
    s32 imgSizShift;
    u32 texFormat;
    s32 imgFmt;
    s32 texFlags;
    s32 line;
    s32 imgSizTileBytes;
    s32 i;
    s32 size;
    Gfx *dl;

    dl = (Gfx *) *dlist;
    texFormat = tex->format & 0xF;
    texFlags = (tex->format >> 4) & 0xF;
    texWidth = tex->width;
    texHeight = tex->height;
    switch (texFormat) {
    case 0:
        tileImgSiz = G_IM_SIZ_32b;
        imgSiz = G_IM_SIZ_32b;
        imgSizIncr = 0;
        imgSizShift = 0;
        imgSizTileBytes = 2;
        break;
    case 1:
    case 4:
        tileImgSiz = G_IM_SIZ_16b;
        imgSiz = G_IM_SIZ_16b;
        imgSizIncr = 0;
        imgSizShift = 0;
        imgSizTileBytes = 2;
        break;
    case 2:
    case 5:
        tileImgSiz = G_IM_SIZ_8b;
        imgSiz = G_IM_SIZ_16b;
        imgSizIncr = 1;
        imgSizShift = 1;
        imgSizTileBytes = 1;
        break;
    default:
        tileImgSiz = G_IM_SIZ_4b;
        imgSiz = G_IM_SIZ_16b;
        imgSizIncr = 3;
        imgSizShift = 2;
        imgSizTileBytes = 0;
        break;
    }
    switch (texFormat) {
    case 0:
    case 1:
        imgFmt = G_IM_FMT_RGBA;
        if ((texFlags == 0) || (texFlags == 2)) {
            tex->flags |= 4;
        }
        break;
    case 4:
    case 5:
    case 6:
        imgFmt = G_IM_FMT_IA;
        tex->flags |= 4;
        break;
    default:
        imgFmt = G_IM_FMT_I;
        break;
    }
    if (tileImgSiz == 0) {
        line = texWidth >> 1;
    } else {
        line = texWidth * imgSizTileBytes;
    }
    if (tex->unk1B >= 2) {
        i = 0;
        size = 0;
        for (; i < tex->unk1B; i++) {
            size += (texWidth >> i) * (texHeight >> i);
        }
        gDPSetTextureImage(dl++, imgFmt, imgSiz, 1, OS_PHYSICAL_TO_K0(tex + 1));
        gDPSetTile(dl++, imgFmt, imgSiz, 0, tmem, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPLoadSync(dl++);
        gDPLoadBlock(dl++, G_TX_LOADTILE, 0, 0, ((size + imgSizIncr) >> imgSizShift) - 1, 0);
        gDPPipeSync(dl++);
        for (i = 0; i < tex->unk1B; i++) {
            gDPSetTile(dl++, imgFmt, tileImgSiz, (line + 7) >> 3, tmem, rtile, 0, tex->unk1E,
                       tex->unk1F - i, i, tex->unk1C, tex->isCompressed - i, i);
            gDPSetTileSize(dl++, rtile, 0, 0, (texWidth - 1) << G_TEXTURE_IMAGE_FRAC,
                           (texHeight - 1) << G_TEXTURE_IMAGE_FRAC);
            tmem += ((line + 7) >> 3) * texHeight;
            rtile++;
            texWidth >>= 1;
            texHeight >>= 1;
            line >>= 1;
        }
        gSPTexture(dl++, 0, 0, tex->unk1B - 1, 0, 1);
        gSPEndDisplayList(dl++);
    } else {
        gDPSetTextureImage(dl++, imgFmt, imgSiz, 1, OS_PHYSICAL_TO_K0(tex + 1));
        gDPSetTile(dl++, imgFmt, imgSiz, 0, tmem, G_TX_LOADTILE, 0, tex->unk1E, tex->unk1F, 0,
                   tex->unk1C, tex->isCompressed, 0);
        gDPLoadSync(dl++);
        gDPLoadBlock(dl++, G_TX_LOADTILE, 0, 0,
                     ((texWidth * texHeight + imgSizIncr) >> imgSizShift) - 1, 0);
        gDPPipeSync(dl++);
        gDPSetTile(dl++, imgFmt, tileImgSiz, (line + 7) >> 3, tmem, rtile, 0, tex->unk1E,
                   tex->unk1F, 0, tex->unk1C, tex->isCompressed, 0);
        gDPSetTileSize(dl++, rtile, 0, 0, (texWidth - 1) << G_TEXTURE_IMAGE_FRAC,
                       (texHeight - 1) << G_TEXTURE_IMAGE_FRAC);
    }
    *dlist = (u8 *) dl;
}
#ifdef NON_MATCHING
extern f32 D_80082670;

s32 func_80036544(u8 *arg0, s32 *arg1, s32 arg2, f32 *arg3, s32 arg4) {
    s32 var_a3;
    f32 temp_f0;
    f32 var_f0;
    f32 var_f12;
    f32 var_f16;
    f32 var_f2;
    f32 var_f6;
    f32 var_f6_2;
    s32 temp_a0;
    s32 temp_a2;
    s32 temp_v0;
    u8 temp_t1;
    u8 temp_t3;
    u8 temp_t5;
    u8 temp_t8;
    u8 *temp_t0;

    temp_v0 = *arg1;
    if (!(temp_v0 & 1)) {
        return 0;
    }
    temp_t0 = *TEXTURE_FIELD(arg0, u8 ***, 0x10);
    temp_a0 = temp_v0 & 8;
    temp_a2 = temp_v0 & 4;
    var_a3 = 0;
    temp_f0 = *arg3;
    if (!(TEXTURE_FIELD(temp_t0, s16 *, 4) & 0x40)) {
        if (temp_a2 != 0) {
            temp_t3 = TEXTURE_FIELD(arg0, u8 *, 0);
            var_f6 = (f32)temp_t3;
            if ((s32)temp_t3 < 0) {
                var_f6 += 4294967296.0f;
            }
            var_f2 = var_f6 - 0.5f;
            if (temp_a0 != 0) {
                var_f12 = 0.5f;
            } else {
                var_f12 = 0.0f;
            }
        } else {
            temp_t5 = TEXTURE_FIELD(arg0, u8 *, 0);
            var_f16 = (f32)temp_t5;
            if ((s32)temp_t5 < 0) {
                var_f16 += 4294967296.0f;
            }
            var_f12 = 0.0f;
            var_f2 = var_f16 - D_80082670;
        }
    } else {
        var_f12 = 0.0f;
        if ((temp_a2 != 0) || (temp_a0 == 0) || !(TEXTURE_FIELD(temp_t0, u8 *, 3) & 2)) {
            temp_t1 = TEXTURE_FIELD(arg0, u8 *, 0);
            var_f6_2 = (f32)temp_t1;
            if ((s32)temp_t1 < 0) {
                var_f6_2 += 4294967296.0f;
            }
            var_f2 = var_f6_2 - 1.0f;
        } else {
            temp_t8 = TEXTURE_FIELD(arg0, u8 *, 0);
            var_f2 = (f32)temp_t8;
            if ((s32)temp_t8 < 0) {
                var_f2 += 4294967296.0f;
            }
        }
    }
    if (temp_v0 & 2) {
        var_f0 = temp_f0 - ((f32)(arg2 * arg4) / 60.0f);
        if (var_f0 < var_f12) {
            if (temp_a2 != 0) {
                var_a3 = 1;
                if (temp_a0 != 0) {
                    *arg1 = temp_v0 & ~2;
                    var_f0 = (var_f12 - var_f0) + var_f12;
                    var_a3 = 1;
                } else {
                    var_f0 = var_f12;
                }
            } else {
                var_a3 = 1;
                if (temp_a0 != 0) {
                    var_f0 += var_f2;
                } else {
                    var_f0 = var_f12;
                    var_a3 = 1;
                }
            }
        }
    } else {
        var_f0 = temp_f0 + ((f32)(arg2 * arg4) / 60.0f);
        if (var_f2 < var_f0) {
            var_a3 = 1;
            if (temp_a2 != 0) {
                *arg1 = temp_v0 | 2;
                var_f0 = var_f2 - (var_f0 - var_f2);
            } else if (temp_a0 != 0) {
                var_f0 -= var_f2;
            } else {
                var_f0 = var_f2;
            }
        }
    }
    *arg3 = var_f0;
    return var_a3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80036544.s")
#endif

void func_800367A4(u8 *arg0, s32 *arg1, s32 arg2, f32 *arg3, s32 arg4) {
    Sprite sprite;
    TextureFrameHeader *texture;

    texture = (TextureFrameHeader *)arg0;
    sprite.textures = &texture;
    sprite.numberOfFrames = (u8)(texture->numOfTextures >> 8);
    func_80036544((u8 *)&sprite, arg1, arg2, arg3, arg4);
}

/* JFG's texAnimateTexture body, with Mickey's four-bit flag relocation and
 * random-number entry point retained as local target-specific evidence. */
void func_800367E8(TextureFrameHeader *texture, u32 *triangleBatchInfoFlags,
                   s32 *arg2, s32 updateRate) {
    s32 breakVar;
    u16 *frameAdvanceDelay;
    u8 blink;
    s32 arg2Temp = *arg2;
    s32 flags = *triangleBatchInfoFlags;

    if (flags & (1 << 21)) {
        blink = D_8007BDA0;
        if (!(flags & (1 << 22))) {
            if (blink == 0) {
                if (func_800299E8(0, 0x3FF) > 0x3EF) {
                    flags &= ~(1 << 23);
                    flags |= (1 << 22);
                }
            } else if (blink != 2) {
                flags &= ~(1 << 23);
                flags |= (1 << 22);
            }
        } else if (!(flags & (1 << 23))) {
            arg2Temp += texture->frameAdvanceDelay * updateRate;
            if (arg2Temp >= texture->numOfTextures) {
                if (blink == 3) {
                    arg2Temp = texture->numOfTextures - 1;
                } else {
                    arg2Temp = ((texture->numOfTextures * 2) - arg2Temp) - 1;
                    if (arg2Temp < 0) {
                        arg2Temp = 0;
                        flags &= ~((1 << 23) | (1 << 22));
                    } else {
                        flags |= (1 << 23);
                    }
                }
            }
        } else {
            arg2Temp -= texture->frameAdvanceDelay * updateRate;
            if (arg2Temp < 0) {
                arg2Temp = 0;
                flags &= ~((1 << 23) | (1 << 22));
            }
        }
        D_8007BDA0 = 0;
    } else if (flags & (1 << 22)) {
        if (!(flags & (1 << 23))) {
            arg2Temp += texture->frameAdvanceDelay * updateRate;
        } else {
            frameAdvanceDelay = &texture->frameAdvanceDelay;
            arg2Temp -= (*frameAdvanceDelay) * updateRate;
        }
        do {
            breakVar = FALSE;
            if (arg2Temp < 0) {
                arg2Temp = -arg2Temp;
                flags &= ~(1 << 23);
                breakVar = TRUE;
            }
            if (arg2Temp >= texture->numOfTextures) {
                arg2Temp = ((texture->numOfTextures * 2) - arg2Temp) - 1;
                flags |= (1 << 23);
                breakVar = TRUE;
            }
        } while (breakVar);
    } else if (!(flags & (1 << 23))) {
        arg2Temp += texture->frameAdvanceDelay * updateRate;
        while (arg2Temp >= texture->numOfTextures) {
            arg2Temp -= texture->numOfTextures;
        }
    } else {
        arg2Temp -= texture->frameAdvanceDelay * updateRate;
        while (arg2Temp < 0) {
            arg2Temp += texture->numOfTextures;
        }
    }
    *arg2 = arg2Temp;
    *triangleBatchInfoFlags = flags;
}
#ifdef NON_MATCHING
#undef TEXTURE_FIELD
#endif
