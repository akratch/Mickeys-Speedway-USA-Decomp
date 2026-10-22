#include "PR/ultratypes.h"

typedef struct Overlay62Gfx {
    u32 word0;
    u32 word1;
} Overlay62Gfx;

typedef struct Overlay62Root {
    u8 pad00[0x25C];
    void *primary;
    void *items[6];
} Overlay62Root;

typedef struct Overlay62Transform {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[2];
    f32 scale;
    f32 screenX;
    f32 screenY;
    f32 zero0;
    u8 pad18[0x10];
    f32 zero1;
} Overlay62Transform;

extern f32 gOverlay62PulseReloc;
extern s32 D_800D3180;
extern u8 gOverlay62State[];
extern s32 gOverlay62Values[];
extern s32 gOverlay62Value8;
extern s32 gOverlay62ValueC;
extern Overlay62Root *D_8007C0B8;
extern Overlay62Gfx *D_800D3140;
extern u8 D_800D3144[];
extern u8 D_800D3148[];
extern u8 gOverlay62FirstItemReloc[];
extern u8 gOverlay62CommitContextReloc[];

#define gOverlay62Value4 gOverlay62Values[0]
#define gOverlay62Value10 (*(s32 *)(gOverlay62State + 0x10))
#define gOverlay62Handle14 (*(void **)(gOverlay62State + 0x14))
#define gOverlay62Handle18 (*(void **)(gOverlay62State + 0x18))

extern void overlay45SetMode(void *handle, s32 alpha);
extern void rcpSetScreenColour(s32 red, s32 green, s32 blue, s32 alpha);
extern void mainChangeLevel(s32 type, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5);
extern void camStandardOrtho(Overlay62Gfx **commands,
                                         void *viewport);
extern void sprSetTextureFilter(s32 layer);
extern void camDo2DSprite(Overlay62Gfx **commands, void *resourceA,
                                     void *resourceB,
                                     Overlay62Transform *transform,
                                     void *handle, s32 arg5, s32 alpha);
extern void sprSetTextureFilter(s32 mode);
extern void fontUseFont(s32 mode);
extern void fontBackground(s32 a, s32 b, s32 c, s32 d);
extern void fontColour(s32 red, s32 green, s32 blue,
                                            s32 alpha, s32 intensity);
extern void fontPrintXY(Overlay62Gfx **commands, s32 x, s32 y,
                                   void *item, s32 mode);
extern void texAnimateTexSprite(s32 handle, s32 *state, s32 mode,
                                      void *context, s32 updateRate);
extern void func_8002F618(Overlay62Gfx **commands, s32 *state,
                                    s32 x, s32 y, s32 red, s32 green,
                                    s32 blue, s32 alpha);

#define OVERLAY62_EMIT(commands_, op_, value_) do { \
    Overlay62Gfx *packet_ = *(commands_); \
    *(commands_) = packet_ + 1; \
    packet_->word0 = (op_); \
    packet_->word1 = (value_); \
} while (0)

/* Exact C: 294 words, the 0x88 frame, and all 71 relocation records.
 *
 * Two spellings are load-bearing and each was proved against the object.
 * First, the two fading colour components are *multiplications* by negative
 * constants, not shifts of a negated operand. The target negates the timer
 * into the assembler temporary, and `ugen` never allocates that register
 * outside its own `.set noat` sequences, so the word can only come from the
 * `as1` expansion of a constant multiply -- and two such expansions in one
 * block share a single negation into it.
 *
 * Second, `red` and `green` are two separate full expressions, not one chained
 * assignment. A chained `red = green = X` leaves the pair a single web, so
 * `red & 0xFF` reads back green's reloaded copy, `blue` takes a colour ahead
 * of green's instead of behind it, and the whole colour lane rotates for the
 * rest of the function. Writing each component out gives `red` its own pool
 * colour and puts `blue` after `green` in colour order.
 *
 * `screenBase` keeps a `volatile` qualifier it does not need semantically: the
 * value is the same constant on all three paths and IDO folds it away without
 * the qualifier, costing six words. Recorded in docs/cleanup-queue.md as a
 * naturalization candidate. */
