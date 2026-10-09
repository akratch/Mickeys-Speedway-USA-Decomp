/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:texLoadTextureAddr. Mickey's symbols, linked bytes, and
 * relocations remain authoritative.
 */

#include "PR/ultratypes.h"

extern s32 D_8007BDA4;
extern void texLoadTexture(s32 textureId);

void texLoadTextureAddr(s32 textureId, s32 address) {
    D_8007BDA4 = address;
    texLoadTexture(textureId);
    D_8007BDA4 = 0;
}
