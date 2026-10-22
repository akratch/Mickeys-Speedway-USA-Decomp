#include "PR/ultratypes.h"

typedef struct Overlay57Draw32A0Record {
    s32 word0;
    s32 word4;
    u32 packed8;
} Overlay57Draw32A0Record;

extern s32 gO57Draw32A0Available4CC;
extern s32 gO57Draw32A0Selection1A0;
extern s32 gO57Draw32A0State118;
extern s32 gO57Draw32A0Envelope124;
extern s32 gO57Draw32A0TableMode18C[];
extern u32 gO57Draw32A0WordTable4B8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records3F8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records458[];
extern Overlay57Draw32A0Record gO57Draw32A0Work34C[];
extern u32 gO57Draw32A0Masks3C8[];
extern u32 gO57Draw32A0Shifts3D8[];
extern u8 gO57Draw32A0Local08[];
extern u8 gO57Draw32A0Local18[];
extern u8 gO57Draw32A0Local28[];

extern s32 D_8007C1A0;
extern u8 D_800D3140[];
extern u8 gO57Draw32A0StripeReloc[];
extern u16 D_800D3128[];

extern s32 overlay84GetCurrent(void);
extern s32 overlay84IsUnitScale(void);
extern void frontDrawRectangles(void *anchor, s32 count,
                                  Overlay57Draw32A0Record *records, s32 mode);
extern void fontUseFont(s32 mode);
extern void fontColour(s32 red, s32 green, s32 blue, s32 alpha,
                                  s32 envelope);
extern void fontPrintXY(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void fontPrintXY(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void fontPrintXY(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void fontPrintXY(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void func_8002FB34(void *anchor,
                                   Overlay57Draw32A0Record *record,
                                   f32 scale, f32 position, f32 xScale,
                                   f32 yScale, s32 color, s32 command);

/* Overlay 57 text +0x32A0..+0x35E0. Exact: 208 words, frame 0x70, 53/53
 * relocation identities.
 *
 * Both envelope arms are compound assignments through the global rather than
 * a local assigned and then stored. Written the other way, uopt materialises
 * the value a second time -- the saturating arm hoists a duplicate `li 254`
 * into the guard's delay slot, and the decay arm keeps a separate carrier --
 * and each duplicate rotates the whole block-local temp ring behind it.
 *
 * The mask and shift tables are indexed by the loop counter. Declared as
 * pointer cursors they reserve two frame cells the target does not have,
 * which is the 8 bytes by which the frame used to exceed 0x70. */
void overlay57Draw32A0(s32 updateRate) {
    Overlay57Draw32A0Record *records;
    Overlay57Draw32A0Record *cursor;
    s32 envelope;
    s32 rowCount[1];
    s32 position;
    s32 i;
    s32 rising;

    if (gO57Draw32A0Available4CC == 0) {
        return;
    }

    rising = 0;
    if ((D_8007C1A0 == 1) &&
        (overlay84GetCurrent() < 5) &&
        (overlay84IsUnitScale() != 0)) {
        gO57Draw32A0Selection1A0 = overlay84GetCurrent();
        if (gO57Draw32A0State118 == 1) {
            rising = 1;
        }
    }

    if (rising != 0) {
        envelope = (gO57Draw32A0Envelope124 += updateRate * 8);
        if (envelope >= 0xFF) {
            gO57Draw32A0Envelope124 = 0xFE;
            envelope = gO57Draw32A0Envelope124;
        }
    } else {
        envelope = (gO57Draw32A0Envelope124 =
                    gO57Draw32A0Envelope124 - updateRate * 32);
        if (envelope < 0) {
            gO57Draw32A0Envelope124 = 0;
            return;
        }
    }

    if (gO57Draw32A0TableMode18C[0] != 0) {
        rowCount[0] = 4;
        records = gO57Draw32A0Records3F8;
        position = 0x3E;
    } else {
        rowCount[0] = 3;
        records = gO57Draw32A0Records458;
        position = 0x4D;
    }

    rising = gO57Draw32A0WordTable4B8[gO57Draw32A0Selection1A0] |
             ((envelope * 5) >> 3);
    i = 1;
    cursor = &records[1];
    do {
        i++;
        cursor->packed8 = rising;
        cursor++;
    } while (i < 8);

    frontDrawRectangles(D_800D3140, 8, records, 0);
    fontUseFont(2);
    fontColour(0xFF, 0xFF, 0xFF, 0xFF,
                          gO57Draw32A0Envelope124);
    fontPrintXY(D_800D3140, 0xF8, position,
                            gO57Draw32A0StripeReloc, 4);
    position += 0x1E;
    fontPrintXY(D_800D3140, 0xF8, position,
                            gO57Draw32A0Local08, 4);
    position += 0x1E;
    fontPrintXY(D_800D3140, 0xF8, position,
                            gO57Draw32A0Local18, 4);
    position += 0x1E;
    if (gO57Draw32A0TableMode18C[0] != 0) {
        fontPrintXY(D_800D3140, 0xF8, position,
                                gO57Draw32A0Local28, 4);
    }

    i = 0;
    position -= 0x48;
    if (rowCount[0] > 0) {
        do {
            gO57Draw32A0Work34C[0].packed8 =
                ((D_800D3128[4 + gO57Draw32A0Selection1A0] &
                  gO57Draw32A0Masks3C8[i]) >> gO57Draw32A0Shifts3D8[i]) << 16;
            func_8002FB34(D_800D3140,
                                   gO57Draw32A0Work34C, 248.0f,
                                   (f32)position, 1.0f, 1.0f,
                                   gO57Draw32A0Envelope124 | -256, 0x2003);
            i++;
            position += 0x1E;
        } while (i != rowCount[0]);
    }
}