void overlay62Update(s32 updateRate) {
    s32 alpha;
    volatile s32 screenBase;
    s32 intensity;
    s32 red;
    s32 green;
    s32 blue;
    Overlay62Transform transform;

    if (gOverlay62Value8 > 0) {
        gOverlay62Value8 -= updateRate;
        if (gOverlay62Value8 < 0) {
            gOverlay62Value8 = 0;
        }

        alpha = 0xFF - gOverlay62Value8 * 8;
        screenBase = 0xF8;
        intensity = 0x78 + ((gOverlay62Value8 * 0xDC) >> 5);
        red = 0x40 + ((gOverlay62Value8 * -0x40) >> 5);
        green = 0x40 + ((gOverlay62Value8 * -0x40) >> 5);
        blue = 0x80 + ((gOverlay62Value8 * -0x80) >> 5);

        overlay45SetMode(gOverlay62Handle14, alpha);
        rcpSetScreenColour(red & 0xFF, green & 0xFF, blue & 0xFF,
                                 green);
    } else if ((gOverlay62ValueC != 0) || (gOverlay62Value10 != 0)) {
        gOverlay62ValueC -= updateRate;
        if (gOverlay62ValueC <= 0) {
            gOverlay62ValueC = 0;
            mainChangeLevel(0xC, 0, 0, 4, 1, 0);
            gOverlay62Value10 = 1;
        }

        alpha = gOverlay62ValueC * 8;
        screenBase = 0xF8;
        intensity = 0x154 + ((gOverlay62ValueC * -0xDC) >> 5);
        red = (gOverlay62ValueC << 6) >> 5;
        green = (gOverlay62ValueC << 6) >> 5;
        blue = (gOverlay62ValueC << 7) >> 5;
    } else {
        alpha = 0xFF;
        screenBase = 0xF8;
        intensity = 0x78;
        red = 0x40;
        green = 0x40;
        blue = 0x80;

        if ((D_800D3180 & 0x9000) != 0) {
            gOverlay62ValueC = 0x20;
        }
    }

    rcpSetScreenColour(red & 0xFF, green & 0xFF, blue & 0xFF, green);
    overlay45SetMode(gOverlay62Handle14, alpha);
    camStandardOrtho(&D_800D3140,
                                 D_800D3144);

    OVERLAY62_EMIT(&D_800D3140, 0xE7000000, 0);
    OVERLAY62_EMIT(&D_800D3140, 0xFA000000, 0xFFFFFFFF);
    OVERLAY62_EMIT(&D_800D3140, 0xFB000000, 0xFFFFFF00);

    transform.x = 0;
    transform.y = 0;
    transform.z = 0;
    transform.screenX = (f32)(screenBase - 0xA0);
    transform.screenY = (f32)(0x78 - intensity);
    transform.scale = 1.0f;
    transform.zero0 = 0.0f;
    transform.zero1 = 0.0f;

    sprSetTextureFilter(0);
    camDo2DSprite(&D_800D3140,
                             D_800D3144,
                             D_800D3148, &transform,
                             gOverlay62Handle18, 0, 0xFF);
    sprSetTextureFilter(1);
    fontUseFont(2);
    fontBackground(0, 0, 0, 0);
    fontColour(0xFF, 0xFF, 0, 0xFF, alpha);

    fontPrintXY(&D_800D3140, 0xC4, 0x28,
                           gOverlay62FirstItemReloc, 4);
    fontPrintXY(&D_800D3140, 0x74, 0x50,
                           D_8007C0B8->items[0], 4);
    fontPrintXY(&D_800D3140, 0x74, 0x5A,
                           D_8007C0B8->items[1], 4);
    fontPrintXY(&D_800D3140, 0x74, 0x64,
                           D_8007C0B8->items[2], 4);
    fontPrintXY(&D_800D3140, 0x74, 0x6E,
                           D_8007C0B8->items[3], 4);
    fontPrintXY(&D_800D3140, 0x74, 0x78,
                           D_8007C0B8->items[4], 4);
    fontPrintXY(&D_800D3140, 0x74, 0xB4,
                           D_8007C0B8->items[5], 4);

    texAnimateTexSprite(*(s32 *)(gOverlay62State + 4),
                              gOverlay62Values, 2,
                              gOverlay62CommitContextReloc, updateRate);
    gOverlay62Values[2] = (s32)(gOverlay62PulseReloc * 65536.0f);
    func_8002F618(&D_800D3140,
                            gOverlay62Values, 0x74, 0xCC,
                            0xFF, 0xFF, 0xFF, alpha);
}
