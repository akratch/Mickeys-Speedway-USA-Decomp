#include "PR/ultratypes.h"

typedef struct Overlay20Command {
    u32 w0;
    u32 w1;
} Overlay20Command;

typedef struct Overlay20TileSource {
    u8 pad00[0xA];
    s16 width;
    s16 rows;
    u8 pad0E[4];
    u8 variant;
    u8 pad13[0xD];
    s32 textureOffsets[2];
    s32 outputOffset;
    void *resource;
} Overlay20TileSource;

/* Overlay 20 table-1 record 13 resolves the call at function +0x38 through
 * ORT 206 to the resident texDPTextureX. */
extern void texDPTextureX(Overlay20Command **commands, void *resource,
                          s32 arg2, s32 arg3);

#define OVERLAY20_SHIFTL(value, shift, width)                              \
    ((u32)(((u32)(value) & ((1U << (width)) - 1)) << (shift)))

/* PROVENANCE: source-shape crosswalk from Diddy Kong Racing
 * include/PR/gbi.h:gDma1p and include/f3ddkr.h:gSPPolygon. Mickey's ten-byte
 * vertex records decide the length expression here; the published DKR macros
 * only supply the command construction idiom. */
#define OVERLAY20_VERTEX(packet, address, count, lengthFlags)               \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL(4, 24, 8) |                               \
                 OVERLAY20_SHIFTL(((count) << 3) |                          \
                                       ((u32)(address) & 6),                \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL((((count) << 3) + ((count) << 1) + 8) |   \
                                       (lengthFlags),                       \
                                   0, 16);                                  \
        _g->w1 = (u32)(address);                                            \
    }

#define OVERLAY20_POLYGON(packet, address, count, textured)                 \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL((((count) - 1) << 4) | (textured),        \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL(5, 24, 8) |                               \
                 OVERLAY20_SHIFTL((count) << 4, 0, 16);                    \
        _g->w1 = (u32)(address);                                            \
    }

/* Mickey's 0x90-byte frame proves a seven-entry chunk array. Keeping it after
 * the six preceding scalar/cursor declarations gives the retail sp+0x5C home;
 * all 134 instruction words and the helper relocation are exact. */
void overlay20BuildTileCommands(Overlay20Command **commands,
                                Overlay20TileSource *source, s32 arg2) {
    s32 width;
    s32 remainingRows;
    s32 chunkCount;
    s32 *writeCursor;
    s32 *readCursor;
    s32 textureOffset;
    s32 chunks[7];
    s32 outputOffset;
    s32 chunkWidth;
    s32 doubledWidth;

    texDPTextureX(commands, source->resource, arg2, 0);
    width = source->width;
    chunkCount = 0;
    writeCursor = chunks;
    if (width != 0) {
        do {
            if (width >= 9) {
                *writeCursor = 8;
                chunkCount++;
                writeCursor++;
                width -= 8;
            } else {
                *writeCursor = width;
                chunkCount++;
                writeCursor++;
                width = 0;
            }
        } while (width != 0);
    }

    textureOffset = source->textureOffsets[source->variant];
    remainingRows = source->rows;
    outputOffset = source->outputOffset;
    while (remainingRows--) {
        width = 0;
        if (chunkCount > 0) {
            readCursor = chunks;
            do {
                chunkWidth = *readCursor;
                OVERLAY20_VERTEX((*commands)++,
                                 textureOffset + (s32)0x80000000,
                                 chunkWidth + 1, 0);
                width++;
                readCursor++;
                OVERLAY20_VERTEX(
                    (*commands)++,
                    textureOffset + source->width * 10 + (s32)0x8000000A,
                    chunkWidth + 1, 0x1200);
                doubledWidth = chunkWidth << 1;
                OVERLAY20_POLYGON((*commands)++,
                                  outputOffset + (s32)0x80000000,
                                  doubledWidth, 1);
                textureOffset += chunkWidth * 10;
                outputOffset += doubledWidth << 4;
            } while (width != chunkCount);
        }
        textureOffset += 10;
    }
}
