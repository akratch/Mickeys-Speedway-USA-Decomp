#include "overlays/overlay_062.h"

/* Tier B: separate frame/flags, fade state and resource handle cells.
 * Original declarations and names remain Tier D; no native/API closure. */
f32 o62AnimationFrame;
s32 o62AnimationFlags;
s32 o62FadeInTicks;
s32 o62FadeOutTicks;
s32 o62TransitionState;
void *o62Descriptor;
void *o62Sprite;

/* DKR v77/v80 and JFG have no exact donor for this resource initializer. */
void overlay62Initialize(void) {
    loadFrontEndList(&gOverlay62Float0);
    gOverlay62Value4Initial = (s32)gOverlay62Handle18;
    o62AnimationFrame_o062Reloc = 0.0f;
    o62AnimationFlags_o062Reloc = 13;
    fontUseFont(3);
    fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
    o62Descriptor_o062Reloc = overlay45CreateDescriptor(
        D_8007C0B8->primary, 0x28, 0x28, 0);
    o62Sprite_o062Reloc = texLoadSprite(0x93, 0);
    o62FadeInTicks_o062Reloc = 0x20;
    o62FadeOutTicks_o062Reloc = 0;
    o62TransitionState_o062Reloc = 0;
    overlay45SetMode(o62Descriptor_o062Reloc, 0);
}

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

    if (o62FadeInTicks_o062Reloc > 0) {
        o62FadeInTicks_o062Reloc -= updateRate;
        if (o62FadeInTicks_o062Reloc < 0) {
            o62FadeInTicks_o062Reloc = 0;
        }

        alpha = 0xFF - o62FadeInTicks_o062Reloc * 8;
        screenBase = 0xF8;
        intensity = 0x78 + ((o62FadeInTicks_o062Reloc * 0xDC) >> 5);
        red = 0x40 + ((o62FadeInTicks_o062Reloc * -0x40) >> 5);
        green = 0x40 + ((o62FadeInTicks_o062Reloc * -0x40) >> 5);
        blue = 0x80 + ((o62FadeInTicks_o062Reloc * -0x80) >> 5);

        overlay45SetMode(o62Descriptor_o062Reloc, alpha);
        rcpSetScreenColour(red & 0xFF, green & 0xFF, blue & 0xFF,
                                 green);
    } else if ((o62FadeOutTicks_o062Reloc != 0) || (o62TransitionState_o062Reloc != 0)) {
        o62FadeOutTicks_o062Reloc -= updateRate;
        if (o62FadeOutTicks_o062Reloc <= 0) {
            o62FadeOutTicks_o062Reloc = 0;
            mainChangeLevel(0xC, 0, 0, 4, 1, 0);
            o62TransitionState_o062Reloc = 1;
        }

        alpha = o62FadeOutTicks_o062Reloc * 8;
        screenBase = 0xF8;
        intensity = 0x154 + ((o62FadeOutTicks_o062Reloc * -0xDC) >> 5);
        red = (o62FadeOutTicks_o062Reloc << 6) >> 5;
        green = (o62FadeOutTicks_o062Reloc << 6) >> 5;
        blue = (o62FadeOutTicks_o062Reloc << 7) >> 5;
    } else {
        alpha = 0xFF;
        screenBase = 0xF8;
        intensity = 0x78;
        red = 0x40;
        green = 0x40;
        blue = 0x80;

        if ((D_800D3180 & 0x9000) != 0) {
            o62FadeOutTicks_o062Reloc = 0x20;
        }
    }

    rcpSetScreenColour(red & 0xFF, green & 0xFF, blue & 0xFF, green);
    overlay45SetMode(o62Descriptor_o062Reloc, alpha);
    camStandardOrtho(&D_800D3140,
                                 &D_800D3144);

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
                             &D_800D3144,
                             D_800D3148, &transform,
                             o62Sprite_o062Reloc, 0, 0xFF);
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
                              &o62AnimationFlags_o062Reloc, 2,
                              &o62AnimationFrame_o062Reloc, updateRate);
    gOverlay62Values[2] = (s32)(o62AnimationFrame_o062Reloc * 65536.0f);
    func_8002F618(&D_800D3140,
                            gOverlay62Values, 0x74, 0xCC,
                            0xFF, 0xFF, 0xFF, alpha);
}

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay62ReleaseAll(void) {
    overlay62ReleaseReloc(o62Sprite_o062Reloc);
    overlay62ReleaseReloc(o62Descriptor_o062Reloc);
    overlay62ReleaseReloc(gOverlay62InlineResource);
    func_8002C5F4();
}
