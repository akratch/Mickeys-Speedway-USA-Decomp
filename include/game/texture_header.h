#ifndef GAME_TEXTURE_HEADER_H
#define GAME_TEXTURE_HEADER_H

#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

/* PROVENANCE: consolidates Mickey's views in textures_35024.c,
 * textures_354C8.c and rcpFast3d.c; their published JFG comparisons are
 * disclosed in those translation units. Mickey's consumers decide the fields.
 *
 * Tier B: Mickey's loader, frame helpers and renderers consume this 32-byte
 * prefix. Explicit unknown bytes preserve the serialized prefix, not compiler
 * padding. Original declarations and complete payload bounds remain unknown. */
typedef struct TextureHeader {
    u16 numberOfInstances;
    u8 format;
    u8 storageFlags; /* Loader compression bit0; frame-loop bit1. */
    s16 renderFlags;
    u16 width;
    u16 height;
    u8 unknown0A;
    u8 posX;
    u8 unknown0C;
    u8 posY;
    u16 textureSize;
    u16 numOfTextures;
    u16 frameAdvanceDelay;
    Gfx *cmd;
    u16 numberOfCommands;
    u8 cpuTextureKey;
    u8 mipLevels;
    u8 tileModeS;
    u8 maskS;
    u8 tileModeT;
    u8 maskT;
} TextureHeader;

typedef TextureHeader TextureFrameHeader;
typedef TextureHeader RcpTextureInfo;

#ifdef TARGET_N64
typedef char TextureHeaderSize[(sizeof(TextureHeader) == 32) ? 1 : -1];
typedef char TextureHeaderPointerWidth[(sizeof(void *) == 4) ? 1 : -1];
#endif

#endif
