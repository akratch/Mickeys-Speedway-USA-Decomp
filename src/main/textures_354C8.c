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
    u8 metadata[6];
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
    s32 oldBlockedFlags;
    s32 numTextures;
    s32 frameIndex;
    s32 nextFrame;
    s32 hasTexture;
    s32 settingsIndex;
    s32 tableFlags;

    if (D_8007BD8C != 0) {
        oldBlockedFlags = D_8007BD90;
        func_80034920(dlist);
        D_8007BD90 = oldBlockedFlags;
    }

    hasTexture = 0;
    dl = *dlist;
    if (tex != NULL) {
        numTextures = tex->numOfTextures >> 8;
        frameIndex = frame >> 16;
        if ((numTextures >= 2) && (frameIndex < numTextures) &&
            (D_8007BD94 == 0)) {
            currentTexture = ((u8 *)tex) + (frameIndex * tex->textureSize) +
                             sizeof(TextureFrameHeader);
            if ((tex->flags & 0x40) && (tex->unk1B < 2)) {
                nextFrame = frameIndex + 1;
                if (nextFrame >= numTextures) {
                    nextFrame = numTextures - 1;
                    if (tex->spriteFlags & 2) {
                        nextFrame = 0;
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
        settings = D_8007B680;
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
                dl->words.w0 = 0x06000000;
                dl->words.w1 = (u32)textureCommands;
                dl++;
            } else {
                dl->words.w0 = 0x07060030;
                dl->words.w1 = (u32)textureCommands + 0x80000000;
                dl++;
                if ((tex->flags & 0x40) && (tex->unk1B < 2)) {
                    dl->words.w0 = textureCommands[6].words.w0;
                    dl->words.w1 = (u32)nextTexture;
                    dl++;
                    textureCommands += 7;
                    dl->words.w0 = 0x07060030;
                    dl->words.w1 = (u32)textureCommands + 0x80000000;
                    dl++;
                }
            }
        }
    } else {
        settings = D_8007B980;
    }

    if ((flags & 0x80) && (D_8007BD98 != 0)) {
        flags = (flags & ~0x80) | 4;
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
    if ((D_800D302C != ((settingsIndex << 8) | tableFlags)) ||
        (D_800D3020 != settings)) {
        D_800D302C = (settingsIndex << 8) | tableFlags;
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
/* Bounded reproof (2026-09-05): this complete semantic reconstruction emits
 * 274 instructions versus the exact 272-instruction / 0x440-byte owner, with
 * 265 differing target-offset words, normalized edit distance 238, and first
 * mismatch +0x8. The candidate frame is 0x30 versus target 0x40 and it emits
 * 37 versus 39 text relocations. Thirty-one overlay callers authenticate the
 * resident identity. Explicit word copies for both render-state table commands
 * are the retained strict structural gain. All 119 flag identities and ten
 * natural declaration, carrier, control-flow, and command-output forms are nonexact.
 * A fidelity-clean allocator trace identifies flags as the missing s0 web;
 * forcing that color is diagnostic only and disrupts the remaining topology.
 * Preserve the fallback. Resume only with a source-authentic lifetime form
 * that naturally carries flags in s0 while retaining the command structure. */
/* PLATEAU-HANDOFF:func_800349A4:start
 * symbol: func_800349A4
 * score: 265 differing words
 * frame: 0x30 (target 0x40)
 * relocations: 37
 * first-mismatch: +0x8
 * summary: Complete semantic C emits 274 versus 272 instructions at normalized distance 238; all 119 flags and ten natural forms are nonexact, and a fidelity-clean trace isolates the missing saved-register flag web.
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
 * compiler output remain authoritative. */
void func_80034E54(Gfx **arg0, Sprite *arg1, s32 arg2, f32 arg3, u8 arg4) {
    TextureRenderSettings *settings;
    TextureFrameHeader *texture;
    Sprite *sprite = arg1;
    Gfx *dl;
    Gfx *frameCommands;
    f32 frame;
    s32 frameIndex;
    s32 settingsIndex;
    s32 opacity;
    s32 tableFlags;
    s32 stateKey;
    s32 restoreColor;
    s32 frameCount;
    s32 texturesPerFrame;
    s32 nextFrame;
    s32 currentTexture;
    s32 nextTexture;
    s32 i;
    s32 j;

    frameCount = sprite->numberOfFrames;
    frame = arg3;
    if ((f32)frameCount <= frame) {
        frame -= (s32)(frame / frameCount) * frameCount;
    } else if (frame < 0.0f) {
        frame = 0.0f;
    }
    frameIndex = (s32)frame;
    arg2 |= sprite->drawFlags;
    arg2 &= ~D_8007BD90;
    dl = *arg0;
    settingsIndex = 0;
    switch (arg2 & 0xC000) {
    case 0x4000:
        settingsIndex = 0x10;
        break;
    case 0x8000:
        settingsIndex = 0x20;
        break;
    }
    if (arg2 & 0x40) {
        settingsIndex |= 1;
        opacity = (u8)((frame - frameIndex) * 255.0f);
    } else {
        opacity = 0xFF;
    }
    if (D_8007BD88 == 0) {
        settingsIndex |= 2;
    }
    if (D_8007BD80 == 0) {
        if (arg2 & 0x200) {
            settingsIndex |= 4;
            if (D_8007BD9C == 0) {
                dl->words.w0 = 0xFA000000;
                dl->words.w1 = (sprite->metadata[0] << 24) |
                               (sprite->metadata[1] << 16) |
                               (sprite->metadata[2] << 8) | arg4;
                dl++;
                dl->words.w0 = 0xFB000000;
                dl->words.w1 = (sprite->metadata[3] << 24) |
                               (sprite->metadata[4] << 16) |
                               (sprite->metadata[5] << 8) | opacity;
                dl++;
            } else {
                dl->words.w0 = 0xFA000000;
                dl->words.w1 = (D_800D3038 << 24) | (D_800D3039 << 16) |
                               (D_800D303A << 8) | arg4;
                dl++;
                dl->words.w0 = 0xFB000000;
                dl->words.w1 = (D_800D303B << 24) | (D_800D303C << 16) |
                               (D_800D303D << 8) | opacity;
                dl++;
            }
        } else {
            if (arg2 & 0x400) {
                settingsIndex |= 8;
                dl->words.w0 = 0xFA000000;
                dl->words.w1 = (sprite->metadata[0] << 24) |
                               (sprite->metadata[1] << 16) |
                               (sprite->metadata[2] << 8) | arg4;
                dl++;
                dl->words.w0 = 0xFB000000;
                dl->words.w1 = (opacity & 0xFF) | ~0xFF;
                dl++;
            } else if (arg2 & 0x40) {
                dl->words.w0 = 0xFB000000;
                dl->words.w1 = (opacity & 0xFF) | ~0xFF;
                dl++;
            }
        }
    }
    settings = &D_8007BA80[settingsIndex];
    tableFlags = settings->flags | (arg2 & settings->mask);
    stateKey = (settingsIndex << 8) | tableFlags;
    restoreColor = arg2 & 0x200;
    if ((D_800D302C != stateKey) || (D_800D3020 != D_8007BA80)) {
        D_800D302C = stateKey;
        D_800D3020 = D_8007BA80;
        dl->words.w0 = 0xE7000000;
        dl->words.w1 = 0;
        dl++;
        if (tableFlags & 2) {
            if (D_800D3030 == 0) {
                dl->words.w0 = 0xB7000000;
                dl->words.w1 = 1;
                dl++;
            }
            D_800D3030 = 1;
        } else {
            if (D_800D3030 != 0) {
                dl->words.w0 = 0xB6000000;
                dl->words.w1 = 1;
                dl++;
            }
            D_800D3030 = 0;
        }
        if (tableFlags & 8) {
            if (D_800D3034 == 0) {
                dl->words.w0 = 0xB7000000;
                dl->words.w1 = 0x10000;
                dl++;
            }
            D_800D3034 = 1;
        } else {
            if (D_800D3034 != 0) {
                dl->words.w0 = 0xB6000000;
                dl->words.w1 = 0x10000;
                dl++;
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
    if (texture->pad1A != 0) {
        func_8004ADE8(frame, texture->pad1A, texture, dl, arg2);
    }
    if (sprite->drawFlags & 0x40) {
        nextFrame = frameIndex + 1;
        frameCommands = sprite->frameDisplayLists[0];
        texturesPerFrame = sprite->numberOfTextures / frameCount;
        currentTexture = texturesPerFrame * frameIndex;
        if (nextFrame >= frameCount) {
            nextFrame--;
            if (sprite->spriteFlags != 0) {
                nextFrame = 0;
            }
        }
        nextTexture = texturesPerFrame * nextFrame;
        for (i = 0; i < texturesPerFrame; i++) {
            texture = sprite->textures[currentTexture + i];
            dl->words.w0 = 0x07070038;
            dl->words.w1 = (u32)texture->cmd + 0x80000000;
            dl++;
            dl->words.w0 = 0x07070038;
            dl->words.w1 = (u32)sprite->textures[nextTexture + i]->cmd +
                           0x80000038;
            dl++;
            for (j = 0; j < sprite->commandOffsets[i]; j++) {
                dl->words.w0 = frameCommands->words.w0;
                dl->words.w1 = frameCommands->words.w1;
                dl++;
                frameCommands++;
            }
        }
        dl->words.w0 = 0xE7000000;
        dl->words.w1 = 0;
        dl++;
    } else {
        dl->words.w0 = 0x06000000;
        dl->words.w1 = (u32)sprite->frameDisplayLists[frameIndex];
        dl++;
    }
    if (restoreColor != 0) {
        dl->words.w0 = 0xFA000000;
        dl->words.w1 = -1;
        dl++;
    }
    *arg0 = dl;
}
/* PLATEAU-HANDOFF:func_80034E54:start
 * symbol: func_80034E54
 * score: 461 differing target-offset words
 * frame: 0x80 (target 0xB0)
 * relocations: 43
 * first-mismatch: +0x0
 * summary: Complete JFG-guided semantic C emits 459 versus 467 instructions; the retained tree improves the placeholder by five target-offset words, but it lacks the target's s1 carrier and 0x30 non-save frame bytes.
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
Sprite *func_800355A0(s32 spriteId, s32 flags) {
    Sprite *refSprite;
    Sprite *newSprite;
    s32 cacheNum;
    SpriteAsset *spriteAsset;
    TextureFrameHeader *texture;
    s32 i;
    s32 size;
    s8 allocFailed;
    s8 cacheFull;
    s16 numTextures;
    s32 arenaCount; /* extra decl before offset locals holds frame 0x68 */
    s32 displayListOffset;
    s32 textureOffset;
    s32 vertexOffset;
    s32 commandOffset;

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
        if (newSprite) {
        }
        if (node[0] == -1) {
            cacheNum = i;
        }
    }

    if (cacheNum == -1) {
        cacheFull = 1;
        cacheNum = D_800D3008;
        D_800D3008++;
    }

    size = D_800D2FF8[spriteId];
    spriteAsset = D_800D3000;
    piRomLoadSection(0x15, (u32)spriteAsset, size,
                     D_800D2FF8[spriteId + 1] - size);

    numTextures = spriteAsset->frameTexOffsets[spriteAsset->numberOfFrames];
    i = numTextures;
    if (numTextures < spriteAsset->numberOfFrames) {
        i = spriteAsset->numberOfFrames;
    }

    displayListOffset =
        (s32)align16((u8 *)(spriteAsset->numberOfFrames * 4 + 0x18))
        + ((i * 2) * 16);
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

    /* Reconstructs the align16 result; a named triangleOffset local takes s2. */
    D_800D3018 = (SpriteTriangle *)((u8 *)newSprite
        + (displayListOffset - ((i * 2) * 16)));
    D_800D3014 = (Gfx *)((u8 *)newSprite + displayListOffset);
    D_800D3010 = (SpriteVertex *)((u8 *)newSprite + vertexOffset);
    newSprite->textures =
        (TextureFrameHeader **)((u8 *)newSprite + textureOffset);
    newSprite->commandOffsets = (u8 *)newSprite + commandOffset;

    allocFailed = 0;
    /* Defined allocator cue retained from the bounded permuter: this arena
     * offset is dead after the pointer stores and is reused as zero. */
    textureOffset = 0;
    for (i = textureOffset; i < numTextures; i++) {
        D_8007BD84 = 0x8E;
        texture = func_80034448(spriteAsset->baseTextureId + i);
        newSprite->textures[i] = texture;
        if (newSprite->textures[i] == (void *)textureOffset) {
            allocFailed = 1;
        }
        D_8007BD84 = 0x90;
        D_800D2FF4 = 1;
    }

    D_800D2FF4 = textureOffset;
    if (allocFailed) {
        for (i = textureOffset; i < numTextures; i++) {
            texture = newSprite->textures[i];
            if (texture != (void *)textureOffset) {
                func_800347A0(texture);
            }
        }
        if (cacheFull) {
            D_800D3008--;
        }
        mmFree(newSprite);
        return (void *)textureOffset;
    }

    newSprite->numberOfTextures = numTextures;
    newSprite->metadata[textureOffset] = spriteAsset->metadata[textureOffset];
    newSprite->metadata[1] = spriteAsset->metadata[1];
    newSprite->metadata[2] = spriteAsset->metadata[2];
    newSprite->metadata[3] = spriteAsset->metadata[3];
    newSprite->metadata[4] = spriteAsset->metadata[4];
    newSprite->metadata[5] = spriteAsset->metadata[5];
    newSprite->numberOfFrames = spriteAsset->numberOfFrames;
    for (i = textureOffset; i < spriteAsset->numberOfFrames; i++) {
        newSprite->frameDisplayLists[i] = D_800D3014;
        func_80035ADC(spriteAsset, newSprite, i);
        if (newSprite->drawFlags & 0x40) {
            i = spriteAsset->numberOfFrames;
        }
    }
    newSprite->drawFlags |= spriteAsset->flags & 0x600;

    if (D_800D3008 >= 100) {
        return (void *)textureOffset;
    }
    D_800D2FFC[cacheNum << 1] = spriteId;
    D_800D2FFC[(cacheNum << 1) + 1] = (s32)newSprite;
    newSprite->numberOfInstances = 1;
    return newSprite;
}
/* PLATEAU-HANDOFF:func_800355A0:start
 * symbol: func_800355A0
 * score: 109 differing words
 * frame: 0x68
 * relocations: 44
 * first-mismatch: +0x48
 * summary: Size and frame closed (269 words, 0x68). 109 masked; recover vs triangleOffset spill is the remaining 3-word insertion. Do not colour yet.
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
#ifdef NON_MATCHING
/* PROVENANCE: Jet Force Gemini's public src/textures.c:func_80057C50 provides
 * the broad source structure; Mickey's headers, target assembly, and ROM bytes
 * remain authoritative for this game's fields, control flow, and command words.
 * The reconstruction keeps byte-oriented command cursors and expresses the
 * command words as ordinary C stores rather than transplanted instructions. */
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((u8 *)(expr) + (offset)))
void func_80035F48(u8 **dlist, TextureFrameHeader *tex, s32 rtile,
                   s32 tmem) {
    s32 sp84;
    s32 sp80;
    s32 sp7C;
    s32 sp78;
    s32 sp70;
    u8 *sp4C;
    u8 *sp48;
    s32 sp10;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_lo;
    s32 temp_s1;
    s32 temp_s1_2;
    s32 temp_t3;
    s32 temp_t3_2;
    s32 temp_t4;
    s32 temp_t6;
    s32 temp_t7;
    s32 temp_t7_2;
    s32 temp_t8;
    s32 temp_t8_2;
    s32 temp_t9;
    s32 temp_v1_2;
    s32 var_a2;
    s32 var_a3;
    s32 var_s0;
    s32 var_t3;
    s32 var_t3_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    u16 var_t1;
    u16 var_t2;
    u32 temp_v0;
    u8 temp_a0;
    u8 temp_v1;
    u8 *temp_t0;
    u8 *temp_t0_10;
    u8 *temp_t0_11;
    u8 *temp_t0_2;
    u8 *temp_t0_3;
    u8 *temp_t0_4;
    u8 *temp_t0_5;
    u8 *temp_t0_6;
    u8 *temp_t0_7;
    u8 *temp_t0_8;
    u8 *temp_t0_9;
    u8 *var_t0;
    u8 *var_t0_2;

    temp_v1 = M2C_FIELD(tex, u8 *, 2);
    var_s0 = rtile;
    temp_t0 = *dlist;
    temp_v0 = temp_v1 & 0xF;
    temp_t7 = ((s32)temp_v1 >> 4) & 0xF;
    var_t2 = M2C_FIELD(tex, u16 *, 6);
    var_t1 = M2C_FIELD(tex, u16 *, 8);
    switch (temp_v0) {
    case 0:
        sp84 = 3;
        sp80 = 3;
        sp7C = 0;
        sp78 = 0;
        var_v0 = 2;
        break;
    case 1:
    case 4:
        sp84 = 2;
        sp80 = 2;
        sp7C = 0;
        sp78 = 0;
        var_v0 = 2;
        break;
    case 2:
    case 5:
        sp84 = 1;
        sp80 = 2;
        sp7C = 1;
        sp78 = 1;
        var_v0 = 1;
        break;
    default:
        sp84 = 0;
        sp80 = 2;
        sp7C = 3;
        sp78 = 2;
        var_v0 = 0;
        break;
    }
    switch (temp_v0) {
    case 0:
    case 1:
        sp70 = 0;
        if ((temp_t7 == 0) || (temp_t7 == 2)) {
            M2C_FIELD(tex, s16 *, 4) = (s16)(M2C_FIELD(tex, s16 *, 4) | 4);
        }
        break;
    case 4:
    case 5:
    case 6:
        sp70 = 3;
        M2C_FIELD(tex, s16 *, 4) = (s16)(M2C_FIELD(tex, s16 *, 4) | 4);
        break;
    default:
        sp70 = 4;
        break;
    }
    if (sp84 == 0) {
        var_a2 = (s32)var_t2 >> 1;
    } else {
        var_a2 = var_t2 * var_v0;
    }
    temp_a0 = M2C_FIELD(tex, u8 *, 0x1B);
    temp_t3 = (sp70 & 7) << 0x15;
    var_a3 = tmem;
    if ((s32)temp_a0 >= 2) {
        var_v0_2 = 0;
        var_v1 = 0;
        if ((s32)temp_a0 > 0) {
            do {
                temp_lo = ((s32)var_t2 >> var_v0_2) * ((s32)var_t1 >> var_v0_2);
                var_v0_2 += 1;
                var_v1 += temp_lo;
            } while (var_v0_2 < (s32)temp_a0);
            var_v0_2 = 0;
        }
        temp_t3_2 = (sp70 & 7) << 0x15;
        temp_t4 = (sp80 & 3) << 0x13;
        M2C_FIELD(temp_t0, s32 *, 0) = (s32)(temp_t3_2 | 0xFD000000 | temp_t4);
        M2C_FIELD(temp_t0, void **, 4) = (void *)((u8 *)tex + 0x80000020);
        temp_t0_2 = temp_t0 + 8;
        temp_t6 = temp_t3_2 | 0xF5000000;
        sp10 = temp_t6;
        M2C_FIELD(temp_t0_2, s32 *, 4) = 0x07000000;
        M2C_FIELD(temp_t0_2, s32 *, 0) = (s32)(temp_t6 | temp_t4 | (var_a3 & 0x1FF));
        temp_t0_3 = temp_t0_2 + 8;
        sp4C = temp_t0_3;
        temp_t0_4 = temp_t0_3 + 8;
        M2C_FIELD(sp4C, s32 *, 0) = 0xE6000000;
        var_t3 = 0x7FF;
        M2C_FIELD(sp4C, s32 *, 4) = 0;
        sp48 = temp_t0_4;
        temp_t0_5 = temp_t0_4 + 8;
        M2C_FIELD(sp48, s32 *, 0) = 0xF3000000;
        temp_a0_2 = ((s32)(var_v1 + sp7C) >> sp78) - 1;
        if (temp_a0_2 < 0x7FF) {
            var_t3 = temp_a0_2;
        }
        M2C_FIELD(sp48, s32 *, 4) = (s32)(((var_t3 & 0xFFF) << 0xC) | 0x07000000);
        M2C_FIELD(temp_t0_5, s32 *, 0) = 0xE7000000;
        M2C_FIELD(temp_t0_5, s32 *, 4) = 0;
        var_t0 = temp_t0_5 + 8;
        if ((s32)M2C_FIELD(tex, u8 *, 0x1B) > 0) {
            do {
                temp_t8 = (s32)(var_a2 + 7) >> 3;
                M2C_FIELD(var_t0, s32 *, 0) = (s32)(sp10 | ((sp84 & 3) << 0x13) | ((temp_t8 & 0x1FF) << 9) | (var_a3 & 0x1FF));
                temp_t7_2 = (var_s0 & 7) << 0x18;
                temp_s1 = var_v0_2 & 0xF;
                temp_t0_6 = var_t0 + 8;
                M2C_FIELD(var_t0, s32 *, 4) = (s32)(temp_t7_2 | ((M2C_FIELD(tex, u8 *, 0x1E) & 3) << 0x12) | (((M2C_FIELD(tex, u8 *, 0x1F) - var_v0_2) & 0xF) << 0xE) | (temp_s1 << 0xA) | ((M2C_FIELD(tex, u8 *, 0x1C) & 3) << 8) | (((M2C_FIELD(tex, u8 *, 0x1D) - var_v0_2) & 0xF) * 0x10) | temp_s1);
                M2C_FIELD(temp_t0_6, s32 *, 0) = 0xF2000000;
                M2C_FIELD(temp_t0_6, s32 *, 4) = (s32)(temp_t7_2 | ((((var_t2 - 1) * 4) & 0xFFF) << 0xC) | (((var_t1 - 1) * 4) & 0xFFF));
                var_v0_2 += 1;
                var_t0 = temp_t0_6 + 8;
                var_a3 += temp_t8 * var_t1;
                var_s0 += 1;
                var_t2 = (u16)((s32)var_t2 >> 1);
                var_t1 = (u16)((s32)var_t1 >> 1);
                var_a2 >>= 1;
            } while (var_v0_2 < (s32)M2C_FIELD(tex, u8 *, 0x1B));
        }
        var_t0_2 = var_t0 + 8;
        M2C_FIELD(var_t0, s32 *, 0) = (s32)((((M2C_FIELD(tex, u8 *, 0x1B) - 1) & 7) << 0xB) | 0xBB000000 | 1);
        M2C_FIELD(var_t0, s32 *, 4) = 0;
        M2C_FIELD(var_t0_2, s32 *, 0) = 0xB8000000;
        M2C_FIELD(var_t0_2, s32 *, 4) = 0;
    } else {
        temp_t9 = (sp80 & 3) << 0x13;
        temp_s1_2 = temp_t3 | 0xF5000000;
        temp_t0_7 = temp_t0 + 8;
        temp_a0_3 = var_a3 & 0x1FF;
        M2C_FIELD(temp_t0, s32 *, 0) = (s32)(temp_t3 | 0xFD000000 | temp_t9);
        M2C_FIELD(temp_t0, void **, 4) = (void *)((u8 *)tex + 0x80000020);
        M2C_FIELD(temp_t0_7, s32 *, 0) = (s32)(temp_s1_2 | temp_t9 | temp_a0_3);
        temp_t0_8 = temp_t0_7 + 8;
        M2C_FIELD(temp_t0_7, s32 *, 4) = (s32)(((M2C_FIELD(tex, u8 *, 0x1E) & 3) << 0x12) | 0x07000000 | ((M2C_FIELD(tex, u8 *, 0x1F) & 0xF) << 0xE) | ((M2C_FIELD(tex, u8 *, 0x1C) & 3) << 8) | ((M2C_FIELD(tex, u8 *, 0x1D) & 0xF) * 0x10));
        temp_t0_9 = temp_t0_8 + 8;
        M2C_FIELD(temp_t0_8, s32 *, 0) = 0xE6000000;
        M2C_FIELD(temp_t0_8, s32 *, 4) = 0;
        M2C_FIELD(temp_t0_9, s32 *, 0) = 0xF3000000;
        temp_v1_2 = ((s32)((var_t2 * var_t1) + sp7C) >> sp78) - 1;
        sp10 = temp_s1_2;
        temp_t0_10 = temp_t0_9 + 8;
        if (temp_v1_2 < 0x7FF) {
            var_t3_2 = temp_v1_2;
        } else {
            var_t3_2 = 0x7FF;
        }
        M2C_FIELD(temp_t0_9, s32 *, 4) = (s32)(((var_t3_2 & 0xFFF) << 0xC) | 0x07000000);
        M2C_FIELD(temp_t0_10, s32 *, 0) = 0xE7000000;
        M2C_FIELD(temp_t0_10, s32 *, 4) = 0;
        temp_t0_11 = temp_t0_10 + 8;
        M2C_FIELD(temp_t0_11, s32 *, 0) = (s32)(sp10 | ((sp84 & 3) << 0x13) | ((((s32)(var_a2 + 7) >> 3) & 0x1FF) << 9) | temp_a0_3);
        temp_t8_2 = (var_s0 & 7) << 0x18;
        var_t0_2 = temp_t0_11 + 8;
        M2C_FIELD(temp_t0_11, s32 *, 4) = (s32)(temp_t8_2 | ((M2C_FIELD(tex, u8 *, 0x1E) & 3) << 0x12) | ((M2C_FIELD(tex, u8 *, 0x1F) & 0xF) << 0xE) | ((M2C_FIELD(tex, u8 *, 0x1C) & 3) << 8) | ((M2C_FIELD(tex, u8 *, 0x1D) & 0xF) * 0x10));
        M2C_FIELD(var_t0_2, s32 *, 0) = 0xF2000000;
        M2C_FIELD(var_t0_2, s32 *, 4) = (s32)(temp_t8_2 | ((((var_t2 - 1) * 4) & 0xFFF) << 0xC) | (((var_t1 - 1) * 4) & 0xFFF));
    }
    *dlist = var_t0_2 + 8;
}
#undef M2C_FIELD
/* PLATEAU-HANDOFF:func_80035F48:start
 * symbol: func_80035F48
 * score: 371 differing words
 * frame: 0x88 (target 0x90)
 * relocations: 4
 * first-mismatch: +0x0
 * summary: Complete semantic C emits 362 versus 383 instructions; delaying the TMEM carrier improves positional differences by 14 words, but the texture pointer occupies s0 and displaces the target's rtile carrier.
 * PLATEAU-HANDOFF:func_80035F48:end
 */
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80035F48.s")
#endif
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

void func_800367A4(u8 *arg0, s32 *arg1, s32 arg2, f32 *arg3, s32 arg4) {
    volatile s64 frame_pad[2];
    u8 *sp20;
    u8 **sp34;
    u8 sp24;

    sp20 = arg0;
    sp34 = &sp20;
    sp24 = (u8)((s32)TEXTURE_FIELD(arg0, u16 *, 0x10) >> 8);
    func_80036544(&sp24, arg1, arg2, arg3, arg4);
    (void)sp34;
    (void)frame_pad;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_80036544.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_354C8/func_800367A4.s")
#endif

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
