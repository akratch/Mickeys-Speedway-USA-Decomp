#include "PR/ultratypes.h"
#include "overlay33GbiSubset.inc"

typedef struct Overlay33Locals {
    u8 pad88[2];
    u8 alpha;
    u8 blue;
    u8 green;
    u8 red;
    s16 unused;
    s16 high;
    s16 low;
    u32 height;
    u32 width;
    u32 pad9C;
    Gfx *other;
    Gfx *cursor;
} Overlay33Locals;

extern s32 D_800D1C38;
extern u8 D_8[];
extern u8 D_1808[];
/* The colour-image array read here and the display-list-end array written
 * at the tail are two distinct overlay objects: the reads carry a zero
 * low half, the store carries 0x3788. */
extern void *D_800D1C28[];
extern void *gOverlay33DisplayLists[];
extern void *D_800D2FAC;

extern void viGetCurrentSize(u32 *width, u32 *height);
extern void rsp_segment(Gfx **cursor, s32 pass, void *value);
extern void rcpInitSp(Gfx **cursor);
extern void rcpInitDpNoSize(Gfx **cursor);
extern void texDPInit(Gfx **cursor);
extern void camSetNo(void *value);
extern void camSetView(Gfx **cursor, Gfx **other);
extern void trackGetFog(void *value, s16 *low, s16 *high,
                                   s16 *unused, u8 *red, u8 *green,
                                   u8 *blue, u8 *alpha);
extern void sprSetTextureFilter(s32 value);

/* Mickey-local reconstruction using the reviewed SDK macro subset below.
 *
 * The fog block is the SDK's `gSPFogPosition` shape: the `gMoveWd` packet
 * word is written before the range is read back, and each half of the packed
 * word goes through `_SHIFTL`.  `_SHIFTL(q, 16, 16)` is `(q & 0xFFFF) << 16`;
 * uopt drops the mask as redundant under the shift but keeps the masked value
 * as its own web, which is the register-to-register copy the target spends
 * ahead of the second divide. */
void overlay33BuildDisplayList(void) {
    Overlay33Locals locals;

    viGetCurrentSize(&locals.width, &locals.height);
    locals.cursor = (Gfx *)&D_8[D_800D1C38 * 0xC00];
    locals.other = (Gfx *)&D_1808[D_800D1C38 * 0xC00];

    rsp_segment(&locals.cursor, 0, 0);
    rsp_segment(&locals.cursor, 1,
                            D_800D1C28[D_800D1C38]);
    rsp_segment(&locals.cursor, 2, D_800D2FAC);
    rcpInitSp(&locals.cursor);
    rcpInitDpNoSize(&locals.cursor);

    gSPClearGeometryMode(locals.cursor++, 1);
    gDPSetColorImage(locals.cursor++, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                     locals.width,
                     (u32)D_800D1C28[D_800D1C38] +
                         0x80000000U);
    gDPSetScissor(locals.cursor++, 0, 0, 0, locals.width, locals.height);
    gMoveWd(locals.cursor++, 2, 0, 0);
    gDPSetFillColor(locals.cursor++, 0);
    gDPSetCycleType(locals.cursor++, G_CYC_FILL);
    gDPFillRectangle(locals.cursor++, 0, 0, locals.width - 1,
                     locals.height - 1);
    gDPPipeSync(locals.cursor++);

    texDPInit(&locals.cursor);
    gMoveWd(locals.cursor++, 2, 0, 0);
    gSPClearGeometryMode(locals.cursor++, 0x1000);
    gDPSetBlendColor(locals.cursor++, 0, 0, 0, 0x64);
    gDPSetPrimColor(locals.cursor++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor(locals.cursor++, 0xFF, 0xFF, 0xFF, 0);
    gDPPipeSync(locals.cursor++);

    camSetNo(0);
    camSetView(&locals.cursor, &locals.other);
    gDPPipeSync(locals.cursor++);

    trackGetFog(0, &locals.low, &locals.high, &locals.unused,
                           &locals.red, &locals.green, &locals.blue,
                           &locals.alpha);
    gDPSetFogColor(locals.cursor++, locals.red, locals.green, locals.blue,
                   0xFF);

    {
        Gfx *fog = locals.cursor++;
        s32 range;

        fog->words.w0 = 0xBC000008;
        range = locals.high - locals.low;
        fog->words.w1 = _SHIFTL(128000 / range, 16, 16) |
                        _SHIFTL(((500 - locals.low) * 256) / range, 0, 16);
        sprSetTextureFilter(0);
    }
    gMoveWd(locals.cursor++, 10, 0, 0);
    gDPFullSync(locals.cursor++);
    gSPEndDisplayList(locals.cursor++);

    gOverlay33DisplayLists[D_800D1C38] = locals.cursor;
    sprSetTextureFilter(1);
}
